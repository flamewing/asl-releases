	cpu	4004

	nop

	jcn	z, Next
	jcn	nz, Next
	jcn	c, Next
	jcn	nc, Next
	jcn	t, Next
	jcn	nt, Next
	jcm	z, Next
	jcm	nz, Next
	jcm	c, Next
	jcm	nc, Next
	jcm	t, Next
	jcm	nt, Next
Next:
	fim	r0r1, 12h
	fim	r2r3, 23h
	fim	r4r5, 34h
	fim	r6r7, 45h
	fim	r8r9, 56h
	fim	rarb, 67h
	fim	rcrd, 78h
	fim	rerf, 89h

	src	r0r1
	src	r2r3
	src	r4r5
	src	r6r7
	src	r8r9
	src	rarb
	src	rcrd
	src	rerf

	fin	r0r1
	fin	r2r3
	fin	r4r5
	fin	r6r7
	fin	r8r9
	fin	rarb
	fin	rcrd
	fin	rerf

	jin	r0r1
	jin	r2r3
	jin	r4r5
	jin	r6r7
	jin	r8r9
	jin	rarb
	jin	rcrd
	jin	rerf

	jun	123h
	jms	456h

	inc	r0
	inc	r1
	inc	r2
	inc	r3
	inc	r4
	inc	r5
	inc	r6
	inc	r7
	inc	r8
	inc	r9
	inc	ra
	inc	rb
	inc	rc
	inc	rd
	inc	re
	inc	rf

loop:
	isz	r0, loop
	isz	r1, loop
	isz	r2, loop
	isz	r3, loop
	isz	r4, loop
	isz	r5, loop
	isz	r6, loop
	isz	r7, loop
	isz	r8, loop
	isz	r9, loop
	isz	ra, loop
	isz	rb, loop
	isz	rc, loop
	isz	rd, loop
	isz	re, loop
	isz	rf, loop

	add	a, r0
	add	a, r1
	add	a, r2
	add	a, r3
	add	a, r4
	add	a, r5
	add	a, r6
	add	a, r7
	add	a, r8
	add	a, r9
	add	a, ra
	add	a, rb
	add	a, rc
	add	a, rd
	add	a, re
	add	a, rf

	sub	a, r0
	sub	a, r1
	sub	a, r2
	sub	a, r3
	sub	a, r4
	sub	a, r5
	sub	a, r6
	sub	a, r7
	sub	a, r8
	sub	a, r9
	sub	a, ra
	sub	a, rb
	sub	a, rc
	sub	a, rd
	sub	a, re
	sub	a, rf

	ld	a, r0
	ld	a, r1
	ld	a, r2
	ld	a, r3
	ld	a, r4
	ld	a, r5
	ld	a, r6
	ld	a, r7
	ld	a, r8
	ld	a, r9
	ld	a, ra
	ld	a, rb
	ld	a, rc
	ld	a, rd
	ld	a, re
	ld	a, rf

        xch	r0
        xch	r1
        xch	r2
        xch	r3
        xch	r4
        xch	r5
        xch	r6
        xch	r7
        xch	r8
        xch	r9
        xch	ra
        xch	rb
        xch	rc
        xch	rd
        xch	re
        xch	rf

	bbl	1
	bbl	3
	bbl	0dh

	ldm	1
	ldm	3
	ldm	0dh

	wrm
	wmp
	wrr
	wpm
	wr0
	wr1
	wr2
	wr3
	sbm
	rdm
	rdr
	adm
	ad0
	ad1
	ad2
	ad3
	rd0
	rd1
	rd2
	rd3

	clb
	clc
	iac
	cmc
	cma
	ral
	rar
	tcc
	dac
	tcs
	stc
	daa
	kbp
	dcl

	data	1,2,3,4,5
	data	"This is a test"
