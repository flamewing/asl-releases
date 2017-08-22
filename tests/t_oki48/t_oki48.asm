	cpu	msm80c48

	halt			; same as IDL for Intel 80C48
	hlts
	flt
	fltt
	fres

	dec	@r0		; interesting: same machine codes as for Siemens 80C382!
	dec	@r1
	djnz	@r0,$
	djnz	@r1,$

	mov	a,p1
	mov	a,p2
	mov	p1,@r3
	movp1	p,@r3
