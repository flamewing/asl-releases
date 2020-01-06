	cpu	68040

	org	$1000

	; automatic length deduction must use 16 bit displacement for
	; BSR instruction, since a displacement of 0 is not allowed
	; for a 8-bit displacement:

	bsr	next
next:	nop

	; similar here: note that this is a programming error since
        ; the instruction is 4 bytes long, so we branch *into* the
	; instruction itself!

	bsr	*+2

	; same code as in first example

	bsr.l	next_l
next_l: nop

	; this would result in an error since an 8-bit displacement
	; of 0 is not allowed, and the BSR instruction cannot be
	; replaced with a NOP.

;	bsr.s	next_s
;next_s:	nop

	; in this case a branch to the next instruction can be
	; replaced with a NOP.  Will throw a warning however...

	if	mompass=3
 	 expect 60
	endif
	 bra.s	 next_b
	if	mompass=3
	 endexpect
	endif
next_b: nop
