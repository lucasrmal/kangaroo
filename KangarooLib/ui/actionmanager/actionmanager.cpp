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
  @file  actionmanager.cpp
  This file contains the method definitions for the ActionManager class

  @author   Lucas Rioux Maldague
  @date     May 24th 2010
  @version  3.0

*/

#include "actionmanager.h"
#include "actioncontainer.h"
#include "command.h"
#include "qttoolbardialog.h"
#include "../core.h"
#include "../mainwindow.h"

#include <QMenuBar>
#include <QMenu>
#include <QSettings>
#include <QCoreApplication>
#include <QToolBar>

namespace KLib
{
ActionManager*    ActionManager::m_instance            = nullptr;
QtToolBarManager* ActionManager::m_toolBarManager      = nullptr;

const char*       STD_MENUS::MENUBAR    = "Menubar";
const char*       STD_MENUS::MENU_FILE  = "File";
const char*       STD_MENUS::MENU_MANAGE  = "Manage";
const char*       STD_MENUS::MENU_TOOLS = "Tools";
const char*       STD_MENUS::MENU_VIEW  = "View";
const char*       STD_MENUS::MENU_HELP  = "Help";
const char*       STD_MENUS::MENU_FILE_OPENRECENT  = "File.OpenRecent";
const char*       STD_MENUS::MENU_VIEW_DOCKS       = "View.Docks";

ActionManager* ActionManager::instance()
{
  if ( !m_instance)
    m_instance = new ActionManager();

  return m_instance;
}

QtToolBarManager* ActionManager::toolBarManager()
{
  if ( !m_toolBarManager)
  {
    m_toolBarManager = new QtToolBarManager();
    m_toolBarManager->setMainWindow(Core::instance()->mainWindow());
  }

  return m_toolBarManager;
}

void ActionManager::addToolBar(QToolBar* p_toolbar, const QString& p_name)
{
  if (p_toolbar)
  {
    p_toolbar->setObjectName(p_name);
    p_toolbar->setToolButtonStyle(Qt::ToolButtonTextBesideIcon);

    toolBarManager()->addToolBar(p_toolbar, "");
    Core::instance()->mainWindow()->addToolBar(p_toolbar);
  }
}

QToolBar* ActionManager::toolbar(const QString& p_name)
{
  QList<QToolBar*> toolbars = toolBarManager()->toolBars();

  foreach (QToolBar* tbar, toolbars)
  {
    if (tbar->objectName() == p_name)
      return tbar;
  }

  return nullptr;
}

ActionManager::ActionManager()
{
  //Fetch the Core instance
  Core* core = Core::instance();

  //Create the menubar
  ActionContainer* menuBar = new ActionContainer(STD_MENUS::MENUBAR, core->mainWindow()->menuBar());
  m_containers[STD_MENUS::MENUBAR] = menuBar;

  //Add the default actions

  ActionContainer* mnuFile  = createMenu(STD_MENUS::MENUBAR, STD_MENUS::MENU_FILE, QObject::tr("&File"));
  ActionContainer* mnuManage = createMenu(STD_MENUS::MENUBAR, STD_MENUS::MENU_MANAGE, QObject::tr("&Manage"));
  ActionContainer* mnuTools = createMenu(STD_MENUS::MENUBAR, STD_MENUS::MENU_TOOLS, QObject::tr("&Tools"));
  ActionContainer* mnuView = createMenu(STD_MENUS::MENUBAR, STD_MENUS::MENU_VIEW, QObject::tr("&View"));
  ActionContainer* mnuHelp  = createMenu(STD_MENUS::MENUBAR, STD_MENUS::MENU_HELP, QObject::tr("&Help"));

  //File
  QAction* mnuOpen_File    = mnuFile->menu()->addAction(Core::icon("open"), QObject::tr("&Open"), core, SLOT(selectOpenFile()));

  ActionContainer* mnuRecentFiles = createMenu(STD_MENUS::MENU_FILE, STD_MENUS::MENU_FILE_OPENRECENT, QObject::tr("&Recent Files"));
  mnuRecentFiles->menu()->setIcon(Core::icon("open_recent"));

#ifdef QT_DEBUG
  mnuFile->menu()->addAction(QObject::tr("Show XML File"), core, SLOT(showXMLFile()));
#endif

  QAction* mnuClose        = mnuFile->menu()->addAction(Core::icon("close"), QObject::tr("&Close"), core, SLOT(unloadFile()));
  mnuFile->menu()->addSeparator();
  QAction* mnuPicManager   = mnuFile->menu()->addAction(Core::icon("picture-manager"), QObject::tr("&Picture Manager"), core, SLOT(showPictureManager()));
  mnuFile->menu()->addSeparator();
  QAction* mnuSave         = mnuFile->menu()->addAction(Core::icon("save"), QObject::tr("&Save"), core, SLOT(saveFile()));
  QAction* mnuSaveAs       = mnuFile->menu()->addAction(Core::icon("save-as"), QObject::tr("Save &As"), core, SLOT(saveFileAs()));
  mnuFile->menu()->addSeparator();
  QAction* mnuQuit         = mnuFile->menu()->addAction(Core::icon("quit"), QObject::tr("&Quit"), Core::instance()->mainWindow(), SLOT(close()));

  Q_UNUSED(mnuManage)

  //Tools
  //QActionmnuTools->menu()->addSeparator();
  QAction* mnuToolbars     = mnuTools->menu()->addAction(Core::icon("configure-toolbars"), QObject::tr("Configure &Toolbars"), core, SLOT(showToolbarsConfiguration()));
  QAction* mnuSettings  = mnuTools->menu()->addAction(Core::icon("configure"), QObject::tr("%1 &Settings").arg(Core::APP_NAME), core, SLOT(showSettings()));

  //View
  Q_UNUSED(mnuView)
  ActionContainer* mnuDocks = createMenu(STD_MENUS::MENU_VIEW, STD_MENUS::MENU_VIEW_DOCKS, QObject::tr("&Docks"));
  mnuDocks->menu()->setIcon(Core::icon("view-docks"));


  //Help
  QAction* mnuHelpContents = mnuHelp->menu()->addAction(Core::icon("help-contents"), QObject::tr("&Help"), core, SLOT(showHelpContents()));
  mnuHelp->menu()->addSeparator();
  QAction* mnuAboutPlugins = mnuHelp->menu()->addAction(Core::icon("plugin"), QObject::tr("About &Plugins"), core, SLOT(showAboutPlugins()));
  QAction* mnuAboutSCM     = mnuHelp->menu()->addAction(Core::icon("about"), QObject::tr("&About %1").arg(Core::APP_NAME), core, SLOT(showAbout()));

  registerAction(mnuOpen_File,    "File.Open",         QKeySequence::Open);
  registerAction(mnuClose,        "File.Close");
  registerAction(mnuPicManager,   "File.PictureManager");
  registerAction(mnuSave,         "File.Save",         QKeySequence::Save);
  registerAction(mnuSaveAs,       "File.SaveAs",       QKeySequence::SaveAs);
  registerAction(mnuQuit,         "File.Quit",         QKeySequence::Quit);
  registerAction(mnuToolbars,     "Tools.ConfigureToolbars");
  registerAction(mnuSettings,  "Tools.Settings");
  //        registerAction(mnuCalculator,   "View.Docks.Calculator",  QKeySequence("F2"));
  registerAction(mnuHelpContents, "Help.HelpContents", QKeySequence::HelpContents);
  registerAction(mnuAboutPlugins, "Help.AboutPlugins");
  registerAction(mnuAboutSCM,     "Help.About");
}

ActionManager::~ActionManager()
{
  foreach (ActionContainer* container, m_containers)
  {
    delete container;
  }

  foreach (Command* command, m_commands)
  {
    delete command;
  }
}

ActionContainer* ActionManager::actionContainer(const QString& p_id) const
{
  return m_containers.value(p_id, nullptr);
}

Command* ActionManager::command(const QString& p_id) const
{
  return m_commands.value(p_id, nullptr);
}

ActionContainer* ActionManager::createMenu(const QString& p_parentId, const QString& p_id, const QString& p_title, const QIcon& p_icon)
{
  //Check if the ActionContainer already exists
  if (actionContainer(p_id))
    return nullptr;

  //Check if the parent exists
  ActionContainer* parent = actionContainer(p_parentId);
  if ( ! parent)
    return nullptr;

  QMenu* newMenu = nullptr;

  if (parent->menu())
  {
    newMenu = parent->menu()->addMenu(p_icon, p_title);
  }
  else if (parent->menuBar())
  {
    newMenu = parent->menuBar()->addMenu(p_icon, p_title);
  }
  else return nullptr;

  //Add the new ActionContainer
  newMenu->setObjectName(p_id);
  ActionContainer* newContainer = new ActionContainer(p_id, newMenu);
  m_containers[p_id] = newContainer;

  return newContainer;
}

ActionContainer* ActionManager::insertMenu(const QString& p_parentId,
                                           const QString& _beforeId,
                                           const QString& p_id,
                                           const QString& p_title,
                                           const QIcon& p_icon)
{
  //Check if the ActionContainer already exists
  if (actionContainer(p_id))
    return nullptr;

  //Check if the parent exists
  ActionContainer* parent = actionContainer(p_parentId);
  if ( ! parent)
    return nullptr;

  QAction* before;

  //Check if the before exists (cannot be a menubar...)
  if (m_containers.contains(_beforeId) && m_containers[_beforeId]->menu())
  {
    before = m_containers[_beforeId]->menu()->menuAction();
  }
  else if (m_commands.contains(_beforeId))
  {
    before = m_commands[_beforeId]->action();
  }
  else return nullptr;

  QMenu* newMenu = nullptr;

  if (parent->menu())
  {
    newMenu = new QMenu(p_title, parent->menu());
    newMenu->setIcon(p_icon);
    parent->menu()->insertMenu(before, newMenu);
  }
  else if (parent->menuBar())
  {
    newMenu = new QMenu(p_title, parent->menuBar());
    newMenu->setIcon(p_icon);
    parent->menuBar()->insertMenu(before, newMenu);
  }
  else return nullptr;

  //Add the new ActionContainer
  newMenu->setObjectName(p_id);
  ActionContainer* newContainer = new ActionContainer(p_id, newMenu);
  m_containers[p_id] = newContainer;

  return newContainer;
}


ActionContainer* ActionManager::registerMenu(QMenu* p_menu, const QString& p_id)
{
  //Check if an action container already exists
  if (actionContainer(p_id))
    return nullptr;

  p_menu->setObjectName(p_id);
  ActionContainer* container = new ActionContainer(p_id, p_menu);
  m_containers[p_id] = container;

  return container;
}

Command* ActionManager::registerAction(QAction* p_action, const QString& p_id, const QKeySequence& p_defaultShortcut)
{
  //Check if the Command already exists
  if (command(p_id))
    return nullptr;

  //Add the new Command
  p_action->setObjectName(p_id);
  Command* newCommand = new Command(p_id, p_action, p_defaultShortcut);
  m_commands[p_id] = newCommand;

  //Check if a shortcut already exists for this command
  QSettings settings;

  if (settings.contains("ActionManager/" + p_id))
  {
    newCommand->action()->setShortcut(settings.value("ActionManager/" + p_id).value<QKeySequence>());
  }
  else if ( !p_defaultShortcut.isEmpty())
  {
    settings.setValue("ActionManager/" + p_id, p_defaultShortcut);
    newCommand->action()->setShortcut(p_defaultShortcut);
  }

  //Find the category
  QString catName = categoryForCommandId(p_id);

  if ( !catName.isEmpty())
  {
    catName = QObject::tr("Other");
  }
  else //Try to find an actioncontainer with the same id to get a translated version
  {
    ActionContainer* cat = actionContainer(catName);

    if (cat && cat->menu())
      catName = cat->menu()->title();
  }

  //Add the action to the Toolbar Manager
  toolBarManager()->addAction(newCommand->action(), catName);

  return newCommand;
}

bool ActionManager::setShortcut(const QString& p_id, const QKeySequence& p_shortcut)
{
  Command* cmd = command(p_id);

  if (cmd)
  {
    QSettings settings;

    cmd->action()->setShortcut(p_shortcut);

    settings.setValue("ActionManager/" + p_id, p_shortcut);

    return true;
  }
  else return false;
}

QKeySequence ActionManager::shortcut(const QString& p_id) const
{
  Command* cmd = command(p_id);

  if (cmd)
  {
    return cmd->action()->shortcut();
  }

  else return QKeySequence();
}

QString ActionManager::categoryForCommandId(const QString& p_id)
{
  if (p_id.indexOf('.') != -1)
  {
    return p_id.section('.', 0, 0);
  }
  else
  {
    return "";
  }
}

} //Namespace
