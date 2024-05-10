#ifndef FORMSELECTACCOUNT_H
#define FORMSELECTACCOUNT_H

#include <QWidget>

#include "../../klib.h"
#include "../widgets/accountselector.h"
#include "camsegdialog.h"

namespace KLib {

class FormSelectAccount : public CAMSEGDialog {
  Q_OBJECT

 public:
  /**
    Creates a new account selection dialog.
  */
  explicit FormSelectAccount(AccountSelectorParams selectorParams,
                             QWidget* parent = nullptr);

  /**
    Convenience function that acts as a wrapper over the class. It's the
    recommended way of selecting an account.

    @return True if the user accepted the dialog, or false if he rejected it.
  */
  static bool selectAccount(QWidget* parent, int* selected_account_id,
                            const QString& title,
                            AccountSelectorParams selectorParams,
                            int initial_account_id = Constants::NO_ID);

 public slots:
  void accept() override;

 private:
  void loadUI();

  AccountSelector* account_selector_;
};

}  // namespace KLib

#endif  // FORMSELECTACCOUNT_H
