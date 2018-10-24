#include "Arduino.h"

#if defined(__AVR_ATtiny10__) || defined(__AVR_ATtiny9__) || defined(__AVR_ATtiny4__) || defined(__AVR_ATtiny5__)

unsigned char initFlag = 0;

/*
 * Configure Pin 1 (PB0) for 8 bit, no prescaling PWM output and write value
 */
void pwm0 (unsigned char value) {
  if ((initFlag & 0x01) == 0) {
    TCCR0A = (TCCR0A & 0xC1) | 0x81;
    TCCR0B = (TCCR0B & 0x07) | 0x01;
    OCR0AH = 0x00;
    DDRB = (1 << A0);
    initFlag |= 0x01;
  }
  OCR0AL = value;
}

/*
 * Configure Pin 3 (PB1) for 8 bit, no prescaling PWM output and write value
 */
void pwm1 (unsigned char value) {
  if ((initFlag & 0x02) == 0) {
    TCCR0A = (TCCR0A & 0x31) | 0x21;
    TCCR0B = (TCCR0B & 0x07) | 0x01;
    OCR0BH = 0x00;
    DDRB = (1 << A1);
    initFlag |= 0x02;
  }
  OCR0BL = value;
}

#if defined(__AVR_ATtiny10__) || defined(__AVR_ATtiny5__)

unsigned char analogRead (unsigned char pin) {
  ADMUX = pin;
  DIDR0 |= (1 << pin);
  ADCSRB = 0;
  // Adjust ADC prescaler to system clock to maintain 125,000 Hz ADC clock
  ADCSRA = (1 << ADEN) + (1 << ADSC) + (6 - CLKPSR);
  while ((ADCSRA & (1 << ADSC)) != 0)
    ;
  return ADCL;
}

#endif

void dummy (unsigned char pin, unsigned char value) {}  // used by analogWrite() macro

int main (void) {
  // Set clock prescaler based on F_CPU setting (assume CKDIV8 fuse not set)
  CCP = 0xD8;			        // Unprotect CLKPSR reg
#if F_CPU == 8000000
  CLKPSR = 0;             // Set clock to 8 MHz
#elif F_CPU == 4000000
  CLKPSR = 1;             // Set clock to 4 MHz
#elif F_CPU == 2000000
  CLKPSR = 2;             // Set clock to 2 MHz
#elif F_CPU == 1000000
  CLKPSR = 3;             // Set clock to 1 MHz
#elif F_CPU == 500000
  CLKPSR = 4;             // Set clock to 500 kHz
#elif F_CPU == 250000
  CLKPSR = 5;             // Set clock to 250 kHz
#elif F_CPU == 125000
  CLKPSR = 6;             // Set clock to 125 kHz
#elif F_CPU == 62500
  CLKPSR = 7;             // Set clock to 62.5 kHz
#elif F_CPU == 31250
  CLKPSR = 8;             // Set clock to 31.25 kHz
#else
  CLKPSR = 0;             // Custom frequency, set prescaler to 1:1
#endif
  setup();
  while (true) {
    loop();
  }
}

#else
#error undefined chip
#endif
