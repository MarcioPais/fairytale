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
#ifndef MISC_HPP
#define MISC_HPP

#include "../common.hpp"

namespace Misc {

  inline std::uint32_t PopCount(std::uint32_t x) {
    x -= ((x >> 1) & 0x55555555);
    x = ((x >> 2) & 0x33333333) + (x & 0x33333333);
    x = ((x >> 4) + x) & 0x0f0f0f0f;
    x = ((x >> 8) + x) & 0x00ff00ff;
    x = ((x >> 16) + x) & 0x0000ffff;
    return x;
  }

  inline std::uint32_t IntLog2(std::uint32_t x) {
  #ifdef MSC
    DWORD tmp = 0;
    if (x > 0)
      BitScanReverse(&tmp, x);
    return tmp;
  #elif defined(GCC) || defined(CLANG)
    if (x > 0)
      x = 31 - __builtin_clz(x);
    return x;
  #else
    //copy the leading "1" bit to its left (0x03000000 -> 0x03ffffff)
    x |= (x >> 1);
    x |= (x >> 2);
    x |= (x >> 4);
    x |= (x >> 8);
    x |= (x >>16);
    //how many trailing bits do we have (except the first)?
    return PopCount(x >> 1);
  #endif
  }

}  // namespace Misc

#endif  // MISC_HPP