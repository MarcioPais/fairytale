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
#ifndef IMAGE_HPP
#define IMAGE_HPP

#include "../common.hpp"
#include "../streams/stream.hpp"
#include "misc.hpp"

namespace Image {

  inline bool HasGrayscalePalette(Streams::Stream& input, int const num_entries = 256, bool const has_alpha = false) {
    std::int64_t const offset = input.Position();
    int const stride = 3 + static_cast<int>(has_alpha);
    int result = ((num_entries > 0) && (num_entries <=256)) ? 0x100 : 0;
    int order = 1;
    for (int i = 0; (i < num_entries * stride) && (result >= 0x100); i++) {
      int const b = input.GetByte();
      if (b == EOF) {
        result = 0;
        break;
      }
      else if (i == 0) {
        result = 0x100 | b;
        order = 1 - 2 * static_cast<int>(b > static_cast<int>(Misc::IntLog2(num_entries)/4));
        continue;
      }
      //"j" is the index of the current byte in this color entry
      int j = i % stride;
      if (j == 0) {
        // load first component of this entry
        int k = (b - (result & 0xFF)) * order;
        result = ((k >= 0) && (k <= 8)) ? 0x100 | b : 0;
      }
      else if (((j < 3) && (b != (result & 0xFF))) || ((j==3) && (b != 0) && (b != 0xFF) /*alpha/"attribute" component must be zero or 0xFF*/))
        result = 0;
    }
    input.Seek(offset);
    return result >= 0x100;
  }

}  // namespace Image

#endif  // IMAGE_HPP