#include "accountactions.h"

#include <KangarooLib/model/account.h>
#include <KangarooLib/model/ledger.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/transactionmanager.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/dialogs/formselectaccount.h>
#include <KangarooLib/ui/mainwindow.h>

#include <QMessageBox>

#include "../AccountCreateEdit/formeditaccount.h"

using namespace KLib;

void AccountActions::createAccount(Account* _parent) {
  FormEditAccount formCreate(nullptr, Core::instance()->mainWindow());
  formCreate.setParentAccount(_parent);
  formCreate.exec();
}

void AccountActions::editAccount(Account* _account) {
  if (_account) {
    FormEditAccount formEdit(_account, Core::instance()->mainWindow());
    formEdit.exec();
  }
}

bool AccountActions::closeAccount(Account* _account) {
  if (!(_account && _account->canBeClosed())) {
    return false;
  }

  QString msg = QObject::tr(
                    "Close this account: <b>%1</b>? <br><br>"
                    "Note that if schedules are linked with the account, they "
                    "will be deleted. "
                    "Reopening the account will not "
                    "recreate these schedules.")
                    .arg(_account->name());

  if (QMessageBox::question(Core::instance()->mainWindow(),
                            QObject::tr("Close Account"), msg,
                            QMessageBox::Yes | QMessageBox::No,
                            QMessageBox::No) == QMessageBox::Yes) {
    try {
      _account->setOpen(false);
      return true;
    } catch (ModelException e) {
      QMessageBox::warning(
          Core::instance()->mainWindow(), QObject::tr("Close Account"),
          QObject::tr("An error has occured while closing the account:\n\n%1")
              .arg(e.description()));
    }
  }

  return false;
}

bool AccountActions::reopenAccount(Account* _account) {
  if (!_account) return false;

  if (QMessageBox::question(
          Core::instance()->mainWindow(), QObject::tr("Reopen Account"),
          QObject::tr("Reopen account %1 ?").arg(_account->name()),
          QMessageBox::Yes | QMessageBox::No,
          QMessageBox::No) == QMessageBox::Yes) {
    try {
      _account->setOpen(true);
      return true;
    } catch (ModelException e) {
      QMessageBox::warning(
          Core::instance()->mainWindow(), QObject::tr("Reopen Account"),
          QObject::tr(
              "An error has occured while re-opening the account:\n\n%1")
              .arg(e.description()));
    }
  }

  return false;
}

bool AccountActions::removeAccount(Account* _account) {
  if (!(_account && _account->canBeRemoved())) {
    return false;
  }

  if (QMessageBox::question(
          Core::instance()->mainWindow(), QObject::tr("Confirm Delete"),
          QObject::tr("Delete this account: <b>%1</b>?").arg(_account->name()) +
              "<br><br>" + QObject::tr("<b>This action cannot be undone.</b>") +
              "<br><br>" +
              QObject::tr("Note that if schedules are linked with the account, "
                          "they will be deleted."),
          QMessageBox::Yes | QMessageBox::No,
          QMessageBox::No) == QMessageBox::Yes) {
    try {
      _account->parent()->removeChild(_account->id());
      return true;
    } catch (ModelException e) {
      QMessageBox::warning(
          Core::instance()->mainWindow(), QObject::tr("Remove Account"),
          QObject::tr("An error has occured while removing the account:\n\n%1")
              .arg(e.description()));
    }
  }

  return false;
}

bool AccountActions::reassignAllTransactions(KLib::Account* _account) {
  if (!_account) {
    return false;
  }

  int new_account_id = Constants::NO_ID;
  if (!FormSelectAccount::selectAccount(
          Core::instance()->mainWindow(), &new_account_id,
          QObject::tr("Reassign All Transactions To Account"),
          {.typeFlags = AccountTypeFlags::Flag_AllButInvTrad}) ||
      new_account_id == _account->id()) {
    return false;
  }

  std::vector<int> all_transactions;
  for (auto it = _account->ledger()->transactions()->begin();
       it != _account->ledger()->transactions()->end(); ++it) {
    all_transactions.push_back(it.value()->id());
  }

  QStringList errors;
  TransactionManager::instance()->reassignTransactions(
      all_transactions, _account->id(), new_account_id, &errors);
  if (!errors.empty()) {
    QMessageBox::warning(Core::instance()->mainWindow(),
                         QObject::tr("Reassign Transactions"),
                         QObject::tr("The following errors occured:\n\n %1")
                             .arg(errors.join("\n")));
  }
  return true;
}

bool AccountActions::moveAllChildren(KLib::Account* _account) {
  if (!_account) {
    return false;
  }

  int new_account_id = Constants::NO_ID;
  if (!FormSelectAccount::selectAccount(
          Core::instance()->mainWindow(), &new_account_id,
          QObject::tr("Move All Children To Account"),
          {.typeFlags =
               AccountTypeFlags::Flag_All & ~AccountTypeFlags::Flag_Trading}) ||
      new_account_id == _account->id()) {
    return false;
  }

  Account* new_parent = Account::getAccount(new_account_id);

  std::vector<Account*> children = _account->getChildren();
  QStringList errors;
  for (Account* child : children) {
    try {
      child->moveToParent(new_parent);
    } catch (const ModelException& e) {
      errors << QObject::tr("Unable to move %1: %2")
                    .arg(child->name(), e.description());
    }
  }
  if (!errors.empty()) {
    QMessageBox::warning(Core::instance()->mainWindow(),
                         QObject::tr("Move All Children To Account"),
                         QObject::tr("The following errors occured:\n\n %1")
                             .arg(errors.join("\n")));
  }
  return true;
}
