		ifndef	__st7spiinc	; avoid multiple inclusion
__st7spiinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPI.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST72xxx SPI                       *
;*                                                                          *
;****************************************************************************

SPIDR		label	$0021		; Data I/O Register
SPICR		label	$0022		; Control Register
SPIE		bit	SPICR,7		;  Serial peripheral interrupt enable
SPE		bit	SPICR,6		;  Serial peripheral output enable
SPR2		bit	SPICR,5		;  Divider Enable
MSTR		bit	SPICR,4		;  Master
CPOL		bit	SPICR,3		;  Clock polarity
CPHA		bit	SPICR,2		;  Clock phase
SPR1		bit	SPICR,1		;  Serial peripheral rate
SPR0		bit	SPICR,0
SPISR		label	$0023		; Status Register
SPIF		bit	SPISR,7		;  Serial Peripheral data transfer flag
WCOL		bit	SPISR,6		;  Write Collision status
MODF		bit	SPISR,4		;  Mode Fault flag

		restore
		endif			; __st7spiinc
