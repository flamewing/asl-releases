	cpu	68hc12
	page	0

	aba

	abx

	aby

	adca	#45
        adca	$45
        adca	$1234
        adca	>$45
        adca	,y
        adca	5,x
        adca	-5,sp
        adca	*,pc
        adca	*+10,pc
        adca	*+17,pc
        adca	a,y
        adca	b,sp
        adca	d,pc
        adca	55,y
        adca	-55,sp
        adca	<5,y
        adca	<-5,sp
        adca	*+18,pc
        adca	*+258,pc
        adca	555,y
        adca	-555,sp
	adca	>55,y
        adca	>-55,sp
        adca	*+259,pc
        adca	[d,sp]
        adca	[d,x]
	adca	[-5,y]
        adca	[500,y]
        adca	4,+y
        adca	4,y+
        adca	4,-y
        adca	4,y-

        adcb	#55
        adcb	$55
        adcb	,x
        adcb	$1234

        adda	#55
        adda	$55
        adda	,x
        adda	$1234

        addb	#55
        addb	$55
        addb	,x
        addb	$1234

        addd	#55
        addd	$55
        addd	,x
        addd	$1234

        anda	#55
        anda	$55
        anda	,x
        anda	$1234

        andb	#55
        andb	$55
        andb	,x
        andb	$1234

        andcc	#$fe

        asl	$55
        asl	,x
        asl	$1234

        asla

        aslb

        asld

        asr	$55
        asr	,x
        asr	$1234

        asra

        asrb

        bcc	*

        bclr	$20 #$40
	bclr	$20,#$40
	bclr	$1234 #$40
	bclr	$1234,#$40
	bclr	,x $40
	bclr	$20,y,$40
        bclr	*,pc,$40

        bcs	*

        beq	*

        bge	*

        bgnd

        bgt	*

        bhi	*

        bhs	*

        bita	#$55
        bita	$55
        bita	,x
        bita	$1234

        bitb	#$55
        bitb	$55
        bitb	,x
        bitb	$1234

        ble	*

        blo	*

        bls	*

        blt	*

        bmi	*

        bne	*

        bpl	*

        bra	*

        brclr	$20 #$40 *
        brclr	$2000,#$40,*
        brclr	,x,#$40,*
        brclr	*,pc,#$40,*

        brn	*

        brset	$20 #$40 *
        brset	$2000,#$40,*
        brset	,x,#$40,*
        brset	*,pc,#$40,*

        bset	$20 #$40
	bset	$20,#$40
	bset	$1234 #$40
	bset	$1234,#$40
	bset	,x $40
	bset	$20,y,$40

        bsr	*

        bvc	*

        bvs	*

        call	$2000,5
        call	5,y,6
        call	200,y,7
        call	20000,y,8
        call	[d,y]
        call	[20,y]

        cba

        clc
        andcc	#$fe

        cli
        andcc	#$ef

        clr	$55
        clr	,x
        clr	$1234

        clra

        clrb

        clv
        andcc	#$fd

        cmpa	#55
        cmpa	$55
        cmpa	,x
        cmpa	$1234

        cmpb	#55
        cmpb	$55
        cmpb	,x
        cmpb	$1234

        com	$55
        com	,x
        com	$1234

        coma

        comb

        cpd	#55
        cpd	$55
        cpd	,x
        cpd	$1234

        cps	#55
        cps	$55
        cps	,x
        cps	$1234

        cpx	#55
        cpx	$55
        cpx	,x
        cpx	$1234

        cpy	#55
        cpy	$55
        cpy	,x
        cpy	$1234

        daa

        dbeq	x,*

        dbne	x,*

        dec	$55
        dec	,x
        dec	$1234

        deca

        decb

        des
        leas	-1,sp

        dex

        dey

        ediv

        edivs

        emacs	$1234

        emaxd	,x

        emaxm	,x

        emind	,x

        eminm	,x

        emul

        emuls

        eora	#55
        eora	$55
        eora	,x
        eora	$1234

        eorb	#55
        eorb	$55
        eorb	,x
        eorb	$1234

        etbl	5,y

        exg	ccr,sp

        fdiv

        ibeq	a,*

        ibne	a,*

        idiv

        idivs

        inc	$55
        inc	,x
        inc	$1234

        inca

        incb

        ins
        leas	1,sp

        inx

        iny

	jmp	$2000
	jmp	$20
        jmp	[d,x]

	jsr	$2000
	jsr	$20
        jsr	[d,x]

        lbcc	*

        lbcs	*

        lbeq	*

        lbge	*

        lbgt	*

        lbhi	*

        lbhs	*

        lble	*

        lblo	*

        lbls	*

        lblt	*

        lbmi	*

        lbne	*

        lbpl	*

        lbra	*

        lbrn	*

        lbvc	*

        lbvs	*

        ldaa	#55
        ldaa	$55
        ldaa	,x
        ldaa	$1234

        ldab	#55
        ldab	$55
        ldab	,x
        ldab	$1234

        ldd	#55
        ldd	$55
        ldd	,x
        ldd	$1234

        lds	#55
        lds	$55
        lds	,x
        lds	$1234

        ldx	#55
        ldx	$55
        ldx	,x
        ldx	$1234

        ldy	#55
        ldy	$55
        ldy	,x
        ldy	$1234

        leas	2000,sp

        leax	,y

        leay	,x

        lsl	$55
        lsl	,x
        lsl	$1234

        lsla

        lslb

        lsld

        lsr	$55
        lsr	,x
        lsr	$1234

        lsra

        lsrb

        lsrd

        maxa	,x

        maxm	,x

        mem

        mina	,x

        minm	,x

        movb	#$55 $1234
        movb	#$55,$1234
        movb	#$55 2,y
        movb	#$55,2,y
        movb	#$55,next_a,pc
next_a: movb	$1234 $3456
	movb	$1234,$3456
        movb	$1234 2,y
        movb	$1234,2,y
        movb	$1234,next_b,pc
next_b: movb	2,y $1234
	movb	2,y,$1234
        movb	next_c,pc,$1234
next_c: movb	2,y 2,y
	movb	2,y,2,y
        movb	next_d,pc next_d,pc
next_d:
        movw	#$55 $1234
        movw	#$55,$1234
        movw	#$55 2,y
        movw	#$55,2,y
        movw	#$55,next_e,pc
next_e: movw	$1234 $3456
	movw	$1234,$3456
        movw	$1234 2,y
        movw	$1234,2,y
        movw	$1234,next_f,pc
next_f: movw	2,y $1234
	movw	2,y,$1234
        movw	next_g,pc,$1234
next_g: movw	2,y 2,y
	movw	2,y,2,y
        movw	next_h,pc next_h,pc
next_h:

	mul

        neg	$55
        neg	,x
        neg	$1234

        nega

        negb

        nop

        oraa	#55
        oraa	$55
        oraa	,x
        oraa	$1234

        orab	#55
        orab	$55
        orab	,x
        orab	$1234

        orcc    #$10

        psha

        pshb

        pshc

        pshd

        pshx

        pshy

        pula

        pulb

        pulc

        puld

        pulx

        puly

        rev

        revw

        rol	$55
        rol	,x
        rol	$1234

        rola

        rolb

        ror	$55
        ror	,x
        ror	$1234

        rora

        rorb

        rtc

        rti

        rts

        sba

        sbca	#55
        sbca	$55
        sbca	,x
        sbca	$1234

        sbcb	#55
        sbcb	$55
        sbcb	,x
        sbcb	$1234

        sec
        orcc	#$01

        sei
        orcc	#$10

        sev
        orcc	#$02

	sex	a,d

        staa	$55
        staa	,x
        staa	$1234

        stab	$55
        stab	,x
        stab	$1234

        std	$55
        std	,x
        std	$1234

        sts	$55
        sts	,x
        sts	$1234

        stx	$55
        stx	,x
        stx	$1234

        sty	$55
        sty	,x
        sty	$1234

        stop

        suba	#55
        suba	$55
        suba	,x
        suba	$1234

        subb	#55
        subb	$55
        subb	,x
        subb	$1234

        subd	#55
        subd	$55
        subd	,x
        subd	$1234

        swi

        tab

        tap
        tfr	a,ccr

        tba

        tbeq	d,*

        tbl	a,x

        tbne	d,*

        tfr	a,b
        tfr	x,y

        tpa
        tfr	ccr,a

        trap	#$42

        tst	$55
        tst	,x
        tst	$1234

        tsta

        tstb

	tsx
        tfr	sp,x

        tsy
        tfr	sp,y

        txs
        tfr	x,sp

        tys
        tfr	y,sp

        wai

        wav

        xgdx
        exg	d,x

        xgdy
        exg	d,y
