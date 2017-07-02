#ifndef ACCOUNTREWARDSTAB_H
#define ACCOUNTREWARDSTAB_H

#include "../AccountCreateEdit/accountedittab.h"
#include "rewardsprogram.h"

class QComboBox;
class QLineEdit;
class QRadioButton;
class QPushButton;
class QGroupBox;
class QAbstractItemModel;

namespace KLib
{
    class AccountSelector;
}

class AccountRewardsTab : public AccountEditTab
{
    Q_OBJECT

    public:
        explicit AccountRewardsTab(QWidget *parent = nullptr);

        QString tabName() const override;
        QString tabIcon() const override;

        bool enableIf(int _accountType) const override;
        void fillData(const KLib::Account* _account) override;

        void save(KLib::Account* _account) const override;

        QStringList validate() const override;

        void onAccountTypeChanged() override;

    private slots:
        void onNoProgram();
        void onNewProgram();
        void onExistingProgram();

        void onNewTargetAccount();
        void onExistingTargetAccount();

        void onNewSourceAccount();
        void onExistingSourceAccount();

        void onProgramOrCurrencyChanged();

        void addCurrency();
        void editTiers();

    private:
        //Rewards Program
        QRadioButton* m_optNoProgram;
        QRadioButton* m_optExistingProgram;
        QRadioButton* m_optNewProgram;
        QComboBox* m_cboRewardsProgram;
        QLineEdit* m_txtProgramName;
        QComboBox* m_cboCurrency;
        QPushButton* m_btnAddCurrency;
        QPushButton* m_btnEditTiers;
        //Need the tier editor...

        //Rewards Target Account
        QGroupBox* m_grpTargetAccount;
        QRadioButton* m_optExistingTargetAccount;
        QRadioButton* m_optNewTargetAccount;
        KLib::AccountSelector* m_cboTargetAccount;
        QLineEdit* m_txtTargetAccountName;
        KLib::AccountSelector* m_cboTargetParentAccount;

        //Rewards Source Account
        QGroupBox* m_grpSourceAccount;
        QRadioButton* m_optExistingSourceAccount;
        QRadioButton* m_optNewSourceAccount;
        KLib::AccountSelector* m_cboSourceAccount;
        QLineEdit* m_txtSourceAccountName;
        KLib::AccountSelector* m_cboSourceParentAccount;


        QTimer*    m_timer;
        QList<RewardTier> m_tiers;
        QString m_currentCurrency;

};

#endif // ACCOUNTREWARDSTAB_H
