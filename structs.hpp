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
#ifndef STRUCTS_HPP
#define STRUCTS_HPP

#include "common.hpp"
#include <array>

namespace zLib {
  static constexpr std::size_t MAX_PENALTY_BYTES = 64;
}

namespace Structures {
  typedef struct DeflateInfo {
    struct {
      int parameters;
      std::uint8_t combination;
      std::uint8_t window;
    } zLib;
    std::size_t penalty_bytes_count;
    std::array<std::uint8_t, zLib::MAX_PENALTY_BYTES> penalty_bytes;
    std::array<std::int64_t, zLib::MAX_PENALTY_BYTES> differ_positions;
    std::int64_t compressed_length;
    std::int64_t uncompressed_length;
  } DeflateInfo;

  typedef struct ImageInfo {
    std::int32_t width;
    std::int32_t height;
    std::int32_t stride;
    std::uint8_t bpp;
    bool grayscale;
  } ImageInfo;

  typedef struct AudioInfo {
    std::uint8_t channels;
    std::uint8_t bps;
    std::uint8_t mode;
  } AudioInfo;

  typedef struct ParsingData {
    DeflateInfo deflate;
    ImageInfo image;
    AudioInfo audio;
  } ParsingData;
  static_assert(std::is_trivially_copyable<Structures::ParsingData>::value, "Parsing info structs must be trivially copyable");
}  // namespace Structs

#endif  // STRUCTS_HPP