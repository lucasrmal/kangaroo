#ifndef PORTFOLIOSECTORMODEL_H
#define PORTFOLIOSECTORMODEL_H

#include <QAbstractTableModel>
#include <KangarooLib/amount.h>
#include "portfolio.h"

using KLib::Amount;

namespace PortfolioSectorModelColumn
{
    enum Column
    {
        Sector = 0,
        CostBasis,
        MarketValue,
        ProfitLoss,
        PercProfitLoss,
        NumPositions,
        PercPortfolio,
        NumColumns
    };
}

struct SectorPosition
{
    QString name;

    Amount costBasis;
    Amount marketValue;
    Amount profitLoss() const    { return marketValue - costBasis; }
    double percProfitLoss() const { return costBasis == 0 ? 0.0
                                                          : (-1.0+(marketValue.toDouble()
                                                                   / costBasis.toDouble()))*100.0; }

    double percPortfolio;

    QList<int> positions;
};


class PortfolioSectorModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        PortfolioSectorModel(Portfolio* _portfolio, QObject* _parent = nullptr);

        int      rowCount(const QModelIndex& _parent = QModelIndex()) const     { return m_sectors.count(); }
        int      columnCount(const QModelIndex& _parent = QModelIndex()) const  { return PortfolioSectorModelColumn::NumColumns; }

        QVariant data(const QModelIndex& _index, int _role) const;
        QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const;

    public slots:
        void onPositionAdded(KLib::Security* _sec);
        void onPositionRemoved(KLib::Security* _sec, int _row);
        void onPositionDataChanged(KLib::Security* _sec);
        void onPortfolioSecurityModified(KLib::Security* _sec);

    private:
        void updatePercPortfolio();
        void updateSector(SectorPosition& _pos);
        void addPosition(KLib::Security* _sec, bool _updatePercPortfolio = true);

        Portfolio* m_portfolio;
        QList<SectorPosition> m_sectors;

        QHash<QString, int> m_sectorIndex;
        QHash<int, int> m_securityIndex;


};

#endif // PORTFOLIOSECTORMODEL_H
