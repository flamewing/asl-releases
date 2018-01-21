		ifndef	__spminc
__spminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPM.INC                                                 *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega program memory access   *
;*                                                                          *
;****************************************************************************

SPMCR		port	0x37		; Store Program Memory Control Register
SPMIE		avrbit	SPMCR,7		; Interrupt Enable
RWWSB		avrbit	SPMCR,6		; Read-While-Write Section Busy
RWWSRE		avrbit	SPMCR,4		; Read-While-Write Section Rd Enable
BLBSET		avrbit	SPMCR,3		; Boot Lock Bit Set
PGWRT		avrbit	SPMCR,2		; Page Write
PGERS		avrbit	SPMCR,1		; Page Erase
SPMEN		avrbit	SPMCR,0		; Store Program Memory Enable

		restore			; re-enable listing

		endif			; __spminc
