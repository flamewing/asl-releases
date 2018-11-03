		ifndef	__52xxwdtinc		; avoid multiple inclusion
__52xxwdtinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXWDT.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Watchdog Timer   *
;*                                                                          *
;****************************************************************************

WDT_WCR		equ		MBAR_WDT+0	; Watchdog Control Register (16b), renamed to avoid conflict with wakeup control register
WAIT		cfbit		WDT_WCR,3	;  Wait Mode Bit
DOZE		cfbit		WDT_WCR,2	;  Doze Mode Bit
HALTED		cfbit		WDT_WCR,1	;  Halted Mode Bit
EN		cfbit		WDT_WCR,0	;  Watchdog Enable Bit
WMR		equ		MBAR_WDT+2	; Watchdog Modulus Register (16b)
WCNTR		equ		MBAR_WDT+4	; Watchdog Count Register (16b)
WSR		equ		MBAR_WDT+6	; Watchdog Service Register (16b)

		restore				; re-enable listing

                endif                           ; __52xxwdtinc
