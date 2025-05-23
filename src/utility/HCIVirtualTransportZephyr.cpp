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
extern "C" int bt_h4_vnd_setup(const struct device *dev);
struct h4_config {
       const struct device *uart;
       k_thread_stack_t *rx_thread_stack;
       size_t rx_thread_stack_size;
       struct k_thread *rx_thread;
};

int HCIVirtualTransportZephyrClass::begin()
{
  bt_enable_raw(__rx_queue);
#if defined(CONFIG_BT_HCI_SETUP)
  const struct device *dev = DEVICE_DT_GET(DT_CHOSEN(zephyr_bt_hci));
  if (!dev) {
    return 0;
  }
  const struct h4_config *cfg = (const struct h4_config *)dev->config;
  bt_h4_vnd_setup(cfg->uart);
#else
  set_public_address(__rx_queue);
#endif

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
  buf = (struct net_buf*)k_fifo_get(__rx_queue, K_MSEC(10));
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
  struct net_buf *__net_buf = bt_buf_get_tx(bt_buf_type_from_h4(data[0], BT_BUF_OUT), K_FOREVER, &data[1], length - 1);
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
