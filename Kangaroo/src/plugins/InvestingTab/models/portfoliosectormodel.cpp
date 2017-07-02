#include "portfoliosectormodel.h"
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/security.h>

#include <QLocale>

using namespace KLib;

PortfolioSectorModel::PortfolioSectorModel(Portfolio* _portfolio, QObject* _parent) :
    QAbstractTableModel(_parent),
    m_portfolio(_portfolio)
{
    //Load all sectors and positions
    for (const Position& p : m_portfolio->positions())
    {
        addPosition(p.security, false);
    }

    updatePercPortfolio();

    connect(m_portfolio, &Portfolio::positionAdded, this, &PortfolioSectorModel::onPositionAdded);
    connect(m_portfolio, &Portfolio::positionRemoved, this, &PortfolioSectorModel::onPositionAdded);
    connect(m_portfolio, &Portfolio::portfolioSecurityModified, this, &PortfolioSectorModel::onPortfolioSecurityModified);
    connect(m_portfolio, &Portfolio::positionDataChanged, this, &PortfolioSectorModel::onPositionDataChanged);
}

QVariant PortfolioSectorModel::data(const QModelIndex& _index, int _role) const
{
    if (!_index.isValid()|| _index.row() >= rowCount())
        return QVariant();

    const SectorPosition& pos = m_sectors[_index.row()];

    if (_role == Qt::DisplayRole)
    {

        switch (_index.column())
        {
        case PortfolioSectorModelColumn::Sector:
            return pos.name.isEmpty() ? tr("Unspecified") : pos.name;

        case PortfolioSectorModelColumn::CostBasis:
            return m_portfolio->currency()->formatAmount(pos.costBasis);

        case PortfolioSectorModelColumn::MarketValue:
            return m_portfolio->currency()->formatAmount(pos.marketValue);

        case PortfolioSectorModelColumn::ProfitLoss:
            return m_portfolio->formatGainLoss(pos.profitLoss());

        case PortfolioSectorModelColumn::PercProfitLoss:
            return Portfolio::formatPercChange(pos.percProfitLoss());

        case PortfolioSectorModelColumn::NumPositions:
            return QString::number(pos.positions.count());

        case PortfolioSectorModelColumn::PercPortfolio:
            return QLocale().toString(pos.percPortfolio, 'f', 2) + " %";
        }
    }
    else if (_role == Qt::EditRole)
    {
        switch (_index.column())
        {
        case PortfolioSectorModelColumn::CostBasis:
            return pos.costBasis.toDouble();

        case PortfolioSectorModelColumn::MarketValue:
            return pos.marketValue.toDouble();

        case PortfolioSectorModelColumn::ProfitLoss:
            return pos.profitLoss().toDouble();

        case PortfolioSectorModelColumn::PercProfitLoss:
            return pos.percProfitLoss();

        case PortfolioSectorModelColumn::NumPositions:
            return pos.positions.size();

        case PortfolioSectorModelColumn::PercPortfolio:
            return pos.percPortfolio;
        }
    }
    else if (_role == Qt::TextColorRole)
    {
        switch (_index.column())
        {
        case PortfolioSectorModelColumn::ProfitLoss:
            return Portfolio::colorForAmount(m_sectors[_index.row()].profitLoss());

        case PortfolioSectorModelColumn::PercProfitLoss:
            return Portfolio::colorForAmount(m_sectors[_index.row()].percProfitLoss());

        }
    }

    return QVariant();
}

QVariant PortfolioSectorModel::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
    if (_orientation == Qt::Horizontal && _role == Qt::DisplayRole)
    {
        switch (_section)
        {
        case PortfolioSectorModelColumn::Sector:
            return tr("Sector");

        case PortfolioSectorModelColumn::CostBasis:
            return tr("Cost Basis");

        case PortfolioSectorModelColumn::MarketValue:
            return tr("Market Value");

        case PortfolioSectorModelColumn::ProfitLoss:
            return tr("P&L");

        case PortfolioSectorModelColumn::PercProfitLoss:
            return tr("%Return");

        case PortfolioSectorModelColumn::NumPositions:
            return tr("#Positions");

        case PortfolioSectorModelColumn::PercPortfolio:
            return tr("%Portfolio");
        }
    }

    return QVariant();
}

void PortfolioSectorModel::updatePercPortfolio()
{
    Amount total = 0;

    //Compute the total
    for (const SectorPosition& pos : m_sectors)
    {
        total += pos.marketValue;
    }

    //Update the %
    for (SectorPosition& pos : m_sectors)
    {
        pos.percPortfolio = 100.0 * (pos.marketValue.toDouble() / total.toDouble());
    }

    emit dataChanged(index(0,            PortfolioSectorModelColumn::PercPortfolio),
                     index(rowCount()-1, PortfolioSectorModelColumn::PercPortfolio));
}

void PortfolioSectorModel::updateSector(SectorPosition& _pos)
{
    _pos.costBasis = _pos.marketValue = 0;

    for (int idSec : _pos.positions)
    {
        const Position& pos = m_portfolio->position(idSec);

        _pos.costBasis + pos.costBasis();
        _pos.marketValue + pos.marketValue();
    }

    emit dataChanged(index(m_sectorIndex[_pos.name], PortfolioSectorModelColumn::CostBasis),
                     index(m_sectorIndex[_pos.name], PortfolioSectorModelColumn::NumPositions));
}

void PortfolioSectorModel::addPosition(KLib::Security* _sec, bool _updatePercPortfolio)
{
    const Position& pos = m_portfolio->position(_sec);

    //Find where to add it
    if (!m_sectorIndex.contains(_sec->sector()))
    {
        SectorPosition p;
        p.name        = _sec->sector();
        p.costBasis   = pos.costBasis();
        p.marketValue = pos.marketValue();
        p.positions.append(_sec->id());

        beginInsertRows(QModelIndex(), m_sectors.count(), m_sectors.count());
        m_sectorIndex[_sec->sector()] = m_sectors.count();
        m_sectors.append(p);
        endInsertRows();
    }
    else
    {
        SectorPosition& p = m_sectors[m_sectorIndex[_sec->sector()]];

        p.costBasis += pos.costBasis();
        p.marketValue += pos.marketValue();
        p.positions.append(_sec->id());

        emit dataChanged(index(m_sectorIndex[_sec->sector()], PortfolioSectorModelColumn::CostBasis),
                         index(m_sectorIndex[_sec->sector()], PortfolioSectorModelColumn::NumPositions));

    }

    m_securityIndex[_sec->id()] = m_sectorIndex[_sec->sector()];

    //Update the % of sector
    if (_updatePercPortfolio)
    {
        updatePercPortfolio();
    }
}

void PortfolioSectorModel::onPositionAdded(KLib::Security* _sec)
{
    addPosition(_sec);
}

void PortfolioSectorModel::onPositionRemoved(KLib::Security* _sec, int)
{
    //Find where to remove it and delete the sector if necessary
    if (m_securityIndex.contains(_sec->id()))
    {
        SectorPosition& p = m_sectors[m_securityIndex[_sec->id()]];

        p.positions.removeAll(_sec->id());

        if (p.positions.isEmpty()) //Remove the sector
        {
            beginRemoveRows(QModelIndex(), m_securityIndex[_sec->id()], m_securityIndex[_sec->id()]);
            m_sectors.removeAt(m_securityIndex[_sec->id()]);

            //Rebuild the indexes
            m_securityIndex.clear();
            m_sectorIndex.clear();

            for (int i = 0; i < m_sectors.size(); ++i)
            {
                SectorPosition& sec = m_sectors[i];

                m_sectorIndex[sec.name] = i;

                for (int secId : sec.positions)
                {
                    m_securityIndex[secId] = i;
                }
            }

            endRemoveRows();
        }
        else
        {
            updateSector(p);
        }

        updatePercPortfolio();
    }
}

void PortfolioSectorModel::onPositionDataChanged(Security* _sec)
{
    if (m_securityIndex.contains(_sec->id()))
    {
        updateSector(m_sectors[m_securityIndex[_sec->id()]]);
        updatePercPortfolio();
    }
}

void PortfolioSectorModel::onPortfolioSecurityModified(Security* _sec)
{
    //Check if the security is in another sector now

    if (m_securityIndex.contains(_sec->id())
        && _sec->sector() != m_sectors[m_securityIndex[_sec->id()]].name)
    {
        onPositionRemoved(_sec,-1);
        onPositionAdded(_sec);
    }

}

