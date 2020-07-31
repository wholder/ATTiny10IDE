
<p align="center"><img src="/images/ATTiny10IDE%20Screenshot.png"></p>

**Update:** As of July 29, 2020, In order to support the 64 bit-only Catalina release of macOs, I have revised the code to use _thinned_ versions of [Microchip's 64 bit GNU Toolchains](https://www.microchip.com/mplab/avr-support/avr-and-arm-toolchains-c-compilers) rather than using 32 bit GNU toolchains extracted from the Arduino project.

# ATTiny10IDE (for 64 bit macOS, Windows & Linux)
ATTiny10IDE is a simple, GNU-based IDE I originally wrote to simplify writing code for the ATTiny10 Series Microcontrollers using C. C++ or Assembly language.  This new version is a greatly expanded and updated version I started working on in 2014.  [See this article for some additional details on how this project started](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer).  For this new release, I've added support for the ATTiny25/45/85 and ATTiny24/44/84 series of microcontrollers, as well support for programming them using an ICSP Programmer.  And, there's also an experimental feature for automatic generation of function prototypes.

If you just want to try out the program, you don't need to download and compile the source code, as I maintain a pre-built, executable JAR file in the [out/artifacts/ATTiny10IDE_jar](https://github.com/wholder/ATTiny10IDE/tree/master/out/artifacts/ATTiny10IDE_jar) folder from which you can download and run ATTiny10IDE as long as you have Java installed on your system.  On a Mac, simply double click the ATTiny10IDE.jar file to run it once you've downloaded it, although you'll probably have to right click and select "Open" the  first time you run ATTiny10IDE due to the Mac OS X security check and the fact that I'm currently unable to digitally sign the JAR file.  You should also be able to double click and run using Windows, but some versions of Windows may require you to [enable this ability](https://www.addictivetips.com/windows-tips/run-a-jar-file-on-windows/) first.  You'll need to give the .jar file RUN permission to execute on Linux and using a USB-based device programmer, such as the AVRISP MkII, may also require you to [create an install a rules file](https://stackoverflow.com/questions/5412727/avrisp-mkii-doesnt-work-with-avrdude-on-linux).

## Recent Updates

 - Now using thinned versions of [Microchip's 64 bit GNU Toolchains](https://www.microchip.com/mplab/avr-support/avr-and-arm-toolchains-c-compilers)
 - Reorganized Action menu and added Target menu
 - Switched TinyX5 and TinyX4 cores and libs to [ATTinyCore by Spence Konde](https://github.com/SpenceKonde/ATTinyCore)
 - Now uses .d files to determine dependencies for included library files
 - New Programmer selection dialog replaces menu-based selection and configuration
 - Fixed a regression that prevented Windows version from working properly
 - Fixed linker command so passing string literals as parameters now works (long-standing bug)
 - Added ability to save HEX file and LST file
 
## Help Requested

So far this is just a one man project and I struggle at times to do adequate testing, especially on Windows and Linux, when I release a nw version. So, if anyone spots a problem, or has a question or a sugegstion, [please post an issue](https://github.com/wholder/ATTiny10IDE/issues) so I can investigate.  You can also email me at questions4wayne at gmail dot com.

## Requirements
I suggest using [Java 8 JRE](https://www.java.com) or JDK, or later for ATTiny10IDE, but the code also seems to run under the OpenJDK Java on Linux.  Note: I wrote ATTiny10IDE on a Mac Pro using the _Community_ version of [IntelliJ IDEA from JetBrains](https://www.jetbrains.com/idea/) and OS X is the only environment where I have extensively tested and used ATTiny10IDE .  I've included GNU/AVR toolchains for Mac and 64 bit Windows and 64 bit Linux, but have not extensively tested the code under Windows or Linux.  Feel free to report any issues you discover.  I'll do my best, when time permits, to investigate them, but I cannot guarantee fixes, timely or otherwise.
## Using ATTiny10IDE
ATTiny10IDE supports coding for the ATTiny10 Series of Microcontrollers in C and C++ using the GNU AVR-C++ compiler to generate code.  In addition, ATTiny10IDE supports two ways to write and assemble code the ATTiny10.  One method uses the GNU AVR-AS assembler using a non Atmel-type syntax.  The other uses a [homebrew assembler I wrote that supports an Atmel-like syntax](https://sites.google.com/site/wayneholder/attiny-4-5-9-10-assembly-ide-and-programmer).  The file extension you choose for your code file tells ATTiny10IDE how to process the code:

    .c   - Compile as C file using avr-gcc
    .cpp - Compile as C++ file using avr-g++
    .s   - Assemble using avr-as and link with avr-ld
    .asm - Assemble with my homebrew assembler (Note: only supports the ATTiny10)
    
So, this means you'll have to save your source file with the appropriate extension before you can compile, or assemble it.  In addition, you can also write [inline assembly code](https://web.stanford.edu/class/ee281/projects/aut2002/yingzong-mouse/media/GCCAVRInlAsmCB.pdf) in a C/C++ (.c) file.  You'll find examples of these different approaches to writing code in the [examples folder](https://github.com/wholder/ATTiny10IDE/tree/master/examples).
## Programming ATTiny10 Chips
ATTiny10IDE supports several different ways to program your compiled code into an ATTiny10 chip.  One is to build and use the [Arduino-based, High Voltage programmer/emulator](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer) I designed for the original version of my IDE.  However, ATTiny10IDE also now supports a way to generate an Arduino Sketch (program) which, when run, can directly program an ATTiny10 chip using only Arduino I/O lines 2 - 6, like this:

<p align="center"><img src="/images/TPI-Prog-Socket.png"></p>
    
Here are the steps you can follow to program an ATTiny10 using this method:
 1. Use "**`File->Open`**" to load the _source_ file, or type your code into the "Source Code" pane and save it using the appropriate file extension.
 2. Select "**`Actions->Build`**" to compile. or assemble the code.
 3. Select "**`Actions->Generate Arduino Programmer Code`**" and save to a .ino (Arduino Sketch) file when prompted.
 4. Quit ATTiny10IDE so it does not interfere with the Arduino IDE's access to the Arduino's serial port.
 5. Load the file you generated into the Arduino (it will prompt you to create a folder for this file.  Choose Yes.)
 6. Program the Sketch into the Arduino using the "Upload" Button.
 7. Open the "**Serial Monitor**" window in the Arduino IDE (upper right button) and set the baud rate to 115200.
 8. Follow the istructions the Sketch prints to connect the Arduino to the ATTiny10.
 9. Verify the ATTiny10 is properly connected by using the Identify ATtiny10 command by typing '**`I`**' and pressing "**`Send`**".
 10. If Identify ATtiny10 responds with "Device: ATtiny10", you are clear to program the ATtiny10 by typing '**`P`**' and pressing "**`Send`**"

You can use this Sketch to program as many ATTiny10s as needed, but you can also use it as a general-purpose ATTiny10 programmer to upload and program other code, like this:

 1. First "Quit" the Arduino IDE so ATTiny10IDE will have access to the Serial Port on the Arduino running the Sketch.
 2. Start ATTiny10IDE and use "**`Settings->Programmer`**" to open the Programmer dialog and select the "Arduino TPI" option.
 3. Open, or write the source code you want to compile and program into the ATtiny10.
 4. Select "**`Actions->Build`**" to compile. or assemble the code.
 5. Connect the Arduino to the ATTiny10 using the same connections shown above.
 5. Select "**`Actions->Read Device Signature`**" to verify the ATTiny10 is properly connected.
 6. Select "**`Actions->Program Flash and Fuse(s)`**" to upload and program the code into the ATtiny10.

<p align="center"><img src="/images/Arduino%20Programmer.JPG" width="600" height="440"></p>

I've also created a small, inexpensive PCB that implements the wiring shown in the above photo.  You can [order it from OSH Park](https://oshpark.com/shared_projects/ZBxayCTS), for $1.10 for three copies of the PCB.  _Note: I don't receive anything from OSH Park for offering this.  I do it just to make it a bit easier for you to program ATTiny10 chips using ATTiny10IDE._  In the following photo, the ATTiny10 is soldered to an SOT-23-6 to DIP 6 adapter PCB.  This kind of SMD to DIP adapter is widely available from many sources, or you can [order 3 copies of a SOT-23-6 to DIP 6 adapter I designed from OSH Park](https://oshpark.com/shared_projects/vuWx5EkE).  To complete the first adapter PCB you'll need two, [3 pin, .1 inch spacing female headers](https://www.pololu.com/product/1013), which serve as a socket for the SOT-23-6 to DIP adapter PCB, and a 5 pin strip of .1 inch spacing male headers, which you can snap off from a [40 pin, .1 inch breakaway header strip](https://www.pololu.com/product/965).  You also need two, 3 pin strips of the male header for the SOT-23-6 adapter PCB, too.

<p align="center"><img src="/images/OSH%20Park%20Adapter.jpg" width="600" height="458"></p>

To use the programming adpater, simply plug it in to data pins 2 through 6 on the Arduino and then plug the SOT-23-6 to DIP adapter into the female headers on the programming adapter PCB being careful to align pin 1 to pin one on both boards.  Once programmed, the SOT-23-6 to DIP adapter makes it easy to plug it into a breadboarded circuit to test your code.
## Include `Arduino.h` to code "sketches" in ATTiny10IDE
When coding in C or C++, ATTiny10IDE expects you to put your code in a function named `main()` and handle all the details of using the low-level ATTiny10 architecture which involves ading and writing to hardware registers to perform I/O operations.  However, if your file starts by including the header file `Arduino.h`, like this:

    #include "Ardiuno.h"
    
you'll be able to write code in a way that is familiar to users of the Arduino IDE.  Instead of writing a `main()` function, your one-time initialization code will go in a function named `setup()` and code that executes again and again will go into a function named `loop()`.  Arduino calls a program like this a "sketch".  The example program `Blank.h` is a sample starting sketch you can copy to start coding.  It looks like this:

    #include "Arduino.h"

    void setup() {
      // put your setup code here, to run once:
    }
    
    void loop() {
      // put your main code here, to run repeatedly:
    }
    
#### ATTiny10 Convenience Functions
In addition, `Arduino.h` provides a set of convenience functions you can call to do digital and analog I/O in a fashion similar to how the Arduino IDE handles these operations.  These include `pinMode()`, `digitalWrite()`, `digitalRead()`, `analogWrite()`, `analogRead()`, `delay()` and `delayMicroseconds()`.  The digital I/O functions are implemented as `#define` macros, which the GNU compiler is able to efficient convert to quite optimal code.  But, be aware that you'll get the most compact and efficient code when the values passed for pin numbers resolve to constants at compile time.  For example, a call to `digitalWrite(2, HIGH)` compiles to the single instruction, `sbi	0x02,2`, in ATTiny10 assembly language.

Calls to `analogRead()` and `analogWrite()` go to actual C functions because some initialization steps are required the first time one of these functions is called.  For this reason, trying to use the same pin for both analog and digital I/o is not recommended, as the initialization code will not run a second time.  If you need to do this kind of function switching with pins, you'll need to develve into how to write code that directly manipulates the low-level I/O registers in the ATTiny10.  Note: check out some of the more advanced example programs, such as `TimerBlink.c` and `PulsingLED.c`, to see how you can use C/C++ code to directly access the ATTiny10's I/O registers.

Calls to `delay()` and `delayMicroseconds()` are also handled by `#define` macros but redirect to the [AVRlib](http://www.nongnu.org/avr-libc/) functions `_delay_ms()` and `_delay_us()`, respectively. The timing of these macros depends on the  `#pragma clock` to directive (see "Advanced Features") tell the compiler what clock speed the ATTiny10 is using, which the compiler needs to calculate the number of loop iterations needed for a specific delay.  But, by default, this is handled automatically and the clock for code using `Arduino.h` is set to 8 MHz.

If a different clock speed is needed, you can use the `clock` pragma and set it one of the following values `8000000`, `4000000`, `2000000`, `1000000`, `500000`, `250000`, `125000,` `62500`, or `31250`.  Be sure to insert the pragma before the lined that includes `Arduino.h` because code in `Arduino.h` depends on the value you select.  For example:

    #pragma clock 2000000    // Set clock to 2 MHz
    #include "Arduino.h"

Note: `analogRead()` depends on the setting of the system clock to set the clock prescaler used by the ADC, so `CLK_250000` is the lowest clock speed recommended if you are using `analogRead()`.

Note: You can also set a custom frequency using `#pragma clock` which will set the system clock prescaler to 1:1.  However, this feature is intended for use with other chips in the ATTiny family.

## Dealing with the ATTiny10's Limited RAM Space
The ATTiny10 prpovides only 32 bytes of RAM space for variables and the program call stack.  So, it's important to use `char` and `unsigned char` variables instead of `int` and `unsigned int` wherever possible and limit function call depth (recursion is not recommended), use stack variables carefully (variables declared in functions) and globally-declared arrays.  Fortunately, the AVR architecture has 32 registers available and the GNU compiler makes very eficient use of them, but there are still a few tricks that can help keep RAM usage under control.

One essential trick is [using the `PROGMEM` directive](http://www.atmel.com/webdoc/avrlibcreferencemanual/pgmspace_1pgmspace_data.html) to create `const` arrays that are stored in Flash memory rather than RAM.  This only works for read only arrays, but using it for table lookup, or strings of text makes it possible to do things would be impossible without it.  The example program `Morse.c` shows how to use `PROGMEM` to store both a lookup table for ASCII to Morse code conversion as well as for a `char[]` string for the message text.

## Advanced Experimental Features
The following sections cover advanced topics that assume you have some knowledge of the hardware architecture of the ATTiny series of microconttrollers.  To understand these sections, you should be familiar with the latest version of the [Atmel Datasheet for the ATiny4, ATiny5, ATiny9, ATiny10 8-bit AVR Microcontrollers](http://www.atmel.com/images/atmel-8127-avr-8-bit-microcontroller-attiny4-attiny5-attiny9-attiny10_datasheet.pdf).

#### Automatic Prototype Generation
If enabled in the Preferences Menu, the Build process will atempt to generate function prototypes for all functions in the main source file before running the compiler.  Note: this code is still experimental and ir currently unable to correctly parse certain C++ constructs.  Use at your own discretion.

#### Setting Fuses
I've made some non standard additions using the `#pragma` directive as a way to provide a way to send additional information to the programmer, such as to program the 3 special bits, of "fuses" in the ATTiny10's Configuration Byte.  These are:

 1. The `RSTDISBL` fuse changes the RESET pin (pin 6) into an I/O pin. which gives you 4 I/O pins instead of 3.  However, **WARNING**, if you program this fuse you'll lose the ability to reprogram the ATTiny10 until this fuse is cleared ([a special, high voltage programmer is required to do this](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer)).  Also, if you do choose to use this pin for I/O it's best to use it only as an INPUT pin, since even a HV programmer can have trouble reseting the RSTDISBL fuse is pin 6 is used an an output.  Therefore, I recommend that you consider this fuse off limits until you are absolutely sure you know what you're doing, or have a stack of ATTiny10 chips to waste.
 2. The `WDTON` fuse, which forces the Watchdog Timer to alway be on.  You can enable the Watchdog Timer in software so, again this fuse is only needed for special applications.
 3. The `CLKOUT` fuse changes the function of `PORTB` `PB2` (pin 4) so that is ouputs the System Clock.
 
To set the `WDTON` fuse, for example, simply add a line like this to the top of your source file:
 
    #pragma fuses WDTON

Additional fuse names can be added using a space, or a comma to separate them.

#### Setting the Clock Rate
If you choose to use the AVRLIB function `_delay_ms()` to code software delay loops you need to tell the compiler what clock rate you intend to program into the ATTiny10 so that it can calculate the proper number of iterations for the delay loops.  To do this, add a line of like like this near the top of your source file:

    #pragma clock 8000000
    
The above line, for example, tells the compiler you intend to run the ATTiny10 at 8 MHz.  Note: using this directive does not program the ATTiny10 to run at 8 MHz.  It simply tells the compiler to assume the ATTiny10 is running at this rate.  To actually set the clock rate you'll need to add code that configures the ATTiny10's clock prescaler to an appropriate value at startup.  Note: if you're including `Arduino.h`, the clock rate is automatically set to 8 MHz.

#### Calibrating the ATTiny10's Internal Clock
This ATTiny10 contains an internal clock that's roughly calibrated at the factory.  However, the accuracy of this clock can vary with temperature and the voltage at which the ATTiny10 operates.  The accuracy of the ATTiny10's clock can be important for applications that are timing critical, such as implementing serial communication with software loops.  To more closely calibrate the clock, ATTiny10IDE has a feature that lets you run a small test program that compares the ATTiny10's clock against the Arduino's clock, which is usually set by a more precise, crystal oscillator (some Arduinos use a far less accurate ceramic "resonator" instead of a crystal and these are not recommended as a reference for calibraing the ATTiny10's clock.)

To calibrate an ATTiny10's clock, choose "`Calibrate Clock`" from the "`Actions`" menu.  This will cause ATTiny10IDE to download and run a small program to the ATTiny10 that will iteratively test various calibration values and print out the one it chooses as the best.  However, you must then add some code to your program to push this value into the OSCCAL register at startup.  See `TimerBlink.c` example program to see how this is done.  Note: each ATTiny10 will require a unique calibation value, so this step will need to be repeated for each ATTiny chip you program that requires a more accurately calibrated clock.

#### Device Selection
In many of the example source files you'll see a line of text that reads "`#pragma chip attiny10`", which is used to tell the compiler or assembler what type of AVR chip to target.  ATTiny10 is the default value used if you omit this line.  You could use this to select one of the chips in the ATTiny10 family, such as the ATTiny4, 5, or 9, but there is little reason to do so, as these chips are less capable than the ATTiny10 and don't cost significantly less, if you are even able to purchase them.

## Support for other ATTiny Chips (Warning, Beta status)

ATTiny10IDE currently supports using the ATTiny25/45/85 and ATTiny24/44/84 using ATTiny libraries originally developed by David A. Mellis, but later extended and improved by Spence Konde, James Sleeman and many others (see library headers and source files for further info).  As with the ATTiny10 series, ATTiny10IDE allows you to code in assembly (as a .s file), or plain C, or C++ and a `main()` function.  Or, if you include the "Arduino.h" header, ATTiny10IDE will then support coding as an Arduino-like sketch using `setup()` and `loop()` functions.  Here's an example of a basic "Blink" sketch for the ATTiny85 written like an Arduino sketch:

    #pragma chip attiny85
    #pragma efuse 0xFF        // default value
    #pragma hfuse 0xDF        // default value
    #pragma lfuse 0x62        // default value
    
    #include "Arduino.h"
    
    void setup() {
      pinMode(0, OUTPUT);
    }
    
    void loop() {
      digitalWrite(0, HIGH);
      delay(500);
      digitalWrite(0, LOW);
      delay(500);
    }

Notice the use of `#pragma` statements to select the chip type (`attiny85`) as well as the values for the fuses.  In addition, the `define` pragma allows you to pass in values to the compiler to enable, or disable certain features of the Arduino-compatible library code.

## Credit and Thanks

This project would have been much harder and much less cool without help from the following open source projects, or freely available software.

- [Java Simple Serial Connector 2.8.0](https://github.com/scream3r/java-simple-serial-connector) - JSSC is used to communicate with the Arduino-based programmer
- [JSyntaxPane](https://github.com/nordfalk/jsyntaxpane) - Now uses [CppSyntaxPane](https://github.com/wholder/CppSyntaxPane), which is based on JSyntaxPane.
- [IntelliJ IDEA from JetBrains](https://www.jetbrains.com/idea/) (my favorite development environment for Java coding. Thanks JetBrains!)
- [ANTLR version 4.7.1](http://www.antlr.org) - ALTLRv4 was used to generate the parser for the new (experimental) automatic prototype type generation feature.
- [CPP14 Antlr Grammar](https://github.com/antlr/grammars-v4/blob/master/cpp/CPP14.g4) - CPP14 was used as the base, C++ grammer for the automatic prototype type generation feature.
- Bus Pirate photo by [Tylerl at English Wikipedia, CC BY 3.0](https://commons.wikimedia.org/w/index.php?curid=57110132)

## MIT License

Copyright 2014-2019 Wayne Holder

Permission is hereby granted, free of charge, to any person obtaining a copy of this software and associated documentation files (the "Software"), to deal in the Software without restriction, including without limitation the rights to use, copy, modify, merge, publish, distribute, sublicense, and/or sell copies of the Software, and to permit persons to whom the Software is furnished to do so, subject to the following conditions:

The above copyright notice and this permission notice shall be included in all copies or substantial portions of the Software.

THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY, FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS IN THE SOFTWARE.