/*****************************************************************************
scanRegisters.ino

This was a testing program to scan through all of the spectro::lyzer's
holding registers.
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
byte modbusAddress = 0x01;  // The sensor's modbus address, or SlaveID

// Define pin number variables
const int DEREPin = -1;   // The pin controlling Recieve Enable and Driver Enable
                          // on the RS485 adapter, if applicable (else, -1)
                          // Setting HIGH enables the driver (arduino) to send text
                          // Setting LOW enables the receiver (sensor) to send text
const int SSRxPin = 10;  // Recieve pin for software serial (Rx on RS485 adapter)
const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)

// Construct software serial object for Modbus
#if defined(ARDUINO_AVR_UNO)
    #include <SoftwareSerial.h>
    SoftwareSerial modbusSerial(SSRxPin, SSTxPin);
#endif

// Construct the modbus instance
modbusMaster modbus;

void printPaddedHex(byte val)
{
    if (val < 16) {Serial.print("0"); Serial.print(val, HEX);}
    else Serial.print(val, HEX);
}

void printPaddedBin(byte val)
{
    if (val >= 0b10000000) {Serial.print(""); Serial.print(val, BIN);}
    else if (val >= 0b01000000) {Serial.print("0"); Serial.print(val, BIN);}
    else if (val >= 0b00100000) {Serial.print("00"); Serial.print(val, BIN);}
    else if (val >= 0b00010000) {Serial.print("000"); Serial.print(val, BIN);}
    else if (val >= 0b00001000) {Serial.print("0000"); Serial.print(val, BIN);}
    else if (val >= 0b00000100) {Serial.print("00000"); Serial.print(val, BIN);}
    else if (val >= 0b00000010) {Serial.print("000000"); Serial.print(val, BIN);}
    else /*if (val > 0b00000001)*/ {Serial.print("0000000"); Serial.print(val, BIN);}
    // if (val > 0b0000000010000000) {Serial.print("00000000"); Serial.print(val, BIN);}
    // if (val > 0b0000000001000000) {Serial.print("000000000"); Serial.print(val, BIN);}
    // if (val > 0b0000000000100000) {Serial.print("0000000000"); Serial.print(val, BIN);}
    // if (val > 0b0000000000010000) {Serial.print("000000000000"); Serial.print(val, BIN);}
    // if (val > 0b0000000000001000) {Serial.print("0000000000000"); Serial.print(val, BIN);}
    // if (val > 0b0000000000000100) {Serial.print("00000000000000"); Serial.print(val, BIN);}
    // if (val > 0b0000000000000010) {Serial.print("000000000000000"); Serial.print(val, BIN);}
    // if (val > 0b0000000000000001) {Serial.print("0000000000000000"); Serial.print(val, BIN);}
}

void printPaddedString(String val)
{
    int len = val.length();
    if (len == 4) {Serial.print(""); Serial.print(val);}
    if (len == 3) {Serial.print(" "); Serial.print(val);}
    if (len == 2) {Serial.print("  "); Serial.print(val);}
    if (len == 1) {Serial.print("   "); Serial.print(val);}
    if (len == 0) {Serial.print("    "); Serial.print(val);}
}

void printPaddedInt16(uint16_t val)
{
    if (val < 10) {Serial.print("    "); Serial.print(val);}
    else if (val < 100) {Serial.print("   "); Serial.print(val);}
    else if (val < 1000) {Serial.print("  "); Serial.print(val);}
    else if (val < 10000) {Serial.print(" "); Serial.print(val);}
    else Serial.print(val);
}

// ---------------------------------------------------------------------------
// Main setup function
// ---------------------------------------------------------------------------
void setup()
{
    if (DEREPin > 0) pinMode(DEREPin, OUTPUT);

    Serial.begin(57600);  // Main serial port for debugging via USB Serial Monitor

    #if defined(ARDUINO_AVR_UNO)
        modbusSerial.begin(38400);  // port for communicating with sensor
        modbus.begin(modbusAddress, modbusSerial, DEREPin);
    #else
        Serial1.begin(38400, SERIAL_8O1);  // port for communicating with sensor
        modbus.begin(modbusAddress, &Serial1, DEREPin);
    #endif

    // Turn on debugging
    // modbus.setDebugStream(&Serial);

    // Start up note
    Serial.println("Full scan of all input and holding registers");

    // Allow the sensor and converter to warm up
    Serial.println("Waiting for sensor and adapter to be ready.");
    delay(500);

    // Print a header
    Serial.println("Register Type,  Reg #,     Hex     ,                Binary               ,  Char ,     uInt16  ,  Float32");

    // Get values two at a time
    for (long i = 0; i < 65535; i++)
    {
        Serial.print("Input (0x04),    ");
        printPaddedInt16(i);
        Serial.print(", ");
        if (modbus.getRegisters(0x04, i, 2))
        {
            printPaddedHex(modbus.responseBuffer[3]);
            Serial.print(" ");
            printPaddedHex(modbus.responseBuffer[4]);
            Serial.print(" ");
            printPaddedHex(modbus.responseBuffer[5]);
            Serial.print(" ");
            printPaddedHex(modbus.responseBuffer[6]);
            Serial.print(",  ");
            printPaddedBin(modbus.responseBuffer[3]);
            Serial.print(" ");
            printPaddedBin(modbus.responseBuffer[4]);
            Serial.print(" ");
            printPaddedBin(modbus.responseBuffer[5]);
            Serial.print(" ");
            printPaddedBin(modbus.responseBuffer[6]);
            Serial.print(",  ");
            printPaddedString(modbus.StringFromFrame(4));
            Serial.print(",  ");
            printPaddedInt16(modbus.uint16FromFrame(bigEndian, 3));
            Serial.print(" ");
            printPaddedInt16(modbus.uint16FromFrame(bigEndian, 5));
            Serial.print(",  ");
            Serial.print(modbus.float32FromFrame(bigEndian, 3), 4);
            Serial.println();
        }
        else Serial.println("Read Register Failed!");
        i++;
    }
    Serial.println("=======================");
    Serial.println("\n\n\n\n\n\n\n\n");
    Serial.println("=======================");
    Serial.println("Register Type,  Reg #,     Hex     ,                Binary               ,  Char ,     uInt16  ,  Float32");

    // Get values two at a time
    for (long i = 0; i < 65535; i++)
    {
        Serial.print("Holding (0x03),  ");
        printPaddedInt16(i);
        Serial.print(", ");
        if (modbus.getRegisters(0x03, i, 2))
        {
            printPaddedHex(modbus.responseBuffer[3]);
            Serial.print(" ");
            printPaddedHex(modbus.responseBuffer[4]);
            Serial.print(" ");
            printPaddedHex(modbus.responseBuffer[5]);
            Serial.print(" ");
            printPaddedHex(modbus.responseBuffer[6]);
            Serial.print(",  ");
            printPaddedBin(modbus.responseBuffer[3]);
            Serial.print(" ");
            printPaddedBin(modbus.responseBuffer[4]);
            Serial.print(" ");
            printPaddedBin(modbus.responseBuffer[5]);
            Serial.print(" ");
            printPaddedBin(modbus.responseBuffer[6]);
            Serial.print(",  ");
            printPaddedString(modbus.StringFromFrame(4));
            Serial.print(",  ");
            printPaddedInt16(modbus.uint16FromFrame(bigEndian, 3));
            Serial.print(" ");
            printPaddedInt16(modbus.uint16FromFrame(bigEndian, 5));
            Serial.print(",  ");
            Serial.print(modbus.float32FromFrame(bigEndian, 3), 4);
            Serial.println();
        }
        else Serial.println("Read Register Failed!");
        i++;
    }
    Serial.println("=======================");
}

// ---------------------------------------------------------------------------
// Main loop function
// ---------------------------------------------------------------------------
void loop()
{}
