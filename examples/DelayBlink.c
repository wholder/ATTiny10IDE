#pragma clock 8000000
#pragma chip attiny10

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+


#include <avr/io.h>
#include <util/delay.h>

int main (void) {
  // Set clock to 8 MHz
  CCP = 0xD8;			// Unprotect CLKPSR reg
  CLKPSR = 0x00;	// Divide by 1
  // Calibrate Oscillator
  OSCCAL = 0x58;
  // set PORTD for output
  DDRB = 0x0F;
  while (1) {
    // Blink at 1 Hz Rate
	  PORTB = 0x0F;
    _delay_ms(500);
    PORTB = 0x00;
    _delay_ms(500); 
  }
}
