#ifndef FORMSELECTACCOUNT_H
#define FORMSELECTACCOUNT_H

#include <QWidget>

#include "klib.h"
#include "ui/dialogs/camsegdialog.h"
#include "ui/widgets/accountselector.h"

namespace KLib {

class FormSelectAccount : public CAMSEGDialog {
  Q_OBJECT

 public:
  /**
    Creates a new account selection dialog.
  */
  explicit FormSelectAccount(int account_selection_flags,
                             int account_type_flags, QWidget* parent = NULL);

  /**
    Convenience function that acts as a wrapper over the class. It's the
    recommended way of selecting an account.

    @return True if the user accepted the dialog, or false if he rejected it.
  */
  static bool selectAccount(QWidget* parent, int* selected_account_id,
                            int account_selection_flags = Flag_None,
                            int account_type_flags = AccountTypeFlags::Flag_All,
                            int initial_account_id = Constants::NO_ID);

 public slots:
  void accept() override;

 private:
  void loadUI();

  const int account_selection_flags_;
  const int account_type_flags_;
  AccountSelector* account_selector_;
};

}  // namespace KLib

#endif  // FORMSELECTACCOUNT_H
