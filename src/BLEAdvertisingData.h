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

#ifndef _BLE_ADVERTISING_DATA_H_
#define _BLE_ADVERTISING_DATA_H_

#include <Arduino.h>
#include "utility/BLEUuid.h"

#define MAX_AD_DATA_LENGTH (31)

enum BLEAdField {
  BLEFieldShortLocalName = 0x08,
  BLEFieldCompleteLocalName = 0x09,
  BLEFieldAdvertisedService16 = 0x02,
  BLEFieldAdvertisedService128 = 0x06,
  BLEFieldServiceData = 0x16,
  BLEFieldManufacturerData = 0xFF,

  BLEAdFieldLast
};

class BLEAdvertisingData {
public:
  BLEAdvertisingData(); 
  virtual ~BLEAdvertisingData();

  void setAdvertisedServiceUuid(const char* advertisedServiceUuid);
  void setManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength);
  void setManufacturerData(const uint16_t companyId, const uint8_t manufacturerData[], int manufacturerDataLength);
  void setLocalName(const char *localName);
  void setAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length);

  bool updateData();

  uint8_t* getData();
  int getDataLength();

private:
  bool addAdvertisedServiceUuid(const char* advertisedServiceUuid);
  bool addManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength);
  bool addManufacturerData(const uint16_t companyId, const uint8_t manufacturerData[], int manufacturerDataLength);
  bool addLocalName(const char *localName);
  bool addAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length);

  bool addField(BLEAdField field, const char* data);
  bool addField(BLEAdField field, const uint8_t* data, uint8_t length);

  uint8_t _data[MAX_AD_DATA_LENGTH];
  int _dataLength;

  const char* _advertisedServiceUuid; 
  const uint8_t* _manufacturerData;
  int _manufacturerDataLength;
  uint16_t _manufacturerCompanyId;
  bool _hasManufacturerCompanyId;
  const char* _localName;
  uint16_t _serviceDataUuid;
  const uint8_t* _serviceData;
  int _serviceDataLength;
};

#endif
