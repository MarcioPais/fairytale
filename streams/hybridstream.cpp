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

#include "hybridstream.hpp"
#include "../storage/manager.hpp"

namespace Streams {

  HybridStream::HybridStream(std::int64_t const size, std::shared_ptr<Storage::Pool> pool, Storage::AllocationStrategy const strategy) :
    pool(pool),
    arena(pool->Allocate(size, strategy)),
    reference_count(0),
    priority(Streams::Priority::Normal),
    keep_alive(false)
  {
    capacity_ = available_ = arena->blocks.size() * Storage::BLOCK_SIZEi64;
  }

  HybridStream::~HybridStream() {
    pool->Deallocate(*arena);
  }

  void HybridStream::Close() {
    pool->Deallocate(*arena);
    available_ = 0;
  }

  void HybridStream::Restore() {
    if (Active())
      return;
    pool->Reallocate(*arena, capacity_, Storage::AllocationStrategy::Hot);
    arena->position = 0;
    available_ = capacity_;
  }

  bool HybridStream::CommitToDisk() {
    return pool->MoveToColdStorage(*arena);
  }

  bool HybridStream::Active() {
    return arena->blocks.size() > 0;
  }

  bool HybridStream::Seek(std::int64_t const offset) {
    return pool->Seek(*arena, offset) == offset;
  }

  std::int64_t HybridStream::Position() {
    return arena->position;
  }

  std::int64_t HybridStream::Size() {
    return capacity_ - available_;
  }

  int HybridStream::GetByte() {
    std::uint8_t byte;
    if (pool->Read(&byte, 1, *arena) != 1)
      return EOF;
    return static_cast<int>(byte);
  }

  bool HybridStream::PutByte(std::uint8_t const b) {
    std::uint8_t byte = b;
    std::size_t written = pool->Write(&byte, 1, *arena);
    available_ = std::min<std::int64_t>(available_, capacity_ - arena->position);
    return written == 1;
  }

  std::size_t HybridStream::Read(void* buffer, std::size_t const count) {
    return pool->Read(buffer, count, *arena);
  }

  std::size_t HybridStream::Write(void* buffer, std::size_t const count) {
    std::size_t written = pool->Write(buffer, count, *arena);
    available_ = std::min<std::int64_t>(available_, capacity_ - arena->position);
    return written;
  }

}