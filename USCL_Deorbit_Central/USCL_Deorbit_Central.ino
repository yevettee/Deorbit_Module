/*
  -----------------------------------------------------------------------------------------------
 | BLE_IMU_PERIPHERAL - Wireless IMU Communication with central device
 |
 | Arduino Boards Tested: Nano 33 BLE Sense as a peripheral & Nano 33 BLE as central.
 | Code not tested for multiple peripherals

 | This sketch works alongside the BLE_IMU_CENTRAL sketch to communicate with an Arduino Nano 33 BLE. 
 | This sketch can also be used with a generic BLE central app, like LightBlue (iOS and Android) or
 | nRF Connect (Android), to interact with the services and characteristics created in this sketch.
 
 | This example code is adapted from the ArduinoBLE library, available in the public domain.
 | Authors: Aaron Yurkewich & Pilar Zhang Qiu
 | Latest Update: 25/02/2021
 
 | USCL Arduino BLE Revision
 | Updated Date : 28/10/2023
 | Add Gyroscope data into BLE advertise packet
  -----------------------------------------------------------------------------------------------
*/

#include <ArduinoBLE.h>
#include <Arduino_LSM9DS1.h>
//#include <Arduino_LSM6DS3.h> // Uncomment this if your peripheral is the Nano 33 IoT

// ------------------------------------------ BLE UUIDs ------------------------------------------
#define BLE_UUID_PERIPHERAL               "19B10000-E8F2-537E-4F6C-D104768A1214" //please chnage to a unique value that matches BLE_IMU_CENTRAL
#define BLE_UUID_CHARACT_LED              "19B10001-E8F2-537E-4F6C-E104768A1214" //please chnage to a unique value that matches BLE_IMU_CENTRAL

#define BLE_UUID_CHARACT_ACCX             "29B10001-E8F2-537E-4F6C-a204768A1215" //please chnage to a unique value that matches BLE_IMU_CENTRAL
#define BLE_UUID_CHARACT_ACCY             "39B10001-E8F2-537E-4F6C-a204768A1215" //please chnage to a unique value that matches BLE_IMU_CENTRAL
#define BLE_UUID_CHARACT_ACCZ             "49B10001-E8F2-537E-4F6C-a204768A1215" //please chnage to a unique value that matches BLE_IMU_CENTRAL

#define BLE_UUID_CHARACT_GYROX            "59B10001-E8F2-537E-4F6C-a204768A1215"
#define BLE_UUID_CHARACT_GYROY            "69B10001-E8F2-537E-4F6C-a204768A1215"
#define BLE_UUID_CHARACT_GYROZ            "79B10001-E8F2-537E-4F6C-a204768A1215"

#define BLE_UUID_CHARACT_VAR1             "89B10001-E8F2-537E-4F6C-a204768A1215"
#define BLE_UUID_CHARACT_VAR2             "99B10001-E8F2-537E-4F6C-a204768A1215"
#define BLE_UUID_CHARACT_VAR3             "A9B10001-E8F2-537E-4F6C-a204768A1215"

BLEService Deorbit_Module_Service(BLE_UUID_PERIPHERAL); // BLE LED Service

// BLE LED Switch Characteristic - custom 128-bit UUID, read and writable by central
BLEByteCharacteristic switchCharacteristic(BLE_UUID_CHARACT_LED, BLERead | BLEWrite);

BLEFloatCharacteristic accXCharacteristic(BLE_UUID_CHARACT_ACCX, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic accYCharacteristic(BLE_UUID_CHARACT_ACCY, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic accZCharacteristic(BLE_UUID_CHARACT_ACCZ, BLERead | BLENotify | BLEWrite);

BLEFloatCharacteristic gyroXCharacteristic(BLE_UUID_CHARACT_GYROX, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic gyroYCharacteristic(BLE_UUID_CHARACT_GYROY, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic gyroZCharacteristic(BLE_UUID_CHARACT_GYROZ, BLERead | BLENotify | BLEWrite);

BLEFloatCharacteristic var1Characteristic(BLE_UUID_CHARACT_VAR1, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic var2Characteristic(BLE_UUID_CHARACT_VAR2, BLERead | BLENotify | BLEWrite);
BLEFloatCharacteristic var3Characteristic(BLE_UUID_CHARACT_VAR3, BLERead | BLENotify | BLEWrite);

const int ledPin = LED_BUILTIN; // pin to use for the LED

float accelX, accelY, accelZ;
float gyroX, gyroY, gyroZ;
float var1 = 1.0f;
float var2 = 2.0f;
float var3 = 3.0f; // ".0f" is fucking important!!!!!

// ------------------------------------------ VOID SETUP ------------------------------------------
void setup() {
  Serial.begin(9600); 
  //while (!Serial); //uncomment to view the IMU data in the peripheral serial monitor

  // begin IMU initialization
  if (!IMU.begin()) {
    Serial.println("Failed to initialize IMU!");
    while (1);
  }

  // set LED pin to output mode
  pinMode(ledPin, OUTPUT);

  // begin BLE initialization
  if (!BLE.begin()) {
    Serial.println("starting BLE failed!");
    while (1);
  }

  // set advertised local name and service UUID:
  BLE.setLocalName("Deorbit_Module");
  BLE.setAdvertisedService(Deorbit_Module_Service);

  // add the characteristic to the service
  Deorbit_Module_Service.addCharacteristic(switchCharacteristic);
  Deorbit_Module_Service.addCharacteristic(accXCharacteristic);
  Deorbit_Module_Service.addCharacteristic(accYCharacteristic);
  Deorbit_Module_Service.addCharacteristic(accZCharacteristic);

  Deorbit_Module_Service.addCharacteristic(gyroXCharacteristic);
  Deorbit_Module_Service.addCharacteristic(gyroYCharacteristic);
  Deorbit_Module_Service.addCharacteristic(gyroZCharacteristic);

  Deorbit_Module_Service.addCharacteristic(var1Characteristic);
  Deorbit_Module_Service.addCharacteristic(var2Characteristic);
  Deorbit_Module_Service.addCharacteristic(var3Characteristic);

  // add service
  BLE.addService(Deorbit_Module_Service);

  // set the initial value for the characeristic:
  switchCharacteristic.writeValue(0);

  // set advertising interval
  // BLE.setAdvertisingInterval(320); // 200 ms (default : 160 => 100ms)
  // reference : https://www.arduino.cc/reference/en/libraries/arduinoble/ble.setadvertisinginterval/
  // start advertising
  BLE.advertise();

  Serial.println("BLE Peripheral");
}

// ------------------------------------------ VOID LOOP ------------------------------------------
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
          // Serial.println("LED on");
          digitalWrite(ledPin, HIGH);         // will turn the LED on
        } else {                              // a 0 value
          // Serial.println(F("LED off"));
          digitalWrite(ledPin, LOW);          // will turn the LED off
        }
      }

      if (IMU.gyroscopeAvailable()) {
        IMU.readGyroscope(gyroX, gyroY, gyroZ);

        gyroXCharacteristic.writeValue(gyroX);
        gyroYCharacteristic.writeValue(gyroY);
        gyroZCharacteristic.writeValue(gyroZ);
        
        // Serial.println("Gyro Data");
        // Serial.print(x); 
        // Serial.print('\t');
        // Serial.print(y); 
        // Serial.print('\t');         
        // Serial.print(z); 
        // Serial.println('\t'); 
      }

      if (IMU.accelerationAvailable()) {
        IMU.readAcceleration(accelX, accelY, accelZ);

        accXCharacteristic.writeValue(accelX);
        accYCharacteristic.writeValue(accelY);
        accZCharacteristic.writeValue(accelZ);

        // Serial.println("Gyro Data");
        // Serial.print(x); 
        // Serial.print('\t');
        // Serial.print(y); 
        // Serial.print('\t');         
        // Serial.print(z); 
        // Serial.println('\t'); 
      }
      var1Characteristic.writeValue(var1);
      var2Characteristic.writeValue(var2);
      var3Characteristic.writeValue(var3);
    }

      // when the central disconnects, print it out:
      Serial.print(F("Disconnected from central: "));
      Serial.println(central.address());
    }
  }