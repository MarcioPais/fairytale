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
#ifndef STREAM_HPP
#define STREAM_HPP

#include "../common.hpp"

namespace Streams {

  enum class Priority { High = 1, Normal, Low };

  class Stream {
  public:
    Stream() = default;
    Stream(const Stream&) = delete;
    Stream& operator=(const Stream&) = delete;
    Stream(Stream&&) = delete;
    Stream& operator=(Stream&&) = delete;
    virtual bool Seek(std::int64_t const offset) = 0;
    virtual std::int64_t Position() = 0;
    virtual std::int64_t Size() = 0;
    virtual bool PutByte(std::uint8_t const b) = 0;
    virtual int GetByte() = 0;
    virtual std::size_t Read(void* buffer, std::size_t const count) = 0;
    virtual std::size_t Write(void* buffer, std::size_t const count) = 0;
  };

}  // namespace Streams

#endif  // STREAM_HPP