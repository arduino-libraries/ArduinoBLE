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

#ifndef _ATT_H_
#define _ATT_H_

#include <Arduino.h>

#include "BLEDevice.h"

#define ATT_CID 0x0004

class ATTClass {
public:
  ATTClass();
  virtual ~ATTClass();

  void setMaxMtu(uint16_t maxMtu);

  void addConnection(uint16_t handle, uint8_t role, uint8_t peerBdaddrType,
                    uint8_t peerBdaddr[6], uint16_t interval,
                    uint16_t latency, uint16_t supervisionTimeout,
                    uint8_t masterClockAccuracy);

  void handleData(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);

  void removeConnection(uint8_t handle, uint16_t reason);

  bool connected() const;
  bool connected(uint16_t handle, const uint8_t address[6]) const;

  bool disconnect();

  BLEDevice central();

  bool handleNotify(uint16_t handle, const uint8_t* value, int length);
  bool handleInd(uint16_t handle, const uint8_t* value, int length);

  void setEventHandler(BLEDeviceEvent event, BLEDeviceEventHandler eventHandler);

private:
  void mtuReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void findInfoReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void findByTypeReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void readByTypeReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void readOrReadBlobReq(uint16_t connectionHandle, uint8_t opcode, uint8_t dlen, uint8_t data[]);
  void readByGroupReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void writeReqOrCmd(uint16_t connectionHandle, uint8_t op, uint8_t dlen, uint8_t data[]);
  void prepWriteReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void execWriteReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void handleCnf(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void sendError(uint16_t connectionHandle, uint8_t opcode, uint16_t handle, uint8_t code);

private:
  uint16_t _maxMtu;
  uint16_t _connectionHandle;
  uint8_t _peerAddress[6];
  uint16_t _mtu;
  volatile bool _cnf;

  uint16_t _longWriteHandle;
  uint8_t* _longWriteValue;
  uint16_t _longWriteValueLength;

  BLEDeviceEventHandler _eventHandlers[BLEDeviceLastEvent];
};

extern ATTClass ATT;

#endif
