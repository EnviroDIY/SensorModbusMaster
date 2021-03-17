# KNH002 Hardware RS-485 Hardware for Envirodiy Mayfly

This Mayfly wingboard interface supports   
- RS485 on up to three physical connectors. Uses the MAX22025 for RS485 physical interface, and has the feautre of managing the RS485 half duplex states.   
- improved power supply to 1.9W continuous, voltage dependent on booster.
- new feature - local 120ohm termination 2mm jack. The installer can activate the RS485 120ohm termination resistor if needed.   
 This RS485 physical line interface designed for long lines if required, one remote 120ohms, and a local 120ohms termination resistor.  
- new feature - battery monitoring, V and mAHrs 
- new feature - 12V output has 155mA resetable fuse. Guarenteed trip at 350mA. If output wire is short circuit it will protect battery/processor reliability.
     (The hybrid design used Mayfly 0.5b 3.3V that was limited to ~500mA or 1.6W, but not short circuit limited ) 
- new feature Powering routing that allows connection directly to the LiIon battery.  
   This is a more efficient boost generation and greater power resilency by routing power directly from the battery. 
   Frr the +12V current supply, the current supplied an be 155mA/250mA (1.9W continuous/3Wsurge)   
- Low ESR capacitor on the +12V generation input pins to avoid power surge propagation
- new feature - TTL lines have power safe data lines.  (not tested)
- new feature - ground wire. Connect the RS485 GND through a thick 18AWG wire to external ground to conduct external power surges. 
- a dual led shows RED flashes for transmit to RS485 instrument, Green flashes from RS485 instrumnent (response)   

Changes from the old Mayfly RS485 wingboard
a) doesn't use the unmanaged hybrid.  These hybrids where of unspecified quality, and often didn't work for me. 

https://github.com/neilh10/SensorModbusMaster/wiki  Building Guide - tbd  see https://github.com/EnviroDIY/YosemitechModbus#suggested-setup-with-an-envirodiy-mayfly

https://github.com/neilh10/SensorModbusMaster/issues/1

History
2021-Mar-10 Circuit diagram for KNH002revision6    
   The 12V fuse is 155mA hold, 330mA Trip changed from previous 180mA. A line short causes the heating in the electronic fuse, which causes it to go to high resistance, limiting the current flow.       
   The LED resistor for the Green is set to 60ohms make the emitted mcd similar to the Red Led which is 120ohms. For succesful sensor poll, a user should see the Red/Green flashes, For unsucesfull polling with no response will see a Red Flash.  A technical detail, the PCB footprint Rev6 is correct, on the Rev4 it had an error, swapping the colours.
   The connector labeled "May" is physically closest to the Mayfly bat Jp1 and polarised to be the same as Mayfly JP1. Rev4 had it the other way around.      
   The battery monitoring IC STC3100  monitors the LiIon battery Voltage, Current and power used mAhr/coloumbs. This was introduced in Rev5, Rev4 had a different IC that became unavailable.     
   Rev6 PCB https://oshpark.com/shared_projects/tlFl6OPg    
   https://github.com/neilh10/SensorModbusMaster/tree/release1/hardware/knh002-MayflyWingShield/rev6    
   

2021-Feb-10 KNH002revision4 https://github.com/neilh10/SensorModbusMaster/tree/release1/hardware/knh002-MayflyWingShield 
    All boards and libs are in  KNH002rev4_2102101316published.zip  (as of Mar10 not recommended as has a number of issues corrected in rev6)

Background (from original Mayfly Wingboard)    

The [Modbus RTU](https://en.wikipedia.org/wiki/Modbus) *logical* communication protocol is typically transmitted using the *physical* [RS-485](https://en.wikipedia.org/wiki/RS-485) serial communication signaling standard.

The Mayfly micro-controllers,  transmit serial signals using transistor-to-transistor logic, or [TTL serial](https://learn.sparkfun.com/tutorials/serial-communication/wiring-and-hardware).

Sensors with Modbus will need to an RS485-to-TTL adapter module to communicate. Modbus sensors typically rated to be able to use 8-30V.  12V usally covers modst Modbus sensors - see each sensor specification. This step up Voltage converter is provided on board. 
Last, the electrical "logic level" signals returned from the board must be compatible with the Mayfly's 3.3V logic and not take power when the RS485 circuits are powered down.

- Early DIY prototype:
  https://github.com/EnviroDIY/YosemitechModbus#suggested-setup-with-an-envirodiy-mayfly
