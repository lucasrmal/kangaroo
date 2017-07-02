#include "hometab.h"
#include "formedithometabwidgets.h"

#include <QLabel>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QDebug>
#include <QSettings>
#include <QPushButton>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/command.h>
#include <QAction>
#include <QDate>

using namespace KLib;

QHash<QString, HomeWidgetInfo> HomeTab::m_homeWidgets = QHash<QString, HomeWidgetInfo>();

HomeTab::HomeTab(QWidget* _parent) :
    QScrollArea(_parent)
{
    QLabel* lblTitle = new QLabel();
    lblTitle->setText(QString("<b>%1 %2 - %3</b>")
                      .arg(Core::APP_NAME)
                      .arg(Core::APP_VERSION)
                      .arg(QDate::currentDate().toString(Qt::DefaultLocaleLongDate)));

    QAction* actViewAllAccounts = ActionManager::instance()->command("View.AllAccounts")->action();
    QAction* actViewIncExp = ActionManager::instance()->command("View.IncomeExpenseAccounts")->action();

    QPushButton* btnViewAccounts = new QPushButton(Core::icon("bank"), tr("All Accounts"), this);
    QPushButton* btnViewIncExp   = new QPushButton(Core::icon("income-account"), tr("Income and Expense Accounts"), this);
    QPushButton* btnConfigure    = new QPushButton(Core::icon("configure"), tr("Configure"), this);

    btnViewAccounts->setFlat(true);
    btnViewIncExp->setFlat(true);
    btnConfigure->setFlat(true);

    connect(btnViewAccounts, &QPushButton::clicked, actViewAllAccounts, &QAction::trigger);
    connect(btnViewIncExp,   &QPushButton::clicked, actViewIncExp,      &QAction::trigger);
    connect(btnConfigure,    &QPushButton::clicked, this,               &HomeTab::editWidgets);

    QWidget* scrollWidget = new QWidget(this);
    setWidgetResizable(true);
    setWidget(scrollWidget);

    QWidget* wTop = new QWidget(this);
    QHBoxLayout* layoutTop = new QHBoxLayout(wTop);
    layoutTop->setContentsMargins(0,0,0,0);
    layoutTop->addWidget(lblTitle);
    layoutTop->addStretch(3);
    layoutTop->addWidget(btnViewAccounts);
    layoutTop->addWidget(btnViewIncExp);
    layoutTop->addWidget(btnConfigure);



    m_layout = new QVBoxLayout(scrollWidget);
    m_layout->addWidget(wTop);
    //m_layout->setContentsMargins(0,0,0,0);
    //setLayout(m_layout);

    loadWidgets();

    m_layout->addStretch(10);
}

HomeTab::~HomeTab()
{
    saveWidgets();
}

void HomeTab::editWidgets()
{
    FormEditHomeTabWidgets* frm = new FormEditHomeTabWidgets(this);
    frm->exec();
    delete frm;
}

void HomeTab::removeWidget()
{

}

void HomeTab::configureWidget()
{

}

void HomeTab::loadWidgets()
{
    QSettings settings;
    QStringList displayed = settings.value("HomeWidget/Displayed").toStringList();

    if (displayed.isEmpty()) //Load default
    {
        for (const HomeWidgetInfo& info : m_homeWidgets)
        {
            if (info.showByDefault)
            {
                displayed << info.code;
            }
        }
    }

    //Now load these widgets
    for (const QString& code : displayed)
    {
        if (m_homeWidgets.contains(code))
        {
            IHomeWidget* w = m_homeWidgets[code].build(this);
            w->setMaximumHeight(MAX_WIDGET_HEIGHT);

            m_layout->addWidget(w);
            m_displayedWidgets.append(HomeWidgetPair(code, w));
        }
    }
}

void HomeTab::saveWidgets()
{
    QSettings settings;
    QStringList displayed;

    for (const HomeWidgetPair& p : m_displayedWidgets)
    {
        displayed << p.first;
    }

    settings.setValue("HomeWidget/Displayed", displayed);
}

void HomeTab::registerHomeWidget(const HomeWidgetInfo& _infos)
{
    QString error;
    //Check the required infos
    if (_infos.code.isEmpty())
    {
        error = tr("Widget code may not be empty.");
    }
    else if (_infos.name.isEmpty())
    {
        error = tr("Widget name may not be empty.");
    }
    else if (m_homeWidgets.contains(_infos.code))
    {
        error = tr("A widget with the same code already exists.");
    }

    if (!error.isEmpty())
    {
        qDebug() << tr("Unable to load Home Widget %1: %2").arg(_infos.code).arg(error);
        return;
    }

    m_homeWidgets.insert(_infos.code, _infos);
}



