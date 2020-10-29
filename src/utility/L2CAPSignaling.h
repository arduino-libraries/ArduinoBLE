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

#ifndef _L2CAP_SIGNALING_H_
#define _L2CAP_SIGNALING_H_

#include <Arduino.h>

#define SIGNALING_CID 0x0005

class L2CAPSignalingClass {
public:
  L2CAPSignalingClass();
  virtual ~L2CAPSignalingClass();

  virtual void addConnection(uint16_t handle, uint8_t role, uint8_t peerBdaddrType,
                    uint8_t peerBdaddr[6], uint16_t interval,
                    uint16_t latency, uint16_t supervisionTimeout,
                    uint8_t masterClockAccuracy);

  virtual void handleData(uint16_t connectionHandle, uint8_t dlen, uint8_t data[]);

  virtual void removeConnection(uint8_t handle, uint16_t reason);

  virtual void setConnectionInterval(uint16_t minInterval, uint16_t maxInterval);

  virtual void setSupervisionTimeout(uint16_t supervisionTimeout);

private:
  virtual void connectionParameterUpdateRequest(uint16_t handle, uint8_t identifier, uint8_t dlen, uint8_t data[]);
  virtual void connectionParameterUpdateResponse(uint16_t handle, uint8_t identifier, uint8_t dlen, uint8_t data[]);

private:
  uint16_t _minInterval;
  uint16_t _maxInterval;
  uint16_t _supervisionTimeout;
};

extern L2CAPSignalingClass& L2CAPSignaling;

#endif
