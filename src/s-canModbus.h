/*
 *s-canModbus.h
*/

#ifndef s-canModbus_h
#define s-canModbus_h

#include <Arduino.h>


class s-can
{

public:

    // This sets a stream for debugging information to go to;
    void setDebugStream(Stream *stream){_debugStream = stream;}


private:

    // Define a small-endian frame as a union - that is a special class type that
    // can hold only one of its non-static data members at a time, in this case,
    // either 4-bytes OR a single float.
    // With avr-gcc (Arduino's compiler), integer and floating point variables are
    // all physically stored in memory in little-endian byte order, so this union
    // is all that is needed to get the correct float value from the small-endian
    // hex frames returned by s-can's Modbus Sensors
    union SeFrame {
      float Float;
      byte Byte[4];
    };

    // This is just as above, but for a 2-byte interger
    union SeFrame2 {
      uint16_t Int;
      byte Byte[2];
    };

    // This functions return the float from a 4-byte small-endian array beginning
    // at a specific index of another array.
    float floatFromFrame(byte indata[], int stindex);

    // This functions inserts a float as a 4-byte small endian array into another
    // array beginning at the specified index.
    void floatIntoFrame(byte indata[], int stindex, float value);

    // This flips the device/receive enable to DRIVER so the arduino can send text
    void driverEnable(void);

    // This flips the device/receive enable to RECIEVER so the sensor can send text
    void recieverEnable(void);

    // This empties the serial buffer
    void emptyResponseBuffer(Stream *stream);

    // A debugging function for prettily printing raw modbus frames
    // This is purely for debugging
    void printFrameHex(byte modbusFrame[], int frameLength);

    // Calculates a Modbus RTC cyclical redudancy code (CRC)
    // and adds it to the last two bytes of a frame
    void insertCRC(byte modbusFrame[], int frameLength);

    // This sends a command to the sensor bus and listens for a response
    int sendCommand(byte command[], int commandLength);

    int _model;  // The sensor model
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

    int respSize;
    byte responseBuffer[18];  // This needs to be bigger than the largest response

    // The modbus protocol defines that there can be no more than 1.5 characters
    // of silence between characters in a frame and any space over 3.5 characters
    // defines a new frame.
    // At 34800 baud with 1 start bit, 8 data bits, odd parity, and 2 stop bits
    // (the transmission mode for the S::CAN spectro::lyzer) 1 character takes ~0.286ms
    // So the readBytes() command should time out within 1ms
    const uint32_t modbusTimeout = 500;  // The time to wait for response after a command (in ms)
    const int modbusFrameTimeout = 1;  // the time to wait between characters within a frame (in ms)

    static float junk_val;
    static byte junk_byte;

};

#endif
