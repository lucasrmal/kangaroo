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

#include "io.h"

#include <QFile>
#include <QRegExp>
#include <QXmlStreamReader>
#include <QXmlStreamWriter>
#include <stdexcept>

#include "../interfaces/ifilemanager.h"
#include "../model/account.h"
#include "../model/currency.h"
#include "../model/documentmanager.h"
#include "../model/institution.h"
#include "../model/investmentlotsmanager.h"
#include "../model/payee.h"
#include "../model/picturemanager.h"
#include "../model/pricemanager.h"
#include "../model/schedule.h"
#include "../model/security.h"
#include "../model/transactionmanager.h"
#include "archive.h"

namespace KLib {

IO* IO::m_instance = nullptr;

const int IO::LATEST_FILE_VERSION = 1;

const char* StdTags::ROOT = "kangaroo_file";
const char* StdTags::ACCOUNT = "account";
const char* StdTags::CURRENCY = "currency";
const char* StdTags::CURRENCY_MGR = "currencies";
const char* StdTags::INSTITUTION = "institution";
const char* StdTags::INSTITUTION_MGR = "institutions";
const char* StdTags::PRICE = "price";
const char* StdTags::PRICE_MGR = "prices";
const char* StdTags::EXCHANGE_PAIR = "exchangepair";
const char* StdTags::PAYEE = "payee";
const char* StdTags::PAYEE_MGR = "payees";
const char* StdTags::SECURITY = "security";
const char* StdTags::SECURITY_MGR = "securities";
const char* StdTags::SECURITY_INFOS = "security_infos";
const char* StdTags::SECURITY_INFO = "security_info";
const char* StdTags::TRANSACTION = "transaction";
const char* StdTags::INVEST_TRANSACTION = "inv_transaction";
const char* StdTags::TRANSACTION_MGR = "transactions";
const char* StdTags::SPLIT = "split";
const char* StdTags::INVEST_LOT = "inv_lot";
const char* StdTags::INVEST_LOT_USAGE = "inv_lots_usage";
const char* StdTags::INVEST_LOT_SPLIT = "inv_lots_split";
const char* StdTags::INVEST_LOT_TRANSFER = "inv_lot_transfer";
const char* StdTags::INVEST_LOT_MGR = "inv_lots";
const char* StdTags::PROPERTIES = "properties";
const char* StdTags::PROPERTY = "property";
const char* StdTags::PICTURE = "picture";
const char* StdTags::PICTURE_MGR = "pictures";
const char* StdTags::DOCUMENT = "document";
const char* StdTags::DOCUMENT_MGR = "documents";
const char* StdTags::SCHEDULE = "schedule";
const char* StdTags::SCHEDULE_REC = "recurrence";
const char* StdTags::SCHEDULE_MGR = "schedules";

IO::IO() : m_fileVersion(LATEST_FILE_VERSION) {
  // Register default storeables

  registerStored(StdTags::ACCOUNT, Account::getTopLevel());
  registerStored(StdTags::CURRENCY_MGR, CurrencyManager::instance());
  registerStored(StdTags::INSTITUTION_MGR, InstitutionManager::instance());
  registerStored(StdTags::PAYEE_MGR, PayeeManager::instance());
  registerStored(StdTags::PRICE_MGR, PriceManager::instance());
  registerStored(StdTags::SECURITY_MGR, SecurityManager::instance());
  registerStored(StdTags::TRANSACTION_MGR, TransactionManager::instance());
  registerStored(StdTags::SCHEDULE_MGR, ScheduleManager::instance());
  registerStored(StdTags::INVEST_LOT_MGR, InvestmentLotsManager::instance());

  // Register default file managers
  registerFileManager(StdTags::PICTURE_MGR, StdTags::PICTURE_MGR,
                      PictureManager::instance());
  registerFileManager(StdTags::DOCUMENT_MGR, StdTags::DOCUMENT_MGR,
                      DocumentManager::instance());

  m_isDirty = false;
}

IO* IO::instance() {
  if (!m_instance) m_instance = new IO();

  return m_instance;
}

void IO::loadNew() {
  for (IStored* s : m_registered) {
    s->loadNew();
  }
  for (IStored* s : m_registered) {
    s->afterLoad();
  }

  m_path = "";
  m_name = "";
  m_fileVersion = LATEST_FILE_VERSION;
  m_isDirty = false;
}

void IO::load(const QString& _path) {
  //        QFile* file = new QFile(_path);

  //        /* If we can't open it, let's show an error message. */
  //        if (!file->open(QIODevice::ReadOnly | QIODevice::Text))
  //        {
  //           throw IOException(tr("Unable to open file."));
  //        }

  Archive archive;
  archive.openArchive(_path);

  //--------------------------LOAD MAIN XML FILE--------------------------
  QXmlStreamReader xml(archive.openMainFile());

  /* We'll parse the XML until we reach end of it.*/
  while (!xml.atEnd() && !xml.hasError()) {
    /* Read next element.*/
    QXmlStreamReader::TokenType token = xml.readNext();
    /* If token is just StartDocument, we'll go to next.*/
    if (token == QXmlStreamReader::StartDocument) {
      continue;
    } else if (xml.name() == StdTags::ROOT) {
      if (token == QXmlStreamReader::StartElement) {
        QXmlStreamAttributes att = xml.attributes();
        m_name = getOptAttribute("name", att);
        m_fileVersion = getAttribute("version", att).toInt();

        if (m_fileVersion > LATEST_FILE_VERSION) {
          throw IOException(
              tr("The version of this file unsupported by this version. "
                 "Please update your software."));
        }
      }

      continue;
    }

    /* If token is StartElement, we'll see if we can read it.*/
    if (token == QXmlStreamReader::StartElement) {
      for (QString key : m_registered.keys()) {
        if (xml.name() == key) {
          m_registered[key]->load(xml);
        }
      }
    }
  }
  /* Error handling. */
  QString error;
  if (xml.hasError()) {
    error == xml.errorString();
  }

  xml.clear();
  archive.closeMainFile();

  if (!error.isEmpty()) {
    throw IOException(QString("Parsing error: %1").arg(xml.errorString()));
  }

  //--------------------------LOAD FILE MANAGERS--------------------------

  for (auto i = m_fileManagers.begin(); i != m_fileManagers.end(); ++i) {
    archive.setCurrentDirectory(i.key());
    archive.loadFiles(i.value());
  }

  //------------------------------AFTER LOAD------------------------------

  // Close the archive
  archive.closeArchive();

  // Call AfterLoad
  for (IStored* s : m_registered) {
    s->afterLoad();
  }

  m_path = _path;
  m_isDirty = false;
}

void IO::save(const QString& _as) {
  m_path = _as;

  //        QFile* file = new QFile(_as);

  //        /* If we can't open it, let's show an error message. */
  //        if (!file->open(QIODevice::WriteOnly))
  //        {
  //           throw IOException("Unable to open file.");
  //        }

  Archive archive;
  archive.openArchive(m_path, true);

  //--------------------------SAVE MAIN XML FILE--------------------------

  QXmlStreamWriter xml(archive.openMainFile(true));
  xml.setAutoFormatting(true);
  xml.writeStartDocument();

  // Root tag
  xml.writeStartElement(StdTags::ROOT);
  xml.writeAttribute("name", m_name);
  xml.writeAttribute("version", QString::number(LATEST_FILE_VERSION));

  // Write each registered Stored
  for (QString key : m_registered.keys()) {
    xml.writeStartElement(key);
    m_registered[key]->save(xml);
    xml.writeEndElement();
  }

  xml.writeEndElement();
  xml.writeEndDocument();

  archive.closeMainFile();
  //        file->close();
  //        delete file;

  //--------------------------SAVE FILE MANAGERS--------------------------
  for (auto i = m_fileManagers.begin(); i != m_fileManagers.end(); ++i) {
    archive.setCurrentDirectory(i.key());
    i.value()->saveFiles(archive);
  }

  //------------------------------AFTER SAVE------------------------------
  archive.closeArchive();

  m_isDirty = false;
  m_fileVersion = LATEST_FILE_VERSION;

  emit isCleanNow();
}

QString IO::xmlFileContents() const {
  QString ret;
  QXmlStreamWriter xml(&ret);
  xml.setAutoFormatting(true);
  xml.writeStartDocument();

  // Root tag
  xml.writeStartElement(StdTags::ROOT);
  xml.writeAttribute("name", m_name);
  xml.writeAttribute("version", QString::number(LATEST_FILE_VERSION));

  // Write each registered Stored
  for (QString key : m_registered.keys()) {
    xml.writeStartElement(key);
    m_registered[key]->save(xml);
    xml.writeEndElement();
  }

  xml.writeEndElement();
  xml.writeEndDocument();
  return ret;
}

void IO::unload() {
  for (IStored* s : m_registered) {
    s->unload();
  }
}

void IO::setName(const QString& _name) {
  if (_name != m_name) {
    m_name = _name;

    emit nameChanged();

    if (!isDirty()) emit isDirtyNow();
  }
}

QString IO::getAttribute(const QString& _name, QXmlStreamAttributes& _attrib) {
  if (_attrib.hasAttribute(_name)) {
    return _attrib.value(_name).toString();
  } else {
    throw IOException(QString("Required attribute %1 not found").arg(_name));
  }
}

QString IO::getOptAttribute(const QString& _name, QXmlStreamAttributes& _attrib,
                            const QVariant& _default) {
  if (_attrib.hasAttribute(_name)) {
    return _attrib.value(_name).toString();
  } else {
    return _default.toString();
  }
}

void IO::registerStored(const QString& _xmlKey, IStored* _st) {
  QRegExp reg("^[A-Za-z_]+$");

  if (!reg.exactMatch(_xmlKey)) {
    throw IOException(
        tr("This XML key is invalid. XML keys can only be composed of letters "
           "and underscores."));
  } else if (m_registered.contains(_xmlKey)) {
    throw IOException(tr("A Storeable with this key is already registered: %1.")
                          .arg(_xmlKey));
  }

  m_registered.insert(_xmlKey, _st);

  connect(_st, &IStored::modified, this, &IO::gotModifiedSignal);
}

void IO::registerFileManager(const QString& _xmlKey,
                             const QString& _directoryName,
                             IFileManager* _manager) {
  QRegExp reg("^[A-Za-z_]+$");

  if (!reg.exactMatch(_directoryName)) {
    throw IOException(
        tr("This directory is invalid. Directories name can only be composed "
           "of letters and underscores."));
  } else if (m_fileManagers.contains(_directoryName)) {
    throw IOException(
        tr("An IFileManager with this directory is already registered: %1.")
            .arg(_directoryName));
  } else if (m_registered.contains(_xmlKey)) {
    throw IOException(tr("A Storeable with this key is already registered: %1.")
                          .arg(_xmlKey));
  }

  m_fileManagers.insert(_directoryName, _manager);
  m_registered.insert(_xmlKey, _manager);

  connect(_manager, &IFileManager::modified, this, &IO::gotModifiedSignal);
}

void IO::gotModifiedSignal() {
  if (!m_isDirty) {
    m_isDirty = true;
    emit isDirtyNow();
  }
}
}
