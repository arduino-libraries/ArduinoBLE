/*
  LED Control

  This example scans for BLE peripherals until one with the advertised service
  "19b10000-e8f2-537e-4f6c-d104768a1214" UUID is found. Once discovered and connected,
  it will remotely control the BLE Peripheral's LED, when the button is pressed or released.

  The circuit:
  - STEVAL-MKSBOX1V1, B-L475E-IOT01A1, B_L4S5I_IOT01A, or a Nucleo board plus the X-NUCLEO-IDB05A1 or the X-NUCLEO-BNRG2A1

  You can use it with another board that is compatible with this library and the
  Peripherals -> LED example.

  This example code is in the public domain.
*/

#include <STM32duinoBLE.h>

#if defined(ARDUINO_STEVAL_MKSBOX1V1)
/* STEVAL-MKSBOX1V1 */
SPIClass SpiHCI(PC3, PD3, PD1);
HCISpiTransportClass HCISpiTransport(SpiHCI, SPBTLE_1S, PD0, PD4, PA8, 1000000, SPI_MODE1);
BLELocalDevice BLE(&HCISpiTransport);
const int buttonPin = PG1; // set buttonPin to digital pin PG1
#elif defined(ARDUINO_DISCO_L475VG_IOT) || defined(ARDUINO_B_L4S5I_IOT01A)
/* B-L475E-IOT01A1 or B_L4S5I_IOT01A */
SPIClass SpiHCI(PC12, PC11, PC10);
HCISpiTransportClass HCISpiTransport(SpiHCI, SPBTLE_RF, PD13, PE6, PA8, 8000000, SPI_MODE0);
BLELocalDevice BLE(&HCISpiTransport);
const int buttonPin = PC13; // set buttonPin to digital pin PC13
#else
/* Shield IDB05A1 with SPI clock on D3 */
SPIClass SpiHCI(D11, D12, D3);
HCISpiTransportClass HCISpiTransport(SpiHCI, SPBTLE_RF, A1, A0, D7, 8000000, SPI_MODE0);
BLELocalDevice BLE(&HCISpiTransport);
const int buttonPin = PC13; // set buttonPin to digital pin PC13
/* Shield IDB05A1 with SPI clock on D13 */
/*#define SpiHCI SPI
HCISpiTransportClass HCISpiTransport(SpiHCI, SPBTLE_RF, A1, A0, D7, 8000000, SPI_MODE0);
BLELocalDevice BLE(&HCISpiTransport);
const int buttonPin = PC13; // set buttonPin to digital pin PC13 */
/* Shield BNRG2A1 with SPI clock on D3 */
/*SPIClass SpiHCI(D11, D12, D3);
HCISpiTransportClass HCISpiTransport(SpiHCI, BLUENRG_M2SP, A1, A0, D7, 1000000, SPI_MODE1);
BLELocalDevice BLE(&HCISpiTransport);
const int buttonPin = PC13; // set buttonPin to digital pin PC13 */
/* Shield BNRG2A1 with SPI clock on D13 */
/*#define SpiHCI SPI
HCISpiTransportClass HCISpiTransport(SpiHCI, BLUENRG_M2SP, A1, A0, D7, 1000000, SPI_MODE1);
BLELocalDevice BLE(&HCISpiTransport);
const int buttonPin = PC13; // set buttonPin to digital pin PC13 */
#endif

// variables for button
int oldButtonState = LOW;
int initialButtonState = LOW;

void setup() {
  Serial.begin(115200);
  while (!Serial);

  // configure the button pin as input
  pinMode(buttonPin, INPUT);

  // initialize the BLE hardware
  BLE.begin();

  // Get initial button state
  initialButtonState = digitalRead(buttonPin);
  oldButtonState = initialButtonState;

  Serial.println("BLE Central - LED control");

  // start scanning for peripherals
  int ret = 1;
  do
  {
    ret = BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
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

    if (peripheral.localName() != "LED") {
      return;
    }

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

    controlLed(peripheral);

    // peripheral disconnected, start scanning again
    ret = 1;
    do
    {
      ret = BLE.scanForUuid("19b10000-e8f2-537e-4f6c-d104768a1214");
      if (ret == 0)
      {
        BLE.end();
        BLE.begin();
      }
    } while(ret == 0);
  }
}

void controlLed(BLEDevice peripheral) {
  // connect to the peripheral
  Serial.println("Connecting ...");

  if (peripheral.connect()) {
    Serial.println("Connected");
  } else {
    Serial.println("Failed to connect!");
    return;
  }

  // discover peripheral attributes
  Serial.println("Discovering attributes ...");
  if (peripheral.discoverAttributes()) {
    Serial.println("Attributes discovered");
  } else {
    Serial.println("Attribute discovery failed!");
    peripheral.disconnect();
    return;
  }

  // retrieve the LED characteristic
  BLECharacteristic ledCharacteristic = peripheral.characteristic("19b10001-e8f2-537e-4f6c-d104768a1214");

  if (!ledCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    return;
  } else if (!ledCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable LED characteristic!");
    peripheral.disconnect();
    return;
  }

  while (peripheral.connected()) {
    // while the peripheral is connected

    // read the button pin
    int buttonState = digitalRead(buttonPin);

    if (oldButtonState != buttonState) {
      // button changed
      oldButtonState = buttonState;

      if (buttonState != initialButtonState) {
        Serial.println("button pressed");

        // button is pressed, write 0x01 to turn the LED on
        ledCharacteristic.writeValue((byte)0x01);
      } else {
        Serial.println("button released");

        // button is released, write 0x00 to turn the LED off
        ledCharacteristic.writeValue((byte)0x00);
      }
    }
  }

  Serial.println("Peripheral disconnected");
}
