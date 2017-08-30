/*****************************************************************************
getParameterValues.ino

This prints basic meta-data about a sensor to the first serial port and then
begins taking measurements from the sensor.

The sensor model and address can easily be modified to use this sketch with any
Yosemitech modbus sensor.
*****************************************************************************/

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <SoftwareSerial.h>
#include <scanModbus.h>

// ---------------------------------------------------------------------------
// Set up the sensor specific information
//   ie, pin locations, addresses, calibrations and related settings
// ---------------------------------------------------------------------------

// Define the sensor's modbus address
byte modbusAddress = 0x04;  // The sensor's modbus address, or SlaveID
// Yosemitech ships sensors with a default ID of 0x01.

// Define pin number variables
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text
const int SSRxPin = 10;  // Recieve pin for software serial (Rx on RS485 adapter)
const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)

// Construct software serial object for Modbus
SoftwareSerial modbusSerial(SSRxPin, SSTxPin);

// Construct the Yosemitech modbus instance
scan sensor;
bool success;

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{
    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor
    modbusSerial.begin(38400);  // The modbus serial stream
    // The default baud rate for the spectro::lyzer is 38400

    // Start up the sensor
    sensor.begin(modbusAddress, &modbusSerial, DEREPin);

    // Turn on debugging
    sensor.setDebugStream(&Serial);

    // Start up note
    Serial.println("S::CAN Spect::lyzer Test");

    // Allow the sensor and converter to warm up
    Serial.println("Waiting for sensor and adapter to be ready.");
    delay(500);

    // Print out all of the setup information
    sensor.printSetup(Serial);

    // Print out the device status
    uint16_t status;
    status = sensor.getDeviceStatus();
    Serial.print("Current device status is: ");
    Serial.println(status, BIN);
    sensor.printDeviceStatus(status, Serial);
    Serial.println("=======================");
    Serial.println("=======================");

    // Set up and turn on logging
    sensor.setCleaningMode(automatic);
    sensor.setCleaningInterval(5);
    sensor.setCleaningDuration(5);
    sensor.setCleaningWait(20);
    sensor.setLoggingInterval(150);
    sensor.setLoggingMode(0);
    Serial.println("=======================");
    Serial.println("=======================");

}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    // Print out the device status
    uint16_t status;
    status = sensor.getDeviceStatus();
    Serial.print("Current device status is: ");
    Serial.println(status, BIN);
    sensor.printDeviceStatus(status, Serial);

    Serial.print("Last sample was taken at ");
    Serial.print((unsigned long)(sensor.getSampleTime()));
    Serial.println(" seconds past Jan 1, 1970");

    // set up the values
    float value1, value2, value3, value4, value5, value6, value7, value8;

    // Get values one at a time
    for (int i = 1; i < 9; i++)
    {
        Serial.println("----");
        status = sensor.getParameterValue(i, value1);
        Serial.print("Value of parameter Number ");
        Serial.print(i);
        Serial.print(" is: ");
        Serial.print(value1);
        Serial.print(" ");
        Serial.print(sensor.getUnits(i));
        Serial.print(" with status code: ");
        Serial.println(status, BIN);
        sensor.printParameterStatus(status, Serial);
    }

    // Get all the values together
    sensor.getAllParameterValues(value1, value2, value3, value4, value5, value6, value7, value8);
    Serial.println("Value1, value2, value3, value4, value5, value6, value7, value8");
    Serial.print(value1);
    Serial.print(", ");
    Serial.print(value2);
    Serial.print(", ");
    Serial.print(value3);
    Serial.print(", ");
    Serial.print(value4);
    Serial.print(", ");
    Serial.print(value5);
    Serial.print(", ");
    Serial.print(value6);
    Serial.print(", ");
    Serial.print(value7);
    Serial.print(", ");
    Serial.println(value8);

    // Wait 5 minutes
    delay(300000L);
}
