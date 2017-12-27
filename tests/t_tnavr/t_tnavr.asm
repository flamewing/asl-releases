	cpu	attiny10

	include	"regavr.inc"

	add	r16,r17
	adc	r17,r18
	sub	r18,r19
	subi	r19,20
	sbc	r20,r19
	sbci	r21,20
	and	r22,r21
	andi	r23,22
	or	r24,r23
	ori	r25,24
	eor	r26,r25
	com	r27
	neg	r28
	sbr	r29,5
	cbr	r30,6
	inc	r31
	dec	r16
	tst	r17
	clr	r18
	ser	r19

	rjmp	*
	ijmp
	rcall	*
	icall
	ret
	reti
	cpse	r20,r19
	cp	r21,r20
	cpc	r22,r21
	cpi	r23,22
	sbrc	r24,2
	sbrs	r25,3
	sbic	10,1
	sbis	11,2
	brbs	6,*
	brbc	7,*
	breq	*
	brne	*
	brcs	*
	brcc	*
	brsh	*
	brlo	*
	brmi	*
	brpl	*
	brge	*
	brlt	*
	brhs	*
	brhc	*
	brts	*
	brtc	*
	brvs	*
	brvc	*
	brie	*
	brid	*

	lsl	r26
	lsr	r27
	rol	r28
	ror	r29
	asr	r30
	swap	r31
	bset	2
	bclr	3
	sbi	10,1
	cbi	11,2
	bst	r16,3
	bld	r17,4

	sec
	clc
	sen
	cln
	sez
	clz
	sei
	cli
	ses
	cls
	sev
	clv
	set
	clt
	seh
	clh

	mov	r18,r17
	ldi	r19,18
	ld	r20,x
	ld	r21,x+
	ld	r22,-x
	ld	r23,y
	ld	r24,y+
	ld	r25,-y
	ld	r26,z
	ld	r27,z+
	ld	r28,-z
	lds	r29,28
	st	x,r30
	st	x+,r31
	st	-x,r16
	st	y,r17
	st	y+,r18
	st	-y,r19
	st	z,r20
	st	z+,r21
	st	-z,r22
	sts	22,r23
	in	r24,23
	out	24,r25
	push	r26
	pop	r27

	nop
	sleep
	wdr
	break

	data	1,2,3
	data	"abcd"
