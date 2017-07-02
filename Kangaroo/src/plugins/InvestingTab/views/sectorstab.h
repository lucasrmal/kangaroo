#ifndef SECTORSTAB_H
#define SECTORSTAB_H

#include <QWidget>

class QLabel;
class QTableView;
class QStackedWidget;
class QPushButton;

class Portfolio;
class PortfolioSectorModel;

namespace KLib
{
    class PercChart;
}

class SectorsTab : public QWidget
{
    Q_OBJECT

    public:
        explicit SectorsTab(Portfolio* _portfolio, QWidget *parent = nullptr);

    private slots:
        void buttonToggled(bool _toggled);

        void onPortfolioNameChanged();

    private:
        Portfolio* m_portfolio;
        PortfolioSectorModel* m_model;
        QList<QPushButton*> m_buttons;

        QLabel*          m_lblTitle;
        QStackedWidget*  m_stack;
        QTableView*      m_table;
        KLib::PercChart* m_pieChart;
        KLib::PercChart* m_barChart;


};

#endif // SECTORSTAB_H
