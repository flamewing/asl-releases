		ifndef	__regmc1inc	; avoid multiple inclusion
__regmc1inc	equ	1

		save
		listing	off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGMC1.INC                                              *
;*                                                                          *
;*   contains SFR and Bit Definitions for ST7MC1xx                          *
;*                                                                          *
;*   Source: ST7MC Data Sheet, Rev. 13, April 2009                          *
;*                                                                          *
;****************************************************************************

;----------------------------------------------------------------------------
; Memory Addresses

RAMSTART	label	$0080		; Start Address Internal RAM
		switch  SUBSTR(MOMCPUNAME,STRLEN(MOMCPUNAME)-1,1)
		case	"2"
RAMEND		label	$01ff		; End     "        "      "
		case	"4"
RAMEND		label	$37f
		case	"6"
RAMEND		label	$47f
		case	"7","9"
RAMEND		label	$67f
		elsecase
		error	"Huh?"
		endcase

;----------------------------------------------------------------------------
; Interrupt Vectors

PWM_ART_vect	label	$ffe0		; PWM ART Interrupt
ADC_vect	label	$ffe2		; ADC Interrupt, shared with...
AVD_vect	label	$ffe2		; Voltage Detector Interrupt
LINSCI_vect	label	$ffe4		; LINSCI Interrupts
TA_vect		label	$ffe8		; Timer A Peripheral Interrupts
MTC_C_D_vect	label	$ffec		; Event C or Event D
MTC_R_Z_vect	label	$ffee		; Event R or Event Z
MTC_U_vect	label	$fff0		; Event U or Current Loop or Sampling Out
EI2_vect	label	$fff2		; External Interrupt Vector EI2
EI1_vect	label	$fff4		; External Interrupt Vector EI1
EI0_vect	label	$fff6		; External Interrupt Vector EI0
MCC_RTC_vect	label	$fff8		; Main Clock Controller Time Base Interrupt, shared with...
CSS_vect	label	$fff8		; Safe Oscillator Activation Interrupt
MCES_vect	label	$fffa		; Motor Control Emergency Stop or Speed error Interrupt
TRAP_vect	label	$fffc		; TRAP (software) Interrupt Vector
RESET_vect	label	$fffe		; RESET Vector

;----------------------------------------------------------------------------
; GPIO

		include	"gpio.inc"
		__defgpio "PA",$0000
		__defgpio "PB",$0003
		__defgpio "PC",$0006
		__defgpio "PD",$0009
		__defgpio "PE",$000c
		__defgpio "PF",$000f
		__defgpio "PG",$0012
		__defgpio "PH",$0015

;----------------------------------------------------------------------------
; LINSCI

		include	"linsci.inc"
		__deflinsci $0018

;----------------------------------------------------------------------------
; ITC

ISPR0		label	$0024		; Interrupt Software Priority Register 0
I0_0		bit	ISPR0,0		; Vector $fffa
I1_0		bit	ISPR0,1
I0_1		bit	ISPR0,2		; Vector $fff8
I1_1		bit	ISPR0,3
I0_2		bit	ISPR0,4		; Vector $fff6
I1_2		bit	ISPR0,5
I0_3		bit	ISPR0,6		; Vector $fff4
I1_3		bit	ISPR0,7
ISPR1		label	$0025		; Interrupt Software Priority Register 1
I0_4		bit	ISPR1,0		; Vector $fff2
I1_4		bit	ISPR1,1
I0_5		bit	ISPR1,2		; Vector $fff0
I1_5		bit	ISPR1,3
I0_6		bit	ISPR1,4		; Vector $ffee
I1_6		bit	ISPR1,5
I0_7		bit	ISPR1,6		; Vector $ffec
I1_7		bit	ISPR1,7
ISPR2		label	$0026		; Interrupt Software Priority Register 2
I0_8		bit	ISPR2,0		; Vector $ffea
I1_8		bit	ISPR2,1
I0_9		bit	ISPR2,2		; Vector $ffe8
I1_9		bit	ISPR2,3
I0_10		bit	ISPR2,4		; Vector $ffe6
I1_10		bit	ISPR2,5
I0_11		bit	ISPR2,6		; Vector $ffe4
I1_11		bit	ISPR2,7
ISPR3		label	$0027		; Interrupt Software Priority Register 3
I0_12		bit	ISPR3,0		; Vector $ffe2
I1_12		bit	ISPR3,1
I0_13		bit	ISPR3,2		; Vector $ffe0
I1_13		bit	ISPR3,3
EICR		label	$0028		; External Interrupt Control Register
IS1		bfield	EICR,6,2	;  ei2 Sensitivity
IPB		bit	EICR,5		;  Interrupt Polarity for Port C
IS2		bfield	EICR,3,2	;  ei1 Sensitivity
IS3		bfield	EICR,1,2	;  ei0 Sensitivity
IPA		bit	EICR,0		;  Interrupt Polarity for Port D

;----------------------------------------------------------------------------
; Flash

FCSR		label	$0029		; Flash Control/Status Register

;----------------------------------------------------------------------------
; Window Watchdog

WDGCR		label	$002a		; Window Watchdog Control Register
WDGA		bit	WDGCR,7		; Activation Bit
WDGWR		label	$002b		; Window Watchdog Window Register

;----------------------------------------------------------------------------
; MCC

MCCSR		label	$002c		; Main Clock Control / Status Register
MCO		bit	MCCSR,7		;  Main Clock Out Enable Bit
CP		bfield	MCCSR,5,2	;  CPU Clock Prescaler
SMS		bit	MCCSR,4		;  Slow Mode Selection Bit
TB		bfield	MCCSR,2,2	;  Time Base Control
OIE		bit	MCCSR,1		;  Oscillator Interrupt Enable
OIF		bit	MCCSR,0		;  Oscillator Interrupt Flag
MCCBCR		label	$002d		; Main Clock Controller: Beep Control Register
ADSTS		bit	MCCBCR,3	;  A/D Converter Sample Time Stretch
ADCIE		bit	MCCBCR,2	;  A/D Converter Interrupt Enable
BC		bfield	MCCBCR,0,2	;  Beep Control

;----------------------------------------------------------------------------
; Analog/Digital Converter

		include	"adc10.inc"
		__defadc10 $002e
SLOW		bit	ADCDRL,3	;  Slow Mode

;----------------------------------------------------------------------------
; Timer

		include	"timer.inc"
		__deftimer "TA",$0030
TATIMD		bit	TASR,2		;  Timer A Disable

;----------------------------------------------------------------------------
; System Integrity

SICSR		label	$0040		; System Integrity Control/Status Register
PAGE		bit	SICSR,7		;  SICSR Register Page Selection
AVDIE		bit	SICSR,6		;  (Page 0) Voltage Detector Interrupt Enable
AVDF		bit	SICSR,5		;  (Page 0) Voltage Detector Flag
LVDRF		bit	SICSR,4		;  (Page 0) LVD Reset Flag
CSSIE		bit	SICSR,2		;  (Page 0) Clock Security System Interrupt Enable
CSSD		bit	SICSR,1		;  (Page 0) Clock Security System Detection
WDGRF		bit	SICSR,0		;  (Page 0) Watchdog Reset Flag
VCOEN		bit	SICSR,5		;  (Page 1) VCO Enable
LOCK		bit	SICSR,4		;  (Page 1) PLL Locked
PLLEN		bit	SICSR,3		;  (Page 1) PLL Enable
CKSEL		bit	SICSR,1		;  (Page 1) Clock Source Selection

;----------------------------------------------------------------------------
; Motor Controller

MTIM		label	$0050		; (Page 0) Timer Counter High Register
MTIML		label	$0051		; (Page 0) Timer Counter Low Register
MZPRV		label	$0052		; (Page 0) Capture Zn-1 Register
MZREG		label	$0053		; (Page 0) Capture Zn Register
MCOMP		label	$0054		; (Page 0) Compare Cn+1 Register
MDREG		label	$0055		; (Page 0) Demagnetization Register
MWGHT		label	$0056		; (Page 0) An Weight Register
MPRSR		label	$0057		; Prescaler & Sampling Register
MIMR		label	$0058		; Interrupt Mask Register
PUM		bit	MIMR,7		;  PWM Update Mask Bit
SEM		bit	MIMR,6		;  Speed Error Mask Bit
RIM		bit	MIMR,5		;  Ratio Update Interrupt Mask Bit
CLIM		bit	MIMR,4		;  Current Limitation Interrupt Mask Bit
EIM		bit	MIMR,3		;  Emergency Stop Interrupt Mask Bit
ZIM		bit	MIMR,2		;  Back EMF Zero-crossing Interrupt Mask Bit
DIM		bit	MIMR,1		;  End of Demagnetization Interrupt Mask Bit
CIM		bit	MIMR,0		;  Commutation / Capture Interrupt Mask Bit
MISR		label	$0059		; Interrupt Status Register
PUI		bit	MISR,7		;  PWM Update Interrupt Flag
RPI		bit	MISR,6		;  Ratio Increment Interrupt Flag
RMI		bit	MISR,5		;  Ratio Decrement Interrupt Flag
CLI		bit	MISR,4		;  Current Limitation Interrupt Flag
EI		bit	MISR,3		;  Emergency stop Interrupt Flag
ZI		bit	MISR,2		;  BEMF Zero-Crossing Interrupt Flag
DI		bit	MISR,1		;  End of Demagnetization Interrupt Flag
CI		bit	MISR,0		;  Commutation / Capture Interrupt Flag
MCRA		label	$005a		; Control Register A
MOE		bit	MCRA,7		;  Output Enable Bit
CKE		bit	MCRA,6		;  Clock Enable Bit
SR		bit	MCRA,5		;  Sensor ON/OFF
DAC		bit	MCRA,4		;  Direct Access to Phase State Register
V0C1		bit	MCRA,3		;  Voltage/Current Mode
SWA		bit	MCRA,2		;  Switched/Autoswitched Mode
PZ		bit	MCRA,1		;  Protection from Parasitic Zero-Crossing Event Detection
DCB		bit	MCRA,0		;  Data Capture Bit
MCRB		label	$005b		; Control Register B
CPB		bit	MCRB,6		;  Compare Bit for Zero-Crossing Detection
HDM		bit	MCRB,5		;  Hardware Demagnetization Event Mask Bit
SDM		bit	MCRB,4		;  Simulated Demagnetization Event Mask Bit
OCV		bit	MCRB,3		;  Over Current Handling in Voltage Mode
OS		bfield	MCRB,0,3	;  Operating Output Mode Selection Bits
MCRC		label	$005c		; Control Register C
SEI_OI		bit	MCRC,7		;  Speed Error Interrupt Flag / MTIM Overflow Flag
EDIR_HZ		bit	MCRC,6		;  Encoder Direction Bit / Hardware Zero-Crossing Event Bit
SZ		bit	MCRC,5		;  Simulated Zero-Crossing Event Bit
SC		bit	MCRC,4		;  Simulated Commutation Event Bit
SPLG		bit	MCRC,3		;  Sampling Z Event at High Frequency in Sensorless Mode
VR		bfield	MCRC,0,3	;  BEMF/Demagnetisation Reference Threshold
MPHST		label	$005d		; Phase State Register
IS		bfield	MPHST,6,2	;  Input Selection bits
OO5		bit	MPHST,5		;  Channel On/Off bits
OO4		bit	MPHST,4
OO3		bit	MPHST,3
OO2		bit	MPHST,2
OO1		bit	MPHST,1
OO0		bit	MPHST,0
MDFR		label	$005e		; D event Filter Register
MCFR		label	$005f		; Current feedback Filter Register
RPGS		bit	MCFR,7		;  Register Page Selection
MT_RST		bit	MCFR,6		;  Reset MTC Registers
CFF		bfield	MCFR,3,3	;  Current Feedback Filter Bits
CFW		bfield	MCFR,0,3	;  Current Window Filter Bits
MREF		label	$0060		; Reference Register
MPCR		label	$0061		; PWM Control Register
MREP		label	$0062		; Repetition Counter Register
MCPWH		label	$0063		; Compare Phase W Preload Register High
MCPWL		label	$0064		; Compare Phase W Preload Register Low
MCPVH		label	$0065		; Compare Phase V Preload Register High
MCPVL		label	$0066		; Compare Phase V Preload Register Low
MCPUH		label	$0067		; Compare Phase U Preload Register High
MCPUL		label	$0068		; Compare Phase U Preload Register Low
MCP0H		label	$0069		; Compare Phase 0 Preload Register High
MCP0L		label	$006a		; Compare Phase 0 Preload Register Low

MDTG		label	$0050		; (Page 1) Dead Time Generator Enable
MPOL		label	$0051		; (Page 1) Polarity Register
MPWME		label	$0052		; (Page 1) PWM Register
MCONF		label	$0053		; (Page 1) Configuration Register
MPAR		label	$0054		; (Page 1) Parity Register
MZRF		label	$0055		; (Page 1) Z event Filter Register
MSCR		label	$0056		; (Page 1) Sampling Clock Register

;----------------------------------------------------------------------------
; Debug Module

		include	"dm.inc"
		__defdm	$006b

;----------------------------------------------------------------------------
; PWM ART

		include	"pwm_art.inc"
		__defpwmart $0074

;----------------------------------------------------------------------------
; OpAmp

OACSR		label	$007f		; OPAMP Control/Status Register

		restore
		endif			; __regmc1inc
