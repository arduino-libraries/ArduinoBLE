/*
  This file is part of the ArduinoBLE library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#if defined(ARDUINO_AVR_UNO_WIFI_REV2) || defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(TARGET_NANO_RP2040_CONNECT)
#include "HCINinaSpiTransport.h"
#include <WiFiNINA.h>

int HCINinaSpiTransportClass::begin()
{
  WiFiDrv::wifiDriverInit();
  return WiFiDrv::bleBegin();
}

void HCINinaSpiTransportClass::end()
{
  WiFiDrv::bleEnd();
}

void HCINinaSpiTransportClass::wait(unsigned long timeout)
{
  for (unsigned long start = millis(); (millis() - start) < timeout;) {
    if (available()) {
      break;
    }
  }
}

int HCINinaSpiTransportClass::available()
{
  return WiFiDrv::bleAvailable();
}

int HCINinaSpiTransportClass::peek()
{
  int res=-1;
  WiFiDrv::blePeek((uint8_t*)&res, 1); // read a single byte, if nothing is returned we return -1

  return res;
}

int HCINinaSpiTransportClass::read()
{
  int res=-1;
  WiFiDrv::bleRead((uint8_t*)&res, 1); // read a single byte, if nothing is returned we return -1

  return res;
}

size_t HCINinaSpiTransportClass::write(const uint8_t* data, size_t length)
{
  return WiFiDrv::bleWrite(data, length);
}

HCINinaSpiTransportClass HCINinaSpiTransport;
HCITransportInterface& HCITransport = HCINinaSpiTransport;

#endif