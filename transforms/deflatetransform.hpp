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
#ifndef DEFLATETRANSFORM_HPP
#define DEFLATETRANSFORM_HPP

#include "../common.hpp"
#include "../structs.hpp"
#include "transform.hpp"
#include "../misc/mtflist.hpp"
#include <zlib.h>
#include <array>

namespace zLib {
  static constexpr std::size_t  POSSIBLE_COMBINATIONS = 81;
  static constexpr std::size_t  BLOCK_SIZE = 0x8000;
  static constexpr std::int64_t BLOCK_SIZEi64 = static_cast<std::int64_t>(zLib::BLOCK_SIZE);
  static inline int ParseHeader(std::uint16_t const header) {
    switch (header) {
      case 0x2815: return 0;
      case 0x2853: return 1;
      case 0x2891: return 2;
      case 0x28cf: return 3;
      case 0x3811: return 4;
      case 0x384f: return 5;
      case 0x388d: return 6;
      case 0x38cb: return 7;
      case 0x480d: return 8;
      case 0x484b: return 9;
      case 0x4889: return 10;
      case 0x48c7: return 11;
      case 0x5809: return 12;
      case 0x5847: return 13;
      case 0x5885: return 14;
      case 0x58c3: return 15;
      case 0x6805: return 16;
      case 0x6843: return 17;
      case 0x6881: return 18;
      case 0x68de: return 19;
      case 0x7801: return 20;
      case 0x785e: return 21;
      case 0x789c: return 22;
      case 0x78da: return 23;
    }
    return -1;
  }
  static inline int InflateInit(z_streamp stream, int const parameters) {
    return (parameters != -1) ? inflateInit(stream) : inflateInit2(stream, -MAX_WBITS);
  }
  static inline void SetupStream(z_streamp stream) {
    stream->zalloc  = Z_NULL;
    stream->zfree   = Z_NULL;
    stream->opaque  = Z_NULL;
    stream->next_in = Z_NULL;
    stream->avail_in = 0;
  }
}  // namespace zLib

class DeflateTransform : public Transform {
private:
  MTFList<zLib::POSSIBLE_COMBINATIONS> MTF;
  std::array<std::uint8_t, zLib::BLOCK_SIZE * 2> input_block;
  std::array<std::uint8_t, zLib::BLOCK_SIZE>     output_block;
  std::array<std::uint8_t, zLib::BLOCK_SIZE * 2> recompressed_block;
  std::array<std::size_t , zLib::POSSIBLE_COMBINATIONS> recompressed_block_positions;
  std::array<std::size_t , zLib::POSSIBLE_COMBINATIONS> differ_counts;
  std::array<std::uint8_t, zLib::POSSIBLE_COMBINATIONS * zLib::MAX_PENALTY_BYTES> penalty_bytes;
  std::array<std::int64_t, zLib::POSSIBLE_COMBINATIONS * zLib::MAX_PENALTY_BYTES> differ_positions;
  std::array<z_stream    , zLib::POSSIBLE_COMBINATIONS> recompressed_streams;
  std::array<std::int64_t, zLib::POSSIBLE_COMBINATIONS> recompressed_streams_sizes;
  struct {
    struct {
      std::array<std::uint8_t, zLib::BLOCK_SIZE> input_block;
      std::array<std::uint8_t, zLib::BLOCK_SIZE> recompressed_block;
      z_stream backup_stream;
      std::int64_t offset;  // stream offset before starting skip mode
      int main_return;
    } saved;
    std::int64_t threshold = zLib::BLOCK_SIZEi64;  // threshold of activation
    std::size_t id;                                // zlib combination currently being tested
    bool active;                                   // true if we're trying only the first available combination
  } skip_mode;
  z_stream main_stream;
  std::int64_t total_in;
  std::size_t trials;
  int main_return;
  bool Validate(Structures::DeflateInfo const& data);
  void ClearBuffers();
  void SetupParameters(Structures::DeflateInfo& data);
  bool AttemptBlockRecompression(std::int64_t const base, std::int64_t const length, std::size_t const id);
public:
  Streams::HybridStream* Attempt(Streams::Stream& input, Storage::Manager& manager, void* info = nullptr);
  bool Apply(Streams::Stream& input, Streams::Stream& output, void* info = nullptr);
  bool Undo(Streams::Stream& input, Streams::Stream& output, void* info = nullptr);
};

#endif  // DEFLATETRANSFORM_HPP