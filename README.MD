<p align="center"><img src="https://github.com/wholder/ATTiny10IDE/blob/master/images/ATTiny10IDE%20Screenshot.png"></p>


# ATTiny10IDE
ATTiny10IDE is a simple, GNU-based IDE I wrote to simplify writing code for the ATTiny10 Series Microcontrollers using CPP and Assembler.  It's a greatly expanded and updated version of a project I started back in 2014.  [Please read this article for some additional details](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer).  

If you just want to try out the program, you don't need to download and compile the source code, as I try to maintain a pre-built, executable JAR file in the [out/artifacts/ATTiny10IDE_jar](https://github.com/wholder/ATTiny10IDE/tree/master/out/artifacts/ATTiny10IDE_jar) folder from which you can download and run ATTiny10IDE as long as you have Java installed on your system.  On a Mac, simply double click the ATTiny10IDE.jar file to run it once you've downloaded it, although you'll probably have to right click and select "Open" the  first time you run ATTiny10IDE due to the Mac OS X security check and the fact that I'm currently unable to digitally sign the JAR file.  You should also be able to double click and run using Windows, but some versions of Windows may require you to [enable this ability](https://www.addictivetips.com/windows-tips/run-a-jar-file-on-windows/) first.  The current version does not run on Linux.
## Requirements
ATTiny10IDE requires Java 8 JRE or JDK, or later.
Note: I wrote ATTiny10IDE on a Mac Pro using the _Community_ version of [IntelliJ IDEA from JetBrains](https://www.jetbrains.com/idea/) and OS X is the only environment where I have extensively tested and used ATTiny10IDE .  I've included GNU toolchains for both Mac and Windows so it should also run on MS Windows.  Feel free to report any issues you discover.  I'll do my best, when time permits, to investigate them, but I cannot guarantee fixes, timely or otherwise.
## Using ATTiny10IDE
ATTiny10IDE supports coding for the ATTiny10 Series of Microcontrollers in C/C++ and uses the GNU AVR-C++ compiler to generate code.  In addition, ATTiny10IDE supports two ways to write and assemble code the ATTiny10.  One method uses the GNU AVR-AS assembler using a non Atmel-type syntax.  The other uses a [homebrew assembler I wrote that supports an Atmel-like syntax](https://sites.google.com/site/wayneholder/attiny-4-5-9-10-assembly-ide-and-programmer).  The file extension you choose for your code file tells ATTiny10IDE how to process the code:

    .c   - Compile as C/C++ file using AVR-G++
    .s   - Assemble using AVR-AS
    .asm - Assemble with my homebrew assembler
So, this means you'll have to save your source file with the appropriate extension before you can compile, or assemble it.  In addition, you can also write [inline assembly code](https://web.stanford.edu/class/ee281/projects/aut2002/yingzong-mouse/media/GCCAVRInlAsmCB.pdf) in a C/C++ (.c) file.  You'll find examples of these different approaches to writing code in the [examples folder](https://github.com/wholder/ATTiny10IDE/tree/master/examples).
## Programming ATTiny10 Chips
ATTiny10IDE supports several different ways to program your compiled code into an ATTiny10 chip.  One is to build and use the [Arduino-based, High Voltage programmer/emulator](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer) I designed for the original version of my IDE.  However, ATTiny10IDE also now supports a way to generate an Arduino Sketch (program) which, when run, can directly program an ATTiny10 chip using only Arduino I/O lines 2 - 6, like this:

    Arduino Pin 2  -> Pin 5 on ATtiny10
    Arduino Pin 3  // Pin 3 on ATtiny10
    Arduino Pin 4  // Pin 1 on ATtiny10
    Arduino Pin 5  // Pin 6 on ATtiny10
    Arduino Pin 6  // Pin 2 on ATtiny10
Here are the steps you can follow to program an ATTiny10 using this method:
 1. Use "**`File->Open`**" to load the source file, or type your code into the "Source Code" pane and save it using the appropriate file extension.
 2. Select "**`Actions->Build`**" to compile. or assemble the code.
 3. Select "**`Actions->Generate Arduino Programmer Code`**" and save to a .ino (Arduino Sketch) file when prompted.
 4. Quit ATTiny10IDE so it does not interfere with the Arduino IDE's access to the Arduino's serial port.
 5. Load the file you generated into the Arduino (it will prompt you to create a folder for this file.  Choose Yes.)
 6. Program the Sketch into the Arduino using the "Upload" Button.
 7. Open the "**Serial Monitor**" window in the Arduino IDE (upper right button) and set the baud rate to 115200.
 8. Follow the istructions the Sketch prints to connect the Arduino to the ATTiny10.
 9. Verify the ATTiny10 is properly connected by using the Identify ATtiny10 command by typing '`**`I`**`' and pressing "**`Send`**".
 10. If Identify ATtiny10 responds with "Device: ATtiny10", you are clear to program the ATtiny10 by typing '**`P`**' and pressing "**`Send`**"

You can use this Sketch to program as many ATTiny10s as needed, but you can also use it as a general-purpose ATTiny10 programmer to upload and program other code, like this:

 1. First "Quit" the Arduino IDE so ATTiny10IDE will have access to the Serial Port on the Arduino running the Sketch.
 2. Start ATTiny10IDE and set "**`Settings-Baud Rate`**" to 115200 and "**`Settings->Port`**" to select the Arduino running the Sketch.
 3. Open, or write the source code you want to compile and program into the ATtiny10.
 4. Select "**`Actions->Build`**" to compile. or assemble the code.
 5. Connect the Arduino to the ATTiny10 using the same connections shown above.
 5. Select "**`Actions->Device Signature`**" to verify the ATTiny10 is properly connected.
 6. Select "**`Actions->Program Device`**" to upload and program the code into the ATtiny10.

<p align="center"><img src="https://github.com/wholder/ATTiny10IDE/blob/master/images/Arduino%20Programmer.JPG"></p>

I've also created a small, inexpensive PCB that implements the wiring shown in the above photo.  Uou can [order it from OSH Park](https://oshpark.com/shared_projects/ZBxayCTS), for $1.10 for three copies of the PCB.  _Note: I don't receive anything from OSH Park for offering this.  I do it just to make it a bit easier for you to program ATTiny10 chips using ATTiny10IDE._  In the following photo, the ATTiny10 is soldered to an SOT-23-6 to DIP 6 adapter PCB.  This kind of SMD to DIP adapter is widely available from many sources, or you can [order 3 copies of a SOT-23-6 to DIP 6 adapter I designed from OSH Park](https://oshpark.com/shared_projects/vuWx5EkE).  To complete the first adapter PCB you'll need two, [3 pin, .1 inch spacing female headers](https://www.pololu.com/product/1013), which serve as a socket for the SOT-23-6 to DIP adapter PCB, and a 5 pin strip of .1 inch spacing male headers, which you can snap off from a [40 pin, .1 inch breakaway header strip](https://www.pololu.com/product/965).  You also need two, 3 pin strips of the male header for the SOT-23-6 adapter PCB, too.

<p align="center"><img src="https://github.com/wholder/ATTiny10IDE/blob/master/images/OSH%20Park%20Adapter.jpg"></p>

To use the programming adpater, simpy plug it in to data pins 2 through 6 on the Arduino and then plug the SOT-23-6 to DIP adapter into the female headers on the programming adapter PCB being careful to align pin 1.  Once programmed, the SOT-23-6 to DIP adapter makes it easy to plug it into a breadboarded circuit to test your code.

#### Setting Fuses
I've made some non standard additions using the `#pragma` directive as a way to provide a way to send additional information to the programmer, such as to program the 3 special bits, of "fuses" in the ATTiny10's Configuration Byte.  These are:
 1. The `RSTDISBL` fuse changes the RESET pin (pin 6) into an I/O pin. which gives you 4 I/O pins instead of 3.  However, **WARNING**, if you program this fuse you'll lose the ability to reprogram the ATTiny10 until this fuse is cleared ([a special, high voltage programmer is required to do this](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer)).  Also, if you do choose to use this pin for I/O it's best to use it only as an INPUT pin, since even a HV programmer can have trouble reseting the RSTDISBL fuse is pin 6 is used an an output.  Therefore, I recommend that you consider this fuse off limits until you are absolutely sure you know what you're doing, or have a stack of ATTiny10 chips to waste.
 2. The `WDTON` fuse, which forces the Watchdog Timer to alway be on.  You can enable the Watchdog Timer in software so, again this fuse is only needed for special applications.
 3. The `CLKOUT` fuse changes the function of `PORTB` `PB2` (pin 4) so that is ouputs the System Clock.
 
 To set the `WDTON` fuse, for example, simply add a line like this to the top of your source file:
 
     #pragma fuses WDTON

Additional fuse names can be added using a space, or a comma to separate them.
#### Setting the Clock Rate
If you choose to use the AVRLIB function `_delay()` to code software delay loops (see the `DelayBlink.c` example program) you need to tell the compiler what clock rate you intend to program into the ATTiny10 so that it can calculate the proper number of iterations for the delay loops.  To do this, add a line of like like this near the top of your source file:

    #pragma clock 8000000
    
The above line, for example, tells the compiler you intend to run the ATTiny10 at 8 MHz.  Note: using this directive does not program the ATTiny10 to run at 8 MHz.  It simply tells the compiler to assume the ATTiny10 is running at this rate.  To actually set the clock rate you'll need to add code that configures the ATTiny10's clock prescaler to an appropriate value at startup.  Again, see the `DelayBlink.c` example program to see how to set the ATTiny10 to run at 8 MHz.
#### Calibrating the ATTiny10's Internal Clock
This ATTiny10 contains an internal clock that's roughly calibrated at the factory.  However, the accuracy of this clock can vary with temperature and the voltage at which the ATTiny10 operates.  The accuracy of the ATTiny10's clock can be important for applications that are timing critical, sush as implementing serial communication with software loops.  To more closely calibrate the clock, ATTiny10IDE has a feature that lets you run a small test program that compares the ATTiny10's clock against the Arduino's clock, which is usually set by a more precise, crystal oscillator (some Arduinos use a far less accurate ceramic "resonator" instead of a crystal and these are not recommended as a reference for calibraing the ATTiny10's clock.)

To calibrate an ATTiny10's clock, choose "`Calibrate Clock`" from the "`Actions`" menu.  This will cause ATTiny10IDE to download and run a small program to the ATTiny10 that will iteratively test various calibration values and print out the one it chooses as the best.  However, you must then add some code to your program to push this value into the OSCCAL register at startup.  See `DelayBlink.c` example program to see how this is done.  Note: each ATTiny10 will require a unique calibation value, so this step will need to be repeated for each ATTiny chip you program that requires a more accurately calibrated clock.
## Credit and Thanks
This project would have been much harder and much less cool without help from the following open source projects, or freely available software.
- [Java Simple Serial Connector 2.8.0](https://github.com/scream3r/java-simple-serial-connector) JSSC is used to communicate with the Arduino-based programmer
- [JSyntaxPane](https://github.com/nordfalk/jsyntaxpane) Provides C/C++ Syntax Highlighting for ATTiny10IDE
- [IntelliJ IDEA from JetBrains](https://www.jetbrains.com/idea/) (my favorite development environment for Java coding. Thanks JetBrains!)
## MIT License
Copyright 2014-2017 Wayne Holder

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.