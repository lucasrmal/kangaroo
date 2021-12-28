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

#ifndef MODEL_CONSTANTS_H
#define MODEL_CONSTANTS_H

#include <string>

namespace kangaroo
{

    struct Constants
    {

        static const int kNoId;

        static const std::string_view DEFAULT_CURRENCY_CODE;
        static const std::string_view DEFAULT_CURRENCY_NAME;

        static const std::string_view DEFAULT_DATE_FORMAT;

    };

}

#endif // MODEL_CONSTANTS_H
