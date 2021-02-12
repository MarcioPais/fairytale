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
#ifndef ANALYSER_HPP
#define ANALYSER_HPP

#include "common.hpp"
#include "structs.hpp"
#include "deduper.hpp"
#include "parsers/parser.hpp"
#include "parsers/deflateparser.hpp"
#include "parsers/jpegparser.hpp"
#include "parsers/bitmapparser.hpp"
#include "parsers/modparser.hpp"
#include <vector>

class Analyser {
private:
  std::vector<std::shared_ptr<Parser<Parsers::Types::Strict>>> strict;
  std::vector<std::shared_ptr<Parser<Parsers::Types::Fuzzy>>> fuzzy;
  Structures::ParsingData data;
public:
  explicit Analyser(const std::vector<std::pair<const Parsers::Names, const std::shared_ptr<void>>>& parsers);
  ~Analyser() = default;
  Analyser(const Analyser&) = delete;
  Analyser& operator=(const Analyser&) = delete;
  Analyser(Analyser&&) = delete;
  Analyser& operator=(Analyser&&) = delete;
  bool Process(Block& block, Storage::Manager& manager, Deduper* deduper = nullptr);
};

#endif  // ANALYSER_HPP