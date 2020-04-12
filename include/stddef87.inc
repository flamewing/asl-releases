		ifndef  stddef87inc     ; avoid multiple inclusion
stddef87inc     equ     1

		save
		listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF87.INC                                            *
;*                                                                          *
;*   Contains Register and Memory Addresses for TLCS-870 Processors         *
;*   Port and Register Namen according to Toshiba Convention                *
;*                                                                          *
;****************************************************************************

                if      (MOMCPU<>556032)&&(MOMCPU<>556064)&&(MOMCPU<>556096)&&(MOMCPU<>556144)
		 fatal  "wrong target set: only 87C00, 87C20, 87C40 oder 87C70 supported"
		endif

                if      MOMPASS=1
		 message "TLCS-870 Address Definitions"
		 message "(C) 1994 Alfred Arnold"
                 message "Target: \{MOMCPU}"
		endif

;----------------------------------------------------------------------------
; Ports

P0		equ	00h
P1		equ	01h
P2		equ	02h
P3		equ	03h
P4		equ	04h
P5		equ	05h
P6		equ	06h
                if      MOMCPU<>87C20h
P7		 equ	07h
		endif
                if      MOMCPU=87C70h
P8		 equ	08h
P9		 equ	09h
		endif
P0CR		equ	0Ah
P1CR		equ	0BH
                if      MOMCPU=87C70h
P4CR		 equ	0Ch
		endif
                if      (MOMCPU=87C00h)||(MOMCPU=87C40h)
P6CR		 equ	0Ch
P7CR		 equ	0Dh
		endif
                if      MOMCPU=87C20h
P6CR		 equ	29h
		endif

; Keyboard Interface

                if      MOMCPU=87C70h
KEYDR		 equ	P4
KEYSR		 equ	2Ah
		endif

; Comparators

                if      MOMCPU=87C70h
CMPCR		 equ	0Eh
CMPDR		 equ	0Fh
		endif

; Timers: a bit messy

                if      MOMCPU=87C70h
TREG1		 equ	10h
TREG1L		 equ	TREG1
TREG1H		 equ	TREG1+1
		elseif
TREG1A		 equ	10h
TREG1AL		 equ	TREG1A
TREG1AH		 equ	TREG1A+1
TREG1B		 equ	12h
                 if     MOMCPU<>87C20h
TREG1BL		  equ	TREG1B
TREG1BH		  equ	TREG1B+1
		 elseif
TC1CR		  equ	13h
		 endif
		endif
                if      MOMCPU=87C20h
TREG5		 equ	1Dh
TC5CR		 equ	1Eh
TREG6		 equ	16h
TC6CR		 equ	17h
		elseif
TC1CR		 equ	14h
TC2CR		 equ	15h
                 if     MOMCPU<>87C70h
TREG2		  equ	16h
TREG2L		  equ	TREG2
TREG2H		  equ	TREG2+1
		 endif
		endif
TREG3A		equ	18h
TREG3B		equ	19h
TC3CR		equ	1Ah
TREG4		equ	1Bh
TC4CR		equ	1Ch

; Serial: not that simple either

                if      (MOMCPU=87C00h)||(MOMCPU=87C40h)
SIO1SR		 equ	20h
                 if     MOMCPU=87C40h
SIO1CR1		  equ	20h
SIO1CR2		  equ	21h
SIO2CR1		  equ	22h
SIO2CR2		  equ	23h
SIO2SR		  equ	22h
		 elseif
SIO2SR		  equ	21h
SIO1CR		  equ	20h
SIO2CR		  equ	21h
SIOBCR		  equ	22h
SIOWCR		  equ	23h
		 endif
SIO1_BUFFER	 equ	0FF0h
SIO2_BUFFER	 equ	0FF8h
		elseif
SIOSR		 equ	20h
SIOCR1		 equ	20h
SIOCR2		 equ	21h
SIO_BUFFER	 equ	0FF0h
		endif


; High Speed Serial

                if      MOMCPU=87C70h
HSOSR		 equ	24h
HSOCR		 equ	24h
HSO_BUFFER	 equ	0FF8H
		endif

; AD Converter

                if      MOMCPU=87C40h
ADCCR		 equ	0Eh
ADCDR		 equ	0Fh
		endif

; LCD

                if      MOMCPU=87C20h
LCDCR		 equ	28h
LCD_BUFFER	 equ	0F80h
		endif

; Vacuum Fluorescent Display

                if      MOMCPU=87C70h
VFTCR1		 equ	28h
VFTSR		 equ	29h
VFTCR2		 equ	29h
		endif

; Watchdog

WDTCR1		equ	34h
WDTCR2		equ	35h

; System Control

TBTCR		equ	36h
SYSCR1		equ	38h
SYSCR2		equ	39h
GPSW		equ	3Fh
GRBS		equ	3Fh

; Interrupts

EINTCR		equ	37h
EIR		equ	3Ah
EIRL		equ	EIR
EIRH		equ	EIR+1
IL		equ	3ch
ILL		equ	EIR
ILH		equ	EIR+1

; Fixed Memory Areas

IRAM            equ     000C0H    ; Internal RAM 256/512/512/512 Bytes
                if      MOMCPU=87C00h
IRAMEND          equ    0013FH
		elseif
IRAMEND		 equ	0023FH
		endif
IEAREA          equ     0FFC0H    ; Interrupt Entry Area
IEAREAEND       equ     0FFFFH
IROM            equ     0E000H    ; Internal (P)ROM 8K
IROMEND         equ     0FFFFH
EXTMEM          equ     IRAMEND+1 ; External Memory
MEMEND          equ     IROM-1

;---------------------------------------------------------------------------

		restore		; allow listing again

		endif		; stddef87inc
