	cpu	8085
	z80syntax on
	page	0

cnt	equ	b
cnt_hi	equ	b
cnt_lo	equ	c
count	equ	bc
src	equ	d
src_hi	equ	d
src_lo	equ	e
source	equ	de
dest	equ	h
dest_hi	equ	h
dest_lo	equ	l
destination equ	hl

bswap	macro	reg,save
	if	save
	 push	psw
	endif
	switch	reg
	case	b
	mov	a,b
	mov	b,c
	mov	c,a
	case	d
	mov	a,d
	mov	d,e
	mov	e,a
	case	h
	mov	a,h
	mov	h,l
	mov	l,a
	elsecase
	error	"unknown 16-bit register to swap"
	endcase
	if	save
	 pop	psw
	endif
	endm

	bswap	b,true
	bswap	d,true
	bswap	h,true
	bswap	bc,true
	bswap	de,true
	bswap	hl,true
	bswap	cnt,true
	bswap	src,true
	bswap	dest,true
	bswap	count,true
	bswap	source,true
	bswap	destination,true
	expect	9990
	bswap	l,true
	endexpect

	inx	b
	inx	d
	inx	h
	inx	bc
	inx	de
	inx	hl
	inx	cnt
	inx	src
	inx	dest
	inx	count
	inx	source
	inx	destination

	dcr	b
	dcr	c
	dcr	d
	dcr	e
	dcr	h
	dcr	l
	dcr	cnt_hi
	dcr	cnt_lo
	dcr	src_hi
	dcr	src_lo
	dcr	dest_hi
	dcr	dest_lo

	ld	a,(bc)
	ld	a,(de)
	ld	a,(hl)
	ld	a,(cnt)
	ld	a,(src)
	ld	a,(dest)
	ld	a,(count)
	ld	a,(source)
	ld	a,(dest)
