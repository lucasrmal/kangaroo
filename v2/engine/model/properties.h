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

#ifndef PROPERTIES_H
#define PROPERTIES_H

#include <QHash>
#include <QVariant>
#include "../interfaces/scriptable.h"

class QXmlStreamReader;
class QXmlStreamWriter;

namespace KLib {

    class Properties : public QObject
    {
        Q_OBJECT
        K_SCRIPTABLE(Properties)

        Q_PROPERTY(int count READ count)

        public:
            Properties() {}
            virtual ~Properties() {}

            Q_INVOKABLE QVariant get(const QString& _key, const QVariant& _default = QVariant()) const
            {
                return m_properties.contains(_key) ? m_properties[_key]
                                                   : _default;
            }

            Q_INVOKABLE bool contains(const QString& _key) const { return m_properties.contains(_key); }
            Q_INVOKABLE void set(const QString _key, const QVariant& _value);
            Q_INVOKABLE void remove(const QString& _key);
            Q_INVOKABLE void clear();

            Q_INVOKABLE QStringList keys() const { return m_properties.keys(); }

            int count() const { return m_properties.count(); }

            void load(QXmlStreamReader& _reader);
            void save(QXmlStreamWriter& _writer) const;

        signals:
            void modified();

        private:
            QHash<QString, QVariant> m_properties;
    };

}

Q_DECLARE_METATYPE(KLib::Properties*)

#endif // PROPERTIES_H
