/*
  This file is part of the STM32duinoBLE library.
  Copyright (c) 2019 STMicroelectronics. All rights reserved.

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

#ifndef _HCI_SPI_TRANSPORT_H_
#define _HCI_SPI_TRANSPORT_H_

#include "HCITransport.h"
#include "SPI.h"

#if defined(ARDUINO_STEVAL_MKSBOX1V1)
#define STBTLE_1S /* STEVAL_MKSBOX1V1 */
#elif defined(ARDUINO_DISCO_L475VG_IOT)
#define STBTLE_RF /* B-L475E-IOT01A1 */
#else
#define STBTLE_RF /* Shield IDB05A1 */
#endif /* ARDUINO_STEVAL_MKSBOX1V1 */

#define BLE_MODULE_SPI_BUFFER_SIZE 128

class HCISpiTransportClass : public HCITransportInterface {
public:
  HCISpiTransportClass(SPIClass& spi, uint8_t cs_pin, uint8_t spi_irq, uint8_t ble_rst, unsigned long frequency, int spi_mode);
  virtual ~HCISpiTransportClass();

  virtual int begin();
  virtual void end();

  virtual void wait(unsigned long timeout);

  virtual int available();
  virtual int peek();
  virtual int read();

  virtual size_t write(const uint8_t* data, size_t length);

private:
#if defined(STBTLE_RF)
  void wait_for_blue_initialize();
  void wait_for_enable_ll_only();
  void enable_ll_only();
#endif /* STBTLE_RF */
  SPIClass* _spi;
  uint8_t _cs_pin;
  uint8_t _spi_irq;
  uint8_t _ble_rst;
  unsigned long _frequency;
  int _spi_mode;
  uint8_t _rxbuff[BLE_MODULE_SPI_BUFFER_SIZE];
  uint16_t _read_index;
  uint16_t _write_index;
  uint16_t _write_index_initial;
  uint8_t _initial_phase;
};

#endif /* _HCI_SPI_TRANSPORT_H_ */
