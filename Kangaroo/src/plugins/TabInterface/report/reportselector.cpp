#include "reportselector.h"
#include "report.h"
#include "formsetsettings.h"
#include "reportviewer.h"
#include "../TabInterface/tabinterface.h"

#include <KangarooLib/ui/widgets/pdfviewer.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/controller/io.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>

#include <QMenu>
#include <QMenuBar>
#include <QDateTime>
#include <QTreeWidget>
#include <QFileInfo>
#include <QDir>
#include <QMessageBox>
#include <QHeaderView>
#include <QDebug>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QToolButton>

using namespace KLib;

KLib::ActionContainer* ReportSelector::m_reportMenu = nullptr;
QAction* ReportSelector::m_actOpen = nullptr;
QAction* ReportSelector::m_actOpenCustom = nullptr;
QAction* ReportSelector::m_actReloadAll = nullptr;
QAction* ReportSelector::m_actPrint = nullptr;
QAction* ReportSelector::m_actSaveAs = nullptr;
QAction* ReportSelector::m_actReload = nullptr;
QMenu*   ReportSelector::m_reportSelectorMenu = nullptr;

QHash<QString, ChartInfo> ReportSelector::m_chartInfos = QHash<QString, ChartInfo>();

ReportSelector::ReportSelector(QWidget *parent) :
    QWidget(parent),
    m_tree(new QTreeWidget(this))
{
    setObjectName("Reports");

    QHBoxLayout* buttonLayout = new QHBoxLayout();

    btnOpen = new QToolButton(this);
    btnOpenCustom = new QToolButton(this);
    btnReloadAll = new QToolButton(this);

    buttonLayout->addWidget(btnOpen);
    buttonLayout->addWidget(btnOpenCustom);
    buttonLayout->addStretch(5);
    buttonLayout->addWidget(btnReloadAll);

    QPalette p = palette();
    p.setColor(QPalette::Button, p.color(QPalette::Window));
    setPalette(p);


    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addLayout(buttonLayout);
    layout->addWidget(m_tree);
    layout->setContentsMargins(4,0,0,0);

    reloadReports();

    m_tree->header()->hide();

    m_tree->setContextMenuPolicy(Qt::CustomContextMenu);


    //Report Menu
    if (!m_reportMenu)
    {
        m_reportMenu = ActionManager::instance()->insertMenu(STD_MENUS::MENUBAR,
                                                             STD_MENUS::MENU_TOOLS,
                                                             "Report",
                                                             tr("&Report"));

        m_reportSelectorMenu = new QMenu();


        m_actSaveAs = m_reportMenu->menu()->addAction(Core::icon("save-as"), tr("&Save To File"));
        m_actPrint = m_reportMenu->menu()->addAction(Core::icon("print"), tr("&Print"));
        m_reportMenu->menu()->addSeparator();
        m_actReload = m_reportMenu->menu()->addAction(Core::icon("view-refresh"), tr("&Reload"));

        m_actOpen = m_reportSelectorMenu->addAction(Core::icon("document-quickview"), tr("Open using &default settings"));
        m_actOpenCustom = m_reportSelectorMenu->addAction(Core::icon("document-preview"), tr("Open using &custom settings"));
        m_reportSelectorMenu->addSeparator();
        m_actReloadAll = m_reportSelectorMenu->addAction(Core::icon("view-refresh"), tr("Reload all reports"));

//        ActionManager::instance()->registerAction(m_actOpen,       "Report.OpenDefault");
//        ActionManager::instance()->registerAction(m_actOpenCustom, "Report.OpenCustom");
        ActionManager::instance()->registerAction(m_actReload,     "Report.Reoad");
        ActionManager::instance()->registerAction(m_actSaveAs,     "Report.SaveToFile");
        ActionManager::instance()->registerAction(m_actPrint,      "Report.Print");
    }
    else
    {
        m_reportMenu->menu()->setEnabled(true);
        m_reportSelectorMenu->setEnabled(true);
    }

    m_actOpen->setEnabled(false);
    m_actOpenCustom->setEnabled(false);
    m_actSaveAs->setEnabled(false);
    m_actPrint->setEnabled(false);
    m_actReload->setEnabled(false);

    btnOpen->setDefaultAction(m_actOpen);
    btnOpenCustom->setDefaultAction(m_actOpenCustom);
    btnReloadAll->setDefaultAction(m_actReloadAll);

    connect(m_actOpen,       &QAction::triggered, this, &ReportSelector::openDefault);
    connect(m_actOpenCustom, &QAction::triggered, this, &ReportSelector::openCustom);
    connect(m_actReloadAll,  &QAction::triggered, this, &ReportSelector::reloadReports);
    connect(m_actSaveAs,     &QAction::triggered, this, &ReportSelector::saveCurrent);
    connect(m_actPrint,      &QAction::triggered, this, &ReportSelector::printCurrent);
    connect(m_actReload,     &QAction::triggered, this, &ReportSelector::reloadCurrent);

    connect(TabInterface::instance(), SIGNAL(currentChanged(int)), this, SLOT(currentTabChanged()));

    connect(m_tree, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

    connect(m_tree, SIGNAL(itemDoubleClicked(QTreeWidgetItem*,int)),
            this, SLOT(itemDoubleClicked(QTreeWidgetItem*,int)));

    connect(m_tree, SIGNAL(itemSelectionChanged()), this, SLOT(enableOpenActions()));
}

void ReportSelector::enableOpenActions()
{
    bool enab = m_tree->selectedItems().count();
    m_actOpen->setEnabled(enab);
    m_actOpenCustom->setEnabled(enab);
}

ReportSelector::~ReportSelector()
{
    onUnload();
    unloadReports();
    m_reportMenu->menu()->setEnabled(false);
}

void ReportSelector::loadRecursive(QTreeWidgetItem* _item, const QDir& _currentDir)
{
    foreach(QFileInfo entry, _currentDir.entryInfoList(QDir::NoFilter, QDir::Name | QDir::DirsFirst))
    {
        if (entry.fileName() != "." && entry.fileName() != "..")
        {
            if (entry.isDir())
            {
                QTreeWidgetItem* subItem = new QTreeWidgetItem(_item);
                subItem->setText(0, entry.fileName());
                subItem->setIcon(0, Core::icon("folder-yellow"));
                subItem->setData(0, Qt::UserRole, entry.absoluteFilePath());

                loadRecursive(subItem, QDir(entry.absoluteFilePath()));
            }
            else if (entry.fileName().endsWith(".xml"))
            {
                Report* r = Report::loadFromFile(entry.absoluteFilePath());

                if (r)
                {
                    QTreeWidgetItem* subItem = new QTreeWidgetItem(_item);
                    subItem->setText(0, r->name);
                    subItem->setIcon(0, Core::icon(r->icon));
                    subItem->setData(0, Qt::UserRole, m_reports.count());

                    r->path = _currentDir.absoluteFilePath(r->path);
                    m_reports << r;
                }
            }
        }
    }
}

void ReportSelector::reloadReports()
{
    m_tree->clear();

    //Reports
    QTreeWidgetItem* reportsRoot = new QTreeWidgetItem(m_tree);
    reportsRoot->setText(0, tr("Reports"));
    reportsRoot->setIcon(0, Core::icon("folder-yellow"));

    loadRecursive(reportsRoot, QDir(Core::path(Path_Reports)));

    //Charts
    m_chartsRoot = new QTreeWidgetItem(m_tree);
    m_chartsRoot->setText(0, tr("Charts"));
    m_chartsRoot->setIcon(0, Core::icon("folder-yellow"));

    for (const ChartInfo& i : m_chartInfos)
    {
        QTreeWidgetItem* subItem = new QTreeWidgetItem(m_chartsRoot);
        subItem->setText(0, i.name);
        subItem->setIcon(0, i.icon);
        subItem->setData(0, Qt::UserRole, i.code);
    }

    m_tree->expandAll();
}

void ReportSelector::unloadReports()
{
    m_tree->clear();

    for (Report* r : m_reports)
    {
        delete r;
    }
}

void ReportSelector::openReport(int _id, bool _openSettings)
{
    Report* report = m_reports[_id];
    ReportSettings settings = report->settings;
    Report::setSettingsToDefault(settings);

    //Settings
    if (_openSettings || Report::requiresManualSettings(settings))
    {
        FormSetSettings* s = new FormSetSettings(settings, this);
        bool cont = s->exec();
        delete s;

        if (!cont)
        {
            return;
        }
    }

    QString html = report->generateHtml(settings);
    QString tempid = QString("report_%1").arg(QDateTime::currentDateTime().toTime_t());

    ReportViewer* view = new ReportViewer(report, settings);
    view->setContent(html);

    TabInterface::instance()->addRegisteredTab(view,
                                               report->name,
                                               tempid,
                                               true,
                                               Core::icon(report->icon));

    m_reportViewers << view;
}

void ReportSelector::openChart(const QString& _code)
{
    if (!m_chartInfos.contains(_code))
        return;

    if (!m_chartViewers.contains(_code))
    {
        m_chartViewers.insert(_code, m_chartInfos[_code].build(this));
    }

    TabInterface::instance()->addRegisteredTab(m_chartViewers[_code],
                                               m_chartInfos[_code].name,
                                               QString("chart_%1").arg(_code),
                                               true,
                                               m_chartInfos[_code].icon);
}

void ReportSelector::registerChart(const ChartInfo& _infos)
{
    QString error;
    //Check the required infos
    if (_infos.code.isEmpty())
    {
        error = tr("Chart code may not be empty.");
    }
    else if (_infos.name.isEmpty())
    {
        error = tr("Chart name may not be empty.");
    }
    else if (m_chartInfos.contains(_infos.code))
    {
        error = tr("A chart with the same code already exists.");
    }

    if (!error.isEmpty())
    {
        qDebug() << tr("Unable to load Chart %1: %2").arg(_infos.code).arg(error);
        return;
    }

    m_chartInfos.insert(_infos.code, _infos);
}

void ReportSelector::itemDoubleClicked(QTreeWidgetItem* _item, int)
{
    if (!_item)
        return;

    if (_item->parent() == m_chartsRoot)
    {
        openChart(_item->data(0, Qt::UserRole).toString());
    }
    else
    {
        openReport(_item->data(0, Qt::UserRole).toInt());
    }
}

void ReportSelector::currentTabChanged()
{
    bool enab = qobject_cast<ReportViewer*>(TabInterface::instance()->currentWidget()) != nullptr;

    m_actSaveAs->setEnabled(enab);
    m_actPrint->setEnabled(enab);
    m_actReload->setEnabled(enab);
}

void ReportSelector::openDefault()
{
    if (m_tree->selectedItems().count())
    {
        openReport(m_tree->currentItem()->data(0, Qt::UserRole).toInt());
    }
}

void ReportSelector::openCustom()
{
    if (m_tree->selectedItems().count())
    {
        openReport(m_tree->currentItem()->data(0, Qt::UserRole).toInt(), true);
    }
}

void ReportSelector::printCurrent()
{
    ReportViewer* v = qobject_cast<ReportViewer*>(TabInterface::instance()->currentWidget());

    if (v)
    {
        v->print();
    }
}

void ReportSelector::saveCurrent()
{
    ReportViewer* v = qobject_cast<ReportViewer*>(TabInterface::instance()->currentWidget());

    if (v)
    {
        v->saveAs();
    }
}

void ReportSelector::reloadCurrent()
{
    ReportViewer* v = qobject_cast<ReportViewer*>(TabInterface::instance()->currentWidget());

    if (v)
    {
        v->reload();
    }
}

void ReportSelector::onUnload()
{
    for (ReportViewer* t : m_reportViewers)
    {
        t->deleteLater();
    }

    for (IChart* c : m_chartViewers)
    {
        c->deleteLater();
    }

    m_reportViewers.clear();
    m_chartViewers.clear();
}

void ReportSelector::contextMenuRequested(const QPoint& _pos)
{
    m_reportSelectorMenu->popup(m_tree->viewport()->mapToGlobal(_pos));
}
