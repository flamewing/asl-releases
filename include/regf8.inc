                ifndef  __regf8inc     ; avoid multiple inclusion
__regf8inc	equ     1

                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGF8.INC                                               *
;*                                                                          *
;*   Purpose : Contains SFR and Memory Definitions for F8 Processors        *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; include proper CPU-specific register definitions

		switch	MOMCPUNAME
		; no ROM, no XRAM, no Serial I/O
		case    "F3850","MK3850"
__hasserial       equ	0

		; no ROM, 64b XRAM @ 4KB, no Serial I/O
		case	"MK3874","MK38P70/02","MK38CP70/02","MK97400","MK97410"
XRAMSTART	  label 0fc0h
XRAMEND		  label	0fffh
__hasserial	  equ	0

		; no ROM, 64b XRAM @ 64KB, no Serial I/O
		case	"MK97500","MK97501","MK97503"
XRAMSTART	  label 0ffc0h
XRAMEND		  label	0ffffh
__hasserial	  equ	0

		; 1K ROM, no XRAM, no Serial I/O
		case	"MK3870/10","MK38C70/10"
ROMSTART	  label	0
ROMEND		  label 3ffh
__hasserial	  equ	0

		; 1K ROM, 64b XRAM, no Serial I/O
		case	"MK3870/12"
ROMSTART	  label	0
ROMEND		  label 3ffh
XRAMSTART	  label 0fc0h
XRAMEND		  label	0fffh
__hasserial	  equ	0

		; 2K ROM, no XRAM, no Serial I/O
		case	"MK3870","MK38C70","MK3870/20","MK38C70/20"
ROMSTART	  label	0
ROMEND		  label	7ffh
__hasserial	  equ	0

		; 2K ROM, 64b XRAM, no Serial I/O
		case	"MK3870/22","MK3875/22","MK3876"
ROMSTART	  label	0
ROMEND		  label	7ffh
XRAMSTART	  label 0fc0h
XRAMEND		  label	0fffh
__hasserial	  equ	0

		; 3K ROM, no XRAM, no Serial I/O
		case	"MK3870/30"
ROMSTART	  label	0
ROMEND		  label	0bffh
__hasserial	  equ	0

		; 3K ROM, 64b XRAM, no Serial I/O
		case	"MK3870/32"
ROMSTART	  label	0
ROMEND		  label	0bffh
XRAMSTART	  label 0fc0h
XRAMEND		  label	0fffh
__hasserial	  equ	0

		; 4K ROM, no XRAM, no Serial I/O
		case	"MK3870/40"
ROMSTART	  label	0
ROMEND		  label	0fffh
__hasserial	  equ	0

		; 4K ROM, 64b XRAM, no Serial I/O
		case	"MK3870/42","MK3872","MK3875","MK3875/42"
ROMSTART	  label	0
ROMEND		  label	0fbfh
XRAMSTART	  label 0fc0h
XRAMEND		  label	0fffh
__hasserial	  equ	0

		; 1K ROM, no XRAM, Serial I/O
		case	"MK3873/10"
ROMSTART	  label	0
ROMEND		  label 3ffh
__hasserial	  equ	1

		; 1K ROM, 64b XRAM, Serial I/O
		case	"MK3873/12"
ROMSTART	  label	0
ROMEND		  label 3ffh
XRAMSTART	  label 0fc0h
XRAMEND		  label	0fffh
__hasserial	  equ	1

		; 2K ROM, no XRAM, Serial I/O
		case	"MK3873","MK3873/20"
ROMSTART	  label	0
ROMEND		  label 7ffh
__hasserial	  equ	1

		; 2K ROM, 64b XRAM, Serial I/O
		case	"MK3873/22"
ROMSTART	  label	0
ROMEND		  label 7ffh
XRAMSTART	  label 0fc0h
XRAMEND		  label	0fffh
__hasserial	  equ	1

		elsecase
		  error  "wrong processor target set: only F3850, MK3850, MK3870, MK3870/10, MK3870/12,"
		  error  "MK3870/20, MK3870/22, MK3870/30, MK3870/32, MK3870/40, MK3870/42, MK3872,"
		  error	 "MK3873, MK3873/10, MK3873/12, MK3873/20, MK3873/22, MK3874, MK3875, MK3875/22,"
		  error	 "MK3875/42, MK3876, MK38P70/02, MK38C70, MK38C70/10, MK38C70/20, MK97400,"
		  fatal	 "MK97410, MK97500, MK97501, or MK97503 allowed!"
                endcase

                if      MOMPASS=1
                 message "F8-Definitions (C) 2018 Alfred Arnold, Haruo Asano"
                endif

;----------------------------------------------------------------------------
; Ports

PORT0		port	0
PORT1		port	1
PORT4		port	4
PORT5		port	5

;----------------------------------------------------------------------------
; Interrupt Control

PORT6		port	6
EI_EN		equ	1 << 0		; External Interrupt Enable
TI_EN		equ	1 << 1		; Timer Interrupt Enable
EI_LEVEL	equ	1 << 2		; External Interrupt Active Level
T_START		equ	1 << 3		; Start/Stop Timer
T_MODE		equ	1 << 4		; Pulse Width/internal Timer
T_SCALE_2	equ	1 << 5		; /2 Prescale
T_SCALE_5	equ	1 << 6		; /5 Prescale
T_SCALE_20	equ	1 << 7		; /20 Prescale

;----------------------------------------------------------------------------
; Timer

PORT7		port	7

;----------------------------------------------------------------------------
; Serial In/Out

		if	__hasserial
		
PORTC		port	12		; Baud Rate Control Port
PORTD		port	13		; Serial Port Control & Status Register
SI_EN		equ	1 << 0		;  (w) Serial Port Interrupt Enable
SER_TXRX	equ	1 << 1		;  (w) Transmit/Receive Operation
SER_MODE	equ	1 << 2		;  (w) Synchronous/Asynchronous operation
SER_SEARCH	equ	1 << 3		;  (w) Search Mode
SER_SDET	equ	1 << 4		;  (w) Start Detect
SER_N0		equ	1 << 5		;  (w) Word Length Select
SER_N1		equ	1 << 6
SER_N2		equ	1 << 7
SER_OUFL	equ	1 << 6		;  (r) overflow/underflow
SER_RDY		equ	1 << 7		;  (r) ready
PORTE		port	14		; Shift Register Buffer Upper Half
PORTF		port	15		; Shift Register Buffer Lower Half		
		
		endif			; __hasserial

;----------------------------------------------------------------------------

                restore                 ; wieder erlauben

                endif			; __regf8inc
