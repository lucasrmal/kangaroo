#ifndef SPENDINGCHART_H
#define SPENDINGCHART_H

#include "../ihomewidget.h"
#include <QAbstractTableModel>
#include <QDate>

class QComboBox;
class QSpinBox;
class QPushButton;

namespace KLib
{
    class Account;
    class PercChart;
}

enum class CategoryValueType
{
    Spending,
    Income,
};

class CategoryValueModel : public QAbstractTableModel
{
    Q_OBJECT

    public:

        explicit    CategoryValueModel(CategoryValueType _type, QObject* _parent);

        int         rowCount(const QModelIndex& _parent = QModelIndex()) const override;
        int         columnCount(const QModelIndex& _parent = QModelIndex()) const override;

        QVariant    data(const QModelIndex& _index, int _role) const override;
        QVariant    headerData(int _section, Qt::Orientation _orientation, int _role) const override;

        CategoryValueType type() const { return m_type; }

        static const QString CATEGORY_PROP_TAG;

    public slots:
        void        setBalancesBetween(const QDate& _start, const QDate& _end);

        bool configure();

    private slots:
        void onAccountAdded(KLib::Account* _a);
        void onAccountRemoved(KLib::Account* _a);
        void onAccountModified(KLib::Account* _a);

    private:
        void loadData();
        bool isRightType(KLib::Account* _a);
        int  indexFor(KLib::Account* _a, bool* _inList = nullptr);

        QList<KLib::Account*>   m_accounts;
        QHash<int, int>         m_accountIndex;

        CategoryValueType m_type;
        QDate             m_startDate;
        QDate             m_endDate;
};

class CategoryValueChart : public IHomeWidget
{
    Q_OBJECT

    enum class BalanceType
    {
        ThroughToday = 0,
        AllTime,
        Year,
        Month
    };

    public:
        CategoryValueChart(CategoryValueType _type, QWidget* _parent = nullptr);

        static QString titleFor(CategoryValueType _type);

    public slots:
        void configure() override;

    private slots:
        void onChangeBalanceType(int _index);
        void updateBalanceDates();
        void goNext();
        void goPrevious();

    protected:
        QString title() const override;

    private:
        KLib::PercChart*    m_chart;
        CategoryValueModel* m_model;

        QComboBox*  m_cboBalanceSelector;
        QSpinBox*   m_spinYear;
        QComboBox*  m_cboMonth;
        QPushButton* m_btnPrevious;
        QPushButton* m_btnNext;
};

#endif // SPENDINGCHART_H
