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

#include "picturemanager.h"
#include "modelexception.h"
#include "../controller/archive.h"
#include "../controller/io.h"

#include <QImageReader>
#include <QXmlStreamReader>

namespace KLib
{
    PictureManager* PictureManager::m_instance = new PictureManager();

    const QSize PictureManager::THUMBNAIL_SIZE = QSize(48, 48);
    const char* PictureManager::SAVE_FILE_FORMAT = "PNG";

    PictureManager::PictureManager() :
        m_nextId(0)
    {
        connect(this, &PictureManager::pictureAdded,    this, &PictureManager::modified);
        connect(this, &PictureManager::pictureRemoved,  this, &PictureManager::modified);
        connect(this, &PictureManager::pictureModified, this, &PictureManager::modified);
    }

    PictureManager::~PictureManager()
    {
        unload();
    }

    const Picture* PictureManager::get(int _id) const
    {
        if (!m_pictures.contains(_id))
        {
            ModelException::throwException(tr("The picture %1 does not exists!").arg(_id), this);
        }

        return m_pictures[_id];
    }

    QPixmap& PictureManager::pixmap(int _id) const
    {
        if (!m_pictures.contains(_id))
        {
            ModelException::throwException(tr("The picture %1 does not exists!").arg(_id), this);
        }

        return m_pictures[_id]->picture;
    }

    int PictureManager::add(const QString& _name, const QPixmap& _picture)
    {
        Picture* p = new Picture(m_nextId++, _name, _picture);
        m_pictures[p->id] = p;
        emit pictureAdded(p);
        return p->id;
    }

    void PictureManager::remove(int _id)
    {
        if (!m_pictures.contains(_id))
        {
            ModelException::throwException(tr("The picture %1 does not exists!").arg(_id), this);
        }

        emit pictureRemoved(m_pictures[_id]);

        delete m_pictures.take(_id);
    }

    void PictureManager::setName(int _id, const QString& _name)
    {
        if (!m_pictures.contains(_id))
        {
            ModelException::throwException(tr("The picture %1 does not exists!").arg(_id), this);
        }

        m_pictures[_id]->name = _name;
        emit pictureModified(m_pictures[_id]);
    }

    void PictureManager::setPicture(int _id, const QPixmap& _picture)
    {
        if (!m_pictures.contains(_id))
        {
            ModelException::throwException(tr("The picture %1 does not exists!").arg(_id), this);
        }

        m_pictures[_id]->picture = _picture;
        m_pictures[_id]->computeThumbnail();
        emit pictureModified(m_pictures[_id]);
    }

    void Picture::computeThumbnail()
    {
        if (picture.isNull())
        {
            thumbnail = QPixmap();
        }
        else
        {
            thumbnail = picture.scaled(PictureManager::THUMBNAIL_SIZE,
                                       Qt::KeepAspectRatio,
                                       Qt::SmoothTransformation);
        }
    }

    QList<Picture*> PictureManager::pictures() const
    {
        return m_pictures.values();
    }

    void PictureManager::loadFile(const QString& _name, QIODevice* _file)
    {
        int id = _name.toInt();

        if (m_pictures.contains(id))
        {
            QImageReader reader(_file, PictureManager::SAVE_FILE_FORMAT);
            m_pictures[id]->picture = QPixmap::fromImageReader(&reader);
            m_pictures[id]->computeThumbnail();
        }
        else
        {
            qDebug() << tr("Unknown picture file received: %1").arg(_name);
        }
    }

    void PictureManager::saveFiles(Archive& _archive)
    {
        for (Picture* p : m_pictures)
        {
            _archive.writeFile(QString::number(p->id), [p](QIODevice* d)
            {
                p->picture.save(d, PictureManager::SAVE_FILE_FORMAT);
            });
        }
    }

    void PictureManager::load(QXmlStreamReader& _reader)
    {
        unload();

        // While not at end of pictures
        while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::PICTURE_MGR))
        {
            if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::PICTURE)
            {
                QXmlStreamAttributes attributes = _reader.attributes();
                Picture* p = new Picture(IO::getAttribute("id", attributes).toInt(),
                                         IO::getAttribute("name", attributes),
                                         QPixmap());
                m_nextId = std::max(m_nextId, p->id+1);

                m_pictures[p->id] = p;
            }

            _reader.readNext();
        }
    }

    void PictureManager::save(QXmlStreamWriter& _writer) const
    {
        for (Picture* p : m_pictures)
        {
            _writer.writeEmptyElement(StdTags::PICTURE);
            _writer.writeAttribute("id", QString::number(p->id));
            _writer.writeAttribute("name", p->name);
        }
    }

    void PictureManager::afterLoad()
    {
        //Check if all the pictures have been loaded, delete them if invalid.
        QList<int> bad;

        for (Picture* p : m_pictures)
        {
            if (p->picture.isNull())
            {
                bad << p->id;
            }
        }

        for (int i : bad)
        {
            delete m_pictures.take(i);
        }
    }

    void PictureManager::unload()
    {
        for (Picture* p : m_pictures)
        {
            delete p;
        }

        m_pictures.clear();
        m_nextId = 0;
    }

}
