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
#ifndef MTFLIST_HPP
#define MTFLIST_HPP

#include "../common.hpp"
#include <array>

template<std::size_t size>
class MTFList {
  static_assert(size > 0, "M.T.F. list size must be a positive integer");
  static_assert(size < SHRT_MAX, "M.T.F. list size must be smaller than SHRT_MAX");
private:
  std::array<std::pair<std::size_t, std::size_t>, size> list;
  std::size_t root, index;
public:
  MTFList() : root(0), index(0) {
    for (std::size_t i = 0; i < size; i++) {
      list[i].first = i - 1;
      list[i].second = i + 1;
    }
    list[size - 1].second = SIZE_MAX;
  }
  MTFList(const MTFList&) = delete;
  MTFList& operator=(const MTFList&) = delete;
  MTFList(MTFList&&) = delete;
  MTFList& operator=(MTFList&&) = delete;
  ALWAYS_INLINE std::size_t First() {
    index = root;
    return index;
  }
  ALWAYS_INLINE std::size_t Next() {
    if (index != SIZE_MAX)
      index = list[index].second;
    return index;
  }
  inline void Update(std::size_t const i) {
    index = i;
    if (index == root)
      return;
    std::size_t const previous = list[index].first;
    std::size_t const next = list[index].second;
    if (previous != SIZE_MAX)
      list[previous].second = next;
    if (next != SIZE_MAX)
      list[next].first = previous;
    list[root].first = index;
    list[index].second = root;
    root = index;
    list[root].first = SIZE_MAX;
  }
};

#endif  // MTFLIST_HPP