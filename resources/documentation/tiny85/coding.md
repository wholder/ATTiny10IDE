## Writing Code for the ATTiny25/45/85

ATTiny10IDE currently supports using the ATTiny25/45/85 and ATTiny24/44/84 using ATTiny libraries originally developed by David A. Mellis, but later extended and improved by Spence Konde, James Sleeman and many others (see library headers and source files for further info).  As with the ATTiny10 series, ATTiny10IDE allows you to code in assembly (as a .s file), or plain C, or C++ and a `main()` function.  Or, if you include the "Arduino.h" header, ATTiny10IDE will then support coding as an Arduino-like sketch using `setup()` and `loop()` functions.  Here's an example of a basic "Blink" sketch for the ATTiny85 written like an Arduino sketch:

    #pragma chip attiny85
    #pragma efuse 0xFF        // default value
    #pragma hfuse 0xDF        // default value
    #pragma lfuse 0x62        // default value
    
    #include "Arduino.h"
    
    void setup() {
      pinMode(0, OUTPUT);       // Setup pin 5 as an OUTPUT
    }
    
    void loop() {
      digitalWrite(0, HIGH);    // Turn on LED on pin 5
      delay(500);
      digitalWrite(0, LOW);     // Turn off LED on pin 5
      delay(500);
    }

Notice the use of `#pragma` statements to select the chip type (`attiny85`) as well as the values for the fuses.  In addition, the `define` pragma allows you to pass in values to the compiler to enable, or disable certain features of the Arduino-compatible library code. 

## Writing C-style Code for the ATTiny4/5/9/10

For comparison, here's functionally the same program as the Arduino-style one shown above, but which is coded in C and uses a `main()` function instead of `setup()` and `loop()` methods:

    #pragma chip attiny85
    
    #pragma lfuse 0x62          // default value
    #pragma hfuse 0xDF          // default value
    #pragma efuse 0xFF          // default value
    
    #include <avr/io.h>
    #include <util/delay.h>
    
    int main () {
      // put your setup code here, to run once:
        DDRB |= (1 << PB0);     // Setup pin 5 as an OUTPUT
      while (true) {
      // put your code to run repeatedly inside this loop
        PORTB |= (1 << PB0);    // Turn on LED on pin 5
        _delay_ms(500);
        PORTB &= ~(1 << PB0);   // Turn off LED on pin 5
        _delay_ms(500);
      }
    }
    
This `#includes` in this code make use of the [avr-libc](https://www.nongnu.org/avr-libc/) libraries to access additional function such as `_delay_ms()`, which is defined in "`util/delay.h`", and port and pin definitions, such as `DDRB`, `PORTB` and `PB0`, which are defined in "`avr/io.h`".
