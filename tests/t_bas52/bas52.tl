
	;**************************************************************
	;
TIMER_LOAD:; Load the timer
	;
	;*************************************************************
	;
	ACALL	$-1		;DELAY FOUR CLOCKS
	CLR	TR1		;STOP IT WHILE IT'S LOADED
	MOV	TH1,T_HH
	MOV	TL1,T_LL
	CLR	TF1		;CLEAR THE OVERFLOW FLAG
	SETB	TR1		;START IT NOW
	RET
	;

