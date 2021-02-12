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
#ifndef BLOCKS_HPP
#define BLOCKS_HPP

#include "archive.hpp"
#include "metadata.hpp"
#include "codec.hpp"
#include "../block.hpp"
#include <vector>
#include <list>

namespace Archive {
  namespace Blocks {

    class ChunkInfo {
    public:
      using BlockInfoEntry = std::pair<ULEB128::int64, void*>;
      ULEB128::int64 size;
      std::uint32_t checksum;
      ULEB128::int64 codec_sequence_id;
      ULEB128::int64 block_count;
      Block::Type block_type;
      Archive::Metadata metadata;
      std::vector<BlockInfoEntry> block_info_entries;

      class List :
        public Archive::Structure::Iterable<std::list<std::shared_ptr<Archive::Blocks::ChunkInfo>>>,
        public Archive::Structure::IWritable,
        public Archive::Structure::IReadable
      {
      private:
        std::shared_ptr<Archive::Codec::Definitions> codecs;
      public:
        std::int64_t Read(IO::IReader& reader, std::int64_t const range);
        bool Write(IO::IWriter& writer);
      };
    };
    
    class BlockNode {
    public:
      ULEB128::int64 number_childs;
      void* info;
      std::vector<ULEB128::int64> child_ids;

      class List :
        public Archive::Structure::Indexable<std::vector<std::shared_ptr<Archive::Blocks::BlockNode>>>,
        public Archive::Structure::IWritable,
        public Archive::Structure::IReadable
      {
      public:
        ULEB128::int64 block_count;
        Block::Type block_type;
        Archive::Metadata metadata;

        std::int64_t Read(IO::IReader& reader, std::int64_t const range);
        bool Write(IO::IWriter& writer);
      };
    };

    class Segmentation :
      public Archive::Structure,
      public Archive::Structure::IWritable,
      public Archive::Structure::IReadable
    {
    private:
      Archive::Blocks::ChunkInfo::List chunks;
      Archive::Blocks::BlockNode::List nodes;
      std::shared_ptr<Archive::Codec::Definitions> codecs;
    public:
      std::int64_t Read(IO::IReader& reader, std::int64_t const range);
      bool Write(IO::IWriter& writer);
    };
  }  // namespace Blocks
}  // namespace Archive

#endif  // BLOCKS_HPP