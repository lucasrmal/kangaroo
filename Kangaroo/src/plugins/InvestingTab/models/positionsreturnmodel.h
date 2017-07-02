#ifndef POSITIONSRETURNMODEL_H
#define POSITIONSRETURNMODEL_H

#include <QAbstractTableModel>
#include <QDate>
#include <KangarooLib/amount.h>

class Portfolio;

namespace KLib
{
    class Security;
}

//namespace PositionsReturnColumn
//{
//    enum Column
//    {
//        Name = 0,
//        NumColumns
//    };
//}


/*
 * Rows: returns. 3 months, 6 months, 1 year, 3 years
 *
 * Columns: 1: Portfolio. 2+: Indices? (S&P 500, Dow, ...)
 */


namespace ReturnPeriod
{
    enum Periods
    {
        ThreeMonths,
        SixMonths,
        OneYear,
        ThreeYears,
        FiveYears,

        NUM_PERIODS
    };
}

class PositionsReturnModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        PositionsReturnModel(Portfolio* _portfolio, QObject* _parent = nullptr);

        const QDate& reportDate() const         { return m_reportDate; }
        void setReportDate(const QDate& _date);

        int rowCount(const QModelIndex& _parent = QModelIndex()) const;
        int columnCount(const QModelIndex& _parent = QModelIndex()) const;

        QVariant data(const QModelIndex& _index, int _role) const;
        QVariant headerData(int _section, Qt::Orientation _orientation,
                            int _role = Qt::DisplayRole) const;

        static const int TOTAL_YEAR_RANGE = 5;

    public slots:
        void onPositionAdded(KLib::Security* _sec);
        void onPositionRemoved(KLib::Security* _sec, int _row);

    private slots:
        void computeReturns();
        void onPortfolioDataChanged();

    private:
        typedef QPair<double, bool> Return;

        Portfolio*      m_portfolio;
        QDate           m_reportDate;

        QVector<Return> m_periodReturns;
        QVector<Return> m_annualReturns;
        double          m_returns[ReturnPeriod::NUM_PERIODS];

        QVector<QPair<QDate, KLib::Amount> > m_values;

        friend class DetailedPositionsReturnModel;
        friend class PositionsValueModel;
};

class DetailedPositionsReturnModel : public QAbstractTableModel
{
    Q_OBJECT

    public:
        enum class RangeType
        {
            Annual,
            Quarterly
        };


        DetailedPositionsReturnModel(RangeType _type, PositionsReturnModel* _returnModel, QObject* _parent = nullptr);

        int rowCount(const QModelIndex& _parent = QModelIndex()) const;
        int columnCount(const QModelIndex& _parent = QModelIndex()) const;

        QVariant data(const QModelIndex& _index, int _role) const;
        QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const;

    private:
        RangeType m_type;
        PositionsReturnModel* m_returnModel;
};

class PositionsValueModel : public QAbstractTableModel
{
    Q_OBJECT

    public:

        PositionsValueModel(PositionsReturnModel* _returnModel, QObject* _parent = nullptr);

        int rowCount(const QModelIndex& _parent = QModelIndex()) const;
        int columnCount(const QModelIndex& _parent = QModelIndex()) const;

        QVariant data(const QModelIndex& _index, int _role) const;
        QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const;

    private:
        PositionsReturnModel* m_returnModel;
};





#endif // POSITIONSRETURNMODEL_H
