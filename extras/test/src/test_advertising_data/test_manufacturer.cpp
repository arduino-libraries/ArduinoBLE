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

TEST_CASE("Test manufacturer data setting", "[ArduinoBLE::BLEAdvertisingData]")
{
  BLEAdvertisingData advData;
  int oldRemainingLength = advData.remainingLength();
  uint16_t companyId = 0x1100;
  bool retVal;

  WHEN("Set correct manufacturer data without id")
  {
    const uint8_t data[] = {0x00, 0x11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    const uint8_t goldenData[] = {(sizeof(data) + 1), BLEFieldManufacturerData, 
                                  0x00, 0x11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    retVal = advData.setManufacturerData(data, sizeof(data));
    REQUIRE(retVal);
    REQUIRE( (sizeof(data) + 2) == (oldRemainingLength - advData.remainingLength()) );
    oldRemainingLength = advData.remainingLength();

    advData.updateData();
    REQUIRE(advData.dataLength() == sizeof(goldenData));
    REQUIRE( 0 == memcmp(goldenData, advData.data(), sizeof(goldenData)) );
  }

  WHEN("Set correct manufacturer data given a manufacturer id")
  {
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    const uint8_t goldenData[] = {(sizeof(data) + sizeof(companyId) + 1), BLEFieldManufacturerData, 
                                  0x00, 0x11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    retVal = advData.setManufacturerData(companyId, data, sizeof(data));
    REQUIRE(retVal);
    REQUIRE( (sizeof(data) + sizeof(companyId) + 2) == (oldRemainingLength - advData.remainingLength()) );
    oldRemainingLength = advData.remainingLength();

    advData.updateData();
    REQUIRE(advData.dataLength() == sizeof(goldenData));
    REQUIRE( 0 == memcmp(goldenData, advData.data(), sizeof(goldenData)) );
  }

  WHEN("Set too long manufacturer data given id")
  {
    // extreme case, 1 byte more than the maximum
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};
    retVal = advData.setManufacturerData(companyId, data, sizeof(data));
    REQUIRE(!retVal);
    REQUIRE( oldRemainingLength == advData.remainingLength() );
    advData.updateData();
    REQUIRE( advData.dataLength() == (MAX_AD_DATA_LENGTH - oldRemainingLength) );
  }

  WHEN("Set too long manufacturer data without id")
  {
    // extreme case, 1 byte more than the maximum
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28, 29};
    retVal = advData.setManufacturerData(data, sizeof(data));
    REQUIRE(!retVal);
    REQUIRE( oldRemainingLength == advData.remainingLength() );
    advData.updateData();
    REQUIRE( advData.dataLength() == (MAX_AD_DATA_LENGTH - oldRemainingLength) );
  }

  WHEN("Set manufacturer data without id after setting manufacturer data given id")
  {
    advData.clear();
    const uint8_t dataGivenId[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                                  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
    retVal = advData.setManufacturerData(companyId, dataGivenId, sizeof(dataGivenId));
    REQUIRE(retVal);
    retVal = advData.setManufacturerData(data, sizeof(data));
    REQUIRE(retVal);
    REQUIRE( 0 == advData.remainingLength() );
    advData.updateData();
    REQUIRE( advData.dataLength() == (MAX_AD_DATA_LENGTH) );
  }

  WHEN("Set manufacturer data given id after setting manufacturer data without id")
  {
    advData.clear();
    // dataGivenId is too long!! it should not pass
    const uint8_t dataGivenId[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                                  16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27, 28};
    retVal = advData.setManufacturerData(data, sizeof(data));
    REQUIRE(retVal);
    retVal = advData.setManufacturerData(companyId, dataGivenId, sizeof(dataGivenId));
    REQUIRE(!retVal);
  }

  WHEN("Overwrite manufacturer data with one as long as max data length")
  {
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
    const uint8_t goldenData[] = {(sizeof(data) + sizeof(companyId) + 1), BLEFieldManufacturerData, 
                                  0x00, 0x11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11,
                                  12, 13, 14, 15, 16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
    retVal = advData.setManufacturerData(companyId, data, sizeof(data));
    REQUIRE(retVal);

    advData.updateData();
    REQUIRE( 0 == advData.remainingLength() );
    REQUIRE( 0 == advData.availableForWrite() );
    REQUIRE( 0 == memcmp(goldenData, advData.data(), sizeof(goldenData)) );
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
