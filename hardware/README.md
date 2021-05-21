# Hardware for using ModbusRTU with RS-485 on Arduino

### NOTICE: Hardware designs have been moved to the [Mayfly-Modbus-Wing](https://github.com/EnviroDIY/Mayfly-Modbus-Wing) repo!

## Why Modbus Hardware is Required for Arduino

The [Modbus RTU](https://en.wikipedia.org/wiki/Modbus) *logical* communication protocol is typically transmitted using the *physical* [RS-485](https://en.wikipedia.org/wiki/RS-485) serial communication signaling standard.

Arduino-framework micro-controllers, on the other hand, transmit serial signals using transistor-to-transistor logic, or [TTL serial](https://learn.sparkfun.com/tutorials/serial-communication/wiring-and-hardware).

Therefore, most sensors using this library will need to an RS485-to-TTL adapter module to communicate. Also, most Modbus sensors require 5-12V of power, so a separate step-up power converter may be necessary to run it off a 3.3V Arduino micro-controller. Last, the electrical "logic level" signals returned from a sensor will typically have a HIGH (or "1") returned at the same voltage that powers the sensor, which means that a logic-level shifter/converter is required to ensure that communication signals being received by the Arduino micro-controller are 3.3V. Luckily, most RS485-to-TTL adapter modules also perform logic-level voltage conversions.

For all these reasons, additional hardware is usually required to interface a Modbus sensor with an Arduino micro-controller.

We have developed some guidance, designs and documentation different interface hardware solutions that have worked for us. This information is now available in the [EnviroDIY/Mayfly-Modbus-Wing](https://github.com/EnviroDIY/Mayfly-Modbus-Wing) repo.
