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

#include "jpegparser.hpp"

JpegParser::JpegParser(const std::shared_ptr<void>& options) : allow_progressive(false) {
  if (options != nullptr)
    allow_progressive = *std::static_pointer_cast<bool>(options);
  priority = Parsers::GetPriority(Parsers::Names::JPEG);
}

bool JpegParser::Parse(Block* block, Structures::ParsingData& data, Storage::Manager& manager) {
  UNUSED(data);
  UNUSED(manager);
  if (block == nullptr)
    return false;
  assert(!block->done);
  assert(block->type == Block::Type::Default);
  assert((block->data != nullptr) && ((block->level == 0) || reinterpret_cast<Streams::HybridStream*>(block->data)->Active()));
  std::int64_t i = 0; // current position relative to the initial block
  std::int64_t length = block->length;
  if (length < 512)
    return false;
  position = block->offset; // current position relative to the stream of the initial block
  if (!block->data->Seek(position))
    return false;
  bool result = false;
  std::uint32_t previous_4_bytes = 0;
  while (i < length) {
    std::size_t j = 0, bytes_read = block->data->Read(&buffer[0], buffer.size());
    while ((j < bytes_read) && (i < length)) {
      std::uint8_t c = buffer[j++];
      previous_4_bytes = (previous_4_bytes << 8) | c;
      i++, position++;
      // Start of detection code
      if ( ((previous_4_bytes & 0xFFFFFF00u) == 0xFFD8FF00u /*SOI marker, followed by:*/) && (
            (c == JPEG::Markers::SOF0) || 
            (c == JPEG::Markers::SOF1) || 
           ((c == JPEG::Markers::SOF2) && allow_progressive) ||
            (c == JPEG::Markers::DHT ) ||
           ((c >= JPEG::Markers::DQT ) && (c < 0xFF))
        )) {
        bool done = false, found = false, has_quantization_table = (c == JPEG::Markers::DQT), progressive = (c == JPEG::Markers::SOF2);
        std::int64_t start = position, offset = start - 2;
        // process markers
        do {
          if (!block->data->Seek(offset) || (block->data->Read(&buffer[0], 5) != 5) || (buffer[0] != 0xFF))
            break;

          std::int64_t marker_length = static_cast<std::int64_t>(buffer[2]) * 256 + static_cast<std::int64_t>(buffer[3]);
          switch (buffer[1]) {
            case JPEG::Markers::DQT: {
              // FF DB XX XX QtId ...
              // Marker length (XX XX) must be = 2 + multiple of 65 <= 260
              // QtId:
              // bit 0..3: number of QT (0..3, otherwise error)
              // bit 4..7: precision of QT, 0 = 8 bit, otherwise 16 bit
              if ((marker_length <= 262) && (((marker_length - 2) % 65) == 0) && (buffer[4] <= 3)) {
                has_quantization_table = true;
                offset += marker_length + 2;
              }
              else
                done = true;
              break;
            }
            case JPEG::Markers::DHT: {
              offset += marker_length + 2;
              done = (((buffer[4] & 0xF) > 3) || ((buffer[4] >> 4) > 1));
              break;
            }
            case JPEG::Markers::SOS: {
              found = has_quantization_table;
              done = true;
              break;
            }
            case JPEG::Markers::EOI: {  //EOI with no SOS?
              done = true;
              break; 
            }
            case JPEG::Markers::SOF2: {
              progressive = allow_progressive;
              if (progressive) {
                offset += marker_length + 2;
                done = (buffer[4] != 0x08);
              }
              else
                done = true;
              break;
            }
            case JPEG::Markers::SOF0: {
              offset += marker_length + 2;
              done = (buffer[4] != 0x08);
              break;
            }
            default: offset += marker_length + 2;
          }
        } while (!done);
        
        if (found) {
          // we seem to have found a valid image, now try to find a valid EOI marker
          found = done = false;
          offset += 5;
          bool is_marker = (buffer[4] == 0xFF);
          do {
            bytes_read = block->data->Read(&buffer[0], buffer.size());

            for (std::size_t k = 0; !done && (k < bytes_read); k++) {
              c = buffer[k];
              offset++;
              if (!is_marker)
                is_marker = (c == 0xFF);
              else {
                done = ((c > 0) && (((c & 0xF8) != 0xD0) /*skip Restart Markers RST0 to RST7*/) && ((progressive) ? (c != JPEG::Markers::DHT) && (c != JPEG::Markers::SOS) : true));
                found = (c == JPEG::Markers::EOI);
                is_marker = false;
              }
            }
          } while (!done && (bytes_read > 0));
        }

        if (found) {
          Block::Segmentation segmentation{};
          segmentation.offset = start - 4;
          segmentation.length = offset - segmentation.offset;
          segmentation.type = Block::Type::JPEG;
          block = block->Segment(segmentation);

          i += offset - start;
          position = offset;
          result = true;
        }
        previous_4_bytes = 0;
        break;
      }
      // End of detection code
    }
    if ((i < length) && !block->data->Seek(position))
      return result;
  }
  return result;
}