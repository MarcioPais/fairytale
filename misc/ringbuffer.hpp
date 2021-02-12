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
#ifndef RINGBUFFER_HPP
#define RINGBUFFER_HPP

#include "../common.hpp"
#include <array>

template<class T, std::size_t N>
class RingBuffer {
  static_assert((N > 0) && (IS_POWER_OF_2(N)), "RingBuffer: size must be a positive power of 2");
private:
  std::array<T, N> data;
  std::size_t offset;
  static constexpr std::size_t mask = N-1;
public:
  RingBuffer() : data{}, offset(0) {}
  ALWAYS_INLINE void Reset() {
    data.fill({});
    offset = 0;
  }
  ALWAYS_INLINE void Fill(const T& value) {
    data.fill(value);
  }
  ALWAYS_INLINE void Add(const T& value) {
    data[offset & mask] = value;
    offset++;
  }
  ALWAYS_INLINE std::size_t Position() {
    return offset;
  }
  T& operator[](const std::size_t i) {
    return data[i & mask];
  }
  const T& operator[](const std::size_t i) const {
    return data[i & mask];
  }
  T& operator()(const std::size_t i) {
    return data[(offset - i) & mask];
  }
  const T& operator()(const std::size_t i) const {
    return data[(offset - i) & mask];
  }
};

#endif  // RINGBUFFER_HPP