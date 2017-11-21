		ifndef	__spminc
__spminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File SPM.INC                                                 *
;*                                                                          *
;*   Contains bit & register definitions for ATmega program memory access   *
;*                                                                          *
;****************************************************************************

SPMCR		port	0x37		; Store Program Memory Control Register
SPMIE		equ	7		; interrupt enable
RWWSB		equ	6		; read-while-write section busy
RWWSRE		equ	4		; read-while-write section rd enable
BLBSET		equ	3		; boot lock bit set
PGWRT		equ	2		; page write
PGERS		equ	1		; page erase
SPMEN		equ	0		; store program memory enable

		restore			; re-enable listing

		endif			; __spminc
