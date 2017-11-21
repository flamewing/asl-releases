		ifndef	__macscinc
__macscinc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MACSC.INC                                               *
;*                                                                          *
;*   Contains bit & register definitions for RFRF2 MAC Symbol Counter       *
;*                                                                          *
;****************************************************************************

SCCSR		sfr	0xdb		; Symbol Counter Compare Source Register
SCCS31		equ	5		; Symbol Counter Compare Source select register for Compare Unit 3
SCCS30		equ	4	
SCCS21		equ	3		; Symbol Counter Compare Source select register for Compare Unit 2
SCCS20		equ	2
SCCS11		equ	1		; Symbol Counter Compare Source select register for Compare Unit 1
SCCS10		equ	0

SCCNTHH		sfr	0xe4		; Symbol Counter Register HH-Byte
SCCNTHL		sfr	0xe3		; Symbol Counter Register HL-Byte
SCCNTLH		sfr	0xe2		; Symbol Counter Register LH-Byte
SCCNTLL		sfr	0xe1		; Symbol Counter Register LL-Byte

SCTSRHH		sfr	0xec		; Symbol Counter Frame Timestamp Register HH-Byte
SCTSRHL		sfr	0xeb		; Symbol Counter Frame Timestamp Register HL-Byte
SCTSRLH		sfr	0xea		; Symbol Counter Frame Timestamp Register LH-Byte
SCTSRLL		sfr	0xe9		; Symbol Counter Frame Timestamp Register LL-Byte

SCTSTRHH	sfr	0xfc		; Symbol Counter Transmit Frame Timestamp Register HH-Byte
SCTSTRHL	sfr	0xfb		; Symbol Counter Transmit Frame Timestamp Register HL-Byte
SCTSTRLH	sfr	0xfa		; Symbol Counter Transmit Frame Timestamp Register LH-Byte
SCTSTRLL	sfr	0xf9		; Symbol Counter Transmit Frame Timestamp Register LL-Byte

SCRSTRHH	sfr	0xda		; Symbol Counter Received Frame Timestamp Register HH-Byte
SCRSTRHL	sfr	0xd9		; Symbol Counter Received Frame Timestamp Register HL-Byte
SCRSTRLH	sfr	0xd8		; Symbol Counter Received Frame Timestamp Register LH-Byte
SCRSTRLL	sfr	0xd7		; Symbol Counter Received Frame Timestamp Register LL-Byte

SCBTSRHH	sfr	0xe8		; Symbol Counter Beacon Timestamp Register HH-Byte
SCBTSRHL	sfr	0xe7		; Symbol Counter Beacon Timestamp Register HL-Byte
SCBTSRLH	sfr	0xe6		; Symbol Counter Beacon Timestamp Register LH-Byte
SCBTSRLL	sfr	0xe5		; Symbol Counter Beacon Timestamp Register LL-Byte

SCOCR1HH	sfr	0xf8		; Symbol Counter Output Compare Register 1 HH-Byte
SCOCR1HL	sfr	0xf7		; Symbol Counter Output Compare Register 1 HL-Byte
SCOCR1LH	sfr	0xf6		; Symbol Counter Output Compare Register 1 LH-Byte
SCOCR1LL	sfr	0xf5		; Symbol Counter Output Compare Register 1 LL-Byte

SCOCR2HH	sfr	0xf4		; Symbol Counter Output Compare Register 2 HH-Byte
SCOCR2HL	sfr	0xf3		; Symbol Counter Output Compare Register 2 HL-Byte
SCOCR2LH	sfr	0xf2		; Symbol Counter Output Compare Register 2 LH-Byte
SCOCR2LL	sfr	0xf1		; Symbol Counter Output Compare Register 2 LL-Byte

SCOCR3HH	sfr	0xf0		; Symbol Counter Output Compare Register 3 HH-Byte
SCOCR3HL	sfr	0xef		; Symbol Counter Output Compare Register 3 HL-Byte
SCOCR3LH	sfr	0xee		; Symbol Counter Output Compare Register 3 LH-Byte
SCOCR3LL	sfr	0xed		; Symbol Counter Output Compare Register 3 LL-Byte

SCCR0		sfr	0xdc		; Symbol Counter Control Register 0
SCRES		equ	7		; Symbol Counter Synchronization
SCMBTS		equ	6		; Manual Beacon Timestamp
SCEN		equ	5		; Symbol Counter enable
SCCKSEL		equ	4		; Symbol Counter Clock Source select
SCTSE		equ	3		; Symbol Counter Automatic Timestamping enable
SCCMP3		equ	2		; Symbol Counter Compare Unit 3 Mode select
SCCMP2		equ	1		; Symbol Counter Compare Unit 2 Mode select
SCCMP1		equ	0		; Symbol Counter Compare Unit 1 Mode select

SCCR1		sfr	0xdd		; Symbol Counter Control Register 1
SCBTSM		equ	5		; Symbol Counter Beacon Timestamp Mask Register
SCCKDIV2	equ	4		; Clock divider for synchronous clock source (16MHz Transceiver Clock)
SCCKDIV1	equ	3
SCCKDIV0	equ	2
SCEECLK		equ	1		; Enable External Clock Source on PG2
SCENBO		equ	0		; Backoff Slot Counter enable

SCSR		sfr	0xde		; Symbol Counter Status Register
SCBSY		equ	0		; Symbol Counter busy

SCIRQS		sfr	0xe0		; Symbol Counter Interrupt Status Register
IRQSBO		equ	4		; Backoff Slot Counter IRQ
IRQSOF		equ	3		; Symbol Counter Overflow IRQ
IRQSCP3		equ	2		; Compare Unit 3 Compare Match IRQ
IRQSCP2		equ	1		; Compare Unit 2 Compare Match IRQ
IRQSCP1		equ	0		; Compare Unit 1 Compare Match IRQ

SCIRQM		sfr	0xdf		; Symbol Counter Interrupt Mask Register
IRQMBO		equ	4		; Backoff Slot Counter IRQ enable
IRQMOF		equ	3		; Symbol Counter Overflow IRQ enable
IRQMCP3		equ	2		; Symbol Counter Compare Match 3 IRQ enable
IRQMCP2		equ	1		; Symbol Counter Compare Match 2 IRQ enable
IRQMCP1		equ	0		; Symbol Counter Compare Match 1 IRQ enable

		restore			; re-enable listing

		endif			; __macscinc
