#include "balances.h"
#include "../model/pricemanager.h"
#include "../model/currency.h"

namespace KLib
{
    Amount convert(const QString& _cur, const Amount& _a)
    {
        if (CurrencyManager::instance()->has(_cur))
        {
            return _a.toPrecision(CurrencyManager::instance()->get(_cur)->precision());
        }
        else
        {
            return _a;
        }
    }

    Amount Balances::inCurrency(const QString& _currency, const QDate& _date) const
    {
        Amount total;

        for (auto i = bal.begin(); i != bal.end(); ++i)
        {
            total += i.value() * PriceManager::instance()->rate(i.key(), _currency, _date);
        }

        return total;
    }

    Balances::Balances(const QString& _cur, const Amount& _a)
    {
        bal.insert(_cur, convert(_cur, _a));
    }

    Balances& Balances::operator+=(const Balances& _other)
    {
        for (auto i = _other.bal.begin(); i != _other.bal.end(); ++i)
        {
            add(i.key(), i.value());
        }

        return *this;
    }

    Balances& Balances::operator-=(const Balances& _other)
    {
        for (auto i = _other.bal.begin(); i != _other.bal.end(); ++i)
        {
            add(i.key(), -i.value());
        }

        return *this;
    }

    void Balances::add(const QString& _cur, const Amount& _a)
    {
        if (!bal.contains(_cur))
        {
            bal.insert(_cur, convert(_cur, _a));
        }
        else
        {
            bal[_cur] += _a; //convert(_cur, _a);
        }
    }

    bool Balances::operator==(const Balances& _other) const
    {
        for (auto i = _other.bal.begin(); i != _other.bal.end(); ++i)
        {
            if (!bal.contains(i.key()) || bal.value(i.key()) != i.value())
            {
                return false;
            }
        }

        for (auto i = bal.begin(); i != bal.end(); ++i)
        {
            if (!_other.bal.contains(i.key()))
            {
                return false;
            }
        }

        return true;
    }
}

