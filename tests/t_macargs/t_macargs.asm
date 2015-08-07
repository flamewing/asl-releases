	cpu	68000

	padding	off

dc_len	macro	args
	dc.ATTRIBUTE	ARGCOUNT
	if	ARGCOUNT<>0
	 dc.ATTRIBUTE	 ALLARGS
	endif
	endm

	dc_len.b	1,2,3
	dc_len.w	1
	dc_len.l	5,6,7,8,9,10
	dc_len.q

moveq2	macro	arg
__LABEL__ moveq	#arg,d2
	endm	

loop	moveq2	20
	dbra	d2,loop
