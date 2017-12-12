#pragma clock 8000000
#pragma chip attiny10
#pragma fuses rstdisbl

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+


#include <avr/io.h>
#include <util/delay.h>
#include <avr/interrupt.h>
#include <stdbool.h>

#define	COUNT (8000000 / 4800)

#define	OUTPIN (1 << PB2)		// Pin 4

#define INPIN1 (1 << PB0)  	// Pin 1
#define INPIN2 (1 << PB1)  	// Pin 3
#define INPIN3 (1 << PB3)  	// Pin 6 (RESET)

#define	KEY1	('1')
#define	KEY2	('2')
#define	KEY3	('3')
#define	KEY4	('4')
#define	KEY5	('5')
#define	KEY6	('6')

volatile bool 				 tick;
volatile char 				 bits;
volatile unsigned int  outReg;
volatile unsigned char serialBuffer[4];
volatile unsigned char serialIn = 0, serialOut = 0, bufCnt = 0;

ISR (TIM0_COMPA_vect) {
	tick = true;
	if (bits > 0) {
		bits--;
		if ((outReg & 1) != 0) {
			PORTB |= OUTPIN;
		} else {
			PORTB &= ~OUTPIN;
		}
		outReg >>= 1;
	}
	if (bits == 0 && bufCnt > 0) {
		// Load next byte to transmit
		outReg = ((unsigned int) serialBuffer[serialOut] << 1) | 0x200;
		serialOut = (serialOut + 1) % sizeof(serialBuffer);
		bufCnt--;
		bits = 10;
	}
}

void send (unsigned char out) {
	if (bufCnt < sizeof(serialBuffer)) {
		serialBuffer[serialIn] = out;
		serialIn = (serialIn + 1) % sizeof(serialBuffer);
		bufCnt++;
	}
}

void tickWait () {
	while (!tick)
		;
	tick = false;
}

void pinInput (char pinBit) {
	DDRB &= ~pinBit;				// Set pin as input
	PORTB |= pinBit;				// Engage pull up
}

void pinLow (char pinBit) {
	PORTB &= ~pinBit;				// Set pin low
	DDRB |= pinBit;					// Set pin as output
}

unsigned char getKey () {
	unsigned char pins = PINB;
  if ((pins & INPIN1) == 0) {
    return KEY3;
  } else if ((pins & INPIN2) == 0) {
    return KEY2;
  } else if ((pins & INPIN3) == 0) {
    return KEY1;
  }
	pinLow(INPIN1);
	tickWait();
	pins = PINB;
	pinInput(INPIN1);
  if ((pins & INPIN2) == 0) {
    return KEY5;
  } else if ((pins & INPIN3) == 0) {
    return KEY6;
  }
	pinLow(INPIN2);
  tickWait();
	pins = PINB;
	pinInput(INPIN2);
  if ((pins & INPIN3) == 0) {
    return KEY4;
  }
  return 0;
}

int main (void) {
	// Set clock to 8 MHz
	CCP = 0xD8;			// Unprotect CLKPSR reg
	CLKPSR = 0x00;	// Divide by 1
	// Calibrate Oscillator
	OSCCAL = 0x9A;
  // set PB2 as output
  DDRB = OUTPIN;
	// Engage pull up resistor for all input pins
	PUEB = INPIN1 | INPIN2 | INPIN3;
	PORTB = OUTPIN;
	// Setup timer to interrupt at 4800 bps rate
	TCCR0A = 0;
	TCCR0B = 0x09;	// CTC (OCR0A) and clock is 1:1
	TCCR0C = 0;
	OCR0AH = COUNT >> 8;
	OCR0AL = COUNT & 0xFF;
	TIMSK0 = (1 << OCIE0A);
	// Enable interrupts
	sei();
	unsigned char timer = 0, lastKey = 0;
  while (true) {
		if (tick) {
			tick = false;
			if (timer++ == 0) {
  			unsigned char key = getKey();
  			if (key != lastKey) {
    			if (key > 0) {
						send(key);
						send(0x0D);
						send(0x0A);
    			}
    			lastKey = key;
  			}
			}
		}
  }
}
