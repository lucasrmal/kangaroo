/*
This file is part of Kangaroo.
Copyright (C) 2014 Lucas Rioux-Maldague

Kangaroo is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Kangaroo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Kangaroo. If not, see <http://www.gnu.org/licenses/>.
*/

#include "tabinterfaceplugin.h"

#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/dialogs/formselectaccount.h>

#include <QMenu>

#include "allaccountstab.h"
#include "centralwidget.h"
#include "incomeexpensetab.h"
#include "ledgertab.h"
#include "tabinterface.h"

using namespace KLib;

TabInterfacePlugin::TabInterfacePlugin()
    : m_centralWidget(nullptr),
      m_allAccountsTab(nullptr),
      m_incomeExpenseTab(nullptr) {}

bool TabInterfacePlugin::initialize(QString& p_errorMessage) {
  Q_UNUSED(p_errorMessage)

  LedgerTabManager::instance();

  // CentralWidget
  m_centralWidget = CentralWidget::instance();
  Core::instance()->setCentralWidget(m_centralWidget);

  // Create menus
  ActionContainer* mnuView =
      ActionManager::instance()->actionContainer(STD_MENUS::MENU_VIEW);

  QAction* mnuDocks = ActionManager::instance()
                          ->actionContainer(STD_MENUS::MENU_VIEW_DOCKS)
                          ->menu()
                          ->menuAction();

  mnuShowAllAccounts =
      new QAction(Core::icon("bank"), tr("All Accounts"), mnuView->menu());
  mnuShowIncomeExpenses =
      new QAction(Core::icon("income-account"),
                  tr("Income and Expense Accounts"), mnuView->menu());
  mnuLookupAccount =
      new QAction(Core::icon("find"), tr("Lookup Accounts"), mnuView->menu());

  mnuView->menu()->insertAction(mnuDocks, mnuShowAllAccounts);
  mnuView->menu()->insertAction(mnuDocks, mnuShowIncomeExpenses);
  mnuView->menu()->insertAction(mnuDocks, mnuLookupAccount);

  mnuView->menu()->insertSeparator(mnuDocks);

  QAction* clt = mnuView->menu()->addAction(
      Core::icon("tab-close"), tr("Close current tab"),
      TabInterface::instance(), SLOT(closeCurrentTab()));

  QAction* clat = mnuView->menu()->addAction(
      Core::icon(""), tr("Close all tabs"), TabInterface::instance(),
      SLOT(closeAllCloseableTabs()));

  ActionManager::instance()->registerAction(clt, "View.CloseTab",
                                            QKeySequence::Close);
  ActionManager::instance()->registerAction(clat, "View.CloseAllTab",
                                            QKeySequence("Ctrl+Shift+W"));
  ActionManager::instance()->registerAction(
      mnuShowIncomeExpenses, "View.IncomeExpenseAccounts", QKeySequence("F10"));
  ActionManager::instance()->registerAction(
      mnuShowAllAccounts, "View.AllAccounts", QKeySequence("F9"));
  ActionManager::instance()->registerAction(
      mnuLookupAccount, "View.LookupAccount", QKeySequence("Ctrl+D"));

  mnuShowIncomeExpenses->setEnabled(false);
  mnuShowAllAccounts->setEnabled(false);
  mnuLookupAccount->setEnabled(false);

  connect(mnuShowAllAccounts, &QAction::triggered, this,
          &TabInterfacePlugin::showAllAccountsTab);
  connect(mnuShowIncomeExpenses, &QAction::triggered, this,
          &TabInterfacePlugin::showIncomeExpenseTab);
  connect(mnuLookupAccount, &QAction::triggered, this,
          &TabInterfacePlugin::lookupAccount);

  return true;
}

void TabInterfacePlugin::showAllAccountsTab() {
  if (!m_allAccountsTab) {
    m_allAccountsTab = new AllAccountsTab();
  }

  if (TabInterface::instance()->hasTab("allaccounts")) {
    TabInterface::instance()->setFocus("allaccounts");
  } else {
    TabInterface::instance()->addRegisteredTab(
        m_allAccountsTab, tr("All Accounts"), "allaccounts", true,
        Core::icon("bank"));
  }
}

void TabInterfacePlugin::showIncomeExpenseTab() {
  if (!m_incomeExpenseTab) {
    m_incomeExpenseTab = new IncomeExpenseTab();
  }

  if (TabInterface::instance()->hasTab("incomeexpense")) {
    TabInterface::instance()->setFocus("incomeexpense");
  } else {
    TabInterface::instance()->addRegisteredTab(
        m_incomeExpenseTab, tr("Income and Expense"), "incomeexpense", true,
        Core::icon("income-account"));
  }
}

void TabInterfacePlugin::lookupAccount() {
  int selected_account_id = Constants::NO_ID;
  if (!KLib::FormSelectAccount::selectAccount(
          TabInterface::instance(), &selected_account_id, tr("Lookup Account"),
          {.typeFlags = KLib::AccountTypeFlags::Flag_All &
                        ~KLib::AccountTypeFlags::Flag_Trading})) {
    return;
  }

  LedgerTabManager::instance()->openLedger(
      Account::getTopLevel()->account(selected_account_id));
}

void TabInterfacePlugin::checkSettings(QSettings& settings) const {
  Q_UNUSED(settings)
}

void TabInterfacePlugin::onLoad() {
  m_centralWidget->onLoad();
  LedgerTabManager::instance()->onLoad();
  mnuShowIncomeExpenses->setEnabled(true);
  mnuShowAllAccounts->setEnabled(true);
  mnuLookupAccount->setEnabled(true);
}

void TabInterfacePlugin::onUnload() {
  mnuShowIncomeExpenses->setEnabled(false);
  mnuShowAllAccounts->setEnabled(false);
  mnuLookupAccount->setEnabled(false);

  if (m_centralWidget) {
    m_centralWidget->onUnload();
  }

  LedgerTabManager::instance()->onUnload();

  if (m_incomeExpenseTab) {
    m_incomeExpenseTab->deleteLater();
    m_incomeExpenseTab = nullptr;
  }

  if (m_allAccountsTab) {
    m_allAccountsTab->deleteLater();
    m_allAccountsTab = nullptr;
  }
}

void TabInterfacePlugin::onShutdown() {
  if (m_centralWidget) {
    m_centralWidget->deleteLater();
  }
}

QString TabInterfacePlugin::name() const { return "TabInterface"; }

QString TabInterfacePlugin::version() const { return "1.2"; }

QString TabInterfacePlugin::description() const {
  return tr("Provides the central tabbing widget.");
}

QString TabInterfacePlugin::author() const { return Core::APP_AUTHOR; }

QString TabInterfacePlugin::copyright() const { return Core::APP_COPYRIGHT; }

QString TabInterfacePlugin::url() const { return Core::APP_WEBSITE; }

QStringList TabInterfacePlugin::requiredPlugins() const {
  return {"AccountCreateEdit", "CreateBook", "ImportExport"};
}
