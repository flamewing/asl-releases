	cpu	mcore
	page	0
	supmode	on

	bkpt
	doze
	rfi
	rte
	stop
	sync
	wait

	abs	r0
        asrc	r1
        brev	r2
        clrf	r3
        clrt	r4
        decf	r5
        decgt	r6
        declt	r7
        decne	r8
        dect	r9
        divs	r10
        divu	r11
        ff1	r12
        incf	r13
        inct	r14
        jmp	r15
        jsr	r0
        lslc	r1
        lsrc	r2
        mvc	r3
        mvcv	r4
        not	r5
        sextb	r6
        sexth	r7
        tstnbz	r8
        xsr	r9
        xtrb0	r10
        xtrb1	r11
        xtrb2	r12
        xtrb3	r13
        zextb	r14
        zexth	r15

        addc	r1,r2
        addu	r2,r3
        and	r3,r4
        andn	r4,r5
        asr	r5,r6
        bgenr	r6,r7
        cmphs	r7,r8
        cmplt	r8,r9
        cmpne	r9,r10
        ixh	r10,r11
        ixw	r11,r12
        lsl	r12,r13
        lsr	r13,r14
        mov	r14,r15
        movf	r1,r2
        movt	r2,r3
        mult	r3,r4
        or	r4,r5
        rsub	r5,r6
        subc	r6,r7
        subu	r7,r8
        tst	r8,r9
        xor	r9,r10

	addi	r2,1
        andi	r3,2
        asri	r4,3
        bclri	r5,4
        bseti	r6,5
        btsti	r7,6
        cmplti	r8,7
        cmpnei	r9,8
        lsli	r10,9
        lsri	r11,10
        rotli	r12,11
        rsubi	r13,12
        subi	r14,13

	bgeni	r4,0
	bgeni	r4,6
	bgeni	r4,7
	bgeni	r4,12
	bgeni	r4,31
	bmaski	r7,32
	bmaski	r7,1
	bmaski	r7,4
	bmaski	r7,7
	bmaski	r7,8
	bmaski	r7,22

	bf	*-$20
	br	*
	bsr	next
	bt	*+$200
next:

	align	4
	jmpi	[*+4]
	jsri	[*+2]

	ldb	r4,(0,r3)
	ldh	r4,(r3)
	ldw	r4,(r3,12)
	ld	r4,(10,r3,2)
	ld.b	r4,(3,r3)
	ld.h	r4,(r3,6)
	ld.w	r4,(24,r3)
	stb	r4,(0,r3)
	sth	r4,(r3)
	stw	r4,(r3,12)
	st	r4,(10,r3,2)
	st.b	r4,(3,r3)
	st.h	r4,(r3,6)
	st.w	r4,(24,r3)

	ldm	r2-r15,(r0)
	stm	r4-r15,(r0)
	ldq	r4-r7,(r3)
	stq	r4-r7,(r12)

	loopt	r4,*

	align	4
	lrm	r4,[*+4]
	lrm	r4,[*+2]

	mfcr	r12,cr5
        mfcr    r12,fpc
	mtcr	r2,cr26

ptrreg	reg	r10
	movi	r10,123
	movi	ptrreg,123

	trap	#2
