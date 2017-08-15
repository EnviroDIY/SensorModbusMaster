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
{return false;}

// This just returns the given slave ID.  If you don't know your slave ID, you
// must find it some other way
byte scan::getSlaveID(void)
{return _slaveID;}

// This sets a new modbus slave ID
bool scan::setSlaveID(byte newSlaveID)
{return false;}

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

//----------------------------------------------------------------------------
//                           PRIVATE HELPER FUNCTIONS
//----------------------------------------------------------------------------

// This functions return the float from a 4-byte small-endian array beginning
// at a specific index of another array.
float scan::floatFromFrame(byte indata[], int stindex)
{
    SeFrame Sefram;
    Sefram.Byte[0] = indata[stindex];
    Sefram.Byte[1] = indata[stindex + 1];
    Sefram.Byte[2] = indata[stindex + 2];
    Sefram.Byte[3] = indata[stindex + 3];
    return Sefram.Float;
}
// This functions inserts a float as a 4-byte small endian array into another
// array beginning at the specified index.
void scan::floatIntoFrame(byte indata[], int stindex, float value)
{
    SeFrame Sefram;
    Sefram.Float = value;
    indata[stindex] = Sefram.Byte[0];
    indata[stindex + 1] = Sefram.Byte[1];
    indata[stindex + 2] = Sefram.Byte[2];
    indata[stindex + 3] = Sefram.Byte[3];
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
        int bytesRead = _stream->readBytes(responseBuffer, 20);
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
