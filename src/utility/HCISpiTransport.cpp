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

#ifdef ARDUINO_ARCH_STM32

#include "HCISpiTransport.h"

#if defined(ARDUINO_STEVAL_MKSBOX1V1)
SPIClass SpiHCI(PC3, PD3, PD1); /* STEVAL_MKSBOX1V1 */
#elif defined(ARDUINO_DISCO_L475VG_IOT)
SPIClass SpiHCI(PC12, PC11, PC10); /* B-L475E-IOT01A1 */
#else
SPIClass SpiHCI(D11, D12, D3); /* Shield IDB05A1 */
#endif /* ARDUINO_STEVAL_MKSBOX1V1 */
volatile int data_avail = 0;

HCISpiTransportClass::HCISpiTransportClass(SPIClass& spi, uint8_t cs_pin, uint8_t spi_irq, uint8_t ble_rst, unsigned long frequency, int spi_mode) :
  _spi(&spi),
  _cs_pin(cs_pin),
  _spi_irq(spi_irq),
  _ble_rst(ble_rst),
  _frequency(frequency),
  _spi_mode(spi_mode)
{
  _read_index = 0;
  _write_index = 0;
  _write_index_initial = 0;
  _initial_phase = 1;
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

#if defined(STBTLE_RF)
  // Wait for Blue Initialize
  wait_for_blue_initialize();
#endif /* STBTLE_RF */

#if defined(STBTLE_1S)
  // Wait a while for the reset of the BLE module
  delay(300);
#endif /* STBTLE_1S */

  return 1;
}

void HCISpiTransportClass::end()
{
  detachInterrupt(_spi_irq);
  _spi->end();
}

void HCISpiTransportClass::wait(unsigned long timeout)
{
  for (unsigned long start = millis(); (millis() - start) < timeout;)
  {
    if (available())
    {
      break;
    }
  }
}

int HCISpiTransportClass::available()
{
  if(_read_index != _write_index)
  {
    return 1;
  } else if(data_avail)
  {
    int ble_reset = 0;

    if(digitalRead(_spi_irq) == 0)
    {
      return 0;
    }

    data_avail = 0;

    while(digitalRead(_spi_irq) == 1 && _write_index != BLE_MODULE_SPI_BUFFER_SIZE)
    {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};

      _spi->beginTransaction(SPISettings(_frequency, MSBFIRST, _spi_mode));

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

#if defined(STBTLE_RF) 
      /* device is ready */
      if(header_master[0] == 0x02) 
      {
#endif /* STBTLE_RF */
        uint16_t byte_count = (header_master[4] << 8) | header_master[3];

        if(byte_count > 0)
        {
          if(_initial_phase)
          {
            /* avoid to read more data that available size of the buffer */
            if(byte_count > (BLE_MODULE_SPI_BUFFER_SIZE - _write_index_initial))
            {
              byte_count = (BLE_MODULE_SPI_BUFFER_SIZE - _write_index_initial);
            }

            /* Read the response */
            for(int j=0; j < byte_count; j++)
            {
              _rxbuff[_write_index_initial] = _spi->transfer(0xFF);
              _write_index_initial++;
            }

#if defined(STBTLE_RF)
            /* Check if the message is a Blue Initialize */
            /* If so we need to send the command to enable LL_ONLY */
            if(byte_count == 6)
            {
              if(_rxbuff[_write_index_initial - 6] == 0x04 &&
                 _rxbuff[_write_index_initial - 5] == 0xFF &&
                 _rxbuff[_write_index_initial - 4] == 0x03 &&
                 _rxbuff[_write_index_initial - 3] == 0x01 &&
                 _rxbuff[_write_index_initial - 2] == 0x00 &&
                 _rxbuff[_write_index_initial - 1] == 0x01)
              {
                ble_reset = 1;
              }
            }
#endif /* STBTLE_RF */
#if defined(STBTLE_1S)
            /* Check if the message is a CMD_COMPLETE */
            /* We suppose that the first CMD is always a HCI_RESET */
            if(byte_count == 7)
            {
              if(_rxbuff[_write_index_initial - 7] == 0x04 &&
                 _rxbuff[_write_index_initial - 6] == 0x0E && 
                 _rxbuff[_write_index_initial - 5] == 0x04 &&
                 _rxbuff[_write_index_initial - 4] == 0x01 &&
                 _rxbuff[_write_index_initial - 3] == 0x03 &&
                 _rxbuff[_write_index_initial - 2] == 0x0C &&
                 _rxbuff[_write_index_initial - 1] == 0x00)
              {
                ble_reset = 1;
              }
            }
#endif /* STBTLE_1S */
          } else
          {
            /* avoid to read more data that available size of the buffer */
            if(byte_count > (BLE_MODULE_SPI_BUFFER_SIZE - _write_index))
            {
              byte_count = (BLE_MODULE_SPI_BUFFER_SIZE - _write_index);
              /* SPI buffer is full but we still have data to store, so we set the data_avail flag to true */
              data_avail = 1;
            }

            /* Read the response */
            for(int j=0; j < byte_count; j++)
            {
              _rxbuff[_write_index] = _spi->transfer(0xFF);
              _write_index++;
            }
          }
        }
#if defined(STBTLE_RF)
      }
#endif /* STBTLE_RF */

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();
    }

    if(ble_reset)
    {
#if defined(STBTLE_RF)
      /* BLE chip was reset: we need to enable LL_ONLY */
      enable_ll_only();
      wait_for_enable_ll_only();
#endif /* STBTLE_RF */
#if defined(STBTLE_1S)
      /* BLE chip was reset: we need to wait for a while */
      delay(300);
#endif /* STBTLE_1S */

      /* Now we can update the write index and close the initial phase */
      _write_index = _write_index_initial;
      _initial_phase = 0;
      _write_index_initial = 0;
    }

    if(_read_index != _write_index)
    {
      return 1;
    } else
    {
      return 0;
    }
  } else
  {
    return 0;
  }
}

int HCISpiTransportClass::peek()
{
  int peek_val = -1;

  if(_read_index != _write_index)
  {
    peek_val = _rxbuff[_read_index];
  }

  return peek_val;
}

int HCISpiTransportClass::read()
{
  int read_val = -1;

  if(_read_index != _write_index)
  {
    read_val = _rxbuff[_read_index];
    _read_index++;
    if(_read_index == _write_index)
    {
      /* Reset buffer index */
      _read_index = 0;
      _write_index = 0;
    }
  }

  return read_val;
}

size_t HCISpiTransportClass::write(const uint8_t* data, size_t length)
{
  uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  void *my_data = (void *)data;
  int result = 0;
  uint32_t tickstart = millis();

  do
  {
#if defined(STBTLE_1S)
    uint32_t tickstart_data_available = millis();
#endif /* STBTLE_1S */
    result = 0;

    _spi->beginTransaction(SPISettings(_frequency, MSBFIRST, _spi_mode));

    digitalWrite(_cs_pin, LOW);

#if defined(STBTLE_1S)
    /*
     * Wait until BlueNRG-1 is ready.
     * When ready it will raise the IRQ pin.
     */
    while(!(digitalRead(_spi_irq) == 1))
    {
      if((millis() - tickstart_data_available) > 1000)
      {
        result = -3;
        break;
      }
    }

    if(result == -3)
    {
      digitalWrite(_cs_pin, HIGH);
      _spi->endTransaction();
      break;
    }
#endif /* STBTLE_1S */

    /* Write the header */
    _spi->transfer(header_master, 5);

#if defined(STBTLE_RF)
    /* device is ready */
    if(header_master[0] == 0x02) 
    {
      if(header_master[1] >= length)
#endif /* STBTLE_RF */
#if defined(STBTLE_1S)
      if((int)((((uint16_t)header_master[2])<<8) | ((uint16_t)header_master[1])) >= (int)length)
#endif /* STBTLE_1S */
      {
        /* Write the data */
        _spi->transfer(my_data, length);
      } else
      {
        result = -2;
      }
#if defined(STBTLE_RF)
    } else
    {
      result = -1;
    }
#endif /* STBTLE_RF */

    digitalWrite(_cs_pin, HIGH);

    _spi->endTransaction();

    if((millis() - tickstart) > 1000)
    {
      result = -3;
      break;
    }
  } while(result < 0);

  if(result < 0)
  {
    return 0;
  } else
  {
    return length;
  }
}

#if defined(STBTLE_RF)
void HCISpiTransportClass::wait_for_blue_initialize()
{
  int event_blue_initialize = 0;
  uint8_t event[16];

  do
  {
    while(!data_avail);

    if(digitalRead(_spi_irq) == 0)
    {
      continue;
    }

    data_avail = 0;
    while(digitalRead(_spi_irq) == 1)
    {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};

      _spi->beginTransaction(SPISettings(_frequency, MSBFIRST, _spi_mode));

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      /* device is ready */
      if(header_master[0] == 0x02) 
      {
        /* device is ready */
        uint16_t byte_count = (header_master[4] << 8) | header_master[3];

        if(byte_count > 0)
        {
          /* Read the response */
          if(byte_count == 6)
          {
            for(int j=0; j < byte_count; j++)
            {
              event[j] = _spi->transfer(0xFF);
            }

            if(event[0] == 0x04 && 
               event[1] == 0xFF &&
               event[2] == 0x03 &&
               event[3] == 0x01 &&
               event[4] == 0x00 &&
               event[5] == 0x01)
            {
              event_blue_initialize = 1;
            }
          } else
          {
            for(int j=0; j < byte_count; j++)
            {
              _spi->transfer(0xFF);
            }
          }
        }
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();
    }
  } while(!event_blue_initialize);
}

void HCISpiTransportClass::wait_for_enable_ll_only()
{
  uint8_t data[8];
  int status = 0;

  do
  {
    while(!data_avail);

    if(digitalRead(_spi_irq) == 0)
    {
      continue;
    }

    data_avail = 0;
    while(digitalRead(_spi_irq) == 1)
    {
      uint8_t header_master[5] = {0x0b, 0x00, 0x00, 0x00, 0x00};

      _spi->beginTransaction(SPISettings(_frequency, MSBFIRST, _spi_mode));

      digitalWrite(_cs_pin, LOW);

      /* Write the header */
      _spi->transfer(header_master, 5);

      /* device is ready */
      if(header_master[0] == 0x02) 
      {
        /* device is ready */
        uint16_t byte_count = (header_master[4] << 8) | header_master[3];

        if(byte_count > 0)
        {
          /* Read the response */
          for(int j=0; j < byte_count; j++)
          {
            data[j] = _spi->transfer(0xFF);
          }

          if(byte_count == 7)
          {
            if(data[0] == 0x04 && 
               data[1] == 0x0E &&
               data[2] == 0x04 &&
               data[3] == 0x01 &&
               data[4] == 0x0C &&
               data[5] == 0xFC &&
               data[6] == 0x00)
            {
              status = 1;
            }
          }
        }
      }

      digitalWrite(_cs_pin, HIGH);

      _spi->endTransaction();
    }
  } while(!status);
}

void HCISpiTransportClass::enable_ll_only()
{
  uint8_t header_master[5] = {0x0a, 0x00, 0x00, 0x00, 0x00};
  uint8_t cmd[7] = {0x01,0x0C,0xFC,0x03,0x2C,0x01,0x01}; // Enable LL_ONLY
  int result = 0;

  do
  {
    result = 0;

    _spi->beginTransaction(SPISettings(_frequency, MSBFIRST, _spi_mode));

    digitalWrite(_cs_pin, LOW);

    /* Write the header */
    _spi->transfer(header_master, 5);

    /* device is ready */
    if(header_master[0] == 0x02) 
    {
      /* Write the data */
      if(header_master[1] >= 7)
      {
        /* Write the data */
        _spi->transfer((void *)cmd, 7);
      } else
      {
        result = -2;
      }
    } else
    {
      result = -1;
    }

    digitalWrite(_cs_pin, HIGH);

    _spi->endTransaction();
  } while (result < 0);
}
#endif /* STBTLE_RF */

#if defined(ARDUINO_STEVAL_MKSBOX1V1)
HCISpiTransportClass HCISpiTransport(SpiHCI, PD0, PD4, PA8, 1000000, SPI_MODE1); /* STEVAL_MKSBOX1V1 */
#elif defined(ARDUINO_DISCO_L475VG_IOT)
HCISpiTransportClass HCISpiTransport(SpiHCI, PD13, PE6, PA8, 8000000, SPI_MODE0); /* B-L475E-IOT01A1 */
#else
HCISpiTransportClass HCISpiTransport(SpiHCI, A1, A0, D7, 8000000, SPI_MODE0); /* Shield IDB05A1 */
#endif /* ARDUINO_STEVAL_MKSBOX1V1 */
HCITransportInterface& HCITransport = HCISpiTransport;

#endif /* ARDUINO_ARCH_STM32 */
