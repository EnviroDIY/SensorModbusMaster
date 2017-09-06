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


// Define a little-endian frame as a union - that is a special class type that
// can hold only one of its non-static data members at a time.
// With avr-gcc (Arduino's compiler), integer and floating point variables are
// all physically stored in memory in little-endian byte order, so this union
// is all that is needed to translate modbus byte data into the other data forms.
// NB: The byte order of big-endian data must be reversed when it is put in this
// frame format.
typedef union leFrame {
    byte Byte[4];        // 4 bytes will occupy 4 bytes
    float Float;         // a single float occupies 4 bytes
    int32_t Int32;       // a single 32bit interger occupies 4 bytes
    uint32_t uInt32;     // a single 32bit interger occupies 4 bytes
    int16_t Int16[2];    // 2 16bit intergers will occupy 4 bytes
    uint16_t uInt16[2];  // 2 16bit intergers will occupy 4 bytes
    char Char[4];        // 4 characters will occupy 4 bytes
} leFrame;



class modbusMaster
{

public:

    // This function sets up the communication
    // It should be run during the arduino "setup" function.
    // The "stream" device must be initialized prior to running this.
    // Per modbus specifications, the stream must have:
    //    - 1 start bit
    //    - 8 data bits, least significant bit sent first
    //    - 1 stop bit if parity is used-2 bits if no parity
    // Note that neither SoftwareSerial, AltSoftSerial, nor NeoSoftwareSerial
    // will support either even or odd parity!
    bool begin(byte modbusSlaveID, Stream *stream, int enablePin = -1);
    bool begin(byte modbusSlaveID, Stream &stream, int enablePin = -1);

    // These higher-level functions return a variety of data from a single or pair of data registers
    uint16_t bitmaskFromRegister(byte regType, int regNum, endianness endian=bigEndian);
    uint16_t uint16FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    int16_t int16FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    uint16_t pointerFromRegister(byte regType, int regNum, endianness endian=bigEndian);
    int8_t pointerTypeFromRegister(byte regType, int regNum, endianness endian=bigEndian);
    float float32FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    uint32_t uint32FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    int32_t int32FromRegister(byte regType, int regNum, endianness endian=bigEndian);
    uint32_t tai64FromRegister(byte regType, int regNum);
    String StringFromRegister(byte regType, int regNum, int charLength);
    void charFromRegister(byte regType, int regNum, char outChar[], int charLength);

    // These mid-level functions return a variety of data from an input "frame"
    // By default the frame is the response buffer
    // Using these functions will be helpful if you wish to decrease the serial
    // traffic by sending one "getRegisters" request for many registers and then
    // parse that result into many different results.
    uint16_t bitmaskFromFrame(endianness endian=bigEndian, int start_index=3);
    uint16_t uint16FromFrame(endianness endian=bigEndian, int start_index=3);
    int16_t int16FromFrame(endianness endian=bigEndian, int start_index=3);
    uint16_t pointerFromFrame(endianness endian=bigEndian, int start_index=3);
    int8_t pointerTypeFromFrame(endianness endian=bigEndian, int start_index=3);
    float float32FromFrame(endianness endian=bigEndian, int start_index=3);
    uint32_t uint32FromFrame(endianness endian=bigEndian, int start_index=3);
    int32_t int32FromFrame(endianness endian=bigEndian, int start_index=3);
    uint32_t tai64FromFrame(int start_index=3);
    String StringFromFrame(int charLength, int start_index=3);
    void charFromFrame(char outChar[], int charLength, int start_index=3);

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
    bool setRegisters(int16_t startRegister, int16_t numRegisters, byte value[]);

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

    // This sets a stream for debugging information to go to;
    void stopDebugging(void){_debugStream = &nullstream;}



//----------------------------------------------------------------------------
//                            PRIVATE FUNCTIONS
//----------------------------------------------------------------------------

private:

    // This flips the device/receive enable to DRIVER so the arduino can send text
    void driverEnable(void);

    // This flips the device/receive enable to RECIEVER so the sensor can send text
    void recieverEnable(void);

    // This empties the serial buffer
    void emptyResponseBuffer(Stream *stream);

// A function for prettily printing raw modbus frames
void printFrameHex(byte modbusFrame[], int frameLength);

    // Calculates a Modbus RTC cyclical redudancy code (CRC)
    void calculateCRC(byte modbusFrame[], int frameLength);
    // Adds the CRC to a modbus frame
    void insertCRC(byte modbusFrame[], int frameLength);

    // This slices one array out of another
    void sliceArray(byte inputArray[], byte outputArray[],
                    int start_index, int numBytes, bool reverseOrder=false);

    // This converts data in a register into a little-endian frame
    // little-endian frames are needed because all Arduino processors are little-endian
    leFrame leFrameFromRegister(int varLength,
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

    // This needs to be bigger than the largest response
    // For 8 parameters with 8 registers each:
    // 64 registers * 2 bytes per register + 5 frame bytes
    static byte responseBuffer[RESPONSE_BUFFER_SIZE];
    static byte crcFrame[2];

    // The modbus protocol defines that there can be no more than 1.5 characters
    // of silence between characters in a frame and any space over 3.5 characters
    // defines a new frame.
    const uint32_t modbusTimeout = MODBUS_TIMEOUT;  // The time to wait for response after a command (in ms)
    const int modbusFrameTimeout = MODBUS_FRAME_TIMEOUT;  // the time to wait between characters within a frame (in ms)

};

#endif
