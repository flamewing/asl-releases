		cpu	6804

		include	stddef04.inc

		clra
		clrx
		clry
		coma
		rola
		asla
		inca
		incx
		incy
		deca
		decx
		decy
		tax
		tay
		txa
		tya
		rts
		rti
		nop

targ:		beq	targ
		bne	targ
		blo	targ
		bcs	targ
		bhs	targ
		bcc	targ

		jmp	$123
		jsr	$456

		add	(x)
		add	(y)
		add	$30
		add	#$40
		sub	(x)
		sub	(y)
		sub	$50
		sub	#$60
		cmp	(x)
		cmp	(y)
		cmp	$70
		cmp	#$80
		and	(x)
		and	(y)
		and	$90
		and	#$a0

		lda	(x)
		sta	(y)
		lda	$82
		sta	$40
		lda	#55

		ldxi	#0
		ldyi	#-1

		mvi	$12,#$45

		dec	$82
		inc	$40
		dec	(y)
		inc	(x)

		bset	1,$12
		bclr    3,$34

		brset	5,$56,targ
		brclr	7,$78,targ
