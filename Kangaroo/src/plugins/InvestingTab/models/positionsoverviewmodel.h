#ifndef POSITIONSOVERVIEWMODEL_H
#define POSITIONSOVERVIEWMODEL_H

#include <QAbstractTableModel>
#include <KangarooLib/amount.h>

class Portfolio;

namespace KLib
{
    class Security;
}

namespace PositionsOverviewColumn
{
    enum Column
    {
        Name = 0,
        Symbol,
        LastPrice,
        Change,
        NumShares,
        DayGain,
        CostPerShare,
        CostBasis,
        MarketValue,
        ProfitLoss,
        PercProfitLoss,
        PercPortfolio,
        NumColumns
    };
}

class PositionsOverviewModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        PositionsOverviewModel(Portfolio* _portfolio, QObject* _parent = nullptr);

        int rowCount(const QModelIndex& _parent = QModelIndex()) const;
        int columnCount(const QModelIndex& _parent = QModelIndex()) const;

        QVariant data(const QModelIndex& _index, int _role) const;
        QVariant headerData(int _section, Qt::Orientation _orientation,
                            int _role = Qt::DisplayRole) const;

        //void sort(int column, Qt::SortOrder order = Qt::AscendingOrder);

    public slots:
        void onPositionAdded(KLib::Security* _sec);
        void onPositionRemoved(KLib::Security* _sec, int _row);
        void updatePosition(KLib::Security* _sec);

    private:
        void updateOpenChange();

        Portfolio* m_portfolio;

        KLib::Amount m_totalOpen;
        KLib::Amount m_totalChange;

        double totalPercChange() const { return m_totalChange == 0 ? 0
                                                          : 100.0 * m_totalChange.toDouble() / m_totalOpen.toDouble(); }
};

#endif // POSITIONSOVERVIEWMODEL_H
