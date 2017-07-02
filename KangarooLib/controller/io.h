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

#ifndef IO_H
#define IO_H

#include <QString>
#include <QHash>
#include <QVariant>

class QXmlStreamAttributes;

namespace KLib {

    class IStored;
    class IFileManager;

    struct StdTags
    {
        static const char* ROOT;
        static const char* ACCOUNT;
        static const char* CURRENCY;
        static const char* CURRENCY_MGR;
        static const char* INSTITUTION;
        static const char* INSTITUTION_MGR;
        static const char* PRICE;
        static const char* EXCHANGE_PAIR;
        static const char* PRICE_MGR;
        static const char* PAYEE;
        static const char* PAYEE_MGR;
        static const char* SECURITY;
        static const char* SECURITY_MGR;
        static const char* SECURITY_INFOS;
        static const char* SECURITY_INFO;
        static const char* TRANSACTION;
        static const char* INVEST_TRANSACTION;
        static const char* TRANSACTION_MGR;
        static const char* SPLIT;
        static const char* INVEST_LOT;
        static const char* INVEST_LOT_USAGE;
        static const char* INVEST_LOT_SPLIT;
        static const char* INVEST_LOT_TRANSFER;
        static const char* INVEST_LOT_MGR;
        static const char* PROPERTIES;
        static const char* PROPERTY;
        static const char* PICTURE;
        static const char* PICTURE_MGR;
        static const char* DOCUMENT;
        static const char* DOCUMENT_MGR;
        static const char* SCHEDULE;
        static const char* SCHEDULE_REC;
        static const char* SCHEDULE_MGR;
    };

    class IOException : public std::exception
    {
        public:

            IOException(const QString& _what) throw() : m_what(_what) {}

            virtual ~IOException()  throw() {}

            QString description() const throw() { return m_what; }

            virtual const char* what() const throw()
            {
                return m_what.toLocal8Bit().data();
            }

        private:
            QString m_what;
    };

    class IO : public QObject
    {
        Q_OBJECT

        IO();
        public:

            static IO* instance();

            void loadNew();
            void load(const QString& _path);
            void save(const QString& _as = QString());
            QString xmlFileContents() const;
            void unload();

            QString currentName() const { return m_name; }
            QString currentPath() const { return m_path; }
            int fileVersion() const { return m_fileVersion; }

            void setName(const QString& _name);

            bool isNew() const { return m_path.isEmpty(); }
            bool isDirty() const { return m_isDirty; }

            void registerStored(const QString& _xmlKey, IStored* _st);
            void registerFileManager(const QString& _xmlKey, const QString& _directoryName, IFileManager* _manager);

            static QString getAttribute(const QString& _name, QXmlStreamAttributes& _attrib);
            static QString getOptAttribute(const QString& _name,
                                           QXmlStreamAttributes& _attrib,
                                           const QVariant& _default = QVariant());

            static const int LATEST_FILE_VERSION;

        signals:
            void nameChanged();

            void isCleanNow();
            void isDirtyNow();

        private slots:
            void gotModifiedSignal();

        private:
            QString m_path;
            QString m_name;
            int m_fileVersion;

            QHash<QString, IStored*> m_registered;
            QHash<QString, IFileManager*> m_fileManagers;

            bool m_isDirty;

            static IO* m_instance;
    };

}

#endif // IO_H
