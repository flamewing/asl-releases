	cpu	kenbak
	page	0

	org	4		; locations 0..3 are registers

	; register aliases

reg_a	equ	a
reg_b	equ	b
reg_x	equ	x

	add	a,#10
	add	reg_a,#10
	add	a,b
	add	reg_a,reg_b
	add	a,10,x
	add	reg_a,10,reg_x

	; bit symbols

bit1	bit	4,12h
bit1a	bit	bit1
bit2	bit	bit1+1

	set0	4,12h
	set0	bit1
	set0	bit1a
	set1	5,12h
	set1	bit2

	skp0	4,12h
	skp0	4,12h,$+4
	skp0	bit1
	skp0	bit1,$+4
	skp0	bit1a
	skp0	bit1a,$+4

	skp1	5,12h
	skp1	5,12h,$+4
	skp1	bit2
	skp1	bit2,$+4
