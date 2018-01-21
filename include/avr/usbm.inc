		ifndef	__usbminc
__usbminc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGUSBM.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for ATmega USB Block               *
;*                                                                          *
;****************************************************************************

USBCON		sfr	0xd8		; USB General Control Register
USBE		avrbit	USBCON,7	; USB Macro Enable Bit
FRZLK		avrbit	USBCON,5	; Freeze USB Clock Bit

UDCON		sfr	0xe0		; USB Device Control Registers
DETACH		avrbit	UDCON,0		; Detach Bit
RMWKUP		avrbit	UDCON,1		; Remote Wake-up Bit

UDINT		sfr	0xe1		; USB Device Interrupt Register
SUSPI		avrbit	UDINT,0		; Suspend Interrupt Flag
SOFI		avrbit	UDINT,2		; Start Of Frame Interrupt Flag
EORSTI		avrbit	UDINT,3		; End Of Reset Interrupt Flag
WAKEUPI		avrbit	UDINT,4		; Wake-up CPU Interrupt Flag
EORSMI		avrbit	UDINT,5		; End Of Resume Interrupt Flag
UPRSMI		avrbit	UDINT,6		; Upstream Resume Interrupt Flag

UDIEN		sfr	0xe2		; USB Device Interrupt Enable Register
SUSPE		avrbit	UDIEN,0		; Suspend Interrupt Enable Bit
SOFE		avrbit	UDIEN,2		; Start Of Frame Interrupt Enable Bit
EORSTE		avrbit	UDIEN,3		; End Of Reset Interrupt Enable Bit
WAKEUPE		avrbit	UDIEN,4		; Wake-up CPU Interrupt Enable Bit
EORSME		avrbit	UDIEN,5		; End Of Resume Interrupt Enable Bit
UPRSME		avrbit	UDIEN,6		; Upstream Resume Interrupt Enable Bit

UDADDR		sfr	0xe3		; USB Device Address Register
ADDEN		avrbit	UDADDR,7	; Address Enable Bit

UDFNUML		sfr	0xe4		; USB Device Frame Number LSB
UDFNUMH		sfr	0xe5		; USB Device Frame Number MSB

UDMFN		sfr	0xe6		; USB Device Micro Frame Number
FNCERR		avrbit	UDMFN,4		; Frame Number CRC Error Flag

UENUM		sfr	0xe9		; USB Endpoint Number Register
EPNUM0		avrbit	UENUM,0
EPNUM1		avrbit	UENUM,1
EPNUM2		avrbit	UENUM,2

UERST		sfr	0xea		; USB Endpoint Reset Register
EPRST0		avrbit	UERST,0		; Endpoint 0 FIFO Reset
EPRST1		avrbit	UERST,1		; Endpoint 1 FIFO Reset
EPRST2		avrbit	UERST,2		; Endpoint 2 FIFO Reset
EPRST3		avrbit	UERST,3		; Endpoint 3 FIFO Reset
EPRST4		avrbit	UERST,4		; Endpoint 4 FIFO Reset

UECONX		sfr	0xeb		; USB Endpoint Control Register
EPEN		avrbit	UECONX,0	; Endpoint Enable Bit
RSTDT		avrbit	UECONX,3	; Reset Data Toggle Bit
STALLRQC	avrbit	UECONX,4	; STALL Request Clear Handshake Bit
STALLRQ		avrbit	UECONX,5	; STALL Request Handshake Bit

UECFG0X		sfr	0xec		; USB Endpoint Configuration 0 Register
EPDIR		avrbit	UECFG0X,0	; Endpoint Direction Bit
EPTYPE0		avrbit	UECFG0X,6	; Endpoint Type
EPTYPE1		avrbit	UECFG0X,7

UECFG1X		sfr	0xed		; USB Endpoint Configuration 1 Register
ALLOC		avrbit	UECFG1X,1	; Endpoint Allocation Bit
EPBK0		avrbit	UECFG1X,2	; Endpoint Bank Bits
EPBK1		avrbit	UECFG1X,3
EPSIZE0		avrbit	UECFG1X,4	; Endpoint Size Bits
EPSIZE1		avrbit	UECFG1X,5
EPSIZE2		avrbit	UECFG1X,6

UESTA0X		sfr	0xee		; USB Endpoint Status 0 Register
NBUSYBK0	avrbit	UESTA0X,0	; Busy Bank Flag
NBUSYBK1	avrbit	UESTA0X,1
DTSEQ0		avrbit	UESTA0X,2	; Data Toggle Sequencing Flag
DTSEQ1		avrbit	UESTA0X,3
UNDERFI		avrbit	UESTA0X,5	; Underflow Error Interrupt Flag
OVERFI		avrbit	UESTA0X,6	; Overflow Error Interrupt Flag
CFGOK		avrbit	UESTA0X,7	; Configuration Status Flag

UESTA1X		sfr	0xef		; USB Endpoint Status 1 Register
CURRBK0		avrbit	UESTA1X,0	; Current Bank
CURRBK1		avrbit	UESTA1X,1
CTRLDIR		avrbit	UESTA1X,2	; Control Direction

UEINTX		sfr	0xe8		; USB Endpoint Interrupt Register
FIFOCON		avrbit	UEINTX,7	; FIFO Control Bit
NAKINI		avrbit	UEINTX,6	; NAK IN Received Interrupt Flag
RWAL		avrbit	UEINTX,5	; Read/Write Allowed Flag
NAKOUTI		avrbit	UEINTX,4	; NAK OUT Received Interrupt Flag
RXSTPI		avrbit	UEINTX,3	; Received SETUP Interrupt Flag
RXOUTI		avrbit	UEINTX,2	; Received OUT Data Interrupt Flag
STALLEDI	avrbit	UEINTX,1	; STALLEDI Interrupt Flag
TXINI		avrbit	UEINTX,0	; Transmitter Ready Interrupt Flag

UEIENX		sfr	0xf0		; USB Endpoint Interrupt Enable Register
FLERRE		avrbit	UEIENX,7	; Flow Error Interrupt Enable Flag
NAKINE		avrbit	UEIENX,6	; NAK IN Interrupt Enable Bit
NAKOUTE		avrbit	UEIENX,4	; NAK OUT Interrupt Enable Bit
RXSTPE		avrbit	UEIENX,3	; Received SETUP Interrupt Enable Flag
RXOUTE		avrbit	UEIENX,2	; Received OUT Data Interrupt Enable Flag
STALLEDE	avrbit	UEIENX,1	; Stalled Interrupt Enable Flag
TXINE		avrbit	UEIENX,0	; Transmitter Ready Interrupt Enable Flag

UEDATX		sfr	0xf1		; USB Data Endpoint Register

UEBCLX		sfr	0xf2		; USB Endpoint Byte Count Register

UEINT		sfr	0xf4		; USB Endpoint Number Interrupt Register
EPINT0		avrbit	UEINT,0		; Endpoint Interrupts Bits
EPINT1		avrbit	UEINT,1
EPINT2		avrbit	UEINT,2
EPINT3		avrbit	UEINT,3
EPINT4		avrbit	UEINT,4

		restore			; re-enable listing

		endif			; __usbminc
