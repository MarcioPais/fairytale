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

#include "deduper.hpp"

bool Deduper::Match(Block& block0, Block& block1, Storage::Manager& manager) {
  // start by checking full hashes and lengths
  if ((&block0 == &block1) || (block0.type != block1.type) || (block0.hash != block1.hash) || (block0.length != block1.length))
    return false;
  struct {
    bool dormant;
    Streams::HybridStream* hstream;
    Streams::FileStream* fstream;
  } blocks[2] {
    {false, reinterpret_cast<Streams::HybridStream*>(block0.data), reinterpret_cast<Streams::FileStream*>(block0.data)},
    {false, reinterpret_cast<Streams::HybridStream*>(block1.data), reinterpret_cast<Streams::FileStream*>(block1.data)}
  };
  // attempt to revive streams if needed
  if (block0.level > 0) {
    if ((block0.data == nullptr) || (!blocks[0].hstream->Active() && !block0.Revive(manager)))
      return false;
    blocks[0].hstream->keep_alive = true;
  }
  else {
    blocks[0].dormant = blocks[0].fstream->Dormant();
    if (blocks[0].dormant && !blocks[0].fstream->WakeUp())
      return false;
  }

  if (block1.level > 0) {
    if ((block1.data == nullptr) || (!blocks[1].hstream->Active() && !block1.Revive(manager))) {
      if (block0.level > 0)
        blocks[0].hstream->keep_alive = false;
      return false;
    }
    blocks[1].hstream->keep_alive = true;
  }
  else {
    blocks[1].dormant = blocks[1].fstream->Dormant();
    if (blocks[1].dormant && !blocks[1].fstream->WakeUp()) {
      if (block0.level > 0)
        blocks[0].hstream->keep_alive = false;
      else if (blocks[0].dormant)
        blocks[0].fstream->Sleep();
      return false;
    }
  }
  // now proceed to compare them
  bool result = true;
  std::int64_t length = block0.length;
  std::int64_t offsets[2] { block0.offset, block1.offset };  // need to keep track of the offsets, because the blocks may share the same stream
  try {
    while ((length > 0) && result) {
      std::size_t const size = static_cast<std::size_t>(std::min<std::int64_t>(Storage::BLOCK_SIZEi64, length));
      block0.data->Seek(offsets[0]);
      std::size_t const bytes_read = block0.data->Read(&buffer[0], size);
      block1.data->Seek(offsets[1]);
      result = (bytes_read > 0) && (bytes_read == block1.data->Read(&buffer[Storage::BLOCK_SIZE], size));
      if (result) {
        result = (std::memcmp(&buffer[0], &buffer[Storage::BLOCK_SIZE], size) == 0);
        offsets[0] += bytes_read;
        offsets[1] += bytes_read;
        length -= static_cast<std::int64_t>(bytes_read);
      }
    }
  }
  catch (Storage::Exhausted const&) {
    result = false;
  }
  // clean up
  if (block0.level > 0)
    blocks[0].hstream->keep_alive = false;
  else if (blocks[0].dormant)
    blocks[0].fstream->Sleep();

  if (block1.level > 0)
    blocks[1].hstream->keep_alive = false;
  else if (blocks[1].dormant)
    blocks[1].fstream->Sleep();

  return result;
}

void Deduper::Process(Block& start, Block* end, Storage::Manager& manager) {
  if ((start.data == nullptr) || (start.level >= Block::MAX_RECURSION_LEVEL))
    return;
  Block* block = &start;
  while ((block != nullptr) && (block != end)) {
    assert(block->hashed);
    bool found = false;
    // loop through all entries with this hash
    for (auto it = map.find(block->hash); (it != map.end()) && map.key_eq()(it->first, block->hash); ++it) {
      found = (it->second == block);
      // block was already processed?
      if (UNLIKELY(found))
        break;
      else if (Match(*it->second, *block, manager)) {
        // free any previously allocated info for this block
        block->DeleteInfo();
        // now free any childs
        block->DeleteChilds(manager);
        if (block->level > 0) {
          Streams::HybridStream* stream = reinterpret_cast<Streams::HybridStream*>(block->data);
          // free the stream if possible
          if ((block->offset == 0) && (block->length == block->data->Size())) {
            assert(stream->reference_count == 0);
            manager.Delete(stream);
          }
          // otherwise just decrease its reference count
          else
            stream->reference_count -= (stream->reference_count > 0);
        }
        block->type = Block::Type::Dedup;
        // info now points to the block we deduplicated from
        block->info = it->second;
        block->done = true;
        found = true;
        break;
      }
    }
    if (!found)
      map.emplace(
        std::piecewise_construct,
        std::forward_as_tuple(block->hash),
        std::forward_as_tuple(block)
      );
    // recurse if possible
    if (block->child != nullptr)
      Process(*block->child, nullptr, manager);
    block = block->next;
  }
}
