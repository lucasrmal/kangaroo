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

#ifndef GENERALSETTINGS_H
#define GENERALSETTINGS_H

#include "../../settingsmanager.h"

class QComboBox;
class QSpinBox;

namespace KLib {

    class GeneralSettings : public ISettingsPage
    {
        Q_OBJECT

        public:
            GeneralSettings(QWidget* parent = nullptr);

            QStringList validate() override;

            void save() override;
            void load() override;

        private:
            void loadLanguages();
            void loadDateFormats();

            QComboBox* m_cboLanguage;
            QComboBox* m_cboDateFormat;
            QSpinBox*  m_spinMaxRecent;
    };
	
} //Namespace

#endif // GENERALSETTINGS_H
