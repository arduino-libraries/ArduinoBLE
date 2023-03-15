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

#include "utility/ATT.h"
#include "utility/GAP.h"
#include "utility/GATT.h"
#include "utility/L2CAPSignaling.h"

#include "BLELocalDevice.h"

BLELocalDevice::BLELocalDevice(HCITransportInterface *HCITransport, uint8_t ownBdaddrType) :
  _HCITransport(HCITransport), _ownBdaddrType(ownBdaddrType)
{
  _advertisingData.setFlags(BLEFlagsGeneralDiscoverable | BLEFlagsBREDRNotSupported);
}

BLELocalDevice::~BLELocalDevice()
{
}

int BLELocalDevice::begin()
{
  HCI.setTransport(_HCITransport);

  if (!HCI.begin()) {
    end();
    return 0;
  }

  delay(100);

  if (HCI.reset() != 0) {
    end();

    return 0;
  }

  uint8_t randomNumber[8];
  if (HCI.leRand(randomNumber) != 0) {
    end();
    return 0;
  }
  /* Random address only requires 6 bytes (48 bits)
   * Force both MSB bits to b00 in order to define Static Random Address
   */
  randomNumber[5] |= 0xC0;

  // Copy the random address in private variable as it will be sent to the BLE chip
  randomAddress [0] = randomNumber[0];
  randomAddress [1] = randomNumber[1];
  randomAddress [2] = randomNumber[2];
  randomAddress [3] = randomNumber[3];
  randomAddress [4] = randomNumber[4];
  randomAddress [5] = randomNumber[5];

  if (HCI.leSetRandomAddress((uint8_t*)randomNumber) != 0) {
    end();
    return 0;
  }

  uint8_t hciVer;
  uint16_t hciRev;
  uint8_t lmpVer;
  uint16_t manufacturer;
  uint16_t lmpSubVer;

  if (HCI.readLocalVersion(hciVer, hciRev, lmpVer, manufacturer, lmpSubVer) != 0) {
    end();
    return 0;
  }

  if (HCI.setEventMask(0x3FFFFFFFFFFFFFFF) != 0) {
    end();
    return 0;
  }

  uint16_t pktLen;
  uint8_t maxPkt;

  if (HCI.readLeBufferSize(pktLen, maxPkt) != 0) {
    end();
    return 0;
  }

  GATT.begin();

  GAP.setOwnBdaddrType(_ownBdaddrType);

  ATT.setOwnBdaddrType(_ownBdaddrType);

  return 1;
}

void BLELocalDevice::end()
{
  GATT.end();

  HCI.end();
}

void BLELocalDevice::getRandomAddress(uint8_t buff[6])
{
  buff [0] = randomAddress[0];
  buff [1] = randomAddress[1];
  buff [2] = randomAddress[2];
  buff [3] = randomAddress[3];
  buff [4] = randomAddress[4];
  buff [5] = randomAddress[5];
}

void BLELocalDevice::poll()
{
  HCI.poll();
}

void BLELocalDevice::poll(unsigned long timeout)
{
  HCI.poll(timeout);
}

bool BLELocalDevice::connected() const
{
  HCI.poll();

  return ATT.connected();
}

bool BLELocalDevice::disconnect()
{
  return ATT.disconnect();
}

String BLELocalDevice::address() const
{
  uint8_t addr[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00 };
  HCI.readBdAddr(addr);

  char result[18];
  sprintf(result, "%02x:%02x:%02x:%02x:%02x:%02x", addr[5], addr[4], addr[3], addr[2], addr[1], addr[0]);

  return result;
}

int BLELocalDevice::rssi()
{
  BLEDevice central = ATT.central();

  if (central) {
    return central.rssi();
  }

  return 127;
}

bool BLELocalDevice::setAdvertisedServiceUuid(const char* advertisedServiceUuid)
{
  return _advertisingData.setAdvertisedServiceUuid(advertisedServiceUuid);
}

bool BLELocalDevice::setAdvertisedService(const BLEService& service)
{
  return setAdvertisedServiceUuid(service.uuid());
}

bool BLELocalDevice::setAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length)
{
  return _advertisingData.setAdvertisedServiceData(uuid, data, length);
}

bool BLELocalDevice::setManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength)
{
  return _advertisingData.setManufacturerData(manufacturerData, manufacturerDataLength);
}

bool BLELocalDevice::setManufacturerData(const uint16_t companyId, const uint8_t manufacturerData[], int manufacturerDataLength)
{
  return _advertisingData.setManufacturerData(companyId, manufacturerData, manufacturerDataLength);
}

bool BLELocalDevice::setLocalName(const char *localName)
{
  return _scanResponseData.setLocalName(localName);  
}

void BLELocalDevice::setAdvertisingData(BLEAdvertisingData& advertisingData)
{
  _advertisingData = advertisingData;
  if (!_advertisingData.hasFlags()) {
    _advertisingData.setFlags(BLEFlagsGeneralDiscoverable | BLEFlagsBREDRNotSupported);
  }
}

void BLELocalDevice::setScanResponseData(BLEAdvertisingData& scanResponseData)
{
  _scanResponseData = scanResponseData;
}

BLEAdvertisingData& BLELocalDevice::getAdvertisingData()
{
  return _advertisingData;
}

BLEAdvertisingData& BLELocalDevice::getScanResponseData()
{
  return _scanResponseData;
}

void BLELocalDevice::setDeviceName(const char* deviceName)
{
  GATT.setDeviceName(deviceName);
}

void BLELocalDevice::setAppearance(uint16_t appearance)
{
  GATT.setAppearance(appearance);
}

void BLELocalDevice::addService(BLEService& service)
{
  GATT.addService(service);
}

int BLELocalDevice::advertise()
{
  _advertisingData.updateData();
  _scanResponseData.updateData();
  return GAP.advertise( _advertisingData.data(), _advertisingData.dataLength(), 
                        _scanResponseData.data(), _scanResponseData.dataLength());
}

void BLELocalDevice::stopAdvertise()
{
  GAP.stopAdvertise();
}

int BLELocalDevice::scan(bool withDuplicates)
{
  return GAP.scan(withDuplicates);
}

int BLELocalDevice::scanForName(String name, bool withDuplicates)
{
  return GAP.scanForName(name, withDuplicates);
}

int BLELocalDevice::scanForUuid(String uuid, bool withDuplicates)
{
  return GAP.scanForUuid(uuid, withDuplicates);
}

int BLELocalDevice::scanForAddress(String address, bool withDuplicates)
{
  return GAP.scanForAddress(address, withDuplicates);
}

int BLELocalDevice::stopScan()
{
  return GAP.stopScan();
}

BLEDevice BLELocalDevice::central()
{
  HCI.poll();

  return ATT.central();
}

BLEDevice BLELocalDevice::available()
{
  HCI.poll();

  return GAP.available();
}

void BLELocalDevice::setEventHandler(BLEDeviceEvent event, BLEDeviceEventHandler eventHandler)
{
  if (event == BLEDiscovered) {
    GAP.setEventHandler(event, eventHandler);
  } else {
    ATT.setEventHandler(event, eventHandler);
  }
}

void BLELocalDevice::setAdvertisingInterval(uint16_t advertisingInterval)
{
  GAP.setAdvertisingInterval(advertisingInterval);
}

void BLELocalDevice::setConnectionInterval(uint16_t minimumConnectionInterval, uint16_t maximumConnectionInterval)
{
  L2CAPSignaling.setConnectionInterval(minimumConnectionInterval, maximumConnectionInterval);
}

void BLELocalDevice::setSupervisionTimeout(uint16_t supervisionTimeout)
{
  L2CAPSignaling.setSupervisionTimeout(supervisionTimeout);
}

void BLELocalDevice::setConnectable(bool connectable)
{
  GAP.setConnectable(connectable);
}

void BLELocalDevice::setTimeout(unsigned long timeout)
{
  ATT.setTimeout(timeout);
}

void BLELocalDevice::debug(Stream& stream)
{
  HCI.debug(stream);
}

void BLELocalDevice::noDebug()
{
  HCI.noDebug();
}
