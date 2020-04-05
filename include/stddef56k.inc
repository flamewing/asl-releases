                ifndef  stddef56inc     ; avoid multiple inclusion
stddef56inc     equ     1

                save
                listing off		; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF56.INC                                            *
;*   								            *
;*   Contains SFR Definitions for the DSP56000                              *
;*                                                                          *
;****************************************************************************

                if      (MOMCPU<>352256)
                 fatal  "wrong target selected: only 56000 allowed"
		endif

                if      MOMPASS=1
                 message "DSP56000 SFR Definitions (C) 1993 Alfred Arnold"
		endif

;---------------------------------------------------------------------------

pbc		xsfr	$ffe0		; Port B Bus Control Register
pbddr		xsfr	$ffe2		; Port B Data Direction Register
pbd		xsfr	$ffe4		; Port B Data Register

pcc		xsfr	$ffe1		; Port C Control Register
pcddr		xsfr	$ffe3		; Port C Data Direction Register
pcd		xsfr	$ffe5		; Port C Data Register

hcr		xsfr	$ffe8		; Host Control Register
hsr		xsfr	$ffe9		; Host Status Register
htx		xsfr	$ffeb		; Host Transmit Register
hrx		xsfr	htx		; Host Receive Register

cra		xsfr	$ffec		; SSI Control Register A
crb		xsfr	$ffed		; SSI Control Register B
tsr		xsfr	$ffee		; SSI Status/Time Slot Register

tx		xsfr	$ffef		; SCI Transmit Register
rx		xsfr	tx		; SCI Receive Register
scr		xsfr	$fff0		; SCI Interface Control Register
ssr		xsfr	$fff1		; SCI Interface Status Register
sccr		xsfr	$fff2		; SCI Control Register
stxa		xsfr	$fff3		; SCI Transmit Data Address Register
srx		xsfr	$fff4		; SCI Receive Register Base Addres
stx		xsfr	srx		; SCI Transmit Register Base Addres
srx_lo		xsfr	srx		; SCI Receive Register in Bits 0..7
stx_lo		xsfr	stx		; SCI Transmit Register in Bits 0..7
srx_mid		xsfr	srx+1		; SCI Receive Register in Bits 8..15
stx_mid		xsfr	stx+1		; SCI Transmit Register in Bits 8..15
srx_hi		xsfr	srx+2		; SCI Receive Register in Bits 16..23
stx_hi		xsfr	stx+2		; SCI Transmit Register in Bits 16..23

bcr		xsfr	$fffe		; Bus Control Register
ipr		xsfr	$ffff		; Interrupt Priority Register

;---------------------------------------------------------------------------

		restore                 ; allow listing again

		endif			; stddef56inc


