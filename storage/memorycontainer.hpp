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
#ifndef MEMORYCONTAINER_HPP
#define MEMORYCONTAINER_HPP

#include "container.hpp"
#include <unordered_map>

namespace Storage {

  class MemoryContainer final : public Container<Storage::Type::Memory, std::unordered_map> {
  private:
    std::unique_ptr<std::uint8_t[]> buffer;
  public:
    MemoryContainer(std::int64_t size);
    bool Read(Storage::Block& block, Storage::Buffer& buf);
    bool Write(Storage::Block& block, Storage::Buffer& buf);
  };

}  // namespace Storage

#endif  // MEMORYCONTAINER_HPP
