#ifndef REPORTSELECTOR_H
#define REPORTSELECTOR_H

#include <QWidget>
#include "ichart.h"

class QTreeWidget;
class Report;
class QTreeWidgetItem;
class QDir;

namespace KLib
{
    class ActionContainer;
}

class ReportViewer;
class QToolButton;

class QAction;
class QMenu;

class ReportSelector : public QWidget
{
    Q_OBJECT

    public:
        explicit ReportSelector(QWidget *parent = nullptr);

        ~ReportSelector();

        void openReport(int _id, bool _openSettings = false);
        void openChart(const QString& _code);


        static void registerChart(const ChartInfo& _infos);

    public slots:
        void reloadReports();
        void itemDoubleClicked(QTreeWidgetItem* _item, int);

        void currentTabChanged();

        void openDefault();
        void openCustom();

        void printCurrent();
        void saveCurrent();
        void reloadCurrent();

        void onUnload();

        void contextMenuRequested(const QPoint& _pos);
        void enableOpenActions();

    private:
        void unloadReports();
        void loadRecursive(QTreeWidgetItem* _item, const QDir& _currentDir);

        QList<Report*>       m_reports;
        QList<ReportViewer*> m_reportViewers;
        QHash<QString, IChart*> m_chartViewers;

        QTreeWidgetItem* m_chartsRoot;

        QToolButton* btnOpen;
        QToolButton* btnOpenCustom;
        QToolButton* btnReloadAll;

        static QAction* m_actOpen;
        static QAction* m_actOpenCustom;
        static QAction* m_actReloadAll;
        static QAction* m_actReload;
        static QAction* m_actSaveAs;
        static QAction* m_actPrint;

        static QMenu* m_reportSelectorMenu;
        static KLib::ActionContainer* m_reportMenu;

        static QHash<QString, ChartInfo> m_chartInfos;

        QTreeWidget* m_tree;

};

#endif // REPORTSELECTOR_H
