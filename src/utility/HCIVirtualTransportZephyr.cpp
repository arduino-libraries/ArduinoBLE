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

#if defined(__ZEPHYR__) && !defined(ARDUINO_UNO_Q)

#include "HCIVirtualTransportZephyr.h"
#include "HCI.h"

#include <zephyr/bluetooth/buf.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_raw.h>
#include <zephyr/bluetooth/controller.h>
#include <zephyr/drivers/uart.h>
#include <zephyr/drivers/hwinfo.h>

static K_FIFO_DEFINE(rx_queue);
struct k_fifo* __rx_queue = &rx_queue;
extern "C" int bt_h4_vnd_setup(const struct device *dev);

#if CONFIG_CYW4343W_MURATA_1DX
// External firmware defines.
extern const uint8_t brcm_patchram_buf[];
extern const int brcm_patch_ram_length;

enum {
  HCI_VND_OP_FW_DOWNLOAD  = 0xFC2E,
  HCI_VND_OP_WRITE_RAM    = 0xFC4C,
  HCI_VND_OP_LAUNCH_RAM   = 0xFC4E,
  HCI_VND_OP_SET_BAUDRATE = 0xFC18,
};

static int cyw4343_set_baudrate(const struct device *uart, uint32_t baudrate) {
    struct __attribute__ ((packed)) {
      uint16_t zero;
      uint32_t baud;
    } param = { 0, baudrate };

	struct uart_config uart_cfg;
	if (uart_config_get(uart, &uart_cfg)) {
		return -1;
	}

    if (HCI.sendCommand(HCI_VND_OP_SET_BAUDRATE, sizeof(param), (void *) &param)) {
      return -1;
    }

    uart_cfg.baudrate = baudrate;
    if (uart_configure(uart, &uart_cfg)) {
        return -1;
    }

    uart_irq_rx_enable(uart);
    return 0;
}

static int cyw4343_download_firmware(const struct device *uart) {
  #define MURATA_NODE DT_CHILD(DT_CHOSEN(zephyr_bt_hci), murata_1dx)
  uint32_t operational_speed = DT_PROP_OR(MURATA_NODE, hci_operation_speed, 115200);
  uint32_t fw_download_speed = DT_PROP_OR(MURATA_NODE, fw_download_speed, 115200);

  // Reset controller
  if (HCI.sendCommand(0x0C03, 0, NULL)) {
    return -1;
  }

  // Switch to fast baudrate
  if ((operational_speed != fw_download_speed) && 
      cyw4343_set_baudrate(uart, fw_download_speed)) {
      return -1;
  }

  // Start firmware downloader.
  if (HCI.sendCommand(HCI_VND_OP_FW_DOWNLOAD, 0, NULL)) {
    return -1;
  }

  // Load the firmware image.
  for (int offset=0; offset < brcm_patch_ram_length;) {
    uint8_t  length = brcm_patchram_buf[offset + 2];
    uint16_t opcode = (brcm_patchram_buf[offset + 0]) |
                      (brcm_patchram_buf[offset + 1] << 8);

    // Opcode should be write RAM or launch RAM.
    if (opcode != HCI_VND_OP_WRITE_RAM && opcode != HCI_VND_OP_LAUNCH_RAM) {
      return -1;
    }

    if (HCI.sendCommand(opcode, length, (void *) &brcm_patchram_buf[offset + 3])) {
      return -1;
    }

    offset += length + 3;
  }

  // Delay after firmware loading.
  delay(250);

  // Switch back to the default baudrate.
  if ((operational_speed != fw_download_speed) && 
      cyw4343_set_baudrate(uart, operational_speed)) {
      return -1;
  }

  return 0;
}
#endif

HCIVirtualTransportZephyrClass::HCIVirtualTransportZephyrClass() {

}

HCIVirtualTransportZephyrClass::~HCIVirtualTransportZephyrClass() {

}

int HCIVirtualTransportZephyrClass::begin() {
  k_fifo_init(&rx_queue);
  bt_enable_raw(__rx_queue);

#if CONFIG_BT_HCI_SETUP
  const struct device *uart = DEVICE_DT_GET(DT_PARENT(DT_CHOSEN(zephyr_bt_hci)));
  if (!uart) {
    return 0;
  }

  if (bt_h4_vnd_setup(uart)) {
    return 0;
  }

#if CONFIG_CYW4343W_MURATA_1DX
  if (cyw4343_download_firmware(uart)) {
    return 0;
  }
#endif /* CONFIG_CYW4343W_MURATA_1DX */
#endif /* CONFIG_BT_HCI_SETUP */

#if CONFIG_BT_LL_SW_SPLIT
  // Use unique device id for BD addr.
  uint8_t bd_addr[6] = { 0xFA, 0xFA, 0xFA, 0xFA, 0xFA, 0xFA };

#if CONFIG_HWINFO
  hwinfo_get_device_id(bd_addr, sizeof(bd_addr));
#endif  /* CONFIG_HWINFO */

  // Set public address for controller.
  bt_ctlr_set_public_addr(bd_addr);
#endif  /* CONFIG_BT_LL_SW_SPLIT */

  rxbuf.clear();

  return 1;
}

void HCIVirtualTransportZephyrClass::end() {

}

void HCIVirtualTransportZephyrClass::wait(unsigned long timeout) {
  delay(timeout);
}

int HCIVirtualTransportZephyrClass::available() {
  static struct net_buf *buf = NULL;

  if (rxbuf.available()) {
    return rxbuf.available();
  }

  buf = (struct net_buf *) k_fifo_get(__rx_queue, K_MSEC(10));
  if (!buf) {
    return 0;
  }

  for (int i=0; i<buf->len; i++) {
    rxbuf.store_char((uint8_t)buf->data[i]);
  }

  net_buf_pull(buf, buf->len);
  if (!buf->len) {
    net_buf_unref(buf);
    buf = NULL;
  }

  return rxbuf.available();
}

int HCIVirtualTransportZephyrClass::peek() {
  return -1;
}

int HCIVirtualTransportZephyrClass::read() {
  if (rxbuf.available()) {
    return rxbuf.read_char();
  }

  return -1;
}

size_t HCIVirtualTransportZephyrClass::write(const uint8_t* data, size_t length) {
  enum bt_buf_type type = bt_buf_type_from_h4(data[0], BT_BUF_OUT);
  struct net_buf *buf = bt_buf_get_tx(type, K_FOREVER, &data[1], length - 1);

  if (buf) {
    auto err = bt_send(buf);
    if (err) {
    	net_buf_unref(buf);
    }
    return length;
  }
  return 0;
}

HCIVirtualTransportZephyrClass HCIVirtualTransportZephyr;
HCITransportInterface& HCITransport = HCIVirtualTransportZephyr;
#endif
