#include "investmentsettings.h"

#include <QFormLayout>

InvestmentSettings::InvestmentSettings(QWidget* _parent)
    : AccountEditTab(_parent) {
  m_defaultDividendAccountSelector =
      new KLib::AccountSelector(KLib::AccountTypeFlags::Flag_Income, this);

  QFormLayout* layout = new QFormLayout(this);
  layout->addRow(tr("Default &Dividend Account:"),
                 m_defaultDividendAccountSelector);
}

void InvestmentSettings::fillData(const KLib::Account* _account) {
  if (!enableIf(_account->type()) ||
      _account->idDefaultDividendAccount() == KLib::Constants::NO_ID) {
    return;
  }
  m_defaultDividendAccountSelector->setCurrentAccount(
      _account->idDefaultDividendAccount());
}

void InvestmentSettings::save(KLib::Account* _account) const {
  if (!enableIf(_account->type()) ||
      !m_defaultDividendAccountSelector->currentAccount()) {
    return;
  }

  _account->setIdDefaultDividendAccount(
      m_defaultDividendAccountSelector->currentAccount()->id());
}
