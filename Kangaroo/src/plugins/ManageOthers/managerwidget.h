#ifndef MANAGERWIDGET_H
#define MANAGERWIDGET_H

enum class ManageType
{
    Currencies,
    Institutions,
    Payees,
    Securities,
    Indexes,
    Prices,
};

unsigned int qHash(ManageType _key, unsigned int seed = 0);

#include <QWidget>
#include <QTableView>

class QAction;
class QTreeView;

class BetterTableView : public QTableView
{
        Q_OBJECT

    public:
        BetterTableView(QWidget* _parent);

        QList<int> selectedRows() const;

    signals:
        void selectedRowsChanged(QList<int> _rows);

    protected:
        void selectionChanged(const QItemSelection& _selected, const QItemSelection& _deselected);
};



class ManagerWidget : public QWidget
{
    Q_OBJECT

    public:

        ManagerWidget(ManageType _type, QWidget* _parent = 0);

    private slots:
        void add();
        void remove();
        void edit();
        void search();
        void merge();

        void onSelectedRowsChanged(QList<int> _rows);
        void onSelectionChangeTreeView();

    private:
        ManageType m_type;

        BetterTableView* m_tableView;
        QTreeView*       m_treeView;

        QAction* m_actAdd;
        QAction* m_actEdit;
        QAction* m_actMerge;
        QAction* m_actDelete;
        QAction* m_actFind;



};

#endif // MANAGERWIDGET_H
