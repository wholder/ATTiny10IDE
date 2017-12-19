#pragma clock 8000000
#pragma chip attiny10

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

#include <avr/io.h>
#include <util/delay.h>

#define LED_PIN PB2
#define setPin(PIN) (PORTB |= (1 << PIN))
#define clrPin(PIN) (PORTB &= ~(1 << PIN))

int main (void) {
  // Set clock to 8 MHz
  CCP = 0xD8;			    // Unprotect CLKPSR reg
  CLKPSR = 0x00;	     // Set Clock Prescaler to Divide by 1
  // Calibrate Oscillator (use "Action->Calibrate Clock" to get OSCCAL value)
  OSCCAL = 0x58;
  // set PORTB for output
  DDRB = (1 << LED_PIN);
  while (1) {
    // Blink at 1 Hz Rate
	  setPin(LED_PIN);
    _delay_ms(500);
	  clrPin(LED_PIN);
    _delay_ms(500); 
  }
}
