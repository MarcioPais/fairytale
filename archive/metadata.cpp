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

#include "metadata.hpp"

bool Archive::Metadata::Add(std::shared_ptr<Archive::Metadata::Tag> tag) {
  if (tag == nullptr)
    return false;
  return (tag->id != Archive::Tag::TERMINATOR) ? data.emplace(tag->id, std::move(tag)).second : false;  // emplace will fail if an element with the same id already exists
}

bool Archive::Metadata::Remove(ULEB128::int64 const id) {
  return data.erase(id) > 0;
}

std::shared_ptr<Archive::Metadata::Tag> Archive::Metadata::Find(ULEB128::int64 const id) {
  auto iter = data.find(id);
  return (iter != data.end()) ? iter->second : nullptr;
}

std::int64_t Archive::Metadata::Read(IO::IReader& reader, std::int64_t const range) {
  UNUSED(reader);
  UNUSED(range);
  return 0;
}

bool Archive::Metadata::Write(IO::IWriter& writer) {
  UNUSED(writer);
  return false;
}
