		ifndef	__macscinc
__macscinc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File MACSC.INC                                               *
;*                                                                          *
;*   Contains Bit & Register Definitions for RFRF2 MAC Symbol Counter       *
;*                                                                          *
;****************************************************************************

SCCSR		sfr	0xdb		; Symbol Counter Compare Source Register
SCCS31		avrbit	SCCSR,5		; Symbol Counter Compare Source Select Register for Compare Unit 3
SCCS30		avrbit	SCCSR,4	
SCCS21		avrbit	SCCSR,3		; Symbol Counter Compare Source Select Register for Compare Unit 2
SCCS20		avrbit	SCCSR,2
SCCS11		avrbit	SCCSR,1		; Symbol Counter Compare Source Select Register for Compare Unit 1
SCCS10		avrbit	SCCSR,0

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
SCRES		avrbit	SCCR0,7		; Symbol Counter Synchronization
SCMBTS		avrbit	SCCR0,6		; Manual Beacon Timestamp
SCEN		avrbit	SCCR0,5		; Symbol Counter Enable
SCCKSEL		avrbit	SCCR0,4		; Symbol Counter Clock Source Select
SCTSE		avrbit	SCCR0,3		; Symbol Counter Automatic Timestamping Enable
SCCMP3		avrbit	SCCR0,2		; Symbol Counter Compare Unit 3 Mode Select
SCCMP2		avrbit	SCCR0,1		; Symbol Counter Compare Unit 2 Mode Select
SCCMP1		avrbit	SCCR0,0		; Symbol Counter Compare Unit 1 Mode Select

SCCR1		sfr	0xdd		; Symbol Counter Control Register 1
SCBTSM		avrbit	SCCR1,5		; Symbol Counter Beacon Timestamp Mask Register
SCCKDIV2	avrbit	SCCR1,4		; Clock divider for synchronous Clock source (16MHz Transceiver Clock)
SCCKDIV1	avrbit	SCCR1,3
SCCKDIV0	avrbit	SCCR1,2
SCEECLK		avrbit	SCCR1,1		; Enable External Clock Source on PG2
SCENBO		avrbit	SCCR1,0		; Backoff Slot Counter Enable

SCSR		sfr	0xde		; Symbol Counter Status Register
SCBSY		avrbit	SCSR,0		; Symbol Counter busy

SCIRQS		sfr	0xe0		; Symbol Counter Interrupt Status Register
IRQSBO		avrbit	SCIRQS,4	; Backoff Slot Counter IRQ
IRQSOF		avrbit	SCIRQS,3	; Symbol Counter Overflow IRQ
IRQSCP3		avrbit	SCIRQS,2	; Compare Unit 3 Compare Match IRQ
IRQSCP2		avrbit	SCIRQS,1	; Compare Unit 2 Compare Match IRQ
IRQSCP1		avrbit	SCIRQS,0	; Compare Unit 1 Compare Match IRQ

SCIRQM		sfr	0xdf		; Symbol Counter Interrupt Mask Register
IRQMBO		avrbit	SCIRQM,4	; Backoff Slot Counter IRQ Enable
IRQMOF		avrbit	SCIRQM,3	; Symbol Counter Overflow IRQ Enable
IRQMCP3		avrbit	SCIRQM,2	; Symbol Counter Compare Match 3 IRQ Enable
IRQMCP2		avrbit	SCIRQM,1	; Symbol Counter Compare Match 2 IRQ Enable
IRQMCP1		avrbit	SCIRQM,0	; Symbol Counter Compare Match 1 IRQ Enable

		restore			; re-enable listing

		endif			; __macscinc
