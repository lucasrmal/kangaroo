#include "accounttreewidget.h"

#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/controller/accountcontroller.h>
#include <KangarooLib/model/account.h>
#include <QDebug>
#include <QHeaderView>
#include <QMenu>

using namespace KLib;

QSize AccountTreeDelegate::sizeHint(const QStyleOptionViewItem& option, const QModelIndex& index) const
{
    Q_UNUSED(option)
    Q_UNUSED(index)
    return QSize(20,20);
}

AccountTreeWidget::AccountTreeWidget(QWidget *parent) :
    QTreeView(parent),
    m_controller(NULL)
{
    resetModel();
    setExpandsOnDoubleClick(false);
    setItemDelegate(new AccountTreeDelegate(this));
    setSortingEnabled(true);
    header()->setSortIndicator(AccountColumn::NAME, Qt::AscendingOrder);
    //header()->setStretchLastSection(false);

    setContextMenuPolicy(Qt::CustomContextMenu);
    //addAction(new QAction("toto", this));
    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

//    connect(this->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onSelectionChanged()));
//    connect(this->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(onSelectionChanged()));
}

void AccountTreeWidget::onSelectionChanged()
{
    QModelIndexList current = this->selectionModel()->selectedIndexes();

    if (current.size() > 0)
        emit currentAccountChanged(static_cast<AccountController::AccountNode*>(m_controller->mapToSource(current[0]).internalPointer())->account);
}

void AccountTreeWidget::contextMenuRequested(QPoint pos)
{
    QMenu* menu = ActionManager::instance()->actionContainer("Account")->menu();
    menu->popup(viewport()->mapToGlobal(pos));
}

void AccountTreeWidget::selectionChanged(const QItemSelection& _selected, const QItemSelection& _deselected)
{
    onSelectionChanged();
    return QTreeView::selectionChanged(_selected, _deselected);
}

void AccountTreeWidget::resetModel(bool _openOnly)
{
    if (m_controller) m_controller->deleteLater();

    m_controller = new AccountSortFilterProxyModel(this);
    m_controller->setSourceModel(new AccountController(Account::getTopLevel(), _openOnly, this));
    setModel(m_controller);
    expandAll();
    setColumnWidth(0, 400);
    setColumnWidth(1, 140);
    setColumnWidth(2, 140);
    //setColumnWidth(3, 120);
    //setColumnWidth(4, 120);

    setStyleSheet("QTreeView {alternate-background-color: #FFF3E0;background-color: #FFFFFF; }");
    setAlternatingRowColors(true);
}

KLib::Account* AccountTreeWidget::currentAccount() const
{
    QModelIndexList current = this->selectionModel()->selectedIndexes();

    if (!m_controller || current.isEmpty())
        return NULL;

    return static_cast<AccountController::AccountNode*>(m_controller->mapToSource(current[0]).internalPointer())->account;
}
