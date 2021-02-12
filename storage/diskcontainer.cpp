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

#include "diskcontainer.hpp"

namespace Storage {

  DiskContainer::DiskContainer(std::int64_t size) : Container(size) {
    // get temporary physical storage
    file = GetTempFile(size);
  }

  DiskContainer::~DiskContainer() {
    fclose(file);
  }

  bool DiskContainer::Claim(Storage::Arena& arena, Storage::MemoryContainer& memory) {
    // count how many blocks need to be replaced
    std::size_t const n = std::count_if(arena.blocks.begin(), arena.blocks.end(), [](Storage::Block const* b) { return b->type != Storage::Type::Disk; });
    if (((static_cast<std::int64_t>(n) * Storage::BLOCK_SIZEi64) > available_) || (n == 0))
      return n == 0;

    Storage::Buffer buf{};
    std::size_t const length = arena.blocks.size();
    auto iter = free.begin();
    bool ok = true;
    arena.blocks.reserve(length + n);
    for (std::size_t i=0; ok && (i<length); i++) {
      Storage::Block* block = arena.blocks[i];
      if (block->type != Storage::Type::Disk) {
        try {
          ok = memory.Read(*block, buf);     // read the block data
          if (!ok)
            break;
          Storage::Block* b = iter->second;  // get free block
          ok = Write(*b, buf);               // write the data to it
          if (!ok)
            break;
          used[b->offset] = b;               // add it to the used map
          iter = free.erase(iter);           // remove it from the free map
          arena.blocks.emplace_back(b);      // append it to the arena
        }
        catch (...) { ok = false; };
      }
    }
    if (ok) {
      memory.Deallocate(arena);  // can't remove the memory blocks just yet
      for (std::size_t i=0, j=length, k=arena.blocks.size(); (i<length) && (j<k); i++) {
        // replace every memory block with the corresponding disk block
        if (arena.blocks[i]->type != Storage::Type::Disk) {
          assert(arena.blocks[j]->type == Storage::Type::Disk);
          arena.blocks[i] = arena.blocks[j];
          available_ -= Storage::BLOCK_SIZEi64;
          j++;
        }
      }
    }
    else {  // failed to commit all blocks to disk, must undo changes
      for (std::size_t i=length; i<arena.blocks.size(); i++) {
        Storage::Block* b = arena.blocks[i];
        assert(b->type == Storage::Type::Disk);
        free[b->offset] = b;
        used.erase(b->offset);
      }
    }
    // resize and trim
    arena.blocks.erase(arena.blocks.begin() + length, arena.blocks.end());
    arena.blocks.shrink_to_fit();
    return ok;
  }

  bool DiskContainer::Read(Storage::Block& block, Storage::Buffer& buf) {
    assert(block.type == Storage::Type::Disk);
    if (fseeko(file, block.offset, SEEK_SET) != 0)
      return false;
    return std::fread(buf.data(), 1, buf.size(), file) == buf.size();
  }

  bool DiskContainer::Write(Storage::Block& block, Storage::Buffer& buf) {
    assert(block.type == Storage::Type::Disk);
    if (fseeko(file, block.offset, SEEK_SET) != 0)
      return false;
    return std::fwrite(buf.data(), 1, buf.size(), file) == buf.size();
  }

}