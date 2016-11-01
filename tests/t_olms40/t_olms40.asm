	cpu	msm5840
	page	0

	org	0

	cla
	cll
	clh
	lai	12
	ld	a,#12		; alternate
	lli	13
	ld	dpl,#13		; alternate
	lhi	14
	ld	dph,#14		; alternate
	l
	ld	a,(dp)		; alternate
	lm	2
	lal
	ld	a,dpl		; alternate
	lla
	ld	dpl,a		; alternate
	law
	ld	a,w		; alternate
	lax
	ld	a,x		; alternate
	lay
	ld	a,y		; alternate
	lay
	ld	a,y		; alternate
	laz
	ld	a,z		; alternate
	si
	smi	3
	lwa
	ld	w,a		; alternate
	lxa
	ld	x,a		; alternate
	lya
	ld	y,a		; alternate
	lza
	ld	z,a		; alternate
	lpa
	ld	pp,a		; alternate
	lti	0efh
	ld	t,0efh		; alternate
	rth
	ld	a,th		; alternate
	rtl
	ld	a,tl		; alternate

	xa
	xl
	xch
	x
	xm	1
	xax

	ina
	inc	a		; alternate
	inl
	inc	dpl		; alternate
	inm
	inc	(dp)		; alternate
	inw
	inc	w		; alternate
	inx
	inc	x		; alternate
	iny
	inc	y		; alternate
	inz
	inc	z		; alternate
	dca
	dec	a		; alternate
	dcl
	dec	dpl		; alternate
	dcm
	dec	(dp)		; alternate
	dcw
	dec	w		; alternate
	dcx
	dec	x		; alternate
	dcy
	dec	y		; alternate
	dcz
	dec	z		; alternate
	dch
	dec	dph		; alternate

	cao
	and
	or
	eor
	ral

	ac
	acs
	as
	ais	7
	das
	cm
	aws
	axs
	ays
	azs

	spb	0
	bset	(pp),0		; alternate
	rpb	1
	bclr	(pp),1		; alternate
	smb	2
	bset	(dp),2		; alternate
	rmb	3
	bclr	(dp),3		; alternate
	tab	0
	btst	a,0		; alternate
	tmb	1
	btst	(dp),1		; alternate
	tkb	2
	thb	1		; port H is only 2 bits wide!
	ti
	ttm
	tc
	sc
	rc

	j	$
	jc	$
	ja
	cal	3ffh
	rt

	obs
	otd
	oa
	ob
	op
	oab
	opm
	ia
	ib
	ib
	ik
	iab

	ei
	di
	et
	dt
	ect
	dct
	hlt
	exp
	nop
