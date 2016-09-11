	cpu	320c40

	include stddef4x.inc

	page 	0

	single	0.0
	single	6.0
	single	-3.0
	single	0.046875

	addi3	r2,r3,r4
	addi3	*+ar5(1),r3,r4
	addi3	*-ar5(1),r3,r4
	addi3	*++ar5(1),r3,r4
	addi3	*--ar5(1),r3,r4
	addi3	*ar5++(1),r3,r4
	addi3	*ar5--(1),r3,r4
	addi3	*ar5++(1)%,r3,r4
	addi3	*ar5--(1)%,r3,r4
	addi3	*ar5(ir0),r3,r4
	addi3	*-ar5(ir0),r3,r4
	addi3	*++ar5(ir0),r3,r4
	addi3	*--ar5(ir0),r3,r4
	addi3	*ar5++(ir0),r3,r4
	addi3	*ar5--(ir0),r3,r4
	addi3	*ar5++(ir0)%,r3,r4
	addi3	*ar5--(ir0)%,r3,r4
	addi3	*ar5(ir1),r3,r4
	addi3	*-ar5(ir1),r3,r4
	addi3	*++ar5(ir1),r3,r4
	addi3	*--ar5(ir1),r3,r4
	addi3	*ar5++(ir1),r3,r4
	addi3	*ar5--(ir1),r3,r4
	addi3	*ar5++(ir1)%,r3,r4
	addi3	*ar5--(ir1)%,r3,r4
	addi3	*ar5,r3,r4
	addi3	*ar5++(ir0)b,r3,r4

	; type 2 instr

	addi	23,r3,r4
	addi	*+ar5(23),r3,r4
	addi	23,*+ar5(23),r4
	addi	*+ar2(25),*+ar5(23),r4

	; test all registers!

	addi	die,r11,r9


	; new instructions

	irp	op,mpyshi,mpyuhi
	op	r9,r10
	op	@1234,r10
	op	*+ar5(22),r10
	op	35,r10
	op	r4,r5,r10
	op	r4,*++ar5(1),r10
	op	*++ar4(1),r5,r10
	op	*++ar4(1),*++ar5(1),r10
	op	4,r5,r10
	op	*+ar4(10),r5,r10
	op	4,*+ar5(10),r10
	op	*+ar4(10),*+ar5(10),r10
	endm

	irp	op,lb0,lb1,lb2,lb3,lbu0,lbu1,lbu2,lbu3,lh0,lh1,lhu0,lhu1
	op	r10,r4
	op	@1234,r10
	op	*++ar5(1),ar2
	op	42,rs
	endm

	lda	r4,ar2
	lda	@1234h,ar2
	lda	*ar0,ar2
	lda	1234,ar2
	;lda	r0,r2		; not allowed since dest register must be address register
	;lda	ar2,ar2		; not allowed since src & dest may not use same register
	;lda	*ar2,ar2	; ditto
	;lda	*ar5(ir0),ir0	; ditto
	;lda	*ar5(ir1),ir1	; ditto
	;lda	*ar5++(ir0)b,ir0; ditto

	ldep	ivtp,ar5
	ldpe	ar5,tvtp

	ldhi	44h,r2

	ldpk	2000h

	stik	10,@2000h
	stik	-10,*ar5

	irp	op,lwl0,lwl1,lwl2,lwl3,lwr0,lwr1,lwr2,lwr3
	op	r10,r4
	op	@1234,r10
	op	*++ar5(1),ar2
	op	42,rs
	endm

	irp	op,mb0,mb1,mb2,mb3,mh0,mh1
	op	r10,r4
	op	@1234,r10
	op	*++ar5(1),ar2
	op	42,rs
	endm

	irp	op,rcpf,rsqrf
	op	r10,r4
	op	@1234h,r10
	op	*++ar5(1),ar2
	op	2.0,r4
	endm

	retine
	retined
	retsne

	bne	$+5
	bned	$+5
	bneaf	$+5
	bneat	$+5

	; CALL/BR/BRD take displacement instead of absolute address on C4x!

	br	$+5
	brd	$+5
	call	$+5
	laj	$+5

	; similar for RPTB, except that it now also accepts an address from a register

	rptb	ar2
	rptb	$+5

	lajne	ar4
	lajne	$+5

	; TRAP (and LATcc) vectors now have 9 bits

	trapne	127
	latne	127

	frieee	@1234h,r2
	frieee	*ar5,r2

	toieee	r3,r2
	toieee	@1234h,r2
	toieee	*ar5,r2
	toieee	2.0,r2

	frieee	*++ar5,r4
||	stf	r6,*++ar6
	stf	r6,*++ar6
||	frieee	*++ar5,r4

	toieee	*++ar5,r4
||	stf	r6,*++ar6
	stf	r6,*++ar6
||	toieee	*++ar5,r4
