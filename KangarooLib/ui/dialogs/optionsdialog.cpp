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

#include "optionsdialog.h"

#include <QRadioButton>
#include <QVBoxLayout>
#include <QLabel>

namespace KLib
{

    OptionsDialog::OptionsDialog(const QPixmap& _icon,
                                 const QString& _title,
                                 const QString& _message,
                                 const QStringList& _options,
                                 int _selectedOption,
                                 QWidget *parent) :
        CAMSEGDialog(DialogWithPicture, OkCancelButtons, parent)
    {
        setBothTitles(_title);
        setPicture(_icon);

        QVBoxLayout* layout = new QVBoxLayout(centralWidget());

        QLabel* lbl = new QLabel(_message, this);
        layout->addWidget(lbl);

        for (const QString& s : _options)
        {
            QRadioButton* rb = new QRadioButton(s, this);
            layout->addWidget(rb);
            m_options << rb;
        }

        if (_selectedOption < m_options.count() && m_options.count())
        {
            m_options[_selectedOption < 0 ? 0 : _selectedOption]->setChecked(true);
        }
    }

    int OptionsDialog::askForOptions(const QPixmap& _icon,
                                     const QString& _title,
                                     const QString& _message,
                                     const QStringList& _options,
                                     int _selectedOption,
                                     QWidget *parent)
    {
        OptionsDialog dlg(_icon, _title, _message, _options, _selectedOption, parent);

        if (dlg.exec() == QDialog::Accepted)
        {
            return dlg.selectedOption();
        }
        else
        {
            return -1;
        }
    }

    int OptionsDialog::selectedOption() const
    {
        int i = 0;
        for (QRadioButton* b : m_options)
        {
            if (b->isChecked())
            {
                return i;
            }

            ++i;
        }

        return -1;
    }

}

