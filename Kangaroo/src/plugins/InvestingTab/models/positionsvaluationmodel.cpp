#include "positionsvaluationmodel.h"
#include "portfolio.h"

#include <KangarooLib/model/security.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/currency.h>

using namespace KLib;

PositionsValuationModel::PositionsValuationModel(Portfolio* _portfolio, QObject* _parent) :
    QAbstractTableModel(_parent),
    m_portfolio(_portfolio)
{
    connect(m_portfolio, &Portfolio::positionAdded,             this, &PositionsValuationModel::onPositionAdded);
    connect(m_portfolio, &Portfolio::positionRemoved,           this, &PositionsValuationModel::onPositionRemoved);
    connect(m_portfolio, &Portfolio::portfolioSecurityModified, this, &PositionsValuationModel::updatePosition);
    connect(m_portfolio, &Portfolio::positionDataChanged,       this, &PositionsValuationModel::updatePosition);
    connect(m_portfolio, &Portfolio::securityPriceModified,     this, &PositionsValuationModel::updatePosition);

    //QSettings s;
    //m_updateInterval = s.value("StockInfo/PriceUpdateInterval", DEFAULT_UPDATE_INTERVAL).toInt();
}

int PositionsValuationModel::rowCount(const QModelIndex&) const
{
    return m_portfolio->positions().count();
}

int PositionsValuationModel::columnCount(const QModelIndex&) const
{
    return PositionsValuationColumn::NumColumns;
}

QVariant PositionsValuationModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid() || _index.row() >= rowCount())
        return QVariant();

    if (_role == Qt::TextAlignmentRole)
    {
        if (_index.column() == PositionsValuationColumn::Name || _index.column() == PositionsValuationColumn::Symbol)
        {
            return (int)(Qt::AlignLeft | Qt::AlignVCenter);
        }
        else
        {
            return (int) (Qt::AlignRight | Qt::AlignVCenter);
        }
    }
    if (_role == Qt::DisplayRole || _role == Qt::EditRole)
    {
        const Position& p = m_portfolio->positions()[_index.row()];

        auto amountFormat = [_role] (const Amount& amount) {

            return _role == Qt::DisplayRole ? QVariant(amount.toString())
                                            : QVariant(amount.toDouble());
        };

        auto priceFormat = [_role, p] (const Amount& amount) {

            return _role == Qt::DisplayRole ? QVariant(p.currency->formatAmount(amount))
                                            : QVariant(amount.toDouble());
        };

        switch (_index.column())
        {
        case PositionsValuationColumn::Name:
            return p.security->name();

        case PositionsValuationColumn::Symbol:
            return p.security->symbol();

        case PositionsValuationColumn::PricePerShare:
            return priceFormat(p.lastPrice());

        case PositionsValuationColumn::YearHigh:
            return priceFormat(p.security->securityInfo(SecurityInfo::YearHigh));

        case PositionsValuationColumn::YearLow:
            return priceFormat(p.security->securityInfo(SecurityInfo::YearLow));

        case PositionsValuationColumn::FiftyDayAverage:
            return priceFormat(p.security->securityInfo(SecurityInfo::FiftyDayAverage));

        case PositionsValuationColumn::TwoHundredDayAverage:
            return priceFormat(p.security->securityInfo(SecurityInfo::TwoHundredDayAverage));

        case PositionsValuationColumn::BookValuePerShare:
            return priceFormat(p.security->securityInfo(SecurityInfo::BookValuePerShare));

        case PositionsValuationColumn::DividendPerShare:
            return priceFormat(p.security->securityInfo(SecurityInfo::DividendPerShare));

        case PositionsValuationColumn::DividendYield:
            return _role == Qt::DisplayRole ? QVariant(p.security->securityInfo(SecurityInfo::DividendYield).toString() + " %")
                                            : QVariant(p.security->securityInfo(SecurityInfo::DividendYield).toDouble());

        case PositionsValuationColumn::PERatio:
            return amountFormat(p.security->securityInfo(SecurityInfo::PERatio));

        case PositionsValuationColumn::EPS:
            return priceFormat(p.security->securityInfo(SecurityInfo::EPS));

        case PositionsValuationColumn::EPSEstimateCurYear:
            return priceFormat(p.security->securityInfo(SecurityInfo::EPSEstimateCurYear));

        case PositionsValuationColumn::EPSEstimateNextYear:
            return priceFormat(p.security->securityInfo(SecurityInfo::EPSEstimateNextYear));

        case PositionsValuationColumn::PriceToBook:
            return amountFormat(p.security->securityInfo(SecurityInfo::PriceToBook));

        case PositionsValuationColumn::PriceToSales:
            return amountFormat(p.security->securityInfo(SecurityInfo::PriceToSales));

        case PositionsValuationColumn::PEGRatio:
            return amountFormat(p.security->securityInfo(SecurityInfo::PEGRatio));

        case PositionsValuationColumn::ShortRatio:
            return amountFormat(p.security->securityInfo(SecurityInfo::ShortRatio));

        default:
            return QVariant();
        }
    }

    return QVariant();
}

QVariant PositionsValuationModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (_orientation != Qt::Horizontal || _role != Qt::DisplayRole)
        return QVariant();

    switch (_section)
    {
    case PositionsValuationColumn::Name:
        return tr("Name");

    case PositionsValuationColumn::Symbol:
        return tr("Symbol");

    case PositionsValuationColumn::PricePerShare:
        return tr("Price");

    case PositionsValuationColumn::YearHigh:
        return tr("Year High");

    case PositionsValuationColumn::YearLow:
        return tr("Year Low");

    case PositionsValuationColumn::FiftyDayAverage:
        return tr("50 day");

    case PositionsValuationColumn::TwoHundredDayAverage:
        return tr("200 day");

    case PositionsValuationColumn::BookValuePerShare:
        return tr("Book/share");

    case PositionsValuationColumn::DividendPerShare:
        return tr("Div/share");

    case PositionsValuationColumn::DividendYield:
        return tr("Div%");

    case PositionsValuationColumn::PERatio:
        return tr("PE");

    case PositionsValuationColumn::EPS:
        return tr("EPS");

    case PositionsValuationColumn::EPSEstimateCurYear:
        return tr("EPS Est. Cur Y");

    case PositionsValuationColumn::EPSEstimateNextYear:
        return tr("EPS Est. Next Y");

    case PositionsValuationColumn::PriceToBook:
        return tr("Price/Book");

    case PositionsValuationColumn::PriceToSales:
        return tr("Price/Sales");

    case PositionsValuationColumn::PEGRatio:
        return tr("PEG");

    case PositionsValuationColumn::ShortRatio:
        return tr("Short Ratio");

    default:
        return QVariant();
    }
}

void PositionsValuationModel::onPositionAdded(Security* _sec)
{
    int row = m_portfolio->indexOf(_sec);

    if (row != -1)
    {
        beginInsertRows(QModelIndex(), row, row);
        endInsertRows();
    }
}

void PositionsValuationModel::onPositionRemoved(Security*, int _row)
{
    beginRemoveRows(QModelIndex(), _row, _row);
    endRemoveRows();
}

void PositionsValuationModel::updatePosition(Security* _sec)
{
    int row = m_portfolio->indexOf(_sec);
    emit dataChanged(index(row, 0),
                     index(row, PositionsValuationColumn::NumColumns-1));
}




