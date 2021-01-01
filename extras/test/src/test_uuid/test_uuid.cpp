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

#include <catch.hpp>

#include "utility/BLEUuid.h"

TEST_CASE("BLE uuid test", "[ArduinoBLE::BLEUuid]")
{
  WHEN("Set and retrieve uuid")
  {
    bool verify;

    const char* goldenUuid = "19b10010-e8f2-537e-4f6c-d104768a1214";
    // little-endian order
    const uint8_t goldenData[] = {0x14,0x12,0x8A,0x76,0x04,0xD1,0x6C,0x4F,0x7E,0x53,0xF2,0xE8,0x10,0x00,0xB1,0x19};
    uint8_t goldenLength = 16;

    BLEUuid test(goldenUuid);

    uint8_t testLength = test.length();
    verify = (goldenLength == testLength);
    REQUIRE(verify);

    const uint8_t *testData = test.data();
    verify = ( 0 == (memcmp(goldenData, testData, sizeof(goldenData))) );
    REQUIRE(verify);

    const char *testUuid = test.uuidToString(testData, testLength);
    verify = ( 0 == strcmp(testUuid, goldenUuid) );
    REQUIRE(verify);

    // print the uuid
    WARN("test: " << testUuid << ",  golden: " << goldenUuid);
  }
}