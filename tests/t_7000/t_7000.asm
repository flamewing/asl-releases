	cpu	sh7600
	page 	0
	supmode	on

	rts
	rte
	rtb
	clrt
	sett
	clrmac
	nop
	sleep
	div0u
	brk

	movt	r4
	cmp/pz	r6
	cmp/pl	r12
	rotl	r5
	rotr	sp
	rotcl	r8
	rotcr	r2
	shal	r1
	shar	r13
	shll	r9
	shlr	r11
	shll2	r4
	shlr2	r10
	shll8	r15
	shlr8	r3
	shll16	r5
	shlr16	r7
	stbr	r5
	ldbr	sp
        dt	r6

	tas	@sp

	xtrct	r0,r1
	addc	r1,r2
	addv	r2,r3
	cmp/hs	r3,r4
	cmp/ge	r4,r5
	cmp/hi	r5,r6
	cmp/gt	r6,r7
	cmp/str	r7,r8
	div1	r8,r9
	div0s	r9,r10
	muls.w	r10,r11
	mulu.w	r11,r12
	neg	r12,r13
	negc	r13,r14
	sub	r14,r15
	subc	r15,r0
	subv	r0,r1
	not	r1,r2
        dmuls.l	r2,r3
        dmulu.l	r3,r4
        mul	r4,r5

	swap.b	r5,r10
	swap.w	r6,r11
	exts.b	r7,r12
	exts.w	r8,r13
	extu.b	r9,r14
	extu.w	r10,r15

	add	r3,r5
	add	#-30,r4
	cmp/eq	r3,r5
	cmp/eq	#-30,r0
	mac	@r5+,@r7+
        mac.l	@r5+,@r7+

	and	r5,r9
	and	#30,r0
	and.b	#$55,@(r0,gbr)

	or	r7,sp
	or	#%10111011,r0
	or	#123,@(gbr,r0,3-5+2)

	tst	r4,r12
	tst.l	#20,r0
	tst.b	#33,@(r0,gbr)

	xor	r5,r3
	xor.l	#45,r0
	xor	#%10,@(gbr,r0)


	bt	targ
targ:	bf	targ
	bt/s	targ
        bf/s	targ
	bra	targ
	bsr	targ
	jmp	@r5
	jsr	@r10

	ldc	r5,sr
	ldc	r10,gbr
	ldc	r7,vbr
	ldc	@r6+,sr
	ldc	@r9+,gbr
	ldc	@r13+,vbr
	stc	sr,r4
	stc	gbr,r3
	stc	vbr,r7
	stc	sr,@-r6
	stc	gbr,@-r4
	stc	vbr,@-r14
	lds	r5,mach
	lds	r10,macl
	lds	r7,pr
	lds	@r6+,mach
	lds	@r9+,macl
	lds	@r13+,pr
	sts	mach,r4
	sts	macl,r3
	sts	pr,r7
	sts	mach,@-r6
	sts	macl,@-r4
	sts	pr,@-r14

        mova    ldata,r0

	mov.l	r5,r13

	mov.b	r1,@-r2
	mov.b	@r2+,r1
	mov.w	r3,@-r4
	mov.w	@r4+,r3
	mov.l	r6,@-r5
	mov.l	@r5+,r6

	mov.b	r12,@r5
	mov.b	@r5,r12
	mov.w	r5,@r14
	mov.w	@r14,r5
	mov.l	r7,@r7
	mov.l	@r7,r7

	mov.b	r0,@(r6,3)
	mov.b	@(r6,1+2),r0
	mov.w	r0,@(10,r12)
	mov.w	@(10,r12),r0
	mov.l	r7,@(60,r3)
	mov.l	@(2*30,r3),r7

	mov.b	r4,@(r0,r5)
	mov.b	@(r5,r0),r4
	mov.w	r6,@(r0,r10)
	mov.w	@(r10,r0),r6
	mov.l	r7,@(r0,r6)
	mov.l	@(r6,r0),r7

	mov.b	r0,@(gbr,30)
	mov.b	@(30,gbr),r0
	mov.w	r0,@(gbr,60)
	mov.w	@(10,5*5*2,gbr),r0
	mov.l	r0,@(gbr,120)
	mov.l	@(120,gbr),r0

	mov	#120,r4
	mov.w	wdata,r3
	mov.l	ldata,r12

	trapa	#$21

	mov	#$1234,r3
	mov	#$8000,r3
	mov	#$11223344,r3
	mov	#$0000,r3
	mov	#$1122,r3
	mov	#$3344,r3

        bra	next
        mov	#$1234,r3
        bra	next
        mov	#$11223344,r3
        nop
next:	nop
	nop

	section test
	mov	#$1234,r5
        nop
        ltorg
	endsection

	align	4
ldata:	dc.l	$12345678
wdata:	dc.w	$1234

	ltorg

        cpu	sh7700

        clrs
        sets
        ldtlb

        ldc	r4,ssr
        ldc	r4,spc
        ldc	r4,r0_bank
        ldc	r4,r1_bank
        ldc	r4,r2_bank
        ldc	r4,r3_bank
        ldc	r4,r4_bank
        ldc	r4,r5_bank
        ldc	r4,r6_bank
        ldc	r4,r7_bank
        ldc.l	@r4+,ssr
        ldc.l	@r4+,spc
        ldc.l	@r4+,r0_bank
        ldc.l	@r4+,r1_bank
        ldc.l	@r4+,r2_bank
        ldc.l	@r4+,r3_bank
        ldc.l	@r4+,r4_bank
        ldc.l	@r4+,r5_bank
        ldc.l	@r4+,r6_bank
        ldc.l	@r4+,r7_bank
        stc	ssr,r4
        stc	spc,r4
        stc	r0_bank,r4
        stc	r1_bank,r4
        stc	r2_bank,r4
        stc	r3_bank,r4
        stc	r4_bank,r4
        stc	r5_bank,r4
        stc	r6_bank,r4
        stc	r7_bank,r4
        stc.l	ssr,@-r4
        stc.l	spc,@-r4
        stc.l	r0_bank,@-r4
        stc.l	r1_bank,@-r4
        stc.l	r2_bank,@-r4
        stc.l	r3_bank,@-r4
        stc.l	r4_bank,@-r4
        stc.l	r5_bank,@-r4
        stc.l	r6_bank,@-r4
        stc.l	r7_bank,@-r4

        pref	@r5

        shad	r3,r7
        shld	r12,sp

	dsp	on

	irp	reg,dsr,a0,x0,x1,y0,y1
	lds	r7,reg
	lds	@r9+,reg
	endm

	irp	reg,dsr,a0,x0,x1,y0,y1
	sts	reg,r7
	sts	reg,@-r9
	endm
