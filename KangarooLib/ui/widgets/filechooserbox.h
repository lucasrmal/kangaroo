/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2008-2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
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

#ifndef FILECHOOSERBOX_H
#define FILECHOOSERBOX_H

#include <QWidget>

class QLineEdit;
class QPushButton;

namespace KLib {

    class FileChooserBox : public QWidget
    {
        Q_OBJECT

        public:

            enum OperationType
            {
                Operation_Open,
                Operation_Save,
                Operation_GetDirectory
            };

                            FileChooserBox(const OperationType p_type, const QStringList& p_filter = QStringList(), QWidget *parent = 0);
                            FileChooserBox(const OperationType p_type, const QString& p_filePath, const QStringList& p_filter = QStringList(), QWidget *parent = 0);

            QString         filePath() const                                { return m_filePath; }
            bool            hasFile() const                                 { return !m_filePath.isEmpty(); }
            void            setFilePath(const QString& p_filePath);

            QStringList     filter() const                                  { return m_filter; }
            void            setFilter(const QStringList& p_filter);

            QString         selectedFormat() const                          { return m_selectedFormat; }

            OperationType   operationType() const                           { return m_type; }
            void            setOperationType(const OperationType p_type)    { m_type = p_type; }

            QString         dialogCaption() const                           { return m_dialogCaption; }
            void            setDialogCaption(const QString& p_caption)      { m_dialogCaption = p_caption; }

        public slots:
            void            openBrowseDialog();

        private:
            void            setupUI();

            OperationType   m_type;
            QString         m_filePath;
            QString         m_selectedFormat;
            QStringList     m_filter;
            QString         m_dialogCaption;

            QLineEdit*      m_lineEdit;
            QPushButton*    btnBrowse;

            QString         m_lastPath;

    };

} //Namespace

#endif // FILECHOOSERBOX_H
