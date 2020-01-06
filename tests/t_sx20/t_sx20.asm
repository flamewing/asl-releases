		cpu	sx20
		page	0
		include	"regsx20.inc"

		and	$08,w
		and	w,$09
		and	w,#$ff

		not	$0d

		or	$08,w
		or	w,$09
		or	w,#$ff

		xor	$08,w
		xor	w,$09
		xor	w,#$ff

		add	$08,w
		add	w,$09

		clr	w
		clr	!wdt
		clr	$0e

		dec	$0a
		decsz	$0b

		inc	$0c
		incsz	$0d

		rl	$04

		rr	$05

		sub	$0a,w

		swap	$06

		clrb	$0f.4
		sb	$0e.5
		setb	$0d.6
		snb	$0c.7

		mov	$0e,w
		mov	w,$09
		mov	w,$0d-w
		mov	w,#$aa
		mov	w,/$0a
		mov	w,--$0b
		mov	w,++$0e
		mov	w,<<$0c
		mov	w,>>$0d
		mov	w,<>$08
		mov	w,m
		movsz	w,--$0b
		movsz	w,++$0e
		mov	m,w
		mov	m,#$a
		mov	!$06,w
		mov	!option,w

		test	$07

		call	$034
		jmp	$145

		nop

		ret
		retp
		reti
		retiw
		retw	$bb

		iread

		sleep

		page	$600

		bank	$90

		assume	fsr:$10
		mov	w,$13
		expect	110
		 mov	 w,$33
		endexpect

		assume	fsr:$30
		expect	110
		 mov	 w,$13
		endexpect
		mov	w,$33

		assume	status:$00
		jmp	$056
		expect	110
		 jmp	 $256
		endexpect
		expect	110
		 jmp	 $456
		endexpect
		expect	110
		 jmp	 $656
		endexpect

		assume	status:$40
		expect	110
		 jmp	 $056
		endexpect
		expect	110
		 jmp	 $256
		endexpect
		jmp	$456
		expect	110
		 jmp	 $656
		endexpect

mybit		bit	$12.3
mybit1		bit	$32.3
		assume	fsr:$10
		setb	mybit
		expect	110
		 setb	 mybit1
		endexpect
		assume	fsr:$30
		expect	110
		 setb	 mybit
		endexpect
		setb	mybit1

		; emulated instructions

		clrb	C
		clc
		clrb	Z
		clz
		setb	C
		sec
		setb	Z
		sez
		mov	PC,w
		jmp	w
		add	PC,w
		jmp	pc+w
		mov	m,#14
		mode	14
		xor	w,#$ff
		not	w
		sb	c
		sc
		sb	z
		sz
		align	2
		snb	PC.0
		sb	PC.0
		skip
		skip