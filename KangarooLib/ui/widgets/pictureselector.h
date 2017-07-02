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

#ifndef PICTURESELECTOR_H
#define PICTURESELECTOR_H

#include <QLabel>

namespace KLib
{

    class PictureSelector : public QLabel
    {
        Q_OBJECT

        public:
            explicit PictureSelector(QWidget *parent = 0);

            void setIdPicture(int _id);
            int idPicture() const { return  m_idPicture; }

            void setDefaultPicture(const QPixmap& _picture);

        protected:
            void mousePressEvent(QMouseEvent* _event);

        public slots:
            void selectPicture();
            void updatePicture();

        signals:
            void pictureChanged();

        private:
            int m_idPicture;
            QPixmap m_defaultPicture;

    };

}

#endif // PICTURESELECTOR_H
