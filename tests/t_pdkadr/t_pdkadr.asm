	cpu	pms150

immval	equ	-1
memadr	equ	0x25,data
ioadr	sfr	0x1e

	; immediate values: either explicit with # prefix,
        ; or implicit with symbol from nameless segment

	mov	a,#-1
	mov	a,-1
	mov	a,#0x80
	mov	a,0x80
	mov	a,immval
	mov	a,#immval

	; usage of [] enforces data address, or implicit usage by symbol type

	mov	a,memadr
	mov	a,[0x25]
	mov	a,[memadr]
	mov	a,3+memadr
	mov	a,memadr+3
	mov	a,3[memadr]
	mov	a,memadr[3]

	; similar for I/O address

	mov	a,ioadr
	mov	a,io(0x1e)
	mov	a,io(ioadr)
	mov	a,io(0x1e+1)
	mov	a,io(ioadr+1)
	mov	a,io(1+ioadr)
