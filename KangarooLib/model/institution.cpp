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

#include "institution.h"
#include "modelexception.h"
#include "account.h"
#include "picturemanager.h"
#include "../controller/io.h"
#include <QXmlStreamReader>

namespace KLib
{

int InstitutionManager::m_nextId = 0;
InstitutionManager* InstitutionManager::m_instance = new InstitutionManager();

Institution::Institution() :
    m_isActive(true),
    m_idPicture(Constants::NO_ID)
{
}

void Institution::setName(const QString& _name)
{
    m_name = _name;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setActive(bool _active)
{
    m_isActive = _active;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setAddress(const QString& _address)
{
    m_address = _address;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setCountry(const QString& _country)
{
    m_country = _country;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setPhone(const QString& _phone)
{
    m_phone = _phone;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setFax(const QString& _fax)
{
    m_fax = _fax;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setContactName(const QString& _contact)
{
    m_contactName = _contact;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setWebsite(const QString& _website)
{
    m_website = _website;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setInstitutionNumber(const QString& _number)
{
    m_institutionNumber = _number;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setRoutingNumber(const QString& _number)
{
    m_routingNumber = _number;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setNote(const QString& _note)
{
    m_note = _note;
    if (!onHoldToModify())
        emit modified();
}

void Institution::setIdPicture(int _id)
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

void Institution::load(QXmlStreamReader& _reader)
{
    QXmlStreamAttributes attributes = _reader.attributes();

    m_id                = IO::getAttribute("id", attributes).toInt();
    m_name              = IO::getAttribute("name", attributes);
    m_isActive          = IO::getOptAttribute("active", attributes, "true") == "true";
    m_idPicture         = IO::getOptAttribute("picture", attributes, Constants::NO_ID).toInt();
    m_address           = IO::getOptAttribute("address", attributes);
    m_country           = IO::getOptAttribute("country", attributes);
    m_phone             = IO::getOptAttribute("phone", attributes);
    m_fax               = IO::getOptAttribute("fax", attributes);
    m_contactName       = IO::getOptAttribute("contactname", attributes);
    m_website           = IO::getOptAttribute("website", attributes);
    m_institutionNumber = IO::getOptAttribute("institutionnumber", attributes);
    m_routingNumber     = IO::getOptAttribute("routingnumber", attributes);
    m_note              = IO::getOptAttribute("note", attributes);
}

void Institution::save(QXmlStreamWriter &_writer) const
{
    _writer.writeEmptyElement(StdTags::INSTITUTION);
    _writer.writeAttribute("id", QString::number(m_id));
    _writer.writeAttribute("name", m_name);
    _writer.writeAttribute("active", m_isActive ? "true" : "false");
    _writer.writeAttribute("picture", QString::number(m_idPicture));
    _writer.writeAttribute("address", m_address);
    _writer.writeAttribute("country", m_country);
    _writer.writeAttribute("phone", m_phone);
    _writer.writeAttribute("fax", m_fax);
    _writer.writeAttribute("contactname", m_contactName);
    _writer.writeAttribute("website", m_website);
    _writer.writeAttribute("institutionnumber", m_institutionNumber);
    _writer.writeAttribute("routingnumber", m_routingNumber);
    _writer.writeAttribute("note", m_note);
}

void InstitutionManager::load(QXmlStreamReader& _reader)
{
    unload();

    // While not at end
    while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::INSTITUTION_MGR))
    {
        if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::INSTITUTION)
        {
            Institution* o = new Institution();
            o->load(_reader);
            m_nextId = std::max(m_nextId, o->m_id + 1);
            m_index[o->m_id] = m_institutions.size();
            m_institutions.append(o);            

            connect(o, SIGNAL(modified()), this, SLOT(onModified()));
        }

        _reader.readNext();
    }
}

void InstitutionManager::save(QXmlStreamWriter &_writer) const
{
    for (Institution* o : m_institutions)
    {
        o->save(_writer);
    }
}

void InstitutionManager::unload()
{
    for (Institution* i : m_institutions)
    {
        delete i;
    }

    m_institutions.clear();
    m_index.clear();
    m_nextId = 0;
}

QStringList InstitutionManager::countries() const
{
    QSet<QString> list;

    for (Institution* i : m_institutions)
    {
        if (!i->country().isEmpty())
        {
            list.insert(i->country());
        }
    }

    return list.toList();
}

Institution* InstitutionManager::add(const QString& _name, int _idPicture)
{
    Institution* o = new Institution();
    o->m_id = m_nextId++;
    o->m_name = _name;

    if (_idPicture != Constants::NO_ID)
    {
        //Check that it exists
        PictureManager::instance()->get(_idPicture);
    }

    o->m_idPicture = _idPicture;

    m_index[o->m_id] = m_institutions.size();
    m_institutions.append(o);
    emit modified();
    emit institutionAdded(o);

    connect(o, SIGNAL(modified()), this, SLOT(onModified()));

    return o;
}

Institution* InstitutionManager::get(int _id) const
{
    if (!m_index.contains(_id))
    {
        ModelException::throwException(tr("No such institution."), this);
        return nullptr;
    }

    return m_institutions[m_index[_id]];
}

Institution* InstitutionManager::at(int _i) const
{
    if (_i < 0 || _i >= m_institutions.count())
    {
        ModelException::throwException(tr("Invalid index %1").arg(_i), this);
        return nullptr;
    }

    return m_institutions[_i];
}

void InstitutionManager::remove(int _id)
{
    if (m_index.contains(_id))
    {
        int idx = m_index[_id];
        emit institutionRemoved(m_institutions[idx]);

        m_institutions[idx]->deleteLater();
        m_institutions.removeAt(idx);

        for (Account* a : Account::getTopLevel()->accounts())
        {
            if (_id == a->idInstitution())
            {
                a->setIdInstitution(Constants::NO_ID);
            }
        }

        // Need to do -1 to every index greater than the index we removed
        for (int i = idx; i < m_institutions.size(); ++i)
        {
            m_index[m_institutions[i]->id()] = i;
        }

        emit modified();
    }
}

void InstitutionManager::merge(const QSet<int>& _ids, int _idTo)
{
    if (_ids.size() < 1)
    {
        ModelException::throwException(tr("The merge set must be at least 2."), this);
        return;
    }
    else if (!_ids.contains(_idTo))
    {
        ModelException::throwException(tr("The merge set must include the destination institution id."), this);
        return;
    }

    //Check if all ids are valid
    for (int id : _ids)
    {
        get(id);
    }

    QSet<int> newSet = _ids;
    newSet.remove(_idTo); //We don't modify accounts that already have this institution...

    //Do the merge
    for (Account* a : Account::getTopLevel()->accounts())
    {
        if (newSet.contains(a->idInstitution()))
        {
            a->setIdInstitution(_idTo);
        }
    }

    //Delete the institutions & recompute the index
    for (int i = 0; i < m_institutions.size(); ++i)
    {
        if (newSet.contains(m_institutions[i]->id()))
        {
            emit institutionRemoved(m_institutions[i]);
            m_institutions[i]->deleteLater();
            m_institutions.removeAt(i);

            emit modified();
        }

        m_index[m_institutions[i]->id()] = i;
    }
}

void InstitutionManager::onModified()
{
    Institution* o = static_cast<Institution*>(sender());

    if (o)
    {
        emit institutionModified(o);
        emit modified();
    }
}

}
