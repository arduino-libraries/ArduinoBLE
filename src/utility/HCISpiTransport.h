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

typedef enum BLEChip_s {
  SPBTLE_RF,
  SPBTLE_1S,
  BLUENRG_M2SP,
  BLUENRG_M0,
  BLUENRG_LP
} BLEChip_t;

#ifndef BLE_SPI_BYTE_ORDER
  #define BLE_SPI_BYTE_ORDER  MSBFIRST
#endif
#define BLE_MODULE_SPI_BUFFER_SIZE 128

class HCISpiTransportClass : public HCITransportInterface {
  public:
    HCISpiTransportClass(SPIClass &spi, BLEChip_t ble_chip, uint8_t cs_pin, uint8_t spi_irq, uint8_t ble_rst, uint32_t frequency, uint8_t spi_mode);
    virtual ~HCISpiTransportClass();

    virtual int begin();
    virtual void end();

    virtual void wait(unsigned long timeout);

    virtual int available();
    virtual int peek();
    virtual int read();

    virtual size_t write(const uint8_t *data, size_t length);

  private:
    void wait_for_blue_initialize();
    void wait_for_enable_ll_only();
    void enable_ll_only();
    void wait_for_aci_gatt_init();
    void aci_gatt_init();
    void wait_for_aci_gap_init();
    void aci_gap_init();
    void wait_for_aci_read_config_parameter();
    void aci_read_config_parameter();
    void hci_reset();
    void set_address();
    void wait_for_set_address();
    SPIClass *_spi;
    SPISettings _spiSettings;
    BLEChip_t _ble_chip;
    uint8_t _cs_pin;
    uint8_t _spi_irq;
    uint8_t _ble_rst;
    uint8_t _rxbuff[BLE_MODULE_SPI_BUFFER_SIZE];
    uint16_t _read_index;
    uint16_t _write_index;
    uint16_t _write_index_initial;
    uint8_t _initial_phase;
    uint8_t _random_addr[6];
    bool _random_addr_done;
};

#endif /* _HCI_SPI_TRANSPORT_H_ */
