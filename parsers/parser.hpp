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
#ifndef PARSER_HPP
#define PARSER_HPP

#include "../common.hpp"
#include "../structs.hpp"
#include "../block.hpp"
#include "../storage/storage.hpp"
#include "../storage/manager.hpp"

#define PARSER_TABLE \
/* Name, Priority */ \
  PARSER_ROW(Deflate, 0) \
  PARSER_ROW(Mod, 7) \
  PARSER_ROW(Bitmap, 8) \
  PARSER_ROW(JPEG, 9) \

namespace Parsers {

  enum class Types {
    Strict,
    Fuzzy
  };
  static constexpr Types All[] = { Types::Strict, Types::Fuzzy };

#define PARSER_ROW(a,b) a,
  enum class Names {
    PARSER_TABLE
    Count
  };
#undef PARSER_ROW

#define PARSER_ROW(a,b) {Parsers::Names::a, b},
  static constexpr std::pair<Parsers::Names, int> List[] = {
    PARSER_TABLE
  };
#undef PARSER_ROW
  using Names_type = std::underlying_type<Parsers::Names>::type;
  static constexpr int GetPriority(Parsers::Names name, Parsers::Names_type count = static_cast<Parsers::Names_type>(Parsers::Names::Count)) {
    return (count == 0) ? throw std::logic_error("Unrecognized parser") :
           (Parsers::List[count-1].first == name) ? Parsers::List[count-1].second :
           Parsers::GetPriority(name, count-1);
  }

}  // namespace Parsers

template<Parsers::Types type>
class Parser {
protected:
  Storage::Buffer buffer;
  std::int64_t position;
public:
  int priority;
  virtual bool Parse(Block* block, Structures::ParsingData& data, Storage::Manager& manager) = 0;
};

#endif  // PARSER_HPP