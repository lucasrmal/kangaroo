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

#ifndef MODEL_ICON_MANAGER_H
#define MODEL_ICON_MANAGER_H

#include <string>
#include <vector>

#include "absl/status/status.h"
#include "model/object-manager.h"
#include "model/proto/icon.pb.h"

namespace kangaroo::model {

class IconManager : public ObjectManager<Icon> {
 public:
  static IconManager* instance() { return instance_; }

  std::string PathForIcon(int64_t icon_id) const;

 protected:
  absl::Status ValidateRemove(const Icon& icon) const;
  std::vector<ObjectIndexT*> Indexes() override { return {}; }
  std::vector<const ObjectValidatorT*> Validators() const override {
    return {};
  }

 private:
  static IconManager* instance_;
};

}  // namespace kangaroo::model

#endif  // MODEL_ICON_MANAGER_H
