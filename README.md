# STM32duinoBLE

This library is a fork of ArduinoBLE library to add the support of  STM32WB, SPBTLE-RF, SPBTLE-1S, BLUENRG-M2SP, BLUENRG-LP and BLUENRG-M0  BLE modules.

It was successfully tested with the NUCLEO-WB15CC, P-NUCELO_WB55RG, STM32WB5MM-DK, X-NUCLEO-IDB05A2 or
X-NUCLEO-IDB05A1 or X-NUCLEO-BNRG2A1 expansion board and a NUCLEO-F401RE or NUCLEO-L476RG or NUCLEO-L053R8,
with B-L475E-IOT01A and with STEVAL-MKSBOX1V1.

 - In order to use this library with SM32WBxx series, you need to update the STM32WB Copro Wireless Binaries with stm32wbxx_BLE_HCILayer_fw.bin depending of your mcu:

https://github.com/STMicroelectronics/STM32CubeWB/tree/master/Projects/STM32WB_Copro_Wireless_Binaries

  Each subdirectories contains binaries and Release_Notes.html which explain how to update it.

 - In order to use this library with STEVAL-MKSBOX1V1, you need to update the firmware of the SPBTLE-1S BLE module mounted on that board as described in the following wiki page:

https://github.com/stm32duino/Arduino_Core_STM32/wiki/STM32duinoBLE#stm32duinoble-with-steval_mksbox1v1

- In order to use this library with X-NUCLEO-BNRG2A1, you need to update the firmware of the BLUENRG-M2SP BLE module mounted on that expansion board as described in the following wiki page:

https://github.com/stm32duino/Arduino_Core_STM32/wiki/STM32duinoBLE#stm32duinoble-with-x-nucleo-bnrg2a1

For more information about ArduinoBLE library please visit the official web page at:

https://www.arduino.cc/en/Reference/ArduinoBLE

# Configuration

### STM32WB

STM32Cube_WPAN has several configuration options, which are set in the `app_conf.h`.
This package has a default configuration named `app_conf_default.h`.
The user can include the file `app_conf_custom.h` to customize the BLE application.
Options wrapped in `#ifndef`, `#endif` in `app_conf_default.h` can be overwritten.
Additional options can be added.

### Shield

The user can include the file `ble_spi_conf.h` to define which shield and configuration to use from the following list:

 * [X-NUCLEO-IDB05A2](https://www.st.com/en/ecosystems/x-nucleo-idb05a2.html)
    * `IDB05A2_SPI_CLOCK_D3`: SPI clock on D3
    * `IDB05A2_SPI_CLOCK_D13` SPI clock on D13
 * [X-NUCLEO-IDB05A1](https://www.st.com/en/ecosystems/x-nucleo-idb05a1.html)
    * `IDB05A1_SPI_CLOCK_D3`: SPI clock on D3
    * `IDB05A1_SPI_CLOCK_D13`: SPI clock on D13
 * [X-NUCLEO-BNRG2A1](https://www.st.com/en/ecosystems/x-nucleo-bnrg2a1.html)
    * `BNRG2A1_CLOCK_D3`: SPI clock on D3
    * `BNRG2A1_CLOCK_D13`: SPI clock on D13
 * `CUSTOM_BLE_SPI`: define a custom configuration, it requires below definition:
    * `BLE_SPI_MISO`: SPI MISO pin
    * `BLE_SPI_MOSI`: SPI MOSI pin
    * `BLE_SPI_CLK`: SPI CLocK pin
    * `BLE_SPI_CS`: SPI Chip Select pin
    * `BLE_SPI_IRQ`: SPI IRQ pin
    * `BLE_SPI_FREQ`: SPI bus frequency
    * `BLE_SPI_MODE`: can be one of the below `SPIMode`:
      * `SPI_MODE0`
      * `SPI_MODE1`
      * `SPI_MODE2`
      * `SPI_MODE0`
    * `BLE_CHIP_TYPE`: can be one of the below `BLEChip_t`:
      * `SPBTLE_RF`
      * `SPBTLE_1S`
      * `BLUENRG_M2SP`
      * `BLUENRG_M0`
      * `BLUENRG_LP`
    * `BLE_RESET`: BLE reset pin

#### Examples

To use the [X-NUCLEO-IDB05A2](https://www.st.com/en/ecosystems/x-nucleo-idb05a2.html) with SPI clock on D3, define in `ble_spi_conf.h`:
```C
#define IDB05A2_SPI_CLOCK_D3
```
This is equivalent to the below configuration using the `CUSTOM_BLE_SPI`:
```C
#define CUSTOM_BLE_SPI
#define BLE_SPI_MISO    D12
#define BLE_SPI_MOSI    D11
#define BLE_SPI_CLK     D3
#define BLE_SPI_CS      A1
#define BLE_SPI_IRQ     A0
#define BLE_SPI_FREQ    8000000
#define BLE_SPI_MODE    SPI_MODE0
#define BLE_CHIP_TYPE   BLUENRG_M0
#define BLE_RESET       D7
```

#### Using a SPI BLE module on STM32WB

If required, user can use a compatible BLE module over SPI.

In the `ble_spi_conf.h`, define `USE_BLE_SPI`.

## License

```
Copyright (c) 2019 STMicroelectronics. All rights reserved.
Copyright (c) 2019 Arduino SA. All rights reserved.

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
```
