#include "formselectaccount.h"

#include <QMessageBox>
#include <QVBoxLayout>

#include "ui/core.h"

namespace KLib {

FormSelectAccount::FormSelectAccount(int account_selection_flags,
                                     int account_type_flags, QWidget* parent)
    : CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent),
      account_selection_flags_(account_selection_flags),
      account_type_flags_(account_type_flags) {
  loadUI();
}

bool FormSelectAccount::selectAccount(QWidget* parent, int* selected_account_id,
                                      int account_selection_flags,
                                      int account_type_flags,
                                      int initial_account_id) {
  bool returnValue = false;
  FormSelectAccount dialog(account_selection_flags, account_type_flags, parent);

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

  account_selector_ = new AccountSelector(
      account_selection_flags_, account_type_flags_, Constants::NO_ID, this);

  QVBoxLayout* mainLayout = new QVBoxLayout();
  mainLayout->addWidget(account_selector_);

  centralWidget()->setLayout(mainLayout);
  account_selector_->setFocus();
}

}  // namespace KLib
