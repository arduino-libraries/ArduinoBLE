/*
  This file is part of the ArduinoBLE library.
  Copyright (c) 2018-2025 Arduino SA. All rights reserved.

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

#if defined(ARDUINO_UNO_Q)

#include "HCIVirtualTransportRPC.h"

HCIVirtualTransportRPCClass::HCIVirtualTransportRPCClass() : initialized(false) {
}

HCIVirtualTransportRPCClass::~HCIVirtualTransportRPCClass() {
}

int HCIVirtualTransportRPCClass::begin() {
  // Initialize the Bridge if not already started
  if (!Bridge) {
    if (!Bridge.begin()) {
      return 0;
    }
  }

  // Open HCI device through Bridge RPC
  if (!HCI.begin("hci0")) {
    return 0;
  }

  rxbuf.clear();
  initialized = true;

  return 1;
}

void HCIVirtualTransportRPCClass::end() {
  if (initialized) {
    HCI.end();
    initialized = false;
  }
}

void HCIVirtualTransportRPCClass::wait(unsigned long timeout) {
  delay(timeout);
}

int HCIVirtualTransportRPCClass::available() {
  if (!initialized) {
    return 0;
  }

  // Return buffered data if available
  if (rxbuf.available()) {
    return rxbuf.available();
  }

  // Check if data is available from Bridge HCI and buffer it
  if (HCI.available()) {
    uint8_t packet[258];
    int received = HCI.recv(packet, sizeof(packet));

    if (received > 0) {
      // Store received data in ring buffer
      for (int i = 0; i < received; i++) {
        rxbuf.store_char(packet[i]);
      }
    }
  }

  return rxbuf.available();
}

int HCIVirtualTransportRPCClass::peek() {
  return -1;
}

int HCIVirtualTransportRPCClass::read() {
  if (rxbuf.available()) {
    return rxbuf.read_char();
  }

  return -1;
}

size_t HCIVirtualTransportRPCClass::write(const uint8_t* data, size_t length) {
  if (!initialized) {
    return 0;
  }

  int sent = HCI.send(data, length);
  if (sent < 0) {
    return 0;
  }

  return sent;
}

HCIVirtualTransportRPCClass HCIVirtualTransportRPC;
HCITransportInterface& HCITransport = HCIVirtualTransportRPC;

#endif
