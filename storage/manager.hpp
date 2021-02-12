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
#ifndef MANAGER_HPP
#define MANAGER_HPP

#include "storage.hpp"
#include "pool.hpp"
#include "../streams/filestream.hpp"
#include "../streams/hybridstream.hpp"
#include <unordered_set>

namespace Storage {

  class Manager final : public Storage::Holder {
  private:
    static constexpr std::size_t DEFAULT_BUCKET_COUNT = 4096;
    std::shared_ptr<Storage::Pool> pool;
    std::unordered_set<Streams::HybridStream*> streams;

    void Purge(std::int64_t const request);
  public:
    Manager(std::int64_t const hot_storage, std::int64_t const cold_storage);
    ~Manager();
    Manager(const Manager&) = delete;
    Manager& operator=(const Manager&) = delete;
    Manager(Manager&&) = delete;
    Manager& operator=(Manager&&) = delete;
    void Deallocate(Streams::HybridStream& stream);
    void Delete(Streams::HybridStream* stream);
    Streams::HybridStream* Allocate(std::int64_t size);
    void Reallocate(Streams::HybridStream& stream);
    const std::int64_t& available() const override;
  };

}  // namespace Storage

#endif  // MANAGER_HPP