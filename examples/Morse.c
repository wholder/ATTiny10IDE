#pragma clock 8000000

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+
//

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include "Arduino.h"

#define LED_PIN PB2

unsigned char   state = 0;
unsigned char   idx = 0;
char            let;
unsigned int    pat;
unsigned char   pLen;

const unsigned char letters[] PROGMEM = {
    // Note: 1 is DASH, 0 is DOT
    (2 << 4) | 0b0100,      // A
    (4 << 4) | 0b1000,      // B
    (4 << 4) | 0b1010,      // C    
    (3 << 4) | 0b1000,      // D
    (1 << 4) | 0b0000,      // E
    (4 << 4) | 0b0010,      // F
    (3 << 4) | 0b1100,      // G
    (4 << 4) | 0b0000,      // H
    (2 << 4) | 0b0000,      // I
    (4 << 4) | 0b0111,      // J
    (3 << 4) | 0b1010,      // K
    (4 << 4) | 0b0100,      // L
    (2 << 4) | 0b1100,      // M
    (2 << 4) | 0b1000,      // N
    (3 << 4) | 0b1110,      // O
    (4 << 4) | 0b0110,      // P
    (4 << 4) | 0b1101,      // Q
    (3 << 4) | 0b0100,      // R
    (3 << 4) | 0b0000,      // S
    (1 << 4) | 0b1000,      // T
    (3 << 4) | 0b0010,      // U
    (4 << 4) | 0b0001,      // V
    (3 << 4) | 0b0110,      // W
    (4 << 4) | 0b1001,      // X
    (4 << 4) | 0b1011,      // Y
    (4 << 4) | 0b1100,      // Z
};

const char message[] PROGMEM = "HELLO WORLD ";

void setup (void) {
  // set LED_PIN for output
  pinMode(LED_PIN, OUTPUT);
	// Setup Timer0 overflow interrupt
	TCCR0A = 0x00;		// normal counter operation
	TCCR0B = 0x03;		// timer clock = 8 MHz / 64
	TIMSK0 = 0x01;		// enable overflow interrupt
	sei(); 				    // Enable Global Interrupts
}

void loop () {
}

ISR (TIM0_OVF_vect) {
  switch (state) {
  case 0:             // Fetch next letter of message
    let = pgm_read_byte(&message[idx++]);
      if (let == 0) {
          idx = 0;
    } else if (let == ' ') {
          pat = 0;
          pLen = 2;
    } else {
      unsigned char morse = pgm_read_byte(&letters[let - 'A']);
      unsigned char len = (morse >> 4) & 0x0F;
      morse &= 0x0F;
      pat = 0;
      pLen = 0;
      unsigned char ii;
      // Build flash pattern for letter
      for (ii = 0; ii < len; ii++) {
        if ((morse & 0x08) != 0) {
          // DASH
          pat <<= 3;
          pat |= 0b110;
          pLen += 3;
        } else {
          // DOT
          pat <<= 2;
          pat |= 0b10;
          pLen += 2;
        }
        morse <<= 1;
      }
      // Add gap after letter
      pat <<= 1;
      pLen += 1;
      // Left justify pattern
      pat <<= (16 - pLen);
    }
    state = 1;
    break;
  case 1:
    if (pLen > 0) {
      if ((pat & 0x8000) != 0) {
        digitalWrite(LED_PIN, HIGH);
      } else {
        digitalWrite(LED_PIN, LOW);
      }
      pat <<= 1;
      pLen--;
    } else {
      state = 0;
    }
    break;
  }
}
