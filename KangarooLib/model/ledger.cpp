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

#include "ledger.h"
#include "transactionmanager.h"
#include "pricemanager.h"
#include "account.h"
#include "investmenttransaction.h"
#include "investmentlotsmanager.h"
#include <stdexcept>
#include <QPair>
#include "modelexception.h"
#include "security.h"
#include "payee.h"
#include "../controller/io.h"
#include "currency.h"

namespace KLib {

    namespace AugmentedTreapSum
    {
        template<>
        bool isEmpty<Balances>(const Balances& _s) {
            return _s.isEmpty();
        }

        template<>
        Balances makeEmpty<Balances>()
        {
            return Balances();
        }
    }

    namespace FragmentedTreap
    {
        template<>
        Balances transform<SplitFraction, Balances>(const SplitFraction& _ratio, const Balances& _previous)
        {
            Balances b;
            for (auto i = _previous.begin(); i != _previous.end(); ++i)
            {
                b.add(i.key(), InvestmentTransaction::balanceAfterSplit(i.value(), _ratio));
            }
            return b;
        }

        template<>
        SplitFraction makeEmpty<SplitFraction>()
        {
            return SplitFraction(1,1);
        }
    }

LedgerManager* LedgerManager::m_instance = nullptr;
const QDate LedgerManager::m_today = QDate::currentDate();

Ledger::Ledger(Account* _account) : m_account(_account)
{
}

Amount Ledger::balanceAt(const QDate& _date, const QString& _currency) const
{
    return balanceIn(balancesBetween(QDate(), _date), _currency, _date);
}

Amount Ledger::balance(const QString& _currency) const
{
    return balanceIn(balancesBetween(QDate(), QDate()), _currency, QDate());
}

Amount Ledger::balanceToday(const QString& _currency) const
{
    return balanceIn(balancesBetween(QDate(), QDate::currentDate()), _currency, QDate::currentDate());
}

Amount Ledger::balanceBetween(const QDate& _from, const QDate& _to, const QString& _currency) const
{
    if (m_transactions.isEmpty())
        return 0;

    return balanceIn(balancesBetween(_from, _to), _currency, _to);
}

Balances Ledger::balancesBetween(const QDate& _from, const QDate& _to) const
{
    if (!_to.isValid() && !_to.isValid())
    {
        return m_transactions.sum();
    }
    else if (!_from.isValid()) // from is not valid, to is
    {
        return m_transactions.sumTo(_to);
    }
    else if (!_to.isValid()) // to is not valid, from is
    {
        return m_transactions.sumFrom(_from);
    }
    else //Both are valid
    {
        return m_transactions.sumBetween(_from, _to);
    }
}

Amount Ledger::balanceIn(const Balances& _balances, const QString& _currency, const QDate& _date) const
{
    if (_currency.isEmpty() && account()->mainCurrency().isEmpty()) //Security-based account
    {
        return _balances.contains("") ? _balances.value("") : 0;
    }
    else if (_currency.isEmpty()) //Currency-based account
    {
        Amount inMain;

        for (auto i = _balances.begin(); i != _balances.end(); ++i)
        {
            if (i.value() != 0)
            {
                inMain += i.value()
                        * PriceManager::instance()->rate(i.key(),
                                                         account()->mainCurrency(),
                                                         _date);
            }
        }

        return inMain;
    }
    else
    {
        return _balances.contains(_currency) ? _balances[_currency] : 0;
    }
}

TransactionRange Ledger::transactionRange(const QDate& _begin, const QDate& _end) const
{
    return TransactionRange(_begin.isValid() ? m_transactions.lowerBound(_begin)
                                             : m_transactions.begin(),
                            _end.isValid() ? m_transactions.upperBound(_end)
                                           : m_transactions.end());
}

QLinkedList<KLib::Transaction*> Ledger::transactionsBetween(const QDate& _from, const QDate& _to) const
{
    TransactionRange range = transactionRange(_from, _to);
    QLinkedList<Transaction*> list;

    for (auto i = range.first; i != range.second; ++i)
    {
        list.append(i.value());
    }

    return list;
}

Amount Ledger::balanceBefore(const Transaction* _tr, const QString& _currency) const
{
    if (!_tr)
    {
        return balance(_currency);
    }
    else
    {
        QDate lastDate;
        return balanceIn(balancesBefore(_tr, lastDate), _currency, lastDate);
    }
}

Balances Ledger::balancesBefore(const KLib::Transaction* _tr) const
{
    QDate d;
    return balancesBefore(_tr, d);
}

Balances Ledger::balancesBefore(const KLib::Transaction* _tr, QDate& _lastDate) const
{
    if (!_tr)
    {
        return balances();
    }
    else
    {
        auto i = m_transactions.find(_tr->date(), const_cast<Transaction*>(_tr));
        _lastDate = _tr->date();
        Balances balances = m_transactions.sumBefore(i);

        //Find previous date...
        --i;
        if (i != m_transactions.end())
        {
          _lastDate = i.key();
        }

        return balances;
    }
}

Amount Ledger::costBasisAt(const QDate& _date) const
{
    if (!_date.isValid())
    {
        return costBasisBefore(nullptr);
    }
    else
    {
        auto i = m_transactions.upperBound(_date); //Returns iterator to first transaction after _date.
        return costBasisBefore(i == m_transactions.end() ? nullptr : i.value());
    }
}

KLib::Amount Ledger::costBasisBefore(const Transaction* _tr) const
{
    try
    {
        Security* s = SecurityManager::instance()->get(m_account->idSecurity());
        int precCur = CurrencyManager::instance()->get(s->currency())->precision();
        Amount cost(0, precCur);
        Amount balance(0, s->precision());

        for (auto i = m_transactions.begin(); i != m_transactions.end() && i.value() != _tr; ++i)
        {
            const InvestmentTransaction* trans = qobject_cast<InvestmentTransaction*>(i.value());

            if (!trans)
                continue;

            switch (trans->action())
            {
            case InvestmentAction::Buy:
            //case InvestmentAction::ShortCover:
            case InvestmentAction::ReinvestDiv:
            case InvestmentAction::ReinvestDistrib:
                cost += (trans->shareCount()*trans->pricePerShare() + trans->fee()).toPrecision(precCur);
                balance += trans->shareCount();
                break;

            case InvestmentAction::Sell:
                if (balance - trans->shareCount() == 0)
                {
                    cost.clear();
                    balance.clear();
                }
                else
                {
                    cost -= cost*(trans->shareCount().abs().toDouble() / balance.toDouble());
                    balance -= trans->shareCount();
                }
                break;

            case InvestmentAction::ShortSell:
                cost += trans->pricePerShare()*trans->shareCount() - trans->fee();
                balance += trans->shareCount();
                break;

            case InvestmentAction::ShortCover:
                if (trans->shareCount() + balance == 0) //Covered everything
                {
                    cost.clear();
                    balance.clear();
                }
                else
                {
                    cost *= (-trans->shareCount().toDouble() / balance.toDouble());
                    balance += trans->shareCount();
                }
                break;

            case InvestmentAction::Transfer:
            case InvestmentAction::Swap:
                if (trans->idInvestmentAccount() != m_account->id()) //If the from account is the other, compute other's cost basis
                {
                    int otherAccount = trans->idInvestmentAccount();
                    Amount othBalance = LedgerManager::instance()->ledger(otherAccount)->balanceBefore(i.value());

                    if (othBalance == trans->shareCount())
                    {
                        cost += LedgerManager::instance()->ledger(otherAccount)->costBasisBefore(i.value());
                    }
                    else
                    {
                        cost += LedgerManager::instance()->ledger(otherAccount)->costBasisBefore(i.value())
                                * trans->shareCount().toDouble() / othBalance.toDouble();
                    }

                    balance += trans->splitFor(InvestmentSplitType::InvestmentTo).amount;
                }
                else //Remove shares
                {
                    if (trans->shareCount().abs() == balance)
                    {
                        cost.clear();
                        balance.clear();
                    }
                    else
                    {
                        cost -= cost* (trans->shareCount().abs().toDouble() / balance.toDouble());
                        balance -= trans->shareCount().abs();
                    }
                }
                break;

            case InvestmentAction::StockSplit:
                balance = InvestmentTransaction::balanceAfterSplit(balance, trans->splitFraction());
                break;

//            case InvestmentAction::Distribution:
//                if (trans->distribComposition().contains(DistribType::ReturnOfCapital)
//                    && trans->distribComposition().value(DistribType::ReturnOfCapital) > 0)
//                {
//                    curcost = trans->splitFor(InvestmentSplitType::DistributionSource).amount //Will be negative - we want that.
//                              * (trans->distribComposition().value(DistribType::ReturnOfCapital, 0)/100.0);
//                    cost += curcost;
//                }
//                break;


//            case InvestmentAction::UndistributedCapitalGain:
//            case InvestmentAction::CostBasisAdjustment:
//                curcost = trans->basisAdjustment();
//                cost += curcost;
//                break;

            default:
                break;
            }

            //Check if distrib or reinvested dist and ReturnOfCapital
//            if (trans->action() == InvestmentAction::ReinvestDistrib
//                && trans->distribComposition().contains(DistribType::ReturnOfCapital)
//                && trans->distribComposition().value(DistribType::ReturnOfCapital) > 0)
//            {
//                curcost = trans->splitFor(InvestmentSplitType::DistributionSource).amount //Will be negative - we want that.
//                          * (trans->distribComposition().value(DistribType::ReturnOfCapital)/100.0);
//                cost += curcost;
//            }
        }

        return cost;
    }
    catch (ModelException) {}

    return 0;
}

QSet<QString> Ledger::currenciesUsed(const QDate& _from, const QDate& _to) const
{
    QSet<QString> currencies;
    TransactionRange range = transactionRange(_from, _to);

    for (auto i = range.first; i != range.second; ++i)
    {
        for (Transaction::Split s : i.value()->splits())
        {
            if (s.idAccount == idAccount() && !s.currency.isEmpty())
            {
                currencies.insert(s.currency);
            }
        }
    }

    return currencies;
}

int Ledger::idAccount() const
{
    return m_account->id();
}

QDate Ledger::firstTransactionDate() const
{
    return m_transactions.count() ? m_transactions.firstKey()
                                  : QDate();
}

QDate Ledger::lastTransactionDate() const
{
    return m_transactions.count() ? m_transactions.lastKey()
                                  : QDate();
}

//Transaction* LedgerManager::addTransaction(const QDate& _date,
//                                           QList<Transaction::Split> _splits,
//                                           const QString& _no,
//                                           int _idPayee,
//                                           const QString& _memo,
//                                           const QHash<QString, QVariant>& _properties)
//{
//    //Check if the splits balance
//    if (!Transaction::splitsBalance(_splits))
//    {
//        ModelException::throwException(tr("The transaction does not balance."), nullptr);
//    }



//    if (_idPayee != Constants::NO_ID)
//    {
//        PayeeManager::instance()->get(_idPayee);
//    }

//    Transaction* temp = new Transaction();
//    temp->m_no = _no;
//    temp->m_date = _date;
//    temp->m_splits = _splits;
//    temp->m_idPayee = _idPayee;
//    temp->m_memo = _memo;
//    temp->checkIfCurrencyExchange();

//    for (auto i = _properties.begin(); i != _properties.end(); ++i)
//    {
//        temp->properties()->set(i.key(), i.value());
//    }

//    return addTransaction(temp);
//}

KLib::Transaction* LedgerManager::addTransaction(Transaction* _tr)
{
    if (!_tr)
    {
        ModelException::throwException(tr("The transaction is null."), nullptr);
    }

    //Check the date
    if (!_tr->date().isValid())
    {
        _tr->deleteLater();
        ModelException::throwException(tr("The transaction's date must be valid."), nullptr);
    }

    //Check the ID
    if (_tr->id() != Constants::NO_ID)
    {
        ModelException::throwException(tr("The transaction cannot have a valid ID."), nullptr);
    }

    //Check if investment transaction
    InvestmentTransaction* inv_tr = qobject_cast<InvestmentTransaction*>(_tr);

    if (inv_tr && inv_tr->action() == InvestmentAction::Invalid)
    {
        _tr->deleteLater();
        ModelException::throwException(tr("The transaction cannot have an invalid action."), nullptr);
    }

    //Find the ledgers related to the transaction
    QHash<int, Balances> amountsPerAccount;

    foreach (const Transaction::Split &s, _tr->splits())
    {
        if (!m_ledgers.contains(s.idAccount))
        {
            _tr->deleteLater();
            ModelException::throwException(tr("Cannot add a split for account %1: it is a placeholder.")
                                           .arg(Account::getTopLevel()->account(s.idAccount)->name()), nullptr);
        }
        else
        {
            amountsPerAccount[s.idAccount].add(s.currency, s.amount);
        }
    }

    //Check for zero amounts
    if (!inv_tr)
    {
        for (auto i = amountsPerAccount.begin(); i != amountsPerAccount.end(); ++i)
        {
            if (i.value().isEmpty())
            {
                _tr->deleteLater();
                ModelException::throwException(tr("The total splits for account %1 cannot be zero.")
                                               .arg(Account::getTopLevel()->getChild(i.key())->name()), nullptr);
                return nullptr;
            }
        }
    }

    _tr->m_id = TransactionManager::newId();
    _tr->checkIfCurrencyExchange();



    for (auto i = amountsPerAccount.begin(); i != amountsPerAccount.end(); ++i)
    {
        //Add the values to the respective ledger's balances

        Balances priorBalance = m_ledgers[i.key()]->m_transactions.sum();
        m_ledgers[i.key()]->m_transactions.insert(_tr->date(), _tr, i.value());

        if (inv_tr && inv_tr->action() == InvestmentAction::StockSplit)
        {
            addStockSplit(inv_tr);
        }
        else
        {
            checkIfBalancesChanged(i.key(), _tr->date(), priorBalance);
        }

        emit m_ledgers[i.key()]->modified();
    }

    connectSignals(_tr);

    if (inv_tr)
    {
        connectInvestmentSignals(inv_tr);
        inv_tr->addToInvestmentLotsManager();
    }

    TransactionManager::instance()->add(_tr);

    for (Transaction::Split s : _tr->m_splits)
    {
        emit splitAdded(s, _tr);
    }

    return _tr;
}

void LedgerManager::removeTransaction(int _id)
{
    Transaction* tr = TransactionManager::instance()->get(_id);

    if (tr)
    {
        //Check if investment transaction
        InvestmentTransaction* inv_tr = qobject_cast<InvestmentTransaction*>(tr);

        if (inv_tr)
        {
            if (inv_tr->action() == InvestmentAction::StockSplit)
            {
                removeStockSplit(inv_tr);
            }

            inv_tr->m_action = InvestmentAction::Invalid;
        }

        QList<Transaction::Split> oldSplits = tr->m_splits;
        tr->m_splits.clear();

        for (Transaction::Split& s : oldSplits)
        {
            if (m_ledgers.contains(s.idAccount))
            {
                Balances priorBalance = m_ledgers[s.idAccount]->m_transactions.sum();

                //If the account has multiple splits in this transaction, the transaction will only be removed once,
                //no need to emit the signal multiple times then.
                if (m_ledgers[s.idAccount]->m_transactions.remove(tr->date(), tr))
                    emit m_ledgers[s.idAccount]->modified();

                checkIfBalancesChanged(s.idAccount, tr->date(), priorBalance);
            }

            emit splitRemoved(s, tr);
        }

        TransactionManager::instance()->remove(_id);
    }
}

void LedgerManager::onSplitAdded(const Transaction::Split& _split)
{
    Transaction* tr = qobject_cast<Transaction*>(sender());


    if (m_ledgers.contains(_split.idAccount) && tr)
    {
        Balances priorBalance = m_ledgers[_split.idAccount]->m_transactions.sum();

        if (!m_ledgers[_split.idAccount]->m_transactions.contains(tr->date(), tr))
        {
            m_ledgers[_split.idAccount]->m_transactions.insert(tr->date(),
                                                               tr,
                                                               Transaction::totalsForAccount(_split.idAccount,
                                                                                             tr->splits()));
        }
        else
        {
            m_ledgers[_split.idAccount]->m_transactions.setWeight(tr->date(),
                                                                  tr,
                                                                  Transaction::totalsForAccount(_split.idAccount,
                                                                                                tr->splits()));
        }

        checkIfBalancesChanged(_split.idAccount, tr->date(), priorBalance);

        emit m_ledgers[_split.idAccount]->modified();
    }

    emit splitAdded(_split, tr);
}

void LedgerManager::onSplitRemoved(const Transaction::Split& _split)
{
    Transaction* tr = qobject_cast<Transaction*>(sender());

    if (m_ledgers.contains(_split.idAccount))
    {
        Balances priorBalance = m_ledgers[_split.idAccount]->m_transactions.sum();

        //Only remove the transaction if the split was unique with this account number in the transaction.
        if (!tr->relatedTo(_split.idAccount))
        {
            m_ledgers[_split.idAccount]->m_transactions.remove(tr->date(), tr);
        }
        else
        {
            m_ledgers[_split.idAccount]->m_transactions.setWeight(tr->date(),
                                                                  tr,
                                                                  Transaction::totalsForAccount(_split.idAccount,
                                                                                                tr->splits()));
        }

        checkIfBalancesChanged(_split.idAccount, tr->date(), priorBalance);

        emit m_ledgers[_split.idAccount]->modified();
    }

    emit splitRemoved(_split, tr);
}

void LedgerManager::onSplitAmountChanged(const Transaction::Split& _split)
{
    Transaction* tr = qobject_cast<Transaction*>(sender());

    if (m_ledgers.contains(_split.idAccount) && tr)
    {
        Balances priorBalance = m_ledgers[_split.idAccount]->m_transactions.sum();
        m_ledgers[_split.idAccount]->m_transactions.setWeight(tr->date(),
                                                              tr,
                                                              Transaction::totalsForAccount(_split.idAccount,
                                                                                            tr->splits()));
        checkIfBalancesChanged(_split.idAccount, tr->date(), priorBalance);

        emit m_ledgers[_split.idAccount]->modified();
    }

    emit splitAmountChanged(_split, tr);
}

void LedgerManager::removeStockSplit(InvestmentTransaction* _inv_tr)
{
    int idAccount = _inv_tr->idInvestmentAccount();
    Ledger* ledger = m_ledgers[idAccount];

    if (ledger->m_splits.contains(_inv_tr->id()))
    {
        Balances priorBalance = ledger->m_transactions.sum();
        QDate curDate = ledger->m_splits[_inv_tr->id()];

        ledger->m_transactions.joinFragmentsAt(curDate);
        ledger->m_splits.remove(_inv_tr->id());

        checkIfBalancesChanged(idAccount, curDate, priorBalance);
    }
}

void LedgerManager::addStockSplit(InvestmentTransaction* _inv_tr)
{
    int idAccount = _inv_tr->idInvestmentAccount();
    Ledger* ledger = m_ledgers[idAccount];

    //If not split or if already in there...
    if (_inv_tr->action() != InvestmentAction::StockSplit || ledger->m_splits.contains(_inv_tr->id()))
        return;

    Balances priorBalance = ledger->m_transactions.sum();

    ledger->m_transactions.splitFragmentAt(_inv_tr->date(), _inv_tr->splitFraction());
    ledger->m_splits[_inv_tr->id()] = _inv_tr->date();

    checkIfBalancesChanged(idAccount, _inv_tr->date(), priorBalance);
}

void LedgerManager::connectSignals(Transaction* _tr) const
{
    connect(_tr, &Transaction::splitAdded, this, &LedgerManager::onSplitAdded);
    connect(_tr, &Transaction::splitRemoved, this, &LedgerManager::onSplitRemoved);
    connect(_tr, &Transaction::splitAmountChanged, this, &LedgerManager::onSplitAmountChanged);
    connect(_tr, &Transaction::transactionDateChanged, this, &LedgerManager::onTransactionDateChanged);
}

void LedgerManager::connectInvestmentSignals(InvestmentTransaction* _tr) const
{
    connect(_tr, &InvestmentTransaction::investmentActionChanged, this, &LedgerManager::onInvestmentActionChanged);
    connect(_tr, &InvestmentTransaction::stockSplitAmountChanged, this, &LedgerManager::onStockSplitAmountChanged);
}

void LedgerManager::checkIfBalancesChanged(int _idAccount, const QDate& _date, const Balances& _prior)
{
    Balances diff = m_ledgers[_idAccount]->m_transactions.sum() - _prior;

    if (!diff.isEmpty())
    {
        emit balanceChanged(_idAccount, diff);

        if (_date <= m_today)
        {
            emit balanceTodayChanged(_idAccount, diff);
        }
    }
}

void LedgerManager::onInvestmentActionChanged(InvestmentAction _previous)
{
    InvestmentTransaction* tr = qobject_cast<InvestmentTransaction*>(sender());

    if (!tr || tr->action() == _previous)
        return;

    if (_previous == InvestmentAction::StockSplit)
    {
        removeStockSplit(tr);
    }
    else if (tr->action() == InvestmentAction::StockSplit)
    {
        addStockSplit(tr);
    }
}

void LedgerManager::onStockSplitAmountChanged()
{
    InvestmentTransaction* tr = qobject_cast<InvestmentTransaction*>(sender());

    if (!tr || tr->action() != InvestmentAction::StockSplit)
        return;

    int idAccount = tr->idInvestmentAccount();
    Ledger* ledger = m_ledgers[idAccount];

    if (ledger->m_splits.contains(tr->id()))
    {
        Balances diff = ledger->m_transactions.sum();

        ledger->m_transactions.setFragmentRatio(ledger->m_splits.value(tr->id()), tr->splitFraction());
        diff = ledger->m_transactions.sum() - diff;

        emit balanceChanged(idAccount, diff);

        if (tr->date() <= m_today)
        {
            emit balanceTodayChanged(idAccount, diff);
        }
    }
    else
    {
        addStockSplit(tr);
    }
}

void LedgerManager::onTransactionDateChanged(const QDate& _old)
{
    Transaction* tr = qobject_cast<Transaction*>(sender());

    if (tr)
    {
        if (_old == tr->date())
        {
            return;
        }

        //See if it's a stock split
        InvestmentTransaction* inv_tr = qobject_cast<InvestmentTransaction*>(tr);

        if (inv_tr)
        {
            removeStockSplit(inv_tr);

            if (inv_tr->action() == InvestmentAction::StockSplit)
            {
                addStockSplit(inv_tr);
            }

            m_ledgers[inv_tr->idInvestmentAccount()]->m_transactions.move(_old, tr, tr->date());
        }
        else
        {
            QSet<int> moved;

            for (Transaction::Split s : tr->splits())
            {
                if (!moved.contains(s.idAccount))
                {
                    Balances b = m_ledgers[s.idAccount]->m_transactions.sum();
                    m_ledgers[s.idAccount]->m_transactions.move(_old, tr, tr->date());
                    moved.insert(s.idAccount);
                    b = m_ledgers[s.idAccount]->m_transactions.sum() - b;

                    if (!b.isEmpty()) //Change in balance caused by stock split
                    {
                        emit balanceChanged(s.idAccount, b);
                    }
                }

                if (tr->date() <= m_today && _old > m_today) //Moved from future to today or before
                {
                    Balances b;
                    b.add(s.currency, s.amount);

                    emit balanceTodayChanged(s.idAccount, b);
                }
                else if (tr->date() > m_today && _old <= m_today) //Moved from today or before to future
                {
                    Balances b;
                    b.add(s.currency, s.amount);
                    emit balanceTodayChanged(s.idAccount, b);
                }
            }
        }
    }

    emit transactionDateChanged(tr, _old);
}

void LedgerManager::addAccount(Account *_acc)
{
    if (! _acc->isPlaceholder())
    {
        m_ledgers[_acc->id()] = _acc->ledger();
    }
}

void LedgerManager::removeAccount(Account *_acc)
{
    if (! _acc->isPlaceholder())
    {
        Ledger* l = m_ledgers[_acc->id()];

        if (l->count())
        {
            ModelException::throwException(tr("Impossible to remove account as it has transactions. "
                                              "Either delete all transactions manually or close the account."), _acc);
        }

        m_ledgers.remove(_acc->id());
    }
}

void LedgerManager::load()
{
    // Add all transactions to the corresponding ledgers

    for (Transaction* t : TransactionManager::instance()->transactions())
    {
        //Compute the total balance for each account per transaction. The hash is necessary
        //since it is possible to have the same account multiple times per transaction.

        QHash<int, Balances> balancesPerAccount;

        foreach (Transaction::Split s, t->splits())
        {
            if (!m_ledgers.contains(s.idAccount))
            {
                throw IOException(tr("Unrecognized account ID in transaction # %1.").arg(t->id()));
            }
            else
            {
                balancesPerAccount[s.idAccount].add(s.currency, s.amount);
            }
        }

        for (auto i = balancesPerAccount.begin(); i != balancesPerAccount.end(); ++i)
        {
            m_ledgers[i.key()]->m_transactions.insert(t->date(), t, i.value());
        }

        connectSignals(t);

        //Check if investment transaction
        InvestmentTransaction* inv_tr = qobject_cast<InvestmentTransaction*>(t);

        if (inv_tr)
        {
            connectInvestmentSignals(inv_tr);

            if (inv_tr->action() == InvestmentAction::StockSplit)
            {
                addStockSplit(inv_tr);
            }
        }
    }
}

void LedgerManager::unload()
{
    m_ledgers.clear();
}



}
