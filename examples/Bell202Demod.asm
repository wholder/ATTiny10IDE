.device attiny10

;           +====+
;  PWMA/PB0 |*   | PB3 (RESET)
;       GND |    | Vcc
;  PWMB/PB1 |    | PB2 (CLKO)
;           +====+

; Implements Bell 202 modem demodulator
;	Mark:  1200 (binary 1)
;	Space: 2200 (binary 0)

.fuses rstdisbl

.equ COUNT = 832		; 8 MHz / 832 = 9600 samples/sec

.equ srcl  = R16
.equ srch  = R17
.equ cof1l = R18
.equ cof1h = R19
.equ cof2l = R20
.equ cof2h = R21
.equ cof3l = R22
.equ cof3h = R23
.equ cof4l = R24
.equ cof4h = R25
.equ mask  = R26		; xl

.DSEG

.org 0x40
buf: 	.BYTE 8			; 8 byte data buffer
idx: 	.BYTE 1			; data buffer index
ofilt: 	.BYTE 1			; Output filter


.CSEG
.org 0
;Interrupt vector table
	rjmp reset			; All Resets	
	reti				; External Interrupt Request 0
	reti				; Pin Change Interrupt Request 0
	reti				; Timer/Counter0 Input Capture
	reti				; Timer/Counter0 Overflow
	rjmp tint			; Timer/Counter0 Compare Match A
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
	ldi r16, 0x84		; ATTiny10 Value
	out OSCCAL, r16
; Set PB3=IN, PB2=OUT, PB1=OUT, PB0=OUT
	ldi r16, (0<<DDRB3) | (1<<DDRB2) | (1<<DDRB1) | (1<<DDRB0)
	out	DDRB, r16
; Disable all pull ups
	ldi r16, (0<<PUEB3) | (0<<PUEB2) | (0<<PUEB1) | (0<<PUEB0)
	out PUEB, r16
; Enable PB3 (pin 6) as analog input
	ldi r16, (1<<ADC3D) | (0<<ADC2D) | (0<<ADC1D) | (0<<ADC0D)
	out DIDR0, r16

; Setup timer in CTC mode using OCR0A, timer prescaler = 1:1, toggle OC0A (pin 1)
	ldi r16, (0<<COM0A1) | (0<<COM0A0) | (0<<COM0B1) | (0<<COM0B0) | (0<<WGM01) | (0<<WGM00)
	out TCCR0A,r16
	ldi r16, (0<<ICNC0) | (0<<ICES0) | (0<<WGM03) | (1<<WGM02) | (0<<CS02) | (0<<CS01) | (1<<CS00)
	out TCCR0B, r16
; Enable CTC interrupt on 
	ldi r16, (0<<ICIE0) | (0<<OCIE0B) | (1<<OCIE0A) | (0<<TOIE0)
	out TIMSK0, r16
; Set OCR0A to 9600 resets/sec (must set high byte first)
	ldi r16, (COUNT >> 8)
	out OCR0AH, r16
	ldi r16, (COUNT & 0xFF)
	out OCR0AL, r16

; Select PB3 (pin 6) as analog input
	ldi r16, (1<<MUX1) | (1<<MUX0)
	out ADMUX, r16
; Enable ADC, int amd set ADC clock prescaler to /32 for an ADC clk freq of 250KHz
	ldi r16, (1<<ADEN) | (0<<ADSC) | (0<<ADATE) | (0<<ADIE) | (1<<ADPS2 | (0<<ADPS1) | (0<<ADPS0)
	out ADCSRA, r16
' Set ADC start conversion trigger to CTC match A
	ldi r16, (0<<ADTS2) | (0<<ADTS1) | (0<<ADTS0)
	out ADCSRB, r16
; setup idx value (points to buf)
	ldi r16, buf
	sts idx, r16	
 Enable interrupts
	sei
; Wait for interrupt
wait:
	rjmp wait

tint:
; reset watchdog timer
	WDR
; CTC interrupt handler
	sbi PORTB, PORTB1	; set PB1 (timing flag)
	sbi ADCSRA, ADSC	; start conversion
cwait:
	in r16, ADCSRA
	andi r16, (1<<ADIF)	; End of Conversion?
	breq cwait
	ldi mask, buf + 7	; 0x47
	in r16, ADCL		; read ADC (unsigned 8 bit)
	clr zh
	lds zl, idx			; Load idx
	st  Z+, r16			; Store value as unsigned 8 bit in buf
	and zl, mask		; Set idx modulo 8
	sts idx, zl			; Save idx
; Bell 202 Demodulation code
    ld  srcl,Z+         ; src = data[idx++];
    and zl, mask
    clr srch
    subi srcl,128
    sbci srch,0
    lsl srcl            ; src <<= 1;
    rol srch
    lsl srcl            ; src <<= 1;
    rol srch
    lsl srcl            ; src <<= 1;
    rol srch
    lsl srcl            ; src <<= 1;
    rol srch
    lsl srcl            ; src <<= 1;
    rol srch
    lsl srcl            ; src <<= 1;
    rol srch
    mov cof1l,srcl      ; cof1 = src;
    mov cof1h,srch
    clr cof2l           ; cof2 = 0;
    clr cof2h
    mov cof3l,srcl      ; cof3 = src;
    mov cof3h,srch
    clr cof4l           ; cof4 = 0;
    clr cof4h
    ld  srcl,Z+         ; src = data[idx++];
    and zl, mask
    clr srch
    subi srcl,128
    sbci srch,0
    add cof1l,srcl      ; cof1 += src;
    adc cof1h,srch
    add cof2l,srcl      ; cof2 += src;
    adc cof2h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof1l,srcl      ; cof1 += src;
    adc cof1h,srch
    add cof2l,srcl      ; cof2 += src;
    adc cof2h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof1l,srcl      ; cof1 += src;
    adc cof1h,srch
    add cof2l,srcl      ; cof2 += src;
    adc cof2h,srch
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof1l,srcl      ; cof1 += src;
    adc cof1h,srch
    add cof2l,srcl      ; cof2 += src;
    adc cof2h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    ld  srcl,Z+         ; src = data[idx++];
    and zl, mask
    clr srch
    subi srcl,128
    sbci srch,0
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof2l,srcl      ; cof2 += src;
    adc cof2h,srch
    ld  srcl,Z+         ; src = data[idx++];
    and zl, mask
    clr srch
    subi srcl,128
    sbci srch,0
    sub cof1l,srcl      ; cof1 -= src;
    sbc cof1h,srch
    add cof2l,srcl      ; cof2 += src;
    adc cof2h,srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof1l,srcl      ; cof1 -= src;
    sbc cof1h,srch
    add cof2l,srcl      ; cof2 += src;
    adc cof2h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof1l,srcl      ; cof1 -= src;
    sbc cof1h,srch
    add cof2l,srcl      ; cof2 += src;
    adc cof2h,srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof1l,srcl      ; cof1 -= src;
    sbc cof1h,srch
    add cof2l,srcl      ; cof2 += src;
    adc cof2h,srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
    ld  srcl,Z+         ; src = data[idx++];
    and zl, mask
    clr srch
    subi srcl,128
    sbci srch,0
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof1l,srcl      ; cof1 -= src;
    sbc cof1h,srch
    ld  srcl,Z+         ; src = data[idx++];
    and zl, mask
    clr srch
    subi srcl,128
    sbci srch,0
    sub cof1l,srcl      ; cof1 -= src;
    sbc cof1h,srch
    sub cof2l,srcl      ; cof2 -= src;
    sbc cof2h,srch
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof1l,srcl      ; cof1 -= src;
    sbc cof1h,srch
    sub cof2l,srcl      ; cof2 -= src;
    sbc cof2h,srch
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof1l,srcl      ; cof1 -= src;
    sbc cof1h,srch
    sub cof2l,srcl      ; cof2 -= src;
    sbc cof2h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof1l,srcl      ; cof1 -= src;
    sbc cof1h,srch
    sub cof2l,srcl      ; cof2 -= src;
    sbc cof2h,srch
    add cof3l,srcl      ; cof3 += src;
    adc cof3h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    ld  srcl,Z+         ; src = data[idx++];
    and zl, mask
    clr srch
    subi srcl,128
    sbci srch,0
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    add cof4l,srcl      ; cof4 += src;
    adc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof2l,srcl      ; cof2 -= src;
    sbc cof2h,srch
    ld  srcl,Z+         ; src = data[idx++];
    and zl, mask
    clr srch
    subi srcl,128
    sbci srch,0
    add cof1l,srcl      ; cof1 += src;
    adc cof1h,srch
    sub cof2l,srcl      ; cof2 -= src;
    sbc cof2h,srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof1l,srcl      ; cof1 += src;
    adc cof1h,srch
    sub cof2l,srcl      ; cof2 -= src;
    sbc cof2h,srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof1l,srcl      ; cof1 += src;
    adc cof1h,srch
    sub cof2l,srcl      ; cof2 -= src;
    sbc cof2h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    lsl srcl            ; src <<= 1;
    rol srch
    add cof1l,srcl      ; cof1 += src;
    adc cof1h,srch
    sub cof2l,srcl      ; cof2 -= src;
    sbc cof2h,srch
    sub cof3l,srcl      ; cof3 -= src;
    sbc cof3h,srch
    sub cof4l,srcl      ; cof4 -= src;
    sbc cof4h,srch
; desc = abs(cof3 >> 8) + abs(cof4 >> 8) - abs(cof1 >> 8) - abs(cof2 >> 8)
	tst	cof1h
	brpl p1
	neg cof1h
p1:
	tst	cof2h
	brpl p2
	neg cof2h
p2:
	tst	cof3h
	brpl p3
	neg cof3h
p3:
	tst	cof4h
	brpl p4
	neg cof4h
p4:
	add cof3h, cof4h
	sub cof3h, cof1h
	sub cof3h, cof2h
; Low Pass Filter out[n] = out[n-1] + ((in[n-1] - out[n-1]) >> 2)
	lds r16,ofilt		; r16 = out[n-1]
	mov r17,cof3h		; r17 = in[n-1] (cof3h)
	sub r17,r16			; r17 = in[n-1] - out[n-1]
	asr r17				; r17 >>= 1
	asr r17				; r17 >>= 1
	add r16,r17			; r16 = out[n-1] + ((in[n-1] - out[n-1]) >> 2)
	sts ofilt,r16		; out[n-1] = r16
; compute mark/space value
	brpl space
	sbi PORTB, PORTB2	; Mark (1)
	cbi PORTB, PORTB1	; clear timing flag
	reti
space:
	cbi PORTB, PORTB2	; Space (0)
	cbi PORTB, PORTB1	; clear timing flag
	reti

