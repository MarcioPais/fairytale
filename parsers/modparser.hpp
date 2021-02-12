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
#ifndef MODPARSER_HPP
#define MODPARSER_HPP

#include "parser.hpp"
#include "../misc/ringbuffer.hpp"

class ModParser : public Parser<Parsers::Types::Strict> {
private:
  static constexpr std::size_t WINDOW_SIZE = 0x800;
  static constexpr std::size_t SIGNATURE_END_OFFSET = 1084;
  RingBuffer<std::uint8_t, WINDOW_SIZE> wnd;
public:
  ModParser();
  bool Parse(Block* block, Structures::ParsingData& data, Storage::Manager& manager);
};

#endif  // MODPARSER_HPP