/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  US
 */

#include "investmentledgercontroller.h"
#include "../../model/account.h"
#include "../../model/ledger.h"
#include "../../model/modelexception.h"
#include "../../model/security.h"
#include "../../model/pricemanager.h"
#include "../../model/currency.h"
#include "../../model/security.h"
#include "../../ui/widgets/accountselector.h"
#include "../../ui/widgets/amountedit.h"
#include "../../ui/widgets/ledgerwidget.h"
#include "../../ui/widgets/splitfractionwidget.h"
#include "../../ui/dialogs/formdistributioncomposition.h"
#include "../../ui/dialogs/formgainlosswizard.h"
#include "../../ui/core.h"
#include "../../ui/mainwindow.h"

#include <QMessageBox>
#include <QTableView>
#include <QComboBox>
#include <QAction>

namespace KLib
{
const QString InvestmentLedgerController::INVALID_TRANSFER_TEXT = QObject::tr("<< Invalid! >>");

InvestmentLedgerController::InvestmentLedgerController(Ledger* _ledger, QObject* _parent) :
  LedgerController(_ledger,
                   new InvestmentLedgerBuffer(_ledger->account()->idSecurity()),
                   _parent),
  m_security(SecurityManager::instance()->get(m_ledger->account()->idSecurity())),
  m_currency(CurrencyManager::instance()->get(m_security->currency()))
{
  m_invBuffer = static_cast<InvestmentLedgerBuffer*>(m_buffer);
  m_invBuffer->clear();

  connect(m_security, SIGNAL(modified()), this, SLOT(onSecurityModified()));
}

void InvestmentLedgerController::onSecurityModified()
{
  //Check if the currency was changed
  if (m_security->currency() != m_currency->code())
  {
    m_currency = CurrencyManager::instance()->get(m_security->currency());
  }
}

int InvestmentLedgerController::columnCount(const QModelIndex& _parent) const
{
  Q_UNUSED(_parent)
  return InvestmentLedgerColumn::NumColumns;
}

LedgerWidgetDelegate* InvestmentLedgerController::buildDelegate(LedgerWidget* _widget) const
{
  return new InvestmentLedgerWidgetDelegate(this, _widget);
}

QModelIndex InvestmentLedgerController::firstEditableIndex(const QModelIndex& _index) const
{
  if (_index.isValid())
  {
    return _index.parent().isValid() ? index(_index.row(), InvestmentLedgerColumn::TRANSFER, _index.parent())
                                     : index(_index.row(), InvestmentLedgerColumn::DATE);
  }
  else
  {
    return _index;
  }
}

QModelIndex InvestmentLedgerController::lastEditableIndex(const QModelIndex& _index) const
{
  if (_index.isValid())
  {
    QSet<InvestmentAction> allowsPrice = {InvestmentAction::Buy, InvestmentAction::Sell,
                                          InvestmentAction::ShortSell, InvestmentAction::ShortCover,
                                          InvestmentAction::ReinvestDistrib, InvestmentAction::ReinvestDiv };

    InvestmentAction ac = (InvestmentAction) data(index(/* row */ _index.parent().isValid()
                                                        ? _index.parent().row()
                                                        : _index.row(),
                                                        /* col */ InvestmentLedgerColumn::ACTION),
                                                  Qt::EditRole).toInt();

    return _index.parent().isValid() || !allowsPrice.contains(ac)
        ? index(_index.row(), InvestmentLedgerColumn::QUANTITY, _index.parent())
        : index(_index.row(), InvestmentLedgerColumn::PRICE);
  }
  else
  {
    return _index;
  }
}

Qt::ItemFlags InvestmentLedgerController::flags(const QModelIndex& _index) const
{
  if (!_index.isValid() || !m_ledger->account()->isOpen())
    return QAbstractItemModel::flags(_index);

  static const QSet<InvestmentAction> allowsPrice = {
    InvestmentAction::Buy, InvestmentAction::Sell, InvestmentAction::ShortSell, InvestmentAction::ShortCover,
    InvestmentAction::ReinvestDistrib, InvestmentAction::ReinvestDiv, InvestmentAction::Fee };

  InvestmentAction ac = (InvestmentAction) data(index(/* row */ _index.parent().isValid()
                                                      ? _index.parent().row()
                                                      : _index.row(),
                                                      /* col */ InvestmentLedgerColumn::ACTION),
                                                Qt::EditRole).toInt();

  if (_index.column() == col_balance()            // Balance column
      || (_index.parent().isValid()               // Sub-rows can only edit transfer or Quantity.
          && _index.column() != InvestmentLedgerColumn::TRANSFER
          && _index.column() != InvestmentLedgerColumn::QUANTITY)
      || (_index.column() == col_status()         // Status, Flag and Cleared cannot be edited
          || _index.column() == col_flag()
          || _index.column() == col_cleared())
      || (_index.column() == InvestmentLedgerColumn::PRICE // If no price for this action
          && !allowsPrice.contains(ac)))
  {
    return QAbstractItemModel::flags(_index);
  }
  else
  {
    return QAbstractItemModel::flags(_index) | Qt::ItemIsEditable;
  }
}

int InvestmentLedgerController::subRowCount(const Transaction* _tr) const
{
  const InvestmentTransaction* trans = qobject_cast<const InvestmentTransaction*>(_tr);

  return trans ? rowCountForAction(trans->action())-1 : 0;
}

Balances InvestmentLedgerController::cacheBalance(int _cacheRow) const
{
  const InvestmentTransaction* trans = qobject_cast<const InvestmentTransaction*>(transactionAtRow(_cacheRow));

  if (trans && trans->action() == InvestmentAction::StockSplit && _cacheRow > 0)
  {
    Balances b = m_cache.balanceAt(_cacheRow-1);
    b.add("", InvestmentTransaction::balanceAfterSplit(b.value(""), trans->splitFraction()));
    return b - Balances("", m_cache.balanceAt(_cacheRow-1).value(""));
  }
  else
  {
    return LedgerController::cacheBalance(_cacheRow);
  }
}

QVariant InvestmentLedgerController::cacheData(int _column, int _cacheRow, int _row, bool _editRole) const
{
  const InvestmentTransaction* trans = qobject_cast<const InvestmentTransaction*>(m_cache[_cacheRow].transaction());

  if (!trans)
    return QVariant();

  auto formatIdAccount = [_editRole, this] (const Transaction::Split& s) {
    if (_editRole)
    {
      return QVariant::fromValue<AccountCurrency>(AccountCurrency(s.idAccount, s.currency));
    }
    else
    {
      Account* a = Account::getTopLevel()->account(s.idAccount);

      return a ? QVariant(Account::getTopLevel()->getPath(a->id(), this->accountHeightDisplayed()))
               : QVariant(INVALID_TRANSFER_TEXT);
    }
  };

  switch (_column)
  {
  case InvestmentLedgerColumn::ACTION:
    if (_editRole)
    {
      return _row == 0 ? QVariant((int) trans->action()) : QVariant();
    }
    else if (_row == 0)
    {
      return InvestmentTransaction::actionToString(trans->action());
    }
    else
    {
      return QString("%1:").arg(InvestmentLedgerController::labelForRow(_row, trans->action()));
    }

  case InvestmentLedgerColumn::TRANSFER:
  {
    switch (_row)
    {
    case 0:
      switch (trans->action())
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
      case InvestmentAction::Fee:
        return formatIdAccount(trans->splitFor(InvestmentSplitType::CostProceeds));

      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        return formatIdAccount(trans->splitFor(InvestmentSplitType::DistributionSource));

      case InvestmentAction::Transfer:
        if (trans->splitFor(InvestmentSplitType::InvestmentFrom).idAccount == account()->id())
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::InvestmentTo));
        }
        else
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::InvestmentFrom));
        }

      case InvestmentAction::Swap:
        if (trans->splitFor(InvestmentSplitType::InvestmentFrom).idAccount == account()->id())
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::InvestmentFrom));
        }
        else
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::InvestmentTo));
        }

      default:
        break;
      }
      break;

    case 1:
      switch (trans->action())
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
        if (trans->hasSplitFor(InvestmentSplitType::Fee))
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::Fee));
        }
        else
        {
          return QVariant();
        }


      case InvestmentAction::Swap:
        if (trans->splitFor(InvestmentSplitType::InvestmentFrom).idAccount == account()->id())
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::InvestmentTo));
        }
        else
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::InvestmentFrom));
        }

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        return formatIdAccount(trans->splitFor(InvestmentSplitType::DistributionDest));

      default:
        break;
      }
      break;

    case 2:
      switch (trans->action())
      {
      //                case InvestmentAction::Buy:
      //                case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        if (trans->hasSplitFor(InvestmentSplitType::Tax))
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::Tax));
        }
        else
        {
          return QVariant();
        }

      default:
        break;
      }
      break;

    case 3:
      switch (trans->action())
      {
      case InvestmentAction::ShortCover:
      case InvestmentAction::Sell:
        if (trans->hasSplitFor(InvestmentSplitType::GainLoss))
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::GainLoss));
        }
        else
        {
          return QVariant();
        }

      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        if (trans->hasSplitFor(InvestmentSplitType::CashInLieu))
        {
          return formatIdAccount(trans->splitFor(InvestmentSplitType::CashInLieu));
        }
        else
        {
          return QVariant();
        }

      default:
        break;
      }
      break;
    }

    return QVariant();
  }
  case InvestmentLedgerColumn::QUANTITY:
  {
    Amount a;
    bool inCurrency = false;

    switch (_row)
    {
    case 0:
      switch (trans->action())
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        a = trans->splitFor(InvestmentSplitType::Investment).amount;
        break;
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
      case InvestmentAction::Fee:
        a = -trans->splitFor(InvestmentSplitType::Investment).amount;
        break;

      case InvestmentAction::Transfer:
      case InvestmentAction::Swap:
        if (trans->splitFor(InvestmentSplitType::InvestmentFrom).idAccount == account()->id())
        {
          a = trans->splitFor(InvestmentSplitType::InvestmentFrom).amount;
        }
        else
        {
          a = trans->splitFor(InvestmentSplitType::InvestmentTo).amount;
        }
        break;

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        a = -trans->splitFor(InvestmentSplitType::DistributionSource).amount;
        inCurrency = true;
        break;

      case InvestmentAction::StockSplit:
        return tr("%1 for %2").arg(trans->splitFraction().first).arg(trans->splitFraction().second);

      case InvestmentAction::CostBasisAdjustment:
      case InvestmentAction::UndistributedCapitalGain:
        a = trans->basisAdjustment();
        inCurrency = true;
        break;

      default:
        break;
      }
      break;

    case 1:
      switch (trans->action())
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
        if (trans->hasSplitFor(InvestmentSplitType::Fee))
        {
          a = trans->splitFor(InvestmentSplitType::Fee).amount;
          inCurrency = true;
        }
        else
        {
          return QVariant();
        }
        break;

      case InvestmentAction::Swap:
        if (trans->splitFor(InvestmentSplitType::InvestmentFrom).idAccount == account()->id())
        {
          a = trans->splitFor(InvestmentSplitType::InvestmentTo).amount;
        }
        else
        {
          a = trans->splitFor(InvestmentSplitType::InvestmentFrom).amount;
        }
        break;

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        a = trans->splitFor(InvestmentSplitType::DistributionDest).amount;
        inCurrency = true;
        break;

      case InvestmentAction::UndistributedCapitalGain:
        a = trans->taxPaid();
        inCurrency = true;
        break;

      default:
        break;
      }
      break;

    case 2:
      switch (trans->action())
      {
      //                case InvestmentAction::Buy:
      //                case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        if (trans->hasSplitFor(InvestmentSplitType::Tax))
        {
          a = trans->splitFor(InvestmentSplitType::Tax).amount;
          inCurrency = true;
        }
        else
        {
          return QVariant();
        }
        break;

      default:
        break;
      }
      break;

    case 3:
      switch (trans->action())
      {
      case InvestmentAction::ShortCover:
      case InvestmentAction::Sell:
        if (trans->hasSplitFor(InvestmentSplitType::GainLoss))
        {
          a = -trans->splitFor(InvestmentSplitType::GainLoss).amount;
          inCurrency = true;
        }
        else
        {
          return QVariant();
        }
        break;

      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        if (trans->hasSplitFor(InvestmentSplitType::CashInLieu))
        {
          a = trans->splitFor(InvestmentSplitType::CashInLieu).amount;
          inCurrency = true;
        }
        else
        {
          return QVariant();
        }
        break;

      default:
        break;
      }
      break;

    default:
      break;
    }

    return a == 0 ? QVariant()
                  : QVariant(inCurrency && !_editRole ? m_currency->formatAmount(a) : a.toString());
  }

  case InvestmentLedgerColumn::PRICE:
  {
    if (_row != 0)
      return QVariant();

    switch (trans->action())
    {
    case InvestmentAction::Buy:
    case InvestmentAction::ShortCover:
    case InvestmentAction::ReinvestDiv:
    case InvestmentAction::ReinvestDistrib:
    case InvestmentAction::Sell:
    case InvestmentAction::ShortSell:
    case InvestmentAction::Fee:
      return _editRole ? trans->pricePerShare().toString()
                       : m_currency->formatAmount(trans->pricePerShare());
    default:
      return _editRole ? 0 : QVariant();
    }
  }
  default:
    return LedgerController::cacheData(_column, _cacheRow, _row, _editRole);
  }
}

QVariant InvestmentLedgerController::headerData(int _section, Qt::Orientation _orientation, int _role) const
{
  if (_orientation == Qt::Horizontal && _role == Qt::DisplayRole)
  {
    switch (_section)
    {
    case InvestmentLedgerColumn::ACTION:
      return tr("Action");

    case InvestmentLedgerColumn::TRANSFER:
      return tr("Account");

    case InvestmentLedgerColumn::QUANTITY:
      return tr("Shares/Amt");

    case InvestmentLedgerColumn::PRICE:
      return tr("Price per share");
    }
  }

  return LedgerController::headerData(_section, _orientation, _role);
}

bool InvestmentLedgerController::canEditTransaction(const Transaction* _transaction, QString* _message) const
{
  if (!qobject_cast<const InvestmentTransaction*>(_transaction))
  {
    if (_message) *_message = tr("This ledger can only edit investment transactions.");
    return false;
  }
  else
  {
    return true;
  }
}

int InvestmentLedgerController::rowCountForAction(InvestmentAction _action)
{
  switch (_action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::ShortSell:
    return 2;

  case InvestmentAction::Sell:
  case InvestmentAction::ShortCover:
    return 4;

  case InvestmentAction::Transfer:
  case InvestmentAction::Fee:
    return 1;

  case InvestmentAction::Swap:
    return 2;

  case InvestmentAction::ReinvestDistrib:
  case InvestmentAction::ReinvestDiv:
    return 4;

  case InvestmentAction::Dividend:
  case InvestmentAction::Distribution:
    return 3;

  case InvestmentAction::CostBasisAdjustment:
    return 1;

  case InvestmentAction::UndistributedCapitalGain:
    return 2;

  default:
    return 1;
  }
}

QString InvestmentLedgerController::labelForRow(int _row, InvestmentAction _action)
{
  switch (_action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::ShortSell:
  case InvestmentAction::Sell:
  case InvestmentAction::ShortCover:
    switch (_row)
    {
    case 1:
      return tr("Fee/Comm");
    case 2:
      return tr("Tax");
    case 3:
      return tr("Gain/Loss");
    }
    break;

  case InvestmentAction::Swap:
    if (_row == 1)
    {
      return tr("Swap To");
    }
    break;

  case InvestmentAction::ReinvestDistrib:
  case InvestmentAction::ReinvestDiv:
    switch (_row)
    {
    case 1:
      return tr("Fee/Comm");
    case 2:
      return tr("Tax");
    case 3:
      return tr("Cash In Lieu");
    }
    break;

  case InvestmentAction::Dividend:
  case InvestmentAction::Distribution:
    switch (_row)
    {
    case 1:
      return tr("Deposit In");
    case 2:
      return tr("Tax");
    }

    break;

  case InvestmentAction::UndistributedCapitalGain:
    if (_row == 1)
    {
      return tr("Tax Paid");
    }
    break;

  default:
    break;
  }

  return QString();
}

void InvestmentLedgerController::editDistributionComposition(const QList<int>& _rows)
{
  if (!_rows.count()
      || (modifiedRow() != InvestmentLedgerBuffer::NO_ROW
          && (_rows.count() > 1 || _rows.first() != modifiedRow())))
  {
    return;
  }

  DistribComposition comp; bool set=false;

  for (int row : _rows)
  {
    if (row == newTransactionRow())
    {
      if (_rows.count() > 1)
      {
        return;
      }

      InvestmentAction a = (InvestmentAction) m_buffer->data(InvestmentLedgerColumn::ACTION,
                                                             0, true, this).toInt();

      if (a != InvestmentAction::Distribution
          && a != InvestmentAction::ReinvestDistrib)
      {
        return;
      }

      comp = qobject_cast<InvestmentLedgerBuffer*>(m_buffer)->distribComposition;
    }
    else
    {
      const InvestmentTransaction* tr = qobject_cast<const InvestmentTransaction*>(transactionAtRow(row));

      if (tr && (tr->action() == InvestmentAction::Distribution
                 || tr->action() == InvestmentAction::ReinvestDistrib))
      {
        if (!set) comp = tr->distribComposition();
      }
      else
      {
        return;
      }
    }
  }

  FormDistributionComposition form(comp, Core::instance()->mainWindow());

  if (form.exec() == QDialog::Accepted)
  {
    if (_rows.count() == 1 && _rows.first() == newTransactionRow())
    {
      qobject_cast<InvestmentLedgerBuffer*>(m_buffer)->distribComposition = comp;
    }
    else
    {
      for (int row : _rows)
      {
        InvestmentTransaction* tr = const_cast<InvestmentTransaction*>(
                                      qobject_cast<const InvestmentTransaction*>(transactionAtRow(row)));

        if (tr && (tr->action() == InvestmentAction::Distribution
                   || tr->action() == InvestmentAction::ReinvestDistrib))
        {
          tr->setDistribComposition(comp);
        }
      }
    }
  }
}

void InvestmentLedgerController::openGainLossWizard(const QModelIndex& _index)
{
  int mainRow = mapToCacheRow(_index);

  if (modifiedRow() != InvestmentLedgerBuffer::NO_ROW
      && modifiedRow() != mainRow)
  {
    return;
  }

  GainLossData d;
  m_invBuffer->loadGainLossData(d);
  d.idAccount = account()->id();

  if (m_buffer->row == newTransactionRow())
  {
    d.idTransaction = Constants::NO_ID;
  }
  else
  {
    d.idTransaction = transactionAtRow(m_buffer->row)->id();
  }

  FormGainLossWizard formGL(d, Core::instance()->mainWindow());
  if (formGL.exec() == QDialog::Accepted)
  {
    m_invBuffer->gainLoss = formGL.gainLoss();
    m_invBuffer->lots = formGL.lots();

    QModelIndex idx = index(2, InvestmentLedgerColumn::QUANTITY, index(m_buffer->row, 0));
    emit dataChanged(idx, idx);
  }
}

void InvestmentLedgerController::addExtraActions(LedgerWidget* _widget, std::function<void(int, LedgerAction*)> _addAction)
{
  LedgerAction* actGainLoss = new LedgerAction(new QAction(Core::icon("office-chart-line"),
                                                           tr("Gain Loss &Wizard"),
                                                           this), true,
                                               [this, _widget](const QList<int>&)
  {
    return account()->isOpen()
        && hasModifiedRow()
        && (m_invBuffer->action == InvestmentAction::Sell
            || m_invBuffer->action == InvestmentAction::ShortCover);
  },
  [this, _widget]()
  {
    if (hasModifiedRow())
    {
      openGainLossWizard(_widget->currentIndex());
    }
  });

  LedgerAction* actDistribComposition = new LedgerAction(new QAction(Core::icon("office-chart-pie"),
                                                                     tr("D&istribution Composition"),
                                                                     this), true,
                                                         [this, _widget](const QList<int>& rows)
  {
    if (account()->isOpen())
    {
      if (hasModifiedRow())
      {
        return m_invBuffer->action == InvestmentAction::Distribution
            || m_invBuffer->action == InvestmentAction::ReinvestDistrib;
      }
      else
      {
        bool enableDist = rows.count();
        for (int row : rows)
        {
          InvestmentAction a = (InvestmentAction) data(index(row, InvestmentLedgerColumn::ACTION),
                                                       Qt::EditRole).toInt();

          enableDist = enableDist
                       && (a == InvestmentAction::Distribution || a == InvestmentAction::ReinvestDistrib);
        }

        return enableDist;
      }
    }
    else
    {
      return false;
    }
  },
  [this, _widget]() { editDistributionComposition(_widget->selectedRows()); });

  _addAction(50, actGainLoss);
  _addAction(60, actDistribComposition);
}


//--------------------------------------------- BUFFER ---------------------------------------------

InvestmentLedgerBuffer::InvestmentLedgerBuffer(int _idSecurity) :
  m_security(SecurityManager::instance()->get(_idSecurity))
{
}


void InvestmentLedgerBuffer::clear()
{
  int prevRowCount = rowCount();
  LedgerBuffer::clear();

  action = InvestmentAction::Invalid;
  lots.clear();
  distribComposition.clear();
  splitFraction = QPair<int, int>(0,0);
  basisAdjustment = taxPaid = swapTo = cashInLieu = fee = quantity = pricePerShare = gainLoss = 0;
  idFeeAccount = idTaxAccount = idCashInLieuAccount = idDivDistToAccount = idGainLossAccount = Constants::NO_ID;

  rowCountChanged(prevRowCount);
}

void InvestmentLedgerBuffer::rowCountChanged(int _previous)
{
  int current = rowCount();

  for (int i = current; i < _previous; ++i)
  {
    emit rowRemoved(i);
  }

  for (int i = _previous; i < current; ++i)
  {
    emit rowInserted(i);
  }
}

void InvestmentLedgerBuffer::displayNetAmount()
{
  const QString cur = m_security->currency();

  auto getNetAmount = [&cur] (int _idAccount, const Amount& _gross) {

    try
    {
      if (_gross != 0 && _idAccount != Constants::NO_ID)
      {
        Account* a = Account::getTopLevel()->account(_idAccount);

        return _gross * (a->allCurrencies().contains(cur)
                         ? 1
                         : PriceManager::instance()->rate(a->mainCurrency(), cur));
      }
    }
    catch (ModelException) {}

    return Amount();
  };


  Amount net;

  switch (action)
  {
  case InvestmentAction::Buy:
    net = pricePerShare * quantity
          + getNetAmount(idFeeAccount, fee);
    break;

  case InvestmentAction::ShortCover:
    net = pricePerShare * quantity
          + getNetAmount(idFeeAccount, fee)
          + getNetAmount(idTaxAccount, taxPaid);
    break;

  case InvestmentAction::Sell:
    net = pricePerShare * quantity
          - getNetAmount(idFeeAccount, fee)
          - getNetAmount(idTaxAccount, taxPaid);
    break;

  case InvestmentAction::Fee:
    net = pricePerShare * quantity;
    break;

  case InvestmentAction::ShortSell:
    net = pricePerShare * quantity
          - getNetAmount(idFeeAccount, fee);
    break;

  case InvestmentAction::ReinvestDistrib:
  case InvestmentAction::ReinvestDiv:
    net = pricePerShare * quantity
          + getNetAmount(idCashInLieuAccount, cashInLieu)
          + getNetAmount(idFeeAccount, fee)
          + getNetAmount(idTaxAccount, taxPaid);
    break;

  default:
    emit showMessage(QString());
    return;
  }

  emit showMessage(tr("Net: %1")
                   .arg(CurrencyManager::instance()->get(cur)->formatAmount(net)));
}

QVariant InvestmentLedgerBuffer::data(int _column, int _row, bool _editRole, const LedgerController* _controller) const
{
  const InvestmentLedgerController* inv_con = qobject_cast<const InvestmentLedgerController*>(_controller);

  auto formatToQVariant = [_editRole, inv_con](const Amount& _a, bool inCurrency) {
    return _editRole ? QVariant(_a.toString())
                     : (_a == 0 ? QVariant()
                                : inCurrency ? QVariant(inv_con->currency()->formatAmount(_a))
                                             : QVariant(_a.toString()));
  };

  switch (_column)
  {
  case InvestmentLedgerColumn::ACTION:
    if (_editRole)
    {
      return _row == 0 ? QVariant((int) action) : QVariant();
    }
    else if (_row == 0)
    {
      return InvestmentTransaction::actionToString(action);
    }
    else
    {
      return QString("%1:").arg(InvestmentLedgerController::labelForRow(_row, action));
    }
  case InvestmentLedgerColumn::TRANSFER:
  {
    auto formatIdAccount = [_editRole, _controller] (int idAccount, const QString& currency) {
      if (_editRole)
      {
        return QVariant::fromValue<AccountCurrency>(AccountCurrency(idAccount, currency));
      }
      else
      {
        return idAccount == Constants::NO_ID ? QVariant()
                                             : QVariant(Account::getTopLevel()
                                                        ->getPath(idAccount,
                                                                  _controller->accountHeightDisplayed()));
      }
    };

    switch (_row)
    {
    case 0:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
      case InvestmentAction::Transfer:
      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
      case InvestmentAction::Fee:
        return formatIdAccount(idTransfer, transferCurrency);

      case InvestmentAction::Swap:
        return formatIdAccount(_controller->account()->id(), QString());

      default:
        break;
      }
      break;

    case 1:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
        return formatIdAccount(idFeeAccount, transferCurrency);

      case InvestmentAction::Swap:
        return formatIdAccount(idTransfer, QString());
        break;

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        return formatIdAccount(idDivDistToAccount, transferCurrency);
        break;

      default:
        break;
      }
      break;

    case 2:
      switch (action)
      {
      //                case InvestmentAction::Buy:
      //                case InvestmentAction::ShortSell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        return formatIdAccount(idTaxAccount, transferCurrency);

      default:
        break;
      }
      break;

    case 3:
      switch (action)
      {
      case InvestmentAction::ShortCover:
      case InvestmentAction::Sell:
        return formatIdAccount(idGainLossAccount, transferCurrency);

      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        return formatIdAccount(idCashInLieuAccount, transferCurrency);

      default:
        break;
      }
      break;
    }

    return QVariant();
  }

  case InvestmentLedgerColumn::QUANTITY:
  {
    switch (_row)
    {
    case 0:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
      case InvestmentAction::Transfer:
      case InvestmentAction::Swap:
      case InvestmentAction::Fee:
        return formatToQVariant(quantity, false);

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        return formatToQVariant(quantity, true);

      case InvestmentAction::StockSplit:
        return tr("%1 for %2").arg(splitFraction.first).arg(splitFraction.second);

      case InvestmentAction::CostBasisAdjustment:
      case InvestmentAction::UndistributedCapitalGain:
        return formatToQVariant(basisAdjustment, true);

      default:
        break;
      }
      break;

    case 1:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
        return formatToQVariant(fee, true);

      case InvestmentAction::Swap:
        return formatToQVariant(swapTo, false);

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        return formatToQVariant(quantity-taxPaid, true);

      case InvestmentAction::UndistributedCapitalGain:
        return formatToQVariant(taxPaid, true);

      default:
        break;
      }
      break;

    case 2:
      switch (action)
      {
      //                case InvestmentAction::Buy:
      //                case InvestmentAction::ShortSell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        return formatToQVariant(taxPaid, true);

      default:
        break;
      }
      break;

    case 3:
      switch (action)
      {
      case InvestmentAction::ShortCover:
      case InvestmentAction::Sell:
        return formatToQVariant(gainLoss, true);

      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        return formatToQVariant(cashInLieu, true);

      default:
        break;
      }
      break;
    }

    return QVariant();
  }

  case InvestmentLedgerColumn::PRICE:
    if (_row == 0)
    {
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
      case InvestmentAction::Fee:
        return formatToQVariant(pricePerShare, true);

      default:
        return QVariant();
      }
    }
    else
    {
      return QVariant();
    }

  default:
    return LedgerBuffer::data(_column, _row, _editRole, _controller);
  }
}

bool InvestmentLedgerBuffer::setData(int _column, int _row, const QVariant& _value, LedgerController* _controller)
{
  //This represents the "groups" for idTransfer: if we change in the same group, we do not need
  //to reset idTransfer (same account types), otherwise it is necessary.
  QHash<InvestmentAction, int> cat;
  cat[InvestmentAction::Buy] = 1;
  cat[InvestmentAction::Sell] = 1;
  cat[InvestmentAction::Fee] = 1;
  cat[InvestmentAction::ShortSell] = 1;
  cat[InvestmentAction::ShortCover] = 1;
  cat[InvestmentAction::Transfer] = 2;
  cat[InvestmentAction::Swap] = 3;
  cat[InvestmentAction::Dividend] = 4;
  cat[InvestmentAction::Distribution] = 4;
  cat[InvestmentAction::ReinvestDiv] = 4;
  cat[InvestmentAction::ReinvestDistrib] = 4;

  bool ok = false;

  switch (_column)
  {
  case InvestmentLedgerColumn::ACTION:
    if (_row == 0 && action != (InvestmentAction) _value.toInt())
    {
      int prevRowCount = rowCount();
      InvestmentAction newAction = (InvestmentAction) _value.toInt();

      if (cat.value(newAction) != cat.value(action)) //cat[invalid] will return 0.
      {
        idTransfer = Constants::NO_ID;
        emit _controller->dataChanged(_controller->index(row, InvestmentLedgerColumn::TRANSFER),
                                      _controller->index(row, InvestmentLedgerColumn::TRANSFER));
        emit _controller->requestCheckActions();
      }

      action = newAction;
      rowCountChanged(prevRowCount);
      ok = true;
    }
    break;

  case InvestmentLedgerColumn::TRANSFER:
  {
    AccountCurrency ac = _value.value<AccountCurrency>();

    switch (_row)
    {
    case 0:
      if (action != InvestmentAction::Swap && cat.value(action) != 0)
      {
        idTransfer = ac.first;
        ok = true;
      }
      break;

    case 1:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
        idFeeAccount = ac.first;
        ok = true;
        break;

      case InvestmentAction::Swap:
        idTransfer = ac.first;
        ok = true;
        break;

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        idDivDistToAccount = ac.first;
        ok = true;
        break;

      default:
        break;
      }
      break;

    case 2:
      switch (action)
      {
      //                case InvestmentAction::Buy:
      //                case InvestmentAction::ShortSell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        idTaxAccount = ac.first;
        ok = true;
        break;

      default:
        break;
      }
      break;

    case 3:
      switch (action)
      {
      case InvestmentAction::ShortCover:
      case InvestmentAction::Sell:
        idGainLossAccount = ac.first;
        ok = true;
        break;

      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        idCashInLieuAccount = ac.first;
        ok = true;
        break;

      default:
        break;
      }
      break;
    }

    break;
  }
  case InvestmentLedgerColumn::QUANTITY:
  {
    int curPrec = CurrencyManager::instance()->get(m_security->currency())->precision();
    int curSwap = 2;

    try
    {
      Account* a = Account::getTopLevel()->account(idTransfer);
      if (a)
        curSwap = SecurityManager::instance()->get(a->idSecurity())->precision();
    }
    catch (...) {}

    switch (_row)
    {
    case 0:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ShortSell:
      case InvestmentAction::Transfer:
      case InvestmentAction::Swap:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Fee:
        quantity = Amount::fromUserLocale(_value.toString(), m_security->precision());
        ok = true;
        break;

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        quantity = Amount::fromUserLocale(_value.toString(), curPrec);
        ok = true;
        break;

      case InvestmentAction::StockSplit:
      {
        QStringList frac = _value.toString().split(':');

        if (frac.count() == 2)
        {
          splitFraction.first = frac[0].toInt();
          splitFraction.second = frac[1].toInt();
          ok = true;
        }
        break;
      }
      case InvestmentAction::CostBasisAdjustment:
      case InvestmentAction::UndistributedCapitalGain:
        basisAdjustment = Amount::fromUserLocale(_value.toString());
        ok = true;
        break;

      default:
        break;
      }
      break;

    case 1:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortSell:
        fee = Amount::fromUserLocale(_value.toString(), curPrec);
        ok = true;
        break;

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        quantity = taxPaid + Amount::fromUserLocale(_value.toString(), curPrec);
        ok = true;
        break;

      case InvestmentAction::Swap:
        swapTo = Amount::fromUserLocale(_value.toString(), curSwap);
        ok = true;
        break;

        //                case InvestmentAction::Dividend:
        //                case InvestmentAction::Distribution:
        //                    a = trans->splitFor(InvestmentSplitType::DistributionDest).amount;
        //                    break;

      case InvestmentAction::UndistributedCapitalGain:
        taxPaid = Amount::fromUserLocale(_value.toString(), curPrec);
        ok = true;
        break;

      default:
        break;
      }
      break;

    case 2:
      switch (action)
      {
      //                case InvestmentAction::Buy:
      //                case InvestmentAction::ShortSell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Sell:
      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        taxPaid = Amount::fromUserLocale(_value.toString(), curPrec);
        ok = true;
        break;

      default:
        break;
      }
      break;

    case 3:
      switch (action)
      {
      case InvestmentAction::ShortCover:
      case InvestmentAction::Sell:
        gainLoss = Amount::fromUserLocale(_value.toString(), curPrec);
        ok = true;
        break;

      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        cashInLieu = Amount::fromUserLocale(_value.toString(), curPrec);
        ok = true;
        break;

      default:
        break;
      }
      break;

    }

    break;
  }
  case InvestmentLedgerColumn::PRICE:
    if (_row == 0)
    {
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ShortSell:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Fee:
        pricePerShare = Amount::fromUserLocale(_value.toString(), m_security->precision());
        ok = true;
        break;

      default:
        break;

      }
    }
    break;

  default:
    return LedgerBuffer::setData(_column, _row, _value, _controller);
  }

  if (ok)
  {
    displayNetAmount();
  }

  return ok;
}

void InvestmentLedgerBuffer::load(const Transaction* _transaction, const LedgerController* _controller)
{
  const InvestmentTransaction* tr = qobject_cast<const InvestmentTransaction*>(_transaction);

  if (!tr) return;

  int prevRowCount = rowCount();

  action              = tr->action();
  idTransfer          = tr->idTransferAccount();
  pricePerShare       = tr->pricePerShare();
  lots                = tr->lots();
  distribComposition  = tr->distribComposition();

  if (tr->hasSplitFor(InvestmentSplitType::Fee))
  {
    fee = tr->splitFor(InvestmentSplitType::Fee).amount;
    idFeeAccount = tr->splitFor(InvestmentSplitType::Fee).idAccount;
  }

  if (tr->hasSplitFor(InvestmentSplitType::CashInLieu))
  {
    cashInLieu = tr->splitFor(InvestmentSplitType::CashInLieu).amount;
    idCashInLieuAccount = tr->splitFor(InvestmentSplitType::CashInLieu).idAccount;
  }

  if (tr->hasSplitFor(InvestmentSplitType::DistributionDest))
  {
    idDivDistToAccount = tr->splitFor(InvestmentSplitType::DistributionDest).idAccount;
  }

  if (tr->hasSplitFor(InvestmentSplitType::GainLoss))
  {
    gainLoss = -tr->splitFor(InvestmentSplitType::GainLoss).amount;
    idGainLossAccount = tr->splitFor(InvestmentSplitType::GainLoss).idAccount;
  }

  if (tr->hasSplitFor(InvestmentSplitType::Tax))
  {
    taxPaid = tr->splitFor(InvestmentSplitType::Tax).amount;
    idTaxAccount = tr->splitFor(InvestmentSplitType::Tax).idAccount;
  }

  if (tr->action() == InvestmentAction::Transfer
      || tr->action() == InvestmentAction::Swap)
  {
    if (tr->splitFor(InvestmentSplitType::InvestmentFrom).idAccount == _controller->account()->id())
    {
      idTransfer = tr->splitFor(InvestmentSplitType::InvestmentTo).idAccount;
    }
    else
    {
      idTransfer = tr->splitFor(InvestmentSplitType::InvestmentFrom).idAccount;
    }
  }

  //Quantity
  switch (action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::ShortCover:
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::ReinvestDistrib:
    quantity = tr->splitFor(InvestmentSplitType::Investment).amount;
    break;

  case InvestmentAction::Sell:
  case InvestmentAction::ShortSell:
  case InvestmentAction::Fee:
    quantity = -tr->splitFor(InvestmentSplitType::Investment).amount;
    break;

  case InvestmentAction::Distribution:
  case InvestmentAction::Dividend:
    quantity = -tr->splitFor(InvestmentSplitType::DistributionSource).amount;
    break;

  case InvestmentAction::StockSplit:
    splitFraction = tr->splitFraction();
    break;

  case InvestmentAction::Swap:
    if (tr->splitFor(InvestmentSplitType::InvestmentFrom).idAccount == _controller->account()->id())
    {
      swapTo = tr->splitFor(InvestmentSplitType::InvestmentTo).amount;
      quantity = tr->splitFor(InvestmentSplitType::InvestmentFrom).amount;
    }
    else
    {
      swapTo = tr->splitFor(InvestmentSplitType::InvestmentFrom).amount;
      quantity = tr->splitFor(InvestmentSplitType::InvestmentTo).amount;
    }
    break;

  case InvestmentAction::Transfer:
    if (tr->splitFor(InvestmentSplitType::InvestmentFrom).idAccount == _controller->account()->id())
    {
      quantity = tr->splitFor(InvestmentSplitType::InvestmentFrom).amount;
    }
    else
    {
      quantity = tr->splitFor(InvestmentSplitType::InvestmentTo).amount;
    }
    break;

  case InvestmentAction::UndistributedCapitalGain:
    taxPaid = tr->taxPaid();

  case InvestmentAction::CostBasisAdjustment: //Yes pass through.
    basisAdjustment = tr->basisAdjustment();
    break;

  default:
    break;
  }

  // Now load the normal stuff (no, date, attachments, etc.) Won't load splits anymore since our
  // specified version of loadSplits() is empty.
  LedgerBuffer::load(_transaction, _controller);

  transferCurrency = m_security->currency();
  rowCountChanged(prevRowCount);

  emit _controller->requestCheckActions();
}

void InvestmentLedgerBuffer::loadGainLossData(GainLossData& _data) const
{
  _data.action            = action;
  _data.fee               = fee;
  _data.pricePerShare     = pricePerShare;
  _data.numShares         = quantity;
  _data.transactionDate   = date;
  _data.lots              = lots;
}

int InvestmentLedgerBuffer::rowCount() const
{
  return InvestmentLedgerController::rowCountForAction(action);
}

QStringList InvestmentLedgerBuffer::validate(int& _firstErrorColumn, const LedgerController* _controller)
{
  QStringList errors;
  const QString invCur = m_security->currency();
  _firstErrorColumn = 0;

  //---------------------- Helper Functions ----------------------

  auto checkAccount = [] (int _idAccount) {

    if (_idAccount == Constants::NO_ID)
    {
      return false;
    }
    else
    {
      Account* a = Account::getTopLevel()->account(_idAccount);

      if (!a || a->isPlaceholder())
      {
        return false;
      }
    }

    return true;
  };

  auto checkAccountCurrency = [&invCur] (int _idAccount) {

    Account* a = Account::getTopLevel()->account(_idAccount);

    return a->allCurrencies().contains(invCur);
  };

  auto addError = [&_firstErrorColumn, &errors] (const QString& _error, int _column) {

    errors << _error;
    if (_firstErrorColumn == -1)
      _firstErrorColumn = _column;
  };


  //---------------------- Validation Check ----------------------

  //Date
  if (!date.isValid())
  {
    errors << QObject::tr("The date is invalid.");
    _firstErrorColumn = _controller->col_date();
  }

  //Investment stuff (accounts+amounts)
  switch (action)
  {
  case InvestmentAction::Sell:
  case InvestmentAction::ShortCover:

    if (idGainLossAccount != Constants::NO_ID || gainLoss != 0)
    {
      if (!checkAccount(idGainLossAccount))
      {
        addError(tr("The gain/loss account is invalid."),
                 InvestmentLedgerColumn::TRANSFER);
      }
      else if (!checkAccountCurrency(idGainLossAccount))
      {
        addError(tr("The gain/loss account's currency must be the same as the security's currency (%1).")
                 .arg(invCur),
                 InvestmentLedgerColumn::TRANSFER);
      }

      if (gainLoss == 0)
      {
        addError(tr("The gain/loss must be greater than zero."),
                 InvestmentLedgerColumn::QUANTITY);
      }
    }

  case InvestmentAction::Buy: //YES pass through.
  case InvestmentAction::ShortSell:
  case InvestmentAction::ReinvestDistrib:
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::Fee:

    if (!checkAccount(idTransfer))
    {
      addError(tr("The cost/proceeds account is invalid."),
               InvestmentLedgerColumn::TRANSFER);
    }
    else if (!checkAccountCurrency(idTransfer))
    {
      addError(tr("The cost/proceeds account's currency must be the same as the security's currency (%1).")
               .arg(invCur),
               InvestmentLedgerColumn::TRANSFER);
    }

    if (pricePerShare < 0)
    {
      addError(tr("The price per share must be at least zero.")
               .arg(invCur),
               InvestmentLedgerColumn::TRANSFER);
    }

    if (idFeeAccount != Constants::NO_ID || fee != 0)
    {
      if (!checkAccount(idFeeAccount))
      {
        addError(tr("The fee account is invalid."),
                 InvestmentLedgerColumn::TRANSFER);
      }
      else if (!checkAccountCurrency(idFeeAccount))
      {
        addError(tr("The fee account's currency must be the same as the security's currency (%1).")
                 .arg(invCur),
                 InvestmentLedgerColumn::TRANSFER);
      }

      if (fee <= 0)
      {
        addError(tr("The fee must be greater than zero."),
                 InvestmentLedgerColumn::QUANTITY);
      }
    }

    if (idTaxAccount != Constants::NO_ID || taxPaid != 0)
    {
      if (!checkAccount(idTaxAccount))
      {
        addError(tr("The tax account is invalid."),
                 InvestmentLedgerColumn::TRANSFER);
      }
      else if (!checkAccountCurrency(idTaxAccount))
      {
        addError(tr("The tax account's currency must be the same as the security's currency (%1).")
                 .arg(invCur),
                 InvestmentLedgerColumn::TRANSFER);
      }

      if (taxPaid <= 0)
      {
        addError(tr("The tax must be greater than zero."),
                 InvestmentLedgerColumn::QUANTITY);
      }
    }

    if (action == InvestmentAction::ReinvestDistrib ||
        action == InvestmentAction::ReinvestDiv)
    {
      if (idCashInLieuAccount != Constants::NO_ID || cashInLieu != 0)
      {
        if (!checkAccount(idCashInLieuAccount))
        {
          addError(tr("The cash in lieu account is invalid."),
                   InvestmentLedgerColumn::TRANSFER);
        }
        else if (!checkAccountCurrency(idCashInLieuAccount))
        {
          addError(tr("The cash in lieu account's currency must be the same as the security's currency (%1).")
                   .arg(invCur),
                   InvestmentLedgerColumn::TRANSFER);
        }

        if (cashInLieu <= 0)
        {
          addError(tr("The cash in lieu must be greater than zero."),
                   InvestmentLedgerColumn::QUANTITY);
        }
      }
    }

    break;

  case InvestmentAction::Transfer:
    if (!checkAccount(idTransfer))
    {
      addError(tr("The transfer account is invalid."),
               InvestmentLedgerColumn::TRANSFER);
    }
    else
    {
      Account* a = Account::getTopLevel()->account(idTransfer);
      if (a->idSecurity() != _controller->account()->idSecurity())
      {
        addError(tr("The transfer account and this account must "
                    "have the same security. Use a Swap otherwise."),
                 InvestmentLedgerColumn::TRANSFER);
      }
    }

    if (quantity == 0)
    {
      addError(tr("The transfer amount must not be zero."),
               InvestmentLedgerColumn::QUANTITY);
    }

    break;

  case InvestmentAction::Swap:
    if (!checkAccount(idTransfer))
    {
      addError(tr("The swap account is invalid."),
               InvestmentLedgerColumn::TRANSFER);
    }
    else
    {
      Account* a = Account::getTopLevel()->account(idTransfer);
      if (a->idSecurity() == _controller->account()->idSecurity())
      {
        addError(tr("The transfer account and this account must "
                    "not have the same security. Use a Transfer otherwise."),
                 InvestmentLedgerColumn::TRANSFER);
      }
      else if (a->idSecurity() == Constants::NO_ID
               || a->type() != AccountType::INVESTMENT)
      {
        addError(tr("The swap account must be a valid investment account."),
                 InvestmentLedgerColumn::TRANSFER);
      }
    }

    if (quantity == 0)
    {
      addError(tr("The swap from amount must not be zero."),
               InvestmentLedgerColumn::QUANTITY);
    }

    if (swapTo == 0)
    {
      addError(tr("The swap to amount must not be zero."),
               InvestmentLedgerColumn::QUANTITY);
    }

    if ((quantity > 0 && swapTo > 0)
        || (quantity < 0 && swapTo < 0))
    {
      addError(tr("Swap from and swap to amounts must have different signs."),
               InvestmentLedgerColumn::QUANTITY);
    }

    break;

  case InvestmentAction::StockSplit:
    if (splitFraction.first <= 0
        || splitFraction.second <= 0)
    {
      addError(tr("The split fraction is invalid. Both parts of the fraction must be greater than zero."),
               InvestmentLedgerColumn::QUANTITY);
    }
    break;

  case InvestmentAction::Dividend:
  case InvestmentAction::Distribution:
    if (quantity <= 0)
    {
      addError(tr("The dividend/distribution amount must be greater than zero."),
               InvestmentLedgerColumn::QUANTITY);
    }

    if (!checkAccount(idTransfer))
    {
      addError(tr("The source account is invalid."),
               InvestmentLedgerColumn::TRANSFER);
    }
    else if (!checkAccountCurrency(idTransfer))
    {
      addError(tr("The source account's currency must be the same as the security's currency (%1).")
               .arg(invCur),
               InvestmentLedgerColumn::TRANSFER);
    }

    if (!checkAccount(idDivDistToAccount))
    {
      addError(tr("The target account is invalid."),
               InvestmentLedgerColumn::TRANSFER);
    }
    else if (!checkAccountCurrency(idDivDistToAccount))
    {
      addError(tr("The target account's currency must be the same as the security's currency (%1).")
               .arg(invCur),
               InvestmentLedgerColumn::TRANSFER);
    }

    if (idTaxAccount != Constants::NO_ID || taxPaid != 0)
    {
      if (!checkAccount(idTaxAccount))
      {
        addError(tr("The tax account is invalid."),
                 InvestmentLedgerColumn::TRANSFER);
      }
      else if (!checkAccountCurrency(idTaxAccount))
      {
        addError(tr("The tax account's currency must be the same as the security's currency (%1).")
                 .arg(invCur),
                 InvestmentLedgerColumn::TRANSFER);
      }

      if (taxPaid <= 0)
      {
        addError(tr("The tax must be greater than zero."),
                 InvestmentLedgerColumn::QUANTITY);
      }
    }

    if (taxPaid > quantity)
    {
      addError(tr("The tax must be smaller than the dividend/distribution amount."),
               InvestmentLedgerColumn::QUANTITY);
    }

    break;

  case InvestmentAction::CostBasisAdjustment:
    if (basisAdjustment == 0)
    {
      addError(tr("The basis adjustment amount is invalid."),
               InvestmentLedgerColumn::QUANTITY);
    }
    break;

  case InvestmentAction::UndistributedCapitalGain:
    if (basisAdjustment <= 0)
    {
      addError(tr("The capital gain amount is invalid."),
               InvestmentLedgerColumn::QUANTITY);
    }

    if (taxPaid < 0)
    {
      addError(tr("The tax paid amount is invalid."),
               InvestmentLedgerColumn::QUANTITY);
    }
    break;

  default:
    addError(tr("The transaction type is invalid."),
             InvestmentLedgerColumn::ACTION);
  }

  //Need to add all other cases, check if balance, check account types, etc.

  return errors;
}

bool InvestmentLedgerBuffer::saveToTransaction(Transaction* _transaction, LedgerController* _controller)
{
  InvestmentTransaction* inv_transaction = qobject_cast<InvestmentTransaction*>(_transaction);

  if (!inv_transaction)
  {
    return false;
  }

  const QString invCur = m_security->currency();
  QList<Transaction::Split> splits;
  QList<InvestmentSplitType> types;
  Amount net;

  auto addTradingSplits = [&splits, &types] () {
    Transaction::addTradingSplits(splits);
    while (types.count() < splits.count())
      types << InvestmentSplitType::Trading;
  };

  //Add basic stuff
  inv_transaction->setDate(date);
  inv_transaction->setNo(no);
  inv_transaction->setMemo(memo);
  inv_transaction->setNote(note);
  inv_transaction->setFlagged(flagged);
  inv_transaction->setClearedStatus(clearedStatus);
  inv_transaction->setAttachments(attachments);

  switch (action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::ShortCover:

    //Compute net amount
    net = quantity*pricePerShare
          + fee
          + taxPaid;

    splits << Transaction::Split(-net, idTransfer, invCur);
    types << InvestmentSplitType::CostProceeds;

    splits << Transaction::Split(quantity, _controller->account()->id(), "");
    types << InvestmentSplitType::Investment;

    if (fee > 0)
    {
      splits << Transaction::Split(fee, idFeeAccount, invCur);
      types << InvestmentSplitType::Fee;
    }

    if (taxPaid > 0)
    {
      splits << Transaction::Split(taxPaid, idTaxAccount, invCur);
      types << InvestmentSplitType::Tax;
    }

    if (action == InvestmentAction::ShortCover && gainLoss != 0)
    {
      splits << Transaction::Split(-gainLoss, idGainLossAccount, invCur);
      types << InvestmentSplitType::GainLoss;
    }

    addTradingSplits();
    inv_transaction->makeBuySellFee(action, pricePerShare, splits, types, lots);
    break;

  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::ReinvestDistrib:

    //Compute net amount
    net = quantity*pricePerShare
          + fee
          + taxPaid;

    //This "if" must be first since it may modify the net amount
    if (cashInLieu > 0)
    {
      splits << Transaction::Split(cashInLieu, idCashInLieuAccount, invCur);
      types << InvestmentSplitType::CashInLieu;
      net += cashInLieu;
    }

    splits << Transaction::Split(-net, idTransfer, invCur);
    types << InvestmentSplitType::DistributionSource;

    splits << Transaction::Split(quantity, _controller->account()->id(), "");
    types << InvestmentSplitType::Investment;

    if (fee > 0)
    {
      splits << Transaction::Split(fee, idFeeAccount, invCur);
      types << InvestmentSplitType::Fee;
    }

    if (taxPaid > 0)
    {
      splits << Transaction::Split(taxPaid, idTaxAccount, invCur);
      types << InvestmentSplitType::Tax;
    }

    if (action == InvestmentAction::ShortCover && gainLoss != 0)
    {
      splits << Transaction::Split(-gainLoss, idGainLossAccount, invCur);
      types << InvestmentSplitType::GainLoss;
    }

    addTradingSplits();
    inv_transaction->makeReinvestedDivDist(action, pricePerShare, splits, types, distribComposition);
    break;

  case InvestmentAction::Sell:
  case InvestmentAction::ShortSell:
    //Compute net amount
    net = quantity*pricePerShare
          - fee
          - taxPaid;

    splits << Transaction::Split(net, idTransfer, invCur);
    types << InvestmentSplitType::CostProceeds;

    splits << Transaction::Split(-quantity, _controller->account()->id(), "");
    types << InvestmentSplitType::Investment;

    if (fee > 0)
    {
      splits << Transaction::Split(fee, idFeeAccount, invCur);
      types << InvestmentSplitType::Fee;
    }

    if (taxPaid > 0)
    {
      splits << Transaction::Split(taxPaid, idTaxAccount, invCur);
      types << InvestmentSplitType::Tax;
    }

    if (action == InvestmentAction::Sell && gainLoss != 0)
    {
      splits << Transaction::Split(-gainLoss, idGainLossAccount, invCur);
      types << InvestmentSplitType::GainLoss;
    }

    addTradingSplits();
    inv_transaction->makeBuySellFee(action, pricePerShare, splits, types, lots);
    break;

  case InvestmentAction::Fee:
    //Compute net amount
    net = quantity*pricePerShare;

    splits << Transaction::Split(net, idTransfer, invCur);
    types << InvestmentSplitType::CostProceeds;

    splits << Transaction::Split(-quantity, _controller->account()->id(), "");
    types << InvestmentSplitType::Investment;

    addTradingSplits();
    inv_transaction->makeBuySellFee(action, pricePerShare, splits, types, lots);
    break;

  case InvestmentAction::Distribution:
  case InvestmentAction::Dividend:

    splits << Transaction::Split(-quantity, idTransfer, invCur);
    types << InvestmentSplitType::DistributionSource;

    splits << Transaction::Split(quantity-taxPaid, idDivDistToAccount, invCur);
    types << InvestmentSplitType::DistributionDest;

    if (taxPaid > 0)
    {
      splits << Transaction::Split(taxPaid, idTaxAccount, invCur);
      types << InvestmentSplitType::Tax;
    }

    inv_transaction->makeDivDist(action, _controller->account()->id(), splits, types, distribComposition);
    break;

  case InvestmentAction::Transfer:
  case InvestmentAction::Swap:

    if (quantity > 0)
    {
      splits << Transaction::Split(-quantity, _controller->account()->id(), "");
      types << InvestmentSplitType::InvestmentFrom;

      if (action == InvestmentAction::Swap)
      {
        splits << Transaction::Split(swapTo, idTransfer, "");
        types << InvestmentSplitType::InvestmentTo;

        addTradingSplits();
      }
      else
      {
        splits << Transaction::Split(quantity, idTransfer, "");
        types << InvestmentSplitType::InvestmentTo;
      }
    }
    else
    {
      splits << Transaction::Split(quantity, _controller->account()->id(), "");
      types << InvestmentSplitType::InvestmentTo;


      if (action == InvestmentAction::Swap)
      {
        splits << Transaction::Split(swapTo, idTransfer, "");
        types << InvestmentSplitType::InvestmentFrom;

        addTradingSplits();
      }
      else
      {
        splits << Transaction::Split(-quantity, idTransfer, "");
        types << InvestmentSplitType::InvestmentFrom;
      }
    }

    inv_transaction->makeTransferSwap(action, splits, types);
    break;


  case InvestmentAction::StockSplit:
    inv_transaction->makeSplit(_controller->account()->id(), splitFraction);
    break;

  case InvestmentAction::UndistributedCapitalGain:
    inv_transaction->makeUndistributedCapitalGain(_controller->account()->id(), basisAdjustment, taxPaid);
    break;

  case InvestmentAction::CostBasisAdjustment:
    inv_transaction->makeCostBasisAdjustment(_controller->account()->id(), basisAdjustment);
    break;

  default:
    return false;

  }

  return true;
}

//--------------------------------------------- DELEGATE ---------------------------------------------

QWidget* InvestmentLedgerWidgetDelegate::createEditor(QWidget* _parent,
                                                      const QStyleOptionViewItem& _option,
                                                      const QModelIndex& _index) const
{

  Security* cur_sec = static_cast<const InvestmentLedgerController*>(_index.model())->security();
  const Account* cur_account = static_cast<const LedgerController*>(_index.model())->account();

  const int row = _index.parent().isValid() ? _index.row()+1 : 0;
  const int mainRow = _index.parent().isValid() ? _index.parent().row() : _index.row();

  const InvestmentAction action = (InvestmentAction) _index.model()
                                  ->data(_index.model()->index(mainRow, InvestmentLedgerColumn::ACTION),
                                         Qt::EditRole).toInt();

  switch (_index.column())
  {
  case InvestmentLedgerColumn::ACTION:
  {
    QComboBox* cboType = new QComboBox(_parent);

    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::Buy),
                     (int) InvestmentAction::Buy);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::Sell),
                     (int) InvestmentAction::Sell);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::ShortSell),
                     (int) InvestmentAction::ShortSell);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::ShortCover),
                     (int) InvestmentAction::ShortCover);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::Transfer),
                     (int) InvestmentAction::Transfer);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::Swap),
                     (int) InvestmentAction::Swap);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::StockSplit),
                     (int) InvestmentAction::StockSplit);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::Dividend),
                     (int) InvestmentAction::Dividend);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::Distribution),
                     (int) InvestmentAction::Distribution);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::ReinvestDiv),
                     (int) InvestmentAction::ReinvestDiv);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::ReinvestDistrib),
                     (int) InvestmentAction::ReinvestDistrib);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::UndistributedCapitalGain),
                     (int) InvestmentAction::UndistributedCapitalGain);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::CostBasisAdjustment),
                     (int) InvestmentAction::CostBasisAdjustment);
    cboType->addItem(InvestmentTransaction::actionToString(InvestmentAction::Fee),
                     (int) InvestmentAction::Fee);

    cboType->insertSeparator(2);
    cboType->insertSeparator(5);
    cboType->insertSeparator(9);
    cboType->insertSeparator(14);

    setCurrentEditor(cboType);
    break;
  }
  case InvestmentLedgerColumn::TRANSFER:
  {
    switch (row)
    {
    case 0:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ShortSell:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_AllButInvTrad,
                                             cur_sec->currency(),
                                             Constants::NO_ID,
                                             _parent));
        break;

      case InvestmentAction::Fee:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_IncomeExpense,
                                             cur_sec->currency(),
                                             Constants::NO_ID,
                                             _parent));
        break;

      case InvestmentAction::Transfer:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             cur_sec,
                                             cur_account->id(),
                                             _parent));
        break;

      case InvestmentAction::Dividend:
      case InvestmentAction::StockDividend:
      case InvestmentAction::Distribution:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_Income,
                                             cur_sec->currency(),
                                             Constants::NO_ID,
                                             _parent));
        break;

      default:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_None,
                                             Constants::NO_ID,
                                             _parent));
        break;
      }
      break;

    case 1:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ShortSell:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_IncomeExpense,
                                             cur_sec->currency(),
                                             Constants::NO_ID,
                                             _parent));
        break;

      case InvestmentAction::Swap:
        setCurrentEditor(new AccountSelector(Flag_None,
                                             AccountTypeFlags::Flag_Investment,
                                             cur_account->id(),
                                             _parent));
        break;

      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_AllButInvTrad,
                                             cur_sec->currency(),
                                             Constants::NO_ID,
                                             _parent));
        break;

      default:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_None,
                                             Constants::NO_ID,
                                             _parent));
        break;
      }
      break;

    case 2:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ShortSell:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::Dividend:
      case InvestmentAction::Distribution:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_IncomeExpense,
                                             cur_sec->currency(),
                                             Constants::NO_ID,
                                             _parent));
        break;

      default:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_None,
                                             Constants::NO_ID,
                                             _parent));
        break;
      }
      break;

    case 3:
      switch (action)
      {
      case InvestmentAction::Buy:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ShortSell:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_IncomeExpense,
                                             cur_sec->currency(),
                                             Constants::NO_ID,
                                             _parent));
        break;

      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::ReinvestDistrib:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_AllButInvTrad,
                                             cur_sec->currency(),
                                             Constants::NO_ID,
                                             _parent));
        break;

      default:
        setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                             AccountTypeFlags::Flag_None,
                                             Constants::NO_ID,
                                             _parent));
        break;
      }
      break;

    default:
      setCurrentEditor(new AccountSelector(Flag_MultipleCurrencies,
                                           AccountTypeFlags::Flag_None,
                                           Constants::NO_ID,
                                           _parent));
      break;

    } //End switch row
    break;
  }
  case InvestmentLedgerColumn::QUANTITY:
  {
    if (row == 0 && action == InvestmentAction::StockSplit)
    {
      SplitFractionWidget* w = new SplitFractionWidget(_parent);
      setCurrentEditor(w);
    }
    else
    {
      int prec = CurrencyManager::instance()->get(cur_sec->currency())->precision();

      switch (action)
      {
      case InvestmentAction::Swap:
        if (row == 1)
        {
          prec = cur_sec->precision();
        }
        //No break

      case InvestmentAction::Buy:
      case InvestmentAction::Sell:
      case InvestmentAction::ShortCover:
      case InvestmentAction::ShortSell:
      case InvestmentAction::Transfer:
      case InvestmentAction::ReinvestDistrib:
      case InvestmentAction::ReinvestDiv:
      case InvestmentAction::Fee:
        if (row == 0)
        {
          prec = cur_sec->precision();
        }
        break;

      default:
        break;
      }

      AmountEdit* amtEdit = new AmountEdit(prec, _parent);
      bool allowNeg = false;

      switch (action)
      {
      case InvestmentAction::Transfer:
      case InvestmentAction::Swap:
        allowNeg = true;
        break;

      case InvestmentAction::Sell:
      case InvestmentAction::ShortCover:
        if (row == 3)
        {
          allowNeg = true;
        }
        break;
      case InvestmentAction::CostBasisAdjustment:
        if (row == 0)
        {
          allowNeg = true;
        }
        break;

      default:
        break;
      }


      if (allowNeg)
      {
        amtEdit->setMinimum(-INFINITY);
      }
      else
      {
        amtEdit->setMinimum(0);
      }

      amtEdit->setFocus();
      setCurrentEditor(amtEdit);
    }
    break;
  }
  case InvestmentLedgerColumn::PRICE:
  {
    AmountEdit* amtEdit = new AmountEdit(cur_sec->precision(), _parent);
    amtEdit->setMinimum(0);
    amtEdit->setFocus();
    setCurrentEditor(amtEdit);
    break;
  }
  default:
    return LedgerWidgetDelegate::createEditor(_parent, _option, _index);
  }

  return currentEditor();
}

void InvestmentLedgerWidgetDelegate::setEditorData(QWidget* _editor, const QModelIndex& _index) const
{
  const int row = _index.parent().isValid() ? _index.row()+1 : 0;
  const int mainRow = _index.parent().isValid() ? _index.parent().row() : _index.row();
  const InvestmentAction action = (InvestmentAction) _index.model()
                                  ->data(_index.model()->index(mainRow, InvestmentLedgerColumn::ACTION),
                                         Qt::EditRole).toInt();

  switch (_index.column())
  {
  case InvestmentLedgerColumn::ACTION:
  {
    QComboBox* cboType = static_cast<QComboBox*>(_editor);
    cboType->setCurrentIndex(cboType->findData(_index.model()->data(_index, Qt::EditRole).toInt()));
    if (cboType->currentIndex() == -1)
      cboType->setCurrentIndex(0);
    break;
  }
  case InvestmentLedgerColumn::QUANTITY:
    if (row == 0 && action == InvestmentAction::StockSplit)
    {
      SplitFractionWidget* w = static_cast<SplitFractionWidget*>(_editor);
      QStringList l = _index.model()->data(_index, Qt::EditRole).toString().split(":",
                                                                                  QString::SkipEmptyParts);
      if (l.size() == 2)
      {
        w->setSplitFraction(QPair<int, int>(l[0].toInt(), l[1].toInt()));
      }
      break;
    } //Otherwise, don't break and fall through
  case InvestmentLedgerColumn::PRICE:
  {
    AmountEdit* amtEdit = static_cast<AmountEdit*>(_editor);
    amtEdit->setAmount(Amount::fromUserLocale(_index.model()->data(_index, Qt::EditRole).toString(),
                                              amtEdit->precision()));
    break;
  }
  default:
    LedgerWidgetDelegate::setEditorData(_editor, _index);
  }
}

void InvestmentLedgerWidgetDelegate::setModelData(QWidget* _editor, QAbstractItemModel* _model, const QModelIndex& _index) const
{
  const int row = _index.parent().isValid() ? _index.row()+1 : 0;
  const int mainRow = _index.parent().isValid() ? _index.parent().row() : _index.row();
  const InvestmentAction action = (InvestmentAction) _index.model()
                                  ->data(_index.model()->index(mainRow, InvestmentLedgerColumn::ACTION),
                                         Qt::EditRole).toInt();

  switch (_index.column())
  {
  case InvestmentLedgerColumn::ACTION:
  {
    QComboBox* cboType = static_cast<QComboBox*>(_editor);
    _model->setData(_index, cboType->currentData(), Qt::EditRole);
    break;
  }
  case InvestmentLedgerColumn::QUANTITY:
    if (row == 0 && action == InvestmentAction::StockSplit)
    {
      SplitFractionWidget* w = static_cast<SplitFractionWidget*>(_editor);
      _model->setData(_index,
                      QString("%1:%2")
                      .arg(w->splitFraction().first)
                      .arg(w->splitFraction().second),
                      Qt::EditRole);
      break;
    } //Otherwise, don't break and fall through
  case InvestmentLedgerColumn::PRICE:
  {
    AmountEdit* amtEdit = static_cast<AmountEdit*>(_editor);
    _model->setData(_index, amtEdit->amount().toString(), Qt::EditRole);
    break;
  }
  default:
    LedgerWidgetDelegate::setModelData(_editor, _model, _index);
  }
}

void InvestmentLedgerWidgetDelegate::setColumnWidth(LedgerWidget* _view) const
{
  LedgerWidgetDelegate::setColumnWidth(_view);
  _view->setColumnWidth(InvestmentLedgerColumn::ACTION, 100);
}



}
