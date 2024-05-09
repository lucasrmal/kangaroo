#include "formmultiinvestmententry.h"

#include <QDateEdit>
#include <QFormLayout>
#include <QGridLayout>
#include <QHeaderView>
#include <QLabel>
#include <QMessageBox>
#include <QSettings>
#include <QTableView>
#include <functional>

#include "../../model/account.h"
#include "../../model/currency.h"
#include "../../model/investmenttransaction.h"
#include "../../model/ledger.h"
#include "../../model/modelexception.h"
#include "../../model/pricemanager.h"
#include "../../model/security.h"
#include "../../model/transaction.h"
#include "../../util/balances.h"
#include "../core.h"
#include "../widgets/amountedit.h"

namespace KLib {

RowInfo::RowInfo(Account* _account)
    : m_account(_account), m_security(nullptr), m_currency(nullptr) {
  if (m_account->idSecurity() != -1) {
    m_security = SecurityManager::instance()->get(account()->idSecurity());
    m_currency = CurrencyManager::instance()->get(m_security->currency());
  }
}

MultiInvestmentModel::MultiInvestmentModel(
    const QLinkedList<Account*>& _accounts, QObject* _parent)
    : QAbstractTableModel(_parent) {
  for (Account* a : _accounts) {
    m_rows.push_back(RowInfo(a));
  }
}

int MultiInvestmentModel::rowCount(const QModelIndex&) const {
  return m_rows.count();
}

int MultiInvestmentModel::columnCount(const QModelIndex&) const {
  return Columns::NumColumns;
}

QVariant MultiInvestmentModel::data(const QModelIndex& _index,
                                    int _role) const {
  if ((_role != Qt::DisplayRole && _role != Qt::EditRole) ||
      !_index.isValid() || _index.row() >= rowCount()) {
    return QVariant();
  }

  const RowInfo& info = m_rows[_index.row()];

  switch (_index.column()) {
    case COLUMN_ACCOUNT:
      return info.account()->name();

    case COLUMN_MEMO:
      return info.memo;

    case COLUMN_SHARES:
      if (info.shares != 0) {
        return info.shares.toString();
      }

    case COLUMN_AMOUNT: {
      Amount a;
      if (info.amount != 0) {
        a = info.amount;
      } else if (info.shares != 0) {
        a = info.shares * info.pricePerShare;
      } else {
        return QVariant();
      }

      if (_role == Qt::DisplayRole) {
        return info.currency()->formatAmount(a);
      } else {
        return a.toString();
      }
    }

    case COLUMN_PRICE_PER_SHARE: {
      Amount a;

      if (info.pricePerShare != 0) {
        a = info.pricePerShare;
      } else if (info.shares != 0) {
        a = info.amount / info.shares.toDouble();
      } else {
        return QVariant();
      }

      if (_role == Qt::DisplayRole) {
        return info.currency()->formatAmount(a);
      } else {
        return a.toString();
      }
    }
    default:
      return QVariant();
  }
}

QVariant MultiInvestmentModel::headerData(int _section,
                                          Qt::Orientation _orientation,
                                          int _role) const {
  if (_role != Qt::DisplayRole || _orientation != Qt::Horizontal) {
    return QVariant();
  }

  switch (_section) {
    case COLUMN_ACCOUNT:
      return tr("Account");

    case COLUMN_MEMO:
      return tr("Memo");

    case COLUMN_SHARES:
      return tr("Shares");

    case COLUMN_AMOUNT:
      return tr("Total");

    case COLUMN_PRICE_PER_SHARE:
      return tr("Price per share");

    default:
      return QVariant();
  }
}

Qt::ItemFlags MultiInvestmentModel::flags(const QModelIndex& _index) const {
  if (_index.isValid() && _index.row() <= rowCount() &&
      _index.column() != COLUMN_ACCOUNT) {
    return QAbstractTableModel::flags(_index) | Qt::ItemIsEditable;
  } else {
    return QAbstractTableModel::flags(_index);
  }
}

bool MultiInvestmentModel::setData(const QModelIndex& _index,
                                   const QVariant& _value, int _role) {
  if (_role != Qt::EditRole || !_index.isValid() ||
      _index.row() >= rowCount()) {
    return false;
  }

  RowInfo& info = m_rows[_index.row()];

  switch (_index.column()) {
    case COLUMN_MEMO:
      info.memo = _value.toString();
      break;

    case COLUMN_SHARES:
      info.shares = Amount::fromUserLocale(_value.toString(),
                                           info.security()->precision());
      break;

    case COLUMN_AMOUNT:
      info.amount = Amount::fromUserLocale(_value.toString(),
                                           info.currency()->precision());
      info.pricePerShare = 0;
      break;

    case COLUMN_PRICE_PER_SHARE:
      info.pricePerShare = Amount::fromUserLocale(_value.toString(),
                                                  info.security()->precision());
      info.amount = 0;
      break;

    default:
      return false;
  }

  emit dataChanged(index(_index.row(), 0),
                   index(_index.row(), columnCount() - 1));
  return true;
}

void MultiInvestmentModel::save(const QDate& _date,
                                const Account* m_brokerageAccount) {
  for (RowInfo& info : m_rows) {
    if (info.shares != 0 && (info.amount != 0 || info.pricePerShare != 0)) {
      if (info.pricePerShare == 0) {
        info.pricePerShare =
            info.amount.toPrecision(info.security()->precision()) /
            info.shares.toDouble();
      } else {
        info.amount = (info.pricePerShare * info.shares)
                          .toPrecision(info.currency()->precision());
      }

      QList<Transaction::Split> splits;
      QList<InvestmentSplitType> types;

      splits << Transaction::Split(-info.amount, m_brokerageAccount->id(),
                                   info.currency()->code());
      types << InvestmentSplitType::CostProceeds;

      splits << Transaction::Split(info.shares, info.account()->id(), "");
      types << InvestmentSplitType::Investment;

      Transaction::addTradingSplits(splits);
      while (types.count() < splits.count()) {
        types << InvestmentSplitType::Trading;
      }

      InvestmentTransaction* transaction = new InvestmentTransaction;
      transaction->setDate(_date);
      transaction->setMemo(info.memo);
      transaction->makeBuySellFee(InvestmentAction::Buy, info.pricePerShare,
                                  splits, types);
      LedgerManager::instance()->addTransaction(transaction);
    }
  }
}

FormMultiInvestmentEntry::FormMultiInvestmentEntry(int _brokerageAccountId,
                                                   QWidget* _parent)
    : CAMSEGDialog(DialogWithPicture, OkCancelButtons, _parent),
      m_brokerageAccount(Account::getTopLevel()->account(_brokerageAccountId)) {
  m_dteEdit = new QDateEdit(QDate::currentDate(), this);
  m_lblTotal = new QLabel(this);
  QLabel* lblAccount = new QLabel(m_brokerageAccount->name(), this);
  QFont font = lblAccount->font();
  font.setBold(true);
  lblAccount->setFont(font);
  m_lblTotal->setFont(font);
  m_lblTotal->setAlignment(Qt::AlignHCenter | Qt::AlignRight);

  m_transactionTable = new QTableView(this);
  m_transactionTable->verticalHeader()->setVisible(false);

  QFormLayout* formLayout = new QFormLayout();
  formLayout->addRow(tr("Brokerage account: "), lblAccount);
  formLayout->addRow(tr("Transaction date: "), m_dteEdit);

  QWidget* widget = new QWidget;
  setCentralWidget(widget);
  QGridLayout* layout = new QGridLayout();
  layout->addItem(formLayout, 0, 0, 2, 1);
  layout->addWidget(m_lblTotal, 0, 1, 1, 1, Qt::AlignRight);
  layout->addWidget(m_transactionTable, 2, 0, 1, 2);
  widget->setLayout(layout);

  setBothTitles(tr("Investment Multi Buy"));
  setPicture(Core::pixmap("brokerage-account"));

  createModel();
}

void FormMultiInvestmentEntry::accept() {
  QStringList errors;

  if (!m_dteEdit->date().isValid()) {
    errors << tr("The investment date is invalid.");
  }

  int numInvestments = 0;
  for (const RowInfo& info : m_model->rowInfo()) {
    if (info.shares != 0) {
      ++numInvestments;

      if (info.amount == 0 && info.pricePerShare == 0) {
        errors << tr("Enter an amount or a price per share for %1.")
                      .arg(info.account()->name());
      }
    } else if (info.amount != 0 || info.pricePerShare != 0) {
      errors
          << tr("Enter a number of shares for %1.").arg(info.account()->name());
    }
  }

  if (numInvestments == 0) {
    errors << tr("At least one investment must be entered.");
  }

  if (errors.count() > 0) {
    QString strErrors;
    for (QString s : errors) {
      strErrors += "\t" + s + "\n";
    }
    QMessageBox::information(this, tr("Save Changes"),
                             tr("The following errors prevent the action to be "
                                "executed:\n %1.")
                                 .arg(strErrors),
                             QMessageBox::Ok);
    return;
  }

  // Save.
  try {
    m_model->save(m_dteEdit->date(), m_brokerageAccount);
    done(QDialog::Accepted);
  } catch (ModelException e) {
    QMessageBox::warning(
        this, tr("Save Changes"),
        tr("An error occured while saving the transaction(s): %1")
            .arg(e.description()),
        QMessageBox::Ok);
  }
}

void FormMultiInvestmentEntry::updateTotal() {
  Balances total;

  for (const RowInfo& info : m_model->rowInfo()) {
    if (info.shares != 0) {
      Amount amt =
          info.amount != 0 ? info.amount : info.shares * info.pricePerShare;
      total.add(info.currency()->code(),
                amt.toPrecision(info.currency()->precision()));
    }
  }

  QString str = tr("Total: ");

  for (auto i = total.begin(); i != total.end(); ++i) {
    const Currency* c = CurrencyManager::instance()->get(i.key());
    str += c->formatAmount(*i);

    if (i + 1 != total.end()) {
      str += "<br>";
    }
  }

  if (total.isEmpty()) {
    str = tr("No investment created.");
  }

  m_lblTotal->setText(str);
}

void FormMultiInvestmentEntry::createModel() {
  QLinkedList<Account*> accounts;

  // If the parent of this account is a placeholder and has only 1 brokerage
  // account, use the parent. Otherwise, use this account.
  bool useParent = false;
  if (m_brokerageAccount->hasParent() &&
      m_brokerageAccount->parent()->isPlaceholder()) {
    int numBrokerage = 0;

    for (const Account* a : m_brokerageAccount->parent()->getChildren()) {
      if (a->type() == AccountType::BROKERAGE) ++numBrokerage;
    }

    useParent = numBrokerage == 1;
  }

  std::function<void(Account*)> findAccounts;
  findAccounts = [this, &accounts, &findAccounts](Account* account) {
    if (account->type() == AccountType::INVESTMENT && account->isOpen() &&
        !account->isPlaceholder() &&
        m_brokerageAccount->supportsCurrency(SecurityManager::instance()
                                                 ->get(account->idSecurity())
                                                 ->currency())) {
      accounts.push_back(account);
    }

    for (Account* a : account->getChildren()) {
      findAccounts(a);
    }
  };

  findAccounts(useParent ? m_brokerageAccount->parent() : m_brokerageAccount);

  m_model = new MultiInvestmentModel(accounts, this);
  m_transactionTable->setItemDelegate(
      new MultiInvestmentEntryDelegate(m_model, this));
  m_transactionTable->setModel(m_model);

  m_transactionTable->setColumnWidth(MultiInvestmentModel::COLUMN_ACCOUNT, 250);
  m_transactionTable->setColumnWidth(MultiInvestmentModel::COLUMN_MEMO, 250);
  m_transactionTable->setColumnWidth(MultiInvestmentModel::COLUMN_SHARES, 100);
  m_transactionTable->setColumnWidth(MultiInvestmentModel::COLUMN_AMOUNT, 100);
  m_transactionTable->setColumnWidth(
      MultiInvestmentModel::COLUMN_PRICE_PER_SHARE, 100);
  m_transactionTable->horizontalHeader()->setSectionResizeMode(
      MultiInvestmentModel::COLUMN_MEMO, QHeaderView::Stretch);

  connect(m_model, SIGNAL(dataChanged(QModelIndex, QModelIndex)), this,
          SLOT(updateTotal()));
}

MultiInvestmentEntryDelegate::MultiInvestmentEntryDelegate(
    const MultiInvestmentModel* _model, QObject* _parent)
    : QStyledItemDelegate(_parent), m_model(_model) {
  QSettings settings;
  m_rowHeight = settings.value("Ledger/RowHeight").toInt();
}

QSize MultiInvestmentEntryDelegate::sizeHint(
    const QStyleOptionViewItem& _option, const QModelIndex& _index) const {
  return QSize(QStyledItemDelegate::sizeHint(_option, _index).width(),
               m_rowHeight);
}

QWidget* MultiInvestmentEntryDelegate::createEditor(
    QWidget* _parent, const QStyleOptionViewItem& _option,
    const QModelIndex& _index) const {
  switch (_index.column()) {
    case MultiInvestmentModel::COLUMN_SHARES:
    case MultiInvestmentModel::COLUMN_PRICE_PER_SHARE: {
      AmountEdit* amtEdit = new AmountEdit(
          m_model->rowInfo().at(_index.row()).security()->precision(), _parent);
      amtEdit->setMinimum(0);
      amtEdit->setFocus();
      return amtEdit;
    }
    case MultiInvestmentModel::COLUMN_AMOUNT: {
      AmountEdit* amtEdit = new AmountEdit(
          m_model->rowInfo().at(_index.row()).currency()->precision(), _parent);
      amtEdit->setMinimum(0);
      amtEdit->setFocus();
      return amtEdit;
    }
    default:
      return QStyledItemDelegate::createEditor(_parent, _option, _index);
  }
}

void MultiInvestmentEntryDelegate::setEditorData(
    QWidget* _editor, const QModelIndex& _index) const {
  switch (_index.column()) {
    case MultiInvestmentModel::COLUMN_SHARES:
    case MultiInvestmentModel::COLUMN_PRICE_PER_SHARE:
    case MultiInvestmentModel::COLUMN_AMOUNT:
      static_cast<AmountEdit*>(_editor)->setAmount(Amount::fromUserLocale(
          _index.model()->data(_index, Qt::EditRole).toString()));
      break;

    default:
      return QStyledItemDelegate::setEditorData(_editor, _index);
  }
}
void MultiInvestmentEntryDelegate::setModelData(
    QWidget* _editor, QAbstractItemModel* _model,
    const QModelIndex& _index) const {
  switch (_index.column()) {
    case MultiInvestmentModel::COLUMN_SHARES:
    case MultiInvestmentModel::COLUMN_PRICE_PER_SHARE:
    case MultiInvestmentModel::COLUMN_AMOUNT:
      _model->setData(_index,
                      static_cast<AmountEdit*>(_editor)->amount().toString(),
                      Qt::EditRole);
      break;

    default:
      return QStyledItemDelegate::setModelData(_editor, _model, _index);
  }
}

void MultiInvestmentEntryDelegate::updateEditorGeometry(
    QWidget* _editor, const QStyleOptionViewItem& _option,
    const QModelIndex& _index) const {
  Q_UNUSED(_index)
  _editor->setGeometry(_option.rect);
}

}  // namespace KLib
