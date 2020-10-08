/*
  Climate Sensor

  This example creates a BLE peripheral which emulates a Xiami Mijia climate
  sensor.  This device encodes the temperature and humidity into the service
  data, so BLE apps can detect the current readings without connecting.  For
  more information see here:
  https://github.com/mspider65/Xiaomi-Mijia-Bluetooth-Temperature-and-Humidity-Sensor

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>


void setup() {
  Serial.begin(9600);    // initialize serial communication
  while (!Serial);

  pinMode(LED_BUILTIN, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  /* Set a local name for the BLE device
     This name will appear in advertising packets
     and can be used by remote devices to identify this BLE device
     The name can be changed but maybe be truncated based on space left in advertisement packet
  */

  // read all the sensor values if you have something like a HTS sensor
  float temperature = 28.5; // HTS.readTemperature();
  float humidity    = 42.0; // HTS.readHumidity();

  // convert the temperatures to a 16-bit byte format
  uint16_t tempBytes = temperature * 10.0;
  uint16_t humidityBytes = humidity * 10.0;

  // prepare the service data payload, this emulates a Xiamo Mijia BLE Climate Sensor
  uint16_t uuid = 0xFE95;
  uint8_t serviceDataLength = 18;
  uint8_t serviceData[] = {0x50, 0x20, 0xAA, 0x01, 0x17, 0xEE, 0x7F, 0xB5, 0x91, 0xFB, 0xE8, 0x0D, 0x10, 0x04, tempBytes & 0xFF, tempBytes>>8, humidityBytes & 0xFF, humidityBytes>>8};

  // Set the service data
  BLE.setAdvertisedServiceData(uuid, serviceData, serviceDataLength);

  /* Start advertising BLE.  It will start continuously transmitting BLE
     advertising packets and will be visible to remote BLE central devices
     until it receives a new connection */

  // start advertising
  BLE.advertise();

  Serial.println("Bluetooth device active and advertising...");
}

void loop() {
  digitalWrite(LED_BUILTIN, HIGH);
  delay(500);

  digitalWrite(LED_BUILTIN, LOW);
  delay(500);
}
