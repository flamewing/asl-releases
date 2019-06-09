		cpu	stm8
		page	0

		adc	a,#$55		; A9 55
		adc	a,$10		; B9 10
		adc	a,$1000		; C9 10 00
		adc	a,(x)		; F9
		adc	a,($10,x)	; E9 10
		adc	a,($1000,x)	; D9 10 00
		adc	a,(y)		; 90 F9
		adc	a,($10,y)	; 90 E9 10
		adc	a,($1000,y)	; 90 D9 10 00
		adc	a,($10,sp)	; 19 10
		adc	a,[$10.w]	; 92 C9 10
		adc	a,[$10]		; 92 C9 10 (same, support for 8-bit pointers removed in STM8)
		adc	a,[>$10.w]	; 72 C9 00 10 (forced long address)
		adc	a,[$1000.w]	; 72 C9 10 00 (implicit long address)
		adc	a,([$10.w],x)	; 92 D9 10
		adc	a,([>$10.w],x)  ; 72 D9 00 10 (forced long address)
		adc	a,([$1000.w],x)	; 72 D9 10 00 (implicit long address)
		adc	a,([$10.w],y)	; 91 D9 10

		add	a,#$55		; AB 55
		add	a,$10		; BB 10
		add	a,$1000		; CB 10 00
		add	a,(x)		; FB
		add	a,($10,x)	; EB 10
		add	a,($1000,x)	; DB 10 00
		add	a,(y)		; 90 FB
		add	a,($10,y)	; 90 EB 10
		add	a,($1000,y)	; 90 DB 10 00
		add	a,($10,sp)	; 1B 10
		add	a,[$10.w]	; 92 CB 10
		add	a,[$10]		; 92 CB 10 (same, support for 8-bit pointers removed in STM8)
		add	a,[>$10.w]	; 72 CB 00 10 (forced long address)
		add	a,[$1000.w]	; 72 CB 10 00 (implicit long address)
		add	a,([$10.w],x)	; 92 DB 10
		add	a,([>$10.w],x)  ; 72 DB 00 10 (forced long address)
		add	a,([$1000.w],x)	; 72 DB 10 00 (implicit long address)
		add	a,([$10.w],y)	; 91 DB 10

		add	x,#$1000	; 1C 10 00
		addw	x,#$1000	; 1C 10 00
		add	x,$1000		; 72 BB 10 00
		addw	x,$1000		; 72 BB 10 00
		add	x,($10,sp)	; 72 FB 10
		addw	x,($10,sp)	; 72 FB 10
		add	y,#$1000	; 72 A9 10 00
		addw	y,#$1000	; 72 A9 10 00
		add	y,$1000		; 72 B9 10 00
		addw	y,$1000		; 72 B9 10 00
		add	y,($10,sp)	; 72 F9 10
		addw	y,($10,sp)	; 72 F9 10
		add	sp,#$9		; 5B 09
		addw	sp,#$9		; 5B 09

		and	a,#$55		; A4 55
		and	a,$10		; B4 10
		and	a,$1000		; C4 10 00
		and	a,(x)		; F4
		and	a,($10,x)	; E4 10
		and	a,($1000,x)	; D4 10 00
		and	a,(y)		; 90 F4
		and	a,($10,y)	; 90 E4 10
		and	a,($1000,y)	; 90 D4 10 00
		and	a,($10,sp)	; 14 10
		and	a,[$10.w]	; 92 C4 10
		and	a,[$1000.w]	; 72 C4 10 00
		and	a,([$10.w],x)	; 92 D4 10
		and	a,([$1000.w],x)	; 72 D4 10 00
		and	a,([$10],y)	; 91 D4 10

		bccm	$1000,#2	; 90 15 10 00

		bcp	a,#$55		; A5 55
		bcp	a,$10		; B5 10
		bcp	a,$1000		; C5 10 00
		bcp	a,(x)		; F5
		bcp	a,($10,x)	; E5 10
		bcp	a,($1000,x)	; D5 10 00
		bcp	a,(y)		; 90 F5
		bcp	a,($10,y)	; 90 E5 10
		bcp	a,($1000,y)	; 90 D5 10 00
		bcp	a,($10,sp)	; 15 10
		bcp	a,[$10.w]	; 92 C5 10
		bcp	a,[$1000.w]	; 72 C5 10 00
		bcp	a,([$10.w],x)	; 92 D5 10
		bcp	a,([$1000.w],x)	; 72 D5 10 00
		bcp	a,([$10.w],y)	; 91 D5 10

		bcpl	$1000,#2	; 90 14 10 00

		break			; 8B

		bres	$1000,#7	; 72 1f 10 00

		bset	$1000,#1	; 72 12 10 00

loop:		btjf	$1000,#1,loop	; 72 03 10 00 FB

		btjt	$1000,#1,loop	; 72 02 10 00 F6

		call	$1000		; CD 10 00
		call	(x)		; FD
		call	($10,x)		; ED 10
		call	($1000,x)	; DD 10 00
		call	(y)		; 90 FD
		call	($10,y)		; 90 ED 10
		call	($1000,y)	; 90 DD 10 00
		call	[$10.w]		; 92 CD 10
		call	[$1000.w]	; 72 CD 10 00
		call	([$10.w],x)	; 92 DD 10
		call	([$1000.w],x)	; 72 DD 10 00
		call	([$10.w],y)	; 91 DD 10

		callf	$35aa00		; 8D 35 AA 00
		callf	[$2ffc.e]	; 92 8D 2F FC

		callr	PC+$15		; AD 13

		ccf			; 8C

		clr	a		; 4F
		clr	$10		; 3F 10
		clr	$1000		; 72 5F 10 00
		clr	(x)		; 7F
		clr	($10,x)		; 6F 10
		clr	($1000,x)	; 72 4F 10 00
		clr	(y)		; 90 7F
		clr	($10,y)		; 90 6F 10
		clr	($1000,y)	; 90 4F 10 00
		clr	($10,sp)	; 0F 10
		clr	[$10.w]		; 92 3F 10
		clr	[$1000.w]	; 72 3F 10 00
		clr	([$10.w],x)	; 92 6F 10
		clr	([$1000.w],x)	; 72 6F 10 00
		clr	([$10.w],y)	; 91 6F 10

		clr	x		; 5F
		clrw	x		; 5F
		clr	y		; 90 5F
		clrw	y		; 90 5F

		cp	a,#$10		; A1 10
		cp	a,$10		; B1 10
		cp	a,$1000		; C1 10 00
		cp	a,(x)		; F1
		cp	a,($10,x)	; E1 10
		cp	a,($1000,x)	; D1 10 00
		cp	a,(y)		; 90 F1
		cp	a,($10,y)	; 90 E1 10
		cp	a,($1000,y)	; 90 D1 10 00
		cp	a,($10,sp)	; 11 10
		cp	a,[$10.w]	; 92 C1 10
		cp	a,[$1000.w]	; 72 C1 10 00
		cp	a,([$10.w],x)	; 92 D1 10
		cp	a,([$1000.w],x)	; 72 D1 10 00
		cp	a,([$10.w],y)	; 91 D1 10

		cp	x,#$10		; A3 00 10
		cpw	x,#$10		; A3 00 10
		cp	x,$10		; B3 10
		cpw	x,$10		; B3 10
		cp	x,$1000		; C3 10 00
		cpw	x,$1000		; C3 10 00
		cp	x,(y)		; 90 F3
		cpw	x,(y)		; 90 F3
		cp	x,($10,y)	; 90 E3 10
		cpw	x,($10,y)	; 90 E3 10
		cp	x,($1000,y)	; 90 D3 10 00
		cpw	x,($1000,y)	; 90 D3 10 00
		cp	x,($10,sp)	; 13 10
		cpw	x,($10,sp)	; 13 10
		cp	x,[$10.w]	; 92 C3 10
		cpw	x,[$10.w]	; 92 C3 10
		cp	x,[$1000.w]	; 72 C3 10 00
		cpw	x,[$1000.w]	; 72 C3 10 00
		cp	x,([$10.w],y)	; 91 D3 10
		cpw	x,([$10.w],y)	; 91 D3 10

		cp	y,#$10		; 90 A3 00 10
		cpw	y,#$10		; 90 A3 00 10
		cp	y,$10		; 90 B3 10
		cpw	y,$10		; 90 B3 10
		cp	y,$1000		; 90 C3 10 00
		cpw	y,$1000		; 90 C3 10 00
		cp	y,(x)		; F3
		cpw	y,(x)		; F3
		cp	y,($10,x)	; E3 10
		cpw	y,($10,x)	; E3 10
		cp	y,($1000,x)	; D3 10 00
		cpw	y,($1000,x)	; D3 10 00
		cp	y,[$10.w]	; 91 C3 10
		cpw	y,[$10.w]	; 91 C3 10
		cp	y,([$10.w],x)	; 92 D3 10
		cpw	y,([$10.w],x)	; 92 D3 10
		cp	y,([$1000.w],x)	; 72 D3 10 00
		cpw	y,([$1000.w],x)	; 72 D3 10 00

		cpl	a		; 43
		cpl	$10		; 33 10
		cpl	$1000		; 72 53 10 00
		cpl	(x)		; 73
		cpl	($10,x)		; 63 10
		cpl	($1000,x)	; 72 43 10 00
		cpl	(y)		; 90 73
		cpl	($10,y)		; 90 63 10
		cpl	($1000,y)	; 90 43 10 00
		cpl	($10,sp)	; 03 10
		cpl	[$10]		; 92 33 10
		cpl	[$1000.w]	; 72 33 10 00
		cpl	([$10],x)	; 92 63 10
		cpl	([$1000.w],x)	; 72 63 10 00
		cpl	([$10],y)	; 91 63 10

		cpl	x		; 53
		cplw	x		; 53
		cpl	y		; 90 53
		cplw	y		; 90 53

		dec	a		; 4A
		dec	$10		; 3A 10
		dec	$1000		; 72 5A 10 00
		dec	(x)		; 7A
		dec	($10,x)		; 6A 10
		dec	($1000,x)	; 72 4A 10 00
		dec	(y)		; 90 7A
		dec	($10,y)		; 90 6A 10
		dec	($1000,y)	; 90 4A 10 00
		dec	($10,sp)	; 0A 10
		dec	[$10]		; 92 3A 10
		dec	[$1000.w]	; 72 3A 10 00
		dec	([$10],x)	; 92 6A 10
		dec	([$1000.w],x)	; 72 6A 10 00
		dec	([$10],y)	; 91 6A 10

		dec	x		; 5A
		decw	x		; 5A
		dec	y		; 90 5A
		decw	y		; 90 5A

		div	x,a
		div	y,a
		div	x,y
		divw	x,y

		exg	a,xl		; 41
		exg	xl,a		; 41
		exg	a,yl		; 61
		exg	yl,a		; 61
		exg	a,$1000		; 31 10 00
		exg	$1000,a		; 31 10 00
		exg	a,$10		; 31 00 10
		exg	$10,a		; 31 00 10

		exg	x,y		; 51
		exg	y,x		; 51
		exgw	x,y		; 51
		exgw	y,x		; 51

		halt			; 8E

		inc	a		; 4C
		inc	$10		; 3C 10
		inc	$1000		; 72 5C 10 00
		inc	(x)		; 7C
		inc	($10,x)		; 6C 10
		inc	($1000,x)	; 72 4C 10 00
		inc	(y)		; 90 7C
		inc	($10,y)		; 90 6C 10
		inc	($1000,y)	; 90 4C 10 00
		inc	($10,sp)	; 0C 10
		inc	[$10]		; 92 3C 10
		inc	[$1000.w]	; 72 3C 10 00
		inc	([$10],x)	; 92 6C 10
		inc	([$1000.w],x)	; 72 6C 10 00
		inc	([$10],y)	; 91 6C 10

		inc	x		; 5C
		incw	x		; 5C
		inc	y		; 90 5C
		incw	y		; 90 5C

		iret			; 80

		jp	$1000		; CC 10 00
		jp	(x)		; FC
		jp	($10,x)		; EC 10
		jp	($1000,x)	; DC 10 00
		jp	(y)		; 90 FC
		jp	($10,y)		; 90 EC 10
		jp	($1000,y)	; 90 DC 10 00
		jp	[$10.w]		; 92 CC 10
		jp	[$1000.w]	; 72 CC 10 00
		jp	([$10.w],x)	; 92 DC 10
		jp	([$1000.w],x)	; 72 DC 10 00
		jp	([$10.w],y)	; 91 DC 10

		jpf	$2ffffc		; AC 2F FF FC
		jpf	[$2ffc.e]	; 92 AC 2F FC

		jra	PC+$15		; 20 13

		jrc	PC+$15		; 25 13
		jreq	PC+$15		; 27 13
		jrf	PC+$15		; 21 13
		jrh	PC+$15		; 90 29 12
		jrih	PC+$15		; 90 2F 12
		jril	PC+$15		; 90 2E 12
		jrm	PC+$15		; 90 2D 12
		jrmi	PC+$15		; 2B 13
		jrnc	PC+$15		; 24 13
		jrne	PC+$15		; 26 13
		jrnh	PC+$15		; 90 28 12
		jrnm	PC+$15		; 90 2C 12
		jrnv	PC+$15		; 28 13
		jrpl	PC+$15		; 2A 13
		jrsge	PC+$15		; 2E 13
		jrsgt	PC+$15		; 2C 13
		jrsle	PC+$15		; 2D 13
		jrslt	PC+$15		; 2F 13
		jrt	PC+$15		; 20 13
		jruge	PC+$15		; 24 13
		jrugt	PC+$15		; 22 13
		jrule	PC+$15		; 23 13
		jrult	PC+$15		; 25 13
		jrv	PC+$15		; 29 13

		ld	a,#$55		; A6 55
		ld	a,$50		; B6 50
		ld	a,$5000		; C6 50 00
		ld	a,(x)		; F6
		ld	a,($50,x)	; E6 50
		ld	a,($5000,x)	; D6 50 00
		ld	a,(y)		; 90 F6
		ld	a,($50,y)	; 90 E6 50
		ld	a,($5000,y)	; 90 D6 50 00
		ld	a,($50,sp)	; 7B 50
		ld	a,[$50.w]	; 92 C6 50
		ld	a,[$5000.w]	; 72 C6 50 00
		ld	a,([$50.w],x)	; 92 D6 50
		ld	a,([$5000.w],x)	; 72 D6 50 00
		ld	a,([$50.w],y)	; 91 D6 50
		ld	$50,a		; B7 50
		ld	$5000,a		; C7 50 00
		ld	(x),a		; F7
		ld	($50,x),a	; E7 50
		ld	($5000,x),a	; D7 50 00
		ld	(y),a		; 90 F7
		ld	($50,y),a	; 90 E7 50
		ld	($5000,y),a	; 90 D7 50 00
		ld	($50,sp),a	; 6B 50
		ld	[$50.w],a	; 92 C7 50
		ld	[$5000.w],a	; 72 C7 50 00
		ld	([$50.w],x),a	; 92 D7 50
		ld	([$5000.w],x),a	; 72 D7 50 00
		ld	([$50.w],y),a	; 91 D7 50
		ld	xl,a		; 97
		ld	a,xl		; 9F
		ld	yl,a		; 90 97
		ld	a,yl		; 90 9F
		ld	xh,a		; 95
		ld	a,xh		; 9E
		ld	yh,a		; 90 95
		ld	a,yh		; 90 9E

		ldf	a,$500000	; BC 50 00 00
		ldf	a,($500000,x)	; AF 50 00 00
		ldf	a,($500000,y)	; 90 AF 50 00 00
		ldf	a,([$5000.e],x)	; 92 AF 50 00
		ldf	a,([$5000.e],y)	; 91 AF 50 00
		ldf	a,[$5000.e]	; 92 BC 50 00

		ldf	$500000,a	; BD 50 00 00
		ldf	($500000,x),a	; A7 50 00 00
		ldf	($500000,y),a	; 90 A7 50 00 00
		ldf	([$5000.e],x),a	; 92 A7 50 00
		ldf	([$5000.e],y),a	; 91 A7 50 00
		ldf	[$5000.e],a	; 92 BD 50 00

		ldw	x,#$55aa	; AE 55 AA
		ldw	x,$50		; BE 50
		ldw	x,$5000		; CE 50 00
		ldw	x,(x)		; FE
		ldw	x,($50,x)	; EE 50
		ldw	x,($5000,x)	; DE 50 00
		ldw	x,($50,sp)	; 1E 50
		ldw	x,[$50.w]	; 92 CE 50
		ldw	x,[$5000.w]	; 72 CE 50 00
		ldw	x,([$50.w],x)	; 92 DE 50
		ldw	x,([$5000.w],x)	; 72 DE 50 00
		ldw	$50,x		; BF 50
		ldw	$5000,x		; CF 50 00
		ldw	(x),y		; FF
		ldw	($50,x),y	; EF 50
		ldw	($5000,x),y	; DF 50 00
		ldw	($50,sp),x	; 1F 50
		ldw	[$50.w],x	; 92 CF 50
		ldw	[$5000.w],x	; 72 CF 50 00
		ldw	([$50.w],x),y	; 92 DF 50
		ldw	([$5000.w],x),y	; 72 DF 50 00
		ldw	y,#$55aa	; 90 AE 50 00
		ldw	y,$50		; 90 BE 50
		ldw	y,$5000		; 90 CE 50 00
		ldw	y,(y)		; 90 FE
		ldw	y,($50,y)	; 90 EE 50
		ldw	y,($5000,y)	; 90 DE 50 00
		ldw	y,($50,sp)	; 16 50
		ldw	y,[$50.w]	; 91 CE 50
		ldw	y,([$50.w],y)	; 91 DE 50
		ldw	$50,y		; 90 BF 50
		ldw	$5000,y		; 90 CF 50 00
		ldw	(y),x		; 90 FF
		ldw	($50,y),x	; 90 EF 50
		ldw	($5000,y),x	; 90 DF 50 00
		ldw	($50,sp),y	; 17 50
		ldw	[$50.w],y	; 91 CF 50
		ldw	([$50.w],y),x	; 91 DF 50
		ldw	y,x		; 90 93
		ldw	x,y		; 93
		ldw	x,sp		; 96
		ldw	sp,x		; 94
		ldw	y,sp		; 90 96
		ldw	sp,y		; 90 94

		; ld automatically interpreted as ldw due to src/dest arg

		ld	x,#$55aa	; AE 55 AA
		ld	x,$50		; BE 50
		ld	x,$5000		; CE 50 00
		ld	x,(x)		; FE
		ld	x,($50,x)	; EE 50
		ld	x,($5000,x)	; DE 50 00
		ld	x,($50,sp)	; 1E 50
		ld	x,[$50.w]	; 92 CE 50
		ld	x,[$5000.w]	; 72 CE 50 00
		ld	x,([$50.w],x)	; 92 DE 50
		ld	x,([$5000.w],x)	; 72 DE 50 00
		ld	$50,x		; BF 50
		ld	$5000,x		; CF 50 00
		ld	(x),y		; FF
		ld	($50,x),y	; EF 50
		ld	($5000,x),y	; DF 50 00
		ld	($50,sp),x	; 1F 50
		ld	[$50.w],x	; 92 CF 50
		ld	[$5000.w],x	; 72 CF 50 00
		ld	([$50.w],x),y	; 92 DF 50
		ld	([$5000.w],x),y	; 72 DF 50 00
		ld	y,#$55aa	; 90 AE 50 00
		ld	y,$50		; 90 BE 50
		ld	y,$5000		; 90 CE 50 00
		ld	y,(y)		; 90 FE
		ld	y,($50,y)	; 90 EE 50
		ld	y,($5000,y)	; 90 DE 50 00
		ld	y,($50,sp)	; 16 50
		ld	y,[$50.w]	; 91 CE 50
		ld	y,([$50.w],y)	; 91 DE 50
		ld	$50,y		; 90 BF 50
		ld	$5000,y		; 90 CF 50 00
		ld	(y),x		; 90 FF
		ld	($50,y),x	; 90 EF 50
		ld	($5000,y),x	; 90 DF 50 00
		ld	($50,sp),y	; 17 50
		ld	[$50.w],y	; 91 CF 50
		ld	([$50.w],y),x	; 91 DF 50
		ld	y,x		; 90 93
		ld	x,y		; 93
		ld	x,sp		; 96
		ld	sp,x		; 94
		ld	y,sp		; 90 96
		ld	sp,y		; 90 94

		; ld automatically interpreted as mov due to src/dest arg
                ; note that opposed to ld, mov does not modify Z and N flags!

		ld	$80,#$aa	; 35 AA 00 80
		ld	$8000,#$aa	; 35 AA 80 00
		ld	$80,$10		; 45 10 80
		ld	$8000,$10	; 55 00 10 80 00
		ld	$80,$1000	; 55 10 00 00 80
		ld	$8000,$1000	; 55 10 00 80 00

		mov	$80,#$aa	; 35 AA 00 80
		mov	$8000,#$aa	; 35 AA 80 00
		mov	$80,$10		; 45 10 80
		mov	$8000,$10	; 55 00 10 80 00
		mov	$80,$1000	; 55 10 00 00 80
		mov	$8000,$1000	; 55 10 00 80 00

		mul	x,a		; 42
		mul	y,a		; 90 42
	
		neg	a		; 40
		neg	$f5		; 30 F5
		neg	$f5c2		; 72 50 F5 C2
		neg	(x)		; 70
		neg	($f5,x)		; 60 F5
		neg	($f5c2,x)	; 72 40 F5 C2
		neg	(y)		; 90 70
		neg	($f5,y)		; 90 60 F5
		neg	($f5c2,y)	; 90 40 F5 C2
		neg	($f5,sp)	; 00 F5
		neg	[$f5.w]		; 92 30 F5
		neg	[$f5c2.w]	; 72 30 F5 C2
		neg	([$f5],x)	; 92 60 F5
		neg	([$f5c2.w],x)	; 72 60 F5 C2
		neg	([$f5],y)	; 91 60 F5

		neg	x		; 50
		negw	x		; 50
		neg	y		; 90 50
		negw	y		; 90 50

		nop			; 9D

		or	a,#$55		; AA 55
		or	a,$10		; BA 10
		or	a,$1000		; CA 10 00
		or	a,(x)		; FA
		or	a,($10,x)	; EA 10
		or	a,($1000,x)	; DA 10 00
		or	a,(y)		; 90 FA
		or	a,($10,y)	; 90 EA 10
		or	a,($1000,y)	; 90 DA 10 00
		or	a,($10,sp)	; 1A 10
		or	a,[$10.w]	; 92 CA 10
		or	a,[$1000.w]	; 72 CA 10 00
		or	a,([$10.w],x)	; 92 DA 10
		or	a,([$1000.w],x)	; 72 DA 10 00
		or	a,([$10],y)	; 91 DA 10

		pop	a		; 84
		pop	cc		; 86
		pop	$1000		; 32 10 00

		pop	x		; 85
		popw	x		; 85
		pop	y		; 90 85
		popw	y		; 90 85

		push	a		; 88
		push	cc		; 8A
		push	#$10		; 4B 10
		push	$1000		; 3B 10 00

		push	x		; 89
		pushw	x		; 89
		push	y		; 90 89
		pushw	y		; 90 89

		rcf			; 98

		ret			; 81

		retf			; 87

		rim			; 9A

		rlc	a		; 49
		rlc	$10		; 39 10
		rlc	$1000		; 72 59 10 00
		rlc	(x)		; 79
		rlc	($10,x)		; 69 10
		rlc	($1000,x)	; 72 49 10 00
		rlc	(y)		; 90 79
		rlc	($10,y)		; 90 69 10
		rlc	($1000,y)	; 90 49 10 00
		rlc	($10,sp)	; 09 10
		rlc	[$10]		; 92 39 10
		rlc	[$1000.w]	; 72 39 10 00
		rlc	([$10],x)	; 92 69 10
		rlc	([$1000.w],x)	; 72 69 10 00
		rlc	([$10],y)	; 91 69 10

		rlc	x		; 59
		rlcw	x		; 59
		rlc	y		; 90 59
		rlcw	y		; 90 59

		rlwa	x		; 02
		rlwa	x,a		; 02
		rlwa	y		; 90 02
		rlwa	y,a		; 90 02

		rrc	a		; 46
		rrc	$10		; 36 10
		rrc	$1000		; 72 56 10 00
		rrc	(x)		; 76
		rrc	($10,x)		; 66 10
		rrc	($1000,x)	; 72 46 10 00
		rrc	(y)		; 90 76
		rrc	($10,y)		; 90 66 10
		rrc	($1000,y)	; 90 46 10 00
		rrc	($10,sp)	; 06 10
		rrc	[$10]		; 92 36 10
		rrc	[$1000.w]	; 72 36 10 00
		rrc	([$10],x)	; 92 66 10
		rrc	([$1000.w],x)	; 72 66 10 00
		rrc	([$10],y)	; 91 66 10

		rrc	x		; 56
		rrcw	x		; 56
		rrc	y		; 90 56
		rrcw	y		; 90 56

		rrwa	x		; 01
		rrwa	x,a		; 01
		rrwa	y		; 90 01
		rrwa	y,a		; 90 01

		rvf			; 9C

		sbc	a,#$55		; A2 55
		sbc	a,$10		; B2 10
		sbc	a,$1000		; C2 10 00
		sbc	a,(x)		; F2
		sbc	a,($10,x)	; E2 10
		sbc	a,($1000,x)	; D2 10 00
		sbc	a,(y)		; 90 F2
		sbc	a,($10,y)	; 90 E2 10
		sbc	a,($1000,y)	; 90 D2 10 00
		sbc	a,($10,sp)	; 12 10
		sbc	a,[$10.w]	; 92 C2 10
		sbc	a,[$10]		; 92 C2 10 (same, support for 8-bit pointers removed in STM8)
		sbc	a,[>$10.w]	; 72 C2 00 10 (forced long address)
		sbc	a,[$1000.w]	; 72 C2 10 00 (implicit long address)
		sbc	a,([$10.w],x)	; 92 D2 10
		sbc	a,([>$10.w],x)  ; 72 D2 00 10 (forced long address)
		sbc	a,([$1000.w],x)	; 72 D2 10 00 (implicit long address)
		sbc	a,([$10.w],y)	; 91 D2 10

		scf			; 99

		sim			; 9B

		sla	a		; 48
		sla	$15		; 38 15
		sla	$1505		; 72 58 15 05
		sla	(x)		; 78
		sla	($15,x)		; 68 15
		sla	($1505,x)	; 72 48 15 05
		sla	(y)		; 90 78
		sla	($15,y)		; 90 68 15
		sla	($1505,y)	; 90 48 15 05
		sla	($15,sp)	; 08 15
		sla	[$15]		; 92 38 15
		sla	[$1505.w]	; 72 38 15 05
		sla	([$15],x)	; 92 68 15
		sla	([$1505.w],x)	; 72 68 15 05
		sla	([$15],y)	; 91 68 15

		sla	x		; 58
		slaw	x		; 58
		sla	y		; 90 58
		slaw	y		; 90 58

		sll	a		; 48
		sll	$15		; 38 15
		sll	$1505		; 72 58 15 05
		sll	(x)		; 78
		sll	($15,x)		; 68 15
		sll	($1505,x)	; 72 48 15 05
		sll	(y)		; 90 78
		sll	($15,y)		; 90 68 15
		sll	($1505,y)	; 90 48 15 05
		sll	($15,sp)	; 08 15
		sll	[$15]		; 92 38 15
		sll	[$1505.w]	; 72 38 15 05
		sll	([$15],x)	; 92 68 15
		sll	([$1505.w],x)	; 72 68 15 05
		sll	([$15],y)	; 91 68 15

		sll	x		; 58
		sllw	x		; 58
		sll	y		; 90 58
		sllw	y		; 90 58

		sra	a		; 47
		sra	$15		; 37 15
		sra	$1505		; 72 57 15 05
		sra	(x)		; 77
		sra	($15,x)		; 67 15
		sra	($1505,x)	; 72 47 15 05
		sra	(y)		; 90 77
		sra	($15,y)		; 90 67 15
		sra	($1505,y)	; 90 47 15 05
		sra	($15,sp)	; 07 15
		sra	[$15]		; 92 37 15
		sra	[$1505.w]	; 72 37 15 05
		sra	([$15],x)	; 92 67 15
		sra	([$1505.w],x)	; 72 67 15 05
		sra	([$15],y)	; 91 67 15

		sra	x		; 57
		sraw	x		; 57
		sra	y		; 90 57
		sraw	y		; 90 57

		srl	a		; 44
		srl	$15		; 34 15
		srl	$1505		; 72 54 15 05
		srl	(x)		; 74
		srl	($15,x)		; 64 15
		srl	($1505,x)	; 72 44 15 05
		srl	(y)		; 90 74
		srl	($15,y)		; 90 64 15
		srl	($1505,y)	; 90 44 15 05
		srl	($15,sp)	; 04 15
		srl	[$15]		; 92 34 15
		srl	[$1505.w]	; 72 34 15 05
		srl	([$15],x)	; 92 64 15
		srl	([$1505.w],x)	; 72 64 15 05
		srl	([$15],y)	; 91 64 15

		srl	x		; 54
		srlw	x		; 54
		srl	y		; 90 54
		srlw	y		; 90 54

		sub	a,#$55		; A0 55
		sub	a,$10		; B0 10
		sub	a,$1000		; C0 10 00
		sub	a,(x)		; F0
		sub	a,($10,x)	; E0 10
		sub	a,($1000,x)	; D0 10 00
		sub	a,(y)		; 90 F0
		sub	a,($10,y)	; 90 E0 10
		sub	a,($1000,y)	; 90 D0 10 00
		sub	a,($10,sp)	; 10 10
		sub	a,[$10.w]	; 92 C0 10
		sub	a,[$10]		; 92 C0 10 (same, support for 8-bit pointers removed in STM8)
		sub	a,[>$10.w]	; 72 C0 00 10 (forced long address)
		sub	a,[$1000.w]	; 72 C0 10 00 (implicit long address)
		sub	a,([$10.w],x)	; 92 D0 10
		sub	a,([>$10.w],x)  ; 72 D0 00 10 (forced long address)
		sub	a,([$1000.w],x)	; 72 D0 10 00 (implicit long address)
		sub	a,([$10.w],y)	; 91 D0 10

		sub	x,#$5500	; 1D 55 00
		subw	x,#$5500	; 1D 55 00
		sub	x,$1000		; 72 B0 10 00
		subw	x,$1000		; 72 B0 10 00
		sub	x,($10,sp)	; 72 F0 10
		subw	x,($10,sp)	; 72 F0 10
		sub	y,#$5500	; 72 A2 55 00
		subw	y,#$5500	; 72 A2 55 00
		sub	y,$1000		; 72 B2 10 00
		subw	y,$1000		; 72 B2 10 00
		sub	y,($10,sp)	; 72 F2 10
		subw	y,($10,sp)	; 72 F2 10
		sub	sp,#$9		; 52 09
                subw	sp,#$9		; 52 09

		swap	a		; 4E
		swap	$15		; 3E 15
		swap	$1505		; 72 5E 15 05
		swap	(x)		; 7E
		swap	($15,x)		; 6E 15
		swap	($1505,x)	; 72 4E 15 05
		swap	(y)		; 90 7E
		swap	($15,y)		; 90 6E 15
		swap	($1505,y)	; 90 4E 15 05
		swap	($15,sp)	; 0E 15
		swap	[$15]		; 92 3E 15
		swap	[$1505.w]	; 72 3E 15 05
		swap	([$15],x)	; 92 6E 15
		swap	([$1505.w],x)	; 72 6E 15 05
		swap	([$15],y)	; 91 6E 15

		swap	x		; 5E
		swapw	x		; 5E
		swap	y		; 90 5E
		swapw	y		; 90 5E

		tnz	a		; 4D
		tnz	$15		; 3D 15
		tnz	$1505		; 72 5D 15 05
		tnz	(x)		; 7D
		tnz	($15,x)		; 6D 15
		tnz	($1505,x)	; 72 4D 15 05
		tnz	(y)		; 90 7D
		tnz	($15,y)		; 90 6D 15
		tnz	($1505,y)	; 90 4D 15 05
		tnz	($15,sp)	; 0D 15
		tnz	[$15]		; 92 3D 15
		tnz	[$1505.w]	; 72 3D 15 05
		tnz	([$15],x)	; 92 6D 15
		tnz	([$1505.w],x)	; 72 6D 15 05
		tnz	([$15],y)	; 91 6D 15

		tnz	x		; 5D
		tnzw	x		; 5D
		tnz	y		; 90 5D
		tnzw	y		; 90 5D

		trap			; 83

		wfe			; 72 8F

		wfi			; 8F

		xor	a,#$55		; A8 55
		xor	a,$10		; B8 10
		xor	a,$1000		; C8 10 00
		xor	a,(x)		; F8
		xor	a,($10,x)	; E8 10
		xor	a,($1000,x)	; D8 10 00
		xor	a,(y)		; 90 F8
		xor	a,($10,y)	; 90 E8 10
		xor	a,($1000,y)	; 90 D8 10 00
		xor	a,($10,sp)	; 18 10
		xor	a,[$10.w]	; 92 C8 10
		xor	a,[$1000.w]	; 72 C8 10 00
		xor	a,([$10.w],x)	; 92 D8 10
		xor	a,([$1000.w],x)	; 72 D8 10 00
		xor	a,([$10],y)	; 91 D8 10

mybit		bit	$1234,6
		bset	$1234,6
		bset	mybit
		btjt	$1234,6,PC + 20
		btjt	mybit,PC + 20