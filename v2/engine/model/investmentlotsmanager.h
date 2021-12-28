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

#ifndef INVESTMENTLOTSMANAGER_H
#define INVESTMENTLOTSMANAGER_H

#include "stored.h"
#include "../amount.h"
#include <QDate>

namespace KLib
{
    class InvestmentTransaction;
    enum class InvestmentAction;
    class InvestmentLotsManager;

    typedef QHash<int, Amount> Lots;

    enum class InvestmentActionClass
    {
        Long = 0,
        Short = 1,

        Transfer = 2,

        Invalid = -1
    };

    struct ILotAvailabilityCalculator
    {
        ILotAvailabilityCalculator(int _idTransaction = Constants::NO_ID,
                                   const QDate& _transactionDate = QDate()) :
            idTransaction(_idTransaction),
            transactionDate(_transactionDate) {}

        virtual ~ILotAvailabilityCalculator() {}
        virtual void adjustAvailability(Lots& _previous, int _idInvestmentAccount) const = 0;

        virtual int sortingPriority() const = 0;

        static void cleanLots(Lots& _lots);

        int     idTransaction;
        QDate   transactionDate;
    };

    struct Lot : public ILotAvailabilityCalculator
    {
        Lot();
        Lot(int _id, const Amount& _amount, int _idInvestmentAccount, int _idTransaction,
            InvestmentAction _action, const QDate& _transactionDate) :
            ILotAvailabilityCalculator(_idTransaction, _transactionDate),
            idLot(_id),
            amount(_amount),
            idInvestmentAccount(_idInvestmentAccount),
            action(_action) {}

        int     idLot;
        Amount  amount;     ///< Always positive
        int     idInvestmentAccount;
        InvestmentAction action;

        void adjustAvailability(Lots& _previous, int _idInvestmentAccount) const override;
        int sortingPriority() const override { return 0; }

        //Amount  availableBalanceCache;

        //void recomputeAvailableBalance();
    };

    struct LotSplit : public ILotAvailabilityCalculator
    {
        LotSplit(int _idInvestmentAccount, int _idTransaction,
                 const QPair<int, int>& _splitFraction, const QDate& _transactionDate) :
            ILotAvailabilityCalculator(_idTransaction, _transactionDate),
            idInvestmentAccount(_idInvestmentAccount),
            splitFraction(_splitFraction) {}

        LotSplit() :
            idInvestmentAccount(Constants::NO_ID) {}

        int     idInvestmentAccount;
        QPair<int, int> splitFraction;

        void adjustAvailability(Lots& _previous, int _idInvestmentAccount) const override;
        int sortingPriority() const override { return -1; }
    };

    struct LotTransferSwap : public ILotAvailabilityCalculator
    {
        LotTransferSwap() :
            idAccountFrom(Constants::NO_ID),
            idAccountTo(Constants::NO_ID) {}

        LotTransferSwap(int _idAccountFrom,
                        int _idAccountTo,
                        int _idTransaction,
                        const Lots& _lots,
                        const QDate& _transactionDate) :
            ILotAvailabilityCalculator(_idTransaction, _transactionDate),
            idAccountFrom(_idAccountFrom),
            idAccountTo(_idAccountTo),
            lots(_lots) { cleanLots(lots); }

        int idAccountFrom;
        int idAccountTo;
        Lots lots;

        void adjustAvailability(Lots& _previous, int _idInvestmentAccount) const override;
        int sortingPriority() const override { return 1; }
    };

    struct LotUsage : public ILotAvailabilityCalculator
    {
        LotUsage() :
            idInvestmentAccount(Constants::NO_ID) {}

        LotUsage(int _idInvestmentAccount, int _idTransaction, const Lots& _lots, const QDate& _transactionDate) :
            ILotAvailabilityCalculator(_idTransaction, _transactionDate),
            idInvestmentAccount(_idInvestmentAccount),
            lots(_lots) { cleanLots(lots); }

        int idInvestmentAccount;
        Lots lots;          ///< Always positive

        void adjustAvailability(Lots& _previous, int _idInvestmentAccount) const override;
        int sortingPriority() const override { return 1; }
    };

    class InvestmentLotsManager : public IStored
    {
        Q_OBJECT

        InvestmentLotsManager();

        typedef QPair<QDate, int> PriorityDate;

        public:
            static InvestmentLotsManager* instance() { return m_instance; }

            ~InvestmentLotsManager();

            /**
             * @brief Registers/updates a transaction's lot
             * @param _transaction The transaction.
             *
             * Will register a new lot for the transaction if required (Buy, ShortSell). Otherwise, will
             * de-register this transaction, or update the lot quantities.
             */
            void updateTransactionSplit(InvestmentTransaction* _transaction);

            /**
             * @brief Removes a transaction from the lot system (both lots and usages)
             * @param _transaction The transaction.
             */
            void removeTransaction(InvestmentTransaction* _transaction);

            /**
             * @brief Inserts/updates the lots and quantities used by this transaction
             * @param _transaction The transaction.
             * @param _lots The lots/quantities used
             */
            void updateUsages(InvestmentTransaction* _transaction, const Lots&_lots);

            void updateDate(InvestmentTransaction* _transaction);

            /**
             * @brief Computes a list of the available lots for the specified action and investment account
             * @param _actionClass The action class (Long, Short or Transfer)
             * @param _idInvestmentAccount The investment account ID.
             * @return For each lot number, the number of shares available. Does not return fully-used lots.
             */
            Lots lotsAvailable(InvestmentActionClass _actionClass,
                               int _idInvestmentAccount,
                               const QDate& _date) const;

            /**
             * @brief Same as lotsAvailable, but excluding a transaction.
             *
             * @param _idTransaction ID of the transaction to exclude
             *
             * @see lotsAvailable()
             */
            Lots lotsAvailableExcluding(InvestmentActionClass _actionClass,
                                        int _idInvestmentAccount,
                                        const QDate& _date,
                                        int _idTransaction) const;

            /**
             * @brief Validate lots
             * @param _action The action (only supported actions are Sell, Transfer, ShortCover)
             * @param _idInvestmentAccount The investment account ID.
             * @param _lots For each lot number, the number of shares to be used.
             * @param[out] _error The error message if invalid.
             * @return If valid.
             *
             * Checks if the lots are valid for this action/investment account,
             * and if each has enough remaining space.
             */
            bool validateUsage(InvestmentAction _action,
                                  int _idInvestmentAccount,
                                  int _idTransaction,
                                  const Lots& _lots,
                                  const QDate& _date,
                                  QString& _error) const;

            Lots lotsForTransaction(InvestmentTransaction* _transaction) const;

            const LotUsage* lotsUsed(InvestmentTransaction* _transaction) const;
            const LotTransferSwap* lotsTransferred(InvestmentTransaction* _transaction) const;
            int lotNumber(InvestmentTransaction* _transaction) const;

            const Lot* lot(int _id) const;
            InvestmentTransaction* transactionForLot(int _id) const;

            static bool validateLotsCount(const Lots& _lots, const Amount& _amount);

            static InvestmentActionClass investmentActionClass(const InvestmentAction& _action);

        protected:
            void load(QXmlStreamReader& _reader) override;
            void save(QXmlStreamWriter& _writer) const override;

            void afterLoad() override;
            void unload() override;

        private:

            bool lotsHaveSameClass(const Lots& _lots) const;

            //In the future, separate by account? (QMap of QMap)
            //Contains everything.
            QMultiMap<PriorityDate, ILotAvailabilityCalculator*> m_available; //Ordered by transaction date AND order

            //QMultiMap<int, ILotAvailabilityCalculator*> m_usagesTransfersSwaps; //Ordered by account ID



            //Indexes by transaction ID
            QHash<int, Lot*> m_indexLots;
            QHash<int, LotSplit*> m_indexSplits;
            QHash<int, LotUsage*> m_indexUsages;
            QHash<int, LotTransferSwap*> m_indexTransfersSwaps;

            //Lot index
            QHash<int, Lot*> m_lots;         ///< 1st: lot id. second: lot

//            //QHash<int, Lot*> m_lots;         ///< 1st: lot id. second: lot
//            QHash<int, int> m_transactions; ///< 1st: transaction id, second: lot id
//            QHash<int, Lots> m_usages;      ///< 1st: transaction id, second: lots used (lots amounts are positive)
//            QHash<int, LotTransferSwap> m_transfers; ///< 1st: transaction id, second: transfered (lots amounts are positive)

            int m_nextId;

            static InvestmentLotsManager* m_instance;

            friend class Lot;

    };

}

#endif // INVESTMENTLOTSMANAGER_H
