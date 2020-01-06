		ifndef	__pxsinc	; avoid multiple inclusion
__pxsinc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File PXS.INC                                                 *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8T ProxSense                   *
;*                                                                          *
;*   Source: RM0312 (DocID022352 Rev 3)                                     *
;*                                                                          *
;****************************************************************************

__defpxs	macro	Base
PXS_CR1		label	Base+$00		; ProxSense control register 1
PXS_PXSEN	bit	PXS_CR1,7		;  ProxSense enable
PXS_START	bit	PXS_CR1,6		;  Start conversion
PXS_LOW_POWER	bit	PXS_CR1,5		;  Low power mode
PXS_CR2		label	Base+$01		; ProxSense control register 2
PXS_EOCITEN	bit	PXS_CR2,7		;  End of conversion interrupt enable
PXS_FCCITEN	bit	PXS_CR2,6		;  First conversion completion interrupt enable
PXS_NOISEDETEN	bit	PXS_CR2,5		;  Noise detection enable
PXS_RXGROUP	bit	PXS_CR2,3		;  Rx group selection
PXS_RXCOUPLING	bit	PXS_CR2,2		;  Reduce coupling between receiver lines
PXS_SYNCEN	bit	PXS_CR2,1		;  Enable synchronization (SYNC) feature
PXS_SYNCEDGE	bit	PXS_CR2,0		;  Synchronization edge selection
PXS_CR3		label	Base+$02		; ProxSense control register 3
PXS_STAB	bfield	PXS_CR3,6,2		;  Selection for stabilization time after ProxSense power-on
PXS_BIAS	bfield	PXS_CR3,4,2		;  Sample and hold strength selection
PXS_VTHR	bfield	PXS_CR3,0,4		;  Threshold voltage (VTHR) selection
PXS_ISR		label	Base+$04		; ProxSense interrupt and status register
PXS_EOCF	bit	PXS_ISR,7		;  End of conversion flag
PXS_FCCF	bit	PXS_ISR,6		;  First conversion completion flag
PXS_NOISEDETF	bit	PXS_ISR,5		;  Noise detection flag
PXS_CIPF	bit	PXS_ISR,4		;  Conversion in progress flag
PXS_SYNCPF	bit	PXS_ISR,3		;  Synchronization (SYNC) pending flag
PXS_SYNC_OVRF	bit	PXS_ISR,2		;  Synchronization (SYNC) overflow flag
PXS_CKCR1	label	Base+$06		; ProxSense clock control register 1
PXS_PRESC	bfield	PXS_CKCR1,4,3		;  Frequency selection for ProxSense clock
PXS_ANADEAD	bit	PXS_CKCR1,1		;  Ensures the deadtime (non-overlap interval) between the UP and PASS phases using a short analog delay rather than a half-cycle deadtime
PXS_INCPHASE	bit	PXS_CKCR1,0		;  Increases the length of each of the UP and PASS phases by one half of a clock cycle
PXS_CKCR2	label	Base+$07		; ProxSense clock control register 2
UPLEN		bfield	PXS_CKCR2,4,3		;  Length of UP phase
PASSLEN		bfield	PXS_CKCR2,0,3		;  Length of PASS phase
PXS_RXENRH	label	Base+$08		; ProxSense receiver enable register high
PXS_RXEN9	bit	PXS_RXENRH,1		;  Enable receiver channel 9
PXS_RXEN8	bit	PXS_RXENRH,0		;  Enable receiver channel 8
PXS_RXENRL	label	Base+$09		; ProxSense receiver enable register low
PXS_RXEN7	bit	PXS_RXENRL,7		;  Enable receiver channel 7
PXS_RXEN6	bit	PXS_RXENRL,6		;  Enable receiver channel 6
PXS_RXEN5	bit	PXS_RXENRL,5		;  Enable receiver channel 5
PXS_RXEN4	bit	PXS_RXENRL,4		;  Enable receiver channel 4
PXS_RXEN3	bit	PXS_RXENRL,3		;  Enable receiver channel 3
PXS_RXEN2	bit	PXS_RXENRL,2		;  Enable receiver channel 2
PXS_RXEN1	bit	PXS_RXENRL,1		;  Enable receiver channel 1
PXS_RXEN0	bit	PXS_RXENRL,0		;  Enable receiver channel 0
__N		set	0
		rept	3
		__decstr __NS,__N+1
PXS_RXCR{__NS}H	label	Base+$0a+(__N*2)	; ProxSense receiver control register n high
PXS_RXCR{__NS}_9 bit	PXS_RXCR{__NS}H,1	; ProxSense receiver channel 9 control bit n
PXS_RXCR{__NS}_8 bit	PXS_RXCR{__NS}H,0	; ProxSense receiver channel 8 control bit n
PXS_RXCR{__NS}L	label	Base+$0b+(__N*2)	; ProxSense receiver control register n low
PXS_RXCR{__NS}_7 bit	PXS_RXCR{__NS}L,7	; ProxSense receiver channel 7 control bit n
PXS_RXCR{__NS}_6 bit	PXS_RXCR{__NS}L,6	; ProxSense receiver channel 6 control bit n
PXS_RXCR{__NS}_5 bit	PXS_RXCR{__NS}L,5	; ProxSense receiver channel 5 control bit n
PXS_RXCR{__NS}_4 bit	PXS_RXCR{__NS}L,4	; ProxSense receiver channel 4 control bit n
PXS_RXCR{__NS}_3 bit	PXS_RXCR{__NS}L,3	; ProxSense receiver channel 3 control bit n
PXS_RXCR{__NS}_2 bit	PXS_RXCR{__NS}L,2	; ProxSense receiver channel 2 control bit n
PXS_RXCR{__NS}_1 bit	PXS_RXCR{__NS}L,1	; ProxSense receiver channel 1 control bit n
PXS_RXCR{__NS}_0 bit	PXS_RXCR{__NS}L,0	; ProxSense receiver channel 0 control bit n
__N		set	__N+1
		endm
PXS_RXINSRH	label	Base+$12		; ProxSense receiver inactive state register high
PXS_RXINS9	bit	PXS_RXINSRH,1		;  Receiver channel 9 inactive state selection
PXS_RXINS8	bit	PXS_RXINSRH,0		;  Receiver channel 8 inactive state selection
PXS_RXINSRL	label	Base+$13		; ProxSense receiver inactive state register low
PXS_RXINS7	bit	PXS_RXINSRL,7		;  Receiver channel 7 inactive state selection
PXS_RXINS6	bit	PXS_RXINSRL,6		;  Receiver channel 6 inactive state selection
PXS_RXINS5	bit	PXS_RXINSRL,5		;  Receiver channel 5 inactive state selection
PXS_RXINS4	bit	PXS_RXINSRL,4		;  Receiver channel 4 inactive state selection
PXS_RXINS3	bit	PXS_RXINSRL,3		;  Receiver channel 3 inactive state selection
PXS_RXINS2	bit	PXS_RXINSRL,2		;  Receiver channel 2 inactive state selection
PXS_RXINS1	bit	PXS_RXINSRL,1		;  Receiver channel 1 inactive state selection
PXS_RXINS0	bit	PXS_RXINSRL,0		;  Receiver channel 0 inactive state selection
PXS_TXENRH	label	Base+$16		; ProxSense transmit enable register high
PXS_TXEN15	bit	PXS_TXENRH,7		;  Transmit output 15 function enable
PXS_TXEN14	bit	PXS_TXENRH,6		;  Transmit output 14 function enable
PXS_TXEN13	bit	PXS_TXENRH,5		;  Transmit output 13 function enable
PXS_TXEN12	bit	PXS_TXENRH,4		;  Transmit output 12 function enable
PXS_TXEN11	bit	PXS_TXENRH,3		;  Transmit output 11 function enable
PXS_TXEN10	bit	PXS_TXENRH,2		;  Transmit output 10 function enable
PXS_TXEN9	bit	PXS_TXENRH,1		;  Transmit output 9 function enable
PXS_TXEN8	bit	PXS_TXENRH,0		;  Transmit output 8 function enable
PXS_TXENRL	label	Base+$17		; ProxSense transmit enable register low
PXS_TXEN7	bit	PXS_TXENRH,7		;  Transmit output 7 function enable
PXS_TXEN6	bit	PXS_TXENRH,6		;  Transmit output 6 function enable
PXS_TXEN5	bit	PXS_TXENRH,5		;  Transmit output 5 function enable
PXS_TXEN4	bit	PXS_TXENRH,4		;  Transmit output 4 function enable
PXS_TXEN3	bit	PXS_TXENRH,3		;  Transmit output 3 function enable
PXS_TXEN2	bit	PXS_TXENRH,2		;  Transmit output 2 function enable
PXS_TXEN1	bit	PXS_TXENRH,1		;  Transmit output 1 function enable
PXS_TXEN0	bit	PXS_TXENRH,0		;  Transmit output 0 function enable
PXS_MAXRH	label	Base+$1a		; ProxSense maximum counter value register high
PXS_MAXRL	label	Base+$1b		; ProxSense maximum counter value register low
PXS_MAXENRH	label	Base+$1c		; ProxSense maximum counter enable register high
PXS_MAXEN9	bit	PXS_MAXENRH,1		;  Enabled maximum counter limit for Receive channel 9
PXS_MAXEN8	bit	PXS_MAXENRH,0		;  Enabled maximum counter limit for Receive channel 8
PXS_MAXENRL	label	Base+$1d		; ProxSense maximum counter enable register low
PXS_MAXEN7	bit	PXS_MAXENRL,7		;  Enabled maximum counter limit for Receive channel 7
PXS_MAXEN6	bit	PXS_MAXENRL,6		;  Enabled maximum counter limit for Receive channel 6
PXS_MAXEN5	bit	PXS_MAXENRL,5		;  Enabled maximum counter limit for Receive channel 5
PXS_MAXEN4	bit	PXS_MAXENRL,4		;  Enabled maximum counter limit for Receive channel 4
PXS_MAXEN3	bit	PXS_MAXENRL,3		;  Enabled maximum counter limit for Receive channel 3
PXS_MAXEN2	bit	PXS_MAXENRL,2		;  Enabled maximum counter limit for Receive channel 2
PXS_MAXEN1	bit	PXS_MAXENRL,1		;  Enabled maximum counter limit for Receive channel 1
PXS_MAXEN0	bit	PXS_MAXENRL,0		;  Enabled maximum counter limit for Receive channel 0
PXS_RXSRH	label	Base+$1e		; ProxSense receiver status register high
PXS_VALID9	bit	PXS_RXSRH,1		;  Valid bit for conversion data for receiver channel 9 is valid
PXS_VALID8	bit	PXS_RXSRH,0		;  Valid bit for conversion data for receiver channel 8 is valid
PXS_RXSRL	label	Base+$1f		; ProxSense receiver status register low
PXS_VALID7	bit	PXS_RXSRL,7		;  Valid bit for conversion data for receiver channel 7 is valid
PXS_VALID6	bit	PXS_RXSRL,6		;  Valid bit for conversion data for receiver channel 6 is valid
PXS_VALID5	bit	PXS_RXSRL,5		;  Valid bit for conversion data for receiver channel 5 is valid
PXS_VALID4	bit	PXS_RXSRL,4		;  Valid bit for conversion data for receiver channel 4 is valid
PXS_VALID3	bit	PXS_RXSRL,3		;  Valid bit for conversion data for receiver channel 3 is valid
PXS_VALID2	bit	PXS_RXSRL,2		;  Valid bit for conversion data for receiver channel 2 is valid
PXS_VALID1	bit	PXS_RXSRL,1		;  Valid bit for conversion data for receiver channel 1 is valid
PXS_VALID0	bit	PXS_RXSRL,0		;  Valid bit for conversion data for receiver channel 0 is valid
__N		set	0
		rept	10
		__decstr __NS,__N
PXS_RX{__NS}CNTRH	label	Base+$20+(__N*2)	; ProxSense counter register receiver channel n high
PXS_RX{__NS}CNTRL	label	Base+$21+(__N*2)	; ProxSense counter register receiver channel n low
PXS_RX{__NS}CSSELR	label	Base+$40+__N		; ProxSense receiver sampling capacitor selection register n
PXS_RX{__NS}EPCCSELR	label	Base+$50+__N		; ProxSense receiver electrode parasitic compensation capacitor selection register n
__N		set	__N+1
		endm
		endm

		restore
		endif			; __pxsinc
