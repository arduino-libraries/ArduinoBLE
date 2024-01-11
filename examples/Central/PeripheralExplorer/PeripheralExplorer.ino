/*
  Peripheral Explorer

  This example scans for BLE peripherals until one with a particular name ("LED")
  is found. Then connects, and discovers + prints all the peripheral's attributes.

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

  You can use it with another board that is compatible with this library and the
  Peripherals -> LED example.

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
const int buttonPin = PC13; // set buttonPin to digital pin PC13
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

  Serial.println("BLE Central - Peripheral Explorer");

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

    // see if peripheral is a LED
    if (peripheral.localName() == "LED") {
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

      explorerPeripheral(peripheral);

      // peripheral disconnected, we are done
      while (1) {
        // do nothing
      }
    }
  }
}

void explorerPeripheral(BLEDevice peripheral) {
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

  // read and print device name of peripheral
  Serial.println();
  Serial.print("Device name: ");
  Serial.println(peripheral.deviceName());
  Serial.print("Appearance: 0x");
  Serial.println(peripheral.appearance(), HEX);
  Serial.println();

  // loop the services of the peripheral and explore each
  for (int i = 0; i < peripheral.serviceCount(); i++) {
    BLEService service = peripheral.service(i);

    exploreService(service);
  }

  Serial.println();

  // we are done exploring, disconnect
  Serial.println("Disconnecting ...");
  peripheral.disconnect();
  Serial.println("Disconnected");
}

void exploreService(BLEService service) {
  // print the UUID of the service
  Serial.print("Service ");
  Serial.println(service.uuid());

  // loop the characteristics of the service and explore each
  for (int i = 0; i < service.characteristicCount(); i++) {
    BLECharacteristic characteristic = service.characteristic(i);

    exploreCharacteristic(characteristic);
  }
}

void exploreCharacteristic(BLECharacteristic characteristic) {
  // print the UUID and properties of the characteristic
  Serial.print("\tCharacteristic ");
  Serial.print(characteristic.uuid());
  Serial.print(", properties 0x");
  Serial.print(characteristic.properties(), HEX);

  // check if the characteristic is readable
  if (characteristic.canRead()) {
    // read the characteristic value
    characteristic.read();

    if (characteristic.valueLength() > 0) {
      // print out the value of the characteristic
      Serial.print(", value 0x");
      printData(characteristic.value(), characteristic.valueLength());
    }
  }
  Serial.println();

  // loop the descriptors of the characteristic and explore each
  for (int i = 0; i < characteristic.descriptorCount(); i++) {
    BLEDescriptor descriptor = characteristic.descriptor(i);

    exploreDescriptor(descriptor);
  }
}

void exploreDescriptor(BLEDescriptor descriptor) {
  // print the UUID of the descriptor
  Serial.print("\t\tDescriptor ");
  Serial.print(descriptor.uuid());

  // read the descriptor value
  descriptor.read();

  // print out the value of the descriptor
  Serial.print(", value 0x");
  printData(descriptor.value(), descriptor.valueLength());

  Serial.println();
}

void printData(const unsigned char data[], int length) {
  for (int i = 0; i < length; i++) {
    unsigned char b = data[i];

    if (b < 16) {
      Serial.print("0");
    }

    Serial.print(b, HEX);
  }
}
