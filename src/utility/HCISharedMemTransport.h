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

#ifndef _HCI_SHARED_MEM_TRANSPORT_H_
#define _HCI_SHARED_MEM_TRANSPORT_H_

#include "HCITransport.h"

/* STM32WB include files */
#include "stm32wbxx_ll_rcc.h"
#include "stm32wbxx_ll_ipcc.h"
#include "stm32wbxx_ll_system.h"
#include "STM32_WPAN/tl.h"
#include "STM32_WPAN/shci.h"
#include "STM32_WPAN/shci_tl.h"
#include "STM32_WPAN/app_conf.h"

/* this one is for printing info content when HW serial enabled */
//#define PRINT_IPCC_INFO

/* this CONFIG_DEBUG must be defined for -Og option */
//#define CONFIG_DEBUG

/******************************************************************************
 * BLE config parameters
 ******************************************************************************/
/*  Defined from WB Cube reference SW */
#define CFG_TLBLE_EVT_QUEUE_LENGTH 5
#define CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE 255   /**< Set to 255 with the memory manager and the mailbox */
#define TL_BLE_EVENT_FRAME_SIZE ( TL_EVT_HDR_SIZE + CFG_TLBLE_MOST_EVENT_PAYLOAD_SIZE )
#define POOL_SIZE (CFG_TLBLE_EVT_QUEUE_LENGTH*4*DIVC(( sizeof(TL_PacketHeader_t) + TL_BLE_EVENT_FRAME_SIZE ), 4))

#define CONFIG_DATA_PUBADDR_OFFSET          (0x00) /**< Bluetooth public address */
#define CONFIG_DATA_PUBADDR_LEN             (6)

#define BT_BUF_CMD 1
#define BT_BUF_ACL_OUT 2

/* timeout (in ms) to wait for an incoming event */
#define BLE_IPCC_TIMEOUT 10000

/* to received BLE packet from the SharedMem */
void evt_received(TL_EvtPacket_t *hcievt);

/* to send BLE packet to the SharedMem */
uint16_t mbox_write(uint8_t type, uint16_t len, const uint8_t *pData);

class HCISharedMemTransportClass : public HCITransportInterface {
  public:
    HCISharedMemTransportClass();
    virtual ~HCISharedMemTransportClass();

    virtual int begin();
    virtual void end();

    virtual void wait(unsigned long timeout);

    virtual int available();
    virtual int peek();
    virtual int read();

    virtual size_t write(const uint8_t *data, size_t length);

  private:

    /* method to initialize the BLE device */
    void transport_init(void);
    void start_ble_rf(void);
    void stm32wb_reset(void);
    int stm32wb_start_ble(void);
    int bt_ipm_set_addr(void);
    int bt_ipm_set_power(void);

    uint8_t _random_addr[6];
};

#endif /* _HCI_SHARED_MEM_TRANSPORT_H_ */
