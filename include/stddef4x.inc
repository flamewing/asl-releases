                save
                listing off   ; kein Listing über diesen File

;****************************************************************************
;*                                                                          *
;*   AS - File STDDEF4X.INC                                                 *
;*   								            *
;*   Purpose : contains register and address definitions for TMS320C4x-CPUs *
;* 									    *
;*   last change : 2016-08-27                                               *
;*									    *
;****************************************************************************

                ifndef  stddef4xinc     ; avoid multiple inclusion

stddef4xinc     equ     1

                if      (MOMCPUNAME<>"320C40")&&(MOMCPUNAME<>"320C44")
                 fatal  "wrong processor type set: use only 320C40/320C44!"
		endif

                if      MOMPASS=1
                 message "TMS320C4x-Definitionen (C) 2016 Alfred Arnold"
		endif

;------------------------------------------------------------------------------
; Memory Control

GMICR		equ	00100000h	; global memory interface control register
LMICR		equ	00100004h	; local memory interface control register

;------------------------------------------------------------------------------
; Timer

__deftimer	macro	num,base
__CHAN  set     num
__CHANS set     "\{__CHAN}"
T{__CHANS}CTRL          equ     base+0h	; control register
T{__CHANS}CNT           equ     base+4h	; counter register
T{__CHANS}PERIOD        equ     base+8h	; period register
                endm

		__deftimer 0,00100020h
		__deftimer 1,00100030h

;------------------------------------------------------------------------------
; Communication Ports

__defcomm	macro	num,base
__CHAN  set     num
__CHANS set     "\{__CHAN}"
CPCR{__CHANS}            equ    base+0       ; control register
IFIFO{__CHANS}POS0       equ    base+1       ; input FIFO position 0
OFIFO{__CHANS}POS7       equ    base+2       ; output FIFO position 7
P{__CHANS}RESET          equ    base+3       ; software reset  
		endm

		if	MOMCPUNAME="320C40"
		 __defcomm 0,00100040h
		 __defcomm 3,00100070h
		endif

		__defcomm 1,00100050h
		__defcomm 2,00100060h
		__defcomm 4,00100080h
		__defcomm 5,00100090h

;------------------------------------------------------------------------------
; Channel Registers

__defdma	macro	num,base
__CHAN	set	num
__CHANS	set	"\{__CHAN}"
DMA{__CHANS}CTRL	equ	base+0	; control register
DMA{__CHANS}SRCADDR	equ	base+1	; source address
DMA{__CHANS}SRCIDX	equ	base+2	; source address index
DMA{__CHANS}TCNT	equ	base+3	; transfer counter
DMA{__CHANS}DSTADDR	equ	base+4	; destination address
DMA{__CHANS}DSTIDX	equ	base+5	; destination address index
DMA{__CHANS}LINKPTR	equ	base+6	; link pointer
DMA{__CHANS}AUXTCNT	equ	base+7	; auxiliary transfer counter
DMA{__CHANS}AUXLINKPTR	equ	base+8	; auxiliary link pointer
		endm

		__defdma	0,001000a0h
		__defdma	1,001000b0h
		__defdma	2,001000c0h
		__defdma	3,001000d0h
		__defdma	4,001000e0h
		__defdma	5,001000f0h

;------------------------------------------------------------------------------
; Interrupt Vector Addresses

INTVEC_RESET	equ	0
INTVEC_NMI	equ     1
INTVEC_TINT0	equ	2
__TMPINTVEC	set	0
		rept	4
INTVEC_IIOF{"\{__TMPINTVEC}"} equ __TMPINTVEC+3
__TMPINTVEC	set	__TMPINTVEC+1
                endm
__TMPINTVEC	set	0
		rept	6
INTVEC_ICFULL{"\{__TMPINTVEC}"}		equ	__TMPINTVEC*4+0dh
INTVEC_ICRDY{"\{__TMPINTVEC}"}		equ	__TMPINTVEC*4+0eh
INTVEC_OCRDY{"\{__TMPINTVEC}"}		equ	__TMPINTVEC*4+0fh
INTVEC_OCEMPTY{"\{__TMPINTVEC}"}	equ	__TMPINTVEC*4+10h
__TMPINTVEC	set	__TMPINTVEC+1
                endm
__TMPINTVEC	set	0
		rept	6
INTVEC_DMATINT{"\{__TMPINTVEC}"}	equ	__TMPINTVEC+25h
__TMPINTVEC	set	__TMPINTVEC+1
                endm
INTVEC_TINT1	equ	02bh

		endif			; stddef4xinc
		restore			; allow listing again
