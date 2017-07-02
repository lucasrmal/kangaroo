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

#include "yahooquotesplugin.h"
#include "yahooquote.h"

#include <KangarooLib/ui/core.h>
#include <KangarooLib/controller/onlinequotes.h>

using namespace KLib;

YahooQuotesPlugin::YahooQuotesPlugin()
{
}

bool YahooQuotesPlugin::initialize(QString& p_errorMessage)
{
    Q_UNUSED(p_errorMessage)

    OnlineQuotes::instance()->registerQuoteSource(new YahooQuote(), true);

    return true;
}

void YahooQuotesPlugin::checkSettings(QSettings& settings) const
{
    Q_UNUSED(settings)
}

void YahooQuotesPlugin::onLoad()
{
}

void YahooQuotesPlugin::onUnload()
{
}

QString YahooQuotesPlugin::name() const
{
    return "YahooQuotes";
}

QString YahooQuotesPlugin::version() const
{
    return "1.0";
}

QString YahooQuotesPlugin::description() const
{
    return tr("");
}

QString YahooQuotesPlugin::author() const
{
    return Core::APP_AUTHOR;
}

QString YahooQuotesPlugin::copyright() const
{
    return Core::APP_COPYRIGHT;
}

QString YahooQuotesPlugin::url() const
{
    return Core::APP_WEBSITE;
}

QStringList YahooQuotesPlugin::requiredPlugins() const
{
    return QStringList();
}


