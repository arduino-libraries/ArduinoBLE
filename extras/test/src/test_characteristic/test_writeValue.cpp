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


TEST_CASE("Test characteristic writeValue", "[ArduinoBLE::BLECharacteristic]")
{
  WHEN("Create a bool characteristic")
  {
    BLEBoolCharacteristic boolCharacteristic("Bool", BLERead | BLEIndicate | BLEWrite);
    bool v = false;;
    int written = boolCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(bool) );
  }

  WHEN("Create a boolean characteristic")
  {
    BLEBooleanCharacteristic booleanCharacteristic("Boolean", BLERead | BLEIndicate | BLEWrite);
    bool v = false;
    int written = booleanCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(bool) );
  }

  WHEN("Create a char characteristic")
  {
    BLECharCharacteristic charCharacteristic("Char", BLERead | BLEIndicate | BLEWrite);
    char v = 'a';
    int written = charCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(char) );
  }

  WHEN("Create a unsigned char characteristic")
  {
    BLEUnsignedCharCharacteristic unsignedCharCharacteristic("UnsignedChar", BLERead | BLEIndicate | BLEWrite);
    unsigned char v = 0x01;
    int written = unsignedCharCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned char) );
  }

  WHEN("Create a byte characteristic")
  {
    BLEByteCharacteristic byteCharacteristic("Byte", BLERead | BLEIndicate | BLEWrite);
    byte v = 0x01;
    int written = byteCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(byte) );
  }

  WHEN("Create a short characteristic")
  {
    BLEShortCharacteristic shortCharacteristic("Short", BLERead | BLEIndicate | BLEWrite);
    short v = -1;
    int written = shortCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(short) );
  }

  WHEN("Create a unsigned short characteristic")
  {
    BLEUnsignedShortCharacteristic unsignedShortCharacteristic("UnsignedShort", BLERead | BLEIndicate | BLEWrite);
    unsigned short v = 1;
    int written = unsignedShortCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned short) );
  }

  WHEN("Create a word characteristic")
  {
    BLEWordCharacteristic wordCharacteristic("Word", BLERead | BLEIndicate | BLEWrite);
    word v = -1;
    int written = wordCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(word) );
  }

  WHEN("Create a int characteristic")
  {
    BLEIntCharacteristic intCharacteristic("Int", BLERead | BLEIndicate | BLEWrite);
    int v = -1;
    int written = intCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(int) );
  }

  WHEN("Create a unsigned int characteristic")
  {
    BLEUnsignedIntCharacteristic unsignedIntCharacteristic("UnsignedInt", BLERead | BLEIndicate | BLEWrite);
    unsigned int v = 1;
    int written = unsignedIntCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned int) );
  }

  WHEN("Create a long characteristic")
  {
    BLELongCharacteristic longCharacteristic("Long", BLERead | BLEIndicate | BLEWrite);
    long v = -1;
    int written = longCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(long) );
  }

  WHEN("Create a unsigned long characteristic")
  {
    BLEUnsignedLongCharacteristic unsignedLongCharacteristic("UnsignedLong", BLERead | BLEIndicate | BLEWrite);
    unsigned long v = 1;
    int written = unsignedLongCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned long) );
  }

  WHEN("Create a float characteristic")
  {
    BLEFloatCharacteristic floatCharacteristic("Float", BLERead | BLEIndicate | BLEWrite);
    float v = -1.0f;
    int written = floatCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(float) );
  }

  WHEN("Create a double characteristic")
  {
    BLEDoubleCharacteristic doubleCharacteristic("Double", BLERead | BLEIndicate | BLEWrite);
    double v = -1.0;
    int written = doubleCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(double) );
  }

  WHEN("Create a string characteristic")
  {
    const int maxStringLength = 64;
    BLEStringCharacteristic stringCharacteristic("String", BLERead | BLEIndicate | BLEWrite, maxStringLength);
    const char* v = "Hello";
    int written = stringCharacteristic.writeValue(v);
    REQUIRE( written == min(strlen(v), maxStringLength) );
  }

  WHEN("Create a too long string characteristic")
  {
    const int maxStringLength = 4;
    BLEStringCharacteristic stringCharacteristic("String", BLERead | BLEIndicate | BLEWrite, maxStringLength);
    const char* v = "Hello";
    int written = stringCharacteristic.writeValue(v);
    REQUIRE( written == min(strlen(v), maxStringLength) );
  }

}
