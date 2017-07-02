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

#include "transactionmanager.h"
#include "modelexception.h"
#include "../controller/io.h"
#include <QXmlStreamReader>
#include "account.h"
#include "investmenttransaction.h"
#include "investmentlotsmanager.h"

namespace KLib
{

int TransactionManager::m_nextId = 0;
TransactionManager* TransactionManager::m_instance = new TransactionManager();

Transaction* TransactionManager::get(int _id) const
{
    if (!m_transactions.contains(_id))
    {
        ModelException::throwException(tr("No such transaction."), this);
        return nullptr;
    }

    return m_transactions[_id];
}

void TransactionManager::add(Transaction *_transaction)
{
    m_transactions.insert(_transaction->id(), _transaction);
    connect(_transaction, SIGNAL(modified()), this, SIGNAL(modified()));
    emit modified();
}

void TransactionManager::remove(int _id)
{
    if (m_transactions.contains(_id))
    {
        Transaction* trans = m_transactions[_id];

        if (qobject_cast<InvestmentTransaction*>(trans))
        {
            InvestmentLotsManager::instance()
                    ->removeTransaction(qobject_cast<InvestmentTransaction*>(trans));
        }

        trans->deleteLater();
        m_transactions.remove(_id);

        emit modified();
    }
}

int TransactionManager::newId()
{
    return m_nextId++;
}

void TransactionManager::load(QXmlStreamReader& _reader)
{
    unload();

//    auto checkIfOldInvestmentTr = [] (Transaction* transaction)
//    {
//        if (transaction->type() == TransactionType::Standard)
//        {
//            return transaction;
//        }

//        //Otherwise, create an investment transaction from the existing transaction.
//        Account* investmentAccount = nullptr;

//        //Find the transaction account id
//        for (const Transaction::Split& s : transaction->splits())
//        {
//            Account* a = Account::getTopLevel()->account(s.idAccount);
//            if (a && a->type() == AccountType::INVESTMENT
//                && a->idSecurity() != Constants::NO_ID)
//            {
//                investmentAccount = a;
//                break;
//            }
//        }

//        if (!investmentAccount)
//            return transaction;

//        //Create the new transaction and the proxy to read the old one
//        InvestmentTransaction* itr = new InvestmentTransaction();
//        itr->m_id = transaction->id();
//        itr->m_date = transaction->date();
//        itr->m_memo = transaction->memo();
//        itr->m_clearedStatus = transaction->clearedStatus();
//        itr->m_flagged = transaction->isFlagged();
//        itr->m_idPayee = transaction->idPayee();
//        itr->m_note = transaction->note();

//        InvestmentTransactionProxy p(transaction, investmentAccount);
//        QList<Transaction::Split> splits = transaction->splits();
//        QList<InvestmentSplitType> types;

//        //Find the transfer account
//        auto i = splits.begin();
//        while (i != splits.end())
//        {
//            if (i->idAccount == investmentAccount->id())
//            {
//                if (transaction->type() == TransactionType::Invest_Transfer)
//                {
//                    if (p.numShares() > 0) //Transfer TO
//                    {
//                        types << InvestmentSplitType::InvestmentTo;
//                    }
//                    else //Transfer FROM
//                    {
//                        types << InvestmentSplitType::InvestmentFrom;
//                    }
//                }
//                else
//                {
//                    types << InvestmentSplitType::Investment;
//                }
//            }
//            else if (i->idAccount == p.idTransferAccount())
//            {
//                if (transaction->type() == TransactionType::Invest_Reinvest)
//                {
//                    types <<  InvestmentSplitType::DistributionSource;
//                }
//                else if (transaction->type() == TransactionType::Invest_Transfer)
//                {
//                    if (p.numShares() > 0) //Transfer TO, so other is FROM
//                    {
//                        types << InvestmentSplitType::InvestmentFrom;
//                    }
//                    else //Transfer FROM, so other is TO
//                    {
//                        types << InvestmentSplitType::InvestmentTo;
//                    }
//                }
//                else
//                {
//                    types << InvestmentSplitType::CostProceeds;
//                }
//            }
//            else if (i->idAccount == p.idCommissionAccount()
//                     && transaction->type() != TransactionType::Invest_Transfer
//                     && transaction->type() != TransactionType::Invest_Split
//                     && !types.contains(InvestmentSplitType::Fee))
//            {
//                types <<  InvestmentSplitType::Fee;
//            }
//            else if (i->idAccount == p.idGainLossAccount()
//                     && transaction->type() == TransactionType::Invest_Sell
//                     && !types.contains(InvestmentSplitType::GainLoss))
//            {
//                types << InvestmentSplitType::GainLoss;
//            }
//            else if (i->idAccount == p.idOtherAccount()
//                     && transaction->type() == TransactionType::Invest_Reinvest
//                     && !types.contains(InvestmentSplitType::CashInLieu))
//            {
//                types << InvestmentSplitType::CashInLieu;
//            }
//            else
//            {
//                i = splits.erase(i);
//                continue;
//            }

//            ++i;
//        }

//        //Add trading splits and corresponding types
//        Transaction::addTradingSplits(splits);

//        while (types.count() < splits.count())
//            types << InvestmentSplitType::Trading;

//        switch (transaction->type())
//        {
//        case TransactionType::Invest_Buy:
//            itr->makeBuySell(InvestmentAction::Buy,
//                             p.pricePerShare(),
//                             splits,
//                             types);
//            break;

//        case TransactionType::Invest_Sell:
//            itr->makeBuySell(InvestmentAction::Sell,
//                             p.pricePerShare(),
//                             splits,
//                             types);
//            break;

//        case TransactionType::Invest_Reinvest:
//            itr->makeReinvestedDivDist(InvestmentAction::ReinvestDiv,
//                                       p.pricePerShare(),
//                                       splits,
//                                       types);
//            break;

//        case TransactionType::Invest_Transfer:
//            itr->makeTransferSwap(InvestmentAction::Transfer,
//                                  splits,
//                                  types);
//            break;

//        default:
//            throw IOException(QObject::tr("Stock Splits Conversion is Not Supported!"));
//        }

//        delete transaction;
//        return (Transaction*) itr;
//    };

    // While not at end
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::TRANSACTION_MGR))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::TRANSACTION)
        {
            Transaction* o = new Transaction();
            o->load(_reader);
            m_nextId = std::max(m_nextId, o->m_id + 1);

//            try
//            {
//                o = checkIfOldInvestmentTr(o);
//            }
//            catch (ModelException e)
//            {
//                throw IOException(tr("Exception occured while transforming transaction %1 "
//                                     "to an investment transaction:\n\n%2").arg(o->id()).arg(e.description()));
//            }

            m_transactions.insert(o->id(), o);
            connect(o, &Transaction::modified, this, &TransactionManager::modified);
        }
        else if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::INVEST_TRANSACTION)
        {
            InvestmentTransaction* o = new InvestmentTransaction();
            o->load(_reader);
            m_nextId = std::max(m_nextId, o->m_id + 1);

            m_transactions.insert(o->id(), o);
            connect(o, &Transaction::modified, this, &TransactionManager::modified);
        }

        _reader.readNext();
    }
}

void TransactionManager::afterLoad()
{
    for (Transaction* t : m_transactions)
    {
        t->checkIfCurrencyExchange();

//        for (Transaction::Split& s : t->m_splits)
//        {
//            if (s.currency.isEmpty())
//            {
//                s.currency = Account::getTopLevel()->account(s.idAccount)->mainCurrency();
//            }
//        }
    }
}

void TransactionManager::save(QXmlStreamWriter &_writer) const
{
    for (Transaction* o : m_transactions)
    {
        o->save(_writer);
    }
}

void TransactionManager::unload()
{
    for (Transaction* i : m_transactions)
    {
        delete i;
    }

    m_transactions.clear();
    m_nextId = 0;
}

}

