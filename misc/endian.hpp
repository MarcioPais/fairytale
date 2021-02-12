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
#ifndef ENDIAN_HPP
#define ENDIAN_HPP

#include "../common.hpp"

#if (defined(__has_include) && __has_include(<endian.h>)) || defined(LINUX)
#  include <endian.h>
#elif (defined(__has_include) && __has_include(<machine/endian.h>)) || defined(BSD)
#  include <machine/endian.h>
#elif (defined(__has_include) && __has_include(<libkern/OSByteOrder.h>)) || defined(__APPLE__)
#  include <libkern/OSByteOrder.h>
#elif (defined(__has_include) && __has_include(<sys/byteorder.h>)) || defined(SOLARIS)
#  include <sys/byteorder.h>
#elif defined(__has_include) && __has_include(<sys/param.h>)
#  include <sys/param.h>
#elif defined(__has_include) && __has_include(<sys/isadefs.h>)
#  include <sys/isadefs.h>
#endif

#ifndef __ORDER_BIG_ENDIAN__
#  define __ORDER_BIG_ENDIAN__ 4321
#endif
#ifndef __ORDER_LITTLE_ENDIAN__
#  define __ORDER_LITTLE_ENDIAN__ 1234
#endif
#ifndef __ORDER_PDP_ENDIAN__
#  define __ORDER_PDP_ENDIAN__ 3412
#endif

#if !defined(__BYTE_ORDER__) && defined(WINDOWS)
#  define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#  define IS_LITTLE_ENDIAN
#endif

#if !defined(IS_LITTLE_ENDIAN) && !defined(IS_BIG_ENDIAN) && !defined(IS_PDP_ENDIAN)
#  if (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_BIG_ENDIAN__) || \
      (defined(__BYTE_ORDER)   && __BYTE_ORDER   == __BIG_ENDIAN) || \
      (defined( _BYTE_ORDER)   &&  _BYTE_ORDER   ==  _BIG_ENDIAN) || \
      (defined(  BYTE_ORDER)   &&   BYTE_ORDER   ==   BIG_ENDIAN) || \
      defined(__BIG_ENDIAN__) || \
      defined(_BIG_ENDIAN) || \
      defined(__ARMEB__) || defined(__THUMBEB__) || defined(__AARCH64EB__) || \
      defined(_MIPSEB) || defined(__MIPSEB) || defined(__MIPSEB__) || \
      defined(ARCH_PPC)
#    define IS_BIG_ENDIAN
#  elif (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_LITTLE_ENDIAN__) || \
        (defined(__BYTE_ORDER)   && __BYTE_ORDER == __LITTLE_ENDIAN) || \
        (defined( _BYTE_ORDER)   &&  _BYTE_ORDER ==  _LITTLE_ENDIAN) || \
        (defined(  BYTE_ORDER)   &&   BYTE_ORDER ==   LITTLE_ENDIAN) || \
        defined(__LITTLE_ENDIAN__) || \
        defined(_LITTLE_ENDIAN) || \
        defined(__ARMEL__) || defined(__THUMBEL__) || defined(__AARCH64EL__) || defined(_M_ARM) || \
        defined(_MIPSEL) || defined(__MIPSEL) || defined(__MIPSEL__) || \
        defined(ARCH_X86) || defined(ARCH_ITANIUM)
#    define IS_LITTLE_ENDIAN
#  elif (defined(__BYTE_ORDER__) && __BYTE_ORDER__ == __ORDER_PDP_ENDIAN__) || \
        (defined(__BYTE_ORDER)   && __BYTE_ORDER   == __PDP_ENDIAN)
#    define IS_PDP_ENDIAN
#  endif
#endif

#ifndef __BYTE_ORDER__
#  ifdef IS_BIG_ENDIAN
#    define __BYTE_ORDER__ __ORDER_BIG_ENDIAN__
#  elif defined(IS_LITTLE_ENDIAN)
#    define __BYTE_ORDER__ __ORDER_LITTLE_ENDIAN__
#  elif defined(IS_PDP_ENDIAN)
#    define __BYTE_ORDER__ __ORDER_PDP_ENDIAN__
#  endif
#endif

enum class Endian {
  little = __ORDER_LITTLE_ENDIAN__,
  big    = __ORDER_BIG_ENDIAN__,
  native = __BYTE_ORDER__
};

#ifdef MSC
#  define bswap16(x) _byteswap_ushort((x))
#  define bswap32(x) _byteswap_ulong((x))
#  define bswap64(x) _byteswap_uint64((x))
#elif (__GNUC__ > 4) || (__GNUC__ == 4 && __GNUC_MINOR__ >= 8)
#  define bswap16(x) __builtin_bswap16((x))
#  define bswap32(x) __builtin_bswap32((x))
#  define bswap64(x) __builtin_bswap64((x))
#elif defined(__has_builtin) && __has_builtin(bswap16)
#  define bswap16(x) __builtin_bswap16((x))
#  define bswap32(x) __builtin_bswap32((x))
#  define bswap64(x) __builtin_bswap64((x))
#else
static ALWAYS_INLINE std::uint16_t bswap16(std::uint16_t x) {
  return (( x  >> 8 ) & 0xffu ) | (( x  & 0xffu ) << 8 );
}
static ALWAYS_INLINE std::uint32_t bswap32(std::uint32_t x) {
  return ((( x & 0xff000000u ) >> 24 ) |
          (( x & 0x00ff0000u ) >> 8  ) |
          (( x & 0x0000ff00u ) << 8  ) |
          (( x & 0x000000ffu ) << 24 ));
}
static ALWAYS_INLINE std::uint64_t bswap64(std::uint64_t x) {
  return ((( x & 0xff00000000000000ull ) >> 56 ) |
          (( x & 0x00ff000000000000ull ) >> 40 ) |
          (( x & 0x0000ff0000000000ull ) >> 24 ) |
          (( x & 0x000000ff00000000ull ) >> 8  ) |
          (( x & 0x00000000ff000000ull ) << 8  ) |
          (( x & 0x0000000000ff0000ull ) << 24 ) |
          (( x & 0x000000000000ff00ull ) << 40 ) |
          (( x & 0x00000000000000ffull ) << 56 ));
}
#endif

#ifdef IS_BIG_ENDIAN
// Big-Endian to/from Host conversions, 64 bit
#  define htobe64(x) (x)
#  define be64toh(x) (x)
#  define betoh64(x) (x)
// Big-Endian to/from Host conversions, 32 bit
#  define htobe32(x) (x)
#  define be32toh(x) (x)
#  define betoh32(x) (x)
// Big-Endian to/from Host conversions, 16 bit
#  define htobe16(x) (x)
#  define be16toh(x) (x)
#  define betoh16(x) (x)
// Little-Endian to/from Host conversions, 64 bit
#  define htole64(x) bswap64(x)
#  define le64toh(x) bswap64(x)
#  define letoh64(x) bswap64((x))
// Little-Endian to/from Host conversions, 32 bit
#  define htole32(x) bswap32(x)
#  define le32toh(x) bswap32(x)
#  define letoh32(x) bswap32((x))
// Little-Endian to/from Host conversions, 16 bit
#  define htole16(x) bswap16(x)
#  define le16toh(x) bswap16(x)
#  define letoh16(x) bswap16((x))
#elif defined(IS_LITTLE_ENDIAN)
// Big-Endian to/from Host conversions, 64 bit
#  define htobe64(x) bswap64((x))
#  define be64toh(x) bswap64((x))
#  define betoh64(x) bswap64((x))
// Big-Endian to/from Host conversions, 32 bit
#  define htobe32(x) bswap32((x))
#  define be32toh(x) bswap32((x))
#  define betoh32(x) bswap32((x))
// Big-Endian to/from Host conversions, 16 bit
#  define htobe16(x) bswap16((x))
#  define be16toh(x) bswap16((x))
#  define betoh16(x) bswap16((x))
// Little-Endian to/from Host conversions, 64 bit
#  define htole64(x) (x)
#  define le64toh(x) (x)
#  define letoh64(x) (x)
// Little-Endian to/from Host conversions, 32 bit
#  define htole32(x) (x)
#  define le32toh(x) (x)
#  define letoh32(x) (x)
// Little-Endian to/from Host conversions, 16 bit
#  define htole16(x) (x)
#  define le16toh(x) (x)
#  define letoh16(x) (x)
#elif defined(IS_PDP_ENDIAN)
  WARNING("Unsupported PDP endianness detected")
#else
  WARNING("Unknown platform endianness")
#endif

#endif  // ENDIAN_HPP