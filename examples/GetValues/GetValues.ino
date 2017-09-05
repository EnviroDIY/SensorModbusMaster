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
    Serial1.begin(38400, SERIAL_8N2);
    // modbusSerial.begin(38400);  // The modbus serial stream
    // The default baud rate for the spectro::lyzer is 38400

    // Start up the sensor
    // sensor.begin(modbusAddress, &modbusSerial, DEREPin);
    sensor.begin(modbusAddress, Serial1, DEREPin);

    // Turn on debugging
    // sensor.setDebugStream(&Serial);

    // Start up note
    Serial.println("S::CAN Spect::lyzer Test");

    // Allow the sensor and converter to warm up
    Serial.println("Waiting for sensor and adapter to be ready.");
    delay(500);

    // Print out all of the setup information
    sensor.setLoggingMode(1);
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
    // sensor.setDebugStream(&Serial);

    Serial.println("Turn on Logging");
    sensor.setLoggingMode(0);
    Serial.print("Current logging mode is: ");
    Serial.println(sensor.getLoggingMode());

    Serial.println("Set cleaning to automatic (2)");
    sensor.setCleaningMode(automatic);
    Serial.print("Current cleaning mode is: ");
    Serial.println(sensor.getCleaningMode());

    Serial.println("Set the cleaning interval to every 11 readings");
    sensor.setCleaningInterval(11);
    Serial.print("Current cleaning interval is: ");
    Serial.println(sensor.getCleaningInterval());

    Serial.println("Set the cleaning duration to 4 seconds");
    sensor.setCleaningDuration(4);
    Serial.print("Current cleaning duration is: ");
    Serial.println(sensor.getCleaningDuration());

    Serial.println("Set the wait period after cleaning to 10 seconds");
    sensor.setCleaningWait(10);
    Serial.print("Current wait period after cleaning is: ");
    Serial.println(sensor.getCleaningWait());

    Serial.println("Set the measurement interval to 5*60 seconds");
    sensor.setMeasInterval(5*60);
    Serial.print("Current measurement interval is: ");
    Serial.println(sensor.getMeasInterval());

    Serial.println("Set the logging interval to 5 minutes");
    sensor.setLoggingInterval(5);
    Serial.print("Current logging interval is: ");
    Serial.println(sensor.getLoggingInterval());

    // sensor.stopDebugging();

    // Print out all of the setup information again
    // sensor.printSetup(Serial);

    // sensor.setDebugStream(&Serial);
    sensor.printParameterHeader(Serial);
    // sensor.stopDebugging();

    // Wait to allow spec to take data and put it into registers
    delay(35000);
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
    float value;

    // Get values one at a time
    for (int i = 1; i < sensor.getParameterCount()+1; i++)
    {
        Serial.println("----");
        status = sensor.getParameterValue(i, value);
        Serial.print("Value of parameter Number ");
        Serial.print(i);
        Serial.print(" is: ");
        Serial.print(value);
        Serial.print(" ");
        Serial.print(sensor.getUnits(i));
        Serial.print(" with status code: ");
        Serial.println(status, BIN);
        sensor.printParameterStatus(status, Serial);
    }

    // sensor.printParameterData(Serial);

    // Wait 2 minutes
    delay(120000);
}
