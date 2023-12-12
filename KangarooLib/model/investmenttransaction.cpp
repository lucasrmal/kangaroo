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

#include "investmenttransaction.h"
#include "modelexception.h"
#include "account.h"
#include "security.h"
#include "ledger.h"
#include "../controller/io.h"
#include "investmentlotsmanager.h"
#include <QXmlStreamReader>

namespace KLib
{
QString InvestmentTransaction::actionToString(InvestmentAction _action)
{
  switch (_action)
  {
  case InvestmentAction::Buy:
    return tr("Buy");

  case InvestmentAction::Sell:
    return tr("Sell");

  case InvestmentAction::ShortSell:
    return tr("Short Sell");

  case InvestmentAction::ShortCover:
    return tr("Short Cover");

  case InvestmentAction::Transfer:
    return tr("Transfer");

  case InvestmentAction::Swap:
    return tr("Swap");

  case InvestmentAction::Spinoff:
    return tr("Spinoff");

  case InvestmentAction::StockSplit:
    return tr("Split");

  case InvestmentAction::StockDividend:
    return tr("Stock Dividend");

  case InvestmentAction::Dividend:
    return tr("Dividend");

  case InvestmentAction::Distribution:
    return tr("Distribution");

  case InvestmentAction::ReinvestDiv:
    return tr("Reinvest Dividend");

  case InvestmentAction::ReinvestDistrib:
    return tr("Reinvest Distribution");

  case InvestmentAction::UndistributedCapitalGain:
    return tr("Undistributed Capital Gain");

  case InvestmentAction::CostBasisAdjustment:
    return tr("Cost Basis Adjustment");

  case InvestmentAction::Fee:
    return tr("Fee");

  case InvestmentAction::Invalid:
    return tr("Invalid");

  }

  return QString();
}

QString InvestmentTransaction::distribTypeToString(DistribType _type)
{
  switch (_type)
  {
  case DistribType::ReturnOfCapital:
    return tr("Return of Capital");

  case DistribType::CapitalGain:
    return tr("Capital Gain");

  case DistribType::Other:
    return tr("Other");
  }

  return QString();
}

Amount InvestmentTransaction::balanceAfterSplit(const Amount& _balanceBefore, const SplitFraction& _fraction)
{
  return _fraction.first == _fraction.second ? _balanceBefore
                                             : (_balanceBefore * _fraction.first) / _fraction.second;
}

InvestmentTransaction::InvestmentTransaction() :
  m_action(InvestmentAction::Invalid)
{
}

Transaction* InvestmentTransaction::copyTo(int _idTo) const
{
  InvestmentTransaction* tr = new InvestmentTransaction();
  tr->m_id = _idTo;
  copyTo(tr);
  return tr;
}

void InvestmentTransaction::copyTo(Transaction* _other) const
{
  Transaction::copyTo(_other);
  InvestmentTransaction* inv_tr = qobject_cast<InvestmentTransaction*>(_other);

  if (inv_tr)
  {
    inv_tr->m_action             = m_action;
    inv_tr->m_pricePerShare      = m_pricePerShare;
    inv_tr->m_types              = m_types;
    inv_tr->m_distribComposition = m_distribComposition;
    inv_tr->m_splitFraction      = m_splitFraction;
    inv_tr->m_basisAdjustment    = m_basisAdjustment;
    inv_tr->m_taxPaid            = m_taxPaid;
  }
}

void InvestmentTransaction::setSplits(const QList<KLib::Transaction::Split>& _splits)
{
  Q_UNUSED(_splits)
  ModelException::throwException(tr("Cannot call setSplits() on an InvestmentTransaction!"), this);
}

void InvestmentTransaction::setDate(const QDate& _date)
{
  if (_date != date())
  {
    Transaction::setDate(_date);
    InvestmentLotsManager::instance()->updateDate(this);
  }
}

const Transaction::Split& InvestmentTransaction::splitFor(InvestmentSplitType _type) const
{
  if (!m_types.contains(_type))
  {
    ModelException::throwException(tr("No such split!"), this);
  }

  return m_splits[m_types[_type]];
}

bool InvestmentTransaction::hasSplitFor(InvestmentSplitType _type) const
{
  return m_types.contains(_type);
}

//    Amount InvestmentTransaction::balanceAfterSplit(const Amount& balanceBefore) const
//    {
//        if (m_action == InvestmentAction::Split && _account == idInvestmentAccount())
//        {
//            return (balanceBefore * m_splitFraction.first) / m_splitFraction.second;
//        }
//        else
//        {
//            return 0;
//        }
//    }

//    Balances InvestmentTransaction::totalFor(int _account) const
//    {
//        //Everything is the same except for Split transactions
//        if (m_action == InvestmentAction::Split && _account == idInvestmentAccount())
//        {
//            return (Account::getTopLevel()->account(idInvestmentAccount())
//                    ->ledger()->balanceBefore(this)
//                    * m_splitFraction.first) / m_splitFraction.second;
//        }

//    }

int InvestmentTransaction::idInvestmentAccount() const
{
  switch (m_action)
  {
  case InvestmentAction::Invalid:
    ModelException::throwException(tr("The transaction is invalid!"), this);
    return Constants::NO_ID;

  case InvestmentAction::Transfer:
  case InvestmentAction::Swap:
    return splitFor(InvestmentSplitType::InvestmentFrom).idAccount;

  default:
    return splitFor(InvestmentSplitType::Investment).idAccount;
  }
}

int InvestmentTransaction::idTransferAccount() const
{
  switch (m_action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::Sell:
  case InvestmentAction::ShortCover:
  case InvestmentAction::ShortSell:
  case InvestmentAction::Fee:
    return splitFor(InvestmentSplitType::CostProceeds).idAccount;

  case InvestmentAction::Transfer:
  case InvestmentAction::Swap:
  case InvestmentAction::Spinoff:
    return splitFor(InvestmentSplitType::InvestmentTo).idAccount;

  case InvestmentAction::Dividend:
  case InvestmentAction::Distribution:
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::ReinvestDistrib:
    return splitFor(InvestmentSplitType::DistributionSource).idAccount;

  default:
    return Constants::NO_ID;
  }
}

Amount InvestmentTransaction::shareCount() const
{
  switch (m_action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::Sell:
  case InvestmentAction::ShortSell:
  case InvestmentAction::ShortCover:
  case InvestmentAction::StockDividend:
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::ReinvestDistrib:
  case InvestmentAction::Fee:
    return splitFor(InvestmentSplitType::Investment).amount;

  case InvestmentAction::Transfer:
  case InvestmentAction::Swap:
    return splitFor(InvestmentSplitType::InvestmentFrom).amount;
  case InvestmentAction::Spinoff:
    return splitFor(InvestmentSplitType::InvestmentTo).amount;

  default:
    return 0;
  }
}

Amount InvestmentTransaction::netPricePerShare() const
{
  if (!hasSplitFor(InvestmentSplitType::Fee))
    return m_pricePerShare;

  switch (m_action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::ShortCover:
  case InvestmentAction::StockDividend:
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::ReinvestDistrib:
    return m_pricePerShare + m_splits[m_types[InvestmentSplitType::Fee]].amount
        / splitFor(InvestmentSplitType::Investment).amount;

  case InvestmentAction::Sell:
  case InvestmentAction::ShortSell:
  case InvestmentAction::Fee:
    return m_pricePerShare - m_splits[m_types[InvestmentSplitType::Fee]].amount
        / splitFor(InvestmentSplitType::Investment).amount;

  default:
    return 0;
  }
}

Amount InvestmentTransaction::taxPaid() const
{
  switch (m_action)
  {
  case InvestmentAction::Sell:
  case InvestmentAction::ShortSell:
  case InvestmentAction::ShortCover:
  case InvestmentAction::StockDividend:
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::ReinvestDistrib:
    if (hasSplitFor(InvestmentSplitType::Tax))
    {
      return m_splits[m_types[InvestmentSplitType::Tax]].amount;
    }
    break;

  default:
    break;
  }

  return m_taxPaid;
}

Amount InvestmentTransaction::fee() const
{
  return hasSplitFor(InvestmentSplitType::Fee) ? m_splits[m_types[InvestmentSplitType::Fee]].amount
      : 0;
}

Amount InvestmentTransaction::gainLoss() const
{
  return hasSplitFor(InvestmentSplitType::GainLoss) ? m_splits[m_types[InvestmentSplitType::GainLoss]].amount
      : 0;
}

//    Lots InvestmentTransaction::lots() const
//    {
//        return InvestmentLotsManager::instance()->lotsForTransaction(this);
//    }

int InvestmentTransaction::idInvestmentToAccount() const
{
  switch (m_action)
  {
  case InvestmentAction::Transfer:
  case InvestmentAction::Swap:
  case InvestmentAction::Spinoff:
    return splitFor(InvestmentSplitType::InvestmentTo).idAccount;

  default:
    return Constants::NO_ID;
  }
}

void InvestmentTransaction::makeBuySellFee(InvestmentAction _action,
                                           const Amount& _pricePerShare,
                                           const QList<Split>& _splits,
                                           const QList<InvestmentSplitType>& _types,
                                           const Lots& _lots)
{
  //Check action
  checkAction(_action, {InvestmentAction::Buy, InvestmentAction::Sell,
                        InvestmentAction::ShortSell, InvestmentAction::ShortCover,
                        InvestmentAction::Fee });

  QSet<InvestmentSplitType> optional = {InvestmentSplitType::Tax, InvestmentSplitType::Fee};

  if (_action == InvestmentAction::Sell || _action == InvestmentAction::ShortCover)
  {
    optional.insert(InvestmentSplitType::GainLoss);
  }
  else if (_action == InvestmentAction::Fee)
  {
    optional.clear();
  }

  //Check splits
  checkSplits(_splits,
              _types,
              /* req */ {InvestmentSplitType::CostProceeds,
                         InvestmentSplitType::Investment,
                         InvestmentSplitType::Trading},
              optional);

  //Can now save the data!
  //Make a backup first.
  InvestmentAction oldAction = m_action;
  auto oldTypes = m_types;
  auto oldSlits = m_splits;

  m_action = _action;
  m_splits = _splits;
  setTypes(_types);

  //Check the lots (these will validate and update the lots)
  try
  {
    if (_action == InvestmentAction::Buy || _action == InvestmentAction::ShortSell)
    {
      InvestmentLotsManager::instance()->updateTransactionSplit(this);
      m_lots.clear();
    }
    else if (_action != InvestmentAction::Fee)
    {
      InvestmentLotsManager::instance()->updateUsages(this,
                                                      _lots);
      m_lots = _lots;
    }
  }
  catch (ModelException)
  {
    //Restore the backup
    m_action = oldAction;
    m_types = oldTypes;
    m_splits = oldSlits;

    throw;
  }

  emitSplitSignals(oldSlits, m_splits);

  m_pricePerShare = _pricePerShare;

  if (oldAction != _action)
    emit investmentActionChanged(oldAction);

  if (!onHoldToModify())
    emit modified();
}

void InvestmentTransaction::makeTransferSwap(InvestmentAction _action,
                                             const QList<Split>& _splits,
                                             const QList<InvestmentSplitType>& _types,
                                             const Lots& _lots)
{
  //Check action
  checkAction(_action, {InvestmentAction::Transfer, InvestmentAction::Swap});

  QSet<InvestmentSplitType> req = {InvestmentSplitType::InvestmentFrom, InvestmentSplitType::InvestmentTo};

  if (_action == InvestmentAction::Swap)
  {
    req.insert(InvestmentSplitType::Trading);
  }

  //Check splits
  checkSplits(_splits,
              _types,
              req);

  //Can now save the data!
  //Make a backup first.
  InvestmentAction oldAction = m_action;
  auto oldTypes = m_types;
  auto oldSlits = m_splits;

  m_action = _action;
  m_splits = _splits;
  setTypes(_types);

  //Check the lots (these will validate and update the lots)
  try
  {
    InvestmentLotsManager::instance()->updateUsages(this,
                                                    _lots);
    m_lots = _lots;
  }
  catch (ModelException)
  {
    //Restore the backup
    m_action = oldAction;
    m_types = oldTypes;
    m_splits = oldSlits;

    throw;
  }

  emitSplitSignals(oldSlits, m_splits);

  if (oldAction != _action)
    emit investmentActionChanged(oldAction);

  if (!onHoldToModify())
    emit modified();
}

void InvestmentTransaction::makeSpinoff(const QList<Split>& _splits,
                                        const QList<InvestmentSplitType>& _types,
                                        const Lots& _lots)
{
  QSet<InvestmentSplitType> req = {InvestmentSplitType::Investment, InvestmentSplitType::InvestmentTo,
                                   InvestmentSplitType::Trading};

  //Check splits
  checkSplits(_splits, _types, req);

  //Can now save the data!
  //Make a backup first.
  InvestmentAction oldAction = m_action;
  auto oldTypes = m_types;
  auto oldSlits = m_splits;

  m_action = InvestmentAction::Spinoff;
  m_splits = _splits;
  setTypes(_types);

  //Check the lots (these will validate and update the lots)
  try
  {
    InvestmentLotsManager::instance()->updateUsages(this,
                                                    _lots);
    m_lots = _lots;
  }
  catch (ModelException)
  {
    //Restore the backup
    m_action = oldAction;
    m_types = oldTypes;
    m_splits = oldSlits;

    throw;
  }

  emitSplitSignals(oldSlits, m_splits);

  if (oldAction != InvestmentAction::Spinoff)
    emit investmentActionChanged(oldAction);

  if (!onHoldToModify())
    emit modified();
}

void InvestmentTransaction::makeSplit(int _idInvestmentAccount, const SplitFraction& _fraction)
{
  //Check the investment account
  checkIdInvestmentAccount(_idInvestmentAccount);

  //Check the split fraction
  if (_fraction.first <= 0 || _fraction.second <= 0 || _fraction.first == _fraction.second)
  {
    ModelException::throwException(tr("The split fraction is invalid."), this);
    return;
  }

  //Can now save the data!
  auto oldSlits = m_splits;
  auto oldAction = m_action;
  auto oldFrac = m_splitFraction;

  m_action = InvestmentAction::StockSplit;
  m_splitFraction = _fraction;
  m_types.clear();
  m_splits.clear();
  m_lots.clear();

  //Add anchor split
  addAnchorSplit(_idInvestmentAccount);

  InvestmentLotsManager::instance()->updateTransactionSplit(this);
  m_lots.clear();

  emitSplitSignals(oldSlits, m_splits);

  if (oldAction != InvestmentAction::StockSplit)
  {
    emit investmentActionChanged(oldAction);
  }
  else if (oldFrac != m_splitFraction)
  {
    emit stockSplitAmountChanged();
  }

  if (!onHoldToModify())
    emit modified();
}

void InvestmentTransaction::makeReinvestedDivDist(InvestmentAction _action,
                                                  const Amount& _pricePerShare,
                                                  const QList<Split>& _splits,
                                                  const QList<InvestmentSplitType>& _types,
                                                  const DistribComposition& _composition)
{
  //Check action
  checkAction(_action, {InvestmentAction::ReinvestDiv, InvestmentAction::ReinvestDistrib});

  //Check splits
  checkSplits(_splits,
              _types,
              /* req */ {InvestmentSplitType::DistributionSource,
                         InvestmentSplitType::Investment,
                         InvestmentSplitType::Trading},
              /* opt */ {InvestmentSplitType::Fee,
                         InvestmentSplitType::Tax,
                         InvestmentSplitType::CashInLieu});

  //Check composition if dist. (this will catch both sum errors and not empty if Dividend errors)
  if (m_action == InvestmentAction::ReinvestDistrib)
  {
    checkDistribComposition(_action, _composition);
  }

  //Can now save the data!
  auto oldSlits = m_splits;
  auto oldAction = m_action;

  m_action = _action;
  m_pricePerShare = _pricePerShare;
  m_splits = _splits;
  setTypes(_types);
  setDistribComposition(_composition);

  InvestmentLotsManager::instance()->updateTransactionSplit(this);
  m_lots.clear();

  emitSplitSignals(oldSlits, m_splits);

  if (oldAction != _action)
  {
    emit investmentActionChanged(oldAction);
  }

  if (!onHoldToModify())
    emit modified();
}

void InvestmentTransaction::makeDivDist(InvestmentAction _action,
                                        int _idInvestmentAccount,
                                        const QList<Split>& _splits,
                                        const QList<InvestmentSplitType>& _types,
                                        const DistribComposition& _composition)
{
  //Check action
  checkAction(_action, {InvestmentAction::Dividend, InvestmentAction::Distribution});

  //Check the investment account
  checkIdInvestmentAccount(_idInvestmentAccount);

  //Check the splits
  checkSplits(_splits,
              _types,
              /* req */ {InvestmentSplitType::DistributionSource,
                         InvestmentSplitType::DistributionDest},
              /* opt */ {InvestmentSplitType::Tax,
                         InvestmentSplitType::Trading});

  //Check composition if dist. (this will catch both sum errors and not empty if Dividend errors)
  if (m_action == InvestmentAction::Distribution)
  {
    checkDistribComposition(_action, _composition);
  }

  auto oldSlits = m_splits;
  auto oldAction = m_action;

  m_action = _action;
  m_splits = _splits;
  setTypes(_types);
  setDistribComposition(_composition);
  m_lots.clear();

  //Add an "anchor" split to the investment account.
  addAnchorSplit(_idInvestmentAccount);

  InvestmentLotsManager::instance()->removeTransaction(this);

  emitSplitSignals(oldSlits, m_splits);

  if (oldAction != _action)
  {
    emit investmentActionChanged(oldAction);
  }

  if (!onHoldToModify())
    emit modified();
}

void InvestmentTransaction::makeCostBasisAdjustment(int _idInvestmentAccount, const Amount& _adjustment)
{
  //Check the investment account
  checkIdInvestmentAccount(_idInvestmentAccount);
  auto oldSlits = m_splits;
  auto oldAction = m_action;

  m_action = InvestmentAction::CostBasisAdjustment;
  m_basisAdjustment = _adjustment;

  m_splits.clear();
  m_types.clear();
  m_lots.clear();
  InvestmentLotsManager::instance()->removeTransaction(this);

  //Add an "anchor" split to the investment account.
  addAnchorSplit(_idInvestmentAccount);

  emitSplitSignals(oldSlits, m_splits);

  if (oldAction != InvestmentAction::CostBasisAdjustment)
  {
    emit investmentActionChanged(oldAction);
  }

  if (!onHoldToModify())
    emit modified();
}

void InvestmentTransaction::makeUndistributedCapitalGain(int _idInvestmentAccount,
                                                         const Amount& _capitalGain,
                                                         const Amount& _taxPaid)
{
  //Check the investment account
  checkIdInvestmentAccount(_idInvestmentAccount);
  auto oldSlits = m_splits;
  auto oldAction = m_action;

  m_action = InvestmentAction::UndistributedCapitalGain;
  m_basisAdjustment = _capitalGain;
  m_taxPaid = _taxPaid;

  m_splits.clear();
  m_types.clear();
  m_lots.clear();
  InvestmentLotsManager::instance()->removeTransaction(this);

  //Add an "anchor" split to the investment account.
  addAnchorSplit(_idInvestmentAccount);

  emitSplitSignals(oldSlits, m_splits);

  if (oldAction != InvestmentAction::CostBasisAdjustment)
  {
    emit investmentActionChanged(oldAction);
  }

  if (!onHoldToModify())
    emit modified();
}

void InvestmentTransaction::setDistribComposition(const DistribComposition& _composition)
{
  if (m_action == InvestmentAction::Distribution
      || m_action == InvestmentAction::ReinvestDistrib)
  {
    checkDistribComposition(m_action, _composition);
    m_distribComposition = _composition;

    if (_composition.isEmpty())
    {
      m_distribComposition[DistribType::Other] = 100;
    }
  }
  else
  {
    m_distribComposition.clear();
  }

  if (!onHoldToModify())
    emit modified();
}

void InvestmentTransaction::checkDistribComposition(InvestmentAction _action, const DistribComposition& _composition)
{
  if (_composition.isEmpty()) return;

  Amount t;
  for (Amount a : _composition)
  {
    t += a;
  }

  if (_action != InvestmentAction::Distribution
      && _action != InvestmentAction::ReinvestDistrib)
  {
    ModelException::throwException(tr("Distribution composition can only "
                                      "be set for Distribution or Reinvested Distribution transactions."),
                                   this);
  }
  else if (t != 100)
  {
    ModelException::throwException(tr("Distribution composition must add up to 100."),
                                   this);
  }
}

void InvestmentTransaction::checkSplits(const QList<Split>& _splits,
                                        const QList<InvestmentSplitType>& _types,
                                        const QSet<InvestmentSplitType>& _required,
                                        const QSet<InvestmentSplitType>& _optional,
                                        InvestmentAction _action)
{
  //First, must balance
  if (!Transaction::splitsBalance(_splits))
  {
    ModelException::throwException(tr("The splits do not balance."), this);
  }

  //Next: must have same number of splits and types
  if (_splits.count() != _types.count())
  {
    ModelException::throwException(tr("There must be the same number of splits and types."), this);
  }

  QString actCurrency;
  int idSecurity = Constants::NO_ID, idSecurityOther = Constants::NO_ID;
  int idAcctFrom = Constants::NO_ID, idAcctTo = Constants::NO_ID;

  //Check the account types and non-zero
  for (int i = 0; i < _splits.count(); ++i)
  {
    if (_splits[i].amount == 0)
    {
      ModelException::throwException(tr("Cannot have zero amount splits!."), this);
    }

    Account* a = Account::getTopLevel()->getChild(_splits[i].idAccount);

    switch (_types[i])
    {
    case InvestmentSplitType::Investment:
      idSecurity = a->idSecurity();
      idAcctFrom = a->id();

      if (a->type() != AccountType::INVESTMENT)
      {
        ModelException::throwException(tr("Wrong account for this type (investment)."), this);
      }
      break;

    case InvestmentSplitType::InvestmentFrom:
      idSecurity = a->idSecurity();
      idAcctFrom = a->id();

      if (a->idSecurity() == Constants::NO_ID)
      {
        ModelException::throwException(tr("Wrong account for this type (investment from)."), this);
      }
      break;

    case InvestmentSplitType::InvestmentTo:
      idSecurityOther = a->idSecurity();
      idAcctTo = a->id();

      if (a->idSecurity() == Constants::NO_ID)
      {
        ModelException::throwException(tr("Wrong account for this type (investment to)."), this);
      }
      break;

    case InvestmentSplitType::Trading:
      if (a->type() != AccountType::TRADING)
      {
        ModelException::throwException(tr("Wrong account for this type (trading)."), this);
      }
      break;

    case InvestmentSplitType::CostProceeds:
      actCurrency = _splits[i].currency; //we want to go through, no break

    default:
      if (a->type() == AccountType::INVESTMENT)
      {
        ModelException::throwException(tr("Wrong account for this type (non investment)."), this);
      }
      break;
    }
  }

  //Check if the cost/proceed currency matches the investment currency
  if (!actCurrency.isEmpty() && idSecurity != Constants::NO_ID
      && SecurityManager::instance()->get(idSecurity)->currency() != actCurrency)
  {
    ModelException::throwException(tr("The cost/proceeds currency has to match "
                                      "the trading currency of the security."), this);
  }

  if (idAcctFrom == idAcctTo && idAcctFrom != Constants::NO_ID)
  {
    ModelException::throwException(tr("The source and destination accounts must differ."), this);
  }

  if (_action == InvestmentAction::Transfer && idSecurity != idSecurityOther)
  {
    ModelException::throwException(tr("The source and destination securities for the transfer must match."), this);
  }
  else if ((_action == InvestmentAction::Swap || _action == InvestmentAction::Spinoff) && idSecurity == idSecurityOther)
  {
    ModelException::throwException(tr("The source and destination securities for the swap must differ."), this);
  }

  //Next: must contain exactly one of each (except trading), contain all the required, and be a subset of req \cup opt
  QHash<InvestmentSplitType, int> counts;
  QSet<InvestmentSplitType> types;

  for (InvestmentSplitType t : _types)
  {
    if (t != InvestmentSplitType::Trading)
    {
      if (counts.contains(t))
      {
        counts[t] += 1;
      }
      else
      {
        counts[t] = 1;
      }
    }

    types.insert(t);
  }

  for (int i : counts)
  {
    if (i != 1)
    {
      ModelException::throwException(tr("At most one split of each type (except Trading) is allowed."), this);
    }
  }

  if (!types.contains(_required))
  {
    ModelException::throwException(tr("At least one required split is missing."), this);
  }

  if (!(_required | _optional).contains(types)) //Operator Union. It's ok ;-)
  {
    ModelException::throwException(tr("The split types must be a subset of the required "
                                      "and optional types for this investment transaction action."), this);
  }

  //If we're here, then all is well!
}

void InvestmentTransaction::checkAction(InvestmentAction _action, const QSet<InvestmentAction>& _allowed)
{
  if (!_allowed.contains(_action))
  {
    ModelException::throwException(tr("Invalid investment transaction action!"), this);
  }
}

void InvestmentTransaction::checkIdInvestmentAccount(int _idInvestmentAccount)
{
  //The next line will throw an exception if the account doesn't exist
  Account* a = Account::getTopLevel()->account(_idInvestmentAccount);

  //Now check that it's an investment account
  if (a->type() != AccountType::INVESTMENT)
  {
    ModelException::throwException(tr("The investment split must relate to an investment account."), this);
  }
}

void InvestmentTransaction::setTypes(const QList<InvestmentSplitType>& _types)
{
  m_types.clear();

  for (int i = 0; i < _types.count(); ++i)
  {
    m_types[_types[i]] = i;
    m_splits[i].userData = QString::number((int) _types[i]);
  }
}

void InvestmentTransaction::addAnchorSplit(int _idInvestmentAccount)
{
  Split s(0, _idInvestmentAccount, "");
  s.userData = QString::number((int) InvestmentSplitType::Investment);

  m_types[InvestmentSplitType::Investment] = m_splits.count();
  m_splits << s;
}

void InvestmentTransaction::addToInvestmentLotsManager()
{
  switch (m_action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::ShortSell:
  case InvestmentAction::StockSplit:
  case InvestmentAction::ReinvestDistrib:
  case InvestmentAction::ReinvestDiv:
    InvestmentLotsManager::instance()->updateTransactionSplit(this);
    break;

  case InvestmentAction::Sell:
  case InvestmentAction::ShortCover:
  case InvestmentAction::Transfer:
  case InvestmentAction::Swap:
  case InvestmentAction::Spinoff:
    InvestmentLotsManager::instance()->updateUsages(this, m_lots);
    break;

  default:
    break;
  }
}

void InvestmentTransaction::emitSplitSignals(const QList<Split>& _old, const QList<Split>& _new)
{
  for (const Split& s : _old)
  {
    emit splitRemoved(s);
  }

  for (const Split& s : _new)
  {
    emit splitAdded(s);
  }
}

QString InvestmentTransaction::autoMemo() const {
  // Provide an "auto-memo" if none is provided.
  if (!m_memo.isEmpty()) {
    return m_memo;
  }

  QString memo;

  switch (m_action) {
  case InvestmentAction::Buy:
    memo = tr("Bought %1 shares at %2 per share. Total cost: %3").arg(shareCount().toString()).arg(pricePerShare().toString()).arg(splitFor(InvestmentSplitType::CostProceeds).invertedFormattedAmount());
    break;
  case InvestmentAction::Sell:
    memo = tr("Sold %1 shares at %2 per share. Total proceeds: %3").arg((-1 * shareCount()).toString()).arg(pricePerShare().toString()).arg(splitFor(InvestmentSplitType::CostProceeds).formattedAmount());
    break;
  case InvestmentAction::Dividend:
  case InvestmentAction::Distribution:
    memo = tr("%1: %2").arg(actionToString(m_action)).arg(splitFor(InvestmentSplitType::DistributionSource).invertedFormattedAmount());
    break;
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::ReinvestDistrib:
    memo = tr("Reinvested %1: %2 shares at %3 per share.").arg(splitFor(InvestmentSplitType::DistributionSource).invertedFormattedAmount()).arg(shareCount().toString()).arg(pricePerShare().toString());
    break;
  case InvestmentAction::Fee:
    memo = tr("Fee: %1").arg(actionToString(m_action)).arg(splitFor(InvestmentSplitType::Fee).formattedAmount());
    break;
  default:
    break;
  }

  if (!memo.isEmpty()) {
    memo = QString("[%1] %2").arg(SecurityManager::instance()->get(Account::getTopLevel()->account(idInvestmentAccount())->idSecurity())->symbol()).arg(memo);
  }

  return memo;
}

QString InvestmentTransaction::transactionColor() const {
  switch (m_action) {
  case InvestmentAction::Buy:
    return "#e5fae1";
  case InvestmentAction::Sell:
    return "#fae1e1";
  case InvestmentAction::Dividend:
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::Distribution:
  case InvestmentAction::ReinvestDistrib:
    return "#f8fae1";
  case InvestmentAction::Fee:
    return "#f5f5f5";
  default:
    return "";
  }
}

void InvestmentTransaction::load(QXmlStreamReader& _reader)
{
  QXmlStreamAttributes attributes = _reader.attributes();

  m_action = (InvestmentAction) IO::getAttribute("action", attributes).toInt();

  switch (m_action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::ShortSell:
  case InvestmentAction::Sell:
  case InvestmentAction::ShortCover:
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::Fee:
    m_pricePerShare = Amount::fromStoreable(IO::getAttribute("pricepershare", attributes));
    break;

  case InvestmentAction::StockSplit:
  {
    QStringList frac = IO::getAttribute("splitfraction", attributes).split(":", QString::SkipEmptyParts);
    if (frac.size() != 2)
    {
      throw IOException(tr("Invalid value for the split fraction."));
    }
    m_splitFraction.first = frac[0].toInt();
    m_splitFraction.second = frac[1].toInt();
    break;
  }
  case InvestmentAction::ReinvestDistrib:
    m_pricePerShare = Amount::fromStoreable(IO::getAttribute("pricepershare", attributes));
  case InvestmentAction::Distribution:    //Yes fall through
  {
    QStringList l = IO::getAttribute("distribcomposition", attributes).split(",", QString::SkipEmptyParts);
    m_distribComposition.clear();

    for (const QString& s : l)
    {
      QStringList d = s.split(":", QString::SkipEmptyParts);

      if (d.size() != 2)
      {
        throw IOException(tr("Invalid value for the distribution composition."));
      }
      m_distribComposition.insert((DistribType) d[0].toInt(), Amount::fromStoreable(d[1]));
    }
    break;
  }
  case InvestmentAction::UndistributedCapitalGain:
    m_taxPaid = Amount::fromStoreable(IO::getAttribute("taxpaid", attributes)); //Pass through!

  case InvestmentAction::CostBasisAdjustment: //Yes fall through
    m_basisAdjustment = Amount::fromStoreable(IO::getAttribute("basisadjustment", attributes));
    break;

  case InvestmentAction::Invalid:

  default:
    break;
  }

  //Read the transaction
  if (_reader.readNextStartElement()
      &&_reader.tokenType() == QXmlStreamReader::StartElement
      && _reader.name() == StdTags::TRANSACTION)
  {
    Transaction::load(_reader);
  }
  else
  {
    throw IOException(tr("Transaction element missing for an investment transaction!."));
  }

  //Load the types
  int i = 0;
  for (const Split& s : m_splits)
  {
    m_types[(InvestmentSplitType) s.userData.toInt()] = i++;
  }

  //Add to lot manager
  //        addToInvestmentLotsManager();
}

void InvestmentTransaction::save(QXmlStreamWriter& _writer) const
{
  _writer.writeStartElement(StdTags::INVEST_TRANSACTION);
  _writer.writeAttribute("action", QString::number((int) m_action));

  //Do not write attributes that are not related to the transaction
  switch (m_action)
  {
  case InvestmentAction::Buy:
  case InvestmentAction::ShortSell:
  case InvestmentAction::Sell:
  case InvestmentAction::ShortCover:
  case InvestmentAction::ReinvestDiv:
  case InvestmentAction::Fee:
    _writer.writeAttribute("pricepershare", m_pricePerShare.toStoreable());
    break;

  case InvestmentAction::StockSplit:
    _writer.writeAttribute("splitfraction", QString("%1:%2").arg(m_splitFraction.first)
                           .arg(m_splitFraction.second));
    break;


  case InvestmentAction::ReinvestDistrib:
    _writer.writeAttribute("pricepershare", m_pricePerShare.toStoreable()); //Yes fall through
  case InvestmentAction::Distribution:
  {
    QStringList l;

    for (auto i = m_distribComposition.begin(); i != m_distribComposition.end(); ++i)
    {
      l << QString("%1:%2").arg((int) i.key()).arg(i.value().toStoreable());
    }

    _writer.writeAttribute("distribcomposition", l.join(','));
    break;
  }

  case InvestmentAction::UndistributedCapitalGain:
    _writer.writeAttribute("taxpaid", m_taxPaid.toStoreable()); //Yes fall through
  case InvestmentAction::CostBasisAdjustment:
    _writer.writeAttribute("basisadjustment", m_basisAdjustment.toStoreable());
    break;

  default:
    break;
  }

  //This will save the rest of the transaction as a sub tag
  Transaction::save(_writer);

  _writer.writeEndElement();
}

}


unsigned int qHash(KLib::InvestmentSplitType _key, unsigned int seed)
{
  return qHash((int) _key, seed);
}

unsigned int qHash(KLib::InvestmentAction _key, unsigned int seed)
{
  return qHash((int) _key, seed);
}

unsigned int qHash(KLib::DistribType _key, unsigned int seed)
{
  return qHash((int) _key, seed);
}
