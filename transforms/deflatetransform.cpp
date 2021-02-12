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

#include "deflatetransform.hpp"
#include "../misc/uleb128.hpp"

bool DeflateTransform::Validate(Structures::DeflateInfo const& data) {
  assert(data.penalty_bytes_count < zLib::MAX_PENALTY_BYTES);
  if (data.compressed_length >= 256LL)
    return true;
  constexpr std::int64_t BIAS = 2, REFERENCE = 8;
  assert((BIAS >= 0) && (REFERENCE > BIAS));
  std::int64_t segmentation_cost =  // best-case estimate of how many bytes will the segmentation structure cost us, not accounting for possible 3-way block split
                                   1 +  // block id
                                   1 +  // block type
                                   1 +  // zLib combination
                                   1 +  // zLib window
                                   1 +  // zLib number of penalty bytes
                                   static_cast<std::int64_t>(data.penalty_bytes_count) +
                                   1 +  // child block id
                                   1 +  // child block type
                                   ULEB128::Cost(data.compressed_length); // child block compressed length estimate, assuming compression on-par with deflate
  for (std::size_t i = 0; i <= data.penalty_bytes_count; i++)
    segmentation_cost += ULEB128::Cost(data.differ_positions[i]);
  return (data.compressed_length * BIAS > segmentation_cost * REFERENCE);
}

void DeflateTransform::ClearBuffers() {
  input_block.fill(0);
  output_block.fill(0);
  recompressed_block.fill(0);
  recompressed_block_positions.fill(0);
  differ_counts.fill(0);
  penalty_bytes.fill(0);
  differ_positions.fill(-1);
  recompressed_streams.fill({});
  recompressed_streams_sizes.fill(0);
  skip_mode = {};
  main_stream = {};
}

void DeflateTransform::SetupParameters(Structures::DeflateInfo& data) {
  data.zLib.combination = zLib::POSSIBLE_COMBINATIONS;
  data.zLib.window = (data.zLib.parameters == -1) ? 0 : MAX_WBITS + 10 + static_cast<std::uint8_t>(data.zLib.parameters / 4);
  int const ctype = data.zLib.parameters % 4;
  int const minclevel = (data.zLib.window == 0) ? 1 : (ctype == 3) ? 7 : (ctype == 2) ? 6 : (ctype == 1) ? 2 : 1;
  int const maxclevel = (data.zLib.window == 0) ? 9 : (ctype == 3) ? 9 : (ctype == 2) ? 6 : (ctype == 1) ? 5 : 1;
  for (std::size_t i = 0; i < zLib::POSSIBLE_COMBINATIONS; i++) {
    int clevel = static_cast<int>(i / 9) + 1;
    // Early skip if invalid parameter
    if ((clevel < minclevel) || (clevel > maxclevel))
      differ_counts[i] = zLib::MAX_PENALTY_BYTES;
    recompressed_block_positions[i] = zLib::BLOCK_SIZE * 2;
  }
  total_in = 0;
  trials = 0;
  main_return = Z_STREAM_END;
}

bool DeflateTransform::AttemptBlockRecompression(std::int64_t const base, std::int64_t const length, std::size_t const id) {
  trials++;
  // update the recompressed stream buffer information
  recompressed_streams[id].next_in   = &output_block[0];
  recompressed_streams[id].avail_in  = static_cast<uInt>(zLib::BLOCK_SIZE - main_stream.avail_out);
  recompressed_streams[id].next_out  = &recompressed_block[recompressed_block_positions[id]];
  recompressed_streams[id].avail_out = static_cast<uInt>(zLib::BLOCK_SIZE * 2 - recompressed_block_positions[id]);
  // now deflate/recompress it
  int ret = deflate(&recompressed_streams[id], (total_in == length) ? Z_FINISH : Z_NO_FLUSH);
  if ((ret != Z_BUF_ERROR) && (ret != Z_STREAM_END) && (ret != Z_OK)) {
    differ_counts[id] = zLib::MAX_PENALTY_BYTES;
    return false;
  }
  // proceed to compare the recompressed data to the original
  std::size_t const end = zLib::BLOCK_SIZE * 2 - static_cast<std::size_t>(recompressed_streams[id].avail_out);  // recompressed block end offset
  // we need to handle 32-bit overflow
  std::int64_t const new_total_out = static_cast<std::int64_t>(recompressed_streams[id].total_out), total_out_low = recompressed_streams_sizes[id] & ULONG_MASK;
  recompressed_streams_sizes[id] += ((total_out_low <= new_total_out) ? new_total_out : 0x100000000LL + new_total_out) - total_out_low;

  std::size_t tail = static_cast<std::size_t>(std::max<std::int64_t>((main_return == Z_STREAM_END) ?length - recompressed_streams_sizes[id] : 0LL, 0LL));
  std::size_t i = recompressed_block_positions[id];

  std::uint_fast64_t* pRec = reinterpret_cast<std::uint_fast64_t*>(&recompressed_block[i]);
  std::uint_fast64_t* pIn  = reinterpret_cast<std::uint_fast64_t*>(&input_block[i]);
  constexpr std::size_t stride = sizeof(std::uint_fast64_t);
  while ((i + stride < end) && ((*pRec) == (*pIn))) {
    pRec++, pIn++, i += stride;
  }
  std::int64_t offset = base + static_cast<std::int64_t>(i) - zLib::BLOCK_SIZEi64;
  assert(offset >= 0);
  for (; i < end; i++, offset++) {
    if ((offset < length) && (recompressed_block[i] != input_block[i])) {
      if (++differ_counts[id] < zLib::MAX_PENALTY_BYTES) {
        std::size_t const position = id * zLib::MAX_PENALTY_BYTES + differ_counts[id];
        differ_positions[position] = offset;
        penalty_bytes[position] = input_block[i];
      }
      else
        return false;
    }
  }
  if (tail > 0) {
    if (differ_counts[id] + tail < zLib::MAX_PENALTY_BYTES) {
      std::size_t position = id * zLib::MAX_PENALTY_BYTES + differ_counts[id] + 1;
      differ_counts[id] += tail;
      for (i = 0; i < tail; i++, position++, offset++) {
        differ_positions[position] = offset;
        penalty_bytes[position] = input_block[end + i];
      }
    }
    else {
      differ_counts[id] = zLib::MAX_PENALTY_BYTES;
      return false;
    }
  }
  recompressed_block_positions[id] = end;
  return true;
}

Streams::HybridStream* DeflateTransform::Attempt(Streams::Stream& input, Storage::Manager& manager, void* info) {
  if (info == nullptr)
    return nullptr;
  Structures::DeflateInfo* data = reinterpret_cast<Structures::DeflateInfo*>(info);
  if (data->compressed_length > manager.capacity())
    return nullptr;
  ClearBuffers();
  if (zLib::InflateInit(&main_stream, data->zLib.parameters) != Z_OK)
    return nullptr;
  SetupParameters(*data);
  std::int64_t const initial_position = input.Position();
  std::size_t index = SIZE_MAX;  // zlib combination used
  bool found = false;

  for (std::int64_t offset = 0; offset < data->compressed_length; offset += zLib::BLOCK_SIZEi64) {  // stream offset
    assert(offset == total_in);
    // amount of data to read this iteration
    std::size_t block_size = static_cast<std::size_t>(std::min<std::int64_t>(data->compressed_length - offset, zLib::BLOCK_SIZEi64));
    // see how many trials we need to run
    trials = 0;
    for (std::size_t j = 0; j < zLib::POSSIBLE_COMBINATIONS; j++) {
      if (differ_counts[j] >= zLib::MAX_PENALTY_BYTES)
        continue;  // this combination has already been ruled out
      trials++;
      if (recompressed_block_positions[j] >= zLib::BLOCK_SIZE)
        recompressed_block_positions[j] -= zLib::BLOCK_SIZE;  // prevent overflows
      assert(recompressed_block_positions[j] < recompressed_block.size());
    }
    // early break if there's nothing left to test
    if (trials == 0)
      break;
    // rotate block contents
    std::memmove(&recompressed_block[0], &recompressed_block[zLib::BLOCK_SIZE], zLib::BLOCK_SIZE);
    std::memmove(&input_block[0], &input_block[zLib::BLOCK_SIZE], zLib::BLOCK_SIZE);
    // finish filling the input block with data from the input stream
    if (input.Read(&input_block[zLib::BLOCK_SIZE], block_size) != block_size)
      break;
    // see if we can switch to skip mode
    if ((!skip_mode.active) && (offset >= skip_mode.threshold) && (trials > 1)) {
      if (inflateCopy(&skip_mode.saved.backup_stream, &main_stream) == Z_OK) {
        std::memmove(&skip_mode.saved.input_block[0], &input_block[0], zLib::BLOCK_SIZE);
        std::memmove(&skip_mode.saved.recompressed_block[0], &recompressed_block[0], zLib::BLOCK_SIZE);
        skip_mode.saved.main_return = main_return;
        skip_mode.saved.offset = offset;
        skip_mode.threshold += zLib::BLOCK_SIZEi64;
        for (skip_mode.id = MTF.First(); (skip_mode.id != SIZE_MAX) && (differ_counts[skip_mode.id] >= zLib::MAX_PENALTY_BYTES); skip_mode.id = MTF.Next());
        assert(skip_mode.id != SIZE_MAX);
        skip_mode.active = true;
      }
    }
    // a single (compressed) input block may lead to several (uncompressed) output blocks
    main_stream.next_in = &input_block[zLib::BLOCK_SIZE];
    main_stream.avail_in = static_cast<uInt>(block_size);
    do {
      trials = 0;
      // decompress (inflate) block
      main_stream.next_out = &output_block[0];
      main_stream.avail_out = static_cast<uInt>(zLib::BLOCK_SIZE);
      main_return = inflate(&main_stream, Z_FINISH);
      // update total input bytes used, taking care to handle 32-bit overflows
      std::int64_t const new_total_in = static_cast<std::int64_t>(main_stream.total_in), total_in_low = total_in & ULONG_MASK;
      total_in += ((total_in_low <= new_total_in) ? new_total_in : 0x100000000LL + new_total_in) - total_in_low;

      if (skip_mode.active) {
        if (!AttemptBlockRecompression(offset, data->compressed_length, skip_mode.id)) {
          // this combination has failed to fully recreate the stream, so we must resume trying available combinations
          skip_mode.active = false;
          inflateEnd(&main_stream);
          if ((inflateCopy(&main_stream, &skip_mode.saved.backup_stream) != Z_OK) || (!input.Seek(initial_position + skip_mode.saved.offset))) {
            // failed to restore previous state, force a failure
            inflateEnd(&skip_mode.saved.backup_stream);
            differ_counts.fill(zLib::MAX_PENALTY_BYTES);
            trials = 0;
            break;
          }
          inflateEnd(&skip_mode.saved.backup_stream);
          // restore buffers, taking into account that they'll be rotated at the start of the next iteration
          std::memmove(&input_block[zLib::BLOCK_SIZE], &skip_mode.saved.input_block[0], zLib::BLOCK_SIZE);
          std::memmove(&recompressed_block[zLib::BLOCK_SIZE], &skip_mode.saved.recompressed_block[0], zLib::BLOCK_SIZE);
          // restore other parameters
          main_return = skip_mode.saved.main_return;
          offset = skip_mode.saved.offset - zLib::BLOCK_SIZEi64;  // offset will now be incremented in the for loop, so account for that
          total_in = skip_mode.saved.offset;
        }
        else if (main_return == Z_STREAM_END) {
          std::size_t const count = differ_counts[skip_mode.id];
          if (count == 0) {  // perfect match found
            index = skip_mode.id;
            found = true;
          }
          else {
            // we need penalty bytes to losslessly recreate the stream with this combination.
            // to ensure this combination is picked up, invalidate all others
            differ_counts.fill(zLib::MAX_PENALTY_BYTES);
            differ_counts[skip_mode.id] = count;
          }
          break;
        }
      }
      else {  // recompress (deflate) block with all possible valid parameters
        for (std::size_t j = MTF.First(); j != SIZE_MAX; j = MTF.Next()) {
          if (differ_counts[j] >= zLib::MAX_PENALTY_BYTES)
            continue;  // this combination has already been ruled out
          else if (recompressed_streams[j].next_out == nullptr) {  // initialize recompressed stream, if needed
            int const level = (static_cast<int>(j) / 9) + 1;
            int const memLevel = (static_cast<int>(j) % 9) + 1;
            int ret = deflateInit2(&recompressed_streams[j], level, Z_DEFLATED, data->zLib.window - MAX_WBITS, memLevel, Z_DEFAULT_STRATEGY);
            if (ret != Z_OK) {
              differ_counts[j] = zLib::MAX_PENALTY_BYTES;
              continue;
            }
          }
          if (!AttemptBlockRecompression(offset, data->compressed_length, j))
            continue;
          // early break on perfect match
          else if ((main_return == Z_STREAM_END) && (differ_counts[j] == 0)) {
            index = j;
            found = true;
            break;
          }
        }
      }
    } while ((main_stream.avail_out == 0) && (main_return == Z_BUF_ERROR) && (trials > 0));
    // we're done with this input block
    if (((main_return != Z_BUF_ERROR) && (main_return != Z_STREAM_END)) || (trials == 0))
      break;
  }

  // clean-up and get best match
  std::size_t min_differ_count = 0;
  if (found) {
    for (std::size_t i = 0; i < zLib::POSSIBLE_COMBINATIONS; i++) {
      if (recompressed_streams[i].next_out != nullptr)
        deflateEnd(&recompressed_streams[i]);
    }
  }
  else {
    min_differ_count = zLib::MAX_PENALTY_BYTES;
    for (std::size_t i = 0; i < zLib::POSSIBLE_COMBINATIONS; i++) {
      if (recompressed_streams[i].next_out != nullptr)
        deflateEnd(&recompressed_streams[i]);
      if (differ_counts[i] < min_differ_count)
        min_differ_count = differ_counts[index = i];
    }
  }
  inflateEnd(&main_stream);
  if (min_differ_count >= zLib::MAX_PENALTY_BYTES)
    return nullptr;

  // save reconstruction info
  data->penalty_bytes_count = min_differ_count;
  data->zLib.combination = static_cast<std::uint8_t>(index);
  std::size_t offset = index * zLib::MAX_PENALTY_BYTES;
  // the mismatch positions are delta-coded as the distance to the previous mismatch
  for (std::size_t i = 0; i < min_differ_count; i++, offset++)
    data->differ_positions[i] = differ_positions[offset + 1] - differ_positions[offset] - 1;
  data->differ_positions[min_differ_count] = data->compressed_length - differ_positions[offset];
  
  offset = index * zLib::MAX_PENALTY_BYTES + 1;
  for (std::size_t i = 0; i < min_differ_count; i++, offset++)
    data->penalty_bytes[i] = penalty_bytes[offset];

  // validate detection
  if (!Validate(*data))
    return nullptr;
  MTF.Update(index);
  // now try to output the decompressed stream
  if (!input.Seek(initial_position))
    return nullptr;
  Streams::HybridStream* output = manager.Allocate(data->uncompressed_length);
  if (output == nullptr)
    return output;
  if (!Apply(input, *output, info)) {
    manager.Delete(output);
    return nullptr;
  }
  else
    return output;
}

bool DeflateTransform::Apply(Streams::Stream& input, Streams::Stream& output, void* info) {
  if (info == nullptr)
    return false;
  Structures::DeflateInfo* data = reinterpret_cast<Structures::DeflateInfo*>(info);
  z_stream stream;
  zLib::SetupStream(&stream);
  int ret = zLib::InflateInit(&stream, data->zLib.parameters);
  if (ret != Z_OK)
    return false;
  for (std::int64_t i = 0; i < data->compressed_length; i += zLib::BLOCK_SIZEi64) {
    std::size_t block_size = static_cast<std::size_t>(std::min<std::int64_t>(data->compressed_length - i, zLib::BLOCK_SIZEi64));
    if (input.Read(&input_block[0], block_size) != block_size)
      break;
    stream.next_in  = &input_block[0];
    stream.avail_in = static_cast<uInt>(block_size);
    do {
      stream.next_out = &output_block[0];
      stream.avail_out = static_cast<uInt>(zLib::BLOCK_SIZE);
      ret = inflate(&stream, Z_FINISH);
      std::size_t const count = zLib::BLOCK_SIZE - static_cast<std::size_t>(stream.avail_out);
      if (output.Write(&output_block[0], count) != count){
        inflateEnd(&stream);
        return false;
      }
    } while ((stream.avail_out == 0) && (ret == Z_BUF_ERROR));
    if ((ret != Z_BUF_ERROR) && (ret != Z_STREAM_END))
      break;
  }
  inflateEnd(&stream);
  return (ret == Z_STREAM_END);
}

bool DeflateTransform::Undo(Streams::Stream& input, Streams::Stream& output, void* info) {
  if (info == nullptr)
    return false;
  Structures::DeflateInfo* data = reinterpret_cast<Structures::DeflateInfo*>(info);
  differ_positions[0] = -1LL;
  for (std::size_t i = 0; i < data->penalty_bytes_count; i++)
    differ_positions[i+1] = data->differ_positions[i] + differ_positions[i] + 1;
  std::int64_t length = data->differ_positions[data->penalty_bytes_count] + differ_positions[data->penalty_bytes_count], position = 0;
  std::size_t diff_index = 1;
  z_stream stream;
  zLib::SetupStream(&stream);
  int ret = deflateInit2(&stream, (data->zLib.combination / 9) + 1, Z_DEFLATED, data->zLib.window - MAX_WBITS, (data->zLib.combination % 9) + 1, Z_DEFAULT_STRATEGY);
  if (ret != Z_OK)
    return false;
  for (std::int64_t i = 0; i < data->uncompressed_length; i += zLib::BLOCK_SIZEi64) {
    std::size_t block_size = static_cast<std::size_t>(std::min<std::int64_t>(data->uncompressed_length - i, zLib::BLOCK_SIZEi64));
    if (input.Read(&input_block[0], block_size) != block_size)
      break;
    stream.next_in  = &input_block[0];
    stream.avail_in = static_cast<uInt>(block_size);
    do {
      stream.next_out = &output_block[0];
      stream.avail_out = static_cast<uInt>(zLib::BLOCK_SIZE);
      ret = deflate(&stream, ((i + static_cast<std::int64_t>(block_size)) == data->uncompressed_length) ? Z_FINISH : Z_NO_FLUSH);
      if ((ret != Z_BUF_ERROR) && (ret != Z_STREAM_END) && (ret != Z_OK))
        break;
      std::int64_t const have = std::min<std::int64_t>(zLib::BLOCK_SIZEi64 - static_cast<std::int64_t>(stream.avail_out), length - position);
      while ((diff_index <= data->penalty_bytes_count) && (differ_positions[diff_index] >= position) && (differ_positions[diff_index] < position + have)) {
        output_block[static_cast<std::size_t>(differ_positions[diff_index] - position)] = data->penalty_bytes[diff_index-1];
        diff_index++;
      }
      position += have;
      std::size_t const count = static_cast<std::size_t>(have);
      if (output.Write(&output_block[0], count) != count) {
        deflateEnd(&stream);
        return false;
      }
    } while (stream.avail_out == 0);
  }
  if (diff_index <= data->penalty_bytes_count){
    std::size_t const count = data->penalty_bytes_count - diff_index + 1;
    position += static_cast<std::int64_t>(count);
    if (output.Write(&data->penalty_bytes[diff_index-1], count) != count)
      position = 0;
  }
  deflateEnd(&stream);
  return (position == length);
}