		page	0

		cpu	97c241

		unlk
		szf
		svf
		ssf
		scf
		rzf
		rvf
		rsf
		rets
		reti
		ret
		rcf
		nop
		halt
		ei
		di
		czf
		cvf
		csf
		ccf

Targ:		jr	Targ
		calr	Targ
		jr	$+1002h

		jrc	z,Targ
		jrc	ult,Targ
		jrc	ge,Targ

		jrbc	5,1234h,$+108h
		jrbs	1,345h,Targ

		djnz	rw5,Targ
		djnz.d	(123456h),$+100ah
		djnzc	rw6,nov,Targ
		djnzc.d	(rd2+123456h),eq,$+100ah

		link	20h
		link	3334h
		retd	-122
		retd	-1234

		swi	7

		cpl	rb2
		cpl	rw12
		cpl	rd14

		cpl.b	rb5
		cpl.w	rw15
		cpl.d	rd4

		cpl.b	(123h)
		cpl.w	(123h)
		cpl.d	(123h)

		cpl.b	(123456h)
		cpl.w	(123456h)
		cpl.d	(123456h)

		cpl.b	(rw2)
		cpl.w	(rw5)
		cpl.d	(rw0)

		cpl.b	(rd10)
		cpl.w	(rd6 )
		cpl.d	(rd12)

		cpl.b	(rw4++)
		cpl.w	(rw6++)
		cpl.d	(rw9++)

		cpl.b	(--rw8)
		cpl.w	(--rw7)
		cpl.d	(--rw5)

		cpl.b	(rd2++)
		cpl.w	(rd10++)
		cpl.d	(rd14++)

		cpl.b	(--rd4)
		cpl.w	(--rd8)
		cpl.d	(--rd12)

		cpl.b	(rw3+123h)
		cpl.w	(rw7+123h)
		cpl.d	(rw1+123h)

		cpl.b	(rd2-123h)
		cpl.w	(rd8-123h)
		cpl.d	(rd4-123h)

		cpl.b	(rw4 +123456h)
		cpl.w	(rw7 +123456h)
		cpl.d	(rw14+123456h)

		cpl.b	(rd6 -123456h)
		cpl.w	(rd12-123456h)
		cpl.d	(rd10-123456h)

		cpl.b	(rw0+12h)
		cpl.w	(rw0+12h)
		cpl.d	(rw0+12h)

		cpl.b	(rd0-12h)
		cpl.w	(rd0-12h)
		cpl.d	(rd0-12h)

		cpl.b	(rw0+12345h)
		cpl.w	(rw0+12345h)
		cpl.d	(rw0+12345h)

		cpl.b	(rd0-12345h)
		cpl.w	(rd0-12345h)
		cpl.d	(rd0-12345h)

		cpl.b	(sp+12h)
		cpl.w	(sp+12h)
		cpl.d	(sp+12h)

		cpl.b	(sp+89h)
		cpl.w	(sp+89h)
		cpl.d	(sp+89h)

		cpl.b	(sp+12345h)
		cpl.w	(sp+12345h)
		cpl.d	(sp+12345h)

		cpl.b	(pc-89h)
		cpl.w	(pc-89h)
		cpl.d	(pc-89h)

		cpl.b	(pc-12345h)
		cpl.w	(pc-12345h)
		cpl.d	(pc-12345h)

		cpl.b	(rw2 *4)
		cpl.w	(rw5 *4)
		cpl.d	(rw10*4)

		cpl.b	(13h+rd10*8)
		cpl.w	(rd14*8+12h)
		cpl.d	(rd4 *8+12h)

		cpl.b	(rw7*2-12345h)
		cpl.w	(rw9*2-12345h)
		cpl.d	(rw1*2-12345h)

		cpl.b	(rd10*2+12345h)
		cpl.w	(rd4 *2+12345h)
		cpl.d	(rd6 *2+12345h)

		cpl.b	(rw4  + rw6  *8 + 12h)
		cpl.w	(rw5  + rd8  *4 + 12h)
		cpl.d	(rd4  + rw9  *2 + 12h)
		cpl.b	(rd10 + rd14 *1 + 12h)
		cpl.w	(sp   + rw5  *2 + 12h)
		cpl.d	(sp   + rd2  *4 + 12h)
		cpl.b   (pc   + rw11 *8 + 12h)
		cpl.w   (pc   + rd4  *4 + 12h)

		cpl	sp
		cpl	isp
		cpl	esp
		cpl	pbp
		cpl	cbp
		cpl	psw
		cpl	imc
		cpl	cc

		call	targ
		call	(rw2)
		call.w	123456h
		clr	rb5
		clr.w	(rw3-4)
		clr.d	(--rd2)
		exts	rw1
		exts.d  (sp+20)
		extz.w	(pc-7)
		extz.d	rd12
		jp	Targ
		jp.w	(rd4++)
		jp.d	(--rw5)
		mirr	rb1
		mirr.w	(Targ)
		mirr.d	(sp+1234h)
		neg.d	rd8
		pop.b	(rd10++)
		push.d	12345678h
		pusha	Targ
		rvby.w	(sp+rw4*4+0aah)
		tjp.d	(rd6)
		tst.w	rw13

		lda	rd6,(rd4+12345h)

		add3	rw4,(rd4),(rw2)
		sub3	rw4,(rd4),1000
		add3	rw4,(rd4),100
		sub3	rd2,rd10,(123456h)
		add3	rd8,(rw2*4+12345h),10
		sub3	rb5,rb3,rb15

		mac	rw4,rb3,rb1
		macs	rd14,(rw4-2),(rd2+4)

		cpsz	(rd6++),rb2,rw15
		cpsn.w	(--rd4),(--rd10),rw14
		lds.d	(rd8++),10,rw13
		lds.w	(--rd10),1000,rw12

		rrm.b	rb5,rw3,4
		rlm.d   (123456h),rw4,(123456h)

		bfex	rw7,rd10,2,12
		bfexs.b	rw9,(rd4),4,6
		bfin.w	(sp+5),rw3,10,3
		bfex	rw10,rd12,20,5
		bfexs.d	rw12,(rd4),30,2

		abcd	rb5,(rw2*4)
		abcd	rw4,(1234h)
		abcd	(1234h),rd4
		adc.w	(rd4+2),(rd2*8+5)
		adc.b	(Targ),4
		cbcd.b	(Targ),99h
		cpc	rb5,(rw4+rw4)
		cpc	rb5,(2)
		max	rb5,rb2
		maxs	rw5,(Targ)
		min.d	(Targ),(rw2)
		mins.d	(rd4+3),(rd6-3)
		sbc.w	(rd4),(rd2)
		sbc.d	(Targ),1
		sbcd	rw4,(Targ)
		sbcd	(rd2),rd0

		andcf	rw10,15
		andcf.b	(rd2),(rw4)
		andcf.d	(rw10),(Targ)
		andcf.w	(Targ),1
		ldcf	rw10,15
		ldcf.b	(rd2),(rw4)
		ldcf.d	(rw10),(Targ)
		ldcf.w	(Targ),1
		orcf	rw10,15
		orcf.b	(rd2),(rw4)
		orcf.d	(rw10),(Targ)
		orcf.w	(Targ),1
		stcf	rw10,15
		stcf.b	(rd2),(rw4)
		stcf.d	(rw10),(Targ)
		stcf.w	(Targ),1
		tset	rw10,15
		tset.b	(rd2),(rw4)
		tset.d	(rw10),(Targ)
		tset.w	(Targ),1
		xorcf	rw10,15
		xorcf.b	(rd2),(rw4)
		xorcf.d	(rw10),(Targ)
		xorcf.w	(Targ),1

		bs0b	rb10,rd12
		bs0f	(Targ),rw3
		bs1b.w	rb10,(Targ)
		bs1f.b	(Targ),(rw10+2)

		chk.d:g	(Targ),rd8
		chk.d	(Targ),rd8
		chks.w	(rd6),(rd4)
		chks.w	(rw2),(Targ)

		mul	rd2,rw1
		muls.w	(rw2),(Targ)
		div.d	(Targ),(rw2)
		divs	rw4,rb8

		add	rw11,2
		add	(rw4),rd8
		add.w	(rd4+2),(rd6+rd8*4+100)
		add.b	rb2,150
		add.w	rw2,150
		add.d	rd2,150
		add.d	(Targ),2
		add.w:a	rw14,(Targ)   ; !!! automatische Wahl sonst S

		ld	rd10,2000
		cp	rd8,(Targ)
		sub.w	(rd4++),(rd6++)

		and	rw4,rw3
		and.d	rd10,rd12
		or	rd10,(rd8)
		or.w	(Targ),(rd4+rw2*4-5)
		xor.d	(Targ),12345678h
		xor	rd2,1000h
		or	rb2,rb7
		and.w	(Targ),rw4
		xor.d	(rw2),(Targ)

		bres	rb4,1
		bset	rw7,5
		bchg	rw12,10
		btst	rd4,8
		bres	rd12,20
		bset.w	(rd4),10
		bchg.d	(rw2),15
		btst.b	(Targ),rb1
		bres	rb1,(Targ)

		ex	rb1,rb7
		ex	rw5,(rd4)
		ex.d	(Targ),(rw1)
		ex.b	(rw1),(Targ)

		rl	rw4,1
		rlc.w	(rd6*2),1
		rr	rd0,5
		rrc	rw1,(rd4)
		sla	rw7,(Targ)
		sll.d	(Targ),3
		sra.w	(rd4),(rw4)
		srl.b	(rw4),(rd4)

		add.w	(rw4+0eah),rw7
		add.w	(sp+rw4*4+1234h),(sp+28h)
		add.w:g	(rw4+12345h),(10028h)
		add.w	(rw4+12345h),(10028h)
		add.w:g	(rw4+56h),3579h
		add.w	(rw4+56h),3579h
		jrc	nz,$-1234h

