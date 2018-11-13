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

#ifndef _BLE_CHARACTERISTIC_H_
#define _BLE_CHARACTERISTIC_H_

#include <stdint.h>

#include "BLEDescriptor.h"

enum BLECharacteristicEvent {
  BLESubscribed = 0,
  BLEUnsubscribed = 1,
//BLERead = 2, // defined in BLEProperties.h
  BLEWritten = 3,

  BLECharacteristicEventLast
};

class BLECharacteristic;
class BLEDevice;

typedef void (*BLECharacteristicEventHandler)(BLEDevice device, BLECharacteristic characteristic);

class BLELocalCharacteristic;

class BLECharacteristic  {
public:
  BLECharacteristic();
  BLECharacteristic(const char* uuid, uint8_t properties, int valueSize, bool fixedLength = false);
  BLECharacteristic(const char* uuid, uint8_t properties, const char* value);
  virtual ~BLECharacteristic();

  uint8_t properties() const;

  int valueSize() const;
  const uint8_t* value() const;
  int valueLength() const;
  uint8_t operator[] (int offset) const;

  int writeValue(const uint8_t value[], int length);
  int writeValue(const char* value);

  int setValue(const uint8_t value[], int length) { return writeValue(value, length); }
  int setValue(const char* value) { return writeValue(value); }

  int broadcast();

  bool written();
  bool subscribed();

  void addDescriptor(BLEDescriptor& descriptor);

  operator bool() const;

  void setEventHandler(int event, BLECharacteristicEventHandler eventHandler);

protected:
  friend class BLELocalCharacteristic;
  friend class BLELocalService;

  BLECharacteristic(BLELocalCharacteristic* local);

  BLELocalCharacteristic* local();

private:
  BLELocalCharacteristic* _local;
};

#endif
