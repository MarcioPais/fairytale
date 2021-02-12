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
#ifndef DIRECTORY_HPP
#define DIRECTORY_HPP

#include "archive.hpp"
#include "metadata.hpp"
#include <vector>
#include <unordered_map>

namespace Archive {
  namespace Directory {

    typedef struct Entry {
      ULEB128::int64 parent_id;
      ULEB128::int64 length;
      std::string name;
      Archive::Metadata metadata;
    } Entry;

    class Tree :
      private Archive::Structure,
      public  Archive::Structure::Indexable<std::vector<std::shared_ptr<Archive::Directory::Entry>>>,
      public  Archive::Structure::IWritable,
      public  Archive::Structure::IReadable
    {
    private:
      std::unordered_map<Archive::Internal::MapKey, std::shared_ptr<Archive::Directory::Entry>, Archive::Internal::MapHash> map;
    public:
      std::int64_t Read(IO::IReader& reader, std::int64_t const range);
      bool Write(IO::IWriter& writer);
    };

  }  // namespace Directory
}  // namespace Archive

#endif  // DIRECTORY_HPP