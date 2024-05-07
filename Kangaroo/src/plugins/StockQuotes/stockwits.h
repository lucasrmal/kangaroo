#ifndef STOCKWITS_H
#define STOCKWITS_H

#include <KangarooLib/interfaces/iquote.h>

#include "qnetworkreply.h"

class StockWits : public KLib::IQuote {
  Q_OBJECT

 public:
  explicit StockWits(QObject* parent = 0);

  virtual ~StockWits() = default;

  virtual QString name() const { return tr("StockWits"); }
  virtual QString id() const { return "StockWits"; }

  virtual int makeRequest(Type type, const QList<KLib::PricePair>& pairs);

 private slots:
  void onReply(QNetworkReply* reply);

 private:
  QNetworkAccessManager* network_manager_;
};

#endif  // STOCKWITS_H
