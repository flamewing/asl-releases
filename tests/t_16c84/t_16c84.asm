	cpu	16c84

	clrw
	nop
	clrwdt
	retfie
	return
	sleep
	option

	addlw	1
	andlw	100
	iorlw	$55
	movlw	%10101010
	retlw	0
	sublw	300-400
	xorlw	186

	addwf	3,f
	addwf	4,w
	addwf	5
	addwf	6,0
	addwf	7,1
	andwf	8
	comf	9,f
	comf	10,w
	comf	11
	comf	12,0
	comf	13,1
	decf	14
	decfsz	15
	incf	16
	incfsz	17
	iorwf	18
	movf	19
	rlf	20
	rrf	21
	subwf	22
	swapf	23
	xorwf	24

	bcf	17,4
	bsf	500,6
	btfsc	23,3
	btfss	20,0

	clrf	20
	movwf	33

	tris	6

	call	$200
	goto	$300
