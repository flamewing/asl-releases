	cpu	kcpsm3

	CONSTANT const1, 02
	NAMEREG s08, treg


	load	treg, const1
	
	nop
	
	load	s0, 1
	jump	test1
	jump	Z, test1
	jump	NZ, test1
	jump	C, test1
	jump	NC, test1

test1:
	call	test2
	call	Z, test2
	call	NZ, test2
	call	C, test2
	call	NC, test2

test2:
	return
	return	Z
	return	NZ
	return	C
	return	NC

	load	s3, 21
	and	s12, 03
	or	s4, 19
	xor	s7, 71

	load	s1, s5
	and	s1, s5
	or	s1, s5
	xor	s1, s5

	add	s9, 21
	addcy	s9, 03
	sub	s9, 19
	subcy	s9, 71

	add	s1, s4
	addcy	s1, s4
	sub	s1, s4
	subcy	s1, s4

	sr0	s12
	sr1	s12
	srx	s12
	sra	s12
	rr	s12

	sl0	s12
	sl1	s12
	slx	s12
	sla	s12
	rl	s12

	input	s6, 21
	input	s11, (s1)
	output	s14, 21
	output	s15, (s1)

	returni	enable
	returni	disable

	enable	interrupt
	disable	interrupt

        ; new KCPSM3 instructions

	compare	s1,s7
	compare	s6,-3

	fetch	s2,vari
	fetch	s9,(s4)

	store	s3,vari
	store	s6,(s14)

	test	s8,0aah
	test	s12,s3

	segment	data

	org	01ah
vari:

