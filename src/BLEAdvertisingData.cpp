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

#define AD_FIELD_OVERHEAD (2)

BLEAdvertisingData::BLEAdvertisingData() :
  _dataLength(0),
  _rawData(NULL),
  _rawDataLength(0),
  _advertisedServiceUuid(NULL),
  _manufacturerData(NULL),
  _manufacturerDataLength(0),
  _manufacturerCompanyId(0),
  _hasManufacturerCompanyId(false),
  _localName(NULL),
  _serviceData(NULL),
  _serviceDataLength(0),
  _flags(0),
  _hasFlags(false),
  _remainingLength(MAX_AD_DATA_LENGTH)
{
}

BLEAdvertisingData::~BLEAdvertisingData()
{
}

inline bool BLEAdvertisingData::updateRemainingLength(int fieldLength)
{
  if (fieldLength <= _remainingLength) {
    _remainingLength -= fieldLength;
    return true;
  }
  return false;
}

int BLEAdvertisingData::remainingLength() const
{
  return _remainingLength;
}

int BLEAdvertisingData::availableForWrite()
{
  int available = (_remainingLength - AD_FIELD_OVERHEAD);
  if (available < 0) available = 0;
  return available; 
}

void BLEAdvertisingData::clear()
{
  _remainingLength = MAX_AD_DATA_LENGTH;
  _rawData = NULL;
  _rawDataLength = 0;
  _hasFlags = false;
  _localName = NULL;
  _manufacturerData = NULL;
  _manufacturerDataLength = 0;
  _hasManufacturerCompanyId = false;
  _advertisedServiceUuid = NULL;
  _serviceData = NULL;
  _serviceDataLength = 0;
}

void BLEAdvertisingData::copy(const BLEAdvertisingData& adv)
{
  _remainingLength = adv._remainingLength;
  _rawData = adv._rawData;
  _rawDataLength = adv._rawDataLength;
  _flags = adv._flags;
  _hasFlags = adv._hasFlags;
  _localName = adv._localName;
  _manufacturerData = adv._manufacturerData;
  _manufacturerDataLength = adv._manufacturerDataLength;
  _manufacturerCompanyId = adv._manufacturerCompanyId;
  _hasManufacturerCompanyId = adv._hasManufacturerCompanyId;
  _advertisedServiceUuid = adv._advertisedServiceUuid;
  _serviceDataUuid = adv._serviceDataUuid;
  _serviceData = adv._serviceData;
  _serviceDataLength = adv._serviceDataLength;
}

BLEAdvertisingData& BLEAdvertisingData::operator=(const BLEAdvertisingData &other) 
{
  copy(other);
  return *this;
}

bool BLEAdvertisingData::setAdvertisedServiceUuid(const char* advertisedServiceUuid)
{
  BLEUuid uuid(advertisedServiceUuid);
  bool success = updateRemainingLength(uuid.length() + AD_FIELD_OVERHEAD);
  if (success) {
    _advertisedServiceUuid = advertisedServiceUuid;
  }
  return success;
}

bool BLEAdvertisingData::setAdvertisedService(const BLEService& service)
{
  return setAdvertisedServiceUuid(service.uuid());
}

bool BLEAdvertisingData::setManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength)
{
  bool success = updateRemainingLength(manufacturerDataLength + AD_FIELD_OVERHEAD);
  if (success) {
    _manufacturerData = manufacturerData;
    _manufacturerDataLength = manufacturerDataLength;
    _hasManufacturerCompanyId = false;
  }
  return success;
}

bool BLEAdvertisingData::setManufacturerData(const uint16_t companyId, const uint8_t manufacturerData[], int manufacturerDataLength)
{
  bool success = updateRemainingLength(manufacturerDataLength + sizeof(companyId) + AD_FIELD_OVERHEAD);
  if (success) {
    _manufacturerData = manufacturerData;
    _manufacturerDataLength = manufacturerDataLength;
    _manufacturerCompanyId = companyId;
    _hasManufacturerCompanyId = true;
  }
  return success;
}

bool BLEAdvertisingData::setAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length)
{
  bool success = updateRemainingLength(length + sizeof(uuid) + AD_FIELD_OVERHEAD);
  if (success) {
    _serviceDataUuid = uuid;
    _serviceData = data;
    _serviceDataLength = length;
  }
  return success;
}

bool BLEAdvertisingData::setLocalName(const char *localName)
{
  bool success = updateRemainingLength(strlen(localName) + AD_FIELD_OVERHEAD);
  if (success) {
    _localName = localName;
  }
  return success;
}

bool BLEAdvertisingData::setRawData(const uint8_t* data, int length)
{
  if (length > MAX_AD_DATA_LENGTH) {
    length = MAX_AD_DATA_LENGTH;
  }
  _rawData = data;
  _rawDataLength = length;
  return true;
}

bool BLEAdvertisingData::setRawData(const BLEAdvertisingRawData& rawData)
{
  _rawData = rawData.data;
  _rawDataLength = rawData.length;
  if (_rawDataLength > MAX_AD_DATA_LENGTH) {
    _rawDataLength = MAX_AD_DATA_LENGTH;
  }
  return true;
}

bool BLEAdvertisingData::setFlags(uint8_t flags)
{
  bool success = updateRemainingLength(sizeof(flags) + AD_FIELD_OVERHEAD);
  if (success) {
    _hasFlags = true;
    _flags = flags;
  }
  return success;
}

bool BLEAdvertisingData::updateData()
{
  // Success indicates whether all the fields have been inserted
  bool success = true;
  // Reset data 
  _dataLength = 0;
  // If rawData is present, then only rawData is inserted in the advertising packet
  if (_rawData && _rawDataLength) {
    return addRawData(_rawData, _rawDataLength);
  }
  // Try to add flags into the current advertising packet
  if (_hasFlags) {
    success &= addFlags(_flags);
  }
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

bool BLEAdvertisingData::hasFlags() const
{
  return _hasFlags;
}

bool BLEAdvertisingData::addLocalName(const char *localName)
{
  bool success = false;
  if (strlen(localName) > (MAX_AD_DATA_LENGTH - AD_FIELD_OVERHEAD)) {
    success = addField(BLEFieldShortLocalName, (uint8_t*)localName, (MAX_AD_DATA_LENGTH - AD_FIELD_OVERHEAD));
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
    advField = BLEFieldIncompleteAdvertisedService128;
  } else {
    advField = BLEFieldIncompleteAdvertisedService16;
  }
  return addField(advField, uuid.data(), uuidLen);
}

bool BLEAdvertisingData::addManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength)
{
  return addField(BLEFieldManufacturerData, manufacturerData, manufacturerDataLength);
}

bool BLEAdvertisingData::addManufacturerData(const uint16_t companyId, const uint8_t manufacturerData[], int manufacturerDataLength)
{
  int tempDataLength = manufacturerDataLength + sizeof(companyId);
  uint8_t tempData[MAX_AD_DATA_LENGTH];
  memcpy(tempData, &companyId, sizeof(companyId));
  memcpy(&tempData[sizeof(companyId)], manufacturerData, manufacturerDataLength);
  return addField(BLEFieldManufacturerData, tempData, tempDataLength);
}

bool BLEAdvertisingData::addAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length)
{
  int tempDataLength = length + sizeof(uuid);
  uint8_t tempData[MAX_AD_DATA_LENGTH];
  memcpy(tempData, &uuid, sizeof(uuid));
  memcpy(&tempData[sizeof(uuid)], data, length);
  return addField(BLEFieldServiceData, tempData, tempDataLength);
}

bool BLEAdvertisingData::addRawData(const uint8_t* data, int length)
{
  // Bypass addField to add the integral raw data
  if (length > (MAX_AD_DATA_LENGTH - _dataLength)) {
    // Not enough space 
    return false;
  }
  memcpy(&_data[_dataLength], data, length);
  _dataLength += length;
  return true;
}

bool BLEAdvertisingData::addFlags(uint8_t flags)
{
  return addField(BLEFieldFlags, &flags, sizeof(flags));
}

bool BLEAdvertisingData::addField(BLEAdField field, const char* data)
{
  int dataLength = strlen(data);
  return addField(field, (uint8_t *)data, dataLength);
}

bool BLEAdvertisingData::addField(BLEAdField field, const uint8_t* data, int length)
{
  int fieldLength = length + AD_FIELD_OVERHEAD; // Considering data TYPE and LENGTH fields
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
