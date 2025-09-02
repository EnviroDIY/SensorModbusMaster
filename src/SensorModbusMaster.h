/**
 * @file SensorModbusMaster.h
 * @copyright Stroud Water Research Center
 * Part of the EnviroDIY SensorModbusMaster library for Arduino.
 * @license This library is published under the BSD-3 license.
 * @author Sara Geleskie Damiano <sdamiano@stroudcenter.org>
 *
 * @brief Contains the modbusMaster class declarations.
 */

#ifndef SensorModbusMaster_h
#define SensorModbusMaster_h

#include <Arduino.h>

//----------------------------------------------------------------------------
//                        ENUMERATIONS FOR CONFIGURING DEVICE
//----------------------------------------------------------------------------

/**
 * @brief The size of the response buffer for the modbus devices.
 *
 * Per the Specification and Implementation Guide for MODBUS over serial line, the
 * maximum response size is 256 bytes - which is the size we use.
 *
 * If you know you will never make any modbus requests for a modbus response this long,
 * decrease this number to save memory space.
 */
#define RESPONSE_BUFFER_SIZE 256
/**
 * @brief The size of the command buffer for the modbus devices.
 *
 * Per the Specification and Implementation Guide for MODBUS over serial line, the
 * maximum command size is 256 bytes - which is the size we use.
 *
 * If you know in advance the size of the largest command you will send, you can
 * decrease this number to save memory space.
 */
#define COMMAND_BUFFER_SIZE 256
/**
 * @brief The default time to wait for response after a command (in ms)
 */
#define MODBUS_TIMEOUT 500
/**
 * @brief The default time to wait between characters within a frame (in ms)
 *
 * The modbus protocol defines that there can be no more than 1.5 characters of silence
 * between characters in a frame and any space over 3.5 characters defines a new frame.
 */
#define MODBUS_FRAME_TIMEOUT 4

/**
 * @brief The "endianness" of returned values
 */
typedef enum endianness {
    littleEndian = 0,  ///< little endian
    bigEndian          ///< big endian
} endianness;

/**
 * @brief The types of "pointers" to other modbus addresses.
 *
 * Sometimes values in registers are set as the address of a pointer to look at for the
 * real value rather than being set as the values themselves. This pointer type tells
 * you in what section of the memory map to look for the value being pointed to.
 */
typedef enum pointerType {
    holdingRegister = 0,  ///< pointer to a holding register
    inputRegister,        ///< pointer to an input register
    inputContacts,        ///< pointer to a input contact
    outputCoil            ///< pointer to a output coil
} pointerType;

/**
 * @brief The types of errors that can occur during Modbus communication.
 */
typedef enum modbusErrorCode {
    // standard error codes
    NO_ERROR         = 0x00,      ///< All good
    ILLEGAL_FUNCTION = 0x01,      ///< The function code received in the query is not an
                                  ///< allowable action for the slave.
    ILLEGAL_DATA_ADDRESS = 0x02,  ///< The data address received in the query is not an
                                  ///< allowable address for the slave.
    ILLEGAL_DATA_VALUE = 0x03,  ///< A value contained in the query data field is not an
                                ///< allowable value for the slave.
    SLAVE_DEVICE_FAILURE = 0x04,  ///< An unrecoverable error occurred while the slave
                                  ///< was attempting to perform the requested action.
    ACKNOWLEDGE = 0x05,  ///< The server has accepted the request and is processing it,
                         ///< but a long duration of time is required to do so.
    SLAVE_DEVICE_BUSY = 0x06,  ///< The server is engaged in processing a long-duration
                               ///< program command. The client should retransmit the
                               ///< message later when the server is free.
    NEGATIVE_ACKNOWLEDGE = 0x07,  ///< The server cannot perform the program function
                                  ///< received in the query.
    MEMORY_PARITY = 0x08,         ///< The server attempted to read extended memory, but
                                  ///< detected a parity error in the memory.
    GATEWAY_PATH_UNAVAILABLE = 0x0A,  ///< The gateway is not available
    GATEWAY_TARGET_DEVICE_FAILED_TO_RESPOND =
        0x0B,  ///< The gateway target device failed to respond
    // additional codes in this library
    WRONG_SLAVE_ID = 0xD,  ///< The response is not from the correct modbus slave
    BAD_CRC        = 0xE,  ///< The CRC check on the response failed
    NO_RESPONSE    = 0xF,  ///< No response was received at all

} modbusErrorCode;


/**
 * @brief A frame for holding parts of a response.
 * Define a little-endian frame as a union - that is a special class type that can hold
 * only one of its non-static data members at a time.
 *
 * With avr-gcc (Arduino's compiler), integer and floating point variables are all
 * physically stored in memory in little-endian byte order, so this union is all that is
 * needed to translate modbus byte data into the other data forms.
 *
 * @note: The byte order of big-endian data must be reversed when it is put in this
 * frame format.
 */
typedef union leFrame {
    byte     Byte[4];    ///< 4 bytes will occupy 4 bytes
    char     Char[4];    ///< 4 characters will occupy 4 bytes
    uint16_t uInt16[2];  ///< 2 16bit unsigned integers will occupy 4 bytes
    int16_t  Int16[2];   ///< 2 16bit integers will occupy 4 bytes
    uint32_t uInt32;     ///< a single 32bit unsigned integer occupies 4 bytes
    int32_t  Int32;      ///< a single 32bit integer occupies 4 bytes
    float    Float32;    ///< a single float occupies 4 bytes
} leFrame;

// Define the sizes (in bytes) of several data types
// There are generally 2 bytes in each register, so this is double the number of
// registers
#define BYTE_SIZE 1     ///< The size of a single byte in bytes
#define UINT16_SIZE 2   ///< The size of a uint16_t in bytes
#define INT16_SIZE 2    ///< The size of an int16_t in bytes
#define UINT32_SIZE 4   ///< The size of a uint32_t in bytes
#define INT32_SIZE 4    ///< The size of an int32_t in bytes
#define FLOAT32_SIZE 4  ///< The size of a float32 in bytes
#define TAI64_SIZE 8    ///< The size of a 64-bit timestamp in bytes
#define TAI64N_SIZE \
    12  ///< The size of a 64-bit timestamp plus a 32-bit nanosecond in bytes
#define TAI64NA_SIZE \
    16  ///< The size of a 64-bit timestamp plus a 32-bit nanosecond plus a 32-bit
        ///< attosecond in bytes
#define POINTER_SIZE 2  ///< The size of a pointer in bytes (when using Arduino system)

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

/**
 * @brief The class for communicating with modbus devices.
 */
class modbusMaster {

 public:

    /**
     * @anchor ctor_and_begin
     * @name Constructors and Begins
     *
     * Functions to create the modbusMaster object and set up the communication with the
     * Arduino stream connected to the modbus device.
     */
    /**@{*/
    /**
     * @brief Default constructor
     */
    modbusMaster();
    /**
     * @brief Construct a new modbus Master object
     *
     * @param stream A pointer to the Arduino stream object to communicate with.
     */
    modbusMaster(Stream* stream);
    /**
     * @brief Construct a new modbus Master object
     *
     * @param stream A reference to the Arduino stream object to communicate with.
     */
    modbusMaster(Stream& stream);
    /**
     * @copydoc modbusMaster(Stream*)
     * @param enablePin A pin on the Arduino processor to use to send an enable signal
     * to an RS485 to TTL adapter. Use a negative number if this does not apply.
     */
    modbusMaster(Stream* stream, int8_t enablePin);
    /**
     * @copydoc modbusMaster(Stream&)
     * @param enablePin A pin on the Arduino processor to use to send an enable signal
     * to an RS485 to TTL adapter. Use a negative number if this does not apply.
     */
    modbusMaster(Stream& stream, int8_t enablePin);

    /**
     * @brief Equivalent to a constructor - used to assign members of the modbusMaster
     * object
     * @copydetails modbusMaster(Stream* stream)
     * @return Always returns true
     */
    bool begin(Stream* stream);

    /**
     * @brief Equivalent to a constructor - used to assign members of the modbusMaster
     * object
     * @copydetails modbusMaster(Stream& stream)
     * @return Always returns true
     */
    bool begin(Stream& stream);

    /**
     * @brief Equivalent to a constructor - used to assign members of the modbusMaster
     * object
     * @copydetails modbusMaster(Stream* stream, int8_t enablePin)
     * @return Always returns true
     */
    bool begin(Stream* stream, int8_t enablePin);

    /**
     * @brief Equivalent to a constructor - used to assign members of the modbusMaster
     * object
     * @copydetails modbusMaster(Stream& stream, int8_t enablePin)
     * @return Always returns true
     */
    bool begin(Stream& stream, int8_t enablePin);
    /**@}*/

    /**
     * @anchor setters_and_getters
     * @name Object setters and getters
     *
     * Functions to set and get properties of the modbusMaster object.
     */
    /**@{*/
    /**
     * @brief Set the enable pin
     *
     * @param enablePin The pin connected the the enable (direction control) of an RS485
     * to TTL adapter.
     */
    void setEnablePin(int8_t enablePin);
    /**
     * @brief Get the enable pin
     *
     * @return The pin connected the the enable (direction control) of an RS485 to TTL
     * adapter.
     */
    int8_t getEnablePin();
    /**
     * @brief Set the timeout to wait for a response from a sensor after a modbus
     * command.
     *
     * By default, this is #MODBUS_TIMEOUT (500 milliseconds).
     *
     * @param timeout The timeout value in milliseconds.
     */
    void setCommandTimeout(uint32_t timeout);
    /**
     * @brief Get the command timeout
     *
     * By default, this is #MODBUS_TIMEOUT (500 milliseconds).
     *
     * @return The command timeout value in milliseconds.
     */
    uint32_t getCommandTimeout();
    /**
     * @brief Set the frame timeout - the time to wait between characters within a frame
     * (in ms)
     *
     * The modbus protocol defines that there can be no more than 1.5 characters of
     * silence between characters in a frame and any space over 3.5 characters defines a
     * new frame.
     *
     * By default, this is #MODBUS_FRAME_TIMEOUT (4 milliseconds).
     *
     * @param timeout The timeout value in milliseconds.
     */
    void setFrameTimeout(uint32_t timeout);
    /**
     * @brief Get the frame timeout - the time to wait between characters within a frame
     * (in ms)
     *
     * By default, this is #MODBUS_FRAME_TIMEOUT (4 milliseconds).
     *
     * @return The frame timeout value in milliseconds.
     */
    uint32_t getFrameTimeout();

    /**
     * @brief Set the number of times to retry a command before giving up
     *
     * By default, this is 10.
     *
     * @param retries The number of times to retry a command before giving up
     */
    void setCommandRetries(uint8_t retries);
    /**
     * @brief Get the current number of times to retry a command before giving up
     *
     * By default, this is 10.
     *
     * @return The number of times to retry a command before giving up
     */
    uint8_t getCommandRetries();
    /**
     * @brief Set the stream for communication
     *
     * The "stream" device must be initialized prior to running this.
     * Per modbus specifications, the stream must have:
     *    - 1 start bit
     *    - 8 data bits, least significant bit sent first
     *    - 1 stop bit if parity is used - 2 bits if no parity
     * Note that neither SoftwareSerial, AltSoftSerial, nor NeoSoftwareSerial
     * will support either even or odd parity!
     *
     * @param stream A pointer to the Arduino stream object to communicate with.
     */
    void setStream(Stream* stream);
    /**
     * @brief Set the stream for communication
     *
     * Per modbus specifications, the stream must have:
     *    - 1 start bit
     *    - 8 data bits, least significant bit sent first
     *    - 1 stop bit if parity is used - 2 bits if no parity
     * Note that neither SoftwareSerial, AltSoftSerial, nor NeoSoftwareSerial
     * will support either even or odd parity!
     *
     * @param stream A reference to the Arduino stream object to communicate with.
     */
    void setStream(Stream& stream);
    /**
     * @brief Get a pointer to the stream for communication
     *
     * @return A pointer to the Arduino stream object used for communication.
     */
    Stream* getStream();
    /**@}*/


    // ===================================================================== //
    /**
     * @anchor functions_by_datatype
     * @name Functions to get and set data for specific datatypes
     *
     * Common parameters for all register reading functions:
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The number of the **first** register of interest.
     */
    // ===================================================================== //
    /**@{*/

    // ===================================================================== //
    /**
     * @anchor uint16_t_functions
     * @name Functions to get and set unsigned 16-bit integer (uint16_t) data
     */
    // ===================================================================== //
    /**
     * @brief Get the numbered input or holding register and return it as a uint16_t.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The register number of interest.
     * @param endian The endianness of the uint16_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @return The uint16_t held in the register.
     */
    uint16_t uint16FromRegister(byte slaveId, byte regType, int regNum,
                                endianness endian = bigEndian);
    /**
     * @brief Set a holding register to a uint16_t.
     *
     * @param regNum The register number of interest.
     * @param value The value to set the register to.
     * @param endian The endianness used to write the uint16_t. Optional with a default
     * of big endian, which is required by modbus specifications.
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). Optional with a
     * default value of false.
     * @return True if the register was successfully set, false if not.
     */
    bool uint16ToRegister(byte slaveId, int regNum, uint16_t value,
                          endianness endian = bigEndian, bool forceMultiple = false);

    /**
     * @brief Read a uint16_t out of the response buffer frame.
     *
     * @param endian The endianness of the uint16_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the uint16_t in the response frame.
     * Optional with a default of 3.
     * @return The uint16_t held in the buffer frame.
     */
    uint16_t uint16FromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Insert a uint16_t into the working byte frame
     *
     * @param value The value to add to the frame.
     * @param endian The endianness used to write the uint16_t. Optional with a default
     * of big endian, which is required by modbus specifications.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the uint16_t in the response frame.
     * Optional with a default of 0.
     */
    void uint16ToFrame(uint16_t value, endianness endian, byte* destFrame,
                       int start_index = 0);


    // ===================================================================== //
    /**
     * @anchor int16_t_functions
     * @name Functions to get and set signed 16-bit integer (int16_t) data
     */
    // ===================================================================== //
    /**
     * @brief Get the numbered input or holding register and return it as an int16_t.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The register number of interest.
     * @param endian The endianness of the int16_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @return The int16_t held in the register.
     */
    int16_t int16FromRegister(byte slaveId, byte regType, int regNum,
                              endianness endian = bigEndian);
    /**
     * @brief Set a holding register to an int16_t.
     *
     * @param regNum The register number of interest.
     * @param value The value to set the register to.
     * @param endian The endianness used to write the int16_t. Optional with a default
     * of big endian, which is required by modbus specifications.
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). Optional with a
     * default value of false.
     * @return True if the register was successfully set, false if not.
     */
    bool int16ToRegister(byte slaveId, int regNum, int16_t value,
                         endianness endian = bigEndian, bool forceMultiple = false);

    /**
     * @brief Read an int16_t out of the response buffer frame.
     *
     * @param endian The endianness of the int16_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the int16_t in the response frame.
     * Optional with a default of 3.
     * @return The int16_t held in the buffer frame.
     */
    int16_t int16FromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Insert an int16_t into the working byte frame
     *
     * @param value The value to add to the frame.
     * @param endian The endianness used to write the int16_t. Optional with a default
     * of big endian, which is required by modbus specifications.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the int16_t in the response frame.
     * Optional with a default of 0.
     */
    void int16ToFrame(int16_t value, endianness endian, byte* destFrame,
                      int start_index = 0);


    // ===================================================================== //
    /**
     * @anchor float_functions
     * @name Functions to get and set 32-bit float data
     */
    // ===================================================================== //
    /**
     * @brief Get two input or holding registers starting at the specified number and
     * return them as a 32-bit float.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The number of the first of the two registers of interest.
     * @param endian The endianness of the 32-bit float in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return The 32 bit float held in the register.
     */
    float float32FromRegister(byte slaveId, byte regType, int regNum,
                              endianness endian = bigEndian);
    /**
     * @brief Set two holding registers to a 32-bit float
     *
     * @param regNum The number of first of the two registers of interest.
     * @param value The value to set the register to.
     * @param endian The endianness of the 32-bit float in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return True if the registers were successfully set, false if not.
     */
    bool float32ToRegister(byte slaveId, int regNum, float value,
                           endianness endian = bigEndian);

    /**
     * @brief Read a 32-bit float out of the response buffer frame.
     *
     * @param endian The endianness of the 32-bit float in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the 32-bit float in the response
     * frame. Optional with a default of 3.
     * @return The 32-bit float held in the buffer frame.
     */
    float float32FromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Insert a 32-bit float into the working byte frame
     *
     * @param value The value to add to the frame.
     * @param endian The endianness used to write the 32-bit float. Optional with a
     * default of big endian, which is required by modbus specifications. Only big and
     * little endian are supported. Mixed endianness is *NOT* supported.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the 32-bit float in the response
     * frame. Optional with a default of 0.
     */
    void float32ToFrame(float value, endianness endian, byte* destFrame,
                        int start_index = 0);


    // ===================================================================== //
    /**
     * @anchor uint32_t_functions
     * @name Functions to get and set unsigned 32-bit integer (uint32_t) data
     */
    // ===================================================================== //
    /**
     * @brief Get two input or holding registers starting at the specified number and
     * return them as a uint32_t
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The number of the first of the two registers of interest.
     * @param endian The endianness of the uint32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return The uint32_t held in the register.
     */
    uint32_t uint32FromRegister(byte slaveId, byte regType, int regNum,
                                endianness endian = bigEndian);
    /**
     * @brief Set two holding registers to a uint32_t
     *
     * @param regNum The number of first of the two registers of interest.
     * @param value The value to set the register to.
     * @param endian The endianness of the uint32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return True if the registers were successfully set, false if not.
     */
    bool uint32ToRegister(byte slaveId, int regNum, uint32_t value,
                          endianness endian = bigEndian);
    /**
     * @brief Insert a uint32_t into the working byte frame
     *
     * @param value The value to add to the frame.
     * @param endian The endianness used to write the uint32_t. Optional with a default
     * of big endian, which is required by modbus specifications. Only big and little
     * endian are supported. Mixed endianness is *NOT* supported.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the uint32_t in the response frame.
     * Optional with a default of 0.
     */
    void uint32ToFrame(uint32_t value, endianness endian, byte* destFrame,
                       int start_index = 0);
    /**
     * @brief Read a uint32_t out of the response buffer frame.
     *
     * @param endian The endianness of the uint32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the uint32_t in the response frame.
     * Optional with a default of 3.
     * @return The uint32_t held in the buffer frame.
     */
    uint32_t uint32FromFrame(endianness endian = bigEndian, int start_index = 3);


    // ===================================================================== //
    /**
     * @anchor int32_t_functions
     * @name Functions to get and set signed 32-bit integer (int32_t) data
     */
    // ===================================================================== //
    /**
     * @brief Get two input or holding registers starting at the specified number and
     * return them as an int32_t
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The number of the first of the two registers of interest.
     * @param endian The endianness of the int32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return The int32_t held in the register.
     */
    int32_t int32FromRegister(byte slaveId, byte regType, int regNum,
                              endianness endian = bigEndian);
    /**
     * @brief Set two holding registers to an int32_t
     *
     * @param regNum The number of first of the two registers of interest.
     * @param value The value to set the register to.
     * @param endian The endianness of the int32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return True if the registers were successfully set, false if not.
     */
    bool int32ToRegister(byte slaveId, int regNum, int32_t value,
                         endianness endian = bigEndian);

    /**
     * @brief Read an int32_t out of the response buffer frame.
     *
     * @param endian The endianness of the int32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the int32_t in the response frame.
     * Optional with a default of 3.
     * @return The int32_t held in the buffer frame.
     */
    int32_t int32FromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Insert an int32_t into the working byte frame
     *
     * @param value The value to add to the frame.
     * @param endian The endianness used to write the int32_t. Optional with a default
     * of big endian, which is required by modbus specifications. Only big and little
     * endian are supported. Mixed endianness is *NOT* supported.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the int32_t in the response frame.
     * Optional with a default of 0.
     */
    void int32ToFrame(int32_t value, endianness endian, byte* destFrame,
                      int start_index = 0);


    // ===================================================================== //
    /**
     * @anchor byte_functions
     * @name Functions to get and set bytes (half registers)
     */
    // ===================================================================== //
    /**
     * @brief Get the numbered input or holding register and return one byte of it.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The register number of interest.
     * @param byteNum The byte number to return (1 for upper or 2 for lower)
     * @return The byte held in the register.
     */
    byte byteFromRegister(byte slaveId, byte regType, int regNum, int byteNum);
    /**
     * @brief Set one byte of a holding register.
     *
     * The byte will be inserted as a full 16-bit register with the unused byte set to
     * 0.
     *
     * @param regNum The register number of interest.
     * @param byteNum The byte number to set (1 for upper or 2 for lower)
     * @param value The value to set the byte to.
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). Optional with a
     * default value of false.
     * @return True if the register was successfully set, false if not.
     */
    bool byteToRegister(byte slaveId, int regNum, int byteNum, byte value,
                        bool forceMultiple = false);

    /**
     * @brief Read a single byte out of the response buffer frame.
     *
     * @param start_index The starting position of the byte in the response frame.
     * Optional with a default of 3.
     * @return The byte held in the buffer frame.
     */
    byte byteFromFrame(int start_index = 3);
    /**
     * @brief Insert a single byte into the working byte frame.
     *
     * The byte will be inserted as a 16-bit value with the unused byte set to 0.
     *
     * @param value The byte to write
     * @param byteNum The byte number to set (1 for upper or 2 for lower)
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the byte in the response frame.
     * Optional with a default of 0.
     */
    void byteToFrame(byte value, int byteNum, byte* destFrame, int start_index = 0);


    // ===================================================================== //
    /**
     * @anchor tai64_format
     * @name The TAI64 timestamp format
     *
     * Within this library, the support for TAI64, TAI64N and TAI64NA has these
     * limitations:
     *  - The time value must be in four (TAI64), six (TAI64N), or eight (TAI64NA)
     * contiguous 16-bit registers
     *  - The time value must always be fully big endian
     *  - The full 64-bit timestamp is cropped to 32-bits and only the lower 32-bits are
     * used as if it was a 32-bit unix timestamp.
     *    - The upper 32-bits of the TAI64 timestamp will be 0x40000000 until the year
     * 2106.
     *
     * > TAI stands for Temps Atomique International, the current international
     * > real-time standard.
     * >
     * > TAI64 defines a 64-bit integer format where each value identifies a particular
     * > SI second. The duration of SI seconds is defined through a count of state
     * > transitions of the cesium atom and understood as constant. Time is structured
     * > as a sequence of seconds anchored on the start of the year 1970 in the
     * > Gregorian calendar, when atomic time (TAI) became the international standard
     * > for real time. The standard defines 262 seconds before the year 1970, and
     * > another 262 from this epoch onward, thus covering a span of roughly 300 billion
     * > years, enough for most applications.
     *
     * > The extensions TAI64N and TAI64NA allow for finer time resolutions by
     * > referring to particular nanoseconds and attoseconds (10-18 s), respectively,
     * > within a particular second.
     *
     * The defining paper: [Toward a Unified Timestamp with explicit
     * precision](https://www.demographic-research.org/volumes/vol12/6/12-6.pdf)
     *
     * [An excellent (and easier to read) explanation of the TAI64 formats on Stack
     * Overflow](https://stackoverflow.com/questions/50907211/what-is-a-tai64-time-format)
     */
    // ===================================================================== //
    /**
     * @anchor tai64_functions
     * @name Functions to get and set 64-bit timestamp (TAI64) data
     *
     * Common parameters for TAI64 reading functions:
     *
     * @param nanoseconds A reference to another uint32_t to populate with the
     * nanoseconds - for TAI64N and TAI64NA
     * @param attoseconds A reference to another uint32_t to populate with the
     * attoseconds - for TAI64NA
     * @return The lower 32 bits of the TAI64 timestamp.
     *
     * Common parameters for TAI64 writing functions:
     *
     * @param seconds The lower 32-bits of the timestamp. The upper 32-bits will always
     * be set to 0x40000000, which will be the correct value until the year 2106.
     * @param nanoseconds The 32-bit nanosecond count.
     * @param attoseconds The 32-bit attoseconds count.
     */
    // ===================================================================== //
    /**
     * @brief Get four input or holding registers starting at the specified number,
     * convert them to a TAI64 (64-bit timestamp), and return the lower 32-bits as a
     * unix timestamp.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The number of the first of the four registers of interest.
     * @return The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64FromRegister(byte slaveId, byte regType, int regNum);
    /**
     * @brief Get six input or holding registers starting at the specified number,
     * convert them to a TAI64N (64-bit timestamp followed by a 32-bit nanosecond
     * count), and return an equivalent 32-bits unix timestamp.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The number of the first of the six registers of interest.
     * @param nanoseconds A reference to another uint32_t to populate with the
     * nanoseconds.
     * @return The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64NFromRegister(byte slaveId, byte regType, int regNum,
                                uint32_t& nanoseconds);
    /**
     * @brief Get eight input or holding registers starting at the specified number,
     * convert them to a TAI64NA (64-bit timestamp followed by a 32-bit nanosecond count
     * and then a 32-bit attosecond count), and return an equivalent 32-bits unix
     * timestamp.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The number of the first of the eight registers of interest.
     * @param nanoseconds A reference to another uint32_t to populate with the
     * nanoseconds.
     * @param attoseconds A reference to another uint32_t to populate with the
     * attoseconds.
     * @return The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64NAFromRegister(byte slaveId, byte regType, int regNum,
                                 uint32_t& nanoseconds, uint32_t& attoseconds);
    /**
     * @brief Set four holding registers to a TAI64 (64-bit timestamp)
     *
     * @param regNum The number of first of the four registers of interest.
     * @param seconds The lower 32-bits of the timestamp. The upper 32-bits will always
     * be set to 0x40000000, which will be the correct value until the year 2106.
     * @return True if the registers were successfully set, false if not.
     */
    bool TAI64ToRegister(byte slaveId, int regNum, uint32_t seconds);
    /**
     * @brief Set six holding registers to a TAI64N (64-bit timestamp followed by a
     * 32-bit nanosecond count)
     *
     * @param regNum The number of first of the six registers of interest.
     * @param seconds The lower 32-bits of the timestamp. The upper 32-bits will always
     * be set to 0x40000000, which will be the correct value until the year 2106.
     * @param nanoseconds The 32-bit nanosecond count.
     * @return True if the registers were successfully set, false if not.
     */
    bool TAI64NToRegister(byte slaveId, int regNum, uint32_t seconds,
                          uint32_t nanoseconds);
    /**
     * @brief Set eight holding registers to a TAI64NA (64-bit timestamp followed by a
     * 32-bit nanosecond count and then a 32-bit attosecond count)
     *
     * @param regNum The number of first of the eight registers of interest.
     * @param seconds The lower 32-bits of the timestamp. The upper 32-bits will always
     * be set to 0x40000000, which will be the correct value until the year 2106.
     * @param nanoseconds The 32-bit nanosecond count.
     * @param attoseconds The 32-bit attoseconds count.
     * @return True if the registers were successfully set, false if not.
     */
    bool TAI64NAToRegister(byte slaveId, int regNum, uint32_t seconds,
                           uint32_t nanoseconds, uint32_t attoseconds);

    /**
     * @brief Read a TAI64 (64-bit timestamp) out of the response buffer frame and
     * return the lower 32-bits as a unix timestamp.
     *
     * @param start_index The starting position of the TAI64 in the response frame.
     * Optional with a default of 3.
     * @return The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64FromFrame(int start_index = 3);
    /**
     * @brief Read a TAI64N (64-bit timestamp followed by a 32-bit nanosecond count) out
     * of the response buffer frame and return an equivalent 32-bits unix timestamp.
     *
     * @param nanoseconds A reference to another uint32_t to populate with the
     * nanoseconds.
     * @param start_index The starting position of the TAI64N in the response frame.
     * Optional with a default of 3.
     * @return The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64NFromFrame(uint32_t& nanoseconds, int start_index = 3);
    /**
     * @brief Read a TAI64NA (64-bit timestamp followed by a 32-bit nanosecond count and
     * then a 32-bit attosecond count) out of the response buffer frame and return an
     * equivalent 32-bits unix timestamp.
     *
     * @param nanoseconds A reference to another uint32_t to populate with the
     * nanoseconds.
     * @param attoseconds A reference to another uint32_t to populate with the
     * attoseconds.
     * @param start_index The starting position of the TAI64NA in the response frame.
     * Optional with a default of 3.
     * @return The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64NAFromFrame(uint32_t& nanoseconds, uint32_t& attoseconds,
                              int start_index = 3);
    /**
     * @brief Insert a TAI64 (64-bit timestamp) into the working byte frame
     *
     * @param seconds The lower 32-bits of the timestamp. The upper 32-bits will always
     * be set to 0x40000000, which will be the correct value until the year 2106.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the TAI64 in the response frame.
     * Optional with a default of 0.
     */
    void TAI64ToFrame(uint32_t seconds, byte* destFrame, int start_index = 0);
    /**
     * @brief Insert a TAI64N (64-bit timestamp followed by a 32-bit nanosecond count)
     * into the working byte frame
     *
     * @param seconds The lower 32-bits of the timestamp. The upper 32-bits will always
     * be set to 0x40000000, which will be the correct value until the year 2106.
     * @param nanoseconds The 32-bit nanosecond count.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the TAI64N in the response frame.
     * Optional with a default of 0.
     */
    void TAI64NToFrame(uint32_t seconds, uint32_t nanoseconds, byte* destFrame,
                       int start_index = 0);
    /**
     * @brief Insert a TAI64NA (64-bit timestamp followed by a 2-bit nanosecond count
     * and then a 32-bit attosecond count) into the working byte frame
     *
     * @param seconds The lower 32-bits of the timestamp. The upper 32-bits will always
     * be set to 0x40000000, which will be the correct value until the year 2106.
     * @param nanoseconds The 32-bit nanosecond count.
     * @param attoseconds The 32-bit attoseconds count.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the TAI64N in the response frame.
     * Optional with a default of 0.
     */
    void TAI64NAToFrame(uint32_t seconds, uint32_t nanoseconds, uint32_t attoseconds,
                        byte* destFrame, int start_index = 0);


    // ===================================================================== //
    /**
     * @anchor pointer_functions
     * @name Functions to get and set pointers to other registers
     */
    // ===================================================================== //
    /**
     * @brief Get the numbered input or holding register and return it as an 16-bit
     * pointer.
     *
     * This should be a pointer to another registry address within the modbus registers.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The register number of interest.
     * @param endian The endianness of the 16-bit pointer in the modbus register.
     * Optional with a default of big endian, which is required by modbus
     * specifications.
     * @return The 16-bit pointer held in the register.
     */
    uint16_t pointerFromRegister(byte slaveId, byte regType, int regNum,
                                 endianness endian = bigEndian);
    /**
     * @brief Get the numbered input or holding register and return it as a 8-bit
     * pointer type.
     *
     * This should be the type of register pointed to by pointer contained within a
     * different modbus register.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The register number of interest.
     * @param endian The endianness of the pointer type in the modbus register.
     * Optional with a default of big endian, which is required by modbus
     * specifications.
     * @return The 8-bit pointer type held in the register. This will be an
     * object of type #pointerType.
     */
    int8_t pointerTypeFromRegister(byte slaveId, byte regType, int regNum,
                                   endianness endian = bigEndian);
    /**
     * @brief Set a holding register to a 16-bit pointer.
     *
     * @param regNum The register number of interest.
     * @param value The value to set the register to.
     * @param point The type of the pointer, (#pointerType) ie, which section of the
     * modbus memory is being pointed to.
     * @param endian The endianness used to write the 16-bit pointer. Optional with a
     * default of big endian, which is required by modbus specifications.
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). Optional with a
     * default value of false.
     * @return True if the register was successfully set, false if not.
     */
    bool pointerToRegister(byte slaveId, int regNum, uint16_t value, pointerType point,
                           endianness endian = bigEndian, bool forceMultiple = false);

    /**
     * @brief Read a 16-bit pointer out of the response buffer frame.
     *
     * This should be a pointer to another registry address within the modbus registers.
     *
     * @param endian The endianness of the 16-bit pointer in the modbus register.
     * Optional with a default of big endian, which is required by modbus
     * specifications.
     * @param start_index The starting position of the 16-bit pointer in the response
     * frame. Optional with a default of 3.
     * @return The 16-bit pointer held in the buffer frame.
     */
    uint16_t pointerFromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Read a 16-bit pointer out of the response buffer frame.
     *
     * This should be the type of register pointed to by pointer contained within a
     * different modbus register.
     *
     * @param endian The endianness of the pointer type in the modbus register.
     * Optional with a default of big endian, which is required by modbus
     * specifications.
     * @param start_index The starting position of the 16-bit pointer in the response
     * frame. Optional with a default of 3.
     * @return The 8-bit pointer type held in the buffer frame. This will be an
     * object of type #pointerType.
     */
    int8_t pointerTypeFromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Insert a 16-bit pointer into the working byte frame.
     *
     * @param value The value of the 16-bit pointer
     * @param point The type of the pointer, (#pointerType) ie, which section of the
     * modbus memory is being pointed to.
     * @param endian The endianness used to write the 16-bit pointer. Optional with a
     * default of big endian, which is required by modbus specifications.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the byte in the response frame.
     * Optional with a default of 0.
     */
    void pointerToFrame(uint16_t value, pointerType point, endianness endian,
                        byte* destFrame, int start_index = 0);


    // ===================================================================== //
    /**
     * @anchor String_functions
     * @name Functions to get and set Strings
     *
     * @note These are for Arduino String objects, not simple character arrays.
     */
    // ===================================================================== //
    /**
     * @brief Get a group of input or holding registers, convert them to characters,
     * combine them, and return a single String.
     *
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The number of the first of the registers of interest.
     * @param charLength The number of characters to return. NOTE: There are *TWO*
     * characters per register!
     * @return The text from the registers.
     */
    String StringFromRegister(byte slaveId, byte regType, int regNum, int charLength);
    /**
     * @brief Set a series of holding registers to the characters in a String.
     *
     * @param regNum The first of the registers of interest
     * @param value The String to set the registers to.
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). This only
     * applies if the String is two characters or less. Optional with a default value of
     * false.
     * @return True if the registers were successfully set, false if not.
     */
    bool StringToRegister(byte slaveId, int regNum, String value,
                          bool forceMultiple = false);

    /**
     * @brief Read a String out of the response buffer frame.
     *
     * @param charLength The number of characters to return.
     * @param start_index The starting position of the characters in the response
     * frame. Optional with a default of 3.
     * @return The text from the registers.
     */
    String StringFromFrame(int charLength, int start_index = 3);
    /**
     * @brief Insert a String into the working byte frame.
     *
     * @param value The string to insert.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the byte in the response frame.
     * Optional with a default of 0.
     */
    void StringToFrame(String value, byte* destFrame, int start_index = 0);


    // ===================================================================== //
    /**
     * @anchor character_array_functions
     * @name Functions to get and set character arrays
     *
     * @note These are for simple character arrays, not Arduino String objects.
     */
    // ===================================================================== //
    /**
     * @brief Get a group of input or holding registers, convert them to characters and
     * put them into the given character array.
     *
     * There is no return from this function.
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param regType The register type; use 0x03 for a holding register (read/write) or
     * 0x04 for an input register (read only)
     * @param regNum The number of the first of the registers of interest.
     * @param outChar A pointer or constant pointer to a character array to fill with the content of the registers.
     * @param charLength The number of characters to return. NOTE: There are *TWO*
     * characters per register!
     */
    void charFromRegister(byte slaveId, byte regType, int regNum, char* outChar,
                          int charLength);
    /// @copydoc charFromRegister(byte, byte, int, char[], int)
    void charFromRegister(byte slaveId, byte regType, int regNum, const char* outChar,
                          int charLength);
    /**
     * @brief Set a series of holding registers to the characters in a character array.
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param regNum The first of the registers of interest
     * @param inChar A pointer or constant pointer to the character array to set the
     * registers to.
     * @param charLength The number of characters to set from in the array.
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). This only
     * applies if the character array is two characters or less. Optional with a default
     * value of false.
     * @return True if the registers were successfully set, false if not.
     */
    bool charToRegister(byte slaveId, int regNum, char* inChar, int charLength,
                        bool forceMultiple = false);
    /// @copydoc charToRegister(byte, int, char[], int, bool)
    bool charToRegister(byte slaveId, int regNum, const char* inChar, int charLength,
                        bool forceMultiple = false);

    /**
     * @brief Read characters out of the response buffer frame and put them into a
     * character array.
     *
     * There is no return from this function.
     *
     * @param outChar A character array to fill with the content of the response buffer
     * (or a pointer or constant pointer to one).
     * @param charLength The number of characters to return.
     * @param start_index The starting position of the characters in the response
     * frame. Optional with a default of 3.
     */
    void charFromFrame(char outChar[], int charLength, int start_index = 3);
    /// @copydoc charFromFrame(char[], int, int)
    void charFromFrame(char* outChar, int charLength, int start_index = 3);
    /// @copydoc charFromFrame(char[], int, int)
    void charFromFrame(const char* outChar, int charLength, int start_index = 3);
    /**
     * @brief Insert a character array into the working byte frame.
     *
     * @param inChar A pointer or constant pointer to the character array to set the
     * registers to.
     * @param charLength The number of characters to copy from in the array.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the byte in the response frame.
     * Optional with a default of 0.
     */
    void charToFrame(char* inChar, int charLength, byte* destFrame,
                     int start_index = 0);
    /// @copydoc charToFrame(char[], int, byte*, int)
    void charToFrame(const char* inChar, int charLength, byte* destFrame,
                     int start_index = 0);
    /**@}*/


    // ===================================================================== //
    /**
     * @anchor data_getters
     * @name Data Getters
     *
     * @brief Functions to get data from one or more registers, coils, or discrete
     * inputs.
     *
     * @remark If you do not supply an output buffer, these commands put data into the
     * internal library buffer. With the exception of the commands to get a single coil
     * or a single discrete output, these functions do *not* return the data directly.
     *
     * @note These commands put only the **content of the registers** (or coils or
     * inputs) into the buffer. They do **not** add the full returned modbus frame.  The
     * data in the buffer will be stripped of the modbus protocol bytes - including the
     * address, command number, CRC, and any error codes. If there was no problem with
     * the response, these functions return true.  If one of these functions return
     * false and you need to check for an error code, call the getLastError() function
     * immediately after the command.
     */
    // ===================================================================== //
    /**@{*/

    /**
     * @brief Get data from either holding or input registers and copy the output to the
     * supplied buffer.
     *
     * @note This command puts only the **content of the registers** into the buffers.
     * It does **not** add the full returned modbus frame.  The data in the buffer will
     * be stripped of the modbus protocol characters.
     *
     * @remark No more than 125 registers can be read at once.
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param readCommand The command to use to read data. For a holding register
     * readCommand = 0x03. For an input register readCommand = 0x04.
     * @param startRegister The starting register number.
     * @param numRegisters The number of registers to read.
     * @param buff The buffer to copy the output data to.
     * @return Zero if the response didn't return the expected number of bytes or if
     * there was an error in the modbus response; otherwise, the number of bytes in the
     * response.
     */
    int16_t getRegisters(byte slaveId, byte readCommand, int16_t startRegister,
                         int16_t numRegisters, byte* buff);
    /**
     * @brief Get the status of a single output coil
     *
     * The read command for output coils is 0x01.
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param coilAddress The address of the coil to read.
     * @return The status of the coil (true for ON, false for OFF).
     */
    bool getCoil(byte slaveId, int16_t coilAddress);
    /**
     * @brief Get the data from a range of output coils. Each coil is a single bit.
     *
     * The read command for output coils is 0x01.
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param startCoil The starting coil number.
     * @param numCoils The number of coils to read.
     * @param buff A pre-allocated buffer to store the retrieved coil values.
     * @return Zero if the response didn't return the expected number of bytes or if
     * there was an error in the modbus response; otherwise, the number of bytes in the
     * response.
     */
    int16_t getCoils(byte slaveId, int16_t startCoil, int16_t numCoils, byte* buff);
    /**
     * @brief Get the status of a single discrete input
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param inputAddress The address of the discrete input to read.
     * @return The status of the discrete input (true for ON, false for OFF).
     */
    bool getDiscreteInput(byte slaveId, int16_t inputAddress);
    /**
     * @brief Get a range of discrete inputs
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param startInput The starting input number.
     * @param numInputs The number of discrete inputs to read.
     * @param buff A pre-allocated buffer to store the retrieved coil values.
     * @return Zero if the response didn't return the expected number of bytes or if
     * there was an error in the modbus response; otherwise, the number of bytes in the
     * response.
     */
    int16_t getDiscreteInputs(byte slaveId, int16_t startInput, int16_t numInputs,
                              byte* buff);
    /**@}*/


    // ===================================================================== //
    /**
     * @anchor data_setters
     * @name Data Setters
     *
     * @brief Functions to set data to one or more holding registers or coils.
     *
     * @remark If you do not supply an input buffer, these commands write data from the
     * internal library buffer.
     *
     * @note The data in the input buffer should *not* contain any modbus
     * protocol bytes - including the address, command number or CRC. If there was no
     * problem with the response on setting values, these functions return true.  If one
     * of these functions return false and you need to check for an error code, call the
     * getLastError() function immediately after the command.
     */
    // ===================================================================== //
    /**@{*/
    /**
     * @brief Set the value of one or more holding registers using Modbus commands 0x06
     * or 0x10 (16).
     *
     * Input registers cannot be written by a Modbus controller/master
     *
     * @remark No more than 123 registers can be set at once.
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param startRegister The starting register number.
     * @param numRegisters The number of registers to write.
     * @param value A pointer to the byte array with the values to write.
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). This only
     * applies if a single register is being set. Optional with a default value of
     * false.
     * @return True if the modbus slave returned the expected number of input
     * values; false if there was a failure.
     */
    bool setRegisters(byte slaveId, int16_t startRegister, int16_t numRegisters,
                      byte* value, bool forceMultiple = false);

    /**
     * @brief Set the value of a single output coil using Modbus command 0x05.
     *
     * Output coils are single-bit values that can be either ON (1) or OFF (0).
     * Input (discrete) contacts cannot be written by a Modbus controller/master.
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param coilAddress The address of the coil to set.
     * @param value The value to set the coil to (true for ON, false for OFF).
     * @return True if the proper modbus slave correctly responded to the command; false
     * otherwise.
     */
    bool setCoil(byte slaveId, int16_t coilAddress, bool value);

    /**
     * @brief Set the value of one or more output coils using modbus command 0x0F
     *
     * Input (discrete) contacts cannot be written by a Modbus controller/master.
     *
     * @note This function always uses Modbus command 0x0F
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param startCoil The address of the first coil to set.
     * @param numCoils The number of coils to set.
     * @param value A pointer to a byte array containing the values to set the coils to.
     * @return True if the proper modbus slave correctly responded to the command; false
     * otherwise.
     */
    bool setCoils(byte slaveId, int16_t startCoil, int16_t numCoils, byte* value);
    /**@}*/


    // ===================================================================== //
    /**
     * @anchor low_level_functions
     * @name Low level functions
     */
    // ===================================================================== //
    /**@{*/
    /**
     * @brief A generic get data function that can be used for any data type or size
     *
     * Use these commands:
     * - For a coil readCommand = 0x01
     * - For a discrete input readCommand = 0x02
     * - For a holding register readCommand = 0x03
     * - For an input register readCommand = 0x04.
     *
     * A register command will return 2 bytes per register - the size should be number
     * registers x 2.
     *
     * A coil or discrete input command will return 1 bit per coil - the size should be
     * number coils / 8.
     *
     * @note You could use this function for other uncommon modbus commands (ie, get
     * diagnostics). To do so:
     * - set the readCommand parameter to the appropriate command code
     * - set the startAddress to the first two bytes of the command (ie, sub-function
     * High and sub-function Low)
     * - set the numChunks to the next two bytes of the command (ie, Data High and Data
     * Low)
     * - set the expectedReturnBytes to the expected size of the response (highly
     * variable)
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param readCommand The command to use to read data
     * @param startAddress The first address to read from
     * @param numChunks The number of chunks of data to read
     * @param expectedReturnBytes The expected return size in bytes- set to 0 to have
     * the size calculated based on the command and the number of chunks requested.
     * @return Zero if the response didn't return the expected number of bytes or if
     * there was an error in the modbus response; otherwise, the number of bytes in the
     * response.
     */
    int16_t getModbusData(byte slaveId, byte readCommand, int16_t startAddress,
                          int16_t numChunks, uint8_t expectedReturnBytes = 0);


    /**
     * @brief Send a command to the modbus slave.
     *
     * This is the lowest level function.
     *
     * This takes a command, adds the proper CRC, sends it to the sensor bus, and
     * listens for a response.
     *
     * If it receives a response from the correct slave with the correct CRC, it returns
     * the number of bytes received and put into the responseBuffer.
     *
     * If it receives a response from the wrong slave, an incorrect CRC, or an
     * exception, it will print notice of the error to the debugging stream and return
     * 0xn0FF where n is the error code.
     *
     * If no response is received, this returns 0.
     *
     * @note The maximum response size for a Modbus RTU frame is 256 bytes (125
     * registers plus overhead).  If you get a return value of >256, it means there
     * was an error and you should parse the error code.
     *
     * @param slaveId The modbus slave ID to use in the request
     * @param command The fully formed command to send to the Modbus slave.
     * @param commandLength The length of the outgoing command.
     * @return The number of bytes received from the Modbus slave.
     */
    uint16_t sendCommand(byte slaveId, byte* command, int commandLength);
    /**@}*/

    // ===================================================================== //
    /**
     * @anchor debugging_functions
     * @name Debugging functions
     *
     * These are purely debugging functions to print out the raw hex data sent between
     * the Arduino and the modbus slave.
     */
    // ===================================================================== //
    /**@{*/
    /**
     * @brief Set a stream for debugging information to go to.
     * @param stream An Arduino stream object
     */
    void setDebugStream(Stream* stream) {
        _debugStream = stream;
    }
    /// @copydoc modbusMaster::setDebugStream(Stream*)
    void setDebugStream(Stream& stream) {
        _debugStream = &stream;
    }

    /**
     * @brief Un-set the stream for debugging information to go to; stop debugging.
     */
    void stopDebugging(void) {
        _debugStream = nullptr;
    }
    /**@}*/

    // ===================================================================== //
    /**
     * @anchor error_functions
     * @name Error functions
     *
     * Functions to monitor the error codes.
     */
    // ===================================================================== //
    /**@{*/
    /**
     * @brief Get last modbus error code
     * @return The last modbus error code
     */
    modbusErrorCode getLastError(void) {
        return lastError;
    }

    /**
     * @brief Prints information about the last error to the debugging stream
     * @note If there is not a debugging stream set, this function will have no effect.
     */
    void printLastError(void);
    /**@}*/

    // ===================================================================== //
    /**
     * @anchor internal_buffers
     * @name Internal Buffers
     */
    // ===================================================================== //
    /**@{*/
    /**
     * @brief The response buffer for incoming messages from the Modbus slave.
     */
    static byte responseBuffer[RESPONSE_BUFFER_SIZE];

    /**
     * @brief The command buffer for outgoing messages to the Modbus slave.
     */
    static byte commandBuffer[COMMAND_BUFFER_SIZE];
    /**@}*/


    //----------------------------------------------------------------------------
    //                            PRIVATE FUNCTIONS
    //----------------------------------------------------------------------------

 private:

    /**
     * @brief This flips the device/receive enable to DRIVER so the arduino can send
     * text
     */
    void driverEnable(void);

    /**
     * @brief This flips the device/receive enable to RECEIVER so the sensor can send
     * text
     */
    void receiverEnable(void);

    /**
     * @brief This empties the serial buffer
     *
     * @param stream The Arduino stream to dump data from
     */
    void emptySerialBuffer(Stream* stream);

    /**
     * @brief A function for prettily printing raw modbus RTU frames
     *
     * @param modbusFrame The modbus frame to print
     * @param frameLength The length of the frame to print
     */
    void printFrameHex(byte* modbusFrame, int frameLength);

    /**
     * @brief Calculates a Modbus RTC cyclical redundancy code (CRC)
     *
     * @param modbusFrame The modbus frame to calculate the CRC from
     * @param frameLength The length of the frame
     */
    void calculateCRC(byte* modbusFrame, int frameLength);

    /**
     * @brief Adds the CRC to a modbus RTU frame
     *
     * @param modbusFrame The modbus frame to add the CRC to
     * @param frameLength The length of the frame
     */
    void insertCRC(byte* modbusFrame, int frameLength);

    /**
     * @brief This slices one array out of another
     *
     * @param inputArray The source array
     * @param outputArray The destination array
     * @param start_index The starting position to copy from
     * @param numBytes The number of bytes to copy
     * @param reverseOrder Whether to reverse the order of bytes when copying the data.
     * Optional with a default value of false.
     */
    void sliceArray(byte inputArray[], byte outputArray[], int start_index,
                    int numBytes, bool reverseOrder = false);

    /**
     * @brief This converts data in a modbus RTU frame into a little-endian data frame.
     *
     * Little-endian frames data are needed because all Arduino processors are
     * little-endian
     *
     * @param varLength The length of the variable to convert the endianness of.
     * @param endian The starting endianness; optional with a default value of
     * bigEndian.
     * @param start_index The starting position of the variable in the frame. Optional
     * with a default value of 3.
     * @return A frame object with the converted endianness.
     */
    leFrame leFrameFromFrame(int varLength, endianness endian = bigEndian,
                             int start_index = 3);

    // Utility templates for writing to the debugging stream
    template <typename T>
    inline void debugPrint(T last) {
        if (_debugStream != nullptr) { _debugStream->print(last); }
    }

    template <typename T, typename... Args>
    inline void debugPrint(T head, Args... tail) {
        if (_debugStream != nullptr) {
            _debugStream->print(head);
            this->debugPrint(tail...);
        }
    }

    /**
     * @brief The stream instance (serial port) for communication with the Modbus slave
     * (usually over RS485)
     */
    Stream* _stream;
    /**
     * @brief The pin controlling the driver/receiver enable on the RS485-to-TLL chip,
     * if applicable.
     */
    int8_t _enablePin;
    /**
     * @brief The stream instance (serial port) for debugging
     */
    Stream* _debugStream;

    /**
     * @brief A temporary working buffer to store CRC calculation results.
     */
    static byte crcFrame[2];

    /**
     * @brief The time to wait for response after a command (in ms)
     */
    uint32_t modbusTimeout = MODBUS_TIMEOUT;
    /**
     * @brief The time to wait between characters within a frame (in ms)
     *
     * The modbus protocol defines that there can be no more than 1.5 characters of
     * silence between characters in a frame and any space over 3.5 characters defines a
     * new frame.
     */
    int modbusFrameTimeout = MODBUS_FRAME_TIMEOUT;

    /**
     * @brief The number of times to retry a command before giving up
     */
    uint8_t commandRetries = 10;

    /**
     * @brief The last error code returned by the modbus command
     */
    modbusErrorCode lastError = NO_ERROR;
};

#endif
