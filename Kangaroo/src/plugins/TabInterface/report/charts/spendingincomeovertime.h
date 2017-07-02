#ifndef SPENDINGINCOMEOVERTIME_H
#define SPENDINGINCOMEOVERTIME_H

#include "../ichart.h"

namespace KLib
{
    class ReturnChart;
    class AccountSelector;
}

class QStandardItemModel;
class QComboBox;
class QSpinBox;
class QPushButton;
class QCheckBox;

class SpendingIncomeOverTime : public IChart
{
    Q_OBJECT

        enum class PeriodType
        {
            Month,
            YTD,
            Year,

            YTDTwoYearsComp,
            YTDThreeYearsComp,

            TwoYearsComp,
            ThreeYearsComp,

            ThreeYears,
            FiveYears,
            TenYears
        };

    public:
        SpendingIncomeOverTime(QWidget* _parent = nullptr);

        void print(const QPrinter& _printer) const override { Q_UNUSED(_printer) }

        void saveAs(const QString& _path) const override { Q_UNUSED(_path) }

    public slots:
        void refresh() override;

    private slots:
        void updatePeriodSelection();

    private:
        QStandardItemModel* m_model;

        KLib::ReturnChart* m_chart;
        KLib::AccountSelector* m_selector;

        QComboBox* m_cboDisplayType;
        QComboBox* m_cboMonth;
        QSpinBox*  m_spinYear;
        QCheckBox* m_chkIncludeSubtree;
        QCheckBox* m_chkShowAverage;

        QPushButton* m_btnRefresh;

};

#endif // SPENDINGINCOMEOVERTIME_H
