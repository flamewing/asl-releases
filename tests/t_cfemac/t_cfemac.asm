		cpu		mcf5470

		page		0

proc		macro		name
		section		name
		public		name
name		equ		*
		endm

endp		macro
		rts
		endsection
		endm

		proc		fft16
		move.l		(a2),d4
		move.w		(a0),d2
		move.w		(a1),d7
		msacl.w		d0.u,d4.u,<<,(a3),d5,ACC0
		msac.w		d0.l,d5.u,<<,ACC0
		mac.w		d0.l,d4.u,<<,ACC1
		msac.w		d0.u,d5.u,<<,ACC1
		movclr.l	ACC0,d3
		movclr.l	ACC1,d1
		add.l		d2,d3
		move.w		d3,(a0)+
		add.l		d2,d2
		sub.l		d3,d2
		move.w		d2,(a2)+
		add.l		d7,d1
		move.w		d1,(a1)+
		add.l		d7,d7
		sub.l		d1,d7
		move.w		d7,(a3)+
		endp

		proc		fft32
		movem.l		(a4),d0-d1
next_bf:	adda.l		d6,a4
		move.l		(a0),d2
		move.l		(a2),d4
		move.l		d2,ACC0
		msacl.l		d0,d4,(a3),d5,ACC0
		msacl.l		d1,d5,(a1),a6,ACC0
		macl.l		d1,d4,4(a4),d1,ACC1
		msacl.l		d0,d5,(a4),d0,ACC1
		movclr.l	ACC0,d3
		move.l		d3,(a0)+
		add.l		d2,d2
		sub.l		d3,d2
		movclr.l	ACC1,d3
		add.l		a6,d3
		move.l		d2,(a2)+
		move.l		d3,(a1)+
		adda.l		a6,a6
		suba.l		d3,a6
		move.l		a6,(a3)+
		adda.l		d6,a4
		move.l		(a0),d2
		move.l		(a2),d4
		move.l		d2,ACC0
		msacl.l		d0,d4,(a3),d5,ACC0
		msacl.l		d1,d5,(a1),a6,ACC0
		macl.l		d1,d4,4(a4),d1,ACC1
		msacl.l		d0,d5,(a4),d0,ACC1
		movclr.l	ACC0,d3
		move.l		d3,(a0)+
		add.l		d2,d2
		sub.l		d3,d2
		movclr.l	ACC1,d3
		add.l		a6,d3
		move.l		d2,(a2)+
		move.l		d3,(a1)+
		adda.l		a6,a6
		suba.l		d3,a6
		move.l		a6,(a3)+
		addq.l		#4,d7
		cmp.l		a5,d7
		bcs.b		next_bf
		endp

		proc		fir16
		move.w		(a3)+, d4
		move.w		d2, d3
		move.w		-(a4), d2
		swap		d2
		swap		d3
		mac.w		d4.l, d2.u, <<, ACC0
		mac.w		d4.l, d2.l, <<, ACC1
		mac.w		d4.l, d3.l, <<, ACC3
		subq		#1, d5
		beq		.EndIn1E
.ForIn1EBeg:	move.l		(a3)+, d4
.ForIn1E:	subq.l		#2, d5
		blt		.EndIn1E
		mac.w		d4.u, d2.u, <<, ACC1
		mac.w		d4.u, d2.l, <<, ACC2
		mac.w		d4.u, d3.u, <<, ACC3
		move.l		d2, d3
		move.l		-(a4), d2
		mac.w		d4.u, d2.l, <<, ACC0
		mac.w		d4.l, d2.u, <<, ACC0
		mac.w		d4.l, d2.l, <<, ACC1
		mac.w		d4.l, d3.u, <<, ACC2
		mac.w		d4.l, d3.l, <<, (a3)+, d4, ACC3
		bra .ForIn1E
.EndIn1E:
		endp

		proc		fir32
.FORk4:		cmp.l		d0, d2
		bhi		.ENDFORk4
		mac.l		a6,d5,<<,-(a1),d5,ACC3
		mac.l		a6,d4,<<,ACC2
		mac.l		a6,d3,<<,ACC1
		mac.l		a6,d6,<<,(a3)+,a6,ACC0
		mac.l		a6,d4,<<,-(a1),d4,ACC3
		mac.l		a6,d3,<<,ACC2
		mac.l		a6,d6,<<,ACC1
		mac.l		a6,d5,<<,(a3)+,a6,ACC0
		mac.l		a6,d3,<<,-(a1),d3,ACC3
		mac.l		a6,d6,<<,ACC2
		mac.l		a6,d5,<<,ACC1
		mac.l		a6,d4,<<,(a3)+,a6,ACC0
		mac.l		a6,d6,<<,-(a1),d6,ACC3
		mac.l		a6,d5,<<,ACC2
		mac.l		a6,d4,<<,ACC1
		mac.l		a6,d3,<<,(a3)+,a6,ACC0
		addq.l		#4,d2
		bra		.FORk4
.ENDFORk4:
		endp

		proc		iir16
		move.l		(a3)+, d4
		move.w		d2, d3
		move.w		-(a4), d2
		swap		d2
		swap		d3
		move.w		d0, d1			; was mac.w in source?
		move.w		-(a5), d0		; was mac.w in source?
		swap		d0
		swap		d1
		mac.w		d4.u, d2.u, <<, ACC0
		mac.w		d4.u, d2.l, <<, ACC1
		mac.w		d4.u, d2.u, <<, ACC2
		mac.w		d4.u, d3.l, <<, ACC3
		mac.w		d4.l, d2.u, <<, ACC0
		mac.w		d4.l, d2.l, <<, ACC1
		mac.w		d4.l, d2.u, <<, ACC2
		mac.w		d4.l, d3.l, <<, ACC3
		subq		#1, d5
		beq		.EndIn1E
.ForIn1E:
		subq.l		#2, d5
		blt		.EndIn1E
		mac.w		d4.u, d2.u, <<, ACC1
		mac.w		d4.u, d2.l, <<, ACC2
		mac.w		d4.u, d3.u, <<, ACC3
		mac.w		d4.l, d0.u, <<, ACC1
		mac.w		d4.l, d0.l, <<, ACC2
		mac.w		d4.l, d1.u, <<, ACC3
		move.l		d2, d3
		move.l		-(a4), d2
		move.l		d0, d1
		move.l		-(a5), d0
		mac.w		d4.u, d2.l, <<, ACC0
		mac.w		d4.l, d0.l, <<, ACC0
		move.l		(a3)+, d4
		mac.w		d4.u, d2.u, <<, ACC0
		mac.w		d4.u, d2.l, <<, ACC1
		mac.w		d4.u, d3.l, <<, ACC2
		mac.w		d4.u, d3.l, <<, ACC3
		mac.w		d4.l, d0.u, <<, ACC0
		mac.w		d4.l, d0.l, <<, ACC1
		mac.w		d4.l, d1.u, <<, ACC2
		mac.w		d4.l, d1.l, <<, (a3)+, d4, ACC3
		bra		.ForIn1E
.EndIn1E:
		endp

		proc		iir32
.FORk1:
		cmp.l		d1,d2
		bcc		.ENDFORk1
		adda.l		#4,a3
		mac.l		a6,d5,<<,-(a1),d5,ACC3
		mac.l		a6,d4,<<,ACC2
		mac.l		a6,d3,<<,ACC1
		mac.l		a6,d6,<<,(a3)+,a6,ACC0
		adda.l		#4,a3
		mac.l		a6,d4,<<,-(a1),d4,ACC3
		mac.l		a6,d3,<<,ACC2
		mac.l		a6,d6,<<,ACC1
		mac.l		a6,d5,<<,(a3)+,a6,ACC0
		add.l		#4,a3
		mac.l		a6,d3,<<,-(a1),d3,ACC3
		mac.l		a6,d6,<<,ACC2
		mac.l		a6,d5,<<,ACC1
		mac.l		a6,d4,<<,(a3)+,a6,ACC0
		adda.l		#4,a3
		mac.l		a6,d6,<<,-(a1),d6,ACC3
		mac.l		a6,d5,<<,ACC2
		mac.l		a6,d4,<<,ACC1
		mac.l		a6,d3,<<,(a3)+,a6,ACC0
		addq.l		#4,d3
		bra		.FORk1
.ENDFORk1
		endp
