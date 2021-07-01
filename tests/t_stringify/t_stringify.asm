	cpu	8051

cnt	set	0
	while	cnt<>10
	if (cnt#2)==1
var{"\{CNT}"} equ	cnt
	else
var{CNT} equ	cnt
	endif
cnt	set	cnt+1
	endm

	db	var1,var9,var2,var8
