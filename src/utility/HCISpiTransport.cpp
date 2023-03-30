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

#include "HCISpiTransport.h"

volatile int data_avail = 0;

HCISpiTransportClass::HCISpiTransportClass(SPIClass &spi, BLEChip_t ble_chip, uint8_t cs_pin, uint8_t spi_irq, uint8_t ble_rst, uint32_t frequency, uint8_t spi_mode) :
  _spi(&spi),
  _ble_chip(ble_chip),
  _cs_pin(cs_pin),
  _spi_irq(spi_irq),
  _ble_rst(ble_rst)
{
  _spiSettings = SPISettings(frequency, (BitOrder)BLE_SPI_BYTE_ORDER, spi_mode);
  _read_index = 0;
  _write_index = 0;
  _write_index_initial = 0;
  _initial_phase = 1;
  _random_addr_done = false;
}

HCISpiTransportClass::~HCISpiTransportClass()
{
}

extern "C" void SPI_Irq_Callback(void)
{
  data_avail = 1;
}

int HCISpiTransportClass::begin()
{
  _read_index = 0;
  _write_index = 0;
  _write_index_initial = 0;
  _initial_phase = 1;
  memset(_rxbuff, 0, sizeof(_rxbuff));
  pinMode(_cs_pin, OUTPUT);
  digitalWrite(_cs_pin, HIGH);

  _spi->begin();

  pinMode(_spi_irq, INPUT);
  attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);

  // Reset chip
  pinMode(_ble_rst, OUTPUT);
  digitalWrite(_ble_rst, LOW);
  delay(5);
  digitalWrite(_ble_rst, HIGH);
  delay(5);

  if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0 || _ble_chip == BLUENRG_LP || _ble_chip == BLUENRG_M2SP) {
    // Wait for Blue Initialize
    wait_for_blue_initialize();
  } else if (_ble_chip == SPBTLE_1S) {
    // Wait a while for the reset of the BLE module
    delay(300);
  } else {
    // BLE chip not supported
    return 0;
  }

  return 1;
}

void HCISpiTransportClass::end()
{
  detachInterrupt(_spi_irq);
  _spi->end();
}

void HCISpiTransportClass::wait(unsigned long timeout)
{
  for (unsigned long start = millis(); (millis() - start) < timeout;) {
    if (available()) {
      break;
    }
  }
}

int HCISpiTransportClass::available()
{
  if (_ble_chip != SPBTLE_RF && _ble_chip != SPBTLE_1S && _ble_chip != BLUENRG_M2SP && _ble_chip != BLUENRG_M0 && _ble_chip != BLUENRG_LP) {
    return 0;
  }

  if (_read_index != _write_index) {
    return 1;
  } else if (data_avail) {
    int ble_reset = 0;

    if (digitalRead(_spi_irq) == 0) {
      return 0;
    }

    data_avail = 0;

    // Wait for BlueNRG-LP to be ready (needs to be done after each HCI RESET)
    if (_ble_chip == BLUENRG_LP && _initial_phase) {
      delay(100);
    }

    while (digitalRead(_spi_irq) == 1 && _write_index != BLE_MODULE_SPI_BUFFER_SIZE) {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        detachInterrupt(_spi_irq);
      }

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
        /* device is ready */
        if (header_master[0] == 0x02) {
          uint16_t byte_count = (header_master[4] << 8) | header_master[3];

          if (byte_count > 0) {
            if (_initial_phase) {
              /* avoid to read more data that available size of the buffer */
              if (byte_count > (BLE_MODULE_SPI_BUFFER_SIZE - _write_index_initial)) {
                byte_count = (BLE_MODULE_SPI_BUFFER_SIZE - _write_index_initial);
              }

              /* Read the response */
              for (int j = 0; j < byte_count; j++) {
                _rxbuff[_write_index_initial] = _spi->transfer(0x00);
                _write_index_initial++;
              }

              /* Check if the message is a Blue Initialize */
              /* If so we need to send the command to enable LL_ONLY */
              if (byte_count == 6) {
                if (_rxbuff[_write_index_initial - 6] == 0x04 &&
                    _rxbuff[_write_index_initial - 5] == 0xFF &&
                    _rxbuff[_write_index_initial - 4] == 0x03 &&
                    _rxbuff[_write_index_initial - 3] == 0x01 &&
                    _rxbuff[_write_index_initial - 2] == 0x00 &&
                    _rxbuff[_write_index_initial - 1] == 0x01) {
                  ble_reset = 1;
                }
              }
            } else {
              /* avoid to read more data that available size of the buffer */
              if (byte_count > (BLE_MODULE_SPI_BUFFER_SIZE - _write_index)) {
                byte_count = (BLE_MODULE_SPI_BUFFER_SIZE - _write_index);
                /* SPI buffer is full but we still have data to store, so we set the data_avail flag to true */
                data_avail = 1;
              }

              /* Read the response */
              for (int j = 0; j < byte_count; j++) {
                _rxbuff[_write_index] = _spi->transfer(0x00);
                _write_index++;
              }
            }
          }
        }
      } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        uint16_t byte_count = (header_master[4] << 8) | header_master[3];

        if (byte_count > 0) {
          if (_initial_phase) {
            /* avoid to read more data that available size of the buffer */
            if (byte_count > (BLE_MODULE_SPI_BUFFER_SIZE - _write_index_initial)) {
              byte_count = (BLE_MODULE_SPI_BUFFER_SIZE - _write_index_initial);
            }

            /* Read the response */
            for (int j = 0; j < byte_count; j++) {
              _rxbuff[_write_index_initial] = _spi->transfer(0x00);
              _write_index_initial++;
            }

            /* Check if the message is a CMD_COMPLETE */
            /* We suppose that the first CMD is always a HCI_RESET */
            if (byte_count == 7) {
              if (_rxbuff[_write_index_initial - 7] == 0x04 &&
                  _rxbuff[_write_index_initial - 6] == 0x0E &&
                  _rxbuff[_write_index_initial - 5] == 0x04 &&
                  _rxbuff[_write_index_initial - 4] == 0x01 &&
                  _rxbuff[_write_index_initial - 3] == 0x03 &&
                  _rxbuff[_write_index_initial - 2] == 0x0C &&
                  _rxbuff[_write_index_initial - 1] == 0x00) {
                ble_reset = 1;
              }
            }
          } else {
            /* avoid to read more data that available size of the buffer */
            if (byte_count > (BLE_MODULE_SPI_BUFFER_SIZE - _write_index)) {
              byte_count = (BLE_MODULE_SPI_BUFFER_SIZE - _write_index);
              /* SPI buffer is full but we still have data to store, so we set the data_avail flag to true */
              data_avail = 1;
            }

            /* Read the response */
            for (int j = 0; j < byte_count; j++) {
              _rxbuff[_write_index] = _spi->transfer(0x00);
              _write_index++;
            }
          }
        }
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
      }
    }

    if (ble_reset) {
      if (_ble_chip == BLUENRG_M2SP) {
        wait_for_blue_initialize();
      }

      if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0 || _ble_chip == BLUENRG_LP || _ble_chip == BLUENRG_M2SP) {
        /* BLE chip was reset: we need to enable LL_ONLY */
        enable_ll_only();
        wait_for_enable_ll_only();
      } else if (_ble_chip == SPBTLE_1S) {
        /* BLE chip was reset: we need to wait for a while */
        delay(300);
      }

      /* Call Gatt Init and Gap Init to activate the random BLE address */
      if (!_random_addr_done) {
        aci_gatt_init();
        wait_for_aci_gatt_init();
        aci_gap_init();
        wait_for_aci_gap_init();
        /* Call Read Config Parameter to retrieve the random BLE address */
        aci_read_config_parameter();
        wait_for_aci_read_config_parameter();
        if (_ble_chip == BLUENRG_LP) {
          hci_reset();
          _read_index = _write_index = _write_index_initial = 0;
          _initial_phase = 1;
        } else {
          /* Now we can update the write index and close the initial phase */
          _write_index = _write_index_initial;
          _initial_phase = 0;
          _write_index_initial = 0;
        }
      } else {
        set_address();
        wait_for_set_address();
        /* Now we can update the write index and close the initial phase */
        _write_index = _write_index_initial;
        _initial_phase = 0;
        _write_index_initial = 0;
      }
    }

    if (_read_index != _write_index) {
      return 1;
    } else {
      return 0;
    }
  } else {
    return 0;
  }
}

int HCISpiTransportClass::peek()
{
  int peek_val = -1;

  if (_read_index != _write_index) {
    peek_val = _rxbuff[_read_index];
  }

  return peek_val;
}

int HCISpiTransportClass::read()
{
  int read_val = -1;

  if (_read_index != _write_index) {
    read_val = _rxbuff[_read_index];
    _read_index++;
    if (_read_index == _write_index) {
      /* Reset buffer index */
      _read_index = 0;
      _write_index = 0;
    }
  }

  return read_val;
}

size_t HCISpiTransportClass::write(const uint8_t *data, size_t length)
{
  uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  void *my_data = (void *)data;
  int result = 0;
  uint32_t tickstart = millis();

  if (_ble_chip != SPBTLE_RF && _ble_chip != SPBTLE_1S && _ble_chip != BLUENRG_M2SP && _ble_chip != BLUENRG_M0 && _ble_chip != BLUENRG_LP) {
    return 0;
  }

  do {
    if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
      result = 0;

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      /* device is ready */
      if (header_master[0] == 0x02) {
        if (header_master[1] >= length) {
          /* Write the data */
          _spi->transfer(my_data, length);
        } else {
          result = -2;
        }
      } else {
        result = -1;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      if ((millis() - tickstart) > 1000) {
        result = -3;
        break;
      }
    } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
      uint32_t tickstart_data_available = millis();
      result = 0;

      detachInterrupt(_spi_irq);

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /*
       * Wait until BlueNRG-1 is ready.
       * When ready it will raise the IRQ pin.
       */
      while (!(digitalRead(_spi_irq) == 1)) {
        if ((millis() - tickstart_data_available) > 1000) {
          result = -3;
          break;
        }
      }

      if (result == -3) {
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
        break;
      }

      /* Write the header */
      _spi->transfer(header_master, 5);

      if ((int)((((uint16_t)header_master[2]) << 8) | ((uint16_t)header_master[1])) >= (int)length) {
        /* Write the data */
        _spi->transfer(my_data, length);
      } else {
        result = -2;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);

      if ((millis() - tickstart) > 1000) {
        result = -3;
        break;
      }
    }
  } while (result < 0);

  if (result < 0) {
    return 0;
  } else {
    return length;
  }
}

void HCISpiTransportClass::wait_for_blue_initialize()
{
  int event_blue_initialize = 0;
  uint8_t event[16];

  do {
    while (!data_avail);

    if (digitalRead(_spi_irq) == 0) {
      continue;
    }

    data_avail = 0;
    while (digitalRead(_spi_irq) == 1) {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        detachInterrupt(_spi_irq);
      }

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
        /* device is ready */
        if (header_master[0] == 0x02) {
          /* device is ready */
          uint16_t byte_count = (header_master[4] << 8) | header_master[3];

          if (byte_count > 0) {
            /* Read the response */
            if (byte_count == 6) {
              for (int j = 0; j < byte_count; j++) {
                event[j] = _spi->transfer(0x00);
              }

              if (event[0] == 0x04 &&
                  event[1] == 0xFF &&
                  event[2] == 0x03 &&
                  event[3] == 0x01 &&
                  event[4] == 0x00 &&
                  event[5] == 0x01) {
                event_blue_initialize = 1;
              }
            } else {
              for (int j = 0; j < byte_count; j++) {
                _spi->transfer(0x00);
              }
            }
          }
        }
      } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP) {
        uint16_t byte_count = (header_master[4] << 8) | header_master[3];

        if (byte_count > 0) {
          /* Read the response */
          if (byte_count == 6) {
            for (int j = 0; j < byte_count; j++) {
              event[j] = _spi->transfer(0x00);
            }

            if (event[0] == 0x04 &&
                event[1] == 0xFF &&
                event[2] == 0x03 &&
                event[3] == 0x01 &&
                event[4] == 0x00 &&
                event[5] == 0x01) {
              event_blue_initialize = 1;
            }
          } else {
            for (int j = 0; j < byte_count; j++) {
              _spi->transfer(0x00);
            }
          }
        }
      } else if (_ble_chip == BLUENRG_LP) {
        uint8_t byte_count = (header_master[4] << 8) | header_master[3];

        if (byte_count > 0) {
          /* Read the response */
          if (byte_count == 7) {
            for (int j = 0; j < byte_count; j++) {
              event[j] = _spi->transfer(0x00);
            }

            if (event[0] == 0x82 &&
                event[1] == 0xFF &&
                event[2] == 0x03 &&
                event[3] == 0x00 &&
                event[4] == 0x01 &&
                event[5] == 0x00 &&
                event[6] == 0x01) {
              event_blue_initialize = 1;
            }
          } else {
            for (int j = 0; j < byte_count; j++) {
              _spi->transfer(0x00);
            }
          }
        }
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
      }
    }
  } while (!event_blue_initialize);
}

void HCISpiTransportClass::wait_for_enable_ll_only()
{
  uint8_t data[8];
  int status = 0;

  do {
    while (!data_avail);

    if (digitalRead(_spi_irq) == 0) {
      continue;
    }

    data_avail = 0;
    while (digitalRead(_spi_irq) == 1) {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        detachInterrupt(_spi_irq);
      }

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
        /* device is ready */
        if (header_master[0] == 0x02) {
          /* device is ready */
          uint16_t byte_count = (header_master[4] << 8) | header_master[3];

          if (byte_count > 0) {
            /* Read the response */
            for (int j = 0; j < byte_count; j++) {
              data[j] = _spi->transfer(0x00);
            }

            if (byte_count >= 7) {
              if (data[0] == 0x04 &&
                  data[1] == 0x0E &&
                  data[2] == 0x04 &&
                  data[3] == 0x01 &&
                  data[4] == 0x0C &&
                  data[5] == 0xFC &&
                  data[6] == 0x00) {
                status = 1;
              }
            }
          }
        }
      } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        uint16_t byte_count = (header_master[4] << 8) | header_master[3];

        if (byte_count > 0) {
          /* Read the response */
          for (int j = 0; j < byte_count; j++) {
            data[j] = _spi->transfer(0x00);
          }

          if (byte_count >= 7) {
            if (data[0] == 0x04 &&
                data[1] == 0x0E &&
                data[2] == 0x04 &&
                data[3] == 0x01 &&
                data[4] == 0x0C &&
                data[5] == 0xFC &&
                data[6] == 0x00) {
              status = 1;
            }
          }
        }
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
      }
    }
  } while (!status);
}

void HCISpiTransportClass::enable_ll_only()
{
  uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  uint8_t cmd[7] = {0x01, 0x0C, 0xFC, 0x03, 0x2C, 0x01, 0x01}; // Enable LL_ONLY
  int result = 0;

  do {
    if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
      result = 0;

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      /* device is ready */
      if (header_master[0] == 0x02) {
        /* Write the data */
        if (header_master[1] >= 7) {
          /* Write the data */
          _spi->transfer((void *)cmd, 7);
        } else {
          result = -2;
        }
      } else {
        result = -1;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();
    } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
      uint32_t tickstart_data_available = millis();
      result = 0;

      detachInterrupt(_spi_irq);

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      while (!(digitalRead(_spi_irq) == 1)) {
        if ((millis() - tickstart_data_available) > 1000) {
          result = -3;
          break;
        }
      }

      if (result == -3) {
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
        break;
      }

      /* Write the header */
      _spi->transfer(header_master, 5);

      if ((int)((((uint16_t)header_master[2]) << 8) | ((uint16_t)header_master[1])) >= 7) {
        /* Write the data */
        _spi->transfer((void *)cmd, 7);
      } else {
        result = -2;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
    }
  } while (result < 0);
}

void HCISpiTransportClass::wait_for_aci_gatt_init()
{
  uint8_t data[8];
  int status = 0;

  do {
    while (!data_avail);

    if (digitalRead(_spi_irq) == 0) {
      continue;
    }

    data_avail = 0;
    while (digitalRead(_spi_irq) == 1) {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        detachInterrupt(_spi_irq);
      }

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
        /* device is ready */
        if (header_master[0] == 0x02) {
          /* device is ready */
          uint16_t byte_count = (header_master[4] << 8) | header_master[3];

          if (byte_count > 0) {
            /* Read the response */
            for (int j = 0; j < byte_count; j++) {
              data[j] = _spi->transfer(0x00);
            }

            if (byte_count >= 7) {
              if (data[0] == 0x04 &&
                  data[1] == 0x0E &&
                  data[2] == 0x04 &&
                  data[3] == 0x01 &&
                  data[4] == 0x01 &&
                  data[5] == 0xFD &&
                  data[6] == 0x00) {
                status = 1;
              }
            }
          }
        }
      } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        uint16_t byte_count = (header_master[4] << 8) | header_master[3];

        if (byte_count > 0) {
          /* Read the response */
          for (int j = 0; j < byte_count; j++) {
            data[j] = _spi->transfer(0x00);
          }

          if (byte_count >= 7) {
            if (data[0] == 0x04 &&
                data[1] == 0x0E &&
                data[2] == 0x04 &&
                data[3] == 0x01 &&
                data[4] == 0x01 &&
                data[5] == 0xFD &&
                data[6] == 0x00) {
              status = 1;
            }
          }
        }
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
      }
    }
  } while (!status);
}

void HCISpiTransportClass::aci_gatt_init()
{
  uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  uint8_t cmd[4] = {0x01, 0x01, 0xFD, 0x00}; // ACI_GATT_INIT
  int result = 0;

  do {
    if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
      result = 0;

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      /* device is ready */
      if (header_master[0] == 0x02) {
        /* Write the data */
        if (header_master[1] >= 4) {
          /* Write the data */
          _spi->transfer((void *)cmd, 4);
        } else {
          result = -2;
        }
      } else {
        result = -1;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();
    } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
      uint32_t tickstart_data_available = millis();
      result = 0;

      detachInterrupt(_spi_irq);

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      while (!(digitalRead(_spi_irq) == 1)) {
        if ((millis() - tickstart_data_available) > 1000) {
          result = -3;
          break;
        }
      }

      if (result == -3) {
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
        break;
      }

      /* Write the header */
      _spi->transfer(header_master, 5);

      if ((int)((((uint16_t)header_master[2]) << 8) | ((uint16_t)header_master[1])) >= 4) {
        /* Write the data */
        _spi->transfer((void *)cmd, 4);
      } else {
        result = -2;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
    }
  } while (result < 0);
}

void HCISpiTransportClass::wait_for_aci_gap_init()
{
  uint8_t data[14];
  int status = 0;

  do {
    while (!data_avail);

    if (digitalRead(_spi_irq) == 0) {
      continue;
    }

    data_avail = 0;
    while (digitalRead(_spi_irq) == 1) {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        detachInterrupt(_spi_irq);
      }

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
        /* device is ready */
        if (header_master[0] == 0x02) {
          /* device is ready */
          uint16_t byte_count = (header_master[4] << 8) | header_master[3];

          if (byte_count > 0) {
            /* Read the response */
            for (int j = 0; j < byte_count; j++) {
              data[j] = _spi->transfer(0x00);
            }

            if (byte_count >= 13) {
              if (data[0] == 0x04 &&
                  data[1] == 0x0E &&
                  data[2] == 0x0A &&
                  data[3] == 0x01 &&
                  data[4] == 0x8A &&
                  data[5] == 0xFC &&
                  data[6] == 0x00) {
                status = 1;
              }
            }
          }
        }
      } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        uint16_t byte_count = (header_master[4] << 8) | header_master[3];

        if (byte_count > 0) {
          /* Read the response */
          for (int j = 0; j < byte_count; j++) {
            data[j] = _spi->transfer(0x00);
          }

          if (byte_count >= 13) {
            if (data[0] == 0x04 &&
                data[1] == 0x0E &&
                data[2] == 0x0A &&
                data[3] == 0x01 &&
                data[4] == 0x8A &&
                data[5] == 0xFC &&
                data[6] == 0x00) {
              status = 1;
            }
          }
        }
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
      }
    }
  } while (!status);
}

void HCISpiTransportClass::aci_gap_init()
{
  uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  uint8_t cmd_lp[8] = {0x01, 0x8A, 0xFC, 0x04, 0x0F, 0x00, 0x00, 0x00}; // ACI_GAP_INIT
  uint8_t cmd_others[7] = {0x01, 0x8A, 0xFC, 0x03, 0x0F, 0x00, 0x00}; // ACI_GAP_INIT
  uint8_t *cmd, cmd_size;
  if (_ble_chip == BLUENRG_LP) {
    cmd = cmd_lp;
    cmd_size = 8;
  } else {
    cmd = cmd_others;
    cmd_size = 7;
  }
  int result = 0;

  do {
    if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
      result = 0;

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      /* device is ready */
      if (header_master[0] == 0x02) {
        /* Write the data */
        if (header_master[1] >= cmd_size) {
          /* Write the data */
          _spi->transfer((void *)cmd, cmd_size);
        } else {
          result = -2;
        }
      } else {
        result = -1;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();
    } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
      uint32_t tickstart_data_available = millis();
      result = 0;

      detachInterrupt(_spi_irq);

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      while (!(digitalRead(_spi_irq) == 1)) {
        if ((millis() - tickstart_data_available) > 1000) {
          result = -3;
          break;
        }
      }

      if (result == -3) {
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
        break;
      }

      /* Write the header */
      _spi->transfer(header_master, 5);

      if ((int)((((uint16_t)header_master[2]) << 8) | ((uint16_t)header_master[1])) >= cmd_size) {
        /* Write the data */
        _spi->transfer((void *)cmd, cmd_size);
      } else {
        result = -2;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
    }
  } while (result < 0);
}

void HCISpiTransportClass::wait_for_aci_read_config_parameter()
{
  uint8_t data[15];
  int status = 0;

  do {
    while (!data_avail);

    if (digitalRead(_spi_irq) == 0) {
      continue;
    }

    data_avail = 0;
    while (digitalRead(_spi_irq) == 1) {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        detachInterrupt(_spi_irq);
      }

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
        /* device is ready */
        if (header_master[0] == 0x02) {
          /* device is ready */
          uint16_t byte_count = (header_master[4] << 8) | header_master[3];

          if (byte_count > 0) {
            /* Read the response */
            for (int j = 0; j < byte_count; j++) {
              data[j] = _spi->transfer(0x00);
            }

            if (byte_count >= 13) {
              if (data[0] == 0x04 &&
                  data[1] == 0x0E &&
                  data[2] == 0x0A &&
                  data[3] == 0x01 &&
                  data[4] == 0x0D &&
                  data[5] == 0xFC &&
                  data[6] == 0x00) {
                memcpy(_random_addr, &data[7], 6);
                status = 1;
              }
            }
          }
        }
      } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        uint16_t byte_count = (header_master[4] << 8) | header_master[3];

        if (byte_count > 0) {
          /* Read the response */
          for (int j = 0; j < byte_count; j++) {
            data[j] = _spi->transfer(0x00);
          }

          if (byte_count >= 14) {
            if (data[0] == 0x04 &&
                data[1] == 0x0E &&
                data[2] == 0x0B &&
                data[3] == 0x01 &&
                data[4] == 0x0D &&
                data[5] == 0xFC &&
                data[6] == 0x00) {
              memcpy(_random_addr, &data[8], 6);
              status = 1;
              _random_addr_done = true;
            }
          }
        }
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
      }
    }
  } while (!status);
}

void HCISpiTransportClass::aci_read_config_parameter()
{
  uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  uint8_t cmd[5] = {0x01, 0x0D, 0xFC, 0x01, 0x80}; // ACI_READ_CONFIG_PARAMETER
  int result = 0;

  do {
    if (_ble_chip == SPBTLE_RF || _ble_chip == BLUENRG_M0) {
      result = 0;

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      /* device is ready */
      if (header_master[0] == 0x02) {
        /* Write the data */
        if (header_master[1] >= 5) {
          /* Write the data */
          _spi->transfer((void *)cmd, 5);
        } else {
          result = -2;
        }
      } else {
        result = -1;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();
    } else if (_ble_chip == SPBTLE_1S || _ble_chip == BLUENRG_M2SP || _ble_chip == BLUENRG_LP) {
      uint32_t tickstart_data_available = millis();
      result = 0;

      detachInterrupt(_spi_irq);

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      while (!(digitalRead(_spi_irq) == 1)) {
        if ((millis() - tickstart_data_available) > 1000) {
          result = -3;
          break;
        }
      }

      if (result == -3) {
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
        break;
      }

      /* Write the header */
      _spi->transfer(header_master, 5);

      if ((int)((((uint16_t)header_master[2]) << 8) | ((uint16_t)header_master[1])) >= 5) {
        /* Write the data */
        _spi->transfer((void *)cmd, 5);
      } else {
        result = -2;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
    }
  } while (result < 0);
}

void HCISpiTransportClass::hci_reset()
{
  uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  uint8_t cmd[4] = {0x01, 0x03, 0x0C, 0x00}; // HCI_RESET
  int result = 0;

  do {
    if (_ble_chip == BLUENRG_LP) {
      uint32_t tickstart_data_available = millis();
      result = 0;

      detachInterrupt(_spi_irq);

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      while (!(digitalRead(_spi_irq) == 1)) {
        if ((millis() - tickstart_data_available) > 1000) {
          result = -3;
          break;
        }
      }

      if (result == -3) {
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
        break;
      }

      /* Write the header */
      _spi->transfer(header_master, 5);

      if ((int)((((uint16_t)header_master[2]) << 8) | ((uint16_t)header_master[1])) >= 4) {
        /* Write the data */
        _spi->transfer((void *)cmd, 4);
      } else {
        result = -2;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
    }
  } while (result < 0);
}

void HCISpiTransportClass::set_address()
{
  uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  uint8_t cmd[10] = {0x01, 0x05, 0x20, 0x06}; // SET ADDR
  int result = 0;
  memcpy(&cmd[4], _random_addr, 6);

  do {
    if (_ble_chip == BLUENRG_LP) {
      uint32_t tickstart_data_available = millis();
      result = 0;

      detachInterrupt(_spi_irq);

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      while (!(digitalRead(_spi_irq) == 1)) {
        if ((millis() - tickstart_data_available) > 1000) {
          result = -3;
          break;
        }
      }

      if (result == -3) {
        digitalWrite(_cs_pin, HIGH);
        _spi->endTransaction();
        attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
        break;
      }

      /* Write the header */
      _spi->transfer(header_master, 5);

      if ((int)((((uint16_t)header_master[2]) << 8) | ((uint16_t)header_master[1])) >= 10) {
        /* Write the data */
        _spi->transfer((void *)cmd, 10);
      } else {
        result = -2;
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
    }
  } while (result < 0);
}

void HCISpiTransportClass::wait_for_set_address()
{
  uint8_t data[15];
  int status = 0;

  if (_ble_chip != BLUENRG_LP) {
    return;
  }

  do {
    while (!data_avail);

    if (digitalRead(_spi_irq) == 0) {
      continue;
    }

    data_avail = 0;
    while (digitalRead(_spi_irq) == 1) {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};
      uint16_t byte_count = 0;

      detachInterrupt(_spi_irq);

      _spi->beginTransaction(_spiSettings);

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      byte_count = (header_master[4] << 8) | header_master[3];

      if (byte_count > 0) {
        /* Read the response */
        for (int j = 0; j < byte_count; j++) {
          data[j] = _spi->transfer(0x00);
        }

        if (byte_count >= 7) { // 040E0401052000
          if (data[0] == 0x04 &&
              data[1] == 0x0E &&
              data[2] == 0x04 &&
              data[3] == 0x01 &&
              data[4] == 0x05 &&
              data[5] == 0x20 &&
              data[6] == 0x00) {
            status = 1;
          }
        }
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();

      attachInterrupt(_spi_irq, SPI_Irq_Callback, RISING);
    }
  } while (!status);
}
