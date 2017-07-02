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

#ifndef ISTORED_H
#define ISTORED_H

#include "../klib.h"
#include "../interfaces/scriptable.h"
#include <QString>
#include <QObject>

class QXmlStreamReader;
class QXmlStreamWriter;

namespace KLib
{

    class IStored : public QObject
    {
        Q_OBJECT
        K_SCRIPTABLE(IStored)

        Q_PROPERTY(int id READ id)

        public:
            virtual ~IStored() {}

            int id() const { return m_id; }

            Q_INVOKABLE virtual void holdToModify()  { m_onModifyHold = true; }
            Q_INVOKABLE virtual void doneHoldToModify();

            Q_INVOKABLE bool onHoldToModify() const { return m_onModifyHold; }

        signals:
            void modified();

        protected:
            IStored() : m_id(Constants::NO_ID), m_onModifyHold(false) {}
            IStored(int _id) : m_id(_id), m_onModifyHold(false) {}

            /**
             * @brief Will be called after all loading is done (ie. after the XML and all files are loaded).
             */
            virtual void afterLoad() {}

            /**
             * @brief load Loads the object from the XML file.
             * @param _reader
             */
            virtual void load(QXmlStreamReader& _reader) = 0;

            /**
             * @brief load Saves the object to the XML file.
             * @param _reader
             */
            virtual void save(QXmlStreamWriter& _writer) const = 0;

            /**
             * @brief Creates a "blank state" for the object
             */
            virtual void loadNew() { unload(); }

            /**
             * @brief Unloads the resources
             */
            virtual void unload() {}

            int m_id;

            bool m_onModifyHold;

            friend class IO;
    };

}

Q_DECLARE_METATYPE(KLib::IStored*)

#endif // ISTORED_H
