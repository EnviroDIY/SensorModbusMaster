/*
 *SensorModbusMaster.h
*/

#ifndef SensorModbusMaster_h
#define SensorModbusMaster_h

#include <Arduino.h>

//----------------------------------------------------------------------------
//                        ENUMERATIONS FOR CONFIGURING DEVICE
//----------------------------------------------------------------------------

// This needs to be bigger than the largest response
// Pet the Specification and Implementation Guide for MODBUS over serial line,
// this value is 256 bytes.  (This translates into a maximum of 125 registers
// to be read via Modbus/RTU and 123 by Modbus/TCP.)
// If you know you will never make any modbus calls longer than this, decrease
// this number to save memory space.
#define RESPONSE_BUFFER_SIZE 256
// The modbus protocol defines that there can be no more than 1.5 characters
// of silence between characters in a frame and any space over 3.5 characters
// defines a new frame.
#define MODBUS_TIMEOUT 500
#define MODBUS_FRAME_TIMEOUT 4

// The "endianness" of returned values
typedef enum endianness
{
    littleEndian = 0,
    bigEndian
} endianness;

// The types of "pointers" to other modbus addresses
typedef enum pointerType
{
    holdingRegister = 0,
    inputRegister,
    inputContacts,
    outputCoil
} pointerType;


// Define a little-endian frame as a union - that is a special class type that
// can hold only one of its non-static data members at a time.
// With avr-gcc (Arduino's compiler), integer and floating point variables are
// all physically stored in memory in little-endian byte order, so this union
// is all that is needed to translate modbus byte data into the other data forms.
// NB: The byte order of big-endian data must be reversed when it is put in this
// frame format.
typedef union leFrame {
    byte Byte[4];        // 4 bytes will occupy 4 bytes
    char Char[4];        // 4 characters will occupy 4 bytes
    uint16_t uInt16[2];  // 2 16bit unsigned integers will occupy 4 bytes
    int16_t Int16[2];    // 2 16bit integers will occupy 4 bytes
    uint32_t uInt32;     // a single 32bit unsigned integer occupies 4 bytes
    int32_t Int32;       // a single 32bit integer occupies 4 bytes
    float Float32;       // a single float occupies 4 bytes
} leFrame;

// Define the sizes (in bytes) of several data types
// There are generally 2 bytes in each register, so this is double the number of registers
#define BYTE_SIZE 1
#define UINT16_SIZE 2
#define INT16_SIZE 2
#define UINT32_SIZE 4
#define INT32_SIZE 4
#define FLOAT32_SIZE 4
#define TAI64_SIZE 8
#define TAI64N_SIZE 12
#define TAI64NA_SIZE 16
#define POINTER_SIZE 2

// NOTE:  The TAI64 is a 64 bit (4 register) data type BUT:
// The first 32 bits (two registers) will be 0x4000 0000 until the year 2106;
// I'm ignoring it for the next 90 years to avoid using 64 bit math
// The next 32 bits (two registers) have the actual seconds past Jan 1, 1970
// In the case of TAI64N data there will be an additional 32 bits (two
// registers) of data representing the nanosecond portion of the time.
// In the case of TAI64NA data there will be an additional 32 bits (two
// registers) of data representing the nanosecond portion of the time and yet
// another 32 bits (two registers) representing the attosecond count.
// Per the TAI64 standard, this value is always big-endian
// https://www.tai64.com/

class modbusMaster
{

public:

    // This function sets up the communication
    // It should be run during the arduino "setup" function.
    // The "stream" device must be initialized prior to running this.
    // Per modbus specifications, the stream must have:
    //    - 1 start bit
    //    - 8 data bits, least significant bit sent first
    //    - 1 stop bit if parity is used - 2 bits if no parity
    // Note that neither SoftwareSerial, AltSoftSerial, nor NeoSoftwareSerial
    // will support either even or odd parity!
    bool begin(byte modbusSlaveID, Stream *stream, int enablePin = -1);
    bool begin(byte modbusSlaveID, Stream &stream, int enablePin = -1);

    // These higher-level functions return a variety of data from a single or pair of data registers
    uint16_t uint16FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    int16_t int16FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    float float32FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    uint32_t uint32FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    int32_t int32FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    uint32_t TAI64FromRegister(byte regType, int regNum);
    uint32_t TAI64NFromRegister(byte regType, int regNum, uint32_t &nanoseconds);
    uint32_t TAI64NAFromRegister(byte regType, int regNum, uint32_t &nanoseconds, uint32_t &attoseconds);
    byte byteFromRegister(byte regType, int regNum, int byteNum);
    uint16_t pointerFromRegister(byte regType, int regNum, endianness endian=bigEndian);
    int8_t pointerTypeFromRegister(byte regType, int regNum, endianness endian=bigEndian);
    String StringFromRegister(byte regType, int regNum, int charLength);
    void charFromRegister(byte regType, int regNum, char outChar[], int charLength);

    // These set data in registers to a variety of data types
    bool uint16ToRegister(int regNum, uint16_t value, endianness endian=bigEndian, bool forceMultiple=false);
    bool int16ToRegister(int regNum, int16_t value, endianness endian=bigEndian, bool forceMultiple=false);
    bool float32ToRegister(int regNum, float value, endianness endian=bigEndian);
    bool uint32ToRegister(int regNum, uint32_t value, endianness endian=bigEndian);
    bool int32ToRegister(int regNum, int32_t value, endianness endian=bigEndian);
    bool TAI64ToRegister(int regNum, uint32_t seconds);
    bool TAI64NToRegister(int regNum, uint32_t seconds, uint32_t nanoseconds);
    bool TAI64NAToRegister(int regNum, uint32_t seconds, uint32_t nanoseconds, uint32_t attoseconds);
    bool byteToRegister(int regNum, int byteNum, byte value, bool forceMultiple=false);
    bool pointerToRegister(int regNum, uint16_t value, pointerType point, endianness endian=bigEndian, bool forceMultiple=false);
    bool StringToRegister(int regNum, String value, bool forceMultiple=false);
    bool charToRegister(int regNum, char inChar[], int charLength, bool forceMultiple=false);

    // These mid-level functions return a variety of data from an input modbus "frame"
    // Currently, the only "frame" available is the response buffer.
    // Using these functions will be helpful if you wish to decrease the serial
    // traffic by sending one "getRegisters" request for many registers and then
    // parse that result into many different results.
    uint16_t uint16FromFrame(endianness endian=bigEndian, int start_index=3);
    int16_t int16FromFrame(endianness endian=bigEndian, int start_index=3);
    float float32FromFrame(endianness endian=bigEndian, int start_index=3);
    uint32_t uint32FromFrame(endianness endian=bigEndian, int start_index=3);
    int32_t int32FromFrame(endianness endian=bigEndian, int start_index=3);
    uint32_t TAI64FromFrame(int start_index=3);
    uint32_t TAI64NFromFrame(uint32_t &nanoseconds, int start_index=3);
    uint32_t TAI64NAFromFrame(uint32_t &nanoseconds, uint32_t &attoseconds, int start_index=3);
    byte byteFromFrame(int start_index=3);
    uint16_t pointerFromFrame(endianness endian=bigEndian, int start_index=3);
    int8_t pointerTypeFromFrame(endianness endian=bigEndian, int start_index=3);
    String StringFromFrame(int charLength, int start_index=3);
    void charFromFrame(char outChar[], int charLength, int start_index=3);

    // These insert values into a longer modbus data frame.
    // These are useful in creating a single long frame which can be sent out
    // in one "setRegisters" command.
    void uint16ToFrame(uint16_t value, endianness endian, byte modbusFrame[], int start_index=0);
    void int16ToFrame(int16_t value, endianness endian, byte modbusFrame[], int start_index=0);
    void float32ToFrame(float value, endianness endian, byte modbusFrame[], int start_index=0);
    void uint32ToFrame(uint32_t value, endianness endian, byte modbusFrame[], int start_index=0);
    void int32ToFrame(int32_t value, endianness endian, byte modbusFrame[], int start_index=0);
    void TAI64ToFrame(uint32_t seconds, byte modbusFrame[], int start_index=0);
    void TAI64NToFrame(uint32_t seconds, uint32_t nanoseconds, byte modbusFrame[], int start_index=0);
    void TAI64NAToFrame(uint32_t seconds, uint32_t nanoseconds, uint32_t attoseconds, byte modbusFrame[], int start_index=0);
    void byteToFrame(byte value, int byteNum, byte modbusFrame[], int start_index=0);
    void pointerToFrame(uint16_t value, pointerType point, endianness endian, byte modbusFrame[], int start_index=0);
    void StringToFrame(String value, byte modbusFrame[], int start_index=0);
    void charToFrame(char inChar[], int charLength, byte modbusFrame[], int start_index=0);

    // This gets data from either a holding or input register
    // For a holding register readCommand = 0x03
    // For an input register readCommand = 0x04
    bool getRegisters(byte readCommand, int16_t startRegister, int16_t numRegisters);

    // This gets data from either an output coil or an input contact
    // For a output coil readCommand = 0x01
    // For an input contact readCommand = 0x02
    // TODO - implement this
    bool getCoils(byte readCommand, int16_t startRegister, int16_t numRegisters)
    {return false;}

    // This sets the value of one or more holding registers
    // Modbus commands 0x06 and 0x10
    // Input registers cannot be written by a Modbus controller/master
    bool setRegisters(int16_t startRegister, int16_t numRegisters, byte value[],
                      bool forceMultiple=false);

    // This sets the value of one or more output coils
    // Modbus commands 0x05 and 0x0F
    // Input contacts cannot be written by a Modbus controller/master
    // TODO - implement this
    bool setCoils(int16_t startRegister, int16_t numRegisters, byte value[])
    {return false;}

    // This is the lowest level command.
    // This takes a command, adds the proper CRC, sends it to the sensor bus,
    // and listens for a response.
    // If it recieves a response from the correct slave with the correct CRC,
    // it returns the number of bytes recieved and put into the responseBuffer.
    // If it recieves a response from the wrong slave, an incorrect CRC, or
    // an exception, it will print notice of the error to the debugging stream
    // and return 0.
    int sendCommand(byte command[], int commandLength);

    // These are purely debugging functions to print out the raw hex data
    // sent between the Arduino and the modbus slave.
    // This sets a stream for debugging information to go to;
    void setDebugStream(Stream *stream){_debugStream = stream;}
    void setDebugStream(Stream &stream){_debugStream = &stream;}

    // This sets a stream for debugging information to go to;
    void stopDebugging(void){_debugStream = &nullstream;}


    // This needs to be bigger than the largest response
    // For 8 parameters with 8 registers each:
    // 64 registers * 2 bytes per register + 5 frame bytes
    static byte responseBuffer[RESPONSE_BUFFER_SIZE];



//----------------------------------------------------------------------------
//                            PRIVATE FUNCTIONS
//----------------------------------------------------------------------------

private:

    // This flips the device/receive enable to DRIVER so the arduino can send text
    void driverEnable(void);

    // This flips the device/receive enable to RECIEVER so the sensor can send text
    void recieverEnable(void);

    // This empties the serial buffer
    void emptySerialBuffer(Stream *stream);

// A function for prettily printing raw modbus RTU frames
void printFrameHex(byte modbusFrame[], int frameLength);

    // Calculates a Modbus RTC cyclical redudancy code (CRC)
    void calculateCRC(byte modbusFrame[], int frameLength);
    // Adds the CRC to a modbus RTU frame
    void insertCRC(byte modbusFrame[], int frameLength);

    // This slices one array out of another
    void sliceArray(byte inputArray[], byte outputArray[],
                    int start_index, int numBytes, bool reverseOrder=false);

    // This converts data in a modbus RTU frame into a little-endian data frame
    // little-endian frames data are needed because all Arduino processors are little-endian
    leFrame leFrameFromFrame(int varLength,
                                endianness endian=bigEndian,
                                int start_index=3);

    byte _slaveID;  // The sensor slave id
    Stream *_stream;  // The stream instance (serial port) for communication with the RS485
    int _enablePin;  // The pin controlling the driver/receiver enable on the RS485-to-TLL chip

    // This creates a null stream to use for "debugging" if you don't want to
    // actually print to a real stream.
    struct NullStream : public Stream
    {
        NullStream( void ) { return; }
        int available( void ) { return 0; }
        void flush( void ) { return; }
        int peek( void ) { return -1; }
        int read( void ){ return -1; }
        size_t write( uint8_t u_Data ){ return 0; }
        size_t write(const uint8_t *buffer, size_t size) { return 0; }
    };
    NullStream nullstream;
    Stream *_debugStream = &nullstream;  // The stream instance (serial port) for debugging

    static byte crcFrame[2];

    // The modbus protocol defines that there can be no more than 1.5 characters
    // of silence between characters in a frame and any space over 3.5 characters
    // defines a new frame.
    const uint32_t modbusTimeout = MODBUS_TIMEOUT;  // The time to wait for response after a command (in ms)
    const int modbusFrameTimeout = MODBUS_FRAME_TIMEOUT;  // the time to wait between characters within a frame (in ms)

};

#endif
