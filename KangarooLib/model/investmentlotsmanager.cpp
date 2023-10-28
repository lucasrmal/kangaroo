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

#include "investmentlotsmanager.h"
#include "investmenttransaction.h"
#include "modelexception.h"
#include "ledger.h"
#include "transactionmanager.h"
#include "account.h"
#include "../controller/io.h"
#include <QXmlStreamReader>
#include <QLinkedList>
#include <QDebug>

namespace KLib
{
    InvestmentLotsManager* InvestmentLotsManager::m_instance = new InvestmentLotsManager();

    //Availability calculators

    void Lot::adjustAvailability(Lots& _previous, int _idInvestmentAccount) const
    {
        if (_idInvestmentAccount == idInvestmentAccount)
        {
            if (_previous.contains(idLot))
            {
                _previous[idLot] += amount;
            }
            else
            {
                _previous.insert(idLot, amount);
            }
        }
    }

    void LotSplit::adjustAvailability(Lots& _previous, int _idInvestmentAccount) const
    {
        if (_idInvestmentAccount == idInvestmentAccount)
        {
            for (Amount& a : _previous)
            {
                a = (a*splitFraction.first)/splitFraction.second;
            }
        }
    }

    void LotTransferSwap::adjustAvailability(Lots& _previous, int _idInvestmentAccount) const
    {
        if (_idInvestmentAccount == idAccountFrom
            || _idInvestmentAccount == idAccountTo)
        {
            for (auto i = lots.begin(); i != lots.end(); ++i)
            {
                Amount toAdd = _idInvestmentAccount == idAccountTo ? i.value()
                                                                   : -i.value();
                if (_previous.contains(i.key()))
                {
                    _previous[i.key()] += toAdd;
                }
                else
                {
                    _previous.insert(i.key(), toAdd);
                }
            }
        }
    }

    void LotUsage::adjustAvailability(Lots& _previous, int _idInvestmentAccount) const
    {
        if (_idInvestmentAccount == idInvestmentAccount)
        {
            for (auto i = lots.begin(); i != lots.end(); ++i)
            {
                if (_previous.contains(i.key()))
                {
                    _previous[i.key()] -= i.value();
                }
                else
                {
                    _previous.insert(i.key(), -i.value());
                }
            }
        }
    }


    InvestmentLotsManager::InvestmentLotsManager() :
        m_nextId(0)
    {
    }

    InvestmentLotsManager::~InvestmentLotsManager()
    {
        unload();
    }

    void InvestmentLotsManager::updateTransactionSplit(InvestmentTransaction* _transaction)
    {
        if (!_transaction || _transaction->id() == Constants::NO_ID)
            return;

        switch (_transaction->action())
        {
        case InvestmentAction::Buy:
        case InvestmentAction::ShortSell:
        case InvestmentAction::ReinvestDistrib:
        case InvestmentAction::ReinvestDiv:
        {            
            //Check if the transaction had a lot number/usage before
            Lot* lot = m_indexLots.contains(_transaction->id()) ? m_indexLots[_transaction->id()]
                                                                : nullptr;

            if (m_indexUsages.contains(_transaction->id())
                || m_indexTransfersSwaps.contains(_transaction->id())
                || m_indexSplits.contains(_transaction->id()))
            {
                //This will remove the usage and update the available balances
                removeTransaction(_transaction);
            }

            const Transaction::Split& investmentSplit = _transaction->splitFor(InvestmentSplitType::Investment);

            if (!lot)
            {
                //Add a new lot
                lot = new Lot(m_nextId++,
                              investmentSplit.amount.abs(),
                              investmentSplit.idAccount,
                              _transaction->id(),
                              _transaction->action(),
                              _transaction->date());

                m_indexLots[_transaction->id()] = lot;
                m_available.insert(PriorityDate(_transaction->date(), lot->sortingPriority()), lot);
                m_lots[lot->idLot] = lot;
            }
            else
            {
                //Check if the date changed
                if (_transaction->date() != lot->transactionDate)
                {
                    m_available.remove(PriorityDate(lot->transactionDate, lot->sortingPriority()),
                                       lot);
                    m_available.insert(PriorityDate(_transaction->date(), lot->sortingPriority()),
                                       lot);
                    lot->transactionDate = _transaction->date();
                }

                lot->action = _transaction->action();
                lot->amount = investmentSplit.amount.abs();
                lot->idInvestmentAccount = investmentSplit.idAccount;
            }

            emit modified();
            break;
        }
        case InvestmentAction::StockSplit:
        {
            if (m_indexUsages.contains(_transaction->id())
                || m_indexTransfersSwaps.contains(_transaction->id())
                || m_indexLots.contains(_transaction->id()))
            {
                //This will remove the usage and update the available balances
                removeTransaction(_transaction);
            }

            LotSplit* split = m_indexSplits.contains(_transaction->id())
                              ? m_indexSplits[_transaction->id()]
                              : nullptr;


            if (!split)
            {
                //Add a new split
                LotSplit* split = new LotSplit(_transaction->idInvestmentAccount(),
                                               _transaction->id(),
                                               _transaction->splitFraction(),
                                               _transaction->date());

                m_indexSplits[_transaction->id()] = split;
                m_available.insert(PriorityDate(_transaction->date(), split->sortingPriority()), split);
            }
            else
            {
                //Check if the date changed
                if (_transaction->date() != split->transactionDate)
                {
                    m_available.remove(PriorityDate(split->transactionDate, split->sortingPriority()),
                                       split);
                    m_available.insert(PriorityDate(_transaction->date(), split->sortingPriority()),
                                       split);
                    split->transactionDate = _transaction->date();
                }

                split->idInvestmentAccount = _transaction->idInvestmentAccount();
                split->splitFraction = _transaction->splitFraction();
            }

            emit modified();

            break;
        }
        default:
            ModelException::throwException(tr("Cannot call updateTransactionSplit() on a transaction other than Buy/ShortSell/Reinvest/Split."),
                                           this);
        }
    }

    void InvestmentLotsManager::updateDate(InvestmentTransaction* _transaction)
    {
        if (!_transaction || _transaction->id() == Constants::NO_ID)
            return;

        ILotAvailabilityCalculator* current = nullptr;

        if (m_indexLots.contains(_transaction->id()))
        {
            current = m_indexLots[_transaction->id()];
        }
        else if (m_indexSplits.contains(_transaction->id()))
        {
            current = m_indexSplits[_transaction->id()];
        }
        else if (m_indexUsages.contains(_transaction->id()))
        {
            current = m_indexUsages[_transaction->id()];
        }
        else if (m_indexTransfersSwaps.contains(_transaction->id()))
        {
            current = m_indexTransfersSwaps[_transaction->id()];
        }

        if (current)
        {
            m_available.remove(PriorityDate(current->transactionDate, current->sortingPriority()), current);
            current->transactionDate = _transaction->date();
            m_available.insert(PriorityDate(current->transactionDate, current->sortingPriority()), current);
        }

    }

    void InvestmentLotsManager::removeTransaction(InvestmentTransaction* _transaction)
    {
        if (!_transaction || _transaction->id() == Constants::NO_ID)
            return;

        ILotAvailabilityCalculator* object = nullptr;

        if (m_indexLots.contains(_transaction->id()))
        {
            //Remove the lot. Some transactions may become invalid (over usage/nonexistent lots), but
            //this should not cause problems and will be fixed next time these transaction are modified.
            object = m_indexLots[_transaction->id()];
            m_lots.remove(m_indexLots[_transaction->id()]->idLot);
            m_indexLots.remove(_transaction->id());
        }

        if (m_indexSplits.contains(_transaction->id()))
        {
            object = m_indexSplits[_transaction->id()];
            m_indexSplits.remove(_transaction->id());
        }

        if (m_indexUsages.contains(_transaction->id()))
        {
            object = m_indexUsages[_transaction->id()];
            m_indexUsages.remove(_transaction->id());
        }

        if (m_indexTransfersSwaps.contains(_transaction->id()))
        {
            object = m_indexTransfersSwaps[_transaction->id()];
            m_indexTransfersSwaps.remove(_transaction->id());
        }

        if (object)
        {
            m_available.remove(PriorityDate(object->transactionDate, object->sortingPriority()), object);
            delete object;
            emit modified();
        }
    }

    void InvestmentLotsManager::updateUsages(InvestmentTransaction* _transaction,
                                             const Lots&_lots)
    {
        if (!_transaction || _transaction->id() == Constants::NO_ID)
            return;

        //Check if the transaction had a lot number/usage before
        bool hadUsage = m_indexUsages.contains(_transaction->id());
        bool hadTransfer = m_indexTransfersSwaps.contains(_transaction->id());
        QString error; //For error messages

        switch (_transaction->action())
        {
        case InvestmentAction::Sell:
        case InvestmentAction::ShortCover:
        case InvestmentAction::Transfer:
        case InvestmentAction::Swap:
        {
            //Validate the count
            if (!_lots.isEmpty() && !validateLotsCount(_lots, _transaction->shareCount().abs()))
            {
                ModelException::throwException(tr("The number of shares in the lot does not match "
                                                  "the number of shares in the transaction."), this);
                return;
            }

            if (m_indexLots.contains(_transaction->id())
                || m_indexSplits.contains(_transaction->id())
                || (hadUsage && (_transaction->action() == InvestmentAction::Transfer
                                 || _transaction->action() == InvestmentAction::Swap))
                || (hadTransfer
                    && _transaction->action() != InvestmentAction::Transfer
                    && _transaction->action() != InvestmentAction::Swap))
            {
                //This will remove the lot
                removeTransaction(_transaction);
                hadUsage = hadTransfer = false;
            }



            if (!_lots.isEmpty() && !hadUsage && !hadTransfer)
            {
                if (!validateUsage(_transaction->action(),
                                   _transaction->idInvestmentAccount(),
                                   Constants::NO_ID,
                                   _lots,
                                   _transaction->date(),
                                   error))
                {
                    ModelException::throwException(tr("Unable to add the usages: %1").arg(error), this);
                    return;
                }
                else
                {
                    if (_transaction->action() == InvestmentAction::Transfer
                        || _transaction->action() == InvestmentAction::Swap)
                    {
                        LotTransferSwap* ts = new LotTransferSwap(_transaction->idInvestmentAccount(),
                                                                  _transaction->idInvestmentToAccount(),
                                                                  _transaction->id(),
                                                                  _lots,
                                                                  _transaction->date());
                        m_indexTransfersSwaps[_transaction->id()] = ts;
                        m_available.insert(PriorityDate(_transaction->date(), ts->sortingPriority()), ts);
                    }
                    else
                    {
                        LotUsage* us = new LotUsage(_transaction->idInvestmentAccount(),
                                                    _transaction->id(),
                                                    _lots,
                                                    _transaction->date());
                        m_indexUsages[_transaction->id()] = us;
                        m_available.insert(PriorityDate(_transaction->date(), us->sortingPriority()), us);
                    }
                }
            }
            else if (!_lots.isEmpty()) //We know that the previous transaction had the same "category" (USAGE or TRANSFER)
            {
                ILotAvailabilityCalculator* current = (_transaction->action() == InvestmentAction::Transfer
                                                       || _transaction->action() == InvestmentAction::Swap)
                                                      ? static_cast<ILotAvailabilityCalculator*>(m_indexTransfersSwaps[_transaction->id()])
                                                      : static_cast<ILotAvailabilityCalculator*>(m_indexUsages[_transaction->id()]);

                if (!validateUsage(_transaction->action(),
                                   _transaction->idInvestmentAccount(),
                                   _transaction->id(),
                                   _lots,
                                   _transaction->date(),
                                   error))
                {
                    ModelException::throwException(tr("Unable to add the usages: %1").arg(error), this);
                    return;
                }

//                //Check all the same type
//                if (!lotsHaveSameClass(_lots))
//                {
//                    ModelException::throwException(tr("Lots must have the same action (buy or short sell)"), this);
//                    return;
//                }

//                //Lots must correspond to action
//                if (_transaction->action() != InvestmentAction::Transfer
//                    && _transaction->action() != InvestmentAction::Swap
//                    && lot(_lots.begin().key())->action != _transaction->action())
//                {
//                    ModelException::throwException(tr("Lots must have the same action than the transaction"), this);
//                    return;
//                }

//                //See if there is availability
//                Lots available = lotsAvailable(investmentActionClass(lot(_lots.begin().key())->action),
//                                               _transaction->idInvestmentAccount(),
//                                               _transaction->date());

//                if (current->transactionDate <= _transaction->date()) //Otherwise it will not have been included in available
//                {
//                    const Lots& toAdd = (_transaction->action() == InvestmentAction::Transfer
//                                         || _transaction->action() == InvestmentAction::Swap)
//                                 ? m_indexTransfersSwaps[_transaction->id()]->lots
//                                 : m_indexUsages[_transaction->id()]->lots;

//                    for (auto i = toAdd.begin(); i != toAdd.end(); ++i)
//                    {
//                        if (m_lots.contains(i.key())) //In case the lot was removed...
//                        {
//                            if (!available.contains(i.key()))
//                            {
//                                available.insert(i.key(), i.value());
//                            }
//                            else
//                            {
//                                available[i.key()] += i.value();
//                            }
//                        }
//                    }
//                }

//                //Remove the new amounts
//                for (auto i = _lots.begin(); i != _lots.end(); ++i)
//                {
//                    if (!available.contains(i.key()))
//                    {
//                        //No availability!
//                        ModelException::throwException(tr("No availability for lot %1.").arg(i.key()), this);
//                        return;
//                    }
//                    else
//                    {
//                        available[i.key()] -= i.value();
//                    }
//                }

//                //Check if everything is at least 0.
//                for (auto i = available.begin(); i != available.end(); ++i)
//                {
//                    if (i.value() < 0)
//                    {
//                        ModelException::throwException(tr("No availability for lot %1.").arg(i.key()), this);
//                        return;
//                    }
//                }

                //Ok, all is well, so lets save the changes.
                if (current->transactionDate != _transaction->date())
                {
                    m_available.remove(PriorityDate(current->transactionDate, current->sortingPriority()), current);
                    m_available.insert(PriorityDate(_transaction->date(), current->sortingPriority()), current);
                    current->transactionDate = _transaction->date();
                }

                if (_transaction->action() == InvestmentAction::Transfer
                    || _transaction->action() == InvestmentAction::Swap)
                {
                    LotTransferSwap* ts = static_cast<LotTransferSwap*>(current);

                    ts->lots = _lots;
                    ts->idAccountTo = _transaction->idInvestmentToAccount();
                    ts->idAccountFrom = _transaction->idInvestmentAccount();
                    ILotAvailabilityCalculator::cleanLots(ts->lots);
                }
                else
                {
                    LotUsage* us = static_cast<LotUsage*>(current);

                    us->lots = _lots;
                    us->idInvestmentAccount = _transaction->idInvestmentToAccount();
                    ILotAvailabilityCalculator::cleanLots(us->lots);
                }
            }
            else //Nothing in the lot, just clear.
            {
                removeTransaction(_transaction);
            }

            emit modified();
            break;
        }
        default:
            ModelException::throwException(tr("Cannot call updateUsages() on a transaction "
                                              "other than Sell/ShortCover/Transfer."),
                                           this);
        }
    }

    Lots InvestmentLotsManager::lotsAvailableExcluding(InvestmentActionClass _actionClass,
                                                       int _idInvestmentAccount,
                                                       const QDate& _date,
                                                       int _idTransaction) const
    {
        Lots available;

        //Add the lots+splits
        for (auto i = m_available.begin(); i != m_available.end() && i.key().first <= _date; ++i)
        {
            if (i.value()->idTransaction != _idTransaction)
            {
                i.value()->adjustAvailability(available, _idInvestmentAccount);
            }
        }

        //Remove the lots with a wrong action class
        QLinkedList<int> toRemove;

        for (auto i = available.begin(); i != available.end(); ++i)
        {
            if (i.value() <= 0)
            {
                toRemove.append(i.key());
                continue;
            }

            switch (_actionClass)
            {
            case InvestmentActionClass::Long:
            case InvestmentActionClass::Short:
                if (investmentActionClass(m_lots[i.key()]->action) != _actionClass)
                {
                    toRemove.append(i.key());
                }

            case InvestmentActionClass::Transfer:
                continue;

            case InvestmentActionClass::Invalid:
                return Lots();
            }
        }

        for (int i : toRemove)
        {
            available.remove(i);
        }

        return available;
    }

    Lots InvestmentLotsManager::lotsAvailable(InvestmentActionClass  _actionClass,
                                              int _idInvestmentAccount,
                                              const QDate& _date) const
    {
        return lotsAvailableExcluding(_actionClass,
                                      _idInvestmentAccount,
                                      _date,
                                      Constants::NO_ID);
    }

    bool InvestmentLotsManager::validateUsage(InvestmentAction _action,
                                                 int _idInvestmentAccount,
                                                 int _idTransaction,
                                                 const Lots& _lots,
                                                 const QDate& _date,
                                                 QString& _error) const
    {
        _error.clear();

        if (!_lots.count())
        {
            return true;
        }

        //Check if all lots exist
        for (int i : _lots.keys())
        {
            if (!m_lots.contains(i))
            {
                _error = tr("Unknown lot #%1").arg(i);
                return false;
            }
        }

        //Check if all lots the same action
        if (!lotsHaveSameClass(_lots))
        {
            _error = tr("Lots must have the same type.");
            return false;
        }

        //Lots must correspond to action
        if (_action != InvestmentAction::Transfer
            && _action != InvestmentAction::Swap
            && investmentActionClass(lot(_lots.begin().key())->action) != investmentActionClass(_action))
        {
            _error = tr("Lots must have the same action than the specified action");
            return false;
        }

        //See if there is availability (cannot check with transfer, must be Buy, Reinvest or ShortSell)
        Lots available = lotsAvailableExcluding(investmentActionClass(lot(_lots.begin().key())->action),
                                                _idInvestmentAccount,
                                                _date,
                                                _idTransaction);

        //Remove the new splits
        for (auto i = _lots.begin(); i != _lots.end(); ++i)
        {
            if (!available.contains(i.key()))
            {
                //No availability!
                _error = tr("No availability for lot %1.").arg(i.key());
                return false;
            }
            else if (available[i.key()] - i.value() < 0)
            {
                _error = tr("No availability for lot %1.").arg(i.key());
                return false;
            }
        }

        return true;
    }

    Lots InvestmentLotsManager::lotsForTransaction(InvestmentTransaction* _transaction) const
    {
        if (!_transaction || _transaction->id() == Constants::NO_ID)
            return Lots();

        switch (_transaction->action())
        {
        case InvestmentAction::Sell:
        case InvestmentAction::ShortCover:
            if (m_indexUsages.contains(_transaction->id()))
            {
                return m_indexUsages[_transaction->id()]->lots;
            }
            break;
        case InvestmentAction::Transfer:
        case InvestmentAction::Swap:
            if (m_indexTransfersSwaps.contains(_transaction->id()))
            {
                return m_indexTransfersSwaps[_transaction->id()]->lots;
            }
            break;

        default:
            break;
        }

        return Lots();
    }

    const LotUsage* InvestmentLotsManager::lotsUsed(InvestmentTransaction* _transaction) const
    {
        if (m_indexUsages.contains(_transaction->id()))
        {
            return m_indexUsages[_transaction->id()];
        }
        else
        {
            return nullptr;
        }
    }

    const LotTransferSwap* InvestmentLotsManager::lotsTransferred(InvestmentTransaction* _transaction) const
    {
        if (m_indexTransfersSwaps.contains(_transaction->id()))
        {
            return m_indexTransfersSwaps[_transaction->id()];
        }
        else
        {
            return nullptr;
        }
    }

    int InvestmentLotsManager::lotNumber(InvestmentTransaction* _transaction) const
    {
        return m_indexLots.contains(_transaction->id()) ? m_indexLots[_transaction->id()]->idLot
                                                           : Constants::NO_ID;
    }

    const Lot* InvestmentLotsManager::lot(int _id) const
    {
        if (!m_lots.contains(_id))
        {
            ModelException::throwException(tr("Lot %1 does not exists!").arg(_id), this);
        }

        return m_lots[_id];
    }

    InvestmentTransaction* InvestmentLotsManager::transactionForLot(int _id) const
    {
        const Lot* l = lot(_id);
        return qobject_cast<InvestmentTransaction*>(TransactionManager::instance()->get(l->idTransaction));
    }

    bool InvestmentLotsManager::validateLotsCount(const Lots& _lots, const Amount& _amount)
    {
        Amount total = _amount;

        for (const Amount& a : _lots)
        {
            if (a < 0)
                return false;

            total -= a;
        }

        return total == 0;
    }

    void ILotAvailabilityCalculator::cleanLots(Lots& _lots)
    {
        QLinkedList<int> toRemove;

        for (auto i = _lots.begin(); i != _lots.end(); ++i)
        {
            if (i.value() == 0)
            {
                toRemove.append(i.key());
            }
        }

        for (int i : toRemove)
        {
            _lots.remove(i);
        }
    }

    Lot::Lot() :
        idLot(Constants::NO_ID),
        idInvestmentAccount(Constants::NO_ID),
        action(InvestmentAction::Invalid)
    {
    }

    bool InvestmentLotsManager::lotsHaveSameClass(const Lots& _lots) const
    {
        if (_lots.empty())
        {
            return true;
        }

        InvestmentActionClass lotClass = investmentActionClass(lot(_lots.begin().key())->action);

        for (auto i = _lots.begin(); i != _lots.end(); ++i)
        {
            if (investmentActionClass(lot(i.key())->action) != lotClass)
            {
                return false;
            }
        }

        return true;
    }

    InvestmentActionClass InvestmentLotsManager::investmentActionClass(const InvestmentAction& _action)
    {
        switch (_action)
        {
        case InvestmentAction::Buy:
        case InvestmentAction::ReinvestDistrib:
        case InvestmentAction::ReinvestDiv:
        case InvestmentAction::Sell:
            return InvestmentActionClass::Long;

        case InvestmentAction::ShortSell:
        case InvestmentAction::ShortCover:
            return InvestmentActionClass::Short;

        case InvestmentAction::Transfer:
        case InvestmentAction::Swap:
            return InvestmentActionClass::Transfer;

        default:
            return InvestmentActionClass::Invalid;
        }
    }

    void InvestmentLotsManager::load(QXmlStreamReader& _reader)
    {

        while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::INVEST_LOT_MGR))
        {
            QXmlStreamAttributes attributes = _reader.attributes();

            if (_reader.tokenType() == QXmlStreamReader::StartElement
                && (_reader.name() == StdTags::INVEST_LOT_USAGE
                    || _reader.name() == StdTags::INVEST_LOT_TRANSFER))
            {
                Lots lots;
                int idTransaction = IO::getAttribute("idtransaction", attributes).toInt();
                QStringList list = IO::getAttribute("lots", attributes).split(',', QString::SkipEmptyParts);

                for (const QString& s : list)
                {
                    QStringList cur = s.split(':', QString::SkipEmptyParts);

                    if (cur.count() != 2)
                    {
                        throw IOException(tr("Unexpected number of arguments in investment lot usage."));
                    }

                    lots[cur[0].toInt()] = Amount::fromStoreable(cur[1]);
                }

                if (lots.count())
                {
                    if (_reader.name() == StdTags::INVEST_LOT_USAGE)
                    {
                        LotUsage* lot = new LotUsage();
                        lot->idTransaction = idTransaction;
                        lot->lots = lots;
                        m_indexUsages[idTransaction] = lot;
                    }
                    else //Transfer
                    {
                        LotTransferSwap* lot = new LotTransferSwap();
                        lot->idTransaction = idTransaction;
                        lot->lots = lots;
                        m_indexTransfersSwaps[idTransaction] = lot;
                    }
                }
            }
            else if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::INVEST_LOT_SPLIT)
            {
                LotSplit* lot = new LotSplit();
                lot->idTransaction = IO::getAttribute("idtransaction", attributes).toInt();
                m_indexSplits[lot->idTransaction] = lot;
            }
            else if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::INVEST_LOT)
            {
                Lot* l = new Lot();
                l->idLot = IO::getAttribute("id", attributes).toInt();
                l->amount = Amount::fromStoreable(IO::getAttribute("amount", attributes));
                l->idTransaction = IO::getAttribute("idtransaction", attributes).toInt();

                m_lots[l->idLot] = l;
                m_indexLots[l->idTransaction] = l;
                m_nextId = std::max(m_nextId, l->idLot);
            }

            _reader.readNext();
        }
    }

    void InvestmentLotsManager::save(QXmlStreamWriter& _writer) const
    {
        _writer.writeStartElement(StdTags::INVEST_LOT_MGR);

        //Save usages
        for (const LotUsage* usage : m_indexUsages)
        {
            QStringList lots;
            for (auto j = usage->lots.begin(); j != usage->lots.end(); ++j)
            {
                lots.append(QString("%1:%2").arg(j.key()).arg(j.value().toStoreable()));
            }

            _writer.writeEmptyElement(StdTags::INVEST_LOT_USAGE);
            _writer.writeAttribute("idtransaction", QString::number(usage->idTransaction));
            _writer.writeAttribute("lots", lots.join(","));
        }

        //Save splits
        for (const LotSplit* split : m_indexSplits)
        {
            _writer.writeEmptyElement(StdTags::INVEST_LOT_SPLIT);
            _writer.writeAttribute("idtransaction", QString::number(split->idTransaction));
        }

        //Save transfers/swaps
        for (const LotTransferSwap* trsw : m_indexTransfersSwaps)
        {
            QStringList lots;
            for (auto j = trsw->lots.begin(); j != trsw->lots.end(); ++j)
            {
                lots.append(QString("%1:%2").arg(j.key()).arg(j.value().toStoreable()));
            }

            _writer.writeEmptyElement(StdTags::INVEST_LOT_TRANSFER);
            _writer.writeAttribute("idtransaction", QString::number(trsw->idTransaction));
            _writer.writeAttribute("lots", lots.join(","));
        }

        //Save the lots
        for (const Lot* lot : m_indexLots)
        {
            _writer.writeEmptyElement(StdTags::INVEST_LOT);
            _writer.writeAttribute("id", QString::number(lot->idLot));
            _writer.writeAttribute("amount", lot->amount.toStoreable());
            _writer.writeAttribute("idtransaction", QString::number(lot->idTransaction));
        }

        _writer.writeEndElement();

    }

    void InvestmentLotsManager::afterLoad()
    {
        //Load accounts, actions for lots
        QList<int> toRemove;
        for (Lot* lot : m_indexLots)
        {
            try
            {
                const InvestmentTransaction* trans = qobject_cast<InvestmentTransaction*>(
                                                      TransactionManager::instance()->get(lot->idTransaction));

                if (!trans)
                {
                    qDebug() << tr("Expected Investment transaction, got Transaction!");
                    toRemove << lot->idLot;
                    continue;
                }

                lot->action = trans->action();
                lot->idInvestmentAccount = trans->idInvestmentAccount();
                lot->transactionDate = trans->date();

                //Add to the availability index
                m_available.insert(PriorityDate(lot->transactionDate, lot->sortingPriority()), lot);
            }
            catch (ModelException)
            {
                toRemove << lot->idLot;
            }
        }

        for (int idLot : toRemove)
        {
            m_indexLots.remove(m_lots[idLot]->idTransaction);
            m_lots.remove(idLot);
        }

        //Load the split fractions
        for (LotSplit* lot : m_indexSplits)
        {
            const InvestmentTransaction* trans = qobject_cast<InvestmentTransaction*>(
                                                  TransactionManager::instance()->get(lot->idTransaction));

            if (!trans)
            {
                throw IOException(tr("Expected Investment transaction, got Transaction!"));
            }

            lot->splitFraction = trans->splitFraction();
            lot->idInvestmentAccount = trans->idInvestmentAccount();
            lot->transactionDate = trans->date();

            //Add to the availability index
            m_available.insert(PriorityDate(lot->transactionDate, lot->sortingPriority()), lot);
        }

        //Load the usages
        for (LotUsage* lot : m_indexUsages)
        {
            InvestmentTransaction* trans = qobject_cast<InvestmentTransaction*>(
                                                  TransactionManager::instance()->get(lot->idTransaction));

            if (!trans)
            {
                throw IOException(tr("Expected Investment transaction, got Transaction!"));
            }

            lot->idInvestmentAccount = trans->idInvestmentAccount();
            lot->transactionDate = trans->date();
            trans->m_lots = lot->lots;

            //Add to the availability index
            m_available.insert(PriorityDate(lot->transactionDate, lot->sortingPriority()), lot);
        }

        //Load the transfers
        for (LotTransferSwap* lot : m_indexTransfersSwaps)
        {
            InvestmentTransaction* trans = qobject_cast<InvestmentTransaction*>(
                                                  TransactionManager::instance()->get(lot->idTransaction));

            if (!trans)
            {
                throw IOException(tr("Expected Investment transaction, got Transaction!"));
            }

            lot->idAccountFrom = trans->idInvestmentAccount();
            lot->idAccountTo = trans->idInvestmentToAccount();
            lot->transactionDate = trans->date();
            trans->m_lots = lot->lots;

            //Add to the availability index
            m_available.insert(PriorityDate(lot->transactionDate, lot->sortingPriority()), lot);
        }

    }

    void InvestmentLotsManager::unload()
    {
        m_nextId = 0;
        m_available.clear();

        for (Lot* l : m_indexLots)
        {
            delete l;
        }

        for (LotSplit* l : m_indexSplits)
        {
            delete l;
        }

        for (LotUsage* l : m_indexUsages)
        {
            delete l;
        }

        for (LotTransferSwap* l : m_indexTransfersSwaps)
        {
            delete l;
        }

        m_indexLots.clear();
        m_indexSplits.clear();
        m_indexUsages.clear();
        m_indexTransfersSwaps.clear();
        m_lots.clear();
        m_available.clear();
    }

}
