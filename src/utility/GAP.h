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

#ifndef _GAP_H_
#define _GAP_H_

class GAPClass {
public:
  GAPClass();
  virtual ~GAPClass();

  void setAdvertisedServiceUuid(const char* advertisedServiceUuid);
  void setManufacturerData(const uint8_t manufacturerData[], int manufacturerDataLength);
  void setLocalName(const char *localName);

  bool advertising();
  int advertise();
  void stopAdvertise();

  void setAdvertisingInterval(uint16_t advertisingInterval);
  void setConnectable(bool connectable); 

protected:
  friend class BLELocalCharacteristic;

  void setAdvertisedServiceData(uint16_t uuid, const uint8_t data[], int length);

private:
  bool _advertising;

  const char* _advertisedServiceUuid;
  const uint8_t* _manufacturerData;
  int _manufacturerDataLength;
  const char* _localName;
  uint16_t _advertisingInterval;
  bool _connectable;

  uint16_t _serviceDataUuid;
  const uint8_t* _serviceData;
  int _serviceDataLength;
};

extern GAPClass GAP;

#endif
