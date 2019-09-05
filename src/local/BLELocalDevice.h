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

#ifndef _BLE_LOCAL_DEVICE_H_
#define _BLE_LOCAL_DEVICE_H_

#include "BLEDevice.h"
#include "BLEService.h"

class BLELocalDevice {
public:
  BLELocalDevice();
  virtual ~BLELocalDevice();

  int begin();
  void end();

  void poll();
  void poll(unsigned long timeout);

  bool connected() const;
  bool disconnect();

  String address() const;

 int rssi();

  void setAdvertisedServiceUuid(const char* advertisedServiceUuid);
  void setAdvertisedService(const BLEService& service);
  void setManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength);
  void setManufacturerData(const uint16_t companyId, const uint8_t manufacturerData[], int manufacturerDataLength);
  void setLocalName(const char *localName);

  void setDeviceName(const char* deviceName);
  void setAppearance(uint16_t appearance);

  void addService(BLEService& service);

  int advertise();
  void stopAdvertise();

  int scan(bool withDuplicates = false);
  int scanForName(String name, bool withDuplicates = false);
  int scanForUuid(String uuid, bool withDuplicates = false);
  int scanForAddress(String address, bool withDuplicates = false);
  void stopScan();

  BLEDevice central();
  BLEDevice available();

  void setEventHandler(BLEDeviceEvent event, BLEDeviceEventHandler eventHandler);

  void setAdvertisingInterval(uint16_t advertisingInterval);
  void setConnectionInterval(uint16_t minimumConnectionInterval, uint16_t maximumConnectionInterval);
  void setConnectable(bool connectable); 

  void setTimeout(unsigned long timeout);

  void debug(Stream& stream);
  void noDebug();

private:
};

extern BLELocalDevice BLE;

#endif
