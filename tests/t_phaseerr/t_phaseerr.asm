	cpu	68000
	page	0

	; these result in n*4 bytes in pass 1, and n*6 bytes in pass 2

	rept	100
	jsr	sub
	endm

	; Currently needed to trigger the "repass anyway needed because symbol changed"
        ; condition before the first branch is evaluated.  In a typical program, there
        ; is always some label present to trigger it.
dummy:

	; without detection mechanism, the values of PC in pass 2 and of 'skip' from
        ; pass 1 are so much apart that we would get an out-of-branch condition:

	bra.s	skip
	nop
	nop
	nop
	nop
	nop
	nop
skip:

	; Usually, one would try to define symbols before they are used the
        ; first time.  Do it different here, just to demonstrate the situation:

sub	equ	$10000

