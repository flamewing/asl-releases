	cpu	rabbit2000

	; without prefix

	nop
	inc	iy
	ld	a,(iy+4)

	; with prefix

	altd
	altd	nop
	altd	inc iy
	altd	ld a,(iy+4)

	; used to test correct marking of unknown opcode

	;altd	nix
