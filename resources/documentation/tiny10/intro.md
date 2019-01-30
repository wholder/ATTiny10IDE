ATTiny10IDE supports coding for the ATTiny10 Series of Microcontrollers in C and C++ using the GNU AVR-C++ compiler to generate code.  In addition, ATTiny10IDE supports two ways to write and assemble code for the ATTiny10.  One method uses the GNU AVR-AS assembler using a non Atmel-type syntax.  The other uses a [homebrew assembler I wrote that supports an Atmel-like syntax](https://sites.google.com/site/wayneholder/attiny-4-5-9-10-assembly-ide-and-programmer).  The file extension you choose for your code file tells ATTiny10IDE how to process the code:

    .c   - Compile as C file using avr-gcc
    .cpp - Compile as C++ file using avr-g++
    .s   - Assemble using avr-as and link with avr-ld
    .asm - Assemble with my homebrew assembler (Note: only supports the ATTiny10)
    
So, this means you'll have to save your source file with the appropriate extension before you can compile, or assemble it.  In addition, you can also write [inline assembly code](https://web.stanford.edu/class/ee281/projects/aut2002/yingzong-mouse/media/GCCAVRInlAsmCB.pdf) in a C/C++ (.c) file.  You'll find examples of these different approaches to writing code in the [GitHub examples folder](https://github.com/wholder/ATTiny10IDE/tree/master/examples).

## Introducing the ATTiny4/5/9/10 Series Microcontrollers

The ATTiny10 series microcontrollers include the following devices, all of which are available in 6-pin SOT and 8-pad UDFN packages in the following speed grades 0 - 4 MHz @ 1.8 - 5.5V, 0 - 8 MHz @ 2.7 - 5.5V and 0 - 12 MHz @ 4.5 - 5.5V:

### ATTTiny4/5/9/10 configurations

+ ATTiny4 - 512 bytes Flash, 32 bytes RAM
+ ATTiny5 - 512 bytes Flash, 32 bytes RAM with 4-channel, 8-bit ADC
+ ATTiny9 - 1024 bytes Flash, 32 bytes RAM
+ ATTiny10 - 1024 bytes Flash, 32 bytes RAM with 4-channel, 8-bit ADC

All devices in the ATTiny10 Series include a 16 bit timer with Pre-scaler and Two PWM Channels, Programmable Watchdog Timer with Separate On-chip Oscillator and an on-chip Analog Comparator.

#### Analog to Digital converters
The ATTiny5 and ATTiny10 each have 4 channel, 8-bit ADC converters. The ATTiny4 and ATTiny9 do not.

