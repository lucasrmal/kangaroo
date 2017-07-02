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

#ifndef FORMPICTUREMANAGER_H
#define FORMPICTUREMANAGER_H

#include "camsegdialog.h"

class QListView;
class QLineEdit;

namespace KLib
{

    class FormAddEditPicture : public CAMSEGDialog
    {
        Q_OBJECT

        public:

            explicit FormAddEditPicture(int _id, QWidget* _parent);

            int idAddedPicture() const;
        public slots:
            void accept();
            void selectPicture();

        private slots:
            void updatePicture();

        private:
            QLabel* m_lblPicture;
            QLineEdit* m_txtName;

            QPixmap m_currentPicture;


            int m_id;
    };

    class FormPictureManager : public CAMSEGDialog
    {
        Q_OBJECT

        public:

            enum PictureSelectionMode
            {
                AllowNoPicture,
                MustSelectPicture
            };


            /**
             * @brief This modes is for managing pictures
             * @param parent
             */
            explicit FormPictureManager(QWidget *parent = 0);

            /**
             * @brief This mode is for selecting a picture
             * @param _current The current selected picture.
             * @param parent
             */
            explicit FormPictureManager(int _current, PictureSelectionMode _mode, QWidget *parent = 0);

            int currentPictureId() const;

            QSize sizeHint() const;

        public slots:
            void accept();
            void selectNone();
            void addPicture();
            void removePicture();
            void editPicture();

        private:
            void setupUI();
            void selectPicture(int _id);

            QListView* m_view;
            bool m_inSelect;
            bool m_selectedNone;

    };

}

#endif // FORMPICTUREMANAGER_H
