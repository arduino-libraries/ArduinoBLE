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

#ifndef _BLE_ATTRIBUTE_H_
#define _BLE_ATTRIBUTE_H_

#include "BLEUuid.h"

enum BLEAttributeType {
  BLETypeUnknown        = 0x0000,

  BLETypeService        = 0x2800,
  BLETypeCharacteristic = 0x2803,
  BLETypeDescriptor     = 0x2900
};

class BLEAttribute
{
public:
  BLEAttribute(const char* uuid);
  virtual ~BLEAttribute();

  const char* uuid() const;
  const uint8_t* uuidData() const;
  uint8_t uuidLength() const;

  virtual enum BLEAttributeType type() const;

  int retain();
  int release();

private:
  BLEUuid               _uuid;
  int _refCount;
};

#endif
