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

#include "managepricesplugin.h"
#include "pricestab.h"
#include "../TabInterface/centralwidget.h"

#include <QAction>
#include <QMenu>

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>

using namespace KLib;

ManagePricesPlugin::ManagePricesPlugin() :
    m_tab(NULL)
{
}

bool ManagePricesPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ActionContainer* mnuEdit = ActionManager::instance()->actionContainer(STD_MENUS::MENU_MANAGE);

    mnuPrices = mnuEdit->menu()->addAction(Core::icon("price"), tr("&Prices"), this, SLOT(managePrices()));
    ActionManager::instance()->registerAction(mnuPrices, "Manage.Prices");

    mnuPrices->setEnabled(false);

    return true;
}

void ManagePricesPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void ManagePricesPlugin::onLoad()
{
    mnuPrices->setEnabled(true);
}

void ManagePricesPlugin::onUnload()
{
    mnuPrices->setEnabled(false);

    if (m_tab)
    {
        m_tab->deleteLater();
        m_tab = NULL;
    }
}

void ManagePricesPlugin::managePrices()
{
    if (!m_tab)
    {
        m_tab = new PricesTab();
    }

    CentralWidget::instance()->addRegisteredTab(m_tab, tr("Prices"), "Prices", true);
}

QString ManagePricesPlugin::name() const
{
    return "ManagePrices";
}

QString ManagePricesPlugin::version() const
{
    return "1.0";
}

QString ManagePricesPlugin::description() const
{
    return tr("Provides a tab to view and edit prices.");
}

QString ManagePricesPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString ManagePricesPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString ManagePricesPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList ManagePricesPlugin::requiredPlugins() const
{
    return QStringList("TabInterface");
}


