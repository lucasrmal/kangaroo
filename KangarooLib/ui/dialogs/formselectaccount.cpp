#include "formselectaccount.h"

#include <QMessageBox>
#include <QVBoxLayout>

#include "ui/core.h"

namespace KLib {

FormSelectAccount::FormSelectAccount(AccountSelectorParams selectorParams,
                                     QWidget* parent)
    : CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
      account_selector_(new AccountSelector(std::move(selectorParams), this)) {
  loadUI();
}

bool FormSelectAccount::selectAccount(QWidget* parent, int* selected_account_id,
                                      const QString& title,
                                      AccountSelectorParams selectorParams,
                                      int initial_account_id) {
  bool returnValue = false;
  FormSelectAccount dialog(std::move(selectorParams), parent);
  dialog.setBothTitles(title);

  if (initial_account_id != Constants::NO_ID) {
    dialog.account_selector_->setCurrentAccount(initial_account_id);
  }

  if (dialog.exec() == QDialog::Accepted &&
      dialog.account_selector_->currentAccount()) {
    returnValue = true;
    *selected_account_id = dialog.account_selector_->currentAccount()->id();
  }

  return returnValue;
}

void FormSelectAccount::accept() {
  // End date > start date
  if (!account_selector_->currentAccount()) {
    QMessageBox::information(this, this->windowTitle(),
                             tr("An account must be selected."));
    account_selector_->setFocus();
    return;
  }

  // Close the dialog
  done(QDialog::Accepted);
}

void FormSelectAccount::loadUI() {
  setBothTitles(tr("Select Account"));
  setPicture(Core::pixmap("bank-account"));

  QVBoxLayout* mainLayout = new QVBoxLayout();
  mainLayout->addWidget(account_selector_);

  centralWidget()->setLayout(mainLayout);
  account_selector_->setFocus();
}

}  // namespace KLib
