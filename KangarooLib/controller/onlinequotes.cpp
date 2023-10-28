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

#include "onlinequotes.h"
#include "../interfaces/iquote.h"
#include "../model/security.h"
#include "../model/modelexception.h"

#include <QDebug>
#include <stdexcept>

namespace KLib
{

OnlineQuotes* OnlineQuotes::m_instance = new OnlineQuotes();

OnlineQuotes::~OnlineQuotes()
{
    for (IQuote* q : m_sources)
    {
        delete q;
    }
}

void OnlineQuotes::updateAll()
{
    // Make a list of all currencies
    QHash<IQuote*, QList<PricePair> > currencies;
    QHash<IQuote*, QList<PricePair> > stocks;

    for (IQuote* q : m_sources)
    {
        currencies[q] = QList<PricePair>();
        stocks[q] = QList<PricePair>();
    }

    for (ExchangePair* p : PriceManager::instance()->pairs())
    {
        IQuote* updateSource = nullptr;

        try
        {
            updateSource = get(p->updateSource());
        }
        catch (...)
        {
            if (m_default.isEmpty())
            {
                return; //No quote source!
            }
            else
            {
                updateSource = get(QString());
            }
        }

        try
        {
            if (!p->isSecurity() && p->autoUpdate())
            {
                currencies[updateSource] << PricePair(p->from(), p->to());
            }
            else if (p->autoUpdate() && p->securityFrom() && !p->securityFrom()->symbol().isEmpty())
            {
                stocks[updateSource] << PricePair(p->securityFrom()->symbol(), "");
            }
        }
        catch (ModelException e)
        {
            qDebug() << e.description();
            continue;
        }
    }


    for (IQuote* q : m_sources)
    {
        int a = q->makeRequest(IQuote::Currency, currencies[q]);
        int b = q->makeRequest(IQuote::Stock, stocks[q]);

        if (a != -1)
            m_currentUpdate << a;
        if (b != -1)
            m_currentUpdate << b;
    }

    if (m_currentUpdate.isEmpty())
        emit requestDone();
}

void OnlineQuotes::onQuoteReady(IQuote::Type _type, const PricePair& _pair, double _quote,
                                const QHash<SecurityInfo, Amount> _extraInfos)
{
    try
    {
        QString from = _pair.first;

        if (_type == IQuote::Stock)
        {
            Security* s = SecurityManager::instance()->get(_pair.first);

            if (!_extraInfos.isEmpty())
            {
                s->holdToModify();

                for (auto i = _extraInfos.begin(); i != _extraInfos.end(); ++i)
                {
                    s->setSecurityInfo(i.key(), i.value());
                }

                s->doneHoldToModify();
            }

            from = KLib::PriceManager::securityId(s->id());

        }
        PriceManager::instance()->get(from, _pair.second)->set(QDate::currentDate(), _quote);
        emit quoteReady(_pair, _quote);
    }
    catch (ModelException)
    {
        emit requestError(tr("Received unknown security from server: %1-%2").arg(_pair.first).arg(_pair.second));
    }
}

void OnlineQuotes::onRequestError(int _id, const QString &_message)
{
    emit requestError(_message);
    onRequestDone(_id);
}

void OnlineQuotes::onRequestDone(int _id)
{
    m_currentUpdate.removeAll(_id);

    if (m_currentUpdate.isEmpty())
    {
        emit requestDone();
    }
}

void OnlineQuotes::registerQuoteSource(IQuote* _source, bool _default)
{
    if (m_sources.contains(_source->id()))
    {
        throw std::runtime_error("This quote source is already registered!");
    }

    m_sources[_source->id()] = _source;

    connect(_source, &IQuote::quoteReady,   this, &OnlineQuotes::onQuoteReady);
    connect(_source, &IQuote::requestError, this, &OnlineQuotes::onRequestError);
    connect(_source, &IQuote::requestDone,  this, &OnlineQuotes::onRequestDone);

    if (_default || m_sources.size() == 1)
    {
        m_default = _source->id();
    }
}

void OnlineQuotes::setDefault(const QString& _id)
{
    if (!m_sources.contains(_id))
    {
        throw std::runtime_error("This quote source does not exists");
    }

    m_default = _id;
}

IQuote* OnlineQuotes::get(const QString& _id) const
{
    if (_id.isEmpty())
    {
        return getDefault();
    }
    else if (!m_sources.contains(_id))
    {
        throw std::runtime_error("This quote source does not exists");
    }

    return m_sources[_id];
}

IQuote* OnlineQuotes::getDefault() const
{
    if (m_default.isEmpty())
    {
        throw std::runtime_error("No default online quote source has been set!");
    }

    return m_sources[m_default];
}

}
