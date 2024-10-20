	cpu	8051
	page	0

	; NOTE: The if construct only works because of short circuit
	; evaluation of the && operator.  If a symbol is forward-defined,
	; the lhs will evaluate to false in the first pass, and
	; the rhs will not be evaluated at all, so no error message
	; about things having to be defined in pass one: 

; This fork rejects questionable forward values in IF on later passes too.
; Use DEFINED alone, keeping forward targets in the long form.
jz	macro	target
	if	defined(target) && (abs($-(target)) <= 125)
	!jz	target
	elseif
	!jnz	+
	jmp	target
	endif
+
	endm

bk2	nop
	db	128 dup (0)
bk1	nop
	jz	bk1
	jz	bk2
	jz	fwd1
	jz	fwd2
	nop
	nop
	nop
fwd1	db	128 dup (0)
fwd2	nop

