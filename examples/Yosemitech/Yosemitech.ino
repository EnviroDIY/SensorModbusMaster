/*****************************************************************************
Yosemitech.ino

This is just getting and setting various registers of a Yosemitech DO sensor
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <SensorModbusMaster.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define the sensor's modbus address
byte modbusAddress = 0x04;  // The sensor's modbus address, or SlaveID
// Yosemitech ships sensors with a default ID of 0x01.

// Define pin number variables
const int PwrPin = 22;  // The pin sending power to the sensor *AND* RS485 adapter
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text
const int SSRxPin = 10;  // Recieve pin for software serial (Rx on RS485 adapter)
const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)

// Construct software serial object for Modbus
// #if defined(ARDUINO_AVR_UNO)
    #include <SoftwareSerial.h>
    SoftwareSerial modbusSerial(SSRxPin, SSTxPin);
// #endif

// AltSoftSerial object for Modbus
// #include <AltSoftSerial.h>  // include the AltSoftSerial library
// AltSoftSerial modbusSerial;

// Construct the modbus instance
modbusMaster sensor;


// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{
    pinMode(PwrPin, OUTPUT);
    digitalWrite(PwrPin, HIGH);

    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor

    // #if defined(ARDUINO_AVR_UNO)
        modbusSerial.begin(9600);  // port for communicating with sensor
        sensor.begin(modbusAddress, modbusSerial, DEREPin);
    // #else
    //     Serial1.begin(9600, SERIAL_8N1);  // port for communicating with sensor
    //     sensor.begin(modbusAddress, &Serial1, DEREPin);
    // #endif

    // Turn on debugging
    sensor.setDebugStream(&Serial);

    // Allow the sensor and converter to warm up
    Serial.println("Waiting for sensor and adapter to be ready.");
    delay(1500);

    // Get the sensor serial number
    Serial.println(sensor.StringFromRegister(0x03, 0x0900, 14));
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{}
