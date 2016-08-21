		save
		listing	off

		ifndef	reg78310inc	; avoid multiple inclusion
reg78310inc	equ	1

		if 	(MOMCPUNAME<>"78310")
		 fatal	 "wrong processor type: include only for 78310"
		endif

		if 	MOMPASS=1
		 message "uPD78310/78312 SFR Definitions"
		 message "(C) 2016 Alfred Arnold"
		endif

P0		equ	0ff00h		; Port 0
P1		equ	0ff01h		; Port 1
P2		equ	0ff02h		; Port 2
P3		equ	0ff03h		; Port 3
P4		equ	0ff04h		; Port 4
P5		equ	0ff05h		; Port 5

CR00L		equ	0ff08h		; up/down counter unit 0, capture register 0, low byte
CR00H		equ	0ff09h		; up/down counter unit 0, capture register 0, high byte
CR01L		equ	0ff0ah		; up/down counter unit 0, capture register 1, low byte
CR01H		equ	0ff0bh		; up/down counter unit 0, capture register 1, high byte
CR10L		equ	0ff0ch		; up/down counter unit 1, capture register 0, low byte
CR10H		equ	0ff0dh		; up/down counter unit 1, capture register 0, high byte
CR11L		equ	0ff0eh		; up/down counter unit 1, capture register 1, low byte
CR11H		equ	0ff0fh		; up/down counter unit 1, capture register 1, high byte

CPT0L		equ	0ff10h		; capture/PWM unit, capture latch 0, low byte
CPT0H		equ	0ff11h		; capture/PWM unit, capture latch 0, high byte
CPT1L		equ	0ff12h		; capture/PWM unit, capture latch 1, low byte
CPT1H		equ	0ff13h		; capture/PWM unit, capture latch 1, high byte
PWM0L		equ	0ff14h		; capture/PWM unit 0, PWM register, low byte
PWM0H		equ	0ff15h		; capture/PWM unit 0, PWM register, high byte
PWM1L		equ	0ff16h		; capture/PWM unit 1, PWM register, low byte
PWM1H		equ	0ff17h		; capture/PWM unit 1, PWM register, high byte
UDC0L		equ	0ff1ch		; up/down counter unit 0, up/down counter reg, low byte
UDC0H		equ	0ff1dh		; up/down counter unit 0, up/down counter reg, high byte
UDC1L		equ	0ff1eh		; up/down counter unit 1, up/down counter reg, low byte
UDC1H		equ	0ff1fh		; up/down counter unit 1, up/down counter reg, high byte

PM0		equ	0ff20h		; port 0, port mode register
PM1		equ	0ff21h		; port 1, port mode register
PM2		equ	0ff22h		; port 2, port mode register
PM3		equ	0ff23h		; port 3, port mode register
PM5		equ	0ff25h		; port 5, port mode register
PMC2		equ	0ff32h		; port 2, port mode control register
PMC3		equ	0ff33h		; port 3, port mode control register

RTPC		equ	0ff38h		; real time output port control register
POL		equ	0ff3ah		; real time output port lower nibble (port 0, buffer reg.)
POH		equ	0ff3bh		; real time output port higher nibble (port 0, buffer reg.)

MM		equ	0ff40h		; memory mapping register (memory expansion)
RFM		equ	0ff41h		; refresh mode register
WDM		equ	0ff42h		; watch dof timer, watch dog mode register
STBC		equ	0ff44h		; stand by modes, stand by control register
TBM		equ	0ff46h		; time base counter, time base mode register
INTM		equ	0ff48h		; interrupt mode register (external interrupts)
ISPR		equ	0ff4ah		; int service priority register

CCW		equ	0ff4eh		; CPU control word
SCM		equ	0ff50h		; serial interface, serial communication mode register
SCC		equ	0ff52h		; serial interface, serial communication control register
BRG		equ	0ff53h		; serial interface, baud rate generator
RxB		equ	0ff56h		; serial interface, receive byte register
TxB		equ	0ff57h		; serial interface, transmit byte register
FRCC		equ	0ff60h		; capture unit, free running counter control register

CPTM		equ	0ff64h		; capture/PWM unit, capture mode register
PWMM		equ	0ff66h		; capture/PWM unit, PWM mode register
ADM		equ	0ff68h		; A/D converter, A/D converter mode register
ADCR		equ	0ff6ah		; A/D converter, conversion result reg
CUIM		equ	0ff70h		; up/down counter, counter unit input mode register
UDCC0		equ	0ff72h		; up/down counter 0, control register
CRC		equ	0ff74h		; up/down counter, capture/compare control reg
UDCC1		equ	0ff7ah		; up/down counter 1, control register
TMC0		equ	0ff80h		; timer unit 0, timer control register
TMC1		equ	0ff82h		; timer unit 1, timer control register
TM0L		equ	0ff88h		; timer unit 0, timer register low byte
TM0H		equ	0ff89h		; timer unit 0, timer register high byte
MD0L		equ	0ff8ah		; timer unit 0, modulo register low byte
MD0H		equ	0ff8bh		; timer unit 0, modulo register high byte
TM1L		equ	0ff8ch		; timer unit 1, timer register low byte
TM1H		equ	0ff8dh		; timer unit 1, timer register high byte
MD1L		equ	0ff8eh		; timer unit 1, modulo register low byte
MD1H		equ	0ff8fh		; timer unit 1, modulo register high byte

CRIC00		equ	0ffc0h		; interrupt control register up/down counter 0 CR00
CRMS00		equ	0ffc1h		; macro service control register up/down counter 0 CR00
CRIC01		equ     0ffc2h		; interrupt control register up/down counter 0 CR01
CRIC10		equ	0ffc4h		; interrupt control register up/down counter 1 CR10
CRMS10		equ	0ffc5h		; macro service control register up/down counter 1 CR10
CRIC11		equ	0ffc6h		; interrupt control register up/down counter 1 CR11
EIC0		equ	0ffc8h		; interrupt control register external interrupt 0
EMS0		equ	0ffc9h		; macro service control register external interrupt 0
EIC1		equ	0ffcah		; interrupt control register external interrupt 1
EMS1		equ	0ffcbh		; macro service control register external interrupt 1
EIC2		equ	0ffcch		; interrupt control register external interrupt 2
EMS2		equ	0ffcdh		; macro service control register external interrupt 2
TMIC0		equ	0ffceh		; interrupt control register timer interrupt 0
TMMS0		equ	0ffcfh		; macro service control register timer interrupt 0
TMIC1		equ	0ffd0h		; interrupt control register timer interrupt 1
TMMS1		equ	0ffd1h		; macro service control register timer interrupt 1
TMIC2		equ	0ffd2h		; interrupt control register timer interrupt 2
TMMS2		equ	0ffd3h		; macro service control register timer interrupt 2
SEIC		equ	0ffdah		; interrupt control register serial error interrupt
SRIC		equ	0ffdch		; interrupt control register serial receive interrupt
SRMS		equ	0ffddh		; macro service control register serial receive interrupt
STIC		equ	0ffdeh		; interrupt control register serial transmit interrupt
STMS		equ	0ffdfh		; macro service control register serial transmit interrupt
ADIC		equ	0ffe0h		; interrupt control register A/D converter interrupt
ADMS		equ	0ffe1h		; macro service control register A/D converter interrupt
TBIC		equ	0ffe2h		; interrupt control register time base counter

SPL		equ	0fffch		; stack pointer, low byte
SPH		equ	0fffdh		; stack pointer, high byte

; these two are built-in symbols and need not be defined
;PSWL		equ	0fffeh		; program status word, low byte
;PSWH		equ	0ffffh		; program status word, high byte

; vector addresses

VEC_RESET	equ	0000h		; RESET input
VEC_NMI		equ	0002h		; NMI input pin
VEC_EXIF0	equ	0004h		; INTE0 input pin
VEC_EXIF1	equ	0006h		; INTE1 input pin
VEC_EXIF2	equ	0008h		; INTE2 input pin
VEC_WDT		equ	000ah		; Watchdog timer
VEC_TBF		equ	000ch		; Time base counter
VEC_TMF0	equ	000eh		; Timer unit
VEC_TMF1	equ	0010h		; Timer unit
VEC_TMF2	equ	0012h		; Timer unit
VEC_CRF00	equ	001ah		; Count unit
VEC_CRF01	equ	001ch		; Count unit
VEC_CRF10	equ	001eh		; Count unit
VEC_CRF11	equ	0020h		; Count unit
VEC_SEF		equ	0022h		; Serial receive error
VEC_SRF		equ	0024h		; Serial receive complete
VEC_STF		equ	0026h		; Serial transmit complete
VEC_ADF		equ	0028h		; A/D converter
VEC_BRK		equ	003eh		; Break instruction

		endif			; reg78310inc
		restore			; restore listing state
