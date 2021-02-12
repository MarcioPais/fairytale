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
#ifndef FACTORY_HPP
#define FACTORY_HPP

class Factory {
protected:
  // possible to construct
  template <typename T, typename... Args>
  static
  typename std::enable_if<
    std::is_constructible<T, Args...>::value,
    std::unique_ptr<T>
  >::type
    Create_(Args&&... args)
  {
    return std::unique_ptr<T>(new T(std::forward<Args>(args)...));
  }

  // impossible to construct
  template <typename T, typename... Args>
  static
  typename std::enable_if<
    !std::is_constructible<T, Args...>::value,
    std::unique_ptr<T>
  >::type
    Create_(Args&&...)
  {
    return std::unique_ptr<T>(nullptr);
  }
};

#endif  // FACTORY_HPP