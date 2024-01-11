/*
  SensorTag Button

  This example scans for BLE peripherals until a TI SensorTag is discovered.
  It then connects to it, discovers the attributes of the 0xffe0 service,
  subscribes to the Simple Key Characteristic (UUID 0xffe1). When a button is
  pressed on the SensorTag a notification is received and the button state is
  outputted to the Serial Monitor when one is pressed.

  The circuit:
  - Boards with integrated BLE or Nucleo board plus one of BLE X-Nucleo shield::
    - B-L475E-IOT01A1
    - B_L4S5I_IOT01A
    - STEVAL-MKBOXPRO
    - STEVAL-MKSBOX1V1,
    - NUCLEO-WB15CC
    - P-NUCLEO-WB55RG
    - STM32WB5MM-DK
    - X-NUCLEO-IDB05A2
    - X-NUCLEO-IDB05A1
    - X-NUCLEO-BNRG2A1
  - TI SensorTag

  This example code is in the public domain.
*/

#include <STM32duinoBLE.h>

#if defined(ARDUINO_STEVAL_MKBOXPRO)
/* STEVAL-MKBOXPRO */
SPIClass SpiHCI(PA7, PA6, PA5);
HCISpiTransportClass HCISpiTransport(SpiHCI, BLUENRG_LP, PA2, PB11, PD4, 1000000, SPI_MODE3);
#if !defined(FAKE_BLELOCALDEVICE)
BLELocalDevice BLEObj(&HCISpiTransport);
BLELocalDevice& BLE = BLEObj;
#endif
#elif defined(ARDUINO_STEVAL_MKSBOX1V1)
/* STEVAL-MKSBOX1V1 */
SPIClass SpiHCI(PC3, PD3, PD1);
HCISpiTransportClass HCISpiTransport(SpiHCI, SPBTLE_1S, PD0, PD4, PA8, 1000000, SPI_MODE1);
#if !defined(FAKE_BLELOCALDEVICE)
BLELocalDevice BLEObj(&HCISpiTransport);
BLELocalDevice& BLE = BLEObj;
#endif
#elif defined(ARDUINO_B_L475E_IOT01A) || defined(ARDUINO_B_L4S5I_IOT01A)
/* B-L475E-IOT01A1 or B_L4S5I_IOT01A */
SPIClass SpiHCI(PC12, PC11, PC10);
HCISpiTransportClass HCISpiTransport(SpiHCI, SPBTLE_RF, PD13, PE6, PA8, 8000000, SPI_MODE0);
#if !defined(FAKE_BLELOCALDEVICE)
BLELocalDevice BLEObj(&HCISpiTransport);
BLELocalDevice& BLE = BLEObj;
#endif
#elif defined(ARDUINO_NUCLEO_WB15CC) || defined(ARDUINO_P_NUCLEO_WB55RG) ||\
      defined(ARDUINO_STM32WB5MM_DK) || defined(ARDUINO_P_NUCLEO_WB55_USB_DONGLE)
HCISharedMemTransportClass HCISharedMemTransport;
#if !defined(FAKE_BLELOCALDEVICE)
BLELocalDevice BLEObj(&HCISharedMemTransport);
BLELocalDevice& BLE = BLEObj;
#endif
#else
/* Shield IDB05A2 with SPI clock on D3 */
SPIClass SpiHCI(D11, D12, D3);
HCISpiTransportClass HCISpiTransport(SpiHCI, BLUENRG_M0, A1, A0, D7, 8000000, SPI_MODE0);
#if !defined(FAKE_BLELOCALDEVICE)
BLELocalDevice BLEObj(&HCISpiTransport);
BLELocalDevice& BLE = BLEObj;
#endif
/* Shield IDB05A2 with SPI clock on D13 */
// #define SpiHCI SPI
// HCISpiTransportClass HCISpiTransport(SpiHCI, BLUENRG_M0, A1, A0, D7, 8000000, SPI_MODE0);
// #if !defined(FAKE_BLELOCALDEVICE)
// BLELocalDevice BLEObj(&HCISpiTransport);
// BLELocalDevice& BLE = BLEObj;
// #endif
/* Shield IDB05A1 with SPI clock on D3 */
// SPIClass SpiHCI(D11, D12, D3);
// HCISpiTransportClass HCISpiTransport(SpiHCI, SPBTLE_RF, A1, A0, D7, 8000000, SPI_MODE0);
// #if !defined(FAKE_BLELOCALDEVICE)
// BLELocalDevice BLEObj(&HCISpiTransport);
// BLELocalDevice& BLE = BLEObj;
// #endif
/* Shield IDB05A1 with SPI clock on D13 */
// #define SpiHCI SPI
// HCISpiTransportClass HCISpiTransport(SpiHCI, SPBTLE_RF, A1, A0, D7, 8000000, SPI_MODE0);
// #if !defined(FAKE_BLELOCALDEVICE)
// BLELocalDevice BLEObj(&HCISpiTransport);
// BLELocalDevice& BLE = BLEObj;
// #endif
/* Shield BNRG2A1 with SPI clock on D3 */
// SPIClass SpiHCI(D11, D12, D3);
// HCISpiTransportClass HCISpiTransport(SpiHCI, BLUENRG_M2SP, A1, A0, D7, 1000000, SPI_MODE1);
// #if !defined(FAKE_BLELOCALDEVICE)
// BLELocalDevice BLEObj(&HCISpiTransport);
// BLELocalDevice& BLE = BLEObj;
// #endif
/* Shield BNRG2A1 with SPI clock on D13 */
// #define SpiHCI SPI
// HCISpiTransportClass HCISpiTransport(SpiHCI, BLUENRG_M2SP, A1, A0, D7, 1000000, SPI_MODE1);
// #if !defined(FAKE_BLELOCALDEVICE)
// BLELocalDevice BLEObj(&HCISpiTransport);
// BLELocalDevice& BLE = BLEObj;
// #endif
#endif

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  Serial.println("BLE Central - SensorTag button");
  Serial.println("Make sure to turn on the device.");

  // start scanning for peripherals
  int ret = 1;
  do
  {
    ret = BLE.scan();
    if (ret == 0)
    {
      BLE.end();
      BLE.begin();
    }
  } while(ret == 0);
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // discovered a peripheral, print out address, local name, and advertised service
    Serial.print("Found ");
    Serial.print(peripheral.address());
    Serial.print(" '");
    Serial.print(peripheral.localName());
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    // Check if the peripheral is a SensorTag, the local name will be:
    // "CC2650 SensorTag"
    if (peripheral.localName() == "CC2650 SensorTag") {
      // stop scanning
      int ret = 1;
      do
      {
        ret = BLE.stopScan();
        if (ret == 0)
        {
          BLE.end();
          BLE.begin();
        }
      } while(ret == 0);

      monitorSensorTagButtons(peripheral);

      // peripheral disconnected, start scanning again
      ret = 1;
      do
      {
        ret = BLE.scan();
        if (ret == 0)
        {
          BLE.end();
          BLE.begin();
         }
      } while(ret == 0);
    }
  }
}

void monitorSensorTagButtons(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");
  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering service 0xffe0 ...");
  if (peripheral.discoverService("ffe0")) {
    Serial.println("Service discovered");
  } else {
    Serial.println("Attribute discovery failed.");
    peripheral.disconnect();

    while (1);
    return;
  }

  // retrieve the simple key characteristic
  BLECharacteristic simpleKeyCharacteristic = peripheral.characteristic("ffe1");

  // subscribe to the simple key characteristic
  Serial.println("Subscribing to simple key characteristic ...");
  if (!simpleKeyCharacteristic) {
    Serial.println("no simple key characteristic found!");
    peripheral.disconnect();
    return;
  } else if (!simpleKeyCharacteristic.canSubscribe()) {
    Serial.println("simple key characteristic is not subscribable!");
    peripheral.disconnect();
    return;
  } else if (!simpleKeyCharacteristic.subscribe()) {
    Serial.println("subscription failed!");
    peripheral.disconnect();
    return;
  } else {
    Serial.println("Subscribed");
    Serial.println("Press the right and left buttons on your SensorTag.");
  }

  while (peripheral.connected()) {
    // while the peripheral is connected

    // check if the value of the simple key characteristic has been updated
    if (simpleKeyCharacteristic.valueUpdated()) {
      // yes, get the value, characteristic is 1 byte so use byte value
      byte value = 0;

      simpleKeyCharacteristic.readValue(value);

      if (value & 0x01) {
        // first bit corresponds to the right button
        Serial.println("Right button pressed");
      }

      if (value & 0x02) {
        // second bit corresponds to the left button
        Serial.println("Left button pressed");
      }
    }
  }

  Serial.println("SensorTag disconnected!");
}
