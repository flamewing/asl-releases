		ifndef	__spmcsr68inc
__spmcsr68inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPMCSR68.INC                                            *
;*                                                                          *
;*Contains Bit & Register Definitions for Newer ATmega Program Memory Access*
;*                                                                          *
;****************************************************************************

SPMCSR		sfr	0x68		; Store Program Memory Control/Status Register
SPMIE		avrbit	SPMCSR,7	; Interrupt Enable
RWWSB		avrbit	SPMCSR,6	; Read-While-Write Section Busy
RWWSRE		avrbit	SPMCSR,4	; Read-While-Write Section Read Enable
BLBSET		avrbit	SPMCSR,3	; Boot Lock Bit set
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SPMEN		avrbit	SPMCSR,0	; Store Program Memory Enable

		restore			; re-enable listing

		endif			; __spmcsr68inc
