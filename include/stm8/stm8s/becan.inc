		ifndef	__stm8sbecaninc	; avoid multiple inclusion
__stm8sbecaninc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File BECAN.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S beCAN                       *
;*                                                                          *
;****************************************************************************

__defbecan	macro	Base
CAN_MCR		label	Base+$00	; CAN master control register
CAN_TTCM	bit	CAN_MCR,7	;  Time Triggered Communication Mode
CAN_ABOM	bit	CAN_MCR,6	;  Automatic Bus-Off Management
CAN_AWUM	bit	CAN_MCR,5	;  Automatic wakeup Mode
CAN_NART	bit	CAN_MCR,4	;  No Automatic Retransmission
CAN_RFLM	bit	CAN_MCR,3	;  Receive FIFO Locked Mode
CAN_TXFP	bit	CAN_MCR,2	;  Transmit FIFO Priority
CAN_SLEEP	bit	CAN_MCR,1	;  Sleep Mode Request
CAN_INRQ	bit	CAN_MCR,0	;  Initialization Request
CAN_MSR		label	Base+$01	; CAN master status register
CAN_RX		bit	CAN_MSR,5	;  Receive
CAN_TX		bit	CAN_MSR,4	;  Transmit
CAN_WKUI	bit	CAN_MSR,3	;  Wakeup Interrupt
CAN_ERRI	bit	CAN_MSR,2	;  Error Interrupt
CAN_SLAK	bit	CAN_MSR,1	;  Sleep Acknowledge
CAN_INAK	bit	CAN_MSR,0	;  Initialization Acknowledge
CAN_TSR		label	Base+$02	; CAN transmit status register
CAN_TXOK2	bit	CAN_TSR,6	;  Transmission OK for mailbox 2
CAN_TXOK1	bit	CAN_TSR,5	;  Transmission OK for mailbox 1
CAN_TXOK0	bit	CAN_TSR,4	;  Transmission OK for mailbox 0
CAN_RQCP2	bit	CAN_TSR,2	;  Request Completed for Mailbox 2
CAN_RQCP1	bit	CAN_TSR,1	;  Request Completed for Mailbox 1
CAN_RQCP0	bit	CAN_TSR,0	;  Request Completed for Mailbox 0
CAN_TPR		label	Base+$03	; CAN transmit priority register
CAN_LOW2	bit	CAN_TPR,7	;  Lowest Priority Flag for Mailbox 2
CAN_LOW1	bit	CAN_TPR,6	;  Lowest Priority Flag for Mailbox 1
CAN_LOW0	bit	CAN_TPR,5	;  Lowest Priority Flag for Mailbox 0
CAN_TME2	bit	CAN_TPR,4	;  Transmit Mailbox 2 Empty
CAN_TME1	bit	CAN_TPR,3	;  Transmit Mailbox 1 Empty
CAN_TME0	bit	CAN_TPR,2	;  Transmit Mailbox 0 Empty
CAN_CODE	bfield	CAN_TPR,0,2	;  Mailbox Code
CAN_RFR		label	Base+$04	; CAN receive FIFO register
RFOM		bit	CAN_RFR,5	;  Release FIFO Output Mailbox
FOVR		bit	CAN_RFR,4	;  FIFO Overrun
FULL		bit	CAN_RFR,3	;  FIFO Full
FMP		bfield	CAN_RFR,0,2	;  FIFO Message Pending
CAN_IER		label	Base+$05	; CAN interrupt enable register
CAN_WKUIE	bit	CAN_IER,7	;  Wakeup Interrupt Enable
CAN_FOVIE	bit	CAN_IER,3	;  FIFO Overrun Interrupt Enable
CAN_FFIE	bit	CAN_IER,2	;  FIFO Full Interrupt Enable
CAN_FMPIE	bit	CAN_IER,1	;  FIFO Message Pending Interrupt Enable
CAN_TMEIE	bit	CAN_IER,0	;  Transmit Mailbox Empty Interrupt Enable
CAN_DGR		label	Base+$06	; CAN diagnosis register
CAN_TXM2E	bit	CAN_DGR,4	;  TX Mailbox 2 enable
CAN_RXSIG	bit	CAN_DGR,3	;  CAN Rx Signal
CAN_SAMP	bit	CAN_DGR,2	;  Last sample point
CAN_SILM	bit	CAN_DGR,1	;  Silent mode
CAN_LBKM	bit	CAN_DGR,0	;  Loop back mode
CAN_PSR		label	Base+$07	; CAN page selection register
CAN_PS		bfield	CAN_FPSR,0,3	;  Page select
CAN_P0		label	Base+$08	; CAN paged register 0
CAN_P1		label	Base+$09	; CAN paged register 1
CAN_P2		label	Base+$0a	; CAN paged register 2
CAN_P3		label	Base+$0b	; CAN paged register 3
CAN_P4		label	Base+$0c	; CAN paged register 4
CAN_P5		label	Base+$0d	; CAN paged register 5
CAN_P6		label	Base+$0e	; CAN paged register 6
CAN_P7		label	Base+$0f	; CAN paged register 7
CAN_P8		label	Base+$10	; CAN paged register 8
CAN_P9		label	Base+$11	; CAN paged register 9
CAN_PA		label	Base+$12	; CAN paged register A
CAN_PB		label	Base+$13	; CAN paged register B
CAN_PC		label	Base+$14	; CAN paged register C
CAN_PD		label	Base+$15	; CAN paged register D
CAN_PE		label	Base+$16	; CAN paged register E
CAN_PF		label	Base+$17	; CAN paged register F
		; Tx Mailbox 0/1/2 -> Page 0/1/5, Receive FIFO -> Page 7
CAN_MCSR	label	CAN_P0		; CAN message control/status register
CAN_TERR	bit	CAN_MCSR,5	;  Transmission error
CAN_ALST	bit	CAN_MCSR,4	;  Arbitration lost
CAN_TXOK	bit	CAN_MCSR,3	;  Transmission OK
CAN_RQCP	bit	CAN_MCSR,2	;  Request completed
CAN_ABRQ	bit	CAN_MCSR,1	;  Abort request for mailbox
CAN_TXRQ	bit	CAN_MCSR,0	;  Transmit mailbox request
CAN_MDLCR	label	CAN_P1		; CAN mailbox data length control register
CAN_TGT		bit	CAN_MDLCR,7	;  Transmit global time
CAN_DLC		bfield	CAN_MDLCR,0,4	;  Data length code
CAN_MIDR1	label	CAN_P2		; CAN mailbox identifier register 1
CAN_IDE		bit	CAN_MIDR1,6	;  Extended identifier
CAN_RTR		bit	CAN_MIDR1,5	;  Remote transmission request
CAN_MIDR2	label	CAN_P3
CAN_MIDR3	label	CAN_P4
CAN_MIDR4	label	CAN_P5
CAN_MDAR1	label	CAN_P6		; CAN mailbox data register x
CAN_MDAR2	label	CAN_P7
CAN_MDAR3	label	CAN_P8
CAN_MDAR4	label	CAN_P9
CAN_MDAR5	label	CAN_PA
CAN_MDAR6	label	CAN_PB
CAN_MDAR7	label	CAN_PC
CAN_MDAR8	label	CAN_PD
CAN_MTSRL	label	CAN_PE		; CAN mailbox time stamp register low
CAN_MTSRH	label	CAN_PF		; CAN mailbox time stamp register high
		; Acceptance Filter 0/1 -> Page 2
CAN_F0R1	label	CAN_P0
CAN_F0R2	label	CAN_P1
CAN_F0R3	label	CAN_P2
CAN_F0R4	label	CAN_P3
CAN_F0R5	label	CAN_P4
CAN_F0R6	label	CAN_P5
CAN_F0R7	label	CAN_P6
CAN_F0R8	label	CAN_P7
CAN_F1R1	label	CAN_P8
CAN_F1R2	label	CAN_P9
CAN_F1R3	label	CAN_PA
CAN_F1R4	label	CAN_PB
CAN_F1R5	label	CAN_PC
CAN_F1R6	label	CAN_PD
CAN_F1R7	label	CAN_PE
CAN_F1R8	label	CAN_PF
		; Acceptance Filter 2/3 -> Page 3
CAN_F2R1	label	CAN_P0
CAN_F2R2	label	CAN_P1
CAN_F2R3	label	CAN_P2
CAN_F2R4	label	CAN_P3
CAN_F2R5	label	CAN_P4
CAN_F2R6	label	CAN_P5
CAN_F2R7	label	CAN_P6
CAN_F2R8	label	CAN_P7
CAN_F3R1	label	CAN_P8
CAN_F3R2	label	CAN_P9
CAN_F3R3	label	CAN_PA
CAN_F3R4	label	CAN_PB
CAN_F3R5	label	CAN_PC
CAN_F3R6	label	CAN_PD
CAN_F3R7	label	CAN_PE
CAN_F3R8	label	CAN_PF
		; Acceptance Filter 4/5 -> Page 4
CAN_F4R1	label	CAN_P0
CAN_F4R2	label	CAN_P1
CAN_F4R3	label	CAN_P2
CAN_F4R4	label	CAN_P3
CAN_F4R5	label	CAN_P4
CAN_F4R6	label	CAN_P5
CAN_F4R7	label	CAN_P6
CAN_F4R8	label	CAN_P7
CAN_F5R1	label	CAN_P8
CAN_F5R2	label	CAN_P9
CAN_F5R3	label	CAN_PA
CAN_F5R4	label	CAN_PB
CAN_F5R5	label	CAN_PC
CAN_F5R6	label	CAN_PD
CAN_F5R7	label	CAN_PE
CAN_F5R8	label	CAN_PF
		; Configuration/Diagnostic -> Page 6
CAN_ESR		label	CAN_P0		; CAN error status register
CAN_LEC		bfield	CAN_ESR,4,3	;  Last error code
CAN_BOFF	bit	CAN_ESR,2	;  Bus-off flag
CAN_EPVF	bit	CAN_ESR,1	;  Error passive flag
CAN_EWGF	bit	CAN_ESR,0	;  Error warning flag
CAN_EIER	label	CAN_P1		; CAN error interrupt enable register
CAN_ERRIE	bit	CAN_EIER,7	;  Error interrupt enable
CAN_LECIE	bit	CAN_EIER,4	;  Last error code interrupt enable
CAN_BOFIE	bit	CAN_EIER,2	;  Bus-Off interrupt enable
CAN_EPVIE	bit	CAN_EIER,1	;  Error passive interrupt enable
CAN_EWGIE	bit	CAN_EIER,0	;  Error warning interrupt enable
CAN_TECR	label	CAN_P2		; CAN transmit error counter register
CAN_RECR	label	CAN_P3		; CAN receive error counter register
CAN_BTR1	label	CAN_P4		; CAN bit timing register 1
CAN_SJW		bfield	CAN_BTR1,6,2	;  Resynchronization jump width
CAN_BRP		bfield	CAN_BTR1,0,6	;  Baud rate prescaler
CAN_BTR2	label	CAN_P5		; CAN bit timing register 2
CAN_BS2		bfield	CAN_BTR2,4,4	;  Bit Segment 2
CAN_BS1		bfield	CAN_BTR2,0,4	;  Bit Segment 1
CAN_FMR1	label	CAN_P8		; CAN filter mode register 1
CAN_FMH3	bit	CAN_FMR1,7	;  Filter 3 mode high
CAN_FML3	bit	CAN_FMR1,6	;  Filter 3 mode low
CAN_FMH2	bit	CAN_FMR1,5	;  Filter 2 mode high
CAN_FML2	bit	CAN_FMR1,4	;  Filter 2 mode low
CAN_FMH1	bit	CAN_FMR1,3	;  Filter 1 mode high
CAN_FML1	bit	CAN_FMR1,2	;  Filter 1 mode low
CAN_FMH0	bit	CAN_FMR1,1	;  Filter 0 mode high
CAN_FML0	bit	CAN_FMR1,0	;  Filter 0 mode low
CAN_FMR2	label	CAN_P9		; CAN filter mode register 2
CAN_FMH5	bit	CAN_FMR2,3	;  Filter 5 mode high
CAN_FML5	bit	CAN_FMR2,2	;  Filter 5 mode low
CAN_FMH4	bit	CAN_FMR2,1	;  Filter 4 mode high
CAN_FML4	bit	CAN_FMR2,0	;  Filter 4 mode low
CAN_FCR1	label	CAN_PA		; CAN filter configuration register 1
CAN_FSC11	bit	CAN_FCR1,6	;  Filter 1 scale configuration
CAN_FSC10	bit	CAN_FCR1,5
CAN_FACT1	bit	CAN_FCR1,4	;  Filter 1 Active
CAN_FSC01	bit	CAN_FCR1,2	;  Filter 0 scale configuration
CAN_FSC00	bit	CAN_FCR1,1
CAN_FACT0	bit	CAN_FCR1,0	;  Filter 0 active
CAN_FCR2	label	CAN_PB
CAN_FSC31	bit	CAN_FCR2,6	;  Filter 3 scale configuration
CAN_FSC30	bit	CAN_FCR2,5
CAN_FACT3	bit	CAN_FCR2,4	;  Filter 3 Active
CAN_FSC21	bit	CAN_FCR2,2	;  Filter 2 scale configuration
CAN_FSC20	bit	CAN_FCR2,1
CAN_FACT2	bit	CAN_FCR2,0	;  Filter 2 active
CAN_FCR3	label	CAN_PC
CAN_FSC51	bit	CAN_FCR3,6	;  Filter 5 scale configuration
CAN_FSC50	bit	CAN_FCR3,5
CAN_FACT5	bit	CAN_FCR3,4	;  Filter 5 Active
CAN_FSC41	bit	CAN_FCR3,2	;  Filter 4 scale configuration
CAN_FSC40	bit	CAN_FCR3,1
CAN_FACT4	bit	CAN_FCR3,0	;  Filter 4 active
		; Rx Mailbox -> Page ?
CAN_MFMIR	label	CAN_P0		; CAN mailbox filter match index register
		endm

		restore
		endif			; __stm8sbecaninc
