#pragma clock 8000000
#pragma chip attiny10

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

#include <avr/io.h>
#include <avr/interrupt.h>

unsigned char leds = 0x00;

int main (void) {
  // Set clock to 8 MHz
  CCP = 0xD8;			// Unprotect CLKPSR reg
  CLKPSR = 0x00;	    // Divide by 1
  // Calibrate Oscillator
  OSCCAL = 0x58;
  // set PB0 for output
  DDRB = (1 << PB0);
  // Setup Timer0 overflow interrupt
  TCCR0A = 0x00;		// normal counter operation
  TCCR0B = 0x03;		// timer clock = 8 MHz / 64
  TIMSK0 = 0x01;		// enable overflow interrupt
  sei(); 				// Enable Global Interrupts
  while (1) {
    // Wait for interrupt
  }
}

ISR (TIM0_OVF_vect) {
  // Blink PORTD.PB2 (pin 4)
  leds ^= (1 << PB0);
  PORTB = leds;
}
