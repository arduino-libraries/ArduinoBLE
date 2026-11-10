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

#if defined(ESP32)

#include "HCIVirtualTransport.h"

// On targets using the new ESP-IDF HCI layer (C5, C6, H2) with NimBLE as host,
// the esp_vhci_host_* API is not available: talk to the HCI transport directly.
#if CONFIG_BT_LE_HCI_INTERFACE_USE_RAM && CONFIG_BT_NIMBLE_ENABLED
#define HCI_ESP_TRANSPORT
#include "esp_hci_transport.h"
#include "esp_hci_internal.h"
#endif

StreamBufferHandle_t rec_buffer;

#if defined(HCI_ESP_TRANSPORT)

static int notify_host_recv(hci_trans_pkt_ind_t type, uint8_t *data, uint16_t len)
{
  static uint8_t pkt[1 + 258];
  uint16_t length;

  (void)len;

  if (type == HCI_EVT_IND) {
    // Event buffer: event code, parameter length, parameters
    length = data[1] + 2;
    memcpy(&pkt[1], data, length);
    r_ble_hci_trans_buf_free(data);
  } else if (type == HCI_ACL_IND) {
    struct os_mbuf *om = (struct os_mbuf *)data;
    length = OS_MBUF_PKTHDR(om)->omp_len;
    if (length > sizeof(pkt) - 1) {
      length = sizeof(pkt) - 1;
    }
    os_mbuf_copydata(om, 0, length, &pkt[1]);
    os_mbuf_free_chain(om);
  } else {
    return 0;
  }

  pkt[0] = type;
  xStreamBufferSend(rec_buffer, pkt, length + 1, portMAX_DELAY);  // !!!potentially waiting forever
  return 0;
}

#else

StreamBufferHandle_t send_buffer;
TaskHandle_t bleHandle;

static void notify_host_send_available(void)
{
}

static int notify_host_recv(uint8_t *data, uint16_t length)
{
  xStreamBufferSend(rec_buffer,data,length,portMAX_DELAY);  // !!!potentially waiting forever
  return 0;
}

static esp_vhci_host_callback_t vhci_host_cb = {
  notify_host_send_available,
  notify_host_recv
};

void bleTask(void *pvParameters)
{
  esp_vhci_host_register_callback(&vhci_host_cb);
  size_t length;
  uint8_t mybuf[258];

  while(true){
    length = xStreamBufferReceive(send_buffer,mybuf,258,portMAX_DELAY);
    while (!esp_vhci_host_check_send_available()) {}
    esp_vhci_host_send_packet(mybuf, length);
  }
}

#endif


HCIVirtualTransportClass::HCIVirtualTransportClass()
{
}

HCIVirtualTransportClass::~HCIVirtualTransportClass()
{
}

int HCIVirtualTransportClass::begin()
{
  btStarted(); // this somehow stops the arduino ide from initializing bluedroid

  rec_buffer = xStreamBufferCreate(258, 1);
#if !defined(HCI_ESP_TRANSPORT)
  send_buffer = xStreamBufferCreate(258, 1);
#endif

  esp_err_t ret = nvs_flash_init();
  if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
    ESP_ERROR_CHECK(nvs_flash_erase());
    ret = nvs_flash_init();
  }
  esp_bt_controller_config_t bt_cfg = BT_CONTROLLER_INIT_CONFIG_DEFAULT();
  
#if CONFIG_IDF_TARGET_ESP32
  bt_cfg.mode = ESP_BT_MODE_BLE; //original esp32 chip
#else
#if !(CONFIG_IDF_TARGET_ESP32C2 || CONFIG_IDF_TARGET_ESP32C5 || CONFIG_IDF_TARGET_ESP32C6 || CONFIG_IDF_TARGET_ESP32H2)
  bt_cfg.bluetooth_mode = ESP_BT_MODE_BLE; //different api for newer models
#endif
#endif

  esp_bt_controller_mem_release(ESP_BT_MODE_CLASSIC_BT);
  esp_bt_controller_init(&bt_cfg);
  esp_bt_controller_enable(ESP_BT_MODE_BLE);
#if defined(HCI_ESP_TRANSPORT)
  hci_transport_host_callback_register(notify_host_recv);
#else
  xTaskCreatePinnedToCore(&bleTask, "bleTask", 2048, NULL, 5, &bleHandle, 0);
#endif
  return 1;
}

void HCIVirtualTransportClass::end()
{
#if defined(HCI_ESP_TRANSPORT)
  hci_transport_host_callback_register(NULL);
  vStreamBufferDelete(rec_buffer);
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
#else
  vStreamBufferDelete(rec_buffer);
  vStreamBufferDelete(send_buffer);
  esp_bt_controller_disable();
  esp_bt_controller_deinit();
  vTaskDelete(bleHandle);
#endif
}

void HCIVirtualTransportClass::wait(unsigned long timeout)
{
  for (unsigned long start = (esp_timer_get_time() / 1000ULL); ((esp_timer_get_time() / 1000ULL) - start) < timeout;) {
    if (available()) {
      break;
    }
  }
}

int HCIVirtualTransportClass::available()
{
  size_t bytes	= xStreamBufferBytesAvailable(rec_buffer);
  return bytes;
}

// never called
int HCIVirtualTransportClass::peek()
{
  return -1;
}

int HCIVirtualTransportClass::read()
{
  uint8_t c;
  if(xStreamBufferReceive(rec_buffer, &c, 1, portMAX_DELAY)) {
    return c;
  }
  return -1;
}

size_t HCIVirtualTransportClass::write(const uint8_t* data, size_t length)
{
#if defined(HCI_ESP_TRANSPORT)
  // The controller expects packets without the H4 indicator, commands in a
  // controller allocated buffer and ACL data in an mbuf.
  if (length < 2) {
    return 0;
  }

  if (data[0] == HCI_CMD_IND) {
    if (length - 1 > HCI_TRANSPORT_CMD_SZ) {
      return 0;
    }
    uint8_t *cmd = r_ble_hci_trans_buf_alloc(ESP_HCI_INTERNAL_BUF_CMD);
    if (cmd == NULL) {
      return 0;
    }
    memcpy(cmd, &data[1], length - 1);
    if (hci_transport_host_cmd_tx(cmd, length - 1) != 0) {
      return 0;
    }
  } else if (data[0] == HCI_ACL_IND) {
    struct os_mbuf *om = os_msys_get_pkthdr(length - 1, ESP_HCI_INTERNAL_ACL_MBUF_LEADINGSPCAE);
    if (om == NULL) {
      return 0;
    }
    if (os_mbuf_append(om, &data[1], length - 1) != 0) {
      os_mbuf_free_chain(om);
      return 0;
    }
    if (hci_transport_host_acl_tx((uint8_t *)om, length - 1) != 0) {
      return 0;
    }
  } else {
    return 0;
  }

  return length;
#else
  size_t result = xStreamBufferSend(send_buffer,data,length,portMAX_DELAY);
  return result;
#endif
}

HCIVirtualTransportClass HCIVirtualTransport;

HCITransportInterface& HCITransport = HCIVirtualTransport;

#endif
