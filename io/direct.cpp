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

#include "direct.hpp"

bool IO::Direct::Handler::Seek(std::int64_t const offset) {
  return stream->Seek(offset);
}

std::int64_t IO::Direct::Handler::Position() {
  return stream->Position();
}

std::int64_t IO::Direct::Handler::Size() {
  return stream->Size();
}

int IO::Direct::Reader::GetByte() {
  return stream->GetByte();
}

std::size_t IO::Direct::Reader::Read(void* buffer, std::size_t const count) {
  return stream->Read(buffer, count);
}

bool IO::Direct::Writer::PutByte(std::uint8_t const b) {
  return stream->PutByte(b);
}

std::size_t IO::Direct::Writer::Write(void* buffer, std::size_t const count) {
  return stream->Write(buffer, count);
}