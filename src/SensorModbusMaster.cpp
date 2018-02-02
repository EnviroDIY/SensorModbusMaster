/*
 *SensorModbusMaster.cpp
*/

#include "SensorModbusMaster.h"

// initialize the response buffer
byte modbusMaster::responseBuffer[RESPONSE_BUFFER_SIZE] = {0x00,};
byte modbusMaster::crcFrame[2] = {0x00,};


//----------------------------------------------------------------------------
//                           HIGHEST LEVEL FUNCTIONS
//----------------------------------------------------------------------------

// This function sets up the communication
// It should be run during the arduino "setup" function.
// The "stream" device must be initialized and begun prior to running this.
bool modbusMaster::begin(byte modbusSlaveID, Stream *stream, int enablePin)
{
    // Give values to variables;
    _slaveID = modbusSlaveID;
    _stream = stream;
    _enablePin = enablePin;

    // Set pin mode for the enable pin
    if (_enablePin > 0) pinMode(_enablePin, OUTPUT);
    recieverEnable();

    _stream->setTimeout(modbusFrameTimeout);

    return true;
}
bool modbusMaster::begin(byte modbusSlaveID, Stream &stream, int enablePin)
{return begin(modbusSlaveID, &stream, enablePin);}


// These functions return a variety of data from a data register
uint16_t modbusMaster::uint16FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, UINT16_SIZE/2);
    return uint16FromFrame(endian);
}
int16_t modbusMaster::int16FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, INT16_SIZE/2);
    return int16FromFrame(endian);
}
float modbusMaster::float32FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, FLOAT32_SIZE/2);
    return float32FromFrame(endian);
}
uint32_t modbusMaster::uint32FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, UINT32_SIZE/2);
    return uint32FromFrame(endian);
}
int32_t modbusMaster::int32FromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, INT32_SIZE/2);
    return int32FromFrame(endian);
}
uint32_t modbusMaster::TAI64FromRegister(byte regType, int regNum)
{
    getRegisters(regType, regNum, TAI64_SIZE/2);
    return TAI64FromFrame();
}
uint32_t modbusMaster::TAI64NFromRegister(byte regType, int regNum, uint32_t &nanoseconds)
{
    getRegisters(regType, regNum, TAI64N_SIZE/2);
    return TAI64NFromFrame(nanoseconds);
}
uint32_t modbusMaster::TAI64NAFromRegister(byte regType, int regNum, uint32_t &nanoseconds, uint32_t &attoseconds)
{
    getRegisters(regType, regNum, TAI64NA_SIZE/2);
    return TAI64NAFromFrame(nanoseconds, attoseconds);
}
byte modbusMaster::byteFromRegister(byte regType, int regNum, int byteNum)
{
    getRegisters(regType, regNum, 1);
    if (byteNum == 1) return byteFromFrame();
    else return byteFromFrame(4);
}
uint16_t modbusMaster::pointerFromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, POINTER_SIZE/2);
    return pointerFromFrame(endian);
}
int8_t modbusMaster::pointerTypeFromRegister(byte regType, int regNum, endianness endian)
{
    getRegisters(regType, regNum, POINTER_SIZE/2);
    return pointerTypeFromFrame(endian);
}
String modbusMaster::StringFromRegister(byte regType, int regNum, int charLength)
{
    getRegisters(regType, regNum, charLength/2);
    return StringFromFrame(charLength);
}
void modbusMaster::charFromRegister(byte regType, int regNum, char outChar[], int charLength)
{
    getRegisters(regType, regNum, charLength/2);
    charFromFrame(outChar, charLength);
}


// These set data in registers to a variety of data types
// For data types that can be only one register long, the modbus command to pre-set
// a single register (0x06) will be used by default.  To force the use of the pre-set
// multiple registers command, set the boolean input for forceMultiple to true.
bool modbusMaster::uint16ToRegister(int regNum, uint16_t value, endianness endian, bool forceMultiple)
{
    byte inputData[UINT16_SIZE] = {0x00,};
    uint16ToFrame(value, endian, inputData);
    return setRegisters(regNum, UINT16_SIZE/2, inputData, forceMultiple);
}
bool modbusMaster::int16ToRegister(int regNum, int16_t value, endianness endian, bool forceMultiple)
{
    byte inputData[INT16_SIZE] = {0x00,};
    int16ToFrame(value, endian, inputData);
    return setRegisters(regNum, INT16_SIZE/2, inputData, forceMultiple);
}
bool modbusMaster::float32ToRegister(int regNum, float value, endianness endian)
{
    byte inputData[FLOAT32_SIZE] = {0x00,};
    float32ToFrame(value, endian, inputData);
    return setRegisters(regNum, FLOAT32_SIZE/2, inputData);
}
bool modbusMaster::uint32ToRegister(int regNum, uint32_t value, endianness endian)
{
    byte inputData[UINT32_SIZE] = {0x00,};
    uint32ToFrame(value, endian, inputData);
    return setRegisters(regNum, UINT32_SIZE/2, inputData);
}
bool modbusMaster::int32ToRegister(int regNum, int32_t value, endianness endian)
{
    byte inputData[INT32_SIZE] = {0x00,};
    int32ToFrame(value, endian, inputData);
    return setRegisters(regNum, INT32_SIZE/2, inputData);
}
bool modbusMaster::TAI64ToRegister(int regNum, uint32_t seconds)
{
    byte inputData[TAI64_SIZE] = {0x00,};
    TAI64ToFrame(seconds, inputData);
    return setRegisters(regNum, TAI64_SIZE/2, inputData);
}
bool modbusMaster::TAI64NToRegister(int regNum, uint32_t seconds, uint32_t nanoseconds)
{
    byte inputData[TAI64N_SIZE] = {0x00,};
    TAI64NToFrame(seconds, nanoseconds, inputData);
    return setRegisters(regNum, TAI64N_SIZE/2, inputData);
}
bool modbusMaster::TAI64NAToRegister(int regNum, uint32_t seconds, uint32_t nanoseconds, uint32_t attoseconds)
{
    byte inputData[TAI64NA_SIZE] = {0x00,};
    TAI64NAToFrame(seconds, nanoseconds, attoseconds, inputData);
    return setRegisters(regNum, TAI64NA_SIZE/2, inputData);
}
bool modbusMaster::byteToRegister(int regNum, int byteNum, byte value, bool forceMultiple)
{
    byte inputData[2] = {0x00,};
    byteToFrame(value, byteNum, inputData);
    return setRegisters(regNum, 1, inputData, forceMultiple);
}
bool modbusMaster::pointerToRegister(int regNum, uint16_t value, pointerType point, endianness endian, bool forceMultiple)
{
    byte inputData[UINT16_SIZE] = {0x00,};
    pointerToFrame(value, point, endian, inputData);
    return setRegisters(regNum, UINT16_SIZE/2, inputData, forceMultiple);
}
bool modbusMaster::StringToRegister(int regNum, String value, bool forceMultiple)
{
    int charLength = value.length();
    byte inputData[charLength];
    StringToFrame(value, inputData);
    return setRegisters(regNum, charLength/2, inputData, forceMultiple);
}
bool modbusMaster::charToRegister(int regNum, char inChar[], int charLength, bool forceMultiple)
{
    byte inputData[charLength];
    charToFrame(inChar, charLength, inputData);
    return setRegisters(regNum, charLength/2, inputData, forceMultiple);
}

//----------------------------------------------------------------------------
//                           MID LEVEL FUNCTIONS
//----------------------------------------------------------------------------


// These functions return a variety of data from an input modbus RTU frame.
// Currently, the only "frame" available is the response buffer.
uint16_t modbusMaster::uint16FromFrame(endianness endian, int start_index)
{return leFrameFromFrame(UINT16_SIZE, endian, start_index).uInt16[0];}

int16_t modbusMaster::int16FromFrame(endianness endian, int start_index)
{return leFrameFromFrame(INT16_SIZE, endian, start_index).Int16[0];}

float modbusMaster::float32FromFrame(endianness endian, int start_index)
{return leFrameFromFrame(FLOAT32_SIZE, endian, start_index).Float32;}

uint32_t modbusMaster::uint32FromFrame(endianness endian, int start_index)
{return leFrameFromFrame(UINT32_SIZE, endian, start_index).uInt32;}

int32_t modbusMaster::int32FromFrame(endianness endian, int start_index)
{return leFrameFromFrame(INT32_SIZE, endian, start_index).Int32;}

uint32_t modbusMaster::TAI64FromFrame(int start_index)
{return leFrameFromFrame(4, bigEndian, start_index + 4).uInt32;}
uint32_t modbusMaster::TAI64NFromFrame(uint32_t &nanoseconds, int start_index)
{
    nanoseconds = leFrameFromFrame(4, bigEndian, start_index + 8).uInt32;
    return leFrameFromFrame(4, bigEndian, start_index + 4).uInt32;
}
uint32_t modbusMaster::TAI64NAFromFrame(uint32_t &nanoseconds, uint32_t &attoseconds, int start_index)
{
    attoseconds = leFrameFromFrame(4, bigEndian, start_index + 12).uInt32;
    nanoseconds = leFrameFromFrame(4, bigEndian, start_index + 8).uInt32;
    return leFrameFromFrame(4, bigEndian, start_index + 4).uInt32;
}

byte modbusMaster::byteFromFrame(int start_index)
{return responseBuffer[start_index];}

uint16_t modbusMaster::pointerFromFrame(endianness endian, int start_index)
{
    leFrame fram;
    if (endian == bigEndian)
    {
        fram.Byte[0] = responseBuffer[start_index + 1]>>2;  // Shift the lower address bit DOWN two
        fram.Byte[1] = responseBuffer[start_index];
    }
    else
    {
        fram.Byte[0] = responseBuffer[start_index]>>2;  // Shift the lower address bit DOWN two
        fram.Byte[1] = responseBuffer[start_index + 1];
    }
    return fram.Int16[0];
}

int8_t modbusMaster::pointerTypeFromFrame(endianness endian, int start_index)
{
    uint8_t pointerRegType;
    // Mask to get the last two bits, which are the type
    if (endian == bigEndian) pointerRegType = responseBuffer[start_index + 1] & 3;
    else pointerRegType = responseBuffer[start_index] & 3;
    return pointerRegType;
}

String modbusMaster::StringFromFrame(int charLength, int start_index)
{
    char charString[charLength+1];
    int j = 0;
    for (int i = start_index; i < start_index + charLength; i++)
    {
        // check that it's a printable character
        if (responseBuffer[i] >= 0x20 && responseBuffer[i] <= 0x7E)
        {
            charString[j] = responseBuffer[i];  // converts from "byte" type to "char" type
            j++;
        }
    }
    if (j < charLength+1) for (int i = j; i < + charLength+1; i++) charString[j] = '\0';
    String string = String(charString);
    string.trim();
    return string;
}

void modbusMaster::charFromFrame(char outChar[], int charLength, int start_index)
{
    int j = 0;
    for (int i = start_index; i < start_index + charLength; i++)
    {
        // check that it's a printable character
        if (responseBuffer[i] >= 0x20 && responseBuffer[i] <= 0x7E)
        {
            outChar[j] = responseBuffer[i];  // converts from "byte" type to "char" type
            j++;
        }
    }
    if (j < charLength) for (int i = j; i < + charLength; i++) outChar[j] = '\0';
}

// These insert values into a longer modbus data frame.
void modbusMaster::uint16ToFrame(uint16_t value, endianness endian, byte modbusFrame[], int start_index)
{
    leFrame fram;
    fram.uInt16[0] = value;
    int end_index = (start_index + UINT16_SIZE) - 1;
    for (int i = 0; i < UINT16_SIZE; i++)
    {
        if (endian == bigEndian)
            modbusFrame[end_index - i] = fram.Byte[i];
        else modbusFrame[start_index + i] = fram.Byte[i];
    }
}
void modbusMaster::int16ToFrame(int16_t value, endianness endian, byte modbusFrame[], int start_index)
{
    leFrame fram;
    fram.Int16[0] = value;
    int end_index = (start_index + INT16_SIZE) - 1;
    for (int i = 0; i < INT16_SIZE; i++)
    {
        if (endian == bigEndian)
            modbusFrame[end_index - i] = fram.Byte[i];
        else modbusFrame[start_index + i] = fram.Byte[i];
    }
}
void modbusMaster::float32ToFrame(float value, endianness endian, byte modbusFrame[], int start_index)
{
    leFrame fram;
    fram.Float32 = value;
    int end_index = (start_index + FLOAT32_SIZE) - 1;
    for (int i = 0; i < FLOAT32_SIZE; i++)
    {
        if (endian == bigEndian)
            modbusFrame[end_index - i] = fram.Byte[i];
        else modbusFrame[start_index + i] = fram.Byte[i];
    }
}
void modbusMaster::uint32ToFrame(uint32_t value, endianness endian, byte modbusFrame[], int start_index)
{
    leFrame fram;
    fram.uInt32 = value;
    int end_index = (start_index + UINT32_SIZE) - 1;
    for (int i = 0; i < UINT32_SIZE; i++)
    {
        if (endian == bigEndian)
            modbusFrame[end_index - i] = fram.Byte[i];
        else modbusFrame[start_index + i] = fram.Byte[i];
    }
}
void modbusMaster::int32ToFrame(int32_t value, endianness endian, byte modbusFrame[], int start_index)
{
    leFrame fram;
    fram.Int32 = value;
    int end_index = (start_index + INT32_SIZE) - 1;
    for (int i = 0; i < INT32_SIZE; i++)
    {
        if (endian == bigEndian)
            modbusFrame[end_index - i] = fram.Byte[i];
        else modbusFrame[start_index + i] = fram.Byte[i];
    }
}
void modbusMaster::TAI64ToFrame(uint32_t seconds, byte modbusFrame[], int start_index)
{
    // The first 4 bytes of the 64-bit value will be 0x400000 until the year 2106
    modbusFrame[start_index] = 0x40;

    leFrame fram;
    fram.Int32 = seconds;
    // The 32-bit seconds start 4 bytes after the 64-bit value starts
    int byte_offset_sec = 4;
    int end_index_sec = start_index + byte_offset_sec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++)
    {
        modbusFrame[end_index_sec - i] = fram.Byte[i];
    }
}
void modbusMaster::TAI64NToFrame(uint32_t seconds, uint32_t nanoseconds, byte modbusFrame[], int start_index)
{
    // The first 4 bytes of the 64-bit value will be 0x400000 until the year 2106
    modbusFrame[start_index] = 0x40;

    leFrame fram;
    fram.Int32 = seconds;
    // The 32-bit seconds start 4 bytes after the 64-bit value starts
    int byte_offset_sec = 4;
    int end_index_sec = start_index + byte_offset_sec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++)
    {
        modbusFrame[end_index_sec - i] = fram.Byte[i];
    }

    fram.Int32 = nanoseconds;
    // The 32-bit nanoseconds start 8 bytes after the 64-bit value starts
    int byte_offset_nanosec = 8;
    int end_index_nanosec = start_index + byte_offset_nanosec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++)
    {
        modbusFrame[end_index_nanosec - i] = fram.Byte[i];
    }
}
void modbusMaster::TAI64NAToFrame(uint32_t seconds, uint32_t nanoseconds, uint32_t attoseconds, byte modbusFrame[], int start_index)
{
    // The first 4 bytes of the 64-bit value will be 0x400000 until the year 2106
    modbusFrame[start_index] = 0x40;

    leFrame fram;
    fram.Int32 = seconds;
    // The 32-bit seconds start 4 bytes after the 64-bit value starts
    int byte_offset_sec = 4;
    int end_index_sec = start_index + byte_offset_sec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++)
    {
        modbusFrame[end_index_sec - i] = fram.Byte[i];
    }

    fram.Int32 = nanoseconds;
    // The 32-bit nanoseconds start 8 bytes after the 64-bit value starts
    int byte_offset_nanosec = 8;
    int end_index_nanosec = start_index + byte_offset_nanosec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++)
    {
        modbusFrame[end_index_nanosec - i] = fram.Byte[i];
    }

    // The 32-bit attoseconds start 12 bytes after the 64-bit value starts
    int byte_offset_attosec = 12;
    int end_index_attosec = start_index + byte_offset_attosec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++)
    {
        modbusFrame[end_index_attosec - i] = fram.Byte[i];
    }
}
void modbusMaster::byteToFrame(byte value, int byteNum, byte modbusFrame[], int start_index)
{
    if (byteNum == 1) modbusFrame[start_index] = value;
    else  modbusFrame[start_index+1] = value;
}
void modbusMaster::pointerToFrame(uint16_t value, pointerType point, endianness endian, byte modbusFrame[], int start_index)
{
    leFrame fram;
    fram.uInt16[0] = value;
    if (endian == bigEndian)
    {
        modbusFrame[start_index + 1] = fram.Byte[0]<<2;  // Shift the lower address bit UP two
        modbusFrame[start_index + 1] |= point;
        modbusFrame[start_index] = fram.Byte[1];
    }
    else
    {
        modbusFrame[start_index] = fram.Byte[0]<<2;  // Shift the lower address bit UP two
        modbusFrame[start_index] |= point;
        modbusFrame[start_index + 1] = fram.Byte[1];
    }
}
void modbusMaster::StringToFrame(String value, byte modbusFrame[], int start_index)
{
    int charLength = value.length();
    char charString[charLength];
    value.toCharArray(charString, charLength);
    int j = 0;
    for (int i = 0; i < charLength; i++)
    {
        modbusFrame[start_index + j] = charString[i];  // converts from "char" type to "byte" type
        j++;
    }
}
void modbusMaster::charToFrame(char inChar[], int charLength, byte modbusFrame[], int start_index)
{
    int j = 0;
    for (int i = 0; i < charLength; i++)
    {
        modbusFrame[start_index + j] = inChar[i];  // converts from "char" type to "byte" type
        j++;
    }
}


//----------------------------------------------------------------------------
//                           LOW LEVEL FUNCTIONS
//----------------------------------------------------------------------------

// This gets data from either a holding or input register
// For a holding register readCommand = 0x03
// For an input register readCommand = 0x04
bool modbusMaster::getRegisters(byte readCommand, int16_t startRegister, int16_t numRegisters)
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
    // # Registers X 2 bytes/register + 5 bytes of modbus RTU frame

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
// The boolean switch to "forceMultiple" will force the command 0x10 (16,
// preset multiple registers) instead of using 0x06 for a single register
bool modbusMaster::setRegisters(int16_t startRegister, int16_t numRegisters,
                                byte value[], bool forceMultiple)
{
    // figure out how long the command will be
    int commandLength;
    if (numRegisters > 1 or forceMultiple) commandLength = numRegisters*2 + 9;
    else commandLength = numRegisters*2 + 6;

    // Create an array for the command
    byte command[commandLength];

    // Put in the slave id and the command
    command[0] = _slaveID;
    if (numRegisters > 1 or forceMultiple) command[1] = 0x10;
    else command[1] = 0x06;

    // Put in the starting register
    leFrame fram = {0,};
    fram.Int16[0] = startRegister;
    command[2] = fram.Byte[1];
    command[3] = fram.Byte[0];

    // Put in the register values
    // For multiple registers, need to add in how many registers and how many bytes
    if (numRegisters > 1 or forceMultiple)
    {
        // Put in the number of registers
        fram.Int16[1] = numRegisters;
        command[4] = fram.Byte[3];
        command[5] = fram.Byte[2];
        // Put in the number of bytes to write
        command[6] = numRegisters*2;
        // Put in the data, allowing 7 extra spaces for the modbus frame structure
        for (int i = 7; i < numRegisters*2 + 7; i++) command[i] = value[i-7];
    }
    // For a single register, only need the data itself
    else
    {
        // Put in the data, allowing 4 extra spaces for the modbus frame structure
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
        if ((numRegisters > 1 or forceMultiple) && respSize == 8 && int16FromFrame(bigEndian, 4) == numRegisters)
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


//----------------------------------------------------------------------------
//                           LOWEST LEVEL FUNCTION
//----------------------------------------------------------------------------

// This sends a command to the sensor bus and listens for a response
int modbusMaster::sendCommand(byte command[], int commandLength)
{
    // Empty the response buffer
    for (int i = 0; i < RESPONSE_BUFFER_SIZE; i++)
        modbusMaster::responseBuffer[i] = 0x00;

    // Add the CRC to the frame
    insertCRC(command, commandLength);

    // Send out the command
    driverEnable();
    emptySerialBuffer(_stream);  // Clear any junk before sending command
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
        emptySerialBuffer(_stream);

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
    else
    {
        _debugStream->println("No response received.");
        return 0;
    }
}



//----------------------------------------------------------------------------
//                           PRIVATE HELPER FUNCTIONS
//----------------------------------------------------------------------------

// This flips the device/receive enable to DRIVER so the arduino can send text
void modbusMaster::driverEnable(void)
{
    if (_enablePin > 0)
    {
        digitalWrite(_enablePin, HIGH);
        _debugStream->println("Driver/Master Enabled");
        delay(8);
    }
}

// This flips the device/receive enable to RECIEVER so the sensor can send text
void modbusMaster::recieverEnable(void)
{
    if (_enablePin > 0)
    {
        digitalWrite(_enablePin, LOW);
        _debugStream->println("Receiver/Slave Enabled");
        delay(8);
    }
}

// This empties the serial buffer
void modbusMaster::emptySerialBuffer(Stream *stream)
{
    while (stream->available() > 0)
    {
        stream->read();
        delay(1);
    }
}

// Just a function to pretty-print the modbus hex frames
// This is purely for debugging
void modbusMaster::printFrameHex(byte modbusFrame[], int frameLength)
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
void modbusMaster::calculateCRC(byte modbusFrame[], int frameLength)
{
    // Reset the CRC frame
    modbusMaster::crcFrame[0] = {0x00};
    modbusMaster::crcFrame[1] = {0x00};

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
void modbusMaster::insertCRC(byte modbusFrame[], int frameLength)
{
    // Calculate the CRC
    calculateCRC(modbusFrame, frameLength);

    // Append the bytes to the end of the frame
    modbusFrame[frameLength - 2] = crcFrame[0];
    modbusFrame[frameLength - 1] = crcFrame[1];
}

// This slices one array out of another
// Used for slicing one or more registers out of a returned modbus RTU frame
void modbusMaster::sliceArray(byte inputArray[], byte outputArray[],
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


// This converts data in a modbus RTU frame into a little-endian data frame
// little-endian data frames are needed because all Arduino processors are little-endian
leFrame modbusMaster::leFrameFromFrame(int varBytes,
                                  endianness endian,
                                  int start_index)
{
    // Set up a temporary output frame
    byte outFrame[varBytes];
    // Slice data from the full response frame into the temporary output frame
    if (endian == bigEndian)
        sliceArray(responseBuffer, outFrame, start_index, varBytes, true);
    else sliceArray(responseBuffer, outFrame, start_index, varBytes, false);
    // Put it into a little-endian frame (the format of all arduino processors)
    leFrame fram = {0,};
    memcpy(fram.Byte, outFrame, varBytes);
    // Return the little-endian frame
    return fram;
}
