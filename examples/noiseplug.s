#pragma chip attiny10

; This work is licensed under the Creative Commons Attribution-ShareAlike 3.0 Unported License.
; To view a copy of this license, visit http://creativecommons.org/licenses/by-sa/3.0/
; or send a letter to Creative Commons, 444 Castro Street, Suite 900, Mountain View, California, 94041, USA.

.global __vectors

SREG = 0x3F
SPH = 0x3E
SPL = 0x3D
CCP = 0x3C
SMCR = 0x3A
CLKPSR = 0x36
TCCR0A = 0x2E
TCCR0B = 0x2D
TCCR0C = 0x2C
TIMSK0 = 0x2B
TIFR0 = 0x2A
OCR0AL = 0x26
PUEB = 0x03
PORTB = 0x02
DDRB = 0x01
PINB = 0x00

LEADSIZE = 174
LEADINIT1 = 1601
LEADINIT2 = 3571

.section .bss

	.comm int_ctr, 1

	.comm lead1, 4
	.comm lead2, 4
	.comm lead3, 4
	
	.comm bassosc, 2
	.comm bassflange, 2
	.comm arposc, 2
	.comm boost, 1
	; leaves 9 bytes for stack

.section .text

__vectors:
NULL:
	clr r16
	ldi r17, 0xD8
	out PUEB, r16
	rjmp main_cont
	
	.type	__vector_4, @function
__vector_4:
	push r16
	in r16, SREG
	push r16
	lds r16,int_ctr
	subi r16, -1
	andi r16, 3
	sts int_ctr, r16
	pop r16
	out SREG, r16
	pop r16
	reti
	.size	__vector_4, .-__vector_4
	
main_cont:
	clr r31
	ldi r30, 0x40
	
clear_sram:
	st Z+, r16
	sbrs r30, 5
	rjmp clear_sram

	ldi r18, 0x5F
	out SPL, r18
	out SPH, r16

	ldi r18, LEADSIZE
	sts lead1, r18
	sts lead2, r18
	sts lead3, r18
	ldi r18, hi8(LEADINIT1)
	sts lead2 + 2, r18
	ldi r18, lo8(LEADINIT1)
	sts lead2 + 3, r18
	ldi r18, hi8(LEADINIT2)
	sts lead3 + 2, r18
	ldi r18, lo8(LEADINIT2)
	sts lead3 + 3, r18

	out CCP, r17
	out CLKPSR, r16
	ldi r17, 5
	out DDRB, r17
	ldi r17, 0x81
	out TCCR0A, r17
	ldi r17, 0x09
	out TCCR0B, r17
	ldi r17, 1
	out SMCR, r17
	out TIMSK0, r17
	sei
	out TIFR0, r17

	; YH and ZH never change
	ldi r31, hi8(bassline+0x4000)
	clr r29
	
	; init i
	clr r17
	clr r18
	;ldi r17, 3
	;ldi r18, 0xF0
	clr r19

mainloop:
	sleep
	clr r16
	lds r20, int_ctr
	tst r20
	brne mainloop
	
	sbi PORTB, 2
	
	subi r19, 0xff
	sbci r18, 0xff
	sbci r17, 0xff
	
	; if ((i >> 13) == 76) i = 16 << 13;
	mov r20, r18
	rol r20
	mov r20, r17
	rol r20
	subi r20, 0x13
	brne norestart
	
	ldi r17, 2
	clr r18
	
norestart:
	;rjmp noarp
	
; ==== BASS ====
	; bassptr(r20) = (i >> 13) & 0xF
	mov r20, r17
	ror r20
	mov r20, r18
	ror r20
	swap r20
	andi r20, 0xF
	
	; if (i >> 19) & 1: bassptr |= 0x10
	sbrc r17, 3
	ori r20, 0x10
	
	; note = notes[bassline[bassptr]]
	ldi r30, lo8(bassline)
	add r30, r20
	ld r20, Z
	ldi r30, lo8(notes)
	add r30, r20
	ld r21, Z+
	ld r20, Z
	
	; if (bassbeat[(i >> 10) & 7]): note <<= 1
	mov r22, r18
	lsr r22
	lsr r22
	andi r22, 7
	ldi r30, lo8(bassbeat)
	add r30, r22
	ld r22, Z
	tst r22				; assuming this resets C
	breq nobassbeat
	rol r21
	rol r20
	
nobassbeat:
	; bassosc += note, ret = (bassosc >> 8) & 0x7F
	lds r22, bassosc
	lds r23, bassosc + 1
	add r23, r21
	adc r22, r20
	sts bassosc, r22
	sts bassosc + 1, r23
	mov r24, r22
	andi r24, 0x7F
	
	; bassflange += note + 1, ret += (bassflange >> 8) & 0x7F
	lds r22, bassflange
	lds r23, bassflange + 1
	inc r21
	add r23, r21
	adc r22, r20
	sts bassflange, r22
	sts bassflange + 1, r23
	andi r22, 0x7F
	add r24, r22
	
	; if !((i >> 6) & 0xF) == 0xF: sample += (bass >> 2)
	lsr r24
	lsr r24
	mov r20, r18
	andi r20, 3
	subi r20, 3
	brne addbass
	mov r20, r19
	andi r20, 0xC0
	subi r20, 0xC0
	breq noaddbass

addbass:	
	add r16, r24
	
noaddbass:	
	
; ==== ARPEGGIO ====
	; arpptr(r30) = arpseq1[arpseq2[i >> 16]][(i >> 14) & 3]
	mov r30, r17
	subi r30, lo8(NULL-arpseq2)
	ld r30, Z
	lsl r30
	lsl r30
	mov r20, r18
	swap r20
	lsr r20
	lsr r20
	andi r20, 3
	or r30, r20
	subi r30, lo8(NULL-arpseq1)
	ld r30, Z
	
	; if (!(i & (1 << 13))): arpptr >>= 14
	sbrs r18, 5
	swap r30
	
	; arpptr = arpeggio[arpptr & 0xF][(i >> 8) & 1]
	andi r30, 0xF
	lsl r30
	mov r20, r18
	andi r20, 1
	or r30, r20
	subi r30, lo8(NULL-arpeggio)
	ld r30, Z
	
	; if (!(i & 0x80)): arpptr >>= 14
	sbrs r19, 7
	swap r30
	
	; note = arpnotes[arpptr & 0xF]
	andi r30, 0xF
	lsl r30
	subi r30, lo8(NULL-arpnotes)
	ld r21, Z+
	ld r20, Z
	
	; arp_osc += note
	lds r22, arposc
	lds r23, arposc + 1
	add r23, r21
	adc r22, r20	
	sts arposc, r22		; keep r22 for later!
	sts arposc + 1, r23
	
	; if (!(i >> 17)): break arp
	mov r20, r17
	lsr r20
	breq noarp
	
	; r20 = arptiming[(i >> 12) & 3]
	mov r30, r18
	swap r30
	andi r30, 3
	subi r30, lo8(NULL-arptiming)
	ld r20, Z
	
	; if (!((r20 << ((i >> 9) & 7)) & 0x80)): break arp
	mov r21, r18
	lsr r21
	andi r21, 7
	breq arptiming_noshift

arptiming_shift:
	lsl r20
	subi r21, 1
	brne arptiming_shift
	
arptiming_noshift:
	sbrs r20, 7
	rjmp noarp
	
	; if (arp_osc & (1 << 12)): sample += 35;
	sbrc r22, 4
	subi r16, -35
	
noarp:

; ==== LEAD ===
	ldi r28, lead1
	ldi r24, 0
	ldi r25, ~1
	rcall lead_voice
	lsr r23
	add r16, r23
	lsr r23
	add r16, r23
	
	ldi r28, lead2
	ldi r24, 4
	ldi r25, ~2
	rcall lead_voice
	lsr r23
	lsr r23
	add r16, r23
	lsr r23
	add r16, r23
	
	ldi r28, lead3
	ldi r24, 8
	ldi r25, ~4
	rcall lead_voice
	lsr r23
	lsr r23
	add r16, r23

	out OCR0AL, r16
	
	cbi PORTB, 2
	rjmp mainloop

; input:
;   dataset pointer in Y
;   inverted boost mask in r25
;   voice delay (4 * voice_nr) in r24
; returns sample in r23

skiplead_top:
	clr r23
	ret

lead_voice:
	ld r23, Y+					; r23 = leadptr
	cpi r23, LEADSIZE
	brne noleadsetup
	
		cpi r17, 4
		brne skiplead_top
		cp r18, r24					; r24 no longer needed now!
		brne skiplead_top
		tst r19
		brne skiplead_top
	
		dec r28
		ldi r23, -1
		st Y+, r23
		ldi r20, 1
		st Y, r20

noleadsetup:
	lds r26, boost				; r26 = boost
	ld r27, Y					; r27 = leadtimer
	
	; if (0 == (i & 0xFF)): clear boost
	tst r19
	brne checkleadtimer
	
		and r26, r25
		sts boost, r26
	
		; if (0 == i & 0x1FF): leadtimer--
		sbrc r18, 0
		rjmp checkleadtimer
	
			dec r27
			st Y, r27
	
checkleadtimer:
	; if (0 == leadtimer): leadptr++
	tst r27
	brne getleaddata
	
		; leadptr++
		dec r28
		inc r23
		st Y+, r23
	
getleaddata:
	; leadptr(4..7) = leadseq[leadptr >> 4];
	mov r30, r23
	andi r23, 0xF
	swap r30
	andi r30, 0xF
	subi r30, lo8(NULL-leadseq)
	ld r30, Z
	swap r30
	or r30, r23
	
	; data = leaddata[leadptr]
	subi r30, lo8(NULL-leaddata)
	ld r24, Z					; r24 = data!
	
	; if (0 == leadtimer) {
	com r25
	tst r27
	brne noleadupdate
	
		; leadtimer = leadtimes[data >> 5]
		mov r30, r24
		swap r30
		lsr r30
		andi r30, 7
		subi r30, lo8(NULL-leadtimes)
		ld r27, Z
		st Y, r27
	
		; boosts |= boostmask
		or r26, r25
		sts boost, r26
	
noleadupdate:
	; data &= 0x1F
	andi r24, 0x1F
	
	; note = notes[data]
	ldi r30, lo8(notes)
	lsl r24
	add r30, r24
	ld r21, Z+
	ld r20, Z
	
	; leadosc += note
	inc r28
	ld r22, Y+
	ld r23, Y
	add r23, r21
	adc r22, r20
	st Y, r23
	st -Y, r22
	
	; sample = ((lead_osc >> 7) & 0x3F) + ((lead_osc >> 7) & 0x1F)
	rol r23
	rol r22
	andi r22, 0x3F
	mov r23, r22
	andi r22, 0x1F
	add r23, r22					; r23 = final boosted sample!
	
	; if (!(boost & boostmask)): take three quarters
	and r26, r25
	brne noreduce
	
;	lsr r23
;	mov r22, r23
;	lsr r23
;	add r23, r22
	
noreduce:
	; if (data == 0) return 0;
	tst r24
	breq skiplead
	
	ret

skiplead:
	clr r23
	ret
	
	.org 0x300
	
notes:
	.word 	-1, 134, 159, 179, 201
arpnotes:
	.word	213, 239, 268, 301, 319, 358, 401, 425, 451, 477, 536, 601, 637, 715
	
bassline:
	.byte	14, 14, 18, 12, 14, 14, 20, 12, 14, 14, 18, 8, 10, 10, 4, 8
	.byte	10, 10, 12, 12, 14, 14, 6, 6, 10, 10, 12, 12

bassbeat:
	.byte	0, 0, 1, 0, 0, 1, 0, 1

arpseq1:
	.byte	0x00, 0x12, 0x00, 0x62
	.byte	0x00, 0x12, 0x00, 0x17
	.byte	0x00, 0x12, 0x00, 0x12
	.byte	0x33, 0x22, 0x00, 0x45

arpseq2:
	.byte	0, 1, 0, 1, 0, 1, 0, 2, 3, 3

arptiming:
	.byte	0x0C, 0x30, 0xFB, 0x0C

arpeggio:
	.byte	0x24, 0x6A
	.byte	0x46, 0x9C
	.byte	0x13, 0x59
	.byte	0x02, 0x47
	.byte	0x24, 0x59
	.byte	0x24, 0x58
	.byte	0x57, 0xAD
	.byte	0x35, 0x9B
	
leadtimes:
	.byte	1, 2, 3, 4, 5, 6, 28, 14

leaddata:
	.byte	0x67, 0x24, 0x20, 0x27, 0x20, 0x28, 0x89, 0x0, 0x28, 0x20, 0x27, 0x20, 0x28, 0x89, 0x0, 0x28
	.byte	0x20, 0x27, 0x20, 0x28, 0x86, 0x0, 0x44, 0x0, 0x63, 0x24, 0x62, 0xA1, 0xE0, 0xE0, 0xE0, 0xE0
	.byte	0x20, 0x29, 0x20, 0x2A, 0x8B, 0x0, 0x4E, 0x0, 0x6F, 0x30, 0x71, 0xAF, 0xE0, 0xE0, 0xE0, 0xE0
	.byte	0x20, 0x29, 0x20, 0x2A, 0x8B, 0x0, 0x4E, 0x0, 0x6F, 0x30, 0x6F, 0xAC, 0xE0, 0xE0, 0xE0, 0xE0
	.byte	0x65, 0x22, 0x20, 0x65, 0x26, 0x87, 0x0, 0x68, 0x69, 0x2B, 0xAA, 0xC0, 0x67, 0x24, 0x20, 0x67
	.byte	0x28, 0x89, 0x0, 0x68, 0x69, 0x2B, 0xAA, 0xC0, 0x65, 0x22, 0x20, 0x65, 0x26, 0xA7, 0x28, 0x20
	.byte	0x69, 0x2B, 0xAA, 0x29, 0x20, 0x68, 0x29, 0xAA, 0x2B, 0x20, 0x69, 0x28, 0x69, 0x67, 0xE0

leadseq:
	.byte	0, 1, 0, 2, 0, 1, 0, 3, 4, 5, 6
