# ChangeLog

All notable changes to this project will be documented in this file.

The format is based on [Keep a Changelog](https://keepachangelog.com/en/1.0.0/)
and its stricter, better defined, brother [Common Changelog](https://common-changelog.org/),
and this project adheres to [Semantic Versioning](https://semver.org/spec/v2.0.0.html).

***

## [Unreleased]

### Changed

### Added

### Removed

### Fixed

***

## [v0.7.0]

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

## [v0.6.8]

### Added

- Added documetation for the wing shield.

***

## [v0.6.7]

### Fixed

- Fix by @neilh11 for a null pointer in the debugging causing crashes and other "undefined behavior."

***

## [v0.6.5]

### Fixed

- Fixed attoseconds for TAI64NA time formats.
- Fixed compiler warnings when compiling with stricter -Wextra flag.

***

## [v0.6.2]

### Added

- Added the ability to use the multiple register writing command when only writing to one register.

***

## [v0.5.3]

### Changed

- Changed the keywords describing the library.

***

## [v0.5.2]

### Added

- Added implementations for TAI64N and TAI64NA

### Fixed

- Fixed TAI64 implementation

***

## [v0.4.8]

### Changed

- Updated ReadMe
- Updated examples

***

## [v0.4.3]

### Fixed

- Fixed issues dealing with unprintable characters in strings

***

## [v0.4.0]

- Added functions to write to registers and frames

***

## [v0.1.2]

### Changed

- Made the response buffer public

***

## [v0.1.0]

### Added

- Initial release

***

[Unreleased]: https://github.com/EnviroDIY/ModularSensors/compare/v0.7.0
[0.7.0]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.6.8
[0.6.8]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.6.8
[0.6.7]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.6.7
[0.6.5]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.6.5
[0.6.2]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.6.2
[0.5.3]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.5.3
[0.5.2]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.5.2
[0.4.8]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.4.8
[0.4.7]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.4.7
[0.4.3]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.4.3
[0.4.0]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.4.0
[0.1.2]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.1.2
[0.1.0]: https://github.com/EnviroDIY/ModularSensors/releases/tag/v0.1.0

[//]: # ( @tableofcontents{XML:1} )

[//]: # ( @m_footernavigation )
