#include "allaccountstab.h"

#include <KangarooLib/model/account.h>

#include <QToolBar>

using namespace KLib;

AllAccountsTab::AllAccountsTab(QWidget* _parent)
    : AccountViewTab(Account::getTopLevel(), true, AccountTypeFlags::Flag_All,
                     _parent) {
  // Create the toolbar
  QToolBar* bottomToolbar = new QToolBar(this);

  QWidget* sep1 = new QWidget();
  sep1->setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Preferred);

  bottomToolbar->setToolButtonStyle(Qt::ToolButtonTextUnderIcon);
  bottomToolbar->setIconSize(QSize(24, 24));

  bottomToolbar->addAction(action(ActionType::Create));
  bottomToolbar->addAction(action(ActionType::Edit));
  bottomToolbar->addAction(action(ActionType::ReassignAllTransactions));
  bottomToolbar->addAction(action(ActionType::CloseReopen));
  bottomToolbar->addAction(action(ActionType::Delete));
  bottomToolbar->addAction(action(ActionType::Open));
  bottomToolbar->addWidget(sep1);
  bottomToolbar->addAction(action(ActionType::Find));
  setToolBar(bottomToolbar);
}
