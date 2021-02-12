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
#ifndef COMMON_HPP
#define COMMON_HPP

#define ALLOW_UNSUPPORTED
#define _FILE_OFFSET_BITS 64

////////////////////////
// Compiler detection //
////////////////////////

#ifdef __clang__
#  ifndef CLANG
#    define CLANG
#  endif
#elif defined(__GNUG__)
#  ifndef GCC
#    define GCC
#  endif
#elif defined(_MSC_VER)
#  ifndef MSC
#    define MSC
#  endif
#elif !defined(ALLOW_UNSUPPORTED)
#  error Unsupported compiler
#endif

#ifdef MSC
#  define STRINGIFY(x) #x
#  define TOSTRING(x) STRINGIFY(x)
#  define PRAGMA(x) __pragma(x)
#  define INFO(info, msg) PRAGMA(message(__FILE__ "(" TOSTRING(__LINE__) "): " #info ": " #msg))
#  define MESSAGE(m) INFO(info, m)
#  define WARNING(w) INFO(warning, w)
#  define TODO(t) INFO(todo, t)
#elif defined(CLANG) || defined(GCC)
#  define PRAGMA(x) _Pragma(#x)
#  define INFO(info, msg) PRAGMA(#info " : " #msg)
#  define MESSAGE(m) INFO(info, m)
#  define WARNING(w) INFO(GCC warning, w)
#  define TODO(t) INFO(,"todo" t)
#endif

///////////////////////////
//  Target OS detection  //
///////////////////////////

#if (defined(_WIN64) || defined(_WIN32))
#  ifndef WINDOWS
#    define WINDOWS
#    include <windows.h>
#  endif
#else
#  ifdef __ANDROID__
#    ifndef ANDROID
#      define ANDROID
#    endif
#  endif
#  ifdef __linux__
#    ifndef LINUX
#      define LINUX
#    endif
#  endif
#  if (defined(unix) || defined(__unix__) || defined(__unix) || (defined(__APPLE__) && defined(__MACH__)))
#    include <sys/param.h> // BSD macro
#    ifndef UNIX
#      define UNIX
#    endif
#  endif
#  if defined(LINUX) || defined(UNIX)
#    include <unistd.h> // _POSIX_VERSION macro
#  endif
#  if defined(__sun) && defined(__SVR4)
#    ifndef SOLARIS
#      define SOLARIS
#    endif
#  endif
#endif
#if !(defined(WINDOWS) || defined(ANDROID) || defined(LINUX) || defined(UNIX) || defined(SOLARIS))
#  ifdef ALLOW_UNSUPPORTED
     WARNING("Unknown target system")
#  else
#    error Unknown target system
#  endif
#endif

/////////////////////////
// Target architecture //
/////////////////////////

#if defined(_M_IX86) || defined(__i386__)
#  define ARCH_X86
#  define ARCH_X86_32
#elif defined(_M_X64) || defined(__x86_64__)
#  define ARCH_X86
#  define ARCH_X86_64
#elif defined(__arm__) || defined(_M_ARM)
#  define ARCH_ARM
#  define ARCH_ARM32
#elif defined(__aarch64__)
#  define ARCH_ARM
#  define ARCH_ARM64
#elif defined(__mips64)
#  define ARCH_MIPS
#  define ARCH_MIPS64
#elif defined(__mips__)
#  define ARCH_MIPS
#  define ARCH_MIPS32
#elif defined(__powerpc__) || defined(__ppc__) || defined(__PPC__) || defined(_M_PPC)
#  define ARCH_PPC
#  define ARCH_PPC32
#elif defined(__powerpc64__) || defined(__ppc64__) || defined(__PPC64__) || defined(__64BIT__) || defined(_LP64) || defined(__LP64__)
#  define ARCH_PPC
#  define ARCH_PPC64
#elif defined(__sparc_v9__) || defined(__sparc_v9)
#  define ARCH_SPARC
#  define ARCH_SPARC64
#elif defined(__sparc__) || defined(__sparc)
#  define ARCH_SPARC
#  define ARCH_SPARC32
#elif defined(__ia64) || defined(__itanium__) || defined(_M_IA64)
#  define ARCH_ITANIUM
#endif

////////////////////////////////////////////
// Compile-time available features checks //
////////////////////////////////////////////

#ifdef ARCH_X86
#  define useSSE (defined(__SSE__) || (_M_IX86_FP >= 1))
#  define useSSE2 (defined(__SSE2__) || (_M_IX86_FP >= 2))
#  define useSSE3 defined(__SSE3__)
#  define useSSSE3 defined(__SSSE3__)
#  define useSSE4_1 defined(__SSE4_1__)
#  define useSSE4_2 defined(__SSE4_2__)
#  define useAVX defined(__AVX__)
#  define useFMA defined(__FMA__)
#  define useAVX2 defined(__AVX2__)
#  define useAVX512F defined(__AVX512F__)
#  define useAVX512CD defined(__AVX512CD__)
#  define useAVX512ER defined(__AVX512ER__)
#  define useAVX512PF defined(__AVX512PF__)
#  define useAVX512VL defined(__AVX512VL__)
#  define useAVX512DQ defined(__AVX512DQ__)
#  define useAVX512BW defined(__AVX512BW__)
#  define useAVX512IFMA defined(__AVX512IFMA__)
#  define useAVX512VBMI defined(__AVX512VBMI__)
#  define useAVX5124VNNIW defined(__AVX5124VNNIW__)
#  define useAVX5124FMAPS defined(__AVX5124FMAPS__)
#  define useAVX512VPOPCNTDQ defined(__AVX512VPOPCNTDQ__)
#  define useAVX512VNNI defined(__AVX512VNNI__)
#  define useAVX512VBMI2 defined(__AVX512VBMI2__)
#  define useAVX512BITALG defined(__AVX512BITALG__)
#  define useAVX512VP2INTERSECT defined(__AVX512VP2INTERSECT__)
#  define useAES defined(__AES__)
#  define useF16C defined(__F16C__)
#  define useBMI defined(__BMI__)
#  define useBMI2 defined(__BMI2__)
#elif defined(ARCH_ARM)
#  define NEON defined(__ARM_NEON__)
#elif defined(ARCH_MIPS)
#  define MSA defined(__mips_msa)
#endif

//////////////////////////////
// Compiler-specific macros //
//////////////////////////////

#ifdef MSC
#  ifndef ALWAYS_INLINE
#    define ALWAYS_INLINE __forceinline
#  endif
#  ifndef NOINLINE
#    define NOINLINE __declspec(noinline)
#  endif
#  ifndef ALIGNAS
#    define ALIGNAS(alignment) __declspec(align(alignment))
#  endif
#elif defined(GCC) || defined(__has_attribute)
#  if !defined(ALWAYS_INLINE) && (defined(GCC) || __has_attribute(always_inline))
#     define ALWAYS_INLINE inline __attribute__((always_inline))
#  endif
#  if !defined(NOINLINE) && (defined(GCC) || __has_attribute(noinline))
#    define NOINLINE __attribute__((noinline))
#  endif
#  if !defined(ALIGNAS) && (defined(GCC) || __has_attribute(aligned))
#    define ALIGNAS(alignment) __attribute__((aligned(alignment)))
#  endif
#else
#  ifndef ALWAYS_INLINE
#    define ALWAYS_INLINE inline
#  endif
#  ifndef NOINLINE
#    define NOINLINE
#  endif
#  ifndef ALIGNAS
#    define ALIGNAS(alignment) alignas(alignment)
#  endif
#endif

#if defined(GCC) || defined(CLANG)
#  ifndef UNLIKELY
#    define UNLIKELY(x) __builtin_expect(!!(x), 0)
#  endif
#  ifndef LIKELY
#    define LIKELY(x) __builtin_expect(!!(x), 1)
#  endif
#else
#  ifndef UNLIKELY
#    define UNLIKELY(x) (x)
#  endif
#  ifndef LIKELY
#    define LIKELY(x) (x)
#  endif
#endif

///////////////////////////////////
// Platform independent includes //
///////////////////////////////////

#include <cstdlib>
#include <cassert>
#include <cstddef>
#include <cstdint>
#include <cinttypes>
#include <climits>
#include <stdexcept>
#include <algorithm>
#include <memory>
#include <cstdio>
#include <iostream>
#include <tuple>

//////////////////////////////
// Platform specific macros //
//////////////////////////////

#ifdef __CYGWIN__
#  ifndef fseeko
#    define fseeko(a, b, c) fseek(a, b, c)
#  endif
#  ifndef ftello
#    define ftello(a) ftell(a)
#  endif
#elif defined(_WIN32)
#  ifndef fseeko
#    define fseeko(a, b, c) _fseeki64(a, b, c)
#  endif
#  ifndef ftello
#    define ftello(a) _ftelli64(a)
#  endif
#else
#  ifndef fseeko
#    define fseeko(a, b, c) fseeko64(a, b, c)
#  endif
#  ifndef ftello
#    define ftello(a) ftello64(a)
#  endif
#endif

//////////////////////////////
// Utility macros/constexpr //
//////////////////////////////

#define MEM_LIMIT(mem) (static_cast<std::size_t>(std::min<uint64_t>(static_cast<std::uint64_t>(mem), static_cast<std::uint64_t>(SIZE_MAX))))
#define IS_POWER_OF_2(x) ((x&(x-1)) == 0)
#define UNUSED(x) (void)(x)

static constexpr std::int64_t ULONG_MASK = static_cast<std::int64_t>(ULONG_MAX);

template<typename T, std::size_t N>
constexpr std::integral_constant<std::size_t, N> countof__(const T(&array)[N]) noexcept { return {}; }

template <typename... Args>
constexpr std::size_t countof(Args&&... args) { return decltype(countof__(std::get<0>(std::tuple<Args...>(args...))))::value; }

/////////////////
// Type traits //
/////////////////

template<typename T, typename enable = void>
struct is_smart_pointer : std::false_type {};

template<typename T>
struct is_smart_pointer<T, typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, std::shared_ptr<typename T::element_type>>::value>::type> : std::true_type {};

template<typename T>
struct is_smart_pointer<T, typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, std::unique_ptr<typename T::element_type>>::value>::type> : std::true_type {};

template<typename T>
struct is_smart_pointer<T, typename std::enable_if<std::is_same<typename std::remove_cv<T>::type, std::weak_ptr<typename T::element_type>>::value>::type> : std::true_type {};

#endif  // COMMON_HPP