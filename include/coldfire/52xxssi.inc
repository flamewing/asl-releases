		ifndef	__52xxssiinc		; avoid multiple inclusion
__52xxssiinc	equ	1

		save
		listing	off			; no listing over this file
		
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File 52XXPIT.INC                                             *
;*                                                                          *
;*   Contains SFR and Bit Definitions for ColdFire MCF52xx Synchronous      *
;*   Serial Interface                                                       *
;*                                                                          *
;****************************************************************************

SSI_TX0		equ		MBAR_SSI+$000	; SSI Transmit Data Register 0 (32b)
SSI_TX1		equ		MBAR_SSI+$004	; SSI Transmit Data Register 1 (32b)
SSI_RX0		equ		MBAR_SSI+$008	; SSI Receive Data Register 0 (32b)
SSI_RX1		equ		MBAR_SSI+$00C	; SSI Receive Data Register 1 (32b)
SSI_CR		equ		MBAR_SSI+$010	; SSI Control Register (32b)
CIS		cfbit		SSI_CR,9	;  Clock idle state.
TCH		cfbit		SSI_CR,8	;  Two channel operation enable.
MCE		cfbit		SSI_CR,7	;  Master clock enable.
I2S		cffield		SSI_CR,5,2	;  I2S mode select.
SYN		cfbit		SSI_CR,4	;  Synchronous mode enable.
NET		cfbit		SSI_CR,3	;  Network mode enable.
RE		cfbit		SSI_CR,2	;  Receiver enable.
TE		cfbit		SSI_CR,1	;  Transmitter.
SSI_EN		cfbit		SSI_CR,0	;  SSI enable.
SSI_ISR		equ		MBAR_SSI+$014	; SSI Interrupt Status Register (32b)
CMDAU		cfbit		SSI_ISR,18	;  AC97 command address register updated.
CMDDU		cfbit		SSI_ISR,17	;  AC97 command data register updated.
RXT		cfbit		SSI_ISR,16	;  AC97 receive tag updated.
RDR1		cfbit		SSI_ISR,15	;  Receive data ready 1.
RDR0		cfbit		SSI_ISR,14	;  Receive data ready 0.
TDE1		cfbit		SSI_ISR,13	;  Transmit data register empty 1.
TDE0		cfbit		SSI_ISR,12	;  Transmit data register empty 0.
ROE1		cfbit		SSI_ISR,11	;  Receiver overrun error 1.
ROE0		cfbit		SSI_ISR,10	;  Receiver overrun error 0.
TUE1		cfbit		SSI_ISR,9	;  Transmitter underrun error 1.
TUE0		cfbit		SSI_ISR,8	;  Transmitter underrun error 0.
TFS		cfbit		SSI_ISR,7	;  Transmit frame sync.
RFS		cfbit		SSI_ISR,6	;  Receive frame sync.
TLS		cfbit		SSI_ISR,5	;  Transmit last time slot.
RLS		cfbit		SSI_ISR,4	;  Receive last time slot.
RFF1		cfbit		SSI_ISR,3	;  Receive FIFO full 1.
RFF0		cfbit		SSI_ISR,2	;  Receive FIFO full 0.
TFE1		cfbit		SSI_ISR,1	;  Transmit FIFO empty 1.
TFE0		cfbit		SSI_ISR,0	;  Transmit FIFO empty 0.
SSI_IER		equ		MBAR_SSI+$018	; SSI Interrupt Enable Register (32b)
RDMAE		cfbit		SSI_IER,22	;  Receive DMA enable.
RIE		cfbit		SSI_IER,21	;  Receive interrupt enable.
TDMAE		cfbit		SSI_IER,20	;  Transmit DMA enable.
TIE		cfbit		SSI_IER,19	;  Transmit interrupt enable.
CMDAU		cfbit		SSI_IER,18	;  AC97 command address register updated interrupt enable.
CMDU		cfbit		SSI_IER,17	;  AC97 command data register updated interrupt enable.
RXT		cfbit		SSI_IER,16	;  AC97 receive tag updated interrupt enable.
RDR1		cfbit		SSI_IER,15	;  Receive data ready 1 interrupt enable.
RDR0		cfbit		SSI_IER,14	;  Receive data ready 0 interrupt enable.
TDE1		cfbit		SSI_IER,13	;  Transmit data register empty 1 interrupt enable.
TDE0		cfbit		SSI_IER,12	;  Transmit data register empty 0 interrupt enable.
ROE1		cfbit		SSI_IER,11	;  Receiver overrun error 1 interrupt enable.
ROE0		cfbit		SSI_IER,10	;  Receiver overrun error 0 interrupt enable.
TUE1		cfbit		SSI_IER,9	;  Transmitter underrun error 1 interrupt enable.
TUE0		cfbit		SSI_IER,8	;  Transmitter underrun error 0 interrupt enable.
TFS		cfbit		SSI_IER,7	;  Transmit frame sync interrupt enable.
RFS		cfbit		SSI_IER,6	;  Receive frame sync interrupt enable.
TLS		cfbit		SSI_IER,5	;  Transmit last time slot interrupt enable.
RLS		cfbit		SSI_IER,4	;  Receive last time slot interrupt enable.
RFF1		cfbit		SSI_IER,3	;  Receive FIFO full 1 interrupt enable.
RFF0		cfbit		SSI_IER,2	;  Receive FIFO full 0 interrupt enable.
TFE1		cfbit		SSI_IER,1	;  Transmit FIFO empty 1 interrupt enable.
TFE0		cfbit		SSI_IER,0	;  Transmit FIFO empty 0 interrupt enable.
SSI_TCR		equ		MBAR_SSI+$01C	; SSI Transmit Configuration Register (32b)
TXBIT0		cfbit		SSI_TCR,9	;  Transmit bit 0 (Alignment).
TFEN1		cfbit		SSI_TCR,8	;  Transmit FIFO enable 1.
TFEN0		cfbit		SSI_TCR,7	;  Transmit FIFO enable 0.
TFDIR		cfbit		SSI_TCR,6	;  Frame sync direction.
TXDIR		cfbit		SSI_TCR,5	;  Clock direction.
TSHFD		cfbit		SSI_TCR,4	;  Transmit shift direction.
TSCKP		cfbit		SSI_TCR,3	;  Transmit clock polarity.
TFSI		cfbit		SSI_TCR,2	;  Transmit frame sync invert.
TFSL		cfbit		SSI_TCR,1	;  Transmit frame sync length. Controls
TEFS		cfbit		SSI_TCR,0	;  Transmit early frame sync.
SSI_RCR		equ		MBAR_SSI+$020	; SSI Receive Configuration Register (32b)
RXEXT		cfbit		SSI_RCR,10	;  Receive data extension.
RXBIT0		cfbit		SSI_RCR,9	;  Receive bit 0 (Alignment).
RFEN1		cfbit		SSI_RCR,8	;  Receive FIFO enable 1.
RFEN0		cfbit		SSI_RCR,7	;  Receive FIFO enable 0.
RXDIR		cfbit		SSI_RCR,5	;  Gated clock enable.
RSHFD		cfbit		SSI_RCR,4	;  Receive shift direction.
RSCKP		cfbit		SSI_RCR,3	;  Receive clock polarity.
RFSI		cfbit		SSI_RCR,2	;  Receive frame sync invert.
RFSL		cfbit		SSI_RCR,1	;  Receive frame sync length.
REFS		cfbit		SSI_RCR,0	;  Receive early frame sync.
SSI_CCR		equ		MBAR_SSI+$024	; SSI Clock Control Register (32b)
DIV2		cfbit		SSI_CCR,18	;  Divide-by-2.
PSR		cfbit		SSI_CCR,17	;  Prescaler range.
WL		cffield		SSI_CCR,13,4	;  Word length.
DC		cffield		SSI_CCR,8,5	;  Frame rate divider control.
PM		cffield		SSI_CCR,0,8	;  Prescaler modulus select.
SSI_FCSR	equ		MBAR_SSI+$02C	; SSI FIFO Control/Status Register (32b)
RFCNT1		cffield		SSI_FCSR,28,4	;  Receive FIFO counter 1.
TFCNT1		cffield		SSI_FCSR,24,4	;  Transmit FIFO counter 1.
RFWM1		cffield		SSI_FCSR,20,4	;  Receive FIFO full watermark 1.
TFWM1		cffield		SSI_FCSR,16,4	;  Transmit FIFO empty watermark 1.
RFCNT0		cffield		SSI_FCSR,12,4	;  Receive FIFO counter 0.
TFCNT0		cffield		SSI_FCSR,8,4	;  Transmit FIFO counter 0.
RFWM0		cffield		SSI_FCSR,4,4	;  Receive FIFO full watermark 0.
TFWM0		cffield		SSI_FCSR,0,4	;  Transmit FIFO empty watermark 0.
SSI_ACR		equ		MBAR_SSI+$038	; SSI AC97 Control Register (32b)
FRDIV		cffield		SSI_ACR,5,6	;  Frame rate divider.
WR		cfbit		SSI_ACR,4	;  Write command.
RD		cfbit		SSI_ACR,3	;  Read command.
TIF		cfbit		SSI_ACR,2	;  Tag in FIFO.
FV		cfbit		SSI_ACR,1	;  Fixed/variable operation.
AC97EN		cfbit		SSI_ACR,0	;  AC97 mode enable.
SSI_ACADD	equ		MBAR_SSI+$03C	; SSI AC97 Command Address Register (32b)
ACADD		cffield		SSI_ACADD,0,19	;  AC97 command address.
SSI_ACDAT	equ		MBAR_SSI+$040	; SSI AC97 Command Data Register (32b)
ACDAT		cffield		SSI_ACDAT,0,20	;  AC97 command data.
SSI_ATAG	equ		MBAR_SSI+$044	; SSI AC97 Tag Register (32b)
ATAG		cffield		SSI_ATAG,0,16	;  AC97 tag.
SSI_TMASK	equ		MBAR_SSI+$048	; SSI Transmit Time Slot Mask Register (32b)
TMASK		cffield		SSI_TMASK,0,32	;  Transmit mask.
SSI_RMASK	equ		MBAR_SSI+$04C	; SSI Receive Time Slot Mask Register (32b)
RMASK		cffield		SSI_RMASK,0,32	;  Receive mask.

		restore				; re-enable listing

                endif                           ; __52xxssiinc
