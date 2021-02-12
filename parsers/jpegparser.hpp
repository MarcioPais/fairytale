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
#ifndef JPEGPARSER_HPP
#define JPEGPARSER_HPP

#include "parser.hpp"

namespace JPEG {
  enum Markers {
    SOF0 = 0xC0,
    SOF1 = 0xC1,
    SOF2 = 0xC2,
    DHT  = 0xC4,
    SOI  = 0xD8,
    EOI  = 0xD9,
    SOS  = 0xDA,
    DQT  = 0xDB
  };
}  // namespace JPEG

class JpegParser : public Parser<Parsers::Types::Strict> {
private:
  bool allow_progressive;
public:
  explicit JpegParser(const std::shared_ptr<void>& options);
  bool Parse(Block* block, Structures::ParsingData& data, Storage::Manager& manager);
};

#endif  // JPEGPARSER_HPP