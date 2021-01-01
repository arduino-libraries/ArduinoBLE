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
#include "BLEDevice.h"

TEST_CASE("BLE discovered device test", "[ArduinoBLE::BLEDevice]")
{

  WHEN("Retrieve local name from advertisement packet")
  {
    // Mocking advertisement packet
    uint8_t advType = 0x03;
    uint8_t eirLength = 6;
    uint8_t eirData[] = {0x05, 0x09, 't', 'e', 's', 't'};
    uint8_t rssi = 0;

    // Expected results
    String goldenName = "test";

    // Simulate device discovery
    BLEDevice device = BLEDevice();
    device.setAdvertisementData(0x03, eirLength, eirData, rssi); 

    bool hasName = device.hasLocalName();
    REQUIRE(hasName);

    String name = device.localName();
    REQUIRE(goldenName == name);

  }

}
