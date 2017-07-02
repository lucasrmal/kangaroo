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

#ifndef INSTITUTION_H
#define INSTITUTION_H

#include<QString>
#include <QVector>
#include <QList>

#include "stored.h"
#include "../interfaces/scriptable.h"

namespace KLib
{

class Account;

class Institution : public IStored
{
    Q_OBJECT
    K_SCRIPTABLE(Institution)
    Q_PROPERTY(QString name READ name WRITE setName)
    Q_PROPERTY(int idPicture READ idPicture WRITE setIdPicture)
    Q_PROPERTY(bool isActive READ isActive WRITE setActive)
    Q_PROPERTY(QString address READ address WRITE setAddress)
    Q_PROPERTY(QString country READ country WRITE setCountry)
    Q_PROPERTY(QString contactName READ contactName WRITE setContactName)
    Q_PROPERTY(QString website READ website WRITE setWebsite)
    Q_PROPERTY(QString institutionNumber READ institutionNumber WRITE setInstitutionNumber)
    Q_PROPERTY(QString routingNumber READ routingNumber WRITE setRoutingNumber)
    Q_PROPERTY(QString note READ note WRITE setNote)

    Institution();

    public:

        virtual ~Institution() {}

        QList<KLib::Account*> accounts() const;

        QString     name() const { return m_name; }
        int         idPicture() const { return m_idPicture; }
        bool        isActive() const { return m_isActive; }

        QString     address() const             { return m_address; }
        QString     country() const             { return m_country; }
        QString     phone() const               { return m_phone; }
        QString     fax() const                 { return m_fax; }
        QString     contactName() const         { return m_contactName; }
        QString     website() const             { return m_website; }
        QString     institutionNumber() const   { return m_institutionNumber; }
        QString     routingNumber() const       { return m_routingNumber; }
        QString     note() const                { return m_note; }

        void        setName(const QString& _name);
        void        setIdPicture(int _id);

        void        setActive(bool _active);
        void        setAddress(const QString& _address);
        void        setCountry(const QString& _country);
        void        setPhone(const QString& _phone);
        void        setFax(const QString& _fax);
        void        setContactName(const QString& _contact);
        void        setWebsite(const QString& _website);
        void        setInstitutionNumber(const QString& _number);
        void        setRoutingNumber(const QString& _number);
        void        setNote(const QString& _note);

    protected:
        void load(QXmlStreamReader& _reader) override;
        void save(QXmlStreamWriter& _writer) const override;

    private:
        QString m_name;
        bool    m_isActive;

        QString m_address;
        QString m_country;
        QString m_phone;
        QString m_fax;
        QString m_contactName;
        QString m_website;
        QString m_institutionNumber;
        QString m_routingNumber;
        QString m_note;

        int     m_idPicture;

        friend class InstitutionManager;
};

class InstitutionManager : public IStored
{
    Q_OBJECT
    K_SCRIPTABLE(InstitutionManager)

    Q_PROPERTY(int count READ count)

    public:

        Q_INVOKABLE KLib::Institution* add(const QString& _name, int _idPicture = Constants::NO_ID);
        Q_INVOKABLE KLib::Institution* get(int _id) const;
        Q_INVOKABLE KLib::Institution* at(int _i) const;
        Q_INVOKABLE void remove(int _id);

        /**
         * @brief merge Merge a set of institutions into a single one
         * @param _ids The IDs of institutions to merge together
         * @param _idTo The destination institution id. Must be included in _ids.
         */
        Q_INVOKABLE  void merge(const QSet<int>& _ids, int _idTo);

        int count() const { return m_institutions.size(); }
        Q_INVOKABLE QStringList countries() const;

        Q_INVOKABLE const QVector<KLib::Institution*>& institutions() const { return m_institutions; }

        static InstitutionManager* instance() { return m_instance; }

    signals:
        void institutionAdded(KLib::Institution* i);
        void institutionRemoved(KLib::Institution* i);
        void institutionModified(KLib::Institution* i);

    protected slots:
        void onModified();

    protected:
        void load(QXmlStreamReader& _reader) override;
        void save(QXmlStreamWriter& _writer) const override;
        void unload() override;

    private:
        QHash<int, int> m_index;
        QVector<Institution*> m_institutions;

        static InstitutionManager* m_instance;
        static int m_nextId;
};

}

Q_DECLARE_METATYPE(KLib::Institution*)
Q_DECLARE_METATYPE(KLib::InstitutionManager*)

#endif // INSTITUTION_H
