		ifndef	__regz86cxxinc
__regz86cxxinc	equ	1
                save
                listing off   ; kein Listing ueber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGZ86CXX.INC                                           *
;*                                                                          *
;*   Contains Bit & Register Definitions for Z86C/Exx                       *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Z86XX common stuff

		include	"z86xx.inc"

;----------------------------------------------------------------------------
; System Control

PCON		sfr	00h		; [ERF Bank F] 
LOW_EMI		__z8bit	PCON,7		;  Low EMI Clock

SMR		sfr	0bh		; [ERF Bank F] Stop Mode Recovery Register
TCLK16		__z8bit	SMR,0		;  SCLK->TCLK Divide by 16
SCLK2		__z8bit SMR,1		;  XTAL->SCLK Divide by 2
SM_REC_SRC	__z8bfield SMR,2,3	;  Stop Mode Recovery Source
STOP_DELAY	__z8bit	SMR,5		;  Stop Delay
STOP_REC_LEVEL	__z8bit SMR,6		;  Stop Recovery Level
STOP_FLAG	__z8bit	SMR,7		;  Stop Flag

RAM_PROTECT	__z8bit IMR,6		; RAM Protect

;----------------------------------------------------------------------------
; GPIO

P1_PSHPULL	__z8bit	PCON,1		; Port 1 Open-Drain(0)
P0_PSHPULL	__z8bit	PCON,2		; Port 0 Open-Drain(0)
P0_LOW_EMI	__z8bit PCON,3		; Port 0 Low EMI(0)
P1_LOW_EMI	__z8bit PCON,4		; Port 1 Low EMI(0)
P2_LOW_EMI	__z8bit PCON,5		; Port 2 Low EMI(0)
P3_LOW_EMI	__z8bit PCON,6		; Port 3 Low EMI(0)

;----------------------------------------------------------------------------
; SPI

		if	__hasspi
SCOMP		 sfr	00h		; [ERF Bank C] SPI Compare
RxBUF		 sfr	01h		; [ERF Bank C] SPI Tx/Rx Data
SCON		 sfr	02h		; [ERF Bank C] SPI Control
SPI_EN		 __z8bit SCON,0		;  Enable SPI
SPI_OVR		 __z8bit SCON,1		;  Overrun
SPI_CLKDIV	 __z8bfield SCON,1,2	;  CLK Divide
SPI_D0_EN	 __z8bit SCON,2		;  DO SPI Port Enable
SPI_COMP_EN	 __z8bit SCON,3		;  Comparator Enable
SPI_RX_AVL	 __z8bit SCON,4		;  Rx Character available
SPI_CLK_PHASE	 __z8bit SCON,5		;  Clock Phase
SPI_CLK_SRC	 __z8bit SCON,6		;  Clock Source
SPI_MASTER	 __z8bit SCON,7		;  Slave/Master
		endif			; __hasspi

;----------------------------------------------------------------------------
; Analog Comparators

AN_ENABLE	__z8bit P3M,1		;  Analog Mode on P31..P33
AN_OUTPUT	__z8bit	PCON,0		;  P34/35/37 as Comparator Outputs

;----------------------------------------------------------------------------
; Watchdog Timer

		if	__haswdt
WDTMR		 sfr	0fh		; [ERF Bank F] Watchdog Timer Mode
WDTTAP		 __z8bfield WDTMR,0,2	; Time-Out Period
WDTHALT		 __z8bit WDTMR,2	; WDT during HALT?
WDTSTOP		 __z8bit WDTMR,3	; WDT during STOP?
WDTXTAL		 __z8bit WDTMR,4	; WDT with XTAL or RC Oscillator? 
		endif

;----------------------------------------------------------------------------

		restore

		endif			; __regz86cxxinc
