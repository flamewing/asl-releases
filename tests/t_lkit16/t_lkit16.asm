	cpu	mn1610alt
	page	0

DIGIT	equ	x'7D'
AUTB	equ	x'67'

	org	x'122d'

	clear	x1
	mvi	x1,x'3'		; X1 <--- X'3'
	clear	x0		; X0 <--- X'0'
;
	mv	r0,r0,m		; If R0 is not negative,
	b	BTA1
	mvi	r1,x'2D'	; then set the sign '-' first
	st	r1,DIGIT(x0)
	ai	x0,x'1'
;
	clear	r1
	s	r1,r0
	mv	r0,r1		; R0 <--- (-1)*R0
BTA1	equ	*
	L	r1,(AUTB)(x1)	; R1 <--- Unit of each Figure
	clear	r2
BTA2	equ	*
	C	r0,r1,pz	; if binary number >= unit, then
	b	BTA3
	s	r0,r1		; subtract unit from binary number &
	ai	r2,x'1'		; count the subtraction.
	b	BTA2
BTA3	equ	*
	mv	r2,r2,nz	; if counter is not zero
	b	BTA6
	sbit	str,x'f'	; then set flag
BTA4	equ	*
	sbit	r2,x'a'
	sbit	r2,x'b'		; get ASCII code
	st	r2,DIGIT(x0)	; & set it into buffer
	ai	x0,x'1'
BTA5	equ	*

	;
	;
	org	x'1262'
BTA6	equ	*
