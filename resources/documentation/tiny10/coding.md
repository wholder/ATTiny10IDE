## Writing Arduino-style Code for the ATTiny4/5/9/10

ATTiny10IDE allows you to code in assembly (as a .s file), or plain C, or C++ using a `main()` function.  Or, if you include the `Arduino.h` header, ATTiny10IDE will then support coding as an Arduino-like sketch using `setup()` and `loop()` functions.

Here's an example of a basic "Blink" sketch for the ATTiny10 written like an Arduino sketch:

    #pragma chip attiny10
    #pragma efuse 0xFF        // default value
    #pragma hfuse 0xDF        // default value
    #pragma lfuse 0x62        // default value
    
    #include "Arduino.h"
    
    void setup() {
      pinMode(2, OUTPUT);       // Setup pin 4 as an OUTPUT
    }
    
    void loop() {
      digitalWrite(2, HIGH);    // Turn on LED on pin 4
      delay(500);
      digitalWrite(2, LOW);     // Turn off LED on pin 4
      delay(500);
    }

Notice the use of `#pragma` statements to select the chip type (`attiny10`) as well as the values for the fuses. (Again, see tha Atmel datasheets for detail.)

## Writing C-style Code for the ATTiny4/5/9/10

For comparison, here's a C program, with the same functionally as the Arduino-style one shown above, but which is coded in C and uses a `main()` function instead of the `setup()` and `loop()` methods:

    #pragma chip attiny10
    #pragma lfuse 0x62          // default value
    #pragma hfuse 0xDF          // default value
    #pragma efuse 0xFF          // default value
    
    #include <avr/io.h>
    #include <util/delay.h>
    
    int main () {
      // put your setup code here, to run once:
        DDRB |= (1 << PB2);     // Setup pin 4 as an OUTPUT
      
      while (true) {
      // put your code to run repeatedly inside this loop
        PORTB |= (1 << PB2);    // Turn on LED on pin 4
        _delay_ms(500);
        PORTB &= ~(1 << PB2);   // Turn off LED on pin 4
        _delay_ms(500);
      }
    }
    
The `#includes` in this code make use of the [avr-libc](https://www.nongnu.org/avr-libc/) libraries to access additional function such as `_delay_ms()`, which is defined in "`util/delay.h`", and port and pin definitions, such as `DDRB`, `PORTB` and `PB2`, which are defined in "`avr/io.h`".

## Include `Arduino.h` to code "sketches" in ATTiny10IDE
When coding in C or C++, ATTiny10IDE expects you to include a function named `main()` and handle all the details of using the low-level ATTiny10 architecture which involves configuring and writing to hardware registers to perform I/O operations.  However, if your file starts by including the header file `Arduino.h`, like this:

    #include "Ardiuno.h"
    
you'll be able to write code in a way that is familiar to users of the Arduino IDE.  Instead of writing a `main()` function, your one-time initialization code will go in a function named `setup()` and code that executes again and again will go into a function named `loop()`.  Arduino calls a program like this a "sketch."  The example program `Blank.h` is a sample starting sketch you can copy to start coding.  It looks like this:

    #include "Arduino.h"

    void setup() {
      // put your setup code here, to run once:
    }
    
    void loop() {
      // put your main code here, to run repeatedly:
    }
    
#### ATTiny10 Convenience Functions

In addition, `Arduino.h` provides a set of convenience functions you can call to do digital and analog I/O in a fashion similar to how the Arduino IDE handles these operations.  These include `pinMode()`, `digitalWrite()`, `digitalRead()`, `analogWrite()`, `analogRead()`, `delay()` and `delayMicroseconds()`.  The digital I/O functions are implemented as `#define` macros, which the GNU compiler is able to efficiently convert to quite optimal code.  But, be aware that you'll get the most compact and efficient code when the values passed for pin numbers resolve to constants at compile time.  For example, a call to `digitalWrite(2, HIGH)` compiles to the single instruction, `sbi	0x02,2`, in ATTiny10 assembly language.

Calls to `analogRead()` and `analogWrite()` go to actual C functions because some initialization steps are required the first time one of these functions is called.  For this reason, trying to use the same pin for both analog and digital I/O is not recommended, as the initialization code will not run a second time.  If you need to do this kind of function switching with pins, you'll need to develve into how to write code that directly manipulates the low-level I/O registers in the ATTiny10.  Note: check out some of the more advanced example programs, such as `TimerBlink.c` and `PulsingLED.c`, to see how you can use C/C++ code to directly access the ATTiny10's I/O registers.

Calls to `delay()` and `delayMicroseconds()` are also handled by `#define` macros but redirect to the [AVRlib](http://www.nongnu.org/avr-libc/) functions `_delay_ms()` and `_delay_us()`, respectively. The timing of these macros depends on the `#pragma clock` directive (see "Advanced Features") tell the compiler what clock speed the ATTiny10 is using, which the compiler needs to calculate the number of loop iterations needed for a specific delay.  But, by default, this is handled automatically and the clock for code using `Arduino.h` is set to 8 MHz.

If a different clock speed is needed, you can use the `clock` pragma and set it one of the following values `8000000`, `4000000`, `2000000`, `1000000`, `500000`, `250000`, `125000,` `62500`, or `31250`.  Be sure to insert the pragma before the line that includes `Arduino.h` because code in `Arduino.h` depends on the value you select. For example:

    #pragma clock 2000000    // Set clock to 2 MHz
    #include "Arduino.h"

Note: `analogRead()` depends on the setting of the system clock to set the clock prescaler used by the ADC, so `CLK_250000` is the lowest clock speed recommended if you are using `analogRead()`.

Note: You can also set a custom frequency using `#pragma clock` which will set the system clock prescaler to 1:1.  However, this feature is intended for use with other chips in the ATTiny family.
