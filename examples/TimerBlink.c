#pragma clock 8000000
#pragma chip attiny10

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

#include <avr/io.h>
#include <avr/interrupt.h>

#define LED_PIN PB2
#define setPin(PIN) (PORTB |= (1 << PIN))
#define clrPin(PIN) (PORTB &= ~(1 << PIN))

char led = false;

int main (void) {
  // Set clock to 8 MHz
  CCP = 0xD8;			     // Unprotect CLKPSR reg
  CLKPSR = 0x00;	     // Set Clock Prescaler to Divide by 1
  // Calibrate Oscillator (use "Action->Calibrate Clock" to get OSCCAL value)
  OSCCAL = 0x69;
  // set PORTB PB2 for output
  DDRB = (1 << LED_PIN);
  // Setup Timer0 overflow interrupt
  TCCR0A = 0x00;		  // Normal counter operation
  TCCR0B = 0x03;		  // Timer clock = 8 MHz / 64 = 125,000 Hz
  TIMSK0 = 0x01;		  // Enable overflow interrupt
  sei(); 				      // Enable Global Interrupts
  while (1) {
    // Wait for interrupt
  }
}

// Timer0 Overflow Interrupt handler (125,000 / 65,536 = Approx 1.907348 Hz)
ISR (TIM0_OVF_vect) {
  // Toggle PORTB Pin PB2 (pin 4) On and Off
  if (led ^= 1) {
    setPin(LED_PIN);
  } else {
    clrPin(LED_PIN);
  }
}
