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
#ifndef BITMAPPARSER_HPP
#define BITMAPPARSER_HPP

#include "parser.hpp"

namespace Bitmap {
  namespace Headers {
    enum : std::uint32_t {
      BITMAPCOREHEADER   =  12,
      BITMAPFILEHEADER   =  14,
      BITMAPINFOHEADER   =  40,
      BITMAPV2INFOHEADER =  52,
      BITMAPV3INFOHEADER =  56,
      BITMAPV4INFOHEADER = 108,
      BITMAPV5INFOHEADER = 124
    };
  }  // namespace Headers

  // Note: A bottom-up bitmap can be compressed, but a top-down bitmap cannot
  enum Compression : std::uint32_t {
    BI_RGB_,
    BI_RLE8_,
    BI_RLE4_,
    BI_BITFIELDS_,
    BI_JPEG_,
    BI_PNG_
  };

  enum LogicalColorSpace : std::uint32_t {
    LCS_CALIBRATED_RGB_      = 0x00000000,
    LCS_sRGB_                = 0x73524742,                     // "sRGB"
    LCS_WINDOWS_COLOR_SPACE_ = 0x57696E20,                     // "Win "
    PROFILE_LINKED_          = 0x4C494E4B,                     // "LINK"
    PROFILE_EMBEDDED_        = 0x4D424544                      // "MBED"
  };

  enum GamutMappingIntent : std::uint32_t {
    LCS_GM_BUSINESS_         = 0x00000001,
    LCS_GM_GRAPHICS_         = 0x00000002,
    LCS_GM_IMAGES_           = 0x00000004,
    LCS_GM_ABS_COLORIMETRIC_ = 0x00000008
  };

  static constexpr std::uint16_t SIGNATURE = 0x424D;           // "BM"

  typedef std::uint32_t FXPT2DOT30;
  typedef struct tagCIEXYZ {
    FXPT2DOT30 ciexyzX;
    FXPT2DOT30 ciexyzY;
    FXPT2DOT30 ciexyzZ;
  } CIEXYZ;
  typedef struct tagICEXYZTRIPLE {
    CIEXYZ ciexyzRed;
    CIEXYZ ciexyzGreen;
    CIEXYZ ciexyzBlue;
  } CIEXYZTRIPLE;

  typedef struct tagBITMAPFILEHEADER {
    std::uint16_t bfType;                                      // File type
    std::uint32_t bfSize;                                      // The size, in bytes, of the bitmap
    std::uint16_t bfReserved1;                                 // Reserved, should be set to zero
    std::uint16_t bfReserved2;                                 // Reserved, should be set to zero
    std::uint32_t bfOffBits;                                   // The offset, in bytes, from the beginning of the BITMAPFILEHEADER structure to the bitmap bits
  } BITMAPFILEHEADER;

  typedef struct tagBITMAPCOREHEADER {
    std::uint32_t bcSize = Bitmap::Headers::BITMAPCOREHEADER;  // Size of this header, in bytes
    std::int16_t  bcWidth;                                     // Image width, in pixels (unsigned if OS21XBITMAPHEADER)
    std::int16_t  bcHeight;                                    // Image height, in pixels (unsigned if OS21XBITMAPHEADER)
    std::uint16_t bcPlanes;                                    // Number of color planes
    std::uint16_t bcBitCount;                                  // Number of bits per pixel
  } BITMAPCOREHEADER;

  typedef struct tagBITMAPV5INFOHEADER {
    std::uint32_t biSize;                                      // Size of this header, in bytes
    std::int32_t  biWidth;                                     // Image width, in pixels
    std::int32_t  biHeight;                                    // Image height, in pixels
    std::uint16_t biPlanes;                                    // Number of color planes
    std::uint16_t biBitCount;                                  // Number of bits per pixel
    std::uint32_t biCompression;                               // Compression method used
    std::uint32_t biSizeImage;                                 // Size of the image, in bytes
    std::int32_t  biXPelsPerMeter;                             // Horizontal resolution, in pixels per meter
    std::int32_t  biYPelsPerMeter;                             // Vertical resolution, in pixels per meter
    std::uint32_t biClrUsed;                                   // Number of color indices in the color table that are actually used
    std::uint32_t biClrImportant;                              // Number of color indices that are considered important for displaying the bitmap
    // End of BITMAPINFOHEADER
    std::uint32_t bV2RedMask;                                  // Color mask that specifies the red component of each pixel
    std::uint32_t bV2GreenMask;                                // Color mask that specifies the green component of each pixel
    std::uint32_t bV2BlueMask;                                 // Color mask that specifies the blue component of each pixel
    // End of BITMAPV2INFOHEADER
    std::uint32_t bV3AlphaMask;                                // Color mask that specifies the alpha component of each pixel
    // End of BITMAPV3INFOHEADER
    std::uint32_t bV4CSType;                                   // Color space of the DIB
    CIEXYZTRIPLE  bV4Endpoints;                                // x, y, and z coordinates of the three colors that correspond to the red, green, and blue endpoints for the logical color space associated with the bitmap
    std::uint32_t bV4GammaRed;                                 // Toned response curve for red
    std::uint32_t bV4GammaGreen;                               // Toned response curve for green
    std::uint32_t bV4GammaBlue;                                // Toned response curve for blue
    // End of BITMAPV4INFOHEADER
    std::uint32_t bV5Intent;                                   // Rendering intent for bitmap
    std::uint32_t bV5ProfileData;                              // The offset, in bytes, from the beginning of the BITMAPV5HEADER structure to the start of the profile data
    std::uint32_t bV5ProfileSize;                              // Size, in bytes, of embedded profile data
    std::uint32_t bV5Reserved;                                 // Reserved, should be set to zero
  } BITMAPV5INFOHEADER;
}  // namespace Bitmap

class BitmapParser : public Parser<Parsers::Types::Strict> {
private:
  struct {
    Bitmap::BITMAPFILEHEADER file;
    Bitmap::BITMAPCOREHEADER core;
    Bitmap::BITMAPV5INFOHEADER info;
  } headers;
  bool file_header_required;
public:
  explicit BitmapParser(const std::shared_ptr<void>& options);
  bool Parse(Block* block, Structures::ParsingData& data, Storage::Manager& manager);
};

#endif  // BITMAPPARSER_HPP