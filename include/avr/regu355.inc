		ifndef	__regu355inc
__regu355inc	equ	1
                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGU355.INC                                             *
;*                                                                          *
;*   Contains Bit & Register Definitions for AT43USB355                     *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Limits

		; no EEPROM!
RAMSTART	equ	0x60,data
RAMEND		equ	0x45f,data
FLASHEND	label	0x5fff

;----------------------------------------------------------------------------
; Chip Configuration

MCUCR		port	0x35		; MCU General Control Register
SM		avrbit	MCUCR,4		; Choose Idle/Power Down Mode
SE		avrbit	MCUCR,5		; Enable Sleep Mode

;----------------------------------------------------------------------------
; GPIO

PINA		port	0x19		; Port A @ 0x19 (IO) ff.
PINB		port	0x16		; Port B @ 0x16 (IO) ff.
PIND		port	0x10		; Port D @ 0x10 (IO) ff.
PINF		port	0x04		; Port F @ 0x04 (IO) ff.

;----------------------------------------------------------------------------
; Interrupt Vectors

		enumconf 1,code
		enum	 INT0_vect=1		; External Interrupt Request 0
		nextenum INT1_vect		; External Interrupt Request 1
		nextenum TIMER1_CAPT_vect	; Timer/Counter 1 Capture Event
		nextenum TIMER1_COMPA_vect	; Timer/Counter 1 Compare Match A
		nextenum TIMER1_COMPB_vect	; Timer/Counter 1 Compare Match B
		nextenum TIMER1_OVF_vect	; Timer/Counter 1 Overflow
		nextenum TIMER0_OVF_vect	; Timer/Counter 0 Overflow
		nextenum SPI_STC_vect		; SPI Serial Transfer Complete
		nextenum Reserved1_vect
		nextenum Reserved2_vect
		nextenum ADC_vect		; ADC Conversion Complete
		nextenum ANA_COMP_vect		; Analog Comparator
		nextenum USB_HW_vect		; USB Hardware

;----------------------------------------------------------------------------
; External Interrupts

ISC00		avrbit	MCUCR,0		; External Interrupt 0 Sense Control
ISC01		avrbit	MCUCR,1
ISC10		avrbit	MCUCR,2		; External Interrupt 1 Sense Control
ISC11		avrbit	MCUCR,3

GIMSK		port	0x3b		; General Interrupt Mask Register
INT0		avrbit	GIMSK,6		; Enable External Interrupt 0
INT1		avrbit	GIMSK,7		; Enable External Interrupt 1

GIFR		port	0x3a		; External Interrupt-Flags
INTF0		avrbit	GIFR,6		; External Interrupt 0 Occured
INTF1		avrbit	GIFR,7		; External Interrupt 1 Occured

;----------------------------------------------------------------------------
; Timers

TCCR0		port	0x33		; Timer/Counter 0 Control Register
CS00		avrbit	TCCR0,0		; Clock Select
CS01		avrbit	TCCR0,1
CS02		avrbit	TCCR0,2
TCNT0		port	0x32		; Timer/Counter 0 Value

TCCR1A		port	0x2f		; Timer/Counter 1 Control Register A
PWM10		avrbit	TCCR1A,0	; Mode of Pulse Width Modulator
PWM11		avrbit	TCCR1A,1
COM1B0		avrbit	TCCR1A,4	; Timer/Counter 1 Compare Mode B
COM1B1		avrbit	TCCR1A,5
COM1A0		avrbit	TCCR1A,6	; Timer/Counter 1 Compare Mode A
COM1A1		avrbit	TCCR1A,7
TCCR1B		port	0x2e		; Timer/Counter 1 Control Register B
CS10		avrbit	TCCR1B,0	; Timer/Counter Clock Select
CS11		avrbit	TCCR1B,1
CS12		avrbit	TCCR1B,2
CTC1		avrbit	TCCR1B,3	; Clear after Equality?
ICES1		avrbit	TCCR1B,6	; Capture Slope Selection
ICNC1		avrbit	TCCR1B,7	; Capture Noise Filter
TCNT1L		port	0x2c		; Timer/Counter 1 Value LSB
TCNT1H		port	0x2d		; Timer/Counter 1 Value MSB
OCR1AL		port	0x2a		; Timer/Counter 1 Output Compare Value A LSB
OCR1AH		port	0x2b		; Timer/Counter 1 Output Compare Value A MSB
OCR1BL		port	0x28		; Timer/Counter 1 Output Compare Value B LSB
OCR1BH		port	0x29		; Timer/Counter 1 Output Compare Value B MSB
ICR1L		port	0x24		; Timer/Counter 1 Input Capture Value LSB
ICR1H		port	0x25		; Timer/Counter 1 Input Capture Value MSB

TIMSK		port	0x39		; Timer Interrupt Mask Register
TOIE0		avrbit	TIMSK,1		; Timer/Counter 0 Overflow Interrupt Enable
TICIE1		avrbit	TIMSK,3		; Timer/Counter 1 Input Capture Interrupt Enable
OCIE1B		avrbit	TIMSK,5		; Timer/Counter 1 Output Compare Interrupt Enable B
OCIE1A		avrbit	TIMSK,6		; Timer/Counter 1 Output Compare Interrupt Enable A
TOIE1		avrbit	TIMSK,7		; Timer/Counter 1 Overflow Interrupt Enable

TIFR		port	0x38		; Timer Interrupt Flag Register

;----------------------------------------------------------------------------
; Watchdog Timer

		include	"wdm21.inc"
WDTOE		avrbit	WDTCR,4		; Turn-Off Enable

;----------------------------------------------------------------------------
; SPI

		include	"spi90.inc"

;----------------------------------------------------------------------------
; A/D Converter

ADMUX		port	0x08		; Multiplexer Selection
MUX3		avrbit	ADMUX,3		; Multiplexer
MUX2		avrbit	ADMUX,2
MUX1		avrbit	ADMUX,1
MUX0		avrbit	ADMUX,0

ADCSR		port	0x07		; Control/Status Register
ADEN		avrbit	ADCSR,7		; Enable ADC
ADSC		avrbit	ADCSR,6		; Start Conversion
ADFR		avrbit	ADCSR,5		; Free Running Select
ADIF		avrbit	ADCSR,4		; Interrupt Flag
ADIE		avrbit	ADCSR,3		; Interrupt Enable
ADPS2		avrbit	ADCSR,2		; Prescaler Select
ADPS1		avrbit	ADCSR,1
ADPS0		avrbit	ADCSR,0

ADCH		port	0x03		; Data Register
ADCL		port	0x02

;----------------------------------------------------------------------------
; USB

FCAR3		sfr	0x1fa2		; Function End-point 3 Control and Acknowledge Register
FCAR2		sfr	0x1fa3		; Function End-point 2 Control and Acknowledge Register
FCAR1		sfr	0x1fa4		; Function End-point 1 Control and Acknowledge Register
FCAR0		sfr	0x1fa5		; Function End-point 0 Control and Acknowledge Register
HCAR0		sfr	0x1fa7		; Hub End-point 0 Control and Acknowledge Register
PSTATE2		sfr	0x1fa9		; Hub Port 2 Bus State Register
PSTATE3		sfr	0x1faa		; Hub Port 3 Bus State Register
HPSCR1		sfr	0x1fb0		; Hub Port 1 Status Change Register
HPSCR2		sfr	0x1fb1		; Hub Port 2 Status Change Register
HPSCR3		sfr	0x1fb2		; Hub Port 3 Status Change Register
HPSTAT1		sfr	0x1fb8		; Hub Port 1 Status Register
HPSTAT2		sfr	0x1fb9		; Hub Port 2 Status Register
HPSTAT3		sfr	0x1fba		; Hub Port 3 Status Register
HPCON		sfr	0x1fc5		; Hub Port Control Register
HSTR		sfr	0x1fc7		; Hub Status Register
FBYTE_CNT3	sfr	0x1fca		; Function End-point 3 Byte Count Register
FBYTE_CNT2	sfr	0x1fcb		; Function End-point 2 Byte Count Register
FBYTE_CNT1	sfr	0x1fcc		; Function End-point 1 Byte Count Register
FBYTE_CNT0	sfr	0x1fcd		; Function End-point 0 Byte Count Register
HBYTE_CNT0	sfr	0x1fcf		; Hub End-point 0 Byte Count Register
FDR3		sfr	0x1fd2		; Function End-point 3 FIFO Data Register
FDR2		sfr	0x1fd3		; Function End-point 2 FIFO Data Register
FDR1		sfr	0x1fd4		; Function End-point 1 FIFO Data Register
FDR0		sfr	0x1fd5		; Function End-point 0 FIFO Data Register
HDR0		sfr	0x1fd7		; Hub End-point 0 FIFO Data Register
FCSR3		sfr	0x1fda		; Function Controller End-point 3 Service Routine Register
FCSR2		sfr	0x1fdb		; Function Controller End-point 2 Service Routine Register
FCSR1		sfr	0x1fdc		; Function Controller End-point 1 Service Routine Register
FCSR0		sfr	0x1fdd		; Function Controller End-point 0 Service Routine Register
HCSR0		sfr	0x1fdf		; Hub Controller End-point 0 Service Routine Register
FEND_P3_CNTR	sfr	0x1fe2		; Function End-point 3 Control Register
FEND_P2_CNTR	sfr	0x1fe3		; Function End-point 2 Control Register
FEND_P1_CNTR	sfr	0x1fe4		; Function End-point 1 Control Register
FEND_P0_CNTR	sfr	0x1fe5		; Function End-point 0 Control Register
HEND_P0_CNTR	sfr	0x1fe7		; Hub End-point 0 Control Register
FADDR		sfr	0x1fee		; Function Address Register
HADDR		sfr	0x1fef		; Hub Address Register
UOVCER		sfr	0x1ff2		; Overcurrent Detect Register
UIER		sfr	0x1ff3		; USB Interrupt Enable Register
SOF_IE		avrbit	UIER,7		; Enable Start of Frame Interrupt
EOF2_IE		avrbit	UIER,6		; Enable EOF2 Interrupt
FEP3_IE		avrbit	UIER,4		; Enable Function End-point 3 Interrupt
HEP0_IE		avrbit	UIER,3		; Enable Hub End-point 0 Interrupt
FEP2_IE		avrbit	UIER,2		; Enable Function End-point 2 Interrupt
FEP1_IE		avrbit	UIER,1		; Enable Function End-point 1 Interrupt
FEP0_IE		avrbit	UIER,0		; Enable Function End-point 0 Interrupt
UIAR		sfr	0x1ff5		; USB Interrupt Acknowledge Register
SOF_INTACK	avrbit	UIAR,7		; Start of Frame Interrupt Acknowledge
EOF2_INTACK	avrbit	UIAR,5		; EOF2 Interrupt Acknowledge
FEP3_INTACK	avrbit	UIAR,4		; Function End-point 3 Interrupt Acknowledge
HEP0_INTACK	avrbit	UIAR,3		; Hub End-point 0 Interrupt Acknowledge
FEP2_INTACK	avrbit	UIAR,2		; Function End-point 2 Interrupt Acknowledge
FEP1_INTACK	avrbit	UIAR,1		; Function End-point 1 Interrupt Acknowledge
FEP0_INTACK	avrbit	UIAR,0		; Function End-point 0 Interrupt Acknowledge
UIMSKR		sfr	0x1ff6		; USB Interrupt Mask Register
SOF_IMSK	avrbit	UIMSKR,7	; Start of Frame Interrupt Mask
EOF2_IMSK	avrbit	UIMSKR,5	; EOF2 Interrupt Mask
FEP3_IMSK	avrbit	UIMSKR,4	; Function End-point 3 Interrupt Mask
HEP0_IMSK	avrbit	UIMSKR,3	; Hub End-point 0 Interrupt Mask
FEP2_IMSK	avrbit	UIMSKR,2	; Function End-point 2 Interrupt Mask
FEP1_IMSK	avrbit	UIMSKR,1	; Function End-point 1 Interrupt Mask
FEP0_IMSK	avrbit	UIMSKR,0	; Function End-point 0 Interrupt Mask
UISR		sfr	0x1ff7		; USB Interrupt Status Register
SOF_INT		avrbit	UISR,7		; Start of Frame Interrupt
EOF2_INT	avrbit	UISR,5		; EOF2 Interrupt
FE3_INT		avrbit	UISR,4		; Function End-point 3 Interrupt
HEP0_INT	avrbit	UISR,3		; Hub End-point 0 Interrupt
FE2_INT		avrbit	UISR,2		; Function End-point 2 Interrupt
FE1_INT		avrbit	UISR,1		; Function End-point 1 Interrupt
FE0_INT		avrbit	UISR,0		; Function End-point 0 Interrupt
SPRSMSK		sfr	0x1ff8		; Suspend/Resume Interrupt Mask Register
BUS_INT_MSK	avrbit	SPRSMSK,3	; USB Reset Interrupt Mask
FRWUP_INT_MSK	avrbit	SPRSMSK,2	; Function Remote Wakeup Interrupt Mask
RSM_INT_MSK	avrbit	SPRSMSK,1	; Resume Interrupt Mask
GLB_SUSP_INT_MSK avrbit	SPRSMSK,0	; Global Suspend Interrupt Enable
SPRSIE		sfr	0x1ff9		; Suspend/Resume Interrupt Enable Register
BUS_IE		avrbit	SPRSIE,3	; USB Reset Interrupt Enable
FRWUP_IE	avrbit	SPRSIE,2	; Function Remote Wakeup Interrupt Enable
RSM_IE		avrbit	SPRSIE,1	; Resume Interrupt Enable
GLB_SUSP_IE	avrbit	SPRSIE,0	; Global Suspend Interrupt Enable
SPRSR		sfr	0x1ffa		; Suspend/Resume Register
BUS_INT		avrbit	SPRSR,3		; USB Bus Interrupt
FRWUP_INT	avrbit	SPRSR,2		; Function Remote Wakeup
RSM_INT		avrbit	SPRSR,1		; Resume
GLB_SUSP_INT	avrbit	SPRSR,0		; Global Suspend
GLB_STATE	sfr	0x1ffb		; Global State Register
FRM_NUM_L	sfr	0x1ffc		; Frame Number Low Register
FRM_NUM_H	sfr	0x1ffd		; Frame Number High Register

		restore			; re-enable listing

		endif			; __regu355inc
