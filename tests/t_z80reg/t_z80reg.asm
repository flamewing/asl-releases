	page	0

	; Base Z80: ordinary byte/word registers, special I/R registers,
	; indirect addressing, and instruction-specific register operands.

	cpu	z80

z_a	reg	a
z_b	reg	b
z_c	reg	c
z_i	reg	i
z_r	reg	r
z_af	reg	af
z_bc	reg	bc
z_de	reg	de
z_hl	reg	hl
z_sp	reg	sp
z_ix	reg	ix
z_iy	reg	iy
z_acc	reg	z_a
family_ixu reg	ixu
family_sr reg	sr

	expect	1350
	ld	a,family_ixu
	endexpect

	expect	1350
	push	family_sr
	endexpect

	ld	b,a
	ld	z_b,z_a
	ld	bc,1234h
	ld	z_bc,1234h
	ld	a,i
	ld	z_a,z_i
	ld	i,a
	ld	z_i,z_a
	ld	a,r
	ld	z_a,z_r
	ld	r,a
	ld	z_r,z_a
	inc	b
	inc	z_b
	add	a,b
	add	z_a,z_b
	push	af
	push	z_af
	pop	ix
	pop	z_ix
	ex	de,hl
	ex	z_de,z_hl
	ex	(sp),iy
	ex	(z_sp),z_iy
	jp	(hl)
	jp	(z_hl)
	jp	(ix)
	jp	(z_ix)
	ld	a,(bc)
	ld	z_a,(z_bc)
	ld	(de),a
	ld	(z_de),z_a
	ld	a,(ix+5)
	ld	z_a,(z_ix+5)
	ld	(iy-3),b
	ld	(z_iy-3),z_b
	in	a,(c)
	in	z_a,(z_c)
	out	(c),a
	out	(z_c),z_a
	ld	a,b
	ld	z_acc,z_b

	; Undocumented Z80 registers.  AF' is intentionally not covered yet.

	cpu	z80undoc

u_a	reg	a
u_c	reg	c
u_f	reg	f
u_ixh	reg	ixh
u_iyl	reg	iyl

	ld	ixh,a
	ld	u_ixh,u_a
	ld	a,iyl
	ld	u_a,u_iyl
	inc	ixh
	inc	u_ixh
	in	f,(c)
	in	u_f,(u_c)

	; Z180 I/O instructions.

	cpu	z180

n_a	reg	a
n_b	reg	b
n_c	reg	c

	in0	a,(12h)
	in0	n_a,(12h)
	out0	(12h),b
	out0	(12h),n_b
	in	b,(c)
	in	n_b,(n_c)
	out	(c),b
	out	(n_c),n_b

	; Rabbit prefix handling must preserve aliases in the following instruction.

	cpu	rabbit2000

r_a	reg	a
r_af	reg	af
r_iy	reg	iy

	altd	ld a,(iy+4)
	altd	ld r_a,(r_iy+4)
	altd	inc iy
	altd	inc r_iy
	altd	push af
	altd	push r_af
	altd	pop af
	altd	pop r_af

	; Z380 index halves, stack-relative addressing and control registers.

	cpu	z380

z3_a	reg	a
z3_hl	reg	hl
z3_sp	reg	sp
z3_ixl	reg	ixl
z3_iyl	reg	iyl
z3_iyu	reg	iyu
z3_xsr	reg	xsr
z3_dsr	reg	dsr

	ld	a,ixu
	ld	z3_a,family_ixu
	ld	iyl,iyu
	ld	z3_iyl,z3_iyu
	inc	ixl
	inc	z3_ixl
	ld	hl,(sp+5)
	ld	z3_hl,(z3_sp+5)
	push	sr
	push	family_sr
	ldctl	xsr,a
	ldctl	z3_xsr,z3_a
	ldctl	a,dsr
	ldctl	z3_a,z3_dsr
	ldctl	sr,hl
	ldctl	family_sr,z3_hl
	ldctl	hl,sr
	ldctl	z3_hl,family_sr
	ina	a,34h
	ina	z3_a,34h
	inaw	hl,34h
	inaw	z3_hl,34h
	ex	(hl),a
	ex	(z3_hl),z3_a

	; Additional instruction-specific paths not handled solely by DecodeAdr.

	cpu	z80

cov_d	reg	d
cov_e	reg	e
cov_h	reg	h
cov_l	reg	l

	ld	d,e
	ld	cov_d,cov_e
	ld	h,l
	ld	cov_h,cov_l
	cpl	a
	cpl	z_a
	neg	a
	neg	z_a

	expect	1350
	ld	z_a,z_af
	endexpect

	cpu	z80undoc

	rrc	b,(iy-3)
	rrc	z_b,(z_iy-3)
	res	c,4,(ix-1)
	res	z_c,4,(z_ix-1)
	set	l,6,(iy+17)
	set	cov_l,6,(z_iy+17)

	expect	1350
	out	(z_c),u_f
	endexpect

	cpu	z380

cov_bc	reg	bc
cov_de	reg	de
cov_ysr reg	ysr

	cplw	hl
	cplw	z3_hl
	negw	hl
	negw	z3_hl
	extsw	hl
	extsw	z3_hl
	addw	hl,de
	addw	z3_hl,cov_de
	adcw	hl,bc
	adcw	z3_hl,cov_bc
	sbcw	hl,de
	sbcw	z3_hl,cov_de
	andw	hl,de
	andw	z3_hl,cov_de
	subw	hl,bc
	subw	z3_hl,cov_bc
	multw	hl,de
	multw	z3_hl,cov_de
	multuw	hl,bc
	multuw	z3_hl,cov_bc
	divuw	hl,ix
	divuw	z3_hl,z_ix
	inw	hl,(c)
	inw	z3_hl,(z_c)
	outw	(c),de
	outw	(z_c),cov_de
	outa	(34h),a
	outa	(34h),z3_a
	outaw	(34h),hl
	outaw	(34h),z3_hl
	ldctl	ysr,a
	ldctl	cov_ysr,z3_a

	expect	1350
	ld	z3_a,u_ixh
	endexpect

	expect	1350
	ld	z3_a,z3_xsr
	endexpect
