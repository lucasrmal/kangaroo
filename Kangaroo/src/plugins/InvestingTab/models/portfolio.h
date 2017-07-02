#ifndef PORTFOLIO_H
#define PORTFOLIO_H

#include <KangarooLib/amount.h>
#include <KangarooLib/model/transaction.h>
#include <QColor>

using KLib::Amount;

namespace KLib
{
    class Account;
    class Security;
    class Currency;
    class ExchangePair;
}

struct Position
{
        struct AccountPosition
        {
            Amount balance;
            Amount costBasis;
        };

        Position(int _idSecurity);

        Amount balance() const;
        Amount costBasis() const;

        Amount lastPrice() const;

        Amount marketValue() const   { return balance() * lastPrice(); }
        Amount profitLoss() const    { return marketValue() - costBasis(); }
        Amount costPerShare() const;
        double percProfitLoss() const;


        //        Amount dayGain() const { return numShares * change; }
//        double percChange() const { return openPrice == 0 ? 0
//                                                          : 100.0 * change.toDouble() / openPrice.toDouble(); }

        void update(int _idAccount);

        KLib::Security* security;
        KLib::Currency* currency;

        QHash<int, AccountPosition> relatedAccounts; ///< List of account IDs in this position
        double percOfPortfolio;


        //QString name;
        //QString symbol;
        //Amount lastPrice;
        //Amount openPrice;
        //Amount change;
};

class Portfolio : public QObject
{
    Q_OBJECT

    public:
        Portfolio(KLib::Account* _account, QObject* _parent = nullptr);

        Amount totalCostBasis() const           { return m_totalCostBasis; }
        Amount totalMarketValue() const         { return m_totalMarketValue; }
        Amount totalProfitLoss() const          { return m_totalMarketValue - m_totalCostBasis; }
        KLib::Currency* currency() const        { return m_currency; }

        double totalPercProfitLoss() const      { return m_totalCostBasis == 0
                                                            ? 0
                                                            : (-1.0+(m_totalMarketValue.toDouble()
                                                                     / m_totalCostBasis.toDouble())) * 100.0; }


        const QList<Position>& positions() { return m_positions; }

        /**
         * @brief Returns a list of all accounts that are under the portfolio tree, including closed ones.
         */
        const QSet<int>& allAccountsInTree() { return m_allAccountsInTree; }

        const Position& position(KLib::Security* _sec) const;
        const Position& position(int _idSecurity) const;
        int positionCount() const { return m_positions.count(); }

        int indexOf(KLib::Security* _sec) const;

        static QColor colorForAmount(const Amount& _a)
        {
            return _a > 0 ? QColor(Qt::darkGreen)
                          : _a < 0 ? QColor(Qt::red)
                                   : QColor(Qt::black);
        }

        static QString formatPercChange(double _perc);

        QString formatGainLoss(const Amount& _a);

        QString portfolioName() const;

    signals:
        void positionAdded(KLib::Security* _sec);
        void positionRemoved(KLib::Security* _sec, int _row);
        void positionDataChanged(KLib::Security* _sec);

        void portfolioSecurityModified(KLib::Security* _sec);
        void portfolioNameChanged();

        void securityPriceModified(KLib::Security* _sec);
        void parentCurrencyPriceModified();

    public slots:
        void onAccountAdded(KLib::Account* _account);
        void onAccountModified(KLib::Account* _account);
        void onAccountRemoved(KLib::Account* _account);

        void onSecurityModified(KLib::Security* _sec);
        void onTransactionDateChanged(KLib::Transaction* _tr, const QDate& _old);
        void onSplitChanged(const KLib::Transaction::Split& _split, KLib::Transaction*);

        void onLastRateModified(KLib::ExchangePair* _p);

    private:
        void rebuildIndexes();
        void updateCalculations();

        void addAccount(KLib::Account* _account);
        void removeAccount(KLib::Account* _account);

        KLib::Account* m_parentAccount;

        QList<Position> m_positions;
        QHash<int, int> m_accountIndex;
        QHash<int, int> m_securityIndex;

        QSet<int> m_allAccountsInTree;

        Amount m_totalCostBasis;
        Amount m_totalMarketValue;
        KLib::Currency* m_currency;
};



#endif // PORTFOLIO_H
