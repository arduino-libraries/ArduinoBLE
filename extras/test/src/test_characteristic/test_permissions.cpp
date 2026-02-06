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

#include <catch2/catch_test_macros.hpp>

#define private public
#define protected public

#include "FakeBLELocalDevice.h"
#include "BLEAdvertisingData.h"
#include "BLETypedCharacteristics.h"
#include "BLELocalCharacteristic.h"
#include "BLEStringCharacteristic.h"
#include "BLEProperty.h"
#include <memory>

int property[] = {
  BLEBroadcast,
  BLERead,
  BLEWriteWithoutResponse,
  BLEWrite,
  BLENotify,
  BLEIndicate,
  BLEAuthSignedWrite,
  BLEExtProp,
  BLERead | BLEWrite | BLENotify
};

int permission[] = {
  BLEEncryption,
  BLEAuthentication,
  BLEAuthorization,
  BLEEncryption | BLEAuthentication
};

const char uuid[][31] = {
  "1 Bool",
  "2 Char",
  "3 UnsignedChar",
  "4 Byte",
  "5 Short",
  "6 UnsignedShort",
  "7 Word",
  "8 Int",
  "9 UnsignedInt",
  "A Long",
  "B UnsignedLong",
  "C Float",
  "D Double",
  "E String"
};

std::unique_ptr<BLECharacteristic> createCharacteristic(const char* uuid, unsigned int properties)
{
  switch(uuid[0])
  {
    case '1':
      return std::unique_ptr<BLECharacteristic>(new BLEBoolCharacteristic(uuid, properties));
    case '2':
      return std::unique_ptr<BLECharacteristic>(new BLECharCharacteristic(uuid, properties));
    case '3':
      return std::unique_ptr<BLECharacteristic>(new BLEUnsignedCharCharacteristic(uuid, properties));
    case '4':
      return std::unique_ptr<BLECharacteristic>(new BLEByteCharacteristic(uuid, properties));
    case '5':
      return std::unique_ptr<BLECharacteristic>(new BLEShortCharacteristic(uuid, properties));
    case '6':
      return std::unique_ptr<BLECharacteristic>(new BLEUnsignedShortCharacteristic(uuid, properties));
    case '7':
      return std::unique_ptr<BLECharacteristic>(new BLEWordCharacteristic(uuid, properties));
    case '8':
      return std::unique_ptr<BLECharacteristic>(new BLEIntCharacteristic(uuid, properties));
    case '9':
      return std::unique_ptr<BLECharacteristic>(new BLEUnsignedIntCharacteristic(uuid, properties));
    case 'A':
      return std::unique_ptr<BLECharacteristic>(new BLELongCharacteristic(uuid, properties));
    case 'B':
      return std::unique_ptr<BLECharacteristic>(new BLEUnsignedLongCharacteristic(uuid, properties));
    case 'C':
      return std::unique_ptr<BLECharacteristic>(new BLEFloatCharacteristic(uuid, properties));
    case 'D':
      return std::unique_ptr<BLECharacteristic>(new BLEDoubleCharacteristic(uuid, properties));
    case 'E':
      return std::unique_ptr<BLECharacteristic>(new BLEStringCharacteristic(uuid, properties, 2));
    default:
      break;
  }
  return nullptr;
}

TEST_CASE("Test characteristic properties and permissions", "[ArduinoBLE::BLECharacteristic]")
{
  WHEN("Create a characteristic")
  {
    for(int i = 0; i < sizeof(property)/sizeof(int); i++)
    {
      for(int j = 0; j < sizeof(permission)/sizeof(int); j++)
      {
        for(int k = 0; k < 14; k++)
        {
          std::unique_ptr<BLECharacteristic> ptr = createCharacteristic(uuid[k], property[i] | permission[j]);
          REQUIRE(ptr != nullptr);
          REQUIRE(ptr->properties() == (property[i]));
          BLELocalCharacteristic * local  = ptr->local();
          REQUIRE(local->permissions() == (permission[j] >> 8));
        }
      }
    }
  }
}
