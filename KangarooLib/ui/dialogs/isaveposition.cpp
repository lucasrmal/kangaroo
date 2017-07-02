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
  @file  isaveposition.cpp
  This file contains the method definitions for the ISavePosition interface

  @author   Lucas Rioux Maldague
  @date     June 12th 2010
  @version  3.0

*/

#include "isaveposition.h"

#include <QSettings>
#include <QWidget>

namespace KLib {

    void ISavePosition::savePosition()
    {
        QSettings settings;
        settings.setValue("WindowPositions/" + m_uniqueName + "/pos", m_widget->pos());
        settings.setValue("WindowPositions/" + m_uniqueName + "/size", m_widget->size());
    }

    void ISavePosition::loadPosition()
    {
        QSettings settings;
        m_widget->resize(settings.value("WindowPositions/" + m_uniqueName + "/size", m_defaultSize).toSize());
        m_widget->move  (settings.value("WindowPositions/" + m_uniqueName + "/pos", m_defaultPosition).toPoint());
    }

}

