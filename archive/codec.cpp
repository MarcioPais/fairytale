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

#include "codec.hpp"

Archive::Codec::Entry::~Entry() {
  switch (id) {
    case Archive::Codec::Names::None: { break; }
    default:;
  }
  // TODO: free memory
}

Archive::Codec::Sequence::iterator Archive::Codec::Sequence::Find(Archive::Codec::Names const id) {
  auto iter = data.begin();
  for (; (iter != data.end()) && ((*iter)->id != id); iter++);
  return iter;
}

bool Archive::Codec::Sequence::Append(std::shared_ptr<Archive::Codec::Entry> entry) {
  if (entry == nullptr)
    return false;
  auto item = Find(entry->id);
  if (item == data.end()) {
    data.emplace_back(std::move(entry));
    return true;
  }
  return false;
}

bool Archive::Codec::Sequence::Remove(Archive::Codec::Names const id) {
  auto item = Find(id);
  if (item != data.end()) {
    data.erase(item);
    return true;
  }
  return false;
}

std::int64_t Archive::Codec::Sequence::Read(IO::IReader& reader, std::int64_t const range) {
  UNUSED(reader);
  UNUSED(range);
  return 0;
}

bool Archive::Codec::Sequence::Write(IO::IWriter& writer) {
  UNUSED(writer);
  return false;
}

std::int64_t Archive::Codec::Definitions::Read(IO::IReader& reader, std::int64_t const range) {
  UNUSED(reader);
  UNUSED(range);
  return 0;
}

bool Archive::Codec::Definitions::Write(IO::IWriter& writer) {
  UNUSED(writer);
  return false;
}
