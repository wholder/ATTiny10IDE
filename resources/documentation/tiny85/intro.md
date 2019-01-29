## The ATTiny25/45/85

ATTiny10IDE supports coding for the ATTiny85 Series of Microcontrollers in C and C++ as well as assembly using the GNU AVR-C++ compiler to generate code.  The file extension you choose for your code file tells ATTiny10IDE how to process the code:

    .c   - Compile as C file using avr-gcc
    .cpp - Compile as C++ file using avr-g++
    .s   - Assemble using avr-as and link with avr-ld
    
So, this means you'll have to save your source file with the appropriate extension before you can compile, or assemble it.  In addition, you can also write [inline assembly code](https://web.stanford.edu/class/ee281/projects/aut2002/yingzong-mouse/media/GCCAVRInlAsmCB.pdf) in a C/C++ (.c) file.  You'll find examples of these different approaches to writing code in the [GitHub examples folder](https://github.com/wholder/ATTiny10IDE/tree/master/examples).

### Introducing the ATTiny85 Series Microcontrollers

The ATTiny10 series microcontrollers include the following devices, all of which are available in 8-pin PDIP, SOIC and TSSOP packages with speed ranges of 0–10 MHz @ 2.7-5.5V, 0-20 MHz @ 4.5-5.5V:

+ ATTiny25 - 2K bytes Flash, 128 bytes RAM, 128 bytes EEPROM
+ ATTiny45 - 4K bytes Flash, 256 bytes RAM, 256 bytes EEPROM
+ ATTiny85 - 8K bytes Flash, 512 bytes RAM, 512 bytes EEPROM

All devices in the ATTiny85 Series include two 8 bit timers (each with a prescaler and two PWM channels), a 4 channel, 10 bit ADC, a Programmable Watchdog Timer with Separate On-chip Oscillator, and an on-chip Analog Comparator.
