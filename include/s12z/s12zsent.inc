		ifndef	__s12z_sent_inc
__s12z_sent_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZSENT.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z SENTTX                    *
;*                                                                          *
;****************************************************************************

STTICKRATE	equ	$09A0		; SENT Transmitter Tick Rate Register (16 bit)
;PRE		s12zfld	STTICKRATE,14:0	;  SENTTX Tick Rate Bits
STPPULSE	equ	$09A2		; SENT Transmitter Pause-Pulse Register (16 bit)
PPEN		s12zbit.w	STPPULSE,15	;  SENTTX Pause Pulse Enable
PPFIXED		s12zbit.w	STPPULSE,14	;  SENTTX Fixed Length Pause Pulse
PPCOUNT		s12zfld.w	STPPULSE,11:0	;  SENTTX Pause Pulse Count Ticks
STCONFIG	equ	$09A4		; SENT Transmitter Configuration Register (16 bit)
TXINIT		s12zbit.w	STCONFIG,15	;  SENTTX Initialization Enable
TXEN		s12zbit.w	STCONFIG,14	;  SENTTX Pin Enable
DNIBBLECOUNT	s12zfld.w	STCONFIG,3:8	;  SENTTX Data Nibble Count
OPTEDGE		s12zbit.w	STCONFIG,4	;  SENTTX Optimized Rising Edge Position
SINGLE		s12zbit.w	STCONFIG,3	;  SENTTX Single Shot Operation
CRCSCN		s12zbit.w	STCONFIG,2	;  SENTTX CRC includes Status- and Communication Nibble
CRCLEG		s12zbit.w	STCONFIG,1	;  SENTTX CRC Legacy Algorithm Enable
CRCBYP		s12zbit.w	STCONFIG,0	;  SENTTX Automatic CRC generation bypass
STINTEN		equ	$09A6		; SENT Transmitter Interrupt Enable Register
PPREIE		s12zbit	STINTEN,4	;  SENTTX Pause Pulse Rising-Edge Interrupt Enable
TUIE		s12zbit	STINTEN,3	;  SENTTX Transmitter Under-run Interrupt Enable
CSIE		s12zbit	STINTEN,2	;  SENTTX Calibration Pulse Start Interrupt Enable
TCIE		s12zbit	STINTEN,1	;  SENTTX Transmission Complete Interrupt Enable
TBEIE		s12zbit	STINTEN,0	;  SENTTX Transmit-Buffer Empty Interrupt Enable
STINTFLG	equ	$09A7		; SENT Transmitter Interrupt Flag Register
PPRE		s12zbit	STINTFLG,4	;  SENTTX Pause Pulse Rising-Edge Flag
TU		s12zbit	STINTFLG,3	;  SENTTX Transmitter Under-run Flag
CS		s12zbit	STINTFLG,2	;  SENTTX Calibration Pulse Start Flag
TC		s12zbit	STINTFLG,1	;  SENTTX Transmission Complete Flag
TBE		s12zbit	STINTFLG,0	;  SENTTX Transmit-Buffer Empty Flag
STTXBUF		equ	$09A8		; SENT Transmit Buffer (32 bit)
STATCONF	s12zfld	STTXBUF,4:28	;  SENTTX Status and Configuration Nibble
CRC		s12zfld	STTXBUF,4:24	;  SENTTX CRC Nibble
DATA0		s12zfld	STTXBUF,4:20	;  SENTTX Data Nibble 0
DATA1		s12zfld	STTXBUF,4:16	;  SENTTX Data Nibble 1
DATA2		s12zfld	STTXBUF,4:12	;  SENTTX Data Nibble 2
DATA3		s12zfld	STTXBUF,4:8	;  SENTTX Data Nibble 3
DATA4		s12zfld	STTXBUF,4:4	;  SENTTX Data Nibble 4
DATA5		s12zfld	STTXBUF,4:0	;  SENTTX Data Nibble 5

		restore			; re-enable listing

		endif			; __s12z_sent_inc
