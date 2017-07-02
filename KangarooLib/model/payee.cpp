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

#include "payee.h"
#include "../controller/io.h"
#include "transaction.h"
#include "picturemanager.h"
#include "transactionmanager.h"
#include "modelexception.h"
#include <QXmlStreamReader>

namespace KLib
{

int PayeeManager::m_nextId = 0;
PayeeManager* PayeeManager::m_instance = new PayeeManager();

Payee::Payee() :
    m_idPicture(Constants::NO_ID)
{
}

void Payee::setName(const QString& _name)
{
    if (_name != m_name)
    {
        try
        {
            PayeeManager::instance()->get(_name);
            ModelException::throwException(tr("A payee with name %1 already exists.").arg(_name), this);
        }
        catch (ModelException)
        {
            QString old = m_name;

            m_name = _name;
            if (!onHoldToModify())
                emit modified();

            emit nameChanged(old, m_name);
        }
    }
}

void Payee::setIdPicture(int _id)
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

void Payee::setAddress(const QString& _address)
{
    m_address = _address;
    if (!onHoldToModify())
        emit modified();
}

void Payee::setCountry(const QString& _country)
{
    m_country = _country;
    if (!onHoldToModify())
        emit modified();
}

void Payee::setPhone(const QString& _phone)
{
    m_phone = _phone;
    if (!onHoldToModify())
        emit modified();
}

void Payee::setFax(const QString& _fax)
{
    m_fax = _fax;
    if (!onHoldToModify())
        emit modified();
}

void Payee::setContactName(const QString& _contact)
{
    m_contactName = _contact;
    if (!onHoldToModify())
        emit modified();
}

void Payee::setWebsite(const QString& _website)
{
    m_website = _website;
    if (!onHoldToModify())
        emit modified();
}

void Payee::setNote(const QString& _note)
{
    m_note = _note;
    if (!onHoldToModify())
        emit modified();
}

Payee* PayeeManager::add(const QString& _name, int _idPicture)
{
    try
    {
        PayeeManager::instance()->get(_name);
        ModelException::throwException(tr("A payee with name %1 already exists.").arg(_name), this);
        return nullptr;
    }
    catch (ModelException)
    {
        Payee* o = new Payee();
        o->m_id = m_nextId++;
        o->m_name = _name;

        if (_idPicture != Constants::NO_ID)
        {
            //Check that it exists
            PictureManager::instance()->get(_idPicture);
        }

        o->m_idPicture = _idPicture;

        m_index[o->m_id] = m_payees.size();
        m_nameIndex[o->m_name] = m_payees.size();
        m_payees.append(o);
        emit modified();
        emit payeeAdded(o);

        connect(o, &Payee::modified, this, &PayeeManager::onModified);
        connect(o, &Payee::nameChanged, this, &PayeeManager::onNameChanged);

        return o;
    }
}

Payee* PayeeManager::get(int _id) const
{
    if (!m_index.contains(_id))
    {
        ModelException::throwException(tr("No such payee."), this);
        return nullptr;
    }

    return m_payees[m_index[_id]];
}

Payee* PayeeManager::get(const QString& _name) const
{
    if (!m_nameIndex.contains(_name))
    {
        ModelException::throwException(tr("No such payee."), this);
        return nullptr;
    }

    return m_payees[m_nameIndex[_name]];
}

Payee* PayeeManager::at(int _i) const
{
    if (_i < 0 || _i >= m_payees.count())
    {
        ModelException::throwException(tr("Invalid index %1").arg(_i), this);
        return nullptr;
    }

    return m_payees[_i];
}

void PayeeManager::merge(const QSet<int>& _ids, int _idTo)
{
    if (_ids.size() < 1)
    {
        ModelException::throwException(tr("The merge set must be at least 2."), this);
        return;
    }
    else if (!_ids.contains(_idTo))
    {
        ModelException::throwException(tr("The merge set must include the destination payee id ."), this);
        return;
    }

    //Check if all ids are valid
    for (int id : _ids)
    {
        get(id);
    }

    QSet<int> newSet = _ids;
    newSet.remove(_idTo); //We don't modify transactions that already have this payee...

    //Do the merge
    for (Transaction* t : TransactionManager::instance()->transactions())
    {
        if (newSet.contains(t->idPayee()))
        {
            t->setIdPayee(_idTo);
        }
    }

    //Delete the payees
    for (int i = 0; i < m_payees.size(); ++i)
    {
        if (newSet.contains(m_payees[i]->id()))
        {
            emit payeeRemoved(m_payees[i]);
            m_payees[i]->deleteLater();
            m_payees.removeAt(i);

            emit modified();
        }

        m_index[m_payees[i]->id()] = i;
        m_nameIndex[m_payees[i]->name()] = i;
    }
}

QStringList PayeeManager::countries() const
{
    QSet<QString> list;

    for (Payee* p : m_payees)
    {
        if (!p->country().isEmpty())
        {
            list.insert(p->country());
        }
    }

    return list.toList();
}

void PayeeManager::remove(int _id)
{
    if (m_index.contains(_id))
    {
        int idx = m_index[_id];
        emit payeeRemoved(m_payees[idx]);
        m_payees[idx]->deleteLater();
        m_payees.removeAt(idx);

        // Need to do -1 to every index greater than the index we removed
        for (int i = idx; i < m_payees.size(); ++i)
        {
            m_index[m_payees[i]->id()] = i;
            m_nameIndex[m_payees[i]->name()] = i;
        }

        // Change all transactions with this payee
        for (Transaction* t : TransactionManager::instance()->transactions())
        {
            if (t->idPayee() == _id)
            {
                t->setIdPayee(Constants::NO_ID);
            }
        }

        emit modified();
    }
}

void PayeeManager::onModified()
{
    Payee* o = static_cast<Payee*>(sender());

    if (o)
    {
        emit payeeModified(o);
        emit modified();
    }

}

void PayeeManager::onNameChanged(const QString& _old, const QString& _new)
{
    Payee* p = qobject_cast<Payee*>(sender());

    if (p)
    {
        m_nameIndex.remove(_old);
        m_nameIndex[_new] = p->id();
    }
}

void PayeeManager::load(QXmlStreamReader& _reader)
{
    unload();

    // While not at end of payees
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::PAYEE_MGR))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::PAYEE)
        {
            Payee* o = new Payee();
            o->load(_reader);
            m_nextId = std::max(m_nextId, o->m_id + 1);
            m_index[o->m_id] = m_payees.size();
            m_nameIndex[o->m_name] = m_payees.size();
            m_payees.append(o);            

            connect(o, &Payee::modified, this, &PayeeManager::onModified);
            connect(o, &Payee::nameChanged, this, &PayeeManager::onNameChanged);
        }

        _reader.readNext();
    }
}

void Payee::load(QXmlStreamReader& _reader)
{
    QXmlStreamAttributes attributes = _reader.attributes();

    m_id            = IO::getAttribute("id", attributes).toInt();
    m_name          = IO::getAttribute("name", attributes);
    m_idPicture     = IO::getOptAttribute("picture", attributes, Constants::NO_ID).toInt();
    m_address       = IO::getOptAttribute("address", attributes);
    m_country       = IO::getOptAttribute("country", attributes);
    m_phone         = IO::getOptAttribute("phone", attributes);
    m_fax           = IO::getOptAttribute("fax", attributes);
    m_contactName   = IO::getOptAttribute("contactname", attributes);
    m_website       = IO::getOptAttribute("website", attributes);
    m_note          = IO::getOptAttribute("note", attributes);
}

void Payee::save(QXmlStreamWriter &_writer) const
{
    _writer.writeEmptyElement(StdTags::PAYEE);
    _writer.writeAttribute("id", QString::number(m_id));
    _writer.writeAttribute("name", m_name);
    _writer.writeAttribute("picture", QString::number(m_idPicture));
    _writer.writeAttribute("address", m_address);
    _writer.writeAttribute("country", m_country);
    _writer.writeAttribute("phone", m_phone);
    _writer.writeAttribute("fax", m_fax);
    _writer.writeAttribute("contactname", m_contactName);
    _writer.writeAttribute("website", m_website);
    _writer.writeAttribute("note", m_note);
}

void PayeeManager::save(QXmlStreamWriter& _writer) const
{
    for (Payee* p : m_payees)
    {
        p->save(_writer);
    }
}

void PayeeManager::unload()
{
    for (Payee* i : m_payees)
    {
        delete i;
    }

    m_payees.clear();
    m_index.clear();
    m_nameIndex.clear();
    m_nextId = 0;
}

}
