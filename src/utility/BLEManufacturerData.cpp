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

#include <stdlib.h>
#include <string.h>

#include "BLEManufacturerData.h"

const char* BLEManufacturerData::manufacturerDataToString(const uint8_t* data, uint8_t length)
{
  static char manufacturerData[32 * 2 + 1];
  char* c = manufacturerData;

  for (int i = 0; i < length - 1; i++) {
    uint8_t b = data[i];

    utoa(b >> 4, c++, 16);
    utoa(b & 0x0f, c++, 16);
  }

  *c = '\0';

  return manufacturerData;
}
