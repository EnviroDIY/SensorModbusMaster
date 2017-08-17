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

// The possible modbus datatypes
typedef enum dataTypes
{
    uint16 = 0,
    pointer,
    pointerType,
    bitmask,
    character,
    float32,
    tai64
} dataTypes;

// The possible modbus datatypes
typedef enum endianness
{
    little = 0,
    big
} endianness;

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

    // This resets all settings to default
    // Please note that after this you will most likely have to re-begin
    // your stream because your baud rate and parity will have changed.
    bool resetSettings(void);

    // This gets the modbus slave ID.  Not supported by many sensors.
    byte getSlaveID(void);

    // This sets a new modbus slave ID
    bool setSlaveID(byte newSlaveID);



//----------------------------------------------------------------------------
//               SETUP INFORMATION FROM THE INPUT REGISTERS
//----------------------------------------------------------------------------

    // Functions for the communication mode
    int getCommunicationMode(int startIndex = 3);
    bool setCommunicationMode(specCommMode mode);
    String printCommMode(uint16_t code);

    // Functions for the serial baud rate
    // (iff communication mode = modbus RTU or modbus ASCII)
    int getBaudRate(int startIndex = 3);
    bool setBaudRate(specBaudRate baud);
    uint16_t printBaudRate(uint16_t code);

    // Functions for the serial parity
    // (iff communication mode = modbus RTU or modbus ASCII)
    int getParity(int startIndex = 3);
    bool setParity(specParity parity);
    String printParity(uint16_t code);

    // Functions for the pointer to the private configuration register
    int getprivateConfigRegister(int startIndex = 3);
    String printRegisterType(uint16_t code);

    // Functions for the "s::canpoint" of the device
    String getScanPoint(int startIndex = 3);
    bool setScanPoint(char charScanPoint[12]);

    // Functions for the cleaning mode configuration
    int getCleaningMode(int startIndex = 3);
    bool setCleaningMode(cleaningMode mode);
    String printCleaningMode(uint16_t code);

    // Functions for the cleaning interval (ie, number of samples between cleanings)
    int getCleaningInterval(int startIndex = 3);
    bool setCleaningInterval(uint16_t intervalSamples);

    // Functions for the cleaning duration in seconds
    int getCleaningDuration(int startIndex = 3);
    bool setCleaningDuration(uint16_t secDuration);

    // Functions for the waiting time between end of cleaning
    // and the start of a measurement
    int getCleaningWait(int startIndex = 3);
    bool setCleaningWait(uint16_t secDuration);

    // Functions for the current system time in seconds from Jan 1, 1970
    long getSystemTime(int startIndex = 3);
    bool setSystemTime(long currentUnixTime);

    // Functions for the measurement interval in seconds
    // (0 - as fast as possible)
    int getMeasInterval(int startIndex = 3);
    bool setMeasInterval(uint16_t secBetween);

    // Functions for the logging Mode (0 = on; 1 = off)
    int getLoggingMode(int startIndex = 3);
    bool setLoggingMode(uint8_t mode);
    String printLoggingMode(uint16_t code);

    // Functions for the ogging interval for data logger in minutes
    // (0 = no logging active)
    int getLoggingInterval(int startIndex = 3);
    bool setLoggingInterval(uint16_t interval);

    // Available number of logged results in datalogger since last clearing
    int getNumLoggedResults(int startIndex = 3);

    // "Index device status public + private & parameter results from logger
    // storage to Modbus registers.  If no stored results are available,
    // results are NaN, Device status bit3 is set."
    // I'm really not sure what this means...
    int getIndexLogResult(int startIndex = 3);



    //----------------------------------------------------------------------------
    //               SETUP INFORMATION FROM THE INPUT REGISTERS
    //----------------------------------------------------------------------------
    // Get the version of the modbus mapping protocol
    float getModbusVersion(int startIndex = 3);

    // This returns a pretty string with the model information
    String getModel(int startIndex = 3);

    // This gets the instrument serial number as a String
    String getSerialNumber(int startIndex = 3);

    // This gets the hardware version of the sensor
    float getHWVersion(int startIndex = 3);

    // This gets the software version of the sensor
    float getSWVersion(int startIndex = 3);

    // This gets the number of times the spec has been rebooted
    // (Device rebooter counter)
    int getHWStarts(int startIndex = 3);

    // This gets the number of parameters the spectro::lyzer is set to measure
    int getParameterCount(int startIndex = 3);

    // This gets the datatype of the parameters and parameter limits
    // This is a check for compatibility
    int getParamterType(int startIndex);
    String printParamterType(uint16_t code);

    // This gets the scaling factor for all parameters which depend on eParameterType
    int getParameterScale(int startIndex);



    //----------------------------------------------------------------------------
    //             PARAMETER INFORMATION FROM THE HOLDING REGISTERS
    //----------------------------------------------------------------------------

    // This returns a pretty string with the parameter measured.
    String getParameter(int parmNumber);

    // This returns a pretty string with the measurement units.
    String getUnits(int parmNumber);

    // This gets the upper limit of the parameter
    // The float variable must be initialized prior to calling this function.
    float getUpperLimit(int parmNumber);

    // This gets the lower limit of the parameter
    // The float variable must be initialized prior to calling this function.
    float getLowerLimit(int parmNumber);



    //----------------------------------------------------------------------------
    //                       ACTUAL SAMPLE TIMES AND VALUES
    //----------------------------------------------------------------------------

    // Last measurement time as a 64-bit count of seconds from Jan 1, 1970
    long getSampleTime(int startIndex = 3);

    // This gets values back from the sensor and puts them into a previously
    // initialized float variable.  The actual return from the function is the
    // int which is a bit-mask describing the parameter status.
    int getValue(int parmNumber, float &value1);

    // This get up to 8 values back from the spectro::lyzer
    bool getAllValues(float &value1, float &value2, float &value3, float &value4,
                      float &value5, float &value6, float &value7, float &value8);

    // This sets a stream for debugging information to go to;
    void setDebugStream(Stream *stream){_debugStream = stream;}


private:

    // Define a small-endian frame as a union - that is a special class type that
    // can hold only one of its non-static data members at a time, in this case,
    // either 4-bytes OR a single float OR a 32 bit interger
    // With avr-gcc (Arduino's compiler), integer and floating point variables are
    // all physically stored in memory in little-endian byte order, so this union
    // is all that is needed to get the correct float value from a small-endian
    // modbus..  S::CAN's version of modbus returns all values in big-endian
    // form, so you must reverse the byte order to make this work.
    union SeFrame {
        byte Byte[4];       // occupies 4 bytes
        float Float;        // occupies 4 bytes
        uint32_t Int32;     // occupies 4 bytes
        uint16_t Int16[2];  // occupies 4 bytes
    };

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

    // This gets data from either a holding or input register
    // For a holding register readCommand = 0x03
    // For an input register readCommand = 0x04
    bool getRegisters(byte readCommand, int16_t startRegister, int16_t numRegisters);

    // This sets the value of one or more holding registers
    // Modbus commands 0x06 and 0x10
    // Input registers cannot be written by a Modbus controller/master
    bool setRegisters(int16_t startRegister, int16_t numRegisters, byte value[]);

    // This slices one array out of another
    void sliceArray(byte inputArray[], byte outputArray[],
                    int start_index, int numBytes, bool reverseOrder=false);

    // These functions return the propertype big-endian data register
    // beginning at a specific index of another array.
    bool dataFromFrame(uint16_t &outputVar, dataTypes regType, byte indata[],
                         int start_index, endianness endian = big);
    bool dataFromFrame(float &outputVar, dataTypes regType, byte indata[],
                         int start_index, endianness endian = big);
    bool dataFromFrame(String &outputVar, dataTypes regType, byte indata[],
                         int start_index, int charLength);
    bool dataFromFrame(uint32_t &outputVar, dataTypes regType, byte indata[],
                         int start_index, endianness endian = big);


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
    uint16_t _configRegType;
    uint16_t _cleaningMode;
    uint16_t _cleaningInterval;
    uint16_t _cleaningDuration;
    uint16_t _cleaningWait;
    uint16_t _measInterval;
    uint16_t _loggingMode;
    uint16_t _loggingInterval;
    uint16_t _numLoggedResults;
    uint16_t _indexLogResult;

    // Setup information from input registers
    bool _gotInputRegSpecSetup = false;
    uint16_t _modbusVersion;
    String _model;
    String _serialNumber;
    String _HWRelease;
    String _SWRelease;
    uint16_t _HWstarts;
    uint16_t _paramCount;
    uint16_t _paramType;
    uint16_t _paramScale;

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
