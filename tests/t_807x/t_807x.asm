	cpu	8070

	nop
	ret
	ssm

	ld	a,10h,pc
	ld	a,10h,sp
	ld	a,-3,p2
	ld	a,-10,p3
	ld	a,#20
	ld	a,12h
	ld	a,@+1,p2
	ld	a,@-1,p3
	ld	a,e
	ld	a,s

	ld	ea,10h,pc
        ld      ea,10h,sp
        ld      ea,-3,p2 
        ld      ea,-10,p3
        ld      ea,#2030h
        ld      ea,12h   
        ld      ea,@+1,p2
        ld      ea,@-1,p3
	ld	ea,pc
	ld	ea,sp
	ld	ea,p2
	ld	ea,p3
	ld	ea,t
	ld	pc,ea
	ld	sp,ea
	ld	p2,ea
	ld	p3,ea
	ld	sp,#1234h
	ld	p2,#1234h
	ld	p3,#1234h
	ld	t,10h,pc
        ld      t,10h,sp
        ld      t,-3,p2 
        ld      t,-10,p3
        ld      t,#2030h
        ld      t,12h   
        ld      t,@+1,p2
        ld      t,@-1,p3
	ld	t,ea
	ld	e,a

	st	a,10h,pc
	st	a,10h,sp
	st	a,-3,p2
	st	a,-10,p3
	st	a,12h
	st	a,@+1,p2
	st	a,@-1,p3

        st      ea,10h,pc
        st      ea,10h,sp
        st      ea,-3,p2 
        st      ea,-10,p3
        st      ea,12h   
        st      ea,@+1,p2
        st      ea,@-1,p3

	add	a,10h,pc
	add	a,10h,sp
	add	a,-3,p2
	add	a,-10,p3
	add	a,#20
	add	a,12h
	add	a,@+1,p2
	add	a,@-1,p3
	add	a,e

        add     ea,10h,pc
        add     ea,10h,sp
        add     ea,-3,p2 
        add     ea,-10,p3
	add	ea,#2030h
        add     ea,12h   
        add     ea,@+1,p2
        add     ea,@-1,p3

	sub	a,10h,pc
	sub	a,10h,sp
	sub	a,-3,p2
	sub	a,-10,p3
	sub	a,#20
	sub	a,12h
	sub	a,@+1,p2
	sub	a,@-1,p3
	sub	a,e

        sub     ea,$,pc
        sub     ea,10h,sp
        sub     ea,-3,p2 
        sub     ea,-10,p3
	sub	ea,#2030h
        sub     ea,12h   
        sub     ea,@+1,p2
        sub     ea,@-1,p3

        mpy	ea,t
	div	ea,t

        and     a,$,pc
        and     a,10h,sp
        and     a,-3,p2
        and     a,-10,p3
        and     a,#20
        and     a,12h
        and     a,@+1,p2
        and     a,@-1,p3
        and     a,e
	and	s, #10h

        or	a,$,pc
        or	a,10h,sp
        or	a,-3,p2
        or	a,-10,p3
        or	a,#20
        or	a,12h
        or	a,@+1,p2
        or	a,@-1,p3
        or	a,e
	or	s, #10h

        xor     a,$,pc
        xor     a,10h,sp
        xor     a,-3,p2
        xor     a,-10,p3
        xor     a,#20
        xor     a,12h
        xor     a,@+1,p2
        xor     a,@-1,p3
        xor     a,e

	xch	a,e
	xch	e,a
	irp	reg,pc,sp,p2,p3
	xch	ea,reg
	xch	reg,ea
	endm

	sr	a
	sr	ea
	srl	a
	rr	a
	rrl	a
	sl	a
	sl	ea

	push	a
	push	ea
	push	pc
	push	p2
	push	p3

	pop	a
	pop	ea
	pop	p2
	pop	p3

	pli	p2,#1234h
	pli	p3,#1234h

	ild	a,$,pc
	ild	a,10h,sp
	ild	a,-3,p2
	ild	a,-10,p3
	ild	a,12h
	ild	a,@+1,p2
	ild	a,@-1,p3

	dld	a,$,pc
	dld	a,10h,sp
	dld	a,-3,p2
	dld	a,-10,p3
	dld	a,12h
	dld	a,@+1,p2
	dld	a,@-1,p3

	jmp	1234h
	jsr	1234h

	call	7

	bnd	$
	bra	$
	bra	10,p2
	bra	-10,p3
	bp	$
	bp	10,p2
	bp	-10,p3
	bz	$
	bz	10,p2
	bz	-10,p3
	bnz	$
	bnz	10,p2
	bnz	-10,p3

	db	1,2,3,4
	dw	1,2,3,4
