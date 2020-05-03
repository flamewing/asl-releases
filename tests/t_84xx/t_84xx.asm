	cpu	8461
	page	0

	add	a,r0
	add	a,r1
	add	a,r2
	add	a,r3
	add	a,r4
	add	a,r5
	add	a,r6
	add	a,r7
	add	a,@r0
	add	a,@r1
	add	a,#55h

	addc	a,r0
	addc	a,r1
	addc	a,r2
	addc	a,r3
	addc	a,r4
	addc	a,r5
	addc	a,r6
	addc	a,r7
	addc	a,@r0
	addc	a,@r1
	addc	a,#55h

	anl	a,r0
	anl	a,r1
	anl	a,r2
	anl	a,r3
	anl	a,r4
	anl	a,r5
	anl	a,r6
	anl	a,r7
	anl	a,@r0
	anl	a,@r1
	anl	a,#55h

	orl	a,r0
	orl	a,r1
	orl	a,r2
	orl	a,r3
	orl	a,r4
	orl	a,r5
	orl	a,r6
	orl	a,r7
	orl	a,@r0
	orl	a,@r1
	orl	a,#55h

	xrl	a,r0
	xrl	a,r1
	xrl	a,r2
	xrl	a,r3
	xrl	a,r4
	xrl	a,r5
	xrl	a,r6
	xrl	a,r7
	xrl	a,@r0
	xrl	a,@r1
	xrl	a,#55h

	inc	a

	dec	a

	clr	a

	cpl	a

	rl	a

	rlc	a

	rr	a

	rrc	a

	da	a

	swap	a

	mov	a,r0
	mov	a,r1
	mov	a,r2
	mov	a,r3
	mov	a,r4
	mov	a,r5
	mov	a,r6
	mov	a,r7
	mov	a,@r0
	mov	a,@r1
	mov	a,#55h
	mov	r0,a
	mov	r1,a
	mov	r2,a
	mov	r3,a
	mov	r4,a
	mov	r5,a
	mov	r6,a
	mov	r7,a
	mov	@r0,a
	mov	@r1,a
	mov	r0,#55h
	mov	r1,#55h
	mov	r2,#55h
	mov	r3,#55h
	mov	r4,#55h
	mov	r5,#55h
	mov	r6,#55h
	mov	r7,#55h
	mov	@r0,#55h
	mov	@r1,#55h

	xch	r0,a
	xch	r1,a
	xch	r2,a
	xch	r3,a
	xch	r4,a
	xch	r5,a
	xch	r6,a
	xch	r7,a
	xch	@r0,a
	xch	@r1,a

	xchd	a,@r0
	xchd	a,@r1

	mov	a,psw
	mov	psw,a
	movp	a,@a

	clr	c

	cpl	c

	inc	r0
	inc	r1
	inc	r2
	inc	r3
	inc	r4
	inc	r5
	inc	r6
	inc	r7
	inc	@r0
	inc	@r1

	dec	r0
	dec	r1
	dec	r2
	dec	r3
	dec	r4
	dec	r5
	dec	r6
	dec	r7
	dec	@r0
	dec	@r1

	jmp	234h

	jmpp	@a

	djnz	r0,$
	djnz	r1,$
	djnz	r2,$
	djnz	r3,$
	djnz	r4,$
	djnz	r5,$
	djnz	r6,$
	djnz	r7,$
	djnz	@r0,$
	djnz	@r1,$

	jb0	$
	jb1	$
	jb2	$
	jb3	$
	jb4	$
	jb5	$
	jb6	$
	jb7	$

	jc	$
	jnc	$
	jz	$
	jnz	$
	jt0	$
	jnt0	$
	jt1	$
	jnt1	$
	jtf	$
	jntf	$

	mov	a,t
	mov	t,a

	strt	cnt
	strt	t
	stop	tcnt
	en	tcnti
	dis	tcnti

	en	i
	dis	i
	
	sel	rb0
	sel	rb1
	sel	mb0
	sel	mb1
	sel	mb2
	sel	mb3

	call	234h
	ret
	retr

	in	a,p0
	in	a,p1
	in	a,p2

	outl	p0,a
	outl	p1,a
	outl	p2,a

	anl	p0,#55h
	anl	p1,#55h
	anl	p2,#55h

	orl	p0,#55h
	orl	p1,#55h
	orl	p2,#55h

	outl	p0,a

	mov	a,s0
	mov	a,s1
	mov	s0,a
	mov	s1,a
	mov	s2,a
	mov	s0,#55h
	mov	s1,#55h
	mov	s2,#55h

	en	si
	dis	si

	nop
