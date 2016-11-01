	cpu	HD614023
	page	0

	lai	7
	ld	#7,a		; AS-specific equivalent
	lbi	12
	ld	#12,b		; AS-specific equivalent
	lmid	3,$7f
	ld	#3,$7f		; AS-specific equivalent
	lmiiy	4
	ld	#4,m+		; AS-specific equivalent

	lab
	ld	b,a		; AS-specific equivalent
	lba
	ld	a,b		; AS-specific equivalent
	lay
	ld	y,a		; AS-specific equivalent
	laspx
	ld	spx,a		; AS-specific equivalent
	laspy
	ld	spy,a		; AS-specific equivalent
	lamr	5
	ld	mr5,a		; AS-specific equivalent
	xmra	1
	xch	a,mr1		; AS-specific equivalent
	xch	mr1,a		; AS-specific equivalent

	lwi	3
	ld	#3,w		; AS-specific equivalent
	lxi	2
	ld	#2,x		; AS-specific equivalent
	lyi	4
	ld	#4,y		; AS-specific equivalent
	lxa
	ld	a,x		; AS-specific equivalent
	lya
	ld	a,y		; AS-specific equivalent
	iy
	inc	y		; AS-specific equivalent
	dy
	dec	y		; AS-specific equivalent
	ayy
	add	a,y		; AS-specific equivalent
	syy
	sub	a,y		; AS-specific equivalent
	xspx
	xch	x,spx		; AS-specific equivalent
	xch	spx,x		; AS-specific equivalent
	xspy
	xch	y,spy		; AS-specific equivalent
	xch	spy,y		; AS-specific equivalent
	xspxy

	lam
	ld	m,a		; AS-specific equivalent
	lamx
	lamy
	lamxy
	lamd	$7e
	ld	$7e,a		; AS-specific equivalent
	lbm
	ld	m,b		; AS-specific equivalent
	lbmx
	lbmy
	lbmxy
	lma
	ld	a,m		; AS-specific equivalent
	lmax
	lmay
	lmaxy
	lmad	$7d
	ld	a,$7d		; AS-specific equivalent
	lmaiy
	ld	a,m+		; AS-specific equivalent
	lmaiyx
	lmady
	ld	a,m-		; AS-specific equivalent
	lmadyx
	xma
	xch	a,m		; AS-specific equivalent
	xch	m,a
	xmax
	xmay
	xmaxy
	xmad	$7d
	xch	a,$7d		; AS-specific equivalent
	xch	$7d,a		; AS-specific equivalent
	xmb
	xch	b,m		; AS-specific equivalent
	xch	m,b		; AS-specific equivalent
	xmbx
	xmby
	xmbxy

	ai	5
	add	#5,a		; AS-specific equivalent
	ib
	inc	b		; AS-specific equivalent
	db
	dec	b		; AS-specific equivalent
	daa
	das
	nega
	comb
	rotr
	rotl
	sec
	bset	ca		; AS-specific equivalent
	rec
	bclr	ca		; AS-specific equivalent
	tc
	btst	ca		; AS-specific equivalent
	am
	add	m,a		; AS-specific equivalent
	amd	$7c
	add	$7c,a		; AS-specific equivalent
	amc
	adc	m,a		; AS-specific equivalent
	amcd	$7b
	adc	$7b,a		; AS-specific equivalent
	smc
	sbc	m,a		; AS-specific equivalent
	smcd	$7a
	sbc	$7a,a		; AS-specific equivalent
	or
	or	b,a		; AS-specific equivalent
	anm
	and	m,a		; AS-specific equivalent
	anmd	$79
	and	$79,a		; AS-specific equivalent
	orm
	or	m,a		; AS-specific equivalent
	ormd	$78
	or	$78,a		; AS-specific equivalent
	eorm
	eor	m,a		; AS-specific equivalent
	eormd	$77
	eor	$77,a		; AS-specific equivalent

	inem	6
	cp	ne,m,#6		; AS-specific equivalent
	cp	ne,#6,m		; AS-specific equivalent
	inemd	5,$76
	cp	ne,$76,#5	; AS-specific equivalent
	cp	ne,#5,$76	; AS-specific equivalent
	anem
	cp	ne,m,a		; AS-specific equivalent
	cp	ne,a,m		; AS-specific equivalent
	anemd	$75
	cp	ne,$75,a	; AS-specific equivalent
	cp	ne,a,$75	; AS-specific equivalent
	bnem
	cp	ne,m,b		; AS-specific equivalent
	cp	ne,b,m		; AS-specific equivalent
	ynei	9
	cp	ne,#9,y		; AS-specific equivalent
	cp	ne,y,#9		; AS-specific equivalent
	ilem	8
	cp	le,m,#8		; AS-specific equivalent
	ilemd	7,$74
	cp	le,$74,#7	; AS-specific equivalent
	alem
	cp	le,m,a		; AS-specific equivalent
	alemd	$73
	cp	le,$73,a	; AS-specific equivalent
	blem
	cp	le,m,b		; AS-specific equivalent
	alei	2
	cp	le,#2,a		; AS-specific equivalent

	sem	1
	bset	1,m		; AS-specific equivalent
	semd	2,$72
	bset	2,$72		; AS-specific equivalent
	rem	3
	bclr	3,m		; AS-specific equivalent
	remd	0,$71
	bclr	0,$71		; AS-specific equivalent
	tm	1
	btst	1,m		; AS-specific equivalent
	tmd	2,$70
	btst	2,$70		; AS-specific equivalent

	br	$ab
	brl	$7ab
	jmpl	$7ab
	cal	5
	call	$7ab
	tbr	4
	rtn
	rtni

	sed
	sedd	1
	red
	redd	2
	td
	tdd	3
	lar	4
	lbr	5
	lra	6
	lrb	7
	p	8

	nop
	sby
	stop

