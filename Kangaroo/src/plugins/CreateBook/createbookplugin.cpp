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

#include "createbookplugin.h"
#include "createbookwizard.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/command.h>
#include <QMenu>
#include <QAction>

using namespace KLib;

CreateBookPlugin::CreateBookPlugin()
{
}

bool CreateBookPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ActionContainer* mnuFile = ActionManager::instance()->actionContainer(STD_MENUS::MENU_FILE);

    mnuCreateNew = new QAction(Core::icon("new-book"), tr("&New Book"), mnuFile->menu());
    //mnuCreateNew->setShortcut(QKeySequence("Ctrl+N"));

    QAction* sep = mnuFile->menu()->insertSeparator(mnuFile->menu()->actions().first());
    mnuFile->menu()->insertAction(sep, mnuCreateNew);


    connect(mnuCreateNew, SIGNAL(triggered()), this, SLOT(createNewBook()));
    ActionManager::instance()->registerAction(mnuCreateNew, "File.NewBook");

    return true;
}

void CreateBookPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void CreateBookPlugin::onLoad()
{
}

void CreateBookPlugin::onUnload()
{
}

void CreateBookPlugin::createNewBook()
{
    if (!Core::instance()->unloadFile())
        return;

    CreateBookWizard* w = new CreateBookWizard(Core::instance()->mainWindow());
    w->exec();
    delete w;
}

QString CreateBookPlugin::name() const
{
    return "CreateBook";
}

QString CreateBookPlugin::version() const
{
    return "1.0";
}

QString CreateBookPlugin::description() const
{
    return tr("Create new book wizard");
}

QString CreateBookPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString CreateBookPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString CreateBookPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList CreateBookPlugin::requiredPlugins() const
{
    return {};
}


