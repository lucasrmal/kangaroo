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

class QXmlStreamReader;
class QXmlStreamWriter;

namespace kangaroo
{

    class IStored : public QObject
    {
        Q_OBJECT
        K_SCRIPTABLE(IStored)

        Q_PROPERTY(int id READ id)

        public:
            virtual ~IStored() {}

            int id() const { return id_; }

            virtual void holdToModify()  { on_modify_hold_ = true; }
            virtual void doneHoldToModify();

            bool onHoldToModify() const { return on_modify_hold_; }

            constexpr int kNoId = -1;


        protected:
            IStored() : id_(kNoId), on_modify_hold_(false) {}
            IStored(int id) : id_(id), on_modify_hold_(false) {}

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

            int id_;
            bool on_modify_hold_;

            friend class IO;
    };

}

#endif // ISTORED_H
