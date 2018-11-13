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
#include "utility/HCI.h"

#include "BLEDevice.h"

BLEDevice::BLEDevice() :
  _handle(0xffff)
{
  memset(_address, 0x00, sizeof(_address));
}

BLEDevice::BLEDevice(uint16_t handle, uint8_t address[6]) :
  _handle(handle)
{
  memcpy(_address, address, sizeof(_address));
}

BLEDevice::~BLEDevice()
{
}

void BLEDevice::poll()
{
  HCI.poll();
}

void BLEDevice::poll(unsigned long timeout)
{
  HCI.poll(timeout);
}

bool BLEDevice::connected() const
{
  HCI.poll();

  if (!(*this)) {
    return false;
  }

  return ATT.connected(_handle, _address);
}

bool BLEDevice::disconnect()
{
  if (_handle != 0xffff) {
    return HCI.disconnect(_handle);
  }

  return false;
}

String BLEDevice::address() const
{
  char result[18];
  sprintf(result, "%02x:%02x:%02x:%02x:%02x:%02x", _address[5], _address[4], _address[3], _address[2], _address[1], _address[0]);

  return result;
}

int BLEDevice::rssi()
{
  if (_handle != 0xffff) {
    return HCI.readRssi(_handle);
  }

  return 127;
}

BLEDevice::operator bool() const
{
  uint8_t zeros[6] = { 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,};

  return ((_handle != 0xffff) && memcmp(_address, zeros, sizeof(zeros)) != 0);
}

bool BLEDevice::operator==(const BLEDevice& rhs) const
{
  return ((_handle == rhs._handle) && memcmp(_address, rhs._address, sizeof(_address)) == 0);
}

bool BLEDevice::operator!=(const BLEDevice& rhs) const
{
  return ((_handle != rhs._handle) || memcmp(_address, rhs._address, sizeof(_address)) != 0);
}
