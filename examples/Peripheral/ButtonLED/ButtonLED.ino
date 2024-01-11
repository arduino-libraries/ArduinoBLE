/*
  Button LED

  This example creates a BLE peripheral with service that contains a
  characteristic to control an LED and another characteristic that
  represents the state of the button.

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

#ifdef USER_BTN
const int buttonPin = USER_BTN; // set buttonPin to on-board user button
#else
const int buttonPin = PC13; // set buttonPin to digital pin PC13 */
#endif

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

const int ledPin = LED_BUILTIN; // set ledPin to on-board LED

BLEService ledService("19B10010-E8F2-537E-4F6C-D104768A1214"); // create service

// create switch characteristic and allow remote device to read and write
BLEByteCharacteristic ledCharacteristic("19B10011-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);
// create button characteristic and allow remote device to get notifications
BLEByteCharacteristic buttonCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify);

void setup() {
  Serial.begin(115200);
  while (!Serial);

  pinMode(ledPin, OUTPUT); // use the LED as an output
  pinMode(buttonPin, INPUT_PULLUP); // use button pin as an input

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  // set the local name peripheral advertises
  BLE.setLocalName("ButtonLED");
  // set the UUID for the service this peripheral advertises:
  BLE.setAdvertisedService(ledService);

  // add the characteristics to the service
  ledService.addCharacteristic(ledCharacteristic);
  ledService.addCharacteristic(buttonCharacteristic);

  // add the service
  BLE.addService(ledService);

  ledCharacteristic.writeValue(0);
  buttonCharacteristic.writeValue(0);

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth device active, waiting for connections...");
}

void loop() {
  // poll for BLE events
  BLE.poll();

  // read the current button pin state
  char buttonValue = digitalRead(buttonPin);

  // has the value changed since the last read
  bool buttonChanged = (buttonCharacteristic.value() != buttonValue);

  if (buttonChanged) {
    // button state changed, update characteristics
    ledCharacteristic.writeValue(buttonValue);
    buttonCharacteristic.writeValue(buttonValue);
  }

  if (ledCharacteristic.written() || buttonChanged) {
    // update LED, either central has written to characteristic or button state has changed
    if (ledCharacteristic.value()) {
      Serial.println("LED on");
      digitalWrite(ledPin, HIGH);
    } else {
      Serial.println("LED off");
      digitalWrite(ledPin, LOW);
    }
  }
}
