	cpu	6502

;--------------------------------------------------------------------------
; tests of named temporary symbols

        ldx     #00
$$loop:	dex
        bne     $$loop

	ldx	RealSymbol
	cpx	#00
	beq	skip
	nop
skip:

        ldx     #00
$$loop: dex
        bne     $$loop

;--------------------------------------------------------------------------
; tests of nameless temporary symbols

-  	ldx 	#00
-  	dex
   	bne 	-
   	lda 	RealSymbol
   	beq 	+
   	jsr 	SomeRtn
   	iny
+  	bne 	--

SomeRtn:
	rts

RealSymbol:
	dfs	1

  	inc	ptr
   	bne 	+      		;branch forward to tax
   	inc 	ptr+1 		;parsed as "not a temporary reference" ie. its a math operator
				;or whatever (in the original Buddy assembler, ptr+1 would refer to the high
				;byte of "ptr").
+ 	tax

 	bpl 	++     		;branch forwared to dex
   	beq 	+      		;branch forward to rts
   	lda 	#0
/  	rts            		; <====== slash used as wildcard.
+ 	dex
   	beq 	-       	;branch backward to rts


ptr:	dfs	2
