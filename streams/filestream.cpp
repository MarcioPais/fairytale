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

#include "filestream.hpp"
#include "../storage/storage.hpp"
#include "../misc/unicode.hpp"

namespace Streams {

  FileStream::FileStream() {
    file = nullptr;
    name = nullptr;
  }

  FileStream::~FileStream() {
    Close();
  }

  bool FileStream::Open(char const* filename, char const* mode) {
    if (file != nullptr)
      return false;
    if (name != nullptr)
      delete name;
    std::size_t const len = std::strlen(filename) + 1;
    name = new char[len]();
    std::memcpy(name, filename, len);
#ifdef WINDOWS
    return _wfopen_s(&file, widen(filename).c_str(), widen(mode).c_str()) == 0;
#else
    return (file = std::fopen(filename, mode)) != nullptr;
#endif
  }

  void FileStream::Close() {
    if (file != nullptr)
      fclose(file);
    if (name != nullptr)
      delete name;
    file = nullptr;
    name = nullptr;
  }

  bool FileStream::Seek(std::int64_t const offset) {
    if (file == nullptr)
      return false;
    return fseeko(file, offset, SEEK_SET) == 0;
  }

  std::int64_t FileStream::Position() {
    if (file == nullptr)
      return EOF;
    return ftello(file);
  }

  bool FileStream::Dormant() {
    return (file == nullptr) && (name != nullptr);
  }

  bool FileStream::WakeUp(char const* mode) {
    if (!Dormant())
      return true;
#ifdef WINDOWS
    return _wfopen_s(&file, widen(name).c_str(), widen(mode).c_str()) == 0;
#else
    return (file = std::fopen(name, mode)) != nullptr;
#endif
  }

  bool FileStream::Sleep() {
    if (Dormant())
      return true;
    if (file != nullptr) {
      fclose(file);
      file = nullptr;
      return true;
    }
    return false;
  }

  std::int64_t FileStream::Size() {
    if (file == nullptr)
      return -1LL;
    std::int64_t const offset = Position();
    if (fseeko(file, 0, SEEK_END) != 0)
      return -1LL;
    std::int64_t const size = Position();
    Seek(offset);
    return size;
  }

  int FileStream::GetByte() {
    if (file == nullptr)
      return EOF;
    return std::getc(file);
  }

  bool FileStream::PutByte(std::uint8_t const b) {
    if (file == nullptr)
      return false;
    return std::fputc(b, file) != EOF;
  }

  std::size_t FileStream::Read(void* buffer, std::size_t const count) {
    if (file == nullptr)
      return 0;
    return std::fread(buffer, 1, count, file);
  }

  std::size_t FileStream::Write(void* buffer, std::size_t const count) {
    if (file == nullptr)
      return 0;
    return std::fwrite(buffer, 1, count, file);
  }

}