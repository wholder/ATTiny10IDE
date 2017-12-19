#pragma clock 1000000
#pragma chip attiny10

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+
//
// Vectors:
//		0:						      All Resets
//		1:	INT0_vect			  External Interrupt Request 0
//		2:	PCINT0_vect			Pin Change Interrupt Request 0
//		3:	TIM0_CAPT_vect	Timer/Counter0 Input Capture
//		4:	TIM0_OVF_vect		Timer/Counter0 Overflow
//		5:	TIM0_COMPA_vect	Timer/Counter0 Compare Match A
//		6:	TIM0_COMPB_vect	Timer/Counter0 Compare Match B
//		7:	ANA_COMP_vect		Analog Comparator
//		8:	WDT_vect			  Watchdog Time-out
//		9:	VLM_vect			  VCC Voltage Level Monitor
//	  10:	ADC_vect			  ADC Conversion Complete

const unsigned char sine[] PROGMEM = {
		3,   6,   9,  15,  23,  31,  41,  53,  64,  75,  86,  99, 108, 119, 130, 140,
	149, 157, 165, 171, 178, 185, 191, 196, 201, 206, 211, 215, 219, 223, 227, 231,
	234, 237, 240, 243, 245, 248, 250, 252, 253, 254, 254, 254, 254, 254, 254, 254,
	254, 253, 252, 250, 248, 245, 243, 240, 237, 234, 231, 227, 223, 219, 215, 211,
	206, 201, 196, 191, 185, 178, 171, 165, 157, 149, 138, 130, 119, 108,  99,  86,
	 75,  64,  53,  41,  31,  23,  15,   9,   6,   3};

int main (void) {
	DDRB   = (1 << PB0);
	PORTB  = (1 << PB0);
	PUEB   = (1 << PUEB3) | (1 << PUEB2) | (1 << PUEB1);
	CCP    = 0xD8;
	CLKPSR = (0 << CLKPS3) | (1 << CLKPS2) | (0 << CLKPS1) | (1 << CLKPS0);
	TCCR0A = 0xC1;
	TCCR0B = 0x81;
	OCR0AH = 0x00;
	SMCR   = (1 << SE);
	asm volatile("sei\n\t" ::);						// sei
  while (1) {
		int idx;
		for (idx = 0; idx < sizeof(sine); idx++) {
			OCR0AL = sine[0x4000 + idx];			// Kludge to load from PGM space
			CCP = 0xD8;
			WDTCSR = (0 << WDE) | (1 << WDIE);
			asm volatile("wdr\n\t" ::);				// wdr
			asm volatile("sleep\n\t" ::);			// sleepr
			CCP = 0xD8;
			WDTCSR = (0 << WDE) | (1 << WDIE) | (1 << WDP0);
			asm volatile("wdr\n\t" ::);				// wdr
			asm volatile("sleep\n\t" ::);			// sleepr
		}
  }
}

ISR (WDT_vect) {
	// Just returns to clear sleep status
}
