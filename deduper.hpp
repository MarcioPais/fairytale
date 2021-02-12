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
#ifndef DEDUPER_HPP
#define DEDUPER_HPP

#include "common.hpp"
#include "block.hpp"
#include "storage/manager.hpp"
#include <unordered_map>
#include <array>

class Deduper {
private:
  struct ListItem {
    Block* block = nullptr;
    ListItem* next = nullptr;
  };
  std::array<uint8_t, Storage::BLOCK_SIZE * 2> ALIGNAS(64) buffer;
  std::unordered_map<std::uint32_t, ListItem> map;
  bool Match(Block& block0, Block& block1, Storage::Manager& manager);
public:
  Deduper() = default;
  ~Deduper();
  Deduper(const Deduper&) = delete;
  Deduper& operator=(const Deduper&) = delete;
  Deduper(Deduper&&) = delete;
  Deduper& operator=(Deduper&&) = delete;
  void Process(Block& start, Block* end, Storage::Manager& manager);
};

#endif  // DEDUPER_HPP