/*
 *scanModbus.h
*/

#ifndef scanModbus_h
#define scanModbus_h

#include <Arduino.h>


//----------------------------------------------------------------------------
//                        ENUMERATIONS FOR CONFIGURING DEVICE
//----------------------------------------------------------------------------

// This needs to be bigger than the largest response
// For 8 parameters with 8 registers each:
// 64 registers * 2 bytes per register + 5 frame
#define MAX_RESPONSE_SIZE 136

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

// The possible spectral sources
typedef enum spectralSource
{
    fingerprint = 0,  // The absorption spectrum as it is measured (fingerprint)[Abs/m]
    compensFP,  // The turbidity-compensated fingerprint [Abs/m]
    derivFP,  // The first derivative of the measured fingerprint (i.e. gradient)[Abs/m]
    diff2oldorgFP,  // The difference between the current fingerprint and the previous one in memory [Abs/m]
    transmission,  // The percent transmission - NOT linear wrt concentrations [%/cm2]
    derivcompFP,  // The first derivative of the turbidity-compensated fingerprint [Abs/m]
    transmission10,  // The percent transmission per 10 cm2 [%/10cm2]
    other  // I don't know what this is, but the modbus registers on the spec have 8 groups of fingerprints..
} spectralSource;

// The "endianness" of returned values
typedef enum endianness
{
    littleEndian = 0,
    bigEndian
} endianness;


// Define a little-endian frame as a union - that is a special class type that
// can hold only one of its non-static data members at a time, in this case,
// either 4-bytes OR a single float OR a 32 bit interger
// With avr-gcc (Arduino's compiler), integer and floating point variables are
// all physically stored in memory in little-endian byte order, so this union
// is all that is needed to get the correct float value from a little-endian
// modbus..  S::CAN's version of modbus returns all values in big-endian
// form, so you must reverse the byte order to make this work.
typedef union leFrame {
    byte Byte[4];        // occupies 4 bytes
    float Float;         // occupies 4 bytes
    int32_t Int32;       // occupies 4 bytes
    uint32_t uInt32;     // occupies 4 bytes
    int16_t Int16[2];    // occupies 4 bytes
    uint16_t uInt16[2];  // occupies 4 bytes
    char Char[4];        // occupies 4 bytes
} leFrame;



//*****************************************************************************
//*****************************************************************************
//*****************************The S::CAN class********************************
//*****************************************************************************
//*****************************************************************************
class scan
{

public:

//----------------------------------------------------------------------------
//                          GENERAL USE FUNCTIONS
//----------------------------------------------------------------------------

    // This function sets up the communication
    // It should be run during the arduino "setup" function.
    // The "stream" device must be initialized prior to running this.
    // The default baud rate for a spectro::lyser or s::can controller is 34800
    // The default parity is **ODD**
    // Per modbus specifications, there is:
    //    - 1 start bit
    //    - 8 data bits, least significant bit sent first
    //    - 1 stop bit if parity is used-2 bits if no parity
    // Note that neither SoftwareSerial, AltSoftSerial, nor NeoSoftwareSerial
    // will support the default odd parity!  This means you must either use a
    // corretnly set up HARDWARE serial port on your Arduino or change the
    // parity setting of the s::can using some other program before connecting
    // it to your Arduino.
    bool begin(byte modbusSlaveID, Stream *stream, int enablePin = -1);
    bool begin(byte modbusSlaveID, Stream &stream, int enablePin = -1);

    // This prints out all of the setup information to the selected stream
    bool printSetup(Stream *stream);
    bool printSetup(Stream &stream);

    // This resets all settings to default
    // Please note that after this you will most likely have to re-begin
    // your stream and sensor after running this function because the
    // baud rate and parity may have changed.  Again, keep in mind that the
    // default parity is ODD, which is not supported by SoftwareSerial.
    bool resetSettings(void);

    // This just returns the slave ID that was entered in the begin function.
    // If you don't know your slave ID, you must find it some other way
    byte getSlaveID(void){return _slaveID;}

    // This sets a new modbus slave ID
    bool setSlaveID(byte newSlaveID);

    // This returns the current device status as a bitmap
    int getDeviceStatus(void);
    // This parses the device status bitmap and prints the resuts to the stream
    void printDeviceStatus(uint16_t bitmask, Stream *stream);
    void printDeviceStatus(uint16_t bitmask, Stream &stream);

    // Prints out the current system status
    // It would be nice if there were a way to actually get it...
    void printSystemStatus(uint16_t bitmask, Stream *stream);
    void printSystemStatus(uint16_t bitmask, Stream &stream);

    // This "wakes" the spectro::lyzer so it's ready to communicate"
    bool wakeSpec(void);

//----------------------------------------------------------------------------
//           FUNCTIONS TO RETURN THE ACTUAL SAMPLE TIMES AND VALUES
//----------------------------------------------------------------------------

    // Last measurement time as a 32-bit count of seconds from Jan 1, 1970
    long getSampleTime(void);

    // This gets values back from the sensor and puts them into a previously
    // initialized float variable.  The actual return from the function is an
    // integer which is a bit-mask describing the parameter status.
    int getParameterValue(int parmNumber, float &value);
    // This parses the parameter status bitmap and prints the resuts to the stream
    void printParameterStatus(uint16_t bitmask, Stream *stream);
    void printParameterStatus(uint16_t bitmask, Stream &stream);

    // This get up to 8 values back from the spectro::lyzer
    bool getAllParameterValues(float &value1, float &value2, float &value3, float &value4,
                      float &value5, float &value6, float &value7, float &value8);
    // This prints the parameter data as **TAB** separated data to a stream
    // NB:  You can use this to print to a file on a SD card!
    void printParameterData(Stream *stream);
    void printParameterData(Stream &stream);

    // This gets spectral values from the sensor and puts them into a previously
    // initialized float array.  The array must have space for 200 values!
    // The actual return from the function is an integer which is a bit-mask
    // describing the fingerprint status (or, well, it would be if I could figure
    //  out which register that value lived in).
    int getFingerprintData(float fpArray[], spectralSource source=fingerprint);
    // This prints the fingerprint data as **TAB** separated data to a stream
    // NB:  You can use this to print to a file on a SD card!
    void printFingerprintData(Stream *stream, spectralSource source=fingerprint);
    void printFingerprintData(Stream &stream, spectralSource source=fingerprint);

    // This prints out a header for a "par" file ini the format that the
    // s::can/ana::xxx software is expecting
    void printParameterHeader(Stream *stream);
    void printParameterHeader(Stream &stream);
    // This prints out a header for a "fp" file ini the format that the
    // s::can/ana::xxx software is expecting
    void printFingerprintHeader(Stream *stream);
    void printFingerprintHeader(Stream &stream);



//----------------------------------------------------------------------------
//              FUNCTIONS TO GET AND CHANGE DEVICE CONFIGURATIONS
//----------------------------------------------------------------------------
// I cannot promise that your device will actually accept any changes from
// these set commands.  It is better to use s::can's software to make any
// changes to the logger configurations.

    // Functions for the communication mode
    int getCommunicationMode(void);
    bool setCommunicationMode(specCommMode mode);
    String parseCommunicationMode(uint16_t code);

    // Functions for the serial baud rate
    // (iff communication mode = modbus RTU or modbus ASCII)
    int getBaudRate(void);
    bool setBaudRate(specBaudRate baud);
    uint16_t parseBaudRate(uint16_t code);

    // Functions for the serial parity
    // (iff communication mode = modbus RTU or modbus ASCII)
    int getParity(void);
    bool setParity(specParity parity);
    String parseParity(uint16_t code);

    // Functions for the pointer to the private configuration register
    int getprivateConfigRegister(void);
    String parseRegisterType(uint16_t code);

    // Functions for the "s::canpoint" of the device
    String getScanPoint(void);
    bool setScanPoint(char charScanPoint[12]);

    // Functions for the cleaning mode configuration
    int getCleaningMode(void);
    bool setCleaningMode(cleaningMode mode);
    String parseCleaningMode(uint16_t code);

    // Functions for the cleaning interval (ie, number of samples between cleanings)
    int getCleaningInterval(void);
    bool setCleaningInterval(uint16_t intervalSamples);

    // Functions for the cleaning duration in seconds
    int getCleaningDuration(void);
    bool setCleaningDuration(uint16_t secDuration);

    // Functions for the waiting time between end of cleaning
    // and the start of a measurement
    int getCleaningWait(void);
    bool setCleaningWait(uint16_t secDuration);

    // Functions for the current system time in seconds from Jan 1, 1970
    long getSystemTime(void);
    bool setSystemTime(long currentUnixTime);

    // Functions for the measurement interval in seconds
    // (0 - as fast as possible)
    int getMeasInterval(void);
    bool setMeasInterval(uint16_t secBetween);

    // Functions for the logging Mode (0 = on; 1 = off)
    int getLoggingMode(void);
    bool setLoggingMode(uint8_t mode);
    String parseLoggingMode(uint16_t code);

    // Functions for the ogging interval for data logger in minutes
    // (0 = no logging active)
    int getLoggingInterval(void);
    bool setLoggingInterval(uint16_t interval);

    // Available number of logged results in datalogger since last clearing
    int getNumLoggedResults(void);

    // "Index device status public + private & parameter results from logger
    // storage to Modbus registers.  If no stored results are available,
    // results are NaN, Device status bit3 is set."
    // I'm really not sure what this means...
    int getIndexLogResult(void);



//----------------------------------------------------------------------------
//           FUNCTIONS TO GET AND CHANGE PARAMETER CONFIGURATIONS
//----------------------------------------------------------------------------
// I cannot promise that your device will actually accept any changes from
// these set commands.  It is better to use s::can's software to make any
// changes to the logger configurations.

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
//          FUNCTIONS TO GET SETUP INFORMATION FROM THE INPUT REGISTERS
//----------------------------------------------------------------------------
// This information can be read, but cannot be changed

    // Get the version of the modbus mapping protocol
    float getModbusVersion(void);

    // This returns a pretty string with the model information
    String getModel(void);

    // This gets the instrument serial number as a String
    String getSerialNumber(void);

    // This gets the hardware version of the sensor
    float getHWVersion(void);

    // This gets the software version of the sensor
    float getSWVersion(void);

    // This gets the number of times the spec has been rebooted
    // (Device rebooter counter)
    int getHWStarts(void);

    // This gets the number of parameters the spectro::lyzer is set to measure
    int getParameterCount(void);

    // This gets the datatype of the parameters and parameter limits
    // This is a check for compatibility
    int getParamterType(void);
    String parseParamterType(uint16_t code);

    // This gets the scaling factor for all parameters which depend on eParameterType
    int getParameterScale(void);



//----------------------------------------------------------------------------
//                       PURELY DEBUGGING FUNCTIONS
//----------------------------------------------------------------------------

    // This sets a stream for debugging information to go to;
    void setDebugStream(Stream *stream){_debugStream = stream;}



//----------------------------------------------------------------------------
//                            PRIVATE FUNCTIONS
//----------------------------------------------------------------------------
//These more-or-less define a fairly complete modbus library on their own.

private:
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
    void calculateCRC(byte modbusFrame[], int frameLength);
    // Adds the CRC to a modbus frame
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

    // This converts data in a register into a little-endian frame
    // little-endian frames are needed because all Arduino processors are little-endian
    leFrame leFrameFromRegister(int varLength,
                                endianness endian=bigEndian,
                                int start_index=3,
                                byte indata[]=responseBuffer);

    // These functions return a variety of data from an input frame
    uint16_t bitmaskFromFrame(endianness endian=bigEndian,
                              int start_index=3,
                              byte indata[]=responseBuffer);
    uint16_t uint16FromFrame(endianness endian=bigEndian,
                             int start_index=3,
                             byte indata[]=responseBuffer);
    int16_t int16FromFrame(endianness endian=bigEndian,
                           int start_index=3,
                           byte indata[]=responseBuffer);
    uint16_t pointerFromFrame(endianness endian=bigEndian,
                              int start_index=3,
                              byte indata[]=responseBuffer);
    int8_t pointerTypeFromFrame(endianness endian=bigEndian,
                                int start_index=3,
                                byte indata[]=responseBuffer);
    float float32FromFrame(endianness endian=bigEndian,
                           int start_index=3,
                           byte indata[]=responseBuffer);
    uint32_t uint32FromFrame(endianness endian=bigEndian,
                             int start_index=3,
                             byte indata[]=responseBuffer);
    int32_t int32FromFrame(endianness endian=bigEndian,
                           int start_index=3,
                           byte indata[]=responseBuffer);
    uint32_t tai64FromFrame(int start_index=3,
                            byte indata[]=responseBuffer);
    String StringFromFrame(int charLength,
                           int start_index=3,
                           byte indata[]=responseBuffer);
    void charFromFrame(char outChar[], int charLength,
                       int start_index=3,
                       byte indata[]=responseBuffer);

    // These functions return a variety of data from a data register
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
    static byte responseBuffer[MAX_RESPONSE_SIZE];
    static byte crcFrame[2];

    // The modbus protocol defines that there can be no more than 1.5 characters
    // of silence between characters in a frame and any space over 3.5 characters
    // defines a new frame.
    // At 34800 baud with 1 start bit, 8 data bits, odd parity, and 2 stop bits
    // (the transmission mode for the S::CAN spectro::lyzer) 1 character takes ~0.286ms
    // So the readBytes() command should time out within 1ms
    const uint32_t modbusTimeout = 500;  // The time to wait for response after a command (in ms)
    const int modbusFrameTimeout = 4;  // the time to wait between characters within a frame (in ms)

    static float junk_val;
    static byte junk_byte;

};

#endif
