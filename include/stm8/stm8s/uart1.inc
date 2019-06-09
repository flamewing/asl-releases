		ifndef	__stm8suart1inc	; avoid multiple inclusion
__stm8suart1inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File UART1.INC                                               *
;*                                                                          *
;*   contains SFR and Bit Definitions for STM8S U(S)ART1                    *
;*                                                                          *
;****************************************************************************

__defusart1	macro	NAME,Base
__NS		set	"\{NAME}_"
{__NS}SR	label	Base+0		; status register
{__NS}TXE	bit	{__NS}SR,7	;  Transmit data register empty
{__NS}TC	bit	{__NS}SR,6	;  Transmission complete
{__NS}RXNE	bit	{__NS}SR,5	;  Read data register not empty
{__NS}IDLE	bit	{__NS}SR,4	;  IDLE line detected
{__NS}OR_LHE	bit	{__NS}SR,3	;  Overrun error/LIN Header Error
{__NS}NF	bit	{__NS}SR,2	;  Noise flag
{__NS}FE	bit	{__NS}SR,1	;  Framing error
{__NS}PE	bit	{__NS}SR,0	;  Parity error
{__NS}DR	label	Base+1		; data register
{__NS}BRR1	label	Base+2		; baud rate register 1
{__NS}BRR2	label	Base+3		; baud rate register 2
{__NS}CR1	label	Base+4		; control register 1
{__NS}R8	bit	{__NS}CR1,7	;  Receive Data bit 8
{__NS}T8	bit	{__NS}CR1,6	;  Transmit data bit 8
{__NS}USARTD	bit	{__NS}CR1,5	;  USART Disable
{__NS}M		bit	{__NS}CR1,4	;  word length
{__NS}WAKE	bit	{__NS}CR1,3	;  Wakeup method
{__NS}PCEN	bit	{__NS}CR1,2	;  Parity control enable
{__NS}PS	bit	{__NS}CR1,1	;  Parity selection
{__NS}PIEN	bit	{__NS}CR1,0	;  Parity interrupt enable
{__NS}CR2	label	Base+5		; control register 2
{__NS}TIEN	bit	{__NS}CR2,7	;  Transmitter interrupt enable
{__NS}TCIEN	bit	{__NS}CR2,6	;  Transmission complete interrupt enable
{__NS}RIEN	bit	{__NS}CR2,5	;  Receiver interrupt enable
{__NS}ILIEN	bit	{__NS}CR2,4	;  IDLE Line interrupt enable
{__NS}TEN	bit	{__NS}CR2,3	;  Transmitter enable
{__NS}REN	bit	{__NS}CR2,2	;  Receiver enable
{__NS}RWU	bit	{__NS}CR2,1	;  Receiver wakeup
{__NS}SBK	bit	{__NS}CR2,0	;  Send break
{__NS}CR3	label	Base+6		; control register 3
{__NS}LINEN	bit	{__NS}CR3,6	;  LIN mode enable
{__NS}STOP	bfield	{__NS}CR3,4,2	;  STOP bits
{__NS}CLKEN	bit	{__NS}CR3,3	;  Clock enable
{__NS}CPOL	bit	{__NS}CR3,2	;  Clock polarity
{__NS}CPHA	bit	{__NS}CR3,1	;  Clock phase
{__NS}LBCL	bit	{__NS}CR3,0	;  Last bit clock pulse
{__NS}CR4	label	Base+7		; control register 4
{__NS}LBDIEN	bit	{__NS}CR4,6	;  LIN Break Detection Interrupt Enable
{__NS}LBDL	bit	{__NS}CR4,5	;  LIN Break Detection Length
{__NS}LBDF	bit	{__NS}CR4,4	;  LIN Break Detection Flag
{__NS}ADD	bfield	{__NS}CR4,0,4	;  Address of the UART node
{__NS}CR5	label	Base+8		; control register 5
{__NS}SCEN	bit	{__NS}CR5,5	;  Smartcard mode enable
{__NS}NACK	bit	{__NS}CR5,4	;  Smartcard NACK enable
{__NS}HDSEL	bit	{__NS}CR5,3	;  Half-Duplex Selection
{__NS}IRLP	bit	{__NS}CR5,2	;  IrDA Low Power
{__NS}IREN	bit	{__NS}CR5,1	;  IrDA mode Enable
{__NS}GTR	label	Base+9		; guard time register
{__NS}PSCR	label	Base+10		; prescaler register
		endm

		restore
		endif			; __stm8susart1inc
