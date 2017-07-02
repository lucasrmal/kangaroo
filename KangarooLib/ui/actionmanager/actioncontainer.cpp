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
  @file  actioncontainer.cpp
  This file contains the method definitions for the ActionContainer class

  @author   Lucas Rioux Maldague
  @date     May 26th 2010
  @version  3.0

*/

#include "actioncontainer.h"

#include <QMenu>

namespace KLib
{
    bool ActionContainer::addAction(QAction* p_action, ActionPosition p_position)
    {
        if (m_menu && p_action)
        {
            if (p_position == AtEnd || m_menu->actions().count() == 0)
            {
                m_menu->addAction(p_action);
            }
            else
            {
                m_menu->insertAction(m_menu->actions().at(0), p_action);
            }

            return true;
        }
        else return false;
    }

    bool ActionContainer::insertActionBefore(QAction* p_action, QAction* p_before)
    {
        if (m_menu && p_action)
        {
            m_menu->insertAction(p_before, p_action);
            return true;
        }
        else return false;
    }

    bool ActionContainer::insertActionAfter(QAction* p_action, QAction* p_after)
    {
        if (m_menu && p_action && p_after)
        {
            int index = m_menu->actions().indexOf(p_after);

            if (index == -1)
                return false;

            if (m_menu->actions().count() == 0 || (m_menu->actions().count() - 1) == index)
            {
                m_menu->addAction(p_action);
            }
            else
            {
                m_menu->insertAction(m_menu->actions().at(index + 1), p_action);
            }

            return true;
        }
        else return false;
    }

    QAction* ActionContainer::insertSeparatorIfNotExists(const QString& p_name, QAction* p_before)
    {
        if (m_menu && !m_separators.value(p_name, NULL)) //Separator does not exists
        {
            QAction* action = new QAction(m_menu);
            action->setSeparator(true);

            insertActionBefore(action, p_before);
            m_separators[p_name] = action;

            return action;
        }
        else if (m_menu)
        {
            return m_separators[p_name];
        }

        return NULL;
    }

} //Namespace


