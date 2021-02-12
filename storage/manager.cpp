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

#include "manager.hpp"
#include "../streams/hybridstream.hpp"

namespace Storage {

  void Manager::Purge(std::int64_t const request) {
    std::vector<std::pair<std::int64_t, Streams::HybridStream*>> vect;
    vect.reserve(streams.size());
    for (auto stream : streams) {
      if (stream->keep_alive || (!stream->Active()))
        continue;
      std::int64_t const cost = (stream->capacity() / std::max<std::int64_t>(1LL, stream->reference_count)) * static_cast<std::int64_t>(stream->priority);
      vect.push_back(std::make_pair(cost, stream));
    }
    std::sort(vect.begin(), vect.end());
    for (auto iter = vect.rbegin(); (iter != vect.rend()) && (pool->available() < request); iter++)
      iter->second->Close();
  }

  Manager::Manager(std::int64_t const hot_storage, std::int64_t const cold_storage) :
    streams(Storage::Manager::DEFAULT_BUCKET_COUNT)
  {
    pool = std::shared_ptr<Storage::Pool>(new Storage::Pool(hot_storage, cold_storage));
    capacity_ = available_ = pool->capacity();
  }

  Manager::~Manager() {
    for (auto stream : streams) {
      stream->Close();
      delete stream;
    }
  }

  void Manager::Deallocate(Streams::HybridStream& stream) {
    if (streams.find(&stream) != streams.end())
      stream.Close();
  }

  void Manager::Delete(Streams::HybridStream* stream) {
    auto iter = streams.find(stream);
    if (iter != streams.end()) {
      stream->Close();
      delete stream;
      streams.erase(iter);
    }
  }

  Streams::HybridStream* Manager::Allocate(std::int64_t size) {
    size = Storage::RoundToBlockMultiple(size);
    if (size > pool->capacity())
      return nullptr;
    else if (size > pool->available()) {
      Purge(size);
      if (size > pool->available())
        return nullptr;
    }
    Streams::HybridStream* stream;
    try {
      stream = new Streams::HybridStream(size, pool);
    }
    catch (Storage::Exhausted const&) {
      return nullptr;
    }
    if (!streams.insert(stream).second) {
      // insertion failed, for some reason
      stream->Close();
      return nullptr;
    }
    return stream;
  }

  void Manager::Reallocate(Streams::HybridStream& stream) {
    if (streams.find(&stream) != streams.end()) {
      std::int64_t size = stream.capacity();
      if (size > pool->available()) {
        Purge(size);
        if (size > pool->available())
          return;
      }
      try {
        stream.Restore();
      }
      catch (Storage::Exhausted const&) {
        stream.Close();
      }
    }
  }

  const std::int64_t & Manager::available() const {
    return pool->available();
  }

}