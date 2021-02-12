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
#ifndef DISKCONTAINER_HPP
#define DISKCONTAINER_HPP

#include "container.hpp"
#include "memorycontainer.hpp"
#include <map>

namespace Storage {

  class DiskContainer final : public Container<Storage::Type::Disk, std::map> {
  private:
    std::FILE* file;
  public:
    DiskContainer(std::int64_t size);
    ~DiskContainer();
    bool Claim(Storage::Arena& arena, Storage::MemoryContainer& memory);
    bool Read(Storage::Block& block, Storage::Buffer& buf);
    bool Write(Storage::Block& block, Storage::Buffer& buf);
  };

}  // namespace Storage

#endif  // DISKCONTAINER_HPP
