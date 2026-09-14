	cpu		sh7615
	page		0
	dsp		on

	; CPU instructions cannot be executed conditionally

	add		r1,r2		; 321C
	expect	1364
	dct add		r1,r2
	endexpect

	; -------------------------
	; No parallel moves:

	; DSP Data Transfer Instructions

	nopx				; F000
	movx		@r4,x0		; F004
	movx		@r4,x1		; F084
	movx		@r5,x0		; F204
	movx		@r5,x1		; F284
	movx		@r4+,x0		; F008
	movx		@r4+,x1		; F088
	movx		@r5+,x0		; F208
	movx		@r5+,x1		; F288
	movx		@r4+r8,x0	; F00C
	movx		@r4+r8,x1	; F08C
	movx		@r5+r8,x0	; F20C
	movx		@r5+r8,x1	; F28C
	movx		a0,@r4		; F024
	movx		a1,@r4		; F0A4
	movx		a0,@r5		; F224
	movx		a1,@r5		; F2A4
	movx		a0,@r4+		; F028
	movx		a1,@r4+		; F0A8
	movx		a0,@r5+		; F228
	movx		a1,@r5+		; F2A8
	movx		a0,@r4+r8	; F02C
	movx		a1,@r4+r8	; F0AC
	movx		a0,@r5+r8	; F22C
	movx		a1,@r5+r8	; F2AC

	nopy				; F000
	movy		@r6,y0		; F001
	movy		@r6,y1		; F041
	movy		@r7,y0		; F101
	movy		@r7,y1		; F141
	movy		@r6+,y0		; F002
	movy		@r6+,y1		; F042
	movy		@r7+,y0		; F102
	movy		@r7+,y1		; F142
	movy		@r6+r9,y0	; F003
	movy		@r6+r9,y1	; F043
	movy		@r7+r9,y0	; F103
	movy		@r7+r9,y1	; F143
	movy		a0,@r6		; F011
	movy		a1,@r6		; F051
	movy		a0,@r7		; F111
	movy		a1,@r7		; F151
	movy		a0,@r6+		; F012
	movy		a1,@r6+		; F052
	movy		a0,@r7+		; F112
	movy		a1,@r7+		; F152
	movy		a0,@r6+r9	; F013
	movy		a1,@r6+r9	; F053
	movy		a0,@r7+r9	; F113
	movy		a1,@r7+r9	; F153

	movs.w		@-r2,a0		; F670
	movs.w		@r3,a1		; F754
	movs.w		@r4+,x0		; F488
	movs.w		@r5+r8,x1	; F59C
	movs.w		y0,@-r2		; F6A1
	movs.w		y1,@r3		; F7B5
	movs.w		m0,@r4+		; F4C9
	movs.w		m1,@r5+r8	; F5ED
	movs.l		@-r2,a0g	; F6F2
	movs.l		@r3,a1g		; F7D6
	movs.l		@r4+,a0		; F47A
	movs.l		@r5+r8,a1	; F55E
	movs.l		x0,@-r2		; F683
	movs.l		x1,@r3		; F797
	movs.l		y0,@r4+		; F4AB
	movs.l		y1,@r5+r8	; F5BF

	; DSP ALU Arithmetic Operation Instructions

	pabs		x0,a0		; F800 8807
	pabs		x0,a1		; F800 8805
	pabs		x0,x0		; F800 8808
	pabs		x0,x1		; F800 8809
	pabs		x0,y0		; F800 880A
	pabs		x0,y1		; F800 880B
	pabs		x0,m0		; F800 880C
	pabs		x0,m1		; F800 880E
	pabs		x1,a0		; F800 8847
	pabs		a0,a0		; F800 8887
	pabs		a1,a0		; F800 88C7
	pabs		y0,a0		; F800 A807
	pabs		y1,a0		; F800 A817
	pabs		m0,a0		; F800 A827
	pabs		m1,a0		; F800 A837
	expect		1364,1364
	dcf pabs	x0,a0
	dct pabs	y0,a0
	endexpect

	padd		x0,y0,a0	; F800 B107
	padd		x0,y0,a1	; F800 B105
	padd		x0,y0,x0	; F800 B108
	padd		x0,y0,x1	; F800 B109
	padd		x0,y0,y0	; F800 B10A
	padd		x0,y0,y1	; F800 B10B
	padd		x0,y0,m0	; F800 B10C
	padd		x0,y0,m1	; F800 B10E
	padd		x0,y1,a0	; F800 B117
	padd		x0,m0,a0	; F800 B127
	padd		x0,m1,a0	; F800 B137
	padd		x1,y0,a0	; F800 B147
	padd		a0,y0,a0	; F800 B187
	padd		a1,y0,a0	; F800 B1C7
	dct padd	x0,y0,a0	; F800 B207
	dcf padd	x0,y0,a0	; F800 B307

	paddc		x0,y0,a0	; F800 B007
	paddc		x0,y0,a1	; F800 B005
	paddc		x0,y0,x0	; F800 B008
	paddc		x0,y0,x1	; F800 B009
	paddc		x0,y0,y0	; F800 B00A
	paddc		x0,y0,y1	; F800 B00B
	paddc		x0,y0,m0	; F800 B00C
	paddc		x0,y0,m1	; F800 B00E
	paddc		x0,y1,a0	; F800 B017
	paddc		x0,m0,a0	; F800 B027
	paddc		x0,m1,a0	; F800 B037
	paddc		x1,y0,a0	; F800 B047
	paddc		a0,y0,a0	; F800 B087
	paddc		a1,y0,a0	; F800 B0C7
	expect		1364,1364
	dct paddc	x0,y0,a0
	dcf paddc	x0,y0,a0
	endexpect

	pclr		a0		; F800 8D07
	pclr		a1		; F800 8D05
	pclr		x0		; F800 8D08
	pclr		x1		; F800 8D09
	pclr		y0		; F800 8D0A
	pclr		y1		; F800 8D0B
	pclr		m0		; F800 8D0C
	pclr		m1		; F800 8D0E
	dct pclr	a0		; F800 8E07
	dcf pclr	a1		; F800 8F05

	pcmp		x0,y0		; F800 8400
	pcmp		x0,y1		; F800 8410
	pcmp		x0,m0		; F800 8420
	pcmp		x0,m1		; F800 8430
	pcmp		x1,y0		; F800 8440
	pcmp		a0,y0		; F800 8480
	pcmp		a1,y0		; F800 84C0
	expect		1364,1364
	dct pcmp	x0,y0
	dcf pcmp	x0,y0
	endexpect

	pcopy		x0,a0		; F800 D907
	pcopy		x0,a1		; F800 D905
	pcopy		x0,x0		; F800 D908
	pcopy		x0,x1		; F800 D909
	pcopy		x0,y0		; F800 D90A
	pcopy		x0,y1		; F800 D90B
	pcopy		x0,m0		; F800 D90C
	pcopy		x0,m1		; F800 D90E
	pcopy		x1,a0		; F800 D947
	pcopy		a0,a0		; F800 D987
	pcopy		a1,a0		; F800 D9C7
	pcopy		y0,a0		; F800 F907
	pcopy		y1,a0		; F800 F917
	pcopy		m0,a0		; F800 F927
	pcopy		m1,a0		; F800 F937
	dcf pcopy	x0,a0		; F800 DB07
	dct pcopy	y0,a0		; F800 FA07

	pneg		x0,a0		; F800 C907
	pneg		x0,a1		; F800 C905
	pneg		x0,x0		; F800 C908
	pneg		x0,x1		; F800 C909
	pneg		x0,y0		; F800 C90A
	pneg		x0,y1		; F800 C90B
	pneg		x0,m0		; F800 C90C
	pneg		x0,m1		; F800 C90E
	pneg		x1,a0		; F800 C947
	pneg		a0,a0		; F800 C987
	pneg		a1,a0		; F800 C9C7
	pneg		y0,a0		; F800 E907
	pneg		y1,a0		; F800 E917
	pneg		m0,a0		; F800 E927
	pneg		m1,a0		; F800 E937
	dcf pneg	x0,a0		; F800 CB07
	dct pneg	y0,a0		; F800 EA07

	psub		x0,y0,a0	; F800 A107
	psub		x0,y0,a1	; F800 A105
	psub		x0,y0,x0	; F800 A108
	psub		x0,y0,x1	; F800 A109
	psub		x0,y0,y0	; F800 A10A
	psub		x0,y0,y1	; F800 A10B
	psub		x0,y0,m0	; F800 A10C
	psub		x0,y0,m1	; F800 A10E
	psub		x0,y1,a0	; F800 A117
	psub		x0,m0,a0	; F800 A127
	psub		x0,m1,a0	; F800 A137
	psub		x1,y0,a0	; F800 A147
	psub		a0,y0,a0	; F800 A187
	psub		a1,y0,a0	; F800 A1C7
	dct psub	x0,y0,a0	; F800 A207
	dcf psub	x0,y0,a0	; F800 A307

	psubc		x0,y0,a0	; F800 A007
	psubc		x0,y0,a1	; F800 A005
	psubc		x0,y0,x0	; F800 A008
	psubc		x0,y0,x1	; F800 A009
	psubc		x0,y0,y0	; F800 A00A
	psubc		x0,y0,y1	; F800 A00B
	psubc		x0,y0,m0	; F800 A00C
	psubc		x0,y0,m1	; F800 A00E
	psubc		x0,y1,a0	; F800 A017
	psubc		x0,m0,a0	; F800 A027
	psubc		x0,m1,a0	; F800 A037
	psubc		x1,y0,a0	; F800 A047
	psubc		a0,y0,a0	; F800 A087
	psubc		a1,y0,a0	; F800 A0C7
	expect		1364,1364
	dct psubc	x0,y0,a0
	dcf psubc	x0,y0,a0
	endexpect

	pdec		x0,a0		; F800 8907
	pdec		x0,a1		; F800 8905
	pdec		x0,x0		; F800 8908
	pdec		x0,x1		; F800 8909
	pdec		x0,y0		; F800 890A
	pdec		x0,y1		; F800 890B
	pdec		x0,m0		; F800 890C
	pdec		x0,m1		; F800 890E
	pdec		x1,a0		; F800 8947
	pdec		a0,a0		; F800 8987
	pdec		a1,a0		; F800 89C7
	pdec		y0,a0		; F800 A907
	pdec		y1,a0		; F800 A917
	pdec		m0,a0		; F800 A927
	pdec		m1,a0		; F800 A937
	dcf pdec	x0,a0		; F800 8B07
	dct pdec	y0,a0		; F800 AA07

	pinc		x0,a0		; F800 9907
	pinc		x0,a1		; F800 9905
	pinc		x0,x0		; F800 9908
	pinc		x0,x1		; F800 9909
	pinc		x0,y0		; F800 990A
	pinc		x0,y1		; F800 990B
	pinc		x0,m0		; F800 990C
	pinc		x0,m1		; F800 990E
	pinc		x1,a0		; F800 9947
	pinc		a0,a0		; F800 9987
	pinc		a1,a0		; F800 99C7
	pinc		y0,a0		; F800 B907
	pinc		y1,a0		; F800 B917
	pinc		m0,a0		; F800 B927
	pinc		m1,a0		; F800 B937
	dcf pinc	x0,a0		; F800 9B07
	dct pinc	y0,a0		; F800 BA07

	pdmsb		x0,a0		; F800 9D07
	pdmsb		x0,a1		; F800 9D05
	pdmsb		x0,x0		; F800 9D08
	pdmsb		x0,x1		; F800 9D09
	pdmsb		x0,y0		; F800 9D0A
	pdmsb		x0,y1		; F800 9D0B
	pdmsb		x0,m0		; F800 9D0C
	pdmsb		x0,m1		; F800 9D0E
	pdmsb		x1,a0		; F800 9D47
	pdmsb		a0,a0		; F800 9D87
	pdmsb		a1,a0		; F800 9DC7
	pdmsb		y0,a0		; F800 BD07
	pdmsb		y1,a0		; F800 BD17
	pdmsb		m0,a0		; F800 BD27
	pdmsb		m1,a0		; F800 BD37
	dcf pdmsb	x0,a0		; F800 9F07
	dct pdmsb	y0,a0		; F800 BE07

	prnd		x0,a0		; F800 9807
	prnd		x0,a1		; F800 9805
	prnd		x0,x0		; F800 9808
	prnd		x0,x1		; F800 9809
	prnd		x0,y0		; F800 980A
	prnd		x0,y1		; F800 980B
	prnd		x0,m0		; F800 980C
	prnd		x0,m1		; F800 980E
	prnd		x1,a0		; F800 9847
	prnd		a0,a0		; F800 9887
	prnd		a1,a0		; F800 98C7
	prnd		y0,a0		; F800 B807
	prnd		y1,a0		; F800 B817
	prnd		m0,a0		; F800 B827
	prnd		m1,a0		; F800 B837
	expect		1364,1364
	dcf prnd	x0,a0
	dct prnd	y0,a0
	endexpect

	; DSP ALU Logical Operation Instructions

	pand		x0,y0,a0	; F800 9507
	pand		x0,y0,a1	; F800 9505
	pand		x0,y0,x0	; F800 9508
	pand		x0,y0,x1	; F800 9509
	pand		x0,y0,y0	; F800 950A
	pand		x0,y0,y1	; F800 950B
	pand		x0,y0,m0	; F800 950C
	pand		x0,y0,m1	; F800 950E
	pand		x0,y1,a0	; F800 9517
	pand		x0,m0,a0	; F800 9527
	pand		x0,m1,a0	; F800 9537
	pand		x1,y0,a0	; F800 9547
	pand		a0,y0,a0	; F800 9587
	pand		a1,y0,a0	; F800 95C7
	dct pand	x0,y0,a0	; F800 9607
	dcf pand	x0,y0,a0	; F800 9707

	por		x0,y0,a0	; F800 B507
	por		x0,y0,a1	; F800 B505
	por		x0,y0,x0	; F800 B508
	por		x0,y0,x1	; F800 B509
	por		x0,y0,y0	; F800 B50A
	por		x0,y0,y1	; F800 B50B
	por		x0,y0,m0	; F800 B50C
	por		x0,y0,m1	; F800 B50E
	por		x0,y1,a0	; F800 B517
	por		x0,m0,a0	; F800 B527
	por		x0,m1,a0	; F800 B537
	por		x1,y0,a0	; F800 B547
	por		a0,y0,a0	; F800 B587
	por		a1,y0,a0	; F800 B5C7
	dct por		x0,y0,a0	; F800 B607
	dcf por		x0,y0,a0	; F800 B707

	pxor		x0,y0,a0	; F800 A507
	pxor		x0,y0,a1	; F800 A505
	pxor		x0,y0,x0	; F800 A508
	pxor		x0,y0,x1	; F800 A509
	pxor		x0,y0,y0	; F800 A50A
	pxor		x0,y0,y1	; F800 A50B
	pxor		x0,y0,m0	; F800 A50C
	pxor		x0,y0,m1	; F800 A50E
	pxor		x0,y1,a0	; F800 A517
	pxor		x0,m0,a0	; F800 A527
	pxor		x0,m1,a0	; F800 A537
	pxor		x1,y0,a0	; F800 A547
	pxor		a0,y0,a0	; F800 A587
	pxor		a1,y0,a0	; F800 A5C7
	dct pxor	x0,y0,a0	; F800 A607
	dcf pxor	x0,y0,a0	; F800 A707

	; DSP Fixed Decimal Point Multiplication Instructions

	pmuls		x0,y0,m0	; F800 4000
	pmuls		x0,y0,m1	; F800 4004
	pmuls		x0,y0,a0	; F800 4008
	pmuls		x0,y0,a1	; F800 400C
	pmuls		x0,y1,m0	; F800 4100
	pmuls		x0,x0,m0	; F800 4200
	pmuls		x0,a1,m0	; F800 4300
	pmuls		x1,y0,m0	; F800 4400
	pmuls		y0,y0,m0	; F800 4800
	pmuls		a1,y0,m0	; F800 4C00

	; DSP Shift Operation Instructions

	psha		x0,y0,a0	; F800 9107
	psha		x0,y0,a1	; F800 9105
	psha		x0,y0,x0	; F800 9108
	psha		x0,y0,x1	; F800 9109
	psha		x0,y0,y0	; F800 910A
	psha		x0,y0,y1	; F800 910B
	psha		x0,y0,m0	; F800 910C
	psha		x0,y0,m1	; F800 910E
	psha		x0,y1,a0	; F800 9117
	psha		x0,m0,a0	; F800 9127
	psha		x0,m1,a0	; F800 9137
	psha		x1,y0,a0	; F800 9147
	psha		a0,y0,a0	; F800 9187
	psha		a1,y0,a0	; F800 91C7
	dct psha	x0,y0,a0	; F800 9207
	dcf psha	x0,y0,a0	; F800 9307
	psha		#23,a0		; F800 0177
	psha		#-23,a0		; F800 0697
	expect		1364,1364
	dct psha	#-23,a0
	dcf psha	#-23,a0
	endexpect

	pshl		x0,y0,a0	; F800 8107
	pshl		x0,y0,a1	; F800 8105
	pshl		x0,y0,x0	; F800 8108
	pshl		x0,y0,x1	; F800 8109
	pshl		x0,y0,y0	; F800 810A
	pshl		x0,y0,y1	; F800 810B
	pshl		x0,y0,m0	; F800 810C
	pshl		x0,y0,m1	; F800 810E
	pshl		x0,y1,a0	; F800 8117
	pshl		x0,m0,a0	; F800 8127
	pshl		x0,m1,a0	; F800 8137
	pshl		x1,y0,a0	; F800 8147
	pshl		a0,y0,a0	; F800 8187
	pshl		a1,y0,a0	; F800 81C7
	dct pshl	x0,y0,a0	; F800 8207
	dcf pshl	x0,y0,a0	; F800 8307
	pshl		#23,a0		; F800 1177
	pshl		#-23,a0		; F800 1697
	expect		1364,1364
	dct pshl	#-23,a0
	dcf pshl	#-23,a0
	endexpect

	; DSP System Control Instructions

	plds		a0,mach		; F800 ED07
	plds		a1,macl		; F800 FD05
	dct plds	x0,mach		; F800 EE08
	dct plds	x1,macl		; F800 FE09
	dcf plds	y0,mach		; F800 EF0A
	dcf plds	y1,macl		; F800 FF0B
	psts		mach,m0		; F800 CD0C
	psts		macl,m1		; F800 DD0E
	dct psts	mach,a0		; F800 CE07
	dct psts	macl,a1		; F800 DE05
	dcf psts	mach,x0		; F800 CF08
	dcf psts	macl,x1		; F800 DF09

	; -------------------------
	; Parallel multiply add/subtract:

	; Iterate Dz->Du in PADD:
	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,y0,m0	; (R) F800 7002
	padd		x0,y0,a1	; F800 B105
+	pmuls		x0,y0,m0	; (R) F800 7003
	padd		x0,y0,x0	; F800 B108
+	pmuls		x0,y0,m0	; (R) F800 7000
	padd		x0,y0,x1	; F800 B109
	expect		1355
+	pmuls		x0,y0,m0	; X1 is not part of Du register set
	endexpect
	padd		x0,y0,y0	; F800 B10A
+	pmuls		x0,y0,m0	; (R) F800 7001
	padd		x0,y0,y1	; F800 B10B
	expect		1355
+	pmuls		x0,y0,m0	; Y1 is not part of Du register set
	endexpect
	padd		x0,y0,m0	; F800 B10C
	expect		1355
+	pmuls		x0,y0,m0	; M0 is not part of Du register set
	endexpect
	padd		x0,y0,m1	; F800 B10E
	expect		1355
+	pmuls		x0,y0,m0	; M1 is not part of Du register set
	endexpect
	; Iterate Sx in PADD:
	padd		x1,y0,a0	; F800 B147
+	pmuls		x0,y0,m0	; (R) F800 7042
	padd		a0,y0,a0	; F800 B187
+	pmuls		x0,y0,m0	; (R) F800 7082
	padd		a1,y0,a0	; F800 B1C7
+	pmuls		x0,y0,m0	; (R) F800 70C2
	; Iterate Sy in PADD:
	padd		x0,y1,a0	; F800 B117
+	pmuls		x0,y0,m0	; (R) F800 7012
	padd		x0,m0,a0	; F800 B127
+	pmuls		x0,y0,m0	; (R) F800 7022
	padd		x0,m1,a0	; F800 B137
+	pmuls		x0,y0,m0	; (R) F800 7032
	; Iterate Se in PMULS:
	padd		x0,y0,a0	; F800 B107
+	pmuls		x1,y0,m0	; (R) F800 7402
	padd		x0,y0,a0	; F800 B107
+	pmuls		y0,y0,m0	; (R) F800 7802
	padd		x0,y0,a0	; F800 B107
+	pmuls		a1,y0,m0	; (R) F800 7C02
	; Iterate Sf in PMULS:
	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,y1,m0	; (R) F800 7102
	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,x0,m0	; (R) F800 7202
	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,a1,m0	; (R) F800 7302
	; Iterate Dg in PMULS:
	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,y0,m1	; (R) F800 7006
	expect		141
	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,y0,a0	; (R) F800 700A
	endexpect
	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,y0,a1	; (R) F800 700E

	; No PMULS in parallel with conditional PADD:
	dct padd	x0,y0,a0	; F800 B207
	expect		1356
+	pmuls		x0,y0,m0
	endexpect

	; No real value doing all this again for PSUB+PMULS:
	psub		x0,y0,a0	; F800 A107
+	pmuls		x0,y0,m0	; (R) F800 6002

	; -------------------------
	; Parallel NOP:

	padd		x0,y0,a0	; F800 B107
+	nopx				; F800 B107

	padd		x0,y0,a0	; F800 B107
+	nopy				; F800 B107

	padd		x0,y0,a0	; F800 B107
+	nopx				; F800 B107
+	nopy				; F800 B107

	padd		x0,y0,a0	; F800 B107
+	nopy				; F800 B107
+	nopx				; F800 B107

	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,y0,m1	; (R) F800 7006
+	nopx				; (R) F800 7006

	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,y0,m1	; (R) F800 7006
+	nopy				; (R) F800 7006

	padd		x0,y0,a0	; F800 B107
+	pmuls		x0,y0,m1	; (R) F800 7006
+	nopx				; (R) F800 7006
+	nopy				; (R) F800 7006

	padd		x0,y0,a0
+	nopx
	expect		1356
+	nopx
	endexpect

	; -------------------------
	; Start combining, instruction order should not matter:

	; ADD + X move
	padd		x0,y0,a0
+	movx		@r4,x0		; F804 B107

	movx		@r4,x0
+	padd		x0,y0,a0	; F804 B107

	; ADD + Y move
	padd		x0,y0,a0
+	movy		@r6,y0		; F801 B107

	movy		@r6,y0
+	padd		x0,y0,a0	; F801 B107

	; ADD + MOVX + MOVY
	padd		x0,y0,a0
+	movx		@r4,x0
+	movy		@r6,y0		; F805 B107

	padd		x0,y0,a0
+	movy		@r6,y0
+	movx		@r4,x0		; F805 B107

	movx		@r4,x0
+	padd		x0,y0,a0
+	movy		@r6,y0		; F805 B107

	movx		@r4,x0
+	movy		@r6,y0
+	padd		x0,y0,a0	; F805 B107

	movy		@r6,y0
+	padd		x0,y0,a0
+	movx		@r4,x0		; F805 B107

	movy		@r6,y0
+	movx		@r4,x0
+	padd		x0,y0,a0	; F805 B107

	; MUL + MOVX
	pmuls		x0,y0,m0
+	movx		@r4,x0		; F804 4000

	movx		@r4,x0
+	pmuls		x0,y0,m0	; F804 4000

	; MUL + MOVY
	pmuls		x0,y0,m0
+	movy		@r6,y0		; F801 4000

	movy		@r6,y0
+	pmuls		x0,y0,m0	; F801 4000

	; MUL + MOVX + MOVY
	pmuls		x0,y0,m0
+	movx		@r4,x0
+	movy		@r6,y0		; F805 4000

	pmuls		x0,y0,m0
+	movy		@r6,y0
+	movx		@r4,x0		; F805 4000

	movx		@r4,x0
+	pmuls		x0,y0,m0
+	movy		@r6,y0		; F805 4000

	movx		@r4,x0
+	movy		@r6,y0
+	pmuls		x0,y0,m0	; F805 4000

	movy		@r6,y0
+	pmuls		x0,y0,m0
+	movx		@r4,x0		; F805 4000

	movy		@r6,y0
+	movx		@r4,x0
+	pmuls		x0,y0,m0	; F805 4000

	; ADD + MUL
	padd		x0,y0,a0
+	pmuls		x0,y0,m0	; F800 7002

	pmuls		x0,y0,m0
+	padd		x0,y0,a0	; F800 7002

	; ADD + MUL + MOVX
	padd		x0,y0,a0
+	pmuls		x0,y0,m0
+	movx		@r4,x0		; F804 7002

	padd		x0,y0,a0
+	movx		@r4,x0
+	pmuls		x0,y0,m0	; F804 7002

	pmuls		x0,y0,m0
+	padd		x0,y0,a0
+	movx		@r4,x0		; F804 7002

	pmuls		x0,y0,m0
+	movx		@r4,x0
+	padd		x0,y0,a0	; F804 7002

	movx		@r4,x0
+	padd		x0,y0,a0
+	pmuls		x0,y0,m0	; F804 7002

	movx		@r4,x0
+	pmuls		x0,y0,m0
+	padd		x0,y0,a0	; F804 7002

	; ADD + MUL + MOVY
	padd		x0,y0,a0
+	pmuls		x0,y0,m0
+	movy		@r6,y0		; F801 7002

	padd		x0,y0,a0
+	movy		@r6,y0
+	pmuls		x0,y0,m0	; F801 7002

	pmuls		x0,y0,m0
+	padd		x0,y0,a0
+	movy		@r6,y0		; F801 7002

	pmuls		x0,y0,m0
+	movy		@r6,y0
+	padd		x0,y0,a0	; F801 7002

	movy		@r6,y0
+	padd		x0,y0,a0
+	pmuls		x0,y0,m0	; F801 7002

	movy		@r6,y0
+	pmuls		x0,y0,m0
+	padd		x0,y0,a0	; F801 7002

	; ADD + MUL + MOVX + MOVY
	padd		x0,y0,a0
+	pmuls		x0,y0,m0
+	movx		@r4,x0
+	movy		@r6,y0		; F805 7002

	padd		x0,y0,a0
+	pmuls		x0,y0,m0
+	movy		@r6,y0
+	movx		@r4,x0		; F805 7002

	padd		x0,y0,a0
+	movx		@r4,x0
+	pmuls		x0,y0,m0
+	movy		@r6,y0		; F805 7002

	padd		x0,y0,a0
+	movx		@r4,x0
+	movy		@r6,y0
+	pmuls		x0,y0,m0	; F805 7002

	padd		x0,y0,a0
+	movy		@r6,y0
+	pmuls		x0,y0,m0
+	movx		@r4,x0		; F805 7002

	padd		x0,y0,a0
+	movy		@r6,y0
+	movx		@r4,x0
+	pmuls		x0,y0,m0	; F805 7002

	pmuls		x0,y0,m0
+	padd		x0,y0,a0
+	movx		@r4,x0
+	movy		@r6,y0		; F805 7002

	pmuls		x0,y0,m0
+	padd		x0,y0,a0
+	movy		@r6,y0
+	movx		@r4,x0		; F805 7002

	pmuls		x0,y0,m0
+	movx		@r4,x0
+	padd		x0,y0,a0
+	movy		@r6,y0		; F805 7002

	pmuls		x0,y0,m0
+	movx		@r4,x0
+	movy		@r6,y0
+	padd		x0,y0,a0	; F805 7002

	pmuls		x0,y0,m0
+	movy		@r6,y0
+	padd		x0,y0,a0
+	movx		@r4,x0		; F805 7002

	pmuls		x0,y0,m0
+	movy		@r6,y0
+	movx		@r4,x0
+	padd		x0,y0,a0	; F805 7002

	movx		@r4,x0
+	padd		x0,y0,a0
+	pmuls		x0,y0,m0
+	movy		@r6,y0		; F805 7002

	movx		@r4,x0
+	padd		x0,y0,a0
+	movy		@r6,y0
+	pmuls		x0,y0,m0	; F805 7002

	movx		@r4,x0
+	pmuls		x0,y0,m0
+	padd		x0,y0,a0
+	movy		@r6,y0		; F805 7002

	movx		@r4,x0
+	pmuls		x0,y0,m0
+	movy		@r6,y0
+	padd		x0,y0,a0	; F805 7002

	movx		@r4,x0
+	movy		@r6,y0
+	padd		x0,y0,a0
+	pmuls		x0,y0,m0	; F805 7002

	movx		@r4,x0
+	movy		@r6,y0
+	pmuls		x0,y0,m0
+	padd		x0,y0,a0	; F805 7002

	movy		@r6,y0
+	padd		x0,y0,a0
+	pmuls		x0,y0,m0
+	movx		@r4,x0		; F805 7002

	movy		@r6,y0
+	padd		x0,y0,a0
+	movx		@r4,x0
+	pmuls		x0,y0,m0	; F805 7002

	movy		@r6,y0
+	pmuls		x0,y0,m0
+	padd		x0,y0,a0
+	movx		@r4,x0		; F805 7002

	movy		@r6,y0
+	pmuls		x0,y0,m0
+	movx		@r4,x0
+	padd		x0,y0,a0	; F805 7002

	movy		@r6,y0
+	movx		@r4,x0
+	padd		x0,y0,a0
+	pmuls		x0,y0,m0	; F805 7002

	movy		@r6,y0
+	movx		@r4,x0
+	pmuls		x0,y0,m0
+	padd		x0,y0,a0	; F805 7002

	; ---------------------------
	; Some negative tests:

	; PMULS only in parallel with non-conditional PADD/PSUB:

	expect		1356
	pabs		x0,a0
+	pmuls		x0,y0,m0
	endexpect

	expect		1356
	dct psub	x0,y0,a0
+	pmuls		x0,y0,m0
	endexpect

	; Cannot have two same types of operations in parallel:

	expect		1356
	padd		x1,y1,a1
+	padd		x0,y0,a0
	endexpect

	expect		1356
	pmuls		x1,y1,m1
+	pmuls		x0,y0,m0
	endexpect

	expect		1356
	movx		@r5+,x1
+	movx		@r4,x0
	endexpect

	expect		1356
	movy		@r7+r9,y1
+	movy		@r6,y0
	endexpect

	; MOVS cannot be paralleled with anything else:

	expect		1356
	movs.l		@r3,a1g
+	padd		x0,y0,a0
	endexpect

	expect		1356
	movs.l		@r3,a1g
+	pmuls		x0,y0,m0
	endexpect

	expect		1356
	movs.l		@r3,a1g
+	movx		@r4,x0
	endexpect

	expect		1356
	movs.l		@r3,a1g
+	movy		@r6,y0
	endexpect

	; ditto for non-DSP instructions

	expect		1356
	mov		r4,r10
+	padd		x0,y0,a0
	endexpect

	expect		1356
	mov		r3,r1
+	pmuls		x0,y0,m0
	endexpect

	expect		1356
	mov		r3,r1
+	movx		@r4,x0
	endexpect

	expect		1356
	mov		r3,r1
+	movy		@r6,y0
	endexpect
