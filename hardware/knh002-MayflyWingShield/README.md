# KNH002 Hardware RS-485 Hardware for Envirodiy Mayfly

*** Provisional ~ for discussion ****

https://github.com/neilh10/SensorModbusMaster/issues/1

https://oshpark.com/shared_projects/ffQNVmrp

This Mayfly wingboard interface supports   
- RS485 on up to three physical connectors.   
- new feature - local 120ohm termination 2mm jack. The installer can simply activate the RS485 120ohm termination resistor if needed.   
 RS485 physical line interface designed for long lines if required, one remote 120ohms, and a local 120ohms termination resistor.  
- new feature - TTL lines have power safe data lines.   
  Powering routing that allows connection directly to the LiIon battery.  This faciliates a more efficient +12V generation and greater power resilency.   
- new feature - power can be taken directly from the LiIon battery (a build option)    
- new advanced feature - fuel gauge LTC2942 ( build option )    

Background (from original Mayfly Wingboard)    

The [Modbus RTU](https://en.wikipedia.org/wiki/Modbus) *logical* communication protocol is typically transmitted using the *physical* [RS-485](https://en.wikipedia.org/wiki/RS-485) serial communication signaling standard.

The Mayfly micro-controllers,  transmit serial signals using transistor-to-transistor logic, or [TTL serial](https://learn.sparkfun.com/tutorials/serial-communication/wiring-and-hardware).

Therefore, most sensors using this library will need to an RS485-to-TTL adapter module to communicate. Also, most Modbus sensors require 5-12V of power, so a separate step-up power converter is used. Last, the electrical "logic level" signals returned from a sensor will typically have a HIGH (or "1") returned at the same voltage that powers the sensor, which means that a logic-level shifter/converter is required to ensure that communication signals being received by the Arduino micro-controller are 3.3V. Luckily, most RS485-to-TTL adapter modules also perform logic-level voltage conversions.


  - This is what we are currently using widely.
- Early DIY prototype:
  - https://github.com/EnviroDIY/YosemitechModbus#suggested-setup-with-an-envirodiy-mayfly
