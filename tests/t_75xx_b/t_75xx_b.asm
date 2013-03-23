	cpu	7566

	lai	4
	lhi	3
	lam	hl+
	lam	hl-
	lam	hl
	lhli	23
	st
	stii	7
	xal
	xam	hl+
	xam	hl-
	xam	hl

	aisc	12
	asc
	acsc
	exl

	cma
	rc
	sc

	ils
	idrs	1fh
	dls
	ddrs	24h

	rmb	1
	smb	3

	jmp	123h
	jcp	target

	call	321h
	cal	100h
	cal	107h
	cal	140h
	cal	147h
	cal	180h
	cal	187h
	cal	1c0h
	cal	1c7h
	;cal	000h
	;cal 	080h
	;cal	200h
	rt
	rts
	tamsp
target:
	skc
	skabt	1
	skmbt	2
	skmbf	3
	skaem
	skaei	13
	ski	0

	tamsio
	tsioam
	sio

	timer
	tcntam

	ipl
	opl
	rpbl
	spbl

	halt
	stop
	nop
