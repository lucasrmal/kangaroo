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

#ifndef DOCUMENTMANAGER_H
#define DOCUMENTMANAGER_H

#include "../interfaces/ifilemanager.h"
#include "../interfaces/scriptable.h"

#include <QDate>
#include <QDateTime>
#include <QMimeType>
#include <QMimeDatabase>
#include <QHash>

namespace KLib
{
    struct Document
    {
        K_SCRIPTABLE(Document)

        public:

        Document(int _id, const QDate& _date, const QByteArray& _contents, const QMimeType& _mime, const QString& _memo = QString()) :
              id(_id),
              date(_date),
              contents(_contents),
              mime(_mime),
              memo(_memo)
        {
        }

        Document() : id(Constants::NO_ID) {}

        int         id;
        QDate       date;
        QByteArray  contents;
        QMimeType   mime;
        QString     memo;

        static QScriptValue toScriptValue(QScriptEngine *engine, const Document &d)
        {
          QScriptValue obj = engine->newObject();
          obj.setProperty("id", d.id);
          obj.setProperty("date", engine->newDate(QDateTime(d.date)));
          //obj.setProperty("contents", QVariant(d.contents));
          obj.setProperty("mime", d.mime.name());
          obj.setProperty("memo", d.memo);
          return obj;
        }

        static void fromScriptValue(const QScriptValue &obj, Document &d)
        {
          d.id = obj.property("id").toNumber();
          d.date = obj.property("date").toDateTime().date();
          //d.contents = obj.property("contents").toVariant().toByteArray();
          d.mime = QMimeDatabase().mimeTypeForName(obj.property("mime").toString());
          d.memo = obj.property("memo").toString();
        }
    };

    class DocumentManager : public IFileManager
    {
        Q_OBJECT
        K_SCRIPTABLE(PictureManager)

        Q_PROPERTY(int count READ count)

        DocumentManager();
        ~DocumentManager();

    public:
        static DocumentManager* instance() { return m_instance; }

        Q_INVOKABLE const Document* get(int _id) const;

        Q_INVOKABLE bool contains(int _id) const { return m_documents.contains(_id); }

        Q_INVOKABLE Document* add(const QString& _filePath);
        Q_INVOKABLE void display(int _id);
        Q_INVOKABLE void remove(int _id);
        Q_INVOKABLE void setMemo(int _id, const QString& _memo);

        Q_INVOKABLE QList<Document*> documents() const;

        int count() const { return m_documents.count(); }

        static const QString TEMP_VIEW_PATH;

    signals:
        void documentAdded(Document* d);
        void documentRemoved(Document* d);
        void documentModified(Document* d);


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
        QHash<int, Document*> m_documents;

        QHash<int, QString> m_openedFiles;

        int m_nextId;

        static DocumentManager* m_instance;

    };

}

#endif // DOCUMENTMANAGER_H
