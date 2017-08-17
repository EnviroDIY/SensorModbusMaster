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
    // Wake up the spec if it was sleeping
    wakeSpec();

    // Get the holding registers
    _gotHoldingRegSpecSetup = getRegisters(0x03, 0, 27);

    // When returning a bunch of registers, as here, to get
    // the byte location in the frame of the desired register use:
    // (3 bytes of Modbus header + (2 bytes/register x (desired register - start register))

    if (_gotHoldingRegSpecSetup)
    {
        // Communication mode is in holding register 1 (1 uint16 register)
        getCommunicationMode(5);
        // Baud rate is in holding register 2 (1 uint16 register)
        getBaudRate(7);
        // Parity is in holding register 3 (1 uint16 register)
        getParity(9);
        // Skipping register 4, which is just to be written to reset all
        // settings back to default.
        // Pointer to the private configuration is in holding register 5
        getprivateConfigRegister(13);
        // Device Location (s::canpoint) is registers 6-11 (char[12])
        getScanPoint(15);
        // Cleaning mode is in holding register 12 (1 uint16 register)
        getCleaningMode(27);
        // Cleaning interval is in holding register 13 (1 uint16 register)
        getCleaningInterval(29);
        // Cleaning duration is in holding register 14 (1 uint16 register)
        getCleaningDuration(31);
        // Cleaning wait time is in holding register 15 (1 uint16 register)
        getCleaningWait(33);
        // System time is in holding registers 16-21 (64-bit timestamp in TAI64 format + padding)
        getSystemTime(35);
        // Measurement interval is in holding register 22 (1 uint16 register)
        getMeasInterval(47);
        // Logging Mode (0 = on; 1 = off) is in holding register 23 (1 uint16 register)
        getLoggingMode(49);
        // Logging interval is in holding register 24 (1 uint16 register)
        getLoggingInterval(51);
        // Available number of logged results is in holding register 25 (1 uint16 register)
        getNumLoggedResults(53);
        // "Index device status" is in holding register 26 (1 uint16 register)
        getIndexLogResult(55);
        // Return true after parsing everything
        return true;
    }
    else return false;
}


// Reset all settings to default
bool scan::resetSettings(void)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = 0x01;
    if (setRegisters(4, 1, byteToSend))
    {
        getSetup();
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



//----------------------------------------------------------------------------
//               SETUP INFORMATION FROM THE HOLDING REGISTERS
//----------------------------------------------------------------------------
// Functions for the communication mode
// The Communication mode is in holding register 1 (1 uint16 register)
int scan::getCommunicationMode(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 1, 1);
    }
    dataFromFrame(_commMode, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_baudRate, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_parity, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_configRegNumber, pointer, responseBuffer, startIndex);
    dataFromFrame(_configRegType, pointerType, responseBuffer, startIndex);
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
    dataFromFrame(_scanPoint, character, responseBuffer, startIndex, 12);
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
}

// Functions for the cleaning mode configuration
// Cleaning mode is in holding register 12 (1 uint16 register)
int scan::getCleaningMode(int startIndex)
{
    if (!_gotHoldingRegSpecSetup)
    {
        getRegisters(0x03, 12, 1);
    }
    dataFromFrame(_cleaningMode, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_cleaningInterval, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_cleaningDuration, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_cleaningWait, uint16, responseBuffer, startIndex);
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
    dataFromFrame(secsPast1970, tai64, responseBuffer, startIndex);
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
    dataFromFrame(_measInterval, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_loggingMode, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_loggingInterval, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_numLoggedResults, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_indexLogResult, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_modbusVersion, uint16, responseBuffer, startIndex);
    SeFrame sefram;
    sefram.Int16[0] = _modbusVersion;
    float mjv = sefram.Byte[1];
    float mnv = (sefram.Byte[0])/100;
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
    dataFromFrame(_model, character, responseBuffer, startIndex, 10);
    _debugStream->print("Instrument model is: ");
    _debugStream->println(_model);
    return _model;
}

// This gets the instrument serial number as a String
String scan::getSerialNumber(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        getRegisters(0x04, 13, 8);
    }
    dataFromFrame(_serialNumber, character, responseBuffer, startIndex, 8);
    _debugStream->print("Instrument Serial Number is: ");
    _debugStream->println(_serialNumber);
    return _model;
}

// This gets the hardware version of the sensor
float scan::getHWVersion(int startIndex)
{
    if (!_gotInputRegSpecSetup)
    {
        getRegisters(0x04, 17, 4);
    }
    dataFromFrame(_model, character, responseBuffer, startIndex, 4);
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
        getRegisters(0x04, 19, 4);
    }
    dataFromFrame(_model, character, responseBuffer, startIndex, 4);
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
    dataFromFrame(_HWstarts, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_paramCount, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_paramType, uint16, responseBuffer, startIndex);
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
    dataFromFrame(_paramScale, uint16, responseBuffer, startIndex);
    _debugStream->print("The parameter scale factor is: ");
    _debugStream->println(_paramScale);
    return _paramScale;
}



//----------------------------------------------------------------------------
//             PARAMETER INFORMATION FROM THE HOLDING REGISTERS
//----------------------------------------------------------------------------

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
long scan::getSampleTime(int startIndex)
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
    if (numRegisters > 1) commandLength = numRegisters*2 + 6;
    else commandLength = numRegisters*2 + 5;

    // Create an array for the command
    byte command[commandLength];

    // Put in the slave id and the command
    command[0] = _slaveID;
    if (numRegisters > 1) command[1] = 0x06;
    else command[1] = 0x10;

    // Put in the starting register
    SeFrame Sefram = {0,};
    Sefram.Int16[0] = startRegister;
    command[2] = Sefram.Byte[1];
    command[3] = Sefram.Byte[0];

    // Put in the number of registers
    Sefram.Int16[1] = numRegisters;
    command[4] = Sefram.Byte[3];
    command[5] = Sefram.Byte[2];

    // Put in the register values
    if (numRegisters > 1)
    {
        command[6] = numRegisters*2;
        for (int i = 7; i < numRegisters*2 + 7; i++)
        {
            command[i] = value[i-7];
        }
    }
    else
    {
        for (int i = 6; i < numRegisters*2; i++)
        {
            command[i] = value[i-6];
        }
    }

    // Send out the command (this adds the CRC)
    int16_t respSize = sendCommand(command, commandLength);

    // The structure of the response should be:
    // {slaveID, fxnCode, Address of 1st register, # Registers, CRC}
    if (respSize == 8 && responseBuffer[0] == _slaveID && responseBuffer[5] == numRegisters)
        return true;
    else return false;
};

// This slices one array out of another
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
bool scan::dataFromFrame(uint16_t &outputVar, dataTypes regType, byte indata[],
                           int start_index, endianness endian)
{
    // Read a substring of the input frame into an "output frame"
    int varLength = 2;
    byte outFrame[varLength] = {0,};
    if (endian == big) sliceArray(indata, outFrame, start_index, varLength, true);
    else sliceArray(indata, outFrame, start_index, varLength, false);

    SeFrame Sefram = {0,};
    memcpy(Sefram.Byte, outFrame, varLength);

    switch (regType)
    {
        case uint16:
        case bitmask:
        {
            outputVar = Sefram.Int16[0];
            return true;
            break;
        }
        case pointer:
        {
            Sefram.Byte[0] = indata[start_index + 1]>>2;  // Bit shift the address lower bits
            outputVar = Sefram.Int16[0];
            return true;
            break;
        }
        case pointerType:
        {
            // Mask to get the last two bits , which are the type
            uint8_t pointerRegType = outFrame[0] & 3;
            outputVar = pointerRegType;
            return true;
            break;
        }
        default:
        {
            return false;
            break;
        }
    }
}
bool scan::dataFromFrame(float &outputVar, dataTypes regType, byte indata[],
                           int start_index, endianness endian)
{
    // Read a substring of the input frame into an "output frame"
    int varLength = 4;
    byte outFrame[varLength] = {0,};
    if (endian == big) sliceArray(indata, outFrame, start_index, varLength, true);
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
bool scan::dataFromFrame(String &outputVar, dataTypes regType, byte indata[],
                           int start_index, int charLength)
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
bool scan::dataFromFrame(uint32_t &outputVar, dataTypes regType, byte indata[],
                           int start_index, endianness endian)
{
    // Read a substring of the input frame into an "output frame"

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
