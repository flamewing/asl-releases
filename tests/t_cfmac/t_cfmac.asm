		cpu		mcf5307		; MAC but no EMAC

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
		and.l		#$ffff0000,d2
		move.l		d2,ACC
		msacl.w		d0.u,d4.u,<<,(a3),d5
		msacl.w		d0.l,d5.u,<<,(a1),d7
		move.l		ACC,d3
		asr.l		d1,d3
		move.w		d3,(a0)+
		add.l		d2,d2
		asr.l		d1,d2
		sub.l		d3,d2
		move.w		d2,(a2)+
		and.l		#$ffff0000,d7
		move.l		d7,ACC
		macl.w		d0.l,d4.u,<<,(a0),d2
		msacl.w		d0.u,d5.u,<<,(a2),d4
		move.l		ACC,d3
		asr.l		d1,d3
		move.w		d3,(a1)+
		add.l		d7,d7
		asr.l		d1,d7
		sub.l		d3,d7
		move.w		d7,(a3)+
		endp

		proc		fft32
		movem.l		(a4),d0-d1
next_bf:	move.l		d2,ACC
		msacl.l		d0,d4,(a3),d5
		msacl.l		d1,d5,(a1),a6
		move.l		ACC,d3
		move.l		d3,(a0)+
		add.l		d2,d2
		sub.l		d3,d2
		move.l		d2,(a2)+
		move.l		a6,ACC
		macl.l		d1,d4,(a0),d2
		msacl.l		d0,d5,(a2),d4
		move.l		ACC,d3
		move.l		d3,(a1)+
		adda.l		a6,a6
		suba.l		d3,a6
		move.l		a6,(a3)+
		adda.l		d6,a4
		addq.l		#4,d7
		cmp.l		a5,d7
		bcs.b		next_bf
		endp

		proc		fir16
.FORk3:		cmp.l		d0,d2
		bcc		.ENDFORk3
		move.l		-(a1),d3
		mac.w		d3.u,d4.l,<<
		mac.w		d3.l,d4.u,<<,(a3)+,d4
		addq.l		#2,d2
		bra		.FORk3
.ENDFORk3:
		endp

		proc		fir32
.FORk3:		move.l		-(a1),d3
		mac.w		d3.u,d4.u,<<,(a3)+,d4,ACC0
		addq.l		#1,d2
		cmp.l		d0, d2
		bcs		.FORk3
		endp

		proc		iir16
.FORk3:		cmp.l		d0,d2
		bcc		.ENDFORk3
		move.l		-(a1),d3
		mac.w		d3.l,d4.u,<<
		move.l		-(a5),d3		; was mac.w in original?
		mac.w		d3.l,d4.l,<<,(a3)+,d4
		addq.l		#1,d2
		bra		.FORk3
.ENDFORk3:
		endp

		proc		iir32
.FORk1:		cmp.l		d1,d2
		bcc		.ENDFORk1
		move.l		-(a1),d3
		mac.w		d3.u,d4.u,<<,(a3)+,d4,ACC0
		move.l		-(a5),d3		; was mac.w in original?
		mac.w		d3.u,d4.u,<<,(a3)+,d4,ACC0
		addq.l		#1,d2
		bra		.FORk1
.ENDFORk1:
		endp
