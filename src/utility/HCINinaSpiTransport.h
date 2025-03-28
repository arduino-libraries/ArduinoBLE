/*
  This file is part of the ArduinoBLE library.

  Copyright (c) 2024 Arduino SA

  This Source Code Form is subject to the terms of the Mozilla Public
  License, v. 2.0. If a copy of the MPL was not distributed with this
  file, You can obtain one at http://mozilla.org/MPL/2.0/.
*/

#ifndef _HCI_NINA_SPI_TRANSPORT_H_
#define _HCI_NINA_SPI_TRANSPORT_H_

#include "HCITransport.h"

class BleDrv
{
public:
    static int bleBegin();
    static void bleEnd();
    static int bleAvailable();
    static int bleRead(uint8_t data[], size_t length);
    static int blePeek(uint8_t data[], size_t length);
    static size_t bleWrite(const uint8_t* data, size_t length);
};

class HCINinaSpiTransportClass : public HCITransportInterface {
public:
  HCINinaSpiTransportClass() {}

  virtual int begin();
  virtual void end();

  virtual void wait(unsigned long timeout);

  virtual int available();
  virtual int peek();
  virtual int read();

  virtual size_t write(const uint8_t* data, size_t length);
};

#endif
