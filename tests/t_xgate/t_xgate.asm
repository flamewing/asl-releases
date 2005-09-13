	cpu	xgate

	nop
	brk
	rts

	irp	instr,bcc,bcs,beq,bge,bgt,bhi,bhs,ble,blo,bls,blt,bmi,bne,bpl,bra,bvc,bvs
	instr	target
	endm
target:

cnt	set	1
	irp	instr,asr,csl,csr,lsl,lsr,rol,ror
	instr	r3,#cnt
	instr	r5,r1
cnt	set	cnt+1
	endm

	irp	instr,add,and,or,sub,xnor
	instr	r5,r6,r7
	instr	r2,r3
	instr	r2,#$9876
	endm

	irp	instr,addh,addl,andh,andl,bith,bitl,cmpl,cpch,orh,orl,subh,subl,xnorh,xnorl,ldl,ldh
	instr	r1,#$56
	endm

	irp	instr,bfext,bfins,bfinsi,bfinsx
	instr	r1,r2,r3
	endm

	irp	instr,adc,sbc
	instr	r5,r6,r7
	instr	r5,r6
	endm

	cpc	r4,r5
	sbc	r0,r4,r5

	mov	r4,r5

	bfffo	r4,r5

	irp	instr,com,neg
	instr	r1,r6
	instr	r3,r3
	instr	r3
	endm

	jal	r5
	par	r5
	sex	r5

	tst	r2
	sub	r0,r2,r0

	irp	instr,csem,ssem
	instr	#4
	instr	r4
	endm

	sif
	sif	r5

	tfr	r2,ccr
	tfr	ccr,r5
	tfr	r4,pc

	cmp	r3,r5
	cmp	r2,#$89ab

	irp	instr,ldb,ldw,stb,stw
	instr	r5,(r1,#20)
	instr	r5,(r3,r2)
	instr	r5,(r4,r2+)
	instr	r5,(r2,-r1)
	instr	r5,(r0,#23)
	instr	r5,(#23)
	instr	r5,(r0,r2)
	instr	r5,(r2)
	instr	r5,(r0,r2+)
	instr	r5,(r2+)
	instr	r5,(r0,-r1)
	instr	r5,(-r1)
	endm

	ldw	r5,#$1234