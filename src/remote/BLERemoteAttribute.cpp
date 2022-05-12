/*
  This file is part of the ArduinoBLE library.
  Copyright (c) 2019 Arduino SA. All rights reserved.

  This library is free software; you can redistribute it and/or
  modify it under the terms of the GNU Lesser General Public
  License as published by the Free Software Foundation; either
  version 2.1 of the License, or (at your option) any later version.

  This library is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the GNU
  Lesser General Public License for more details.

  You should have received a copy of the GNU Lesser General Public
  License along with this library; if not, write to the Free Software
  Foundation, Inc., 51 Franklin Street, Fifth Floor, Boston, MA  02110-1301  USA
*/

#include "utility/BLEUuid.h"

#include "BLERemoteAttribute.h"

std::map<BLERemoteAttribute*,int> BLERemoteAttribute::_refCount;

BLERemoteAttribute::BLERemoteAttribute(const uint8_t uuid[], uint8_t uuidLen) :
  _uuid(BLEUuid::uuidToString(uuid, uuidLen))
{
}

BLERemoteAttribute::~BLERemoteAttribute()
{
}

const char* BLERemoteAttribute::uuid() const
{
  return _uuid.c_str();
}

int BLERemoteAttribute::retain()
{
  _refCount[this]++;

  return _refCount[this];
}

int BLERemoteAttribute::release()
{
  _refCount[this]--;
  if (_refCount[this] == 0) _refCount.erase(this);

  return _refCount[this];
}
