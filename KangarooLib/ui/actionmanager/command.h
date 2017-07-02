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
  @file  command.h
  This file contains the method declarations for the Command class

  @author   Lucas Rioux Maldague
  @date     May 26th 2010
  @version  3.0

*/

#ifndef COMMAND_H
#define COMMAND_H

#include <QKeySequence>

class QAction;

namespace KLib
{
    /**
      The Command class encapsulates a QAction. A Command can only be created using
      the ActionManager::registerAction() method.

      @brief Encapsulates a QAction
    */
    class Command
    {
        Command(const QString& p_id, QAction* p_action, const QKeySequence& p_defaultShortcut = QKeySequence());

    public:

        /**
          @return The command's ID name
        */
        QString         id() const { return m_id; }

        /**
          @return The command's QAction
        */
        QAction*        action() const { return m_action; }

        /**
          @return The command's default shortcut
        */
        QKeySequence    defaultShortcut() const { return m_defaultShortcut; }

        /**
          @return The command's current shortcut
        */
        QKeySequence    shortcut() const;

        /**
          Sets the command's default shortcut

          @param[in] p_defaultShortcut The new default shortcut
        */
        void            setDefaultShortcut(const QKeySequence& p_defaultShortcut) { m_defaultShortcut = p_defaultShortcut; }

        /**
          Cast to QAction
        */
        operator QAction*() { return m_action; }

    private:
        QString         m_id;
        QAction*        m_action;
        QKeySequence    m_defaultShortcut;

        friend class ActionManager;
    };
}

#endif // COMMAND_H
