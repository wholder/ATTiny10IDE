#include "Arduino.h"

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

/*
 *  Configure clock speed by setting prescaler using CLKSP enum
 */
void clockSpeed (CLKSP rate) {
  CCP = 0xD8;			   // Unprotect CLKPSR reg
  CLKPSR = rate;
}

void dummy (unsigned char pin, unsigned char value) {}

int main (void) {
  // Set clock to 8 MHz
  clockSpeed(CLK_8000000);
  setup();
  while (true) {
    loop();
  }
}
