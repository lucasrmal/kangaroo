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

#include "model/payee-manager.h"

namespace kangaroo::model {

PayeeManager* PayeeManager::instance_ = new PayeeManager();

std::vector<std::string> PayeeManager::CountriesInUse() const {
  std::unordered_set<std::string> countries;
  for (auto it = objects_.iterator(); it != objects_.iterator_end(); ++it) {
    if (it->second->country().empty()) continue;
    countries.insert(it->second->country());
  }
  return std::vector<std::string>(countries.begin(), countries.end());
}

Payee* PayeeManager::FindByName(const std::string& name) const {
  return name_index_.FindOne(name);
}

absl::Status PayeeManager::Merge(const std::vector<int64_t>& from, int64_t to) {
  return absl::UnimplementedError("Not Yet Implemented!");
}

absl::Status PayeeManager::ValidateRemove(const Payee& payee) const {
  // TODO(lucasrmal): Update all accounts with this payee id and set to none.
  return absl::UnimplementedError("Not Yet Implemented!");
}

}  // namespace kangaroo::model
