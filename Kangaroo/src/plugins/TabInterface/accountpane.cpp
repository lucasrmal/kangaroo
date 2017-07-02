#include "accountpane.h"
#include "ledgertab.h"
#include "accountactions.h"

#include <QTreeView>
#include <QHeaderView>
#include <KangarooLib/controller/simpleaccountcontroller.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/command.h>

#include <QVBoxLayout>
#include <QPainter>
#include <QMouseEvent>
#include <QMenu>
#include <QAction>
#include <QFontMetrics>
#include <QMenu>
#include <QAction>

using namespace KLib;

KLib::ActionContainer* AccountPane::m_accountMenu = nullptr;

QAction* AccountPane::m_actCreate = nullptr;
QAction* AccountPane::m_actEdit = nullptr;
QAction* AccountPane::m_actClose = nullptr;
QAction* AccountPane::m_actReopen = nullptr;
QAction* AccountPane::m_actRemove = nullptr;
QAction* AccountPane::m_actShowClosed = nullptr;

const QColor AccountPaneTree::CATEGORY_COLOR = QColor(230,230,230);

AccountPane::AccountPane(QWidget *parent) :
    QWidget(parent)
{
    //Create the menu
    if (!m_accountMenu)
    {
        m_accountMenu = ActionManager::instance()->insertMenu(STD_MENUS::MENUBAR,
                                                              STD_MENUS::MENU_MANAGE,
                                                              "Account",
                                                              tr("&Account"));

        m_actCreate = m_accountMenu->menu()->addAction(Core::icon("create-bank-account"), tr("Add &Account"), this, SLOT(createAccount()));
        m_actEdit = m_accountMenu->menu()->addAction(Core::icon("edit"), tr("&Edit Account"), this, SLOT(editAccount()));
        m_accountMenu->menu()->addSeparator();
        m_actClose = m_accountMenu->menu()->addAction(Core::icon("close-bank-account"), tr("&Close Account"), this, SLOT(closeAccount()));
        m_actReopen = m_accountMenu->menu()->addAction(Core::icon("reopen-bank-account"), tr("Re-&open Account"), this, SLOT(reopenAccount()));
        m_actRemove = m_accountMenu->menu()->addAction(Core::icon("trash"), tr("&Delete Account"), this, SLOT(removeAccount()));
        m_accountMenu->menu()->addSeparator();
        m_actShowClosed = m_accountMenu->menu()->addAction(tr("&Show Closed Accounts"));
        m_actShowClosed->setCheckable(true);
        m_actShowClosed->setChecked(false);
        connect(m_actShowClosed, &QAction::triggered, this, &AccountPane::showClosedAccounts);

        ActionManager::instance()->registerAction(m_actCreate,  "Account.Add", QKeySequence("F6"));
        ActionManager::instance()->registerAction(m_actEdit,    "Account.Edit");
        ActionManager::instance()->registerAction(m_actClose,   "Account.Close");
        ActionManager::instance()->registerAction(m_actReopen,  "Account.Reopen");
        ActionManager::instance()->registerAction(m_actRemove,  "Account.Remove");
        ActionManager::instance()->registerAction(m_actShowClosed, "Account.ShowAll");

        m_actEdit->setEnabled(false);
        m_actClose->setEnabled(false);
        m_actReopen->setEnabled(false);
        m_actRemove->setEnabled(false);

    }
    else
    {
        m_accountMenu->menu()->setEnabled(true);
        connect(m_actCreate,  &QAction::triggered, this, &AccountPane::createAccount);
        connect(m_actEdit,    &QAction::triggered, this, &AccountPane::editAccount);
        connect(m_actClose,   &QAction::triggered, this, &AccountPane::closeAccount);
        connect(m_actReopen,  &QAction::triggered, this, &AccountPane::reopenAccount);
        connect(m_actRemove,  &QAction::triggered, this, &AccountPane::removeAccount);
        connect(m_actShowClosed, &QAction::triggered, this, &AccountPane::showClosedAccounts);
    }

    //Model
    m_model = new SimpleAccountController(this);
    m_treeAccounts = new AccountPaneTree(m_model, this);
    m_treeAccounts->setItemDelegate(new AccountPaneDelegate(this));

    m_treeAccounts->setIndentation(6);
    m_treeAccounts->expandAll();
    //m_treeAccounts->setColumnWidth(0, 120);
    //m_treeAccounts->header()->resizeSection(0, 200);
    //m_treeAccounts->header()->resizeSection(1, 35);
    //m_treeAccounts->header()->setStretchLastSection(false);
    //m_treeAccounts->header()->setSectionResizeMode(0, QHeaderView::Stretch);
    m_treeAccounts->setExpandsOnDoubleClick(false);
    m_treeAccounts->hideColumn(1);
    m_treeAccounts->header()->hide();

    QPalette p = palette();
    p.setColor(QPalette::Button, p.color(QPalette::Window));
    setPalette(p);

    QVBoxLayout* layout = new QVBoxLayout(this);
    layout->addWidget(m_treeAccounts);
    layout->setContentsMargins(4,0,0,0);

//    connect(m_treeAccounts, &QTreeView::activated, this, &AccountPane::onItemSelected);
    connect(m_treeAccounts, &QTreeView::clicked, this, &AccountPane::onItemSelected);

    connect(LedgerTabManager::instance(), &LedgerTabManager::currentAccountChanged,
            this, &AccountPane::onAccountTabChanged);

    onAccountTabChanged(nullptr);
}

AccountPane::~AccountPane()
{
    if (m_accountMenu)
    {
        m_accountMenu->menu()->setEnabled(false);
    }
}

void AccountPane::onItemSelected(const QModelIndex& _index)
{
    Account* a = m_model->accountAt(_index);

    if (a)
    {
        LedgerTabManager::instance()->openLedger(a);
    }
}

void AccountPane::createAccount()
{
    AccountActions::createAccount(m_currentAccount);
}

void AccountPane::editAccount()
{
    AccountActions::editAccount(m_currentAccount);
}

void AccountPane::closeAccount()
{
    AccountActions::closeAccount(m_currentAccount);
}

void AccountPane::reopenAccount()
{
    AccountActions::reopenAccount(m_currentAccount);
}

void AccountPane::removeAccount()
{
    AccountActions::removeAccount(m_currentAccount);
}

void AccountPane::showClosedAccounts(bool _show)
{
    auto state = m_treeAccounts->expandedState();
    m_model->setShowClosedAccounts(_show);
    m_treeAccounts->restoreExpandedState(state);
}

void AccountPane::onAccountTabChanged(Account* _account)
{
    if (_account)
    {
        m_actEdit->setEnabled(true);
        m_actClose->setEnabled(_account->canBeClosed());
        m_actReopen->setEnabled(!_account->isOpen());
        m_actRemove->setEnabled(_account->canBeRemoved());
    }
    else
    {
        m_actEdit->setEnabled(false);
        m_actClose->setEnabled(false);
        m_actReopen->setEnabled(false);
        m_actRemove->setEnabled(false);
    }

    m_currentAccount = _account;
}

//////////////////// Delegate /////////////////////////////

AccountPaneDelegate::AccountPaneDelegate(QObject* _parent) :
    QStyledItemDelegate(_parent)
{
}

QSize AccountPaneDelegate::sizeHint(const QStyleOptionViewItem& _option, const QModelIndex& _index) const
{
    QSize s = QStyledItemDelegate::sizeHint(_option, _index);

    if (_index.isValid())
    {
        SimpleAccountController::AccountNode* n = static_cast<SimpleAccountController::AccountNode*>(_index.internalPointer());
        const SimpleAccountController* controller = static_cast<const SimpleAccountController*>(_index.model());

        if (n && n->category != AccountClassification::INVALID)
        {
            s.setHeight(s.height() + 12);
        }
        else if (n && controller->indexIsAfterTypeChange(_index))
        {
            s.setHeight(s.height() + 7); //Need 1 for the line
        }
        else
        {
            s.setHeight(s.height() + 6);
        }
    }

    return s;
}

void AccountPaneDelegate::paint(QPainter* _painter, const QStyleOptionViewItem& _option, const QModelIndex& _index) const
{
    //First, compute how much size the amount takes.
    QModelIndex amountIdx = _index.model()->index(_index.row(), _index.column()+1, _index.parent());
    QFontMetrics fm(amountIdx.data(Qt::FontRole).value<QFont>());
    int width = fm.width(amountIdx.data(Qt::DisplayRole).toString()) + 8;

    //Then, paint the item on a reduced size
    QStyleOptionViewItem optionName = _option;
    optionName.rect.setWidth(optionName.rect.width() - (width));
    optionName.state = _option.state & (~QStyle::State_HasFocus);
    QStyledItemDelegate::paint(_painter, optionName, _index);

    //Finally, paint the amount.
    QStyleOptionViewItem optionAmount = _option;
    optionAmount.rect.setLeft(_option.rect.left() + optionName.rect.width());
    optionAmount.state = _option.state & (~QStyle::State_HasFocus);
    QStyledItemDelegate::paint(_painter, optionAmount, amountIdx);
}

//////////////////// Tree view ////////////////////////////

AccountPaneTree::AccountPaneTree(KLib::SimpleAccountController* _model, QWidget *parent) :
    QTreeView(parent),
    m_model(_model),
    m_rightClickedAccount(nullptr)
{
    setModel(_model);
    setMouseTracking(true);
    setRootIsDecorated(false);

    m_contextMenu_generic = new QMenu(tr("Accounts"), this);
    m_contextMenu_generic->addAction(ActionManager::instance()->command("Account.Add")->action());
    m_contextMenu_generic->addAction(ActionManager::instance()->command("Account.ShowAll")->action());

    m_contextMenu_account = new QMenu(tr("Account"), this);
    m_actCreate = m_contextMenu_account->addAction(Core::icon("create-bank-account"), tr("Add &Account"), this, SLOT(createAccount()));
    m_contextMenu_account->addSeparator();
    m_actEdit = m_contextMenu_account->addAction(Core::icon("edit"), tr("&Edit Account"), this, SLOT(editAccount()));
    m_actClose = m_contextMenu_account->addAction(Core::icon("close-bank-account"), tr("&Close Account"), this, SLOT(closeAccount()));
    m_actReopen = m_contextMenu_account->addAction(Core::icon("reopen-bank-account"), tr("Re-&open Account"), this, SLOT(reopenAccount()));
    m_actRemove = m_contextMenu_account->addAction(Core::icon("trash"), tr("&Delete Account"), this, SLOT(removeAccount()));

    connect(LedgerTabManager::instance(), &LedgerTabManager::currentAccountChanged,
            this, &AccountPaneTree::onAccountTabChanged);
}

QHash<int, bool> AccountPaneTree::expandedState() const
{
    QHash<int, bool> state;

    for (int i = (int) AccountClassification::CASH_FLOW; i < (int) AccountClassification::NumCategories; ++i)
    {
        QModelIndex idx = m_model->indexForCategory((AccountClassification) i);

        if (idx.isValid())
        {
            state[i] = isExpanded(idx);
        }
    }

    return state;
}

void AccountPaneTree::restoreExpandedState(const QHash<int, bool>& _state)
{
    for (int c : _state.keys())
    {
        QModelIndex idx = m_model->indexForCategory((AccountClassification) c);

        if (idx.isValid())
        {
            setExpanded(idx, _state[c]);
        }
    }
}

void AccountPaneTree::onAccountTabChanged(KLib::Account* _account)
{
    //indexForAccount will take care of the Constants::NO_ID case
    QModelIndex i = m_model->indexForAccount(_account);

    if (i.isValid())
    {
        setCurrentIndex(i);
    }
    else
    {
        clearSelection();
    }
}

void AccountPaneTree::createAccount()
{
    AccountActions::createAccount(m_rightClickedAccount);
}

void AccountPaneTree::editAccount()
{
    AccountActions::editAccount(m_rightClickedAccount);
}

void AccountPaneTree::closeAccount()
{
    AccountActions::closeAccount(m_rightClickedAccount);
}

void AccountPaneTree::reopenAccount()
{
    AccountActions::reopenAccount(m_rightClickedAccount);
}

void AccountPaneTree::removeAccount()
{
    AccountActions::removeAccount(m_rightClickedAccount);
}

void AccountPaneTree::drawBranches(QPainter*, const QRect&, const QModelIndex&) const
{

}

void AccountPaneTree::drawRow (QPainter* _painter, const QStyleOptionViewItem& _option, const QModelIndex& _index) const
{
    //Check if the index is a category
    if (_index.isValid())
    {
        SimpleAccountController::AccountNode* n = static_cast<SimpleAccountController::AccountNode*>(_index.internalPointer());

        if (n && n->category != AccountClassification::INVALID)
        {
            _painter->save();
            QRect boxRect(_option.rect.left(),_option.rect.top()+3,
                          _option.rect.width(), _option.rect.height()-6);

            _painter->fillRect(boxRect, CATEGORY_COLOR);
            _painter->setPen(Qt::gray);
            _painter->drawRect(boxRect);
            _painter->restore();

            QStyleOptionViewItem option2 = _option;
            option2.rect = boxRect;
            QTreeView::drawRow(_painter, option2, _index);
            return;
        }
        else if (n && n->account)
        {
            //Do not draw the item until the left edge
            QStyleOptionViewItem option2 = _option;
            QRect boxRect(_option.rect.left()+(indentation()*2),_option.rect.top()+1,
                          _option.rect.width()-(indentation()*2+4), _option.rect.height()-2);

            //Check we are changing type, in which case draw a line
            if (m_model->indexIsAfterTypeChange(_index))
            {
                _painter->save();
                _painter->setPen(Qt::lightGray);
                _painter->drawLine(_option.rect.left()+MARGIN_LINE+(indentation()*2), _option.rect.top(),
                                   _option.rect.right()-MARGIN_LINE, _option.rect.top());
                _painter->restore();

                boxRect.setTop(boxRect.top()+1);
            }

            option2.rect = boxRect;
            QTreeView::drawRow(_painter, option2, _index);
            return;
        }
    }

    QTreeView::drawRow(_painter, _option, _index);
}

void AccountPaneTree::mousePressEvent(QMouseEvent* _event)
{
    //If the mouse click is in a category, just close/open it. Otherwise,
    //use default behavior
    QModelIndex i = indexAt(_event->pos());

    if (i.isValid())
    {
        SimpleAccountController::AccountNode* n = static_cast<SimpleAccountController::AccountNode*>(i.internalPointer());

        if (_event->button() == Qt::LeftButton
            && n->category != AccountClassification::INVALID)
        {
            QModelIndex i2 = i.model()->index(i.row(), 0, i.parent());
            setExpanded(i2, !isExpanded(i2));
            _event->accept();
            return;
        }
        else if (_event->button() == Qt::RightButton && n->account)
        {
            //Show the account context menu with the right actions enabled
            m_actClose->setEnabled(n->account->canBeClosed());
            m_actReopen->setEnabled(!n->account->isOpen());
            m_actRemove->setEnabled(n->account->canBeRemoved());
            m_rightClickedAccount = n->account;
            m_contextMenu_account->popup(viewport()->mapToGlobal(_event->pos()));
            _event->accept();
            return;
        }
        else if (_event->button() == Qt::RightButton)
        {
            //Show the generic context menu
            m_contextMenu_generic->popup(viewport()->mapToGlobal(_event->pos()));
            _event->accept();
            return;
        }
    }
    else if (_event->button() == Qt::RightButton)
    {
        //Show the generic context menu
        m_contextMenu_generic->popup(viewport()->mapToGlobal(_event->pos()));
        _event->accept();
        return;
    }

    QTreeView::mousePressEvent(_event);
}

void AccountPaneTree::keyPressEvent(QKeyEvent* _event)
{
    //We want no key navigation, so eat all events :-) Yummy!
    _event->accept();
}

void AccountPaneTree::mouseMoveEvent(QMouseEvent* _event)
{
    QModelIndex index = indexAt(_event->pos());

    if (index.isValid())
    {
        setCursor(Qt::PointingHandCursor);
    }
    else if (cursor().shape() == Qt::PointingHandCursor)
    {
        unsetCursor();
    }

    QTreeView::mouseMoveEvent(_event);
}












