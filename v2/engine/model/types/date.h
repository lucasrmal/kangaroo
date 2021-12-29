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

#ifndef MODEL_TYPES_DATE_H
#define MODEL_TYPES_DATE_H

#include "absl/time/civil_time.h"

namespace kangaroo {

using date_t = uint32_t;

namespace date {

absl::CivilDay ToCivilDay(date_t date) {
  return absl::CivilDay((date / 10000) % 10000, (date / 100) % 100, date % 100);
}

// Negative years are *not* supported and will result in overflow!
date_t FromCivilDay(absl::CivilDay day) {
  return day.day() + 100 * day.month() + 10000 * day.year();
}

date_t Normalize(date_t date) { return FromCivilDay(ToCivilDay(date)); }

}  // namespace date

}  // namespace kangaroo

#endif  // MODEL_TYPES_DATE_H
