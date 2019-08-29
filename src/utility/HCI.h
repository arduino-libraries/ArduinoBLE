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

#ifndef _HCI_H_
#define _HCI_H_

#include <Arduino.h>

class HCIClass {
public:
  HCIClass();
  virtual ~HCIClass();

  int begin();
  void end();

  void poll();
  void poll(unsigned long timeout);

  int reset();
  int readLocalVersion(uint8_t& hciVer, uint16_t& hciRev, uint8_t& lmpVer,
                       uint16_t& manufacturer, uint16_t& lmpSubVer);
  int readBdAddr(uint8_t addr[6]);

  int readRssi(uint16_t handle);

  int setEventMask(uint64_t eventMask);

  int readLeBufferSize(uint16_t& pktLen, uint8_t& maxPkt);
  int leSetRandomAddress(uint8_t addr[6]);
  int leSetAdvertisingParameters(uint16_t minInterval, uint16_t maxInterval,
                                 uint8_t advType, uint8_t ownBdaddrType,
                                 uint8_t directBdaddrType, uint8_t directBdaddr[6],
                                 uint8_t chanMap,
                                 uint8_t filter);
  int leSetAdvertisingData(uint8_t length, uint8_t data[]);
  int leSetScanResponseData(uint8_t length, uint8_t data[]);
  int leSetAdvertiseEnable(uint8_t enable);
  int leSetScanParameters(uint8_t type, uint16_t interval, uint16_t window, 
                          uint8_t ownBdaddrType, uint8_t filter);
  int leSetScanEnable(uint8_t enabled, uint8_t duplicates);
  int leCreateConn(uint16_t interval, uint16_t window, uint8_t initiatorFilter,
                  uint8_t peerBdaddrType, uint8_t peerBdaddr[6], uint8_t ownBdaddrType,
                  uint16_t minInterval, uint16_t maxInterval, uint16_t latency,
                  uint16_t supervisionTimeout, uint16_t minCeLength, uint16_t maxCeLength);
  int leConnUpdate(uint16_t handle, uint16_t minInterval, uint16_t maxInterval, 
                  uint16_t latency, uint16_t supervisionTimeout);
  int leCancelConn();


  int sendAclPkt(uint16_t handle, uint8_t cid, uint8_t plen, void* data);

  int disconnect(uint16_t handle);

  void debug(Stream& stream);
  void noDebug();

private:
  int sendCommand(uint16_t opcode, uint8_t plen = 0, void* parameters = NULL);

  void handleAclDataPkt(uint8_t plen, uint8_t pdata[]);
  void handleNumCompPkts(uint16_t handle, uint16_t numPkts);
  void handleEventPkt(uint8_t plen, uint8_t pdata[]);

  void dumpPkt(const char* prefix, uint8_t plen, uint8_t pdata[]);

  Stream* _debug;

  int _recvIndex;
  uint8_t _recvBuffer[3 + 255];

  uint16_t _cmdCompleteOpcode;
  int _cmdCompleteStatus;
  uint8_t _cmdResponseLen;
  uint8_t* _cmdResponse;

  uint8_t _maxPkt;
  uint8_t _pendingPkt;

  uint8_t _aclPktBuffer[255];
};

extern HCIClass HCI;

#endif
