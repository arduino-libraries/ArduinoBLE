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

#if defined(__ZEPHYR__)

#include "HCIVirtualTransportZephyr.h"

extern "C" struct k_fifo* __rx_queue;

HCIVirtualTransportZephyrClass::HCIVirtualTransportZephyrClass()
{
}

HCIVirtualTransportZephyrClass::~HCIVirtualTransportZephyrClass()
{
}

extern "C" void set_public_address(struct k_fifo *rx_queue);
int HCIVirtualTransportZephyrClass::begin()
{
  bt_enable_raw(__rx_queue);
  //bt_hci_raw_set_mode(BT_HCI_RAW_MODE_PASSTHROUGH);
  set_public_address(__rx_queue);
  bt_hci_raw_set_mode(BT_HCI_RAW_MODE_H4);
  rxbuf.clear();
  return 1;
}

void HCIVirtualTransportZephyrClass::end()
{
}

void HCIVirtualTransportZephyrClass::wait(unsigned long timeout)
{
  delay(timeout);
}

int HCIVirtualTransportZephyrClass::available()
{
  if (rxbuf.available()) {
    return rxbuf.available();
  }

  static struct net_buf *buf;
	buf = net_buf_get(__rx_queue, K_MSEC(10));
  if (!buf) {
    return 0;
  }

  for (int i = 0; i < buf->len; i++) {
    rxbuf.store_char((uint8_t)buf->data[i]);
  }
  net_buf_pull(buf, buf->len);
  if (!buf->len) {
		net_buf_unref(buf);
		buf = NULL;
	}

  return rxbuf.available();
}

// never called
int HCIVirtualTransportZephyrClass::peek()
{
  return -1;
}

int HCIVirtualTransportZephyrClass::read()
{

  if (rxbuf.available()) {
    return rxbuf.read_char();
  }

  return -1;
}

size_t HCIVirtualTransportZephyrClass::write(const uint8_t* data, size_t length)
{
  struct net_buf *__net_buf = bt_buf_get_tx(BT_BUF_H4, K_MSEC(10), data, length);
  if (__net_buf) {
    auto err = bt_send(__net_buf);
		if (err) {
			net_buf_unref(__net_buf);
		}
    return length;
  }
  return 0;
}

HCIVirtualTransportZephyrClass HCIVirtualTransportZephyr;

HCITransportInterface& HCITransport = HCIVirtualTransportZephyr;

#endif
