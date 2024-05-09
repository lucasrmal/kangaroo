#ifndef INVESTMENTSETTINGS_H
#define INVESTMENTSETTINGS_H

#include <QString>
#include <QStringList>
#include <QWidget>

#include "KangarooLib/model/account.h"
#include "KangarooLib/ui/widgets/accountselector.h"
#include "accountedittab.h"

class InvestmentSettings : public AccountEditTab {
  Q_OBJECT
 public:
  InvestmentSettings(QWidget* _parent = nullptr);

  QString tabName() const override { return tr("Investment Settings"); }
  QString tabIcon() const override { return "investment-account"; }
  bool enableIf(int _accountType) const override {
    return _accountType == KLib::AccountType::INVESTMENT ||
           _accountType == KLib::AccountType::BROKERAGE;
  }

  void fillData(const KLib::Account* _account) override;
  void save(KLib::Account* _account) const override;
  QStringList validate() const override { return {}; }

 private:
  KLib::AccountSelector* m_defaultDividendAccountSelector;
};

#endif  // INVESTMENTSETTINGS_H
