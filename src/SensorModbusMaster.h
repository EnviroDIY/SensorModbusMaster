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
 * This needs to be bigger than the largest response.
 * Per the Specification and Implementation Guide for MODBUS over serial line, this
 * value is 256 bytes.  (This translates into a maximum of 125 registers to be read via
 * Modbus/RTU and 123 by Modbus/TCP.)
 *
 * If you know you will never make any modbus calls longer than this, decrease this
 * number to save memory space.
 *
 */
#define RESPONSE_BUFFER_SIZE 256
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
#define UINT16_SIZE 2   ///< The size of an uint16_t in bytes
#define INT16_SIZE 2    ///< The size of an int16_t in bytes
#define UINT32_SIZE 4   ///< The size of an uint32_t in bytes
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
     * @brief This function sets up the communication with the Arduino stream connected
     * to the modbus device.
     *
     * It should be run during the arduino "setup" function.
     * The "stream" device must be initialized prior to running this.
     * Per modbus specifications, the stream must have:
     *    - 1 start bit
     *    - 8 data bits, least significant bit sent first
     *    - 1 stop bit if parity is used - 2 bits if no parity
     * Note that neither SoftwareSerial, AltSoftSerial, nor NeoSoftwareSerial
     * will support either even or odd parity!
     *
     * @param modbusSlaveID The byte identifier of the modbus slave device.
     * @param stream A pointer to the Arduino stream object to communicate with.
     * @param enablePin A pin on the Arduino processor to use to send an enable signal
     * to an RS485 to TTL adapter. Use a negative number if this does not apply.
     * Optional with a default value of -1.
     * @return True if the starting communication was successful, false if not.
     */
    bool begin(byte modbusSlaveID, Stream* stream, int8_t enablePin = -1);

    /**
     * @brief This function sets up the communication with the Arduino stream connected
     * to the modbus device.
     *
     * It should be run during the arduino "setup" function.
     * The "stream" device must be initialized prior to running this.
     * Per modbus specifications, the stream must have:
     *    - 1 start bit
     *    - 8 data bits, least significant bit sent first
     *    - 1 stop bit if parity is used - 2 bits if no parity
     * Note that neither SoftwareSerial, AltSoftSerial, nor NeoSoftwareSerial
     * will support either even or odd parity!
     *
     * @param modbusSlaveID The byte identifier of the modbus slave device.
     * @param stream A reference to the Arduino stream object to communicate with.
     * @param enablePin A pin on the Arduino processor to use to send an enable signal
     * to an RS485 to TTL adapter. Use a negative number if this does not apply.
     * Optional with a default value of -1.
     * @return True if the starting communication was successful, false if not.
     */
    bool begin(byte modbusSlaveID, Stream& stream, int8_t enablePin = -1);
    /**@}*/


    // ===================================================================== //
    /**
     * @anchor high_level_getters
     * @name High level register fetching functions
     *
     * These higher-level functions return a variety of data from a single or group of
     * input or holding registers
     */
    /**@{*/
    // ===================================================================== //
    /**
     * @brief Get the numbered input or holding register and return it as an uint16_t.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The register number of interest.
     * @param endian The endianness of the uint16_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @return *uint16_t* The uint16_t held in the register.
     */
    uint16_t uint16FromRegister(byte regType, int regNum,
                                endianness endian = bigEndian);
    /**
     * @brief Get the numbered input or holding register and return it as an int16_t.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The register number of interest.
     * @param endian The endianness of the int16_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @return *int16_t* The int16_t held in the register.
     */
    int16_t int16FromRegister(byte regType, int regNum, endianness endian = bigEndian);
    /**
     * @brief Get two input or holding registers starting at the specifiec number and
     * return them as a 32-bit float.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The number of the first of the two registers of interest.
     * @param endian The endianness of the 32-bit float in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return *float* The 32 bit float held in the register.
     */
    float float32FromRegister(byte regType, int regNum, endianness endian = bigEndian);
    /**
     * @brief Get two input or holding registers starting at the specifiec number and
     * return them as an uint32_t
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The number of the first of the two registers of interest.
     * @param endian The endianness of the uint32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return *uint32_t* The uint32_t held in the register.
     */
    uint32_t uint32FromRegister(byte regType, int regNum,
                                endianness endian = bigEndian);
    /**
     * @brief Get two input or holding registers starting at the specifiec number and
     * return them as an int32_t
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The number of the first of the two registers of interest.
     * @param endian The endianness of the int32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return *int32_t* The int32_t held in the register.
     */
    int32_t int32FromRegister(byte regType, int regNum, endianness endian = bigEndian);
    /**
     * @brief Get four input or holding registers starting at the specifiec number,
     * convert them to a TAI64 (64-bit timestamp), and return the lower 32-bits as a
     * unix timestamp.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The number of the first of the four registers of interest.
     * @return *uint32_t* The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64FromRegister(byte regType, int regNum);
    /**
     * @brief Get six input or holding registers starting at the specifiec number,
     * convert them to a TAI64N (64-bit timestamp followed by a 32-bit nanosecond
     * count), and return an equivalent 32-bits unix timestamp.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The number of the first of the six registers of interest.
     * @param nanoseconds A reference to another uint32_t to populate with the
     * nanoseconds.
     * @return *uint32_t* The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64NFromRegister(byte regType, int regNum, uint32_t& nanoseconds);
    /**
     * @brief Get eight input or holding registers starting at the specifiec number,
     * convert them to a TAI64NA (64-bit timestamp followed by a 32-bit nanosecond count
     * and then a 32-bit attosecond count), and return an equivalent 32-bits unix
     * timestamp.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The number of the first of the eight registers of interest.
     * @param nanoseconds A reference to another uint32_t to populate with the
     * nanoseconds.
     * @param attoseconds A reference to another uint32_t to populate with the
     * attoseconds.
     * @return *uint32_t* The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64NAFromRegister(byte regType, int regNum, uint32_t& nanoseconds,
                                 uint32_t& attoseconds);
    /**
     * @brief Get the numbered input or holding register and return one byte of it.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The register number of interest.
     * @param byteNum The byte number to return (1 for upper or 2 for lower)
     * @return *byte* The byte held in the register.
     */
    byte byteFromRegister(byte regType, int regNum, int byteNum);
    /**
     * @brief Get the numbered input or holding register and return it as an 16-bit
     * pointer.
     *
     * This should be a pointer to another registry address within the modbus registers.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The register number of interest.
     * @param endian The endianness of the 16-bit pointer in the modbus register.
     * Optional with a default of big endian, which is required by modbus
     * specifications.
     * @return *uint16_t* The 16-bit pointer held in the register.
     */
    uint16_t pointerFromRegister(byte regType, int regNum,
                                 endianness endian = bigEndian);
    /**
     * @brief Get the numbered input or holding register and return it as a 8-bit
     * pointer type.
     *
     * This should be the type of register pointed to by pointer contained within a
     * different modbus register.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The register number of interest.
     * @param endian The endianness of the pointer type in the modbus register.
     * Optional with a default of big endian, which is required by modbus
     * specifications.
     * @return *int8_t* The 8-bit pointer type held in the register. This will be an
     * object of type #pointerType.
     */
    int8_t pointerTypeFromRegister(byte regType, int regNum,
                                   endianness endian = bigEndian);
    /**
     * @brief Get a group of input or holding registers, convert them to characters,
     * combine them, and return a single String.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The number of the first of the registers of interest.
     * @param charLength The number of characters to return. NOTE: There are *TWO*
     * characters per register!
     * @return *String* The text from the registers.
     */
    String StringFromRegister(byte regType, int regNum, int charLength);
    /**
     * @brief Get a group of input or holding registers, convert them to characters and
     * put them into the given character array.
     *
     * There is no return from this function.
     *
     * @param regType The register type; use 0x03 for a holding register (read only) or
     * 0x04 for an input register (read/write)
     * @param regNum The number of the first of the registers of interest.
     * @param outChar A character array to fill with the content of the registers.
     * @param charLength The number of characters to return. NOTE: There are *TWO*
     * characters per register!
     */
    void charFromRegister(byte regType, int regNum, char outChar[], int charLength);
    /**@}*/


    // ===================================================================== //
    /**
     * @anchor high_level_setters
     * @name High level register setting functions
     *
     * These higher-level functions set data in input registers to a variety of data
     * types
     */
    /**@{*/
    // ===================================================================== //

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
    bool uint16ToRegister(int regNum, uint16_t value, endianness endian = bigEndian,
                          bool forceMultiple = false);
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
    bool int16ToRegister(int regNum, int16_t value, endianness endian = bigEndian,
                         bool forceMultiple = false);
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
    bool float32ToRegister(int regNum, float value, endianness endian = bigEndian);
    /**
     * @brief Set two holding registers to an uint32_t
     *
     * @param regNum The number of first of the two registers of interest.
     * @param value The value to set the register to.
     * @param endian The endianness of the uint32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications. Only
     * big and little endian are supported. Mixed endianness is *NOT* supported.
     * @return True if the registers were successfully set, false if not.
     */
    bool uint32ToRegister(int regNum, uint32_t value, endianness endian = bigEndian);
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
    bool int32ToRegister(int regNum, int32_t value, endianness endian = bigEndian);
    /**
     * @brief Set four holding registers to a TAI64 (64-bit timestamp)
     *
     * @param regNum The number of first of the four registers of interest.
     * @param seconds The lower 32-bits of the timestamp. The upper 32-bits will always
     * be set to 0x40000000, which will be the correct value until the year 2106.
     * @return True if the registers were successfully set, false if not.
     */
    bool TAI64ToRegister(int regNum, uint32_t seconds);
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
    bool TAI64NToRegister(int regNum, uint32_t seconds, uint32_t nanoseconds);
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
    bool TAI64NAToRegister(int regNum, uint32_t seconds, uint32_t nanoseconds,
                           uint32_t attoseconds);
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
    bool byteToRegister(int regNum, int byteNum, byte value,
                        bool forceMultiple = false);
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
    bool pointerToRegister(int regNum, uint16_t value, pointerType point,
                           endianness endian = bigEndian, bool forceMultiple = false);
    /**
     * @brief Set a series of holding registers to the characters in a String.
     *
     * @param regNum The first of the registers of interest
     * @param value The String to set the registers to.
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). This only
     * applies if the String is two charactes or less. Optional with a default value of
     * false.
     * @return True if the registers were successfully set, false if not.
     */
    bool StringToRegister(int regNum, String value, bool forceMultiple = false);
    /**
     * @brief Set a series of holding registers to the characters in a character array.
     *
     * @param regNum The first of the registers of interest
     * @param inChar The character array to set the registers to.
     * @param charLength The number of characters to set from in the array.
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). This only
     * applies if the character array is two charactes or less. Optional with a default
     * value of false.
     * @return True if the registers were successfully set, false if not.
     */
    bool charToRegister(int regNum, char inChar[], int charLength,
                        bool forceMultiple = false);
    /**@}*/


    // ===================================================================== //
    /**
     * @anchor mid_level_getters
     * @name Mid-level data frame result fetching functions
     *
     * These mid-level functions return a variety of data from an input modbus "frame."
     * Currently, the only "frame" available is the response buffer.
     * Using these functions will be helpful if you wish to decrease the serial traffic
     * by sending one "getRegisters" request for many registers and then parse that
     * result into many different results.
     */
    /**@{*/
    // ===================================================================== //

    /**
     * @brief Read an uint16_t out of the response buffer frame.
     *
     * @param endian The endianness of the uint16_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the uint16_t in the response frame.
     * Optional with a default of 3.
     * @return *uint16_t* The uint16_t held in the buffer frame.
     */
    uint16_t uint16FromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Read an int16_t out of the response buffer frame.
     *
     * @param endian The endianness of the int16_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the int16_t in the response frame.
     * Optional with a default of 3.
     * @return *int16_t* The int16_t held in the buffer frame.
     */
    int16_t int16FromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Read a 32-bit float out of the response buffer frame.
     *
     * @param endian The endianness of the 32-bit float in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the 32-bit float in the response
     * frame. Optional with a default of 3.
     * @return *float* The 32-bit float held in the buffer frame.
     */
    float float32FromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Read an uint32_t out of the response buffer frame.
     *
     * @param endian The endianness of the uint32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the uint32_t in the response frame.
     * Optional with a default of 3.
     * @return *uint32_t* The uint32_t held in the buffer frame.
     */
    uint32_t uint32FromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Read an int32_t out of the response buffer frame.
     *
     * @param endian The endianness of the int32_t in the modbus register. Optional
     * with a default of big endian, which is required by modbus specifications.
     * @param start_index The starting position of the int32_t in the response frame.
     * Optional with a default of 3.
     * @return *int32_t* The int32_t held in the buffer frame.
     */
    int32_t int32FromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Read a TAI64 (64-bit timestamp) out of the response buffer frame and
     * return the lower 32-bits as a unix timestamp.
     *
     * @param start_index The starting position of the TAI64 in the response frame.
     * Optional with a default of 3.
     * @return *uint32_t* The equivalent 32-bit unix timestamp.
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
     * @return *uint32_t* The equivalent 32-bit unix timestamp.
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
     * @return *uint32_t* The equivalent 32-bit unix timestamp.
     */
    uint32_t TAI64NAFromFrame(uint32_t& nanoseconds, uint32_t& attoseconds,
                              int start_index = 3);
    /**
     * @brief Read a single byte out of the response buffer frame.
     *
     * @param start_index The starting position of the byte in the response frame.
     * Optional with a default of 3.
     * @return *byte* The byte held in the buffer frame.
     */
    byte byteFromFrame(int start_index = 3);
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
     * @return *uint16_t* The 16-bit pointer held in the buffer frame.
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
     * @return *int8_t* The 8-bit pointer type held in the buffer frame. This will be an
     * object of type #pointerType.
     */
    int8_t pointerTypeFromFrame(endianness endian = bigEndian, int start_index = 3);
    /**
     * @brief Read a String out of the response buffer frame.
     *
     * @param charLength The number of characters to return.
     * @param start_index The starting position of the characters in the response
     * frame. Optional with a default of 3.
     * @return *String* The text from the registers.
     */
    String StringFromFrame(int charLength, int start_index = 3);
    /**
     * @brief Read characters out of the response buffer frame and put them into a
     * character array.
     *
     * There is no return from this function.
     *
     * @param outChar A character array to fill with the content of the response buffer.
     * @param charLength The number of characters to return.
     * @param start_index The starting position of the characters in the response
     * frame. Optional with a default of 3.
     */
    void charFromFrame(char outChar[], int charLength, int start_index = 3);
    /**@}*/


    // ===================================================================== //
    /**
     * @anchor mid_level_setters
     * @name Data frame setting functions
     *
     * These insert values into a longer modbus data frame.
     * These are useful in creating a single long frame which can be sent out in one
     * "setRegisters" command.
     */
    /**@{*/
    // ===================================================================== //

    /**
     * @brief Insert an uint16_t into the working byte frame
     *
     * @param value The value to add to the frame.
     * @param endian The endianness used to write the uint16_t. Optional with a default
     * of big endian, which is required by modbus specifications.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the uint16_t in the response frame.
     * Optional with a default of 0.
     */
    void uint16ToFrame(uint16_t value, endianness endian, byte modbusFrame[],
                       int start_index = 0);
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
    void int16ToFrame(int16_t value, endianness endian, byte modbusFrame[],
                      int start_index = 0);
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
    void float32ToFrame(float value, endianness endian, byte modbusFrame[],
                        int start_index = 0);
    /**
     * @brief Insert an uint32_t into the working byte frame
     *
     * @param value The value to add to the frame.
     * @param endian The endianness used to write the uint32_t. Optional with a default
     * of big endian, which is required by modbus specifications. Only big and little
     * endian are supported. Mixed endianness is *NOT* supported.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the uint32_t in the response frame.
     * Optional with a default of 0.
     */
    void uint32ToFrame(uint32_t value, endianness endian, byte modbusFrame[],
                       int start_index = 0);
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
    void int32ToFrame(int32_t value, endianness endian, byte modbusFrame[],
                      int start_index = 0);
    /**
     * @brief Insert a TAI64 (64-bit timestamp) into the working byte frame
     *
     * @param seconds The lower 32-bits of the timestamp. The upper 32-bits will always
     * be set to 0x40000000, which will be the correct value until the year 2106.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the TAI64 in the response frame.
     * Optional with a default of 0.
     */
    void TAI64ToFrame(uint32_t seconds, byte modbusFrame[], int start_index = 0);
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
    void TAI64NToFrame(uint32_t seconds, uint32_t nanoseconds, byte modbusFrame[],
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
                        byte modbusFrame[], int start_index = 0);
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
    void byteToFrame(byte value, int byteNum, byte modbusFrame[], int start_index = 0);
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
                        byte modbusFrame[], int start_index = 0);
    /**
     * @brief Insert a String into the working byte frame.
     *
     * @param value The string to insert.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the byte in the response frame.
     * Optional with a default of 0.
     */
    void StringToFrame(String value, byte modbusFrame[], int start_index = 0);
    /**
     * @brief Insert a character array into the working byte frame.
     *
     * @param inChar The character array to insert.
     * @param charLength The number of characters to copy from in the array.
     * @param modbusFrame The working byte frame
     * @param start_index The starting position of the byte in the response frame.
     * Optional with a default of 0.
     */
    void charToFrame(char inChar[], int charLength, byte modbusFrame[],
                     int start_index = 0);
    /**@}*/


    // ===================================================================== //
    /**
     * @anchor low_level_functions
     * @name Low level functions
     */
    /**@{*/
    // ===================================================================== //

    /**
     * @brief Get data from either holding or input registers
     *
     * @param readCommand The command to use to read data. For a holding register
     * readCommand = 0x03. For an input register readCommand = 0x04.
     * @param startRegister The starting register number.
     * @param numRegisters The number of registers to read.
     * @return *bool* True if the modbus slave returned the expected number of register
     * values; false if there was a failure.
     */
    bool getRegisters(byte readCommand, int16_t startRegister, int16_t numRegisters);

    // This gets data from either an output coil or an input contact
    // For a output coil readCommand = 0x01
    // For an input contact readCommand = 0x02
    // TODO - implement this
    // bool getCoils(byte readCommand, int16_t startRegister, int16_t numRegisters)

    /**
     * @brief Set the value of one or more holding registers using Modbus commands 0x06
     * or 0x10 (16).
     *
     * Input registers cannot be written by a Modbus controller/master
     *
     * @param startRegister The starting register number.
     * @param numRegisters The number of registers to write.
     * @param value A byte array with the values to write
     * @param forceMultiple Set the forceMultiple boolean flag to 'true' to force the
     * use of the Modbus command for setting multiple resisters (0x10). This only
     * applies if a single register is being set. Optional with a default value of
     * false.
     * @return *bool*
     */
    bool setRegisters(int16_t startRegister, int16_t numRegisters, byte value[],
                      bool forceMultiple = false);

    // This sets the value of one or more output coils
    // Modbus commands 0x05 and 0x0F
    // Input contacts cannot be written by a Modbus controller/master
    // TODO - implement this
    // bool setCoils(int16_t startRegister, int16_t numRegisters, byte value[])

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
     * 0.
     *
     * @param command The fully formed command to send to the Modbus slave.
     * @param commandLength The length of the outgoing command.
     * @return *int* The number of bytes received from the Modbus slave.
     */
    int sendCommand(byte command[], int commandLength);

    // These are purely debugging functions to print out the raw hex data
    // sent between the Arduino and the modbus slave.
    /**
     * @brief Set a stream for debugging information to go to.
     *
     * @param stream An Arduino stream object
     */
    void setDebugStream(Stream* stream) {
        _debugStream = stream;
    }
    /**
     * @copydoc modbusMaster::setDebugStream(Stream* stream)
     */
    void setDebugStream(Stream& stream) {
        _debugStream = &stream;
    }

    /**
     * @brief Un-set the stream for debugging information to go to; stop debugging.
     */
    void stopDebugging(void) {
        _debugStream = nullptr;
    }


    /**
     * @brief The response buffer for incoming messages from the Modbus slave.
     *
     * This needs to be bigger than the largest response.
     * For 8 parameters with 8 registers each:
     *   64 registers * 2 bytes per register + 5 frame bytes
     */
    static byte responseBuffer[RESPONSE_BUFFER_SIZE];
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
    void printFrameHex(byte modbusFrame[], int frameLength);

    /**
     * @brief Calculates a Modbus RTC cyclical redundancy code (CRC)
     *
     * @param modbusFrame The modbus frame to calculate the CRC from
     * @param frameLength The length of the frame
     */
    void calculateCRC(byte modbusFrame[], int frameLength);

    /**
     * @brief Adds the CRC to a modbus RTU frame
     *
     * @param modbusFrame The modbus frame to add the CRC to
     * @param frameLength The length of the frame
     */
    void insertCRC(byte modbusFrame[], int frameLength);

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
     * @param varLength The lenght of the variable to convert the endianness of.
     * @param endian The starting endianness; optional with a defaullt value of
     * bigEndian.
     * @param start_index The starting position of the variable in the frame. Optional
     * with a default value of 3.
     * @return *leFrame* A frame object with the converted endianness.
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

    byte _slaveID;  ///< The sensor slave id
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
    const uint32_t modbusTimeout = MODBUS_TIMEOUT;
    /**
     * @brief The time to wait between characters within a frame (in ms)
     *
     * The modbus protocol defines that there can be no more than 1.5 characters of
     * silence between characters in a frame and any space over 3.5 characters defines a
     * new frame.
     */
    const int modbusFrameTimeout = MODBUS_FRAME_TIMEOUT;
};

#endif
