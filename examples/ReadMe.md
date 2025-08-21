# Examples Using SensorModbusMaster<!--! {#page_the_examples} -->

These example programs demonstrate how to use the SensorModbusMaster library.

___

<!--! @if GITHUB -->

- [Examples Using SensorModbusMaster](#examples-using-sensormodbusmaster)
  - [Reading and Writing Registers](#reading-and-writing-registers)
  - [Scanning Registers](#scanning-registers)

<!--! @endif -->

<!--! @tableofcontents -->

<!--! @m_footernavigation -->

## Reading and Writing Registers<!--! {#examples_read_write_register} -->

This example writes a setting value to a holding register, reads it to confirm the value has changed, and then reads several data values from holding registers.

- [Instructions for the reading and writing example](https://envirodiy.github.io/SensorModbusMaster/example_read_write_register.html)
- [The reading and writing example on GitHub](https://github.com/EnviroDIY/SensorModbusMaster/tree/master/examples/readWriteRegister)

## Scanning Registers<!--! {#examples_scan_registers} -->

This is a testing program to scan through all possible holding registers.
This was written to try to guess the structure of the modbus registers when a map isn't available.

- [Instructions for the registry scanning example](https://envirodiy.github.io/SensorModbusMaster/example_scan_registers.html)
- [The registry scanning example on GitHub](https://github.com/EnviroDIY/SensorModbusMaster/tree/master/examples/scanRegisters)
