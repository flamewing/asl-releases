	cpu	320C541
	page	0

	org	0

	; no arg

	fret
	fretd
	frete
	freted
	nop
	reset
	ret
	retd
	rete
	reted
	retf
	retfd

	; accumulator arg

	exp	a
	exp	b

	min	a
	min	b

	max	a
	max	b

	sat	a
	sat	b

	rol	a
	rol	b

	roltc	a
	roltc	b

	ror	a
	ror	b

	sftc	a
	sftc	b

	cala	a
	cala	b	

	calad	a
	calad	b

	fcala	a
	fcala	b

	fcalad	a
	fcalad	b

	bacc	a
	bacc	b	

	baccd	a
	baccd	b

	fbacc	a
	fbacc	b

	fbaccd	a
	fbaccd	b

	neg	a
	neg	b
	neg	a,a
	neg	a,b
	neg	b,a
	neg	b,b

	norm	a
	norm	b
	norm	a,a
	norm	a,b
	norm	b,a
	norm	b,b

	rnd	a
	rnd	b
	rnd	a,a
	rnd	a,b
	rnd	b,a
	rnd	b,b

	abs	a
	abs	b
	abs	a,a
	abs	a,b
	abs	b,a
	abs	b,b

	cmpl	a
	cmpl	b
	cmpl	a,a
	cmpl	a,b
	cmpl	b,a
	cmpl	b,b

; memory arg

	delay	*ar1
	delay	*ar2-
	delay	*ar3+
	delay	*+ar4
	delay	*ar5-0b
	delay	*ar6-0
	delay	*ar7+0
	delay	*ar1+0b
	delay	*ar2-%
	delay	*ar3-0%
	delay	*ar4+%
	delay	*ar5+0%
	delay	*ar6(30)
	delay	*+ar7(20+15)
	delay	*+ar1(10*(1+5))%
	delay	*(1234h)
	delay	6eh

	poly	*ar1
	poly	*ar2-
	poly	*ar3+
	poly	*+ar4
	poly	*ar5-0b
	poly	*ar6-0
	poly	*ar7+0
	poly	*ar1+0b
	poly	*ar2-%
	poly	*ar3-0%
	poly	*ar4+%
	poly	*ar5+0%
	poly	*ar6(30)
	poly	*+ar7(20+15)
	poly	*+ar1(10*(1+5))%
	poly	*(1234h)
	poly	6eh

	bitt	*ar1
	bitt	*ar2-
	bitt	*ar3+
	bitt	*+ar4
	bitt	*ar5-0b
	bitt	*ar6-0
	bitt	*ar7+0
	bitt	*ar1+0b
	bitt	*ar2-%
	bitt	*ar3-0%
	bitt	*ar4+%
	bitt	*ar5+0%
	bitt	*ar6(30)
	bitt	*+ar7(20+15)
	bitt	*+ar1(10*(1+5))%
	bitt	*(1234h)
	bitt	6eh

	popd	*ar1
	popd	*ar2-
	popd	*ar3+
	popd	*+ar4
	popd	*ar5-0b
	popd	*ar6-0
	popd	*ar7+0
	popd	*ar1+0b
	popd	*ar2-%
	popd	*ar3-0%
	popd	*ar4+%
	popd	*ar5+0%
	popd	*ar6(30)
	popd	*+ar7(20+15)
	popd	*+ar1(10*(1+5))%
	popd	*(1234h)
	popd	6eh

	pshd	*ar1
	pshd	*ar2-
	pshd	*ar3+
	pshd	*+ar4
	pshd	*ar5-0b
	pshd	*ar6-0
	pshd	*ar7+0
	pshd	*ar1+0b
	pshd	*ar2-%
	pshd	*ar3-0%
	pshd	*ar4+%
	pshd	*ar5+0%
	pshd	*ar6(30)
	pshd	*+ar7(20+15)
	pshd	*+ar1(10*(1+5))%
	pshd	*(1234h)
	pshd	6eh

	mar	*ar1
	mar	*ar2-
	mar	*ar3+
	mar	*+ar4
	mar	*ar5-0b
	mar	*ar6-0
	mar	*ar7+0
	mar	*ar1+0b
	mar	*ar2-%
	mar	*ar3-0%
	mar	*ar4+%
	mar	*ar5+0%
	mar	*ar6(30)
	mar	*+ar7(20+15)
	mar	*+ar1(10*(1+5))%
	mar	*(1234h)
	mar	6eh

	ltd	*ar1
	ltd	*ar2-
	ltd	*ar3+
	ltd	*+ar4
	ltd	*ar5-0b
	ltd	*ar6-0
	ltd	*ar7+0
	ltd	*ar1+0b
	ltd	*ar2-%
	ltd	*ar3-0%
	ltd	*ar4+%
	ltd	*ar5+0%
	ltd	*ar6(30)
	ltd	*+ar7(20+15)
	ltd	*+ar1(10*(1+5))%
	ltd	*(1234h)
	ltd	6eh

	reada	*ar1
	reada	*ar2-
	reada	*ar3+
	reada	*+ar4
	reada	*ar5-0b
	reada	*ar6-0
	reada	*ar7+0
	reada	*ar1+0b
	reada	*ar2-%
	reada	*ar3-0%
	reada	*ar4+%
	reada	*ar5+0%
	reada	*ar6(30)
	reada	*+ar7(20+15)
	reada	*+ar1(10*(1+5))%
	reada	*(1234h)
	reada	6eh

	writa	*ar1
	writa	*ar2-
	writa	*ar3+
	writa	*+ar4
	writa	*ar5-0b
	writa	*ar6-0
	writa	*ar7+0
	writa	*ar1+0b
	writa	*ar2-%
	writa	*ar3-0%
	writa	*ar4+%
	writa	*ar5+0%
	writa	*ar6(30)
	writa	*+ar7(20+15)
	writa	*+ar1(10*(1+5))%
	writa	*(1234h)
	writa	6eh

	pshm	*ar1
	pshm	*ar2-
	pshm	*ar3+
	pshm	*+ar4
	pshm	*ar5-0b
	pshm	*ar6-0
	pshm	*ar7+0
	pshm	*ar1+0b
	pshm	*ar2-%
	pshm	*ar3-0%
	pshm	*ar4+%
	pshm	*ar5+0%
;	pshm	*ar6(30)
;	pshm	*+ar7(20+15)
;	pshm	*+ar1(10*(1+5))%
;	pshm	*(1234h)
	pshm	6eh

	; two X-Y-memory operands

	abdst	*ar3+,*ar4
	abdst	*ar2,*ar5+0%

	lms	*ar3+,*ar4
	lms	*ar2,*ar5+0%

	sqdst	*ar3+,*ar4
	sqdst	*ar2,*ar5+0%

	mvdd	*ar3+,*ar4
	mvdd	*ar2,*ar5+0%

	; now the complex stuff...

	add	a,3			; Form 9
	add	b,-5
	add	a,3,a
	add	b,-5,b
	add	a,7,b
	add	b,-10,a
	add	a,asm			; Form 10
	add	b,asm
	add	a,asm,a
	add	a,asm,b
	add	b,asm,a
	add	b,asm,b
	add	#1234,a			; Form 7
	add	#1234,b
	add	#1234,a,a
	add	#1234,a,b
	add	#1234,b,a
	add	#1234,b,b
	add	#1234,10,a
	add	#1234,10,b
	add	#1234,10,a,a
	add	#1234,10,a,b
	add	#1234,10,b,a
	add	#1234,10,b,b
	add	#1234,16,a		; Form 8
	add	#1234,16,b
	add	#1234,16,a,a
	add	#1234,16,a,b
	add	#1234,16,b,a
	add	#1234,16,b,b
        add	*(1234h),a		; Form 1
	add	*+ar4(1234h),b
	add	*ar5+,ts,a		; Form 2
	add	*+ar5,ts,b
	add	*(1234h),16,a		; Form 3
	add	*+ar4(1234h),16,b
	add	*(1234h),16,a,a
	add	*ar5+,16,a,b
	add	*+ar5,16,b,a
	add	*+ar4(1234h),16,b,b
	add	*(1234h),10,a		; Form 4
	add	*+ar4(1234h),-10,b
	add	*(1234h),10,a,a
	add	*ar5+,10,a,b
	add	*+ar5,-10,b,a
	add	*+ar4(1234h),-10,b,b
	add	*ar2,3,a		; Form 5
	add	*ar5+,7,b
	add	*ar5+,*ar4-,b		; Form 6

	sub	a,3			; Form 9
	sub	b,-5
	sub	a,3,a
	sub	b,-5,b
	sub	a,7,b
	sub	b,-10,a
	sub	a,asm			; Form 10
	sub	b,asm
	sub	a,asm,a
	sub	a,asm,b
	sub	b,asm,a
	sub	b,asm,b
	sub	#1234,a			; Form 7
	sub	#1234,b
	sub	#1234,a,a
	sub	#1234,a,b
	sub	#1234,b,a
	sub	#1234,b,b
	sub	#1234,10,a
	sub	#1234,10,b
	sub	#1234,10,a,a
	sub	#1234,10,a,b
	sub	#1234,10,b,a
	sub	#1234,10,b,b
	sub	#1234,16,a		; Form 8
	sub	#1234,16,b
	sub	#1234,16,a,a
	sub	#1234,16,a,b
	sub	#1234,16,b,a
	sub	#1234,16,b,b
        sub	*(1234h),a		; Form 1
	sub	*+ar4(1234h),b
	sub	*ar5+,ts,a		; Form 2
	sub	*+ar5,ts,b
	sub	*(1234h),16,a		; Form 3
	sub	*+ar4(1234h),16,b
	sub	*(1234h),16,a,a
	sub	*ar5+,16,a,b
	sub	*+ar5,16,b,a
	sub	*+ar4(1234h),16,b,b
	sub	*(1234h),10,a		; Form 4
	sub	*+ar4(1234h),-10,b
	sub	*(1234h),10,a,a
	sub	*ar5+,10,a,b
	sub	*+ar5,-10,b,a
	sub	*+ar4(1234h),-10,b,b
	sub	*ar2,3,a		; Form 5
	sub	*ar5+,7,b
	sub	*ar5+,*ar4-,b		; Form 6

; memory operand plus accumulator

	addc	*ar5+,a
	addc	*(1234h),b

	adds	*ar5+,a
	adds	*(1234h),b

	subb	*ar5+,a
	subb	*(1234h),b

	subc	*ar5+,a
	subc	*(1234h),b

	subs	*ar5+,a
	subs	*(1234h),b

	mpyr	*ar5+,a
	mpyr	*(1234h),b

	mpyu	*ar5+,a
	mpyu	*(1234h),b

	squra	*ar5+,a
	squra	*(1234h),b

	squrs	*ar5+,a
	squrs	*(1234h),b

	dadst	*ar5+,a
	dadst	*(1234h),b

	drsub	*ar5+,a
	drsub	*(1234h),b

	dsadt	*ar5+,a
	dsadt	*(1234h),b

	dsub	*ar5+,a
	dsub	*(1234h),b

	dld	*ar5+,a
	dld	*(1234h),b

	ldr	*ar5+,a
	ldr	*(1234h),b

	ldu	*ar5+,a
	ldu	*(1234h),b


	addm	#3456h,*ar6+
	addm	#3456h,*(1234h)

	andm	#3456h,*ar6+
	andm	#3456h,*(1234h)

	cmpm	*ar6+,#3456h
	cmpm	*(1234h),#3456h

	orm	#3456h,*ar6+
	orm	#3456h,*(1234h)

	xorm	#3456h,*ar6+
	xorm	#3456h,*(1234h)

	mpy	*ar6+,a
	mpy	*(1234h),b
	mpy	*ar4+,*ar2-,b
	mpy	#2345h,b

	mpya	a
	mpya	b
	mpya	*ar6+
	mpya	*(1234h)

	squr	*ar6+,a
	squr	*ar6+,b
	squr	*(1234h),a
	squr	*(1234h),b
	squr	a,a
	squr	a,b

	mac	*ar6+,a
	mac	*(1234h),a
	mac	*ar6+,b
	mac	*(1234h),b
	mac	*ar5+,*ar3-,a
	mac	*ar5+,*ar3-,b
	mac	*ar5+,*ar3-,a,a
	mac	*ar5+,*ar3-,a,b
	mac	*ar5+,*ar3-,b,a
	mac	*ar5+,*ar3-,b,b
	mac	#2345h,a
	mac	#2345h,b
	mac	#2345h,a,a
	mac	#2345h,a,b
	mac	#2345h,b,a
	mac	#2345h,b,b
	mac	*ar6+,#2345h,a
	mac	*ar6+,#2345h,b
	mac	*ar6+,#2345h,a,a
	mac	*ar6+,#2345h,a,b
	mac	*ar6+,#2345h,b,a
	mac	*ar6+,#2345h,b,b
	mac	*(1234h),#2345h,a
	mac	*(1234h),#2345h,b
	mac	*(1234h),#2345h,a,a
	mac	*(1234h),#2345h,a,b
	mac	*(1234h),#2345h,b,a
	mac	*(1234h),#2345h,b,b

	macr	*ar6+,a
	macr	*(1234h),a
	macr	*ar6+,b
	macr	*(1234h),b
	macr	*ar5+,*ar3-,a
	macr	*ar5+,*ar3-,b
	macr	*ar5+,*ar3-,a,a
	macr	*ar5+,*ar3-,a,b
	macr	*ar5+,*ar3-,b,a
	macr	*ar5+,*ar3-,b,b

	maca	*ar6+
	maca	*ar6+,b
	maca	*(1234h)
	maca	*(1234h),b
	maca	t,a
	maca	t,b
	maca	t,a,a
	maca	t,a,b
	maca	t,b,a
	maca	t,b,b

	macar	*ar6+
	macar	*ar6+,b
	macar	*(1234h)
	macar	*(1234h),b
	macar	t,a
	macar	t,b
	macar	t,a,a
	macar	t,a,b
	macar	t,b,a
	macar	t,b,b

	masa	*ar6+
	masa	*ar6+,b
	masa	*(1234h)
	masa	*(1234h),b
	masa	t,a
	masa	t,b
	masa	t,a,a
	masa	t,a,b
	masa	t,b,a
	masa	t,b,b

	macd	*ar6+,4567h,a
	macd	*ar6+,4567h,b
	macd	*(1234h),4567h,a
	macd	*(1234h),4567h,b

	macp	*ar6+,4567h,a
	macp	*ar6+,4567h,b
	macp	*(1234h),4567h,a
	macp	*(1234h),4567h,b

	macsu	*ar5+,*ar3-,a
	macsu	*ar5+,*ar3-,b

	mas	*ar6+,a
	mas	*ar6+,b
	mas	*(1234h),a
	mas	*(1234h),b
	mas	*ar5+,*ar3-,a
	mas	*ar5+,*ar3-,b
	mas	*ar5+,*ar3-,a,a
	mas	*ar5+,*ar3-,a,b
	mas	*ar5+,*ar3-,b,a
	mas	*ar5+,*ar3-,b,b

	masr	*ar6+,a
	masr	*ar6+,b
	masr	*(1234h),a
	masr	*(1234h),b
	masr	*ar5+,*ar3-,a
	masr	*ar5+,*ar3-,b
	masr	*ar5+,*ar3-,a,a
	masr	*ar5+,*ar3-,a,b
	masr	*ar5+,*ar3-,b,a
	masr	*ar5+,*ar3-,b,b

	masar	t,a
	masar	t,b
	masar	t,a,a
	masar	t,a,b
	masar	t,b,a
	masar	t,b,b

	dadd	*ar6+,a
	dadd	*ar6+,b
	dadd	*ar6+,a,a
	dadd	*ar6+,a,b
	dadd	*ar6+,b,a
	dadd	*ar6+,b,b
	dadd	*(1234h),a
	dadd	*(1234h),b
	dadd	*(1234h),a,a
	dadd	*(1234h),a,b
	dadd	*(1234h),b,a
	dadd	*(1234h),b,b

	and	*ar6+,a
	and	*ar6+,b
	and	*(1234h),a
	and	*(1234h),b
	and	#2345h,a
	and	#2345h,b
	and	#2345h,a,a
	and	#2345h,a,b
	and	#2345h,b,a
	and	#2345h,b,b
	and	#2345h,10,a
	and	#2345h,10,b
	and	#2345h,10,a,a
	and	#2345h,10,a,b
	and	#2345h,10,b,a
	and	#2345h,10,b,b
	and	#2345h,16,a
	and	#2345h,16,b
	and	#2345h,16,a,a
	and	#2345h,16,a,b
	and	#2345h,16,b,a
	and	#2345h,16,b,b
	and	a
	and	b
	and	a,a
	and	a,b
	and	b,a
	and	b,b
	and	a,7
	and	b,7
	and	a,7,a
	and	a,7,b
	and	b,7,a
	and	b,7,b
	and	a,-7
	and	b,-7
	and	a,-7,a
	and	a,-7,b
	and	b,-7,a
	and	b,-7,b

	or	*ar6+,a
	or	*ar6+,b
	or	*(1234h),a
	or	*(1234h),b
	or	#2345h,a
	or	#2345h,b
	or	#2345h,a,a
	or	#2345h,a,b
	or	#2345h,b,a
	or	#2345h,b,b
	or	#2345h,10,a
	or	#2345h,10,b
	or	#2345h,10,a,a
	or	#2345h,10,a,b
	or	#2345h,10,b,a
	or	#2345h,10,b,b
	or	#2345h,16,a
	or	#2345h,16,b
	or	#2345h,16,a,a
	or	#2345h,16,a,b
	or	#2345h,16,b,a
	or	#2345h,16,b,b
	or	a
	or	b
	or	a,a
	or	a,b
	or	b,a
	or	b,b
	or	a,7
	or	b,7
	or	a,7,a
	or	a,7,b
	or	b,7,a
	or	b,7,b
	or	a,-7
	or	b,-7
	or	a,-7,a
	or	a,-7,b
	or	b,-7,a
	or	b,-7,b

	xor	*ar6+,a
	xor	*ar6+,b
	xor	*(1234h),a
	xor	*(1234h),b
	xor	#2345h,a
	xor	#2345h,b
	xor	#2345h,a,a
	xor	#2345h,a,b
	xor	#2345h,b,a
	xor	#2345h,b,b
	xor	#2345h,10,a
	xor	#2345h,10,b
	xor	#2345h,10,a,a
	xor	#2345h,10,a,b
	xor	#2345h,10,b,a
	xor	#2345h,10,b,b
	xor	#2345h,16,a
	xor	#2345h,16,b
	xor	#2345h,16,a,a
	xor	#2345h,16,a,b
	xor	#2345h,16,b,a
	xor	#2345h,16,b,b
	xor	a
	xor	b
	xor	a,a
	xor	a,b
	xor	b,a
	xor	b,b
	xor	a,7
	xor	b,7
	xor	a,7,a
	xor	a,7,b
	xor	b,7,a
	xor	b,7,b
	xor	a,-7
	xor	b,-7
	xor	a,-7,a
	xor	a,-7,b
	xor	b,-7,a
	xor	b,-7,b

	sfta	a,7
	sfta	b,7
	sfta	a,7,a
	sfta	a,7,b
	sfta	b,7,a
	sfta	b,7,b
	sfta	a,-7
	sfta	b,-7
	sfta	a,-7,a
	sfta	a,-7,b
	sfta	b,-7,a
	sfta	b,-7,b

	sftl	a,7
	sftl	b,7
	sftl	a,7,a
	sftl	a,7,b
	sftl	b,7,a
	sftl	b,7,b
	sftl	a,-7
	sftl	b,-7
	sftl	a,-7,a
	sftl	a,-7,b
	sftl	b,-7,a
	sftl	b,-7,b

	firs	*ar3+,*ar4+,2345h

	bit	*ar5+,15-12

	bitf	*ar6,#2345h
	bitf	*(1234h),#2345h

	cmpr	2,ar4
	cmpr	gt,ar4

	b	1234h
	bd	1234h
	call	1234h
	calld	1234h
	rptb	1234h
	rptbd	1234h

	banz	1234h,*ar3-
	banzd	1234h,*ar3-

	bc	1234h,bio
	bc	1234h,nbio
	bc	1234h,c
	bc	1234h,nc
	bc	1234h,tc
	bc	1234h,ntc
	bc	1234h,aeq
	bc	1234h,aneq
	bc	1234h,agt
	bc	1234h,ageq
	bc	1234h,alt
	bc	1234h,aleq
	bc	1234h,aov
	bc	1234h,anov
	bc	1234h,beq
	bc	1234h,bneq
	bc	1234h,bgt
	bc	1234h,bgeq
	bc	1234h,blt
	bc	1234h,bleq
	bc	1234h,bov
	bc	1234h,bnov
	bc	1234h,unc
	bc	1234h,bio,c,tc
	bc	1234h,aneq,aov
	;bc	1234h,bio,aov
	;bc	1234h,unc,tc
	;bc	1234h,alt,bov
	;bc	1234h,gammel

	bcd	1234h,bio,c,tc

	cc	1234h,bio,c,tc

	ccd	1234h,bio,c,tc

	fb	123456h
	fbd	123456h
	fcall	123456h
	fcalld	123456h

	intr	5
	trap	10h

	rpt	*(1234h)
        nop

	rpt	#2
        nop

	rpt	#1111
        nop

	rptz	a,#1023
	nop

	rptz	b,#10
	nop

	frame	10h
	frame	-2

;	idle	0
	idle	1
	idle	2
	idle	3

sxm	equ	(1 << 4) | 8
	ssbx	sxm
	ssbx	1,8
	rsbx	sxm
	rsbx	1,8

	xc	1,bio
	xc	2,nbio
	xc	1,c
	xc	2,nc
	xc	1,tc
	xc	2,ntc
	xc	1,aeq
	xc	2,aneq
	xc	1,agt
	xc	2,ageq
	xc	1,alt
	xc	2,aleq
	xc	1,aov
	xc	2,anov
	xc	1,beq
	xc	2,bneq
	xc	1,bgt
	xc	2,bgeq
	xc	1,blt
	xc	2,bleq
	xc	1,bov
	xc	2,bnov
	xc	1,unc
	xc	2,bio,c,tc
	xc	1,aneq,aov

	ld	*ar6+,t
        ld      *(1234h),t

	ld	*ar6+,dp
	ld	*(1234h),dp
	ld	#23,dp

	ld	#3,arp

	ld	a,asm
	ld	b,asm
	ld	a,asm,a
	ld	a,asm,b
	ld	b,asm,a
	ld	b,asm,b
	ld	*ar6+,asm
	ld	*(1234h),asm
	ld	#15,asm

	ld	*ar6-,a
	ld	*(1234h),a
	ld	*ar6-,b
	ld	*(1234h),b
	ld	*ar6-,ts,a
	ld	*(1234h),ts,a
	ld	*ar6-,ts,b
	ld	*(1234h),ts,b
	ld	*ar6-,16,a
	ld	*(1234h),16,a
	ld	*ar6-,16,b
	ld	*(1234h),16,b
	ld	*ar6-,13,a
	ld	*(1234h),13,a
	ld	*ar6-,13,b
	ld	*(1234h),13,b
	ld	*ar6-,-13,a
	ld	*(1234h),-13,a
	ld	*ar6-,-13,b
	ld	*(1234h),-13,b
	ld	*ar2+,13,a
	ld	*ar2+,13,b

	ld	a,13,a
	ld	a,13,b
	ld	b,13,a
	ld	b,13,b
	ld	a,-13,a
	ld	a,-13,b
	ld	b,-13,a
	ld	b,-13,b

	ld	#77,a
	ld	#77,b
	ld	#2345h,a
	ld	#2345h,b
	ld	#2345h,16,a
	ld	#2345h,16,b
	ld	#2345h,13,a
	ld	#2345h,13,b

	ldm	6eh,a
	ldm	*ar2,b

	dst	a,*ar2+
	dst	b,*(1234h)

	st	-1,0
	st	trn,5
	st	t,*ar7-
	st	#1234h,*(2345h)
	st	a,*ar4+
	st	b,*ar4+

	sth	a,*ar6+
	sth	b,*ar6+
	sth	a,*(1234h)
	sth	b,*(1234h)
	sth	a,0,*ar6+
	sth	b,0,*ar6+
	sth	a,0,*(1234h)
	sth	b,0,*(1234h)
	sth	a,asm,*ar6+
	sth	b,asm,*ar6+
	sth	a,asm,*(1234h)
	sth	b,asm,*(1234h)
	sth	a,2,*ar5+
	sth	b,2,*ar5+
	sth	a,-2,*ar5+
	sth	b,-2,*ar5+
	sth	a,10,*ar6+
	sth	b,10,*ar6+
	sth	a,10,*(1234h)
	sth	b,10,*(1234h)
	sth	a,-10,*ar6+
	sth	b,-10,*ar6+
	sth	a,-10,*(1234h)
	sth	b,-10,*(1234h)

	stl	a,*ar6+
	stl	b,*ar6+
	stl	a,*(1234h)
	stl	b,*(1234h)
	stl	a,0,*ar6+
	stl	b,0,*ar6+
	stl	a,0,*(1234h)
	stl	b,0,*(1234h)
	stl	a,asm,*ar6+
	stl	b,asm,*ar6+
	stl	a,asm,*(1234h)
	stl	b,asm,*(1234h)
	stl	a,2,*ar5+
	stl	b,2,*ar5+
	stl	a,-2,*ar5+
	stl	b,-2,*ar5+
	stl	a,10,*ar6+
	stl	b,10,*ar6+
	stl	a,10,*(1234h)
	stl	b,10,*(1234h)
	stl	a,-10,*ar6+
	stl	b,-10,*ar6+
	stl	a,-10,*(1234h)
	stl	b,-10,*(1234h)

	stlm	a,6eh
	stlm	b,*ar1-

	stm	-1,6eh
	stm	8765h-65536,*ar7+

	cmps	a,*ar2+
	cmps	b,*(1234h)

	saccd	a,*ar3+0%,alt
	saccd	b,*ar4,beq

	srccd	*ar5-,agt
	strcd	*ar5-,agt

	mvdk	*ar3-,1000h
	mvdk	*(1234h),1000h

	mvdp	0,0fe00h
	mvdp	*(1234h),0fe00h

	mvkd	1000h,*+ar5
	mvkd	1000h,*(1234h)

	mvpd	0fe00h,5
	mvpd	0fe00h,*(1234h)
	mvpd	2000h,*ar7-0
	mvpd	2000h,*(1234h)

	mvdm	300h,69h
	mvmd	69h,8000h

	mvmm	18h,11h

	portw	5,3456h
	portw	*(1234h),3456h
	portr	3456h,5h
	portr	3456h,*(1234h)

	st	b,*ar2-
	||ld	*ar4+,a
	st	a,*ar3+
	||ld	*ar4+,t
	st	a,*ar2+
	||ld	*ar2-,a

	ld	*ar4+,a
	||mac	*ar5+,b

	ld	*ar4+,a
	||macr	*ar5+,b

	ld	*ar4+,a
	||mas	*ar5+,b

	ld	*ar4+,a
	||masr	*ar5+,b

	st	a,*ar3
	||add	*ar5+0%,b

	st	a,*ar3-
	||sub	*ar5+0%,b

	st	a,*ar4-
	||mac	*ar5,b

	st	a,*ar4+
	||macr	*ar5+,b

	st	a,*ar4+
	||mas	*ar5,b

	st	a,*ar4+
	||masr	*ar5+,b

	st	a,*ar3+
	||mpy	*ar5+,b
