	cpu	68000

	moveq	#0,d2
	moveq	#$7f,d2
	moveq	#-1,d2
	moveq	#$ffffffc0,d2   ; equiv. to -64
;	moveq	#$ff,d2		; no longer allowed
;	moveq	#-1000,d2	; expect range underflow
