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

#define private public
#define protected public

#include "FakeBLELocalDevice.h"
#include "BLEAdvertisingData.h"

TEST_CASE("Test local name setting", "[ArduinoBLE::BLEAdvertisingData]")
{
  BLEAdvertisingData advData;
  int oldRemainingLength = advData.remainingLength();
  bool retVal;

  WHEN("Set correct local name")
  {
    const char* name = "test";
    retVal = advData.setLocalName(name);
    REQUIRE(retVal);
    REQUIRE( (strlen(name) + 2) == (oldRemainingLength - advData.remainingLength()) );
    oldRemainingLength = advData.remainingLength();

    advData.updateData();
    REQUIRE(advData.dataLength() == (strlen(name) + 2));
  }

  WHEN("Set too long local name")
  {
    const char* name = "way too long local name (len 32)";
    retVal = advData.setLocalName(name);
    REQUIRE(!retVal);
    REQUIRE( oldRemainingLength == advData.remainingLength() );
    advData.updateData();
    REQUIRE( advData.dataLength() == (MAX_AD_DATA_LENGTH - oldRemainingLength) );
  }

  WHEN("Overwrite local name with a name as long as max data length")
  {
    const char* name = "local name with full length  ";
    retVal = advData.setLocalName(name);
    REQUIRE(retVal);
    REQUIRE( (strlen(name) + 2) == (oldRemainingLength - advData.remainingLength()) );
    oldRemainingLength = advData.remainingLength();

    advData.updateData();
    REQUIRE(advData.dataLength() == (strlen(name) + 2));
    // advData should be full now
    REQUIRE( 0 == advData.remainingLength() );
    REQUIRE( 0 == advData.availableForWrite() );
  }

  WHEN("Check consistency when setting the external advertising data")
  {
    auto goldenData = advData.data();
    BLE.setAdvertisingData(advData);
    BLE.advertise();
    REQUIRE( 0 == memcmp(goldenData, BLE.getAdvertisingData().data(), advData.dataLength()) );
  }

  // Clear BLE advertising data
  advData.clear();
  BLE.setAdvertisingData(advData);
}
