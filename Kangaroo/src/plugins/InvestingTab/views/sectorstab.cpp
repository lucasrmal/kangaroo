#include "sectorstab.h"
#include "../models/portfoliosectormodel.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/widgets/percchart.h>
#include <KangarooLib/model/account.h>
#include <QTableView>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

using namespace KLib;

SectorsTab::SectorsTab(Portfolio* _portfolio, QWidget *parent) :
    QWidget(parent),
    m_portfolio(_portfolio),
    m_model(new PortfolioSectorModel(_portfolio, this)),
    m_table(new QTableView(this)),
    m_pieChart(new PercChart(ChartStyle::Pie,
                             PortfolioSectorModelColumn::Sector,
                             PortfolioSectorModelColumn::MarketValue,
                             Qt::EditRole, this)),
    m_barChart(new PercChart(ChartStyle::HorizontalBar,
                             PortfolioSectorModelColumn::Sector,
                             PortfolioSectorModelColumn::MarketValue,
                             Qt::EditRole, this))
{
    m_table->setModel(m_model);
    m_pieChart->setModel(m_model);
    m_barChart->setModel(m_model);

    m_pieChart->setLegendColumn(PortfolioSectorModelColumn::Sector);
    m_barChart->setLegendColumn(PortfolioSectorModelColumn::Sector);

    m_table->setSortingEnabled(true);
    m_table->sortByColumn(0, Qt::AscendingOrder);
    m_table->setGridStyle(Qt::NoPen);
    m_table->setSelectionMode(QAbstractItemView::SingleSelection);
    m_table->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_table->setColumnWidth(PortfolioSectorModelColumn::Sector, 200);

    m_lblTitle = new QLabel(tr("Sectors - %1").arg(m_portfolio->portfolioName()), this);
    QLabel* lblPicture = new QLabel(this);
    lblPicture->setPixmap(Core::pixmap("office-chart-ring"));

    m_buttons << new QPushButton(/*Core::icon("office-table"),*/     tr("Detail"), this)
              << new QPushButton(/*Core::icon("office-chart-pie"),*/ tr("Pie"), this)
              << new QPushButton(/*Core::icon("office-chart-pie"),*/ tr("Bar"), this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* layoutTop = new QHBoxLayout();

    m_stack = new QStackedWidget(this);
    m_stack->addWidget(m_table);
    m_stack->addWidget(m_pieChart);
    m_stack->addWidget(m_barChart);

    mainLayout->setContentsMargins(0,0,0,0);
    layoutTop->setContentsMargins(6,6,6,0);

    layoutTop->addWidget(lblPicture);
    layoutTop->addWidget(m_lblTitle);
    layoutTop->addStretch(2);

    QHBoxLayout* layoutButton = new QHBoxLayout();
    layoutButton->setSpacing(0);

    for (QPushButton* b : m_buttons)
    {
        layoutButton->addWidget(b);
        b->setCheckable(true);
        b->setAutoExclusive(true);

        connect(b, &QPushButton::toggled, this, &SectorsTab::buttonToggled);
    }

    layoutTop->addLayout(layoutButton);

    m_buttons[0]->setChecked(true);

    mainLayout->addLayout(layoutTop);
    mainLayout->addWidget(m_stack);

    QFont f;
    f.setPointSize(14);
    f.setBold(true);
    m_lblTitle->setFont(f);

}

void SectorsTab::onPortfolioNameChanged()
{
    m_lblTitle->setText(tr("Sectors - %1").arg(m_portfolio->portfolioName()));
}

void SectorsTab::buttonToggled(bool _toggled)
{
    QPushButton* button = qobject_cast<QPushButton*>(sender());

    if (button && _toggled)
    {
        int idx = m_buttons.indexOf(button);

        if (idx != -1)
        {
            m_stack->setCurrentIndex(idx);
        }
    }
}

