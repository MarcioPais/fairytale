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
#ifndef HYBRIDSTREAM_HPP
#define HYBRIDSTREAM_HPP

#include "stream.hpp"
#include "../storage/pool.hpp"

namespace Storage {
  class Manager;
}

namespace Streams {

  class HybridStream final : public Stream, public Storage::Holder {
    friend Storage::Manager;
  private:
    std::shared_ptr<Storage::Pool> const pool;
    std::unique_ptr<Storage::Arena> const arena;
    HybridStream(std::int64_t const size, std::shared_ptr<Storage::Pool> pool, Storage::AllocationStrategy const strategy = Storage::AllocationStrategy::None);
    ~HybridStream();
    void Close();
    void Restore();
    bool CommitToDisk();
  public:
    std::uint32_t reference_count;
    Streams::Priority priority;
    bool keep_alive;

    HybridStream(const HybridStream&) = delete;
    HybridStream& operator=(const HybridStream&) = delete;
    HybridStream(HybridStream&&) = delete;
    HybridStream& operator=(HybridStream&&) = delete;
    bool Active();
    bool Seek(std::int64_t const offset);
    std::int64_t Position();
    std::int64_t Size();
    int GetByte();
    bool PutByte(std::uint8_t const b);
    std::size_t Read(void* buffer, std::size_t const count);
    std::size_t Write(void* buffer, std::size_t const count);
  };

} // namespace Streams

#endif  // HYBRIDSTREAM_HPP