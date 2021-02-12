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
#ifndef TRANSFORMFACTORY_HPP
#define TRANSFORMFACTORY_HPP

#include "../misc/factory.hpp"
#include "../block.hpp"
#include "deflatetransform.hpp"


class TransformFactory : public Factory {
public:
  template <class... Args>
  static std::unique_ptr<Transform> Create(Block::Type const type, Args&&... args) {
    switch (type) {
      case Block::Type::Deflate : return Create_<DeflateTransform>(std::forward<Args>(args)...);
      default: return nullptr;
    }
  }
};

#endif  // TRANSFORMFACTORY_HPP