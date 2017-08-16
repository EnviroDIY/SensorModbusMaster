/*
 *scanModbus.h
*/

#ifndef scanModbus_h
#define scanModbus_h

#include <Arduino.h>

// The communcations modes
typedef enum specCommMode
{
    modbusRTU = 0,
    modbusASCII,
    modbusTCP
} specCommMode;

// The possible serial baud rates
typedef enum specBaudRate
{
    b9600 = 0,
    b19200,
    b38400
} specBaudRate;

// The possible serial parities
typedef enum specParity
{
    noParity = 0,
    even,
    odd
} specParity;

// The possible cleaning modes
typedef enum cleaningMode
{
    noCleaning = 0,
    manual,
    automatic
} cleaningMode;

class scan
{

public:

    // This function sets up the communication
    // It should be run during the arduino "setup" function.
    // The "stream" device must be initialized prior to running this.
    bool begin(byte modbusSlaveID, Stream *stream, int enablePin = -1);
    bool begin(byte modbusSlaveID, Stream &stream, int enablePin = -1);

    // This gets all of the setup information at once
    bool getSetup(void);

    // This gets the modbus slave ID.  Not supported by many sensors.
    byte getSlaveID(void);

    // This sets a new modbus slave ID
    bool setSlaveID(byte newSlaveID);

    // The communication mode
    int getCommunicationMode(void);
    bool setCommunicationMode(specCommMode mode);

    // The serial baud rate (iff communication mode = modbus RTU or modbus ASCII)
    int getBaudRate(void);
    bool setBaudRate(specBaudRate baud);

    // The serial parity (iff communication mode = modbus RTU or modbus ASCII)
    int getParity(void);
    bool setParity(specParity parity);

    // Reset all settings to default
    bool resetSettings(void);

    // Get the "s::canpoint" of the device
    String getScanPoint(void);

    // Cleaning mode configuration
    int getCleaningMode(void);
    bool setCleaningMode(cleaningMode mode);

    // Cleaning interval (ie, number of samples between cleanings)
    int getCleaningInterval(void);
    bool setCleaningInterval(uint16_t intervalSamples);

    // Cleaning duration in seconds
    int getCleaningDuration(void);
    bool setCleaningDuration(uint16_t secDuration);

    // Waiting time between end of cleaning and start of measurement
    int getCleaningWait(void);
    bool setCleaningWait(uint16_t secDuration);

    // Current system time as a 64-bit count of seconds from Jan 1, 1970
    long getSystemTime(void);

    // Measurement interval in seconds (0 - as fast as possible)
    int getMeasInterval(void);
    bool setMeasInterval(uint16_t secBetween);

    // Logging Mode (0 = on; 1 = off)
    int getLoggingMode(void);
    bool setLoggingMode(uint8_t mode);

    // Logging interval for data logger in minutes (0 = no logging active)
    int getLoggingInterval(void);
    bool setLoggingInterval(uint16_t interval);

    // Available number of logged results in datalogger since last clearing
    int getNumLoggedResults(void);

    // "Index device status public + private & parameter results from logger
    // storage to Modbus registers.  If no stored results are available,
    // results are NaN, Device status bit3 is set."
    // I'm really not sure what this means...
    int getIndexLogResult(void);


    // Get the version of the modbus mapping protocol
    // The float variables for the version must be
    // initialized prior to calling this function.
    bool getModbusVersion(float &modbusVersion);

    // This returns a pretty string with the model information
    String getModel(void);

    // This gets the instrument serial number as a String
    String getSerialNumber(void);

    // This gets the hardware and software version of the sensor
    // The float variables for the hardware and software versions must be
    // initialized prior to calling this function.
    // The reference (&) is needed when declaring this function so that
    // the function is able to modify the actual input floats rather than
    // create and destroy copies of them.
    // There is no need to add the & when actually usig the function.
    bool getVersion(float &hardwareVersion, float &softwareVersion);

    // Device rebooter counter
    int getHWStarts(void);

    // This gets the number of parameters the spectro::lyzer is set to measure
    int getParameterCount(void);

    // This returns a pretty string with the parameter measured.
    String getParameter(int parmNumber);

    // This returns a pretty string with the measurement units.
    String getUnits(int parmNumber);

    // This gets the upper limit of the parameter
    // The float variable must be initialized prior to calling this function.
    bool getUpperLimit(int parmNumber, float &upperLimit);

    // This gets the lower limit of the parameter
    // The float variable must be initialized prior to calling this function.
    bool getLowerLimit(int parmNumber, float &lowerLimit);

    // Last measurement time as a 64-bit count of seconds from Jan 1, 1970
    long getSampleTime(void);

    // This gets values back from the sensor and puts them into a previously
    // initialized float variable.  The actual return from the function is the
    // int which is a bit-mask describing the parameter status.
    int getValue(int parmNumber, float &value1);

    // This get up to 8 values back from the spectro::lyzer
    bool getAllValues(float &value1, float &value2, float &value3, float &value4,
                      float &value5, float &value6, float &value7, float &value8);

    // This sets a stream for debugging information to go to;
    void setDebugStream(Stream *stream){_debugStream = stream;}

    // These functions are to convert various s::can register code to strings
    String printCommMode(uint16_t code);
    uint16_t printBaudRate(uint16_t code);
    String printParity(uint16_t code);
    String printCleaningMode(uint16_t code);
    String printRegisterType(uint16_t code);
    String printLoggingMode(uint16_t code);


private:

    // Define a small-endian frame as a union - that is a special class type that
    // can hold only one of its non-static data members at a time, in this case,
    // either 4-bytes OR a single float OR a 32 bit interger
    // With avr-gcc (Arduino's compiler), integer and floating point variables are
    // all physically stored in memory in little-endian byte order, so this union
    // is all that is needed to get the correct float value from a small-endian
    // modbus..  S::CAN's version of modbus returns all values in big-endian
    // form, so you must reverse the byte order to make this work.
    union SeFrame4 {
      float Float;
      uint32_t Int;
      byte Byte[4];
    };

    // This is just as above, but for a 2-bytes
    union SeFrame2 {
      float Float;
      uint16_t Int;
      byte Byte[2];
    };

    // These functions returns the float/int from a 4-byte big-endian array
    // beginning at a specific index of another array.
    float float32FromBEFrame(byte indata[], int start_index);
    uint32_t uint32FromBEFrame(byte indata[], int start_index);

    // This function returns an integer from a 2-byte big-endian array beginning
    // at a specific index of another array.
    int uint16FromBEFrame(byte indata[], int start_index);

    // This returns a "String" from a slice of a character array
    String StringFromFrame(byte indata[], int start_index, int length);

    // These get the register address and register type of a pointer to other
    // information within a modbus register.
    // The register address and type must be initialized prior to calling this function
    // For the register types:
    //   0 (0b00) - Holding register (read by command 0x03, written by 0x06 or 0x10)
    //   1 (0b01) - Input register (read by command 0x04)
    //   2 (0b10) - Discrete input register (read by command 0x02)
    //   3 (0b10) - Coil  (read by command 0x01, written by 0x05)
    int pointerFromBEFrame(byte indata[], int start_index);
    int pointerTypeFromBEFrame(byte indata[], int start_index);

    // This function inserts a float as a 4-byte small endian array into another
    // array beginning at the specified index.
    void floatIntoFrame(byte indata[], int start_index, float value);

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

    // This sends three requests for a single register
    // If the spectro::lyzer is sleeping, it will not respond until the third one
    bool wakeSpec(void);

    byte _slaveID;  // The sensor slave id
    Stream *_stream;  // The stream instance (serial port) for communication with the RS485
    int _enablePin;  // The pin controlling the driver/receiver enable on the RS485-to-TLL chip

    // Setup information from holding registers
    bool _gotHoldingRegSpecSetup = false;
    uint16_t _commMode;
    uint16_t _baudRate;
    uint16_t _parity;
    String _scanPoint;
    uint16_t _configRegNumber;
    uint8_t _configRegType;
    uint16_t _cleaningMode;
    uint16_t _cleaningInterval;
    uint16_t _cleaningDuration;
    uint16_t _cleaningWait;
    uint16_t _measInterval;
    uint16_t _loggingMode;
    uint16_t _loggingInterval;
    uint16_t _numLoggedResults;
    uint16_t _indexLogResult;

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
    byte responseBuffer[135];  // This needs to be bigger than the largest response
                               // For 8 parameters with 8 registers each:
                               // 64 registers * 2 bytes per register + 5 frame bytes

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
