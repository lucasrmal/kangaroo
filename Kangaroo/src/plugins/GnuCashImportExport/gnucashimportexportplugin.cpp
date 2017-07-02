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

#include "gnucashimportexportplugin.h"
#include "gnucashimport.h"
#include <KangarooLib/ui/core.h>

using namespace KLib;

GnuCashImportExportPlugin::GnuCashImportExportPlugin()
{
}

bool GnuCashImportExportPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ImporterManager::instance()->addImporter(new GnuCashImport());

    return true;
}

void GnuCashImportExportPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void GnuCashImportExportPlugin::onLoad()
{
}

void GnuCashImportExportPlugin::onUnload()
{
}

QString GnuCashImportExportPlugin::name() const
{
    return "GnuCashImportExport";
}

QString GnuCashImportExportPlugin::version() const
{
    return "1.0";
}

QString GnuCashImportExportPlugin::description() const
{
    return tr("Provides an Importer for KMyMoney files.");
}

QString GnuCashImportExportPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString GnuCashImportExportPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString GnuCashImportExportPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList GnuCashImportExportPlugin::requiredPlugins() const
{
    return {"ImportExport"};
}


