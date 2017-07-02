#ifndef INCOMEEXPENSETAB_H
#define INCOMEEXPENSETAB_H

#include "accountviewtab.h"

class QComboBox;
class QSpinBox;

class IncomeExpenseTab : public AccountViewTab
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
        explicit IncomeExpenseTab(QWidget* _parent = nullptr);

    private slots:
        void onChangeBalanceType(int _index);
        void updateBalanceDates();

    private:
        QComboBox* m_cboBalanceSelector;
        QSpinBox* m_spinYear;
        QComboBox* m_cboMonth;

};

#endif // INCOMEEXPENSETAB_H
