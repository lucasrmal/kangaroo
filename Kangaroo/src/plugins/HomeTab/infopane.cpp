#include "infopane.h"

#include <QTreeView>
#include <KangarooLib/controller/simpleaccountcontroller.h>

using namespace KLib;

InfoPane::InfoPane(QWidget *parent) :
    QToolBox(parent)
{
    m_treeAccounts = new QTreeView(this);
    m_treeAccounts->setModel(new SimpleAccountController());

    addItem(m_treeAccounts, tr("Accounts"));
}
