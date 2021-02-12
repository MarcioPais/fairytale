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
#ifndef CODEC_HPP
#define CODEC_HPP

#include "archive.hpp"
#include <list>

#define CODEC_TABLE \
/* Name, Size (in bytes) of parameters data */ \
  CODEC_ROW(None, 0) \

namespace Archive {
  namespace Codec {

#define CODEC_ROW(a,b) a,
    enum class Names : std::size_t {
      CODEC_TABLE
      Count
    };
#undef CODEC_ROW

#define CODEC_ROW(a,b) b,
    static constexpr std::size_t ParameterSizes[static_cast<std::size_t>(Archive::Codec::Names::Count)] = {
      CODEC_TABLE
    };
#undef CODEC_ROW

    typedef struct Entry {
      Archive::Codec::Names id;
      void* parameters;
      Entry() = delete;
      explicit Entry(Archive::Codec::Names const id) : id(id), parameters(nullptr) {};
      Entry(Archive::Codec::Names const id, void* parameters) : id(id), parameters(parameters) {};
      ~Entry();
    } Entry;

    class Sequence :
      public Archive::Structure::Iterable<std::list<std::shared_ptr<Archive::Codec::Entry>>>,
      public Archive::Structure::IWritable,
      public Archive::Structure::IReadable
    {
    private:
      iterator Find(Archive::Codec::Names const id);
    public:
      bool Append(std::shared_ptr<Archive::Codec::Entry> entry);
      bool Remove(Archive::Codec::Names const id);
      std::int64_t Read(IO::IReader& reader, std::int64_t const range);
      bool Write(IO::IWriter& writer);
    };

    class Definitions :
      private Archive::Structure,
      public  Archive::Structure::Iterable<std::list<std::shared_ptr<Archive::Codec::Sequence>>>,
      public  Archive::Structure::IWritable,
      public  Archive::Structure::IReadable
    {
    public:
      std::int64_t Read(IO::IReader& reader, std::int64_t const range);
      bool Write(IO::IWriter& writer);
    };

  }  // namespace Codec
}  // namespace Archive

#endif  // CODEC_HPP