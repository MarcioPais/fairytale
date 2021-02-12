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
#ifndef METADATA_HPP
#define METADATA_HPP

#include "archive.hpp"
#include <map>

namespace Archive {
  
  typedef struct Tag {
    static constexpr ULEB128::int64 TERMINATOR = 0;
    ULEB128::int64 id;
    ULEB128::int64 value;
    void* data;
    Tag() = delete;
    explicit Tag(std::int64_t const id) : id(id), value(0), data(nullptr) {};
    Tag(std::int64_t const id, std::int64_t const value) : id(id), value(value), data(nullptr) {};
    Tag(std::int64_t const id, std::int64_t const value, void* data) : id(id), value(value), data(data) {};
  } Tag;
 
  class Metadata : 
    public Archive::Structure::Iterable<std::map<ULEB128::int64, std::shared_ptr<Archive::Tag>>>,
    public Archive::Structure::IWritable,
    public Archive::Structure::IReadable
  {
  public:
    using Tag = Archive::Tag;
    bool Add(std::shared_ptr<Archive::Metadata::Tag> tag);
    bool Remove(ULEB128::int64 const id);
    std::shared_ptr<Archive::Metadata::Tag> Find(ULEB128::int64 const id);
    std::int64_t Read(IO::IReader& reader, std::int64_t const range);
    bool Write(IO::IWriter& writer);
  };

}  // namespace Archive

#endif  // METADATA_HPP