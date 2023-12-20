#include "ledgertab.h"

#include <KangarooLib/model/account.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/picturemanager.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/dialogs/formpicturemanager.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/widgets/ledgerwidget.h>

#include <QAction>
#include <QDebug>
#include <QHBoxLayout>
#include <QKeySequence>
#include <QLabel>
#include <QMenu>
#include <QMessageBox>
#include <QPushButton>
#include <QStatusBar>
#include <QToolBar>
#include <QToolButton>
#include <QVBoxLayout>
#include <QWidgetAction>

#include "../AccountCreateEdit/formeditaccount.h"
#include "tabinterface.h"

#define TABNAME(acc) QString("Account_%1").arg(acc->id())

using namespace KLib;

LedgerTabManager* LedgerTabManager::m_instance = nullptr;

LedgerTab::LedgerTab(Account* _account, QWidget* parent)
    : QWidget(parent), m_account(_account) {
  if (!_account->ledger()) {
    return;
  }

  LedgerTabManager::instance()->registerLedger(this);

  m_ledgerWidget = new LedgerWidget(_account, this);

  QVBoxLayout* layout = new QVBoxLayout(this);
  QHBoxLayout* layoutTop = new QHBoxLayout();
  QVBoxLayout* layoutTopL = new QVBoxLayout();
  QVBoxLayout* layoutTopR = new QVBoxLayout();

  QVBoxLayout* layoutBottom = new QVBoxLayout();

  layout->setContentsMargins(0, 0, 0, 0);
  layoutTop->setContentsMargins(6, 6, 6, 0);

  m_lblPic = new QPushButton(this);
  m_lblAccountName = new QLabel(this);
  m_lblAccountType = new QLabel(this);
  m_lblAccountBalance = new QLabel(this);
  m_lblAccountValue = new QLabel(this);
  // m_btnEditAccount = new QPushButton(Core::icon("edit-bank-account"),
  // tr("Edit"), this);

  m_lblPic->setFixedSize(PictureManager::THUMBNAIL_SIZE);

  QWidget* sep1 = new QWidget();
  sep1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);
  QWidget* sep2 = new QWidget();
  sep2->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  QToolBar* bottomToolbar = new QToolBar(this);
  bottomToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  bottomToolbar->setIconSize(QSize(24, 24));

  // Add the basic actions
  QMenu* mnuNew = new QMenu(this);
  mnuNew->addAction(
      m_ledgerWidget->basicAction(BasicLedgerAction::NewTransaction)->action);
  mnuNew->addAction(
      m_ledgerWidget->basicAction(BasicLedgerAction::NewSchedule)->action);

  QToolButton* btnNew = new QToolButton();
  btnNew->setMenu(mnuNew);
  btnNew->setPopupMode(QToolButton::InstantPopup);
  btnNew->setText(tr("New"));
  btnNew->setIcon(Core::icon("new-row"));
  btnNew->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);

  QWidgetAction* actionBtnNew = new QWidgetAction(this);
  actionBtnNew->setDefaultWidget(btnNew);

  bottomToolbar->addAction(actionBtnNew);
  bottomToolbar->addAction(
      m_ledgerWidget->basicAction(BasicLedgerAction::Edit)->action);
  bottomToolbar->addAction(
      m_ledgerWidget->basicAction(BasicLedgerAction::Delete)->action);
  bottomToolbar->addAction(
      m_ledgerWidget->basicAction(BasicLedgerAction::Reassign)->action);
  bottomToolbar->addAction(
      m_ledgerWidget->basicAction(BasicLedgerAction::Duplicate)->action);

  bottomToolbar->addWidget(sep1);

  // Add the extra actions
  for (LedgerAction* a : m_ledgerWidget->extraActions()) {
    if (a->isPrimary) {
      bottomToolbar->addAction(a->action);
    }
  }

  bottomToolbar->addWidget(sep2);
  bottomToolbar->addAction(LedgerTabManager::instance()->action_find());

  layoutBottom->addWidget(bottomToolbar);

  QFont f;
  f.setPointSize(11);
  f.setBold(true);
  m_lblAccountName->setFont(f);

  f.setPointSize(10);
  m_lblAccountType->setFont(f);

  f.setPointSize(10);
  m_lblAccountBalance->setFont(f);
  m_lblAccountValue->setFont(f);

  m_lblAccountBalance->setAlignment(Qt::AlignRight | Qt::AlignHCenter);
  // m_btnEditAccount->setSizePolicy(QSizePolicy::Minimum,
  // QSizePolicy::Preferred);

  // m_btnEditAccount->setFlat(true);
  m_lblPic->setFlat(true);
  m_lblPic->setIconSize(PictureManager::THUMBNAIL_SIZE);

  m_lblAccountBalance->setAlignment(Qt::AlignRight | Qt::AlignVCenter);
  m_lblAccountValue->setAlignment(Qt::AlignRight | Qt::AlignVCenter);

  layoutTopL->addWidget(m_lblAccountName);
  layoutTopL->addWidget(m_lblAccountType);
  layoutTopR->addWidget(m_lblAccountBalance);
  layoutTopR->addWidget(m_lblAccountValue);
  // layoutTopR->addWidget(m_btnEditAccount);

  layoutTop->addWidget(m_lblPic);
  layoutTop->addLayout(layoutTopL);
  layoutTop->addStretch(2);
  layoutTop->addLayout(layoutTopR);

  layout->addLayout(layoutTop);
  layout->addWidget(m_ledgerWidget);
  layout->addLayout(layoutBottom);
  setLayout(layout);

  updateAccountInfos();
  updateAccountBalance();

  connect(m_ledgerWidget, &LedgerWidget::showMessage, this,
          &LedgerTab::onMessageReceived);
  connect(m_ledgerWidget, &LedgerWidget::selectedRowsTotalChanged, this,
          &LedgerTab::updateBalance);

  connect(m_account, SIGNAL(accountModified(KLib::Account*)), this,
          SLOT(updateAccountInfos()));

  if (m_account->type() == AccountType::INVESTMENT) {
    // We cannot connect directly to the security in case it changes...
    connect(SecurityManager::instance(), &SecurityManager::modified, this,
            &LedgerTab::updateAccountInfos);

    // Update the value
    connect(PriceManager::instance(), &PriceManager::lastRateModified, this,
            &LedgerTab::updateAccountBalance);
  } else if (m_account->type() == AccountType::TRADING &&
             m_account->idSecurity() != Constants::NO_ID) {
    // Security will never change, so we're safe.
    connect(SecurityManager::instance()->get(m_account->idSecurity()),
            &Security::modified, this, &LedgerTab::updateAccountInfos);
  } else if (m_account->type() == AccountType::CREDITCARD) {
    // In case the credit limit is modified...
    connect(m_account, SIGNAL(accountModified(KLib::Account*)), this,
            SLOT(updateAccountBalance()));
  }

  // connect(m_btnEditAccount, &QPushButton::clicked, this,
  // &LedgerTab::editAccount);
  connect(m_lblPic, &QPushButton::clicked, this,
          &LedgerTab::selectAccountPicture);
}

void LedgerTab::updateAccountInfos() {
  m_lblAccountName->setText(
      Account::getTopLevel()->getPath(m_account->id(), 2));
  m_lblAccountType->setText(Account::typeToString(m_account->type()));
  m_lblPic->setIcon(QIcon(m_account->defaultPicture(true)));
}

void LedgerTab::updateAccountBalance() {
  Amount balance = m_account->balanceToday();

  if (m_account->type() == AccountType::INVESTMENT) {
    Security* sec = SecurityManager::instance()->get(m_account->idSecurity());
    Currency* cur = CurrencyManager::instance()->get(sec->currency());

    m_lblAccountBalance->setText(
        tr("Balance: %1 shares").arg(QString::number(balance.toDouble())));
    m_lblAccountValue->setText(
        tr("Value: %1")
            .arg(cur->formatAmount(
                balance * PriceManager::instance()->rate(
                              m_account->idSecurity(), sec->currency()))));
  } else {
    m_lblAccountBalance->setText(
        tr("Balance: %1").arg(m_account->formatAmount(balance)));
  }

  if (m_account->type() == AccountType::CREDITCARD &&
      m_account->properties()->contains("creditlimit")) {
    Amount limit =
        Amount::fromQVariant(m_account->properties()->get("creditlimit"));
    m_lblAccountValue->setText(
        tr("Available Credit: %1")
            .arg(m_account->formatAmount(limit - balance)));
  }
}

void LedgerTab::editAccount() {
  FormEditAccount* formEdit = new FormEditAccount(m_account, this);
  formEdit->exec();
  delete formEdit;
}

void LedgerTab::selectAccountPicture() {
  FormPictureManager picMan(m_account->idPicture(),
                            FormPictureManager::AllowNoPicture, this);

  if (picMan.exec() == QDialog::Accepted) {
    m_account->setIdPicture(picMan.currentPictureId());
  }
}

void LedgerTab::updateBalance(const Amount& _amount) {
  if (!m_ledgerWidget->inEdit()) {
    emit showMessage(
        tr("Selected total: %1").arg(m_account->formatAmount(_amount)));
  }
}

void LedgerTab::onMessageReceived(const QString& _message) {
  // We want to reemit the message ourselves so that LedgerTabManager can
  // identify the sender.
  emit showMessage(_message);
}

LedgerTabManager* LedgerTabManager::instance() {
  if (!m_instance) m_instance = new LedgerTabManager();

  return m_instance;
}

LedgerTabManager::LedgerTabManager() : m_currentTab(nullptr) {
  //    m_icnEnter = Core::icon("enter-modifications");
  //    m_icnEnterSchedule = Core::icon("enter-schedule");

  // Status bar label
  m_lblStatusMessage = new QLabel();
  Core::instance()->mainWindow()->statusBar()->addPermanentWidget(
      m_lblStatusMessage);

  // Create menus
  m_transacMenu = ActionManager::instance()->insertMenu(
      STD_MENUS::MENUBAR, STD_MENUS::MENU_MANAGE, "Transaction",
      tr("&Transaction"));
  m_actFind = m_transacMenu->menu()->addAction(
      Core::icon("find"), tr("Searc&h"), this, SLOT(findTransactions()));

  ActionManager::instance()->registerAction(m_actFind, "Transaction.Search",
                                            QKeySequence("Ctrl+F"));

  m_transacMenu->menu()->setEnabled(false);
  m_actFind->setEnabled(false);

  // Connections to Account changes
  connect(Account::getTopLevel(), &Account::accountModified, this,
          &LedgerTabManager::onAccountModified);

  connect(Account::getTopLevel(), &Account::accountRemoved, this,
          &LedgerTabManager::onAccountRemoved);

  connect(LedgerManager::instance(), &LedgerManager::balanceTodayChanged, this,
          &LedgerTabManager::onBalanceTodayChanged);
}

void LedgerTabManager::setStatusBarMessage(const QString& _message) {
  LedgerTab* t = qobject_cast<LedgerTab*>(sender());

  if (t && t == m_currentTab) {
    m_lblStatusMessage->setText(_message);
  }
}

void LedgerTabManager::onLoad() {
  m_transacMenu->menu()->setEnabled(true);
  m_lblStatusMessage->clear();

  onCurrentTabChanged();
  connect(TabInterface::instance(), SIGNAL(currentChanged(int)), this,
          SLOT(onCurrentTabChanged()));
}

void LedgerTabManager::onUnload() {
  m_transacMenu->menu()->setEnabled(false);
  m_actFind->setEnabled(false);

  for (LedgerTab* t : m_tabs) {
    t->deleteLater();
  }

  m_tabs.clear();
  m_tabIndex.clear();
  m_currentTab = nullptr;
  m_lblStatusMessage->clear();
}

void LedgerTabManager::onCurrentTabChanged() {
  LedgerTab* tab =
      static_cast<LedgerTab*>(TabInterface::instance()->currentWidget());

  m_lblStatusMessage->clear();

  if (m_tabs.contains(tab)) {
    m_currentTab = tab;
    m_actFind->setEnabled(true);
    emit currentAccountChanged(m_currentTab->account());
  } else {
    if (m_currentTab)  // Only emit the changed signal if we go from an account
                       // to no account
    {
      m_currentTab = nullptr;
      emit currentAccountChanged(nullptr);
    }
    m_actFind->setEnabled(false);
  }
}

void LedgerTabManager::onAccountModified(KLib::Account* _a) {
  if (m_tabIndex.contains(_a->id())) {
    LedgerTab* tab = m_tabIndex[_a->id()];

    if (_a->isPlaceholder()) {
      removeTab(m_tabIndex[_a->id()]);
    } else if (!_a->isOpen()) {
      if (tab->m_ledgerWidget->inEdit()) {
        tab->m_ledgerWidget->discardEdit();
      }
    }

    // Update the name
    int tabIndex = TabInterface::instance()->indexOf(tab);
    if (tabIndex != -1) {
      TabInterface::instance()->setTabText(tabIndex, _a->name());
      TabInterface::instance()->setTabIcon(tabIndex,
                                           QIcon(_a->defaultPicture(true)));
    }
  }
}

void LedgerTabManager::onBalanceTodayChanged(int _idAccount,
                                             const Balances& _balances) {
  Q_UNUSED(_balances)

  if (m_tabIndex.contains(_idAccount)) {
    m_tabIndex[_idAccount]->updateAccountBalance();
  }
}

void LedgerTabManager::onAccountRemoved(Account* _a) {
  if (m_tabIndex.contains(_a->id())) {
    removeTab(m_tabIndex[_a->id()]);
  }
}

bool LedgerTabManager::askToContinue() {
  int ans = QMessageBox::question(
      m_currentTab, tr("Save Changes"),
      tr("The current transaction has unsaved changes. Save it?"),
      QMessageBox::Yes | QMessageBox::No | QMessageBox::Cancel,
      QMessageBox::Yes);

  switch (ans) {
    case QMessageBox::Yes:
      m_currentTab->m_ledgerWidget->submitEdit();
    case QMessageBox::No:
      return true;
    default:
      return false;
  }
}

void LedgerTabManager::removeTab(QWidget* _tab) {
  LedgerTab* tab = static_cast<LedgerTab*>(_tab);

  if (tab && m_tabs.contains(tab)) {
    closeLedger(tab->m_account);
    m_tabs.remove(tab);
    m_tabIndex.remove(tab->m_account->id());
    tab->deleteLater();
  }
}

void LedgerTabManager::findTransactions() {}

void LedgerTabManager::openLedger(Account* _a) {
  if (_a->isPlaceholder()) return;

  if (TabInterface::instance()->hasTab(TABNAME(_a))) {
    TabInterface::instance()->setFocus(TABNAME(_a));
  } else {
    QWidget* w = m_tabIndex.contains(_a->id()) ? m_tabIndex[_a->id()]
                                               : new LedgerTab(_a);

    TabInterface::instance()->addRegisteredTab(w, _a->name(), TABNAME(_a), true,
                                               _a->defaultPicture(true));
    TabInterface::instance()->setFocus(TABNAME(_a));
  }
}

void LedgerTabManager::closeLedger(Account* _a) {
  // Simply remove the tab visually; if the user requests it later it will be
  // already in memory and we'll show it.
  if (TabInterface::instance()->hasTab(TABNAME(_a))) {
    TabInterface::instance()->closeTab(TABNAME(_a));
  }
}

void LedgerTabManager::registerLedger(LedgerTab* _tab) {
  m_tabIndex[_tab->m_account->id()] = _tab;
  m_tabs.insert(_tab);

  connect(_tab, &LedgerTab::showMessage, this,
          &LedgerTabManager::setStatusBarMessage);
}
