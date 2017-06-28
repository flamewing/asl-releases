	cpu	65c19
	page	0

	brk
	ora	($12)
	mpy
	tip
	byt	$04
	ora	$12
	asl	$12
	rmb0	$12
	php
	ora	#$12
	asl
	asl	a
	jsb	$ffe0
	jpi	($1234)
	ora	$1234
	asl	$1234
	bbr0	$12,*
	bpl	*
	ora	($12),x
	mpa
	lab
	byt	$14
	ora	$12,x
	asl	$12,x
	rmb1	$12
	clc
	ora	$1234,y
	neg
	neg	a
	jsb	$ffe2
	byt	$1c
	ora	$1234,x
	asl	$1234,x
	bbr1	$12,*
	jsr	$1234
	and	($12)
	psh
	phw
	bit	$12
	and	$12
	rol	$12
	rmb2	$12
	plp
	and	#$12
	rol
	rol	a
	jsb	$ffe4
	bit	$1234
	and	$1234
	rol	$1234
	bbr2	$12,*
	bmi	*
	and	($12),x
	pul
	plw
	byt	$34
	and	$12,x
	rol	$12,x
	rmb3	$12
	sec
	and	$1234,y
	asr
	asr	a
	jsb	$ffe6
	byt	$3c
	and	$1234,x
	rol	$1234,x
	bbr3	$12,*
	rti
	eor	($12)
	rnd
	byt	$43
	byt	$44
	eor	$12
	lsr	$12
	rmb4	$12
	pha
	eor	#$12
	lsr
	lsr	a
	jsb	$ffe8
	jmp	$1234
	eor	$1234
	lsr	$1234
	bbr4	$12,*
	bvc	*
	eor	($12),x
	clw
	byt	$53
	byt	$54
	eor	$12,x
	lsr	$12,x
	rmb5	$12
	cli
	eor	$1234,y
	phy
	jsb	$ffea
	byt	$5c
	eor	$1234,x
	lsr	$1234,x
	bbr5	$12,*
	rts
	adc	($12)
	taw
	byt	$63
	add	$12
	adc	$12
	ror	$12
	rmb6	$12
	pla
	adc	#$12
	ror
	ror	a
	jsb	$ffec
	jmp	($1234)
	adc	$1234
	ror	$1234
	bbr6	$12,*
	bvs	*
	adc	($12),x
	twa
	byt	$73
	add	$12,x
	adc	$12,x
	ror	$12,x
	rmb7	$12
	sei
	adc	$1234,y
	ply
	jsb	$ffee
	jmp	($1234,x)
	adc	$1234,x
	ror	$1234,x
	bbr7	$12,*
	bra	*
	sta	($12)
	byt	$82
	byt	$83
	sty	$12
	sta	$12
	stx	$12
	smb0	$12
	dey
	add	#$12
	txa
	nxt
	sty	$1234
	sta	$1234
	stx	$1234
	bbs0	$12,*
	bcc	*
	sta	($12),x
	byt	$92
	byt	$93
	sty	$12,x
	sta	$12,x
	stx	$12,y
	smb1	$12
	tya
	sta	$1234,y
	txs
	lii
	byt	$9c
	sta	$1234,x
	byt	$9e
	bbs1	$12,*
	ldy	#$12
	lda	($12)
	ldx	#$12
	byt	$a3
	ldy	$12
	lda	$12
	ldx	$12
	smb2	$12
	tay
	lda	#$12
	tax
	lan
	ldy	$1234
	lda	$1234
	ldx	$1234
	bbs2	$12,*
	bcs	*
	lda	($12),x
	sti	$12,#$34
	byt	$b3
	ldy	$12,x
	lda	$12,x
	ldx	$12,y
	smb3	$12
	clv
	lda	$1234,y
	tsx
	ini
	ldy	$1234,x
	lda	$1234,x
	ldx	$1234,y
	bbs3	$12,*
	cpy	#$12
	cmp	($12)
	rba	$1234,#$45
	byt	$c3
	cpy	$12
	cmp	$12
	dec	$12
	smb4	$12
	iny
	cmp	#$12
	dex
	phi
	cpy	$1234
	cmp	$1234
	dec	$1234
	bbs4	$12,*
	bne	*
	cmp	($12),x
	sba	$1234,#$45
	byt	$d3
	exc	$12,x
	cmp	$12,x
	dec	$12,x
	smb5	$12
	cld
	cmp	$1234,y
	phx
	pli
	byt	$dc
	cmp	$1234,x
	dec	$1234,x
	bbs5	$12,*
	cpx	#$12
	sbc	($12)
	bar	$1234,#$45,*
	byt	$e3
	cpx	$12
	sbc	$12
	inc	$12
	smb6	$12
	inx
	sbc	#$12
	nop
	lai
	cpx	$1234
	sbc	$1234
	inc	$1234
	bbs6	$12,*
	beq	*
	sbc	($12),x
	bas	$1234,#$45,*
	byt	$f3
	byt	$f4
	sbc	$12,x
	inc	$12,x
	smb7	$12
	sed
	sbc	$1234,y
	plx
	pia
	byt	$fc
	sbc	$1234,x
	inc	$1234,x
	bbs7	$12,*
