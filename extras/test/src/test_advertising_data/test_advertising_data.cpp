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

TEST_CASE("Test flags override", "[ArduinoBLE::BLEAdvertisingData]")
{
  // Mocking advertisement packet
  BLEAdvertisingData advData;
  // Expected results
  uint8_t defaultData[] = {0x02, BLEFieldFlags, 0x06};
  uint8_t goldenFlags[] = {0x02, 0x01, BLEFlagsBREDRNotSupported};

  WHEN("Default options for flags")
  {
    BLE.advertise();
    REQUIRE( 0 == (memcmp(defaultData, BLE.getAdvertisingData().data(), sizeof(defaultData))) );
    REQUIRE( BLE.getScanResponseData().dataLength() == 0 );
  }

  WHEN("Setting external advertising data which has flags")
  {
    advData.setFlags(BLEFlagsBREDRNotSupported);
    advData.updateData();
    BLE.setAdvertisingData(advData);
    BLE.advertise();
    REQUIRE( 3 == BLE.getAdvertisingData().dataLength());
    REQUIRE( 0 == (memcmp(goldenFlags, advData.data(), sizeof(goldenFlags))) );
    REQUIRE( 0 == (memcmp(goldenFlags, BLE.getAdvertisingData().data(), sizeof(goldenFlags))) );
    REQUIRE( 0 != (memcmp(defaultData, BLE.getAdvertisingData().data(), sizeof(defaultData))) );
  }

  WHEN("Setting external advertising data without flags")
  {
    advData.clear();
    BLE.setAdvertisingData(advData);
    BLE.advertise();
    REQUIRE( 0 == (memcmp(defaultData, BLE.getAdvertisingData().data(), sizeof(defaultData))) );
  }

  // Clear BLE advertising data
  advData.clear();
  BLE.setAdvertisingData(advData);
}

TEST_CASE("Set default flags in an already full advertising data packet", "[ArduinoBLE::BLEAdvertisingData]")
{
    BLEAdvertisingData advData;
    bool retVal;

    const char* name = "tes";
    const uint8_t manufacturerData[24] = {0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                                          17, 18, 19, 20, 21, 22, 23};

    uint8_t defaultFlags[3] = {0x02, 0x01, 0x06}; 

    uint8_t goldenData[31] = { 
       (sizeof(manufacturerData) + 1), BLEFieldManufacturerData, 0, 1, 2, 3, 4, 5, 6, 7, 8, 9, 10, 11, 12, 13, 14, 15, 16,
                                                                17, 18, 19, 20, 21, 22, 23,
        ((uint8_t)(strlen(name) + 1)), BLEFieldCompleteLocalName, 't', 'e', 's'
    }; 

  WHEN("Test flags when advertising data is full")
  {
    retVal = advData.setLocalName("tes");
    retVal = advData.setManufacturerData(manufacturerData, sizeof(manufacturerData));

    BLE.setAdvertisingData(advData);
    BLE.advertise();
    REQUIRE(0 != memcmp(defaultFlags, BLE.getAdvertisingData().data(), sizeof(defaultFlags)));
    REQUIRE(0 == memcmp(goldenData, BLE.getAdvertisingData().data(), sizeof(goldenData)));
  }

  // Clear BLE advertising data
  advData.clear();
  BLE.setAdvertisingData(advData);
}

TEST_CASE("BLE overwrite a full internal advertising data with an external built one", "[ArduinoBLE::BLEAdvertisingData]")
{
  bool verify;
  BLEAdvertisingData advData;
  BLEAdvertisingData internalData;

  WHEN("Copy empty external advertising data")
  {
    BLE.setLocalName("test");
    BLE.setAdvertisedServiceUuid("1818");
    BLE.advertise();
    //WARN("data length: " << BLE.getAdvertisingData().dataLength());
    verify = (BLE.getAdvertisingData().dataLength() == (3 + (2+2)));
    REQUIRE(verify);
    verify = (BLE.getScanResponseData().dataLength() == (4+2));
    REQUIRE(verify);

    BLE.setAdvertisingData(advData);
    BLE.advertise();
    //WARN(BLE.getAdvertisingData().dataLength());
    verify = (BLE.getAdvertisingData().dataLength() == 3);
    REQUIRE(verify);

    BLE.setScanResponseData(advData);
    BLE.advertise();
    verify = (BLE.getScanResponseData().dataLength() == 0);
    REQUIRE(verify);
  }

  // Clear BLE advertising data
  advData.clear();
  BLE.setAdvertisingData(advData);
}

TEST_CASE("BLE test raw data", "[ArduinoBLE::BLEAdvertisingData]")
{
  BLEAdvertisingData advData;
  bool retVal;

  WHEN("Set too large raw data")
  {
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                      11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                      21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31, 32 };
    advData.setRawData(data, sizeof(data));
    REQUIRE(!retVal);
    advData.clear();
  }

  WHEN("Set correct raw data")
  {
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10,
                      11, 12, 13, 14, 15, 16, 17, 18, 19, 20,
                      21, 22, 23, 24, 25, 26, 27, 28, 29, 30, 31};
    retVal = advData.setRawData(data, sizeof(data));
    REQUIRE(retVal);
    advData.updateData();
    REQUIRE( 0 == memcmp(data, advData.data(), sizeof(data)) );
    advData.clear();
  }

  WHEN("Hide other parameters by setting raw data")
  {
    uint8_t data[] = {1, 2, 3, 4, 5, 6, 7, 8, 9, 10};
    advData.setLocalName("test");
    advData.setRawData(data, sizeof(data));

    advData.updateData();
    REQUIRE( 0 == memcmp(data, advData.data(), sizeof(data)) );

    advData.setLocalName("test");
    advData.updateData();
    REQUIRE( 0 == memcmp(data, advData.data(), sizeof(data)) );
    
    advData.clear();
    advData.setLocalName("test");
    advData.updateData();
    REQUIRE( 0 != memcmp(data, advData.data(), sizeof(data)) );
  }

  // Clear BLE advertising data
  advData.clear();
  BLE.setAdvertisingData(advData);
}
