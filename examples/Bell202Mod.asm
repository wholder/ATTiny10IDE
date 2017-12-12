.device ATtiny10

;                          +====+
;  AFSK Output <- PWMA/PB0 |*   | PB3 (RESET)
;                      GND |    | Vcc
;   Data Input -> PWMB/PB1 |    | PB2 (CLKO)
;                          +====+

; Implements Bell 202 modem modulator
;	Mark:  1200 (binary 1)
;	Space: 2200 (binary 0)
;
; *IMPORTANT* CPU Clock must be calibrated to 8 MHz


.equ 

; available fuses: ckout, wdton, rstdisbl
.fuses ckout

.equ	C1200	= 0x480	; 1152
.equ	C2200	= 0x840	; 2112
.equ	DEC		= 0x271	; 625
.equ	TBLSIZE	= 0x30	; 48

.DSEG
 
accl:	.BYTE	1		; acc low byte
acch:	.BYTE	1		; acc high byte
idx:	.BYTE	1		; sinetable index

.CSEG
.org 0
;Interrupt vector table
	rjmp reset			; All Resets	
	reti				; External Interrupt Request 0
	reti				; Pin Change Interrupt Request 0
	reti				; Timer/Counter0 Input Capture
	rjmp timer_ofl	 	; Timer/Counter0 Overflow
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
	ldi r16, 0x70		; ATTiny9 Value
	out OSCCAL, r16
; Set PB3=IN, PB2=IN, PB1=IN, PB0=OUT
	ldi r16, (0<<DDRB3) | (0<<DDRB2) | (0<<DDRB1) | (1<<DDRB0)
	out	DDRB, r16
; Pullup inputs
	ldi r16, (1<<PUEB3) | (1<<PUEB2) | (1<<PUEB1) | (0<<PUEB0)
	out PUEB, r16
; Setup timer in 8 Bit, Fast PWM mode on PWMA, timer prescaler = 1:1
	ldi r16, (1<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (1<<WGM00)
	out TCCR0A,r16
	ldi r16, (0<<ICNC0) | (0<<ICES0) | (0<<WGM03) | (1<<WGM02) | (0<<CS02) | (0<<CS01) | (1<<CS00)
	out TCCR0B, r16
; Set High Byte of PWMA to 0x00
	ldi r16, 0x00
	out	OCR0AH, r16
; Set Low Byte of PWMA to 0x40
	ldi r16, 0x40
	out	OCR0AL, r16
; init acc to C1200 (Mark value)
	ldi r16,C1200 & 0xFF
	sts accl,r16
	ldi r16,C1200 >> 8
	sts	acch,r16
; init idx to 0
	ldi r16,0x00
	sts idx,r16
; Setup Timer overflow interrupt
	ldi r16, (1<<TOIE0)
	out TIMSK0, r16
	sei					; enable interrupts
wait:
	sleep
	rjmp wait
  
timer_ofl:
; interrupt 8MHz/256 = 31,250 Hz
	push r16
	in	r16, SREG
	push r16
	push r17
	push zl
	push zh
; load pointer to base of sinetable
	ldi zh,((sinetable << 1) + 0x4000) >> 8
	ldi zl,((sinetable << 1) + 0x4000) & 0xFF
; output sample of sinetable selected by idx
	lds r16,idx
	clr r17
	add zl,r16
	adc zh,r17
; copy value selected from sinetable to PWMA
	ld r16,Z
	out	OCR0AL,r16
; load acc value for loop
	lds r16,accl
	lds r17,acch
aloop:
; advance idx (module the size of sinetable)
	lds zl,idx
	inc zl
	cpi zl,TBLSIZE
	brcs no_wrap
	subi zl,TBLSIZE
no_wrap:
	sts idx,zl
; subtract DEC from acc
	subi r16,DEC & 0xFF
	sbci r17,DEC >> 8
	brge aloop
; add back C1200 if Mark, else C2200 for Space
	sbic PINB,PINB1			; skip if PINB1 is zero (Space)
	rjmp space
	ldi zl,C2200 & 0xFF		; Space tone
	ldi zh,c2200 >> 8
	rjmp addtoacc
space:
	ldi zl,C1200 & 0xFF		; Mark tone
	ldi zh,c1200 >> 8
addtoacc:
	add r16,zl
	adc r17,zh
	sts accl,r16
	sts acch,r17
; return from interrupt
	pop zh
	pop zl
	pop r17
	pop	r16
	out	SREG, r16
	pop r16
	reti

sinetable:
	.db 0x80, 0x8F, 0x9F, 0xAD, 0xBC, 0xC9, 0xD4, 0xDF, 0xE7, 0xEE, 0xF3, 0xF6, 0xF8, 0xF6, 0xF3, 0xEE
	.db 0xE7, 0xDF, 0xD4, 0xC9, 0xBC, 0xAD, 0x9F, 0x8F, 0x80, 0x70, 0x60, 0x52, 0x43, 0x36, 0x2B, 0x20
	.db 0x18, 0x11, 0x0C, 0x09, 0x08, 0x09, 0x0C, 0x11, 0x18, 0x20, 0x2B, 0x36, 0x43, 0x52, 0x60, 0x70
