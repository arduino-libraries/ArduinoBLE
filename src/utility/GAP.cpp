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

#define GAP_MAX_DISCOVERED_QUEUE_SIZE 5

GAPClass::GAPClass() :
  _advertising(false),
  _scanning(false),
  _advertisedServiceUuid(NULL),
  _manufacturerData(NULL),
  _manufacturerDataLength(0),
  _localName(NULL),
  _advertisingInterval(160),
  _connectable(true),
  _serviceData(NULL),
  _serviceDataLength(0),
  _discoverEventHandler(NULL)
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

void GAPClass::setManufacturerData(const uint16_t companyId, const uint8_t manufacturerData[], int manufacturerDataLength)
{
  uint8_t* tmpManufacturerData = (uint8_t*)malloc(manufacturerDataLength + 2);
  tmpManufacturerData[0] = companyId & 0xff;
  tmpManufacturerData[1] = companyId >> 8;
  memcpy(&tmpManufacturerData[2], manufacturerData, manufacturerDataLength);
  this->setManufacturerData(tmpManufacturerData, manufacturerDataLength + 2);
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

    advertisingData[advertisingDataLen++] = 1 + uuidLen;
    advertisingData[advertisingDataLen++] = (uuidLen > 2) ? 0x06 : 0x02;
    memcpy(&advertisingData[advertisingDataLen], uuid.data(), uuidLen);

    advertisingDataLen += uuidLen;
  } else if (_manufacturerData && _manufacturerDataLength) {
    advertisingData[advertisingDataLen++] = 1 + _manufacturerDataLength;
    advertisingData[advertisingDataLen++] = 0xff;
    memcpy(&advertisingData[advertisingDataLen], _manufacturerData, _manufacturerDataLength);

    advertisingDataLen += _manufacturerDataLength;
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

int GAPClass::scan(bool withDuplicates)
{
  HCI.leSetScanEnable(false, true);

  // active scan, 10 ms scan interval (N * 0.625), 10 ms scan window (N * 0.625), public own address type, no filter
  if (HCI.leSetScanParameters(0x01, 0x0010, 0x0010, 0x00, 0x00) != 0) {
    return false;
  }

  _scanning = true;

  if (HCI.leSetScanEnable(true, !withDuplicates) != 0) {
    return 0;
  }

  return 1;
}

int GAPClass::scanForName(String name, bool withDuplicates)
{
  _scanNameFilter    = name;
  _scanUuidFilter    = "";
  _scanAddressFilter = "";

  return scan(withDuplicates);
}

int GAPClass::scanForUuid(String uuid, bool withDuplicates)
{
  _scanNameFilter    = "";
  _scanUuidFilter    = uuid;
  _scanAddressFilter = "";

  return scan(withDuplicates);
}

int GAPClass::scanForAddress(String address, bool withDuplicates)
{
  _scanNameFilter    = "";
  _scanUuidFilter    = "";
  _scanAddressFilter = address;

  return scan(withDuplicates);
}

void GAPClass::stopScan()
{
  HCI.leSetScanEnable(false, false);

  _scanning = false;

  for (unsigned int i = 0; i < _discoveredDevices.size(); i++) {
    BLEDevice* device = _discoveredDevices.get(i);

    delete device;
  }

  _discoveredDevices.clear();
}

BLEDevice GAPClass::available()
{
  for (unsigned int i = 0; i < _discoveredDevices.size(); i++) {
    BLEDevice* device = _discoveredDevices.get(i);

    if (device->discovered()) {
      BLEDevice result = *device;

      _discoveredDevices.remove(i);
      delete device;

      if (matchesScanFilter(result)) {
        return result;
      } else {
        continue;
      } 
    }
  }

  return BLEDevice();
}

void GAPClass::setAdvertisingInterval(uint16_t advertisingInterval)
{
  _advertisingInterval = advertisingInterval;
}

void GAPClass::setConnectable(bool connectable)
{
  _connectable = connectable;
}

void GAPClass::setEventHandler(BLEDeviceEvent event, BLEDeviceEventHandler eventHandler)
{
  if (event == BLEDiscovered) {
    _discoverEventHandler = eventHandler;
  }
}

void GAPClass::setAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length)
{
  _serviceDataUuid = uuid;
  _serviceData = data;
  _serviceDataLength = length;
}

void GAPClass::handleLeAdvertisingReport(uint8_t type, uint8_t addressType, uint8_t address[6],
                                          uint8_t eirLength, uint8_t eirData[], int8_t rssi)
{
  if (!_scanning) {
    return;
  }

  if (_discoverEventHandler && type == 0x03) {
    // call event handler and skip adding to discover list
    BLEDevice device(addressType, address);

    device.setAdvertisementData(type, eirLength, eirData, rssi);

    if (matchesScanFilter(device)) {
      _discoverEventHandler(device);
    }
    return;
  }

  BLEDevice* discoveredDevice = NULL;
  int discoveredIndex = -1;

  for (unsigned int i = 0; i < _discoveredDevices.size(); i++) {
    BLEDevice* device = _discoveredDevices.get(i);

    if (device->hasAddress(addressType, address)) {
      discoveredDevice = device;
      discoveredIndex = i;

      break;
    }
  }

  if (discoveredDevice == NULL) {
    if (_discoveredDevices.size() >= GAP_MAX_DISCOVERED_QUEUE_SIZE) {
      // drop
      return;
    }

    discoveredDevice = new BLEDevice(addressType, address);

    _discoveredDevices.add(discoveredDevice);
    discoveredIndex = _discoveredDevices.size() - 1;
  }

  if (type != 0x04) {
    discoveredDevice->setAdvertisementData(type, eirLength, eirData, rssi);
  } else {
    discoveredDevice->setScanResponseData(eirLength, eirData, rssi);
  }

  if (discoveredDevice->discovered() && _discoverEventHandler) {
    // remove from list and report as discovered
    BLEDevice device = *discoveredDevice;

    _discoveredDevices.remove(discoveredIndex);
    delete discoveredDevice;

    if (matchesScanFilter(device)) {
      _discoverEventHandler(device);
    }
  }
}

bool GAPClass::matchesScanFilter(const BLEDevice& device)
{
  if (_scanAddressFilter.length() > 0 && !(_scanAddressFilter.equalsIgnoreCase(device.address()))) {
    return false; // drop doesn't match
  } else if (_scanNameFilter.length() > 0 && _scanNameFilter != device.localName()) {
    return false; // drop doesn't match
  } else if (_scanUuidFilter.length() > 0 && !(_scanUuidFilter.equalsIgnoreCase(device.advertisedServiceUuid()))) {
    return false; // drop doesn't match
  }

  return true;
}

GAPClass GAP;
