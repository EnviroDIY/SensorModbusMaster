# scan-Modbus
This library is to communicate between an Arduino AVR processor and an S::CAN spectrometer via Modbus/RS485

To use SoftwareSerial with the spectro::lyzer, you **must** set the parity to **none** (parity option 0 in holding register 3)!  This is not the default value.  SoftwareSerial cannot handle the default odd parity.  This means to connect the instrument again with the S::CAN software, you must either change the parity back to the default of odd or change the settings in the S::CAN software
