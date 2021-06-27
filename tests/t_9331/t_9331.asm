        lda     ,lc0    ,7fh,   ,       ,       ,       ;
        lda     ,lc1    ,7fh,   ,       ,       ,       ;
	lda	,cp	,1ffh,	,	,	,	;
	lda	,ofp	,3fh,	,	,	,	;
	lda	,dp0	,7fh,	,	,	,	;
	lda	,dp1	,7fh,	,	,	,	;
	lda	,dp2	,7fh,	,	,	,	;
	lda	,dp3	,7fh,	,	,	,	;
        lda     ,mod0   ,7fh,   ,       ,       ,       ; MOD0=7fh
        lda     ,mod1   ,7fh,   ,       ,       ,       ;
        lda     ,MOD2   ,16h,   ,       ,       ,       ; MOD2=16h
        lda     ,MOD3   ,16h,   ,       ,       ,       ; MOD2=16h
        lda     ,df0	,16h,   ,       ,       ,       ; MOD2=16h
        lda     ,df1	,16h,   ,       ,       ,       ; MOD2=16h
	lda	,df1	,16h,wf0,	,	,	;
	lda	,df1	,16h,wf1,	,	,	;
	lda	,df1	,16h,wf2,	,	,	;
	lda	,df1	,16h,w1	,	,	,	;
        lda     ,df1	,16h,   ,nop	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,x	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,w0	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,w1	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,y	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,dp	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,xad	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,po	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,so0	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,so1	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,so2	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,xd	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,d	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,c	,	,	; MOD2=16h
        lda     ,df1	,16h,   ,o	,	,	; MOD2=16h
        lda	,df1	,16h,	,	,rnd	,	; MOD2=16h
        lda	,df1	,16h,	,	,x	,	; MOD2=16h
        lda	,df1	,16h,	,	,w0	,	; MOD2=16h
        lda	,df1	,16h,	,	,w1	,	; MOD2=16h
        lda	,df1	,16h,	,	,dp	,	; MOD2=16h
        lda	,df1	,16h,	,	,pi	,	; MOD2=16h
        lda	,df1	,16h,	,	,po	,	; MOD2=16h
        lda	,df1	,16h,	,	,si0	,	; MOD2=16h
        lda	,df1	,16h,	,	,si1	,	; MOD2=16h
        lda	,df1	,16h,	,	,rmr	,	; MOD2=16h
        lda	,df1	,16h,	,	,c	,	; MOD2=16h
        lda	,df1	,16h,	,	,o	,	; MOD2=16h
        lda	,df1	,16h,	,	,d	,	; MOD2=16h
        lda	,df1	,16h,	,	,	,stby	; MOD2=16h
        lda	,df1	,16h,	,	,	,xrd	; MOD2=16h
        lda	,df1	,16h,	,	,	,xref	; MOD2=16h

	nop	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	and	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	or	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	xor	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	not	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	sub	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	abs	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	cmp	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	wcl	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	acs	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	mad	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	mads	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	dmad	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	dmas	,	,	,	,	,	,	,	,	,	,	,	,	,	,	;

	nop	,1	,	,	,	,	,	,	,	,	,	,	,	,	,	;
	nop	,4	,	,	,	,	,	,	,	,	,	,	,	,	,	;

	nop	,	,x	,	,	,	,	,	,	,	,	,	,	,	,	;
	nop	,	,w	,	,	,	,	,	,	,	,	,	,	,	,	;

	nop	,	,	,y	,	,	,	,	,	,	,	,	,	,	,	;
	nop	,	,	,x	,	,	,	,	,	,	,	,	,	,	,	;

	wrs	,	,	,	,0	,	,	,	,	,	,	,	,	,	,	;
	wrs	,	,	,	,w	,	,	,	,	,	,	,	,	,	,	;
	wrs	,	,	,	,ac	,	,	,	,	,	,	,	,	,	,	;

	wrs	,	,	,	,	,wf0	,	,	,	,	,	,	,	,	,	;
	wrs	,	,	,	,	,wf1	,	,	,	,	,	,	,	,	,	;
	wrs	,	,	,	,	,wf2	,	,	,	,	,	,	,	,	,	;
	wrs	,	,	,	,	,w1	,	,	,	,	,	,	,	,	,	;

	wrs	,	,	,	,	,	,w0	,	,	,	,	,	,	,	,	;
	wrs	,	,	,	,	,	,w1	,	,	,	,	,	,	,	,	;

	wrs	,	,	,	,	,	,	,nop	,	,	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,cp+	,	,	,	,	,	,	,	;

	wrs	,	,	,	,	,	,	,	,dp0	,	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,dp1	,	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,dp2	,	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,dp3	,	,	,	,	,	,	;

	wrs	,	,	,	,	,	,	,	,	,nop	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,-	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,+	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,-df0	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,+df0	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,-df1	,	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,+df1	,	,	,	,	,	;

	wrs	,	,	,	,	,	,	,	,	,	,nop	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,x	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,w0	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,w1	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,y	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,dp	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,xad	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,po	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,so0	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,so1	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,so2	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,xd	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,xa	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,d	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,a	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,b	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,c	,	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,o	,	,	,	,	;

	wrs	,	,	,	,	,	,	,	,	,	,	,rnd	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,x	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,w0	,	,	,	;
	nop	,	,	,	,	,	,	,	,	,	,	,w1	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,dp	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,pi	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,po	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,si0	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,si1	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,rmr	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,c	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,o	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,d	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,a	,	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,b	,	,	,	;

	wrs	,	,	,	,	,	,	,	,	,	,	,	,stby	,	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,	,xrd	,	,	;
	nop	,	,	,	,	,	,	,	,	,	,	,	,xref	,	,	;

	wrs	,	,	,	,	,	,	,	,	,	,	,	,	,nop	,	;
	wrs	,	,	,	,	,	,	,	,	,	,	,	,	,ofp+	,	;

	wrs	,	,	,	,	,	,	,	,	,	,	,	,	,	,nop	;
	wrs	,	,	,	,	,	,	,	,	,	,	,	,	,	,er	;

	ldb	,	,0h	,	;
	ldb	,	,1h	,	;
	ldb	,	,123456h,	;
	ldb	,w1	,123456h,	;
	ldb	,x	,123456h,	;
	ldb	,	,123456h,l	;
	ldb	,	,123456h,r	;

	jmp0	,0ffh	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jmp0	,label	,	,	,	,	,	,	,	;
	jmp1	,label	,	,	,	,	,	,	,	;
	jnz0	,label	,	,	,	,	,	,	,	;
	jnz1	,label	,	,	,	,	,	,	,	;
	jz0	,label	,	,	,	,	,	,	,	;
	jz1	,label	,	,	,	,	,	,	,	;
	call	,label	,	,	,	,	,	,	,	;
	call	,label	,cp+	,	,	,	,	,	,	;
	call	,label	,	,dp3	,	,	,	,	,	;
	call	,label	,	,	,+df1	,	,	,	,	;
	call	,label	,	,	,	,dp	,	,	,	;
	call	,label	,	,	,	,	,dp	,	,	;
	call	,label	,	,	,	,	,	,xref	,	;
	call	,label	,	,	,	,	,	,	,ofp+	;
	jnc	,label	,zf	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,zf	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,sf	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,v0f	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,v1f	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,lrf	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,gf0	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,gf1	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,gf2	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,gf3	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,iff0	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,iff1	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	jc	,label	,iff2	,nop	,dp0	,nop	,nop	,rnd	,stby	,nop	;
	ret	,	,	,	,	,	,	,	,	,	,	,	,	;
	ret	,1	,	,	,	,	,	,	,	,	,	,	,	;
	ret	,	,w	,	,	,	,	,	,	,	,	,	,	;
	ret	,	,	,x	,	,	,	,	,	,	,	,	,	;
	ret	,	,	,	,wf2	,	,	,	,	,	,	,	,	;
	ret	,	,	,	,	,w1	,	,	,	,	,	,	,	;
	ret	,	,	,	,	,	,cp+	,	,	,	,	,	,	;
	ret	,	,	,	,	,	,	,dp3	,	,	,	,	,	;
	ret	,	,	,	,	,	,	,	,+df1	,	,	,	,	;
	ret	,	,	,	,	,	,	,	,	,b	,	,	,	;
	ret	,	,	,	,	,	,	,	,	,	,b	,	,	;
	ret	,	,	,	,	,	,	,	,	,y	,	,	,	;
	ret	,	,	,	,	,	,	,	,	,	,x	,	,	;
	ret	,	,	,	,	,	,	,	,	,y	,x	,	,	;
	ret	,	,	,	,	,	,	,	,	,a	,	,	,	;
	ret	,	,	,	,	,	,	,	,	,	,	,xref	,	;
	ret	,	,	,	,	,	,	,	,	,	,	,	,ofp+	;

		,	,	,	,	,	,	,	,	,	,	,	,	,	,	; NOP
		,gfs	,	,	,	,	,	,	,	,	,	,	;
	gmad	,gfs	,	,	,	,	,	,	,	,	,	,	;
	gmas	,gfs	,	,	,	,	,	,	,	,	,	,	;
		,gfc	,	,	,	,	,	,	,	,	,	,	;
	gmad	,gfc	,	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,gf0	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,gf1	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,gf2	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,gf3	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,ov0	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,ov1	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,fchg	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,bnk	,	,	,	,	,	,	,	,	,	;
	gmas	,gfc	,	,	,	,	,	,	,b	,	,	,	;
	gmas	,gfc	,	,	,	,	,	,	,	,b	,	,	;

	org	100h
label:

        end

