	cpu	sh7205
	page	0

	mov.b	@-r1,r0			; 41CB
	expect	1350
	mov.b	@-r1,r6
	endexpect

	mov.w	@-r2,r0			; 42DB
	expect	1350
	mov.w	@-r2,r6
	endexpect

	mov.l	@-r3,r0			; 43EB
	expect	1350
	mov.l	@-r3,r6
	endexpect

	mov.b	r0,@r4+			; 448B
	expect	1350
	mov.b	r6,@r4+
	endexpect

	mov.w	r0,@r5+			; 459B
	expect	1350
	mov.w	r6,@r5+
	endexpect

	mov.l	r0,@r6+			; 46AB
	expect	1350
	mov.l	r6,@r6+
	endexpect

	mov.b	r6,@(4000,r7)		; 3761 0FA0
	expect	1320
	mov.b	r6,@(4096,r7)
	endexpect

	mov.w	r6,@(8000,r8)		; 3861 1FA0
	expect	1325
	mov.w	r6,@(8001,r8)
	endexpect
	expect	1320
	mov.w	r6,@(8192,r8)
	endexpect

	mov.l	r6,@(16000,r9)		; 3961 2FA0
	expect	1325
	mov.l	r6,@(16002,r9)
	endexpect
	expect	1320
	mov.l	r6,@(16384,r9)
	endexpect

	mov.b	@(4000,r10),r6		; 36A1 4FA0
	expect	1320
	mov.b	@(4096,r10),r6
	endexpect

	mov.w	@(8000,r11),r6		; 36B1 5FA0
	expect	1325
	mov.w	@(8001,r11),r6
	endexpect
	expect	1320
	mov.w	@(8192,r11),r6
	endexpect

	mov.l	@(16000,r12),r6		; 36C1 6FA0
	expect	1325
	mov.l	@(16002,r12),r6
	endexpect
	expect	1320
	mov.l	@(16384,r12),r6
	endexpect

	movi20	#$12345,r13		; 0D10 2345
	movi20	#$7ffff,r13		; 0D70 FFFF
	movi20	#-$12345,r13		; 0DE0 DCBB
	movi20	#-$80000,r13		; 0D80 0000
	expect	1320
	movi20	#$80000,r13
	endexpect
	expect	1315
	movi20	#-$80001,r13
	endexpect

	movi20s	#$12345,r14		; 0E11 2345
	movi20s	#$7ffff,r14		; 0E71 FFFF
	movi20s	#-$12345,r14		; 0EE1 DCBB
	movi20s	#-$80000,r14		; 0E81 0000
	expect	1320
	movi20s	#$80000,r14
	endexpect
	expect	1315
	movi20s	#-$80001,r14
	endexpect

	movml	r1,@-sp			; 41F1
	movml.l	r1,@-r15		;  "

	movml	@sp+,r2			; 42F5
	movml.l	@r15+,r2		;  "

	movmu	r3,@-sp			; 43F0
	movmu.l	r3,@-r15		;  "

	movmu	@sp+,r4			; 44F4
	movmu.l	@r15+,r4		;  "

	movrt	r5			; 0539

	expect	1132
	movu	@($120,r6),r7
	endexpect

	movu.b	@r6,r7			; 3761 8000
	movu.b	@(15,r6),r7		; 3761 800F
	movu.b	@(4095,r6),r7		; 3761 8FFF
	expect	1320
	movu.b	@(4096,r6),r7
	endexpect

	movu.w	@r8,r7			; 3781 9000
	movu.w	@(30,r8),r7		; 3781 900F
	movu.w	@(8190,r8),r7		; 3781 9FFF
	expect	1325
	movu.w	@(8189,r8),r7
	endexpect
	expect	1320
	movu.w	@(8192,r8),r7
	endexpect

	nott				; 00D0

	clips.b	r9			; 4991
	clips.w r10			; 4A95
	expect	1132
	clips	r10
	endexpect
	expect	1130
	clips.l	r10
	endexpect

	clipu.b	r11			; 4B81
	clipu.w r12			; 4C85
	expect	1132
	clipu	r12
	endexpect
	expect	1130
	clipu.l	r12
	endexpect

	divs	r0,r13			; 4D94

	divu.l	r0,r14			; 4E84

	mulr.l	r0,r1			; 4180
	expect	1130
	mulr.w	r0,r1
	endexpect
	expect	1350
	mulr	r4,r1
	endexpect

	shad	r2,r6			; 462C

	shld	r3,r6			; 463D

	jsr/n	@r4			; 444B

	jsr/n	@@(tbr)			; 8300
	expect	1325
	jsr/n	@@(6,tbr)
	endexpect
	jsr/n	@@(1020,tbr)		; 83FF
	expect	1320
	jsr/n	@@(1024,tbr)
	endexpect

	rts/n				; 006B

	rtv/n	r5			; 457B

	ldc	r6,tbr			; 464A

	stc	tbr,r7			; 074A

	ldbank	@r8,r0			; 48E5

	stbank	r0,@r9			; 49E1

	band	#5,@r10			; 3A59 4000
	band	#5,@(1,r10)		; 3A59 4001
	band.b	#5,@(4095,r10)		; 3A59 4FFF
	expect	1320
	band	#5,@(4096,r10)
	endexpect

	bandnot	#6,@r11			; 3B69 C000
	bandnot	#6,@(1,r11)		; 3B69 C001
	bandnot.b #6,@(4095,r11)	; 3B69 CFFF
	expect	1320
	bandnot	#6,@(4096,r11)
	endexpect

	bclr	#7,@r12			; 3C79 0000
	bclr	#7,@(1,r12)		; 3C79 0001
	bclr.b	#7,@(4095,r12)		; 3C79 0FFF
	expect	1320
	bclr	#7,@(4096,r12)
	endexpect
	bclr	#7,r12			; 86C7

	bld	#1,@r13			; 3D19 3000
	bld	#1,@(1,r13)		; 3D19 3001
	bld.b	#1,@(4095,r13)		; 3D19 3FFF
	expect	1320
	bld	#1,@(4096,r13)
	endexpect
	bld	#1,r13			; 87D9

	bldnot	#2,@r14			; 3E29 B000
	bldnot	#2,@(1,r14)		; 3E29 B001
	bldnot.b #2,@(4095,r14)		; 3E29 BFFF
	expect	1320
	bldnot	#2,@(4096,r14)
	endexpect

	bor	#3,@r1			; 3139 5000
	bor	#3,@(1,r1)		; 3139 5001
	bor.b	#3,@(4095,r1)		; 3139 5FFF
	expect	1320
	bor	#3,@(4096,r1)
	endexpect

	bornot	#4,@r2			; 3249 D000
	bornot	#4,@(1,r2)		; 3249 D001
	bornot.b #4,@(4095,r2)		; 3249 DFFF
	expect	1320
	bornot	#4,@(4096,r2)
	endexpect

	bset	#5,@r3			; 3359 1000
	bset	#5,@(1,r3)		; 3359 1001
	bset.b	#5,@(4095,r3)		; 3359 1FFF
	expect	1320
	bset	#5,@(4096,r3)
	endexpect
	bset	#5,r3			; 863D

	bst	#6,@r4			; 3469 2000
	bst	#6,@(1,r4)		; 3469 2001
	bst.b	#6,@(4095,r4)		; 3469 2FFF
	expect	1320
	bst	#6,@(4096,r4)
	endexpect
	bst	#6,r4			; 8746

	bxor	#1,@r5			; 3519 6000
	bxor	#1,@(1,r5)		; 3519 6001
	bxor.b	#1,@(4095,r5)		; 3519 6FFF
	expect	1320
	bxor	#1,@(4096,r5)
	endexpect

	fpu	on

	; NOTE: need explicit operand size attribute so
	; the correct displacement range is known when
	; the first argument is parsed:
	fmov.s	@(16380,r10),fr3	; 33A1 7FFF
	expect	1320
	fmov.s	@(16384,r10),fr3
	endexpect
	expect	1325
	fmov.s	@(2,r10),fr3
	endexpect
	fmov.d	@(32760,r11),dr4	; 34B1 7FFF
	expect	1320
	fmov.d	@(32768,r11),dr4
	endexpect
	expect	1325
	fmov.d	@(4,r11),dr4
	endexpect

	fmov	fr3,@(16380,r12)	; 3C31 3FFF
	expect	1320
	fmov	fr3,@(16384,r12)
	endexpect
	expect	1325
	fmov	fr3,@(2,r12)
	endexpect
	fmov	dr4,@(32760,r13)	; 3D41 3FFF
	expect	1320
	fmov	dr4,@(32768,r13)
	endexpect
	expect	1325
	fmov	dr4,@(4,r13)
	endexpect
