		ifndef	__s12z_cpmu_inc
__s12z_cpmu_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZCPMU.INC                                            *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z CPMU                      *
;*                                                                          *
;****************************************************************************

CPMURESERVED00	equ	$06C0
CPMURESERVED01	equ	$06C1
CPMURESERVED02	equ	$06C2
CPMURFLG	equ	$06C3		; Reset Flags Register
PORF		s12zbit	CPMURFLG,6	;  Power on Reset Flag
LVRF		s12zbit	CPMURFLG,5	;  Low Voltage Reset Flag
COPRF		s12zbit	CPMURFLG,3	;  COP Reset Flag
OMRF		s12zbit	CPMURFLG,1	;  Oscillator Clock Monitor Reset Flag
PMRF		s12zbit	CPMURFLG,0	;  PLL Clock Monitor Reset Flag
CPMUSYNR	equ	$06C4		; Synthesizer Register
VCOFRQ		s12zfld	CPMUSYNR,2:6	;  VCO gain
SYNDIV		s12zfld	CPMUSYNR,6:0	;  Divider
CPMUREFDIV	equ	$06C5		; Reference Divider Register
REFFRQ		s12zfld	CPMUREFDIV,2:6	;  REFFRQ
REFDIV		s12zfld	CPMUREFDIV,4:0	;  Divider
CPMUPOSTDIV	equ	$06C6		; Post Divider Register
POSTDIV		s12zfld	CPMUPOSTDIV,5:0	; Divider
CPMUIFLG	equ	$06C7		; Interrupt Flags Register
RTIF		s12zbit	CPMUIFLG,7	;  Real Time Interrupt Flag
LOCKIF		s12zbit	CPMUIFLG,4	;  PLL Lock Interrupt Flag
LOCK		s12zbit	CPMUIFLG,3	;  Lock Status Bit
OSCIF		s12zbit	CPMUIFLG,1	;  Oscillator Interrupt Flag
UPOSC		s12zbit	CPMUIFLG,0	;  Oscillator Status Bit
CPMUINT		equ	$06C8		; Interrupt Enable Register
RTIE		s12zbit	CPMUINT,7	;  Real Time Interrupt Enable Bit
LOCKIE		s12zbit	CPMUINT,4	;  PLL Lock Interrupt Enable Bit
OSCIE		s12zbit	CPMUINT,0	;  Oscillator Corrupt Interrupt Enable Bit
CPMUCLKS	equ	$06C9		; Clock Select Register
PLLSEL		s12zbit	CPMUCLKS,7	;  PLL Select Bit
PSTP		s12zbit	CPMUCLKS,6	;  Pseudo Stop Bit
CSAD		s12zbit	CPMUCLKS,5	;  COP in Stop Mode ACLK Disable
COPOSCSEL1	s12zbit	CPMUCLKS,4	;  COP Clock Select 1
PRE		s12zbit	CPMUCLKS,3	;  RTI Enable During Pseudo Stop Bit
PCE		s12zbit	CPMUCLKS,2	;  COP Enable During Pseudo Stop Bit
RTIOSCSEL	s12zbit	CPMUCLKS,1	;  RTI Clock Select
COPOSCSEL0	s12zbit	CPMUCLKS,0	;  COP Clock Select 0
CPMUPLL		equ	$06CA		; PLL Control Register
FM1		s12zbit	CPMUPLL,5	;  PLL Frequency Modulation Enable Bits
FM0		s12zbit	CPMUPLL,4
CPMURTI		equ	$06CB		; RTI Control Register
RTDEC		s12zbit	CPMURTI,7	;  Decimal or Binary Divider Select Bit
RTR		s12zfld	CPMURTI,7:0	;  Real Time Interrupt Prescale Rate Select Bits (4..6)
					;  Real Time Interrupt Modulus Counter Select Bits (0..3)
CPMUCOP		equ	$06CC		; COP Control Register
WCOP		s12zbit	CPMUCOP,7	;  Window COP Mode Bit
RSBCK		s12zbit	CPMUCOP,6	;  COP and RTI Stop in Active BDM Mode Bit
WRTMASK		s12zbit	CPMUCOP,5	;  Write Mask for WCOP and CR[2:0] Bit
CR		s12zfld	CPMUCOP,3:0	;  COP Watchdog Timer Rate Select
CPMUTEST0	equ	$06CD
CPMUTEST1	equ	$06CE
CPMUARMCOP	equ	$06CF		; COP Timer Arm/Reset Register
CPMUHTCTL	equ	$06D0		; High Temperature Control Register
VSEL		s12zbit	CPMUHTCTL,5	;  Voltage Access Select Bit
HTE		s12zbit	CPMUHTCTL,3	;  High Temperature Sensor/Bandgap Voltage Enable Bit
HTDS		s12zbit	CPMUHTCTL,2	;  High Temperature Detect Status Bit
HTIE		s12zbit	CPMUHTCTL,1	;  High Temperature Interrupt Enable Bit
HTIF		s12zbit	CPMUHTCTL,0	;  High Temperature Interrupt Flag
CPMULVCTL	equ	$06D1		; Low Voltage Control Register
LVDS		s12zbit	CPMULVCTL,2	;  Low-Voltage Detect Status Bit
LVIE		s12zbit	CPMULVCTL,1	;  Low-Voltage Interrupt Enable Bit
LVIF		s12zbit	CPMULVCTL,0	;  Low-Voltage Interrupt Flag
CPMUAPICTL	equ	$06D2		; Autonomous Periodical Interrupt Control Register
APICLK		s12zbit	CPMUAPICTL,7	;  Autonomous Periodical Interrupt Clock Select Bit
APIES		s12zbit	CPMUAPICTL,4	;  Autonomous Periodical Interrupt External Select Bit
APIEA		s12zbit	CPMUAPICTL,3	;  Autonomous Periodical Interrupt External Access Enable Bit
APIFE		s12zbit	CPMUAPICTL,2	;  Autonomous Periodical Interrupt Feature Enable Bit
APIE		s12zbit	CPMUAPICTL,1	;  Autonomous Periodical Interrupt Enable Bit
APIF		s12zbit	CPMUAPICTL,0	;  Autonomous Periodical Interrupt Flag
CPMUACLKTR	equ	$06D3		; Autonomous Clock Trimming Register
ACLKTR		s12zfld	CPMUACLKTR,6:2	;  Autonomous Clock Period Trimming Bits
CPMUAPIR	equ	$06D4		; Autonomous Periodical Interrupt Rate (16 bit)
CPMUAPIRH	equ	$06D4		; Autonomous Periodical Interrupt Rate MSB
CPMUAPIRL	equ	$06D5		; Autonomous Periodical Interrupt Rate LSB
CPMUTEST3	equ	$06D6
CPMUHTTR	equ	$06D7		; High Temperature Trimming Register
HTOE		s12zbit	CPMUHTTR,7	;  High Temperature Offset Enable Bit
HTTR		s12zfld	CPMUHTTR,4:0	;  High Temperature Trimming Bits
CPMUIRCTRIMH	equ	$06D8		; IRC1M Trim Register MSB
TCTRIM		s12zfld	CPMUIRCTRIMH,5:3;  IRC1M temperature coefficient Trim Bits
IRCTRIM		s12zfld.w	CPMUIRCTRIMH,10:0;  IRC1M Frequency Trim Bits
CPMUIRCTRIML	equ	$06D9		; IRC1M Trim Register LSB
CPMUOSC		equ	$06DA		; Oscillator Register
OSCE		s12zbit	CPMUOSC,7	;  Oscillator Enable Bit
CPMUPROT	equ	$06DB		; Protection Register
PROT		s12zbit	CPMUPROT,0	;  Clock Configuration Registers Protection
CPMUTEST2	equ	$06DC
CPMUVREGCTL     equ	$06DD		; Voltage Regulator Control Register
EXTCON		s12zbit	CPMUVREGCTL,2	;  External voltage regulator Enable Bit for VDDC domain
EXTXON		s12zbit	CPMUVREGCTL,1	;  External voltage regulator Enable Bit for VDDX domain
INTXON		s12zbit	CPMUVREGCTL,0	;  Internal voltage regulator Enable Bit for VDDX domain
CPMUOSC2	equ	$06DE		; Oscillator Register 2
OMRE		s12zbit	CPMUOSC2,1	;  select the mode of the external oscillator
OSCMOD		s12zbit	CPMUOSC2,0	;  enable the oscillator clock monitor reset
CPMURESERVED1F	equ	$06DF

		restore			; re-enable listing

		endif			; __s12z_cpmu_inc
