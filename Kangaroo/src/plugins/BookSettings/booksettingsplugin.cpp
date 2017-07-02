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

#include "booksettingsplugin.h"
#include "formbooksettings.h"

#include <QAction>
#include <QMenu>

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/command.h>

using namespace KLib;

BookSettingsPlugin::BookSettingsPlugin()
{
}

bool BookSettingsPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ActionContainer* mnuFile = ActionManager::instance()->actionContainer(STD_MENUS::MENU_FILE);
    Command* bef = ActionManager::instance()->command("File.PictureManager");

    mnuBookSettings = new QAction(Core::icon("modify"), tr("Boo&k Settings"), mnuFile->menu());
    mnuFile->menu()->insertAction(bef ? bef->action() : nullptr, mnuBookSettings);

    connect(mnuBookSettings, &QAction::triggered, this, &BookSettingsPlugin::editBookSettings);

    mnuBookSettings->setEnabled(false);

    return true;
}

void BookSettingsPlugin::editBookSettings()
{
    FormBookSettings* form = new FormBookSettings(Core::instance()->mainWindow());
    form->exec();
    delete form;
}

void BookSettingsPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void BookSettingsPlugin::onLoad()
{
    mnuBookSettings->setEnabled(true);
}

void BookSettingsPlugin::onUnload()
{
    mnuBookSettings->setEnabled(false);
}

QString BookSettingsPlugin::name() const
{
    return "BookSettings";
}

QString BookSettingsPlugin::version() const
{
    return "1.0";
}

QString BookSettingsPlugin::description() const
{
    return tr("");
}

QString BookSettingsPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString BookSettingsPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString BookSettingsPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList BookSettingsPlugin::requiredPlugins() const
{
    return QStringList();
}


