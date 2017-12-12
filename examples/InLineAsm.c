#pragma clock 8000000
#pragma chip attiny10

//           +====+
//  PWMA/PB0 |*   | PB3 (RESET)
//       GND |    | Vcc
//  PWMB/PB1 |    | PB2 (CLKO)
//           +====+

// https://web.stanford.edu/class/ee281/projects/aut2002/yingzong-mouse/media/GCCAVRInlAsmCB.pdf

#include <avr/io.h>

int main (void) {
    // Set clock to 8 MHz
	CCP = 0xD8;			// Unprotect CLKPSR reg
	CLKPSR = 0x00;	    // Divide by 1
    while (1) {
		// Wait for interrupt
      asm volatile(
      "nop\n"
      "nop\n"
      ::);
    }
}
