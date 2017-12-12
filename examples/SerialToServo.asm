.device ATtiny10

;           +====+
;  PWMA/PB0 |*   | PB3 (RESET)
;       GND |    | Vcc
;  PWMB/PB1 |    | PB2 (CLKO)
;           +====+

.fuses rstdisbl

.eq SREG   = 0x3F	; Status Register
.eq SPH    = 0x3E	; Stack Pointer Register (high byte)
.eq SPL    = 0x3D	; Stack Pointer Register (low byte)

.eq PUEB   = 0x03	; Port B Pull-up Enable Control Register
.eq PORTB  = 0x02	; Port B Data Register
.eq DDRB   = 0x01	; Port B Data Direction Register
.eq PINB   = 0x00	; Port B Input Pins

.eq TCCR0A = 0x2E	; Timer/Counter0 Control Register A
.eq TCCR0B = 0x2D	; Timer/Counter0 Control Register B
.eq TIMSK0 = 0x2B	; Timer/Counter Interrupt Mask Register 0

.eq ICR0L  = 0x22	; Input Capture Register Low Byte
.eq ICR0H  = 0x23	; Input Capture Register High Byte

.eq OCR0BL = 0x24	; Compare Register B Low Byte
.eq OCR0BH = 0x25	; Compare Register B High Byte
.eq OCR0AL = 0x26	; Compare Register A Low Byte
.eq OCR0AH = 0x27	; Compare Register A High Byte

.DSEG

AL:  	.BYTE 1
BL:  	.BYTE 1

TAL: 	.BYTE 1
TAH: 	.BYTE 1
TBL: 	.BYTE 1
TBH: 	.BYTE 1

flags:	.BYTE 1

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
; Set clock to 4MHz
	ldi	r16, 0xD8		; Unprotect CLKPSR reg
	out	CCP, r16
	ldi	r16, 1			; Divide by 2
	out	CLKPSR, r16
; Calibrate Oscillator
	ldi r16, 0xA3		; ATTiny10 Value
	out OSCCAL, r16
; Setup I/O Port
	ldi r16, 0x07		; PB0, PB1 and PB2 are outputs
	out DDRB, r16
	ldi r16, 0x08       ; Pullup PB3
	out PUEB, r16
; Setup Timer for 16 bit Fast PWM
	ldi r16,0xA2		; Set A&B on Bottom, Clear on Match
	out TCCR0A, r16
	ldi r16, 0x19		; PWM mode 0xE, timer clock = 4 MHz / 1
	out TCCR0B, r16
	ldi r16, 0xFF		; Set TOP to 0xFFFF
	out ICR0H, r16
	out ICR0L, r16
; Set OCR0A to 6000 Dec
	ldi r16, (6000 >> 8)
	out OCR0AH, r16
	ldi r16, (6000 & 0xFF)
	out OCR0AL, r16
; Set OCR0B to 6000 Dec
	ldi r16, (6000 >> 8)
	out OCR0BH, r16
	ldi r16, (6000 & 0xFF)
	out OCR0BL, r16
; Reset Flags
	sub r16, r16
	sts flags, r16
; Setup Timer overflow interrupt
	ldi r16, 0x01		; enable overflow interrupt
	out TIMSK0, r16
	sei					; enable interrupts
loop:
; process received characters at 9600 baud
	rcall receive
	mov r16, r17
	andi r17, 0x3F
	andi r16, 0xC0
	cpi r16, 0x00		; Set AL
	breq setAL
	cpi r16, 0x40		; Set AH
	breq setAH
	cpi r16, 0x80		; Set BL
	breq setBL
	cpi r16, 0xC0		; Set BH
	breq setBH
	rjmp loop
;
setAL:
	sts AL, r17
	rjmp loop

setBL:
	sts BL, r17
	rjmp loop
;
setAH:
	lds r16, AL
	rcall combine
; save new OCR0A values for interrupt
	cli
	sts TAL, r17
	sts TAH, r18
	lds r16, flags
	ori r16, 0x01		; Set update flag
	sts flags, r16
	sei
	rjmp loop
;
setBH:
	lds r16, BL
	rcall combine
; save new OCR0B values for interrupt
	cli
	sts TBL, r17
	sts TBH, r18
	lds r16, flags
	ori r16, 0x02		; Set update flag
	sts flags, r16
	sei
	rjmp loop
;
combine:
	sub r18, r18		; r18 = 0
; combine 6 bits AH (r17) and AL (r16) values into 12 bit value
	lsl r17				; r17 = -543210-
	lsl r17				; r17 = 543210--
	lsl r17				; r17 = 43210---, c = 5
	rol r18				; r18 = -------5
	lsl r17				; r17 = 3210----, c = 4
	rol r18				; r18 = ------54
	lsl r17				; r17 = 210-----, c = 3
	rol r18				; r18 = -----543
	lsl r17				; r17 = 10------, c = 2
	rol r18				; r18 = ----5432
; r18:r17 is now ----5432:10------
	add r17, r16
; r18:r17 << 1
	lsl r17
	rol r18
; Add 2000 Dec to r18:r17
	ldi r16, (2000 & 0xFF)
	add r17, r16
	ldi r16, (2000 >> 8)
	adc r18, r16
	ret
;
receive:
; wait for start pulse on PB3
	ldi r17, 0x00		; clear serial input register
wait:
	sbic PINB, PINB3
	rjmp wait
	rcall halfdelay
	sbic PINB, PINB3	; verify start bit
	rjmp wait
; sample 8 bits
	rcall bitdelay
	sbic PINB, PINB3	; bit 0
	ori r17, 0x01
	rcall bitdelay
	sbic PINB, PINB3	; bit 1
	ori r17, 0x02
	rcall bitdelay
	sbic PINB, PINB3	; bit 2
	ori r17, 0x04
	rcall bitdelay
	sbic PINB, PINB3	; bit 3
	ori r17, 0x08
	rcall bitdelay
	sbic PINB, PINB3	; bit 4
	ori r17, 0x10
	rcall bitdelay
	sbic PINB, PINB3	; bit 5
	ori r17, 0x20
	rcall bitdelay
	sbic PINB, PINB3	; bit 6
	ori r17, 0x40
	rcall bitdelay
	sbic PINB, PINB3	; bit 7
	ori r17, 0x80
	rcall bitdelay
	sbis PINB, PINB3	; verify stop bit
	rjmp wait
	ret
;
bitdelay:
	ldi r16, 0x86
bitLoop:
	dec r16
	brne bitLoop
	ret
;
halfdelay:
	ldi r16, 0x43
halfLoop:
	dec r16
	brne halfLoop
	ret
;
timer_ofl:
; Overflow (TOP) interrupt handler (~60 Hz)
	push r16
	in	r16, SREG
	push r16
	lds r16, flags
	andi r16, 0x01
	breq skipTA
; Update OCR0AH & OCR0AL registers
	lds r16, TAH
	out OCR0AH, r16
	lds r16, TAL
	out OCR0AL, r16
skipTA:
	lds r16, flags
	andi r16, 0x02
	breq skipTB
; Update OCR0BH & OCR0BL registers
	lds r16, TBH
	out OCR0BH, r16
	lds r16, TBL
	out OCR0BL, r16
skipTB:
	pop	r16
	out	SREG, r16
	pop r16
	reti


