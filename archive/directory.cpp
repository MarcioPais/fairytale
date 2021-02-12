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

#include "directory.hpp"

std::int64_t Archive::Directory::Tree::Read(IO::IReader& reader, std::int64_t const range) {
  UNUSED(reader);
  UNUSED(range);
  return 0;
}

bool Archive::Directory::Tree::Write(IO::IWriter& writer) {
  UNUSED(writer);
  return false;
}
