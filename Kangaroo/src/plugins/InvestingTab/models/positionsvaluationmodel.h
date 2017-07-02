#ifndef POSITIONSMODEL_H
#define POSITIONSMODEL_H

#include <QAbstractTableModel>
#include <QHash>
#include <QSortFilterProxyModel>
#include <KangarooLib/amount.h>
#include <KangarooLib/model/transaction.h>

namespace KLib
{
    class Account;
    class Security;
    class Currency;
}

class QNetworkAccessManager;
class QNetworkReply;
class QSortFilterProxyModel;

class Portfolio;

namespace PositionsValuationColumn
{
    enum Column
    {
        Name = 0,
        Symbol,
        PricePerShare,

        YearHigh,
        YearLow,
        FiftyDayAverage,
        TwoHundredDayAverage,

        BookValuePerShare,
        DividendPerShare,
        DividendYield,
        PERatio,
        EPS,
        EPSEstimateCurYear,
        EPSEstimateNextYear,

        PriceToBook,
        PriceToSales,
        PEGRatio,
        ShortRatio,

        NumColumns
    };
}

class PositionsValuationModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        explicit PositionsValuationModel(Portfolio* _portfolio, QObject* _parent = nullptr);

        int rowCount(const QModelIndex& _parent = QModelIndex()) const;
        int columnCount(const QModelIndex& _parent = QModelIndex()) const;

        QVariant data(const QModelIndex& _index, int _role) const;
        QVariant headerData(int _section, Qt::Orientation _orientation,
                            int _role = Qt::DisplayRole) const;

    public slots:
        void onPositionAdded(KLib::Security* _sec);
        void onPositionRemoved(KLib::Security* _sec, int _row);
        void updatePosition(KLib::Security* _sec);

    private:        
        Portfolio* m_portfolio;

};

#endif // POSITIONSMODEL_H
