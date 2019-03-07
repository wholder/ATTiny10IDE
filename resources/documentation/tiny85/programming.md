## Programming ATTiny25/45/85 Chips

The ATTiny85 series chips are designed to be programmed using a 6-wire serial protocol called In-Circuit Serial Programming (ICSP, or ISP for short.)  The signal used to program the ATTiny85 are, as follows:

+ VCC (Power)
+ Gnd (Ground)
+ MISO (serial output from ATTiny85 and input to programmer)
+ MOSI (serial output from programmer and input to ATTiny85)
+ SCK (serial data clock)
+ RESET (Reset signal)


### AVRISP mkII

The standard way to program ATTiny85 series chips is to use an In-Circuit Serial Programmer, such as the AVRISP mkII, pictured below:

<p align="center"><img src="images/ATAVRISP2.jpg" width="700" height="527"></p>

_Note: In the image above, the 6 pin connector is shown from the top of the connector, which is the opposite from the side where the mating pins are inserted.  In addition, the MISO and MOSI pins on the ATTiny are labelled from the perspective of the ATTiny device being the **slave** device, which is the reverse of how these pins are labelled when the ATTiny is the **master** device, such as the case when the ATTiny is connected to an SPI device it is controlling._

Note: Atmel.Microchip has discontinued the AVRISP mkII, but clones of the AVRISP mkII are available from various on-line sellers.  To use an AVRISP mkII, select it as your programmer in the "Settings->ISP Programmer" menu.  Internally, ATTiny10IDE then uses AVRDUDE to program your ATTiny85.  The following wiring diagam shows how to connect the 6 pins from the AVRISP mkII to your ATTiny85 chip:

<p align="center"><img src="images/ATTiny85-to-AVRISP-mkII.png" width="514" height="293"></p>

You can use the AVRISP mkII to upload and program your code, like this:

 1. Use "**`Settings->ISP Programmer`**" to select **`AVRISP mkII`**.
 2. Open, or write the source code you want to compile and program into the ATtiny85.
 3. Select "**`Actions->Build`**" to compile. or assemble the code.
 4. Connect the Arduino to the ATTiny85 using the connections shown above.
 5. If you've changed the fuse setting from the default settings using the **`#pragma`** **`lfuse`**, **`hfuse`** and/or **`efuse`** directives select "**`Actions->ISP Programmer->Program Fuses`**" to write the new fuse settings to the ATTiny85.  Note: fuse settings are not altered when you upload new program code, so you only need to use this command when you need to set new fuse values. 
 6. Select "**`Actions->Program Flash and Fuse(s)`**" to upload and program the code into the ATtiny85.

### Arduino as ISP

Another way to program an ATTiny85 is to use an ATmega328P-based Arduino (Arduino UNO, for example) as a programmer by uploading a special Sketch called ArduinoISP, which is included under the "Files->Examples" menu:

<p align="center"><img src="images/ArduinoISP.png" width="585" height="269"></p>

After uploading the ArduinoISP sketch to your ATmega328P-based Arduino use the following wiring diagram to connect the Arduino to the ATTiny85:

<p align="center"><img src="images/ATTiny85-to-ArduinoISP.png" width="469" height="293"></p>

You can use the ArduinoiISP Sketch as a general-purpose ATTiny85 programmer to upload and program other code, like this:

 1. First "Quit" the Arduino IDE so ATTiny10IDE will have access to the Serial Port on the Arduino running the Sketch.
 2. Start ATTiny10IDE and set "**`Settings->Serial Port->Baud Rate`**" to 115200 and "**`Settings->Serial Port->Port`**" to select the Arduino running the Sketch.
 3. Open, or write the source code you want to compile and program into the ATtiny85.
 4. Select "**`Actions->Build`**" to compile. or assemble the code.
 5. Connect the Arduino to the ATTiny85 using the connections shown above.
 6. If you've changed the fuse setting from the default settings using the **`#pragma`** **`lfuse`**, **`hfuse`** and/or **`efuse`** directives select "**`Actions->ISP Programmer->Program Fuses`**" to write the new fuse settings to the ATTiny85.  Note: fuse settings are not altered when you upload new program code, so you only need to use this command when you need to set new fuse values. 
 7. Select "**`Actions->Program Flash and Fuse(s)`**" to upload and program the code into the ATtiny85.

