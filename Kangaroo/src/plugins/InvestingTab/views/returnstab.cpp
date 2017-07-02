#include "returnstab.h"
#include "../models/portfolio.h"
#include "../models/positionsreturnmodel.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/widgets/returnchart.h>
#include <KangarooLib/model/account.h>
#include <QVBoxLayout>
#include <QDateEdit>
#include <QLabel>
#include <QPushButton>
#include <QStackedWidget>

using namespace KLib;

ReturnsTab::ReturnsTab(Portfolio* _portfolio, QWidget* _parent) :
    QWidget(_parent),
    m_portfolio(_portfolio),
    m_returnsModel(new PositionsReturnModel(m_portfolio, this))
{

    m_returnsRangeChart = new ReturnChart(ReturnChartStyle::VerticalBar,
                                          ValueType::Percent,
                                          Qt::EditRole,
                                          this);

    m_returnsRangeChart->setModel(m_returnsModel);

    m_quarterlyReturnsChart = new ReturnChart(ReturnChartStyle::Line,
                                              ValueType::Percent,
                                              Qt::EditRole,
                                              this);

    m_quarterlyReturnsChart->setModel(new DetailedPositionsReturnModel(DetailedPositionsReturnModel::RangeType::Quarterly,
                                                                       m_returnsModel, this));

    m_annualReturnsChart = new ReturnChart(ReturnChartStyle::Line,
                                           ValueType::Percent,
                                           Qt::EditRole,
                                           this);

    m_annualReturnsChart->setModel(new DetailedPositionsReturnModel(DetailedPositionsReturnModel::RangeType::Annual,
                                                                    m_returnsModel, this));

    m_valueChart = new ReturnChart(ReturnChartStyle::Line,
                                   ValueType::Amount,
                                   Qt::EditRole,
                                   this);

    m_valueChart->setModel(new PositionsValueModel(m_returnsModel, this));


    m_lblTitle = new QLabel(tr("Returns - %1").arg(m_portfolio->portfolioName()), this);
    QLabel* lblPicture = new QLabel(this);
    lblPicture->setPixmap(Core::pixmap("view-form-table"));

    m_dteAsOf = new QDateEdit(this);
    m_dteAsOf->setCalendarPopup(true);
    m_dteAsOf->setDate(QDate::currentDate());

    m_lblDetails = new QLabel(tr("*Using time-weighted rates of return."), this);
    m_lblDetails->setAlignment(Qt::AlignVCenter | Qt::AlignRight);

    connect(m_dteAsOf, &QDateEdit::dateChanged, this, &ReturnsTab::onDateChanged);

    m_buttons << new QPushButton(/*Core::icon("office-table"),*/     tr("Returns Summary"), this)
              << new QPushButton(/*Core::icon("office-table"),*/     tr("Quarterly Returns"), this)
              << new QPushButton(/*Core::icon("office-table"),*/     tr("Annual Returns"), this)
              << new QPushButton(/*Core::icon("office-table"),*/     tr("Value Over Time"), this);

    QVBoxLayout* mainLayout = new QVBoxLayout(this);
    QHBoxLayout* layoutTop = new QHBoxLayout();

    m_stack = new QStackedWidget(this);
    m_stack->addWidget(m_returnsRangeChart);
    m_stack->addWidget(m_quarterlyReturnsChart);
    m_stack->addWidget(m_annualReturnsChart);
    m_stack->addWidget(m_valueChart);

    mainLayout->setContentsMargins(0,0,0,0);
    layoutTop->setContentsMargins(6,6,6,0);

    layoutTop->addWidget(lblPicture);
    layoutTop->addWidget(m_lblTitle);
    layoutTop->addStretch(2);

    layoutTop->addWidget(new QLabel(tr("As Of:"), this));
    layoutTop->addWidget(m_dteAsOf);
    layoutTop->addSpacing(20);

    QHBoxLayout* layoutButton = new QHBoxLayout();
    layoutButton->setSpacing(0);

    for (QPushButton* b : m_buttons)
    {
        layoutButton->addWidget(b);
        b->setCheckable(true);
        b->setAutoExclusive(true);

        connect(b, &QPushButton::toggled, this, &ReturnsTab::buttonToggled);
    }

    layoutTop->addLayout(layoutButton);

    m_buttons[0]->click();

    mainLayout->addLayout(layoutTop);
    mainLayout->addWidget(m_stack);
    mainLayout->addWidget(m_lblDetails);

    QFont f;
    f.setPointSize(14);
    f.setBold(true);
    m_lblTitle->setFont(f);
}

void ReturnsTab::onPortfolioNameChanged()
{
    m_lblTitle->setText(tr("Sectors - %1").arg(m_portfolio->portfolioName()));
}

void ReturnsTab::onDateChanged(const QDate& _date)
{
    m_returnsModel->setReportDate(_date);
}

void ReturnsTab::buttonToggled(bool _toggled)
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

