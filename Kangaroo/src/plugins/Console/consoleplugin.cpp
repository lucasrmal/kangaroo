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

#include "consoleplugin.h"
#include "formconsole.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>

#include <QAction>
#include <QMenu>

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/command.h>

using namespace KLib;

ConsolePlugin::ConsolePlugin()
{
}

bool ConsolePlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ActionContainer* mnuTools = ActionManager::instance()->actionContainer(STD_MENUS::MENU_TOOLS);

    mnuConsole = new QAction(Core::icon("console"), tr("&Script Console"), mnuTools->menu());
    mnuTools->menu()->insertAction(mnuTools->menu()->actions().first(),
                                   mnuConsole);

    connect(mnuConsole, SIGNAL(triggered()), this, SLOT(openConsole()));
    ActionManager::instance()->registerAction(mnuConsole, "Tools.Console");

    mnuConsole->setEnabled(false);

    return true;
}

void ConsolePlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void ConsolePlugin::openConsole()
{
    FormConsole* frm = new FormConsole(Core::instance()->mainWindow());
    frm->exec();
    delete frm;
}

void ConsolePlugin::onLoad()
{
    mnuConsole->setEnabled(true);
}

void ConsolePlugin::onUnload()
{
    mnuConsole->setEnabled(false);
}

QString ConsolePlugin::name() const
{
    return "Console";
}

QString ConsolePlugin::version() const
{
    return "1.0";
}

QString ConsolePlugin::description() const
{
    return tr("");
}

QString ConsolePlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString ConsolePlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString ConsolePlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList ConsolePlugin::requiredPlugins() const
{
    return QStringList();
}


