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

#include "model/icon-manager.h"

namespace kangaroo::model {

std::string IconManager::PathForIcon(int64_t icon_id) const {
  const Icon* icon = Get(icon_id);
  return icon != nullptr ? icon->path() : "";
}

absl::Status IconManager::ValidateRemove(const Icon& icon) const {
  return absl::UnimplementedError("Not Yet Implemented!");
}

}  // namespace kangaroo::model
