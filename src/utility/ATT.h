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

#define ATT_CID       0x0004

#ifdef DM_CONN_MAX
#define ATT_MAX_PEERS DM_CONN_MAX // Mbed + Cordio
#else
#define ATT_MAX_PEERS 3
#endif

class BLERemoteDevice;

class ATTClass {
public:
  ATTClass();
  virtual ~ATTClass();

  void setMaxMtu(uint16_t maxMtu);
  void setTimeout(unsigned long timeout);

  bool connect(uint8_t peerBdaddrType, uint8_t peerBdaddr[6]);
  bool disconnect(uint8_t peerBdaddrType, uint8_t peerBdaddr[6]);
  bool discoverAttributes(uint8_t peerBdaddrType, uint8_t peerBdaddr[6], const char* serviceUuidFilter);

  void addConnection(uint16_t handle, uint8_t role, uint8_t peerBdaddrType,
                    uint8_t peerBdaddr[6], uint16_t interval,
                    uint16_t latency, uint16_t supervisionTimeout,
                    uint8_t masterClockAccuracy);

  void handleData(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);

  void removeConnection(uint16_t handle, uint8_t reason);

  uint16_t connectionHandle(uint8_t addressType, const uint8_t address[6]) const;
  BLERemoteDevice* device(uint8_t addressType, const uint8_t address[6]) const;
  bool connected() const;
  bool connected(uint8_t addressType, const uint8_t address[6]) const;
  bool connected(uint16_t handle) const;
  uint16_t mtu(uint16_t handle) const;

  bool disconnect();

  BLEDevice central();

  bool handleNotify(uint16_t handle, const uint8_t* value, int length);
  bool handleInd(uint16_t handle, const uint8_t* value, int length);

  void setEventHandler(BLEDeviceEvent event, BLEDeviceEventHandler eventHandler);

  int readReq(uint16_t connectionHandle, uint16_t handle, uint8_t responseBuffer[]);
  int writeReq(uint16_t connectionHandle, uint16_t handle, const uint8_t* data, uint8_t dataLen, uint8_t responseBuffer[]);
  void writeCmd(uint16_t connectionHandle, uint16_t handle, const uint8_t* data, uint8_t dataLen);

private:
  void error(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void mtuReq(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  int mtuReq(uint16_t connectionHandle, uint16_t mtu, uint8_t responseBuffer[]);
  void mtuResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void findInfoReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[]);
  int findInfoReq(uint16_t connectionHandle, uint16_t startHandle, uint16_t endHandle, uint8_t responseBuffer[]);
  void findInfoResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void findByTypeReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[]);
  void readByTypeReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[]);
  int readByTypeReq(uint16_t connectionHandle, uint16_t startHandle, uint16_t endHandle, uint16_t type, uint8_t responseBuffer[]);
  void readByTypeResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void readOrReadBlobReq(uint16_t connectionHandle, uint16_t mtu, uint8_t opcode, uint8_t dlen, uint8_t data[]);
  void readResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void readByGroupReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[]);
  int readByGroupReq(uint16_t connectionHandle, uint16_t startHandle, uint16_t endHandle, uint16_t uuid, uint8_t responseBuffer[]);
  void readByGroupResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void writeReqOrCmd(uint16_t connectionHandle, uint16_t mtu, uint8_t op, uint8_t dlen, uint8_t data[]);
  void writeResp(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void prepWriteReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[]);
  void execWriteReq(uint16_t connectionHandle, uint16_t mtu, uint8_t dlen, uint8_t data[]);
  void handleNotifyOrInd(uint16_t connectionHandle, uint8_t opcode, uint8_t dlen, uint8_t data[]);
  void handleCnf(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);
  void sendError(uint16_t connectionHandle, uint8_t opcode, uint16_t handle, uint8_t code);

  bool exchangeMtu(uint16_t connectionHandle);
  bool discoverServices(uint16_t connectionHandle, BLERemoteDevice* device, const char* serviceUuidFilter);
  bool discoverCharacteristics(uint16_t connectionHandle, BLERemoteDevice* device);
  bool discoverDescriptors(uint16_t connectionHandle, BLERemoteDevice* device);

  int sendReq(uint16_t connectionHandle, void* requestBuffer, int requestLength, uint8_t responseBuffer[]);

private:
  uint16_t _maxMtu;
  unsigned long _timeout;
  struct {
    uint16_t connectionHandle;
    uint8_t role;
    uint8_t addressType;
    uint8_t address[6];
    uint16_t mtu;
    BLERemoteDevice* device;
  } _peers[ATT_MAX_PEERS];

  volatile bool _cnf;

  uint16_t _longWriteHandle;
  uint8_t* _longWriteValue;
  uint16_t _longWriteValueLength;

  struct {
    uint16_t connectionHandle;
    uint8_t op;
    uint8_t* buffer;
    uint8_t length;
  } _pendingResp;

  BLEDeviceEventHandler _eventHandlers[2];
};

extern ATTClass ATT;

#endif
