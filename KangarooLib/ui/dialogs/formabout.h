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

/**
  @file  frmabout.h
  File containing the method declarations for the "frmAbout" UI class

  @author   Lucas Rioux Maldague
  @date     January 8th 2009
  @version  3.0

*/

#ifndef FORMABOUT_H
#define FORMABOUT_H

#include <QDialog>

class QLabel;
class QTabWidget;
class QTextEdit;

namespace KLib {

    class frmAbout : public QDialog
    {
        Q_OBJECT

        public:
            explicit frmAbout(QWidget* _parent = nullptr);

        private slots:
            void launchBrowser(const QString& _address);

        private:
            void loadUI();
            void showLicense();

            QLabel* m_lblAppName;
            QLabel* m_lblIcon;
            QLabel* m_lblInfos;

            QTabWidget* onglets;

            QLabel* m_lblPicture;
            QLabel* m_lblGeneral;
            QLabel* m_lblAuthors;
            QTextEdit* m_txtLicence;

            QPushButton* m_btnClose;
    };

} //Namespace

#endif // FORMABOUT_H
