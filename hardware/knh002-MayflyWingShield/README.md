# KNH002 Hardware RS-485 Hardware for Envirodiy Mayfly

This Mayfly wingboard interface supports   
- RS485 on up to three physical connectors.   
- new feature - local 120ohm termination 2mm jack. The installer can activate the RS485 120ohm termination resistor if needed.   
 This RS485 physical line interface designed for long lines if required, one remote 120ohms, and a local 120ohms termination resistor.  
- new feature - 12V output has 100mA resetable fuse. Trips at 250mA, but will hold 100mA. If output wire is s/c will protect battery/processor reliability.
     (The hybrid design used Mayfly 0.5b 3.3V that was limited to ~500mA or 1.6W, but not short circuit limited ) 
- new feature Powering routing that allows connection directly to the LiIon battery.  
   This is a more efficient +12V generation and greater power resilency taken from the battery. 
   Max +12V current supply of 100mA/250mA (1.2W continuous/3Wsurge) not tested.   
- Low ESR capacitor on the +12V generation input pins to avoid power surge propagation
- new feature - TTL lines have power safe data lines.  (not tested)
- new feature - ground wire. Connect the RS485 GND through a thick 18AWG wire to external ground to conduct external power surges. 
- a dual led shows RED flashes for transmit to RS485 instrument, Green flashes from RS485 instrumnent (response)
- new advanced feature - fuel gauge with accurate battery voltage measurement ( in beta still being tested )    

Changes from the old Mayfly RS485 wingboard
a) doesn't use the unmanaged hybrid.  These hybrids where of unspecified quality, and often didn't work for me. 

https://github.com/neilh10/SensorModbusMaster/wiki  Building Guide - tbd  see https://github.com/EnviroDIY/YosemitechModbus#suggested-setup-with-an-envirodiy-mayfly

https://github.com/neilh10/SensorModbusMaster/issues/1

History
2021-Feb-10 KNH002revision4 https://github.com/neilh10/SensorModbusMaster/tree/release1/hardware/knh002-MayflyWingShield 
    Order place but not tested.  https://oshpark.com/shared_projects/zZYpR4hd
    All boards and libs are in  KNH002rev4_2102101316published.zip

Background (from original Mayfly Wingboard)    

The [Modbus RTU](https://en.wikipedia.org/wiki/Modbus) *logical* communication protocol is typically transmitted using the *physical* [RS-485](https://en.wikipedia.org/wiki/RS-485) serial communication signaling standard.

The Mayfly micro-controllers,  transmit serial signals using transistor-to-transistor logic, or [TTL serial](https://learn.sparkfun.com/tutorials/serial-communication/wiring-and-hardware).

Sensors with Modbus will need to an RS485-to-TTL adapter module to communicate. Modbus sensors typically rated to be able to use 8-30V.  12V usally covers modst Modbus sensors - see each sensor specification. This step up Voltage converter is provided on board. 
Last, the electrical "logic level" signals returned from the board must be compatible with the Mayfly's 3.3V logic and not take power when the RS485 circuits are powered down.

- Early DIY prototype:
  https://github.com/EnviroDIY/YosemitechModbus#suggested-setup-with-an-envirodiy-mayfly
