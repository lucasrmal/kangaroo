/*
 * Kangaroo Lib, the library of Kangaroo PFM
 * Copyright (C) 2015 Lucas Rioux-Maldague <lucasr.mal@gmail.com>
 *
 * This library is free software; you can redistribute it and/or
 * modify it under the terms of the GNU Lesser General Public
 * License as published by the Free Software Foundation; either
 * version 2.1 of the License, or (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
 * Lesser General Public License for more details.
 *
 * You should have received a copy of the GNU Lesser General Public
 * License along with this library; if not, write to the Free Software
 * Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  US
 */

#ifndef IQUOTE_H
#define IQUOTE_H

#include <QString>
#include <QObject>

#include "../model/security.h"
#include "../model/pricemanager.h"

namespace KLib {

class IQuote : public QObject {
 Q_OBJECT
 public:
  enum Type
  {
      Currency,
      Stock
  };

  IQuote(QObject* _parent) : QObject(_parent) {}

  virtual ~IQuote() {}

  virtual QString name() const = 0;
  virtual QString id() const = 0;

  virtual int makeRequest(Type _type, const QList<PricePair>& _pairs) = 0;

 signals:
  void quoteReady(IQuote::Type _type, const PricePair& _pair, double _value,
                  const QHash<SecurityInfo, Amount> _extraInfos = QHash<SecurityInfo, Amount>());
  void requestError(int _id, const QString& _error);
  void requestDone(int _id);
};

}  // namespace KLib

#endif // IQUOTE_H
