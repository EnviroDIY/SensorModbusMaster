/** =========================================================================
 * @example{lineno} getSetAddress.ino
 * @author Anthony Aufdenkampe <aaufdenkampe@limno.com>
 * @copyright Stroud Water Research Center
 * @license This example is published under the BSD-3 license.
 *
 * @brief This example gets the Modbus address for a sensor and optionally
 * sets it to a new address. This example can be used on sensors from 
 * any manufacturer, as long as you know the register for the device address.
 *
 * @m_examplenavigation{example_read_write_register,}
 * @m_footernavigation
 * ======================================================================= */

// ---------------------------------------------------------------------------
// Include the base required libraries
// ---------------------------------------------------------------------------
#include <Arduino.h>
#include <SensorModbusMaster.h>

// ==========================================================================
//  Sensor/Slave Settings
// ==========================================================================

// The sensor slave modbus address used by default to broadcast 
// commands to any or all slave devices.  
//   0x00 (0)   used by many manufactuers
//   0xFF (255) used by some manufactuers, such as YosemiTech
byte modbusBroadcastAddress = 0xFF;   

// Your desired modbus address for your slave device.  
byte newModbusAddress = 0x05; 


// Set the registers for your device
// NOTE: Registers are always two byte integers (int16_t) but are often 
// provided in Hexadecmial form in modbus register maps. C++ can auto-convert 
// from HEC to DEC, and an online "HEX to DEC Converter" can confirm values.

int16_t addressRegister = 0x3000;
  // Common registers for storing device address, by manufacturer:
    // 0x3000 = 12288 for YosemiTech

int16_t serialNumberRegister = 0x0900;
  // Common registers for storing device serial number, by manufacturer:
    // 0x0900 = 2304 for most YosemiTech sensors
    // 0x1400 = 5120 for the YosemiTech Y4000 Multi-parameter sonde

// The Modbus baud rate the sensor uses
int32_t modbusBaudRate = 9600;  // The baud rate the sensor uses

// Time in milliseconds after powering up for the slave device to respond
#define WARM_UP_TIME 1500  


// ==========================================================================
//  Data Logger Options
// ==========================================================================
const int32_t serialBaud = 115200;  // Baud rate for serial monitor

// Define pin number variables
const int sensorPwrPin  = 10;  // The pin sending power to the sensor
const int adapterPwrPin = 22;  // The pin sending power to the RS485 adapter
const int DEREPin       = -1;  // The pin controlling Receive Enable and Driver Enable
                               // on the RS485 adapter, if applicable (else, -1)
                               // Setting HIGH enables the driver (arduino) to send text
                               // Setting LOW enables the receiver (sensor) to send text

// Turn on debugging outputs (i.e. raw Modbus requests & responses)
// by uncommenting next line (i.e. `#define DEBUG`)
#define DEBUG

// ==========================================================================
// Create and Assign a Serial Port for Modbus
// ==========================================================================
// Harware serial ports are prefered when available.
// AltSoftSerial is the most stable alternative for modbus.
//   Select over alternatives with the define below.
#define BUILD_ALTSOFTSERIAL // Comment-out if you prefer alternatives

#if defined(BUILD_ALTSOFTSERIAL)
#include <AltSoftSerial.h>
AltSoftSerial modbusSerial;

#elif defined(ARDUINO_AVR_UNO) || defined(ARDUINO_AVR_FEATHER328P)
// The Uno only has 1 hardware serial port, which is dedicated to comunication with the
// computer. If using an Uno, you will be restricted to using AltSofSerial or
// SoftwareSerial
#include <SoftwareSerial.h>
const int SSRxPin = 10;  // Receive pin for software serial (Rx on RS485 adapter)
const int SSTxPin = 11;  // Send pin for software serial (Tx on RS485 adapter)
#pragma message("Using Software Serial for the Uno on pins 10 and 11")
SoftwareSerial modbusSerial(SSRxPin, SSTxPin);

#elif defined(ESP8266)
#include <SoftwareSerial.h>
#pragma message("Using Software Serial for the ESP8266")
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

// Initialize success flag for set commands
bool       success;


// ==========================================================================
// Working Functions
// ==========================================================================
// A function for pretty-printing the Modbuss Address in Hexadecimal notation,
// from ModularSensors `sensorLocation()`
String prettyprintAddressHex(byte _modbusAddress) {
    String addressHex = F("0x");
    if (_modbusAddress < 0x10) { addressHex += "0"; }
    addressHex += String(_modbusAddress, HEX);
    return addressHex;
}


// ==========================================================================
//  Arduino Setup Function
// ==========================================================================
void setup() {
    // Set various pins as needed
    if (DEREPin >= 0) { pinMode(DEREPin, OUTPUT); }
    if (sensorPwrPin >= 0) {
        pinMode(sensorPwrPin, OUTPUT);
        digitalWrite(sensorPwrPin, HIGH);
    }
    if (adapterPwrPin >= 0) {
        pinMode(adapterPwrPin, OUTPUT);
        digitalWrite(adapterPwrPin, HIGH);
    }

    // Turn on the "main" serial port for debugging via USB Serial Monitor
    Serial.begin(serialBaud);

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
    modbusSerial.begin(modbusBaudRate);
    // Hardware Serial Options:
    // Serial1.begin(modbusBaudRate, SERIAL_8O1);
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

    // Start the modbusMaster instance with the broadcast address
    modbus.begin(modbusBroadcastAddress, modbusSerial, DEREPin);

    // Turn on debugging
#ifdef DEBUG
    modbus.setDebugStream(&Serial);
#endif

    // Start up note
    Serial.println(F("\nRunning the 'getSetAddress()' example sketch."));

    // Allow the sensor and converter to warm up
    Serial.println(F("\nWaiting for sensor and adapter to be ready."));
    Serial.print(F("    Warm up time (ms): "));
    Serial.println(WARM_UP_TIME);
    delay(WARM_UP_TIME);

    // Confirm Modbus Address
    Serial.println(F("\nBroadcast modbus address:"));
    Serial.print(F("    Decimal: "));
    Serial.print(modbusBroadcastAddress, DEC);
    Serial.print(F(", Hexidecimal: "));
    Serial.println(prettyprintAddressHex(modbusBroadcastAddress));

    // Get the current Modbus Address
    Serial.println(F("Getting current modbus address..."));
    // Read a byte from a single holding register
    byte oldModbusAddress = modbus.byteFromRegister(0x03, addressRegister, 1);
    Serial.print(F("    Decimal: "));
    Serial.print(oldModbusAddress, DEC);
    Serial.print(F(", Hexidecimal: "));
    Serial.println(prettyprintAddressHex(oldModbusAddress));

    // Scan method if broadcast address does not work
    if (oldModbusAddress == 0) {
        Serial.println(F("Modbus Address not found!"));
        Serial.println(F("Will scan possible addresses (in future)..."));
        // Add future scan loop function here
    };

    // Reset to current address
    Serial.print(F("\nRestart the modbusMaster instance with the "));
    Serial.println(F("current device address."));
    modbus.begin(oldModbusAddress, modbusSerial, DEREPin);
    delay(WARM_UP_TIME);

    // Get the sensor serial number
    Serial.println(F("\nGetting sensor serial number..."));
    // Read a string from several holding registers
    String SN = modbus.StringFromRegister(0x03, serialNumberRegister, 14);
    Serial.print(F("    Serial Number: "));
    Serial.println(SN);

    // Set modbus address
    Serial.print(F("\nSetting sensor modbus address to: "));
    Serial.println(prettyprintAddressHex(newModbusAddress));
    // Write to a holding register
    success = modbus.byteToRegister(addressRegister, 1, newModbusAddress, true);
    if (success)
        Serial.println(F("    Success!"));
    else
        Serial.println(F("    Failed!"));
    // Restart the modbusMaster instance to use new address
    modbus.begin(newModbusAddress, modbusSerial, DEREPin);
    delay(WARM_UP_TIME);

    Serial.println(F("\nGetting sensor serial number using the new address."));
    SN = modbus.StringFromRegister(0x03, serialNumberRegister, 14);
    Serial.print(F("    Serial Number: "));
    Serial.println(SN);
    
}

// ==========================================================================
// Main loop function
// ==========================================================================
void loop() {
    // // Get data values from read-only input registers (0x04)
    // // Just for show, we will do the exact same thing 2 ways
    // // All values will be read as bigEndian

    // // Some variables to hold results
    // uint16_t deviceStatus = 0;
    // int16_t  doPPM        = 0;
    // uint16_t temperature  = 0;

    // // Method 1:
    // // Get three values one at a time from 3 different registers.
    // // This code is easier to follow, but it requires more back-and-forth between
    // // the Arduino and the sensor so it is a little "slower".
    // deviceStatus = modbus.uint16FromRegister(0x04, 0x00, bigEndian);
    // doPPM        = modbus.int16FromRegister(0x04, 0x01, bigEndian);
    // temperature  = modbus.uint16FromRegister(0x04, 0x02, bigEndian);

    // // Print results
    // Serial.print("Device Status:");
    // Serial.println(deviceStatus);
    // Serial.print("Dissolved Oxygen in ppm:");
    // Serial.println(doPPM);
    // Serial.print("Temperature in °C:");
    // Serial.println(temperature);
    // Serial.println();

    // // Method 2:
    // // Read all three registers at once and parse the values from the response.
    // // This is faster, especially when getting many readings, but it's trickier to
    // // write and understand the code.
    // bool success = modbus.getRegisters(0x04, 0x00, 3);
    // // ^ This gets the values and stores them in an internal "frame" with the hex values
    // // of the response
    // if (success) {
    //     deviceStatus = modbus.uint16FromFrame(bigEndian, 3);
    //     // ^ The first data value is at position 3 in the modbus response frame
    //     // 0 = modbus address, 1 = modbus method, 2 = # registers returned, 3 = 1st
    //     // value returned
    //     doPPM = modbus.int16FromFrame(bigEndian, 5);
    //     // ^ The next data value is at position 5 since each register occupies 2 places
    //     temperature = modbus.uint16FromFrame(bigEndian, 7);
    // }

    // // Print results
    // Serial.print("Device Status:");
    // Serial.println(deviceStatus);
    // Serial.print("Dissolved Oxygen in ppm:");
    // Serial.println(doPPM);
    // Serial.print("Temperature in °C:");
    // Serial.println(temperature);
    // Serial.println();
}
