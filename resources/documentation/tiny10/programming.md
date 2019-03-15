## Programming ATTiny4/5/9/10 Chips

ATTiny10IDE supports several different ways to program your compiled code into an ATTiny10 chip.  One is to build and use the [Arduino-based, High Voltage programmer/emulator](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer) I designed for the original version of my IDE.  However, ATTiny10IDE also now supports a way to generate an Arduino Sketch (program) which, when run, can directly program an ATTiny10 chip using only Arduino I/O lines D2 - D6, like this:

<p align="center"><img src="images/TPI-Prog-Socket.png" width="434" height="303"></p>
    
Here are the steps you can follow to program an ATTiny10 using this method:
 1. Use "**`File->Open`**" to load the source file, or type your code into the "Source Code" pane and save it using the appropriate file extension.
 2. Select "**`Actions->Build`**" to compile, or assemble the code.
 3. Select "**`Actions->Generate Arduino Programmer Code`**" and save to a .ino (Arduino Sketch) file when prompted.
 4. Quit ATTiny10IDE so it does not interfere with the Arduino IDE's access to the Arduino's serial port.
 5. Load the file you generated into the Arduino (it will prompt you to create a folder for this file.  Choose Yes.)
 6. Program the Sketch into the Arduino using the "Upload" Button.
 7. Open the "**Serial Monitor**" window in the Arduino IDE (upper right button) and set the baud rate to 115200.
 8. Follow the instructions the Sketch prints to connect the Arduino to the ATTiny10.
 9. Verify the ATTiny10 is properly connected by using the Identify ATtiny10 command by typing '**`I`**' and pressing "**`Send`**".
 10. If Identify ATTiny10 responds with "Device: ATtiny10", you are clear to program the ATtiny10 by typing '**`P`**' and pressing "**`Send`**"

### Using the Sketch as an ISP Programmer

You can also use this Sketch to program as many ATTiny10s as needed with the code you generated it from in ATTiny10IDE and you can also use it as a general-purpose ATTiny10 programmer to upload and program other code, like this:

 1. First "Quit" the Arduino IDE so ATTiny10IDE will have access to the Serial Port on the Arduino running the Sketch.
 2. Start ATTiny10IDE and use "**`Settings->Programmer`**" to open the Programmer dialog and select the "**`Arduino TPI`**" option.
 3. Open, or write the source code you want to compile and program into the ATtiny10.
 4. Select "**`Actions->Build`**" to compile. or assemble the code.
 5. Connect the Arduino to the ATTiny10 using the same connections shown above.
 5. Select "**`Actions->Read Device Signature`**" to verify the ATTiny10 is properly connected.
 6. Select "**`Actions->Program Flash and Fuse(s)`**" to upload and program the code and update the fuse settings into the ATtiny10.

<p align="center"><img src="images/Arduino%20Programmer.jpg" width="600" height="440"></p>

### Breakout Boards for the ATTiny10

I've also created a small, inexpensive PCB that implements the wiring shown in the above photo.  You can [order it from OSH Park](https://oshpark.com/shared_projects/ZBxayCTS), for $1.10 for three copies of the PCB.  _Note: I don't receive anything from OSH Park for offering this.  I do it just to make it a bit easier for you to program ATTiny10 chips using ATTiny10IDE._  In the following photo, the ATTiny10 is soldered to an SOT-23-6 to DIP 6 adapter PCB.  This kind of SMD to DIP adapter is widely available from many sources, or you can [order 3 copies of a SOT-23-6 to DIP 6 adapter I designed from OSH Park](https://oshpark.com/shared_projects/vuWx5EkE).  To complete the first adapter PCB you'll need two, [3 pin, 0.1 inch spacing female headers](https://www.pololu.com/product/1013), which serve as a socket for the SOT-23-6 to DIP adapter PCB, and a 5 pin strip of 0.1 inch spacing male headers, which you can snap off from a [40 pin, .1 inch breakaway header strip](https://www.pololu.com/product/965).  You also need two, 3 pin strips of the male header for the SOT-23-6 adapter PCB, too.

<p align="center"><img src="images/OSH%20Park%20Adapter.jpg" width="600" height="458"></p>

To use the programming adaptor, the purple card in the above photo, simply plug it in to data pins D2 through D6 on the Arduino and then plug the SOT-23-6 to DIP adapter, the green card in the above photo, into the female headers on the programming adapter PCB being careful to align pin 1 to pin one on both boards.  Once programmed, the SOT-23-6 to DIP adapter makes it easy to plug it into a breadboarded circuit to test your code.

Alternately, can order my [latest design for an ATTiny10 Breakout Board](https://oshpark.com/shared_projects/0ItcNmhS) from OSH park and get 3 copies for only 90 cents!  This includes free shipping.  As shown below, the pinout on the board is designed to plug into  pins 2-7 (see image below) on an Arduino running the programmer sketch, which makes it easy to program.  The resistor and LED are optional, but are designed to work with the "Blink" sketch included with ATTiny10IDE.

<p align="center"><img src="images/T10 Breakout.jpg" width="600" height="347"></p>

##### Ordering an ATTiny10

The part number foor the ATTiny10 is the ATTINY10-TSHR.  This parts is available from [Mouser](https://www.mouser.com/ProductDetail/Microchip-Technology-Atmel/ATTINY10-TSHR?qs=%2fha2pyFaduhIdMJpbSzRi4efM1PiRFAxlSPQ5HDlxN4auVxgwtBzjw%3d%3d) and [DigiKey](https://www.digikey.com/product-detail/en/microchip-technology/ATTINY10-TSHR/ATTINY10-TSHRCT-ND/2136158).  The price for one piece is about 33-34 cents, which makes it one of the cheapest microcontrollers yuo can buy in small quantities.