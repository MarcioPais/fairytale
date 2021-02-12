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
#ifndef DIRECT_HPP
#define DIRECT_HPP

#include "io.hpp"
#include "../streams/stream.hpp"

#ifdef MSC
#  pragma warning(push)
#  pragma warning(disable : 4250)  // inheritance via dominance
#endif

namespace IO {
  namespace Direct {

    class Handler : 
      public virtual IO::IHandler
    {
    protected:
      std::shared_ptr<Streams::Stream> stream;
    public:
      explicit Handler(std::shared_ptr<Streams::Stream> stream) : stream(std::move(stream)) {};
      Handler(const Handler&) = delete;
      Handler& operator=(const Handler&) = delete;
      Handler(Handler&&) = delete;
      Handler& operator=(Handler&&) = delete;
      bool Seek(std::int64_t const offset) override;
      std::int64_t Position() override;
      std::int64_t Size() override;
    };

    class Reader :
      public IO::Direct::Handler,
      public IO::IReader
    {
    public:
      explicit Reader(std::shared_ptr<Streams::Stream> stream) : Handler(stream) {};
      Reader(const Reader&) = delete;
      Reader& operator=(const Reader&) = delete;
      Reader(Reader&&) = delete;
      Reader& operator=(Reader&&) = delete;
      int GetByte();
      std::size_t Read(void* buffer, std::size_t const count);
    };

    class Writer :
      public IO::Direct::Handler,
      public IO::IWriter
    {
    public:
      explicit Writer(std::shared_ptr<Streams::Stream> stream) : Handler(stream) {};
      Writer(const Writer&) = delete;
      Writer& operator=(const Writer&) = delete;
      Writer(Writer&&) = delete;
      Writer& operator=(Writer&&) = delete;
      bool PutByte(std::uint8_t const b);
      std::size_t Write(void* buffer, std::size_t const count);
    };

  }  // namespace Direct
}  // namespace IO

#ifdef MSC
#  pragma warning(pop)
#endif

#endif  // DIRECT_HPP