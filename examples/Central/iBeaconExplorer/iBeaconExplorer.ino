#include <ArduinoBLE.h>

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();
}

void loop() {
  // check if a peripheral has been discovered
  BLEDevice peripheral = BLE.available();

  if (!peripheral) {
    return;
  }
  if (!peripheral.hasManufacturerData()) {
    return;
  }

  String manufacturerData = peripheral.manufacturerData();
  if (manufacturerData.length() < 25 * 2) {
    return;
  }
  if (manufacturerData.substring(0, 8) != "4c000215") {
    return;
  }

  // Discovered an iBeacon
  Serial.println("Discovered an iBeacon");
  Serial.println("-----------------------");

  // UUID
  String id = manufacturerData.substring(8, 16) + "-";
  id += manufacturerData.substring(16, 20) + "-";
  id += manufacturerData.substring(20, 24) + "-";
  id += manufacturerData.substring(24, 28) + "-";
  id += manufacturerData.substring(28, 40);
  Serial.print("UUID: ");
  Serial.println(id);

  // Major ID
  char buf[5];
  manufacturerData.substring(40, 44).toCharArray(buf, 5);
  int major = (int)strtol(buf, NULL, 16);
  Serial.print("Major ID: ");
  Serial.println(major);

  // Minor ID
  manufacturerData.substring(44, 48).toCharArray(buf, 5);
  int minor = (int)strtol(buf, NULL, 16);
  Serial.print("Minor ID: ");
  Serial.println(minor);

  // RSSI
  Serial.print("RSSI: ");
  Serial.println(peripheral.rssi());

  Serial.println();
}
