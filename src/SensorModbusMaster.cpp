/**
 * @file SensorModbusMaster.cpp
 * @copyright Stroud Water Research Center
 * Part of the EnviroDIY SensorModbusMaster library for Arduino.
 * @license This library is published under the BSD-3 license.
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Contains the modbusMaster class definitions.
 */

#include "SensorModbusMaster.h"

// initialize the response buffer
byte modbusMaster::responseBuffer[RESPONSE_BUFFER_SIZE] = {
    0x00,
};
byte modbusMaster::crcFrame[2] = {
    0x00,
};


//----------------------------------------------------------------------------
//                           HIGHEST LEVEL FUNCTIONS
//----------------------------------------------------------------------------

// This function sets up the communication
// It should be run during the arduino "setup" function.
// The "stream" device must be initialized and begun prior to running this.
bool modbusMaster::begin(byte modbusSlaveID, Stream* stream, int8_t enablePin) {
    // Give values to variables;
    _slaveID   = modbusSlaveID;
    _stream    = stream;
    _enablePin = enablePin;

    // Set pin mode for the enable pin
    if (_enablePin >= 0) { pinMode(_enablePin, OUTPUT); }
    receiverEnable();

    _stream->setTimeout(modbusFrameTimeout);

    return true;
}
bool modbusMaster::begin(byte modbusSlaveID, Stream& stream, int8_t enablePin) {
    return begin(modbusSlaveID, &stream, enablePin);
}
void modbusMaster::setEnablePin(int8_t enablePin) {
    _enablePin = enablePin;
}
int8_t modbusMaster::getEnablePin() {
    return _enablePin;
}
void modbusMaster::setCommandTimeout(uint32_t timeout) {
    modbusTimeout = timeout;
}
uint32_t modbusMaster::getCommandTimeout() {
    return modbusTimeout;
}
void modbusMaster::setFrameTimeout(uint32_t timeout) {
    modbusFrameTimeout = timeout;
}
uint32_t modbusMaster::getFrameTimeout() {
    return modbusFrameTimeout;
}
void modbusMaster::setCommandRetries(uint8_t retries) {
    commandRetries = retries;
}
uint8_t modbusMaster::getCommandRetries() {
    return commandRetries;
}


// These functions return a variety of data from a data register
uint16_t modbusMaster::uint16FromRegister(byte regType, int regNum, endianness endian) {
    getRegisters(regType, regNum, UINT16_SIZE / 2);
    return uint16FromFrame(endian);
}
int16_t modbusMaster::int16FromRegister(byte regType, int regNum, endianness endian) {
    getRegisters(regType, regNum, INT16_SIZE / 2);
    return int16FromFrame(endian);
}
float modbusMaster::float32FromRegister(byte regType, int regNum, endianness endian) {
    getRegisters(regType, regNum, FLOAT32_SIZE / 2);
    return float32FromFrame(endian);
}
uint32_t modbusMaster::uint32FromRegister(byte regType, int regNum, endianness endian) {
    getRegisters(regType, regNum, UINT32_SIZE / 2);
    return uint32FromFrame(endian);
}
int32_t modbusMaster::int32FromRegister(byte regType, int regNum, endianness endian) {
    getRegisters(regType, regNum, INT32_SIZE / 2);
    return int32FromFrame(endian);
}
uint32_t modbusMaster::TAI64FromRegister(byte regType, int regNum) {
    getRegisters(regType, regNum, TAI64_SIZE / 2);
    return TAI64FromFrame();
}
uint32_t modbusMaster::TAI64NFromRegister(byte regType, int regNum,
                                          uint32_t& nanoseconds) {
    getRegisters(regType, regNum, TAI64N_SIZE / 2);
    return TAI64NFromFrame(nanoseconds);
}
uint32_t modbusMaster::TAI64NAFromRegister(byte regType, int regNum,
                                           uint32_t& nanoseconds,
                                           uint32_t& attoseconds) {
    getRegisters(regType, regNum, TAI64NA_SIZE / 2);
    return TAI64NAFromFrame(nanoseconds, attoseconds);
}
byte modbusMaster::byteFromRegister(byte regType, int regNum, int byteNum) {
    getRegisters(regType, regNum, 1);
    if (byteNum == 1) {
        return byteFromFrame();
    } else {
        return byteFromFrame(4);
    }
}
uint16_t modbusMaster::pointerFromRegister(byte regType, int regNum,
                                           endianness endian) {
    getRegisters(regType, regNum, POINTER_SIZE / 2);
    return pointerFromFrame(endian);
}
int8_t modbusMaster::pointerTypeFromRegister(byte regType, int regNum,
                                             endianness endian) {
    getRegisters(regType, regNum, POINTER_SIZE / 2);
    return pointerTypeFromFrame(endian);
}
String modbusMaster::StringFromRegister(byte regType, int regNum, int charLength) {
    getRegisters(regType, regNum, charLength / 2);
    return StringFromFrame(charLength);
}
void modbusMaster::charFromRegister(byte regType, int regNum, char outChar[],
                                    int charLength) {
    getRegisters(regType, regNum, charLength / 2);
    charFromFrame(outChar, charLength);
}


uint16_t modbusMaster::uint16FromInputRegister(int regNum, endianness endian) {
    return uint16FromRegister(0x04, regNum, endian);
}
int16_t modbusMaster::int16FromInputRegister(int regNum, endianness endian) {
    return int16FromRegister(0x04, regNum, endian);
}
float modbusMaster::float32FromInputRegister(int regNum, endianness endian) {
    return float32FromRegister(0x04, regNum, endian);
}
uint32_t modbusMaster::uint32FromInputRegister(int regNum, endianness endian) {
    return uint32FromRegister(0x04, regNum, endian);
}
int32_t modbusMaster::int32FromInputRegister(int regNum, endianness endian) {
    return int32FromRegister(0x04, regNum, endian);
}
uint32_t modbusMaster::TAI64FromInputRegister(int regNum) {
    return TAI64FromRegister(0x04, regNum);
}
uint32_t modbusMaster::TAI64NFromInputRegister(int regNum, uint32_t& nanoseconds) {
    return TAI64NFromRegister(0x04, regNum, nanoseconds);
}
uint32_t modbusMaster::TAI64NAFromInputRegister(int regNum, uint32_t& nanoseconds,
                                                uint32_t& attoseconds) {
    return TAI64NAFromRegister(0x04, regNum, nanoseconds, attoseconds);
}
byte modbusMaster::byteFromInputRegister(int regNum, int byteNum) {
    return byteFromRegister(0x04, regNum, byteNum);
}
uint16_t modbusMaster::pointerFromInputRegister(int regNum, endianness endian) {
    return pointerFromRegister(0x04, regNum, endian);
}
int8_t modbusMaster::pointerTypeFromInputRegister(int regNum, endianness endian) {
    return pointerTypeFromRegister(0x04, regNum, endian);
}
String modbusMaster::StringFromInputRegister(int regNum, int charLength) {
    return StringFromRegister(0x04, regNum, charLength);
}
void modbusMaster::charFromInputRegister(int regNum, char outChar[], int charLength) {
    return charFromRegister(0x04, regNum, outChar, charLength);
}


uint16_t modbusMaster::uint16FromHoldingRegister(int regNum, endianness endian) {
    return uint16FromRegister(0x03, regNum, endian);
}
int16_t modbusMaster::int16FromHoldingRegister(int regNum, endianness endian) {
    return int16FromRegister(0x03, regNum, endian);
}
float modbusMaster::float32FromHoldingRegister(int regNum, endianness endian) {
    return float32FromRegister(0x03, regNum, endian);
}
uint32_t modbusMaster::uint32FromHoldingRegister(int regNum, endianness endian) {
    return uint32FromRegister(0x03, regNum, endian);
}
int32_t modbusMaster::int32FromHoldingRegister(int regNum, endianness endian) {
    return int32FromRegister(0x03, regNum, endian);
}
uint32_t modbusMaster::TAI64FromHoldingRegister(int regNum) {
    return TAI64FromRegister(0x03, regNum);
}
uint32_t modbusMaster::TAI64NFromHoldingRegister(int regNum, uint32_t& nanoseconds) {
    return TAI64NFromRegister(0x03, regNum, nanoseconds);
}
uint32_t modbusMaster::TAI64NAFromHoldingRegister(int regNum, uint32_t& nanoseconds,
                                                  uint32_t& attoseconds) {
    return TAI64NAFromRegister(0x03, regNum, nanoseconds, attoseconds);
}
byte modbusMaster::byteFromHoldingRegister(int regNum, int byteNum) {
    return byteFromRegister(0x03, regNum, byteNum);
}
uint16_t modbusMaster::pointerFromHoldingRegister(int regNum, endianness endian) {
    return pointerFromRegister(0x03, regNum, endian);
}
int8_t modbusMaster::pointerTypeFromHoldingRegister(int regNum, endianness endian) {
    return pointerTypeFromRegister(0x03, regNum, endian);
}
String modbusMaster::StringFromHoldingRegister(int regNum, int charLength) {
    return StringFromRegister(0x03, regNum, charLength);
}
void modbusMaster::charFromHoldingRegister(int regNum, char outChar[], int charLength) {
    return charFromRegister(0x03, regNum, outChar, charLength);
}


// These set data in registers to a variety of data types
// For data types that can be only one register long, the modbus command to pre-set
// a single register (0x06) will be used by default.  To force the use of the pre-set
// multiple registers command, set the boolean input for forceMultiple to true.
bool modbusMaster::uint16ToRegister(int regNum, uint16_t value, endianness endian,
                                    bool forceMultiple) {
    byte inputData[UINT16_SIZE] = {
        0x00,
    };
    uint16ToFrame(value, endian, inputData);
    return setRegisters(regNum, UINT16_SIZE / 2, inputData, forceMultiple);
}
bool modbusMaster::int16ToRegister(int regNum, int16_t value, endianness endian,
                                   bool forceMultiple) {
    byte inputData[INT16_SIZE] = {
        0x00,
    };
    int16ToFrame(value, endian, inputData);
    return setRegisters(regNum, INT16_SIZE / 2, inputData, forceMultiple);
}
bool modbusMaster::float32ToRegister(int regNum, float value, endianness endian) {
    byte inputData[FLOAT32_SIZE] = {
        0x00,
    };
    float32ToFrame(value, endian, inputData);
    return setRegisters(regNum, FLOAT32_SIZE / 2, inputData);
}
bool modbusMaster::uint32ToRegister(int regNum, uint32_t value, endianness endian) {
    byte inputData[UINT32_SIZE] = {
        0x00,
    };
    uint32ToFrame(value, endian, inputData);
    return setRegisters(regNum, UINT32_SIZE / 2, inputData);
}
bool modbusMaster::int32ToRegister(int regNum, int32_t value, endianness endian) {
    byte inputData[INT32_SIZE] = {
        0x00,
    };
    int32ToFrame(value, endian, inputData);
    return setRegisters(regNum, INT32_SIZE / 2, inputData);
}
bool modbusMaster::TAI64ToRegister(int regNum, uint32_t seconds) {
    byte inputData[TAI64_SIZE] = {
        0x00,
    };
    TAI64ToFrame(seconds, inputData);
    return setRegisters(regNum, TAI64_SIZE / 2, inputData);
}
bool modbusMaster::TAI64NToRegister(int regNum, uint32_t seconds,
                                    uint32_t nanoseconds) {
    byte inputData[TAI64N_SIZE] = {
        0x00,
    };
    TAI64NToFrame(seconds, nanoseconds, inputData);
    return setRegisters(regNum, TAI64N_SIZE / 2, inputData);
}
bool modbusMaster::TAI64NAToRegister(int regNum, uint32_t seconds, uint32_t nanoseconds,
                                     uint32_t attoseconds) {
    byte inputData[TAI64NA_SIZE] = {
        0x00,
    };
    TAI64NAToFrame(seconds, nanoseconds, attoseconds, inputData);
    return setRegisters(regNum, TAI64NA_SIZE / 2, inputData);
}
bool modbusMaster::byteToRegister(int regNum, int byteNum, byte value,
                                  bool forceMultiple) {
    byte inputData[2] = {
        0x00,
    };
    byteToFrame(value, byteNum, inputData);
    return setRegisters(regNum, 1, inputData, forceMultiple);
}
bool modbusMaster::pointerToRegister(int regNum, uint16_t value, pointerType point,
                                     endianness endian, bool forceMultiple) {
    byte inputData[UINT16_SIZE] = {
        0x00,
    };
    pointerToFrame(value, point, endian, inputData);
    return setRegisters(regNum, UINT16_SIZE / 2, inputData, forceMultiple);
}
bool modbusMaster::StringToRegister(int regNum, String value, bool forceMultiple) {
    int  charLength = value.length();
    byte inputData[charLength];
    StringToFrame(value, inputData);
    return setRegisters(regNum, charLength / 2, inputData, forceMultiple);
}
bool modbusMaster::charToRegister(int regNum, char inChar[], int charLength,
                                  bool forceMultiple) {
    byte inputData[charLength];
    charToFrame(inChar, charLength, inputData);
    return setRegisters(regNum, charLength / 2, inputData, forceMultiple);
}


//----------------------------------------------------------------------------
//                          BULK AND COIL GETTER FUNCTIONS
//----------------------------------------------------------------------------

bool modbusMaster::getRegisters(byte readCommand, int16_t startRegister,
                                int16_t numRegisters, char* buff[]) {
    if (buff == nullptr) { return false; }
    if (!getRegisters(readCommand, startRegister, numRegisters)) { return false; }
    // copy from the responseBuffer, starting at character 3 (the first two are the
    // returned bytes)
    memcpy(buff, responseBuffer + 3, numRegisters * 2);
    // null terminate the buffer
    memset(buff, '\0', numRegisters * 2);
    return true;
}
bool modbusMaster::getHoldingRegisters(int16_t startRegister, int16_t numRegisters,
                                       char* buff[]) {
    if (buff == nullptr) { return false; }
    if (!getRegisters(0x03, startRegister, numRegisters)) { return false; }
    // copy from the responseBuffer, starting at character 3 (the first two are the
    // returned bytes)
    memcpy(buff, responseBuffer + 3, numRegisters * 2);
    // null terminate the buffer
    memset(buff, '\0', numRegisters * 2);
    return true;
}
bool modbusMaster::getInputRegisters(int16_t startRegister, int16_t numRegisters,
                                     char* buff[]) {
    if (buff == nullptr) { return false; }
    if (!getRegisters(0x04, startRegister, numRegisters)) { return false; }
    // copy from the responseBuffer, starting at character 3 (the first two are the
    // returned bytes)
    memcpy(buff, responseBuffer + 3, numRegisters * 2);
    // null terminate the buffer
    memset(buff, '\0', numRegisters * 2);
    return true;
}
bool modbusMaster::getCoil(int16_t coilAddress) {
    if (!getCoils(coilAddress, 1)) { return false; }
    return (bitRead(responseBuffer[3], 0) != 0);
}
bool modbusMaster::getCoils(int16_t startCoil, int16_t numCoils, byte* buff[]) {
    if (buff == nullptr) { return false; }
    if (!getCoils(startCoil, numCoils)) { return false; }
    // copy from the responseBuffer, starting at character 3 (the first two are the
    // returned bytes)
    memcpy(buff, responseBuffer + 3, ceil(numCoils / 8));
    // null terminate the buffer
    memset(buff, '\0', ceil(numCoils / 8));
    return true;
}
bool modbusMaster::getDiscreteInput(int16_t inputAddress) {
    if (!getDiscreteInputs(inputAddress, 1)) { return false; }
    return (bitRead(responseBuffer[3], 0) != 0);
}
bool modbusMaster::getDiscreteInputs(int16_t startInput, int16_t numInputs,
                                     byte* buff[]) {
    if (buff == nullptr) { return false; }
    if (!getDiscreteInputs(startInput, numInputs)) { return false; }
    // copy from the responseBuffer, starting at character 3 (the first two are the
    // returned bytes)
    memcpy(buff, responseBuffer + 3, ceil(numInputs / 8));
    // null terminate the buffer
    memset(buff, '\0', ceil(numInputs / 8));
    return true;
}

//----------------------------------------------------------------------------
//                           MID LEVEL FUNCTIONS
//----------------------------------------------------------------------------


// These functions return a variety of data from an input modbus RTU frame.
// Currently, the only "frame" available is the response buffer.
uint16_t modbusMaster::uint16FromFrame(endianness endian, int start_index) {
    return leFrameFromFrame(UINT16_SIZE, endian, start_index).uInt16[0];
}

int16_t modbusMaster::int16FromFrame(endianness endian, int start_index) {
    return leFrameFromFrame(INT16_SIZE, endian, start_index).Int16[0];
}

float modbusMaster::float32FromFrame(endianness endian, int start_index) {
    return leFrameFromFrame(FLOAT32_SIZE, endian, start_index).Float32;
}

uint32_t modbusMaster::uint32FromFrame(endianness endian, int start_index) {
    return leFrameFromFrame(UINT32_SIZE, endian, start_index).uInt32;
}

int32_t modbusMaster::int32FromFrame(endianness endian, int start_index) {
    return leFrameFromFrame(INT32_SIZE, endian, start_index).Int32;
}

uint32_t modbusMaster::TAI64FromFrame(int start_index) {
    return leFrameFromFrame(4, bigEndian, start_index + 4).uInt32;
}
uint32_t modbusMaster::TAI64NFromFrame(uint32_t& nanoseconds, int start_index) {
    nanoseconds = leFrameFromFrame(4, bigEndian, start_index + 8).uInt32;
    return leFrameFromFrame(4, bigEndian, start_index + 4).uInt32;
}
uint32_t modbusMaster::TAI64NAFromFrame(uint32_t& nanoseconds, uint32_t& attoseconds,
                                        int start_index) {
    attoseconds = leFrameFromFrame(4, bigEndian, start_index + 12).uInt32;
    nanoseconds = leFrameFromFrame(4, bigEndian, start_index + 8).uInt32;
    return leFrameFromFrame(4, bigEndian, start_index + 4).uInt32;
}

byte modbusMaster::byteFromFrame(int start_index) {
    return responseBuffer[start_index];
}

uint16_t modbusMaster::pointerFromFrame(endianness endian, int start_index) {
    leFrame fram;
    if (endian == bigEndian) {
        // Shift the lower address bit DOWN two
        fram.Byte[0] = responseBuffer[start_index + 1] >> 2;
        fram.Byte[1] = responseBuffer[start_index];
    } else {
        // Shift the lower address bit DOWN two
        fram.Byte[0] = responseBuffer[start_index] >> 2;
        fram.Byte[1] = responseBuffer[start_index + 1];
    }
    return fram.Int16[0];
}

int8_t modbusMaster::pointerTypeFromFrame(endianness endian, int start_index) {
    uint8_t pointerRegType;
    // Mask with 3 (0b00000011) to get the last two bits, which are the type
    if (endian == bigEndian) {
        pointerRegType = responseBuffer[start_index + 1] & 3;
    } else {
        pointerRegType = responseBuffer[start_index] & 3;
    }
    return pointerRegType;
}

String modbusMaster::StringFromFrame(int charLength, int start_index) {
    char charString[charLength + 1];
    int  j = 0;
    for (int i = start_index; i < start_index + charLength; i++) {
        // check that it's a printable character
        if (responseBuffer[i] >= 0x20 && responseBuffer[i] <= 0x7E) {
            // converts from "byte" type to "char" type
            charString[j] = responseBuffer[i];
            j++;
        }
    }
    if (j < charLength + 1) {
        for (int i = j; i < +charLength + 1; i++) { charString[j] = '\0'; }
    }
    String string = String(charString);
    string.trim();
    return string;
}

void modbusMaster::charFromFrame(char outChar[], int charLength, int start_index) {
    int j = 0;
    for (int i = start_index; i < start_index + charLength; i++) {
        // check that it's a printable character
        if (responseBuffer[i] >= 0x20 && responseBuffer[i] <= 0x7E) {
            outChar[j] = responseBuffer[i];  // converts from "byte" type to "char" type
            j++;
        }
    }
    if (j < charLength) {
        for (int i = j; i < +charLength; i++) { outChar[j] = '\0'; }
    }
}

// These insert values into a longer modbus data frame.
void modbusMaster::uint16ToFrame(uint16_t value, endianness endian, byte modbusFrame[],
                                 int start_index) {
    leFrame fram;
    fram.uInt16[0] = value;
    int end_index  = (start_index + UINT16_SIZE) - 1;
    for (int i = 0; i < UINT16_SIZE; i++) {
        if (endian == bigEndian) {
            modbusFrame[end_index - i] = fram.Byte[i];
        } else {
            modbusFrame[start_index + i] = fram.Byte[i];
        }
    }
}
void modbusMaster::int16ToFrame(int16_t value, endianness endian, byte modbusFrame[],
                                int start_index) {
    leFrame fram;
    fram.Int16[0] = value;
    int end_index = (start_index + INT16_SIZE) - 1;
    for (int i = 0; i < INT16_SIZE; i++) {
        if (endian == bigEndian) {
            modbusFrame[end_index - i] = fram.Byte[i];
        } else {
            modbusFrame[start_index + i] = fram.Byte[i];
        }
    }
}
void modbusMaster::float32ToFrame(float value, endianness endian, byte modbusFrame[],
                                  int start_index) {
    leFrame fram;
    fram.Float32  = value;
    int end_index = (start_index + FLOAT32_SIZE) - 1;
    for (int i = 0; i < FLOAT32_SIZE; i++) {
        if (endian == bigEndian) {
            modbusFrame[end_index - i] = fram.Byte[i];
        } else {
            modbusFrame[start_index + i] = fram.Byte[i];
        }
    }
}
void modbusMaster::uint32ToFrame(uint32_t value, endianness endian, byte modbusFrame[],
                                 int start_index) {
    leFrame fram;
    fram.uInt32   = value;
    int end_index = (start_index + UINT32_SIZE) - 1;
    for (int i = 0; i < UINT32_SIZE; i++) {
        if (endian == bigEndian) {
            modbusFrame[end_index - i] = fram.Byte[i];
        } else {
            modbusFrame[start_index + i] = fram.Byte[i];
        }
    }
}
void modbusMaster::int32ToFrame(int32_t value, endianness endian, byte modbusFrame[],
                                int start_index) {
    leFrame fram;
    fram.Int32    = value;
    int end_index = (start_index + INT32_SIZE) - 1;
    for (int i = 0; i < INT32_SIZE; i++) {
        if (endian == bigEndian) {
            modbusFrame[end_index - i] = fram.Byte[i];
        } else {
            modbusFrame[start_index + i] = fram.Byte[i];
        }
    }
}
void modbusMaster::TAI64ToFrame(uint32_t seconds, byte modbusFrame[], int start_index) {
    // The first 4 bytes of the 64-bit value will be 0x400000 until the year 2106
    modbusFrame[start_index] = 0x40;

    leFrame fram;
    fram.Int32 = seconds;
    // The 32-bit seconds start 4 bytes after the 64-bit value starts
    int byte_offset_sec = 4;
    int end_index_sec   = start_index + byte_offset_sec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++) {
        modbusFrame[end_index_sec - i] = fram.Byte[i];
    }
}
void modbusMaster::TAI64NToFrame(uint32_t seconds, uint32_t nanoseconds,
                                 byte modbusFrame[], int start_index) {
    // The first 4 bytes of the 64-bit value will be 0x400000 until the year 2106
    modbusFrame[start_index] = 0x40;

    leFrame fram;
    fram.Int32 = seconds;
    // The 32-bit seconds start 4 bytes after the 64-bit value starts
    int byte_offset_sec = 4;
    int end_index_sec   = start_index + byte_offset_sec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++) {
        modbusFrame[end_index_sec - i] = fram.Byte[i];
    }

    fram.Int32 = nanoseconds;
    // The 32-bit nanoseconds start 8 bytes after the 64-bit value starts
    int byte_offset_nanosec = 8;
    int end_index_nanosec   = start_index + byte_offset_nanosec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++) {
        modbusFrame[end_index_nanosec - i] = fram.Byte[i];
    }
}
void modbusMaster::TAI64NAToFrame(uint32_t seconds, uint32_t nanoseconds,
                                  uint32_t attoseconds, byte modbusFrame[],
                                  int start_index) {
    // The first 4 bytes of the 64-bit value will be 0x400000 until the year 2106
    modbusFrame[start_index] = 0x40;

    leFrame fram;
    fram.Int32 = seconds;
    // The 32-bit seconds start 4 bytes after the 64-bit value starts
    int byte_offset_sec = 4;
    int end_index_sec   = start_index + byte_offset_sec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++) {
        modbusFrame[end_index_sec - i] = fram.Byte[i];
    }

    fram.Int32 = nanoseconds;
    // The 32-bit nanoseconds start 8 bytes after the 64-bit value starts
    int byte_offset_nanosec = 8;
    int end_index_nanosec   = start_index + byte_offset_nanosec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++) {
        modbusFrame[end_index_nanosec - i] = fram.Byte[i];
    }

    // The 32-bit attoseconds start 12 bytes after the 64-bit value starts
    fram.Int32              = attoseconds;
    int byte_offset_attosec = 12;
    int end_index_attosec   = start_index + byte_offset_attosec + UINT32_SIZE - 1;
    for (int i = 0; i < UINT32_SIZE; i++) {
        modbusFrame[end_index_attosec - i] = fram.Byte[i];
    }
}
void modbusMaster::byteToFrame(byte value, int byteNum, byte modbusFrame[],
                               int start_index) {
    if (byteNum == 1) {
        modbusFrame[start_index] = value;
    } else {
        modbusFrame[start_index + 1] = value;
    }
}
void modbusMaster::pointerToFrame(uint16_t value, pointerType point, endianness endian,
                                  byte modbusFrame[], int start_index) {
    leFrame fram;
    fram.uInt16[0] = value;
    if (endian == bigEndian) {
        // Shift the lower address bit UP two
        modbusFrame[start_index + 1] = fram.Byte[0] << 2;
        modbusFrame[start_index + 1] |= point;
        modbusFrame[start_index] = fram.Byte[1];
    } else {
        // Shift the lower address bit UP two
        modbusFrame[start_index] = fram.Byte[0] << 2;
        modbusFrame[start_index] |= point;
        modbusFrame[start_index + 1] = fram.Byte[1];
    }
}
void modbusMaster::StringToFrame(String value, byte modbusFrame[], int start_index) {
    int  charLength = value.length();
    char charString[charLength];
    value.toCharArray(charString, charLength);
    int j = 0;
    for (int i = 0; i < charLength; i++) {
        // converts from "char" type to "byte" type
        modbusFrame[start_index + j] = charString[i];
        j++;
    }
}
void modbusMaster::charToFrame(char inChar[], int charLength, byte modbusFrame[],
                               int start_index) {
    int j = 0;
    for (int i = 0; i < charLength; i++) {
        // converts from "char" type to "byte" type
        modbusFrame[start_index + j] = inChar[i];
        j++;
    }
}


//----------------------------------------------------------------------------
//                           LOW LEVEL FUNCTIONS
//----------------------------------------------------------------------------

// This gets data from either a holding or input register
// For a holding register readCommand = 0x03
// For an input register readCommand = 0x04
bool modbusMaster::getRegisters(byte readCommand, int16_t startRegister,
                                int16_t numRegisters) {
    return getModbusData(readCommand, startRegister, numRegisters);
}
bool modbusMaster::getHoldingRegisters(int16_t startRegister, int16_t numRegisters) {
    return getRegisters(0x03, startRegister, numRegisters);
}
bool modbusMaster::getInputRegisters(int16_t startRegister, int16_t numRegisters) {
    return getRegisters(0x04, startRegister, numRegisters);
}
bool modbusMaster::getCoils(int16_t startCoil, int16_t numCoils) {
    return getModbusData(0x01, startCoil, numCoils);
}
bool modbusMaster::getDiscreteInputs(int16_t startInput, int16_t numInputs) {
    return getModbusData(0x02, startInput, numInputs);
}

bool modbusMaster::getModbusData(byte readCommand, int16_t startAddress,
                                 int16_t numChunks, uint8_t expectedReturnBytes) {
    // Create an array for the command
    byte command[8];

    // Put in the slave id and the command
    command[0] = _slaveID;
    command[1] = readCommand;

    // Put in the starting register
    leFrame fram  = {{
        0,
    }};
    fram.Int16[0] = startAddress;
    command[2]    = fram.Byte[1];
    command[3]    = fram.Byte[0];

    // Put in the number of registers
    fram.Int16[1] = numChunks;
    command[4]    = fram.Byte[3];
    command[5]    = fram.Byte[2];

    // The size of the returned frame should be:
    // # Registers X 1 bit or 2 bytes/register + 5 bytes of modbus RTU frame
    if (expectedReturnBytes == 0) {
        switch (readCommand) {
            case 0x01:  // Coils
            case 0x02:  // Discrete Inputs
                expectedReturnBytes = ceil(numChunks / 8.0);
                break;
            case 0x03:  // Holding Registers
            case 0x04:  // Input Registers
                expectedReturnBytes = numChunks * 2;
                break;
            default:
                // If the read command is not recognized, return false
                return false;
        }
    }
    uint8_t returnFrameSize = expectedReturnBytes + 5;

    // Try up to commandRetries times to get the right results
    int  tries   = 0;
    bool success = false;
    while (!success && tries < commandRetries) {
        // Send out the command - this adds the CRC and verifies that the return is from
        // the right slave and has the correct CRC
        // The structure of the responses should be:
        // {slaveID, fxnCode, # bytes, data, CRC (hi/lo)}
        int16_t respSize = sendCommand(command, 8);
        success          = (respSize == returnFrameSize &&
                   responseBuffer[2] == expectedReturnBytes);
        // if we got a valid modbusErrorCode, stop trying
        if (static_cast<int8_t>(lastError) > 0 && static_cast<int8_t>(lastError) < 0x0C) {
            tries = commandRetries;                      // exit the loop
        } else if (!success && lastError == NO_ERROR) {  // print error info
            debugPrint(F("Failed to get requested data on try "), tries + 1, '\n');
            debugPrint(F("  Got back "), respSize, F(" of expected "), returnFrameSize,
                       F(" bytes from slave\n"));
            debugPrint(F("  The slave said it is returning "), responseBuffer[2],
                       F(" bytes of expected "), expectedReturnBytes, '\n');

            delay(25);
        }
        tries++;
    }

    return success;
};

// This sets the value of one or more holding registers
// Modbus commands 0x06 and 0x10 (16)
// The boolean switch to "forceMultiple" will force the command 0x10 (16,
// preset multiple registers) instead of using 0x06 for a single register
bool modbusMaster::setRegisters(int16_t startRegister, int16_t numRegisters,
                                byte value[], bool forceMultiple) {
    // figure out how long the command will be
    int commandLength;
    if (numRegisters > 1 || forceMultiple) {
        // The full command for writing multiple registers has:
        // - slave address (1 byte)
        // - function = 0x10 (1 byte)
        // - starting register address hi/lo (2 bytes)
        // - register quantity hi/lo (2 bytes)
        // - count of bytes to write (1 bytes)
        // - two bytes per register (numRegisters * 2)
        // - CRC hi/lo (2 bytes)
        // For a total size of numRegisters * 2 + 9
        commandLength = numRegisters * 2 + 9;
    } else {
        // The full command for writing a single register has:
        // - slave address (1 byte)
        // - function = 0x06 (1 byte)
        // - starting register address hi/lo (2 bytes)
        // - write data hi/lo (single register = 2 bytes)
        // - CRC hi/lo (2 bytes)
        // For a total size of 8
        commandLength = 8;
    }

    // Create an array for the command
    byte command[commandLength];

    // Put in the slave id and the command
    command[0] = _slaveID;
    if (numRegisters > 1 || forceMultiple) {
        command[1] = 0x10;
    } else {
        command[1] = 0x06;
    }

    // Put in the starting register
    leFrame fram  = {{
        0,
    }};
    fram.Int16[0] = startRegister;
    command[2]    = fram.Byte[1];
    command[3]    = fram.Byte[0];

    // Put in the register values
    // For multiple registers, need to add in how many registers and how many bytes
    if (numRegisters > 1 || forceMultiple) {
        // Put in the number of registers
        fram.Int16[1] = numRegisters;
        command[4]    = fram.Byte[3];
        command[5]    = fram.Byte[2];
        // Put in the number of bytes to write
        command[6] = numRegisters * 2;
        // Put in the data, allowing 7 extra spaces for the modbus frame structure
        for (int i = 7; i < numRegisters * 2 + 7; i++) { command[i] = value[i - 7]; }
    }
    // For a single register, only need the data itself
    else {
        // Put in the data, allowing 4 extra spaces for the modbus frame structure
        for (int i = 4; i < numRegisters * 2 + 4; i++) { command[i] = value[i - 4]; }
    }

    // Try up to commandRetries times to get the right results
    int     tries    = 0;
    int16_t respSize = 0;
    bool    success  = false;
    while (!success && tries < commandRetries) {
        // Send out the command - this adds the CRC and verifies that the return is from
        // the right slave and has the correct CRC
        respSize = sendCommand(command, commandLength);
        // The structure of the response for 0x10 should be:
        // {slaveID, fxnCode, Address of 1st register, # Registers, CRC}
        if (numRegisters > 1 || forceMultiple) {
            success = respSize == 8 && int16FromFrame(bigEndian, 2) == startRegister &&
                int16FromFrame(bigEndian, 4) == numRegisters;
        }
        // The structure of the response for 0x06 should be:
        // {slaveID, fxnCode, Address of 1st register, Value written, CRC}
        if (numRegisters == 1) {
            success = respSize == 8 && int16FromFrame(bigEndian, 2) == startRegister &&
                responseBuffer[4] == value[0] && responseBuffer[5] == value[1];
        }
        // if we got a valid modbusErrorCode, stop trying
        if (static_cast<int8_t>(lastError) > 0 && static_cast<int8_t>(lastError) < 0x0C) {
            tries = commandRetries;                      // exit the loop
        } else if (!success && lastError == NO_ERROR) {  // print error info
            debugPrint(F("Failed to set register[s] on try "), tries + 1, '\n');
            debugPrint(F("  Got back "), respSize, F(" of expected "), 8,
                       F(" bytes\n"));
            debugPrint(F("  The slave said it set registers starting at register "),
                       int16FromFrame(bigEndian, 2), F(" of expected "), startRegister,
                       '\n');
            if (numRegisters > 1 || forceMultiple) {
                debugPrint(F("  The slave said it set "), int16FromFrame(bigEndian, 4),
                           F(" of expected "), numRegisters, F(" registers on try "),
                           tries + 1, '\n');
            }
            if (numRegisters == 1) {
                debugPrint(F("  The slave said it set the register to "),
                           responseBuffer[4], responseBuffer[5], F(" when "), value[0],
                           value[1], F(" was expected"), '\n');
            }
            delay(25);
        }
        tries++;
    }

    return success;
};


bool modbusMaster::setCoil(int16_t coilAddress, bool value) {
    // The full command for writing a single coil has:
    // - slave address (1 byte)
    // - function = 0x05 (1 byte)
    // - coil address hi/lo (2 bytes)
    // - write data hi/lo (2 bytes) [always 0xFF00 or 0x0000]
    // - CRC hi/lo (2 bytes)
    // For a total size of 8
    int commandLength = 8;

    // Create an array for the command
    byte command[commandLength];

    // Put in the slave id and the command
    command[0] = _slaveID;
    command[1] = 0x05;

    // Put in the coil address
    leFrame fram  = {{
        0,
    }};
    fram.Int16[0] = coilAddress;
    command[2]    = fram.Byte[1];
    command[3]    = fram.Byte[0];

    // Put in the coil value
    command[4] = value ? 0xff : 0x00;
    command[5] = 0x00;

    // Try up to commandRetries times to get the right results
    int     tries    = 0;
    int16_t respSize = 0;
    bool    success  = false;
    while (!success && tries < commandRetries) {
        // Send out the command - this adds the CRC and verifies that the return is from
        // the right slave and has the correct CRC
        respSize = sendCommand(command, commandLength);
        // The structure of the response for 0x05 should be:
        // {slaveID, fxnCode, Address of coil (hi/lo), write data hi/lo, CRC (hi/lo)}
        // which is exactly the same as the command itself.
        if (respSize == 8 && strncmp((char*)responseBuffer, (char*)command, 8) == 0) {
            success = true;
        }
        // if we got a valid modbusErrorCode, stop trying
        if (static_cast<int8_t>(lastError) > 0 && static_cast<int8_t>(lastError) < 0x0C) {
            tries = commandRetries;                      // exit the loop
        } else if (!success && lastError == NO_ERROR) {  // print error info
            debugPrint(F("Failed to set a single coil on try "), tries + 1, '\n');
            debugPrint(F("  Got back "), respSize, F(" of expected "), 8,
                       F(" bytes from slave\n"));
            debugPrint(F("  The slave response "),
                       strncmp((char*)responseBuffer, (char*)command, 8) == 0
                           ? F("does ")
                           : F("does not "),
                       F("match the command\n"));
            delay(25);
        }
        tries++;
    }

    return success;
};

bool modbusMaster::setCoils(int16_t startCoil, int16_t numCoils, byte value[]) {
    // figure out how long the command will be
    // The full command for writing multiple coils has:
    // - slave address (1 byte)
    // - function = 0x0F (1 byte)
    // - starting coil address hi/lo (2 bytes)
    // - coil quantity hi/lo (2 bytes)
    // - count of bytes to write (1 bytes)
    // - 1 bit (1/8 byte!) per coil (numCoils / 8)
    // - CRC hi/lo (2 bytes)
    // For a total size of numCoils / 8 + 9
    int commandLength = ceil(numCoils / 8.0) + 9;

    // Create an array for the command
    byte command[commandLength];

    // Put in the slave id and the command
    command[0] = _slaveID;
    command[1] = 0x0F;

    // Put in the starting coil
    leFrame fram  = {{
        0,
    }};
    fram.Int16[0] = startCoil;
    command[2]    = fram.Byte[1];
    command[3]    = fram.Byte[0];

    // Put in the coil values
    // For multiple coils, need to add in how many coils and how many bytes
    // Put in the number of coils
    fram.Int16[1] = numCoils;
    command[4]    = fram.Byte[3];
    command[5]    = fram.Byte[2];
    // Put in the number of bytes to write
    command[6] = ceil(numCoils / 8.0);
    // Put in the data, allowing 7 extra spaces for the modbus frame structure
    for (int i = 7; i < ceil(numCoils / 8.0) + 7; i++) { command[i] = value[i - 7]; }

    // Try up to commandRetries times to get the right results
    int     tries    = 0;
    int16_t respSize = 0;
    bool    success  = false;
    while (!success && tries < commandRetries) {
        // Send out the command - this adds the CRC and verifies that the return is from
        // the right slave and has the correct CRC
        respSize = sendCommand(command, commandLength);
        // The structure of the response for 0x0F should be:
        // {slaveID, fxnCode, Address of 1st coil (hi/lo), # coils (hi/lo), CRC
        // (hi/lo)}
        success = (respSize == 8 && int16FromFrame(bigEndian, 2) == startCoil &&
                   int16FromFrame(bigEndian, 4) == numCoils);
        // if we got a valid modbusErrorCode, stop trying
        if (static_cast<int8_t>(lastError) > 0 && static_cast<int8_t>(lastError) < 0x0C) {
            tries = commandRetries;                      // exit the loop
        } else if (!success && lastError == NO_ERROR) {  // print error info
            debugPrint(F("Failed to set multiple coils on try "), tries + 1, '\n');
            debugPrint(F("Got back "), respSize, F(" of expected "), 8, F(" bytes\n"));
            debugPrint(F("The slave said it set coils starting at coil "),
                       int16FromFrame(bigEndian, 2), F(" of expected "), startCoil);
            debugPrint(F("The slave said it set "), int16FromFrame(bigEndian, 4),
                       F(" of expected "), numCoils, F(" coils\n"));
            delay(25);
        }
        tries++;
    }

    return success;
};


//----------------------------------------------------------------------------
//                           LOWEST LEVEL FUNCTION
//----------------------------------------------------------------------------

// This sends a command to the sensor bus and listens for a response
uint16_t modbusMaster::sendCommand(byte command[], int commandLength) {
    // Empty the response buffer
    for (int i = 0; i < RESPONSE_BUFFER_SIZE; i++) {
        modbusMaster::responseBuffer[i] = 0x00;
    }

    // Add the CRC to the frame
    insertCRC(command, commandLength);

    // Send out the command
    driverEnable();
    emptySerialBuffer(_stream);  // Clear any junk before sending command
    _stream->write(command, commandLength);
    _stream->flush();
    receiverEnable();
    // Print the raw send (for debugging)
    debugPrint("Raw Request >>> ");
    printFrameHex(command, commandLength);

    // Wait for a response
    uint32_t start = millis();
    while (_stream->available() == 0 && millis() - start < modbusTimeout) { delay(1); }


    bool gotGoodResponse = true;
    int  bytesRead       = 0;
    if (_stream->available() > 0) {
        // Read the incoming bytes
        bytesRead = _stream->readBytes(responseBuffer, 135);
        emptySerialBuffer(_stream);

        // Print the raw response (for debugging)
        debugPrint("Raw Response (", bytesRead, " bytes) <<< ");
        printFrameHex(responseBuffer, bytesRead);

        // Verify that the values match with the commands
        if (responseBuffer[0] != _slaveID) {
            gotGoodResponse = false;
            lastError       = WRONG_SLAVE_ID;
        }

        // Verify that the CRC is correct
        calculateCRC(responseBuffer, bytesRead);
        if (crcFrame[0] != responseBuffer[bytesRead - 2] ||
            crcFrame[1] != responseBuffer[bytesRead - 1]) {
            gotGoodResponse = false;
            lastError       = BAD_CRC;
        }

        // Check for exception response
        // An exception response sets the highest bit of the function code in the
        // response.
        if (gotGoodResponse && ((responseBuffer[1] & 0b10000000) == 0b10000000)) {
            gotGoodResponse = false;
            lastError       = static_cast<modbusErrorCode>(responseBuffer[2]);
        }
    } else {
        gotGoodResponse = false;
        lastError       = NO_RESPONSE;
    }

    if (gotGoodResponse) {
        // If everything passes, return the number of bytes
        lastError = NO_ERROR;
        return bytesRead;
    }

    // If we get here, something went wrong
    printLastError();
    return static_cast<uint16_t>(lastError) << 12;
}


void modbusMaster::printLastError(void) {
    _debugStream->print("Modbus Error: ");
    switch (lastError) {
        case NO_ERROR: debugPrint("No Error\n"); break;
        case ILLEGAL_FUNCTION: debugPrint("Illegal Function!\n"); break;
        case ILLEGAL_DATA_ADDRESS: debugPrint("Illegal Data Address!\n"); break;
        case ILLEGAL_DATA_VALUE: debugPrint("Illegal Data Value!\n"); break;
        case SLAVE_DEVICE_FAILURE: debugPrint("Slave Device Failure!\n"); break;
        case ACKNOWLEDGE: debugPrint("Acknowledge...\n"); break;
        case SLAVE_DEVICE_BUSY: debugPrint("Slave Device Busy!\n"); break;
        case NEGATIVE_ACKNOWLEDGE: debugPrint("Negative Acknowledge!\n"); break;
        case MEMORY_PARITY: debugPrint("Memory Parity Error!\n"); break;
        case GATEWAY_PATH_UNAVAILABLE: debugPrint("Gateway Path Unavailable!\n"); break;
        case GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND:
            debugPrint("Gateway Target Device Failed to Respond!\n");
            break;
        case WRONG_SLAVE_ID:
            debugPrint("Response is not from the correct modbus slave!\n");
            break;
        case BAD_CRC: debugPrint("CRC check failed!\n"); break;
        case NO_RESPONSE: debugPrint("No Response!\n"); break;
        default:
            debugPrint("Unknown Error Code: ");
            debugPrint(String(lastError, HEX));
            debugPrint("\n");
            break;
    }
}


//----------------------------------------------------------------------------
//                           PRIVATE HELPER FUNCTIONS
//----------------------------------------------------------------------------

// This flips the device/receive enable to DRIVER so the arduino can send text
void modbusMaster::driverEnable(void) {
    if (_enablePin >= 0) {
        digitalWrite(_enablePin, HIGH);
        debugPrint("RS485 Driver/Master Tx Enabled\n");
        delay(8);
    }
}

// This flips the device/receive enable to RECEIVER so the sensor can send text
void modbusMaster::receiverEnable(void) {
    if (_enablePin >= 0) {
        digitalWrite(_enablePin, LOW);
        debugPrint("RS485 Receiver/Slave Tx Enabled\n");
        // delay(8);
    }
}

// This empties the serial buffer
void modbusMaster::emptySerialBuffer(Stream* stream) {
    while (stream->available() > 0) {
        stream->read();
        delay(modbusFrameTimeout);
    }
}

// Just a function to pretty-print the modbus hex frames
// This is purely for debugging
void modbusMaster::printFrameHex(byte modbusFrame[], int frameLength) {
    debugPrint("{");
    for (int i = 0; i < frameLength; i++) {
        debugPrint("0x");
        if (modbusFrame[i] < 16) debugPrint("0");
        debugPrint(String(modbusFrame[i], HEX));
        if (i < frameLength - 1) debugPrint(", ");
    }
    debugPrint("}\n");
}


// Calculates a Modbus RTC cyclical redundancy code (CRC)
// and adds it to the last two bytes of a frame
// From: https://ctlsys.com/support/how_to_compute_the_modbus_rtu_message_crc/
// and: https://stackoverflow.com/questions/19347685/calculating-modbus-rtu-crc-16
//
void modbusMaster::calculateCRC(byte modbusFrame[], int frameLength) {
    // Reset the CRC frame
    modbusMaster::crcFrame[0] = {0x00};
    modbusMaster::crcFrame[1] = {0x00};

    uint16_t crc = 0xFFFF;
    for (int pos = 0; pos < frameLength - 2; pos++) {
        crc ^= (unsigned int)modbusFrame[pos];  // XOR byte into least sig. byte of crc

        for (int i = 8; i != 0; i--) {  // Loop over each bit
            if ((crc & 0x0001) != 0) {  // If the least significant bit (LSB) is set
                crc >>= 1;              // Shift right and XOR 0xA001
                crc ^= 0xA001;
            } else {        // Else least significant bit (LSB) is not set
                crc >>= 1;  // Just shift right
            }
        }
    }

    // Break into low and high bytes
    byte crcLow  = crc & 0xFF;
    byte crcHigh = crc >> 8;

    // Append the bytes to the end of the frame
    crcFrame[0] = crcLow;
    crcFrame[1] = crcHigh;
}
void modbusMaster::insertCRC(byte modbusFrame[], int frameLength) {
    // Calculate the CRC
    calculateCRC(modbusFrame, frameLength);

    // Append the bytes to the end of the frame
    modbusFrame[frameLength - 2] = crcFrame[0];
    modbusFrame[frameLength - 1] = crcFrame[1];
}

// This slices one array out of another
// Used for slicing one or more registers out of a returned modbus RTU frame
void modbusMaster::sliceArray(byte inputArray[], byte outputArray[], int start_index,
                              int numBytes, bool reverseOrder) {
    if (reverseOrder) {
        // Reverse the order of bytes to get from big-endian to little-endian
        int j = numBytes - 1;
        for (int i = 0; i < numBytes; i++) {
            outputArray[i] = inputArray[start_index + j];
            j--;
        }

    } else {
        for (int i = 0; i < numBytes; i++) {
            outputArray[i] = inputArray[start_index + i];
        }
    }
}


// This converts data in a modbus RTU frame into a little-endian data frame
// little-endian data frames are needed because all Arduino processors are little-endian
leFrame modbusMaster::leFrameFromFrame(int varBytes, endianness endian,
                                       int start_index) {
    // Set up a temporary output frame
    byte outFrame[varBytes];
    // Slice data from the full response frame into the temporary output frame
    if (endian == bigEndian) {
        sliceArray(responseBuffer, outFrame, start_index, varBytes, true);
    } else {
        sliceArray(responseBuffer, outFrame, start_index, varBytes, false);
    }
    // Put it into a little-endian frame (the format of all arduino processors)
    leFrame fram = {{
        0,
    }};
    memcpy(fram.Byte, outFrame, varBytes);
    // Return the little-endian frame
    return fram;
}

// cspell:words fram byte_offset_attosec end_index_attosec
