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

#ifndef MODEL_PRICES_H
#define MODEL_PRICES_H

#include <string>
#include <unordered_map>
#include <utility>
#include <vector>

#include "absl/container/btree_map.h"
#include "model/types/date.h"

namespace kangaroo::model {

class PricePair {
  absl::btree_map<date_t, double, std::greater<date_t>> rates_;

 public:
  using const_iterator = decltype(rates_)::const_iterator;

  double Latest() const;
  double LatestAt(date_t date) const;
  double Get(date_t date) const;
  std::pair<const_iterator, const_iterator> Between(date_t from,
                                                    date_t to) const;

  void InsertOrUpdate(date_t date, double rate);
  void Remove(date_t date);
};

class PriceManager {
 public:
  static PriceManager* instance() { return instance_; }

  PricePair* GetOrCreate(const std::string& from, const std::string& to);
  const PricePair* Get(const std::string& from, const std::string& to) const;

  void Remove(const std::string& from, const std::string& to);

 private:
  std::unordered_map<std::string, PricePair> pairs_;
  static PriceManager* instance_;
};

}  // namespace kangaroo::model

#endif  // MODEL_PRICES_H
