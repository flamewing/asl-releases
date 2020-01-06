	cpu	65ce02
	page	0

	cle
	see

	phz
	plz
	inz
	dez

	tsy
	tys
	tab
	tba
	taz
	tza

	rtn	#$ee
	aug

	stx	$1234,y
	sty	$1234,x

	jsr	($1234)

	irp	instr,ora,and,eor,adc,sta,lda,cmp,sbc
	instr	($aa),z
	endm

	ldz	$1234
	ldz	$1234,x
	ldz	$12
	ldz	$12,x
	ldz	#$55

	cpz	$12
	cpz	$1234
	cpz	#$55

	bcc	*+20
	bcc	*+200
	bcc	<*+20
	bcc	>*+20
	;bcc	<*+200
	bcc	>*+200

	bcs	*+20
	bcs	*+200
	bcs	<*+20
	bcs	>*+20
	;bcs	<*+200
	bcs	>*+200

	beq	*+20
	beq	*+200
	beq	<*+20
	beq	>*+20
	;beq	<*+200
	beq	>*+200

	bmi	*+20
	bmi	*+200
	bmi	<*+20
	bmi	>*+20
	;bmi	<*+200
	bmi	>*+200

	bne	*+20
	bne	*+200
	bne	<*+20
	bne	>*+20
	;bne	<*+200
	bne	>*+200

	bpl	*+20
	bpl	*+200
	bpl	<*+20
	bpl	>*+20
	;bpl	<*+200
	bpl	>*+200

	bru	*+20
	bru	*+200
	bru	<*+20
	bru	>*+20
	;bru	<*+200
	bru	>*+200

	bra	*+20
	bra	*+200
	bra	<*+20
	bra	>*+20
	;bra	<*+200
	bra	>*+200

	bsr	*+20
	bsr	*+200
	;bsr	<*+20
	bsr	>*+20
	;bsr	<*+200
	bsr	>*+200

	bvc	*+20
	bvc	*+200
	bvc	<*+20
	bvc	>*+20
	;bvc	<*+200
	bvc	>*+200

	bvs	*+20
	bvs	*+200
	bvs	<*+20
	bvs	>*+20
	;bvs	<*+200
	bvs	>*+200

	jsr	($1234,x)

	neg
	neg	a

	asr
	asr	a
	asr	$12
	asr	$12,x

	inw	$12
	dew	$12

	asw	$12
	asw	$1234

	row	$12
	row	$1234

	phw	$12
	phw	$1234
	phw	#$1234

	lda	($12,sp),y
	sta	($12,sp),y

	assume	b:$80

	lda	<$8034
	lda	$34
