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
  @file  actioncontainer.h
  This file contains the method declarations for the ActionContainer class

  @author   Lucas Rioux Maldague
  @date     May 26th 2010
  @version  3.0

*/

#ifndef ACTIONCONTAINER_H
#define ACTIONCONTAINER_H

#include <QString>
#include <QHash>

class QAction;
class QMenu;
class QMenuBar;

namespace KLib
{
    /**
      The ActionContainer class encapsulates a menu or a menubar. If a menu is encapsulated, it offers various
      helper methods to add and insert actions and separators to the menu. An ActionContainer can only be created
      with ActionManager::createMenu() (for new menus) or ActionManager::registerMenu() (to convert existing QMenus).

      @brief Encapsulates a menu or a menubar.
    */
    class ActionContainer
    {
        ActionContainer(const QString& p_id, QMenu* p_menu) : m_id(p_id), m_menu(p_menu), m_menuBar(0) {}
        ActionContainer(const QString& p_id, QMenuBar* p_menuBar) : m_id(p_id), m_menu(0), m_menuBar(p_menuBar) {}

    public:

        /**
          Position of the action to add in the menu
        */
        enum ActionPosition
        {
            AtBeginning, ///< At the beginning of the menu
            AtEnd        ///< Ath the end of the menu
        };

        /**
          @return The action manager's ID, which identifies it in the ActionManager
        */
        QString     id() const      { return m_id; }

        /**
          @return The encapsulated menu if it's a menu (most cases, since there's only one menubar), NULL otherwise
        */
        QMenu*      menu() const    { return m_menu; }

        /**
          @return The encapsulated menubar if it's the MenuBar's ActionContainer or NULL if it encapsulates
          a menu.
        */
        QMenuBar*   menuBar() const { return m_menuBar; }

        /**
          Adds an action to the menu. The ActionContainer must be a menu for this to work.

          @param[in] p_action The action to add
          @param[in] p_position Where to add the action

          @return If the action was succesfully added to the menu.
        */
        bool        addAction(QAction* p_action, ActionPosition p_position = AtEnd);

        /**
          Inserts p_action before p_before. The ActionContainer must be a menu for this to work.

          If p_before is NULL or if it does not belongs to this container, it appends p_action
          to the end of the menu.

          @return If the action was succesfully added to the menu.
        */
        bool        insertActionBefore(QAction* p_action, QAction* p_before);

        /**
          Inserts p_action after p_after. The ActionContainer must be a menu for this to work.

          If p_after is NULL or if it does not belongs to this container, it does not add p_action.

          @return If the action was succesfully added to the menu.
        */
        bool        insertActionAfter(QAction* p_action, QAction* p_after);

        /**
          Inserts the separator named p_name before p_before if it does not already exists. If
          p_before is NULL or if it does not belongs to this container, the separator is added
          at the end of the menu.

          @return The action of the separator, or NULL if the ActionContainer is a menubar.
        */
        QAction*    insertSeparatorIfNotExists(const QString& p_name, QAction* p_before = NULL);

    private:
        const QString   m_id;
        QMenu*          m_menu;
        QMenuBar*       m_menuBar;

        QHash<QString, QAction*> m_separators;

        friend class ActionManager;
    };
}

#endif // ACTIONCONTAINER_H
