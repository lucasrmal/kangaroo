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

#include "updatepricesplugin.h"
#include "formupdateprices.h"

#include <KangarooLib/ui/core.h>

#include <QAction>
#include <QMenu>

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <KangarooLib/ui/actionmanager/command.h>

using namespace KLib;

UpdatePricesPlugin::UpdatePricesPlugin()
{
}

bool UpdatePricesPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ActionContainer* mnuTools = ActionManager::instance()->actionContainer(STD_MENUS::MENU_TOOLS);

    mnuUpdate = new QAction(Core::icon("download"), tr("&Update Prices"), mnuTools->menu());
    QAction* sep = mnuTools->menu()->insertSeparator(ActionManager::instance()->command("Tools.ConfigureToolbars")->action());
    mnuTools->menu()->insertAction(sep,
                                   mnuUpdate);

    connect(mnuUpdate, SIGNAL(triggered()), this, SLOT(updatePrices()));
    ActionManager::instance()->registerAction(mnuUpdate, "Tools.UpdatePrices");

    mnuUpdate->setEnabled(false);

    return true;
}

void UpdatePricesPlugin::updatePrices()
{
    FormUpdatePrices frmPrices;
    frmPrices.exec();
}

void UpdatePricesPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void UpdatePricesPlugin::onLoad()
{
    mnuUpdate->setEnabled(true);
}

void UpdatePricesPlugin::onUnload()
{
    mnuUpdate->setEnabled(false);
}

QString UpdatePricesPlugin::name() const
{
    return "UpdatePrices";
}

QString UpdatePricesPlugin::version() const
{
    return "1.0";
}

QString UpdatePricesPlugin::description() const
{
    return tr("Provides an interface to update all prices in the system.");
}

QString UpdatePricesPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString UpdatePricesPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString UpdatePricesPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList UpdatePricesPlugin::requiredPlugins() const
{
    return QStringList();
}


