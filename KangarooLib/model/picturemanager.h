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

#ifndef PICTUREMANAGER_H
#define PICTUREMANAGER_H

#include "../interfaces/ifilemanager.h"
#include <QHash>
#include <QPixmap>

namespace KLib
{
    struct Picture
    {

        public:

        Picture(int _id, const QString& _name, const QPixmap& _picture) :
            id(_id), name(_name), picture(_picture)
        {
            computeThumbnail();
        }

        Picture() : id(Constants::NO_ID) {}

        int id;
        QString name;
        QPixmap picture;
        QPixmap thumbnail;

        void computeThumbnail();
    };

    class PictureManager : public IFileManager
    {
        Q_OBJECT

        Q_PROPERTY(int count READ count)

        PictureManager();
        ~PictureManager();

        public:
            static PictureManager* instance() { return m_instance; }

            Q_INVOKABLE const Picture* get(int _id) const;
            Q_INVOKABLE QPixmap& pixmap(int _id) const;

            Q_INVOKABLE int add(const QString& _name, const QPixmap& _picture);
            Q_INVOKABLE void remove(int _id);
            Q_INVOKABLE void setName(int _id, const QString& _name);
            Q_INVOKABLE void setPicture(int _id, const QPixmap& _picture);

            Q_INVOKABLE QList<Picture*> pictures() const;

            int count() { return m_pictures.count(); }

            static const QSize THUMBNAIL_SIZE;
            static const char* SAVE_FILE_FORMAT;

        signals:
            void pictureAdded(KLib::Picture* p);
            void pictureRemoved(KLib::Picture* p);
            void pictureModified(KLib::Picture* p);


        protected:
            //From IFileManager
            void loadFile(const QString& _name, QIODevice* _file);

            void saveFiles(Archive& _archive);

            //From Stored
            void load(QXmlStreamReader& _reader);
            void save(QXmlStreamWriter& _writer) const;
            void afterLoad();
            void unload();

        private:
            QHash<int, Picture*> m_pictures;

            int m_nextId;

            static PictureManager* m_instance;
    };

}

Q_DECLARE_METATYPE(KLib::Picture)
Q_DECLARE_METATYPE(KLib::PictureManager*)

#endif // PICTUREMANAGER_H
