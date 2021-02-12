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
#ifndef FILE_HPP
#define FILE_HPP

#include "archive.hpp"
#include "metadata.hpp"
#include "blocks.hpp"
#include "directory.hpp"
#include <vector>
#include <unordered_map>

namespace Archive {
  namespace File {

    typedef struct Entry {
      ULEB128::int64 directory_id;
      ULEB128::int64 length;
      std::string name;
      Archive::Metadata metadata;
      ULEB128::int64 number_of_blocks;
      std::vector<std::int64_t> block_ids;
    } Entry;

    class List :
      private Archive::Structure,
      public  Archive::Structure::Indexable<std::vector<std::shared_ptr<Archive::File::Entry>>>,
      public  Archive::Structure::IWritable,
      public  Archive::Structure::IReadable
    {
    private:
      std::unordered_map<Archive::Internal::MapKey, std::shared_ptr<Archive::File::Entry>, Archive::Internal::MapHash> map;
      std::shared_ptr<Archive::Directory::Tree> directories;
      std::shared_ptr<Archive::Blocks::Segmentation> segmentation;
    public:
      std::int64_t Read(IO::IReader& reader, std::int64_t const range);
      bool Write(IO::IWriter& writer);
    };

  }  // namespace File
}  // namespace Archive

#endif  // FILE_HPP