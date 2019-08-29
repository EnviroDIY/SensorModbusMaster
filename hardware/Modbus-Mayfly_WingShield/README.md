# Modbus-Mafly-WingShield

This is an early draft document describing how to acquire parts, construct and use the EnviroDIY **Modbus-Mafly-WingShield** board.

NOTE:
- Communication is hardwired to Mayfly digital pins 5 & 6, which enable use of the [AltSoftSerial](https://github.com/PaulStoffregen/AltSoftSerial) library.
  - Read more at our [Notes on Arduino Streams and Software Serial](https://github.com/EnviroDIY/ModularSensors#notes-on-arduino-streams-and-software-serial).
- Power to the RS485 to TTL Adapter comes from the Mayfly's switched 3.3V power.
- Power to the sensor(s) comes from the Mayfly's switched 5V power supply, which can then be optionally boosted to 9V or 12V depending on the requirements of the sensor.

<img src="https://github.com/EnviroDIY/SensorModbusMaster/blob/master/hardware/Modbus-Mayfly_WingShield/Photos/IMG_6733.JPG"  width="600">

## Parts List

<img src="https://github.com/EnviroDIY/SensorModbusMaster/blob/master/hardware/Modbus-Mayfly_WingShield/Photos/IMG_6747-crop.JPG"  width="600">

- [Modbus-Mayfly-Wing PCB](https://github.com/EnviroDIY/SensorModbusMaster/blob/master/hardware/Modbus-Mayfly_WingShield/Board_Mrk1.png)
  - Users can have the printed circuit board (PCB) made from these [Gerber files](https://github.com/EnviroDIY/SensorModbusMaster/blob/master/hardware/Modbus-Mayfly_WingShield/RS485_Mayfly_Gerbers.zip).
  - These PCB manufacturers that specialize in small, inexpensive jobs for DIYers:
    - [Bay Area Circuits](https://store.bayareacircuits.com)
    - [Advanced Circuits](http://www.4pcb.com)
    - [OSH Park](https://oshpark.com)

- [WINGONEER TTL To RS485 Adapter 485 Serial Port UART Level Converter Module 3.3V 5V](https://www.amazon.com/gp/product/B06XHH6B6R) from Amazon. 
  - There are many other companies that sell this exact same RS485 adapter board (such as [SMAKN](https://www.amazon.com/SMAKN®-Adapter-Serial-Converter-Module/dp/B010723BCE/#product)), so pick any of them that look like this one:
![u3022-wiring-1](https://user-images.githubusercontent.com/5166036/46170137-9825d580-c262-11e8-88a8-5ac84e84da19.jpg)
  - NOTE: It is hard to find documenation on this board, but we know from experience that it can handle signal logic level shifting from anything between 5-12V down to the 3.3V digital signal required by the Mayfly. This is the RS485 adapter board that YosemiTech shipped inside a converter box enclosure along with a 12V power adaptor for their sensors, which is why we started using it with sensors that require higher voltages to power them.

- [Pololu 12V Step-Up Voltage Regulator U3V12F12](https://www.pololu.com/product/2117) or the [Pololu 9V Step-Up Voltage Regulator U3V12F9](https://www.pololu.com/product/2116), depending on your needs.
  - NOTE: If your sensor only needs 5V of power, you can skip buying a regulator and just solder a jumper closed on the Wing PCB to power the sensor from the 5V Mayfly power line.

* Capacitors:
  - You will probably need a capacitor to handle the initial power surge of starting up the sensor. Here are the three that we used:
      * [CAP ALUM 220UF 20% 10V RADIAL](https://www.digikey.com/product-detail/en/wurth-electronics-inc/860020273009/732-8911-1-ND/5728854), for the lowest power demand sensors.
      * [CAP 470 UF 20% 10 V](https://www.digikey.com/product-detail/en/wurth-electronics-inc/860010273011/732-8708-1-ND/5728660), which was the minimum size that would power a YosemiTech Y511 Turbidity sensor with a brush.
      * [CAP ALUM 680UF 20% 6.3V T/H](https://www.digikey.com/product-detail/en/rubycon/6.3ZLQ680MEFC6.3X11/1189-3597-ND/6049820), which was barely the minimum size that would power a YosemiTech Y4000 Sonde with a brush. We usually used two of these, or one of these plus a 470 uF capacitor.

* Various headers, screw terminals from DigiKey:

Part Number | Description | Unit Price
-- | -- | --
WM11221-ND | CONN HEADER VERT 3POS TIN | 0.442
WM4002-ND | CONN HEADER 4POS .100 VERT TIN | 0.3
732-5316-ND | CONN HEADER 3 POS 2.54 | 0.088
609-3214-ND | CONN HEADER 20POS .100 STR 15AU | 0.706
ED10563-ND | CONN TERM BLOCK 2.54MM 4POS PCB | 0.8204

* We also used Grove ports from Seeed Studio.
