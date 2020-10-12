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

TEST_CASE("Test advertised service id setting", "[ArduinoBLE::BLEAdvertisingData]")
{
  BLEAdvertisingData advData;
  int oldRemainingLength = advData.remainingLength();
  bool retVal;

  WHEN("Set correct advertised service")
  {
    const char* service = "00112233445566770011223344556677";
    retVal = advData.setAdvertisedServiceUuid(service);
    REQUIRE(retVal);
    REQUIRE( (16 + 2) == (oldRemainingLength - advData.remainingLength()) );
    oldRemainingLength = advData.remainingLength();

    advData.updateData();
    REQUIRE(advData.dataLength() == (16 + 2));
  }

  WHEN("Set an advertised id too long with respect to the remaining length")
  {
    advData.clear();
    const char* name = "12 chars str";
    const char* service = "00112233445566770011223344556677";
    advData.setLocalName(name);
    oldRemainingLength = advData.remainingLength();
    retVal = advData.setAdvertisedServiceUuid(service);
    REQUIRE(!retVal);

    REQUIRE( oldRemainingLength == advData.remainingLength() );
    advData.updateData();
    REQUIRE( advData.dataLength() == (MAX_AD_DATA_LENGTH - oldRemainingLength) );
  }

  WHEN("Fill to maximum an advertising packet with a service id")
  {
    advData.clear();
    const char* name = "11 char str";
    const char* service = "00112233445566770011223344556677";
    advData.setLocalName(name);
    retVal = advData.setAdvertisedServiceUuid(service);
    REQUIRE(retVal);

    advData.updateData();
    REQUIRE(advData.dataLength() == (MAX_AD_DATA_LENGTH));
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

TEST_CASE("Test service data setting", "[ArduinoBLE::BLEAdvertisingData]")
{
  BLEAdvertisingData advData;
  int oldRemainingLength = advData.remainingLength();
  uint16_t uuid = 0x1100;
  bool retVal;

  WHEN("Set correct service data")
  {
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    const uint8_t goldenData[] = {(sizeof(data) + sizeof(uuid) + 1), BLEFieldServiceData, 
                                  0x00, 0x11, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9};
    retVal = advData.setAdvertisedServiceData(uuid, data, sizeof(data));
    REQUIRE(retVal);
    REQUIRE( (sizeof(data) + 2 + sizeof(uuid)) == (oldRemainingLength - advData.remainingLength()) );
    oldRemainingLength = advData.remainingLength();

    advData.updateData();
    REQUIRE(advData.dataLength() == (sizeof(data) + 2 + sizeof(uuid)));
    REQUIRE(advData.dataLength() == sizeof(goldenData));
    REQUIRE( 0 == memcmp(goldenData, advData.data(), sizeof(goldenData)) );
  }

  WHEN("Set too long service data")
  {
    // extreme case, 1 byte more than the maximum
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26, 27};
    retVal = advData.setAdvertisedServiceData(uuid, data, sizeof(data));
    REQUIRE(!retVal);
    REQUIRE( oldRemainingLength == advData.remainingLength() );
    advData.updateData();
    REQUIRE( advData.dataLength() == (MAX_AD_DATA_LENGTH - oldRemainingLength) );
  }

  WHEN("Overwrite service data with one as long as max data length")
  {
    const uint8_t data[] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15,
                            16, 17, 18, 19, 20, 21, 22, 23, 24, 25, 26};
    retVal = advData.setAdvertisedServiceData(uuid, data, sizeof(data));
    REQUIRE(retVal);

    advData.updateData();
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
