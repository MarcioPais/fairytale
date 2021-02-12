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
#ifndef ARCHIVE_HPP
#define ARCHIVE_HPP

#include "../common.hpp"
#include "../misc/uleb128.hpp"
#include "../io/io.hpp"

namespace Archive {

  class Structure {
  public:
    template<class Container>
    class Iterable {
    public:
      using iterator = typename Container::iterator;
      using const_iterator = typename Container::const_iterator;
      inline iterator begin() noexcept { return data.begin(); }
      inline const_iterator cbegin() const noexcept { return data.cbegin(); }
      inline iterator end() noexcept { return data.end(); }
      inline const_iterator cend() const noexcept { return data.cend(); }
    protected:
      Container data;
    };
    template<class Container>
    class Indexable :
      public Archive::Structure::Iterable<Container>
    {
    public:
      auto operator [](
        typename std::enable_if<
          std::is_pointer<typename Container::value_type>::value ||
          is_smart_pointer<typename Container::value_type>::value,
          typename Container::size_type
        >::type idx
      ) -> decltype(this->data[0]) {
        return (idx < this->data.size()) ? this->data[idx] : nullptr;
      }
      auto size() -> decltype(this->data.size()) {
        return this->data.size();
      }
    };
    class IWritable {
    public:
      virtual ~IWritable() {};
      virtual bool Write(IO::IWriter& writer) = 0;
    };
    class IReadable {
    public:
      virtual ~IReadable() {};
      // reads the structure serialization; limited to range bytes, if range > 0
      // returns the number of bytes read
      virtual std::int64_t Read(IO::IReader& reader, std::int64_t const range = 0) = 0;
    };
  protected:
    ULEB128::int64 size;
    std::uint32_t checksum;
  };

  typedef struct Header {
    static constexpr std::size_t MAGIC_LENGTH = 3;
    static constexpr std::uint8_t MAGIC[MAGIC_LENGTH] = { 0x46, 0x54, 0x4C };
    std::uint8_t  magic[MAGIC_LENGTH];
    std::uint8_t  version;
    std::uint16_t flags;
    std::int64_t  size;
    std::uint8_t  checksum;
  } Header;

  namespace Internal {
    using MapKey = std::pair<std::int64_t, std::string>;
    struct MapHash {
      std::size_t operator () (MapKey const& pair) const {
        return std::hash<MapKey::first_type>()(pair.first) ^ std::hash<MapKey::second_type>()(pair.second);
      }
    };
  }

}  // namespace Archive

#endif  // ARCHIVE_HPP