#include "mortgageloantab.h"

#include <KangarooLib/model/account.h>
#include <KangarooLib/model/properties.h>
#include <KangarooLib/ui/widgets/accountselector.h>

#include <QFormLayout>

#include "realestatehelpers.h"

using namespace KLib;

MortgageLoanTab::MortgageLoanTab(QWidget* _parent) : AccountEditTab(_parent) {
  auto filterFn = [](const Account* a) {
    return a->type() == RealEstateHelpers::REAL_ESTATE_ACCOUNT_ID &&
           a->isOpen() && !a->isPlaceholder();
  };

  m_cboAccount = new AccountSelector({.customFilter = filterFn}, this);

  QFormLayout* layout = new QFormLayout(this);
  layout->addRow(tr("Linked Property:"), m_cboAccount);
}

bool MortgageLoanTab::enableIf(int _accountType) const {
  return _accountType == RealEstateHelpers::MORTGAGE_ACCOUNT_ID;
}

void MortgageLoanTab::fillData(const KLib::Account* _account) {
  if (_account->properties()->contains(RealEstateHelpers::LINKED_PROPERTY)) {
    m_cboAccount->setCurrentAccount(
        _account->properties()
            ->get(RealEstateHelpers::LINKED_PROPERTY, -1)
            .toInt());
  }
}

void MortgageLoanTab::save(KLib::Account* _account) const {
  if (_account->type() == RealEstateHelpers::MORTGAGE_ACCOUNT_ID) {
    if (m_cboAccount->currentAccount()) {
      _account->properties()->set(RealEstateHelpers::LINKED_PROPERTY,
                                  m_cboAccount->currentAccount()->id());
    } else {
      _account->properties()->remove(RealEstateHelpers::LINKED_PROPERTY);
    }
  }
}

QStringList MortgageLoanTab::validate() const { return {}; }
