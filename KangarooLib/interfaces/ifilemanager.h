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

#ifndef IFILEMANAGER_H
#define IFILEMANAGER_H

#include "../model/stored.h"

class QIODevice;

namespace KLib
{
    class Archive;

    class IFileManager : public IStored
    {
        Q_OBJECT


        signals:
            void modified();

        protected:
            virtual void loadFile(const QString& _name, QIODevice* _file) = 0;

            virtual void saveFiles(Archive& _archive) = 0;

            void load(QXmlStreamReader& _reader) { Q_UNUSED(_reader) }

            void save(QXmlStreamWriter& _writer) const { Q_UNUSED(_writer) }

            friend class IO;
            friend class Archive;

    };
}

#endif // IFILEMANAGER_H
