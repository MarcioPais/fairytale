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
#ifndef CPU_X86_HPP
#define CPU_X86_HPP

#ifndef ARCH_X86
#  error Including cpu_x86.hpp on a non-x86 target
#endif

#include "misc/staticconstructor.hpp"
#include "cpu.hpp"

#if defined(GCC) || defined(CLANG)
#  include <cpuid.h>
#elif defined(MSC)
#  include <immintrin.h>
#  include <intrin.h>  // __cpuidex()
#endif

#define BIT_SET(a,b) (((a>>b)&1u) != 0u)
#define EXTRACT_BITS(a, i, c) ((a>>i)&((1u<<c)-1u))
#define X86_FEATURES \
/* feature name, leaf id, register, bit, required bool expression counterpart  */ \
  X86_GET(sse3,               1, ecx,  0, xmmOSXSave) \
  X86_GET(pclmulqdq,          1, ecx,  1,       true) \
  X86_GET(smx,                1, ecx,  6,       true) \
  X86_GET(ssse3,              1, ecx,  9, xmmOSXSave) \
  X86_GET(fma3,               1, ecx, 12, ymmOSXSave) \
  X86_GET(cx16,               1, ecx, 13,       true) \
  X86_GET(dca,                1, ecx, 18,       true) \
  X86_GET(sse4_1,             1, ecx, 19, xmmOSXSave) \
  X86_GET(sse4_2,             1, ecx, 20, xmmOSXSave) \
  X86_GET(x2apic,             1, ecx, 21,       true) \
  X86_GET(movbe,              1, ecx, 22,       true) \
  X86_GET(popcnt,             1, ecx, 23,       true) \
  X86_GET(aes,                1, ecx, 25,       true) \
  X86_GET(avx,                1, ecx, 28, ymmOSXSave) \
  X86_GET(f16c,               1, ecx, 29,       true) \
  X86_GET(rdrnd,              1, ecx, 30,       true) \
  X86_GET(fpu,                1, edx,  0,       true) \
  X86_GET(tsc,                1, edx,  4,       true) \
  X86_GET(cx8,                1, edx,  8,       true) \
  X86_GET(clfsh,              1, edx, 19,       true) \
  X86_GET(mmx,                1, edx, 23,       true) \
  X86_GET(fxsr,               1, edx, 24,       true) \
  X86_GET(sse,                1, edx, 25, xmmOSXSave) \
  X86_GET(sse2,               1, edx, 26, xmmOSXSave) \
  X86_GET(ss,                 1, edx, 27,       true) \
  X86_GET(sgx,                7, ebx,  2,       true) \
  X86_GET(bmi1,               7, ebx,  3,       true) \
  X86_GET(hle,                7, ebx,  4,       true) \
  X86_GET(avx2,               7, ebx,  5, ymmOSXSave) \
  X86_GET(bmi2,               7, ebx,  8,       true) \
  X86_GET(erms,               7, ebx,  9,       true) \
  X86_GET(rtm,                7, ebx, 11,       true) \
  X86_GET(avx512f,            7, ebx, 16, zmmOSXSave) \
  X86_GET(avx512dq,           7, ebx, 17, zmmOSXSave) \
  X86_GET(rdseed,             7, ebx, 18,       true) \
  X86_GET(avx512ifma,         7, ebx, 21, zmmOSXSave) \
  X86_GET(clflushopt,         7, ebx, 23,       true) \
  X86_GET(clwb,               7, ebx, 24,       true) \
  X86_GET(avx512pf,           7, ebx, 26, zmmOSXSave) \
  X86_GET(avx512er,           7, ebx, 27, zmmOSXSave) \
  X86_GET(avx512cd,           7, ebx, 28, zmmOSXSave) \
  X86_GET(sha,                7, ebx, 29,       true) \
  X86_GET(avx512bw,           7, ebx, 30, zmmOSXSave) \
  X86_GET(avx512vl,           7, ebx, 31, zmmOSXSave) \
  X86_GET(avx512vbmi,         7, ecx,  1, zmmOSXSave) \
  X86_GET(avx512vbmi2,        7, ecx,  6, zmmOSXSave) \
  X86_GET(vaes,               7, ecx,  9,       true) \
  X86_GET(vpclmulqdq,         7, ecx, 10,       true) \
  X86_GET(avx512vnni,         7, ecx, 11, zmmOSXSave) \
  X86_GET(avx512bitalg,       7, ecx, 12, zmmOSXSave) \
  X86_GET(avx512vpopcntdq,    7, ecx, 14, zmmOSXSave) \
  X86_GET(avx512_4vnniw,      7, edx,  2, zmmOSXSave) \
  X86_GET(avx512_4vbmi2,      7, edx,  3, zmmOSXSave) \
  X86_GET(avx512vp2intersect, 7, edx,  8, zmmOSXSave) \
  X86_GET(abm,             8x01, ecx,  5,  !is_intel) \
  X86_GET(sse4a,           8x01, ecx,  6,  !is_intel) \
  X86_GET(fma4,            8x01, ecx, 16,  !is_intel) \
  X86_GET(topoext,         8x01, ecx, 22,  !is_intel) \

namespace CPU {
  class x86 {
  public:
    static constexpr std::uint32_t VENDOR_LENGTH = 12u;
    static constexpr std::uint32_t NAME_LENGTH = 48u;

    typedef struct Leaf {
      std::uint32_t eax, ebx, ecx, edx;
    } Leaf;

#define X86_GET(a,b,c,d,e) bool a : 1;
    typedef struct Features {
      X86_FEATURES
    } Features;
#undef X86_GET

    typedef class Info {
    public:
      Features features;
      int family;
      int model;
      int stepping;
      char vendor[VENDOR_LENGTH + 1u];  // null-terminated string
      char name[NAME_LENGTH + 1u];      // null-terminated string
      void ReadVendor(Leaf const leaf) {
        char *p = reinterpret_cast<char*>(vendor);
        *reinterpret_cast<std::uint32_t*>(p   ) = leaf.ebx;
        *reinterpret_cast<std::uint32_t*>(p+4u) = leaf.edx;
        *reinterpret_cast<std::uint32_t*>(p+8u) = leaf.ecx;
        vendor[VENDOR_LENGTH] = '\0';
      }
      void ReadName(std::uint32_t const max_ext_leaf_id) {
        char *p = reinterpret_cast<char*>(name);
        for (std::uint32_t i=0u; i<=32u; i+=16u) {
          Leaf const leaf = CpuId(0x80000002u + (i>>4u), max_ext_leaf_id);
          *reinterpret_cast<std::uint32_t*>(p+i    ) = leaf.eax;
          *reinterpret_cast<std::uint32_t*>(p+i+ 4u) = leaf.ebx;
          *reinterpret_cast<std::uint32_t*>(p+i+ 8u) = leaf.ecx;
          *reinterpret_cast<std::uint32_t*>(p+i+12u) = leaf.edx;
        }
        name[NAME_LENGTH] = '\0';
      }
    } Info;
    static Info info;
    static Cores cores;

  private:
#if defined(GCC) || defined(CLANG)
    static Leaf CpuIdEx(std::uint32_t const leaf_id, int const ecx = 0) {
      Leaf leaf;
      __cpuid_count(leaf_id, ecx, leaf.eax, leaf.ebx, leaf.ecx, leaf.edx);
      return leaf;
    }
    static std::uint32_t GetXCR0Eax(void) {
      std::uint32_t eax, edx;
      __asm(".byte 0x0F, 0x01, 0xD0" : "=a"(eax), "=d"(edx):"c"(0));
      return eax;
    }
#elif defined(MSC)
#  ifdef CpuIdEx  // avoid conflict with intrin.h
#    undef CpuIdEx
#  endif
    static Leaf CpuIdEx(std::uint32_t const leaf_id, int const ecx = 0) {
      std::int32_t data[4];
      __cpuidex(data, static_cast<int>(leaf_id), ecx);
      return *reinterpret_cast<Leaf*>(&data);
    }
    static std::uint32_t GetXCR0Eax(void) { return static_cast<std::uint32_t>(_xgetbv(0)); }
#else
#  error Unsupported compiler
#endif
    static Leaf CpuId(std::uint32_t const leaf_id, std::uint32_t const max_leaf_id = 0) {
      return (leaf_id<=max_leaf_id) ? CpuIdEx(leaf_id, 0) : Leaf{};
    }
    static void ProcessExtendedTopology(std::uint32_t const leaf_id) {
      std::uint32_t threads_per_core = 1u;
      for (int level=0; ; ++level) {
        Leaf const leaf = CpuIdEx(leaf_id, level);
        if ((leaf.eax == 0u) && (leaf.ebx == 0u))
          break;
        std::uint32_t const level_type  = EXTRACT_BITS(leaf.ecx, 8u, 8u);
        std::uint32_t const level_value = EXTRACT_BITS(leaf.ebx, 0u, 16u);
        switch (level_type) {
          case 0x01u: threads_per_core = level_value; break;
          case 0x02u: cores.logical    = level_value; break;
          default: break;
        }
      }
      cores.physical = std::max<std::uint32_t>(1u, cores.logical / std::max<std::uint32_t>(1u, threads_per_core));
    }
    static void init() {
      (void)static_constructor<&x86::init>::run;
      enum Masks : std::uint32_t {
        XMM = 0x2,
        YMM = XMM|0x4,
        ZMM = YMM|0xE0
      };
      enum Signature : std::uint32_t {
        Intel    = 'n'|('t'<<8u)|('e'<<16u)|('l'<<24u),
        AMD      = 'c'|('A'<<8u)|('M'<<16u)|('D'<<24u),
        Hygon    = 'u'|('i'<<8u)|('n'<<16u)|('e'<<24u),
        Zhaoxin1 = 'a'|('u'<<8u)|('l'<<16u)|('s'<<24u),
        Zhaoxin2 = 'a'|('i'<<8u)|(' '<<16u)|(' '<<24u)
      };
      info = {}, cores = { 1u, 1u };

      Leaf const leaf0 = CpuId(0u);  // Highest Function Parameter and Manufacturer ID
      info.ReadVendor(leaf0);
      std::uint32_t const max_leaf_id = leaf0.eax;
      Leaf const leaf1 = CpuId(1u, max_leaf_id);  // Processor Info and Feature Bits
      Leaf const leaf7 = CpuId(7u, max_leaf_id);  // Extended Features
      Leaf const leaf8x00 = CpuIdEx(0x80000000u);  // Highest Extended Function Parameter
      std::uint32_t const max_ext_leaf_id = leaf8x00.eax;
      Leaf const leaf8x01 = CpuId(0x80000001u, max_ext_leaf_id);  // Extended Processor Info and Feature Bits
      info.ReadName(max_ext_leaf_id);  // Processor Brand String
      bool const is_intel   =  (leaf0.ecx == Signature::Intel);
      bool const is_amd     =  (leaf0.ecx == Signature::AMD);
      bool const is_hygon   =  (leaf0.ecx == Signature::Hygon);
      bool const is_zhaoxin = ((leaf0.ecx == Signature::Zhaoxin1) || (leaf0.ecx == Signature::Zhaoxin2));
      info.family = EXTRACT_BITS(leaf1.eax, 8u, 4u);
      info.model  = EXTRACT_BITS(leaf1.eax, 4u, 4u);
      info.stepping = leaf1.eax & 0xFu;
      if ((is_intel || is_amd || is_hygon) && info.family == 15u)
        info.family += EXTRACT_BITS(leaf1.eax, 20u, 8u);
      if ((is_intel && (info.family == 6u || info.family == 15u)) ||
          ((is_amd || is_hygon) && info.family == 15u) ||
          (is_zhaoxin && (info.family == 6u || info.family == 7u)))
        info.model |= EXTRACT_BITS(leaf1.eax, 16u, 4u)<<4u;

      // now detect cpu features
      // TODO: Improve SSE detection on non-AVX cpu's (no xgetbv)

      bool const osxsave = BIT_SET(leaf1.ecx, 27u);  // XSAVE enabled by OS
      bool const is_hyperthreading_capable = BIT_SET(leaf1.edx, 28u) /* htt */ && (!BIT_SET(leaf8x01.ecx, 1u)) /*cmp_legacy*/;
      std::uint32_t const xcr0_eax = (osxsave) ? GetXCR0Eax() : 0u;
#ifdef ARCH_X86_64
      bool const xmmOSXSave =  true;
#else
      bool xmmOSXSave =  false;
      if (osxsave)
        xmmOSXSave = (xcr0_eax & Masks::XMM) == Masks::XMM;
      else {
#  ifdef WINDOWS
        xmmOSXSave = IsProcessorFeaturePresent(PF_XMMI_INSTRUCTIONS_AVAILABLE);
#  elif defined(SYSCTLBYNAME)
        std::uint32_t sse = 0u;
        std::size_t size = sizeof(sse);
        if ((sysctlbyname("hw.optional.sse", &sse, &size, nullptr, 0) != 0) || sse == 0u) {
          size = sizeof(sse);
          if (sysctlbyname("hw.instruction_sse", &sse, &size, nullptr, 0) != 0)
            sse = 0u;
        }
        xmmOSXSave = sse > 0u;
#  elif defined(SYSCTL)
        std::uint32_t sse = 0u, mib[2] ={ CTL_MACHDEP, CPU_SSE };
        std::size_t size = sizeof(sse);
        if ((sysctl(mib, 2, &sse, &size, nullptr, 0) != 0) || sse == 0u) {
          size = sizeof(sse);
          mib[1] = CPU_SSE2;
          if (sysctl(mib, 2, &sse, &size, nullptr, 0) != 0)
            sse = 0u;
        }
        xmmOSXSave = sse > 0u;
#  endif
      }
#endif
      bool const ymmOSXSave = (xcr0_eax & Masks::YMM) == Masks::YMM;
      bool const zmmOSXSave = (xcr0_eax & Masks::ZMM) == Masks::ZMM;
#define X86_GET(a,b,c,d,e) info.features.a = (e) && BIT_SET(leaf##b.c, d);
      {X86_FEATURES}
#undef X86_GET

      // now detect core counts,
      // try to use CPUID as a baseline first
      if (is_intel && max_leaf_id >= 0x1Fu)
        ProcessExtendedTopology(0x1Fu);
      else if ((is_intel || is_amd || is_zhaoxin) && max_leaf_id >= 0x0Bu && info.features.x2apic)
        ProcessExtendedTopology(0x0Bu);
      else if (max_leaf_id >= 1u) {
        cores.logical = EXTRACT_BITS(leaf1.ebx, 16u, 8u);

        if (!is_amd && !is_hygon && max_leaf_id >= 4u) {
          std::uint32_t const eax = CpuIdEx(4u).eax;
          if ((eax & 0x1Fu) != 0u)
            cores.physical = 1u + EXTRACT_BITS(eax, 26u, 6u);
        }

        if (!is_intel && !is_zhaoxin && !info.features.x2apic && max_ext_leaf_id >= 8u) {
          std::uint32_t const ecx = CpuIdEx(0x80000008u).ecx;
          cores.physical = 1u<<EXTRACT_BITS(ecx, 12u, 4u);
          if (cores.physical <= 1u)
            cores.physical = 1u + EXTRACT_BITS(ecx, 0u, 8u);
        }

        if (is_hyperthreading_capable) {
          if (!(cores.physical > 1u)) {
            cores.physical = 1u;
            cores.logical = std::max<std::uint32_t>(cores.logical, 2u);
          }
        }
      }
      // and now try to get accurate OS-dependent info
      CPU::GetCoreCounts(cores.physical, cores.logical, 1u + static_cast<std::uint32_t>(is_hyperthreading_capable));
    }
  };

  x86::Info x86::info;
  Cores x86::cores;
}

#undef X86_FEATURES
#undef BIT_SET
#undef EXTRACT_BITS

#endif  // CPU_X86_HPP