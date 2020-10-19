	cpu	hd6413309
	maxmode	on
	page	0
        relaxed on

        dc.w	1234
        dc.w	$4d2
        dc.w	@2322
        dc.w	%10011010010
        dc.w	4d2h
        dc.w	2322o
        dc.w	10011010010b
        dc.w	0x4d2
	dc.w	02322

	nop
	sleep
	rts
	rte

	eepmov
	eepmov.b
	eepmov.w

	dec	r2h
	dec	#1,r5l
	dec	r4
	dec	#1,e6
	dec	#2,e1
	dec	er1
	dec	#1,er4
	dec	#2,er7
	inc	r2h
	inc	#1,r5l
	inc	r4
	inc	#1,e6
	inc	#2,e1
	inc	er1
	inc	#1,er4
	inc	#2,er7

targ:   bsr	targ
	bsr.s	targ
	bsr.l	targ
	bra	targ
	bra.s	targ
	bra.l	targ
	bt	targ
	bt.s	targ
	bt.l	targ
	brn	targ
	brn.s	targ
	brn.l	targ
	bf	targ
	bf.s	targ
	bf.l	targ
	bhi	targ
	bhi.s	targ
	bhi.l	targ
	bls	targ
	bls.s	targ
	bls.l	targ
	bcc	targ
	bcc.s	targ
	bcc.l	targ
	bhs	targ
	bhs.s	targ
	bhs.l	targ
	bcs	targ
	bcs.s	targ
	bcs.l	targ
	blo	targ
	blo.s	targ
	blo.l	targ
	bne	targ
	bne.s	targ
	bne.l	targ
	beq	targ
	beq.l	targ
	bvc	targ
	bvc.l	targ
	bvs	targ
	bvs.l	targ
	bpl	targ
	bpl.l	targ
	bmi	targ
	bmi.l	targ
	bge	targ
	bge.l	targ
	blt	targ
	blt.l	targ
	bgt	targ
	bgt.l	targ
	ble	targ
	ble.l	targ

	rotl	r4h
	rotl	r2
	rotl	er1
	rotr	r2l
	rotr	r1
	rotr	er5
	rotxl	r2h
	rotxl	e4
	rotxl	er2
	rotxr	r6h
	rotxr	e0
	rotxr	er7
	shal	r3l
	shal	r6
	shal	er0
	shar	r5l
	shar	r2
	shar	er6
	shll	r2h
	shll	e4
	shll	er5
	shlr	r1l
	shlr	r7
	shlr	er4

	not	r7h
	not	r6
	not	er2
	neg	r2l
	neg	e2
	neg	er5

	exts	r5
	extu	e3
	exts	er4
	extu	er6

        and	r5h,r3l
        and	#10,r2h
        and	r7,e1
        and	#%1101,r1
        and	er2,er6
        and	#$12345678,er3
        andc	#$20,ccr
        or	r6h,r4l
        or	#20,r3h
        or	r0,e2
        or	#%11101,r2
        or	er2,er7
        or	#$12345678,er4
        orc	#$30,ccr
        xor	r4h,r2l
        xor     #$08,r1h
        xor	r6,e0
        xor	#%101101,r0
        xor	er1,er5
        xor	#$12345678,er2
        xorc	#$30,ccr

        daa	r1l
        das	r5h

        addx	#10,r5l
        addx	r1h,r1l
        subx	#$55,r7h
        subx	r3h,r0l

        adds	#1,er0
        adds	#2,er4
        adds	#4,er5
        subs	#1,er6
        subs	#2,er3
        subs	#4,er1

        divxs	r4h,e2
        divxs	r4,er5
        divxu	r1l,r3
        divxu	e6,er7
        mulxs	r0l,e6
        mulxs	r5,er2
        mulxu	r7h,r5
        mulxu	e3,er4

	add	r1h,r2l
	add	#$34,r6h
	add	r2,e3
	add	#%10101010101010,e5
	add     er3,er1
	add	#1000000,er4
	sub	r1l,r2h
	sub	r6,e1
	sub	#%10101010101010,r2
	sub     er1,er5
	sub	#1000000,er6
	cmp	r1h,r2l
	cmp	#$34,r6h
	cmp	r2,e3
	cmp	#%10101010101010,e5
	cmp     er3,er1
	cmp	#1000000,er4


	pop	r5
	push	e2
	pop	er1
	push	er6

	mov	r2l,r5h
	mov	r1,e2
	mov	er5,er2

	mov.b	@er4,r6h
	mov.b	r6h,@er4
	mov.w	@er1,e7
	mov.w	e7,@er1
	mov.l	@er5,er2
	mov.l	er2,@er5

	mov.b	@er2+,r5l
	mov.b	r5l,@-er2
	mov.w	@er5+,r4
	mov.w	r4,@-er5
	mov.l	@er6+,er1
	mov.l	er1,@-er6

	mov.b	@(-100,er2),r4l
	mov.b	r4l,@(-100,er2)
	mov.w	@(200,er4),e3
	mov.w	e3,@(200,er4)
	mov.l	@(-300,sp),er5
	mov.l	er5,@(-300,sp)

	mov.b	@(-100000,er4),r3h
	mov.b	r3h,@(-100000,er4)
	mov.w	@(200000,er2),r6
	mov.w	r6,@(200000,er2)
	mov.l	@(-300000,er5),er1
	mov.l	er1,@(-300000,er5)

	mov.b	$ffff20,r1h
	mov.b	r1h,$ffff20
	mov.w	$ffffa4,e6
	mov.w	e6,$ffffa4
	mov.l	$ffffc0,er3
	mov.l	er3,$ffffc0

	mov.b	$1234,r3h
	mov.b	r3h,$1234
	mov.w	$2345,e5
	mov.w	e5,$2345
	mov.l	$3456,er4
	mov.l	er4,$3456

	mov.b	$123456,r3l
	mov.b	r3l,$123456
	mov.w	$234567,r5
	mov.w	r5,$234567
	mov.l	$345678,er7
	mov.l	er7,$345678

	mov.b	#$12,r4l
	mov.w	#$1234,e2
        mov.l   #$12345678,er3

	movfpe	@1234,r4l
	movtpe	r4l,@1234

	band	#4,r2l
	band	#2,@er3
	band	#6,$ffff4e
	biand	#4,r2l
	biand	#2,@er3
	biand	#6,$ffff4e
	bild	#4,r2l
	bild	#2,@er3
	bild	#6,$ffff4e
	bior	#4,r2l
	bior	#2,@er3
	bior	#6,$ffff4e
	bist	#4,r2l
	bist	#2,@er3
	bist	#6,$ffff4e
	bixor	#4,r2l
	bixor	#2,@er3
	bixor	#6,$ffff4e
	bld	#4,r2l
	bld	#2,@er3
	bld	#6,$ffff4e
	bor	#4,r2l
	bor	#2,@er3
	bor	#6,$ffff4e
	bst	#4,r2l
	bst	#2,@er3
	bst	#6,$ffff4e
	bxor	#4,r2l
	bxor	#2,@er3
	bxor	#6,$ffff4e

	bclr	#3,r5h
	bclr	#7,@er6
	bclr	#2,$ffff1a
	bclr	r5l,r6h
	bclr	r1h,@er4
	bclr	r7h,$ffff24
	bnot	#3,r5h
	bnot	#7,@er6
	bnot	#2,$ffff1a
	bnot	r5l,r6h
	bnot	r1h,@er4
	bnot	r7h,$ffff24
	bset	#3,r5h
	bset	#7,@er6
	bset	#2,$ffff1a
	bset	r5l,r6h
	bset	r1h,@er4
	bset	r7h,$ffff24
	btst	#3,r5h
	btst	#7,@er6
	btst	#2,$ffff1a
	btst	r5l,r6h
	btst	r1h,@er4
	btst	r7h,$ffff24

	jmp	@er2
	jmp	$123456
	jmp	@@$35
	jsr	@er2
	jsr	$123456
	jsr	@@$35

	ldc	#23,ccr
	ldc	r5h,ccr
	stc	ccr,r5h
	ldc	@er6,ccr
	stc	ccr,@er6
	ldc	@er3+,ccr
	stc	ccr,@-er3
	ldc	@(100,er1),ccr
	stc	ccr,@(100,er1)
	ldc	@(100000,er5),ccr
	stc	ccr,@(100000,er5)
	ldc	$1234,ccr
	stc	ccr,$1234
	ldc	$123456,ccr
	stc	ccr,$123456

; register aliases

regr0l	equ	r0l
regr1l	equ	r1l
regr2l	equ	r2l
regr3l	equ	r3l
regr4l	equ	r4l
regr5l	equ	r5l
regr6l	equ	r6l
regr7l	equ	r7l
regr0h	equ	r0h
regr1h	equ	r1h
regr2h	equ	r2h
regr3h	equ	r3h
regr4h	equ	r4h
regr5h	equ	r5h
regr6h	equ	r6h
regr7h	equ	r7h

regr0	reg	r0
regr1	reg	r1
regr2	reg	r2
regr3	reg	r3
regr4	reg	r4
regr5	reg	r5
regr6	reg	r6
regr7	reg	r7
rege0	reg	e0
rege1	reg	e1
rege2	reg	e2
rege3	reg	e3
rege4	reg	e4
rege5	reg	e5
rege6	reg	e6
rege7	reg	e7

reger0	equ	er0
reger1	equ	er1
reger2	equ	er2
reger3	equ	er3
reger4	equ	er4
reger5	equ	er5
reger6	equ	er6
reger7	equ	er7

regsp	reg	sp

	; register as plain operand

	addx	#10,r0l
	addx	#10,regr0l
	addx	#10,r1l
	addx	#10,regr1l
	addx	#10,r2l
	addx	#10,regr2l
	addx	#10,r3l
	addx	#10,regr3l
	addx	#10,r4l
	addx	#10,regr4l
	addx	#10,r5l
	addx	#10,regr5l
	addx	#10,r6l
	addx	#10,regr6l
	addx	#10,r7l
	addx	#10,regr7l
	addx	#10,r0h
	addx	#10,regr0h
	addx	#10,r1h
	addx	#10,regr1h
	addx	#10,r2h
	addx	#10,regr2h
	addx	#10,r3h
	addx	#10,regr3h
	addx	#10,r4h
	addx	#10,regr4h
	addx	#10,r5h
	addx	#10,regr5h
	addx	#10,r6h
	addx	#10,regr6h
	addx	#10,r7h
	addx	#10,regr7h

	dec	r0
	dec	regr0
	dec	r1
	dec	regr1
	dec	r2
	dec	regr2
	dec	r3
	dec	regr3
	dec	r4
	dec	regr4
	dec	r5
	dec	regr5
	dec	r6
	dec	regr6
	dec	r7
	dec	regr7
	dec	e0
	dec	rege0
	dec	e1
	dec	rege1
	dec	e2
	dec	rege2
	dec	e3
	dec	rege3
	dec	e4
	dec	rege4
	dec	e5
	dec	rege5
	dec	e6
	dec	rege6
	dec	e7
	dec	rege7

	dec	er0
	dec	reger0
	dec	er1
	dec	reger1
	dec	er2
	dec	reger2
	dec	er3
	dec	reger3
	dec	er4
	dec	reger4
	dec	er5
	dec	reger5
	dec	er6
	dec	reger6
	dec	er7
	dec	reger7

	; register in addressing mode

	mov.b	@er4,r6h
	mov.b	@reger4,regr6h
	mov.b	@er2+,r6h
	mov.b	@reger2+,regr6h
	mov.b	r6h,@-er5
	mov.b	r6h,@-reger5
	mov.l	@(-300,sp),er5
	mov.l	@(-300,regsp),reger5

	; register as bit number

	btst	r5l,r6h
	btst	regr5l,regr6h

	dc	20

	dc.b	5,"Hallo"
	dc.w	1,2,3,4
	dc.l	1,2,3,4
;	dc.q	1,2,3,4		; omit for the sake of non-64-bit-platforms...
	dc.s	1,2,3,4
	dc.d	1,2,3,4
	dc.x	1,2,3,4
	dc.p	1,2,3,4

	; The very special hex syntax, which requires a quote qualify
	; callback in the parser and which is only enabled on request.
	; We optionally also allow a terminating ':

	relaxed	on

	mov	#$aa55  ,r2  ; comment
	mov	#h'aa55 ,r2  ; comment
	mov	#h'aa55',r2  ; comment
	mov	r2 ,@$ffe0   ; comment
	mov	r2 ,@h'ffe0  ; comment
	mov	r2 ,@h'ffe0' ; comment

	; bit symbols

	band	#7,$ffffe0
bit1	bit	#7,$ffffe0
	band	bit1

bitstruct	struct
byte1		ds.b	1
byte2		ds.b	2
lsb8		bit	0,byte1
msb8		bit	#7,byte2
		endstruct

		btst	mystruct_msb8
		btst	mystruct_lsb8

		org	$ffffc0
mystruct	bitstruct
