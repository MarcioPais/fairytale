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

#include "bitmapparser.hpp"
#include "../misc/endian.hpp"
#include "../misc/image.hpp"

BitmapParser::BitmapParser(const std::shared_ptr<void>& options) : headers{}, file_header_required(true) {
  if (options != nullptr)
    file_header_required = *std::static_pointer_cast<bool>(options);
  priority = Parsers::GetPriority(Parsers::Names::Bitmap);
}

bool BitmapParser::Parse(Block* block, Structures::ParsingData& data, Storage::Manager& manager) {
  UNUSED(manager);
  if (block == nullptr)
    return false;
  assert(!block->done);
  assert(block->type == Block::Type::Default);
  assert((block->data != nullptr) && ((block->level == 0) || reinterpret_cast<Streams::HybridStream*>(block->data)->Active()));
  std::int64_t i = 0; // current position relative to the initial block
  std::int64_t length = block->length;
  if (length < 256)
    return false;
  position = block->offset; // current position relative to the stream of the initial block
  if (!block->data->Seek(position))
    return false;
  std::uint64_t wnd[3] = {};  // sliding window, 24 bytes
  std::int64_t offset = 0;  // pixel data offset
  bool result = false, has_file_header = false, has_core_header = false;
  while (i < length) {
    std::size_t j = 0, bytes_read = block->data->Read(&buffer[0], buffer.size());
    while ((j < bytes_read) && (i < length)) {
      wnd[0] = (wnd[0] << 8) | (wnd[1] >> 56);
      wnd[1] = (wnd[1] << 8) | (wnd[2] >> 56);
      wnd[2] = (wnd[2] << 8) | buffer[j++];
      i++, position++;
      // Start of detection code
      headers.file.bfType = static_cast<std::uint16_t>(wnd[0] & 0xFFFF);
      has_file_header = (headers.file.bfType == Bitmap::SIGNATURE);
      headers.file.bfOffBits = (has_file_header ? betoh32(static_cast<std::uint32_t>(wnd[2] >> 32)) : 0);
      headers.info.biSize = betoh32(static_cast<std::uint32_t>(wnd[2]));
      has_core_header = (headers.info.biSize == Bitmap::Headers::BITMAPCOREHEADER);
      headers.info.biSizeImage = 0;
      // quick check for possible detection
      if ((
           (has_file_header && 
             (has_core_header ||
               (headers.info.biSize == Bitmap::Headers::BITMAPINFOHEADER  ) ||
               (headers.info.biSize == Bitmap::Headers::BITMAPV2INFOHEADER) ||
               (headers.info.biSize == Bitmap::Headers::BITMAPV3INFOHEADER) ||
               (headers.info.biSize == Bitmap::Headers::BITMAPV4INFOHEADER) ||
               (headers.info.biSize == Bitmap::Headers::BITMAPV5INFOHEADER)
             )
           ) || (!file_header_required && (headers.info.biSize == Bitmap::Headers::BITMAPINFOHEADER))
          ) && (
           headers.file.bfOffBits <= Bitmap::Headers::BITMAPFILEHEADER + 
                                     Bitmap::Headers::BITMAPV5INFOHEADER + 
                                     (!has_core_header ? 4 * 256 : 3 * 256)
          )
        )
      {
        if (!block->data->Seek(position))
          break;
        offset = (position - 4) + static_cast<std::int64_t>(!has_file_header ? Bitmap::Headers::BITMAPINFOHEADER : headers.file.bfOffBits - Bitmap::Headers::BITMAPFILEHEADER);
        headers.file.bfSize = betoh32(static_cast<std::uint32_t>(wnd[1] >> 32));
        // read and sanitize image width and height
        if (!has_core_header) {
          if ((
               (block->data->Read(&headers.info.biWidth, sizeof(headers.info.biWidth)) != sizeof(headers.info.biWidth)) ||
               ((headers.info.biWidth = static_cast<std::int32_t>(letoh32(headers.info.biWidth))) < 0) ||
                                                                         (headers.info.biWidth >= 0x800000)
              ) || (
               (block->data->Read(&headers.info.biHeight, sizeof(headers.info.biHeight)) != sizeof(headers.info.biHeight)) ||
               (std::abs((headers.info.biHeight = static_cast<std::int32_t>(letoh32(headers.info.biHeight)))) >= 0x800000)
              )
             )
               break;
        }
        else {
          if ((
               (block->data->Read(&headers.core.bcWidth, sizeof(headers.core.bcWidth)) != sizeof(headers.core.bcWidth)) ||
               (((headers.core.bcWidth = static_cast<std::int16_t>(letoh16(headers.core.bcWidth))) & 0xC000) != 0)
              ) || (
               (block->data->Read(&headers.core.bcHeight, sizeof(headers.core.bcHeight)) != sizeof(headers.core.bcHeight)) ||
               (((headers.core.bcHeight = static_cast<std::int16_t>(letoh16(headers.core.bcHeight))) & 0xC000) != 0)
              )
             )
               break;
        }
        // read and sanitize number of color planes
        if ((block->data->Read(&headers.info.biPlanes, sizeof(headers.info.biPlanes)) != sizeof(headers.info.biPlanes)) ||
            ((headers.info.biPlanes = letoh16(headers.info.biPlanes)) != 1))
          break;
        // read and sanitize number of bits per pixel
        if ((block->data->Read(&headers.info.biBitCount, sizeof(headers.info.biBitCount)) != sizeof(headers.info.biBitCount)) ||
            (((headers.info.biBitCount = letoh16(headers.info.biBitCount)) !=  1) &&
                                                (headers.info.biBitCount   !=  4) &&
                                                (headers.info.biBitCount   !=  8) &&
                                                (headers.info.biBitCount   != 24) &&
                                               ((headers.info.biBitCount   != 32) || has_core_header)
            )
           )
             break;
        std::uint32_t const max_palette_entries = 1UL << std::min<std::uint16_t>(headers.info.biBitCount, 8U);       
        if (headers.info.biSize > Bitmap::Headers::BITMAPCOREHEADER) {
          // read and sanitize the type of compression used
          if ((block->data->Read(&headers.info.biCompression, sizeof(headers.info.biCompression)) != sizeof(headers.info.biCompression)) ||
              (((headers.info.biCompression = letoh32(headers.info.biCompression)) != Bitmap::Compression::BI_RGB_) &&
                                                     (headers.info.biCompression   != Bitmap::Compression::BI_BITFIELDS_))
             )
               break;
          // read size of the image, to sanitize later
          if (block->data->Read(&headers.info.biSizeImage, sizeof(headers.info.biSizeImage)) != sizeof(headers.info.biSizeImage))
            break;
          // read horizontal and vertical resolution, ignored
          if ((block->data->Read(&headers.info.biXPelsPerMeter, sizeof(headers.info.biXPelsPerMeter)) != sizeof(headers.info.biXPelsPerMeter)) ||
              (block->data->Read(&headers.info.biYPelsPerMeter, sizeof(headers.info.biYPelsPerMeter)) != sizeof(headers.info.biYPelsPerMeter)))
            break;
          // read and sanitize number of colors used
          if ((block->data->Read(&headers.info.biClrUsed, sizeof(headers.info.biClrUsed)) != sizeof(headers.info.biClrUsed)) ||
              (((headers.info.biClrUsed = letoh32(headers.info.biClrUsed)) != 0) &&
               (headers.info.biClrUsed > max_palette_entries))
             )
               break;
          // read and sanitize number of important colors
          if ((block->data->Read(&headers.info.biClrImportant, sizeof(headers.info.biClrImportant)) != sizeof(headers.info.biClrImportant)) ||
              (((headers.info.biClrImportant = letoh32(headers.info.biClrImportant)) != 0) &&
               (headers.info.biClrImportant > max_palette_entries))
             )
               break;
          if (headers.info.biSize > Bitmap::Headers::BITMAPINFOHEADER) {
            // read and sanitize color masks
            if ((block->data->Read(&headers.info.bV2RedMask  , sizeof(headers.info.bV2RedMask  )) != sizeof(headers.info.bV2RedMask  )) ||
                (block->data->Read(&headers.info.bV2GreenMask, sizeof(headers.info.bV2GreenMask)) != sizeof(headers.info.bV2GreenMask)) ||
                (block->data->Read(&headers.info.bV2BlueMask , sizeof(headers.info.bV2BlueMask )) != sizeof(headers.info.bV2BlueMask )) ||
                ((headers.info.biCompression == Bitmap::Compression::BI_BITFIELDS_) &&
                 ((((headers.info.bV2RedMask   = letoh32(headers.info.bV2RedMask)) != 0x000000FF) &&
                                                        (headers.info.bV2RedMask   != 0x00FF0000)) ||
                   ((headers.info.bV2GreenMask = letoh32(headers.info.bV2GreenMask)) != 0x0000FF00) ||
                  (((headers.info.bV2BlueMask  = letoh32(headers.info.bV2BlueMask)) != 0x000000FF) &&
                                                        (headers.info.bV2BlueMask   != 0x00FF0000))
                 )
                )
               )
                 break;
            if (headers.info.biSize > Bitmap::Headers::BITMAPV2INFOHEADER) {
              // read and sanitize alpha mask
              if ((block->data->Read(&headers.info.bV3AlphaMask, sizeof(headers.info.bV3AlphaMask)) != sizeof(headers.info.bV3AlphaMask)) ||
                  ((headers.info.biCompression == Bitmap::Compression::BI_BITFIELDS_) &&
                   ((headers.info.bV3AlphaMask = letoh32(headers.info.bV3AlphaMask)) != 0xFF000000)
                  )
                 )
                   break;
              if (headers.info.biSize > Bitmap::Headers::BITMAPV3INFOHEADER) {
                // read and sanitize the color space type
                if ((block->data->Read(&headers.info.bV4CSType, sizeof(headers.info.bV4CSType)) != sizeof(headers.info.bV4CSType)) ||
                    (((headers.info.bV4CSType = letoh32(headers.info.bV4CSType)) != Bitmap::LogicalColorSpace::LCS_CALIBRATED_RGB_) &&
                     ((headers.info.biSize < Bitmap::Headers::BITMAPV5INFOHEADER) ||
                      ((headers.info.bV4CSType != Bitmap::LogicalColorSpace::LCS_sRGB_) &&
                       (headers.info.bV4CSType != Bitmap::LogicalColorSpace::LCS_WINDOWS_COLOR_SPACE_) &&
                       (headers.info.bV4CSType != Bitmap::LogicalColorSpace::PROFILE_EMBEDDED_) &&
                       (headers.info.bV4CSType != Bitmap::LogicalColorSpace::PROFILE_LINKED_)
                      )
                     )
                    )
                   )
                     break;
                // read endpoints and gamma, ignored
                if ((block->data->Read(&headers.info.bV4Endpoints, 0x24) != 0x24) ||
                    (block->data->Read(&headers.info.bV4GammaRed  , sizeof(headers.info.bV4GammaRed  )) != sizeof(headers.info.bV4GammaRed  )) ||
                    (block->data->Read(&headers.info.bV4GammaGreen, sizeof(headers.info.bV4GammaGreen)) != sizeof(headers.info.bV4GammaGreen)) ||
                    (block->data->Read(&headers.info.bV4GammaBlue , sizeof(headers.info.bV4GammaBlue )) != sizeof(headers.info.bV4GammaBlue )))
                  break;
                if (headers.info.biSize > Bitmap::Headers::BITMAPV4INFOHEADER) {
                  // read and sanitize rendering intent
                  if ((block->data->Read(&headers.info.bV5Intent, sizeof(headers.info.bV5Intent)) != sizeof(headers.info.bV5Intent)) ||
                      (((headers.info.bV5Intent = letoh32(headers.info.bV5Intent)) != Bitmap::GamutMappingIntent::LCS_GM_ABS_COLORIMETRIC_) &&
                                                         (headers.info.bV5Intent   != Bitmap::GamutMappingIntent::LCS_GM_BUSINESS_) &&
                                                         (headers.info.bV5Intent   != Bitmap::GamutMappingIntent::LCS_GM_GRAPHICS_) &&
                                                         (headers.info.bV5Intent   != Bitmap::GamutMappingIntent::LCS_GM_IMAGES_)
                      )
                     )
                       break;
                  // read and sanitize offset of profile data
                  if ((block->data->Read(&headers.info.bV5ProfileData, sizeof(headers.info.bV5ProfileData)) != sizeof(headers.info.bV5ProfileData)) ||
                      ((
                        (headers.info.bV4CSType == Bitmap::LogicalColorSpace::PROFILE_EMBEDDED_) ||
                        (headers.info.bV4CSType == Bitmap::LogicalColorSpace::PROFILE_LINKED_)
                       ) && (
                        (headers.file.bfOffBits > 0) &&
                        ((headers.info.bV5ProfileData = letoh32(headers.info.bV5ProfileData)) < headers.file.bfOffBits - Bitmap::Headers::BITMAPFILEHEADER)
                       )
                      )
                     )
                       break;
                  // read size of profile data, ignored
                  if (block->data->Read(&headers.info.bV5ProfileSize, sizeof(headers.info.bV5ProfileSize)) != sizeof(headers.info.bV5ProfileSize))
                    break;
                  // read and sanitize "reserved"
                  if ((block->data->Read(&headers.info.bV5Reserved, sizeof(headers.info.bV5Reserved)) != sizeof(headers.info.bV5Reserved)) ||
                      (headers.info.bV5Reserved > 0))
                    break;
                }  // BITMAPV5INFOHEADER
              }  // BITMAPV4INFOHEADER
            }  // BITMAPV3INFOHEADER
          }  // BITMAPV2INFOHEADER
        }  // BITMAPINFOHEADER

        data.image.bpp = static_cast<std::uint8_t>(headers.info.biBitCount);
        std::uint32_t const num_palette_entries = (data.image.bpp > 8) ?
          ((!has_core_header) ? headers.info.biClrUsed : 0) :
          (((!has_core_header) && (headers.info.biClrUsed > 0)) ? headers.info.biClrUsed : max_palette_entries);

        std::uint32_t const palette_size = num_palette_entries * (!has_core_header ? 4U : 3U);
        if (has_file_header && (headers.file.bfOffBits < (Bitmap::Headers::BITMAPFILEHEADER + headers.info.biSize + palette_size)))
          break;  // truncated headers or color table

        headers.info.biSizeImage = letoh32(headers.info.biSizeImage);
        std::int64_t const reported_size = static_cast<std::int64_t>(headers.info.biSizeImage);

        if (!has_core_header) {
          data.image.width = headers.info.biWidth;
          data.image.height = std::abs(headers.info.biHeight);
          // check for possible icon/cursor
          if (!has_file_header && (data.image.height == 2 * data.image.width) &&
            (
              // reported size matches a half-height image followed by a 1bpp AND mask?
              ((reported_size > 0) && (reported_size == ((static_cast<std::int64_t>(data.image.width) * data.image.height * (data.image.bpp + 1LL)) >> 4 ))) ||
              // no reported size, or size is smaller than expected, and dimensions match known icon/cursor dimensions?
              (((reported_size == 0) || (reported_size < ((static_cast<std::int64_t>(data.image.width) * data.image.height * data.image.bpp) >> 3 ))) &&
                (
                  (data.image.width ==   8) || (data.image.width ==  10) || (data.image.width == 14) || (data.image.width == 16) || (data.image.width == 20) ||
                  (data.image.width ==  22) || (data.image.width ==  24) || (data.image.width == 32) || (data.image.width == 40) || (data.image.width == 48) ||
                  (data.image.width ==  60) || (data.image.width ==  64) || (data.image.width == 72) || (data.image.width == 80) || (data.image.width == 96) ||
                  (data.image.width == 128) || (data.image.width == 256)
                )
              )
            )
          )
            data.image.height = data.image.width;

          // if it's a DIB (no file header), we must calculate the data offset based on the number of entries in the color palette
          if (!has_file_header)
            offset += 4LL * num_palette_entries;
        }
        else {  // BITMAPCOREHEADER
          data.image.width  = static_cast<std::int32_t>(headers.core.bcWidth);
          data.image.height = static_cast<std::int32_t>(headers.core.bcHeight);
        }
        data.image.stride = ((data.image.width * data.image.bpp + 31) & (~31)) >> 3;
        std::int64_t const actual_size = static_cast<std::int64_t>(data.image.stride) * data.image.height;

        if (
            // reported image size is smaller than expected
            ((reported_size > 0) && (reported_size < actual_size)) ||
            // report file size is smaller than expected
            (has_file_header && (static_cast<std::int64_t>(headers.file.bfSize) < (actual_size + Bitmap::Headers::BITMAPFILEHEADER + headers.info.biSize + palette_size))) ||
            // image is too small, segmentation overhead would likely be larger than compression gains of using specific image codecs
            (actual_size < 128)
        )
          break;

        if (data.image.bpp == 8) {
          // read color palette to see if image is grayscale
          if (!block->data->Seek(position - 4 + headers.info.biSize))
            break;
          data.image.grayscale = Image::HasGrayscalePalette(*block->data, num_palette_entries, !has_core_header);
        }

        std::int64_t const index = i + (offset - position);  // detection offset relative to the initial block
        if (index < length) {
          std::int64_t const size = std::min<std::int64_t>(actual_size, length - index);

          Block::Segmentation segmentation{};
          segmentation.offset = offset;
          segmentation.length = size;
          segmentation.type = Block::Type::Image;
          segmentation.info = &data.image;
          segmentation.size_of_info = sizeof(Structures::ImageInfo);
          block = block->Segment(segmentation);

          i += offset - position + size;
          position = offset + size;
          wnd[2] = wnd[1] = wnd[0] = 0;
          result = true;
        }
        break;
      }
      // End of detection code
    }
    if ((i < length) && !block->data->Seek(position))
      return result;
  }
  return result;
}