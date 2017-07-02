#include "portfolio.h"

#include <KangarooLib/model/account.h>
#include <KangarooLib/model/security.h>
#include <KangarooLib/model/pricemanager.h>
#include <KangarooLib/model/currency.h>
#include <KangarooLib/model/modelexception.h>
#include <KangarooLib/model/ledger.h>
#include <functional>
#include <QLocale>

using namespace KLib;

Position::Position(int _idSecurity) :
    security(SecurityManager::instance()->get(_idSecurity)),
    currency(CurrencyManager::instance()->get(security->currency())),
    percOfPortfolio(0)
{
}

Amount Position::balance() const
{
    Amount total(0, security->precision());

    for (const AccountPosition& pos : relatedAccounts)
    {
        total += pos.balance;
    }

    return total;
}

Amount Position::costBasis() const
{
    Amount total(0, currency->precision());

    for (const AccountPosition& pos : relatedAccounts)
    {
        total += pos.costBasis;
    }

    return total;
}

Amount Position::lastPrice() const
{
    return PriceManager::instance()->rate(security->id(), security->currency());
}

Amount Position::costPerShare() const
{
    Amount bal = balance();
    return bal == 0 ? 0 : costBasis() / bal;
}

void Position::update(int _idAccount)
{
    if (relatedAccounts.contains(_idAccount))
    {
        Ledger* l = LedgerManager::instance()->ledger(_idAccount);
        relatedAccounts[_idAccount].costBasis = l->costBasis();
        relatedAccounts[_idAccount].balance = l->balanceToday();
    }
}

double Position::percProfitLoss() const
{
    Amount cb = costBasis();
    Amount mkt = marketValue();

    if (cb == 0 && mkt == 0)
    {
        return 0.0;
    }
    else if (cb == 0)
    {
        return 100.0;
    }
    else
    {
        return (-1.0+(mkt.toDouble() / cb.toDouble()))*100.0;
    }
}

Portfolio::Portfolio(Account* _account, QObject* _parent) :
    QObject(_parent),
    m_parentAccount(_account),
    m_currency(CurrencyManager::instance()->get(_account->mainCurrency()))
{
    //Load everything
    if (m_parentAccount == Account::getTopLevel())
    {
        std::function<void(Account*)> loadRecursive;

        loadRecursive = [&loadRecursive, this] (Account* parent)
        {
            if (parent->type() == AccountType::TOPLEVEL ||
                Account::generalType(parent->type()) == AccountType::ASSET)
            {
                for (Account* a : parent->getChildren())
                {
                    if (a->type() == AccountType::INVESTMENT)
                    {
                        m_allAccountsInTree.insert(a->id());

                        if (a->isOpen())
                        {
                            addAccount(a);
                        }
                    }

                    loadRecursive(a);
                }
            }
        };

        loadRecursive(m_parentAccount);
    }
    else
    {
        for (Account* a : m_parentAccount->getChildren())
        {
            if (a->type() == AccountType::INVESTMENT)
            {
                m_allAccountsInTree.insert(a->id());

                if (a->isOpen())
                {
                    addAccount(a);
                }
            }


        }
    }

    updateCalculations();

    //Connections with model managers to update the positions as the data is modified
    connect(LedgerManager::instance(), &LedgerManager::splitAdded, this, &Portfolio::onSplitChanged);
    connect(LedgerManager::instance(), &LedgerManager::splitAmountChanged, this, &Portfolio::onSplitChanged);
    connect(LedgerManager::instance(), &LedgerManager::splitRemoved, this, &Portfolio::onSplitChanged);
    connect(LedgerManager::instance(), &LedgerManager::transactionDateChanged, this, &Portfolio::onTransactionDateChanged);

    connect(Account::getTopLevel(), &Account::accountAdded, this, &Portfolio::onAccountAdded);
    connect(Account::getTopLevel(), &Account::accountRemoved, this, &Portfolio::onAccountRemoved);
    connect(Account::getTopLevel(), &Account::accountModified, this, &Portfolio::onAccountModified);

    connect(SecurityManager::instance(), &SecurityManager::securityModified, this, &Portfolio::onSecurityModified);

    connect(PriceManager::instance(), &PriceManager::lastRateModified, this, &Portfolio::onLastRateModified);
}

void Portfolio::rebuildIndexes()
{
    m_accountIndex.clear();
    m_securityIndex.clear();

    for (int i = 0; i < m_positions.count(); ++i)
    {
        const Position& pos = m_positions[i];

        for (int id : pos.relatedAccounts.keys())
        {
            m_accountIndex[id] = i;
        }

        m_securityIndex[pos.security->id()] = i;
    }
}

void Portfolio::updateCalculations()
{
    m_totalCostBasis = 0;
    m_totalMarketValue = 0;

    for (const Position& p : m_positions)
    {
        double rate = p.currency->code() == m_currency->code() ? 1.0
                                                                   : PriceManager::instance()->rate(p.currency->code(),
                                                                                                    m_currency->code());
        m_totalCostBasis += rate * p.costBasis();
        m_totalMarketValue += rate * p.marketValue();
    }

    //Compute % of portfolio
    for (Position& p : m_positions)
    {
        p.percOfPortfolio = 100.0 * p.marketValue().toDouble() / m_totalMarketValue.toDouble();
    }
}

void Portfolio::addAccount(Account* _account)
{
    bool isNew = false;

    //Check if the position already exists
    if (!m_securityIndex.contains(_account->idSecurity()))
    {
        m_securityIndex[_account->idSecurity()] = m_positions.count();
        m_positions << Position(_account->idSecurity());
        isNew = true;
    }

    Position& pos = m_positions[m_securityIndex[_account->idSecurity()]];

    pos.relatedAccounts[_account->id()] = Position::AccountPosition{ _account->ledger()->balanceToday(),
                                                                     _account->ledger()->costBasis() };

    m_accountIndex[_account->id()] = m_securityIndex[_account->idSecurity()];

    if (isNew)
    {
        emit positionAdded(pos.security);
    }
    else
    {
        emit positionDataChanged(pos.security);
    }
}

void Portfolio::removeAccount(Account* _account)
{
    int idx = m_accountIndex.take(_account->id());
    m_positions[idx].relatedAccounts.remove(_account->id());

    //Check if no account left in position.
    if (m_positions[idx].relatedAccounts.isEmpty())
    {
        Position pos = m_positions.takeAt(idx);
        m_securityIndex.remove(_account->idSecurity());

        //Rebuild the indexes
        rebuildIndexes();

        emit positionRemoved(pos.security, idx);
    }
    else
    {
        emit positionDataChanged(m_positions[idx].security);
    }
}

const Position& Portfolio::position(Security* _sec) const
{
    return m_positions[m_securityIndex[_sec->id()]];
}

const Position& Portfolio::position(int _idSecurity) const
{
    return m_positions[m_securityIndex[_idSecurity]];
}

int Portfolio::indexOf(Security* _sec) const
{
    return m_securityIndex.value(_sec->id(), -1);
}

QString Portfolio::portfolioName() const
{
    if (m_parentAccount == Account::getTopLevel())
    {
        return tr("All Positions");
    }
    else
    {
        return m_parentAccount->name();
    }
}

QString Portfolio::formatGainLoss(const Amount& _a)
{
    QString s = m_currency->formatAmount(_a);
    if (_a > 0)
        s.prepend('+');
    return s;
}

QString Portfolio::formatPercChange(double _perc)
{
    QString s = QLocale().toString(_perc, 'f', 2);
    if (_perc > 0)
        s.prepend('+');
    return s + " %";
}

/****************************************** SLOTS ******************************************/

void Portfolio::onAccountAdded(Account* _account)
{
    if (_account->type() == AccountType::INVESTMENT
        && _account->isOpen()
        && (m_parentAccount == Account::getTopLevel()
            || _account->parent() == m_parentAccount))
    {
        addAccount(_account);
        m_allAccountsInTree.insert(_account->id());
    }
}

void Portfolio::onAccountModified(Account* _account)
{
    if (m_accountIndex.contains(_account->id()))
    {
        //Check if needs to be removed
        if (_account->type() != AccountType::INVESTMENT
            || !_account->isOpen()
            || (m_parentAccount != Account::getTopLevel()
                || _account->parent() != m_parentAccount))
        {
            removeAccount(_account);
        }
        //Check if account security changed
        else if (_account->idSecurity() != m_positions[m_accountIndex[_account->id()]].security->id())
        {
            removeAccount(_account);
            addAccount(_account);
        }

        if (_account == m_parentAccount) //Maybe we changed the name of the portfolio?
        {
            emit portfolioNameChanged();
        }

    }
    else //Check if it needs to be added
    {
        onAccountAdded(_account);
    }
}

void Portfolio::onAccountRemoved(Account* _account)
{
    if (m_accountIndex.contains(_account->id()))
    {
        m_allAccountsInTree.remove(_account->id());
        removeAccount(_account);
    }
}

void Portfolio::onSecurityModified(Security* _sec)
{
    if (m_securityIndex.contains(_sec->id()))
    {
        emit portfolioSecurityModified(_sec);
    }
}

void Portfolio::onSplitChanged(const Transaction::Split& _split, Transaction*)
{
    if (m_accountIndex.contains(_split.idAccount))
    {
        //Update the balance and the cost basis
        m_positions[m_accountIndex[_split.idAccount]].update(_split.idAccount);
        emit positionDataChanged(m_positions[m_accountIndex[_split.idAccount]].security);
    }
}

void Portfolio::onTransactionDateChanged(Transaction* _tr, const QDate& _old)
{
    //Only update if went to/from future.
    if ((_old > QDate::currentDate() && _tr->date() <= QDate::currentDate())
         || (_old <= QDate::currentDate() && _tr->date() > QDate::currentDate()))
    {
        //Update the balance and the cost basis
        for (auto s : _tr->splits())
        {
            if (m_accountIndex.contains(s.idAccount))
            {
                m_positions[m_accountIndex[s.idAccount]].update(s.idAccount);
                emit positionDataChanged(m_positions[m_accountIndex[s.idAccount]].security);
            }
        }
    }
}

void Portfolio::onLastRateModified(ExchangePair* _p)
{
    if (_p->isSecurity())
    {
        Security* from = _p->securityFrom();

        if (m_securityIndex.contains(from->id()))
        {
            updateCalculations();
            emit securityPriceModified(from);
        }
    }
    else if (_p->from() == m_parentAccount->mainCurrency()
             || _p->to() == m_parentAccount->mainCurrency())
    {
        updateCalculations();
        emit parentCurrencyPriceModified();
    }
}
