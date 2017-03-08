	cpu	msm5054
	pagesize	0	; PAGE is a machine insn on OLMS50

	org	0
	assume 	p:1
	add	acc,03h
	add	#6,13h
	sub	acc,14h
	sub	#7,04h
	cmp	acc,05h
	cmp	#8,15h
	xor	acc,16h
	xor	#9,06h
	bit	acc,07h
	bit	#10,17h
	bis	acc,18h
	bis	#11,08h
	bic	acc,09h
	bic	#12,19h

	adjust	7,1fh

	inc	15h
	dec	16h
	asr	17h
	asl	18h
	switch	19h
	kswitch	1ah
	intmode	1bh
	rate	1ch

	clz
	clc
	cla
	sez
	sec
	sea
	bso
	halt
	rstrate
	nop

	mov	acc,02h		; may encode as AX or AP (results in same machine instr)
	mov	acc,12h
	mov	acc,22h
	mov	#-3,03h
	mov	#-3,13h
	mov	04h,acc		; ditto
	mov	14h,acc
	mov	24h,acc

	jmp	123h
	jmp	@0ah
	jmp	@1ah
	jmpio	@0bh
	jmpio	@1bh

	beq	$+2
	;bze	$-2
	bne	$+3
	;bnz	$-3
	bcs	$+4
	;bcc	$-4

	matrix	0bh
	matrix	1bh
	matrix	#6

	format	0ch
	format	1ch
	format	#7

	page	0dh
	page	#8

	dsp	7,0eh
	dsp	8,1eh
	dspf	9,0fh
	dspf	10,1fh

	intenab
	intdsab

	lamp	off
	lamp	on
	backup	off
	backup	on

	buzzer	3,1


	cpu	msm5055

	xtcp	on
	xtcp	off
	freq	7
	buzzer	1
	dsph	7,0eh
	dsph	8,1eh
	dspfh	9,0fh
	dspfh	10,1fh
	adrs	#11
	adrs	01h
	adrs	11h

	cpu	msm5056

	adc	05h
	adc	15h
	sbc	06h
	sbc	16h
	chg	07h
	chg	17h
	chg	47h
	bcs	$+5		; different opcode on 5056++
	blt	$+6
	bcc	$+7		; ditto
	bge	$+8
	bgt	$+9
	ble	$+10
	inp	6,09h
	inp	6,19h
	out	0ah,7
	out	1ah,7
	out	#5,7

	cpu	msm6051

	adc0	03h
	adc2	13h
	sbc4	04h
	sbc14	14h
	inc	05h		; INC/DEC knows AX + AP on MSM60xx
	inc	15h
	inc	25h
	dec	06h
	dec	16h
	dec	36h
	ror	07h
	ror	17h
	rol	08h
	rol	18h
	clg
	seg
	call	123h
	ret
	rti

	beq	$+1	; allows plus and minus displacement, some other opcodes
	beq	$+0
	bze	$+2
	bze	$-1
	bne	$+3
	bne	$-2
	bnz	$+4
	bnz	$-3
	bcs	$+5
	bcs	$-4
	bcc	$+6
	bcc	$-5
	bgt	$+7
	bgt	$-6
	ble	$+8
	ble	$-7
	bge	$+9
	bge	$-8
	blt	$+10
	blt	$-9

	pitch	9
	msa	123h
	mso
	activate
	kenab
	kdsab
	status	0ah
	status	1ah
	flagin	0bh
	flagin	1bh
	s1rate	0ch
	s1rate	1ch
	s2rate	0dh
	s2rate	1dh

	cpu	msm6052

	rdar
	rdar	+
	rdar	-
	rdar	+,z
	rdar	-,z
	rdar	+,n
	rdar	-,n
	rdar	+,z,l
	rdar	-,z,l
	rdar	+,n,l
	rdar	-,n,l
	mvar
	mvar	+
	mvar	-
	mvar	+,z
	mvar	-,z
	mvar	+,n
	mvar	-,n
	mvar	+,l
	mvar	-,l
	mvar	+,z,l
	mvar	-,z,l
	mvar	+,n,l
	mvar	-,n,l

	beq	$+1		; completely different machine codes on 6052
	beq	$-0
	bze	$+2
	bze	$-1
	bne	$+3
	bne	$-2
	bnz	$+4
	bnz	$-3
	bcs	$+5
	bcs	$-4
	bcc	$+6
	bcc	$-5
	bgt	$+7
	bgt	$-6
	ble	$+8
	ble	$-7
	bge	$+9
	bge	$-8
	blt	$+10
	blt	$-9

	in	7,0fh
	in	7,1fh
	out	0eh,12
	out	1eh,12
	out	0dh,30
	out	1dh,30
	out	#0dh,12
	out	#0dh,30

	stop
	halt
	act
	ei
	di
	et
	dt
	ec
	dc
	om
	im
	rst
