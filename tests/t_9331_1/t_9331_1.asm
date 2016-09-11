	nop	,	,	,	,	,	,	,	,	,	,x	,si0	,	,	,	;
	mads	,	,x	,y	,0	,	,	,cp+	,	,	,	,	,	,	,	; - 5 bit
	nop	,	,	,	,	,	,w0	,cp+	,	,	,x	,d	,	,	,	;
	mad	,	,x	,y	,0	,wf0	,	,cp+	,dp0	,+df0	,d	,w0	,	,	,	; a2*Z-2+0  -> AC
	mad	,	,w	,y	,ac	,wf0	,	,cp+	,dp0	,+	,x	,d	,	,	,	; a0*Z-0+AC -> AC
stage1:	mad	,	,x	,y	,ac	,	,	,cp+	,dp0	,+df0	,x	,d	,	,	,	; a1*z-1+AC -> AC
stage2:	mad	,	,x	,y	,ac	,	,	,cp+	,dp0	,	,x	,d	,	,	,	; b1*z-1+AC -> AC
stage3:	mads	,1	,x	,y	,ac	,	,	,	,dp0	,	,	,	,	,	,	; b2*Z-2+AC -> AC
stage4:	mad	,	,x	,y	,0	,wf0	,w0	,cp+	,dp0	,+df0	,d	,w0	,	,	,	; a2*Z-2+0  -> AC
	mad	,	,w	,y	,ac	,wf0	,	,cp+	,dp0	,+	,x	,d	,	,	,	; a0*Z-0+AC -> AC
	mad	,	,x	,y	,ac	,	,	,cp+	,dp0	,+df0	,x	,d	,	,	,	; a1*Z-1+AC -> AC
	mad	,	,x	,y	,ac	,	,	,cp+	,dp0	,	,x	,d	,	,	,	; b1*Z-1+AC -> AC
	mads	,1	,x	,y	,ac	,	,	,	,dp0	,	,	,	,	,	,	; b2*Z-2+AC -> AC
	end
