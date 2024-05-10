#include "accountrewardstab.h"

#include <KangarooLib/controller/currencycontroller.h>
#include <KangarooLib/model/account.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/widgets/accountselector.h>

#include <QComboBox>
#include <QFormLayout>
#include <QGridLayout>
#include <QGroupBox>
#include <QHBoxLayout>
#include <QLabel>
#include <QLineEdit>
#include <QPushButton>
#include <QRadioButton>
#include <QSortFilterProxyModel>
#include <QTimer>

#include "../ManageOthers/formeditcurrency.h"
#include "rewardsplugin.h"
#include "rewardsprogramcontroller.h"
#include "tiereditor.h"

using namespace KLib;

AccountRewardsTab::AccountRewardsTab(QWidget* parent) : AccountEditTab(parent) {
  // Setup UI
  m_timer = new QTimer(this);

  // Rewards Program
  m_optNoProgram = new QRadioButton(tr("No Rewards Program"), this);
  m_optExistingProgram = new QRadioButton(tr("&Existing Program:"), this);
  m_optNewProgram = new QRadioButton(tr("&New Program"), this);
  m_cboRewardsProgram = new QComboBox(this);
  m_txtProgramName = new QLineEdit(this);
  m_cboCurrency = new QComboBox(this);
  m_btnEditTiers = new QPushButton(Core::icon("tiers"), tr("&Tiers"), this);
  m_btnAddCurrency = new QPushButton(Core::icon("list-add"), "", this);
  m_btnAddCurrency->setMaximumWidth(24);

  // Target Account
  m_optExistingTargetAccount = new QRadioButton(tr("E&xisting Account"), this);
  m_optNewTargetAccount = new QRadioButton(tr("Ne&w Account"), this);
  m_cboTargetAccount = new AccountSelector({}, this);

  m_txtTargetAccountName = new QLineEdit(this);
  m_cboTargetParentAccount = new AccountSelector(
      {.selectorFlags = AccountSelectorFlags::Flag_IncludePlaceholders,
       .typeFlags = AccountTypeFlags::Flag_Asset},
      this);

  m_grpTargetAccount = new QGroupBox(tr("Deposit Rewards In"));

  // Source Account
  m_optExistingSourceAccount = new QRadioButton(tr("E&xisting Account"), this);
  m_optNewSourceAccount = new QRadioButton(tr("Ne&w Account"), this);
  m_cboSourceAccount = new AccountSelector({}, this);

  m_txtSourceAccountName = new QLineEdit(this);
  m_cboSourceParentAccount = new AccountSelector(
      {.selectorFlags = AccountSelectorFlags::Flag_IncludePlaceholders,
       .typeFlags = AccountTypeFlags::Flag_Income},
      this);

  m_grpSourceAccount = new QGroupBox(tr("Take Rewards From"));

  QHBoxLayout* rewLayout = new QHBoxLayout();
  rewLayout->addWidget(m_txtProgramName);
  rewLayout->addWidget(m_btnEditTiers);

  QHBoxLayout* curLayout = new QHBoxLayout();
  curLayout->addWidget(m_cboCurrency);
  curLayout->addWidget(m_btnAddCurrency);

  QLabel* lblCurrency = new QLabel(tr("Program &Currency:"), this);
  lblCurrency->setBuddy(m_cboCurrency);

  QFormLayout* formLayout = new QFormLayout(this);
  formLayout->addRow(m_optNoProgram);
  formLayout->addRow(m_optExistingProgram, m_cboRewardsProgram);
  formLayout->addRow(m_optNewProgram, rewLayout);
  formLayout->addRow(lblCurrency, curLayout);
  formLayout->addRow(m_grpTargetAccount);
  formLayout->addRow(m_grpSourceAccount);

  m_cboCurrency->setModel(CurrencyController::sortProxy(this));
  m_cboCurrency->setModelColumn(CurrencyColumn::NAME);

  m_cboRewardsProgram->setModel(new RewardsProgramController(this));
  m_cboRewardsProgram->setModelColumn(RewardsProgramColumn::NAME);

  QFormLayout* formSourceAccount = new QFormLayout(m_grpTargetAccount);
  formSourceAccount->addRow(m_optExistingTargetAccount, m_cboTargetAccount);
  formSourceAccount->addRow(m_optNewTargetAccount, m_txtTargetAccountName);
  formSourceAccount->addRow(tr("Account &Parent:"), m_cboTargetParentAccount);

  QFormLayout* formTargetAccount = new QFormLayout(m_grpSourceAccount);
  formTargetAccount->addRow(m_optExistingSourceAccount, m_cboSourceAccount);
  formTargetAccount->addRow(m_optNewSourceAccount, m_txtSourceAccountName);
  formTargetAccount->addRow(tr("Account &Parent:"), m_cboSourceParentAccount);

  connect(m_optNoProgram, &QRadioButton::toggled, this,
          &AccountRewardsTab::onNoProgram);
  connect(m_optExistingProgram, &QRadioButton::toggled, this,
          &AccountRewardsTab::onExistingProgram);
  connect(m_optNewProgram, &QRadioButton::toggled, this,
          &AccountRewardsTab::onNewProgram);

  connect(m_optExistingTargetAccount, &QRadioButton::toggled, this,
          &AccountRewardsTab::onExistingTargetAccount);
  connect(m_optNewTargetAccount, &QRadioButton::toggled, this,
          &AccountRewardsTab::onNewTargetAccount);

  connect(m_optExistingSourceAccount, &QRadioButton::toggled, this,
          &AccountRewardsTab::onExistingSourceAccount);
  connect(m_optNewSourceAccount, &QRadioButton::toggled, this,
          &AccountRewardsTab::onNewSourceAccount);

  connect(m_btnAddCurrency, &QPushButton::clicked, this,
          &AccountRewardsTab::addCurrency);

  connect(m_btnEditTiers, &QPushButton::clicked, this,
          &AccountRewardsTab::editTiers);

  connect(m_cboRewardsProgram, SIGNAL(currentIndexChanged(int)), this,
          SLOT(onProgramOrCurrencyChanged()));
  connect(m_cboCurrency, SIGNAL(currentIndexChanged(int)), this,
          SLOT(onProgramOrCurrencyChanged()));

  m_optNoProgram->setChecked(true);
  m_optNewTargetAccount->setChecked(true);
  m_optNewSourceAccount->setChecked(true);

  // Only allow to select existing programs if some are defined...
  m_optExistingProgram->setEnabled(m_cboRewardsProgram->count() > 0);
}

void AccountRewardsTab::onNoProgram() {
  m_cboRewardsProgram->setEnabled(false);
  m_txtProgramName->setEnabled(false);
  m_cboCurrency->setEnabled(false);
  m_btnAddCurrency->setEnabled(false);
  m_btnEditTiers->setEnabled(false);
  m_grpTargetAccount->setEnabled(false);
  m_grpSourceAccount->setEnabled(false);
}

void AccountRewardsTab::onExistingProgram() {
  m_cboRewardsProgram->setEnabled(true);
  m_txtProgramName->setEnabled(false);
  m_cboCurrency->setEnabled(false);
  m_btnAddCurrency->setEnabled(false);
  m_btnEditTiers->setEnabled(false);
  m_grpTargetAccount->setEnabled(true);
  m_grpSourceAccount->setEnabled(true);
  m_cboRewardsProgram->setFocus();
  onProgramOrCurrencyChanged();
}

void AccountRewardsTab::onNewProgram() {
  m_cboRewardsProgram->setEnabled(false);
  m_txtProgramName->setEnabled(true);
  m_cboCurrency->setEnabled(true);
  m_btnAddCurrency->setEnabled(true);
  m_btnEditTiers->setEnabled(true);
  m_grpTargetAccount->setEnabled(true);
  m_grpSourceAccount->setEnabled(true);
  m_txtProgramName->setFocus();
  onProgramOrCurrencyChanged();
}

void AccountRewardsTab::onExistingTargetAccount() {
  m_cboTargetAccount->setEnabled(true);
  m_txtTargetAccountName->setEnabled(false);
  m_cboTargetParentAccount->setEnabled(false);
  m_cboTargetAccount->setFocus();
}

void AccountRewardsTab::onNewTargetAccount() {
  m_cboTargetAccount->setEnabled(false);
  m_txtTargetAccountName->setEnabled(true);
  m_cboTargetParentAccount->setEnabled(true);
  m_txtTargetAccountName->setFocus();
}

void AccountRewardsTab::onExistingSourceAccount() {
  m_cboSourceAccount->setEnabled(true);
  m_txtSourceAccountName->setEnabled(false);
  m_cboSourceParentAccount->setEnabled(false);
  m_cboSourceAccount->setFocus();
}

void AccountRewardsTab::onNewSourceAccount() {
  m_cboSourceAccount->setEnabled(false);
  m_txtSourceAccountName->setEnabled(true);
  m_cboSourceParentAccount->setEnabled(true);
  m_txtSourceAccountName->setFocus();
}

void AccountRewardsTab::onProgramOrCurrencyChanged() {
  // Only show accounts with that currency
  QString currency;

  if (m_optExistingProgram->isChecked()) {
    if (m_cboRewardsProgram->count() == 0)  // No rewards program selected
    {
      m_cboTargetAccount->resetSelectorParams({});
      m_cboSourceAccount->resetSelectorParams({});
      m_cboTargetAccount->setCurrentText("");
      m_cboSourceAccount->setCurrentText("");
      return;
    } else {
      currency = RewardsProgramManager::instance()
                     ->get(m_cboRewardsProgram->currentData().toInt())
                     ->currency();
    }
  } else  // New
  {
    currency = m_cboCurrency->currentData().toString();
  }

  // Don't reload if the currency is the same...
  if (m_currentCurrency.isEmpty() || m_currentCurrency != currency) {
    m_cboTargetAccount->resetSelectorParams(
        {.typeFlags = AccountTypeFlags::Flag_Asset,
         .currencyRestrict = currency});
    m_cboTargetAccount->setCurrentText("");

    m_cboSourceAccount->resetSelectorParams(
        {.typeFlags = AccountTypeFlags::Flag_Income,
         .currencyRestrict = currency});
    m_cboSourceAccount->setCurrentText("");

    m_currentCurrency = currency;
  }
}

void AccountRewardsTab::addCurrency() {
  FormEditCurrency form(nullptr, this);
  QString cur;

  if (form.exec() == QDialog::Accepted) {
    cur = form.currency()->code();
    m_timer->setInterval(500);

    connect(m_timer, &QTimer::timeout, [=]() {
      m_cboCurrency->setCurrentIndex(m_cboCurrency->findData(cur));
      m_timer->stop();
      m_timer->disconnect();
    });
    m_timer->start();
  }
}

void AccountRewardsTab::editTiers() {
  if (m_optNewProgram->isChecked()) {
    FormTierEditor* f = new FormTierEditor(m_tiers, this);
    f->exec();
    delete f;
  }
}

QString AccountRewardsTab::tabName() const { return tr("Rewards"); }

QString AccountRewardsTab::tabIcon() const { return "favorites"; }

bool AccountRewardsTab::enableIf(int _accountType) const {
  switch (_accountType) {
    case AccountType::CHECKING:
    case AccountType::PREPAIDCARD:
    case AccountType::CREDITCARD:
      return true;

    default:
      return false;
  }
}

void AccountRewardsTab::fillData(const KLib::Account* _account) {
  Q_UNUSED(_account)

  if (_account->properties()->contains(RewardsProgram::PROP_REWARDS_PROGRAM)) {
    try {
      RewardsProgram* p = RewardsProgramManager::instance()->get(
          _account->properties()
              ->get(RewardsProgram::PROP_REWARDS_PROGRAM)
              .toInt());

      m_optExistingProgram->setChecked(true);
      m_cboRewardsProgram->setCurrentIndex(
          m_cboRewardsProgram->findData(p->id()));
      onProgramOrCurrencyChanged();

      try {
        Account* target = Account::getTopLevel()->account(
            _account->properties()
                ->get(RewardsProgram::PROP_REWARDS_TARGET_ACCOUNT)
                .toInt());
        Account* source = Account::getTopLevel()->account(
            _account->properties()
                ->get(RewardsProgram::PROP_REWARDS_SOURCE_ACCOUNT)
                .toInt());

        m_optExistingTargetAccount->setChecked(true);
        m_optExistingSourceAccount->setChecked(true);

        // Set the account, but do this a little later to let AccountSelector
        // the time to load the accounts...
        m_timer->setInterval(500);

        connect(m_timer, &QTimer::timeout, [=]() {
          m_cboTargetAccount->setCurrentAccount(target->id());
          m_cboSourceAccount->setCurrentAccount(source->id());
          m_timer->stop();
          m_timer->disconnect();
        });
        m_timer->start();
      } catch (...) {
        m_optNewTargetAccount->setChecked(true);
        m_optNewSourceAccount->setChecked(true);
      }
    } catch (...) {
      m_optNoProgram->setChecked(true);
    }
  } else {
    m_optNoProgram->setChecked(true);
  }
}

void AccountRewardsTab::save(KLib::Account* _account) const {
  bool set = !m_optNoProgram->isChecked();

  switch (_account->type()) {
    case AccountType::CHECKING:
    case AccountType::PREPAIDCARD:
    case AccountType::CREDITCARD:
      break;  // OK, don't do anything

    default:
      set = false;  // Don't set anyting
  }

  if (set) {
    RewardsProgram* p = nullptr;
    Account* target = nullptr;
    Account* source = nullptr;

    if (m_optExistingProgram->isChecked()) {
      p = RewardsProgramManager::instance()->get(
          m_cboRewardsProgram->currentData().toInt());
    } else  // New program
    {
      QList<RewardTier> tiers = m_tiers;
      // Set the tier IDs
      for (int i = 0; i < tiers.count(); ++i) {
        tiers[i].id = i;
      }

      p = RewardsProgramManager::instance()->add(
          m_txtProgramName->text(), m_cboCurrency->currentData().toString(),
          tiers);
    }

    if (m_optExistingTargetAccount->isChecked()) {
      target = m_cboTargetAccount->currentAccount();
    } else {
      target = m_cboTargetParentAccount->currentAccount()->addChild(
          m_txtTargetAccountName->text(), RewardsProgram::REWARDS_ACCOUNT_TYPE,
          p->currency(), Constants::NO_ID);
    }

    if (m_optExistingSourceAccount->isChecked()) {
      source = m_cboSourceAccount->currentAccount();
    } else {
      source = m_cboSourceParentAccount->currentAccount()->addChild(
          m_txtSourceAccountName->text(), AccountType::INCOME, p->currency(),
          Constants::NO_ID);
    }

    _account->properties()->set(RewardsProgram::PROP_REWARDS_PROGRAM, p->id());
    _account->properties()->set(RewardsProgram::PROP_REWARDS_TARGET_ACCOUNT,
                                target->id());
    _account->properties()->set(RewardsProgram::PROP_REWARDS_SOURCE_ACCOUNT,
                                source->id());
  } else {
    _account->properties()->remove(RewardsProgram::PROP_REWARDS_PROGRAM);
    _account->properties()->remove(RewardsProgram::PROP_REWARDS_TARGET_ACCOUNT);
    _account->properties()->remove(RewardsProgram::PROP_REWARDS_SOURCE_ACCOUNT);
  }
}

QStringList AccountRewardsTab::validate() const {
  QStringList errors;

  if (!m_optNoProgram->isChecked()) {
    if (m_optExistingProgram->isChecked() &&
        m_cboRewardsProgram->currentIndex() < 0) {
      errors << tr("Select a rewards program.");
    }

    if (m_optNewProgram->isChecked()) {
      if (m_txtProgramName->text().isEmpty()) {
        errors << tr("Enter a name for the rewards program.");
      }
      if (m_cboCurrency->currentIndex() < 0) {
        errors << tr("Add a currency for the rewards program.");
      }

      // Tiers (should be checked by FormTierEditor, but just in case...
      for (const RewardTier& t : m_tiers) {
        if (t.name.isEmpty() || t.rate == 0) {
          errors << tr("At least one tier is invalid.");
          break;
        }
      }
    }

    if (m_optExistingTargetAccount->isChecked() &&
        !m_cboTargetAccount->currentAccount()) {
      errors << tr("Select a deposit account for the rewards program.");
    }
    if (m_optNewTargetAccount->isChecked() &&
        m_txtTargetAccountName->text().isEmpty()) {
      errors << tr("Enter a name for the deposit rewards account.");
    }
    if (m_optNewTargetAccount->isChecked() &&
        !m_cboTargetParentAccount->currentAccount()) {
      errors << tr("Select a parent for the deposit rewards account.");
    }

    if (m_optExistingSourceAccount->isChecked() &&
        !m_cboSourceAccount->currentAccount()) {
      errors << tr("Select an income account for the rewards program.");
    }
    if (m_optNewSourceAccount->isChecked() &&
        m_txtSourceAccountName->text().isEmpty()) {
      errors << tr("Enter a name for the income rewards account.");
    }
    if (m_optNewSourceAccount->isChecked() &&
        !m_cboSourceParentAccount->currentAccount()) {
      errors << tr("Select a parent for the income rewards account.");
    }
  }

  return errors;
}

void AccountRewardsTab::onAccountTypeChanged() {}
