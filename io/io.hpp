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
#ifndef IO_HPP
#define IO_HPP

#include "../common.hpp"

namespace IO {

  class IHandler {
  public:
    virtual ~IHandler() {};
    virtual bool Seek(std::int64_t const offset) = 0;
    virtual std::int64_t Position() = 0;
    virtual std::int64_t Size() = 0;
  };

  class IReader :
    public virtual IO::IHandler
  {
  public:
    virtual ~IReader() {};
    virtual int GetByte() = 0;
    virtual std::size_t Read(void* buffer, std::size_t const count) = 0;
  };

  class IWriter :
    public virtual IO::IHandler
  {
  public:
    virtual ~IWriter() {};
    virtual bool PutByte(std::uint8_t const b) = 0;
    virtual std::size_t Write(void* buffer, std::size_t const count) = 0;
  };

}  // namespace IO

#endif  // IO_HPP