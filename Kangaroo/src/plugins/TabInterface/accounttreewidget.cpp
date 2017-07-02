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

void AccountTreeDelegate::paint(QPainter* _painter, const QStyleOptionViewItem& _option, const QModelIndex& _index) const
{
    //We do not want the focus to be painted
    QStyleOptionViewItem newOption = _option;
    newOption.state = _option.state & (~QStyle::State_HasFocus);
    QStyledItemDelegate::paint(_painter, newOption, _index);
}

AccountTreeWidget::AccountTreeWidget(QWidget* _parent) :
    AccountTreeWidget(Account::getTopLevel(), true, AccountTypeFlags::Flag_All, _parent)
{
}

AccountTreeWidget::AccountTreeWidget(Account* _topLevel, bool _openAccountsOnly, int _flags, QWidget* _parent) :
    QTreeView(_parent),
    m_sourceModel(new AccountController(_topLevel, _openAccountsOnly, _flags, this)),
    m_contextMenu(nullptr)
{
    m_controller = new AccountSortFilterProxyModel(this);
    m_controller->setSourceModel(m_sourceModel);
    setModel(m_controller);
    expandAll();
    setColumnWidth(0, 400);
    setColumnWidth(1, 140);
    setColumnWidth(2, 140);

    //setStyleSheet("QTreeView {alternate-background-color: #FFF3E0;background-color: #FFFFFF; }");
    setStyleSheet("QTreeView {alternate-background-color: #FFF9EF;background-color: #FFFFFF; }");
    setAlternatingRowColors(true);

    setExpandsOnDoubleClick(false);
    setItemDelegate(new AccountTreeDelegate(this));
    setSortingEnabled(true);
    header()->setSortIndicator(AccountColumn::NAME, Qt::AscendingOrder);
    //header()->setStretchLastSection(false);

    setContextMenuPolicy(Qt::CustomContextMenu);

    connect(this, SIGNAL(customContextMenuRequested(QPoint)),
            this, SLOT(contextMenuRequested(QPoint)));

//    connect(this->selectionModel(), SIGNAL(currentChanged(QModelIndex,QModelIndex)), this, SLOT(onSelectionChanged()));
//    connect(this->selectionModel(), SIGNAL(selectionChanged(QItemSelection,QItemSelection)), this, SLOT(onSelectionChanged()));
}

void AccountTreeWidget::setContextMenu(QMenu* _menu)
{
    m_contextMenu = _menu;
}

void AccountTreeWidget::onSelectionChanged()
{
    emit currentAccountChanged(m_sourceModel->accountForIndex(
                                   m_controller->mapToSource(
                                       this->selectionModel()->currentIndex())));
}

void AccountTreeWidget::contextMenuRequested(QPoint pos)
{
    if (m_contextMenu)
        m_contextMenu->popup(viewport()->mapToGlobal(pos));
}

void AccountTreeWidget::selectionChanged(const QItemSelection& _selected, const QItemSelection& _deselected)
{
    onSelectionChanged();
    return QTreeView::selectionChanged(_selected, _deselected);
}

KLib::Account* AccountTreeWidget::currentAccount() const
{
    return m_sourceModel->accountForIndex(
                m_controller->mapToSource(
                    this->selectionModel()->currentIndex()));
}
