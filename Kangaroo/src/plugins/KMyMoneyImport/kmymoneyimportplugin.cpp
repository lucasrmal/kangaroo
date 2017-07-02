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

#include "kmymoneyimportplugin.h"
#include "kmymoneyimport.h"

#include <KangarooLib/ui/core.h>

using namespace KLib;

KMyMoneyImportPlugin::KMyMoneyImportPlugin()
{
}

bool KMyMoneyImportPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    ImporterManager::instance()->addImporter(new KMyMoneyImport());

    return true;
}

void KMyMoneyImportPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void KMyMoneyImportPlugin::onLoad()
{
}

void KMyMoneyImportPlugin::onUnload()
{
}

QString KMyMoneyImportPlugin::name() const
{
    return "KMyMoneyImport";
}

QString KMyMoneyImportPlugin::version() const
{
    return "1.0";
}

QString KMyMoneyImportPlugin::description() const
{
    return tr("Provides an Importer for KMyMoney files.");
}

QString KMyMoneyImportPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString KMyMoneyImportPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString KMyMoneyImportPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList KMyMoneyImportPlugin::requiredPlugins() const
{
    return {"ImportExport"};
}


