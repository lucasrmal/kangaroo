#include "accountviewtab.h"

#include <KangarooLib/controller/accountcontroller.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/ui/core.h>

#include <QCheckBox>
#include <QLabel>
#include <QLineEdit>
#include <QMenu>
#include <QMessageBox>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>

#include "accountactions.h"
#include "accounttreewidget.h"
#include "ledgertab.h"

using namespace KLib;

AccountViewTab::AccountViewTab(KLib::Account* _topLevel, bool _showOpenOnly,
                               int _flags, QWidget* parent)
    : QWidget(parent),
      m_toolbar(nullptr),
      m_findBar(nullptr),
      m_currentAccount(nullptr) {
  m_accountTree = new AccountTreeWidget(_topLevel, _showOpenOnly, _flags, this);
  connect(m_accountTree, SIGNAL(doubleClicked(QModelIndex)), this,
          SLOT(openLedger()));
  connect(m_accountTree, &AccountTreeWidget::currentAccountChanged, this,
          &AccountViewTab::onCurrentAccountChanged);

  m_layout = new QVBoxLayout(this);
  m_layout->setContentsMargins(0, 0, 0, 0);
  m_layout->addWidget(m_accountTree);

  // Create menus
  m_accountMenu = new QMenu(tr("Account"), this);

  m_actOpen = m_accountMenu->addAction(
      Core::icon("open-ledger"), tr("Open &Ledger"), this, SLOT(openLedger()));
  m_accountMenu->addSeparator();
  m_actCreate =
      m_accountMenu->addAction(Core::icon("create-bank-account"), tr("&New"),
                               this, SLOT(createAccount()));
  m_actEdit = m_accountMenu->addAction(Core::icon("edit"), tr("&Edit"), this,
                                       SLOT(editAccount()));
  m_accountMenu->addSeparator();
  m_actReassign =
      m_accountMenu->addAction(Core::icon("reassign"), tr("&Reassign"), this,
                               SLOT(reassignAllTransactions()));
  m_actCloseOpen =
      m_accountMenu->addAction(Core::icon("close-bank-account"), tr("&Close"),
                               this, SLOT(closeOpenAccount()));

  m_actDelete = m_accountMenu->addAction(Core::icon("trash"), tr("&Delete"),
                                         this, SLOT(deleteAccount()));

  m_accountMenu->addSeparator();
  m_actShowAll = m_accountMenu->addAction(tr("&Show All Accounts"));
  m_actShowAll->setCheckable(true);
  m_actShowAll->setChecked(false);
  connect(m_actShowAll, &QAction::triggered, this,
          &AccountViewTab::showAllAccounts);

  m_actFind = new QAction(Core::icon("find"), tr("Search"), this);
  connect(m_actFind, &QAction::triggered, this, &AccountViewTab::findAccounts);

  m_actFind->setShortcut(QKeySequence::Find);

  onCurrentAccountChanged(nullptr);

  m_accountTree->setContextMenu(m_accountMenu);
}

void AccountViewTab::onCurrentAccountChanged(KLib::Account* a) {
  if (a) {
    m_actCloseOpen->setText(a->isOpen() ? tr("&Close") : tr("Re-&open"));
    m_actCloseOpen->setIcon(a->isOpen() ? Core::icon("close-bank-account")
                                        : Core::icon("reopen-bank-account"));

    m_actOpen->setEnabled(!a->isPlaceholder());
    m_actEdit->setEnabled(true);
    m_actReassign->setEnabled(a->type() == AccountType::INCOME ||
                              a->type() == AccountType::EXPENSE);
    m_actCloseOpen->setEnabled(!a->isOpen() || a->canBeClosed());
    m_actDelete->setEnabled(a->childCount() == 0 &&
                            a->type() != AccountType::TOPLEVEL &&
                            (a->isPlaceholder() || a->ledger()->count() == 0));
  } else {
    m_actOpen->setEnabled(false);
    m_actEdit->setEnabled(false);
    m_actReassign->setEnabled(false);
    m_actCloseOpen->setEnabled(false);
    m_actDelete->setEnabled(false);
  }

  m_currentAccount = a;
}

QAction* AccountViewTab::action(ActionType _type) const {
  switch (_type) {
    case ActionType::Create:
      return m_actCreate;

    case ActionType::Open:
      return m_actOpen;

    case ActionType::Edit:
      return m_actEdit;

    case ActionType::ReassignAllTransactions:
      return m_actReassign;

    case ActionType::CloseReopen:
      return m_actCloseOpen;

    case ActionType::Delete:
      return m_actDelete;

    case ActionType::ShowAll:
      return m_actShowAll;

    case ActionType::Find:
      return m_actFind;

    default:
      return nullptr;
  }
}

void AccountViewTab::setToolBar(QToolBar* _toolbar) {
  if (m_toolbar) {
    m_layout->removeWidget(m_toolbar);
    m_toolbar->deleteLater();
  }

  m_toolbar = _toolbar;
  m_layout->addWidget(m_toolbar);
}

void AccountViewTab::resetAccountTree() { m_accountTree->reset(); }

void AccountViewTab::showAllAccounts(bool _show) {
  m_accountTree->controller()->setShowOpenOnly(!_show);
  m_accountTree->expandAll();
  onCurrentAccountChanged(nullptr);
}

void AccountViewTab::filterAccounts(const QString& _filter) {
  m_accountTree->filterController()->setFilterFixedString(_filter);
  m_accountTree->expandAll();
  onCurrentAccountChanged(m_accountTree->currentAccount());
  m_accountTree->scrollTo(m_accountTree->currentIndex());
}

void AccountViewTab::setIncludeChildrenInFilter(bool include) {
  m_accountTree->filterController()->setIncludeChildrenInFilter(include);
  m_accountTree->expandAll();
  onCurrentAccountChanged(m_accountTree->currentAccount());
  m_accountTree->scrollTo(m_accountTree->currentIndex());
}

void AccountViewTab::findAccounts() {
  if (!m_findBar) {
    m_findBar = new QWidget(this);

    QHBoxLayout* layout = new QHBoxLayout(m_findBar);
    layout->setContentsMargins(0, 0, 0, 0);

    QLineEdit* txtSearch = new QLineEdit(m_findBar);
    QToolButton* btnClose = new QToolButton(m_findBar);
    QCheckBox* chkShowChildren =
        new QCheckBox(tr("Include children"), m_findBar);

    btnClose->setAutoRaise(true);
    chkShowChildren->setChecked(true);

    QAction* actClose = new QAction(Core::icon("close"), "", this);
    actClose->setShortcut(QKeySequence("Esc"));
    btnClose->setDefaultAction(actClose);

    layout->addWidget(btnClose);
    layout->addWidget(txtSearch);
    layout->addWidget(chkShowChildren);

    txtSearch->setFocus();

    connect(actClose, &QAction::triggered, this,
            &AccountViewTab::doneFindAccounts);
    connect(txtSearch, &QLineEdit::textEdited, this,
            &AccountViewTab::filterAccounts);
    connect(chkShowChildren, &QCheckBox::toggled, this,
            &AccountViewTab::setIncludeChildrenInFilter);

    if (!m_toolbar) {
      m_layout->addWidget(m_findBar);
    } else {
      m_layout->insertWidget(m_layout->count() - 1, m_findBar);
    }
  }
}

void AccountViewTab::doneFindAccounts() {
  if (m_findBar) {
    m_layout->removeWidget(m_findBar);
    m_findBar->deleteLater();
    m_findBar = nullptr;
    filterAccounts("");
  }
}

void AccountViewTab::createAccount() {
  AccountActions::createAccount(m_currentAccount);
}

void AccountViewTab::editAccount() {
  AccountActions::editAccount(m_currentAccount);
}

void AccountViewTab::reassignAllTransactions() {
  AccountActions::reassignAllTransactions(m_currentAccount);
}

void AccountViewTab::closeOpenAccount() {
  if (m_currentAccount && !m_currentAccount->isOpen())  // Want to re-open
  {
    if (AccountActions::reopenAccount(m_currentAccount))
      onCurrentAccountChanged(m_currentAccount);
  } else if (m_currentAccount)  // Want to close
  {
    if (AccountActions::closeAccount(m_currentAccount))
      onCurrentAccountChanged(m_currentAccount);
  }
}

void AccountViewTab::deleteAccount() {
  if (AccountActions::removeAccount(m_currentAccount))
    onCurrentAccountChanged(nullptr);
}

void AccountViewTab::openLedger() {
  if (m_currentAccount && !m_currentAccount->isPlaceholder()) {
    LedgerTabManager::instance()->openLedger(m_currentAccount);
  }
}
