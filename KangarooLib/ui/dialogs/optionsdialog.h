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

#ifndef OPTIONSDIALOG_H
#define OPTIONSDIALOG_H

#include "camsegdialog.h"

class QRadioButton;

namespace KLib
{
    class OptionsDialog : public CAMSEGDialog
    {
        Q_OBJECT

        public:
            explicit OptionsDialog(const QPixmap& _icon,
                                   const QString& _title,
                                   const QString& _message,
                                   const QStringList& _options,
                                   int _selectedOption = 0,
                                   QWidget *parent = nullptr);

            /**
             * @brief Asks the user to select an option.
             *
             * Returns the index of the option if user selected OK, -1 otherwise.
             */
            static int askForOptions(const QPixmap& _icon,
                                     const QString& _title,
                                     const QString& _message,
                                     const QStringList& _options,
                                     int _selectedOption = 0,
                                     QWidget *parent = nullptr);

            QList<QRadioButton*> options() const { return m_options; }

            int selectedOption() const;

        private:
            QList<QRadioButton*> m_options;
    };

}

#endif // OPTIONSDIALOG_H
