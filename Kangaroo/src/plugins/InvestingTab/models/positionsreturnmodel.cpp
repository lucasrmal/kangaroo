#include "positionsreturnmodel.h"
#include "portfolio.h"

#include <KangarooLib/model/security.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/transaction.h>
#include <KangarooLib/model/investmenttransaction.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/modelexception.h>
#include <cmath>
#include <QDebug>

using namespace KLib;

PositionsReturnModel::PositionsReturnModel(Portfolio* _portfolio, QObject* _parent) :
    QAbstractTableModel(_parent),
    m_portfolio(_portfolio),
    m_reportDate(QDate::currentDate()),
    m_periodReturns(4*TOTAL_YEAR_RANGE),
    m_annualReturns(TOTAL_YEAR_RANGE),
    m_values(4*TOTAL_YEAR_RANGE)
{
    computeReturns();

    connect(m_portfolio, &Portfolio::positionAdded,             this, &PositionsReturnModel::onPositionAdded);
    connect(m_portfolio, &Portfolio::positionRemoved,           this, &PositionsReturnModel::onPositionRemoved);
    connect(m_portfolio, &Portfolio::portfolioSecurityModified, this, &PositionsReturnModel::onPortfolioDataChanged);
    connect(m_portfolio, &Portfolio::positionDataChanged,       this, &PositionsReturnModel::onPortfolioDataChanged);
    connect(m_portfolio, &Portfolio::securityPriceModified,     this, &PositionsReturnModel::onPortfolioDataChanged);
}

void PositionsReturnModel::setReportDate(const QDate& _date)
{
    if (_date.isValid() && _date != m_reportDate)
    {
        m_reportDate = _date;
        computeReturns();
        emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
    }
}

QVariant PositionsReturnModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid() || _index.row() >= rowCount())
        return QVariant();

    switch (_role)
    {
    case Qt::DisplayRole:
        return Portfolio::formatPercChange(m_returns[_index.row()]);

    case Qt::EditRole:
        return m_returns[_index.row()];

    default:
        return QVariant();
    }
}

QVariant PositionsReturnModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{    
    if (_role != Qt::DisplayRole)
        return QVariant();

    if (_orientation == Qt::Vertical)
    {
        switch (_section)
        {
        case ReturnPeriod::ThreeMonths:
            return tr("3 months");

        case ReturnPeriod::SixMonths:
            return tr("6 months");

        case ReturnPeriod::OneYear:
            return tr("1 year");

        case ReturnPeriod::ThreeYears:
            return tr("3 years");

        case ReturnPeriod::FiveYears:
            return tr("5 years");
        }
    }
    else if (_orientation == Qt::Horizontal)
    {
        return m_portfolio->portfolioName();
    }

    return QVariant();
}

int PositionsReturnModel::rowCount(const QModelIndex&) const
{
    return ReturnPeriod::NUM_PERIODS;
}

int PositionsReturnModel::columnCount(const QModelIndex&) const
{
    return 1;
}

void PositionsReturnModel::onPositionAdded(Security* _sec)
{
    int row = m_portfolio->indexOf(_sec);

    if (row != -1)
    {
        onPortfolioDataChanged();
    }
}

void PositionsReturnModel::onPositionRemoved(Security*, int)
{
    onPortfolioDataChanged();
}

void PositionsReturnModel::onPortfolioDataChanged()
{
    computeReturns();
    emit dataChanged(index(0,0), index(rowCount()-1, columnCount()-1));
}

void PositionsReturnModel::computeReturns()
{
    /*
     * We want to compute the returns for each 3-months period. These periods may be
     * further split in sub-periods depending on inflows/outflows.
     *
     * For this, look at all the accounts in the portfolio and add "splits" at every buy/sell.
     * Reinvested dividends/distributions are ignored. Dividends/distributions are added at the
     * end of each period.
     *
     * After this, we compound the periods into the larger periods: 6 months, 1 year, etc.
     */

    try
    {        
        const QDate startDate       = m_reportDate.addYears(-TOTAL_YEAR_RANGE);

        QMap<QDate, Amount>   dividends;
        QMap<QDate, Amount>   flows;
        QHash<int, Account*>  accountCache;
        QHash<int, Security*> securityCache;

        /********************************* COMPUTE FLOWS *********************************/

        auto inCurrency = [this] (const Transaction::Split& s, InvestmentTransaction* invtr)
        {
            return s.amount * PriceManager::instance()->rate(s.currency, m_portfolio->currency()->code(), invtr->date());
        };

        auto valueAt = [this, &securityCache] (const Transaction::Split& s, InvestmentTransaction* invtr)
        {
            return s.amount * PriceManager::instance()->rate(securityCache[s.idAccount]->id(),
                                                             securityCache[s.idAccount]->currency(),
                                                             invtr->date())
                            * PriceManager::instance()->rate(securityCache[s.idAccount]->currency(),
                                                             m_portfolio->currency()->code(),
                                                             invtr->date());
        };

        /* Scan transactions to find sub-period dates */
        for (int id : m_portfolio->allAccountsInTree())
        {
            accountCache[id]  = Account::getTopLevel()->account(id);
            securityCache[id] = SecurityManager::instance()->get(accountCache[id]->idSecurity());

            auto transactions = accountCache[id]->ledger()->transactionsBetween(startDate, m_reportDate);

            for (Transaction* t : transactions)
            {
                InvestmentTransaction* invtr = qobject_cast<InvestmentTransaction*>(t);

                if (invtr)
                {
                    switch (invtr->action())
                    {
                    case InvestmentAction::Buy:     //Always opposite of cash balance, hence -
                    case InvestmentAction::Sell:
                    case InvestmentAction::ShortSell: // **May not be correct!!!**
                    case InvestmentAction::ShortCover:
                        flows[invtr->date()] -= inCurrency(invtr->splitFor(InvestmentSplitType::CostProceeds), invtr);
                        break;

                    case InvestmentAction::Transfer:
                    case InvestmentAction::Swap:
                        if (invtr->splitFor(InvestmentSplitType::InvestmentFrom).idAccount == id) //Outflow
                        {
                            flows[invtr->date()] += valueAt(invtr->splitFor(InvestmentSplitType::InvestmentFrom), invtr);
                        }
                        else //Inflow
                        {
                            flows[invtr->date()] += valueAt(invtr->splitFor(InvestmentSplitType::InvestmentTo), invtr);
                        }
                        break;

                    case InvestmentAction::Dividend:        // Coming **from** source, hence -
                    case InvestmentAction::Distribution:
                        dividends[invtr->date()] -= inCurrency(invtr->splitFor(InvestmentSplitType::DistributionSource), invtr);
                        break;

                    case InvestmentAction::ReinvestDiv:
                    case InvestmentAction::ReinvestDistrib:
                        if (invtr->hasSplitFor(InvestmentSplitType::CashInLieu))
                        {
                            dividends[invtr->date()] += inCurrency(invtr->splitFor(InvestmentSplitType::CashInLieu), invtr);
                        }
                        break;

                    default:
                         break;
                    }
                }
            }
        }

        /* Remove zero-sum flows if they exist */
        auto flowIt = flows.begin();

        while (flowIt != flows.end())
        {
            if (flowIt.value() == 0)
            {
                flowIt = flows.erase(flowIt);
            }
            else
            {
                ++flowIt;
            }
        }


        /********************************* COMPUTE RETURNS *********************************/


        /* Compute the returns for each 3-months */
        QDate periodStart = startDate;
        QDate periodEnd = startDate.addMonths(3);

        // Reset the flows iterator
        flowIt = flows.begin();

        // Get a dividends iteraror
        auto divIt = dividends.begin();

        //Clear the values cache
        QHash<QDate, Amount> valuesTemp;

        /* This is the value **after** all day's transactions have been posted. */
        auto portfolioValueAt = [this, &accountCache, &securityCache, &valuesTemp] (const QDate& _date)
        {
            if (!valuesTemp.contains(_date))
            {
                Amount total = 0;

                for (Account* a : accountCache)
                {
                    total += a->ledger()->balanceAt(_date)
                             * PriceManager::instance()->rate(a->idSecurity(), securityCache[a->id()]->currency(), _date)
                             * PriceManager::instance()->rate(securityCache[a->id()]->currency(), m_portfolio->currency()->code(), _date);
                }

                valuesTemp.insert(_date, total);
            }

            return valuesTemp[_date];
        };

        for (int i = 0; i < 4*TOTAL_YEAR_RANGE; ++i)
        {
            bool hasAnyReturn = false;
            double intraPeriodReturns = 1.0;
            QDate intraStart = periodStart;
            QDate intraEnd = flowIt == flows.end() ? periodEnd
                                                   : std::min(flowIt.key(), periodEnd);

            while (intraEnd <= periodEnd)
            {
                //Find the current flow (0 if simply period end with no flow)
                Amount curFlow = flowIt != flows.end() && flowIt.key() == intraEnd ? *flowIt
                                                                                   : 0;

                Amount startValue = portfolioValueAt(intraStart);
                Amount endValue = portfolioValueAt(intraEnd);

                //Add any dividends
                while (divIt != dividends.end() && divIt.key() <= intraEnd)
                {
                    endValue += divIt.value();
                    ++divIt;
                }

                if (startValue != 0)
                {
                    hasAnyReturn = true;
                    intraPeriodReturns *= ((endValue - curFlow).toDouble()
                                              / startValue.toDouble());
                }

                if (curFlow != 0)
                {
                    ++flowIt;
                }

                if (intraEnd == periodEnd)
                    break;

                intraStart = intraEnd;
                intraEnd = flowIt == flows.end() ? periodEnd
                                                 : std::min(flowIt.key(), periodEnd);
            }

            //Save the return and end value, then go to the next period
            m_periodReturns[i] = Return(intraPeriodReturns - 1.0, hasAnyReturn);
            m_values[i]        = QPair<QDate, Amount>(periodEnd, portfolioValueAt(periodEnd));

            periodStart = periodEnd;
            periodEnd = periodEnd.addMonths(3);
        }

        //Compounding
        typedef QVector<Return>::const_iterator ReturnIterator;

        auto compound = [] (ReturnIterator begin, ReturnIterator end, double years)
        {
            Return r(1.0, false);
            int total = 0, used = 0;

            for (ReturnIterator i = begin; i != end; ++i)
            {
                if (i->second) //We do not include periods in which no amount were invested.
                {
                    r.first *= (1.0 + i->first);
                    r.second = true;
                    ++used;
                }

                ++total;
            }

            r.first -= 1.0;

            //Adjust the fraction of years
            if (used != total)
            {
                years *= (double(used)/double(total));
            }

            //Now annualize (only if at least any of them was good)
            if (r.second)
            {
                r.first = std::pow((r.first + 1.0), 1.0/years) - 1.0;
            }

            return r;
        };

        //Compound the annual returns
        int year = 0;
        for (auto i = m_periodReturns.begin(); i != m_periodReturns.end(); i+=4, ++year)
        {
            m_annualReturns[year] = compound(i, i+4, 1.0);
        }

        //Now compute for the specified periods                  **We do not want to annualize 3 and 6 months**
        m_returns[ReturnPeriod::ThreeMonths] = compound(m_periodReturns.end()-1, m_periodReturns.end(), 1.0).first;
        m_returns[ReturnPeriod::SixMonths]   = compound(m_periodReturns.end()-2, m_periodReturns.end(), 1.0).first;
        m_returns[ReturnPeriod::OneYear]     = m_annualReturns.last().first;
        m_returns[ReturnPeriod::ThreeYears]  = compound(m_annualReturns.end()-3, m_annualReturns.end(), 3.0).first;
        m_returns[ReturnPeriod::FiveYears]   = compound(m_annualReturns.begin(), m_annualReturns.end(), 5.0).first;
    }
    catch (ModelException e)
    {
        qDebug() << tr("An error occured while computing investment returns: %1").arg(e.description());

        for (int i = 0; i < ReturnPeriod::NUM_PERIODS; ++i)
        {
            m_returns[i] = 0.0;
        }
    }
}

/////////////////////////////////////////// DETAILED MODEL //////////////////////////////////////////////////

DetailedPositionsReturnModel::DetailedPositionsReturnModel(RangeType _type, PositionsReturnModel* _returnModel, QObject* _parent) :
    QAbstractTableModel(_parent),
    m_type(_type),
    m_returnModel(_returnModel)
{
    connect(m_returnModel, &PositionsReturnModel::dataChanged, this, &DetailedPositionsReturnModel::dataChanged);
}

int DetailedPositionsReturnModel::rowCount(const QModelIndex&) const
{
    if (m_type == RangeType::Annual)
    {
        return m_returnModel->m_annualReturns.size();
    }
    else
    {
        return m_returnModel->m_periodReturns.size();
    }
}

int DetailedPositionsReturnModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant DetailedPositionsReturnModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid() || _index.row() >= rowCount())
        return QVariant();

    switch (_role)
    {
    case Qt::DisplayRole:
        return Portfolio::formatPercChange(m_type == RangeType::Annual ? m_returnModel->m_annualReturns[_index.row()].first
                                                                       : m_returnModel->m_periodReturns[_index.row()].first);

    case Qt::EditRole:
        return m_type == RangeType::Annual ? m_returnModel->m_annualReturns[_index.row()].first
                                           : m_returnModel->m_periodReturns[_index.row()].first;

    default:
        return QVariant();
    }
}

QVariant DetailedPositionsReturnModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (_role != Qt::DisplayRole)
        return QVariant();

    if (_orientation == Qt::Vertical)
    {
        if (m_type == RangeType::Annual)
        {
            return m_returnModel->reportDate().addYears(-(rowCount()-1-_section)).toString("MMM yyyy");
        }
        else
        {
            return m_returnModel->reportDate().addMonths(-3*(rowCount()-1-_section)).toString("MMM yyyy");
        }

    }
    else if (_orientation == Qt::Horizontal)
    {
        return m_returnModel->m_portfolio->portfolioName();
    }
    else
    {
        return QVariant();
    }
}

/////////////////////////////////////////// VALUE MODEL //////////////////////////////////////////////////

PositionsValueModel::PositionsValueModel(PositionsReturnModel* _returnModel, QObject* _parent) :
    QAbstractTableModel(_parent),
    m_returnModel(_returnModel)
{
    connect(m_returnModel, &PositionsReturnModel::dataChanged, this, &DetailedPositionsReturnModel::dataChanged);
}

int PositionsValueModel::rowCount(const QModelIndex&) const
{
    return m_returnModel->m_values.size();
}

int PositionsValueModel::columnCount(const QModelIndex&) const
{
    return 1;
}

QVariant PositionsValueModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid() || _index.row() >= rowCount())
        return QVariant();

    switch (_role)
    {
    case Qt::DisplayRole:
        return m_returnModel->m_portfolio->currency()->formatAmount(m_returnModel->m_values.at(_index.row()).second);

    case Qt::EditRole:
        return m_returnModel->m_values.at(_index.row()).second.toDouble();

    default:
        return QVariant();
    }
}

QVariant PositionsValueModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (_role != Qt::DisplayRole)
        return QVariant();

    if (_orientation == Qt::Vertical)
    {
        return m_returnModel->m_values.at(_section).first.toString("MMM yyyy");
    }
    else if (_orientation == Qt::Horizontal)
    {
        return QString("%1 (%2)").arg(m_returnModel->m_portfolio->portfolioName())
                                 .arg(m_returnModel->m_portfolio->currency()->code());
    }
    else
    {
        return QVariant();
    }
}



