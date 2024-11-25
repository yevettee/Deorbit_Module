/*
  -----------------------------------------------------------------------------------------------
 | BLE_IMU_CENTRAL - Wireless IMU Communication with peripheral device
 |
 | Arduino Boards Tested: Nano 33 BLE Sense as a peripheral & Nano 33 BLE as central.
 | Code not tested for multiple peripherals
 |
 | This sketch works alongside the BLE_IMU_PERIPHERAL sketch to communicate with another Arduino BLE. 
 | This sketch can also be used with a generic BLE central app, like LightBlue (iOS and Android) or
 | nRF Connect (Android), to interact with the services and characteristics created in this sketch.
 |
 | This example code is adapted from the ArduinoBLE library, available in the public domain.
 | Authors: Aaron Yurkewich & Pilar Zhang Qiu
 | Latest Update: 25/02/2021
  -----------------------------------------------------------------------------------------------
*/
#include <ArduinoBLE.h>
// ------------------------------------------ BLE UUIDs ------------------------------------------
#define BLE_UUID_PERIPHERAL               "19B10000-E8F2-537E-4F6C-D104768A1214"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_LED              "19B10001-E8F2-537E-4F6C-E104768A1214"  //please change to a unique value that matches BLE_IMU_PERIPHERAL

#define BLE_UUID_CHARACT_ACCX             "29B10001-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_ACCY             "39B10001-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL
#define BLE_UUID_CHARACT_ACCZ             "49B10001-E8F2-537E-4F6C-a204768A1215"  //please change to a unique value that matches BLE_IMU_PERIPHERAL

#define BLE_UUID_CHARACT_GYROX            "59B10001-E8F2-537E-4F6C-a204768A1215"
#define BLE_UUID_CHARACT_GYROY            "69B10001-E8F2-537E-4F6C-a204768A1215"
#define BLE_UUID_CHARACT_GYROZ            "79B10001-E8F2-537E-4F6C-a204768A1215"

#define BLE_UUID_CHARACT_VAR1             "89B10001-E8F2-537E-4F6C-a204768A1215"
#define BLE_UUID_CHARACT_VAR2             "99B10001-E8F2-537E-4F6C-a204768A1215"
#define BLE_UUID_CHARACT_VAR3             "A9B10001-E8F2-537E-4F6C-a204768A1215"
// ------------------------------------------ VOID SETUP ------------------------------------------
void setup() {
  Serial.begin(9600);
  while (!Serial);

  // configure the button pin as input
  pinMode(LED_BUILTIN, OUTPUT);

  // initialize the BLE hardware
  BLE.begin();

  Serial.println("BLE Central - Gyroscope control");

  // start scanning for peripherals
  BLE.scanForUuid(BLE_UUID_PERIPHERAL);//
}

// ------------------------------------------ VOID LOOP ------------------------------------------
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
    Serial.print("' ");
    Serial.print(peripheral.advertisedServiceUuid());
    Serial.println();

    if (peripheral.localName() != "Deorbit_Module") {
      return;
    }

    // stop scanning
    BLE.stopScan();

    LED_IMU(peripheral);

    // peripheral disconnected, start scanning again
    BLE.scanForUuid(BLE_UUID_PERIPHERAL);
  }
}

// ------------------------------------------ FUNCTIONS ------------------------------------------
void LED_IMU(BLEDevice peripheral) {
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
  BLECharacteristic ledCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_LED);
  
  BLECharacteristic accXCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_ACCX);
  BLECharacteristic accYCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_ACCY);
  BLECharacteristic accZCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_ACCZ);

  BLECharacteristic gyroXCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_GYROX);
  BLECharacteristic gyroYCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_GYROY);
  BLECharacteristic gyroZCharacteristic = peripheral.characteristic(BLE_UUID_CHARACT_GYROZ);
  
  BLECharacteristic var1Characteristic = peripheral.characteristic(BLE_UUID_CHARACT_VAR1);
  BLECharacteristic var2Characteristic = peripheral.characteristic(BLE_UUID_CHARACT_VAR2);
  BLECharacteristic var3Characteristic = peripheral.characteristic(BLE_UUID_CHARACT_VAR3);
  // check if an specific BLE characteristic exists
  if (!ledCharacteristic) {
    Serial.println("Peripheral does not have LED characteristic!");
    peripheral.disconnect();
    return;
  } else if (!ledCharacteristic.canWrite()) {
    Serial.println("Peripheral does not have a writable LED characteristic!");
    peripheral.disconnect();
    return;
  }

  
  int buttonState = 0;

  float accelX, accelY, accelZ;
  float gyroX, gyroY, gyroZ;
  float var1,var2,var3;

  while (peripheral.connected()) {
    // while the peripheral is connected
    // read the gyroscope values
    accXCharacteristic.readValue( &accelX, 4 );
    accYCharacteristic.readValue( &accelY, 4 );
    accZCharacteristic.readValue( &accelZ, 4 );

    gyroXCharacteristic.readValue( &gyroX, 4 );
    gyroYCharacteristic.readValue( &gyroY, 4 );
    gyroZCharacteristic.readValue( &gyroZ, 4 );

    var1Characteristic.readValue( &var1, 4 );
    var2Characteristic.readValue( &var2, 4 );
    var3Characteristic.readValue( &var3, 4 );

    Serial.print(accelX);
    Serial.print('\t');
    Serial.print(accelY);
    Serial.print('\t');
    Serial.print(accelZ);
    Serial.print('\t');

    Serial.print(gyroX);
    Serial.print('\t');
    Serial.print(gyroY);
    Serial.print('\t');
    Serial.print(gyroZ);
    Serial.print('\t');

    Serial.print(var1);
    Serial.print('\t');
    Serial.print(var2);
    Serial.print('\t');
    Serial.print(var3);
    Serial.print('\t');
    Serial.println();

    // make the LED blink
    if (buttonState == 0)
    {buttonState = 1;}
    else if (buttonState == 1)
    {buttonState = 0;}
    
    digitalWrite(LED_BUILTIN, buttonState);
      if (buttonState == 0) {
        // write 0x01 to turn the LED on
        ledCharacteristic.writeValue((byte)0x01);
      } else {
        // write 0x00 to turn the LED off
        ledCharacteristic.writeValue((byte)0x00);
      }
  }
  Serial.println("Peripheral disconnected");
}