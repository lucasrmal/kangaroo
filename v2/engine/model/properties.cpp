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

#include "properties.h"
#include "modelexception.h"
#include "../controller/io.h"

#include <QXmlStreamReader>

namespace KLib {

    void Properties::set(const QString _key, const QVariant& _value)
    {
        if (_key.isEmpty())
        {
            ModelException::throwException(tr("A property key cannot be empty."), NULL);
        }

        m_properties[_key] = _value;

        emit modified();
    }

    void Properties::remove(const QString& _key)
    {
        if (m_properties.contains(_key))
        {
            m_properties.remove(_key);
            emit modified();
        }
    }

    void Properties::clear()
    {
        if (!m_properties.isEmpty())
        {
            m_properties.clear();
            emit modified();
        }
    }

    void Properties::load(QXmlStreamReader& _reader)
    {
        m_properties.clear();

        while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::PROPERTIES))
        {
            if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::PROPERTY)
            {
                QXmlStreamAttributes att = _reader.attributes();

                m_properties[IO::getAttribute("key", att)] = IO::getAttribute("value", att);
            }

            _reader.readNext();
        }
    }

    void Properties::save(QXmlStreamWriter& _writer) const
    {
        if (m_properties.isEmpty())
            return;

        _writer.writeStartElement(StdTags::PROPERTIES);

        for (auto i = m_properties.begin(); i != m_properties.end(); ++i)
        {
            _writer.writeEmptyElement(StdTags::PROPERTY);
            _writer.writeAttribute("key", i.key());
            _writer.writeAttribute("value", i.value().toString());
        }

        _writer.writeEndElement();
    }

}
