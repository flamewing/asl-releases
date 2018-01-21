		ifndef	__spmcsr37inc
__spm37csrinc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPMCSR.INC                                              *
;*                                                                          *
;*Contains Bit & Register Definitions for Newer ATmega Program Memory Access*
;*                                                                          *
;****************************************************************************

SPMCSR		port	0x37		; Store Program Memory Control/Status Register
SPMIE		avrbit	SPMCSR,7	; Interrupt Enable
RWWSB		avrbit	SPMCSR,6	; Read-While-Write Section Busy
RWWSRE		avrbit	SPMCSR,4	; Read-While-Write Section Read Enable
BLBSET		avrbit	SPMCSR,3	; Boot Lock Bit Set
PGWRT		avrbit	SPMCSR,2	; Page Write
PGERS		avrbit	SPMCSR,1	; Page Erase
SPMEN		avrbit	SPMCSR,0	; Store Program Memory Enable

		restore			; re-enable listing

		endif			; __spmcsr37inc
