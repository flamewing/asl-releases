	cpu	78c10

	block
	calb
	clc
	daa
	di
	ei
	exa
	exh
	exx
	hlt
	jb
	jea
	nega
	nop
	ret
	reti
	rets
	rld
	rrd
	softi
	stc
	stop
	table

	aci	a,0
	aci	h,10
	aci	pc,55h

	adi	a,40
	adi	b,33o
	adi	eom,6

	adinc	a,56
	ani	l,33h
	eqi	mkh,34
	gti	a,20
	lti	c,34
	nei	mkl,10
	offi	a,0ffh
	oni	d,0
	ori	anm,47
	sbi	a,41h
	sui	v,7
	suinb	smh,98

	xri	a,055h
	xri	v,1010b
	xri	pb,40h

	adc	a,v
	adc     v,a
	addnc	a,b
	addnc	b,a
	sub	a,c
	sub	c,a
	sbb	a,d
	sbb	d,a
	subnb	a,e
	subnb	e,a
	ana	a,h
	ana	h,a
	ora	a,l
	ora	l,a
	xra	a,v
	xra	v,a
	gta	a,b
	gta	b,a
	lta	a,c
	lta	c,a
	nea	a,d
	nea	d,a
	eqa	a,e
	eqa	e,a
	ona	a,h
	ona	h,a
	offa	a,l
	offa	l,a

	assume	v:0

	adcw	10h
	addncw	20h
	addw	30h
	subw	40h
	sbbw	50h
	subnbw	60h
	anaw	70h
	oraw	80h
	xraw	90h
	gtaw	0a0h
	ltaw	0b0h
	neaw	0c0h
	eqaw	0d0h
	onaw	0e0h
	offaw	0f0h

	adcx	b
	addncx	d
	addx	h
	subx	d+
	sbbx	h+
	subnbx	d-
	anax	h-
	orax	b
	xrax	d
	gtax	h
	ltax	d+
	neax	h+
	eqax	d-
	onax	h-
	offax	b

	dadc	ea,b
	daddnc	ea,d
	dadd	ea,h
	dsub	ea,b
	dsbb	ea,d
	dsubnb	ea,h
	dan	ea,b
	dor	ea,d
	dxr	ea,h
	dgt	ea,b
	dlt	ea,d
	dne	ea,h
	deq	ea,b
	don	ea,d
	doff	ea,h

	aniw	10h,'A'
	eqiw	20h,'B'
	gtiw	30h,'C'
	ltiw	40h,'D'
	neiw	50h,'E'
	oniw	60h,'F'
	offiw	70h,'G'
	oriw	80h,'H'

	call	1234h
	jmp	5678h
	lbcd	1234h
	sbcd	5678h
	lded	1234h
	sded	5678h
	lhld	1234h
	shld	5678h
	lspd	1234h
	sspd	5678h

	calf	0c08h
	calt	150

	bit	5,20h
	bit	2,0ffh

	dcr	a
	inr	b
	mul	c
	div	a
	sll	b
	slr	c
	sllc	a
	slrc	b
	rll	c
	rlr	a

	dcrw	20h
	inrw	30h
	ldaw	40h
	staw	50h

	inx	ea
	dcx	ea
	inx	h
	dcx	b

        dmov    ea,b
        dmov    h,ea
        dmov    ea,ecpt
        dmov    etm0,ea

	drll	ea
	dsll	ea
	drlr	ea
	dslr	ea

	eadd	ea,b
	esub	ea,c

back:	nop
	jr	back
        jre     $-100
	jr	forw
        jre     $+100
forw:	nop

	ldax	b
	ldax	d
	ldax	h
	ldax	d+
	ldax	h+
	ldax	d-
	ldax	h-
	ldax	d+20
	ldax	d-30
	ldax	h+a
	ldax	h+b
	ldax	h+ea
	ldax	h+40
	ldax	h-50
	stax	b
	stax	d
	stax	h
	stax	d+
	stax	h+
	stax	d-
	stax	h-
	stax	d+20
	stax	d-30
	stax	h+a
	stax	h+b
	stax	h+ea
	stax	h+40
	stax	h-50

	ldeax	h++
	steax	d++
	ldeax	h+ea
	steax	h-5

	lxi	sp,2000h
	lxi	h,101010101010b
	lxi	ea,1001

	mov	a,eal
	mov	h,a
	mov	a,pa
	mov	pa,a
	mov	c,1000h
	mov	1234h,d

	mvi	d,55h
	mvi	eom,0

	mviw	20h,01101001b
	mvix	d,22o

	push	v
	push	b
	pop	h
	pop	ea

	sk	z
	skn	hc

	skit	fsr
	skint	an6
