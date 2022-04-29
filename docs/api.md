# ArduinoBLE library

## BLE class

Used to enable the Bluetooth® Low Energy module.

### `BLE.begin()`

Initializes the Bluetooth® Low Energy device.

#### Syntax

```
BLE.begin()

```

#### Parameters

None

#### Returns
- 1 on success
- 0 on failure

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }


```

### `BLE.end()`

Stops the Bluetooth® Low Energy device.

#### Syntax

```
BLE.end()

```

#### Parameters

None

#### Returns
Nothing

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // ....

  BLE.end();


```

### `BLE.poll()`

Poll for Bluetooth® Low Energy radio events and handle them.

#### Syntax

```
BLE.poll() 
BLE.poll(timeout)

```

#### Parameters

**timeout**: optional timeout in ms, to wait for event. If not specified defaults to 0 ms.

#### Returns
Nothing

#### Example

```arduino

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  

  BLE.poll();


```

### `BLE.setEventHandler()`

Set the event handler (callback) function that will be called when the specified event occurs.

#### Syntax

```
BLE.setEventHandler(eventType, callback)

```

#### Parameters

- **eventType**: event type (BLEConnected, BLEDisconnected)
- **callback**: function to call when event occurs
#### Returns
Nothing.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // ...

  // assign event handlers for connected, disconnected to peripheral
  BLE.setEventHandler(BLEConnected, blePeripheralConnectHandler);
  BLE.setEventHandler(BLEDisconnected, blePeripheralDisconnectHandler);

  


void blePeripheralConnectHandler(BLEDevice central) {
  // central connected event handler
  Serial.print("Connected event, central: ");
  Serial.println(central.address());
}

void blePeripheralDisconnectHandler(BLEDevice central) {
  // central disconnected event handler
  Serial.print("Disconnected event, central: ");
  Serial.println(central.address());
}


```

### `BLE.connected()`

Query if another Bluetooth® Low Energy device is connected

#### Syntax

```
BLE.connected()

```

#### Parameters

None

#### Returns
- **true** if another Bluetooth® Low Energy device is connected, 
- otherwise **false**.

#### Example

```arduino

    // while the central is still connected to peripheral:
    while (BLE.connected()) {

       // ...
    }


```

### `BLE.disconnect()`

Disconnect any Bluetooth® Low Energy devices that are connected

#### Syntax

```
BLE.disconnect()

```

#### Parameters

None

#### Returns
- **true** if any Bluetooth® Low Energy device that was previously connected was disconnected, 
- otherwise **false**.

#### Example

```arduino

  if (BLE.connected()) {
    

    BLE.disconnect();
  }


```

### `BLE.address()`

Query the Bluetooth® address of the Bluetooth® Low Energy device.

#### Syntax

```
BLE.address()

```

#### Parameters

None

#### Returns
- The **Bluetooth® address** of the Bluetooth® Low Energy device (as a String).

#### Example

```arduino

  **String** address = BLE.address();

  Serial.print(“Local address is: “);
  Serial.println(address);


```

### `BLE.rssi()`

Query the RSSI (Received signal strength indication) of the connected Bluetooth® Low Energy device.

#### Syntax

```
BLE.rssi()

```

#### Parameters

None

#### Returns
- The **RSSI** of the connected Bluetooth® Low Energy device, 127 if no Bluetooth® Low Energy device is connected.

#### Example

```arduino

  if (BLE.connected()) {
    

   Serial.print(“RSSI = “);
    Serial.println(BLE.rssi());
  }


```

### `BLE.setAdvertisedServiceUuid()`

Set the advertised service UUID used when advertising.

#### Syntax

```
BLE.setAdvertisedServiceUuid(uuid)

```

#### Parameters

- **uuid:** 16-bit or 128-bit Bluetooth® Low Energy UUID in **String** format

#### Returns
Nothing

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  BLE.setAdvertisedServiceUuid(“19B10000-E8F2-537E-4F6C-D104768A1214");

  // ...  

  // start advertising
  BLE.advertise();


```

### `BLE.setAdvertisedService()`

Set the advertised service UUID used when advertising to the value of the BLEService provided.

#### Syntax

```
BLE.setAdvertisedService(bleService)

```

#### Parameters

- **bleService:** BLEService to use UUID from

#### Returns
Nothing

#### Example

```arduino

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// ...

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  BLE.setAdvertisedService(ledService);

  // ...  

  // start advertising
  BLE.advertise();


```

### `BLE.setManufacturerData()`

Set the manufacturer data value used when advertising.

#### Syntax

```
BLE.setManufacturerData(data, length)

```

#### Parameters

- **data:** byte array containing manufacturer data
- **length:** length of manufacturer data array

#### Returns
Nothing

#### Example

```arduino

 // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  byte data[5] = { 0x01, 0x02, 0x03, 0x04, 0x05};

  BLE.setManufacturerData(data, 5);

  // ...  

  // start advertising
  BLE.advertise();



```

### `BLE.setLocalName()`

Set the local value used when advertising.

#### Syntax

```
BLE.setLocalName(name)

```

#### Parameters

- **name:** local name value to use when advertising

#### Returns
Nothing

#### Example

```arduino

 // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  BLE.setLocalName("LED");

  // ...  

  // start advertising
  BLE.advertise();



```

### `BLE.setDeviceName()`

Set the device name in the built in device name characteristic. If not set, the value defaults “Arduino”.

#### Syntax

```
BLE.setDeviceName(name)

```

#### Parameters

- **name:** device name value

#### Returns
Nothing

#### Example

```arduino

 // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  BLE.setDeviceName("LED");

  // ...  

  // start advertising
  BLE.advertise();



```

### `BLE.setAppearance()`

Set the appearance in the built in appearance characteristic. If not set, the value defaults 0x0000.

#### Syntax

```
BLE.setAppearance(appearance)

```

#### Parameters

- **appearance:** appearance value

#### Returns
Nothing

#### Example

```arduino

 // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  BLE.setAppearance(0x8000);

  // ...  

  // start advertising
  BLE.advertise();



```

### `BLE.addService()`

Add a BLEService to the set of services the Bluetooth® Low Energy device provides

#### Syntax

```
BLE.addService(service)

```

#### Parameters

- **service:** BLEService to add

#### Returns
Nothing

#### Example

```arduino

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service



 // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // ...

  BLE.addService(ledService);

  // ...  


```

### `BLE.advertise()`

Start advertising.

#### Syntax

```
BLE.advertise()

```

#### Parameters

None

#### Returns
- 1 on success,
- 0 on failure.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // ...

  BLE.advertise();

  // ...  


```

### `BLE.stopAdvertise()`

Stop advertising.

#### Syntax

```
BLE.stopAdvertise()

```

#### Parameters

None

#### Returns
Nothing

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // ...

  BLE.advertise();

  // ...  

  BLE.stopAdvertise();


```

### `BLE.central()`

Query the central Bluetooth® Low Energy device connected.

#### Syntax

```
BLE.central()

```

#### Parameters

None

#### Returns
- **BLEDevice** representing the central.

#### Example

```arduino

  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    
  }


```

### `BLE.setAdvertisingInterval()`

Set the advertising interval in units of 0.625 ms. Defaults to 100ms (160 * 0.625 ms) if not provided.

#### Syntax

```
BLE.setAdvertisingInterval(advertisingInterval)

```

#### Parameters

- **advertisingInterval:** advertising interval in units of 0.625 ms

#### Returns
Nothing.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // ...

  BLE.setAdvertisingInterval(320); // 200 * 0.625 ms

  BLE.advertise();


```

### `BLE.setConnectionInterval()`

Set the minimum and maximum desired connection intervals in units of 1.25 ms.

#### Syntax

```
BLE.setConnectionInterval(minimum, maximum)

```

#### Parameters

- **minimum:** minimum desired connection interval in units of 1.25 ms
- **maximum:** maximum desired connection interval in units of 1.25 ms

#### Returns
Nothing.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // ...

  BLE.setConnectionInterval(0x0006, 0x0c80); // 7.5 ms minimum, 4 s maximum



```

### `BLE.setConnectable()`

Set if the device is connectable after advertising, defaults to **true**.

#### Syntax

```
BLE.setConnectable(connectable)

```

#### Parameters

- **true**: the device will be connectable when advertising
- **false**: the device will NOT be connectable when advertising

#### Returns
Nothing.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  // ...

  BLE.setConnectable(false); // make the device unconnectable when advertising



```

### `BLE.scan()`

Start scanning for Bluetooth® Low Energy devices that are advertising.

#### Syntax

```
BLE.scan()
BLE.scan(withDuplicates)

```

#### Parameters

- **withDuplicates:** optional, defaults to **false**. If **true**, advertisements received more than once will not be filtered

#### Returns
- 1 on success, 
- 0 on failure.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
     // ...
  }


```

### `BLE.scanForName()`

Start scanning for Bluetooth® Low Energy devices that are advertising with a particular (local) name.

#### Syntax

```
BLE.scanForName(name)
BLE.scanForName(name, withDuplicates)

```

#### Parameters

-  **name:** (local) name of device (as a **String**) to filter for
- **withDuplicates:** optional, defaults to **false**. If **true**, advertisements received more than once will not be filtered.

#### Returns
- 1 on success,
- 0 on failure.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scanForName("LED");

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
     // ...
  }


```

### `BLE.scanForAddress()`

Start scanning for Bluetooth® Low Energy devices that are advertising with a particular (Bluetooth®) address.

#### Syntax

```
BLE.scanForAddress(address)
BLE.scanForAddress(address, withDuplicates)

```

#### Parameters

- **address:** (Bluetooth®) address (as a String) to filter for
- **withDuplicates:** optional, defaults to **false**. If **true**, advertisements received more than once will not be filtered

#### Returns
- 1 on success, 
- 0 on failure.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scanForAddress("aa:bb:cc:ee:dd:ff");

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
     // ...
  }


```

### `BLE.scanForUuid()`

Start scanning for Bluetooth® Low Energy devices that are advertising with a particular (service) UUID.

#### Syntax

```
BLE.scanForUuid(uuid)
BLE.scanForUuid(uuid, withDuplicates)

```

#### Parameters

- **uuid:** (service) UUID (as a **String**) to filter for
- **withDuplicates:** optional, defaults to **false**. If **true**, advertisements received more than once will not be filtered.

#### Returns
- 1 on success,
- 0 on failure.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scanForUuid("aa10");

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
     // ...
  }


```

### `BLE.stopScan()`

Stop scanning for Bluetooth® Low Energy devices that are advertising.

#### Syntax

```
BLE.stopScan()

```

#### Parameters

None

#### Returns
Nothing

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  
  BLE.stopScan();


```

### `BLE.available()`

Query for a discovered Bluetooth® Low Energy device that was found during scanning.

#### Syntax

```
BLE.available()

```

#### Parameters

Nothing

#### Returns
- **BLEDevice** representing the discovered device.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
     // ...
  }


```

## BLEDevice Class

Used to get information about the devices connected or discovered while scanning

### `bleDevice.poll()`

Poll for Bluetooth® Low Energy radio events for the specified Bluetooth® Low Energy device and handle them.

#### Syntax

```
bleDevice.poll()
bleDevice.poll(timeout)

```

#### Parameters

- **timeout**: optional timeout in ms, to wait for event. If not specified defaults to 0 ms.

#### Returns
Nothing

#### Example

```arduino

  // listen for Bluetooth® Low Energy centrals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    

   central.poll();

    // ...
  }


```

### `bleDevice.connected()`

Query if a Bluetooth® Low Energy device is connected

#### Syntax

```
bleDevice.connected()

```

#### Parameters

None

#### Returns
- **true** if the Bluetooth® Low Energy device is connected,
- otherwise **false**.

#### Example

```arduino

  // listen for Bluetooth® Low Energy centrals to connect:
  BLEDevice central = BLE.central();

    // while the central is still connected
    while (central.connected()) {

       // ...
    }


```

### `bleDevice.disconnect()`

Disconnect the Bluetooth® Low Energy device, if connected

#### Syntax

```
bleDevice.disconnect()

```

#### Parameters

None

#### Returns
- **true** if the Bluetooth® Low Energy device was disconnected,
- otherwise **false**.

#### Example

```arduino

 // listen for Bluetooth® Low Energy centrals to connect:
  BLEDevice central = BLE.central();

  

  central.disconnect();


```

### `bleDevice.address()`

Query the Bluetooth® address of the Bluetooth® Low Energy device.

#### Syntax

```
bleDevice.address()

```

#### Parameters

None

#### Returns
- **Bluetooth® address** of the Bluetooth® Low Energy device (as a String).

#### Example

```arduino

  // listen for Bluetooth® Low Energy peripherals to connect:
  BLEDevice central = BLE.central();

  // if a central is connected to peripheral:
  if (central) {
    Serial.print("Connected to central: ");
    // print the central's MAC address:
    Serial.println(central.address());

    .
  }


```

### `bleDevice.rssi()`

Query the RSSI (Received signal strength indication) of the Bluetooth® Low Energy device.

#### Syntax

```
bleDevice.rssi()

```

#### Parameters

None

#### Returns
- **RSSI** of the connected Bluetooth® Low Energy device, 127 if the Bluetooth® Low Energy device is not connected.

#### Example

```arduino

  if (bleDevice.connected()) {
    

    Serial.print(“RSSI = “);
    Serial.println(bleDevice.rssi());
  }


```

### `bleDevice.characteristic()`

Get a BLECharacteristic representing a Bluetooth® Low Energy characteristic the device provides.

#### Syntax

```
bleDevice.characteristic(index)
bleDevice.characteristic(uuid)
bleDevice.characteristic(uuid, index)

```

#### Parameters

- **index**: index of characteristic
- **uuid**: uuid (as a **String**)

#### Returns
- **BLECharacteristic** for provided parameters

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    BLECharacteristic batteryLevelCharacterisic = peripheral.characteristic("2a19");

    if (batteryLevelCharacterisic) {
      // use the characteristic
    } else {
      Serial.println("Peripheral does NOT have battery level characteristic");
    }

    // ...
  }


```

### `bleDevice.discoverAttributes()`

Discover all of the attributes of Bluetooth® Low Energy device.

#### Syntax

```
bleDevice.discoverAttributes()

```

#### Parameters

None

#### Returns
- **true**, if successful, 
- **false** on failure.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    // ...
  }


```

### `bleDevice.discoverService()`

Discover the attributes of a particular service on the Bluetooth® Low Energy device.

#### Syntax

```
bleDevice.discoverService(serviceUuid)

```

#### Parameters

- **serviceUuid:** service UUID to discover (as a **String**)

#### Returns
- **true**, if successful,
- **false** on failure.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

    Serial.println("Connecting ...");

    if (peripheral.connect()) {
      Serial.println("Connected");
    } else {
      Serial.println("Failed to connect!");
      return;
    }

    // discover service attributes
    Serial.println("Discovering service attributes ...");
    if (peripheral.serviceUuid("fffe")) {
      Serial.println("Service attributes discovered");
    } else {
      Serial.println("Service attribute discovery failed!");
      peripheral.disconnect();
      return;
    }

    // ...
  }


```

### `bleDevice.deviceName()`

Query the device name (BLE characteristic UUID 0x2a00) of a Bluetooth® Low Energy device.

#### Syntax

```
bleDevice.deviceName()

```

#### Parameters

None

#### Returns
- **Device name** (as a String).

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    // ...
  }


```

### `bleDevice.appearance()`

Query the appearance (BLE characteristic UUID 0x2a01) of a Bluetooth® Low Energy device.

#### Syntax

```
bleDevice.appearance()

```

#### Parameters

None

#### Returns
- **Appearance value** (as a number).

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    // ...
  }


```

### `bleDevice.serviceCount()`

Query the number of services discovered for the Bluetooth® Low Energy device.

#### Syntax

```
bleDevice.serviceCount()

```

#### Parameters

None

#### Returns
- The number of **services discovered** for the Bluetooth® Low Energy device.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    int serviceCount = peripheral.serviceCount();

    Serial.print(serviceCount);
    Serial.println(" services discovered");

    // ...
  }


```

### `bleDevice.hasService()`

Query if the Bluetooth® Low Energy device has a particular service.

#### Syntax

```
bleDevice.hasService(uuid)
bleDevice.hasService(uuid, index)

```

#### Parameters

- **uuid**: uuid to check (as a **String**)
- **index**: optional, index of service to check if the device provides more than on. Defaults to 0, if not provided.

#### Returns
- **true**, if the device provides the service,
- **false** otherwise.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    if (peripheral.hasService("180f")) {
      Serial.println("Peripheral has battery service");
    }

    // ...
  }


```

### `bleDevice.service()`

Get a BLEService representing a Bluetooth® Low Energy service the device provides.

#### Syntax

```
bleDevice.service(index)
bleDevice.service(uuid)
bleDevice.service(uuid, index)

```

#### Parameters

- **index**: index of service
- **uuid**: uuid (as a **String**)

#### Returns
- **BLEService** for provided parameters

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    BLEService batteryService = peripheral.service("180f");

    if (batteryService) {
      // use the service
    } else {
      Serial.println("Peripheral does NOT have battery service");
    }

    // ...
  }


```

### `bleDevice.characteristicCount()`

Query the number of characteristics discovered for the Bluetooth® Low Energy device.

#### Syntax

```
bleDevice.characteristicCount()

```

#### Parameters

None

#### Returns
- The **number of characteristics** discovered for the Bluetooth® Low Energy device.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    int characteristicCount = peripheral.characteristicCount();

    Serial.print(characteristicCount);
    Serial.println(" characteristis discovered");

    // ...
  }


```

### `bleDevice.hasCharacteristic()`

Query if the Bluetooth® Low Energy device has a particular characteristic.

#### Syntax

```
bleDevice.hasCharacteristic(uuid)
bleDevice.hasCharacteristic(uuid, index)

```

#### Parameters

- **uuid**: uuid to check (as a **String**)
- **index**: optional, index of characteristic to check if the device provides more than on. Defaults to 0, if not provided.

#### Returns
- **true**, if the device provides the characteristic, 
- **false** otherwise.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    if (peripheral.hasCharacteristic("2a19")) {
      Serial.println("Peripheral has battery level characteristic");
    }

    // ...
  }


```

### `bleDevice.hasLocalName()`

Query if a discovered Bluetooth® Low Energy device is advertising a local name.

#### Syntax

```
bleDevice.hasLocalName()

```

#### Parameters

Nothing

#### Returns
- **true**, if the device is advertising a local name, 
- **false** otherwise.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

    // print the local name, if present
    if (peripheral.hasLocalName()) {
      Serial.print("Local Name: ");
      Serial.println(peripheral.localName());
    }

    // ...
  }


```

### `bleDevice.hasAdvertisedServiceUuid()`

Query if a discovered Bluetooth® Low Energy device is advertising a service UUID.

#### Syntax

```
bleDevice.hasAdvertisedServiceUuid()
bleDevice.hasAdvertisedServiceUuid(index)

```

#### Parameters

- **index**: optional, defaults to 0, the index of the service UUID, if the device is advertising more than one.

#### Returns
- **true**, if the device is advertising a service UUID, 
- **false** otherwise.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

    // print the advertised service UUIDs, if present
    if (peripheral.hasAdvertisedServiceUuid()) {
      Serial.print("Service UUIDs: ");
      for (int i = 0; i < peripheral.advertisedServiceUuidCount(); i++) {
        Serial.print(peripheral.advertisedServiceUuid(i));
        Serial.print(" ");
      }
      Serial.println();
    }

    // ...
  }


```

### `bleDevice.advertisedServiceUuidCount()`

Query the number of advertised services a discovered Bluetooth® Low Energy device is advertising.

#### Syntax

```
bleDevice.advertisedServiceUuidCount()

```

#### Parameters

None

#### Returns
- The **number of advertised services** a discovered Bluetooth® Low Energy device is advertising.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

    // print the advertised service UUIDs, if present
    if (peripheral.hasAdvertisedServiceUuid()) {
      Serial.print("Service UUIDs: ");
      for (int i = 0; i < peripheral.advertisedServiceUuidCount(); i++) {
        Serial.print(peripheral.advertisedServiceUuid(i));
        Serial.print(" ");
      }
      Serial.println();
    }

    // ...
  }


```

### `bleDevice.localName()`

Query the local name a discovered Bluetooth® Low Energy device is advertising with.

#### Syntax

```
bleDevice.localName()

```

#### Parameters

Nothing

#### Returns
- **Advertised local name** (as a String).

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

    // print the local name, if present
    if (peripheral.hasLocalName()) {
      Serial.print("Local Name: ");
      Serial.println(peripheral.localName());
    }

    // ...
  }


```

### `bleDevice.advertisedServiceUuid()`

Query an advertised service UUID discovered Bluetooth® Low Energy device is advertising.

#### Syntax

```
bleDevice.advertisedServiceUuid()
bleDevice.advertisedServiceUuid(index)

```

#### Parameters

- **index**: optional, defaults to 0, the index of the **service UUID**, if the device is advertising more than one.

#### Returns
- Advertised service **UUID** (as a String).

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

    // print the advertised service UUIDs, if present
    if (peripheral.hasAdvertisedServiceUuid()) {
      Serial.print("Service UUIDs: ");
      for (int i = 0; i < peripheral.advertisedServiceUuidCount(); i++) {
        Serial.print(peripheral.advertisedServiceUuid(i));
        Serial.print(" ");
      }
      Serial.println();
    }

    // ...
  }


```

### `bleDevice.connect()`

Connect to a Bluetooth® Low Energy device.

#### Syntax

```
bleDevice.connect()

```

#### Parameters

None

#### Returns
- **true**, if the connection was successful, 
- **false** otherwise.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

    Serial.println("Connecting ...");

    if (peripheral.connect()) {
      Serial.println("Connected");
    } else {
      Serial.println("Failed to connect!");
      return;
    }

    // ...
  }


```

## BLEService Class

Used to enable the services board provides or interact with services a remote board provides.

### `BLEService()`

Create a new Bluetooth® Low Energy service.

#### Syntax

```
BLEService(uuid)

```

#### Parameters

- **uuid**: 16-bit or 128-bit UUID in **String** format

#### Returns
- New **BLEService** with the specified **UUID**

#### Example

```arduino

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service


```

### `bleService.uuid()`

Query the UUID of the specified BLEService.

#### Syntax

```
bleService.uuid()

```

#### Parameters

None

#### Returns
- UUID of the Bluetooth® Low Energy service as a **String**.

#### Example

```arduino

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service


Serial.print(“LED service UUID = “);
Serial.println(ledService.uuid());



```

### `bleService.addCharacteristic()`

Add a BLECharateristic to the Bluetooth® Low Energy service.

#### Syntax

```
bleService.addCharacteristic(bleCharacteristic)

```

#### Parameters

None

#### Returns
Nothing

#### Example

```arduino

BLEService ledService("19B10000-E8F2-537E-4F6C-D104768A1214"); // Bluetooth® Low Energy LED Service

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, readable and writable by central
BLECharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite, 1);




// add the characteristic to the service
ledService.addCharacteristic(switchCharacteristic);



```

### `bleService.characteristicCount()`

Query the number of characteristics discovered for the Bluetooth® Low Energy service.

#### Syntax

```
bleService.characteristicCount()

```

#### Parameters

None

#### Returns
- The **number of characteristics** discovered for the Bluetooth® Low Energy service.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    BLEService batteryService = peripheral.service("180f");

    if (batteryService) {
      // use the service
      int characteristicCount = batteryService.characteristicCount();

      Serial.print(characteristicCount);
      Serial.println(" characteristics discovered in battery service");
    } else {
      Serial.println("Peripheral does NOT have battery service");
    }

    // ...
  }


```

### `bleService.hasCharacteristic()`

Query if the Bluetooth® Low Energy service has a particular characteristic.

#### Syntax

```
bleService.hasCharacteristic(uuid)
bleService.hasCharacteristic(uuid, index)

```

#### Parameters

- **uuid**: uuid to check (as a **String**)
- **index**: optional, index of characteristic to check if the device provides more than on. Defaults to 0, if not provided.

#### Returns
- **true**, if the service provides the characteristic,
- **false** otherwise.

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  
  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    BLEService batteryService = peripheral.service("180f");

    if (batteryService) {
      // use the service
      if (batteryService.hasCharacteristic("2a19")) {
        Serial.println("Battery service has battery level characteristic");
      }
    } else {
      Serial.println("Peripheral does NOT have battery service");
    }

    // ...
  }


```

### `bleService.characteristic()`

Get a BLECharacteristic representing a Bluetooth® Low Energy characteristic the service provides.

#### Syntax

```
bleService.characteristic(index)
bleService.characteristic(uuid)
bleService.characteristic(uuid, index)

```

#### Parameters

- **index**: index of characteristic
- **uuid**: uuid (as a **String**)

#### Returns
- **BLECharacteristic** for provided parameters

#### Example

```arduino

  // begin initialization
  if (!BLE.begin()) {
    Serial.println("starting Bluetooth® Low Energy module failed!");

    while (1);
  }

  Serial.println("BLE Central scan");

  // start scanning for peripheral
  BLE.scan();

  

  BLEDevice peripheral = BLE.available();

  if (peripheral) {
    // ...

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

    BLEService batteryService = peripheral.service("180f");

    if (batteryService) {
      // use the service
      BLECharacteristic batteryLevelCharacterisic = peripheral.characteristic("2a19");

      if (batteryLevelCharacterisic) {
        // use the characteristic
      } else {
        Serial.println("Peripheral does NOT have battery level characteristic");
      }
    } else {
      Serial.println("Peripheral does NOT have battery service");
    }

    // ...
  }


```

## BLECharacteristic Class

Used to enable the characteristics board offers in a service or interact with characteristics a remote board provides.

### `BLECharacteristic()`

Create a new Bluetooth® Low Energy characteristic.

#### Syntax

```
BLECharacteristic(uuid, properties, value, valueSize)
BLECharacteristic(uuid, properties, stringValue)

BLEBoolCharacteristic(uuid, properties)
BLEBooleanCharacteristic(uuid, properties)
BLECharCharacteristic(uuid, properties)
BLEUnsignedCharCharacteristic(uuid, properties)
BLEByteCharacteristic(uuid, properties)
BLEShortCharacteristic(uuid, properties)
BLEUnsignedShortCharacteristic(uuid, properties)
BLEWordCharacteristic(uuid, properties)
BLEIntCharacteristic(uuid, properties)
BLEUnsignedIntCharacteristic(uuid, properties)
BLELongCharacteristic(uuid, properties)
BLEUnsignedLongCharacteristic(uuid, properties)
BLEFloatCharacteristic(uuid, properties)
BLEDoubleCharacteristic(uuid, properties)
```

#### Parameters

- **uuid**: 16-bit or 128-bit UUID in **String** format
- **properties**: mask of the properties (BLEBroadcast, BLERead, BLEWriteWithoutResponse, BLEWrite, BLENotify, BLEIndicate)
- **valueSize**: (maximum) size of characteristic value
- **stringValue**: value as a string

#### Returns
- New **BLECharacteristic** with the specified **UUID** and value

#### Example

```arduino

// Bluetooth® Low Energy Battery Level Characteristic
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",  // standard 16-bit characteristic UUID
    BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes



```

### `bleCharacteristic.uuid()`

Query the UUID of the specified BLECharacteristic.

#### Syntax

```
bleCharacteristic.uuid()

```

#### Parameters

None

#### Returns
- **UUID** of the Bluetooth® Low Energy service as a **String**.

#### Example

```arduino

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);


Serial.print(“Switch characteristic UUID = “);
Serial.println(switchCharacteristic.uuid());



```

### `bleCharacteristic.properties()`

Query the property mask of the specified BLECharacteristic.

#### Syntax

```
bleCharacteristic.properties()

```

#### Parameters

None

#### Returns
- **Properties of the characteristic masked** (BLEBroadcast, BLERead, BLEWriteWithoutResponse, BLEWrite, BLENotify, BLEIndicate)

#### Example

```arduino

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);


byte properties = switchCharacteristic.properties();

if (properties & BLERead) {
  // characteristic is readable ...
}

if (properties & (BLEWrite | BLEWriteWithoutResponse)) {
  // characteristic is writable ...
}


```

### `bleCharacteristic.valueSize()`

Query the maximum value size of the specified BLECharacteristic.

#### Syntax

```
bleCharacteristic.valueSize()

```

#### Parameters

None

#### Returns
- The **maximum value** size of the characteristic (in bytes)

#### Example

```arduino

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);



Serial.print(“value size = “);
Serial.println(switchCharacteristic.valueSize());


```

### `bleCharacteristic.value()`

Query the current value of the specified BLECharacteristic.

#### Syntax

```
bleCharacteristic.value()

```

#### Parameters

None

#### Returns
- The **current value** of the characteristic, value type depends on the constructor used

#### Example

```arduino

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);



      if (switchCharacteristic.value()) {   // any value other than 0
          Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }



```

### `bleCharacteristic.valueLength()`

Query the current value size of the specified BLECharacteristic.

#### Syntax

```
bleCharacteristic.valueLength()

```

#### Parameters

None

#### Returns
- The **current value** size of the characteristic (in bytes)

#### Example

```arduino

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);



Serial.print(“value length = “);
Serial.println(switchCharacteristic.valueLength());


```

### `bleCharacteristic.readValue()`

Read the current value of the characteristic. If the characteristic is on a remote device, a read request will be sent.

#### Syntax

```
bleCharacteristic.readValue(buffer, length)
bleCharacteristic.readValue(value)

```

#### Parameters

- **buffer:** byte array to read value into length: size of buffer argument in bytes 
- **value**: variable to read value into (by reference)

#### Returns
- **Number of bytes** read

#### Example

```arduino

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


```

### `bleCharacteristic.writeValue()`

Write the value of the characteristic. If the characteristic is on a remote device, a write request or command will be sent.

#### Syntax

```
bleCharacteristic.writeValue(buffer, length)
bleCharacteristic.writeValue(value)

```

#### Parameters

- **buffer**: byte array to write value with
- **length**: number of bytes of the buffer argument to write
- **value**: value to write

#### Returns
- 1 on success, 
- 0 on failure

#### Example

```arduino

    // read the button pin
    int buttonState = digitalRead(buttonPin);

    if (oldButtonState != buttonState) {
      // button changed
      oldButtonState = buttonState;

      if (buttonState) {
        Serial.println("button pressed");

        // button is pressed, write 0x01 to turn the LED on
        ledCharacteristic.writeValue((byte)0x01);
      } else {
        Serial.println("button released");

        // button is released, write 0x00 to turn the LED off
        ledCharacteristic.writeValue((byte)0x00);
      }
    }


```

### `bleCharacteristic.setEventHandler()`

Set the event handler (callback) function that will be called when the specified event occurs.

#### Syntax

```
bleCharacteristic.setEventHandler(eventType, callback)

```

#### Parameters

- **eventType**: event type (BLESubscribed, BLEUnsubscribed, BLERead, BLEWritten) 
- **callback**: function to call when the event occurs

#### Returns
Nothing

#### Example

```arduino

// create switch characteristic and allow remote device to read and write
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);




  // assign event handlers for characteristic
  switchCharacteristic.setEventHandler(BLEWritten, switchCharacteristicWritten);



void switchCharacteristicWritten(BLEDevice central, BLECharacteristic characteristic) {
  // central wrote new value to characteristic, update LED
  Serial.print("Characteristic event, written: ");

  if (switchCharacteristic.value()) {
    Serial.println("LED on");
    digitalWrite(ledPin, HIGH);
  } else {
    Serial.println("LED off");
    digitalWrite(ledPin, LOW);
  }
}

  

```

### `bleCharacteristic.broadcast()`

Broadcast the characteristics value as service data when advertising.

#### Syntax

```
bleCharacteristic.broadcast()

```

#### Parameters

None

#### Returns
- 1 on success,
- 0 on failure

#### Example

```arduino

// create button characteristic and allow remote device to get notifications
BLEByteCharacteristic buttonCharacteristic("19B10012-E8F2-537E-4F6C-D104768A1214", BLERead | BLENotify | BLEBroadcast);



buttonCharacteristic.broadcast();



```

### `bleCharacteristic.written()`

Query if the characteristic value has been written by another Bluetooth® Low Energy device.

#### Syntax

```
bleCharacteristic.written()

```

#### Parameters

None

#### Returns
- **true** if the characteristic value has been written by another Bluetooth® Low Energy device,
- **false** otherwise

#### Example

```arduino

// Bluetooth® Low Energy LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic("19B10001-E8F2-537E-4F6C-D104768A1214", BLERead | BLEWrite);




 // listen for Bluetooth® Low Energy peripherals to connect:
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




```

### `bleCharacteristic.subscribed()`

Query if the characteristic has been subscribed to by another Bluetooth® Low Energy device.

#### Syntax

```
bleCharacteristic.subscribed()

```

#### Parameters

None

#### Returns
- **true** if the characteristic value has been subscribed to by another Bluetooth® Low Energy device,
- **false** otherwise

#### Example

```arduino

// Bluetooth® Low Energy Battery Level Characteristic
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",  // standard 16-bit characteristic UUID
    BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes





  if (batteryLevelChar.subscribed()) {
     // set a new value , that well be pushed to subscribed Bluetooth® Low Energy devices
    batteryLevelChar.writeValue(0xab);
  }


```

### `bleCharacteristic.addDescriptor()`

Add a BLEDescriptor to the characteristic.

#### Syntax

```
bleCharacteristic.addDescriptor(bleDescriptor)

```

#### Parameters

- **bleDescriptor**: descriptor to add to the characteristic

#### Returns
Nothing

#### Example

```arduino

// Bluetooth® Low Energy Battery Level Characteristic
BLEUnsignedCharCharacteristic batteryLevelChar("2A19",  // standard 16-bit characteristic UUID
    BLERead | BLENotify); // remote clients will be able to get notifications if this characteristic changes

BLEDescriptor batteryLevelDescriptor("2901", "millis");





  batteryLevelChar.addDescriptor(batteryLevelDescriptor);


```

### `bleCharacteristic.descriptorCount()`

Query the number of Bluetooth® Low Energy descriptors discovered for the characteristic.

#### Syntax

```
bleCharacteristic.descriptorCount()

```

#### Parameters

None

#### Returns
- The **number of Bluetooth® Low Energy descriptors** discovered for the characteristic

#### Example

```arduino

 // loop the descriptors of the characteristic and explore each
  for (int i = 0; i < characteristic.descriptorCount(); i++) {
    BLEDescriptor descriptor = characteristic.descriptor(i);

    // ...
  }


```

### `bleCharacteristic.hasDescriptor()`

Check if a characteristic has a particular descriptor.

#### Syntax

```
bleCharacteristic.hasDescriptor(uuid)
bleCharacteristic.hasDescriptor(uuid, index)

```

#### Parameters

- **index**: index of descriptor
- **uuid**: uuid (as a **String**)

#### Returns
- **true**, if the characteristic has a matching descriptor,
- otherwise **false**.

#### Example

```arduino

  if (characteristic.hasDescriptor("2901")) {
    Serial.println("characteristic has description descriptor");
  }


```

### `bleCharacteristic.descriptor()`

Get a BLEDescriptor that represents a characteristics Bluetooth® Low Energy descriptor.

#### Syntax

```
bleCharacteristic.descriptor(index)
bleCharacteristic.descriptor(uuid)
bleCharacteristic.descriptor(uuid, index)

```

#### Parameters

- **index**: index of descriptor
- **uuid**: uuid (as a **String**)

#### Returns
- BLEDescriptor that represents a characteristics Bluetooth® Low Energy descriptor

#### Example

```arduino

  if (characteristic.hasDescriptor("2901")) {
    Serial.println("characteristic has description descriptor");
  }


```

### `bleCharacteristic.canRead()`

Query if a Bluetooth® Low Energy characteristic is readable.

#### Syntax

```
bleCharacteristic.canRead()

```

#### Parameters

None

#### Returns
- **true**, if characteristic is readable, 
- **false** otherwise

#### Example

```arduino

  if (characteristic.canRead("2901")) {
    Serial.println("characteristic is readable");
  }


```

read

Perform a read request for the characteristic.

#### Syntax

```
bleCharacteristic.read()

```

#### Parameters

None

#### Returns
- **true**, if successful, 
- **false** on failure

#### Example

```arduino

  if (characteristic.read()) {
    Serial.println("characteristic value read");

    // ...
  } else {
    Serial.println("error reading characteristic value");
  }


```

### `bleCharacteristic.canWrite()`

Query if a Bluetooth® Low Energy characteristic is writable.

#### Syntax

```
bleCharacteristic.canWrite()

```

#### Parameters

None

#### Returns
- **true**, if characteristic is writable,
- **false** otherwise

#### Example

```arduino

  if (characteristic.canWrite()) {
    Serial.println("characteristic is writable");
  }


```

### `bleCharacteristic.canSubscribe()`

Query if a Bluetooth® Low Energy characteristic is subscribable.

#### Syntax

```
bleCharacteristic.canSubscribe()

```

#### Parameters

None

#### Returns
- **true**, if characteristic is subscribable,
- **false** otherwise

#### Example

```arduino

  if (characteristic.canSubscribe()) {
    Serial.println("characteristic is subscribable");
  }


```

### `bleCharacteristic.subscribe()`

Subscribe to a Bluetooth® Low Energy characteristics notification or indications.

#### Syntax

```
bleCharacteristic.subscribe()

```

#### Parameters

None

#### Returns
- **true**, on success,
- **false** on failure

#### Example

```arduino

  // ...

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
  }

  // ...


```

### `bleCharacteristic.canUnsubscribe()`

Query if a Bluetooth® Low Energy characteristic is unsubscribable.

#### Syntax

```
bleCharacteristic.canUnsubscribe()

```

#### Parameters

None

#### Returns
- **true**, if characteristic is unsubscribable,
- **false** otherwise

#### Example

```arduino

  if (characteristic.canUnsubscribe()) {
    Serial.println("characteristic is unsubscribable");
  }


```

### `bleCharacteristic.unsubscribe()`

Unsubscribe to a Bluetooth® Low Energy characteristics notifications or indications.

#### Syntax

```
bleCharacteristic.unsubscribe()

```

#### Parameters

None

#### Returns
- **true**, on success,
- **false** on failure

#### Example

```arduino

  // ...

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
  }

  // ...

  simpleKeyCharacteristic.unsubscribe();


```

### `bleCharacteristic.valueUpdated()`

Has the characteristics value been updated via a notification or indication.

#### Syntax

```
bleCharacteristic.valueUpdated()

```

#### Parameters

None

#### Returns
- **true**, if the characteristics value been updated via a notification or indication

#### Example

```arduino

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


```

## BLEDescriptor Class

Used to describe a characteristic the board offers

### `BLEDescriptor()`

Create a new Bluetooth® Low Energy descriptor.

#### Syntax

```
BLEDescriptor(uuid, value, valueSize)
BLEDescriptor(uuid, stringValue)

```

#### Parameters

- **uuid**: 16-bit or 128-bit UUID in string format
- **value**: byte array value
- **valueSize**: size of byte array value
- **stringValue**: value as a string

#### Returns
- New **BLEDescriptor** with the specified **UUID** and value

#### Example

```arduino

BLEDescriptor millisLabelDescriptor("2901", "millis");


```

### `bleDescriptor.uuid()`

Query the UUID of the specified BLEDescriptor.

#### Syntax

```
bleDescriptor.uuid()

```

#### Parameters

None

#### Returns
- **UUID** of the Bluetooth® Low Energy descriptor (as a String).

#### Example

```arduino

BLEDescriptor millisLabelDescriptor("2901", "millis");


Serial.print(“millis label descriptor UUID = “);
Serial.println(millisLabelDescriptor.uuid());



```

### `bleDescriptor.valueSize()`

Query the value size of the specified BLEDescriptor.

#### Syntax

```
bleDescriptor.valueSize()

```

#### Parameters

None

#### Returns
- **Value size** (in bytes) of the Bluetooth® Low Energy descriptor.

#### Example

```arduino

BLEDescriptor millisLabelDescriptor("2901", "millis");


Serial.print(“millis label descriptor value size = “);
Serial.println(millisLabelDescriptor.valueSize());



```

### `bleDescriptor.valueLength()`

Query the length, in bytes, of the descriptor current value.

#### Syntax

```
bleDescriptor.valueLength()

```

#### Parameters

None

#### Returns
- **Length of descriptor** value in bytes.

#### Example

```arduino

  // read the descriptor value
  descriptor.read();

  // print out the value of the descriptor
  Serial.print(", value 0x");
  printData(descriptor.value(), descriptor.valueLength());

  // ...

  void printData(const unsigned char data[], int length) {
    for (int i = 0; i < length; i++) {
      unsigned char b = data[i];

      if (b < 16) {
        Serial.print("0");
      }

      Serial.print(b, HEX);
    }
  }


```

### `bleDescriptor.value()`

Query the value of the specified BLEDescriptor.

#### Syntax

```
bleDescriptor.value()

```

#### Parameters

None

#### Returns
- Value byte array of the **BLE descriptor**.

#### Example

```arduino

BLEDescriptor millisLabelDescriptor("2901", "millis");



  int descriptorValueSize = millisLabelDescriptor.valueSize();
  byte descriptorValue[descriptorValueSize];

  for (int i = 0; i < descriptorValueSize; i++) {
    descriptorValue[i] = millisLabelDescriptor.value()[i];
  }



```

### `bleDescriptor.readValue()`

Read the current value of the descriptor. If the descriptor is on a remote device, a read request will be sent.

#### Syntax

```
bleDescriptor.readValue(buffer, length)
bleDescriptor.readValue(value)

```

#### Parameters

- **buffer**: byte array to read value into
- **length**: size of buffer argument in bytes
- **value**: variable to read value into (by reference)

#### Returns
- **Number of bytes** read

#### Example


```arduino

  byte value = 0;

  /get the value, descriptor is 1 byte so use byte value    
  descriptor.readValue(value);


```

### `bleDescriptor.read()`

Perform a read request for the descriptor.

#### Syntax

```
bleDescriptor.read()

```

#### Parameters

None

#### Returns
- **true**, if successful,
- **false** on failure

#### Example

```arduino

  if (descriptor.read()) {
    Serial.println("descriptor value read");

    // ...
  } else {
    Serial.println("error reading descriptor value");
  }

```
