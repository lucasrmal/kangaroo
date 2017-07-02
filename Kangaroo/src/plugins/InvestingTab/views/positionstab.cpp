#include "positionstab.h"
#include "../models/positionsoverviewmodel.h"
#include "../models/positionsvaluationmodel.h"
#include "../models/portfolio.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/widgets/percchart.h>
#include <KangarooLib/model/account.h>
#include <QTableView>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

using namespace KLib;

PositionsTab::PositionsTab(Portfolio* _portfolio, QWidget *parent) :
    QWidget(parent),
    m_portfolio(_portfolio),
    m_returnModel(new PositionsOverviewModel(_portfolio, this)),
    m_valuationModel(new PositionsValuationModel(_portfolio, this)),
    m_returnsTable(new QTableView(this)),
    m_valuationTable(new QTableView(this)),
    m_pieChart(new PercChart(ChartStyle::Pie,
                             PositionsOverviewColumn::Symbol,
                             PositionsOverviewColumn::MarketValue,
                             Qt::EditRole, this)),
    m_barChart(new PercChart(ChartStyle::HorizontalBar,
                             PositionsOverviewColumn::Symbol,
                             PositionsOverviewColumn::MarketValue,
                             Qt::EditRole, this))
{
    m_returnsTable->setModel(m_returnModel);
    m_valuationTable->setModel(m_valuationModel);
    m_pieChart->setModel(m_returnModel);
    m_barChart->setModel(m_returnModel);

    m_pieChart->setLegendColumn(PositionsOverviewColumn::Name);
    m_barChart->setLegendColumn(PositionsOverviewColumn::Name);

    m_returnsTable->setSortingEnabled(true);
    m_returnsTable->sortByColumn(0, Qt::AscendingOrder);
    m_returnsTable->setGridStyle(Qt::NoPen);
    m_returnsTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_returnsTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::Name, 200);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::Symbol, 70);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::LastPrice, 80);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::Change, 120);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::NumShares, 70);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::DayGain, 80);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::CostPerShare, 80);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::ProfitLoss, 70);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::PercPortfolio, 70);
    m_returnsTable->setColumnWidth(PositionsOverviewColumn::PercProfitLoss, 70);

    m_valuationTable->setGridStyle(Qt::NoPen);
    m_valuationTable->setSelectionMode(QAbstractItemView::SingleSelection);
    m_valuationTable->setSelectionBehavior(QAbstractItemView::SelectRows);
    m_valuationTable->setColumnWidth(PositionsValuationColumn::Name, 200);

    for (int i = PositionsValuationColumn::Symbol; i < PositionsValuationColumn::NumColumns; ++i)
    {
        m_valuationTable->setColumnWidth(i, 70);
    }

    m_lblTitle = new QLabel(tr("Positions - %1").arg(m_portfolio->portfolioName()), this);
    QLabel* lblPicture = new QLabel(this);
    lblPicture->setPixmap(Core::pixmap("view-form-table"));

    m_buttons << new QPushButton(/*Core::icon("office-table"),*/     tr("Returns"), this)
              << new QPushButton(/*Core::icon("office-table"),*/     tr("Valuation"), this)
              << new QPushButton(/*Core::icon("office-chart-pie"),*/ tr("Pie"), this)
              << new QPushButton(/*Core::icon("office-chart-pie"),*/ tr("Bar"), this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* layoutTop = new QHBoxLayout();

    m_stack = new QStackedWidget(this);
    m_stack->addWidget(m_returnsTable);
    m_stack->addWidget(m_valuationTable);
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

        connect(b, &QPushButton::toggled, this, &PositionsTab::buttonToggled);
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

void PositionsTab::onPortfolioNameChanged()
{
    m_lblTitle->setText(tr("Sectors - %1").arg(m_portfolio->portfolioName()));
}

void PositionsTab::buttonToggled(bool _toggled)
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
