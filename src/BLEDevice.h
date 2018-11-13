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

#ifndef _BLE_DEVICE_H_
#define _BLE_DEVICE_H_

#include <Arduino.h>

enum BLEDeviceEvent {
  BLEConnected = 0,
  BLEDisconnected = 1,

  BLEDeviceLastEvent
};

class BLEDevice;

typedef void (*BLEDeviceEventHandler)(BLEDevice device);

class BLEDevice {
public:
  BLEDevice();
  virtual ~BLEDevice();

  virtual void poll();
  virtual void poll(unsigned long timeout);

  virtual bool connected() const;
  virtual bool disconnect();

  virtual String address() const;

  virtual int rssi();

  virtual operator bool() const;
  virtual bool operator==(const BLEDevice& rhs) const;
  virtual bool operator!=(const BLEDevice& rhs) const;

protected:
  friend class ATTClass;

  BLEDevice(uint16_t handle, uint8_t address[6]);

private:
  uint16_t _handle;
  uint8_t _address[6];  
};

#endif
