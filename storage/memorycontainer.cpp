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

#include "memorycontainer.hpp"

namespace Storage {

  MemoryContainer::MemoryContainer(std::int64_t size) : Container(size) {
    // create the memory buffer
    try {
      buffer = std::unique_ptr<std::uint8_t[]>(new std::uint8_t[static_cast<std::size_t>(size)]());
    }
    catch (...) { throw Storage::Exhausted(); }  // exception translation, to ensure consistent behavior
  }

  bool MemoryContainer::Read(Storage::Block& block, Storage::Buffer& buf) {
    assert(block.type == Storage::Type::Memory);
    std::memcpy(buf.data(), &buffer[static_cast<std::size_t>(block.offset)], buf.size());
    return true;
  }

  bool MemoryContainer::Write(Storage::Block& block, Storage::Buffer& buf) {
    assert(block.type == Storage::Type::Memory);
    std::memcpy(&buffer[static_cast<std::size_t>(block.offset)], buf.data(), buf.size());
    return true;
  }

}