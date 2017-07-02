#include "centralwidget.h"
#include "tabinterface.h"
#include "accountpane.h"
#include "report/reportselector.h"
#include "report/ichart.h"
#include "hometab.h"
#include <QSettings>
#include <QToolBox>
#include <KangarooLib/ui/core.h>
#include "welcomescreen.h"

#include "homewidgets/categoryvaluechart.h"
#include "homewidgets/billreminder.h"

#include "report/charts/spendingincomeovertime.h"
#include "report/charts/accountbalancesovertime.h"

#include "../AccountCreateEdit/formeditaccount.h"

/****************** Build-in panes ******************/

class AccountSidePane : public ISidePane
{
    public:
        QString title() const override
        {
            return QObject::tr("Accounts");
        }

        int weight() const override
        {
            return 0;
        }

        QWidget* buildPane(QWidget* _parent) const override
        {
            return new AccountPane(_parent);
        }
};

class ReportsSidePane : public ISidePane
{
    public:
        QString title() const override
        {
            return QObject::tr("Reports");
        }

        int weight() const override
        {
            return 50;
        }

        QWidget* buildPane(QWidget* _parent) const override
        {
            return new ReportSelector(_parent);
        }
};

CentralWidget* CentralWidget::m_instance = nullptr;

CentralWidget::CentralWidget(QWidget* _parent) :
    QSplitter(_parent),
    m_sidePane(nullptr),
    m_homeTab(nullptr)
{
    //Register the built-in tabs
    registerSidePane(new ReportsSidePane());
    registerSidePane(new AccountSidePane());

    //Create the Tab Interface
    TabInterface::instance();

    m_welcomeScreen = new WelcomeScreen();
    addWidget(m_welcomeScreen);
}

CentralWidget* CentralWidget::instance()
{
    if ( !m_instance)
    {
        m_instance = new CentralWidget();


        //Register home widgets
        HomeTab::registerHomeWidget(HomeWidgetInfo("spendingchart",
                                                   CategoryValueChart::titleFor(CategoryValueType::Spending),
                                                   tr("Chart of spending. Can be configured to show monthly or yearly spending."),
                                                   KLib::Core::icon("expense-account"),
                                                   true,
                                                   [](QWidget* parent) {
                                        return new CategoryValueChart(CategoryValueType::Spending, parent);
                                    }));

        HomeTab::registerHomeWidget(HomeWidgetInfo("incomechart",
                                                   CategoryValueChart::titleFor(CategoryValueType::Income),
                                                   tr("Chart of income. Can be configured to show monthly or yearly income."),
                                                   KLib::Core::icon("expense-account"),
                                                   true,
                                                   [](QWidget* parent) {
                                        return new CategoryValueChart(CategoryValueType::Income, parent);
                                    }));

        HomeTab::registerHomeWidget(HomeWidgetInfo("billreminder",
                                                   tr("Warnings and Bill Reminders"),
                                                   tr("Displays various warnings and shows reminders of upcoming unpaid bills."),
                                                   KLib::Core::icon("dialog-warning"),
                                                   true,
                                                   [](QWidget* parent) {
                                        return new BillReminder(parent);
                                    }));

        FormEditAccount::registerTab([](QWidget* parent) { return new AccountWarningsTab(parent); });

        //Register charts
        ReportSelector::registerChart(ChartInfo("spendingincomeovertime",
                                                tr("Spending/Income Over Time"),
                                                KLib::Core::icon("office-chart-bar"),
                                                [](QWidget* parent) {
                                          return new SpendingIncomeOverTime(parent);
                                      }));

        ReportSelector::registerChart(ChartInfo("balanceovertime",
                                                tr("Account Balance Over Time"),
                                                KLib::Core::icon("office-chart-line"),
                                                [](QWidget* parent) {
                                          return new AccountBalancesOverTime(parent);
                                      }));

    }

    return m_instance;
}

void CentralWidget::registerSidePane(ISidePane* _pane)
{
    if (_pane)
    {
        m_panes.insert(_pane->weight(), _pane);
    }
}

void CentralWidget::onLoad()
{
    m_homeTab = new HomeTab(this);
    m_sidePane = new QToolBox(this);

    TabInterface::instance()->addRegisteredTab(m_homeTab, tr("Home"), "home", false, KLib::Core::icon("home"));

    m_welcomeScreen->setParent(nullptr); //Remove it from the splitter

    addWidget(m_sidePane);
    addWidget(TabInterface::instance());

    insertWidget(0, m_sidePane);

    //Insert all the tabs in the side pane
    for (ISidePane* p : m_panes)
    {
        m_sidePane->addItem(p->buildPane(m_sidePane), p->title());
    }

    QSettings s;
    setSizes({ s.value("CentralWidget/InfoPaneSize", 200).toInt(),
               s.value("CentralWidget/TabWidgetSize", 900).toInt()
             });
}

void CentralWidget::saveSizes()
{
    if (sizes().count() == 2)
    {
        QSettings s;
        s.setValue("CentralWidget/InfoPaneSize", sizes().at(0));
        s.setValue("CentralWidget/TabWidgetSize", sizes().at(1));
    }
}

void CentralWidget::onUnload()
{
    saveSizes();
    TabInterface::instance()->closeAllTabs();

    TabInterface::instance()->setParent(nullptr);

    delete m_homeTab;
    delete m_sidePane;

    m_homeTab  = nullptr;
    m_sidePane = nullptr;

    addWidget(m_welcomeScreen);
}
