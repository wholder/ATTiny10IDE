//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

  // Blink LED using Inline Assembly to directly set and clear I/O register bits
  // Note: for details on writing the rather "odd" syntax needed to code in-line assembly
  // using the Gnu compiler see: http://www.nongnu.org/avr-libc/user-manual/inline_asm.html
  
#include <avr/io.h>
#include "Arduino.h"

void setup () {
  // Set bit 2 of Data Direction Register (DDRB) to make PB2 an OUTPUT
  asm volatile("sbi %0, 2" : : "I" (_SFR_IO_ADDR(DDRB)) );
}

// Blink at 1 Hz Rate
void loop () {
  // Set bit 2 of Data Register (PORTB) to make PB2 output HIGH
  asm volatile("sbi %0, 2" : : "I" (_SFR_IO_ADDR(PORTB)) );
  delay(500);
  // Clear bit 2 of Data Register (PORTB) to make PB2 output LOW
  asm volatile("cbi %0, 2" : : "I" (_SFR_IO_ADDR(PORTB)) );
  delay(500);

}
