#ifndef MORTGAGELOANTAB_H
#define MORTGAGELOANTAB_H

#include "../AccountCreateEdit/accountedittab.h"

namespace KLib
{
    class AccountSelector;
}

class MortgageLoanTab : public AccountEditTab
{
    Q_OBJECT

    public:
        MortgageLoanTab(QWidget* _parent = nullptr);

        QString tabName() const override { return tr("Mortgage Loan"); }
        QString tabIcon() const override { return "mortgage"; }

        bool enableIf(int _accountType) const override;
        void fillData(const KLib::Account* _account) override;

        void save(KLib::Account* _account) const override;

        QStringList validate() const override;

    private:
        KLib::AccountSelector* m_cboAccount;
};

#endif // MORTGAGELOANTAB_H
