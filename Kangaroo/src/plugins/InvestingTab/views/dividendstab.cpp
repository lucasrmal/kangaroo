#include "dividendstab.h"
#include "../models/dividendsmodel.h"
#include "../models/portfolio.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/widgets/percchart.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/ui/widgets/dateintervalselector.h>
#include <QTableView>
#include <QVBoxLayout>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

using namespace KLib;

DividendsTab::DividendsTab(Portfolio* _portfolio, QWidget *parent) :
    StandardTab(_portfolio, tr("Dividends"), parent),
    m_model(new DividendsModel(QDate(QDate::currentDate().year(), 1, 1), QDate::currentDate(), _portfolio, this))
{

    m_pieChart = new PercChart(ChartStyle::Pie,
                               DividendsModelColumn::Symbol,
                               DividendsModelColumn::DividendsReceived,
                               Qt::EditRole, this);
    m_pieChart->setModel(m_model);


    m_barChart= new PercChart(ChartStyle::HorizontalBar,
                              DividendsModelColumn::Symbol,
                              DividendsModelColumn::DividendsReceived,
                              Qt::EditRole, this);
    m_barChart->setModel(m_model);

    m_pieChart->setLegendColumn(DividendsModelColumn::Name);
    m_barChart->setLegendColumn(DividendsModelColumn::Name);

    addTab(tr("Pie"), m_pieChart);
    addTab(tr("Bar"), m_barChart);

    m_dateSelector = new DateIntervalSelector(DateIntervalSelector::Flag_All, this);
    m_dateSelector->setYTD();
    connect(m_dateSelector, &DateIntervalSelector::intervalChanged, m_model, &DividendsModel::setInterval);

    m_layoutBottom->addWidget(new QLabel(tr("Interval:"), this));
    m_layoutBottom->addWidget(m_dateSelector);
    m_layoutBottom->addStretch(2);

    connect(this, &DividendsTab::refreshRequested, m_model, &DividendsModel::refresh);
}
