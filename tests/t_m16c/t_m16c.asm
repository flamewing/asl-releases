        cpu     m16c
        page    0

        brk
        exitd
        into
        nop
        reit
        rts
        und
        wait

        rmpa
        smovb.b
        smovf.w
        sstr

lab:
        jgeu    lab
        jc      lab
        jgtu    lab
        jeq     lab
        jz      lab
        jn      lab
        jltu    lab
        jnc     lab
        jleu    lab
        jne     lab
        jnz     lab
        jpz     lab
        jle     lab
        jo      lab
        jge     lab
        jgt     lab
        jno     lab
        jlt     lab

        mov:g	[a1],r0l
        mov:g	[a1],r0h
        mov:g	[a1],r1l
        mov:g	[a1],r1h
        mov:g	[a1],r0
        mov:g	[a1],r1
        mov:g	[a1],r2
	mov:g	[a1],r3
        mov.w:g	[a1],a0
        mov.b:g	[a1],a1
        mov.w:g	[a1],[a0]
        mov.b:g	[a1],[a1]
        mov.w:g	[a1],10[a0]
        mov.b:g	[a1],10[a1]
        mov.w:g	[a1],20[sb]
        mov.b:g	[a1],20[fb]
        mov.w:g	[a1],300[a0]
        mov.b:g	[a1],300[a1]
        mov.w:g	[a1],400[sb]
        mov.w:g	[a1],1234h
        mov.w:g	#123,500[a1]

        mov	#3,r0
        mov.w	#-6,3[sb]

        mov.b	#0,r0l
        mov.b	#0,r0h
        mov.b	#0,5[sb]
        mov.b	#0,10[fb]
        mov.b	#0,1234h

        mov	r2,5[sp]
        mov.b	-12[sp],12[sb]

        mov.w	#10,a0
	mov.b	#12,a1

        mov.b	#3,r0l
        mov.b	#100,r0h
        mov.b	#39,5[sb]
        mov.b	#45h,10[fb]
        mov.b	#4,1234h

        mov.b	r0l,a1
        mov.b	r0h,a1          ; !!!! ergibt G-Modus
        mov.b	5[sb],a1
        mov.b	10[fb],a1
        mov.b	1234h,a1

        mov.b	r0l,r0h
        mov.b	r0h,r0h         ; !!!! ergibt G-Modus
        mov.b	5[sb],r0h
        mov.b	10[fb],r0h
        mov.b	1234h,r0h

        mov.b	r0h,r0l
        mov.b	r0h,r0h         ; !!!! ergibt G-Modus
        mov.b	r0h,5[sb]
        mov.b	r0h,10[fb]
        mov.b	r0h,1234h


        add.b	#100,sp
        add.b	#4,sp
        add.w	#1000,sp
        add.w	#4,sp
	add	#-5,sp

        add.w	#7,100[a0]
        add.b	#8,100[a0]
        add     #100,r0h
        add     r0l,r0l
        add     1234h,r0l
        add     r0h,r0l
        add     [a1],r3

        cmp.w	#7,100[a0]
        cmp.b	#8,100[a0]
        cmp     #100,r0h
        cmp     r0l,r0l
        cmp     1234h,r0l
        cmp     r0h,r0l
        cmp     [a1],r3

        sub.w	#7,100[a0]
        sub.b	#8,100[a0]
        sub.w   #9,100[a0]
        sub     #100,r0h
        sub     r0l,r0l
        sub     1234h,r0l
        sub     r0h,r0l
        sub     [a1],r3

        abs	r2
        abs.b	[a1]
	adcf	r1l
	adcf.w	10[sb]
	neg	r1
	neg.b	1234h
	rolc	r1h
	rorc	r3

        adc	r3,r0
        sbb.w	[a1],2[a1]
        tst.w	#20,a1
        xor.w	#1234h,5678h
        mul	r0h,r0l
        mulu.b  a1,a0

        dec	r0h
        dec.b	[sb]
	dec.w	a0
	inc	r0l
	inc.b	3[fb]
	inc.w	a1

        div.w	#1234h
        div	r2
        divu.w	#1234h
        divu.w	10[a0]
	divx.w	#1234h
	divx.w	3456h

        dadc	#12,r0l
        dadc	#1234,r0
        dadc	r0h,r0l
        dadc	r1,r0

        dadd	#12,r0l
        dadd	#1234,r0
        dadd	r0h,r0l
        dadd	r1,r0

        dsbb	#12,r0l
        dsbb	#1234,r0
        dsbb	r0h,r0l
        dsbb	r1,r0

        dsub	#12,r0l
        dsub	#1234,r0
        dsub	r0h,r0l
        dsub	r1,r0

        exts	r1l
        exts.b	1000[a0]
        exts	r0

;        ****

        rot	#5,r2
        rot.b	#-3,1000[a1]
        rot	r1h,r2
        rot.b	r1h,100[a1]

        sha	#4,r1l
	sha.w	#-4,[a0]
	sha	#1,r3r1
        sha	r1h,r1l
	sha.w	r1h,[a0]
	sha	r1h,r3r1

        shl	#4,r1l
	shl.w	#-4,[a0]
	shl	#1,r3r1
        shl	r1h,r1l
	shl.w	r1h,[a0]
	shl	r1h,r3r1

        ldc	#2,intbl
        ldc	#20,intbh
        ldc	#200,flg
        ldc	#2000,isp
        ldc	#20000,sp
        ldc	r3,sb
        stc	fb,[a0]
        stc	pc,a1a0
        stc	pc,r3r1
        stc	pc,r2r0

        ldctx   1234h,56789h
        stctx   1234h,56789h

        lde     12345h,r3
        lde.b   10000[a0],10[a1]
        lde.b   [a1a0],10[a0]

        ste     r3,12345h
        ste.b   10[a1],10000[a0]
        ste.b   10[a0],[a1a0]

	mova	12[a0],a0
        mova	1000[a1],r3

        movll	r0l,r1h
        movlh	r0l,[a1]
        movhl	1[a1],r0l
        movhh	r0l,r0l

        pop.b	[a1]
	pop.w	1234h
	pop	r0h
	pop.w	a1

        push.b	[a1]
	push.w	1234h
	push	r0h
	push.w	a1
        push.b	#5
        push.w	#1000

        pushc	sp
        popc	fb

        pushm	r2,a1,sb
        popm	r1,a0,fb

        pusha	200[a1]

        xchg	r2,1234h
        xchg	1000[sb],r1l

        stnz	#10,r0l
        stz	#30,10[fb]

        stzx	#40,#50,1234h

        adjnz.w	#1,a1,$
        sbjnz.b	#1,1234h,$

        jmp	$+2
        jmp.b	$+2
        jmp.w	$+2
        jmp.a	$+2
        jmp	$+20
        jmp.w	$+20
        jmp.a	$+20
        jmp	$+200
        jmp.a	$+200
        jmp	12345h

        jsr	$+200
        jsr.a	$+200
        jsr	12345h

        jmpi	r0
        jmpi	a0
        jmpi	a1a0
        jmpi	r3r1
        jmpi.w	[a0]
        jmpi.a	[a0]
        jmpi.w	10[a0]
        jmpi.w	1000[a0]

        jsri	r0
        jsri	a0
        jsri	a1a0
        jsri	r3r1
        jsri.w	[a0]
        jsri.a	[a0]
        jsri.w	10[a0]
        jsri.w	1000[a0]

        jmps	#30
        jsrs	#20

        band	7,r0
        band	12,r1
        band	6,r2
        band	1,r3
        band	15,a0
        band	0,a1
        band	[a0]
        band	[a1]
        band	100[a0]
        band	100[a1]
        band	6,20[sb]
        band	2,15[fb]
        band	3,-4[fb]
        band	1000[a0]
        band	1000[a1]
        band	6,1000[sb]
        band	4,1234h

        bclr	[a1]
        bclr	2,5[sb]

        bnand	[a1]
        bnand	2,5[sb]

        bnor	[a1]
        bnor	2,5[sb]

        bnot	[a1]
        bnot	2,5[sb]

        bntst	[a1]
        bntst	2,5[sb]

        bnxor	[a1]
        bnxor	2,5[sb]

        bor	[a1]
        bor	2,5[sb]

        bset	[a1]
        bset	2,5[sb]

        btst	[a1]
        btst	2,5[sb]

        btstc	[a1]
        btstc	2,5[sb]

        btsts	[a1]
        btsts	2,5[sb]

        bxor	[a1]
        bxor	2,5[sb]

        bmgtu	[a1]
        bmle	[a1]
        bmltu	[a1]
        bmno	[a1]

        bmgtu	c
        bmle	c
        bmltu	c
        bmno	c

        fclr	b
        fset	z

        ldintb	#12345h

        ldipl	#2

        int	#30h

        enter	#33