#ifndef INFOPANE_H
#define INFOPANE_H

#include <QToolBox>

class QTreeView;

class InfoPane : public QToolBox
{
    Q_OBJECT

    public:
        explicit InfoPane(QWidget *parent = nullptr);

    private:
        QTreeView* m_treeAccounts;

};

#endif // INFOPANE_H
