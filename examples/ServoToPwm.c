#pragma clock 8000000

#include <avr/io.h>
#include <avr/interrupt.h>
#include <avr/pgmspace.h>
#include <stdbool.h>

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

#define	PWM_OUT		(1 << PB0)
#define	RC_INPUT 	(1 << PB1)

#define MAX_PULSE	((unsigned int) ((8000000 * .0021) / 64))
#define	MIN_PULSE	((unsigned int) ((8000000 * .0009) / 64))

volatile unsigned char	timer, pwmTime;
volatile unsigned int		lTime, cTime, eTime;

// Interrupts at 8,000,000 / 64 / 256 = 488.28125 Hz
ISR (TIM0_OVF_vect) {
	timer++;
}

ISR (TIM0_COMPA_vect) {
	OCR0AL = pwmTime;
}

int main (void) {
	DDRB   = PWM_OUT;
	PORTB  = PWM_OUT;
	// Enable pull ups on unused pins to save power
	PUEB   = (1 << PUEB3) | (1 << PUEB2) | (1 << PUEB1);
	// System clock is 8 mHz (clock prescale 1:1)
	CCP    = 0xD8;
	CLKPSR = (0 << CLKPS3) | (0 << CLKPS2) | (0 << CLKPS1) | (0 << CLKPS0);
	// Setup clock for 8 bit PWM, timer prescaler 1:64
	TCCR0A = 0x81;					// Fast 8-bit PWM, and clear OC0A on match, set OC0A at bottom
	TCCR0B = 0x0B;					// Fast 8-bit PWM, and prescale 1:64
	OCR0AH = 0x00;					// Clear high byte of compare A
	OCR0AL = 0x00;					// Clear low byte of compare A
	// Enable timer overflow and Compare match A interrupts
	TIMSK0 = (1 << TOIE0) | (1 << OCIE0A);
	OCR0AL = 0;							// Set PWM to minimum
	sei();									// Enable Global Interrupts
  while (1) {
		while ((PINB & RC_INPUT) == 0)
			;
		lTime = ((unsigned int) timer << 8) + (unsigned int) TCNT0L;
		while ((PINB & RC_INPUT) != 0)
			;
		cTime = ((unsigned int) timer << 8) + (unsigned int) TCNT0L;
		eTime = cTime > lTime ? cTime - lTime : ~(lTime - cTime) + 1;
		if (eTime > MAX_PULSE) {
			eTime = MAX_PULSE;
		} else if (eTime < MIN_PULSE) {
			eTime = MIN_PULSE;
		}
		eTime = (eTime - MIN_PULSE)  & 0xFF;
		if (eTime > 0) {
			// Enable PWM outout
			OCR0AL = eTime;
			pwmTime = eTime;
			TCNT0L = 0;
			TCCR0A = 0x81;
		} else {
			// Disable PWM output and set PWM_OUT = 0
			TCCR0A = 0x01;
			PORTB &= ~PWM_OUT;
		}
  }
}

