/*
This file is part of Kangaroo.
Copyright (C) 2014 Lucas Rioux-Maldague

Kangaroo is free software: you can redistribute it and/or modify
it under the terms of the GNU Lesser General Public License as published by
the Free Software Foundation, either version 3 of the License, or
(at your option) any later version.

Kangaroo is distributed in the hope that it will be useful,
but WITHOUT ANY WARRANTY; without even the implied warranty of
MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the
GNU Lesser General Public License for more details.

You should have received a copy of the GNU Lesser General Public License
along with Kangaroo. If not, see <http://www.gnu.org/licenses/>.
*/

#include "manageothersplugin.h"
#include "managerwidget.h"
#include "../TabInterface/tabinterface.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <QMenu>
#include <QAction>
#include <QKeySequence>


using namespace KLib;

ManageOthersPlugin::ManageOthersPlugin()
{
}

bool ManageOthersPlugin::initialize(QString& p_errorMessage)
{
  Q_UNUSED(p_errorMessage)

  ActionContainer* mnuEdit = ActionManager::instance()->actionContainer(STD_MENUS::MENU_MANAGE);

  m_actions[ManageType::Currencies] = mnuEdit->menu()->addAction(Core::icon("currency"),
                                                                 tr("&Currencies"),
                                                                 this, SLOT(openManager()));

  m_actions[ManageType::Institutions] = mnuEdit->menu()->addAction(Core::icon("institution"),
                                                                   tr("&Institutions"),
                                                                   this, SLOT(openManager()));

  m_actions[ManageType::Payees] = mnuEdit->menu()->addAction(Core::icon("payee"),
                                                             tr("&Payees"),
                                                             this, SLOT(openManager()));
  mnuEdit->menu()->addSeparator();

  m_actions[ManageType::Securities] = mnuEdit->menu()->addAction(Core::icon("security"),
                                                                 tr("&Securities"),
                                                                 this, SLOT(openManager()));

  m_actions[ManageType::Indexes] = mnuEdit->menu()->addAction(Core::icon("index"),
                                                              tr("Inde&xes"),
                                                              this, SLOT(openManager()));
  mnuEdit->menu()->addSeparator();

  m_actions[ManageType::Prices] = mnuEdit->menu()->addAction(Core::icon("price"),
                                                             tr("&Rates and Prices"),
                                                             this, SLOT(openManager()));


  ActionManager::instance()->registerAction(m_actions[ManageType::Currencies],
      "Manage.Currencies",
      QKeySequence("Ctrl+Shift+C"));

  ActionManager::instance()->registerAction(m_actions[ManageType::Institutions],
      "Manage.Institutions",
      QKeySequence("Ctrl+Shift+I"));

  ActionManager::instance()->registerAction(m_actions[ManageType::Payees],
      "Manage.Payees",
      QKeySequence("Ctrl+Shift+P"));

  ActionManager::instance()->registerAction(m_actions[ManageType::Securities],
      "Manage.Securities",
      QKeySequence("Ctrl+Shift+U"));

  ActionManager::instance()->registerAction(m_actions[ManageType::Indexes],
      "Manage.Indexes");

  ActionManager::instance()->registerAction(m_actions[ManageType::Prices],
      "Manage.Prices");

  for (QAction* m : m_actions)
  {
    m->setEnabled(false);
  }

  return true;
}

void ManageOthersPlugin::checkSettings(QSettings& settings) const
{
  Q_UNUSED(settings)
}

void ManageOthersPlugin::onLoad()
{
  for (QAction* m : m_actions)
  {
    m->setEnabled(true);
  }
}

void ManageOthersPlugin::onUnload()
{
  for (QAction* m : m_actions)
  {
    m->setEnabled(false);
  }

  for (ManagerWidget* w : m_widgets)
  {
    w->deleteLater();
  }

  m_widgets.clear();
}

void ManageOthersPlugin::openManager()
{
  QAction* action = qobject_cast<QAction*>(sender());
  ManageType t;
  bool found = false;

  //Find which action was called
  for (auto i = m_actions.begin(); i != m_actions.end(); ++i)
  {
    if (*i == action)
    {
      t = i.key();
      found = true;
      break;
    }
  }

  if (!found)
    return;

  //Find the id and the title of the tab
  QString id, title, icon;

  switch (t)
  {
  case ManageType::Currencies:
    id = "currency";
    title = tr("Currencies");
    break;

  case ManageType::Institutions:
    id = "institution";
    title = tr("Institutions");
    break;

  case ManageType::Securities:
    id = "security";
    title = tr("Securities");
    break;

  case ManageType::Payees:
    id = "payee";
    title = tr("Payees");
    break;

  case ManageType::Indexes:
    id = "index";
    title = tr("Indexes");
    break;

  case ManageType::Prices:
    id = "price";
    title = tr("Prices");
    break;

  default:
    return;
  }

  //Show or put focus on the tab
  if (!m_widgets.contains(t))
  {
    m_widgets[t] = new ManagerWidget(t);
  }

  if (TabInterface::instance()->hasTab(id))
  {
    TabInterface::instance()->setFocus(id);
  }
  else
  {
    TabInterface::instance()->addRegisteredTab(m_widgets[t], title, id, true, Core::icon(id));
  }
}

QString ManageOthersPlugin::name() const
{
  return "ManageOthers";
}

QString ManageOthersPlugin::version() const
{
  return "1.0";
}

QString ManageOthersPlugin::description() const
{
  return tr("Provides user interfaces to manage payees, institutions, currencies, securities, indexes and prices.");
}

QString ManageOthersPlugin::author() const
{
  return Core::APP_AUTHOR;
}

QString ManageOthersPlugin::copyright() const
{
  return Core::APP_COPYRIGHT;
}

QString ManageOthersPlugin::url() const
{
  return Core::APP_WEBSITE;
}

QStringList ManageOthersPlugin::requiredPlugins() const
{
  return QStringList("TabInterface");
}


