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

#include "block.hpp"
#include "hashes/CRC32.hpp"
#include "transforms/transformfactory.hpp"

bool Block::Revive(Storage::Manager& manager) {
  assert((data != nullptr) && (level > 0) && (parent != nullptr));
  Streams::HybridStream* stream = reinterpret_cast<Streams::HybridStream*>(data);
  if (stream->Active())
    return true;

  bool parent_was_dormant = false;
  Streams::HybridStream* parent_hstream = reinterpret_cast<Streams::HybridStream*>(parent->data);
  Streams::FileStream*   parent_fstream = reinterpret_cast<Streams::FileStream*>(parent->data);
  // recreate parent stream, if needed
  if (level > 1) {
    if ((parent->data == nullptr) || (!parent_hstream->Active() && !parent->Revive(manager)))
      return false;
    // don't let it be purged
    parent_hstream->keep_alive = true;
  }
  else {
    parent_was_dormant = parent_fstream->Dormant();
    if (parent_was_dormant && !parent_fstream->WakeUp())
      return false;
  }

  bool result = false;
  manager.Reallocate(*stream);
  // when parsing, we had enough storage for the parent stream and this stream.
  // however, when deduping, we might have to restore 2 blocks at once, so this may have failed
  if (stream->Active()) {
    std::unique_ptr<Transform> transform = TransformFactory::Create(parent->type);
    if (transform) {
      parent->data->Seek(parent->offset);
      result = transform->Apply(*parent->data, *stream, parent->info);
      if (!result)
        // something went terribly wrong, panic
        throw std::logic_error("Failed to recover transformed stream");
      stream->keep_alive = true;
    }
  }
  if (level > 1)
    parent_hstream->keep_alive = false;
  else if (parent_was_dormant)
    parent_fstream->Sleep();

  return result;
}

Block* Block::Segment(Block::Segmentation& segmentation) {
  Block* block = this;
  // segment to the left
  if (segmentation.offset > block->offset) {
    Block* new_block = new Block;
    std::memcpy(new_block, block, sizeof(Block));
    block->length = segmentation.offset - block->offset;
    block->next   = new_block;
    block->child  = nullptr;
    if (block->level > 0)
      reinterpret_cast<Streams::HybridStream*>(block->data)->reference_count++;
    block->Hash();
    block = new_block;
  }
  // segment to the right
  if (segmentation.offset - block->offset + segmentation.length < block->length) {
    Block* new_block = new Block;
    std::memcpy(new_block, block, sizeof(Block));
    new_block->data   = block->data;
    new_block->offset = segmentation.offset + segmentation.length;
    new_block->length -= new_block->offset - block->offset;
    new_block->parent = block->parent;
    new_block->next   = block->next, block->next = new_block;
    new_block->child  = nullptr;
    new_block->level  = block->level;
    if (block->level > 0)
      reinterpret_cast<Streams::HybridStream*>(block->data)->reference_count++;
  }

  block->type   = segmentation.type;
  block->offset = segmentation.offset;
  block->length = segmentation.length;
  if (segmentation.size_of_info > 0) {
    block->info = ::operator new(segmentation.size_of_info);
    std::memcpy(block->info, segmentation.info, segmentation.size_of_info);
  }
  block->done = true;
  block->Hash();

  if (segmentation.child.stream != nullptr) {
    block->child = new Block;
    std::memset(block->child, 0, sizeof(Block));
    block->child->type   = segmentation.child.type;
    block->child->data   = segmentation.child.stream;
    block->child->length = segmentation.child.stream->Size();
    block->child->parent = block;
    block->child->level  = block->level + 1;
    if (segmentation.child.size_of_info > 0) {
      block->child->info = ::operator new(segmentation.child.size_of_info);
      std::memcpy(block->child->info, segmentation.child.info, segmentation.child.size_of_info);
    }
    block->child->done = segmentation.child.done;
    block->child->Hash();
  }

  return block->next;
}

Block* Block::Next(std::uint32_t const lvl, bool const skip_done) {
  Block* block = this;
  while ((block == this) || (block->level != lvl) || (block->type == Block::Type::Dedup) || (skip_done && block->done)) {
    if (block->child != nullptr)
      block = block->child;
    else if (block->next != nullptr)
      block = block->next;
    else if ((block->parent != nullptr) && (block->parent->next != nullptr))
      block = block->parent->next;
    else
      return nullptr;
  }
  return block;
}

void Block::DeleteInfo() {
  if (info == nullptr)
    return;
  switch (type) {
    case Block::Type::Deflate: {
      delete static_cast<Structures::DeflateInfo*>(info);
      info = nullptr;
      break;
    }
    default: {}
  }
}

void Block::DeleteChilds(Storage::Manager& manager) {
  if (child == nullptr)
    return;
  Block* block = child;
  while (block != nullptr) {
    block->DeleteInfo();
    block->DeleteChilds(manager);
    if (block->level > 0)
      manager.Delete(reinterpret_cast<Streams::HybridStream*>(block->data));
    Block* next_block = block->next;
    delete block;
    block = next_block;
  }
  child = nullptr;
}

void Block::Hash() {
  // assumes data stream is available
  hash = CRC32::Process(data, offset, length);
  hashed = true;
}