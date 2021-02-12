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
#ifndef FILESTREAM_HPP
#define FILESTREAM_HPP

#include "stream.hpp"

namespace Streams {

  class FileStream : public Stream {
  protected:
    std::FILE* file;
    char* name;
  public:
    FileStream();
    ~FileStream();
    FileStream(const FileStream&) = delete;
    FileStream& operator=(const FileStream&) = delete;
    FileStream(FileStream&&) = delete;
    FileStream& operator=(FileStream&&) = delete;
    bool Open(char const* filename, char const* mode);
    void Close();
    bool Seek(std::int64_t const offset);
    std::int64_t Position();
    bool Dormant();
    bool WakeUp(char const* mode = "rb+");
    bool Sleep();
    std::int64_t Size();
    int GetByte();
    bool PutByte(std::uint8_t const b);
    std::size_t Read(void* buffer, std::size_t const count);
    std::size_t Write(void* buffer, std::size_t const count);
  };

}  // namespace Streams

#endif  // FILESTREAM_HPP