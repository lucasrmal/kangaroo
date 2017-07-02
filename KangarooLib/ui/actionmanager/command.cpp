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
  @file  command.cpp
  This file contains the method definitions for the Command class

  @author   Lucas Rioux Maldague
  @date     May 26th 2010
  @version  3.0

*/

#include "command.h"

#include <QAction>

namespace KLib
{
    Command::Command(const QString& p_id, QAction* p_action, const QKeySequence& p_defaultShortcut) :
            m_id(p_id), m_action(p_action), m_defaultShortcut(p_defaultShortcut)
    {
    }

    QKeySequence Command::shortcut() const
    {
        return m_action->shortcut();
    }
}
