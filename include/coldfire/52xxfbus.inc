		ifndef	__52xxfbusinc		; avoid multiple inclusion
__52xxfbusinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXFBUS.INC                                            *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx FlexBus          *
;*                                                                          *
;****************************************************************************

__N		set		0
		rept	6
		__decstr	__NS,__N
CSAR{__NS}	equ		MBAR_FBUS+(__N*12)+0 ; Chip-Select Address Register (32b)
BA		cffield		CSAR{__NS},16,16     ;  Base Address
CSMR{__NS}	equ		MBAR_FBUS+(__N*12)+4 ; Chip-Select Mask Register (32b)
BAM		cffield		CSMR{__NS},16,16     ;  Base Address Mask
WP		cfbit		CSMR{__NS},8         ; Write Protect
V		cfbit		CSMR{__NS},0         ; Valid
CSCR{__NS}	equ		MBAR_FBUS+(__N*12)+8 ; Chip-Select Control Register (32b)
SWS		cffield		CSCR{__NS},26,6	     ;  Secondary Wait States
SWSEN		cfbit		CSCR{__NS},23        ;  Secondary Wait State Enable
ASET		cffield		CSCR{__NS},20,2      ;  Address Setup
RDAH		cffield		CSCR{__NS},18,2      ;  Read Address Hold Or Deselect
WRAH		cffield		CSCR{__NS},16,2      ;  Write Address Hold Or Deselect
WS		cffield		CSCR{__NS},10,6      ;  Wait States
SBM		cfbit		CSCR{__NS},9         ;  Split Bus Mode (not on MCF54xx)
AA		cfbit		CSCR{__NS},8         ;  Auto-Acknowledge Enable
PS		cffield		CSCR{__NS},6,2       ;  Port Size
BEM		cfbit		CSCR{__NS},5         ;  Byte-Enable Mode
BSTR		cfbit		CSCR{__NS},4         ;  Burst-Read Enable
BSTW		cfbit		CSCR{__NS},3         ;  Burst-Write Enable
__N		set		__N+1
		endm

		restore				; re-enable listing

                endif                           ; __52xxfbusinc
