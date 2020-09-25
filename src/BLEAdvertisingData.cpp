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

#include "BLEAdvertisingData.h"

BLEAdvertisingData::BLEAdvertisingData() :
  _dataLength(0),
  _advertisedServiceUuid(NULL),
  _manufacturerData(NULL),
  _manufacturerDataLength(0),
  _manufacturerCompanyId(0),
  _hasManufacturerCompanyId(false),
  _localName(NULL),
  _serviceData(NULL),
  _serviceDataLength(0)
{
}

BLEAdvertisingData::~BLEAdvertisingData()
{
}

void BLEAdvertisingData::setAdvertisedServiceUuid(const char* advertisedServiceUuid)
{
  _advertisedServiceUuid = advertisedServiceUuid;
}

void BLEAdvertisingData::setManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength)
{
  _manufacturerData = manufacturerData;
  _manufacturerDataLength = manufacturerDataLength;
  _hasManufacturerCompanyId = false;
}

void BLEAdvertisingData::setManufacturerData(const uint16_t companyId, const uint8_t manufacturerData[], int manufacturerDataLength)
{
  _manufacturerData = manufacturerData;
  _manufacturerDataLength = manufacturerDataLength;
  _manufacturerCompanyId = companyId;
  _hasManufacturerCompanyId = true;
}

void BLEAdvertisingData::setAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length)
{
  _serviceDataUuid = uuid;
  _serviceData = data;
  _serviceDataLength = length;
}

void BLEAdvertisingData::setLocalName(const char *localName)
{
  _localName = localName;
}

bool BLEAdvertisingData::updateData()
{
  bool success = true;
  // Reset data 
  _dataLength = 0;
  // Try to add Advertised service uuid into the current advertising packet
  if (_advertisedServiceUuid) {
    success &= addAdvertisedServiceUuid(_advertisedServiceUuid);
  }
  // Try to add Manufacturer data into the current advertising packet
  if (_manufacturerData && _manufacturerDataLength) {
    if (_hasManufacturerCompanyId) {
      success &= addManufacturerData(_manufacturerCompanyId, _manufacturerData, _manufacturerDataLength);
    } else {
      success &= addManufacturerData(_manufacturerData, _manufacturerDataLength);
    }
  }
  // Try to add Service data into the current advertising packet
  if (_serviceData && _serviceDataLength) {
    success &= addAdvertisedServiceData(_serviceDataUuid, _serviceData, _serviceDataLength);
  }
  // Try to add Local name into the current advertising packet
  if (_localName) {
    success &= addLocalName(_localName);
  }
  return success;
}

uint8_t* BLEAdvertisingData::data() 
{
  return _data;
}

int BLEAdvertisingData::dataLength() const
{
  return _dataLength;
}

bool BLEAdvertisingData::addLocalName(const char *localName)
{
  bool success = false;
  if (strlen(localName) > (MAX_AD_DATA_LENGTH - 2)) {
    success = addField(BLEFieldShortLocalName, (uint8_t*)localName, (MAX_AD_DATA_LENGTH - 2));
  } else {
    success = addField(BLEFieldCompleteLocalName, localName);
  }
  return success;
}

bool BLEAdvertisingData::addAdvertisedServiceUuid(const char* advertisedServiceUuid)
{
  BLEUuid uuid(advertisedServiceUuid);
  int uuidLen = uuid.length();
  BLEAdField advField;
  if (uuidLen > 2) {
    advField = BLEFieldAdvertisedService128;
  } else {
    advField = BLEFieldAdvertisedService16;
  }
  return addField(advField, uuid.data(), uuidLen);
}

bool BLEAdvertisingData::addManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength)
{
  return addField(BLEFieldManufacturerData, manufacturerData, manufacturerDataLength);
}

bool BLEAdvertisingData::addManufacturerData(const uint16_t companyId, const uint8_t manufacturerData[], int manufacturerDataLength)
{
  uint8_t tempDataLength = manufacturerDataLength + sizeof(companyId);
  uint8_t tempData[MAX_AD_DATA_LENGTH];
  memcpy(tempData, &companyId, sizeof(companyId));
  memcpy(&tempData[sizeof(companyId)], manufacturerData, manufacturerDataLength);
  return addField(BLEFieldManufacturerData, tempData, tempDataLength);
}

bool BLEAdvertisingData::addAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length)
{
  uint8_t tempDataLength = length + sizeof(uuid);
  uint8_t tempData[MAX_AD_DATA_LENGTH];
  memcpy(tempData, &uuid, sizeof(uuid));
  memcpy(&tempData[sizeof(uuid)], data, length);
  return addField(BLEFieldServiceData, tempData, tempDataLength);
}

bool BLEAdvertisingData::addField(BLEAdField field, const char* data)
{
  uint8_t dataLength = strlen(data);
  return addField(field, (uint8_t *)data, dataLength);
}

bool BLEAdvertisingData::addField(BLEAdField field, const uint8_t* data, uint8_t length)
{
  uint8_t fieldLength = length + 2; // Considering data TYPE and LENGTH fields
  if (fieldLength > (MAX_AD_DATA_LENGTH - _dataLength)) {
    // Not enough space for storing this field
    return false;
  }
  // Insert field into advertising data of the instance
  _data[_dataLength++] = length + 1;
  _data[_dataLength++] = field;
  memcpy(&_data[_dataLength], data, length);
  _dataLength += length;
  return true;
}
