/*****************************************************************************
GetValues.ino

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

    sensor.getSetup();

    // Get the sensor's hardware and software version
    // Serial.println("Getting sensor version.");
    // float hardwareV, softwareV;
    // sensor.getVersion(hardwareV, softwareV);
    // Serial.print("    Current Hardware Version: ");
    // Serial.println(hardwareV);
    // Serial.print("    Current Software Version: ");
    // Serial.println(softwareV);

    // Get the sensor serial number
    // Serial.println("Getting sensor serial number.");
    // String SN = sensor.getSerialNumber();
    // Serial.print("    Serial Number: ");
    // Serial.println(SN);
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{
    // send the command to get the values
    // float temp = 0;
    // float val = 0;
    // if (model == Y532)
    // {
    //     sensor.getValues(val);
    //     sensor.getTemperatureValue(temp);
    // }
    // else sensor.getValues(temp, val);
    // Serial.print(temp);
    // Serial.print("      ");
    // Serial.print(val);
    // Serial.print("      ");
    // Serial.print(millis());
    // Serial.println();


    // Delay between readings
    // Modbus manuals recommend the following re-measure times:
    //     2 s for chlorophyll
    //     2 s for turbidity
    //     3 s for conductivity

    // The turbidity and DO sensors appear return new readings about every 1.6 seconds.
    // The pH sensor returns new readings about every 1.8 seconds.
    // The conductivity sensor only returns new readings about every 2.7 seconds.

    // The teperature sensors can take readings much more quickly.  The same results
    // can be read many times from the registers between the new sensor readings.
    // delay(3000);
}
