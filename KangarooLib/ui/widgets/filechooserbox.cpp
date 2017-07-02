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

#include "filechooserbox.h"

#include <QHBoxLayout>
#include <QPushButton>
#include <QLineEdit>
#include <QFileDialog>

namespace KLib {

    FileChooserBox::FileChooserBox(const OperationType p_type, const QStringList& p_filter, QWidget *parent) :
        QWidget(parent), m_type(p_type), m_filter(p_filter), m_dialogCaption(tr("Select a file")), m_lastPath(QDir::homePath())
    {
        setupUI();
    }

    FileChooserBox::FileChooserBox(const OperationType p_type, const QString& p_filePath, const QStringList& p_filter, QWidget *parent) :
        QWidget(parent), m_type(p_type), m_filePath(p_filePath), m_filter(p_filter), m_dialogCaption(tr("Select a file")), m_lastPath(QDir::homePath())
    {
        setupUI();
    }

    void FileChooserBox::setFilePath(const QString& p_filePath)
    {
        m_filePath = p_filePath;
        m_lineEdit->setText(p_filePath);
    }

    void FileChooserBox::setFilter(const QStringList& p_filter)
    {
        if (m_filter != p_filter)
            m_lineEdit->clear();

        m_filter = p_filter;
    }

    void FileChooserBox::openBrowseDialog()
    {
        QString fullFilter, path;

        foreach (QString str, m_filter)
        {
            if ( !fullFilter.isEmpty())
                fullFilter += ";;" + str;
        }

        if (operationType() == Operation_Open)
        {
            path = QFileDialog::getOpenFileName(this, dialogCaption(), m_lastPath, fullFilter, &m_selectedFormat);
        }
        else if (operationType() == Operation_Save)
        {
            path = QFileDialog::getSaveFileName(this, dialogCaption(), m_lastPath, fullFilter, &m_selectedFormat);
        }
        else if (operationType() == Operation_GetDirectory)
        {
            path = QFileDialog::getExistingDirectory(this, dialogCaption(), m_lastPath);
        }

        if (path.isEmpty())
            return;

        m_lineEdit->setText(path);
        m_filePath = path;
        m_lastPath = path;
    }

    void FileChooserBox::setupUI()
    {
        m_lineEdit = new QLineEdit(m_filePath, this);
        m_lineEdit->setReadOnly(true);

        btnBrowse = new QPushButton(tr("&Browse"), this);
        btnBrowse->setMaximumWidth(75);

        QHBoxLayout* mainLayout = new QHBoxLayout(this);
        mainLayout->addWidget(m_lineEdit);
        mainLayout->addWidget(btnBrowse);

        connect(btnBrowse, SIGNAL(clicked()), this, SLOT(openBrowseDialog()));
    }

} //Namespace
