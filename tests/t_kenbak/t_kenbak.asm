	cpu	kenbak
	page	0

	org	4		; locations 0..3 are registers

	irp	instr,add,sub,load,store
	irp	reg,a,b,x
	instr	reg,#12h
	instr	reg,12h
	instr	reg,a
	instr	reg,b
	instr	reg,x
	instr	reg,(12h)
	instr	reg,12h,x
	instr	reg,(12h),x
	endm
	endm

	irp	instr,and,or,lneg
	instr	a,#12h
	instr	a,12h
	instr	a,a
	instr	a,b
	instr	a,x
	instr	a,(12h)
	instr	a,12h,x
	instr	a,(12h),x
	endm
