/*
  This file is part of the ArduinoBLE library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#if defined(ARDUINO_AVR_UNO_WIFI_REV2) || defined(ARDUINO_SAMD_MKRWIFI1010) || defined(ARDUINO_SAMD_NANO_33_IOT) || defined(TARGET_NANO_RP2040_CONNECT)
#include "HCINinaSpiTransport.h"
#include "spi_drv.h"

enum {
  BLE_BEGIN 			= 0x4A,
  BLE_END 				= 0x4B,
  BLE_AVAILABLE		= 0x4C,
  BLE_PEEK 				= 0x4D,
  BLE_READ 				= 0x4E,
  BLE_WRITE 			= 0x4F,
};

int BleDrv::bleBegin() {
    WAIT_FOR_SLAVE_SELECT();

    SpiDrv::sendCmd(BLE_BEGIN, PARAM_NUMS_0);

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint8_t len = 1;
    uint8_t result = 0;
    SpiDrv::waitResponseCmd(BLE_BEGIN, PARAM_NUMS_1, (uint8_t*)&result, &len);
    SpiDrv::spiSlaveDeselect();

    return result == 0;
}

void BleDrv::bleEnd() {
    WAIT_FOR_SLAVE_SELECT();

    SpiDrv::sendCmd(BLE_END, PARAM_NUMS_0);

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint8_t len = 1;
    uint8_t result = 0;
    SpiDrv::waitResponseCmd(BLE_END, PARAM_NUMS_1, (uint8_t*)&result, &len);
    SpiDrv::spiSlaveDeselect();
}

int BleDrv::bleAvailable() {
    WAIT_FOR_SLAVE_SELECT();
    uint16_t result = 0;

    SpiDrv::sendCmd(BLE_AVAILABLE, PARAM_NUMS_0);

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint8_t len = 2;
    SpiDrv::waitResponseCmd(BLE_AVAILABLE, PARAM_NUMS_1, (uint8_t*)&result, &len);
    SpiDrv::spiSlaveDeselect();

    return result;
}

int BleDrv::bleRead(uint8_t data[], size_t length) {
    WAIT_FOR_SLAVE_SELECT();

    SpiDrv::sendCmd(BLE_READ, PARAM_NUMS_1);

    int commandSize = 7; // 4 for the normal command length + 3 for the parameter
    uint16_t param = length; // TODO check length doesn't exceed 2^16
    SpiDrv::sendParam((uint8_t*)&param, sizeof(param), LAST_PARAM);

    // pad to multiple of 4
    while (commandSize % 4 != 0) {
        SpiDrv::readChar();
        commandSize++;
    }

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint16_t res_len = 0;
    SpiDrv::waitResponseData16(BLE_READ, data, (uint16_t*)&res_len);

    SpiDrv::spiSlaveDeselect();

    return res_len;
}

int BleDrv::blePeek(uint8_t data[], size_t length) {
    WAIT_FOR_SLAVE_SELECT();

    SpiDrv::sendCmd(BLE_PEEK, PARAM_NUMS_1);

    int commandSize = 7; // 4 for the normal command length + 3 for the parameter
    uint16_t param = length; // TODO check length doesn't exceed 2^16
    SpiDrv::sendParam((uint8_t*)&param, sizeof(param), LAST_PARAM);

    // pad to multiple of 4
    while (commandSize % 4 != 0) {
        SpiDrv::readChar();
        commandSize++;
    }

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint16_t res_len = 0;
    SpiDrv::waitResponseData16(BLE_READ, data, (uint16_t*)&res_len);

    SpiDrv::spiSlaveDeselect();

    return res_len;
}

size_t BleDrv::bleWrite(const uint8_t* data, size_t len) {
    WAIT_FOR_SLAVE_SELECT();

    int commandSize = 4;
    SpiDrv::sendCmd(BLE_WRITE, PARAM_NUMS_1);

    SpiDrv::sendBuffer((uint8_t*)data, len, LAST_PARAM);
    commandSize += len+2;

    // pad to multiple of 4
    while (commandSize % 4 != 0) {
        SpiDrv::readChar();
        commandSize++;
    }

    SpiDrv::spiSlaveDeselect();
    //Wait the reply elaboration
    SpiDrv::waitForSlaveReady();
    SpiDrv::spiSlaveSelect();

    uint8_t res_len = 1;
    uint16_t res = 0;
    SpiDrv::waitResponseCmd(BLE_WRITE, PARAM_NUMS_1, (uint8_t*)&res, &res_len);
    SpiDrv::spiSlaveDeselect();

    return res;
}

int HCINinaSpiTransportClass::begin()
{
  SpiDrv::begin();
  return BleDrv::bleBegin();
}

void HCINinaSpiTransportClass::end()
{
  BleDrv::bleEnd();
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
  return BleDrv::bleAvailable();
}

int HCINinaSpiTransportClass::peek()
{
  int res=-1;
  BleDrv::blePeek((uint8_t*)&res, 1); // read a single byte, if nothing is returned we return -1

  return res;
}

int HCINinaSpiTransportClass::read()
{
  int res=-1;
  BleDrv::bleRead((uint8_t*)&res, 1); // read a single byte, if nothing is returned we return -1

  return res;
}

size_t HCINinaSpiTransportClass::write(const uint8_t* data, size_t length)
{
  return BleDrv::bleWrite(data, length);
}

HCINinaSpiTransportClass HCINinaSpiTransport;
HCITransportInterface& HCITransport = HCINinaSpiTransport;

#endif
