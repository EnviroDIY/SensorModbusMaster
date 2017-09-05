/*
 *scanModbus.cpp
*/

#include "scanModbus.h"
#include <TimeLib.h>  // for dealing with the unix time

// initialize the response buffer
byte scan::responseBuffer[MAX_RESPONSE_SIZE] = {0x00,};
byte scan::crcFrame[2] = {0x00,};

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
{return begin(modbusSlaveID, &stream, enablePin);}


// This prints out all of the setup information to the selected stream
bool scan::printSetup(Stream *stream)
{
    // Wake up the spec if it was sleeping
    stream->println("------------------------------------------");
    wakeSpec();

    // When returning a bunch of registers, as here, to get
    // the byte location in the frame of the desired register use:
    // (3 bytes of Modbus header + (2 bytes/register x (desired register - start register))

    // Get the holding registers
    stream->println("------------------------------------------");
    if (getRegisters(0x03, 0, 27))
    {
        // Setup information from holding registers
        stream->print("Communication mode setting is: ");
        stream->print(uint16FromFrame(bigEndian, 5));
        stream->print(" (");
        stream->print(parseCommunicationMode(uint16FromFrame(bigEndian, 5)));
        stream->println(")");

        stream->print("Baud Rate setting is: ");
        stream->print(uint16FromFrame(bigEndian, 7));
        stream->print(" (");
        stream->print(parseBaudRate(uint16FromFrame(bigEndian, 7)));
        stream->println(")");

        stream->print("Parity setting is: ");
        stream->print(uint16FromFrame(bigEndian, 9));
        stream->print(" (");
        stream->print(parseParity(uint16FromFrame(bigEndian, 9)));
        stream->println(")");

        stream->print("Private configuration begins in register ");
        stream->print(pointerFromFrame(bigEndian, 13));
        stream->print(", which is type ");
        stream->print(pointerTypeFromFrame(bigEndian, 13));
        stream->print(" (");
        stream->print(parseRegisterType(pointerTypeFromFrame(bigEndian, 13)));
        stream->println(")");

        stream->print("Current s::canpoint is: ");
        stream->println(StringFromFrame(12, 15));

        stream->print("Cleaning mode setting is: ");
        stream->print(uint16FromFrame(bigEndian, 27));
        stream->print(" (");
        stream->print(parseCleaningMode(uint16FromFrame(bigEndian, 27)));
        stream->println(")");

        stream->print("Cleaning interval is: ");
        stream->print(uint16FromFrame(bigEndian, 29));
        stream->println(" measurements between cleanings");

        stream->print("Cleaning time is: ");
        stream->print(uint16FromFrame(bigEndian, 31));
        stream->println(" seconds");

        stream->print("Wait time between cleaning and sampling is: ");
        stream->print(uint16FromFrame(bigEndian, 33));
        stream->println(" seconds");

        stream->print("Current System Time is: ");
        stream->print((uint32_t)(tai64FromFrame(35)));
        stream->println(" seconds past Jan 1, 1970");

        stream->print("Measurement interval is: ");
        stream->print(uint16FromFrame(bigEndian, 47));
        stream->println(" seconds");

        stream->print("Logging mode setting is: ");
        stream->print(uint16FromFrame(bigEndian, 49));
        stream->print(" (");
        stream->print(parseLoggingMode(uint16FromFrame(bigEndian, 49)));
        stream->println(")");

        stream->print("Logging interval is: ");
        stream->print(uint16FromFrame(bigEndian, 51));
        stream->println(" seconds");

        stream->print(uint16FromFrame(bigEndian, 53));
        stream->println(" results have been logged so far");

        stream->print("Index device status is: ");
        stream->println(uint16FromFrame(bigEndian, 53));
    }
    else return false;

    // Get some of the register input that it's a pain to pull up separtely
    stream->println("------------------------------------------");

    stream->print("Modbus Version is: ");
    stream->println(getModbusVersion());
    stream->print("Hardware Version is: ");
    stream->println(getHWVersion());
    stream->print("Software Version is: ");
    stream->println(getSWVersion());

    // Get rest of the input registers
    int parmCount;
    if (getRegisters(0x04, 0, 25))
    {
        // Setup information from input registers

        stream->print("Instrument model is: ");
        stream->println(StringFromFrame(20, 9));

        stream->print("Instrument Serial Number is: ");
        stream->println(StringFromFrame(8, 29));

        stream->print("Hardware has been restarted: ");
        stream->print(uint16FromFrame(bigEndian, 45));
        stream->println(" times");

        stream->print("There are ");
        parmCount = int16FromFrame(bigEndian, 47);
        stream->print(parmCount);
        stream->println(" parameters being measured");

        stream->print("The data type of the parameters is: ");
        stream->print(uint16FromFrame(bigEndian, 49));
        stream->print(" (");
        stream->print(parseParameterType(uint16FromFrame(bigEndian, 49)));
        stream->println(")");

        stream->print("The parameter scale factor is: ");
        stream->println(uint16FromFrame(bigEndian, 51));
    }
    else return false;

    // Get the parameter info
    for (int i = 1; i < parmCount+1; i++)
    {
        stream->println("------------------------------------------");
        stream->print("Parameter Number ");
        stream->print(i);
        stream->print(" is ");
        stream->print(getParameter(i));
        stream->print(" and has units of ");
        stream->print(getUnits(i));
        stream->print(". The lower limit is ");
        stream->print(getLowerLimit(i));
        stream->print(" and the upper limit is ");
        stream->print(getUpperLimit(i));
        stream->println(".");
    }

    // if all passed, return true
    stream->println("------------------------------------------");
    return true;
}
bool scan::printSetup(Stream &stream) {return printSetup(&stream);}


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
    return setRegisters(0, 1, byteToSend);
}


// This returns the current device status as a bitmap
int scan::getDeviceStatus(void)
{
    return bitmaskFromRegister(0x04, 120);
}
// This parses the device status bitmask and prints out from the codes
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
    // No Codes
    if (bitmask == 0)
        stream->println("Device is operating normally");
}
void scan::printDeviceStatus(uint16_t bitmask, Stream &stream)
{printDeviceStatus(bitmask, &stream);}


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
    // No Codes
    if (bitmask == 0)
        stream->println("System is operating normally");
}
void scan::printSystemStatus(uint16_t bitmask, Stream &stream)
{printSystemStatus(bitmask, &stream);}


// This sends three requests for a single register
// If the spectro::lyzer is sleeping, it will not respond until the third one
// Alternately, we could send " Weckzeichen" (German for "Ringtone") three
// times before each command, which is what ana::lyte and ana::pro do.
// " Weckzeichen" = 0x20, 0x57, 0x65, 0x63, 0x6b, 0x7a, 0x65, 0x69, 0x63, 0x68, 0x65, 0x6e
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




//----------------------------------------------------------------------------
//           FUNCTIONS TO RETURN THE ACTUAL SAMPLE TIMES AND VALUES
//----------------------------------------------------------------------------

// Last measurement time as a 32-bit count of seconds from Jan 1, 1970
// System time is in input registers 104-109
// (96-bit timestamp in TAI64N format - in this case, ignoring the nanoseconds)
uint32_t scan::getSampleTime(void)
{
    return tai64FromRegister(0x04, 104);
}
// This prints out the sample time, formatted as YYYY.MM.DD hh:mm:ss
void scan::printSampleTime(Stream *stream, bool addNL)
{
    // Get the sample timestamp
    uint32_t ts = getSampleTime();
    // Print it out in the right format
    stream->print(year(ts));
    stream->print(".");
    stream->print(month(ts));
    stream->print(".");
    stream->print(day(ts));
    if (hour(ts) < 10)
    {
        stream->print(" 0");
        stream->print(hour(ts));
    }
    else
    {
        stream->print(" ");
        stream->print(hour(ts));
    }
    if (minute(ts) < 10)
    {
        stream->print(":0");
        stream->print(minute(ts));
    }
    else
    {
        stream->print(":");
        stream->print(minute(ts));
    }
    if (second(ts) < 10)
    {
        stream->print(":0");
        stream->print(second(ts));
    }
    else
    {
        stream->print(":");
        stream->print(second(ts));
    }
    if (addNL) stream->println();
}
void scan::printSampleTime(Stream &stream, bool addNL)
{printSampleTime(&stream, addNL);}

// This gets values back from the sensor and puts them into a previously
// initialized float variable.  The actual return from the function is the
// int which is a bit-mask describing the parameter status.
int scan::getParameterValue(int parmNumber, float &value)
{
    int regNumber = 120 + 8*parmNumber;
    // Get the register data
    getRegisters(0x04, regNumber, 8);

    uint16_t status = bitmaskFromFrame();
    value = float32FromFrame(bigEndian, 7);
    return status;
}
void scan::printParameterStatus(uint16_t bitmask, Stream *stream)
{
    // b15
    if ((bitmask & 32768) == 32768)
        stream->println("Parameter reading out of measuring range");
    // b14
    if ((bitmask & 16384) == 16384)
        stream->println("Status of alarm Parameter is 'WARNING'");
    // b13
    if ((bitmask & 8192) == 8192)
        stream->println("Status of alarm Parameter is 'ALARM'");
    // b11
    if ((bitmask & 2048) == 2048)
        stream->println("Maintenance necessary");
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
        stream->println("General parameter error, at least one internal parameter check failed");
    // No Codes
    if (bitmask == 0)
        stream->println("Parameter is operating normally");
}
void scan::printParameterStatus(uint16_t bitmask, Stream &stream)
{printParameterStatus(bitmask, &stream);}
// This prints the data from ALL parameters as delimeter separated data.
// By default, the delimeter is a TAB (\t, 0x09), as expected by the s::can/ana::xxx software.
// This includes the parameter timestamp and status.
// NB:  You can use this to print to a file on a SD card!
void scan::printParameterData(Stream *stream, const char *dlm)
{
    // Print out the timestamp
    printSampleTime(stream, false);
    stream->print(dlm);
    // Get and print the system status
    int sysStat = getSystemStatus();
    if (sysStat == 0) {stream->print("Ok"); stream->print(dlm);}
    else {stream->print("Error"); stream->print(dlm);}
    // Get the number of parameters that are being recorded
    int nparms = getParameterCount();
    for (int i = 0; i < nparms; i++)
    {
        float value;
        getParameterValue(i+1, value);
        stream->print(value, 3);
        stream->print(dlm);
        stream->print(sysStat);
        if (i < nparms-1) stream->print(dlm);
    }
    stream->println();
}
void scan::printParameterData(Stream &stream, const char *dlm)
{printParameterData(&stream, dlm);}


// This gets spectral values from the sensor and puts them into a previously
// initialized float array.  The array must have space for 200 values!
// The actual return from the function is an integer which is a bit-mask
// describing the fingerprint status (or, well, it would be if I could figure
// out which register that value lived in).
int scan::getFingerprintData(float fpArray[], spectralSource source)
{
    int startingReg = 522 + 512*source;

    // Get the register data in 4 batches
    // This could be done using 221 modbus calls each returning 2 registers,
    // but I think it would be better to reduce the serial traffic and make
    // only 4 modbus calls each returning the maximum number of registers.

    // Registers 522-645 (+512*source)
    getRegisters(0x04, startingReg, 124);
    for (int i = 0; i < 62; i++)
    {
        fpArray[i] = float32FromFrame(bigEndian, (i*2 + 3));
    }
    // Registers 646-769 (+512*source)
    getRegisters(0x04, startingReg+124, 124);
    for (int i = 0; i < 62; i++)
    {
        fpArray[i+62] = float32FromFrame(bigEndian, (i*2 + 3));
    }
    // Registers 770-893 (+512*source)
    getRegisters(0x04, startingReg+248, 124);
    for (int i = 0; i < 62; i++)
    {
        fpArray[i+124] = float32FromFrame(bigEndian, (i*2 + 3));
    }
    // Registers 894-963 (+512*source)
    getRegisters(0x04, startingReg+248, 70);
    for (int i = 0; i < 35; i++)
    {
        fpArray[i+186] = float32FromFrame(bigEndian, (i*2 + 3));
    }

    // A total and complete WAG as to the location of the status (521)
    // My other guess is that the status is in 508 (startingReg-14)
    return bitmaskFromRegister(0x04, startingReg-1, bigEndian);
}
// This parses the parameter status bitmap and prints the resuts to the stream
// That is, pending me figuring out the right register for that data...
void scan::printFingerprintStatus(uint16_t bitmask, Stream *stream)
{printParameterStatus(bitmask, stream);}
void scan::printFingerprintStatus(uint16_t bitmask, Stream &stream)
{printParameterStatus(bitmask, &stream);}
// This prints the fingerprint data as delimeter separated data.
// By default, the delimeter is a TAB (\t, 0x09), as expected by the s::can/ana::xxx software.
// This includes the fingerprint timestamp and status
// NB:  You can use this to print to a file on a SD card!
void scan::printFingerprintData(Stream *stream, const char *dlm, spectralSource source)
{
    // Print out the timestamp
    printSampleTime(stream, false);
    stream->print(dlm);
    // Get and print the system status
    if (getSystemStatus() == 0) {stream->print("Ok"); stream->print(dlm);}
    else {stream->print("Error"); stream->print(dlm);}
    // Get the fingerprint values
    float fp[221] = {0,};
    for (int i = 0; i < 221; i++)
    {
        stream->print(fp[i], 4);
        if (i < 220) stream->print(dlm);
    }
}
void scan::printFingerprintData(Stream &stream, const char *dlm, spectralSource source)
{printFingerprintData(&stream, dlm, source);}


// This prints out a header for a "par" file in the format that the
// s::can/ana::xxx software is expecting
// The delimeter is changable, but if you use anything other than the
// default TAB (\t, 0x09) the s::can/ana::xxx software will not read it.
void scan::printHeader(Stream *stream)
{
    stream->print(getSerialNumber());
    stream->print("_");
    stream->print(getPathLength()*10, 0);
    stream->print("_0x0");
    stream->print(getModelType(), HEX);
    stream->print("_");
    stream->print(getModel());
    stream->print("_");
    stream->print(getGlobalCal());
    stream->println("	This file contains data of the current measurement.");

}
void scan::printHeader(Stream &stream){printHeader(&stream);}
void scan::printParameterHeader(Stream *stream, const char *dlm)
{
    printHeader(stream);
    stream->print("Date/Time");
    stream->print(dlm);
    stream->print("Status");
    stream->print(dlm);
    int nparms = getParameterCount();
    for (int i = 0; i < nparms; i++)
    {
        stream->print(getParameter(i+1));
        stream->print("[");
        stream->print(getUnits(i+1));
        stream->print("]");
        stream->print(getLowerLimit(i+1));
        stream->print("-");
        stream->print(getUpperLimit(i+1));
        stream->print("_1");  // This is nominally supposed to be the precision
        // of the measurement, but I don't know how to get that so I'm saying
        // "1", which seems to be the value in many of my files
        stream->print(dlm);
        stream->print(getParameter(i+1));
        stream->print("_0.0_1.0_0.0_0.0");  // These should be the calibration
        // slope and offset for a local cal, but I have no idea how to get those
        // values, so I'm just setting to the global cal.
        if (i < nparms-1) stream->print(dlm);
    }
    stream->println();
}
void scan::printParameterHeader(Stream &stream, const char *dlm)
{printParameterHeader(&stream, dlm);}

// This prints out a header for a "fp" file in the format that the
// s::can/ana::xxx software is expecting
void scan::printFingerprintHeader(Stream *stream, const char *dlm, spectralSource source)
{
    printHeader(stream);
    stream->print("Date/Time");
    stream->print(dlm);
    stream->print("Status");
    stream->print("_");
    stream->print(source);
    for (float i = 200.00; i <= 750.00; i+=2.5)
    {
        stream->print(dlm);
        stream->print(i, 2);
    }
}
void scan::printFingerprintHeader(Stream &stream, const char *dlm, spectralSource source)
{printFingerprintHeader(&stream, dlm, source);}




//----------------------------------------------------------------------------
//              FUNCTIONS TO GET AND CHANGE DEVICE CONFIGURATIONS
//----------------------------------------------------------------------------

// Functions for the communication mode
// The Communication mode is in holding register 1 (1 uint16 register)
int scan::getCommunicationMode(void)
{
    return uint16FromRegister(0x03, 1);
}
bool scan::setCommunicationMode(specCommMode mode)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = mode;
    return setRegisters(1, 1, byteToSend);
}
String scan::parseCommunicationMode(uint16_t code)
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
int scan::getBaudRate(void)
{
    return uint16FromRegister(0x03, 2);
}
bool scan::setBaudRate(specBaudRate baud)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = baud;
    return setRegisters(2, 1, byteToSend);
}
uint16_t scan::parseBaudRate(uint16_t code)
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
int scan::getParity(void)
{
    return uint16FromRegister(0x03, 3);
}
bool scan::setParity(specParity parity)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = parity;
    return setRegisters(3, 1, byteToSend);
}
String scan::parseParity(uint16_t code)
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
int scan::getprivateConfigRegister(void)
{
    return pointerFromRegister(0x03, 5);
}
int scan::getprivateConfigRegisterType(void)
{
    return pointerTypeFromRegister(0x03, 5);
}
String scan::parseRegisterType(uint16_t code)
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

// This reads the global calibration name from the private registers
// NB This is NOT documented
String scan::getGlobalCal(void)
{
    int regNum = getprivateConfigRegister();
    byte regType;
    switch (getprivateConfigRegisterType())
    {
        case 0: regType = 0x03; break;
        case 1: regType = 0x04; break;
        case 2: regType = 0x02; break;
        case 3: regType = 0x05; break;
        default: regType = 0x00; break;
    }
    // Because the address of the private config could be shifted, we have to
    // search for the actual start of the global calibration name
    int testVal;
    int actualReg;
    for (int i = 0; i < 256; i++)
    {
        actualReg = regNum + i;
        testVal = uint16FromRegister(regType, actualReg);
        if (testVal != 0) break;
    }
    return StringFromRegister(regType, actualReg, 12);
}


// Functions for the "s::canpoint" of the device
// Device Location (s::canpoint) is registers 6-11 (char[12])
// This is read only
String scan::getScanPoint(void)
{
    return StringFromRegister(0x03, 6, 12);
}
bool scan::setScanPoint(char charScanPoint[12])
{
    byte sp[12] = {0,};
    for (int i = 0; i < 12; i++) sp[i] = charScanPoint[i];
    return setRegisters(6, 6, sp);
}


// Functions for the cleaning mode configuration
// Cleaning mode is in holding register 12 (1 uint16 register)
int scan::getCleaningMode(void)
{
    return uint16FromRegister(0x03, 12);
}
bool scan::setCleaningMode(cleaningMode mode)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = mode;
    return setRegisters(12, 1, byteToSend);
}
String scan::parseCleaningMode(uint16_t code)
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
int scan::getCleaningInterval(void)
{
    return uint16FromRegister(0x03, 13);
}
bool scan::setCleaningInterval(uint16_t intervalSamples)
{
    // Using a little-endian frame to get into bytes and then reverse the order
    leFrame fram;
    fram.Int16[0] = intervalSamples;
    byte byteToSend[2];
    byteToSend[0] = fram.Byte[1];
    byteToSend[1] = fram.Byte[0];
    return setRegisters(13, 1, byteToSend);
}

// Functions for the cleaning duration in seconds
// Cleaning duration is in holding register 14 (1 uint16 register)
int scan::getCleaningDuration(void)
{
    return uint16FromRegister(0x03, 14);
}
bool scan::setCleaningDuration(uint16_t secDuration)
{
    // Using a little-endian frame to get into bytes and then reverse the order
    leFrame fram;
    fram.Int16[0] = secDuration;
    byte byteToSend[2];
    byteToSend[0] = fram.Byte[1];
    byteToSend[1] = fram.Byte[0];
    return setRegisters(14, 1, byteToSend);
}

// Functions for the waiting time between end of cleaning
// and the start of a measurement
// Cleaning wait time is in holding register 15 (1 uint16 register)
int scan::getCleaningWait(void)
{
    return uint16FromRegister(0x03, 15);
}
bool scan::setCleaningWait(uint16_t secDuration)
{
    // Using a little-endian frame to get into bytes and then reverse the order
    leFrame fram;
    fram.Int16[0] = secDuration;
    byte byteToSend[2];
    byteToSend[0] = fram.Byte[1];
    byteToSend[1] = fram.Byte[0];
    return setRegisters(15, 1, byteToSend);
}

// Functions for the current system time in seconds from Jan 1, 1970
// System time is in holding registers 16-21
// (64-bit timestamp in  in TAI64N format - in this case, ignoring the nanoseconds)
uint32_t scan::getSystemTime(void)
{
    return tai64FromRegister(0x03, 16);
}
bool scan::setSystemTime(uint32_t currentUnixTime)
{
    // Using a little-endian frame to get into bytes and then reverse the order
    leFrame fram;
    fram.Int32 = currentUnixTime;
    byte byteToSend[12] = {0,};
    byteToSend[0] = 0x40;  // It will be for the next 90 years
    byteToSend[4] = fram.Byte[3];
    byteToSend[5] = fram.Byte[2];
    byteToSend[6] = fram.Byte[1];
    byteToSend[7] = fram.Byte[0];
    return setRegisters(16, 6, byteToSend);
}

// Functions for the measurement interval in seconds (0 - as fast as possible)
// Measurement interval is in holding register 22 (1 uint16 register)
int scan::getMeasInterval(void)
{
    return uint16FromRegister(0x03, 22);
}
bool scan::setMeasInterval(uint16_t secBetween)
{
    // Using a little-endian frame to get into bytes and then reverse the order
    leFrame fram;
    fram.Int16[0] = secBetween;
    byte byteToSend[2];
    byteToSend[0] = fram.Byte[1];
    byteToSend[1] = fram.Byte[0];
    return setRegisters(22, 1, byteToSend);
}

// Functions for the logging Mode (0 = on; 1 = off)
// Logging Mode (0 = on; 1 = off) is in holding register 23 (1 uint16 register)
int scan::getLoggingMode(void)
{
    return uint16FromRegister(0x03, 23);
}
bool scan::setLoggingMode(uint8_t mode)
{
    byte byteToSend[2];
    byteToSend[0] = 0x00;
    byteToSend[1] = mode;
    return setRegisters(23, 1, byteToSend);
}
String scan::parseLoggingMode(uint16_t code)
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
int scan::getLoggingInterval(void)
{
    return uint16FromRegister(0x03, 24);
}
bool scan::setLoggingInterval(uint16_t interval)
{
    // Using a little-endian frame to get into bytes and then reverse the order
    leFrame fram;
    fram.Int16[0] = interval;
    byte byteToSend[2];
    byteToSend[0] = fram.Byte[1];
    byteToSend[1] = fram.Byte[0];
    return setRegisters(24, 1, byteToSend);
}

// Available number of logged results in datalogger since last clearing
// Available number of logged results is in holding register 25 (1 uint16 register)
int scan::getNumLoggedResults(void)
{
    return uint16FromRegister(0x03, 25);
}

// "Index device status public + private & parameter results from logger
// storage to Modbus registers.  If no stored results are available,
// results are NaN, Device status bit3 is set."
// I'm really not sure what this means...
// "Index device status" is in holding register 26 (1 uint16 register)
int scan::getIndexLogResult(void)
{
    return uint16FromRegister(0x03, 26);
}



//----------------------------------------------------------------------------
//           FUNCTIONS TO GET AND CHANGE PARAMETER CONFIGURATIONS
//----------------------------------------------------------------------------

// This returns a string with the parameter measured.
// The information on the first parameter is in register 120
// The next parameter begins 120 registers after that, up to 8 parameters
String scan::getParameter(int parmNumber)
{
    int regNumber = 120*parmNumber;
    getRegisters(0x03, regNumber, 4);
    return StringFromFrame(8);
}

// This returns a string with the measurement units.
// This begins 4 registers after the parameter name
String scan::getUnits(int parmNumber)
{
    int regNumber = 120*parmNumber + 4;
    getRegisters(0x03, regNumber, 4);
    return StringFromFrame(8);
}

// This gets the upper limit of the parameter
// This begins 8 registers after the parameter name
float scan::getUpperLimit(int parmNumber)
{
    int regNumber = 120*parmNumber + 8;
    return float32FromRegister(0x03, regNumber);
}

// This gets the lower limit of the parameter
// This begins 10 registers after the parameter name
float scan::getLowerLimit(int parmNumber)
{
    int regNumber = 120*parmNumber + 10;
    return float32FromRegister(0x03, regNumber);
}



//----------------------------------------------------------------------------
//          FUNCTIONS TO GET SETUP INFORMATION FROM THE INPUT REGISTERS
//----------------------------------------------------------------------------
// This information can be read, but cannot be changed

// Get the version of the modbus mapping protocol
// The modbus version is in input register 0
float scan::getModbusVersion(void)
{
    getRegisters(0x04, 0, 1);
    leFrame fram = leFrameFromRegister(2);
    float mjv = fram.Byte[1];
    float mnv = fram.Byte[0];
    mnv = mnv/100;
    float version = mjv + mnv;
    return version;
}

// This returns a byte with the model type
uint16_t scan::getModelType(void)
{
    return uint16FromRegister(0x04, 2);
}

// This returns a pretty string with the model information
String scan::getModel(void)
{
    getRegisters(0x04, 3, 10);
    return StringFromFrame(20);
}

// This gets the instrument serial number as a String
String scan::getSerialNumber(void)
{
    getRegisters(0x04, 13, 4);
    return StringFromFrame(8);
}

// This gets the hardware version of the sensor
float scan::getHWVersion(void)
{
    getRegisters(0x04, 17, 2);
    String _model = StringFromFrame(4);
    float mjv = _model.substring(0,2).toFloat();
    float mnv = (_model.substring(2,4).toFloat())/100;
    float version = mjv + mnv;
    return version;
}

// This gets the software version of the sensor
float scan::getSWVersion(void)
{
    getRegisters(0x04, 19, 2);
    String _model = StringFromFrame(4);
    float mjv = _model.substring(0,2).toFloat();
    float mnv = (_model.substring(2,4).toFloat())/100;
    float version = mjv + mnv;
    return version;
}

// This gets the number of times the spec has been rebooted
// (Device rebooter counter)
int scan::getHWStarts(void)
{
    return uint16FromRegister(0x04, 21);
}

// This gets the number of parameters the spectro::lyzer is set to measure
int scan::getParameterCount(void)
{
    return uint16FromRegister(0x04, 22);
}

// This gets the datatype of the parameters and parameter limits
// This is a check for compatibility
int scan::getParameterType(void)
{
    return uint16FromRegister(0x04, 23);
}
String scan::parseParameterType(uint16_t code)
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
int scan::getParameterScale(void)
{
    return uint16FromRegister(0x04, 24);
}

// This returns the spectral path length in mm
// NB This is not documented - I'm guessing based on register VALUES
float scan::getPathLength(void)
{
    int path = uint16FromRegister(0x04, 520);
    float pathmm = path/10;  // Convert to mm
    return pathmm;
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
//
void scan::calculateCRC(byte modbusFrame[], int frameLength)
{
    // Reset the CRC frame
    crcFrame[2] = {0,};
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
    crcFrame[0] = crcLow;
    crcFrame[1] = crcHigh;
}
void scan::insertCRC(byte modbusFrame[], int frameLength)
{
    // Calculate the CRC
    calculateCRC(modbusFrame, frameLength);

    // Append the bytes to the end of the frame
    modbusFrame[frameLength - 2] = crcFrame[0];
    modbusFrame[frameLength - 1] = crcFrame[1];
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
    _debugStream->print("Raw Request >>> ");
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
        _debugStream->print(" bytes) <<< ");
        printFrameHex(responseBuffer, bytesRead);

        // Verify that the values match with the commands
        if (responseBuffer[0] != _slaveID)
        {
            _debugStream->println("Response is not from the correct modbus slave!");
            return 0;
        }

        // Verify that the CRC is correct
        calculateCRC(responseBuffer, bytesRead);
        if (crcFrame[0] != responseBuffer[bytesRead-2] || crcFrame[1] != responseBuffer[bytesRead-1])
        {
            _debugStream->println("CRC of response is not correct!");
            return 0;
        }

        // Check for exception response
        // An execption response sets the highest bit of the function code in the response.
        if ((responseBuffer[1] & 0b10000000) ==  0b10000000)
        {
            _debugStream->print("Exception:  ");
            if (responseBuffer[2] == 0x01)_debugStream->println("Illegal Function!");
            if (responseBuffer[2] == 0x02)_debugStream->println("Illegal Data Address!");
            if (responseBuffer[2] == 0x03)_debugStream->println("Illegal Data Value!");
            if (responseBuffer[2] == 0x04)_debugStream->println("Slave Device Failure!");
            if (responseBuffer[2] == 0x05)_debugStream->println("Acknowledge...");
            if (responseBuffer[2] == 0x06)_debugStream->println("Slave Device Busy!");
            if (responseBuffer[2] == 0x07)_debugStream->println("Negative Acknowledge!");
            if (responseBuffer[2] == 0x08)_debugStream->println("Memory Parity Error!");
            if (responseBuffer[2] == 0x0A)_debugStream->println("Gateway Path Unavailable!");
            if (responseBuffer[2] == 0x0B)_debugStream->println("Gateway Target Device Failed to Respond!");
            return 0;
        }

        // If everything passes, return the number of bytes
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
    leFrame fram = {0,};
    fram.Int16[0] = startRegister;
    command[2] = fram.Byte[1];
    command[3] = fram.Byte[0];

    // Put in the number of registers
    fram.Int16[1] = numRegisters;
    command[4] = fram.Byte[3];
    command[5] = fram.Byte[2];

    // The size of the returned frame should be:
    // # Registers X 2 bytes/register + 5 bytes of modbus frame

    // Try up to 10 times to get the right results
    int tries = 0;
    int16_t respSize = 0;
    while ((respSize != (numRegisters*2 + 5) && tries < 10))
    {
        // Send out the command (this adds the CRC)
        respSize = sendCommand(command, 8);
        tries++;

        delay(25);
    }

    if (respSize == (numRegisters*2 + 5))
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
    leFrame fram = {0,};
    fram.Int16[0] = startRegister;
    command[2] = fram.Byte[1];
    command[3] = fram.Byte[0];

    // Put in the register values
    if (numRegisters > 1)
    {
        // Put in the number of registers
        fram.Int16[1] = numRegisters;
        command[4] = fram.Byte[3];
        command[5] = fram.Byte[2];
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

    // Try up to 10 times to get the right results
    int tries = 0;
    int16_t respSize = 0;
    bool success = false;
    while ((!success && tries < 10))
    {
        // Send out the command (this adds the CRC)
        respSize = sendCommand(command, commandLength);
        // The structure of the response for 0x10 should be:
        // {slaveID, fxnCode, Address of 1st register, # Registers, CRC}
        if (numRegisters > 1 && respSize == 8 && int16FromFrame(bigEndian, 4) == numRegisters)
            success = true;
        // The structure of the response for 0x06 should be:
        // {slaveID, fxnCode, Address of 1st register, Value written, CRC}
        if (numRegisters == 1 && respSize == 8 && responseBuffer[4] == value[0]
            && responseBuffer[5] == value[1])
            success = true;
        tries++;

        delay(25);
    }

    return success;
};


// This slices one array out of another
// Used for slicing one or more registers out of a returned modbus frame
void scan::sliceArray(byte inputArray[], byte outputArray[],
                int start_index, int numBytes, bool reverseOrder)
{

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
}


// This converts data in a register into a little-endian frame
// little-endian frames are needed because all Arduino processors are little-endian
leFrame scan::leFrameFromRegister(int varLength,
                                  endianness endian,
                                  int start_index,
                                  byte indata[])
{
    // Set up a temporary output frame
    byte outFrame[varLength] = {0,};
    // Slice data from the full response frame into the temporary output frame
    if (endian == bigEndian)
        sliceArray(indata, outFrame, start_index, varLength, true);
    else sliceArray(indata, outFrame, start_index, varLength, false);
    // Put it into a little-endian frame (the format of all arduino processors)
    leFrame fram = {0,};
    memcpy(fram.Byte, outFrame, varLength);
    // Return the little-endian frame
    return fram;
}


// These functions return a variety of data from an input frame
uint16_t scan::bitmaskFromFrame(endianness endian, int start_index, byte indata[])
{
    int varLength = 2;
    return leFrameFromRegister(varLength, endian, start_index, indata).uInt16[0];
}

uint16_t scan::uint16FromFrame(endianness endian, int start_index, byte indata[])
{
   int varLength = 2;
   return leFrameFromRegister(varLength, endian, start_index, indata).uInt16[0];
}

int16_t scan::int16FromFrame(endianness endian, int start_index, byte indata[])
{
    int varLength = 2;
    return leFrameFromRegister(varLength, endian, start_index, indata).Int16[0];
}

uint16_t scan::pointerFromFrame(endianness endian, int start_index, byte indata[])
{
    leFrame fram;
    if (endian == bigEndian)
    {
        fram.Byte[0] = indata[start_index + 1]>>2;  // Shift the lower address bit DOWN two
        fram.Byte[1] = indata[start_index];
    }
    else
    {
        fram.Byte[0] = indata[start_index]>>2;  // Shift the lower address bit DOWN two
        fram.Byte[1] = indata[start_index + 1];
    }
    return fram.Int16[0];
}

int8_t scan::pointerTypeFromFrame(endianness endian, int start_index, byte indata[])
{
    uint8_t pointerRegType;
    // Mask to get the last two bits, which are the type
    if (endian == bigEndian) pointerRegType = indata[start_index + 1] & 3;
    else pointerRegType = indata[start_index] & 3;
    return pointerRegType;
}

float scan::float32FromFrame(endianness endian, int start_index, byte indata[])
{
    int varLength = 4;
    return leFrameFromRegister(varLength, endian, start_index, indata).Float;
}

uint32_t scan::uint32FromFrame(endianness endian, int start_index, byte indata[])
{
    int varLength = 4;
    return leFrameFromRegister(varLength, endian, start_index, indata).uInt32;
}

int32_t scan::int32FromFrame(endianness endian, int start_index, byte indata[])
{
    int varLength = 4;
    return leFrameFromRegister(varLength, endian, start_index, indata).Int32;
}

uint32_t scan::tai64FromFrame(int start_index, byte indata[])
{
    // This is a 64 bit (4 register) data type BUT:
    // The first 32 bits (two registers) will be 0x4000 0000 until the year 2106;
    // I'm ignoring it for the next 90 years to avoid using 64 bit math
    // The next 32 bits (two registers) have the actual seconds past Jan 1, 1970
    // In the case of TAI64N data there will be an additional 32 bits (two
    // registers) of data representing the nanosecond portion of the time.  This
    // data type does *not* deal with that.
    // Per the TAI64 standard, this value is always big-endian
    // https://www.tai64.com/
    int varLength = 4;
    return leFrameFromRegister(varLength, bigEndian, start_index+4, indata).uInt32;
}

String scan::StringFromFrame(int charLength, int start_index, byte indata[])
{
    char charString[charLength+1] = {0,};
    int j = 0;
    for (int i = start_index; i < start_index + charLength; i++)
    {
        charString[j] = responseBuffer[i];  // converts from "byte" type to "char" type
        j++;
    }
    String string = String(charString);
    string.trim();
    return string;
}

void scan::charFromFrame(char outChar[], int charLength, int start_index, byte indata[])
{
    int j = 0;
    for (int i = start_index; i < start_index + charLength; i++)
    {
        outChar[j] = responseBuffer[i];  // converts from "byte" type to "char" type
        j++;
    }
}


// These functions return a variety of data from a data register
uint16_t scan::bitmaskFromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, 1);
    return bitmaskFromFrame(endian);
}
uint16_t scan::uint16FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, 1);
    return uint16FromFrame(endian);
}
int16_t scan::int16FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, 1);
    return int16FromFrame(endian);
}
uint16_t scan::pointerFromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, 1);
    return pointerFromFrame(endian);
}
int8_t scan::pointerTypeFromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, 1);
    return pointerTypeFromFrame(endian);
}
float scan::float32FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, 2);
    return float32FromFrame(endian);
}
uint32_t scan::uint32FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, 2);
    return uint32FromFrame(endian);
}
int32_t scan::int32FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, 2);
    return int32FromFrame(endian);
}
uint32_t scan::tai64FromRegister(byte regType, int regNum)
{
    getRegisters(regType, regNum, 4);
    return tai64FromFrame();
}
String scan::StringFromRegister(byte regType, int regNum, int charLength)
{
    getRegisters(regType, regNum, charLength/2);
    return StringFromFrame(charLength);
}
void scan::charFromRegister(byte regType, int regNum, char outChar[], int charLength)
{
    getRegisters(regType, regNum, charLength/2);
    charFromFrame(outChar, charLength);
}
