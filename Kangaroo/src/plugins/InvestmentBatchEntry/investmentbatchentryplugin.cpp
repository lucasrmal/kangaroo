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

#include "investmentbatchentryplugin.h"
#include "batchentrytab.h"
#include "../TabInterface/tabinterface.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/ui/mainwindow.h>
#include <KangarooLib/ui/actionmanager/actionmanager.h>
#include <KangarooLib/ui/actionmanager/actioncontainer.h>
#include <QMenu>
#include <QAction>
#include <QKeySequence>

using namespace KLib;

InvestmentBatchEntryPlugin::InvestmentBatchEntryPlugin()
{
}

bool InvestmentBatchEntryPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    return true;
}

void InvestmentBatchEntryPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void InvestmentBatchEntryPlugin::onLoad()
{
}

void InvestmentBatchEntryPlugin::onUnload()
{
}

QString InvestmentBatchEntryPlugin::name() const
{
    return "InvestmentBatchEntry";
}

QString InvestmentBatchEntryPlugin::version() const
{
    return "1.0";
}

QString InvestmentBatchEntryPlugin::description() const
{
    return tr("");
}

QString InvestmentBatchEntryPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString InvestmentBatchEntryPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString InvestmentBatchEntryPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList InvestmentBatchEntryPlugin::requiredPlugins() const
{
    return {};
}


