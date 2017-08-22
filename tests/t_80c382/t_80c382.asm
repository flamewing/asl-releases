	cpu	80C382

	idl		; different opcode
	halt		; different opcode than for 80C39
	jni	$	; different opcode

	dec	@r0	; new addressing mode for instruction
	dec	@r1	; new addressing mode for instruction

	djnz	@r0,$	; new addressing mode for instruction
	djnz	@r1,$	; new addressing mode for instruction

	; these ones are no longe supported on the 80C382,
        ; so they must result in an error when uncommented:

	;jf0	$
	;jf1	$
	;clr	f0
	;clr	f1
	;cpl	f0
	;cpl	f1
	;in	a,p2
	;outl	p2,a
	;orl	p2,#1
	;anl	p2,#0feh
	;movd	p7,a
	;movd	a,p7
	;anld	p7,#5
	;orld	p7,#5
	;mov	a,psw
	;mov	psw,a
