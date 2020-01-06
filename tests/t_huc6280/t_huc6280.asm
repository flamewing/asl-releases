	cpu	huc6280
	page	0

	adc	#$45
	adc	$45
	adc	$45,x
	adc	$4567
	adc	$4567,x
	adc	$4567,y
	adc	($45)
	adc	($45,x)
	adc	($45),y

	and	#$45
	and	$45
	and	$45,x
	and	$4567
	and	$4567,x
	and	$4567,y
	and	($45)
	and	($45,x)
	and	($45),y

	asl
	asl	a
	asl	$45
	asl	$45,x
	asl	$4567
	asl	$4567,x

	bbr0	$45,*
	bbr1	$45,*
	bbr2	$45,*
	bbr3	$45,*
	bbr4	$45,*
	bbr5	$45,*
	bbr6	$45,*
	bbr7	$45,*

	bbs0	$45,*
	bbs1	$45,*
	bbs2	$45,*
	bbs3	$45,*
	bbs4	$45,*
	bbs5	$45,*
	bbs6	$45,*
	bbs7	$45,*

	bcc	*

	bcs	*

	beq	*

	bit	#$45
	bit	$45
	bit	$45,x
	bit	$4567
	bit	$4567,x

	bmi	*

	bne	*

	bpl	*

	bra	*

	brk

	bsr	*

	bvc	*

	bvs	*

	cla

	clc

	cld

	cli

	clv

	clx

	cly

	cmp	#$45
	cmp	$45
	cmp	$45,x
	cmp	$4567
	cmp	$4567,x
	cmp	$4567,y
	cmp	($45)
	cmp	($45,x)
	cmp	($45),y

	cpx	#$45
	cpx	$45
	cpx	$4567

	cpy	#$45
	cpy	$45
	cpy	$4567

	csh

	csl

	dec	$45
	dec	$45,x
	dec	$4567
	dec	$4567,x

	dex

	dey

	eor	#$45
	eor	$45
	eor	$45,x
	eor	$4567
	eor	$4567,x
	eor	$4567,y
	eor	($45)
	eor	($45,x)
	eor	($45),y

	inc
	inc	a
	inc	$45
	inc	$45,x
	inc	$4567
	inc	$4567,x

	inx

	iny

	jmp	$4567
	jmp	($4567)
	jmp	($4567,x)

	jsr	$4567

	lda	#$45
	lda	$45
	lda	$45,x
	lda	$4567
	lda	$4567,x
	lda	$4567,y
	lda	($45)
	lda	($45,x)
	lda	($45),y

	ldx	#$45
	ldx	$45
	ldx	$45,y
	ldx	$4567
	ldx	$4567,y

	ldy	#$45
	ldy	$45
	ldy	$45,x
	ldy	$4567
	ldy	$4567,x

	lsr
	lsr	a
	lsr	$45
	lsr	$45,x
	lsr	$4567
	lsr	$4567,x

	nop

	ora	#$45
	ora	$45
	ora	$45,x
	ora	$4567
	ora	$4567,x
	ora	$4567,y
	ora	($45)
	ora	($45,x)
	ora	($45),y

	pha

	php

	phx

	phy

	pla

	plp

	plx

	ply

	rmb0	$45
	rmb1	$45
	rmb2	$45
	rmb3	$45
	rmb4	$45
	rmb5	$45
	rmb6	$45
	rmb7	$45

	rol
	rol	a
	rol	$45
	rol	$45,x
	rol	$4567
	rol	$4567,x

	ror
	ror	a
	ror	$45
	ror	$45,x
	ror	$4567
	ror	$4567,x

	rti

	rts

	sax

	say

	sbc	#$45
	sbc	$45
	sbc	$45,x
	sbc	$4567
	sbc	$4567,x
	sbc	$4567,y
	sbc	($45)
	sbc	($45,x)
	sbc	($45),y

	sec

	sed

	sei

	set

	smb0	$45
	smb1	$45
	smb2	$45
	smb3	$45
	smb4	$45
	smb5	$45
	smb6	$45
	smb7	$45

	st0	#$45
	st1	#$45
	st2	#$45

	sta	$45
	sta	$45,x
	sta	$4567
	sta	$4567,x
	sta	$4567,y
	sta	($45)
	sta	($45,x)
	sta	($45),y

	stx	$45
	stx	$45,y
	stx	$4567

	sty	$45
	sty	$45,x
	sty	$4567

	stz	$45
	stz	$45,x
	stz	$4567
	stz	$4567,x

	sxy

	tai	$4567,$5678,$6789

	tam	#$03

	tax

	tay

	tdd	$4567,$5678,$6789

	tia	$4567,$5678,$6789

	tii	$4567,$5678,$6789

	tin	$4567,$5678,$6789

	tma	#$40

	trb	$45
	trb	$4567

	tsb	$45
	tsb	$4567

	tst	#$45,$56
	tst	#$45,$56,x
	tst	#$45,$5678
	tst	#$45,$5678,x

	tsx

	txa

	txs

	tya

	; now play with the MPRs

	assume	mpr0:$23
	assume	mpr7:$24

	lda	$46004
	lda	$48004
	;lda	$04


