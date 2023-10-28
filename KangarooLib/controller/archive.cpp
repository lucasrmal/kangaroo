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

#include "archive.h"
#include "../interfaces/ifilemanager.h"
#include "io.h"

#include "lib/quazip/quazip.h"
#include "lib/quazip/quazipfile.h"
#include "lib/quazip/quazipfileinfo.h"

#include <QDebug>

namespace KLib {
const QString Archive::MAIN_FILE_NAME = "main.xml";

Archive::Archive() : m_file(nullptr), m_mainFile(nullptr) {}

Archive::~Archive() { closeArchive(); }

void Archive::writeFile(const QString& _name,
                        std::function<void(QIODevice*)> _writer) {
  checkIfArchiveOpened();
  checkIfDirectorySet();

  QuaZipFile newFile(m_file);
  QuaZipNewInfo inf(m_currentDir + "/" + _name);
  inf.setPermissions(QFile::ReadOwner | QFile::WriteOwner);

  if (!newFile.open(QIODevice::WriteOnly, inf)) {
    throw IOException(QObject::tr("Unable to create the file!."));
  }

  _writer(&newFile);
  newFile.close();
}

bool Archive::fileExists(const QString& _name) const {
  checkIfArchiveOpened();

  return m_file->getFileNameList().contains(
      m_currentDir.isEmpty() ? _name : m_currentDir + "/" + _name);
}

void Archive::openArchive(const QString& _path, bool _writeMode) {
  if (m_file) {
    throw IOException(QObject::tr("The archive is already opened."));
  }

  m_file = new QuaZip(_path);

  if (!m_file->open(_writeMode ? QuaZip::mdCreate : QuaZip::mdUnzip)) {
    delete m_file;
    m_file = nullptr;
    throw IOException(
        QObject::tr("Unable to open the file. It may not be a valid Kangaroo"));
  }
}

void Archive::closeArchive() {
  if (m_file) {
    closeMainFile();

    m_file->close();
    delete m_file;
    m_file = nullptr;
    m_currentDir = "";
  }
}

bool Archive::isOpen() const { return m_file && m_file->isOpen(); }

void Archive::setCurrentDirectory(const QString& _dir) { m_currentDir = _dir; }

void Archive::loadFiles(IFileManager* _manager) {
  checkIfArchiveOpened();
  checkIfDirectorySet();

  for (QString filename : m_file->getFileNameList()) {
    if (filename.startsWith(m_currentDir + "/")) {
      // Open the file
      m_file->setCurrentFile(filename);
      QuaZipFile file(m_file);

      if (file.open(QIODevice::ReadOnly)) {
        _manager->loadFile(filename.remove(0, m_currentDir.size() + 1), &file);
        file.close();
      } else {
        qDebug()
            << QObject::tr("Unable to open file %1 in archive.").arg(filename);
      }
    }
  }
}

QIODevice* Archive::openMainFile(bool _writeMode) {
  checkIfArchiveOpened();
  m_currentDir.clear();

  if (m_mainFile) {
    if (m_mainFile->isOpen()) {
      return m_mainFile;
    } else {
      throw IOException(QObject::tr("Unable to open the file!"));
    }
  }

  if (_writeMode) {
    QuaZipNewInfo inf(MAIN_FILE_NAME);
    inf.setPermissions(QFile::ReadOwner | QFile::WriteOwner);

    m_mainFile = new QuaZipFile(m_file);

    if (!m_mainFile->open(QIODevice::WriteOnly, inf)) {
      delete m_mainFile;
      m_mainFile = nullptr;
      throw IOException(QObject::tr("Unable to create the file!."));
    }
  } else {
    if (!fileExists(MAIN_FILE_NAME) ||
        !m_file->setCurrentFile(MAIN_FILE_NAME)) {
      throw IOException(QObject::tr("The file is not a valid Kangaroo file!"));
    }

    m_mainFile = new QuaZipFile(m_file);

    if (!m_mainFile->open(QIODevice::ReadOnly)) {
      delete m_mainFile;
      m_mainFile = nullptr;
      throw IOException(QObject::tr("Unable to open the file!"));
    }
  }

  return m_mainFile;
}

void Archive::closeMainFile() {
  if (m_mainFile) {
    m_mainFile->close();
    delete m_mainFile;
    m_mainFile = nullptr;
  }
}

void Archive::checkIfArchiveOpened() const {
  if (!m_file || !m_file->isOpen()) {
    throw IOException(QObject::tr("The archive is not opened."));
  }
}

void Archive::checkIfDirectorySet() const {
  if (m_currentDir.isEmpty()) {
    throw IOException(QObject::tr("The directory is not set."));
  }
}
}
