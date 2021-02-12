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

#include "pool.hpp"

namespace Storage {

  bool Pool::ReadBlock(Storage::Block& block, Storage::Buffer& buf) {
    if (block.type == Storage::Type::Disk)
      return disk.Read(block, buf);
    else
      return memory.Read(block, buf);
  }

  bool Pool::WriteBlock(Storage::Block& block, Storage::Buffer& buf) {
    if (block.type == Storage::Type::Disk)
      return disk.Write(block, buf);
    else
      return memory.Write(block, buf);
  }

  std::size_t Pool::ProcessRequest(void* buffer, std::size_t count, Storage::Arena& arena, Storage::Pool::Request const request) {
    std::int64_t total = static_cast<std::int64_t>(arena.blocks.size()) * Storage::BLOCK_SIZEi64;
    assert(arena.position <= total);
    if (arena.position + static_cast<std::int64_t>(count) > total)
      count = static_cast<std::size_t>(total - arena.position);
    if (count == 0)
      return 0;
    Storage::Buffer buf{};
    std::size_t offset = static_cast<std::size_t>(arena.position / Storage::BLOCK_SIZEi64);
    std::size_t index = static_cast<std::size_t>(arena.position & Storage::BLOCK_MASKi64);
    Block* b = arena.blocks[offset];
    if (!ReadBlock(*b, buf))
      return 0;
    std::size_t n = std::min<std::size_t>(buf.size() - index, count);
    if (request == Storage::Pool::Request::Read)
      std::memcpy(buffer, buf.data() + index, n);
    else {
      std::memcpy(buf.data() + index, buffer, n);
      if (!WriteBlock(*b, buf))
        return 0;
    }
    arena.position += n;
    offset++;
    if (request == Storage::Pool::Request::Read) {
      for (; n + buf.size() <= count; offset++, n += buf.size(), arena.position += buf.size()) {
        if (!ReadBlock(*arena.blocks[offset], buf))
          return n;
        std::memcpy(static_cast<std::uint8_t*>(buffer) + n, buf.data(), buf.size());
      }
    }
    else {
      for (; n + buf.size() <= count; offset++, n += buf.size(), arena.position += buf.size()) {
        std::memcpy(buf.data(), static_cast<std::uint8_t*>(buffer) + n, buf.size());
        if (!WriteBlock(*arena.blocks[offset], buf))
          return n;
      }
    }
    if (n < count) {
      if (request == Storage::Pool::Request::Read) {
        if (!ReadBlock(*arena.blocks[offset], buf))
          return n;
        std::memcpy(static_cast<std::uint8_t*>(buffer) + n, buf.data(), count - n);
      }
      else {
        std::memcpy(buf.data(), static_cast<std::uint8_t*>(buffer) + n, count - n);
        if (!WriteBlock(*arena.blocks[offset], buf))
          return n;
      }
      arena.position += count - n;
    }
    return count;
  }

  Pool::Pool(std::int64_t const memory_size, std::int64_t const disk_size) : memory(memory_size), disk(disk_size) {
    capacity_ = available_ = disk.capacity() + memory.capacity();
  }

  Pool::~Pool() {
  }

  std::unique_ptr<Storage::Arena> Pool::Allocate(std::int64_t size, Storage::AllocationStrategy const strategy) {
    std::unique_ptr<Storage::Arena> arena(new Arena());
    Reallocate(*arena, size, strategy);
    return arena;
  }

  void Pool::Reallocate(Storage::Arena& arena, std::int64_t size, Storage::AllocationStrategy const strategy) {
    size = RoundToBlockMultiple(size);
    if ((size > available_) || (size < Storage::BLOCK_SIZEi64))
      throw Storage::Exhausted();

    Storage::Type primary = Storage::Type::Memory;
    std::int64_t alloc = 0LL;
    if (strategy == Storage::AllocationStrategy::None) {
      alloc = std::min<std::int64_t>((primary == Storage::Type::Memory) ? memory.available() : disk.available(), size); // TODO: allocation strategy
    }
    else {
      if (strategy == Storage::AllocationStrategy::Cold)
        primary = Storage::Type::Disk;
      alloc = std::min<std::int64_t>((primary == Storage::Type::Memory) ? memory.available() : disk.available(), size);
    }
    if (primary == Storage::Type::Memory)
      memory.Allocate(alloc, arena);
    else
      disk.Allocate(alloc, arena);
    if (alloc < size) {
      if (primary == Storage::Type::Memory)
        disk.Allocate(size - alloc, arena);
      else
        memory.Allocate(size - alloc, arena);
    }
    available_ = disk.available() + memory.available();
  }

  void Pool::Deallocate(Storage::Arena& arena) {
    memory.Deallocate(arena);
    disk.Deallocate(arena);
    available_ = disk.available() + memory.available();
    arena.blocks.clear();
    arena.blocks.shrink_to_fit();
    arena.position = 0;
  }

  std::size_t Pool::Read(void* buffer, std::size_t count, Storage::Arena& arena) {
    return ProcessRequest(buffer, count, arena, Storage::Pool::Request::Read);
  }

  std::size_t Pool::Write(void* buffer, std::size_t count, Storage::Arena& arena) {
    return ProcessRequest(buffer, count, arena, Storage::Pool::Request::Write);
  }

  std::int64_t Pool::Seek(Storage::Arena& arena, std::int64_t const offset) {
    return arena.position = std::max<std::int64_t>(0LL, std::min<std::int64_t>(static_cast<std::int64_t>(arena.blocks.size()) * Storage::BLOCK_SIZEi64, offset));
  }

  bool Pool::MoveToColdStorage(Storage::Arena& arena) {
    return disk.Claim(arena, memory);
  }

}