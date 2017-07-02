#ifndef RETURNSTAB_H
#define RETURNSTAB_H

#include <QWidget>

class Portfolio;
class PositionsReturnModel;
class QLabel;
class QPushButton;
class QStackedWidget;
class QDateEdit;

/*
 *
 * Returns by range: 1, 3, 6, 12 months, 1, 3, 5, 10 years. For each range, % of annual return
 * - Can choose between money-weighted or time-weighted?
 * - Can choose year of report
 *
 *
 * Table with summary: value at beginning, dividends, gain/loss, taxes, sales, etc.
 *
 * Investment value over time
 * - Can choose period (year, month, 2, 3 years) and date
 * - Can view whole portfolio or only one position
 * - Can display any security? (enter symbol)?
 *
 *
 * - Dividend and distributions returns? Chart showing total div/dist per month or per year. Per position also.
 *
 *
 */

namespace KLib
{
    class ReturnChart;
}

class ReturnsTab : public QWidget
{
    Q_OBJECT

    public:
        ReturnsTab(Portfolio* _portfolio, QWidget* _parent = nullptr);

    private slots:
        void buttonToggled(bool _toggled);

        void onPortfolioNameChanged();
        void onDateChanged(const QDate& _date);

    private:
        Portfolio* m_portfolio;
        PositionsReturnModel* m_returnsModel;

        QLabel*             m_lblTitle;
        QList<QPushButton*> m_buttons;
        QStackedWidget*     m_stack;
        QLabel*             m_lblDetails;

        QDateEdit*          m_dteAsOf;

        //Charts
        KLib::ReturnChart* m_returnsRangeChart;
        KLib::ReturnChart* m_quarterlyReturnsChart;
        KLib::ReturnChart* m_annualReturnsChart;
        KLib::ReturnChart* m_valueChart;
};

#endif // RETURNSTAB_H
