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
    BLEBoolCharacteristic boolCharacteristic("Bool", BLEBroadcast| BLEIndicate | BLENotify );
    bool v = false;;
    int written = boolCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(bool) );

    boolCharacteristic.broadcast();
    written = boolCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(bool) );

    BLEDevice device;
    boolCharacteristic.local()->writeCccdValue(device, 0x002);
    written = boolCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    boolCharacteristic.local()->writeCccdValue(device, 0x001);
    written = boolCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a boolean characteristic")
  {
    BLEBooleanCharacteristic booleanCharacteristic("Boolean", BLEBroadcast| BLEIndicate | BLENotify);
    bool v = false;
    int written = booleanCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(bool) );

    booleanCharacteristic.broadcast();
    written = booleanCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(bool) );

    BLEDevice device;
    booleanCharacteristic.local()->writeCccdValue(device, 0x002);
    written = booleanCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    booleanCharacteristic.local()->writeCccdValue(device, 0x001);
    written = booleanCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a char characteristic")
  {
    BLECharCharacteristic charCharacteristic("Char", BLEBroadcast| BLEIndicate | BLENotify);
    char v = 'a';
    int written = charCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(char) );

    charCharacteristic.broadcast();
    written = charCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(char) );

    BLEDevice device;
    charCharacteristic.local()->writeCccdValue(device, 0x002);
    written = charCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    charCharacteristic.local()->writeCccdValue(device, 0x001);
    written = charCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a unsigned char characteristic")
  {
    BLEUnsignedCharCharacteristic unsignedCharCharacteristic("UnsignedChar", BLEBroadcast| BLEIndicate | BLENotify);
    unsigned char v = 0x01;
    int written = unsignedCharCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned char) );

    unsignedCharCharacteristic.broadcast();
    written = unsignedCharCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned char) );

    BLEDevice device;
    unsignedCharCharacteristic.local()->writeCccdValue(device, 0x002);
    written = unsignedCharCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    unsignedCharCharacteristic.local()->writeCccdValue(device, 0x001);
    written = unsignedCharCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a byte characteristic")
  {
    BLEByteCharacteristic byteCharacteristic("Byte", BLEBroadcast| BLEIndicate | BLENotify);
    byte v = 0x01;
    int written = byteCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(byte) );

    byteCharacteristic.broadcast();
    written = byteCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(byte) );

    BLEDevice device;
    byteCharacteristic.local()->writeCccdValue(device, 0x002);
    written = byteCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    byteCharacteristic.local()->writeCccdValue(device, 0x001);
    written = byteCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a short characteristic")
  {
    BLEShortCharacteristic shortCharacteristic("Short", BLEBroadcast| BLEIndicate | BLENotify);
    short v = -1;
    int written = shortCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(short) );

    shortCharacteristic.broadcast();
    written = shortCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(short) );

    BLEDevice device;
    shortCharacteristic.local()->writeCccdValue(device, 0x002);
    written = shortCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    shortCharacteristic.local()->writeCccdValue(device, 0x001);
    written = shortCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a unsigned short characteristic")
  {
    BLEUnsignedShortCharacteristic unsignedShortCharacteristic("UnsignedShort", BLEBroadcast| BLEIndicate | BLENotify);
    unsigned short v = 1;
    int written = unsignedShortCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned short) );

    unsignedShortCharacteristic.broadcast();
    written = unsignedShortCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned short) );

    BLEDevice device;
    unsignedShortCharacteristic.local()->writeCccdValue(device, 0x002);
    written = unsignedShortCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    unsignedShortCharacteristic.local()->writeCccdValue(device, 0x001);
    written = unsignedShortCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a word characteristic")
  {
    BLEWordCharacteristic wordCharacteristic("Word", BLEBroadcast| BLEIndicate | BLENotify);
    word v = -1;
    int written = wordCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(word) );

    wordCharacteristic.broadcast();
    written = wordCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(word) );

    BLEDevice device;
    wordCharacteristic.local()->writeCccdValue(device, 0x002);
    written = wordCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    wordCharacteristic.local()->writeCccdValue(device, 0x001);
    written = wordCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a int characteristic")
  {
    BLEIntCharacteristic intCharacteristic("Int", BLEBroadcast| BLEIndicate | BLENotify);
    int v = -1;
    int written = intCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(int) );

    intCharacteristic.broadcast();
    written = intCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(int) );

    BLEDevice device;
    intCharacteristic.local()->writeCccdValue(device, 0x002);
    written = intCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    intCharacteristic.local()->writeCccdValue(device, 0x001);
    written = intCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a unsigned int characteristic")
  {
    BLEUnsignedIntCharacteristic unsignedIntCharacteristic("UnsignedInt", BLEBroadcast| BLEIndicate | BLENotify);
    unsigned int v = 1;
    int written = unsignedIntCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned int) );

    unsignedIntCharacteristic.broadcast();
    written = unsignedIntCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned int) );

    BLEDevice device;
    unsignedIntCharacteristic.local()->writeCccdValue(device, 0x002);
    written = unsignedIntCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    unsignedIntCharacteristic.local()->writeCccdValue(device, 0x001);
    written = unsignedIntCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a long characteristic")
  {
    BLELongCharacteristic longCharacteristic("Long", BLEBroadcast| BLEIndicate | BLENotify);
    long v = -1;
    int written = longCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(long) );

    longCharacteristic.broadcast();
    written = longCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(long) );

    BLEDevice device;
    longCharacteristic.local()->writeCccdValue(device, 0x002);
    written = longCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    longCharacteristic.local()->writeCccdValue(device, 0x001);
    written = longCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a unsigned long characteristic")
  {
    BLEUnsignedLongCharacteristic unsignedLongCharacteristic("UnsignedLong", BLEBroadcast| BLEIndicate | BLENotify);
    unsigned long v = 1;
    int written = unsignedLongCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned long) );

    unsignedLongCharacteristic.broadcast();
    written = unsignedLongCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(unsigned long) );

    BLEDevice device;
    unsignedLongCharacteristic.local()->writeCccdValue(device, 0x002);
    written = unsignedLongCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    unsignedLongCharacteristic.local()->writeCccdValue(device, 0x001);
    written = unsignedLongCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a float characteristic")
  {
    BLEFloatCharacteristic floatCharacteristic("Float", BLEBroadcast| BLEIndicate | BLENotify);
    float v = -1.0f;
    int written = floatCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(float) );

    floatCharacteristic.broadcast();
    written = floatCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(float) );

    BLEDevice device;
    floatCharacteristic.local()->writeCccdValue(device, 0x002);
    written = floatCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    floatCharacteristic.local()->writeCccdValue(device, 0x001);
    written = floatCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a double characteristic")
  {
    BLEDoubleCharacteristic doubleCharacteristic("Double", BLEBroadcast| BLEIndicate | BLENotify);
    double v = -1.0;
    int written = doubleCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(double) );

    doubleCharacteristic.broadcast();
    written = doubleCharacteristic.writeValue(v);
    REQUIRE( written == sizeof(double) );

    BLEDevice device;
    doubleCharacteristic.local()->writeCccdValue(device, 0x002);
    written = doubleCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    doubleCharacteristic.local()->writeCccdValue(device, 0x001);
    written = doubleCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a string characteristic")
  {
    const int maxStringLength = 64;
    BLEStringCharacteristic stringCharacteristic("String", BLEBroadcast| BLEIndicate | BLENotify, maxStringLength);
    const char* v = "Hello";
    int written = stringCharacteristic.writeValue(v);
    REQUIRE( written == min(strlen(v), maxStringLength) );

    stringCharacteristic.broadcast();
    written = stringCharacteristic.writeValue(v);
    REQUIRE( written == min(strlen(v), maxStringLength) );

    BLEDevice device;
    stringCharacteristic.local()->writeCccdValue(device, 0x002);
    written = stringCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    stringCharacteristic.local()->writeCccdValue(device, 0x001);
    written = stringCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

  WHEN("Create a too long string characteristic")
  {
    const int maxStringLength = 4;
    BLEStringCharacteristic stringCharacteristic("String", BLEBroadcast| BLEIndicate | BLENotify, maxStringLength);
    const char* v = "Hello";
    int written = stringCharacteristic.writeValue(v);
    REQUIRE( written == min(strlen(v), maxStringLength) );

    stringCharacteristic.broadcast();
    written = stringCharacteristic.writeValue(v);
    REQUIRE( written == min(strlen(v), maxStringLength) );

    BLEDevice device;
    stringCharacteristic.local()->writeCccdValue(device, 0x002);
    written = stringCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
    stringCharacteristic.local()->writeCccdValue(device, 0x001);
    written = stringCharacteristic.writeValue(v);
    /* No peers connected */
    REQUIRE( written == 0 );
  }

}
