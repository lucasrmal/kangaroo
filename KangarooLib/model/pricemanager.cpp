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

#include "pricemanager.h"
#include "modelexception.h"
#include "../controller/io.h"
#include "security.h"
//#include "currency.h"

#include <QXmlStreamReader>

namespace KLib
{

    PriceManager* PriceManager::m_instance = new PriceManager();
    const unsigned int PriceManager::DECIMALS_RATE = 8;


    void ExchangePair::set(const QDate& _date, double _rate)
    {
        m_rates[_date] = _rate;
        emit rateSet(_date);
    }

    void ExchangePair::remove(const QDate& _date)
    {
        m_rates.erase(_date);
        emit rateRemoved(_date);
    }

    double ExchangePair::on(const QDate& _date) const
    {
        auto it = m_rates.lower_bound(_date);
        if (it == m_rates.end()) {
            return 0;
        }

        return it->second;
    }

    double ExchangePair::last() const
    {
        return m_rates.empty() ? 0 : m_rates.end()->second;
    }

    const QList<ExchangePair::Rate> ExchangePair::rates() const
    {
        QList<Rate> rat;

        for (auto i = m_rates.begin(); i != m_rates.end(); ++i)
        {
            rat << QPair<QDate, double>(i->first, i->second);
        }

        return rat;
    }

    void ExchangePair::setUpdateSource(const QString& _source)
    {
        if (m_autoUpdate)
        {
            m_updateSource = _source;
            emit modified();
        }
    }

    void ExchangePair::setAutoUpdate(bool _auto)
    {
        m_autoUpdate = _auto;

        if (!m_autoUpdate)
            m_updateSource.clear();

        emit modified();
    }

    Security* ExchangePair::securityFrom() const
    {
        if (isSecurity() && !m_security)
        {
            int idSec = QStringRef(&m_from, 3, m_from.size()-3).toInt();
            m_security = SecurityManager::instance()->get(idSec);
        }

        return m_security;
    }

    void ExchangePair::load(QXmlStreamReader& _reader)
    {
        QXmlStreamAttributes attributes = _reader.attributes();

        m_from = IO::getAttribute("from", attributes);
        m_to   = IO::getAttribute("to", attributes);
        m_autoUpdate   = IO::getOptAttribute("autoupdate", attributes, "true") == "true";
        m_updateSource   = IO::getOptAttribute("updatesource", attributes);

        if (_reader.readNextStartElement())
        {
            while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::EXCHANGE_PAIR))
            {
                if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::PRICE)
                {
                    attributes = _reader.attributes();

                    m_rates.insert({QDate::fromString(IO::getAttribute("date", attributes), Qt::ISODate),
                                    IO::getAttribute("value", attributes).toDouble()});
                }

                _reader.readNext();
            }
        }
    }

    void ExchangePair::save(QXmlStreamWriter &_writer) const
    {
        _writer.writeStartElement(StdTags::EXCHANGE_PAIR);

        _writer.writeAttribute("from", m_from);
        _writer.writeAttribute("to", m_to);
        _writer.writeAttribute("autoupdate", m_autoUpdate ? "true" : "false");
        _writer.writeAttribute("updatesource", m_updateSource);

        for (auto i = m_rates.begin(); i != m_rates.end(); ++i)
        {
            _writer.writeEmptyElement(StdTags::PRICE);
            _writer.writeAttribute("date", i->first.toString(Qt::ISODate));
            _writer.writeAttribute("value", QString::number(i->second));
        }

        _writer.writeEndElement();
    }

/* ####################################################################### */

    PriceManager::PriceManager() :
        m_noEmit(false)
    {

    }

    ExchangePair* PriceManager::add(const QString& _from, const QString& _to)
    {
        if (m_index.contains(PricePair(_from, _to)))
        {
            ModelException::throwException(tr("This exchange pair already exists."), this);
        }
        else if (_from == _to)
        {
            ModelException::throwException(tr("From and to have to be different."), this);
        }

        ExchangePair* p = new ExchangePair();
        p->m_from = _from;
        p->m_to = _to;

        m_index[PricePair(_from, _to)] = m_pairs.size();
        m_pairs << p;

        emit modified();
        emit exchangePairAdded(p);

        connect(p, SIGNAL(rateSet(QDate)), this, SLOT(onRateSet(QDate)));
        connect(p, SIGNAL(rateRemoved(QDate)), this, SLOT(onRateRemoved(QDate)));
        connect(p, SIGNAL(modified()), this, SIGNAL(modified()));

        return p;
    }

    ExchangePair* PriceManager::get(const QString& _from, const QString& _to) const
    {
        if (!m_index.contains(PricePair(_from, _to)))
        {
            ModelException::throwException(tr("This exchange pair does not exists."), this);
        }

        return m_pairs[m_index[PricePair(_from, _to)]];
    }

    ExchangePair* PriceManager::getOrAdd(const QString& _from, const QString& _to)
    {
        if (!m_index.contains(PricePair(_from, _to)))
        {
            return add(_from, _to);
        }
        else
        {
            return m_pairs[m_index[PricePair(_from, _to)]];
        }
    }

    ExchangePair* PriceManager::at(int _i) const
    {
        if (_i < 0 || _i >= m_pairs.count())
        {
            ModelException::throwException(tr("Invalid index %1").arg(_i), this);
            return nullptr;
        }

        return m_pairs[_i];
    }

    void PriceManager::remove(const QString& _from, const QString& _to)
    {
        if (m_index.contains(PricePair(_from, _to)))
        {
            int i = m_index[PricePair(_from, _to)];
            m_index.remove(PricePair(_from, _to));

            ExchangePair* p = m_pairs.takeAt(i);

            //Update the index
            for (; i < m_pairs.count(); ++i)
            {
                m_index[PricePair(m_pairs[i]->from(), m_pairs[i]->to())] = i;
            }

            emit modified();
            emit exchangePairRemoved(p);
            p->deleteLater();
        }
    }

    void PriceManager::removeAll(const QString& _fromTo)
    {
        int i = 0;
        while (i < m_pairs.size())
        {
            ExchangePair* p = m_pairs[i];

            if (p->to() == _fromTo || p->from() == _fromTo)
            {
                emit exchangePairRemoved(p);
                m_index.remove(PricePair(p->from(), p->to()));
                m_pairs.remove(i);
            }
            else
            {
                m_index[PricePair(p->from(), p->to())] = i;
                ++i;
            }
        }
    }

    void PriceManager::onRateSet(const QDate& _date)
    {
        ExchangePair* p = static_cast<ExchangePair*>(sender());

        if (p)
        {
            emit rateSet(p, _date);
            emit modified();

            if (_date == p->m_rates.end()->first)
            {
                emit lastRateModified(p);
            }
        }
    }

    void PriceManager::onRateRemoved(const QDate& _date)
    {
        ExchangePair* p = static_cast<ExchangePair*>(sender());

        if (p)
        {
            emit rateRemoved(p, _date);
            emit modified();

            if (_date > p->m_rates.end()->first)
            {
                emit lastRateModified(p);
            }
        }
    }

    QString PriceManager::securityId(int _idSecurity)
    {
        return QString("SEC%1").arg(_idSecurity);
    }

    double PriceManager::rate(const QString& _from, const QString& _to, const QDate &_date) const
    {
        if (_from == _to)
        {
            return 1.;
        }

        if (m_index.contains(PricePair(_from, _to)))
        {
            if (_date.isValid())
            {
                return get(_from, _to)->on(_date);
            }
            else
            {
                return get(_from, _to)->last();
            }
        }
        // Try the reverse
        else if (!_from.startsWith("SEC")
                 && m_index.contains(PricePair(_to, _from)))
        {
            if (_date.isValid())
            {
                return 1. / get(_to, _from)->on(_date);
            }
            else
            {
                return 1. / get(_to, _from)->last();
            }
        }
        else
        {
            return 0;
        }
    }

    double PriceManager::rate(int _idSecurity, const QString& _to, const QDate &_date) const
    {
        Security* s = SecurityManager::instance()->get(_idSecurity);
        if (s->currency() == _to)
        {
            return rate(securityId(_idSecurity), _to, _date);
        }
        else
        {
            return   rate(securityId(_idSecurity), s->currency(), _date)
                   * rate(s->currency(), _to, _date);
        }
    }

    void PriceManager::load(QXmlStreamReader& _reader)
    {
        unload();

        m_noEmit = true;

        while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::PRICE_MGR))
        {
            if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::EXCHANGE_PAIR)
            {

                ExchangePair* p = new ExchangePair();
                p->load(_reader);
                m_index[PricePair(p->from(), p->to())] = m_pairs.size();
                m_pairs << p;

                connect(p, SIGNAL(rateSet(QDate)), this, SLOT(onRateSet(QDate)));
                connect(p, SIGNAL(rateRemoved(QDate)), this, SLOT(onRateRemoved(QDate)));
            }

            _reader.readNext();
        }

        m_noEmit = false;
    }

    void PriceManager::save(QXmlStreamWriter& _writer) const
    {
        for (ExchangePair* p : m_pairs)
        {
            p->save(_writer);
        }
    }

    void PriceManager::unload()
    {
        for (ExchangePair* p : m_pairs)
        {
            delete p;
        }

        m_pairs.clear();
        m_index.clear();
        m_noEmit = false;
    }

}
