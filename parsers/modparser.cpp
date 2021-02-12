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

#include "modparser.hpp"

ModParser::ModParser() {
  priority = Parsers::GetPriority(Parsers::Names::Mod);
}

bool ModParser::Parse(Block* block, Structures::ParsingData& data, Storage::Manager& manager) {
  UNUSED(manager);
  if (block == nullptr)
    return false;
  assert(!block->done);
  assert(block->type == Block::Type::Default);
  assert((block->data != nullptr) && ((block->level == 0) || reinterpret_cast<Streams::HybridStream*>(block->data)->Active()));
  std::int64_t i = 0; // current position relative to the initial block
  std::int64_t length = block->length;
  if (length < static_cast<std::int64_t>(WINDOW_SIZE + 512))
    return false;
  position = block->offset; // current position relative to the stream of the initial block
  if (!block->data->Seek(position))
    return false;
  wnd.Reset();
  std::uint32_t wnd32[2] = {};
  bool result = false;
  while (i < length) {
    std::size_t j = 0, bytes_read = block->data->Read(&buffer[0], buffer.size());
    while ((j < bytes_read) && (i < length)) {
      std::uint8_t const c = buffer[j++];
      wnd32[1] = (wnd32[1] << 8) | (wnd32[0] >> 24);
      wnd32[0] = (wnd32[0] << 8) | c;
      wnd.Add(c);
      i++, position++;
      // Start of detection code
      if ((wnd.Position() >= SIGNATURE_END_OFFSET) &&
          (wnd(134) <= 0x80) &&
          ((wnd32[1] & 0x80808080) == 0) && (
            (wnd32[0] == 0x4D2E4B2E /* "M.K." */) ||
            (wnd32[0] == 0x4D214B21 /* "M!K!" (ProTracker, when more than 64 patterns are used*/) ||
            (wnd32[0] == 0x464C5434 /* "FLT4" */) ||
            (wnd32[0] == 0x464C5438 /* "FLT8" */) ||
            (wnd32[0] == 0x43443831 /* "CD81" */) ||
            ((wnd32[0] & 0xFFFFFFFC) == 0x54445A30 /* "TDZx", x in 1-3 */) ||
            ((wnd32[0] & 0xFFF7FFFF) == 0x4F435441 /* "OCTA" or "OKTA" */) ||
            ((wnd32[0] & 0xF1FFFFFF) == 0x3043484E /* "xCHN", x is even*/ && (wnd(4) & 0xE) < 10) ||
            (((wnd32[0] & 0xF0F0FFF9) == 0x30304348) && ((c == 0x48) || (c == 0x4E)) && ((wnd(4) < 0x3A) && (wnd(3) < 0x3A)) /* "xxCH" or "xxCN", x in 0-9*/)
          )
         )
      {
        bool const sig_ddCx = (wnd32[0] & 0xFFFF) == 0x4348;  // "..CH" or "..CN"
        bool const sig_CD81 = (wnd32[0] & 0xFFFF) == 0x3831;  // "CD81"
        bool const sig_M_K_ = (wnd32[0] & 0xFFFF) == 0x4B21;  // "M!K!"
        std::uint8_t const c4 = wnd(4);
        std::uint32_t const channels =  sig_ddCx ?
                                          (c4 & 0x0F) * 10 + (wnd(3) & 0x0F) :
                                          ((c4 & 0xF1) == 0x30 /* even digit */) ?
                                            c4 & 0x0F :
                                            (c4 == 0x54 /* "TDZx" */) ?
                                              c & 0x0F :
                                              (c == 0x38 /* "FLT8" */ || c == 0x41 /* "OCTA" or "OKTA" */ || sig_CD81) ? 8 : 4;
        if ((channels == 0) || (sig_ddCx && ((channels & 1) > 0) /*odd*/))
          continue;

        std::size_t size = 0, k = 0;
        do {
          std::size_t const p = (SIGNATURE_END_OFFSET - 42) - k * 30;  //skip sample name
          std::size_t const sample_length = wnd(p) * 512 + wnd(p - 1) * 2;
          if ((sample_length > 0) && ((wnd(p - 2) > 0x0F /*sample finetune, 0..0x0F*/) || (wnd(p - 3) > 0x40 /*sample volume, 0..0x40*/)))
            break;
          size += sample_length;
          k++;
        } while (k < 31);
        if ((k < 31) || (size == 0))
          continue;

        std::uint32_t num_patterns = 1;
        for (k = 0; k < 128; k++)
          num_patterns = std::max<std::uint32_t>(num_patterns, wnd(132-k) + 1);

        if (num_patterns <= ((sig_ddCx || sig_M_K_) ? 128u : 64u)) {
          std::int64_t const offset = position + 256LL * channels * num_patterns;
          data.audio.bps = 8;
          data.audio.channels = 1;
          data.audio.mode = 4;

          Block::Segmentation segmentation{};
          segmentation.offset = offset;
          segmentation.length = size;
          segmentation.type = Block::Type::Audio;
          segmentation.info = &data.audio;
          segmentation.size_of_info = sizeof(Structures::AudioInfo);
          block = block->Segment(segmentation);

          wnd.Reset();
          wnd32[1] = wnd32[0] = 0;
          i += offset + size - position;
          position = offset + size;
          break;
        }
      }
      // End of detection code
    }
    if ((i < length) && !block->data->Seek(position))
      return result;
  }
  return result;
}
