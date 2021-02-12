/*
  This file is part of the Fairytale project

  Copyright (C) 2021 Márcio Pais

  This library is free software: you can redistribute it and/or modify
  it under the terms of the GNU Lesser General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#pragma once
#ifndef ULEB128_HPP
#define ULEB128_HPP

#include "../common.hpp"

namespace ULEB128 {  // Unsigned Little-Endian Base 128 variable length integer encoding
  typedef std::int64_t int64;

  static inline ULEB128::int64 Cost(std::int64_t n) {
    assert(n >= 0);
    ULEB128::int64 cost = 1;
    while (n > 127) {
      n>>=7;
      cost++;
    }
    return cost;
  }
}  // namespace ULEB128

#endif  // ULEB128_HPP