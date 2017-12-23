//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

  // Blink LED using Inline Assembly to directly set and clear I/O register bits
  
#include <avr/io.h>
#include "Arduino.h"

void setup () {
  // Set bit 2 of Data Direction Register (DDRB) to make PB2 an OUTPUT
  asm volatile(
  "sbi 0x01, 2\n"    // Set bit 2 of DDRB
  ::);}

// Blink at 1 Hz Rate
void loop () {
  // Set bit 2 of Data Register (PORTB) to make PB2 output HIGH
  asm volatile(
  "sbi 0x02, 2\n"   // Set bit 2 of PORTB
  ::);
  delay(500);
  // Clear bit 2 of Data Register (PORTB) to make PB2 output LOW
  asm volatile(
  "cbi 0x02, 2\n"   // Clear bit 2 of PORTB
  ::);
  delay(500);
}
