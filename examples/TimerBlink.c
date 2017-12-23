#pragma clock 8000000

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

#include <avr/io.h>
#include <avr/interrupt.h>
#include "Arduino.h"

#define LED_PIN PB2

char led = false;

void setup () {
  // Calibrate Oscillator (use "Action->Calibrate Clock" to get OSCCAL value)
  OSCCAL = 0x69;
  // set PORTB PB2 for output
  pinMode(LED_PIN, OUTPUT);
  // Setup Timer0 overflow interrupt
  TCCR0A = 0x00;		  // Normal counter operation
  TCCR0B = 0x03;		  // Timer clock = 8 MHz / 64 = 125,000 Hz
  TIMSK0 = 0x01;		  // Enable overflow interrupt
  sei(); 				      // Enable Global Interrupts
}

void loop () {
}

// Timer0 Overflow Interrupt handler (125,000 / 65,536 = Approx 1.907348 Hz)
ISR (TIM0_OVF_vect) {
  // Toggle PORTB Pin PB2 (pin 4) On and Off
  if (led ^= 1) {
	  digitalWrite(LED_PIN, HIGH);
  } else {
	  digitalWrite(LED_PIN, LOW);
  }
}
