#include "stockquotesplugin.h"

#include <KangarooLib/controller/onlinequotes.h>
#include <KangarooLib/ui/core.h>

#include "stockwits.h"

using namespace KLib;

StockQuotesPlugin::StockQuotesPlugin() {}

bool StockQuotesPlugin::initialize(QString& p_errorMessage) {
  Q_UNUSED(p_errorMessage)

  OnlineQuotes::instance()->registerQuoteSource(new StockWits(), true);

  return true;
}

void StockQuotesPlugin::checkSettings(QSettings& settings) const {
  Q_UNUSED(settings)
}

void StockQuotesPlugin::onLoad() {}

void StockQuotesPlugin::onUnload() {}

QString StockQuotesPlugin::name() const { return "StockQuotes"; }

QString StockQuotesPlugin::version() const { return "1.0"; }

QString StockQuotesPlugin::description() const { return tr(""); }

QString StockQuotesPlugin::author() const { return Core::APP_AUTHOR; }

QString StockQuotesPlugin::copyright() const { return Core::APP_COPYRIGHT; }

QString StockQuotesPlugin::url() const { return Core::APP_WEBSITE; }

QStringList StockQuotesPlugin::requiredPlugins() const { return QStringList(); }
