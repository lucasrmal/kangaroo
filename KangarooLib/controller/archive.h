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

#ifndef ARCHIVE_H
#define ARCHIVE_H

#include <QString>
#include <functional>

class QIODevice;
class QuaZip;
class QuaZipFile;

namespace KLib
{
    class IFileManager;

    class Archive
    {
        Archive();
        ~Archive();

        public:

            void writeFile(const QString& _name, std::function<void(QIODevice*)> _writer);

            /**
             * @brief fileExists
             * @param _name The name of the file
             * @return If the file _name exists in currentDirectory()
             */
            bool fileExists(const QString& _name) const;

            QString currentDirectory() const { return m_currentDir; }

            static const QString MAIN_FILE_NAME;

        private:
            void openArchive(const QString& _path, bool _writeMode = false);
            void closeArchive();
            bool isOpen() const;

            /**
             * @brief Sets the current directory to _dir. Methods fileExists, writeFile now refer to this directory. If empty,
             *        sets the dir to the root of the archive.
             * @param _dir
             */
            void setCurrentDirectory(const QString& _dir);

            /**
             * @brief loadFiles Loads the file to IFileManager. A directory MUST be set.
             * @param _manager The file manager to which load the files.
             */
            void loadFiles(IFileManager* _manager);

            /**
             * @brief mainFile
             * @return The main XML file of the archive
             *
             * Be sure to call closeMainFile() when you're done with the main file before calling ANY other file read functions!
             *
             * This will reset the current dir to ""
             */
            QIODevice* openMainFile(bool _writeMode = false);

            /**
             * @brief Closes the main file if it is open.
             */
            void closeMainFile();

            void checkIfArchiveOpened() const;
            void checkIfDirectorySet() const;

            QString m_path;
            QString m_currentDir;
            QuaZip* m_file;

            QuaZipFile* m_mainFile;

            friend class IO;
    };

}

#endif // ARCHIVE_H
