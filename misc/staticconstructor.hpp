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
#ifndef STATICCONSTRUCTOR_HPP
#define STATICCONSTRUCTOR_HPP

template<void(*ctor)()>
struct static_constructor {
  struct constructor {
    constructor() { 
      ctor(); 
    }
  };
  static constructor run;
};

template <void(*ctor)()>
typename static_constructor<ctor>::constructor static_constructor<ctor>::run;

#endif  // STATICCONSTRUCTOR_HPP