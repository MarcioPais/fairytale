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
#ifndef CONTAINER_HPP
#define CONTAINER_HPP

#include "storage.hpp"

namespace Storage {

  template<Storage::Type type, template <typename...> class Map>
  class Container: public Storage::Holder {
  protected:
    std::vector<Storage::Block> blocks;
    Map<std::int64_t, Storage::Block*> free;
    Map<std::int64_t, Storage::Block*> used;
  public:
    Container(std::int64_t size) {
      size = RoundToBlockMultiple(size);
      if (size < Storage::BLOCK_SIZEi64)
        throw Storage::Exhausted();
      std::size_t const n = static_cast<std::size_t>(size / Storage::BLOCK_SIZEi64);
      blocks.resize(n);
      capacity_ = 0;
      for (std::size_t i=0; i<n; i++, capacity_ += Storage::BLOCK_SIZEi64) {
        blocks[i].type = type;
        blocks[i].offset = capacity_;
        free[capacity_] = &blocks[i];
      }
      available_ = capacity_;
    }
    virtual ~Container() = default;
    Container(const Container&) = delete;
    Container& operator=(const Container&) = delete;
    Container(Container&&) = delete;
    Container& operator=(Container&&) = delete;
    void Allocate(std::int64_t const size, Storage::Arena& arena) {
      assert((size & Storage::BLOCK_MASKi64) == 0);
      if (size > available_)
        throw Exhausted();
      std::size_t const n = static_cast<std::size_t>(size / Storage::BLOCK_SIZEi64);
      assert(n <= free.size());
      arena.blocks.reserve(arena.blocks.size() + n);
      auto iter = free.begin();
      for (std::size_t i=0; i<n; i++) {
        Storage::Block* b = iter->second;  // get free block
        used[b->offset] = b;               // add it to the used map
        iter = free.erase(iter);           // remove it from the free map
        arena.blocks.emplace_back(b);      // append it to the arena
      }
      available_ -= size;
    }
    void Deallocate(Storage::Arena& arena, bool const erase = false) {
      for (auto b : arena.blocks) {
        if (b->type == type) {
          free[b->offset] = b;                   // add it to the free map
          used.erase(b->offset);                 // remove it from the used map
          available_ += Storage::BLOCK_SIZEi64;  // reclaim available storage
        }
      }
      if (erase)
        arena.blocks.erase(std::remove_if(arena.blocks.begin(), arena.blocks.end(), [](Block const* b) { return b->type == type; }), arena.blocks.end());
    }
    virtual bool Read(Storage::Block& block, Storage::Buffer& buf) = 0;
    virtual bool Write(Storage::Block& block, Storage::Buffer& buf) = 0;
  };

}  // namespace Storage

#endif  // CONTAINER_HPP