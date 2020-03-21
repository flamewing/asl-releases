	cpu	ez8
	page	0

wregbit	defbit	r7,5
regbit	defbit	0f40h,4

	; BCLR on working reg bit:

	bclr	5,r7
	bclr	wregbit

	; BCLR on extended reg bit (translates to ANDX):

	andx	0f40h,#0efh
	bclr	4,0f40h
	bclr	regbit

	; BSET on working reg bit:

	bset	5,r7
	bset	wregbit

	; BSET on extended reg bit (translates to ORX):

	orx	0f40h,#10h
	bset	4,0f40h
	bset	regbit

	; BTJ on working reg bit:

	btj	0,5,r7,$
	btj	0,wregbit,$
	btj	1,5,r7,$
	btj	1,wregbit,$

	; BTJZ on working reg bit:

	btjz	5,r7,$
	btjz	wregbit,$

	; BTJNZ on working reg bit:

	btjnz	5,r7,$
	btjnz	wregbit,$
