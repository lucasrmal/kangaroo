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

#include "security.h"
#include "account.h"
#include "currency.h"
#include "modelexception.h"
#include "picturemanager.h"
#include "pricemanager.h"
#include "institution.h"
#include "../controller/io.h"
#include <QXmlStreamReader>
#include <functional>

#define CHECK_AND_SET(member, param) if (param != member) { \
                                        member = param; \
                                         if (!onHoldToModify()) \
                                            emit modified(); \
                                     }

namespace KLib
{
    Security::Security() :
        m_type(SecurityType::Stock),
        m_precision(0),
        m_idPicture(Constants::NO_ID),
        m_idProvider(Constants::NO_ID),
        m_defaultToPicture(true),
        m_trading(nullptr)
    {
    }

    void Security::setType(SecurityType _type)
    {
        if (m_type == _type)
        {
            return;
        }
        else if (_type == SecurityType::Index
                 || m_type == SecurityType::Index)
        {
            ModelException::throwException(tr("Cannot change a security to become an index or vice-versa."), this);
            return;
        }

        m_type = _type;

        if (m_type != SecurityType::ETF && m_type != SecurityType::MutualFund)
        {
            m_idProvider = Constants::NO_ID;
        }

        if (!onHoldToModify())
            emit modified();
    }

    void Security::setName(const QString& _name)
    {
        CHECK_AND_SET(m_name, _name)
    }

    void Security::setSymbol(const QString& _symbol)
    {
        CHECK_AND_SET(m_symbol, _symbol)
    }

    void Security::setMarket(const QString& _market)
    {
        CHECK_AND_SET(m_market, _market)
    }

    void Security::setSector(const QString& _sector)
    {
        CHECK_AND_SET(m_sector, _sector)
    }

    void Security::setGeoLocation(const QString& _location)
    {
        CHECK_AND_SET(m_geoLocation, _location)
    }

    void Security::setCurrency(const QString& _currency)
    {
        CurrencyManager::instance()->get(_currency);

        CHECK_AND_SET(m_currency, _currency)
    }

    void Security::setPrecision(unsigned int _precision)
    {
        CHECK_AND_SET(m_precision, _precision)
    }

    void Security::setDefaultToSecurityPicture(bool _value)
    {
        CHECK_AND_SET(m_defaultToPicture, _value)
    }

    void Security::setNote(const QString& _note)
    {
        CHECK_AND_SET(m_note, _note)
    }

    void Security::setSecurityInfo(SecurityInfo _info, const Amount& _amount)
    {
        m_infos[_info] = _amount;

        if (!onHoldToModify())
            emit modified();
    }

    QString Security::formatAmount(const Amount& _amount) const
    {
        return _amount.toPrecision(precision()).toString();
    }

    Amount Security::securityInfo(SecurityInfo _info)
    {
        return m_infos.value(_info, 0.0);
    }

    void Security::setIdPicture(int _id)
    {
        if (_id == m_idPicture)
            return;

        if (_id != Constants::NO_ID)
        {
            //Check that it exists
            PictureManager::instance()->get(_id);
        }

        m_idPicture = _id;

        if (!onHoldToModify())
            emit modified();
    }

    void Security::setIdProvider(int _id)
    {
        if (_id == m_idProvider)
            return;

        if (m_type != SecurityType::ETF && m_type != SecurityType::MutualFund)
        {
            return;
        }

        if (_id != Constants::NO_ID)
        {
            //Check that it exists
            InstitutionManager::instance()->get(_id);
        }

        m_idProvider = _id;

        if (!onHoldToModify())
            emit modified();
    }

    QString Security::typeToString(SecurityType _type)
    {
        switch (_type)
        {
        case SecurityType::Stock:
            return tr("Stock");

        case SecurityType::PreferredStock:
            return tr("Preferred Stock");

        case SecurityType::ETF:
            return tr("ETF");

        case SecurityType::MutualFund:
            return tr("Mutual Fund");

        case SecurityType::Bond:
            return tr("Bond");

        case SecurityType::Index:
            return tr("Index");

        default:
            return QString();
        }
    }

    bool Security::canRemove() const
    {
        for (Account* a : Account::getTopLevel()->accounts())
        {
            if (a->idSecurity() == m_id)
            {
                return false;
            }
        }

        return true;
    }

    Account* Security::tradingAccount() const
    {
        if (!m_trading)
            Account::getTopLevel()->createSecurityTradingAccount(m_id);

        return m_trading;
    }

    void Security::load(QXmlStreamReader& _reader)
    {
        QXmlStreamAttributes attributes = _reader.attributes();

        m_id                = IO::getAttribute("id", attributes).toInt();
        m_type              = (SecurityType) IO::getAttribute("type", attributes).toInt();
        m_name              = IO::getAttribute("name", attributes);
        m_market            = IO::getOptAttribute("market", attributes);
        m_sector            = IO::getOptAttribute("sector", attributes);
        m_geoLocation       = IO::getOptAttribute("geolocation", attributes);
        m_symbol            = IO::getAttribute("symbol", attributes);
        m_currency          = IO::getAttribute("currency", attributes);
        m_precision         = IO::getOptAttribute("precision", attributes, 2).toInt();
        m_idPicture         = IO::getOptAttribute("picture", attributes, Constants::NO_ID).toInt();
        m_idProvider        = IO::getOptAttribute("provider", attributes, Constants::NO_ID).toInt();
        m_defaultToPicture  = IO::getOptAttribute("defaults", attributes, "true") == "true";
        m_note              = IO::getOptAttribute("note", attributes);

        if (_reader.readNextStartElement())
        {
            while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::SECURITY))
            {
                if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::SECURITY_INFO)
                {
                    QXmlStreamAttributes att = _reader.attributes();

                    m_infos[(SecurityInfo) IO::getAttribute("key", att).toInt()] = Amount::fromStoreable(IO::getAttribute("value", att));
                }

                _reader.readNext();
            }
        }
    }

    void Security::save(QXmlStreamWriter &_writer) const
    {
        _writer.writeStartElement(StdTags::SECURITY);
        _writer.writeAttribute("id",          QString::number(m_id));
        _writer.writeAttribute("type",        QString::number((int) m_type));
        _writer.writeAttribute("name",        m_name);
        _writer.writeAttribute("market",      m_market);
        _writer.writeAttribute("sector",      m_sector);
        _writer.writeAttribute("geolocation", m_geoLocation);
        _writer.writeAttribute("symbol",      m_symbol);
        _writer.writeAttribute("currency",    m_currency);
        _writer.writeAttribute("precision",   QString::number(m_precision));
        _writer.writeAttribute("picture",     QString::number(m_idPicture));
        _writer.writeAttribute("provider",    QString::number(m_idProvider));
        _writer.writeAttribute("defaults",    m_defaultToPicture ? "true" : "false");
        _writer.writeAttribute("note",        m_note);


        if (!m_infos.isEmpty())
        {
            for (auto i = m_infos.begin(); i != m_infos.end(); ++i)
            {
                _writer.writeEmptyElement(StdTags::SECURITY_INFO);
                _writer.writeAttribute("key", QString::number((int) i.key()));
                _writer.writeAttribute("value", i.value().toStoreable());
            }
        }

        _writer.writeEndElement();
    }

    int SecurityManager::m_nextId = 0;
    SecurityManager* SecurityManager::m_instance = new SecurityManager();

    Security* SecurityManager::add(SecurityType _type,
                                   const QString& _name,
                                   const QString& _symbol,
                                   const QString& _market,
                                   const QString& _sector,
                                   const QString& _currency,
                                   unsigned int _precision)
    {
        CurrencyManager::instance()->get(_currency);

        Security* o = new Security();
        o->m_id         = m_nextId++;
        o->m_type       = _type;
        o->m_name       = _name;
        o->m_symbol     = _symbol;
        o->m_currency   = _currency;
        o->m_precision  = _precision;
        o->m_sector     = _sector;
        o->m_market     = _market;

        if (_type != SecurityType::Index)
        {
            m_index[o->m_id] = IndexLocation(SecurityClass::Security, m_securities.size());
            m_securities.append(o);
            emit securityAdded(o);
        }
        else
        {
            m_index[o->m_id] = IndexLocation(SecurityClass::Index, m_indexes.size());
            m_indexes.append(o);
            emit indexAdded(o);
        }

        emit modified();
        connect(o, SIGNAL(modified()), this, SLOT(onModified()));

        return o;
    }

    void SecurityManager::onModified()
    {
        Security* o = static_cast<Security*>(sender());

        if (o)
        {
            if (o->type() == SecurityType::Index)
            {
                emit indexModified(o);
            }
            else
            {
                emit securityModified(o);
            }

            emit modified();
        }
    }


    Security* SecurityManager::get(int _id) const
    {
        if (!m_index.contains(_id))
        {
            ModelException::throwException("No such security.", this);
            return nullptr;
        }
        else if (m_index[_id].first == SecurityClass::Security)
        {
            return m_securities[m_index[_id].second];
        }
        else //m_index[_id].first == SecurityClass::Index
        {
            return m_indexes[m_index[_id].second];
        }

    }

    Security* SecurityManager::get(const QString& _symbol) const
    {
        foreach (Security* i, m_securities)
        {
            if (i->symbol() == _symbol)
            {
                return i;
            }
        }
        foreach (Security* i, m_indexes)
        {
            if (i->symbol() == _symbol)
            {
                return i;
            }
        }

        ModelException::throwException("No such security.", this);
        return nullptr;
    }

    Security* SecurityManager::securityAt(int _i) const
    {
        if (_i < 0 || _i >= m_securities.count())
        {
            ModelException::throwException(tr("Invalid index %1").arg(_i), this);
            return nullptr;
        }

        return m_securities[_i];
    }

    Security* SecurityManager::indexAt(int _i) const
    {
        if (_i < 0 || _i >= m_indexes.count())
        {
            ModelException::throwException(tr("Invalid index %1").arg(_i), this);
            return nullptr;
        }

        return m_indexes[_i];
    }

    void SecurityManager::remove(int _id)
    {
        if (m_index.contains(_id))
        {
            int                 pos       = m_index[_id].second;
            SecurityClass       sec_class = m_index[_id].first;
            QVector<Security*>& container = sec_class == SecurityClass::Security ? m_securities
                                                                                 : m_indexes;
            Security* s = container[pos];

            if (!s->canRemove())
            {
                ModelException::throwException(tr("Impossible to remove the security, "
                                                  "as it is used in at least one account."), s);
            }

            if (s->type() == SecurityType::Index)
            {
                emit indexRemoved(s);
            }
            else
            {
                emit securityRemoved(s);
            }

            //Remove the price pair
            PriceManager::instance()->removeAll(PriceManager::securityId(s->id()));

            //Delete it
            s->deleteLater();
            container.removeAt(pos);

            // Need to update every index greater than the index we removed
            for (int i = pos; i < container.size(); ++i)
            {
                m_index[container[i]->id()].second = i;
            }

            emit modified();
        }
    }

    QStringList SecurityManager::sectors() const
    {
        QSet<QString> list;

        for (Security* s : m_securities)
        {
            if (!s->sector().isEmpty())
            {
                list.insert(s->sector());
            }
        }

        return list.toList();
    }

    QStringList SecurityManager::markets() const
    {
        QSet<QString> list;

        for (Security* s : m_securities)
        {
            if (!s->market().isEmpty())
            {
                list.insert(s->market());
            }
        }

        return list.toList();
    }

    QHash<QString, QList<KLib::Security*> > SecurityManager::stocksBySector() const
    {
        QHash<QString, QList<KLib::Security*> > list;

        for (Security* s : m_securities)
        {
            if (s->type() == SecurityType::Stock && !s->sector().isEmpty())
            {
                if (!list.contains(s->sector()))
                {
                    list[s->sector()] = QList<Security*>();
                }

                list[s->sector()].append(s);
            }
        }

        return list;
    }

    QHash<QString, QList<KLib::Security*> > SecurityManager::stocksByMarket() const
    {
        QHash<QString, QList<KLib::Security*> > list;

        for (Security* s : m_securities)
        {
            if (s->type() == SecurityType::Stock && !s->market().isEmpty())
            {
                if (!list.contains(s->market()))
                {
                    list[s->market()] = QList<Security*>();
                }

                list[s->market()].append(s);
            }
        }

        return list;
    }


    void SecurityManager::load(QXmlStreamReader& _reader)
    {
        unload();

        // While not at end of payees
        while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::SECURITY_MGR))
        {
            if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::SECURITY)
            {
                Security* o = new Security();
                o->load(_reader);
                connect(o, SIGNAL(modified()), this, SLOT(onModified()));

                m_nextId = std::max(m_nextId, o->m_id + 1);

                if (o->type() != SecurityType::Index)
                {
                    m_index[o->m_id] = IndexLocation(SecurityClass::Security, m_securities.count());
                    m_securities.append(o);
                }
                else // (o->type() == SecurityType::INDEX)
                {
                    m_index[o->m_id] = IndexLocation(SecurityClass::Index, m_indexes.count());
                    m_indexes.append(o);
                }
            }

            _reader.readNext();
        }
    }

    void SecurityManager::save(QXmlStreamWriter &_writer) const
    {
        for (Security* o : m_securities)
        {
            o->save(_writer);
        }

        for (Security* o : m_indexes)
        {
            o->save(_writer);
        }
    }

    void SecurityManager::unload()
    {
        for (Security* i : m_securities)
        {
            delete i;
        }

        for (Security* i : m_indexes)
        {
            delete i;
        }

        m_securities.clear();
        m_indexes.clear();
        m_index.clear();
        m_nextId = 0;
    }

}

unsigned int qHash(KLib::SecurityInfo _key, unsigned int seed)
{
    return qHash((int) _key, seed);
}
