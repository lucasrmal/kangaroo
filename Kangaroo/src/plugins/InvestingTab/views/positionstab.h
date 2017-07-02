#ifndef POSITIONSTAB_H
#define POSITIONSTAB_H

#include <QWidget>

class QTableView;
class QStackedWidget;
class QPushButton;
class QLabel;

class Portfolio;
class PositionsOverviewModel;
class PositionsValuationModel;

namespace KLib
{
    class PercChart;
}

class PositionsTab : public QWidget
{
    Q_OBJECT

    public:
        explicit PositionsTab(Portfolio* _portfolio, QWidget *parent = nullptr);

    private slots:
        void buttonToggled(bool _toggled);

        void onPortfolioNameChanged();

    private:
        Portfolio* m_portfolio;
        PositionsOverviewModel* m_returnModel;
        PositionsValuationModel* m_valuationModel;

        QLabel*             m_lblTitle;
        QList<QPushButton*> m_buttons;
        QStackedWidget*     m_stack;

        QTableView*         m_returnsTable;
        QTableView*         m_valuationTable;
        KLib::PercChart*    m_pieChart;
        KLib::PercChart*    m_barChart;


};

#endif // POSITIONSTAB_H
