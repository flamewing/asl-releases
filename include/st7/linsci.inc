		ifndef	__st7linsciinc	; avoid multiple inclusion
__st7linsciinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File LINSCI.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7 LINSCI                        *
;*                                                                          *
;****************************************************************************

		include	"sci2.inc"

__deflinsci	macro	Base
		__defsci2 Base,6
SCICR3		label	Base+$05	; SCI Control Register 3
LDUM		bit	SCICR3,7	;  LIN Divider Update Method
LINE		bit	SCICR3,6	;  LIN Mode Enable
LSLV		bit	SCICR3,5	;  LIN Slave Mode
LASE		bit	SCICR3,4	;  LIN Auto Synch Enable
LHDM		bit	SCICR3,3	;  LIN Header Detection Method
LHIE		bit	SCICR3,2	;  LIN Header Interrupt Enable
LHDF		bit	SCICR3,1	;  LIN Header Detection Flag
LSF		bit	SCICR3,0	;  LIN Synch Field State
		endm

		restore
		endif			; __st7linsciinc
