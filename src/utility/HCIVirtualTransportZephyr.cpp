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

#if defined(__ZEPHYR__)

#include "HCIVirtualTransportZephyr.h"

#include <zephyr/bluetooth/buf.h>
#include <zephyr/bluetooth/hci.h>
#include <zephyr/bluetooth/hci_raw.h>

static K_FIFO_DEFINE(rx_queue);
struct k_fifo* __rx_queue = &rx_queue;
extern "C" int bt_h4_vnd_setup(const struct device *dev);

HCIVirtualTransportZephyrClass::HCIVirtualTransportZephyrClass() {

}

HCIVirtualTransportZephyrClass::~HCIVirtualTransportZephyrClass() {

}

int HCIVirtualTransportZephyrClass::begin() {
  bt_enable_raw(__rx_queue);

#if CONFIG_BT_HCI_SETUP
  const struct device *uart = DEVICE_DT_GET(DT_PARENT(DT_CHOSEN(zephyr_bt_hci)));
  if (!uart) {
    return 0;
  }

  if (bt_h4_vnd_setup(uart)) {
    return 0;
  }
#endif /* CONFIG_BT_HCI_SETUP */

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
