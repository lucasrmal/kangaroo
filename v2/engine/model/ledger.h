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

#ifndef LEDGER_H
#define LEDGER_H

#include <QMultiMap>
#include <QHash>
#include <QDate>
#include <QLinkedList>
#include "transaction.h"
#include "../interfaces/scriptable.h"
//#include "../util/augmentedtreapmap.h"
#include "../util/fragmentedtreapmap.h"

namespace KLib {

    typedef QPair<int, int> SplitFraction;

    //Specialize for Balances
    namespace AugmentedTreapSum
    {
        template<>
        bool isEmpty<Balances>(const Balances& _s);

        template<>
        Balances makeEmpty<Balances>();
    }

    //Specialize for SplitFraction
    namespace FragmentedTreap
    {
        template<>
        Balances transform<SplitFraction, Balances>(const SplitFraction& _ratio, const Balances& _previous);

        template<>
        SplitFraction makeEmpty<SplitFraction>();

    }

    class Account;    
    class InvestmentTransaction;
    enum class InvestmentAction;

    typedef FragmentedTreapMap1<QDate, Transaction*, SplitFraction,  Balances> LedgerMap;
    typedef LedgerMap::const_iterator TransactionIterator;
    typedef std::pair<TransactionIterator, TransactionIterator> TransactionRange;

    class Ledger : public QObject
    {
        Q_OBJECT
        K_SCRIPTABLE(Ledger)

        Q_PROPERTY(int count READ count)
        Q_PROPERTY(int idAccount READ idAccount)

        public:
            Ledger(KLib::Account* _account);

            virtual ~Ledger() {}

            Q_INVOKABLE const LedgerMap* transactions() const { return &m_transactions; }

            /**
              @brief balanceAt

              @param _date Only consider transactions at or before _date. If invalid, all transactions are considered.
              @param _currency Balance in a specific currency.
              @return The balance at the end of the day on _date.
             *
             * If _currency is specified, returns only the balance in _currency. Otherwise, returns the combined
             * balance in the account's main currency.
            */
            Q_INVOKABLE KLib::Amount balanceAt(const QDate& _date, const QString& _currency = QString()) const;

            /**
             * @brief Today's balance
             * @param _currency Balance in a specific currency.
             * @return The balance
             *
             * If _currency is specified, returns only the balance in _currency. Otherwise, returns the combined
             * balance in the account's main currency.
             */
            Q_INVOKABLE  KLib::Amount balanceToday(const QString& _currency = QString()) const;

            /**
              @brief Balance including all the transactions in the account, including future ones.
             * @param _currency Balance in a specific currency.

              @return The balance.
             *
             * If _currency is specified, returns only the balance in _currency. Otherwise, returns the combined
             * balance in the account's main currency.
            */
            Q_INVOKABLE KLib::Amount balance(const QString& _currency = QString()) const;

            /**
             * @brief Today's balance
             * @param _currency Balance in a specific currency.
             * @return The balance

              @return The balances for all currencies in the account.
             */
            Q_INVOKABLE Balances balancesToday() const { return m_transactions.sumTo(QDate::currentDate()); }

            /**
              @brief Balance including all the transactions in the account, including future ones.

              @return The balances for all currencies in the account.
            */

            Q_INVOKABLE Balances balances() const { return m_transactions.sum(); }

            /**
              @brief balanceBetween

              @return The difference between the balance after to and the balance before from.
              (from beginning of day _from to end of day on _to).
            */
            Q_INVOKABLE KLib::Amount balanceBetween(const QDate& _from, const QDate& _to, const QString& _currency = QString()) const;


            Balances balancesBetween(const QDate& _from, const QDate& _to) const;

            Q_INVOKABLE QLinkedList<KLib::Transaction*> transactionsBetween(const QDate& _from, const QDate& _to) const;

            /**
              @brief costBasisAt
              Only relevant for investment account ledgers.

              @param[in] _date Only consider transactions at or before _date. If invalid, all transactions are considered.
              @return The cost basis for all the shares in the account at _date
            */
            Q_INVOKABLE KLib::Amount costBasisAt(const QDate& _date) const;

            Q_INVOKABLE KLib::Amount costBasisBefore(const KLib::Transaction* _tr) const;

            Q_INVOKABLE KLib::Amount costBasis() const { return costBasisBefore(nullptr); }

            /**
              @brief Returns the balance, converted to _currency, of the account until _tr (exclusive).
            */
            Q_INVOKABLE KLib::Amount balanceBefore(const KLib::Transaction* _tr, const QString& _currency = QString()) const;

            /**
              @brief Returns the balances of the account until _tr (exclusive).
            */
            Q_INVOKABLE Balances balancesBefore(const KLib::Transaction* _tr) const;


            Q_INVOKABLE QDate firstTransactionDate() const;
            Q_INVOKABLE QDate lastTransactionDate() const;

            int count() const { return m_transactions.size(); }

            QSet<QString> currenciesUsed(const QDate& _from = QDate(), const QDate& _to = QDate()) const;

            /**
             * @brief idAccount
             * @return This ledger's account ID.
             */
            int idAccount() const;

            /**
             * @brief cccount
             * @return This ledger's account.
             */
            Q_INVOKABLE KLib::Account* account() const { return m_account; }


        signals:
            void modified();

        private:            
            TransactionRange    transactionRange(const QDate& _begin, const QDate& _end) const;
            Amount              balanceIn(const Balances& _balances, const QString& _currency, const QDate& _date) const;

            Balances balancesBefore(const KLib::Transaction* _tr, QDate& _lastDate) const;

            Account* m_account;

            LedgerMap     m_transactions;
            QHash<int, QDate>  m_splits;

            friend class LedgerManager;

            /*
             * HOW STOCK SPLITS ARE HANDLED
             *
             * Store list of splits ordered by date, with original fraction. When a change occurs, check the list of splits,
             * for each split after, update the balance accordingly.
             *
             * When split fraction changes, new split is added, or split is removed, compute the difference in the balance and
             * adjust it accordingly.
             *
             * Thus, the current-day and full balances are always up-to-date, using the split adjusted amount.     *
             *
             */
    };

    class LedgerManager : public QObject
    {
        Q_OBJECT
        K_SCRIPTABLE(LedgerManager)

        LedgerManager() {}

        public:

            ~LedgerManager() {}

//            Q_INVOKABLE KLib::Transaction* addTransaction(const QDate& _date,
//                                                          QList<KLib::Transaction::Split> _splits,
//                                                          const QString& _no = QString(),
//                                                          int _idPayee = Constants::NO_ID,
//                                                          const QString& _memo = QString(),
//                                                          const QHash<QString, QVariant>& _properties = QHash<QString, QVariant>());

            /**
              @brief Adds a transaction (can be an investment transaction).

              The transaction will be added to the ledger (OR deleted if an exception arises).

              The transaction must not have been added before (id is invalid) It WILL NOT be deleted if ID is valid.

              It will be added to the investment lots manager if necessary,

              Do NOT delete it after calling this function, even if it fails (throws an exception).
              It will be handled by LedgerManager from now on.
            */
            Q_INVOKABLE KLib::Transaction* addTransaction(Transaction* _tr);

            Q_INVOKABLE void removeTransaction(int _id);

            Q_INVOKABLE KLib::Ledger* ledger(int _idAccount) const { return m_ledgers[_idAccount]; }

            static LedgerManager* instance() { return m_instance; }

        signals:
            void transactionDateChanged(KLib::Transaction* _tr, const QDate& _old);
            void splitAmountChanged(const KLib::Transaction::Split& _split, KLib::Transaction* _tr);
            void splitAdded(const KLib::Transaction::Split& _split, KLib::Transaction* _tr);
            void splitRemoved(const KLib::Transaction::Split& _split, KLib::Transaction* _tr);

            void balanceChanged(int _idAccount, const Balances& _difference);
            void balanceTodayChanged(int _idAccount, const Balances& _difference);

        public slots:
            void onSplitAdded(const KLib::Transaction::Split& _split);
            void onSplitRemoved(const KLib::Transaction::Split& _split);
            void onSplitAmountChanged(const KLib::Transaction::Split& _split);
            void onTransactionDateChanged(const QDate& _old);
            void onStockSplitAmountChanged();
            void onInvestmentActionChanged(InvestmentAction _previous);

        private:
            void addAccount(Account* _acc);
            void removeAccount(Account* _acc);

            void removeStockSplit(InvestmentTransaction* _inv_tr);
            void addStockSplit(InvestmentTransaction* _inv_tr);

            void connectSignals(Transaction* _tr) const;
            void connectInvestmentSignals(InvestmentTransaction* _tr) const;

            void checkIfBalancesChanged(int _idAccount, const QDate& _date, const Balances& _prior);

            void load();
            void unload();

            QHash<int, Ledger*> m_ledgers;

            static LedgerManager* m_instance;
            static const QDate m_today;

            friend class Account;
    };

}

Q_DECLARE_METATYPE(KLib::Ledger*)
Q_DECLARE_METATYPE(KLib::LedgerManager*)

#endif // LEDGER_H
