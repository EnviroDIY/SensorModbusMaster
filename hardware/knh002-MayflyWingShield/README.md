# KNH002 Hardware RS-485 Hardware for Envirodiy Mayfly

This Mayfly wingboard interface supports   
- RS485 on up to three physical connectors, all clearly labeled with G V B A   
- improved brightness - a dual line status LED Red/Green a) Red flashes for transmit to RS485 instrument, B) Green flashes from RS485 instrumnent (response)  
- improved power supply to a min 1.9W continuous, voltage dependent on booster. The boost circuit may be able to supply larger surge currents as it's supply climbs/boosts as the LiIon bat is low impedance. These could be as high as 0.7A continuous at lower voltages, valuable for starting up instruments on turnon.
- new feature - Powering routing that allows connection directly to the LiIon battery, for more efficent generation and power resilency.  
   For the power output "12V" current supply, the current supplied can be 155mA to 250mA surge (1.9W continuous/3Wsurge) depending on boost hybrid.  
   Low ESR capacitor on the +12V generation input pins to avoid power surge propagation
- new feature - instrument power "12V" has 155mA resetable fuse (PTC), limiting power drawn on a line short.
- new feature - local RS485 120ohm termination 2mm jack. The installer can activate the RS485 120ohm termination resistor as needed.   
- new feature - battery monitoring, accurate battery (V), energy consumed (mAHrs) and instaneous current mmeasurement.
- new feature - TTL lines have power safe data lines.  (not tested)
- new feature - ground wire. Connect the RS485 GND through a thick 18AWG wire to external ground to conduct external power surges. 
 

Changes from the old Mayfly RS485 wingboard    
a) doesn't use the unmanaged hybrid.  These hybrids where of unspecified quality, and often didn't work for me. 

Building Guide - to-be-updated - see    
https://github.com/neilh10/SensorModbusMaster/wiki    
https://github.com/EnviroDIY/YosemitechModbus#suggested-setup-with-an-envirodiy-mayfly


# KNH002 Circuit Discription
(as per rev6) 
- Typically the 3 physical connections are deciced by the end-user, and soldered onto the board. The silk screen has clearly printed G V B A for each connector on both sides of the board. G=Gnd  V=Voltage and then RS485 designators A and B.     
- Typically the boost hybrid is decided by the end user. BOM has 12V option.   
- U1/ MAX22025 is the RS485 physical interface, and manages the RS486 half-duplex state machine.   
- R2/R3 649ohms are the pull-up down resistors for a long twisted pair transmission line. The value 649ohms was selected based on a remote 120Ohm and local 120ohm terminations. Its optomized for a single instrument termination over a long line, but typically will work with a number instruments terminating locally. Its up to the user to figure out if a complex set of instruments and wire lengths works for them, or do RS485 line calculations for what value of resistors they need.     
- The Max22025 Rxb (Rx buffered) interfaces to the Mafyly via the Ioff Safe SN74LVC234. These allow the MAX22025/SN74LVC234 to be turned off, and not take current from the Mayfl.   
- The Green LED is attached to the Rxb driven by the Max22025 output. When an instrument responds to a poll request, the green LED turns ON when signal is low and powered. Statastically its likely there will be some low portions in the received signal. The green is high intensity to be visible.
- The SN74LVC234 Txb Tx(buffered) drives the RED LED, turning ON when LOW, and board powered. This reflects attempts to poll a target instrumnet. ModularSensors code attempts multiple (3) times to contact an instrument. For the sensor set to read/average 3 times, then for a disconnected instrument there will be3 sets of fast red flashes. 
- Typically the power will be supplied by a battery attached to the J8 labelled "Bat", and ajumper wire will supply power from J9 labelled "May" . On the underside of the board these are clearly labeled "Mayfly" and "Battery" with the + connection identified.   
- Typically the power will be boosted by U2, and C1 is designed to be able to supply fast switching currents on U2. For detailed analysis refer to the capacitor specification and how switching circuits work.    
- The power is switched via U3 SIP32431 Load Switch or similar. This turns ON when 3Vsw is activated by the Mayfly software.  The load switch can support continuous 1.4A. If there is a short circuit in the power loop (before the fuse or if the fuse is bypassed), this devices will emit smoke and become non-operational.   
- The output of U2 has an electronic fuse R8 155mA, protecting from a short on the output "12V".    
-  Fuses are rated at minimium holding current, and guarenteed trip at 350mA. If output wire is short circuit it will protect battery/processor reliability.  
      (The old RS485 hybrid wingboard used the builtin Mayfly regulator that would limit current flows at 3.3V to 0.5A, also limiting power availability to max 1.65W)   
- On building a board, it can be built with power coming from the 5Vsw, by adding R1=0 and not stuffing U3,J8 J9 (and of course U5 or R5)    
- Battery coloumb "Fuel Gauge" monitor - STC3100. This supplies acurrate Liion Battery voltage, coloumb in mAh with a typical 0.2mAh resolution, 8-byte unique ID, 32Ram bytes backed up by LiIon battery. There is a device driver https://github.com/neilh10/STC3100arduino and a ModularSensors "Sensors" is in development.
 


# History   
https://github.com/neilh10/SensorModbusMaster/issues/1    

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
