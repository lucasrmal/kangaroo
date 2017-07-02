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

#include "accounttreewidgetplugin.h"

#include <KangarooLib/ui/core.h>

using namespace KLib;

AccountTreeWidgetPlugin::AccountTreeWidgetPlugin()
{
}

bool AccountTreeWidgetPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    return true;
}

void AccountTreeWidgetPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void AccountTreeWidgetPlugin::onLoad()
{
}

void AccountTreeWidgetPlugin::onUnload()
{
}

QString AccountTreeWidgetPlugin::name() const
{
    return "AccountTreeWidget";
}

QString AccountTreeWidgetPlugin::version() const
{
    return "1.0";
}

QString AccountTreeWidgetPlugin::description() const
{
    return tr("Provides a widget to view the account hierarchy.");
}

QString AccountTreeWidgetPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString AccountTreeWidgetPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString AccountTreeWidgetPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList AccountTreeWidgetPlugin::requiredPlugins() const
{
    return QStringList();
}


