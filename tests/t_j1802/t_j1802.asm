	cpu	1802
	page	0

	; case 1: all is in one page:
        ;  jz becomes bz
	;  jnz becomes bnz

	org	40h

dly1:	glo	2
	jz	skip1
l1:	dec	2
	glo	2
	jnz	l1
skip1:	nop

	; case 2: page boundary in loop body:
        ;  jz becomes lbz
	;  jnz becomes lbnz

	org	1fbh

dly2:	glo	2
	jz	skip2
l2:	dec	2
	glo	2
	jnz	l2
skip2:	nop

	; just to test all instructions at least once:

	jmp	280h
	br	280h
	jz	280h
	bz	280h
	jnz	280h
	bnz	280h
	jdf	280h
	bdf	280h
	jpz	280h
	bpz	280h
	jge	280h
	bge	280h
	jnf	280h
	bnf	280h
	jm	280h
	bm	280h
	jl	280h
	bl	280h
	jq	280h
	bq	280h
	jnq	280h
	bnq	280h

	jmp	2800h
	lbr	2800h
	jz	2800h
	lbz	2800h
	jnz	2800h
	lbnz	2800h
	jdf	2800h
	lbdf	2800h
	jpz	2800h
	lbpz	2800h
	jge	2800h
	lbge	2800h
	jnf	2800h
	lbnf	2800h
	jm	2800h
	lbm	2800h
	jl	2800h
	lbl	2800h
	jq	2800h
	lbq	2800h
	jnq	2800h
	lbnq	2800h
