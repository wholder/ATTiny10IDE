#pragma clock 8000000
#pragma chip attiny10

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+
//

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

unsigned char   state = 0;
unsigned char   idx = 0;
char            let;
unsigned int    pat;
unsigned char   pLen;

#define SOS     0
#define PRE32   1

#if PRE32
unsigned char   preDiv;
#endif

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

#if SOS
const char message[] PROGMEM = "SOS ";
#else
const char message[] PROGMEM = "FLASHING LIGHT PRIZE ";
#endif

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
#if PRE32
	TCCR0B = 0x02;		// timer clock = 8 MHz / 8
#else
	TCCR0B = 0x03;		// timer clock = 8 MHz / 64
#endif
	TIMSK0 = 0x01;		// enable overflow interrupt
	sei(); 				// Enable Global Interrupts
    while (1) {
		// Wait for interrupt
    }
}

ISR (TIM0_OVF_vect) {
#if PRE32
    if ((preDiv++ & 0x03) == 0) {
#endif
    switch (state) {
    case 0:             // Fetch next letter of message
        let = message[0x4000 + idx++];
        if (let == 0) {
            idx = 0;
        } else if (let == ' ') {
            pat = 0;
            pLen = 2;
        } else {
            // adding 0x4000 is kludge needed to load from PGM space
            unsigned char morse = letters[0x4000 + (let - 'A')];
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
                PORTB |= (1 << PB0);
            } else {
                PORTB &= ~(1 << PB0);
            }
            pat <<= 1;
            pLen--;
        } else {
            state = 0;
        }
        break;
    }
#if PRE32
    }
#endif
}
