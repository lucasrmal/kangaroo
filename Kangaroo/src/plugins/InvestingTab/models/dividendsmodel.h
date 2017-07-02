#ifndef DIVIDENDSMODEL_H
#define DIVIDENDSMODEL_H

#include <QAbstractTableModel>
#include <KangarooLib/amount.h>
#include <QDate>

class Portfolio;

namespace DividendsModelColumn
{
    enum Column
    {
        Name = 0,
        Symbol,
        DividendsReceived,
        DividendsPercent,

        NumColumns
    };
}

namespace KLib
{
    class Security;
}

class DividendsModel : public QAbstractTableModel
{
    Q_OBJECT

        struct PositionDividend
        {
            PositionDividend(int _idSecurity/*, const QSet<int>& _related = QSet<int>()*/) :
                idSecurity(_idSecurity),
                percent(0.0)/*,
                relatedAccounts(_related)*/ {}

            int          idSecurity;
            KLib::Amount received;
            double       percent;

//            QSet<int>    relatedAccounts;
        };

        enum class DividendPeriodType
        {
            Day,
            Month,
            Quarter,
            Year
        };

    public:
        DividendsModel(const QDate& _from, const QDate& _to, Portfolio* _portfolio, QObject* _parent = nullptr);

        int rowCount(const QModelIndex& = QModelIndex()) const      { return m_dividendPositions.size(); }
        int columnCount(const QModelIndex& = QModelIndex()) const   { return DividendsModelColumn::NumColumns; }

        QVariant data(const QModelIndex& _index, int _role) const;
        QVariant headerData(int _section, Qt::Orientation _orientation, int _role = Qt::DisplayRole) const;

        const QDate& dateFrom() const  { return m_dateFrom; }
        const QDate& dateTo() const    { return m_dateTo; }

    public slots:
        void setInterval(const QPair<QDate, QDate>& _interval);
        void refresh();

    private slots:
//        void addPosition(const KLib::Security* _security);
//        void reloadSecurityData(const KLib::Security* _security);

    private:
//        void load();
//        void adjustPercentagesAndVisible();
//        void reloadDividendsAt(int _index);

        QSet<int> accountsFor(int _idSecurity) const;

    public:
        Portfolio*      m_portfolio;

        QList<PositionDividend> m_dividendPositions;

        QMap<QDate, KLib::Amount> m_dividendsOverTime;
        DividendPeriodType        m_dividendPeriodType;

        QDate           m_dateFrom;
        QDate           m_dateTo;
};

#endif // DIVIDENDSMODEL_H
