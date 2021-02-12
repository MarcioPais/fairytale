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

#include "deflateparser.hpp"

inline std::uint8_t DeflateParser::ReadBack(std::size_t const x) {
  return window[(wnd_position - WINDOW_LOOKBACK + x) & WINDOW_ACCESS_MASK];
}

inline bool DeflateParser::Validate(std::int64_t const in, std::int64_t const out, bool const brute) {
  return (in > (brute ? 32LL: 16LL)) && (in * 8 <= out * 9);
}

void DeflateParser::ClearBuffers() {
  window.fill(0);
  input_block.fill(0);
  output_block.fill(0);
  skip_positions.fill(false);
  histogram.fill(0);
  gzip = {};
  zip_offset = index = 0;
  wnd_position = 0;
}

void DeflateParser::ProcessByte(std::uint8_t const b) {
  histogram[b]++;
  if (index >= 256) {
    assert(histogram[window[wnd_position]] > 0);
    histogram[window[wnd_position]]--;
  }
  window[wnd_position] = b;
  if (wnd_position < WINDOW_LOOKBACK)
    window[wnd_position + BRUTE_LOOKBACK] = b;
  wnd_position = (wnd_position + 1) & WINDOW_ACCESS_MASK;
}

void DeflateParser::PerformBruteModeSearch(bool& result) {
  if (skip_positions[wnd_position]) {
    skip_positions[wnd_position] = false;
    return;
  }
  std::uint8_t const BTYPE = (window[wnd_position] & 7) >> 1;
  result = (BTYPE == 1) || (BTYPE == 2);
  if (result) {
    std::size_t maximum = 0, used = 0, offset = wnd_position;
    for (std::size_t i = 0; i < BRUTE_ROUNDS; i++, offset += 64u) {
      for (std::size_t j = 0; j < 64u; j++) {
        std::size_t const freq = static_cast<std::size_t>(histogram[window[(offset + j) & WINDOW_ACCESS_MASK]]);
        used += (freq > 0);
        maximum += (freq > maximum);
      }
      if ((maximum >= ((12 + i) << i)) || ((used * (6 - i)) < ((i + 1) * 64))) {
        result = false;
        break;
      }
    }
  }
}

void DeflateParser::GetStreamInfo(Block* block, Structures::DeflateInfo& info, bool const brute) {
  int ret = Z_OK;
  z_stream stream;
  zLib::SetupStream(&stream);
  // Quick check possible stream by decompressing first WINDOW_LOOKBACK bytes
  if (zLib::InflateInit(&stream, info.zLib.parameters) == Z_OK) {
    stream.next_in = &window[(wnd_position - (brute ? 0 : WINDOW_LOOKBACK)) & WINDOW_ACCESS_MASK];
    stream.avail_in = WINDOW_LOOKBACK;
    stream.next_out = &output_block[0];
    stream.avail_out = static_cast<uInt>(zLib::BLOCK_SIZE);
    ret = inflate(&stream, Z_FINISH);
    ret = ((inflateEnd(&stream) == Z_OK) && ((ret == Z_STREAM_END) || (ret == Z_BUF_ERROR)) && (stream.total_in >= 16));
  }
  // Verify possible valid stream and determine its length
  if (ret != 0) {
    zLib::SetupStream(&stream);
    stream.total_in = stream.total_out = 0u;
    if (zLib::InflateInit(&stream, info.zLib.parameters) == Z_OK) {
      if (!block->data->Seek(position - (brute ? BRUTE_LOOKBACKi64 : WINDOW_LOOKBACKi64)))
        return;
      std::size_t block_size = 0;
      std::int64_t total_in = 0, total_out = 0;
      do {
        block_size = block->data->Read(&input_block[0], input_block.size());
        stream.next_in = &input_block[0];
        stream.avail_in = static_cast<uInt>(block_size);
        do {
          stream.next_out = &output_block[0];
          stream.avail_out = static_cast<uInt>(zLib::BLOCK_SIZE);
          ret = inflate(&stream, Z_FINISH);
        } while ((stream.avail_out == 0) && (ret == Z_BUF_ERROR));
        // update totals, taking care to handle 32-bit overflows
        std::int64_t new_total = static_cast<std::int64_t>(stream.total_in), total_low = total_in & ULONG_MASK;
        total_in += ((total_low <= new_total) ? new_total : 0x100000000LL + new_total) - total_low;
        new_total = static_cast<std::int64_t>(stream.total_out), total_low = total_out & ULONG_MASK;
        total_out += ((total_low <= new_total) ? new_total : 0x100000000LL + new_total) - total_low;
        if ((ret == Z_STREAM_END) && Validate(total_in, total_out, brute))
          info.compressed_length = total_in, info.uncompressed_length = total_out;
        if (ret != Z_BUF_ERROR)
          break;
      } while (block_size > 0);
      if (inflateEnd(&stream) != Z_OK)
        info.compressed_length = info.uncompressed_length = 0;
    }
  }
}

DeflateParser::DeflateParser(const std::shared_ptr<void>& options) : configuration{} {
  if (options != nullptr) {
    int const flags = *std::static_pointer_cast<int>(options);
    configuration.use_brute_mode = flags & DeflateParser::Options::UseBruteMode;
    configuration.parse_zip_streams = flags & DeflateParser::Options::ParseZipStreams;
    configuration.parse_gzip_streams = flags & DeflateParser::Options::ParseGZipStreams;
  }
  priority = Parsers::GetPriority(Parsers::Names::Deflate);
}

bool DeflateParser::Parse(Block* block, Structures::ParsingData& data, Storage::Manager& manager) {
  if (block == nullptr)
    return false;
  assert(!block->done);
  assert(block->type == Block::Type::Default);
  assert((block->data != nullptr) && ((block->level == 0) || reinterpret_cast<Streams::HybridStream*>(block->data)->Active()));
  std::int64_t i = 0; // current position relative to the initial block
  std::int64_t length = block->length;
  if (length < WINDOW_LOOKBACKi64)
    return false;
  position = block->offset; // current position relative to the stream of the initial block
  if (!block->data->Seek(position))
    return false;
  bool result = false;
  ClearBuffers();
  while (i < length) {
    std::size_t j = 0, bytes_read = block->data->Read(&buffer[0], buffer.size());
    while ((j < bytes_read) && (i < length)) {
      ProcessByte(buffer[j++]);
      index++, i++, position++;
      // Start of generic detection code
      data.deflate.zLib.parameters = zLib::ParseHeader( (ReadBack(0) << 8) | ReadBack(1) );
      data.deflate.compressed_length = 0;
      data.deflate.uncompressed_length = 0;
      bool valid = (index >= (WINDOW_LOOKBACKi64 - 1)) && (data.deflate.zLib.parameters != -1);
      if (configuration.use_brute_mode && !valid && (index >= WINDOW_ACCESS_MASKi64))
        PerformBruteModeSearch(valid);
      bool const brute = (data.deflate.zLib.parameters == -1) && (configuration.parse_zip_streams && (index != zip_offset)) && (configuration.parse_gzip_streams && (index != gzip.offset));

      if (valid || (configuration.parse_zip_streams && (zip_offset > 0) && (index == zip_offset)) || (configuration.parse_gzip_streams && (gzip.offset > 0) && (index == gzip.offset))) {
        skip_positions[(wnd_position - WINDOW_LOOKBACK) & WINDOW_ACCESS_MASK] = !brute;
        GetStreamInfo(block, data.deflate, brute);
      }

      if (data.deflate.compressed_length > 0) {  // we have a valid stream
        std::int64_t offset = position - (brute ? BRUTE_LOOKBACKi64 : WINDOW_LOOKBACKi64);  // actual initial stream offset
        if (!block->data->Seek(offset))
          break;

        Streams::HybridStream* output = transform.Attempt(*block->data, manager, &data.deflate);
        if (output != nullptr) {
          Block::Segmentation segmentation{};
          segmentation.offset = offset;
          segmentation.length = data.deflate.compressed_length;
          segmentation.type = Block::Type::Deflate;
          segmentation.info = &data.deflate;
          segmentation.size_of_info = sizeof(Structures::DeflateInfo);
          segmentation.child.stream = output;
          block = block->Segment(segmentation);

          output->priority = Streams::Priority::High;
          result = true;
          if (block == nullptr)
            return result;
        }
        // we succeeded, or it was a valid stream but we didn't have enough storage budget for it, so skip it anyway
        if ((output != nullptr) || (data.deflate.zLib.combination < zLib::POSSIBLE_COMBINATIONS)) {
          index = 0;
          i += data.deflate.compressed_length - (brute ? BRUTE_LOOKBACKi64 : WINDOW_LOOKBACKi64);
          position = offset + data.deflate.compressed_length;
          ClearBuffers();
          bytes_read = 0; // to ensure we break
        }
      }
      // End of generic detection code
      // Start of format-specific detection code
      if (configuration.parse_zip_streams && (index > zip_offset))
        zip_offset = 0;
      if (configuration.parse_gzip_streams && (index > gzip.offset))
        gzip.offset = 0;
      if ((index > 0) && (data.deflate.zLib.parameters == -1)) {
        // detect ZIP streams
        if (configuration.parse_zip_streams &&
            (zip_offset == 0) &&
            (ReadBack(0) == 'P') &&
            (ReadBack(1) == 'K') &&
            (ReadBack(2) == '\x3') &&
            (ReadBack(3) == '\x4') &&
            (ReadBack(8) == '\x8') &&
            (ReadBack(9) == '\0'))
        {
          std::int64_t const nlen = ReadBack(26) + ReadBack(27) * 256LL + ReadBack(28) + ReadBack(29) * 256LL;
          if ((nlen < 256) && (block->offset + index + 30 + nlen < block->data->Size()))
            zip_offset = index + 30 + nlen;
        }
        // detect gZip streams
        else if (configuration.parse_gzip_streams &&
                 (gzip.offset == 0) &&
                 (ReadBack(0) == 0x1F) &&
                 (ReadBack(1) == 0x8B) &&
                 (ReadBack(2) == 0x08) &&
                 (((gzip.options = ReadBack(3)) & 0xC0) == 0))
        {
          gzip.offset = index + 10;
          if (gzip.options & gZip::EXTRA)
            gzip.offset += 2LL + ReadBack(10) + ReadBack(11) * 256LL;
          if (gzip.offset >= block->length)
            gzip.offset = 0;
          else {
            if (!block->data->Seek(position + gzip.offset - (2 * WINDOW_LOOKBACKi64 - 1)))
              break;

            if (gzip.options & gZip::NAME) {
              do {
                gzip.offset++;
              } while (block->data->GetByte() > 0);
              if (gzip.offset >= block->length)
                gzip.offset = 0;
            }
            if (gzip.offset && (gzip.options & gZip::COMMENT)) {
              do {
                gzip.offset++;
              } while (block->data->GetByte() > 0);
              if (gzip.offset >= block->length)
                gzip.offset = 0;
            }
            if (gzip.offset && (gzip.options & gZip::CRC)) {
              gzip.offset += 2;
              if (gzip.offset >= block->length)
                gzip.offset = 0;
            }
          }
        }
      }
      // End of format-specific detection code
    }
    if ((i < length) && !block->data->Seek(position))
      return result;
  }
  return result;
}