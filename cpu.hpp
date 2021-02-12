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
#ifndef CPU_HPP
#define CPU_HPP

#include "common.hpp"
#ifdef WINDOWS
#  include <windows.h>
#elif (!defined(__has_include) && defined(BSD)) || (__has_include(<sys/types.h>) && __has_include(<sys/sysctl.h>))
#  define SYSCTL
#  ifndef __OPENBSD__
#    define SYSCTLBYNAME
#  endif
#  include <sys/types.h>
#  include <sys/sysctl.h>
#endif

namespace CPU {
  typedef struct Cores {
    std::uint32_t physical;
    std::uint32_t logical;
  } Cores;

  // estimate of the number of physical and logical cores in the system, some of which may not be available to us
  static void GetCoreCounts(uint32_t& physical, uint32_t& logical, uint32_t const max_threads_per_core = 1u) {
    std::uint32_t nb_physical_cores = 0u, nb_logical_cores = 0u;
#ifdef WINDOWS
    SYSTEM_INFO system_info = {};
    try {
      GetSystemInfo(&system_info);
      logical = system_info.dwNumberOfProcessors;
      physical = logical / std::max<uint32_t>(1u, max_threads_per_core);
    }
    catch (...) {}
    DWORD length = 0u;
    std::size_t offset = 0;
#  if _WIN32_WINNT >= 0x0601 // Win7+
    if (GetLogicalProcessorInformationEx(RelationProcessorCore, nullptr, &length) || length == 0u)
      return;
    std::unique_ptr<std::uint8_t[]> buffer(new std::uint8_t[length]);
    {
      auto const info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get());
      if (!GetLogicalProcessorInformationEx(RelationProcessorCore, info, &length))
        return;
    }
    do {
      auto const info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION_EX>(buffer.get() + offset);
      offset += info->Size;
      ++nb_physical_cores;
      for (WORD i=0u; i<info->Processor.GroupCount; ++i) {
        auto mask = info->Processor.GroupMask[i].Mask;
        for (; mask > 0u; mask&=mask-1)
          ++nb_logical_cores;
      }
    } while (offset < length);
#  elif _WIN32_WINNT >= 0x0501 // XP+
    if (GetLogicalProcessorInformation(nullptr, &length) || length == 0u)
      return;
    std::size_t count = length / sizeof(SYSTEM_LOGICAL_PROCESSOR_INFORMATION);
    std::unique_ptr<SYSTEM_LOGICAL_PROCESSOR_INFORMATION[]> buffer(new SYSTEM_LOGICAL_PROCESSOR_INFORMATION[count]);
    {
      auto const info = reinterpret_cast<PSYSTEM_LOGICAL_PROCESSOR_INFORMATION>(buffer.get());
      if (!GetLogicalProcessorInformation(info, &length))
        return;
    }
    for (; offset<count; ++offset) {
      if (buffer[offset].Relationship == RelationProcessorCore) {
        ++nb_physical_cores;
        auto mask = buffer[offset].ProcessorMask;
        for (; mask > 0u; mask&=mask-1)
          ++nb_logical_cores;
      }
    }
#  endif
#elif defined(_SC_NPROCESSORS_ONLN)
    nb_logical_cores = sysconf(_SC_NPROCESSORS_ONLN);
#elif defined(_SC_NPROC_ONLN)
    nb_logical_cores = sysconf(_SC_NPROC_ONLN);
#elif defined(SYSCTLBYNAME)
    std::size_t size = sizeof(nb_logical_cores);
    if ((sysctlbyname("hw.logicalcpu", &nb_logical_cores, &size, nullptr, 0) !=0 ) || (nb_logical_cores < 1u)) {
      size = sizeof(nb_logical_cores);
      if (sysctlbyname("hw.ncpu", &nb_logical_cores, &size, nullptr, 0) != 0)
        nb_logical_cores = 1u;
    }
    if (sysctlbyname("hw.physicalcpu", &nb_physical_cores, &size, nullptr, 0) != 0)
      nb_physical_cores = 1u;
#elif defined(SYSCTL)
    std::uint32_t mib[2] = { CTL_HW, HW_NCPU };
    std::size_t size = sizeof(nb_logical_cores);
    if (sysctl(mib, 2, &nb_logical_cores, &size, nullptr, 0) != 0)
      nb_physical_cores = 1u;
#endif
    logical  = std::max<uint32_t>({ 1u, logical , nb_logical_cores });
    physical = std::max<uint32_t>({ 1u, physical, nb_physical_cores });
  }
}

#endif  // CPU_HPP