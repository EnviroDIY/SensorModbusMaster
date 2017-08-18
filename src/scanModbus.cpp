/*
 *scanModbus.cpp
*/

#include "scanModbus.h"

// initialize the response buffer
byte scan::responseBuffer[MAX_RESPONSE_SIZE] = {0x00,};

//----------------------------------------------------------------------------
//                          GENERAL USE FUNCTIONS
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
{return begin(modbusSlaveID, stream, enablePin);}


// This prints out all of the setup information to the selected stream
bool scan::printSetup(Stream *stream)
{
    // Wake up the spec if it was sleeping
    stream->println("------------------------------------------");
    wakeSpec();

    // Get the holding registers
    stream->println("------------------------------------------");
    _gotHoldingRegSpecSetup = getRegisters(0x03, 0, 27);

    // When returning a bunch of registers, as here, to get
    // the byte location in the frame of the desired register use:
    // (3 bytes of Modbus header + (2 bytes/register x (desired register - start register))

    if (_gotHoldingRegSpecSetup)
    {
        getCommunicationMode(5);
        getBaudRate(7);
        getParity(9);
        getprivateConfigRegister(13);
        getScanPoint(15);
        getCleaningMode(27);
        getCleaningInterval(29);
        getCleaningDuration(31);
        getCleaningWait(33);
        getSystemTime(35);
        getMeasInterval(47);
        getLoggingMode(49);
        getLoggingInterval(51);
        getNumLoggedResults(53);
        getIndexLogResult(55);
    }
    else return false;

    // Get the input registers
    stream->println("------------------------------------------");
    _gotInputRegSpecSetup = getRegisters(0x04, 0, 25);

    if (_gotInputRegSpecSetup)
    {
        getModbusVersion(3);
        getModel(9);
        getSerialNumber(29);
        getHWVersion(37);
        getSWVersion(41);
        getHWStarts(45);
        getParameterCount(47);
        getParamterType(49);
        getParameterScale(51);
    }
    else return false;

    // if all passed, return true
    stream->println("------------------------------------------");
    return true;
}
bool scan::printSetup(Stream &stream) {return printSetup(stream);}


// Reset all settings to default
bool scan::resetSettings(void)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = 0x01;
    return setRegisters(4, 1, byteToSend);
}


// This sets a new modbus slave ID
bool scan::setSlaveID(byte newSlaveID)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = newSlaveID;
    if (setRegisters(0, 1, byteToSend))
    {
        _slaveID = newSlaveID;
        return true;
    }
    else return false;
}


// This returns the current device status as a bitmap
int scan::getDeviceStatus(void)
{
    // Get the register data
    getRegisters(0x04, 120, 1);

    uint16_t status;
    dataFromFrame(status, bitmask);
    _debugStream->print("Current device status is: ");
    _debugStream->print(bitmask, BIN);
    _debugStream->print(" (");
    printParameterStatus(bitmask, _debugStream);
    _debugStream->println(")");
    return status;
}
// This prints out all of the setup information at once
void scan::printDeviceStatus(uint16_t bitmask, Stream *stream)
{
    // b15
    if ((bitmask & 32768) == 32768)
        stream->println("Device maintenance required");
    // b14
    if ((bitmask & 16384) == 16384)
        stream->println("Device cleaning required");
    // b13
    if ((bitmask & 8192) == 8192)
        stream->println("Device busy");
    // b3
    if ((bitmask & 8) == 8)
        stream->println("Data logger error, no readings can be stored because datalogger is full");
    // b2
    if ((bitmask & 4) == 4)
        stream->println("Missing or devective component detected");
    // b1
    if ((bitmask & 2) == 2)
        stream->println("Probe misuse, operation outside the specified temperature range");
    // b0
    if ((bitmask & 1) == 1)
        stream->println("s::can device reports error during internal check");
}
void scan::printDeviceStatus(uint16_t bitmask, Stream &stream)
{return printDeviceStatus(bitmask, stream);}





//----------------------------------------------------------------------------
//           FUNCTIONS TO RETURN THE ACTUAL SAMPLE TIMES AND VALUES
//----------------------------------------------------------------------------

// Last measurement time as a 32-bit count of seconds from Jan 1, 1970
// System time is in input registers 104-109
// (64-bit timestamp in TAI64 format + padding)
long scan::getSampleTime(int startIndex)
{
    getRegisters(0x04, 104, 6);

    uint32_t secsPast1970 = 0;
    dataFromFrame(secsPast1970, tai64, bigEndian, startIndex);
    _debugStream->print("Last sample was taken at ");
    _debugStream->print((unsigned long)(secsPast1970));
    _debugStream->println(" seconds past Jan 1, 1970");
    return secsPast1970;
}

// This gets values back from the sensor and puts them into a previously
// initialized float variable.  The actual return from the function is the
// int which is a bit-mask describing the parameter status.
int scan::getValue(int parmNumber, float &value)
{
    int regNumber = 12 + 8*parmNumber;
    // Get the register data
    getRegisters(0x04, regNumber, 8);

    uint16_t status;
    dataFromFrame(status, bitmask);
    float parm;
    dataFromFrame(parm, float32, bigEndian, 7);
    _debugStream->print("Value of parameter Number ");
    _debugStream->print(parmNumber);
    _debugStream->print(" is: ");
    _debugStream->print(parm);
    _debugStream->print(" with status code: ");
    _debugStream->print(bitmask, BIN);
    _debugStream->print(" (");
    printParameterStatus(bitmask, _debugStream);
    _debugStream->println(")");
    return parm;
}
void scan::printParameterStatus(uint16_t bitmask, Stream *stream)
{
    // b15
    if ((bitmask & 32768) == 32768)
        stream->println("Parameter reading out of measuring range");
    // b14
    if ((bitmask & 16384) == 16384)
        stream->println("Status of alarm paramter is 'WARNING'");
    // b13
    if ((bitmask & 8192) == 8192)
        stream->println("Status of alarm paramter is 'ALARM'");
    // b5
    if ((bitmask & 32) == 32)
        stream->println("Parameter not ready or not available");
    // b4
    if ((bitmask & 16) == 16)
        stream->println("Incorrect calibration, at least one calibration coefficient invalid");
    // b3
    if ((bitmask & 8) == 8)
        stream->println("Paremeter error, the sensor is outside of the medium or in incorrect medium");
    // b2
    if ((bitmask & 4) == 4)
        stream->println("Parameter error, calibration error");
    // b1
    if ((bitmask & 2) == 2)
        stream->println("Parameter error, hardware error");
    // b0
    if ((bitmask & 1) == 1)
        stream->println("Genereal parameter error, at least one internal parameter check failed");
}

// This get up to 8 values back from the spectro::lyzer
bool scan::getAllValues(float &value1, float &value2, float &value3, float &value4,
                  float &value5, float &value6, float &value7, float &value8)
{
    // Get the register data
    if (getRegisters(0x04, 128, 64))
    {
        dataFromFrame(value1, float32, bigEndian, 7);
        dataFromFrame(value2, float32, bigEndian, 23);
        dataFromFrame(value3, float32, bigEndian, 39);
        dataFromFrame(value4, float32, bigEndian, 55);
        dataFromFrame(value5, float32, bigEndian, 71);
        dataFromFrame(value6, float32, bigEndian, 87);
        dataFromFrame(value7, float32, bigEndian, 103);
        dataFromFrame(value8, float32, bigEndian, 119);
        _debugStream->println("Value1, value2, value3, value4, value5, value6, value7, value8");
        _debugStream->print(value1);
        _debugStream->print(", ");
        _debugStream->print(value2);
        _debugStream->print(", ");
        _debugStream->print(value3);
        _debugStream->print(", ");
        _debugStream->print(value4);
        _debugStream->print(", ");
        _debugStream->print(value5);
        _debugStream->print(", ");
        _debugStream->print(value6);
        _debugStream->print(", ");
        _debugStream->print(value7);
        _debugStream->print(", ");
        _debugStream->println(value8);
        return true;
    }
    else return false;
}



//----------------------------------------------------------------------------
//              FUNCTIONS TO GET AND CHANGE DEVICE CONFIGURATIONS
//----------------------------------------------------------------------------

// Functions for the communication mode
// The Communication mode is in holding register 1 (1 uint16 register)
int scan::getCommunicationMode(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 1, 1);
    }
    dataFromFrame(_commMode, uint16, bigEndian, startIndex);
    _debugStream->print("Communication mode setting is: ");
    _debugStream->print(_commMode);
    _debugStream->print(" (");
    _debugStream->print(printCommMode(_commMode));
    _debugStream->println(")");
    return _commMode;
}
bool scan::setCommunicationMode(specCommMode mode)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = mode;
    if (setRegisters(1, 1, byteToSend))
    {
        _commMode = mode;
        return true;
    }
    else return false;
}
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


// Functions for the serial baud rate (iff communication mode = modbus RTU or modbus ASCII)
// Baud rate is in holding register 2 (1 uint16 register)
int scan::getBaudRate(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 2, 1);
    }
    dataFromFrame(_baudRate, uint16, bigEndian, startIndex);
    _debugStream->print("Baud Rate setting is: ");
    _debugStream->print(_baudRate);
    _debugStream->print(" (");
    _debugStream->print(printBaudRate(_baudRate));
    _debugStream->println(")");
    return _baudRate;
}
bool scan::setBaudRate(specBaudRate baud)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = baud;
    if (setRegisters(2, 1, byteToSend))
    {
        _baudRate = baud;
        return true;
    }
    else return false;
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


// Functions for the serial parity (iff communication mode = modbus RTU or modbus ASCII)
// Parity is in holding register 3 (1 uint16 register)
int scan::getParity(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 3, 1);
    }
    dataFromFrame(_parity, uint16, bigEndian, startIndex);
    _debugStream->print("Parity setting is: ");
    _debugStream->print(_parity);
    _debugStream->print(" (");
    _debugStream->print(printParity(_parity));
    _debugStream->println(")");
    return _parity;
}
bool scan::setParity(specParity parity)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = parity;
    if (setRegisters(3, 1, byteToSend))
    {
        _parity = parity;
        return true;
    }
    else return false;
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

// Functions to get a pointer to the private configuration register
// Pointer to the private configuration is in holding register 5
// This is read only
int scan::getprivateConfigRegister(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 5, 1);
    }
    dataFromFrame(_configRegNumber, pointer, bigEndian, startIndex);
    dataFromFrame(_configRegType, pointerType, bigEndian, startIndex);
    _debugStream->print("Private configuration begins in register ");
    _debugStream->print(_configRegNumber);
    _debugStream->print(", which is type ");
    _debugStream->print(_configRegType);
    _debugStream->print(" (");
    _debugStream->print(printRegisterType(_configRegType));
    _debugStream->println(")");
    return _configRegNumber;
}
String scan::printRegisterType(uint16_t code)
{
    switch (code)
    {
        case 0: return "Holding register";  // 0b00 - read by command 0x03, written by 0x06 or 0x10
        case 1: return "Input register";  // 0b01 - read by command 0x04
        case 2: return "Discrete input register";  // 0b10 - read by command 0x02
        case 3: return "Coil";  // 0b10) - read by command 0x01, written by 0x05
        default: return "Unknown";
    }
}


// Functions for the "s::canpoint" of the device
// Device Location (s::canpoint) is registers 6-11 (char[12])
// This is read only
String scan::getScanPoint(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 6, 6);
    }
    dataFromFrame(_scanPoint, character, 12, startIndex);
    _debugStream->print("Current s::canpoint is: ");
    _debugStream->println(_scanPoint);
    return _scanPoint;
}
bool scan::setScanPoint(char charScanPoint[12])
{
    byte sp[12] = {0,};
    for (int i = 0; i < 12; i++) sp[i] = charScanPoint[i];
    if (setRegisters(6, 6, sp))
    {
        _scanPoint = String(charScanPoint);
        _scanPoint.trim();
        return true;
    }
    else return false;
}

// Functions for the cleaning mode configuration
// Cleaning mode is in holding register 12 (1 uint16 register)
int scan::getCleaningMode(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 12, 1);
    }
    dataFromFrame(_cleaningMode, uint16, bigEndian, startIndex);
    _debugStream->print("Cleaning mode setting is: ");
    _debugStream->print(_cleaningMode);
    _debugStream->print(" (");
    _debugStream->print(printCleaningMode(_cleaningMode));
    _debugStream->println(")");
    return _cleaningMode;
}
bool scan::setCleaningMode(cleaningMode mode)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = mode;
    if (setRegisters(12, 1, byteToSend))
    {
        _cleaningMode = mode;
        return true;
    }
    else return false;
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


// Functions for the cleaning interval (ie, number of samples between cleanings)
// Cleaning interval is in holding register 13 (1 uint16 register)
int scan::getCleaningInterval(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 13, 1);
    }
    dataFromFrame(_cleaningInterval, uint16, bigEndian, startIndex);
    _debugStream->print("Cleaning interval is: ");
    _debugStream->print(_cleaningInterval);
    _debugStream->println(" measurements between cleanings");
    return _cleaningInterval;
}
bool scan::setCleaningInterval(uint16_t intervalSamples)
{
    // Using a small-endian frame to get into bytes and then reverse the order
    SeFrame sefram;
    sefram.Int16[0] = intervalSamples;
    byte byteToSend[2];
    byteToSend[0] = sefram.Byte[1];
    byteToSend[1] = sefram.Byte[0];
    if (setRegisters(13, 1, byteToSend))
    {
        _cleaningInterval = intervalSamples;
        return true;
    }
    else return false;
}

// Functions for the cleaning duration in seconds
// Cleaning duration is in holding register 14 (1 uint16 register)
int scan::getCleaningDuration(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 14, 1);
    }
    dataFromFrame(_cleaningDuration, uint16, bigEndian, startIndex);
    _debugStream->print("Cleaning time is: ");
    _debugStream->print(_cleaningDuration);
    _debugStream->println(" seconds");
    return _cleaningDuration;
}
bool scan::setCleaningDuration(uint16_t secDuration)
{
    // Using a small-endian frame to get into bytes and then reverse the order
    SeFrame sefram;
    sefram.Int16[0] = secDuration;
    byte byteToSend[2];
    byteToSend[0] = sefram.Byte[1];
    byteToSend[1] = sefram.Byte[0];
    if (setRegisters(14, 1, byteToSend))
    {
        _cleaningDuration = secDuration;
        return true;
    }
    else return false;
}

// Functions for the waiting time between end of cleaning
// and the start of a measurement
// Cleaning wait time is in holding register 15 (1 uint16 register)
int scan::getCleaningWait(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 15, 1);
    }
    dataFromFrame(_cleaningWait, uint16, bigEndian, startIndex);
    _debugStream->print("Wait time between cleaning and sampling is: ");
    _debugStream->print(_cleaningWait);
    _debugStream->println(" seconds");
    return _cleaningWait;
}
bool scan::setCleaningWait(uint16_t secDuration)
{
    // Using a small-endian frame to get into bytes and then reverse the order
    SeFrame sefram;
    sefram.Int16[0] = secDuration;
    byte byteToSend[2];
    byteToSend[0] = sefram.Byte[1];
    byteToSend[1] = sefram.Byte[0];
    if (setRegisters(15, 1, byteToSend))
    {
        _cleaningWait = secDuration;
        return true;
    }
    else return false;
}

// Functions for the current system time in seconds from Jan 1, 1970
// System time is in holding registers 16-21
// (64-bit timestamp in TAI64 format + padding)
long scan::getSystemTime(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 16, 6);
    }
    uint32_t secsPast1970 = 0;
    dataFromFrame(secsPast1970, tai64, bigEndian, startIndex);
    _debugStream->print("Current System Time is: ");
    _debugStream->print((unsigned long)(secsPast1970));
    _debugStream->println(" seconds past Jan 1, 1970");
    return secsPast1970;
}
bool scan::setSystemTime(long currentUnixTime)
{
    // Using a small-endian frame to get into bytes and then reverse the order
    SeFrame sefram;
    sefram.Int32 = currentUnixTime;
    byte byteToSend[12] = {0,};
    byteToSend[0] = 0x40;  // It will be for the next 90 years
    byteToSend[4] = sefram.Byte[3];
    byteToSend[5] = sefram.Byte[2];
    byteToSend[6] = sefram.Byte[1];
    byteToSend[7] = sefram.Byte[0];
    return setRegisters(16, 6, byteToSend);
}

// Functions for the measurement interval in seconds (0 - as fast as possible)
// Measurement interval is in holding register 22 (1 uint16 register)
int scan::getMeasInterval(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 22, 1);
    }
    dataFromFrame(_measInterval, uint16, bigEndian, startIndex);
    _debugStream->print("Measurement interval is: ");
    _debugStream->print(_measInterval);
    _debugStream->println(" seconds");
    return _measInterval;
}
bool scan::setMeasInterval(uint16_t secBetween)
{
    // Using a small-endian frame to get into bytes and then reverse the order
    SeFrame sefram;
    sefram.Int16[0] = secBetween;
    byte byteToSend[2];
    byteToSend[0] = sefram.Byte[1];
    byteToSend[1] = sefram.Byte[0];
    if (setRegisters(22, 1, byteToSend))
    {
        _measInterval = secBetween;
        return true;
    }
    else return false;
}

// Functions for the logging Mode (0 = on; 1 = off)
// Logging Mode (0 = on; 1 = off) is in holding register 23 (1 uint16 register)
int scan::getLoggingMode(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 23, 1);
    }
    dataFromFrame(_loggingMode, uint16, bigEndian, startIndex);
    _debugStream->print("Logging mode setting is: ");
    _debugStream->print(_loggingMode);
    _debugStream->print(" (");
    _debugStream->print(printLoggingMode(_loggingMode));
    _debugStream->println(")");
    return _loggingMode;
}
bool scan::setLoggingMode(uint8_t mode)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = mode;
    if (setRegisters(23, 1, byteToSend))
    {
        _loggingMode = mode;
        return true;
    }
    else return false;
}
String scan::printLoggingMode(uint16_t code)
{
    switch (code)
    {
        case 0: return "Logging On";
        default: return "Logging Off";
    }
}


// Functions for the logging interval for data logger in minutes
// (0 = no logging active)
// Logging interval is in holding register 24 (1 uint16 register)
int scan::getLoggingInterval(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 24, 1);
    }
    dataFromFrame(_loggingInterval, uint16, bigEndian, startIndex);
    _debugStream->print("Logging interval is: ");
    _debugStream->print(_loggingInterval);
    _debugStream->println(" seconds");
    return _loggingInterval;
}
bool scan::setLoggingInterval(uint16_t interval)
{
    // Using a small-endian frame to get into bytes and then reverse the order
    SeFrame sefram;
    sefram.Int16[0] = interval;
    byte byteToSend[2];
    byteToSend[0] = sefram.Byte[1];
    byteToSend[1] = sefram.Byte[0];
    if (setRegisters(24, 1, byteToSend))
    {
        _loggingInterval = interval;
        return true;
    }
    else return false;
}

// Available number of logged results in datalogger since last clearing
int scan::getNumLoggedResults(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        // Available number of logged results is in holding register 25 (1 uint16 register)
        getRegisters(0x03, 25, 1);
    }
    dataFromFrame(_numLoggedResults, uint16, bigEndian, startIndex);
    _debugStream->print(_numLoggedResults);
    _debugStream->println(" results have been logged so far");
    return _numLoggedResults;
}

// "Index device status public + private & parameter results from logger
// storage to Modbus registers.  If no stored results are available,
// results are NaN, Device status bit3 is set."
// I'm really not sure what this means...
int scan::getIndexLogResult(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        // "Index device status" is in holding register 26 (1 uint16 register)
        getRegisters(0x03, 26, 1);
    }
    dataFromFrame(_indexLogResult, uint16, bigEndian, startIndex);
    _debugStream->print("Index device status is: ");
    _debugStream->println(_indexLogResult);
    return _indexLogResult;
}



//----------------------------------------------------------------------------
//               SETUP INFORMATION FROM THE INPUT REGISTERS
//----------------------------------------------------------------------------
// Get the version of the modbus mapping protocol
float scan::getModbusVersion(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        getRegisters(0x04, 0, 1);
    }
    dataFromFrame(_modbusVersion, uint16, bigEndian, startIndex);
    SeFrame sefram;
    sefram.Int16[0] = _modbusVersion;
    printFrameHex(sefram.Byte, 4);
    float mjv = sefram.Byte[1];
    float mnv = sefram.Byte[0];
    mnv = mnv/100;
    float version = mjv + mnv;
    _debugStream->print("Modbus Version is: ");
    _debugStream->println(version);
    return version;
}

// This returns a pretty string with the model information
String scan::getModel(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        getRegisters(0x04, 3, 10);
    }
    dataFromFrame(_model, character, 20, startIndex);
    _debugStream->print("Instrument model is: ");
    _debugStream->println(_model);
    return _model;
}

// This gets the instrument serial number as a String
String scan::getSerialNumber(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        getRegisters(0x04, 13, 4);
    }
    dataFromFrame(_serialNumber, character, 8, startIndex);
    _debugStream->print("Instrument Serial Number is: ");
    _debugStream->println(_serialNumber);
    return _serialNumber;
}

// This gets the hardware version of the sensor
float scan::getHWVersion(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        getRegisters(0x04, 17, 2);
    }
    dataFromFrame(_model, character, 4, startIndex);
    float mjv = _model.substring(0,2).toFloat();
    float mnv = (_model.substring(2,4).toFloat())/100;
    float version = mjv + mnv;
    _debugStream->print("Hardware Version is: ");
    _debugStream->println(version);
    return version;
}

// This gets the software version of the sensor
float scan::getSWVersion(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        getRegisters(0x04, 19, 2);
    }
    dataFromFrame(_model, character, 4, startIndex);
    float mjv = _model.substring(0,2).toFloat();
    float mnv = (_model.substring(2,4).toFloat())/100;
    float version = mjv + mnv;
    _debugStream->print("Software Version is: ");
    _debugStream->println(version);
    return version;
}

// This gets the number of times the spec has been rebooted
// (Device rebooter counter)
int scan::getHWStarts(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        // "Index device status" is in holding register 26 (1 uint16 register)
        getRegisters(0x04, 21, 1);
    }
    dataFromFrame(_HWstarts, uint16, bigEndian, startIndex);
    _debugStream->print("Hardware has been restarted: ");
    _debugStream->print(_HWstarts);
    _debugStream->println(" times");
    return _HWstarts;
}

// This gets the number of parameters the spectro::lyzer is set to measure
int scan::getParameterCount(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        // "Index device status" is in holding register 26 (1 uint16 register)
        getRegisters(0x04, 22, 1);
    }
    dataFromFrame(_paramCount, uint16, bigEndian, startIndex);
    _debugStream->print("There are ");
    _debugStream->print(_paramCount);
    _debugStream->println(" parameters being measured");
    return _paramCount;
}

// This gets the datatype of the parameters and parameter limits
// This is a check for compatibility
int scan::getParamterType(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        // "Index device status" is in holding register 26 (1 uint16 register)
        getRegisters(0x04, 23, 1);
    }
    dataFromFrame(_paramType, uint16, bigEndian, startIndex);
    _debugStream->print("The data type of the parameters is: ");
    _debugStream->print(_paramType);
    _debugStream->print(" (");
    _debugStream->print(printParamterType(_paramType));
    _debugStream->println(")");
    return _paramType;
}
String scan::printParamterType(uint16_t code)
{
    switch (code)
    {
        case 0: return "uint16?";
        case 1: return "enum?";
        case 2: return "bitmask?";
        case 3: return "char?";
        case 4: return "float?";
        default: return "Unknown";
    }
}

// This gets the scaling factor for all parameters which depend on eParameterType
int scan::getParameterScale(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        // "Index device status" is in holding register 26 (1 uint16 register)
        getRegisters(0x04, 24, 1);
    }
    dataFromFrame(_paramScale, uint16, bigEndian, startIndex);
    _debugStream->print("The parameter scale factor is: ");
    _debugStream->println(_paramScale);
    return _paramScale;
}



//----------------------------------------------------------------------------
//             PARAMETER INFORMATION FROM THE HOLDING REGISTERS
//----------------------------------------------------------------------------

// This returns a pretty string with the parameter measured.
String scan::getParameter(int parmNumber)
{
    int regNumber = 120*parmNumber;
    // Get the register data
    getRegisters(0x03, regNumber, 4);

    String parm;
    dataFromFrame(parm, character, 8);
    _debugStream->print("Parameter Number ");
    _debugStream->print(parmNumber);
    _debugStream->print(" is: ");
    _debugStream->println(parm);
    return parm;
}

// This returns a pretty string with the measurement units.
String scan::getUnits(int parmNumber)
{
    int regNumber = 120*parmNumber + 4;
    // Get the register data
    getRegisters(0x03, regNumber, 4);

    String parm;
    dataFromFrame(parm, character, 8);
    _debugStream->print("Parameter Number ");
    _debugStream->print(parmNumber);
    _debugStream->print(" has units of: ");
    _debugStream->println(parm);
    return parm;
}

// This gets the upper limit of the parameter
float scan::getUpperLimit(int parmNumber)
{
    int regNumber = 120*parmNumber + 8;
    // Get the register data
    getRegisters(0x03, regNumber, 2);

    float parm;
    dataFromFrame(parm, float32);
    _debugStream->print("Upper limit of parameter Number ");
    _debugStream->print(parmNumber);
    _debugStream->print(" is: ");
    _debugStream->println(parm);
    return parm;
}

// This gets the lower limit of the parameter
float scan::getLowerLimit(int parmNumber)
{
    int regNumber = 120*parmNumber + 10;
    // Get the register data
    getRegisters(0x03, regNumber, 2);

    float parm;
    dataFromFrame(parm, float32);
    _debugStream->print("Lower limit of parameter Number ");
    _debugStream->print(parmNumber);
    _debugStream->print(" is: ");
    _debugStream->println(parm);
    return parm;
}

void scan::printSystemStatus(uint16_t bitmask, Stream *stream)
{
    // b6
    if ((bitmask & 64) == 64)
        stream->println("mA signal is outside of the allowed input range");
    // b5
    if ((bitmask & 32) == 32)
        stream->println("Validation results are not available");
    // b1
    if ((bitmask & 2) == 2)
        stream->println("Invalid probe/sensor; serial number of probe/sensor is different");
    // b0
    if ((bitmask & 1) == 1)
        stream->println("No communication between probe/sensor and controller");
}




//----------------------------------------------------------------------------
//                           PRIVATE HELPER FUNCTIONS
//----------------------------------------------------------------------------

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

// This gets data from either a holding or input register
// For a holding register readCommand = 0x03
// For an input register readCommand = 0x04
bool scan::getRegisters(byte readCommand, int16_t startRegister, int16_t numRegisters)
{
    // Create an array for the command
    byte command[8];

    // Put in the slave id and the command
    command[0] = _slaveID;
    command[1] = readCommand;

    // Put in the starting register
    SeFrame Sefram = {0,};
    Sefram.Int16[0] = startRegister;
    command[2] = Sefram.Byte[1];
    command[3] = Sefram.Byte[0];

    // Put in the number of registers
    Sefram.Int16[1] = numRegisters;
    command[4] = Sefram.Byte[3];
    command[5] = Sefram.Byte[2];

    // Send out the command (this adds the CRC)
    int16_t respSize = sendCommand(command, 8);

    // The size of the returned frame should be:
    // # Registers X 2 bytes/register + 5 bytes of modbus frame
    if (respSize == (numRegisters*2 + 5) && responseBuffer[0] == _slaveID)
        return true;
    else return false;
};

// This sets the value of one or more holding registers
// Modbus commands 0x06 and 0x10 (16)
bool scan::setRegisters(int16_t startRegister, int16_t numRegisters, byte value[])
{
    // figure out how long the command will be
    int commandLength;
    if (numRegisters > 1) commandLength = numRegisters*2 + 7;
    else commandLength = numRegisters*2 + 6;

    // Create an array for the command
    byte command[commandLength] = {0,};

    // Put in the slave id and the command
    command[0] = _slaveID;
    if (numRegisters > 1) command[1] = 0x10;
    else command[1] = 0x06;

    // Put in the starting register
    SeFrame Sefram = {0,};
    Sefram.Int16[0] = startRegister;
    command[2] = Sefram.Byte[1];
    command[3] = Sefram.Byte[0];

    // Put in the register values
    if (numRegisters > 1)
    {
        // Put in the number of registers
        Sefram.Int16[1] = numRegisters;
        command[4] = Sefram.Byte[3];
        command[5] = Sefram.Byte[2];
        // Put in the number of bytes to write
        command[6] = numRegisters*2;
        // Put in the data
        for (int i = 7; i < numRegisters*2 + 7; i++) command[i] = value[i-7];
    }
    else
    {
        // Only have to put in the data
        for (int i = 4; i < numRegisters*2 + 4; i++) command[i] = value[i-4];
    }

    // Send out the command (this adds the CRC)
    // printFrameHex(command,commandLength);
    int16_t respSize = sendCommand(command, commandLength);

    // The structure of the response should be:
    // {slaveID, fxnCode, Address of 1st register, # Registers, CRC}
    if (respSize == 8 && responseBuffer[0] == _slaveID && responseBuffer[5] == numRegisters)
        return true;
    else return false;
};

// This slices one array out of another
// Used for slicing one or more registers out of a returned modbus frame
void scan::sliceArray(byte inputArray[], byte outputArray[],
                int start_index, int numBytes, bool reverseOrder)
{
    // _debugStream->println("------------------");
    // _debugStream->print("  Returned Register Number: ");
    // _debugStream->println((start_index-3)/2);
    // _debugStream->print("  Variable Length: ");
    // _debugStream->print(numBytes);
    // _debugStream->print("  (");
    // _debugStream->print(numBytes/2);
    // _debugStream->println(" Registers)");

    if (reverseOrder)
    {
        // Reverse the order of bytes to get from big-endian to little-endian
        int j = numBytes - 1;
        for (int i = 0; i < numBytes; i++)
        {
            outputArray[i] = inputArray[start_index + j];
            j--;
        }

    }
    else
    {
        for (int i = 0; i < numBytes; i++)
            outputArray[i] = inputArray[start_index + i];
    }

    // _debugStream->print("  Sliced Array:");
    // _debugStream->print("  ");
    // printFrameHex(outputArray, numBytes);
}

// These functions returns data from a register within a modbus frame
// The outputVar must always be initialized prior to calling this function
bool scan::dataFromFrame(uint16_t &outputVar, dataTypes regType,
                         endianness endian,
                         int start_index,
                         byte indata[]
                         )
{
    // Read a substring of the input frame into an "output frame"
    int varLength = 2;
    byte outFrame[varLength] = {0,};
    if (endian == bigEndian)
        sliceArray(indata, outFrame, start_index, varLength, true);
    else sliceArray(indata, outFrame, start_index, varLength, false);
    // Put it into a small-endian frame (the format of all arduino processors)
    SeFrame Sefram = {0,};
    memcpy(Sefram.Byte, outFrame, varLength);

    switch (regType)
    {
        case uint16:
        {
            outputVar = Sefram.Int16[0];
            return true;
        }
        case bitmask:
        {
            outputVar = Sefram.Int16[0];
            return true;
        }
        case pointer:
        {
            Sefram.Byte[0] = indata[start_index + 1]>>2;  // Bit shift the address lower bits
            outputVar = Sefram.Int16[0];
            return true;
        }
        case pointerType:
        {
            // Mask to get the last two bits , which are the type
            uint8_t pointerRegType = outFrame[0] & 3;
            outputVar = pointerRegType;
            return true;
        }
        default:
        {
            return false;
        }
    }
}
bool scan::dataFromFrame(float &outputVar, dataTypes regType,
                         endianness endian,
                         int start_index,
                         byte indata[]
                         )
{
    // Read a substring of the input frame into an "output frame"
    int varLength = 4;
    byte outFrame[varLength] = {0,};
    if (endian == bigEndian)
        sliceArray(indata, outFrame, start_index, varLength, true);
    else sliceArray(indata, outFrame, start_index, varLength, false);

    SeFrame Sefram = {0,};
    memcpy(Sefram.Byte, outFrame, varLength);

    switch (regType)
    {
        case float32:
        {
            outputVar = Sefram.Float;
            return true;
        }
        default: return false;
    }
}
bool scan::dataFromFrame(uint32_t &outputVar, dataTypes regType,
                         endianness endian,
                         int start_index,
                         byte indata[]
                         )
{
    switch (regType)
    {
        case tai64:
        {
            // This is a 6-register data type BUT:
            // The first two registers will be 0x4000 0000 until the year 2106;
            // I'm ignoring it for the next 90 years to avoid using 64bit math
            // The next two registers will have the actual seconds past Jan 1, 1970
            // The last two registers are just 0's and can be ignored.
            // Per the TAI61 standard, this value is always big-endian
            // https://www.tai64.com/
            int varLength = 4;
            byte outFrame[varLength] = {0,};
            sliceArray(indata, outFrame, start_index+4, varLength, true);
            SeFrame Sefram = {0,};
            memcpy(Sefram.Byte, outFrame, varLength);
            outputVar = Sefram.Int32;
            return true;
        }
        default: return false;
    }
}
bool scan::dataFromFrame(String &outputVar, dataTypes regType, int charLength,
                         int start_index,
                         byte indata[]
                         )
{
    switch (regType)
    {
        case character:
        {
            char charString[24] = {0,};  // Pick a value longer than then longest string returned
            int j = 0;
            for (int i = start_index; i < start_index + charLength; i++)
            {
                charString[j] = responseBuffer[i];  // converts from "byte" type to "char" type
                j++;
            }
            String string = String(charString);
            string.trim();
            outputVar = string;
            return true;
        }
        default: return false;
    }
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
