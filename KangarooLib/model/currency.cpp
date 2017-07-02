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

#include "currency.h"
#include "modelexception.h"
#include "account.h"
#include "security.h"
#include "pricemanager.h"
#include "../controller/io.h"
#include "../amount.h"

#include <QXmlStreamReader>
#include <QScriptEngine>

namespace KLib
{
    CurrencyManager* CurrencyManager::m_instance = new CurrencyManager();

    Currency::Currency()
        : m_trading(nullptr)
    {
    }

    void Currency::setName(const QString& _name)
    {
        m_name = _name;
        if (!onHoldToModify())
            emit modified();
    }

    void Currency::setCustomSymbol(const QString& _symbol)
    {
        m_symbol = _symbol;
        if (!onHoldToModify())
            emit modified();
    }

    void Currency::setCode(const QString& _code)
    {
        if (_code.length() != 3)
        {
            ModelException::throwException(tr("The length of the currency code must be 3."), this);
        }

        m_code = _code;
        if (!onHoldToModify())
            emit modified();
    }

    void Currency::setPrecision(unsigned int _precision)
    {
        m_precision = _precision;
        if (!onHoldToModify())
            emit modified();
    }

    bool canRemove_recursiveA(const QString& _code, Account* _a)
    {
        if (!_a)
        {
            return true;
        }
        else if (_a->mainCurrency() == _code)
        {
            return false;
        }
        else
        {
            for (Account* c : _a->getChildren())
            {
                if (!canRemove_recursiveA(_code, c))
                    return false;
            }
        }

        return true;
    }

    bool canDeleteS(const QString& _code)
    {
        for (Security* s : SecurityManager::instance()->securities())
        {
            if (s->currency() == _code)
                return false;
        }

        return true;
    }

    bool Currency::canRemove() const
    {
        return canDeleteS(code()) && canRemove_recursiveA(code(), Account::getTopLevel());
    }

    void Currency::load(QXmlStreamReader& _reader)
    {
        QXmlStreamAttributes attributes = _reader.attributes();

        m_name = IO::getAttribute("name", attributes);
        m_code = IO::getAttribute("code", attributes);
        m_symbol = IO::getAttribute("symbol", attributes);
        m_precision = IO::getOptAttribute("precision", attributes, 2).toInt();
    }

    void Currency::save(QXmlStreamWriter& _writer) const
    {
        _writer.writeEmptyElement(StdTags::CURRENCY);
        _writer.writeAttribute("name", m_name);
        _writer.writeAttribute("code", m_code);
        _writer.writeAttribute("symbol", m_symbol);
        _writer.writeAttribute("precision", QString::number(m_precision));
    }


    Account* Currency::tradingAccount() const
    {
        if (!m_trading)
            Account::getTopLevel()->createCurrencyTradingAccount(m_code);

        return m_trading;
    }

    QString Currency::formatAmount(const Amount& _amount) const
    {
        return symbol() + _amount.toPrecision(precision()).toString();
//        QLocale l;
//        return l.toCurrencyString(_amount.toDouble(), symbol());
    }

    Currency* CurrencyManager::add(const QString &_code,
                                   const QString& _name,
                                   const QString& _customSymbol,
                                   unsigned int _precision)
    {
        if (_code.length() != 3)
        {
            ModelException::throwException(tr("The length of the currency code must be 3."), this);
        }
        else if (m_index.contains(_code))
        {
            ModelException::throwException(tr("A currency (%2) with code %1 already exists.").arg(_code).arg(get(_code)->name()), this);
        }

        Currency* o = new Currency();
        o->m_name = _name;
        o->m_code = _code;
        o->m_symbol = _customSymbol;
        o->m_precision = _precision;

        m_index[_code] = m_currencies.size();
        m_currencies.append(o);
        emit modified();
        emit currencyAdded(o);

        connect(o, SIGNAL(modified()), this, SLOT(onModified()));

        return o;
    }

    void CurrencyManager::onModified()
    {
        Currency* o = static_cast<Currency*>(sender());

        if (o)
        {
            emit currencyModified(o);
            emit modified();
        }
    }

    bool CurrencyManager::has(const QString& _code) const
    {
        return m_index.contains(_code);
    }

    Currency* CurrencyManager::get(const QString& _code) const
    {
        if (m_index.contains(_code))
        {
            return at(m_index[_code]);
        }
        else
        {
            ModelException::throwException(tr("The currency %1 does not exists.").arg(_code), this);
            return nullptr;
        }
    }

    Currency* CurrencyManager::get(const QString& _code, bool _add)
    {
        if (m_index.contains(_code))
        {
            return at(m_index[_code]);
        }
        else if (_add)
        {
            return add(_code, "");
        }
        else
        {
            ModelException::throwException(tr("The currency %1 does not exists.").arg(_code), this);
            return nullptr;
        }
    }

    Currency* CurrencyManager::at(int _i) const
    {
        if (_i < 0 || _i >= m_currencies.count())
        {
            ModelException::throwException(tr("Invalid index %1").arg(_i), this);
            return nullptr;
        }

        return m_currencies[_i];
    }

    void CurrencyManager::remove(const QString& _code)
    {
        if (m_index.contains(_code))
        {
            int idx = m_index[_code];
            Currency* c = m_currencies[idx];

            if (!c->canRemove())
            {
                ModelException::throwException(tr("Impossible to remove the currency, "
                                                  "as it is used in at least one account."), c);
            }

            //Remove the price pair
            PriceManager::instance()->removeAll(c->code());

            m_currencies.removeAt(idx);
            m_index.remove(_code);

            //Update the index
            for (; idx < m_currencies.count(); ++idx)
            {
                m_index[m_currencies[idx]->code()] = idx;
            }

            emit currencyRemoved(c);
            emit modified();
            c->deleteLater();
        }
    }

    void CurrencyManager::load(QXmlStreamReader& _reader)
    {
        unload();

        // While not at end
        while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::CURRENCY_MGR))
        {
            if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::CURRENCY)
            {
                Currency* o = new Currency();
                o->load(_reader);
                m_index[o->code()] = m_currencies.size();
                m_currencies.append(o);

                connect(o, SIGNAL(modified()), this, SLOT(onModified()));
            }

            _reader.readNext();
        }

        //Check if no currencies have been created
        if (count() == 0)
        {
            addDefaultCurrency();
        }

    }

    void CurrencyManager::save(QXmlStreamWriter &_writer) const
    {
        for (Currency* o : m_currencies)
        {
            o->save(_writer);
        }
    }

    void CurrencyManager::loadNew()
    {
        unload();
        addDefaultCurrency();
    }

    void CurrencyManager::unload()
    {
        for (Currency* i : m_currencies)
        {
            delete i;
        }

        m_currencies.clear();
        m_index.clear();
    }

    CurrencyManager* CurrencyManager::instance()
    {
        if (!m_instance->count())
        {
            m_instance->addDefaultCurrency();
        }

        return m_instance;
    }

    void CurrencyManager::addDefaultCurrency()
    {
        add(Constants::DEFAULT_CURRENCY_CODE, Constants::DEFAULT_CURRENCY_NAME);
    }

    QList<CurrencyManager::WorldCurrency> CurrencyManager::worldCurrencies()
    {
        QList<WorldCurrency> list;

        list << WorldCurrency(tr("Albanian lek"), "ALL", "L");
        list << WorldCurrency(tr("Afghan afghani"), "AFN", "؋");
        list << WorldCurrency(tr("Algerian dinar"), "DZN", "د.ج");
        list << WorldCurrency(tr("Angolan kwanza"), "AOA", "Kz");
        list << WorldCurrency(tr("Argentine peso"), "ARS", "$");
        list << WorldCurrency(tr("Armenian dram"), "AMD", "");
        list << WorldCurrency(tr("Aruban florin"), "AWG", "ƒ");
        list << WorldCurrency(tr("Australian dollar"), "AUD", "$");
        list << WorldCurrency(tr("Azerbaijani manat"), "AZN", "");
        list << WorldCurrency(tr("Bahamian dollar"), "BDS", "$");
        list << WorldCurrency(tr("Bahraini dinar"), "BHD", "", 3);
        list << WorldCurrency(tr("Bangladeshi taka"), "BDT", "৳");
        list << WorldCurrency(tr("Barbadian dollar"), "BBD", "$");
        list << WorldCurrency(tr("Belarusian ruble"), "BYR", "Br");
        list << WorldCurrency(tr("Belize dollar"), "BZD", "$");
        list << WorldCurrency(tr("Bermudian dollar"), "BMD", "$");
        list << WorldCurrency(tr("Bhutanese ngultrum"), "BTN", "Nu.");
        list << WorldCurrency(tr("Bolivian boliviano"), "BOB", "Bs.");
        list << WorldCurrency(tr("Bosnia and Herzegovina convertible mark"), "BAM", "KM");
        list << WorldCurrency(tr("Botswana pula"), "BWP", "P");
        list << WorldCurrency(tr("Brazilian real"), "BRL", "R$");
        list << WorldCurrency(tr("British Pound"), "GBP", "£");
        list << WorldCurrency(tr("Brunei dollar"), "BND", "$");
        list << WorldCurrency(tr("Bulgarian lev"), "BGN", "лв");
        list << WorldCurrency(tr("Burmese kyat"), "MKK", "Ks");
        list << WorldCurrency(tr("Burundian franc"), "BIF", "Fr");
        list << WorldCurrency(tr("Cambodian riel"), "KHR", "៛");
        list << WorldCurrency(tr("Canadian Dollar"), "CAD", "$");
        list << WorldCurrency(tr("Cape Verdean escudo"), "CVE", "Esc");
        list << WorldCurrency(tr("Cayman Islands dollar"), "KYD", "$");
        list << WorldCurrency(tr("Central African CFA franc"), "XAF", "Fr");
        list << WorldCurrency(tr("CFP franc"), "XPF", "Fr");
        list << WorldCurrency(tr("Chilean peso"), "CLP", "$");
        list << WorldCurrency(tr("Chinese yuan"), "CNY", "¥");
        list << WorldCurrency(tr("Colombian peso"), "COP", "$");
        list << WorldCurrency(tr("Comorian franc"), "KMF", "Fr");
        list << WorldCurrency(tr("Congolese franc"), "CFD", "Fr");
        list << WorldCurrency(tr("Costa Rican colón"), "CRC", "₡");
        list << WorldCurrency(tr("Croatian kuna"), "HRK", "kn");
        list << WorldCurrency(tr("Cuban convertible peso"), "CUC", "$");
        list << WorldCurrency(tr("Cuban peso"), "CUP", "$");
        list << WorldCurrency(tr("Czech koruna"), "CZK", "Kč");
        list << WorldCurrency(tr("Danish krone"), "DKK", "kr");
        list << WorldCurrency(tr("Djiboutian franc"), "DJF", "Fr");
        list << WorldCurrency(tr("Dominican peso"), "DOP", "$");
        list << WorldCurrency(tr("East Caribbean dollar"), "XCD", "$");
        list << WorldCurrency(tr("Egyptian pound"), "EGP", "£");
        list << WorldCurrency(tr("Eritrean nakfa"), "ERN", "Nfk");
        list << WorldCurrency(tr("Ethiopian birr"), "ETB", "Br");
        list << WorldCurrency(tr("Euro"), "EUR", "€");
        list << WorldCurrency(tr("Falkland Islands pound"), "FKP", "£");
        list << WorldCurrency(tr("Fijian dollar"), "FJD", "$");
        list << WorldCurrency(tr("Gambian dalasi"), "GMD", "D");
        list << WorldCurrency(tr("Georgian lari"), "GEL", "ლ");
        list << WorldCurrency(tr("Ghana cedi"), "GHS", "₵");
        list << WorldCurrency(tr("Gibraltar pound"), "GIP", "£");
        list << WorldCurrency(tr("Guatemalan quetzal"), "GTQ", "Q");
        list << WorldCurrency(tr("Guernsey pound"), "GGP", "£");
        list << WorldCurrency(tr("Guinean franc"), "GNF", "Fr");
        list << WorldCurrency(tr("Guyanese dollar"), "GYD", "$");
        list << WorldCurrency(tr("Haitian gourde"), "HTG", "G");
        list << WorldCurrency(tr("Honduran lempira"), "HNL", "L");
        list << WorldCurrency(tr("Hong Kong dollar"), "HKD", "$");
        list << WorldCurrency(tr("Hungarian forint"), "HUF", "Ft");
        list << WorldCurrency(tr("Icelandic króna"), "ISK", "kr");
        list << WorldCurrency(tr("Indian rupee"), "INR", "₹");
        list << WorldCurrency(tr("Indonesian rupiah"), "IDR", "Rp");
        list << WorldCurrency(tr("Iranian rial"), "IRR", "﷼");
        list << WorldCurrency(tr("Iraqi dinar"), "IQD", "",  3);
        list << WorldCurrency(tr("Israeli new shekel"), "ILS", "₪");
        list << WorldCurrency(tr("Jamaican dollar"), "JMD", "$");
        list << WorldCurrency(tr("Japanese yen"), "JPY", "¥");
        list << WorldCurrency(tr("Jersey pound"), "JEP", "£");
        list << WorldCurrency(tr("Jordanian dinar"), "JOD", "د.ا");
        list << WorldCurrency(tr("Kazakhstani tenge"), "KZT", "₸");
        list << WorldCurrency(tr("Kenyan shilling"), "KES", "Sh");
        list << WorldCurrency(tr("Kuwaiti dinar"), "KWD", "د.ك");
        list << WorldCurrency(tr("Kyrgyzstani som"), "KGS", "лв");
        list << WorldCurrency(tr("Lao kip"), "LAK", "₭");
        list << WorldCurrency(tr("Lebanese pound"), "LBP", "");
        list << WorldCurrency(tr("Lesotho loti"), "LSL", "L");
        list << WorldCurrency(tr("Liberian dollar"), "LRD", "$");
        list << WorldCurrency(tr("Libyan dinar"), "LYD", "");
        list << WorldCurrency(tr("Lithuanian litas"), "LTL", "Lt");
        list << WorldCurrency(tr("Macanese pataca"), "MOP", "P");
        list << WorldCurrency(tr("Macedonian denar"), "MKD", "ден");
        list << WorldCurrency(tr("Malagasy ariary"), "MGA", "Ar");
        list << WorldCurrency(tr("Malawian kwacha"), "MWK", "MK");
        list << WorldCurrency(tr("Malaysian ringgit"), "MYR", "RM");
        list << WorldCurrency(tr("Maldivian rufiyaa"), "MVR", "");
        list << WorldCurrency(tr("Manx pound"), "IMP", "£");
        list << WorldCurrency(tr("Mauritanian ouguiya"), "MRO", "UM");
        list << WorldCurrency(tr("Mauritian rupee"), "MUR", "Rs");
        list << WorldCurrency(tr("Mexican peso"), "MXN", "$");
        list << WorldCurrency(tr("Moldovan leu"), "MDL", "L");
        list << WorldCurrency(tr("Mongolian tögrög"), "MNT", "₮");
        list << WorldCurrency(tr("Moroccan dirham"), "MAD", "د.م.");
        list << WorldCurrency(tr("Mozambican metical"), "MZN", "MT");
        list << WorldCurrency(tr("Namibian dollar"), "NAD", "$");
        list << WorldCurrency(tr("Nepalese rupee"), "NPR", "Rs");
        list << WorldCurrency(tr("Netherlands Antillean guilder"), "ANG", "ƒ");
        list << WorldCurrency(tr("New Taiwan dollar"), "TWD", "$");
        list << WorldCurrency(tr("New Zealand dollar"), "NZD", "$");
        list << WorldCurrency(tr("Nicaraguan córdoba"), "NIO", "C$");
        list << WorldCurrency(tr("Nigerian naira"), "NGN", "₦");
        list << WorldCurrency(tr("North Korean won"), "KPW", "₩");
        list << WorldCurrency(tr("Norwegian krone"), "NOK", "kr");
        list << WorldCurrency(tr("Omani rial"), "OMR", "ر.ع.");
        list << WorldCurrency(tr("Pakistani rupee"), "PKR", "Rs");
        list << WorldCurrency(tr("Panamanian balboa"), "PAB", "B/.");
        list << WorldCurrency(tr("Papua New Guinean kina"), "PGK", "K");
        list << WorldCurrency(tr("Paraguayan guaraní"), "PYG", "₲");
        list << WorldCurrency(tr("Peruvian nuevo sol"), "PEN", "S/.");
        list << WorldCurrency(tr("Philippine peso"), "PHP", "₱");
        list << WorldCurrency(tr("Polish złoty"), "PLN", "zł");
        list << WorldCurrency(tr("Qatari riyal"), "QAR", "ر.ق");
        list << WorldCurrency(tr("Romanian leu"), "RON", "lei");
        list << WorldCurrency(tr("Russian ruble"), "RUB", "р.");
        list << WorldCurrency(tr("Rwandan franc"), "RWF", "Fr");
        list << WorldCurrency(tr("Saint Helena pound"), "SHP", "£");
        list << WorldCurrency(tr("Samoan tālā"), "WST", "T");
        list << WorldCurrency(tr("Saudi riyal"), "SAR", "ر.س");
        list << WorldCurrency(tr("Serbian dinar"), "RSD", "дин");
        list << WorldCurrency(tr("Seychellois rupee"), "SCR", "Rs");
        list << WorldCurrency(tr("Sierra Leonean leone"), "SLL", "Le");
        list << WorldCurrency(tr("Singapore dollar"), "SGD", "$");
        list << WorldCurrency(tr("Solomon Islands dollar"), "SBD", "$");
        list << WorldCurrency(tr("Somali shilling"), "SOS", "Sh");
        list << WorldCurrency(tr("South African rand"), "ZAR", "R");
        list << WorldCurrency(tr("South Korean won"), "KRW", "₩");
        list << WorldCurrency(tr("South Sudanese pound"), "SSP", "£");
        list << WorldCurrency(tr("Sri Lankan rupee"), "LKR", "Rs");
        list << WorldCurrency(tr("Sudanese pound"), "SDG", "£");
        list << WorldCurrency(tr("Surinamese dollar"), "SRD", "$");
        list << WorldCurrency(tr("Swazi lilangeni"), "SZL", "L");
        list << WorldCurrency(tr("Swedish krona"), "SEK", "kr");
        list << WorldCurrency(tr("Swiss franc"), "CHF", "Fr");
        list << WorldCurrency(tr("Syrian pound"), "SYP", "£");
        list << WorldCurrency(tr("São Tomé and Príncipe dobra"), "STD", "Db");
        list << WorldCurrency(tr("Tajikistani somoni"), "TJS", "SM");
        list << WorldCurrency(tr("Tanzanian shilling"), "TZS", "Sh");
        list << WorldCurrency(tr("Thai baht"), "THB", "฿");
        list << WorldCurrency(tr("Tongan paʻanga"), "TOP", "T$");
        list << WorldCurrency(tr("Transnistrian ruble"), "PRB", "p.");
        list << WorldCurrency(tr("Trinidad and Tobago dollar"), "TTD", "$");
        list << WorldCurrency(tr("Tunisian dinar"), "TND", "د.ت");
        list << WorldCurrency(tr("Turkish lira"), "TRY", "");
        list << WorldCurrency(tr("Turkmenistan manat"), "TMT", "m");
        list << WorldCurrency(tr("Ugandan shilling"), "UGX", "Sh");
        list << WorldCurrency(tr("Ukrainian hryvnia"), "UAH", "₴");
        list << WorldCurrency(tr("United Arab Emirates dirham"), "UAE", "د.إ");
        list << WorldCurrency(tr("United States Dollar"), "USD", "$");
        list << WorldCurrency(tr("Uruguayan peso"), "UYU", "$");
        list << WorldCurrency(tr("Uzbekistani som"), "UZS", "лв");
        list << WorldCurrency(tr("Vanuatu vatu"), "VUV", "Vt");
        list << WorldCurrency(tr("Venezuelan bolívar"), "VEF", "Bs F");
        list << WorldCurrency(tr("Vietnamese đồng"), "VND", "₫");
        list << WorldCurrency(tr("West African CFA franc"), "XOF", "Fr");
        list << WorldCurrency(tr("Yemeni rial"), "YER", "﷼");
        list << WorldCurrency(tr("Zambian kwacha"), "ZMV", "ZK");

        return list;
    }

}
