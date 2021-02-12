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
#ifndef DEFLATEPARSER_HPP
#define DEFLATEPARSER_HPP

#include "parser.hpp"
#include "../structs.hpp"
#include "../transforms/deflatetransform.hpp"
#include <array>

namespace gZip {
  enum Flags {
    NONE = 0,
    CRC = 2,
    EXTRA = 4,
    NAME = 8,
    COMMENT = 16
  };
}

class DeflateParser : public Parser<Parsers::Types::Strict> {
private:
  static constexpr std::size_t  WINDOW_LOOKBACK = 32;
  static constexpr std::int64_t WINDOW_LOOKBACKi64 = static_cast<std::int64_t>(WINDOW_LOOKBACK);
  static constexpr std::size_t  BRUTE_LOOKBACK  = 256;
  static constexpr std::int64_t BRUTE_LOOKBACKi64 = static_cast<std::int64_t>(BRUTE_LOOKBACK);
  static constexpr std::size_t  WINDOW_SIZE = WINDOW_LOOKBACK + BRUTE_LOOKBACK;
  static constexpr std::size_t  WINDOW_ACCESS_MASK = BRUTE_LOOKBACK - 1;
  static constexpr std::int64_t WINDOW_ACCESS_MASKi64 = BRUTE_LOOKBACKi64 - 1;
  static constexpr std::size_t  BRUTE_ROUNDS = BRUTE_LOOKBACK >> 6;
  DeflateTransform transform;
  std::array<std::uint8_t, WINDOW_SIZE> window;
  std::array<std::uint8_t, zLib::BLOCK_SIZE> input_block;
  std::array<std::uint8_t, zLib::BLOCK_SIZE> output_block;
  std::array<bool, BRUTE_LOOKBACK> skip_positions;
  std::array<int, 256> histogram;
  struct {
    bool use_brute_mode;
    bool parse_zip_streams;
    bool parse_gzip_streams;
  } configuration;
  struct {
    std::int64_t offset;
    std::uint8_t options;
  } gzip;
  std::int64_t zip_offset;
  std::int64_t index;
  std::size_t wnd_position;
  std::uint8_t ReadBack(std::size_t const x);
  bool Validate(std::int64_t const in, std::int64_t const out, bool const brute);
  void ClearBuffers();
  void ProcessByte(std::uint8_t const b);
  void PerformBruteModeSearch(bool& result);
  void GetStreamInfo(Block* block, Structures::DeflateInfo& info, bool const brute);
public:
  enum Options {
    UseBruteMode = 1,
    ParseZipStreams = 2,
    ParseGZipStreams = 4
  };
  explicit DeflateParser(const std::shared_ptr<void>& options);
  bool Parse(Block* block, Structures::ParsingData& data, Storage::Manager& manager);
};

#endif  //DEFLATEPARSER_HPP