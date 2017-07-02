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

#include "hometabplugin.h"
#include "../TabInterface/centralwidget.h"
#include "hometab.h"
#include "ledgertab.h"
#include "infopane.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <QDockWidget>

using namespace KLib;

HomeTabPlugin::HomeTabPlugin() :
    m_infoPane(nullptr)
{
}

bool HomeTabPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    m_homeTab = new HomeTab();
    LedgerTabManager::instance();

    return true;
}

void HomeTabPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void HomeTabPlugin::onLoad()
{
    m_homeTab->onLoad();
    CentralWidget::instance()->addRegisteredTab(m_homeTab, tr("Accounts"), "Accounts", false);
    LedgerTabManager::instance()->onLoad();

    m_infoPane = new InfoPane();
    m_infoPane->show();
}

void HomeTabPlugin::onUnload()
{
    m_homeTab->onUnload();
    LedgerTabManager::instance()->onUnload();

    m_infoPane->deleteLater();
    m_infoPane = nullptr;
}

QString HomeTabPlugin::name() const
{
    return "HomeTab";
}

QString HomeTabPlugin::version() const
{
    return "1.0";
}

QString HomeTabPlugin::description() const
{
    return tr("Provides a Home Tab that shows the account hiearchy and allows ledgers to be modified.");
}

QString HomeTabPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString HomeTabPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString HomeTabPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList HomeTabPlugin::requiredPlugins() const
{
    return QStringList() << "TabInterface" << "AccountTreeWidget" << "AccountCreateEdit";
}


