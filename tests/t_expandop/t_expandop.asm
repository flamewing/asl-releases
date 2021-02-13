		cpu	1802

		; note this initial "dummy set" before defining the
		; macro is necessary since while reading in the macro,
		; the assembler will scan for a ENDM statement and
		; while doing this, will also perform string expression
                ; expansion in the instruction field .  Not having __COND
                ; set at this point of time would result in an error:

__COND		set	""

		; Note further that while the BROP argument gets expanded
		; in string constants, no implicit uppercase conversion
		; will take place.  Using all-uppercase makes this definition
		; work regardless whether the assembler is in case-sensitive
                ; mode or not:

expandop	macro	BROP,DEST
__COND		set	substr("BROP",1,strlen("BROP"))
		L{__COND} DEST
		endm

		; These pairs of statements will result in the same (long) branch:

		expandop xbr,1000
		lbr	1000
		expandop xbq,1000
		lbq	1000
		expandop xbz,1000
		lbz	1000
		expandop xbdf,1000
		lbdf	1000
		expandop xbnq,1000
		lbnq	1000
		expandop xbnz,1000
		lbnz	1000
		expandop xbnf,1000
		lbnf	1000

