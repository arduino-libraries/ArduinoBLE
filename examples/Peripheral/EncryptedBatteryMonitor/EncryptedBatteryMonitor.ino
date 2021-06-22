/*
  Battery Monitor

  This example creates a BLE peripheral with the standard battery service and
  level characteristic. The A0 pin is used to calculate the battery level.

  The circuit:
  - Arduino MKR WiFi 1010, Arduino Uno WiFi Rev2 board, Arduino Nano 33 IoT,
    Arduino Nano 33 BLE, or Arduino Nano 33 BLE Sense board.

  You can use a generic BLE central app, like LightBlue (iOS and Android) or
  nRF Connect (Android), to interact with the services and characteristics
  created in this sketch.

  This example code is in the public domain.
*/

#include <ArduinoBLE.h>


#define PAIR_BUTTON 3        // button for pairing
#define PAIR_LED 24           // LED used to signal pairing
#define PAIR_LED_ON LOW       // Blue LED on Nano BLE has inverted logic
#define PAIR_INTERVAL 30000   // interval for pairing after button press in ms

#define CTRL_LED LED_BUILTIN


 // BLE Battery Service
BLEService batteryService("180F");

// BLE Battery Level Characteristic
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",  // standard 16-bit characteristic UUID
    BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes
BLEStringCharacteristic stringcharacteristic("183E", BLERead | BLEWrite, 31);


// Add BLEEncryption tag to require pairing. This controls the LED.
BLEUnsignedCharCharacteristic secretValue("2a3F", BLERead | BLEWrite | BLEEncryption);

int oldBatteryLevel = 0;  // last battery level reading from analog input
unsigned long previousMillis = 0;  // last time the battery level was checked, in ms
unsigned long pairingStarted = 0;  // pairing start time when button is pressed
bool wasConnected = 0;
bool acceptOrReject = true;

void setup() {
  Serial.begin(9600);    // initialize serial communication
  while (!Serial);

  pinMode(CTRL_LED, OUTPUT); // initialize the built-in LED pin to indicate when a central is connected
  pinMode(PAIR_LED, OUTPUT);
  pinMode(PAIR_BUTTON, INPUT_PULLUP);


  Serial.println("Serial connected");
  
  // Callback function with confirmation code when new device is pairing.
  BLE.setDisplayCode([](uint32_t confirmCode){
    Serial.println("New device pairing request.");
    Serial.print("Confirm code matches pairing device: ");
    char code[6];
    sprintf(code, "%06d", confirmCode);
    Serial.println(code);
  });
  
  // Callback to allow accepting or rejecting pairing
  BLE.setBinaryConfirmPairing([&acceptOrReject](){
    Serial.print("Should we confirm pairing? ");
    delay(5000);
    if(acceptOrReject){
      acceptOrReject = false;
      Serial.println("yes");
      return true;
    }else{
      acceptOrReject = true;
      Serial.println("no");
      return false;
    }
  });

  // IRKs are keys that identify the true owner of a random mac address.
  // Add IRKs of devices you are bonded with.
  BLE.setGetIRKs([](uint8_t* nIRKs, uint8_t** BDaddrTypes, uint8_t*** BDAddrs, uint8_t*** IRKs){
    // Set to number of devices
    *nIRKs       = 2;

    *BDAddrs     = new uint8_t*[*nIRKs];
    *IRKs        = new uint8_t*[*nIRKs];
    *BDaddrTypes = new uint8_t[*nIRKs];

    // Set these to the mac and IRK for your bonded devices as printed in the serial console after bonding.
    uint8_t device1Mac[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t device1IRK[16]   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};

    uint8_t device2Mac[6]    = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t device2IRK[16]   = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};


    (*BDaddrTypes)[0] = 0; // Type 0 is for pubc address, type 1 is for static random
    (*BDAddrs)[0] = new uint8_t[6]; 
    (*IRKs)[0]    = new uint8_t[16];
    memcpy((*IRKs)[0]   , device1IRK,16);
    memcpy((*BDAddrs)[0], device1Mac, 6);


    (*BDaddrTypes)[1] = 0;
    (*BDAddrs)[1] = new uint8_t[6];
    (*IRKs)[1]    = new uint8_t[16];
    memcpy((*IRKs)[1]   , device2IRK,16);
    memcpy((*BDAddrs)[1], device2Mac, 6);


    return 1;
  });
  // The LTK is the secret key which is used to encrypt bluetooth traffic
  BLE.setGetLTK([](uint8_t* address, uint8_t* LTK){
    // address is input
    Serial.print("Received request for address: ");
    btct.printBytes(address,6);

    // Set these to the MAC and LTK of your devices after bonding.
    uint8_t device1Mac[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t device1LTK[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t device2Mac[6]  = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    uint8_t device2LTK[16] = {0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00};
    

    if(memcmp(device1Mac, address, 6) == 0) {
      memcpy(LTK, device1LTK, 16);
      return 1;
    }else if(memcmp(device2Mac, address, 6) == 0) {
      memcpy(LTK, device2LTK, 16);
      return 1;
    }
    return 0;
  });
  BLE.setStoreIRK([](uint8_t* address, uint8_t* IRK){
    Serial.print(F("New device with MAC : "));
    btct.printBytes(address,6);
    Serial.print(F("Need to store IRK   : "));
    btct.printBytes(IRK,16);
    return 1;
  });
  BLE.setStoreLTK([](uint8_t* address, uint8_t* LTK){
    Serial.print(F("New device with MAC : "));
    btct.printBytes(address,6);
    Serial.print(F("Need to store LTK   : "));
    btct.printBytes(LTK,16);
    return 1;
  });

  while(1){
    // begin initialization
    if (!BLE.begin()) {
      Serial.println("starting BLE failed!");
      delay(200);
      continue;
    }
    Serial.println("BT init");
    delay(200);
    
    /* Set a local name for the BLE device
       This name will appear in advertising packets
       and can be used by remote devices to identify this BLE device
       The name can be changed but maybe be truncated based on space left in advertisement packet
    */

    BLE.setDeviceName("Arduino");
    BLE.setLocalName("BatteryMonitor");

    BLE.setAdvertisedService(batteryService); // add the service UUID
    batteryService.addCharacteristic(batteryLevelChar); // add the battery level characteristic
    batteryService.addCharacteristic(stringcharacteristic);
    batteryService.addCharacteristic(secretValue);

    BLE.addService(batteryService);               // Add the battery service
    batteryLevelChar.writeValue(oldBatteryLevel); // set initial value for this characteristic
    char* stringCharValue = new char[32];
    stringCharValue = "string";
    stringcharacteristic.writeValue(stringCharValue);
    secretValue.writeValue(0);
    
    delay(1000);

    // prevent pairing until button is pressed (will show a pairing rejected message)
    BLE.setPairable(false);
  
    /* Start advertising BLE.  It will start continuously transmitting BLE
       advertising packets and will be visible to remote BLE central devices
       until it receives a new connection */
  
    // start advertising
    if(!BLE.advertise()){
      Serial.println("failed to advertise bluetooth.");
      BLE.stopAdvertise();
      delay(500);
    }else{
      Serial.println("advertising...");
      break;
    }
    BLE.end();
    delay(100);
  }
}


void loop() {
  // wait for a BLE central
  BLEDevice central = BLE.central();


  // If button is pressed, allow pairing for 30 sec
  if (!BLE.pairable() && digitalRead(PAIR_BUTTON) == LOW){
    pairingStarted = millis();
    BLE.setPairable(Pairable::ONCE);
    Serial.println("Accepting pairing for 30s");
  } else if (BLE.pairable() && millis() > pairingStarted + PAIR_INTERVAL){
    BLE.setPairable(false);
    Serial.println("No longer accepting pairing");
  }
  // Make LED blink while pairing is allowed
  digitalWrite(PAIR_LED, (BLE.pairable() ? (millis()%400)<200 : BLE.paired()) ? PAIR_LED_ON : !PAIR_LED_ON); 


  // if a central is connected to the peripheral:
  if (central && central.connected()) {
    if (!wasConnected){
      wasConnected = true;
      Serial.print("Connected to central: ");
      // print the central's BT address:
      Serial.println(central.address());
    }

    // check the battery level every 200ms
    // while the central is connected:
    long currentMillis = millis();
    // if 200ms have passed, check the battery level:
    if (currentMillis - previousMillis >= 1000) {
      previousMillis = currentMillis;
      updateBatteryLevel();
      digitalWrite(CTRL_LED, secretValue.value()>0 ? HIGH : LOW);
    }
  } else if (wasConnected){
    wasConnected = false;
    Serial.print("Disconnected from central: ");
    Serial.println(central.address());
  }
    
}

void updateBatteryLevel() {
  /* Read the current voltage level on the A0 analog input pin.
     This is used here to simulate the charge level of a battery.
  */
  int battery = analogRead(A0);
  int batteryLevel = map(battery, 0, 1023, 0, 100);

  if (batteryLevel != oldBatteryLevel) {      // if the battery level has changed
    // Serial.print("Battery Level % is now: "); // print it
    // Serial.println(batteryLevel);
    batteryLevelChar.writeValue(batteryLevel);  // and update the battery level characteristic
    oldBatteryLevel = batteryLevel;           // save the level for next comparison
  }
}