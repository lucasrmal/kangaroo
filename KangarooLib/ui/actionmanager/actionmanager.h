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
  @file  actionmanager.h
  This file contains the method declarations for the ActionManager class

  @author   Lucas Rioux Maldague
  @date     May 24th 2010
  @version  3.0

*/

#ifndef ACTIONMANAGER_H
#define ACTIONMANAGER_H

#include <QObject>
#include <QHash>
#include <QKeySequence>
#include <QIcon>

class QAction;
class QMenu;
class QtToolBarManager;
class QToolBar;

namespace KLib
{
class ActionContainer;
class Command;

struct STD_MENUS
{
    static const char* MENUBAR;
    static const char* MENU_FILE;
    static const char* MENU_MANAGE;
    static const char* MENU_TOOLS;
    static const char* MENU_VIEW;
    static const char* MENU_HELP;
    static const char* MENU_FILE_OPENRECENT;
    static const char* MENU_VIEW_DOCKS;
};

/**
  The ActionManager manages the application's menus and toolbars. It uses ActionContainers and Commands to do so, which
  are wrappers around QMenu/QMenuBar and QAction, respectively.

  It's important to use this class instead of using directly the MainWindow's menubar, since it will not be possible then to
  assign shortcuts with the SCM Settings dialog and to add actions to the toolbars.

  Here is an example to add a new menu to the menubar:

  @code

  ActionManager* am = ActionManager::instance();

  am->createMenu(ActionManager::Constants::MENUBAR,
                 "MyMenu",
                 tr("MyMenu"));
  @endcode

  To add an action to our menu, with a default shortcut :

  @code

  ActionManager* am = ActionManager::instance();
  ActionContainer* myMenu = am->actionContainer("MyMenu");

  QAction* myAction = myMenu->menu()->addAction("MyAction");
  am->registerAction(myAction, "MyMenu.MyAction", QKeySequence("Ctrl+Shift+F12"));

  @endcode

  @brief Manages menus and toolbars
*/
class ActionManager
{
    ActionManager();

  public:



    ~ActionManager();

    static ActionManager*	    instance();
    static QtToolBarManager*   toolBarManager();
    static void                addToolBar(QToolBar* p_toolbar, const QString& p_name);
    static QToolBar*           toolbar(const QString& p_name);

    ActionContainer*    menuBar() const     { return actionContainer("Menubar"); }
    ActionContainer* 	actionContainer(const QString& p_id) const;
    Command*            command(const QString& p_id) const;

    ActionContainer* 	createMenu(const QString& p_parentId, const QString& p_id, const QString& p_title, const QIcon& p_icon = QIcon());
    ActionContainer*    insertMenu(const QString& p_parentId, const QString& _beforeId, const QString& p_id, const QString& p_title, const QIcon& p_icon = QIcon());
    ActionContainer*    registerMenu(QMenu* p_menu, const QString& p_id);
    Command*            registerAction(QAction* p_action,
                                       const QString& p_id,
                                       const QKeySequence& p_defaultShortcut = QKeySequence());

    bool                setShortcut(const QString& p_id, const QKeySequence& p_shortcut);
    QKeySequence        shortcut(const QString& p_id) const;

    QStringList         actionContainerNames() const { return m_containers.keys(); }
    QStringList         commandNames() const { return m_commands.keys(); }

    static QString             categoryForCommandId(const QString& p_id);

  private:
    static ActionManager*       m_instance;
    static QtToolBarManager*    m_toolBarManager;

    QHash<QString, Command*>    m_commands;
    QHash<QString, ActionContainer*> m_containers;

};

} //Namespace

#endif // ACTIONMANAGER_H
