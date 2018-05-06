		ifndef	__s12z_dbg_inc
__s12z_dbg_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZDBG.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Debug Controller          *
;*                                                                          *
;****************************************************************************

DBGC1		equ	$0100		; Debug Control Register 1
ARM		s12zbit	DBGC1,7		;  Arm Bit
TRIG		s12zbit	DBGC1,6		;  Immediate Trigger Request Bit
BDMBP		s12zbit	DBGC1,4		;  Background Debug Mode Enable
BRKCPU		s12zbit	DBGC1,3		;  CPU Breakpoint Enable
EEVE1		s12zbit	DBGC1,1		;  External Event Enable
DBGC2		equ	$0101		; Debug Control Register2
ABCM		s12zfld DBGC2,2:0		;  A and B Comparator Match Control
DBGSCR1		equ	$0107		; Debug State Control Register 1
C3SC1		s12zbit	DBGSCR1,7	;  Channel 3 State Control
C3SC0		s12zbit	DBGSCR1,6
C1SC1		s12zbit	DBGSCR1,3	;  Channel 1 State Control
C1SC0		s12zbit	DBGSCR1,2
C0SC1		s12zbit	DBGSCR1,1	;  Channel 0 State Control
C0SC0		s12zbit	DBGSCR1,0
DBGSCR2		equ	$0108		; Debug State Control Register 2
DBGSCR3		equ	$0109		; Debug State Control Register 3
DBGEFR		equ	$010A		; Debug Event Flag Register
TRIGF		s12zbit	DBGEFR,6	;  TRIG Flag
EEVF		s12zbit	DBGEFR,4	;  External Event Flag
ME3		s12zbit	DBGEFR,3	;  Match Event
ME1		s12zbit	DBGEFR,1
ME0		s12zbit	DBGEFR,0
DBGSR		equ	$010B		; Debug Status Register
SSF2		s12zbit	DBGSR,2		;  State Sequencer Flag Bits
SSF1		s12zbit	DBGSR,1
SSF0		s12zbit	DBGSR,0
DBGACTL		equ	$0110		; Debug Comparator A Control Register
NDB		s12zbit	DBGACTL,6	;  Not Data Bus
INST		s12zbit	DBGACTL,5	;  Instruction Select
RW		s12zbit	DBGACTL,3	;  Read/Write Comparator Value Bit
RWE		s12zbit	DBGACTL,2	;  Read/Write Enable Bit
COMPE		s12zbit	DBGACTL,0	;  Enable Bit
DBGAA		equ	$0115		; Debug Comparator A Address Register (24 bits)
DBGAAH		equ	$0115		; Debug Comparator A Address Register MSB
DBGAAM		equ	$0116
DBGAAL		equ	$0117		; Debug Comparator A Address Register LSB
DBGAD		equ	$0118		; Debug Comparator A Data Register (32 bits)
DBGAD0		equ	$0118		; Debug Comparator A Data Register MSB
DBGAD1		equ	$0119
DBGAD2		equ	$011A
DBGAD3		equ	$011B		; Debug Comparator A Data Register LSB
DBGADM		equ	$011C		; Debug Comparator A Data Mask Register (32 bits)
DBGADM0		equ	$011C		; Debug Comparator A Data Mask Register MSB
DBGADM1		equ	$011D
DBGADM2		equ	$011E
DBGADM3		equ	$011F		; Debug Comparator A Data Mask Register LSB
DBGBCTL		equ	$0120		; Debug Comparator B Control Register (same bits as in DBGACTL)
DBGBA		equ	$0125		; Debug Comparator B Address Register (24 bits)
DBGBAH		equ	$0125		; Debug Comparator B Address Register MSB
DBGBAM		equ	$0126
DBGBAL		equ	$0127		; Debug Comparator B Address Register LSB
DBGDCTL		equ	$0140		; Debug Comparator D Control Register (same bits as in DBGACTL)
DBGDA		equ	$0145		; Debug Comparator D Address Register (24 bits)
DBGDAH		equ	$0145		; Debug Comparator D Address Register MSB
DBGDAM		equ	$0146
DBGDAL		equ	$0147		; Debug Comparator D Address Register LSB

		restore			; re-enable listing

		endif			; __s12z_dbg_inc
