	cpu	z8601

	ccf
	rcf
	scf

	di
	ei

	halt
	stop

	wdh
	wdt

	iret
	ret

	nop

	and	r7,r13
	and	r5,vari
	and	vari,r4

	and	r3,@r14
	and	r2,@vari
	and	vari,@r6

	and	r3,#5
	and	vari,#77
	and	@r9,#35h
	and	@vari,#10011b

	add	2,5
	adc	r5,#4
	sub	@r0,#20
	sbc	r7,vari
	or	vari,@r5
	tcm	r0,@r8
	tm	@vari,#00001000b
	cp	vari,#20
	xor	r5,#255

	inc	r5
	inc	@r12
	inc	vari

	dec	r6
	dec	vari
	dec	@r5
	dec	@vari
	decw	rr6
	decw	vari
	decw	@r5
	decw	@vari
	decw	@r5

test1:	jr	test1
	jr	f,test1
	jr	uge,test1

	djnz	r5,test1

	call	test1
	call	@vari
	call	@rr10

	jp	test1
	jp	c,test1
	jp	@vari
	jp	@rr6


	ld	r3,r4
	ld	r5,vari
	ld	r6,@r7
	ld	r8,@vari
	ld	r9,vari(r10)
	ld	r11,#45

	ld	vari,r12
	ld	vari,vari
	ld	vari,@r13
	ld	vari,@vari
	ld	vari,#67

	ld	@r14,r15
	ld	@r0,vari
	ld	@r1,#89

	ld	@vari,r2
	ld	@vari,vari
	ld	@vari,#01

	ld	vari(r3),r4


	ldc	r5,@rr6
	ldc	@rr8,r7
	lde	r9,@rr10
	lde	@rr12,r11

	ldci	@r13,@rr14
	ldci	@rr0,@r15
	ldei	@r1,@rr2
	ldei	@rr4,@r3


	srp	#0


	segment	data

	org	0aah
vari:

