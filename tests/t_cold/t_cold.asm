		page		0

		cpu		mcf5202		; ISA A
		supmode		on

		cpu		mcf5208		; ISA A+
		supmode		on

		bitrev		d4		; $00cn
		bitrev.l	d4
		byterev		d6		; $02cn
		byterev.l	d6
		ff1		d2		; $04cn
		ff1.l		d2

		move.l		usp,a3		; $4e6(8n)
		move.l		a5,usp		; $4e6(0n)

		stldsr		#$55aa		; $40e7 $46fc iiii
		stldsr.w	#$55aa

		cpu		mcf5407		; ISA B

		bra.x		*+100000	; $60ff dddd dddd
		bra		*+100000
		bsr.x		*+100000	; $61ff dddd dddd
		bsr		*+100000
		bne.x		*+100000	; $66ff dddd dddd
		bne		*+100000

		cmp.b		d3,d2		; %1011 010(d2) 000(b)|001(w) eeeaaa
		cmp.w		d3,d2
		cmp.b		a3,d2
		cmp.w		a3,d2
		cmp.b		(a5),d2
		cmp.w		(a5),d2
		cmp.b		(a4)+,d2
		cmp.w		(a4)+,d2
		cmp.b		-(a1),d2
		cmp.w		-(a1),d2
		cmp.b		(1000,sp),d2
		cmp.w		(1000,sp),d2
		cmp.b		(-100,d4,a6),d2
		cmp.w		(-100,d4,a6),d2
		cmp.b		($1234),d2
		cmp.b		($1234.w),d2
		cmp.w		($1234),d2
		cmp.w		($1234.w),d2
		cmp.b		($12345678),d2
		cmp.b		($12345678.l),d2
		cmp.w		($12345678),d2
		cmp.w		($12345678.l),d2
		cmp.b		#$56,d2
		cmp.w		#$5678,d2
		cmp.b		(*+1000,pc),d2
		cmp.w		(*+1000,pc),d2
		cmp.b		(*-100,d3,pc),d2
		cmp.w		(*-100,d3,pc),d2

		cmpa.w		d3,a2		; %1011 010(a2) 011(w) eeeaaaa
		cmp.w		d3,a2
		cmpa.w		a3,a2
		cmp.w		a3,a2
		cmpa.w		(a5),a2
		cmp.w		(a5),a2
		cmpa.w		(a4)+,a2
		cmp.w		(a4)+,a2
		cmpa.w		-(a1),a2
		cmp.w		-(a1),a2
		cmpa.w		(1000,sp),a2
		cmp.w		(1000,sp),a2
		cmpa.w		(-100,d4,a6),a2
		cmp.w		(-100,d4,a6),a2
		cmpa.w		($1234),a2
		cmpa.w		($1234.w),a2
		cmp.w		($1234),a2
		cmp.w		($1234.w),a2
		cmpa.w		($12345678),a2
		cmpa.w		($12345678.l),a2
		cmp.w		($12345678),a2
		cmp.w		($12345678.l),a2
		cmpa.w		#$56,a2
		cmp.w		#$5678,a2
		cmpa.w		(*+1000,pc),a2
		cmp.w		(*+1000,pc),a2
		cmpa.w		(*-100,d3,pc),a2
		cmp.w		(*-100,d3,pc),a2

		cmpi.b		#$55,d3
		cmpi.w		#$6666,d3

		intouch		(a4)		; $f42(1|reg) 

		mov3q		#-1,d3		; %1010 iii1 01ee eaaa
		mov3q.l		#-1,d3
		mov3q		#1,d3
		mov3q.l		#1,d3
		mov3q		#7,d3
		mov3q.l		#7,d3
		mov3q		#1,a5
		mov3q.l		#1,a5
		mov3q		#1,(a4)
		mov3q.l		#1,(a4)
		mov3q		#1,(a1)+
		mov3q.l		#1,(a1)+
		mov3q		#1,-(a6)
		mov3q.l		#1,-(a6)
		mov3q		#1,(1000,a6)
		mov3q.l		#1,(a6,1000)
		mov3q		#1,(-100,a6,d5)
		mov3q.l		#1,(d5,a6,-100)
		mov3q		#1,($3456)
		mov3q.l		#1,$3456
		mov3q		#1,($123456)
		mov3q.l		#1,$123456

		irp		instr,mvs,mvz
		irp		src,d3,a3,(a3),(a3)+,-(a3),(1000,a3),(-100,a3,d3),($1234),($12345678),(*+1000,pc),(*-100,pc,d3),#42
		instr		src,d7
		instr.b		src,d7
		instr.w		src,d7
		endm
		endm

		sats		d4		; $4c8(Dn)
		sats.l		d4

		irp		dest,(a3),(a3)+,-(a3),(1000,a3),(-100,a3,d3),($1234),($12345678)
		tas		dest
		tas.b		dest
		endm

		fpu		on

		irp		instr,fabs,fsabs,fdabs
		instr.b		d2,fp4
		instr.w		d2,fp4
		instr.l		d2,fp4
		instr.s		d2,fp4
		irp		src,(a2),(a2)+,-(a2),(1000,a2),(*+1000,pc)
		instr.b		src,fp4
		instr.w		src,fp4
		instr.l		src,fp4
		instr.s		src,fp4
		instr.d		src,fp4
		instr		src,fp4
		endm
		instr		fp2,fp4
		instr.d		fp2,fp4
		instr		fp4
		instr.d		fp4
		endm

		; (E)MAC stuff

		cpu		mcf5470

		irp		instr,mac,msac
		 irp		shift,,<<0,>>0,<<1,>>1
		  instr.l	a4,d3 shift
		  instr.l	a4,d3 shift,(a5),a6,acc
		  instr.l	a4,d3 shift,(1000,a5),a6,acc0
		  instr.l	a4,d3 shift,(a5)&,a6
		  instr.l	a4,d3 shift,(1000,a5)&,a6
		  instr.l	a4,d3 shift,(a5)&mask,a6
		  instr.l	a4,d3 shift,(1000,a5)&mask,a6
		  irp		half1,l,u
		  irp		half2,l,u
		   instr.w	a4.half1,d3.half2 shift
		   ; we might iterate here through mask arg and other EMAC ACCs either...
		  endm
		  endm
		 endm
		endm

		irp		reg,acc,acc0,acc1,acc2,acc3,macsr,mask,accext01,accext23
		 move		reg,d4
		 move.l		reg,d4
		 move		reg,a4
		 move.l		reg,a4
		 move		d4,reg
		 move.l		d4,reg
		 move		a4,reg
		 move.l		a4,reg
		 move		#$12345678,reg
		 move.l		#$12345678,reg
		endm

		move		MACSR,ccr
		move.l		MACSR,ccr

		move		acc2,acc3
		move.l		acc2,acc3

		irp		reg,acc,acc0,acc1,acc2,acc3
		 movclr		reg,d4
		 movclr.l	reg,d4
		 movclr		reg,a4
		 movclr.l	reg,a4
		endm

		irp		instr,maaac,masac,msaac,mssac
		 irp		shift,,<<0,>>0,<<1,>>1
		  instr.l	a4,d3 shift,acc1,acc3
		  irp		half1,l,u
		  irp		half2,l,u
		   instr.w	 a4.half1,d3.half2 shift,acc1,acc3
		  endm
		  endm
		 endm
		endm