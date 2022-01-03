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

#ifndef MODEL_COMMODITY_MANAGER_H
#define MODEL_COMMODITY_MANAGER_H

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "model/institution-manager.h"
#include "model/object-manager.h"
#include "model/proto/commodity.pb.h"
#include "model/validators.h"

namespace kangaroo::model {

struct WorldCurrency {
  std::string name;
  std::string code;
  std::string symbol;
  int precision = 2;
};

class CommodityManager : public ObjectManager<Commodity> {
 public:
  explicit CommodityManager(InstitutionManager* institution_manager)
      : institution_manager_(institution_manager) {}

  const Commodity* FindBySymbol(const std::string& symbol) const;

  static const std::vector<WorldCurrency>& WorldCurrencies();

  static constexpr int kMaxDecimalPlaces = 6;

 protected:
  absl::Status ValidateInsert(const Commodity& commodity) const override;
  absl::Status ValidateUpdate(const Commodity& before,
                              const Commodity& after) const override;
  absl::Status ValidateRemove(const Commodity& commodity) const override;

  std::vector<ObjectIndexT*> Indexes() override { return {&symbol_index_}; }
  std::vector<const ObjectValidatorT*> Validators() const override {
    return {&required_name_validator_, &unique_symbol_validator_};
  }

 private:
  InstitutionManager* institution_manager_;

  ObjectIndex<Commodity, &Commodity::symbol> symbol_index_;
  RequiredValidator<Commodity, &Commodity::name> required_name_validator_ = {
      "name"};
  UniqueValidator<Commodity, &Commodity::symbol> unique_symbol_validator_ = {
      "symbol", &symbol_index_};
};

}  // namespace kangaroo::model

#endif  // MODEL_COMMODITY_MANAGER_H
