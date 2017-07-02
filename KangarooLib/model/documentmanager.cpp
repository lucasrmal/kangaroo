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

#include "documentmanager.h"
#include "modelexception.h"
#include "../controller/archive.h"
#include "../controller/io.h"

#include <QXmlStreamReader>
#include <QDesktopServices>
#include <QFileInfo>
#include <QFile>
#include <QDir>
#include <QUrl>
#include <ctime>

namespace KLib
{
    DocumentManager* DocumentManager::m_instance = new DocumentManager();

    const QString DocumentManager::TEMP_VIEW_PATH = QString("%1/kangaroopfm_temp_%2").arg(QDir::tempPath()).arg(time(nullptr));

    DocumentManager::DocumentManager() :
        m_nextId(0)
    {
        connect(this, &DocumentManager::documentAdded,    this, &DocumentManager::modified);
        connect(this, &DocumentManager::documentRemoved,  this, &DocumentManager::modified);
        connect(this, &DocumentManager::documentModified, this, &DocumentManager::modified);
    }

    DocumentManager::~DocumentManager()
    {
        unload();
    }

    const Document* DocumentManager::get(int _id) const
    {
        if (!m_documents.contains(_id))
        {
            ModelException::throwException(tr("The document %1 does not exists!").arg(_id), this);
        }

        return m_documents[_id];
    }

    Document* DocumentManager::add(const QString& _filePath)
    {
        QFileInfo info(_filePath);

        if (!info.exists())
        {
            ModelException::throwException(tr("The specified file does not exists."), this);
            return nullptr;
        }

        QFile file(_filePath);

        if (!file.open(QFile::ReadOnly))
        {
            ModelException::throwException(tr("Unable to read the specified file: %1").arg(file.errorString()), this);
            return nullptr;
        }

        QMimeDatabase mimedata;

        Document* d = new Document(m_nextId++,
                                   QDate::currentDate(),
                                   file.readAll(),
                                   mimedata.mimeTypeForFile(info),
                                   info.fileName());

        m_documents[d->id] = d;

        return d;
    }

    void DocumentManager::display(int _id)
    {
        if (!m_documents.contains(_id))
        {
            ModelException::throwException(tr("The document %1 does not exists!").arg(_id), this);
        }

        //Check if the file was already opened in the current session
        if (!m_openedFiles.contains(_id))
        {
            Document* d = m_documents[_id];

            m_openedFiles[_id] = QString("%1_%2.%3").arg(TEMP_VIEW_PATH).arg(_id).arg(d->mime.preferredSuffix());

            QFile f(m_openedFiles[_id]);

            if (!f.open(QIODevice::WriteOnly | QIODevice::Truncate))
            {
                m_openedFiles.remove(_id);
                ModelException::throwException(tr("Unable to display the file: %1").arg(f.errorString()), this);
                return;
            }

            f.write(d->contents);
            f.close();
        }

        //Open the file
        if (!QDesktopServices::openUrl(QUrl::fromLocalFile(m_openedFiles[_id])))
            ModelException::throwException(tr("Unable to open the document - check that the required visualization"
                                              "application is installed on the system."), this);
    }

    void DocumentManager::remove(int _id)
    {
        if (!m_documents.contains(_id))
        {
            ModelException::throwException(tr("The document %1 does not exists!").arg(_id), this);
        }

        Document* d = m_documents.take(_id);
        emit documentRemoved(d);
        delete d;
    }

    void DocumentManager::setMemo(int _id, const QString& _memo)
    {
        if (!m_documents.contains(_id))
        {
            ModelException::throwException(tr("The document %1 does not exists!").arg(_id), this);
        }

        m_documents[_id]->memo = _memo;
        emit documentModified(m_documents[_id]);
    }

    QList<Document*> DocumentManager::documents() const
    {
        return m_documents.values();
    }

    void DocumentManager::loadFile(const QString& _name, QIODevice* _file)
    {
        int id = _name.toInt();

        if (m_documents.contains(id))
        {
            m_documents[id]->contents = _file->readAll();
        }
        else
        {
            qDebug() << tr("Unknown document received: %1").arg(_name);
        }
    }

    void DocumentManager::saveFiles(Archive& _archive)
    {
        for (Document* d : m_documents)
        {
            _archive.writeFile(QString::number(d->id), [d] (QIODevice* device)
            {
                device->write(d->contents);
            });
        }
    }

    void DocumentManager::load(QXmlStreamReader& _reader)
    {
        unload();
        QMimeDatabase mimedata;

        while (!(_reader.tokenType() == QXmlStreamReader::EndElement && _reader.name() == StdTags::DOCUMENT_MGR))
        {
            if (_reader.tokenType() == QXmlStreamReader::StartElement && _reader.name() == StdTags::DOCUMENT)
            {
                QXmlStreamAttributes attributes = _reader.attributes();
                Document* d = new Document(IO::getAttribute("id", attributes).toInt(),
                                           QDate::fromString(IO::getAttribute("date", attributes), Qt::ISODate),
                                           QByteArray(),
                                           mimedata.mimeTypeForName(IO::getAttribute("mime", attributes)),
                                           IO::getOptAttribute("memo", attributes, QString()));

                m_nextId = std::max(m_nextId, d->id+1);

                m_documents[d->id] = d;
            }

            _reader.readNext();
        }
    }

    void DocumentManager::save(QXmlStreamWriter& _writer) const
    {
        for (Document* d : m_documents)
        {
            _writer.writeEmptyElement(StdTags::DOCUMENT);
            _writer.writeAttribute("id", QString::number(d->id));
            _writer.writeAttribute("date", d->date.toString(Qt::ISODate));
            _writer.writeAttribute("mime", d->mime.name());
            _writer.writeAttribute("memo", d->memo);
        }
    }

    void DocumentManager::afterLoad()
    {
        //Check if all the documents have been loaded, delete them if invalid.
        QList<int> bad;

        for (Document* d : m_documents)
        {
            if (d->contents.isEmpty())
            {
                bad << d->id;
            }
        }

        for (int i : bad)
        {
            delete m_documents.take(i);
        }
    }

    void DocumentManager::unload()
    {
        //Documents
        for (Document* d : m_documents)
        {
            delete d;
        }

        //Temp files
        QDir d;
        for (const QString& f : m_openedFiles)
        {
            d.remove(f);
        }

        m_documents.clear();
        m_openedFiles.clear();
        m_nextId = 0;
    }

}
