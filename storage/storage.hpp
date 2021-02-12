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
#ifndef STORAGE_HPP
#define STORAGE_HPP

#include "../common.hpp"
#include <vector>
#include <array>
#ifndef MSC
#  include <cstring>
#endif
#if _WIN32_WINNT >= 0x0501 // XP+
#  include <io.h> // _get_osfhandle()
#endif

namespace Storage {

  enum class Type { Memory, Disk };
  enum class AllocationStrategy { None, Cold, Hot };

  class Exhausted : public std::exception {};
  class Corrupted : public std::exception {};

  static constexpr std::size_t BLOCK_SIZE = 4096;
  static_assert(IS_POWER_OF_2(Storage::BLOCK_SIZE) && (Storage::BLOCK_SIZE >= 512), "Storage block size must be a power of 2, at least 512 (bytes)");
  static constexpr std::int64_t BLOCK_SIZEi64 = static_cast<std::int64_t>(Storage::BLOCK_SIZE);
  static constexpr std::size_t BLOCK_MASK = Storage::BLOCK_SIZE-1;
  static constexpr std::int64_t BLOCK_MASKi64 = Storage::BLOCK_SIZEi64-1;
  constexpr std::int64_t RoundToBlockMultiple(const std::int64_t s) {
    return (s + Storage::BLOCK_MASKi64) & (~Storage::BLOCK_MASKi64);
  }

  typedef std::array<uint8_t, Storage::BLOCK_SIZE> Buffer;

  // A Block represents a single hybrid data storage block
  typedef struct Block {
    Storage::Type type;
    std::int64_t offset;
  } Block;

  // An Arena is a fixed size hybrid data storage structure
  typedef struct Arena {
    std::vector<Storage::Block*> blocks;  // storage for this arena
    std::int64_t position = 0;  // relative position in this arena
  } Arena;

  class Holder {
  protected:
    std::int64_t capacity_ = 0;
    std::int64_t available_ = 0;
  public:
    virtual const std::int64_t& capacity() const { return capacity_; }
    virtual const std::int64_t& available() const { return available_; }
  };

  inline std::FILE* GetTempFile(std::int64_t const size = 0) {
    std::FILE* file = nullptr;
#ifdef WINDOWS
    wchar_t szTempFileName[MAX_PATH]{};
    wchar_t lpTempPathBuffer[MAX_PATH+1]{};
    DWORD dwRetVal = GetTempPathW(MAX_PATH, lpTempPathBuffer);
    if ((dwRetVal > MAX_PATH) || (dwRetVal == 0))
      throw Storage::Exhausted();
    if (GetTempFileNameW(lpTempPathBuffer, L"tmp", 0, szTempFileName) == 0)
      throw Storage::Exhausted();
    if (_wfopen_s(&file, szTempFileName, L"w+bRTD") != 0)
      throw Storage::Exhausted();
#else
    file = tmpfile();
#endif
    if (file == nullptr)
      throw Storage::Exhausted();
    if (size > 0) {
      // attempt to pre-allocate the required physical storage
      if (fseeko(file, size-1, SEEK_SET) == 0) {
        if (fputc('\0', file) != EOF)
          fflush(file);
      }
      fseeko(file, 0, SEEK_END);
      std::int64_t allocated = ftello(file);
#if _WIN32_WINNT >= 0x0600 // Vista+
      if (allocated < size) {
        HANDLE handle = reinterpret_cast<HANDLE>(_get_osfhandle(_fileno(file)));
        FILE_ALLOCATION_INFO info;
        info.AllocationSize.QuadPart = static_cast<LONGLONG>(size);
        SetFileInformationByHandle(handle, FileAllocationInfo, &info, sizeof(info));
        if (fseeko(file, size, SEEK_SET) == 0)
          allocated = ftello(file);
      }
#endif
      if (allocated < size) {
        // pre-allocation failed, try actually writing to the file (much slower)
        Storage::Buffer const b{};
        fseeko(file, 0, SEEK_SET);
        allocated += static_cast<std::int64_t>(fwrite(&b, 1, Storage::BLOCK_SIZE-static_cast<std::size_t>(allocated & Storage::BLOCK_MASKi64), file));
        assert((allocated & Storage::BLOCK_MASKi64) == 0);
        while (allocated < size) {
          std::size_t bytes = fwrite(&b, 1, b.size(), file);
          allocated += static_cast<std::int64_t>(bytes);
          if (bytes != b.size())
            break;
        }
        if (allocated < size)
          throw Storage::Exhausted();
      }
    }
    return file;
  }

}  // namespace Storage

#endif  // STORAGE_HPP
