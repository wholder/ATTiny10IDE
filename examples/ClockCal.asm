.device ATtiny10

;             +====+
;  (PWMA) PB0 |*   | PB3 (RESET)
;         GND |    | Vcc
;  (PWMB) PB1 |    | PB2
;             +====+

; available fuses: ckout, wdton, rstdisbl

.fuses ckout

.DSEG
 
.CSEG
.org 0
;Interrupt vector table
	rjmp reset			; All Resets	
	reti				; External Interrupt Request 0
	reti				; Pin Change Interrupt Request 0
	reti				; Timer/Counter0 Input Capture
	reti			 	; Timer/Counter0 Overflow
	reti				; Timer/Counter0 Compare Match A
	reti				; Timer/Counter0 Compare Match B
	reti				; Analog Comparator
	reti				; Watchdog Time-out
	reti				; VCC Voltage Level Monitor
	reti				; ADC Conversion Complete

reset:
; Set Stack Pointer (SP)
	ldi r16, 0x00
	out SPH, r16
	ldi r16, 0x5F
	out SPL, r16
; Set clock to 8MHz
	ldi	r16, 0xD8		; Unprotect CLKPSR reg
	out	CCP, r16
	ldi	r16, 0			; Divide by 1
	out	CLKPSR, r16
; Calibrate Oscillator
	ldi r16, 0x95		; Measured Value
	out OSCCAL, r16
; Setup I/O Port
	ldi r16, (1<<PINB3) | (1<<PINB2) | (0<<PINB1) | (1<<PINB0)
	out DDRB, r16
; Pullup PINB1
	ldi r16, (1<<PINB3) | (1<<PINB2) | (0<<PINB1) | (1<<PINB0)
	out PORTB, r16
; Setup CTC to toggle Pin 1 on count
	ldi r16, 0x40		; Clear Timer on Compare (CTC) Mode
	out TCCR0A, r16
	ldi r16, 0x0C		; CTC and prescale = /256
	out TCCR0B, r16
; Set Output Compare Register to 0x7A12 (8000000 / 256)
	ldi r16, 0x7A
	out	OCR0AH, r16
	ldi r16, 0x12
	out OCR0AL, r16
wait:
	ldi r17, 8			; Bit counter
	ldi r18, 0			; Bit accumulator
bit_wait:
	add r18, r18		; Shift left r18
	rcall pulse_in
	breq wait			; Reset byte loop
	cpi r16, 64
	brlo zero_bit
	inc r18
zero_bit:
	dec r17
	brne bit_wait
; Get her with received byte in r18
	out OSCCAL, r18
	rjmp wait
;
; Wait for pint to go low, then measure length of pulse
; Return r16 as pulse width, else carry set if timeout
pulse_in:
	sbic PINB, PINB1	; skip if PINB1 is LOW
	rjmp pulse_in		; wait for pin to go LOW
	ldi r16, 1
pulse_loop:
	sbic PINB, PINB1	; skip if PINB1 is LOW
	ret
	inc r16
	brne pulse_loop
end_wait:
	sbis PINB, PINB1
	rjmp end_wait
	ret