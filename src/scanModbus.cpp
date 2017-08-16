/*
 *scanModbus.cpp
*/

#include "scanModbus.h"


//----------------------------------------------------------------------------
//                          PUBLIC SENSOR FUNCTIONS
//----------------------------------------------------------------------------


// This function sets up the communication
// It should be run during the arduino "setup" function.
// The "stream" device must be initialized and begun prior to running this.
bool scan::begin(byte modbusSlaveID, Stream *stream, int enablePin)
{
    // Give values to variables;
    _slaveID = modbusSlaveID;
    _stream = stream;
    _enablePin = enablePin;

    // Set pin mode for the enable pin
    if (_enablePin > 0) pinMode(_enablePin, OUTPUT);

    _stream->setTimeout(modbusFrameTimeout);

    return true;
}
bool scan::begin(byte modbusSlaveID, Stream &stream, int enablePin)
{
    // Give values to variables;
    _slaveID = modbusSlaveID;
    _stream = &stream;
    _enablePin = enablePin;

    // Set pin mode for the enable pin
    if (_enablePin > 0) pinMode(_enablePin, OUTPUT);

    _stream->setTimeout(modbusFrameTimeout);

    return true;
}


// This gets all of the setup information at once
bool scan::getSetup(void)
{
    wakeSpec();
    byte getHoldingRegisters[8] = {_slaveID, 0x03, 0x00, 0x00, 0x00, 0x1b, 0x00, 0x00};
                                 // Address, Read,Start@Reg 0,27 Registers,   CRC
    respSize = sendCommand(getHoldingRegisters, 8);

    // When returning a bunch of registers, as here, to get
    // the byte location in the frame of the desired register use:
    // (3 bytes of Modbus frame + (2 bytes/register x (desired register - start register))

    if (respSize == (27*2 + 5) && responseBuffer[0] == _slaveID)
    {
        // Communication mode is in register 1 (1 uint16 register = 2 bytes)
        _commMode = uint16FromBEFrame(responseBuffer, 5);
        _debugStream->print("Communication mode setting is: ");
        _debugStream->print(_commMode);
        _debugStream->print(" (");
        _debugStream->print(printCommMode(_commMode));
        _debugStream->println(")");

        // Baud rate is in register 2 (1 uint16 register = 2 bytes)
        _baudRate = uint16FromBEFrame(responseBuffer, 7);
        _debugStream->print("Baud Rate setting is: ");
        _debugStream->print(_baudRate);
        _debugStream->print(" (");
        _debugStream->print(printBaudRate(_baudRate));
        _debugStream->println(")");

        // Parity is in register 3 (1 uint16 register = 2 bytes)
        _parity = uint16FromBEFrame(responseBuffer, 9);
        _debugStream->print("Parity setting is: ");
        _debugStream->print(_parity);
        _debugStream->print(" (");
        _debugStream->print(printParity(_parity));
        _debugStream->println(")");

        // Skipping register 4, which is just to be written to reset all
        // settings back to default.

        // Pointer to the private configuration is in register 5
        _configRegNumber = pointerFromBEFrame(responseBuffer, 13);
        _configRegType = pointerTypeFromBEFrame(responseBuffer, 13);
        _debugStream->print("Private configuration begins in register ");
        _debugStream->print(_configRegNumber);
        _debugStream->print(", which is type ");
        _debugStream->print(_configRegType);
        _debugStream->print(" (");
        _debugStream->print(printRegisterType(_configRegType));
        _debugStream->println(")");

        // Device Location (s::canpoint) is registers 6-11 (char[12])
        _scanPoint = StringFromFrame(responseBuffer, 15, 12);
        _debugStream->print("Current s::canpoint is: ");
        _debugStream->println(_scanPoint);

        // Cleaning mode is in register 12 (1 uint16 register = 2 bytes)
        _cleaningMode = uint16FromBEFrame(responseBuffer, 27);
        _debugStream->print("Cleaning mode setting is: ");
        _debugStream->print(_cleaningMode);
        _debugStream->print(" (");
        _debugStream->print(printCleaningMode(_cleaningMode));
        _debugStream->println(")");

        // Cleaning interval is in register 13 (1 uint16 register = 2 bytes)
        _cleaningInterval = uint16FromBEFrame(responseBuffer, 29);
        _debugStream->print("Cleaning interval is: ");
        _debugStream->print(_cleaningInterval);
        _debugStream->println(" measurements between cleanings");

        // Cleaning duration is in register 14 (1 uint16 register = 2 bytes)
        _cleaningDuration = uint16FromBEFrame(responseBuffer, 31);
        _debugStream->print("Cleaning time is: ");
        _debugStream->print(_cleaningDuration);
        _debugStream->println(" seconds");

        // Cleaning duration is in register 15 (1 uint16 register = 2 bytes)
        _cleaningWait = uint16FromBEFrame(responseBuffer, 33);
        _debugStream->print("Wait time between cleaning and sampling is: ");
        _debugStream->print(_cleaningWait);
        _debugStream->println(" seconds");

        // System time is in registers 16-21 (64-bit timestamp + padding)
        // Registers 16 and 17 will be 0x4000 0000 until the year 2106;
        // I'm ignoring it for the next 90 years to avoid using 64bit math
        // Registers 18 and 19 will have the seconds past Jan 1, 1970
        // Registers 20 and 21 are just 0's.
        uint32_t secsPast1970 = uint32FromBEFrame(responseBuffer, 39);
        _debugStream->print("Current System Time is: ");
        _debugStream->print((unsigned long)(secsPast1970));
        _debugStream->println(" seconds past Jan 1, 1970");

        // Measurement interval is in register 22 (1 uint16 register = 2 bytes)
        _measInterval = uint16FromBEFrame(responseBuffer, 47);
        _debugStream->print("Measurement interval is: ");
        _debugStream->print(_measInterval);
        _debugStream->println(" seconds");

        // Logging Mode (0 = on; 1 = off) is in register 23 (1 uint16 register = 2 bytes)
        _loggingMode = uint16FromBEFrame(responseBuffer, 49);
        _debugStream->print("Logging mode setting is: ");
        _debugStream->print(_loggingMode);
        _debugStream->print(" (");
        _debugStream->print(printLoggingMode(_loggingMode));
        _debugStream->println(")");

        // Logging interval is in register 24 (1 uint16 register = 2 bytes)
        _loggingInterval = uint16FromBEFrame(responseBuffer, 51);
        _debugStream->print("Logging interval is: ");
        _debugStream->print(_loggingInterval);
        _debugStream->println(" seconds");

        // Available number of logged results is in register 25 (1 uint16 register = 2 bytes)
        _numLoggedResults = uint16FromBEFrame(responseBuffer, 53);
        _debugStream->print(_numLoggedResults);
        _debugStream->println(" results have been logged so far");

        // "Index device status" is in register 26 (1 uint16 register = 2 bytes)
        _indexLogResult = uint16FromBEFrame(responseBuffer, 55);
        _debugStream->print("Index device status is: ");
        _debugStream->println(_indexLogResult);

        // Set the holding register flag to true
        _gotHoldingRegSpecSetup = true;

        // Return true after parsing everything
        return true;
    }
    else return false;
}




// This just returns the given slave ID.  If you don't know your slave ID, you
// must find it some other way
byte scan::getSlaveID(void)
{return _slaveID;}

// This sets a new modbus slave ID
bool scan::setSlaveID(byte newSlaveID)
{
    byte setSlaveID[11] = {_slaveID, 0x06, 0x00, 0x00, 0x00, 0x01, 0x02, 0x00, newSlaveID, 0x00, 0x00};
                         // Address, Write,   Reg 0,   1 Register, 2byte,   newAddress,       CRC
    respSize = sendCommand(setSlaveID, 11);

    if (respSize == 8 && responseBuffer[0] == _slaveID)
    {
        _slaveID = newSlaveID;
        return true;
    }
    else return false;
}

// The communication mode
int scan::getCommunicationMode(void)
{return false;}
bool scan::setCommunicationMode(specCommMode mode)
{return false;}

// The serial baud rate (iff communication mode = modbus RTU or modbus ASCII)
int scan::getBaudRate(void)
{return false;}
bool scan::setBaudRate(specBaudRate baud)
{return false;}

// The serial parity (iff communication mode = modbus RTU or modbus ASCII)
int scan::getParity(void)
{return false;}
bool scan::setParity(specParity parity)
{return false;}

// Reset all settings to default
bool scan::resetSettings(void)
{return false;}

// Get the "s::canpoint" of the device
String scan::getScanPoint(void)
{return "UNKNOWN";}

// Cleaning mode configuration
int scan::getCleaningMode(void)
{return false;}
bool scan::setCleaningMode(cleaningMode mode)
{return false;}

// Cleaning interval (ie, number of samples between cleanings)
int scan::getCleaningInterval(void)
{return false;}
bool scan::setCleaningInterval(uint16_t intervalSamples)
{return false;}

// Cleaning duration in seconds
int scan::getCleaningDuration(void)
{return false;}
bool scan::setCleaningDuration(uint16_t secDuration)
{return false;}

// Waiting time between end of cleaning and start of measurement
int scan::getCleaningWait(void)
{return false;}
bool scan::setCleaningWait(uint16_t secDuration)
{return false;}

// Current system time as a 64-bit count of seconds from Jan 1, 1970
long scan::getSystemTime(void)
{return false;}

// Measurement interval in seconds (0 - as fast as possible)
int scan::getMeasInterval(void)
{return false;}
bool scan::setMeasInterval(uint16_t secBetween)
{return false;}

// Logging Mode (0 = on; 1 = off)
int scan::getLoggingMode(void)
{return false;}
bool scan::setLoggingMode(uint8_t mode)
{return false;}

// Logging interval for data logger in minutes (0 = no logging active)
int scan::getLoggingInterval(void)
{return false;}
bool scan::setLoggingInterval(uint16_t interval)
{return false;}

// Available number of logged results in datalogger since last clearing
int scan::getNumLoggedResults(void)
{return false;}

// "Index device status public + private & parameter results from logger
// storage to Modbus registers.  If no stored results are available,
// results are NaN, Device status bit3 is set."
// I'm really not sure what this means...
int scan::getIndexLogResult(void)
{return false;}


// Get the version of the modbus mapping protocol
// The float variables for the version must be
// initialized prior to calling this function.
bool scan::getModbusVersion(float &modbusVersion)
{return false;}

// This returns a pretty string with the model information
String scan::getModel(void)
{return "UNKNOWN";}

// This gets the instrument serial number as a String
String scan::getSerialNumber(void)
{return "UNKNOWN";}

// This gets the hardware and software version of the sensor
// The float variables for the hardware and software versions must be
// initialized prior to calling this function.
// The reference (&) is needed when declaring this function so that
// the function is able to modify the actual input floats rather than
// create and destroy copies of them.
// There is no need to add the & when actually usig the function.
bool scan::getVersion(float &hardwareVersion, float &softwareVersion)
{return false;}

// Device rebooter counter
int scan::getHWStarts(void)
{return false;}

// This gets the number of parameters the spectro::lyzer is set to measure
int scan::getParameterCount(void)
{return false;}

// This returns a pretty string with the parameter measured.
String scan::getParameter(int parmNumber)
{return "UNKNOWN";}

// This returns a pretty string with the measurement units.
String scan::getUnits(int parmNumber)
{return "UNKNOWN";}

// This gets the upper limit of the parameter
// The float variable must be initialized prior to calling this function.
bool scan::getUpperLimit(int parmNumber, float &upperLimit)
{return false;}

// This gets the lower limit of the parameter
// The float variable must be initialized prior to calling this function.
bool scan::getLowerLimit(int parmNumber, float &lowerLimit)
{return false;}

// Last measurement time as a 64-bit count of seconds from Jan 1, 1970
long scan::getSampleTime(void)
{return false;}

// This gets values back from the sensor and puts them into a previously
// initialized float variable.  The actual return from the function is the
// int which is a bit-mask describing the parameter status.
int scan::getValue(int parmNumber, float &value1)
{return false;}

// This get up to 8 values back from the spectro::lyzer
bool scan::getAllValues(float &value1, float &value2, float &value3, float &value4,
                  float &value5, float &value6, float &value7, float &value8)
{return false;}

// These functions are to convert various s::can register code to strings
String scan::printCommMode(uint16_t code)
{
    switch (code)
    {
        case 0: return "Modbus RTU";
        case 1: return "Modbus ASCII";
        case 2: return "Modbus TCP";
        default: return "Unknown";
    }
}
uint16_t scan::printBaudRate(uint16_t code)
{
    String baud;
    switch (code)
    {
        case 0: baud = "9600"; break;
        case 1: baud = "19200"; break;
        case 2: baud = "38400"; break;
        default: baud = "0"; break;
    }
    uint16_t baudr = baud.toInt();
    return baudr;
}
String scan::printParity(uint16_t code)
{
    switch (code)
    {
        case 0: return "no parity";
        case 1: return "even parity";
        case 2: return "odd parity";
        default: return "Unknown";
    }
}
String scan::printCleaningMode(uint16_t code)
{
    switch (code)
    {
        case 0: return "no cleaning supported";
        case 1: return "manual";
        case 2: return "automatic";
        default: return "Unknown";
    }
}
String scan::printRegisterType(uint16_t code)
{
    switch (code)
    {
        case 0: return "Holding register";
        case 1: return "Input register";
        case 2: return "Discrete input register";
        case 3: return "Coil";
        default: return "Unknown";
    }
}
String scan::printLoggingMode(uint16_t code)
{
    switch (code)
    {
        case 0: return "Logging On";
        default: return "Logging Off";
    }
}

//----------------------------------------------------------------------------
//                           PRIVATE HELPER FUNCTIONS
//----------------------------------------------------------------------------

// This function returns the float from a 4-byte big-endian array beginning
// at a specific index of another array.
float scan::float32FromBEFrame(byte indata[], int start_index)
{
    SeFrame4 Sefram;
    // Reverse the order of bytes to get from big-endian to little-endian
    Sefram.Byte[0] = indata[start_index + 3];
    Sefram.Byte[1] = indata[start_index + 2];
    Sefram.Byte[2] = indata[start_index + 1];
    Sefram.Byte[3] = indata[start_index + 0];
    return Sefram.Float;
}

uint32_t scan::uint32FromBEFrame(byte indata[], int start_index)
{
    SeFrame4 Sefram;
    // Reverse the order of bytes to get from big-endian to little-endian
    Sefram.Byte[0] = indata[start_index + 3];
    Sefram.Byte[1] = indata[start_index + 2];
    Sefram.Byte[2] = indata[start_index + 1];
    Sefram.Byte[3] = indata[start_index + 0];
    return Sefram.Int;
}

// This function returns an integer from a 2-byte big-endian array beginning
// at a specific index of another array.
int scan::uint16FromBEFrame(byte indata[], int start_index)
{
    SeFrame2 Sefram;
    // Reverse the order of bytes to get from big-endian to little-endian
    Sefram.Byte[0] = indata[start_index + 1];
    Sefram.Byte[1] = indata[start_index + 0];
    return Sefram.Int;
}

// This returns a "String" from a slice of a character array
String scan::StringFromFrame(byte indata[], int start_index, int length)
{
    char charString[length+1] = {0,};
    int j = 0;
    for (int i = start_index; i < start_index+length; i++)
    {
        charString[j] = responseBuffer[i];  // converts from "byte" or "byte" type to "char" type
        j++;
    }
    String string = String(charString);
    string.trim();
    return string;
}

// Thes gets the register address and register type of a pointer to other
// information within a modbus register.
// The register address and type must be initialized prior to calling this function
// For the register types:
//   0 (0b00) - Holding register (read by command 0x03, written by 0x06 or 0x10)
//   1 (0b01) - Input register (read by command 0x04)
//   2 (0b10) - Discrete input register (read by command 0x02)
//   3 (0b10) - Coil  (read by command 0x01, written by 0x05)
int scan::pointerFromBEFrame(byte indata[], int start_index)
{
    SeFrame2 Sefram;
    // Reverse the order of bytes to get from big-endian to little-endian
    Sefram.Byte[0] = indata[start_index + 1]>>2;  // Bit shift the address lower bits
    Sefram.Byte[1] = indata[start_index + 0];
    return Sefram.Int;
}
int scan::pointerTypeFromBEFrame(byte indata[], int start_index)
{
    // Mask to get the last two bits , which are the type
    uint8_t regType = indata[start_index + 1] & 3;
    return regType;
}

// This function inserts a float as a 4-byte big endian array into another
// array beginning at the specified index.
void scan::floatIntoFrame(byte indata[], int start_index, float value)
{
    SeFrame4 Sefram;
    Sefram.Float = value;
    indata[start_index] = Sefram.Byte[3];
    indata[start_index + 1] = Sefram.Byte[2];
    indata[start_index + 2] = Sefram.Byte[1];
    indata[start_index + 3] = Sefram.Byte[0];
}

// This flips the device/receive enable to DRIVER so the arduino can send text
void scan::driverEnable(void)
{
    if (_enablePin > 0)
    {
        digitalWrite(_enablePin, HIGH);
        delay(8);
    }
}

// This flips the device/receive enable to RECIEVER so the sensor can send text
void scan::recieverEnable(void)
{
    if (_enablePin > 0)
    {
        digitalWrite(_enablePin, LOW);
        delay(8);
    }
}

// This empties the serial buffer
void scan::emptyResponseBuffer(Stream *stream)
{
    while (stream->available() > 0)
    {
        stream->read();
        delay(1);
    }
}

// Just a function to pretty-print the modbus hex frames
// This is purely for debugging
void scan::printFrameHex(byte modbusFrame[], int frameLength)
{
    _debugStream->print("{");
    for (int i = 0; i < frameLength; i++)
    {
        _debugStream->print("0x");
        if (modbusFrame[i] < 16) _debugStream->print("0");
        _debugStream->print(modbusFrame[i], HEX);
        if (i < frameLength - 1) _debugStream->print(", ");
    }
    _debugStream->println("}");
}


// Calculates a Modbus RTC cyclical redudancy code (CRC)
// and adds it to the last two bytes of a frame
// From: https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/
// and: https://stackoverflow.com/questions/19347685/calculating-modbus-rtu-crc-16
void scan::insertCRC(byte modbusFrame[], int frameLength)
{
    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < frameLength - 2; pos++)
    {
        crc ^= (unsigned int)modbusFrame[pos];  // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--) {    // Loop over each bit
            if ((crc & 0x0001) != 0) {    // If the least significant bit (LSB) is set
                crc >>= 1;                // Shift right and XOR 0xA001
                crc ^= 0xA001;
            }
            else                          // Else least significant bit (LSB) is not set
            crc >>= 1;                    // Just shift right
        }
    }

    // Break into low and high bytes
    byte crcLow = crc & 0xFF;
    byte crcHigh = crc >> 8;

    // Append the bytes to the end of the frame
    modbusFrame[frameLength - 2] = crcLow;
    modbusFrame[frameLength - 1] = crcHigh;
}

// This sends a command to the sensor bus and listens for a response
int scan::sendCommand(byte command[], int commandLength)
{
    // Add the CRC to the frame
    insertCRC(command, commandLength);

    // Send out the command
    driverEnable();
    _stream->write(command, commandLength);
    _stream->flush();
    // Print the raw send (for debugging)
    _debugStream->print("Raw Request: ");
    printFrameHex(command, commandLength);

    // Listen for a response
    recieverEnable();
    uint32_t start = millis();
    while (_stream->available() == 0 && millis() - start < modbusTimeout)
    { delay(1);}


    if (_stream->available() > 0)
    {
        // Read the incoming bytes
        int bytesRead = _stream->readBytes(responseBuffer, 135);
        emptyResponseBuffer(_stream);

        // Print the raw response (for debugging)
        _debugStream->print("Raw Response (");
        _debugStream->print(bytesRead);
        _debugStream->print(" bytes): ");
        printFrameHex(responseBuffer, bytesRead);

        return bytesRead;
    }
    else return 0;
}

// This sends three requests for a single register
// If the spectro::lyzer is sleeping, it will not respond until the third one
bool scan::wakeSpec(void)
{
    _debugStream->println("------>Checking if spectro::lyzer is awake.<------");
    byte get1Register[8] = {_slaveID, 0x03, 0x00, 0x00, 0x00, 0x01, 0x00, 0x00};
                          // Address, Read,   Reg 0,    1 Register,   CRC
                          //
    uint8_t attempts = 0;
    int respSize = 0;
    while (attempts < 3 and respSize < 7)
    {
        respSize = respSize + sendCommand(get1Register, 8);
        attempts ++;
    }
    if (respSize < 7)
    {
        _debugStream->println("------>No response from spectro::lyzer!<------");
        return false;
    }
    else
    {
        _debugStream->println("------>Spectro::lyser is now awake.<------");
        return true;
    }
}
