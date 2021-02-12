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

#include "analyser.hpp"
#include <array>

Analyser::Analyser(const std::vector<std::pair<const Parsers::Names, const std::shared_ptr<void>>>& parsers) {
  std::array<bool, static_cast<std::size_t>(Parsers::Names::Count)> used{};
  for (auto parser : parsers) {
    std::size_t const index = static_cast<std::size_t>(parser.first);
    if ((parser.first > Parsers::Names::Count) || (used[index]))
      continue;
    used[index] = true;
    switch (parser.first) {
      case Parsers::Names::Deflate: {
        strict.push_back(std::make_shared<DeflateParser>(parser.second));
        break;
      }
      case Parsers::Names::JPEG: {
        strict.push_back(std::make_shared<JpegParser>(parser.second));
        break;
      }
      case Parsers::Names::Bitmap: {
        strict.push_back(std::make_shared<BitmapParser>(parser.second));
        break;
      }
      case Parsers::Names::Mod: {
        strict.push_back(std::make_shared<ModParser>());
        break;
      }
      default: {}
    }
  }
  std::sort(
    strict.begin(), strict.end(),
    [](std::shared_ptr<Parser<Parsers::Types::Strict>> const& lhs, std::shared_ptr<Parser<Parsers::Types::Strict>> const& rhs) -> bool {
      return lhs->priority > rhs->priority;
    }
  );
  std::sort(
    fuzzy.begin(), fuzzy.end(),
    [](std::shared_ptr<Parser<Parsers::Types::Fuzzy>> const& lhs, std::shared_ptr<Parser<Parsers::Types::Fuzzy>> const& rhs) -> bool {
      return lhs->priority > rhs->priority;
    }
  );
  data = {};
}

bool Analyser::Process(Block& block, Storage::Manager& manager, Deduper* deduper) {
  if ((block.data == nullptr) || (block.level >= Block::MAX_RECURSION_LEVEL))
    return false;
  Streams::HybridStream* hstream = reinterpret_cast<Streams::HybridStream*>(block.data);
  Streams::FileStream* fstream;
  if ((block.level > 0) && (!hstream->Active() && !block.Revive(manager)))
    return false;
  if (deduper != nullptr)
    deduper->Process(block, nullptr, manager);
  std::uint32_t level = block.level;
  struct {
    bool global;  // true if we found anything at all
    bool level;   // true if we found anything at this recursion level
    bool parser;  // true if the current parser found anything
  } result {};
  Block *b, *next;
  do {
    result.level = false;
    for (auto const type : Parsers::All) {
      std::size_t const size = (type == Parsers::Types::Strict) ? strict.size() : fuzzy.size();
      for (std::size_t i = 0; i < size; i++) {
        b = &block;
        if ((b->level != level) || b->done)
          b = b->Next(level);
        while (b != nullptr) {
          hstream = reinterpret_cast<Streams::HybridStream*>(b->data);
          fstream = reinterpret_cast<Streams::FileStream*>(b->data);

          if (level > 0) {
            // attempt stream revival if needed
            if (!hstream->Active() && !b->Revive(manager))
              break;
            else  // don't let it be purged from storage
              hstream->keep_alive = true;
          }
          else if (!fstream->WakeUp())
            break;

          // get pointer to current "next" block at this recursion level, since the segmentation may change that
          next = b->Next(level);

          result.parser = (type == Parsers::Types::Strict) ? strict[i]->Parse(b, data, manager) : fuzzy[i]->Parse(b, data, manager);
          result.global |= result.level |= result.parser;
          if (result.parser || (next == nullptr)) {
            Block* current = b;
            while ((current != nullptr) && (current != next)) {
              if (UNLIKELY(!current->hashed))
                current->Hash();
              current = current->next;
            }
          }

          if ((deduper != nullptr) && result.parser)
            deduper->Process(*b, next, manager);

          if ((next == nullptr) || (next->data != b->data)) {
            if (level > 0)
              hstream->keep_alive = false;
            else
              fstream->Sleep();
          }
          b = next;
        }
      }
    }
    level++;
  } while (result.level && (level < Block::MAX_RECURSION_LEVEL));

  return result.global;
}