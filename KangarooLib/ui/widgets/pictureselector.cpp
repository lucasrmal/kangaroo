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

#include "pictureselector.h"
#include "../../model/picturemanager.h"
#include "../../model/modelexception.h"
#include "../dialogs/formpicturemanager.h"

namespace KLib
{

    PictureSelector::PictureSelector(QWidget *parent) :
        QLabel(parent),
        m_idPicture(Constants::NO_ID)
    {
        setFixedSize(PictureManager::THUMBNAIL_SIZE);
    }

    void PictureSelector::mousePressEvent(QMouseEvent* _event)
    {
        Q_UNUSED(_event)
        selectPicture();
    }

    void PictureSelector::setIdPicture(int _id)
    {
        m_idPicture = _id;
        updatePicture();
    }

    void PictureSelector::selectPicture()
    {
        FormPictureManager pic(m_idPicture, FormPictureManager::AllowNoPicture, this);

        if (pic.exec() == QDialog::Accepted)
        {
            m_idPicture = pic.currentPictureId();
            updatePicture();
        }
    }

    void PictureSelector::updatePicture()
    {
        if (m_idPicture != Constants::NO_ID)
        {
            try
            {
                setPixmap(PictureManager::instance()->get(m_idPicture)->thumbnail);
                return;
            }
            catch (ModelException)
            {
                m_idPicture = Constants::NO_ID;
            }
        }

        if (!m_defaultPicture.isNull())
        {
            setPixmap(m_defaultPicture);
        }
    }

    void PictureSelector::setDefaultPicture(const QPixmap& _picture)
    {
        m_defaultPicture = _picture;
        updatePicture();
    }

}
