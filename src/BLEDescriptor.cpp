/*
  This file is part of the ArduinoBLE library.
  Copyright (c) 2018 Arduino SA. All rights reserved.

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

#include <stddef.h>

#include "local/BLELocalDescriptor.h"

#include "BLEDescriptor.h"

BLEDescriptor::BLEDescriptor() :
  BLEDescriptor(NULL)
{
}

BLEDescriptor::BLEDescriptor(BLELocalDescriptor* local) :
  _local(local)
{
  if (_local) {
    _local->retain();
  }
}

BLEDescriptor::BLEDescriptor(const char* uuid, const uint8_t value[], int valueSize) :
  BLEDescriptor(new BLELocalDescriptor(uuid, value, valueSize))
{
}

BLEDescriptor::BLEDescriptor(const char* uuid, const char* value) :
  BLEDescriptor(new BLELocalDescriptor(uuid, value))
{
}

BLEDescriptor::~BLEDescriptor()
{
  if (_local && _local->release() <= 0) {
    delete _local;
  }
}

int BLEDescriptor::valueSize() const
{
  if (_local) {
    return _local->valueSize();
  }

  return 0;
}

const uint8_t* BLEDescriptor::value() const
{
  if (_local) {
    return _local->value();
  }

  return NULL;
}

uint8_t BLEDescriptor::operator[] (int offset) const
{
  if (_local) {
    return (*_local)[offset];
  }

  return 0;
}

BLELocalDescriptor* BLEDescriptor::local()
{
  return _local;
}
