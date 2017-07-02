#include "dividendsmodel.h"
#include "portfolio.h"
#include <KangarooLib/model/security.h>
#include <KangarooLib/model/transaction.h>
#include <KangarooLib/model/investmenttransaction.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/pricemanager.h>
#include <algorithm>

using namespace KLib;

DividendsModel::DividendsModel(const QDate& _from, const QDate& _to, Portfolio* _portfolio, QObject* _parent) :
    QAbstractTableModel(_parent),
    m_portfolio(_portfolio),
    m_dateFrom(_from),
    m_dateTo(_to)
{
    refresh();

//    connect(m_portfolio, &Portfolio::positionAdded, this, &DividendsModel::addPosition);
//    connect(m_portfolio, &Portfolio::positionRemoved, this, &DividendsModel::reloadSecurityData);
//    connect(m_portfolio, &Portfolio::parentCurrencyPriceModified, this, &DividendsModel::reloadAllPositions);
//    connect(m_portfolio, &Portfolio::portfolioSecurityModified, this, &DividendsModel::reloadSecurityData);
//    connect(m_portfolio, &Portfolio::positionDataChanged, this, &DividendsModel::reloadSecurityData);
//    connect(m_portfolio, &Portfolio::securityPriceModified, this, &DividendsModel::reloadSecurityData);
}

QVariant DividendsModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid() || _index.row() >= rowCount()
        || (_role != Qt::DisplayRole && _role != Qt::EditRole))
        return QVariant();

    const PositionDividend& div = m_dividendPositions[_index.row()];

    try
    {
        switch (_index.column())
        {
        case DividendsModelColumn::Name:
            return SecurityManager::instance()->get(div.idSecurity)->name();

        case DividendsModelColumn::Symbol:
            return SecurityManager::instance()->get(div.idSecurity)->symbol();

        case DividendsModelColumn::DividendsReceived:
            return _role == Qt::EditRole ? QVariant(div.received.toDouble())
                                         : QVariant(m_portfolio->currency()->formatAmount(div.received));

        case DividendsModelColumn::DividendsPercent:
            return _role == Qt::EditRole ? QVariant(div.percent)
                                         : QVariant(Portfolio::formatPercChange(div.percent));

        default:
            return QVariant();
        }
    }
    catch (...) { return QVariant(); }
}

QVariant DividendsModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (_orientation == Qt::Horizontal && _role == Qt::DisplayRole)
    {
        switch (_section)
        {
        case DividendsModelColumn::Name:
            return tr("Name");

        case DividendsModelColumn::Symbol:
            return tr("Symbol");

        case DividendsModelColumn::DividendsReceived:
            return tr("Dividends");

        case DividendsModelColumn::DividendsPercent:
            return tr("% of Total");
        }
    }

    return QVariant();
}

void DividendsModel::setInterval(const QPair<QDate, QDate>& _interval)
{
    m_dateFrom = _interval.first;
    m_dateTo = _interval.second;
    refresh();
}

void DividendsModel::refresh()
{
    beginResetModel();

    m_dividendPositions.clear();
    double total = 0.0;
    QHash<int, int> securityIndex;
    QMap<QDate, KLib::Amount> tempDividends;

    QDate firstDateFound, lastDateFound;

    for (int idAccount : m_portfolio->allAccountsInTree())
    {
        Account* a = Account::getTopLevel()->account(idAccount);

        if (!securityIndex.contains(a->idSecurity()))
        {
            securityIndex[a->idSecurity()] = m_dividendPositions.size();
            m_dividendPositions << PositionDividend(a->idSecurity());
        }

        PositionDividend& pos = m_dividendPositions[securityIndex[a->idSecurity()]];

        //pos.relatedAccounts.insert(idAccount);

        auto transactions = a->ledger()->transactionsBetween(m_dateFrom, m_dateTo);

        for (Transaction* t : transactions)
        {
            InvestmentTransaction* itr = qobject_cast<InvestmentTransaction*>(t);

            if (itr)
            {
                switch (itr->action())
                {
                case InvestmentAction::Dividend:
                case InvestmentAction::Distribution:
                case InvestmentAction::ReinvestDiv:
                case InvestmentAction::ReinvestDistrib:
                {
                    const Transaction::Split& s = itr->splitFor(InvestmentSplitType::DistributionSource);

                    Amount div = -s.amount //neg. since credit from income account
                                 * PriceManager::instance()->rate(s.currency, m_portfolio->currency()->code());

                    pos.received += div;
                    tempDividends[itr->date()] += div;
                    total += div.toDouble();

                    if (!firstDateFound.isValid())
                    {
                        lastDateFound = firstDateFound = itr->date();
                    }

                    lastDateFound = std::max(lastDateFound, itr->date());

                    break;
                }
                default:
                    break;
                }
            }
        }
    }

    //Clear empty positions and compute percentages
    auto i = m_dividendPositions.begin();

    while (i != m_dividendPositions.end())
    {
        if (i->received == 0)
        {
            i = m_dividendPositions.erase(i);
        }
        else
        {
            i->percent = i->received.toDouble() / total;
            ++i;
        }
    }

    //Compute dividends per period
    m_dividendsOverTime.clear();

    //If at least one transaction was found
    if (firstDateFound.isValid())
    {
        int daysBetween = firstDateFound.daysTo(lastDateFound);

        //Find which type of interval to use
        if (daysBetween >= 730) //If at least 2 years, annual
        {
            m_dividendPeriodType = DividendPeriodType::Year;
        }
        else if (daysBetween >= 366) //If at least 1 year, use quarterly
        {
            m_dividendPeriodType = DividendPeriodType::Quarter;
        }
        else if (firstDateFound.month() != lastDateFound.month()) //If different months, use monthly
        {
            m_dividendPeriodType = DividendPeriodType::Month;
        }
        else
        {
            m_dividendPeriodType = DividendPeriodType::Day;
        }

        //Now compute the amount for each interval
        QDate curEndDate;
        auto i = tempDividends.begin();

        //Find the first end date
        switch (m_dividendPeriodType)
        {
        case DividendPeriodType::Year:
            curEndDate = QDate(i.key().year(), 12, 31);
            break;

        case DividendPeriodType::Quarter:
            curEndDate = QDate(i.key().year(), i.key().month(), i.key().daysInMonth()).addMonths(2);
            break;

        case DividendPeriodType::Month:
            curEndDate = QDate(i.key().year(), i.key().month(), i.key().daysInMonth());
            break;

        case DividendPeriodType::Day:
            curEndDate = i.key();
            break;
        }

        while (i != tempDividends.end())
        {
            Amount curTotal;

            while (i != tempDividends.end() && i.key() <= curEndDate)
            {
                curTotal += i.value();
                ++i;
            }

            m_dividendsOverTime[curEndDate] = curTotal;

            //Increment the end date
            switch (m_dividendPeriodType)
            {
            case DividendPeriodType::Year:
                curEndDate = curEndDate.addYears(1);
                break;

            case DividendPeriodType::Quarter:
                curEndDate = curEndDate.addMonths(3);
                break;

            case DividendPeriodType::Month:
                curEndDate = curEndDate.addMonths(1);
                break;

            case DividendPeriodType::Day:
                curEndDate = curEndDate.addDays(1);
                break;
            }
        }

    }

    endResetModel();
}

//void DividendsModel::load()
//{
//    m_dividendPositions.clear();

//    //Load the account and securities (different than plain portfolio positions since may include other dividends)
//    for (int idAccount : m_portfolio->allAccountsInTree())
//    {
//        int idSecurity = Account::getTopLevel()->account(idAccount)->idSecurity();

//        if (!m_securityIndex.contains(idSecurity))
//        {
//            m_securityIndex[idSecurity] = m_dividendPositions.size();
//            m_dividendPositions << PositionDividend(idSecurity);
//        }

//        m_dividendPositions[m_securityIndex[idSecurity]].relatedAccounts.insert(idAccount);
//    }

//    for (int i = 0; i < m_dividendPositions.size(); ++i)
//    {
//        reloadDividendsAt(i);
//    }

//    adjustPercentagesAndVisible();
//}

//void DividendsModel::reloadAllPositions()
//{
//    for (int i = 0; i < m_dividendPositions.size(); ++i)
//    {
//        reloadDividendsAt(i);
//    }

//    adjustPercentagesAndVisible();
//}

//void DividendsModel::adjustPercentagesAndVisible()
//{
//    beginResetModel();

//    double total = 0.0;
//    m_visiblePositions.clear();

//    //Find the visible positions and compute the total
//    for (const PositionDividend& div : m_dividendPositions)
//    {
//        if (div.received != 0)
//        {
//            m_visiblePositions.append(div);
//            total += div.received.toDouble();
//        }
//    }

//    //Compute proportions
//    for (PositionDividend& div : m_visiblePositions)
//    {
//        div.percent = div.received.toDouble() / total;
//    }

//    endResetModel();
//}

//void DividendsModel::reloadDividendsAt(int _index)
//{
//    PositionDividend& div = m_dividendPositions[_index];
//    div.received = 0;

//    for (int idAccount : div.relatedAccounts)
//    {
//        Account* a = Account::getTopLevel()->account(idAccount);
//        auto transactions = a->ledger()->transactionsBetween(m_dateFrom, m_dateTo);

//        for (Transaction* t : transactions)
//        {
//            InvestmentTransaction* itr = qobject_cast<InvestmentTransaction*>(t);

//            if (itr)
//            {
//                switch (itr->action())
//                {
//                case InvestmentAction::Dividend:
//                case InvestmentAction::Distribution:
//                case InvestmentAction::ReinvestDiv:
//                case InvestmentAction::ReinvestDistrib:
//                {
//                    const Transaction::Split& s = itr->splitFor(InvestmentSplitType::DistributionSource);

//                    div.received -= s.amount //-= since credit from income account
//                                    * PriceManager::instance()->rate(s.currency, m_portfolio->currency()->code());
//                    break;
//                }
//                default:
//                    break;
//                }


//            }
//        }
//    }
//}

//QSet<int> DividendsModel::accountsFor(int _idSecurity) const
//{
//    QSet<int> accounts;

//    for (int idAccount : m_portfolio->allAccountsInTree())
//    {
//        if (Account::getTopLevel()->account(idAccount)->idSecurity() == _idSecurity)
//        {
//            accounts.insert(idAccount);
//        }
//    }

//    return accounts;
//}

///////////////////////////////////////// SLOTS ///////////////////////////////////////

//void DividendsModel::reloadSecurityData(const KLib::Security* _security)
//{
//    if (m_securityIndex.contains(_security->id()))
//    {
//        int idx = m_securityIndex[_security->id()];
//        PositionDividend& div = m_dividendPositions[idx];

//        //Reload accounts
//        div.relatedAccounts = accountsFor(_security->id());

//        if (div.relatedAccounts.isEmpty())
//        {
//            m_dividendPositions.removeAt(idx);
//            m_securityIndex.remove(_security->id());

//            //Rebuild index
//            for (int i = idx; i < m_dividendPositions.size(); ++i)
//            {
//                m_securityIndex[m_dividendPositions[i].idSecurity] = i;
//            }
//        }
//        else
//        {
//            reloadDividendsAt(idx);
//        }

//        adjustPercentagesAndVisible();
//    }
//}

//void DividendsModel::addPosition(const KLib::Security* _security)
//{
//    //Add it at the end
//    m_securityIndex[_security->id()] = m_dividendPositions.size();
//    m_dividendPositions << PositionDividend(_security->id(), accountsFor(_security->id()));

//    reloadDividendsAt(m_dividendPositions.size()-1);

//    adjustPercentagesAndVisible();
//}









