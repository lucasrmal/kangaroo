#include "stockwits.h"

#include <QJsonDocument>
#include <QJsonObject>

StockWits::StockWits(QObject* parent)
    : KLib::IQuote(parent), network_manager_(new QNetworkAccessManager(this)) {
  connect(network_manager_, &QNetworkAccessManager::finished, this,
          &StockWits::onReply);
}

int StockWits::makeRequest(Type type, const QList<KLib::PricePair>& pairs) {
  if (pairs.isEmpty() || type == KLib::IQuote::Currency) return -1;

  QString strPairs;
  for (const KLib::PricePair& p : pairs) {
    strPairs += QString("%1,").arg(p.first);
  }

  strPairs.resize(strPairs.size() - 1);

  QUrl url(QString("https://ql.stocktwits.com/batch?symbols=%1").arg(strPairs));
  QNetworkReply* reply = network_manager_->get(QNetworkRequest(url));

  int num = rand();
  reply->setObjectName(QString::number(num));
  return num;
}

void StockWits::onReply(QNetworkReply* reply) {
  if (!reply) return;

  int queryId = reply->objectName().toInt();

  if (reply->error() != QNetworkReply::NoError) {
    emit requestError(queryId, reply->errorString());
    reply->deleteLater();
    return;
  }

  QJsonParseError error;
  QJsonDocument json_doc = QJsonDocument::fromJson(reply->readAll(), &error);
  qDebug() << json_doc.toJson();

  if (error.error != QJsonParseError::NoError) {
    emit requestError(queryId, error.errorString());
    reply->deleteLater();
    return;
  }

  if (!json_doc.isObject()) {
    emit requestError(queryId, "Unparseable response received!");
    reply->deleteLater();
    return;
  }

  QJsonObject all_quotes = json_doc.object();

  for (const QString& key : all_quotes.keys()) {
    auto it = all_quotes.find(key);
    if (!it->isObject()) {
      continue;
    }

    QJsonObject ticker = it->toObject();

    if (!ticker.contains("PreviousClose")) {
      continue;
    }

    double val = ticker.value("PreviousClose").toDouble();
    if (val == 0) {
      continue;
    }

    // StockWits only supports USD.
    emit quoteReady(KLib::IQuote::Stock, KLib::PricePair(key, "USD"), val);
  }

  reply->deleteLater();
}
