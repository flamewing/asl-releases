	cpu	80296

ax	equ	40
al	equ	ax
ah	equ	al+1
bx	equ	42
cx	equ	44
dx	equ	46

        reti

        eld	ax,[bx]
        eld	ax,0[bx]
        eld	ax,100[bx]   	; nur 24-Bit-Displacement
        eld	ax,1000[bx]
        eld	ax,100000[bx]
        eld	ax,-100[bx]
        eld	ax,-1000[bx]
        eld	ax,-100000[bx]
	eld	ax,123456h

        eldb	al,[bx]
        eldb	al,0[bx]
        eldb	al,100[bx]   	; nur 24-Bit-Displacement
        eldb	al,1000[bx]
        eldb	al,100000[bx]
        eldb	al,-100[bx]
        eldb	al,-1000[bx]
        eldb	al,-100000[bx]
	eldb	al,123456h

        est	ax,[bx]
        est	ax,0[bx]
        est	ax,100[bx]   	; nur 24-Bit-Displacement
        est	ax,1000[bx]
        est	ax,100000[bx]
        est	ax,-100[bx]
        est	ax,-1000[bx]
        est	ax,-100000[bx]
	est	ax,123456h

        estb	al,[bx]
        estb	al,0[bx]
        estb	al,100[bx]   	; nur 24-Bit-Displacement
        estb	al,1000[bx]
        estb	al,100000[bx]
        estb	al,-100[bx]
        estb	al,-1000[bx]
        estb	al,-100000[bx]
	estb	al,123456h

	ecall	$+123456h
	ejmp	$-10h
        ebr	[bx]

        ebmovi	cx,ax

        mac     #12h
        mac     ax,#12h

        macr    bx
        macr    ax,bx

        macrz   10[bx]
        macrz   ax,10[bx]

        macz    -1000[bx]
        macz    ax,-1000[bx]

        smac    [cx]
        smac    ax,[cx]

        smacr   [cx]+
        smacr   ax,[cx]+

        smacrz  1000h
        smacrz  ax,1000h

        smacz   0ffc0h
        smacz   ax,0ffc0h

        msac	ax,#20

        mvac	cx,al

        rpt	ax
        rptnst	#10
        rptnh	[cx]
        rptgt	[cx]+
        rptnc	ax
        rptnvt	#10
        rptnv	[cx]
        rptge	[cx]+
        rptne	ax
        rptst	#10
        rpth	[cx]
        rptle	[cx]+
        rptc	ax
        rptvt	#10
        rptv	[cx]
        rptlt	[cx]+
        rpte	ax
        rpti	#10
        rptinst	[cx]
        rptinh	[cx]+
        rptigt	ax
        rptinc	#10
        rptinvt	[cx]
        rptinv	[cx]+
        rptige	ax
        rptine	#10
        rptist	[cx]
        rptih	[cx]+
        rptile	ax
        rptic	#10
        rptivt	[cx]
        rptiv	[cx]+
        rptilt	ax
        rptie	#10

        assume	wsr:3eh		; 1f80h...1fbfh --> 0c0h...0ffh
        assume	wsr1:9eh	; 0f780h...0f7bfh --> 40h..7fh

        ld	ax,3eh		; normal
        ld	ax,44h		; muﬂ absolut werden
        ld	ax,92h		; wieder normal
        ld	ax,0d0h		; muﬂ wieder absolut werden
        ld	ax,1000h	; muﬂ absolut bleiben
        ld	ax,1f90h	; mit WSR
        ld	ax,2000h	; muﬂ wieder absolut bleiben
        ld      ax,0f7a0h	; mit WSR1
        ld	ax,0fffeh	; muﬂ wieder absolut bleiben
