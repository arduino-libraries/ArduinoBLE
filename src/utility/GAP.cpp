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

#include "BLEUuid.h"
#include "HCI.h"

#include "GAP.h"


GAPClass::GAPClass() :
  _advertising(false),
  _advertisedServiceUuid(NULL),
  _manufacturerData(NULL),
  _manufacturerDataLength(0),
  _localName(NULL),
  _advertisingInterval(160),
  _connectable(true),
  _serviceData(NULL),
  _serviceDataLength(0)
{
}

GAPClass::~GAPClass()
{
}

void GAPClass::setAdvertisedServiceUuid(const char* advertisedServiceUuid)
{
  _advertisedServiceUuid = advertisedServiceUuid;
}

void GAPClass::setManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength)
{
  _manufacturerData = manufacturerData;
  _manufacturerDataLength = manufacturerDataLength;
}

void GAPClass::setLocalName(const char *localName)
{
  _localName = localName;
}

bool GAPClass::advertising()
{
  return _advertising;
}

int GAPClass::advertise()
{
  uint8_t directBdaddr[6] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

  uint8_t type = (_connectable) ? 0x00 : (_localName ? 0x02 : 0x03);

  _advertising = false;

  if (HCI.leSetAdvertisingParameters(_advertisingInterval, _advertisingInterval, type, 0x00, 0x00, directBdaddr, 0x07, 0) != 0) {
    return 0;
  }

  uint8_t advertisingData[31];
  uint8_t advertisingDataLen = 0;

  advertisingData[0] = 0x02;
  advertisingData[1] = 0x01;
  advertisingData[2] = 0x06;
  advertisingDataLen += 3;

  if (_advertisedServiceUuid) {
    BLEUuid uuid(_advertisedServiceUuid);
    int uuidLen = uuid.length();

    advertisingData[3] = 1 + uuidLen;
    advertisingData[4] = (uuidLen > 2) ? 0x06 : 0x02;
    memcpy(&advertisingData[5], uuid.data(), uuidLen);

    advertisingDataLen += (2 + uuidLen);
  } else if (_manufacturerData && _manufacturerDataLength) {
    advertisingData[3] = 1 + _manufacturerDataLength;
    advertisingData[4] = 0xff;
    memcpy(&advertisingData[5], _manufacturerData, _manufacturerDataLength);

    advertisingDataLen += (2 + _manufacturerDataLength);
  }

  if (_serviceData && _serviceDataLength > 0 && advertisingDataLen >= (_serviceDataLength + 4)) {
    advertisingData[advertisingDataLen++] = _serviceDataLength + 3;
    advertisingData[advertisingDataLen++] = 0x16;

    memcpy(&advertisingData[advertisingDataLen], &_serviceDataUuid, sizeof(_serviceDataUuid));
    advertisingDataLen += sizeof(_serviceDataUuid);

    memcpy(&advertisingData[advertisingDataLen],_serviceData, _serviceDataLength);
    advertisingDataLen += _serviceDataLength;
  }

  if (HCI.leSetAdvertisingData(advertisingDataLen, advertisingData) != 0) {
    return 0;
  }

  uint8_t scanResponseData[31];
  uint8_t scanResponseDataLen = 0;

  if (_localName) {
    int localNameLen = strlen(_localName);

    if (localNameLen > 29) {
      localNameLen = 29;
      scanResponseData[1] = 0x08;
    } else {
      scanResponseData[1] = 0x09;
    }

    scanResponseData[0] = 1 + localNameLen;

    memcpy(&scanResponseData[2], _localName, localNameLen);

    scanResponseDataLen += (2 + localNameLen);
  }

  if (HCI.leSetScanResponseData(scanResponseDataLen, scanResponseData) != 0) {
    return 0;
  }

  if (HCI.leSetAdvertiseEnable(0x01) != 0) {
    return 0;
  }

  _advertising = false;

  return 1;
}

void GAPClass::stopAdvertise()
{
  _advertising = false;

  HCI.leSetAdvertiseEnable(0x00);
}

void GAPClass::setAdvertisingInterval(uint16_t advertisingInterval)
{
  _advertisingInterval = advertisingInterval;
}

void GAPClass::setConnectable(bool connectable)
{
  _connectable = connectable;
}

void GAPClass::setAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length)
{
  _serviceDataUuid = uuid;
  _serviceData = data;
  _serviceDataLength = length;
}

GAPClass GAP;
