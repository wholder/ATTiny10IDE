<p align="center"><img src="https://github.com/wholder/ATTiny10IDE/blob/master/images/ATTiny10IDE%20Screenshot.png"></p>


# ATTiny10IDE
ATTiny10IDE is a simple, GNU-based IDE I wrote to simplify writing code for the ATTiny10 Series Microcontrollers using CPP and Assembler.  It's a greatly expanded and updated version of a project I started back in 2014.  [Please read this article for some additional details](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer).  

If you just want to try out the program, you don't need to download and compile the source code, as I try to maintain a pre-built, executable JAR file in the [out/artifacts/ATTiny10IDE_jar](https://github.com/wholder/ATTiny10IDE/tree/master/out/artifacts/ATTiny10IDE_jar) folder from which you can download and run ATTiny10IDE as long as you have Java installed on your system.  On a Mac, simply double click the ATTiny10IDE.jar file to run it once you've downloaded it, although you'll probably have to right click and select "Open" the  first time you run ATTiny10IDE due to the Mac OS X security check and the fact that I'm currently unable to digitally sign the JAR file.  You should also be able to double click and run using Windows, but some versions of Windows may require you to [enable this ability](https://www.addictivetips.com/windows-tips/run-a-jar-file-on-windows/) first.  The current version does not run on Linux.
## Requirements
ATTiny10IDE requires Java 8 JRE or JDK, or later.
Note: I wrote ATTiny10IDE on a Mac Pro using the _Community_ version of [IntelliJ IDEA from JetBrains](https://www.jetbrains.com/idea/) and OS X is the only environment where I have extensively tested and used ATTiny10IDE .  I've included GNU toolchains for both Mac and Windows so it should also run on MS Windows.  Feel free to report any issues you discover.  I'll do my best, when time permits, to investigate them, but I cannot guarantee fixes, timely or otherwise.
## Using ATTiny10IDE
ATTiny10IDE supports coding for the ATTiny10 Series of Microcontrollers in C/C++ and uses the GNU AVR-C++ compiler to generate code.  In addition, ATTiny10IDE supports two ways to write and assemble code the ATTiny10.  One method uses the GNU AVR-AS assembler using a non Atmel-type syntax.  The other uses a [homebrew assembler I wrote that supports an Atmel-like syntax](https://sites.google.com/site/wayneholder/attiny-4-5-9-10-assembly-ide-and-programmer).  The file extension you choose for your code file tells ATTiny10IDE how to process the code:

    .c   - Compile as C/C++ file
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

I've also created a small, inexpensive PCB you can [order from OSH Park](https://oshpark.com/shared_projects/ZBxayCTS), for $1.10 for three copies of the PCB.  Note: I don't receive anything from OSH Park for offering this.  I do it just to make it a bit easier for you to program ATTiny10 chips using ATTiny10IDE.  In the following photo, the ATTiny10 is soldered to an SOT-23-6 to DIP 6 adapter PCB.  This a widely available part you can order on eBay, or you can also [order 3 copies of a SOT-23-6 to DIP 6 adapter I designed from OSH Park](https://oshpark.com/shared_projects/vuWx5EkE).  To complete the first adapter PCB you'll need two, 3 pin, .1 inch spacing female headers, which serve as a socket for the SOT-23-6 to DIP adapter PCB, and a 5 pin strip of .1 inch spacing male headers.  YOu also need two, 3 pin strips of the male header for the SOT-23-6 adapter PCB.

<p align="center"><img src="https://github.com/wholder/ATTiny10IDE/blob/master/images/OSH%20Park%20Adapter.jpg"></p>

To use the programming adpater, simpy plug it in to data pins 2 through 6 on the Arduino and then plug the SOT-23-6 to DIP adapter into the female headers on the programming adapter PCB being careful to align pin 1.  Once programmed, the SOT-23-6 to DIP adapter makes it easy to plug it into a breadboarded circuit to test your code.

## Credit and Thanks
- [Java Simple Serial Connector 2.8.0](https://github.com/scream3r/java-simple-serial-connector) JSSC is used to communicate with the Arduino-based programmer
- [JSyntaxPane](https://github.com/nordfalk/jsyntaxpane) Provides C/C++ Syntax Highlighting for ATTiny10IDE
- [IntelliJ IDEA from JetBrains](https://www.jetbrains.com/idea/) (my favorite development environment for Java coding. Thanks JetBrains!)
## MIT License
Copyright 2014-2017 Wayne Holder

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.