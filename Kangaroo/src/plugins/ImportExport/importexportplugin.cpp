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

#include "importexportplugin.h"
#include "iimporter.h"
#include "iexporter.h"

#include <KangarooLib/ui/core.h>

using namespace KLib;

ImportPlugin::ImportPlugin()
{
}

bool ImportPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    KLib::ExporterManager::instance();
    KLib::ImporterManager::instance();

    return true;
}

void ImportPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void ImportPlugin::onLoad()
{
}

void ImportPlugin::onUnload()
{
}

QString ImportPlugin::name() const
{
    return "ImportExport";
}

QString ImportPlugin::version() const
{
    return "1.0";
}

QString ImportPlugin::description() const
{
    return tr("Provides interfaces to other plugins that offer import and export to/from various file formats.");
}

QString ImportPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString ImportPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString ImportPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList ImportPlugin::requiredPlugins() const
{
    return {};
}


