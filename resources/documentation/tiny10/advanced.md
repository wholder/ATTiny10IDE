## Advanced Experimental Features

The following sections cover advanced topics that assume you have some knowledge of the hardware architecture of the ATTiny series of microconttrollers.  To understand these sections, you should be familiar with the latest version of the [Atmel Datasheet for the ATiny4, ATiny5, ATiny9, ATiny10 8-bit AVR Microcontrollers](http://www.atmel.com/images/atmel-8127-avr-8-bit-microcontroller-attiny4-attiny5-attiny9-attiny10_datasheet.pdf).

#### Automatic Prototype Generation

If enabled in the Preferences Menu, the Build process will attempt to generate function prototypes for all functions in the main source file before running the compiler.  Note: this code is still experimental and is currently unable to correctly parse certain C++ constructs.  Use at your own discretion.

#### Setting Fuses

I've made some non standard additions using the `#pragma` directive as a way to provide a way to send additional information to the programmer, such as to program the 3 special bits, of "fuses" in the ATTiny10's Configuration Byte.  These are:

 1. The `RSTDISBL` fuse changes the RESET pin (pin 6) into an I/O pin. which gives you 4 I/O pins instead of 3.  However, **WARNING**, if you program this fuse you'll lose the ability to reprogram the ATTiny10 until this fuse is cleared ([a special, high voltage programmer is required to do this](https://sites.google.com/site/wayneholder/attiny10-c-ide-and-improved-device-programmer)).  Also, if you do choose to use this pin for I/O it's best to use it only as an INPUT pin, since even a HV programmer can have trouble reseting the RSTDISBL fuse is pin 6 is used an an output.  Therefore, I recommend that you consider this fuse off limits until you are absolutely sure you know what you're doing, or have a stack of ATTiny10 chips to waste.
 2. The `WDTON` fuse, which forces the Watchdog Timer to alway be on.  You can enable the Watchdog Timer in software so, again this fuse is only needed for special applications.
 3. The `CLKOUT` fuse changes the function of `PORTB` `PB2` (pin 4) so that is outputs the System Clock.
 
To set the `WDTON` fuse, for example, simply add a line like this to the top of your source file:
 
    #pragma fuses WDTON

Additional fuse names can be added using a space, or a comma, to separate them.

#### Setting the Clock Rate

If you choose to use the AVRLIB function `_delay_ms()` to code software delay loops you need to tell the compiler what clock rate you intend to program into the ATTiny10 so that it can calculate the proper number of iterations for the delay loops.  To do this, add a line of like like this near the top of your source file:

    #pragma clock 8000000
    
The above line, for example, tells the compiler you intend to run the ATTiny10 at 8 MHz.  Note: using this directive does not program the ATTiny10 to run at 8 MHz.  It simply tells the compiler to assume the ATTiny10 is running at this rate.  To actually set the clock rate you'll need to add code that configures the ATTiny10's clock pre-scaler to an appropriate value at startup.  Note: if you're including `Arduino.h`, the clock rate is automatically set to 8 MHz.

#### Calibrating the ATTiny10's Internal Clock

This ATTiny10 contains an internal clock that's roughly calibrated at the factory.  However, the accuracy of this clock can vary with temperature and the voltage at which the ATTiny10 operates.  The accuracy of the ATTiny10's clock can be important for applications that are timing critical, such as implementing serial communication with software loops.  To more closely calibrate the clock, ATTiny10IDE has a feature that lets you run a small test program that compares the ATTiny10's clock against the Arduino's clock, which is usually set by a more precise, crystal oscillator (some Arduinos use a far less accurate ceramic "resonator" instead of a crystal and these are not recommended as a reference for calibrating the ATTiny10's clock.)

To calibrate an ATTiny10's clock, choose "`Calibrate Clock`" from the "`Actions`" menu.  This will cause ATTiny10IDE to download and run a small program to the ATTiny10 that will iteratively test various calibration values and print out the one it chooses as the best.  However, you must then add some code to your program to push this value to set the OSCCAL register at startup.  See `DelayTimer.c` example program to see how this is done.  Note: each ATTiny10 will require a unique calibartion value, so this step will need to be repeated for each ATTiny chip you program that requires a more accurately calibrated clock.

#### Device Selection

In many of the example source files you'll see a line of text that reads "`#pragma chip attiny10`", which is used to tell the compiler or assembler what type of AVR chip to target.  ATTiny10 is the value used if you omit this line.  You could use this to select one of the chips in the ATTiny10 family, such as the ATTiny4, 5, or 9, but there is little reason to do so, as these chips are less capable than the ATTiny10 and don't cost significantly less, if you are even able to purchase them.


