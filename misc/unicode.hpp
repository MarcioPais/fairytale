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
#ifndef UNICODE_HPP
#define UNICODE_HPP

#include "../common.hpp"

#ifdef WINDOWS
inline std::wstring widen(char const *s) {
  auto const size = MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, nullptr, 0);
  if (size <= 0)
    return std::wstring();
  std::wstring res(static_cast<std::size_t>(size), 0);
  if (MultiByteToWideChar(CP_UTF8, MB_ERR_INVALID_CHARS, s, -1, &res[0], static_cast<int>(res.size())) == 0)
    return std::wstring();
  return res;
}
#endif

#endif  // UNICODE_HPP