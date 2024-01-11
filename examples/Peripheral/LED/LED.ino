/*
  LED

  This example creates a BLE peripheral with service that contains a
  characteristic to control an LED.

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

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

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

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);

const int ledPin = LED_BUILTIN; // pin to use for the LED

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("LED");
  BLE.setAdvertisedService(ledService);

  // add the characteristic to the service
  ledService.addCharacteristic(switchCharacteristic);

  // add service
  BLE.addService(ledService);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("BLE LED Peripheral");
}

void loop() {
  // listen for BLE peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    // while the central is still connected to peripheral:
    while (central.connected()) {
      // if the remote device wrote to the characteristic,
      // use the value to control the LED:
      if (switchCharacteristic.written()) {
        if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }
    }

    // when the central disconnects, print it out:
    Serial.print(F("Disconnected from central: "));
    Serial.println(central.address());
  }
}
