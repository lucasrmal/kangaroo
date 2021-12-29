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

#include "model/price-manager.h"

#include "absl/strings/str_cat.h"

namespace kangaroo::model {
namespace {
std::string MakeKey(const std::string& from, const std::string& to) {
  return absl::StrCat(from, ":", to);
}
}  // namespace

PriceManager* PriceManager::instance_ = new PriceManager();

PricePair* PriceManager::GetOrCreate(const std::string& from,
                                     const std::string& to) {
  if (from == to || from.empty() || to.empty()) {
    return nullptr;
  }
  return &(pairs_[MakeKey(from, to)]);
}
const PricePair* PriceManager::Get(const std::string& from,
                                   const std::string& to) const {
  auto it = pairs_.find(MakeKey(from, to));
  return it == pairs_.end() ? nullptr : &(it->second);
}

void PriceManager::Remove(const std::string& from, const std::string& to) {
  pairs_.erase(MakeKey(from, to));
}

double PriceManager::LatestPrice(const std::string& from,
                                 const std::string& to) const {
  if (const PricePair* pair = Get(from, to); pair != nullptr) {
    return pair->Latest();
  } else if (const PricePair* reverse_pair = Get(to, from);
             reverse_pair != nullptr) {
    return 1. / reverse_pair->Latest();
  } else {
    return 0;
  }
}

// PricePair Implementation

double PricePair::Latest() const {
  auto it = rates_.begin();
  return it == rates_.end() ? 0 : it->second;
}

double PricePair::LatestAt(date_t date) const {
  auto it = rates_.lower_bound(date::Normalize(date));
  return it == rates_.end() ? 0 : it->second;
}

double PricePair::Get(date_t date) const {
  auto it = rates_.find(date::Normalize(date));
  return it == rates_.end() ? 0 : it->second;
}

std::pair<PricePair::const_iterator, PricePair::const_iterator>
PricePair::Between(date_t from, date_t to) const {
  date_t from_n = date::Normalize(from), to_n = date::Normalize(to);
  if (from_n > to_n) {
    return {rates_.end(), rates_.end()};
  }
  return {rates_.lower_bound(date::Normalize(to)),
          rates_.upper_bound(date::Normalize(from))};
}

void PricePair::InsertOrUpdate(date_t date, double rate) {
  rates_.insert_or_assign(date::Normalize(date), rate);
}
void PricePair::Remove(date_t date) { rates_.erase(date::Normalize(date)); }

}  // namespace kangaroo::model
