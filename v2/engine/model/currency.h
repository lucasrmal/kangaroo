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

#ifndef CURRENCY_H
#define CURRENCY_H

#include <string>
#include <string_view>
#include <unordered_map>

#include "model/account.h"
#include "model/stored.h"
#include "model/types/decimal-number.h"

namespace KLib
{
    class Currency : public IStored {
        public:

            Currency();

            std::string name() const   { return m_name; }
            std::string code() const   { return m_code; }
            std::string customSymbol() const { return m_symbol; }
            std::string symbol() const { return m_symbol.isEmpty() ? m_code : m_symbol; }
            unsigned int precision() const { return m_precision; }

            void setName(const std::string& _name);
            void setCustomSymbol(const std::string& _symbol);
            void setPrecision(unsigned int _precision);

            bool canRemove() const;

            Account* tradingAccount() const;

            std::string formatAmount(const Amount& _amount) const;


            static const std::string_view kDefaultCurrencyCode;
            static const std::string_view kDefaultCurrencyName;


        protected:
            void setCode(const std::string& _code);

            void load(QXmlStreamReader& _reader) override;
            void save(QXmlStreamWriter& _writer) const override;

        private:
            std::string m_code;
            std::string m_name;
            std::string m_symbol;
            unsigned int m_precision;

            Account* m_trading;

            friend class CurrencyManager;
            friend class Account;
    };

    class CurrencyManager : public IStored
    {
        public:

            struct WorldCurrency
            {
                WorldCurrency() : precision(2) {}
                WorldCurrency(const std::string _name,
                              const std::string _code,
                              const std::string _symbol,
                              unsigned int _precision = 2) :
                       name(_name),
                       code(_code),
                       symbol(_symbol),
                       precision(_precision) {}

                std::string name;
                std::string code;
                std::string symbol;
                unsigned int precision;
            };

            Currency* add(const std::string& _code,
                                            const std::string& _name,
                                            const std::string& _customSymbol = std::string(),
                                            unsigned int _precision = 2);
            Currency* get(const std::string& _code) const;
            Currency* get(const std::string& _code, bool _add);

            bool has(const std::string& _code) const;

            Currency* at(int _i) const;
            void remove(const std::string &_code);

            const std::vector<Currency*>& currencies() const { return m_currencies; }

            int count() const { return m_currencies.size(); }

            static CurrencyManager* instance();

            static std::vector<WorldCurrency> worldCurrencies();

        protected:
            void load(QXmlStreamReader& _reader) override;
            void save(QXmlStreamWriter& _writer) const override;
            void loadNew() override;
            void unload() override;

        private:
            void addDefaultCurrency();

            std::unordered_map<std::string, int> m_index;
            std::vector<Currency*> m_currencies;

            static CurrencyManager* m_instance;
    };

}

#endif // CURRENCY_H
