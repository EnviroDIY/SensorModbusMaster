/** =========================================================================
 * @example{lineno} scanRegisters.ino
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 * @copyright Stroud Water Research Center
 * @license This example is published under the BSD-3 license.
 *
 * @brief This is a testing program to scan through all possible holding registers.
 *
 * @m_examplenavigation{example_scan_registers,}
 * @m_footernavigation
 * ======================================================================= */

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
byte modbusAddress  = 0x01;   // The sensor's modbus address, or SlaveID
long modbusBaudRate = 38400;  // The baud rate the sensor uses

// Define pin number variables
const int DEREPin = -1;  // The pin controlling Receive Enable and Driver Enable
                         // on the RS485 adapter, if applicable (else, -1)
                         // Setting HIGH enables the driver (arduino) to send text
                         // Setting LOW enables the receiver (sensor) to send text
const int SSRxPin = 10;  // Receive pin for software serial (Rx on RS485 adapter)
const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)


// Construct a Serial object for Modbus
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_FEATHER328P)
// The Uno only has 1 hardware serial port, which is dedicated to comunication with the
// computer. If using an Uno, you will be restricted to using AltSofSerial or
// SoftwareSerial
#include <SoftwareSerial.h>
const int SSRxPin = 10;  // Receive pin for software serial (Rx on RS485 adapter)
const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)
#pragma message("Using Software Serial for the Uno on pins 10 and 11")
SoftwareSerial modbusSerial(SSRxPin, SSTxPin);
// AltSoftSerial modbusSerial;
#elif defined ESP8266
#pragma message("Using Software Serial for the ESP8266")
#include <SoftwareSerial.h>
SoftwareSerial modbusSerial;
#elif defined(NRF52832_FEATHER) || defined(ARDUINO_NRF52840_FEATHER)
#pragma message("Using TinyUSB for the NRF52")
#include <Adafruit_TinyUSB.h>
HardwareSerial& modbusSerial = Serial1;
#elif !defined(NO_GLOBAL_SERIAL1) && !defined(STM32_CORE_VERSION)
// This is just a assigning another name to the same port, for convienence
// Unless it is unavailable, always prefer hardware serial.
#pragma message("Using HarwareSerial / Serial1")
HardwareSerial& modbusSerial = Serial1;
#else
// This is just a assigning another name to the same port, for convienence
// Unless it is unavailable, always prefer hardware serial.
#pragma message("Using HarwareSerial / Serial")
HardwareSerial& modbusSerial = Serial;
#endif

// Construct the modbus instance
modbusMaster modbus;

void printPaddedHex(byte val) {
    if (val < 16) {
        Serial.print("0");
        Serial.print(val, HEX);
    } else
        Serial.print(val, HEX);
}

void printPaddedBin(byte val) {
    if (val >= 0b10000000) {
        Serial.print("");
        Serial.print(val, BIN);
    } else if (val >= 0b01000000) {
        Serial.print("0");
        Serial.print(val, BIN);
    } else if (val >= 0b00100000) {
        Serial.print("00");
        Serial.print(val, BIN);
    } else if (val >= 0b00010000) {
        Serial.print("000");
        Serial.print(val, BIN);
    } else if (val >= 0b00001000) {
        Serial.print("0000");
        Serial.print(val, BIN);
    } else if (val >= 0b00000100) {
        Serial.print("00000");
        Serial.print(val, BIN);
    } else if (val >= 0b00000010) {
        Serial.print("000000");
        Serial.print(val, BIN);
    } else /*if (val > 0b00000001)*/ {
        Serial.print("0000000");
        Serial.print(val, BIN);
    }
    // if (val > 0b0000000010000000) {
    //     Serial.print("00000000");
    //     Serial.print(val, BIN);
    // }
    // if (val > 0b0000000001000000) {
    //     Serial.print("000000000");
    //     Serial.print(val, BIN);
    // }
    // if (val > 0b0000000000100000) {
    //     Serial.print("0000000000");
    //     Serial.print(val, BIN);
    // }
    // if (val > 0b0000000000010000) {
    //     Serial.print("000000000000");
    //     Serial.print(val, BIN);
    // }
    // if (val > 0b0000000000001000) {
    //     Serial.print("0000000000000");
    //     Serial.print(val, BIN);
    // }
    // if (val > 0b0000000000000100) {
    //     Serial.print("00000000000000");
    //     Serial.print(val, BIN);
    // }
    // if (val > 0b0000000000000010) {
    //     Serial.print("000000000000000");
    //     Serial.print(val, BIN);
    // }
    // if (val > 0b0000000000000001) {
    //     Serial.print("0000000000000000");
    //     Serial.print(val, BIN);
    // }
}

void printPaddedString(String val) {
    int len = val.length();
    if (len == 4) {
        Serial.print("");
        Serial.print(val);
    }
    if (len == 3) {
        Serial.print(" ");
        Serial.print(val);
    }
    if (len == 2) {
        Serial.print("  ");
        Serial.print(val);
    }
    if (len == 1) {
        Serial.print("   ");
        Serial.print(val);
    }
    if (len == 0) {
        Serial.print("    ");
        Serial.print(val);
    }
}

void printPaddedInt16(uint16_t val) {
    if (val < 10) {
        Serial.print("    ");
        Serial.print(val);
    } else if (val < 100) {
        Serial.print("   ");
        Serial.print(val);
    } else if (val < 1000) {
        Serial.print("  ");
        Serial.print(val);
    } else if (val < 10000) {
        Serial.print(" ");
        Serial.print(val);
    } else
        Serial.print(val);
}

// ==========================================================================
// Main setup function
// ==========================================================================
void setup() {
    if (DEREPin >= 0) { pinMode(DEREPin, OUTPUT); }

    // Turn on the "main" serial port for debugging via USB Serial Monitor
    Serial.begin(57600);

    // Turn on your modbus serial port
#if defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_FEATHER328P) || \
    defined(ARDUINO_SAM_DUE) || not defined(SERIAL_8O1)
    modbusSerial.begin(modbusBaudRate);
    // NOTE:  The AVR implementation of SoftwareSerial only supports 8N1
    // The hardware serial implementation of the Due also only supports 8N1
#elif defined(ESP8266)
    const int SSRxPin = 13;  // Receive pin for software serial (Rx on RS485 adapter)
    const int SSTxPin = 14;  // Send pin for software serial (Tx on RS485 adapter)
    modbusSerial.begin(modbusBaudRate, SWSERIAL_8O1, SSRxPin, SSTxPin, false);
    // NOTE:  See
    // https://github.com/plerup/espsoftwareserial/blob/40038df/src/SoftwareSerial.h#L120-L160
    // for a list of data/parity/stop bit configurations that apply to the ESP8266's
    // implementation of SoftwareSerial
#else
    modbusSerial.begin(modbusBaudRate, SERIAL_8O1);
    // ^^ use this for 8 data bits - odd parity - 1 stop bit
    // Serial1.begin(modbusBaudRate, SERIAL_8E1);
    // ^^ use this for 8 data bits - even parity - 1 stop bit
    // Serial1.begin(modbusBaudRate, SERIAL_8N2);
    // ^^ use this for 8 data bits - no parity - 2 stop bits
    // Serial1.begin(modbusBaudRate);
    // ^^ use this for 8 data bits - no parity - 1 stop bits
    // Despite being technically "non-compliant" with the modbus specifications
    // 8N1 parity is very common.
#endif

    // Turn on debugging, if desired
    // modbus.setDebugStream(&Serial);

    // Start the modbus instance
    modbus.begin(modbusAddress, modbusSerial, DEREPin);

    // Start up note
    Serial.println("Full scan of all input and holding registers");

    // Allow the sensor and converter to warm up
    Serial.println("Waiting for sensor and adapter to be ready.");
    delay(500);

    // Print a header
    Serial.println("Register Type,  Reg #,     Hex     ,                Binary         "
                   "      ,  Char ,     uInt16  ,  Float32");

    // Get values two at a time
    for (long i = 0; i < 65535; i++) {
        Serial.print("Input (0x04),    ");
        printPaddedInt16(i);
        Serial.print(", ");
        if (modbus.getRegisters(0x04, i, 2)) {
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
        } else
            Serial.println("Read Register Failed!");
        i++;
    }
    Serial.println("=======================");
    Serial.println("\n\n\n\n\n\n\n\n");
    Serial.println("=======================");
    Serial.println("Register Type,  Reg #,     Hex     ,                Binary         "
                   "      ,  Char ,     uInt16  ,  Float32");

    // Get values two at a time
    for (long i = 0; i < 65535; i++) {
        Serial.print("Holding (0x03),  ");
        printPaddedInt16(i);
        Serial.print(", ");
        if (modbus.getRegisters(0x03, i, 2)) {
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
        } else
            Serial.println("Read Register Failed!");
        i++;
    }
    Serial.println("=======================");
}

// ==========================================================================
// Main loop function
// ==========================================================================
void loop() {}
