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
#ifndef POOL_HPP
#define POOL_HPP

#include "memorycontainer.hpp"
#include "diskcontainer.hpp"

namespace Storage {

  class Pool final : public Storage::Holder {
  private:
    enum class Request { Read, Write };
    MemoryContainer memory;  // heap allocated storage
    DiskContainer disk;  // temporary physical storage
    bool ReadBlock(Storage::Block& block, Storage::Buffer& buf);
    bool WriteBlock(Storage::Block& block, Storage::Buffer& buf);
    std::size_t ProcessRequest(void* buffer, std::size_t count, Storage::Arena& arena, Storage::Pool::Request const request);
  public:
    Pool(std::int64_t const memory_size, std::int64_t const disk_size);
    ~Pool();
    Pool(const Pool&) = delete;
    Pool& operator=(const Pool&) = delete;
    Pool(Pool&&) = delete;
    Pool& operator=(Pool&&) = delete;
    std::unique_ptr<Storage::Arena> Allocate(std::int64_t size, Storage::AllocationStrategy const strategy = Storage::AllocationStrategy::None);
    void Reallocate(Storage::Arena& arena, std::int64_t size, Storage::AllocationStrategy const strategy = Storage::AllocationStrategy::None);
    void Deallocate(Storage::Arena& arena);
    std::size_t Read(void* buffer, std::size_t count, Storage::Arena& arena);
    std::size_t Write(void* buffer, std::size_t count, Storage::Arena& arena);
    std::int64_t Seek(Storage::Arena& arena, std::int64_t const offset);
    bool MoveToColdStorage(Storage::Arena& arena);
  };

}  // namespace Storage

#endif  // POOL_HPP