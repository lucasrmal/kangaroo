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

#ifndef INVESTMENTTRANSACTION_H
#define INVESTMENTTRANSACTION_H

//Need to declare these before!

namespace KLib {

enum class InvestmentAction
{
  Buy             = 1,
  Sell            = 2,
  //Add,            //Not sure if we need Add and Remove. Can always do a buy from income, sell to expense (gift?), or sell with cost 0.
  //Remove,
  ShortSell       = 5,
  ShortCover      = 6,
  Transfer        = 10,
  Swap            = 11,
  Spinoff         = 12,
  StockSplit      = 20,
  Dividend        = 30,
  StockDividend   = 31,       ///< Increases the number of shares. May be included in tax reports. Cost basis stays the same.
  Distribution    = 32,
  ReinvestDiv     = 40,
  ReinvestDistrib = 41,
  UndistributedCapitalGain    = 50,   ///< Increase in cost basis, taxed as capital gain. Contains gain and tax components
  CostBasisAdjustment         = 51,
  Fee = 60,
  //Aquisition, Spinoff, security change, ...
  Invalid = -1

  // should we include MiscIncome, MiscExpense, InterestExpense, InterestIncome, ReinvestedInterest
};

enum class InvestmentSplitType
{
  CostProceeds,           ///< Cost of investment bought, sales proceeds
  Investment,             ///< The investment itself (NOT for Transfer)
  InvestmentTo,           ///< The destination investment account for Transfer/Swap/Spinoff
  InvestmentFrom,         ///< The source investment account for Transfer/Swap
  DistributionSource,     ///< Source account of the dist (typically is Income account)
  DistributionDest,       ///< Account in which distribution deposited
  CashInLieu,             ///< Cash instead of fractional shares. For ReinvestDiv/ReinvestDist
  Fee,
  GainLoss,
  Tax,
  Trading
};

enum class DistribType
{
  ReturnOfCapital,            ///< Reduction in cost basis. Canadian equivalent: reduction in ACB
  CapitalGain,                ///< Taxed as capital gain. Does not affect cost basis
  Other,                      ///< Anything else. Usually taxed as regular income levels.
};
}

unsigned int qHash(KLib::InvestmentSplitType _key, unsigned int seed = 0);
unsigned int qHash(KLib::InvestmentAction _key, unsigned int seed = 0);
unsigned int qHash(KLib::DistribType _key, unsigned int seed = 0);

#include "transaction.h"


namespace KLib
{

/**
     * @brief DistribComposition
     *
     * Hash key: type.
     * Hash value: percentage. Has to add up to 100.
     */
typedef QHash<DistribType, Amount> DistribComposition;

typedef QHash<int, Amount> Lots;

typedef QPair<int, int> SplitFraction;

/**
     * @brief The InvestmentTransaction class
     *
     * The InvestmentTransaction class is a special type of transaction that handles different investment actions.
     *
     * It keeps track of which accounts are commisions, capital gains (ex:
     * cap gain distribution, foreign business income, etc.), dividends, withholding tax and others.
     *
     * ******* Before calling any makeGain... an investment account MUST be set. It cannot be changed later.
     *
     * Also keeps track of cost basis, share lots, distribution composition, price per share
     *
     * Need to look at different currencies too...
     *
     * Different ways to compute cost basis: average, FIFO, specific lots
     *
     * "If and when the investment's cost basis falls to zero, any cash distribution
     * becomes immediately taxable, rather that being deferred until the sale of the security"
     *
     * This could be implemented at the report level.
     *
     */
class InvestmentTransaction : public Transaction
{
    Q_OBJECT

  public:
    explicit InvestmentTransaction();

    Transaction* copyTo(int _idTo = Constants::NO_ID) const override;

    InvestmentAction action() const { return m_action; }

    const DistribComposition& distribComposition() const { return m_distribComposition; }

    Amount pricePerShare() const { return m_pricePerShare; }

    /**
             * @brief Net price per share: price per share after removing/adding the fee.
             *
             * Buy/Short Cover/Reinvest: Net Price per Share = PricePerShare + Fee/ShareCount
             * Sell/Short Sell: Net Price per Share = PricePerShare - Fee/ShareCount
             */
    Amount netPricePerShare() const;

    /**
             * @brief Returns the number of shares affected by this transaction
             *
             * The type must be Buy, Sell, ShortSell, ShortCover, Transfer,
             * Swap, StockDividend, ReinvestDiv or ReinvestDistrib.
             *
             * Otherwise returns 0.
             */
    Amount shareCount() const;

    Amount basisAdjustment() const { return m_basisAdjustment; }
    Amount taxPaid() const;
    Amount fee() const;
    Amount gainLoss() const;

    const Lots& lots() const { return m_lots; }

    /**
             * @brief Returns the investment account id related to this transaction
             *
             * If Transfer transaction: returns the FROM part of the transfer.
             */
    int idInvestmentAccount() const;

    /**
             * @brief Returns the investment account id related to this transaction
             *
             * If Transfer transaction: returns the TO part of the transfer, otherwise returns Constants::NO_ID
             */
    int idInvestmentToAccount() const;

    /**
             * @brief Returns the transfer account id related to this transaction.
             *
             * For Transfers, Swaps, this is the TO part of the transfer.
             * For Buy, Sell, ShortSell, ShortCover, this is the CostProceeds account.
             * For Dividend, Distribution, ReinvestDiv, ReinvestDistrib, this is the DistributionSource
             * For others, returns Constant::NO_ID.
             *
             */
    int idTransferAccount() const;

    const Split& splitFor(InvestmentSplitType _type) const;
    bool hasSplitFor(InvestmentSplitType _type) const;

    const SplitFraction& splitFraction() const { return m_splitFraction; }

    void setDate(const QDate& _date) override;


    /**
             * For each split type in the transaction, returns its index in the split list.
             */
    const QHash<InvestmentSplitType, int>& splitTypes() const { return m_types; }

    /**
             * @brief Do not use directly this method, instead the makeX methods must be used.
             *
             * Calling this method WILL throw an exception.
             */
    void setSplits(const QList<KLib::Transaction::Split>& _splits) override;

    /**
     * @brief Changes the transaction to be a buy or a sell.
     * @param _pricePerShare Price per share
     * @param _action The action (Buy, Sell, Short Sell, Short Cover     ONLY)
     * @param _splits The splits
     * @param _types The corresponding types (MUST have _types.length() = _splits.length()).
     * @param _lots The lot numbers and quantities sold in this transaction (ONLY IF Sell, see details)
     *
     * We must have _pricePerShare * number of shares + other credits = other debits.
     *
     * A lot number will be affected to the transaction if it's a buy or a short sell.
     *
     * Required splits:
     *  - CostProceeds (source of funds/destination for proceeds)
     *  - Investment (number of shares)
     *  - Trading (both for investment and for cash)
     *
     * Optional splits:
     *  - Tax
     *  - Commission
     *  - GainLoss (ONLY IF Sell/Cover Short)
     *
     * <b>Lot Numbers</b>
     *
     * These are the lot numbers corresponding to this sell/cover. If omitted, LIFO/ACB is assumed. For
     * future sales, the part of these lots sold will be marked as SOLD in the capital gain estimator.
     * For each lot number, the corresponding amount indicates the number of shares corresponding
     * to this lot. The total number of shares in the lots and of shares sold must match.
     *
     * Note: if this transaction is already a buy/short sell, but it is modified (still a buy), it will
     * keep the same lot number.
     */
    void makeBuySellFee(InvestmentAction _action,
                        const Amount& _pricePerShare,
                        const QList<Split>& _splits,
                        const QList<InvestmentSplitType>& _types,
                        const Lots& _lots = Lots());

    /**
             * @brief Changes the transaction to be a Transfer or a Swap.
             * @param _action The action (Transfer or Swap ONLY)
             * @param _splits The splits
             * @param _types The corresponding types (MUST have _types.length() = _splits.length()).
             * @param _lots The lot numbers and quantities sold in this transaction
             *
             * Required splits:
             *  - InvestmentTo
             *  - InvestmentFrom
             *  - Trading (ONLY IF Swap/Spinoff)
             *
             * <b>Lot Numbers</b>
             *
             * These are the lot numbers corresponding to the transfered shares. If omitted, LIFO/ACB is assumed. For
             * future sales, the part of these lots transfered will not be available in the current account, but available
             * in the transfer account in the capital gain estimator.
             * For each lot number, the corresponding amount indicates the number of shares corresponding
             * to this lot. The total number of shares in the lots and of shares sold must match.
             */
    void makeTransferSwap(InvestmentAction _action,
                          const QList<Split>& _splits,
                          const QList<InvestmentSplitType>& _types,
                          const Lots& _lots = Lots());

    /**
             * @brief Changes the transaction to be a Spinoff.
             * @param _splits The splits
             * @param _types The corresponding types (MUST have _types.length() = _splits.length()).
             * @param _lots The lot numbers and quantities swapped in this transaction
             *
             * Required splits:
             *  - Investment
             *  - InvestmentTo
             *  - Trading
             *
             * <b>Lot Numbers</b>
             *
             * These are the lot numbers corresponding to the transfered shares. If omitted, LIFO/ACB is assumed. For
             * future sales, the part of these lots transfered will not be available in the current account, but available
             * in the transfer account in the capital gain estimator.
             * For each lot number, the corresponding amount indicates the number of shares corresponding
             * to this lot. The total number of shares in the lots and of shares sold must match.
             */
    void makeSpinoff(const QList<Split>& _splits,
                     const QList<InvestmentSplitType>& _types,
                     const Lots& _lots = Lots());

    /**
             * @brief Changes to transaction to be a stock split
             * @param _idInvestmentAccount The investment account related to this split
             * @param _new The ratio of new shares
             * @param _old The ratio of old shares
             *
             * For example, a 2:1 split would be _new=2, _old=1
             */
    void makeSplit(int _idInvestmentAccount, const SplitFraction& _fraction);

    /**
             * @brief Changes the transaction to be a reinvested dividend or distribution
             * @param _action The action (ReinvestDiv or ReinvestDist ONLY)
             * @param _pricePerShare Price per share
             * @param _splits The splits
             * @param _types The corresponding types (MUST have _types.length() = _splits.length()).
             * @param _composition The composition in percentage (ONLY IF Distribution, in which case MUST add up to 100)
             *
             * Required splits:
             *  - DistributionSource
             *  - Investment (number of shares)
             *  - Trading (both for investment and for cash)
             *
             * Optional splits:
             *  - Tax
             *  - Commission
             *  - CashInLieu
             *
             */
    void makeReinvestedDivDist(InvestmentAction _action,
                               const Amount& _pricePerShare,
                               const QList<Split>& _splits,
                               const QList<InvestmentSplitType>& _types,
                               const DistribComposition& _composition = DistribComposition());

    /**
             * @brief Changes the transaction to be a dividend or a distribution
             * @param _action The action (Dividend or Distribution ONLY)
             * @param _idInvestmentAccount The investment account related to this distribution
             * @param _splits The splits
             * @param _types The corresponding types (MUST have _types.length() = _splits.length()).
             * @param _composition The composition in percentage (ONLY IF Distribution, in which case MUST add up to 100)
             *
             * Required splits:
             *  - DistributionSource
             *  - DistributionDestination
             *
             * Optional splits:
             *  - Trading (if currency exchange)
             *
             */
    void makeDivDist(InvestmentAction _action,
                     int _idInvestmentAccount,
                     const QList<Split>& _splits,
                     const QList<InvestmentSplitType>& _types,
                     const DistribComposition& _composition = DistribComposition());

    void makeCostBasisAdjustment(int _idInvestmentAccount,
                                 const Amount& _adjustment);

    void makeUndistributedCapitalGain(int _idInvestmentAccount,
                                      const Amount& _capitalGain,
                                      const Amount& _taxPaid);

    /**
             * @brief Sets the composition of the distribution.
             * @param _composition The composition in percentage (MUST add up to 100)
             *
             * WILL throw an exception if the action is not Distribution.
             */
    void setDistribComposition(const DistribComposition& _composition);

    //QList<Split> splitsFor(InvestmentSplitType _type);

    static QString actionToString(InvestmentAction _action);
    static QString distribTypeToString(DistribType _type);

    static Amount balanceAfterSplit(const Amount& _balanceBefore, const SplitFraction& _fraction);

  signals:
    void stockSplitAmountChanged();
    void investmentActionChanged(InvestmentAction _previous);

  protected:
    void load(QXmlStreamReader& _reader) override;
    void save(QXmlStreamWriter& _writer) const override;

    void copyTo(Transaction* _other) const override;

  private:
    InvestmentAction m_action;          ///< Action
    Amount           m_pricePerShare;   ///< Price per share
    Lots             m_lots;

    QHash<InvestmentSplitType, int> m_types;

    DistribComposition m_distribComposition; ///< Composition of a distribution. Adds up to the distribution split.


    //For a split
    SplitFraction m_splitFraction; ///< First: new, second: old.

    //For cost basis adjustment, undistributed gain
    Amount m_basisAdjustment;
    Amount m_taxPaid;

    void addToInvestmentLotsManager();

    void checkSplits(const QList<Split>& _splits,
                     const QList<InvestmentSplitType>& _types,
                     const QSet<InvestmentSplitType>& _required,
                     const QSet<InvestmentSplitType>& _optional = QSet<InvestmentSplitType>(),
                     InvestmentAction _action = InvestmentAction::Invalid);

    void checkAction(InvestmentAction _action, const QSet<InvestmentAction>& _allowed);

    void checkIdInvestmentAccount(int _idInvestmentAccount);

    void checkDistribComposition(InvestmentAction _action, const DistribComposition& _composition);

    void setTypes(const QList<InvestmentSplitType>& _types);

    void addAnchorSplit(int _idInvestmentAccount);

    void emitSplitSignals(const QList<Split>& _old, const QList<Split>& _new);

    friend class TransactionManager;
    friend class LedgerManager;
    friend class InvestmentLotsManager;
};

}

#endif // INVESTMENTTRANSACTION_H
