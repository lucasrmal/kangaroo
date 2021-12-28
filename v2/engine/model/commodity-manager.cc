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

#include "model/commodity-manager.h"

#include "model/institution-manager.h"

namespace kangaroo::model {

CommodityManager* CommodityManager::instance_ = new CommodityManager();

Commodity* CommodityManager::FindBySymbol(const std::string& symbol) const {
  return symbol_index_.FindOne(symbol);
}

absl::Status CommodityManager::ValidateInsert(
    const Commodity& commodity) const {
  if (!commodity.has_decimal_places() || commodity.decimal_places() < 0 ||
      commodity.decimal_places() > CommodityManager::kMaxDecimalPlaces) {
    return absl::InvalidArgumentError(
        "Decimal places must be between 0 and 6.");
  }
  if (commodity.has_currency()) {
    if (commodity.symbol().size() != 3) {
      return absl::InvalidArgumentError("Currency symbol size must be 3.");
    }
  } else if (commodity.has_security()) {
    if (!commodity.security().has_traded_currency_id()) {
      return absl::InvalidArgumentError(
          "Security must have a traded currency.");
    }
    if (const Commodity* curr =
            Get(commodity.security().has_traded_currency_id());
        curr == nullptr || !curr->has_currency()) {
      return absl::InvalidArgumentError(
          "Security traded currency must exist and be a currency.");
    }
    if (commodity.security().has_icon_id()) {
      return absl::UnimplementedError("Must Check This!");
    }
    if (commodity.security().has_provider_institution_id() &&
        !InstitutionManager::instance()->Get(
            commodity.security().has_provider_institution_id())) {
      return absl::InvalidArgumentError("Provider instition does not exist.");
    }

  } else {
    return absl::InvalidArgumentError(
        "Commodity must be one of Currency or Security.");
  }

  return absl::OkStatus();
}

absl::Status CommodityManager::ValidateRemove(
    const Commodity& commodity) const {
  // TODO(lucasrmal): Update all accounts with this payee id and set to none.
  return absl::UnimplementedError("Not Yet Implemented!");
}

}  // namespace kangaroo::model
