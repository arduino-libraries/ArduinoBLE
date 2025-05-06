/*
  Scan Callback

  This example scans for BLE peripherals and prints out their advertising details:
  address, local name, adverised service UUIDs. Unlike the Scan example, it uses
  the callback style APIs and disables filtering so the peripheral discovery is
  reported for every single advertisement it makes.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>

unsigned long eventNow;
unsigned long eventInt = { 2000 } ;
const unsigned int eventStep = { 250 };
unsigned int eventDelay = { 1000 };
const unsigned int delayStep = { 100 };

unsigned int peripheralsCount = { 0 };

void setup() {
  Serial.begin(9600);
  while (!Serial);

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");

    while (1);
  }

  Serial.println("BLE Central scan callback");

  // set the discovered event handle
  BLE.setEventHandler(BLEDiscovered, bleCentralDiscoverHandler);

  // start scanning for peripherals with duplicates
  BLE.scan(true);
}

void loop() {
  // poll the central for events
  BLE.poll();

  if (millis() > eventNow) {
    Serial.print("Event 1 - ");
    Serial.print("Peripherals: ");
    Serial.print(peripheralsCount);
    peripheralsCount = 0;
    
    Serial.print(" Delay: ");
    Serial.print(eventDelay);
    
    Serial.print(" Interval: ");
    Serial.print(eventInt);

    delay(eventDelay);
    eventDelay += delayStep;
    eventNow = millis() + eventInt;
    eventInt += eventStep;
    
    Serial.print(" - Done.");
    Serial.println();
  }  
}

void bleCentralDiscoverHandler(BLEDevice peripheral) {
  BLE.stopScan();
  peripheralsCount++;

  digitalWrite(LED_BUILTIN, HIGH);
  delay(5);
  digitalWrite(LED_BUILTIN, LOW);
  BLE.scan(true);
}
