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
#ifndef BLOCK_HPP
#define BLOCK_HPP

#include "common.hpp"
#include "streams/stream.hpp"
#include "storage/manager.hpp"

class Block {
public:
  static constexpr std::uint32_t MAX_RECURSION_LEVEL = 4;
  enum class Type {
    Default,
    Dedup,
    Deflate,
    JPEG,
    Image,
    Audio,
    Count
  };
  struct Segmentation {
    std::int64_t offset;
    std::int64_t length;
    Block::Type type;
    void* info = nullptr;
    std::size_t size_of_info;
    struct {
      Streams::Stream* stream;
      Block::Type type;
      void* info;
      std::size_t size_of_info;
      bool done;
    } child;
  };
  Block::Type type;
  Block *parent, *next, *child;
  Streams::Stream* data;
  void* info;
  std::int64_t id, offset, length;
  std::uint32_t level, reference_count, hash;
  bool hashed, done;

  Block() = default;
  Block(const Block&) = delete;
  Block& operator=(const Block&) = delete;
  Block(Block&&) = delete;
  Block& operator=(Block&&) = delete;

  bool Revive(Storage::Manager& manager);
  Block* Segment(Block::Segmentation& segmentation);
  Block* Next(std::uint32_t const lvl, bool const skip_done = true);
  void DeleteInfo();
  void DeleteChilds(Storage::Manager& manager);
  void Hash();
};

#endif  // BLOCK_HPP