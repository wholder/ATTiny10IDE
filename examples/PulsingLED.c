#pragma chip attiny10

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

// Uses Watchdog timer-driven PWM to pulse led connected to PB) (pin 1)
// Code adapted from Adafruit's iNecklace (https://github.com/adafruit/iCufflinks/tree/master/Firmware)

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

#define LED PB0     // PB0 or PB1

const unsigned char cycle[] PROGMEM = {
      3,   6,   9,  15,  23,  31,  41,  53,  64,  75,  86,  99, 108, 119, 130, 140,
    149, 157, 165, 171, 178, 185, 191, 196, 201, 206, 211, 215, 219, 223, 227, 231,
    234, 237, 240, 243, 245, 248, 250, 252, 253, 254, 254, 254, 254, 254, 254, 254,
    254, 253, 252, 250, 248, 245, 243, 240, 237, 234, 231, 227, 223, 219, 215, 211,
    206, 201, 196, 191, 185, 178, 171, 165, 157, 149, 138, 130, 119, 108,  99,  86,
     75,  64,  53,  41,  31,  23,  15,   9,   6,   3,   0
};

unsigned char idx = 0;

int main (void) {
  // Set clock to 256 kHz by setting prescaler to device by 32
  CCP = 0xD8;			   // Unprotect CLKPSR reg
  CLKPSR = (1 << CLKPS2) | (1 << CLKPS0);	       
  // Set PORTB PB0 for output
  DDRB = (1 << LED);
  PORTB = 0x00;
  // Setup for Fast PWM
#if LED == PB1
  TCCR0A = 0x31;
  TCCR0B = 0x81;
  OCR0BH = 0x00;
#else
  TCCR0A = 0xC1;
  TCCR0B = 0x81;
  OCR0AH = 0x00;
#endif
  SMCR = (1 << SE);
  // Enable Global Interrupts
  sei();
  while (1) {
#if LED == PB1
    OCR0BL = cycle[0x4000 + idx];
#else
    OCR0AL = cycle[0x4000 + idx];
#endif
    if (++idx >= sizeof(cycle)) {
      idx = 0;
    }
    // Reset Watchdog Timer to sleep for 4000 cycles
    CCP = 0xD8;
    WDTCSR =  (1 << WDIE) | (1 << WDP0);
    asm volatile(
      "wdr\n"
      "sleep\n"
      ::);
  }
}

// Watchdog Timer Interrupt handler
ISR (WDT_vect) {
  // Do nothing...
}
