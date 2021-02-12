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
#ifndef TRANSFORM_HPP
#define TRANSFORM_HPP

#include "../common.hpp"
#include "../storage/manager.hpp"

class Transform {
public:
  virtual Streams::HybridStream* Attempt(Streams::Stream& input, Storage::Manager& manager, void* info = nullptr) = 0;
  virtual bool Apply(Streams::Stream& input, Streams::Stream& output, void* info = nullptr) = 0;
  virtual bool Undo(Streams::Stream& input, Streams::Stream& output, void* info = nullptr) = 0;
};

#endif  // TRANSFORM_HPP