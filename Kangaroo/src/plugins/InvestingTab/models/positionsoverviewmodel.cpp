#include "positionsoverviewmodel.h"
#include "portfolio.h"

#include <KangarooLib/model/security.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/currency.h>

using namespace KLib;

PositionsOverviewModel::PositionsOverviewModel(Portfolio* _portfolio, QObject* _parent) :
    QAbstractTableModel(_parent),
    m_portfolio(_portfolio)
{
    connect(m_portfolio, &Portfolio::positionAdded,             this, &PositionsOverviewModel::onPositionAdded);
    connect(m_portfolio, &Portfolio::positionRemoved,           this, &PositionsOverviewModel::onPositionRemoved);
    connect(m_portfolio, &Portfolio::portfolioSecurityModified, this, &PositionsOverviewModel::updatePosition);
    connect(m_portfolio, &Portfolio::positionDataChanged,       this, &PositionsOverviewModel::updatePosition);
    connect(m_portfolio, &Portfolio::securityPriceModified,     this, &PositionsOverviewModel::updatePosition);

    updateOpenChange();
}

QVariant PositionsOverviewModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid() || _index.row() >= rowCount())
        return QVariant();

    if (_role == Qt::TextAlignmentRole)
    {
        if (_index.column() == PositionsOverviewColumn::Name || _index.column() == PositionsOverviewColumn::Symbol)
        {
            return (int)(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else
        {
            return (int) (Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    if (_role == Qt::DisplayRole && _index.row() < rowCount()-1)
    {
        const Position& p = m_portfolio->positions()[_index.row()];

        auto percChange = [p]()
        {
            Amount openPrice = p.security->securityInfo(SecurityInfo::OpenPrice);
            Amount change = p.security->securityInfo(SecurityInfo::DayChange);
            return openPrice == 0 ? 0
                                  : 100.0 * change.toDouble() / openPrice.toDouble(); };

        switch (_index.column())
        {
        case PositionsOverviewColumn::Name:
            return p.security->name();

        case PositionsOverviewColumn::Symbol:
            return p.security->symbol();

        case PositionsOverviewColumn::LastPrice:
            return p.currency->formatAmount(p.lastPrice());

        case PositionsOverviewColumn::Change:
            return QString("%1 (%2)").arg(m_portfolio->formatGainLoss(p.security->securityInfo(SecurityInfo::DayChange)))
                                     .arg(Portfolio::formatPercChange(percChange()));

        case PositionsOverviewColumn::NumShares:
            return p.security->formatAmount(p.balance());

        case PositionsOverviewColumn::DayGain:
            return m_portfolio->formatGainLoss(p.security->securityInfo(SecurityInfo::DayChange) * p.balance());

        case PositionsOverviewColumn::CostPerShare:
            return p.currency->formatAmount(p.costPerShare());

        case PositionsOverviewColumn::CostBasis:
            return p.currency->formatAmount(p.costBasis());

        case PositionsOverviewColumn::MarketValue:
            return p.currency->formatAmount(p.marketValue());

        case PositionsOverviewColumn::ProfitLoss:
            return m_portfolio->formatGainLoss(p.profitLoss());

        case PositionsOverviewColumn::PercProfitLoss:
            return Portfolio::formatPercChange(p.percProfitLoss());

        case PositionsOverviewColumn::PercPortfolio:
            return QLocale().toString(p.percOfPortfolio, 'f', 2) + " %";

        }
    }
    else if (_role == Qt::EditRole && _index.row() < rowCount()-1)
    {
        const Position& p = m_portfolio->positions()[_index.row()];

        switch (_index.column())
        {
        case PositionsOverviewColumn::Name:
            return p.security->name();

        case PositionsOverviewColumn::Symbol:
            return p.security->symbol();

        case PositionsOverviewColumn::LastPrice:
            return p.lastPrice().toDouble();

        case PositionsOverviewColumn::Change:
            p.security->securityInfo(SecurityInfo::DayChange).toDouble();

        case PositionsOverviewColumn::NumShares:
            return p.balance().toDouble();

        case PositionsOverviewColumn::DayGain:
            p.security->securityInfo(SecurityInfo::DayChange) * p.balance();

        case PositionsOverviewColumn::CostPerShare:
            p.costPerShare().toDouble();

        case PositionsOverviewColumn::CostBasis:
            return p.costBasis().toDouble();

        case PositionsOverviewColumn::MarketValue:
            return p.marketValue().toDouble();

        case PositionsOverviewColumn::ProfitLoss:
            return p.profitLoss().toDouble();

        case PositionsOverviewColumn::PercProfitLoss:
            return p.percProfitLoss();

        case PositionsOverviewColumn::PercPortfolio:
            return p.percOfPortfolio;

        }
    }
    else if (_role == Qt::TextColorRole && _index.row() < rowCount()-1)
    {
        const Position& p = m_portfolio->positions()[_index.row()];

        switch (_index.column())
        {
        case PositionsOverviewColumn::Change:
            return Portfolio::colorForAmount(p.security->securityInfo(SecurityInfo::DayChange));

        case PositionsOverviewColumn::DayGain:
            return Portfolio::colorForAmount(p.security->securityInfo(SecurityInfo::DayChange));

        case PositionsOverviewColumn::ProfitLoss:
            return Portfolio::colorForAmount(p.profitLoss());

        case PositionsOverviewColumn::PercProfitLoss:
            return Portfolio::colorForAmount(p.percProfitLoss());

        }
    }
    else if (_role == Qt::DisplayRole) // "Total" row
    {
        switch (_index.column())
        {
        case PositionsOverviewColumn::Change:
            return QString("%1 (%2)").arg(m_portfolio->formatGainLoss(m_totalChange))
                                     .arg(Portfolio::formatPercChange(totalPercChange()));

        case PositionsOverviewColumn::DayGain:
            return m_portfolio->formatGainLoss(m_totalChange);

        case PositionsOverviewColumn::CostBasis:
            return m_portfolio->currency()->formatAmount(m_portfolio->totalCostBasis());

        case PositionsOverviewColumn::MarketValue:
            return m_portfolio->currency()->formatAmount(m_portfolio->totalMarketValue());

        case PositionsOverviewColumn::ProfitLoss:
            return m_portfolio->formatGainLoss(m_portfolio->totalProfitLoss());

        case PositionsOverviewColumn::PercProfitLoss:
            return Portfolio::formatPercChange(m_portfolio->totalPercProfitLoss());

        case PositionsOverviewColumn::PercPortfolio:
            return "100.00 %";

        }
    }
    else if (_role == Qt::TextColorRole) // "Total" row
    {
        switch (_index.column())
        {
        case PositionsOverviewColumn::Change:
        case PositionsOverviewColumn::DayGain:
            return Portfolio::colorForAmount(m_totalChange);

        case PositionsOverviewColumn::ProfitLoss:
            return Portfolio::colorForAmount(m_portfolio->totalProfitLoss());

        case PositionsOverviewColumn::PercProfitLoss:
            return Portfolio::colorForAmount(m_portfolio->totalPercProfitLoss());

        }
    }
    else if (_role == Qt::BackgroundColorRole && _index.row() == rowCount()-1)
    {
        return QColor("#E1F0F3");
    }

    return QVariant();
}

QVariant PositionsOverviewModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (_orientation != Qt::Horizontal || _role != Qt::DisplayRole)
        return QVariant();

    switch (_section)
    {
    case PositionsOverviewColumn::Name:
        return tr("Name");

    case PositionsOverviewColumn::Symbol:
        return tr("Symbol");

    case PositionsOverviewColumn::LastPrice:
        return tr("Last Price");

    case PositionsOverviewColumn::Change:
        return tr("Change");

    case PositionsOverviewColumn::NumShares:
        return tr("Shares");

    case PositionsOverviewColumn::DayGain:
        return tr("Day's Gain");

    case PositionsOverviewColumn::CostPerShare:
        return tr("Cost/Share");

    case PositionsOverviewColumn::CostBasis:
        return tr("Cost Basis");

    case PositionsOverviewColumn::MarketValue:
        return tr("Market Value");

    case PositionsOverviewColumn::ProfitLoss:
        return tr("P&L");

    case PositionsOverviewColumn::PercProfitLoss:
        return tr("%Return");

    case PositionsOverviewColumn::PercPortfolio:
        return tr("%Portfolio");

    default:
        return QVariant();
    }
}

int PositionsOverviewModel::rowCount(const QModelIndex&) const
{
    return m_portfolio->positions().count()+1;
}

int PositionsOverviewModel::columnCount(const QModelIndex&) const
{
    return PositionsOverviewColumn::NumColumns;
}

void PositionsOverviewModel::onPositionAdded(Security* _sec)
{
    int row = m_portfolio->indexOf(_sec);

    if (row != -1)
    {
        beginInsertRows(QModelIndex(), row, row);
        endInsertRows();
        updateOpenChange();
    }
}

void PositionsOverviewModel::onPositionRemoved(Security* _sec, int _row)
{
    beginRemoveRows(QModelIndex(), _row, _row);
    endRemoveRows();
    updateOpenChange();
}

void PositionsOverviewModel::updatePosition(Security* _sec)
{
    int row = m_portfolio->indexOf(_sec);
    emit dataChanged(index(row, 0),
                     index(row, PositionsOverviewColumn::NumColumns-1));
    updateOpenChange();
}

void PositionsOverviewModel::updateOpenChange()
{
    m_totalChange = m_totalOpen = 0;

    for (const Position& p : m_portfolio->positions())
    {
        double rate = p.currency->code() == m_portfolio->currency()->code()
                      ? 1.0
                      : PriceManager::instance()->rate(p.currency->code(),
                                                       m_portfolio->currency()->code());

        Amount openPrice = p.security->securityInfo(SecurityInfo::OpenPrice);

        m_totalChange += rate * p.balance() * p.security->securityInfo(SecurityInfo::DayChange);
        m_totalOpen += openPrice == 0 ? rate * p.marketValue()
                                      : rate * openPrice * p.balance();
    }
    emit dataChanged(index(rowCount()-1, 0),
                     index(rowCount()-1, PositionsOverviewColumn::NumColumns-1));
}

