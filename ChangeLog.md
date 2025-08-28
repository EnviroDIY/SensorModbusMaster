# ChangeLog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/) and its stricter, better defined, brother [Common Changelog](https://common-changelog.org/).

This project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

***

## [Unreleased]

### Changed

- Removed all variable length arrays within functions
- No longer trimming returned Strings

### Added

- Implemented a single static command buffer
- Added the define `COMMAND_BUFFER_SIZE` to control the size of the command buffer.
- Added `charToRegister(..)` and `charToFrame(..)` functions, each accepting a pointer to a constant char (const char*).

### Removed

### Fixed

- Fixed return value for sendCommand function.

***

## [1.3.0]

### Changed

- The sendCommand function will now return a value of > 4095 (0x0FFF) if there has been an error. The upper most byte of the uint16_t response to the command is the modbus error code. This makes it much easier to differentiate error responses and no responses.

### Added

- Added extra functions specifying "holding" or "input" in the function name.
- Added the ability to change the number of times to retry commands.
- Added the ability to change the command and frame timeouts.
- Added an enum for modbus error codes.
- Added a variable to store the last error code, which can be checked with `getLastError()` and printed with `printLastError()`.

### Fixed

- Fixed calculate of expected return size

***

## [1.2.0]

### Changed

- When dumping the buffer, wait the frame timeout time between characters instead of a set 1ms.

***

## [1.1.0]

### Added

- Added setters and getters for the enable pin, modbus command timeout, and modbus frame timeout.

***

## [1.0.1]

### Fixed

- Corrected comments on input and holding registers.  Input registers are the ones that are read only; holding registers are read/write.

***

## [1.0.0]

### Added

- Added functions to read coils
- Added functions to write single and multiple coils
- Added functions to read discrete inputs
- Added functions to read holding and input registers without requiring the command type as input
- Added functions to read coils, discrete inputs, holding, and input registers into a user buffer
- Added code spell configuration

### Fixed

- Fixed spelling errors

***

## [0.7.3]

### Changed

- Re-release of 0.7.2 with updates to VERSION and library properties/json

***

## [0.7.2]

### Changed

- Update `readWriteRegister.ino` example to work with AltSoftSerial

### Added

- Add a new `getSetAddress.ino` example #30

### Fixed

- Fix `printFrameHex()` that was broken with v0.7.0 #29

***

## [0.7.1]

### Changed

- Update examples to apply to more processors
- Implement testing of more processors
- Set the platforms/architectures supported list to "*" (all)

***

## [0.7.0]

### Changed

- Applied clang format to source and examples.
- Moved the scanRegisters example from the utils to the examples folder.
- Updated Arduino library keywords file.
- Changed the debugging functionality to not require a define.

### Added

- Added Doxygen documentation comments.
- Created a documentation page using Doxygen.
- Added CI checks with Github actions using reusable workflows.

### Removed

- Removed build files and documentation for custom RS-485 boards.
This information can now be found in its [own repository](https://github.com/EnviroDIY/Mayfly-Modbus-Wing).

***

## [0.6.8]

### Added

- Added documentation for the wing shield.

***

## [0.6.7]

### Fixed

- Fix by [neilh10](https://github.com/neilh10) for a null pointer in the debugging causing crashes and other "undefined behavior."

***

## [0.6.5]

### Fixed

- Fixed attoseconds for TAI64NA time formats.
- Fixed compiler warnings when compiling with stricter `-Wextra` flag.

***

## [0.6.2]

### Added

- Added the ability to use the multiple register writing command when only writing to one register.

***

## [0.5.3]

### Changed

- Changed the keywords describing the library.

***

## [0.5.2]

### Added

- Added implementations for TAI64N and TAI64NA

### Fixed

- Fixed TAI64 implementation

***

## [0.4.8]

### Changed

- Updated ReadMe
- Updated examples

***

## [0.4.3]

### Fixed

- Fixed issues dealing with unprintable characters in strings

***

## [0.4.0]

- Added functions to write to registers and frames

***

## [0.1.2]

### Changed

- Made the response buffer public

***

## [0.1.0]

### Added

- Initial release

***

[Unreleased]: https://github.com/EnviroDIY/SensorModbusMaster/compare/v1.3.0...HEAD
[1.3.0]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v1.3.0
[1.2.0]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v1.2.0
[1.1.0]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v1.1.0
[1.0.1]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v1.0.1
[1.0.0]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.7.3
[0.7.3]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.7.3
[0.7.2]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.7.2
[0.7.1]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.7.1
[0.7.0]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.7.0
[0.6.8]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.6.8
[0.6.7]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.6.7
[0.6.5]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.6.5
[0.6.2]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.6.2
[0.5.3]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.5.3
[0.5.2]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.5.2
[0.4.8]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.4.8
[0.4.3]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.4.3
[0.4.0]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.4.0
[0.1.2]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.1.2
[0.1.0]: https://github.com/EnviroDIY/SensorModbusMaster/releases/tag/v0.1.0

<!--! @m_footernavigation -->
