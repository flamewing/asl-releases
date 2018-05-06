		ifndef	__s12z_vcpim_inc
__s12z_vcpim_inc	equ	1

		save
		listing	off	; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File S12ZVCPIM.INC                                           *
;*                                                                          *
;*   Contains Bit & Register Definitions for S12Z Port Integration Module   *
;*                                                                          *
;****************************************************************************

MODRR0		equ	$0200		; Module Routing Register 0
IIC0RR1		s12zbit	MODRR0,7	;  IIC0 routing
IIC0RR0		s12zbit	MODRR0,6
SCI1RR		s12zbit	MODRR0,5	;  SCI1 routing
SCI0RR		s12zbit	MODRR0,4	;  SCI0 routing
SPI0RR		s12zbit	MODRR0,3	;  SPI0 routing
M0C0RR2		s12zbit	MODRR0,2	;  MSCAN0-CANPHY0 routing
M0C0RR1		s12zbit	MODRR0,1
M0C0RR0		s12zbit	MODRR0,0
MODRR1		equ	$0201		; Module Routing Register 1
T1IC3RR		s12zbit	MODRR1,7	;  TIM1 IC3 routing
T1IC2RR		s12zbit	MODRR1,6	;  TIM1 IC2 routing
TRIG0NEG	s12zbit	MODRR1,2	;  ADC0 trigger input inverted polarity
TRIG0RR1	s12zbit	MODRR1,1	;  ADC0 trigger input routing
TRIG0RR0	s12zbit	MODRR1,0
MODRR2		equ	$0202		; Module Routing Register 2
P0C7RR		s12zbit	MODRR2,7	;  PWM0_7 routing
P0C3RR		s12zbit	MODRR2,3	;  PWM0_3 routing
MODRR3		equ	$0203		; Module Routing Register 3
T0IC3RR1	s12zbit	MODRR3,4	;  TIM0 IC3 routing bit 1
T0IC3RR0	s12zbit	MODRR3,3	;  TIM0 IC3 routing bit 0
T0IC2RR		s12zbit	MODRR3,2	;  TIM0 IC2 routing
T0IC1RR		s12zbit	MODRR3,1	;  TIM0 IC1 routing
ECLKCTL		equ	$0208		; ECLK Control Register
NECLK		s12zbit	ECLKCTL,7	;  Disable ECLK output
IRQCR		equ	$0209		; IRQ Control Register
IRQE		s12zbit	IRQCR,7		;  IRQ select edge sensitive only
IRQEN		s12zbit	IRQCR,6		;  IRQ enable

		if	__porte_mask
PTE		equ	$0260		; Port E Data
PTIE		equ	$0262		; Port E Input Data
DDRE		equ	$0264		; Port E Data Direction Register
PERE		equ	$0266		; Port E Pull Enable Register
PPSE		equ	$0268		; Port E Polarity Select Register
		endif
		if	__porte_mask&128
PTE7		s12zbit	PTE,7
PTIE7		s12zbit	PTIE,7
DDRE7		s12zbit	DDRE,7
PERE7		s12zbit	PERE,7
PPSE7		s12zbit	PPSE,7
		endif
		if	__porte_mask&64
PTE6		s12zbit	PTE,6
PTIE6		s12zbit	PTIE,6
DDRE6		s12zbit	DDRE,6
PERE6		s12zbit	PERE,6
PPSE6		s12zbit	PPSE,6
		endif
		if	__porte_mask&32
PTE5		s12zbit	PTE,5
PTIE5		s12zbit	PTIE,5
DDRE5		s12zbit	DDRE,5
PERE5		s12zbit	PERE,5
PPSE5		s12zbit	PPSE,5
		endif
		if	__porte_mask&16
PTE4		s12zbit	PTE,4
PTIE4		s12zbit	PTIE,4
DDRE4		s12zbit	DDRE,4
PERE4		s12zbit	PERE,4
PPSE4		s12zbit	PPSE,4
		endif
		if	__porte_mask&8
PTE3		s12zbit	PTE,3
PTIE3		s12zbit	PTIE,3
DDRE3		s12zbit	DDRE,3
PERE3		s12zbit	PERE,3
PPSE3		s12zbit	PPSE,3
		endif
		if	__porte_mask&4
PTE2		s12zbit	PTE,2
PTIE2		s12zbit	PTIE,2
DDRE2		s12zbit	DDRE,2
PERE2		s12zbit	PERE,2
PPSE2		s12zbit	PPSE,2
		endif
		if	__porte_mask&2
PTE1		s12zbit	PTE,1
PTIE1		s12zbit	PTIE,1
DDRE1		s12zbit	DDRE,1
PERE1		s12zbit	PERE,1
PPSE1		s12zbit	PPSE,1
		endif
		if	__porte_mask&1
PTE0		s12zbit	PTE,0
PTIE0		s12zbit	PTIE,0
DDRE0		s12zbit	DDRE,0
PERE0		s12zbit	PERE,0
PPSE0		s12zbit	PPSE,0
		endif

		if	__portadh_mask
PTADH		equ	$0280		; Port AD Data MSB
PTIADH		equ	$0282		; Port AD Input Data MSB
DDRADH		equ	$0284		; Port AD Data Direction Register MSB
PERADH		equ	$0286		; Port AD Pull Enable Register MSB
PPSADH		equ	$0288		; Port AD Polarity Select Register MSB
PIEADH		equ	$028C		; Port AD Interrupt Enable Register MSB
PIFADH		equ	$028E		; Port AD Interrupt Flag Register MSB
DIENADH		equ	$0298		; Port AD Digital Input Enable Register MSB
		endif
		if	__portadh_mask&128
PTADH7		s12zbit	PTADH,7
PTIADH7		s12zbit	PTIADH,7
DDRADH7		s12zbit	DDRADH,7
PERADH7		s12zbit	PERADH,7
PPSADH7		s12zbit	PPSADH,7
PIEADH7		s12zbit	PIEADH,7
PIFADH7		s12zbit	PIFADH,7
DIENADH7	s12zbit	DIENADH,7
		endif
		if	__portadh_mask&64
PTADH6		s12zbit	PTADH,6
PTIADH6		s12zbit	PTIADH,6
DDRADH6		s12zbit	DDRADH,6
PERADH6		s12zbit	PERADH,6
PPSADH6		s12zbit	PPSADH,6
PIEADH6		s12zbit	PIEADH,6
PIFADH6		s12zbit	PIFADH,6
DIENADH6	s12zbit	DIENADH,6
		endif
		if	__portadh_mask&32
PTADH5		s12zbit	PTADH,5
PTIADH5		s12zbit	PTIADH,5
DDRADH5		s12zbit	DDRADH,5
PERADH5		s12zbit	PERADH,5
PPSADH5		s12zbit	PPSADH,5
PIEADH5		s12zbit	PIEADH,5
PIFADH5		s12zbit	PIFADH,5
DIENADH5	s12zbit	DIENADH,5
		endif
		if	__portadh_mask&16
PTADH4		s12zbit	PTADH,4
PTIADH4		s12zbit	PTIADH,4
DDRADH4		s12zbit	DDRADH,4
PERADH4		s12zbit	PERADH,4
PPSADH4		s12zbit	PPSADH,4
PIEADH4		s12zbit	PIEADH,4
PIFADH4		s12zbit	PIFADH,4
DIENADH4	s12zbit	DIENADH,4
		endif
		if	__portadh_mask&8
PTADH3		s12zbit	PTADH,3
PTIADH3		s12zbit	PTIADH,3
DDRADH3		s12zbit	DDRADH,3
PERADH3		s12zbit	PERADH,3
PPSADH3		s12zbit	PPSADH,3
PIEADH3		s12zbit	PIEADH,3
PIFADH3		s12zbit	PIFADH,3
DIENADH3	s12zbit	DIENADH,3
		endif
		if	__portadh_mask&4
PTADH2		s12zbit	PTADH,2
PTIADH2		s12zbit	PTIADH,2
DDRADH2		s12zbit	DDRADH,2
PERADH2		s12zbit	PERADH,2
PPSADH2		s12zbit	PPSADH,2
PIEADH2		s12zbit	PIEADH,2
PIFADH2		s12zbit	PIFADH,2
DIENADH2	s12zbit	DIENADH,2
		endif
		if	__portadh_mask&2
PTADH1		s12zbit	PTADH,1
PTIADH1		s12zbit	PTIADH,1
DDRADH1		s12zbit	DDRADH,1
PERADH1		s12zbit	PERADH,1
PPSADH1		s12zbit	PPSADH,1
PIEADH1		s12zbit	PIEADH,1
PIFADH1		s12zbit	PIFADH,1
DIENADH1	s12zbit	DIENADH,1
		endif
		if	__portadh_mask&1
PTADH0		s12zbit	PTADH,0
PTIADH0		s12zbit	PTIADH,0
DDRADH0		s12zbit	DDRADH,0
PERADH0		s12zbit	PERADH,0
PPSADH0		s12zbit	PPSADH,0
PIEADH0		s12zbit	PIEADH,0
PIFADH0		s12zbit	PIFADH,0
DIENADH0	s12zbit	DIENADH,0
		endif

		if	__portadl_mask
PTADL		equ	$0281		; Port AD Data LSB
PTIADL		equ	$0283		; Port AD Input Data LSB
DDRADL		equ	$0285		; Port AD Data Direction Register LSB
PERADL		equ	$0287		; Port AD Pull Enable Register LSB
PPSADL		equ	$0289		; Port AD Polarity Select Register LSB
PIEADL		equ	$028D		; Port AD Interrupt Enable Register LSB
PIFADL		equ	$028F		; Port AD Interrupt Flag Register LSB
DIENADL		equ	$0299		; Port AD Digital Input Enable Register LSB
		endif
		if	__portadl_mask&128
PTADL7		s12zbit	PTADL,7
PTIADL7		s12zbit	PTIADL,7
DDRADL7		s12zbit	DDRADL,7
PERADL7		s12zbit	PERADL,7
PPSADL7		s12zbit	PPSADL,7
PIEADL7		s12zbit	PIEADL,7
PIFADL7		s12zbit	PIFADL,7
DIENADL7	s12zbit	DIENADL,7
		endif
		if	__portadl_mask&64
PTADL6		s12zbit	PTADL,6
PTIADL6		s12zbit	PTIADL,6
DDRADL6		s12zbit	DDRADL,6
PERADL6		s12zbit	PERADL,6
PPSADL6		s12zbit	PPSADL,6
PIEADL6		s12zbit	PIEADL,6
PIFADL6		s12zbit	PIFADL,6
DIENADL6	s12zbit	DIENADL,6
		endif
		if	__portadl_mask&32
PTADL5		s12zbit	PTADL,5
PTIADL5		s12zbit	PTIADL,5
DDRADL5		s12zbit	DDRADL,5
PERADL5		s12zbit	PERADL,5
PPSADL5		s12zbit	PPSADL,5
PIEADL5		s12zbit	PIEADL,5
PIFADL5		s12zbit	PIFADL,5
DIENADL5	s12zbit	DIENADL,5
		endif
		if	__portadl_mask&16
PTADL4		s12zbit	PTADL,4
PTIADL4		s12zbit	PTIADL,4
DDRADL4		s12zbit	DDRADL,4
PERADL4		s12zbit	PERADL,4
PPSADL4		s12zbit	PPSADL,4
PIEADL4		s12zbit	PIEADL,4
PIFADL4		s12zbit	PIFADL,4
DIENADL4	s12zbit	DIENADL,4
		endif
		if	__portadl_mask&8
PTADL3		s12zbit	PTADL,3
PTIADL3		s12zbit	PTIADL,3
DDRADL3		s12zbit	DDRADL,3
PERADL3		s12zbit	PERADL,3
PPSADL3		s12zbit	PPSADL,3
PIEADL3		s12zbit	PIEADL,3
PIFADL3		s12zbit	PIFADL,3
DIENADL3	s12zbit	DIENADL,3
		endif
		if	__portadl_mask&4
PTADL2		s12zbit	PTADL,2
PTIADL2		s12zbit	PTIADL,2
DDRADL2		s12zbit	DDRADL,2
PERADL2		s12zbit	PERADL,2
PPSADL2		s12zbit	PPSADL,2
PIEADL2		s12zbit	PIEADL,2
PIFADL2		s12zbit	PIFADL,2
DIENADL2	s12zbit	DIENADL,2
		endif
		if	__portadl_mask&2
PTADL1		s12zbit	PTADL,1
PTIADL1		s12zbit	PTIADL,1
DDRADL1		s12zbit	DDRADL,1
PERADL1		s12zbit	PERADL,1
PPSADL1		s12zbit	PPSADL,1
PIEADL1		s12zbit	PIEADL,1
PIFADL1		s12zbit	PIFADL,1
DIENADL1	s12zbit	DIENADL,1
		endif
		if	__portadl_mask&1
PTADL0		s12zbit	PTADL,0
PTIADL0		s12zbit	PTIADL,0
DDRADL0		s12zbit	DDRADL,0
PERADL0		s12zbit	PERADL,0
PPSADL0		s12zbit	PPSADL,0
PIEADL0		s12zbit	PIEADL,0
PIFADL0		s12zbit	PIFADL,0
DIENADL0	s12zbit	DIENADL,0
		endif

		if	__portt_mask
PTT		equ	$02C0		; Port T Data
PTIT		equ	$02C1		; Port T Input Data
DDRT		equ	$02C2		; Port T Data Direction Register
PERT		equ	$02C3		; Port T Pull Enable Register
PPST		equ	$02C4		; Port T Polarity Select Register
		endif
		if	__portt_mask&128
PTT7		s12zbit	PTT,7
PTIT7		s12zbit	PTIT,7
DDRT7		s12zbit	DDRT,7
PERT7		s12zbit	PERT,7
PPST7		s12zbit	PPST,7
		endif
		if	__portt_mask&64
PTT6		s12zbit	PTT,6
PTIT6		s12zbit	PTIT,6
DDRT6		s12zbit	DDRT,6
PERT6		s12zbit	PERT,6
PPST6		s12zbit	PPST,6
		endif
		if	__portt_mask&32
PTT5		s12zbit	PTT,5
PTIT5		s12zbit	PTIT,5
DDRT5		s12zbit	DDRT,5
PERT5		s12zbit	PERT,5
PPST5		s12zbit	PPST,5
		endif
		if	__portt_mask&16
PTT4		s12zbit	PTT,4
PTIT4		s12zbit	PTIT,4
DDRT4		s12zbit	DDRT,4
PERT4		s12zbit	PERT,4
PPST4		s12zbit	PPST,4
		endif
		if	__portt_mask&8
PTT3		s12zbit	PTT,3
PTIT3		s12zbit	PTIT,3
DDRT3		s12zbit	DDRT,3
PERT3		s12zbit	PERT,3
PPST3		s12zbit	PPST,3
		endif
		if	__portt_mask&4
PTT2		s12zbit	PTT,2
PTIT2		s12zbit	PTIT,2
DDRT2		s12zbit	DDRT,2
PERT2		s12zbit	PERT,2
PPST2		s12zbit	PPST,2
		endif
		if	__portt_mask&2
PTT1		s12zbit	PTT,1
PTIT1		s12zbit	PTIT,1
DDRT1		s12zbit	DDRT,1
PERT1		s12zbit	PERT,1
PPST1		s12zbit	PPST,1
		endif
		if	__portt_mask&1
PTT0		s12zbit	PTT,0
PTIT0		s12zbit	PTIT,0
DDRT0		s12zbit	DDRT,0
PERT0		s12zbit	PERT,0
PPST0		s12zbit	PPST,0
		endif

		if	__ports_mask
PTS		equ	$02D0		; Port S Data
PTIS		equ	$02D1		; Port S Input Data
DDRS		equ	$02D2		; Port S Data Direction Register
PERS		equ	$02D3		; Port S Pull Enable Register
PPSS		equ	$02D4		; Port S Polarity Select Register
PIES		equ	$02D6		; Port S Interrupt Enable Register
PIFS		equ	$02D7		; Port S Interrupt Flag Register
WOMS		equ	$02DF		; Port S Wired-OR Mode Register
		endif
		if	__ports_mask&128
PTS7		s12zbit	PTS,7
PTIS7		s12zbit	PTIS,7
DDRS7		s12zbit	DDRS,7
PERS7		s12zbit	PERS,7
PPSS7		s12zbit	PPSS,7
PIES7		s12zbit	PIES,7
PIFS7		s12zbit	PIFS,7
WOMS7		s12zbit	WOMS,7
		endif
		if	__ports_mask&64
PTS6		s12zbit	PTS,6
PTIS6		s12zbit	PTIS,6
DDRS6		s12zbit	DDRS,6
PERS6		s12zbit	PERS,6
PPSS6		s12zbit	PPSS,6
PIES6		s12zbit	PIES,6
PIFS6		s12zbit	PIFS,6
WOMS6		s12zbit	WOMS,6
		endif
		if	__ports_mask&32
PTS5		s12zbit	PTS,5
PTIS5		s12zbit	PTIS,5
DDRS5		s12zbit	DDRS,5
PERS5		s12zbit	PERS,5
PPSS5		s12zbit	PPSS,5
PIES5		s12zbit	PIES,5
PIFS5		s12zbit	PIFS,5
WOMS5		s12zbit	WOMS,5
		endif
		if	__ports_mask&16
PTS4		s12zbit	PTS,4
PTIS4		s12zbit	PTIS,4
DDRS4		s12zbit	DDRS,4
PERS4		s12zbit	PERS,4
PPSS4		s12zbit	PPSS,4
PIES4		s12zbit	PIES,4
PIFS4		s12zbit	PIFS,4
WOMS4		s12zbit	WOMS,4
		endif
		if	__ports_mask&8
PTS3		s12zbit	PTS,3
PTIS3		s12zbit	PTIS,3
DDRS3		s12zbit	DDRS,3
PERS3		s12zbit	PERS,3
PPSS3		s12zbit	PPSS,3
PIES3		s12zbit	PIES,3
PIFS3		s12zbit	PIFS,3
WOMS3		s12zbit	WOMS,3
		endif
		if	__ports_mask&4
PTS2		s12zbit	PTS,2
PTIS2		s12zbit	PTIS,2
DDRS2		s12zbit	DDRS,2
PERS2		s12zbit	PERS,2
PPSS2		s12zbit	PPSS,2
PIES2		s12zbit	PIES,2
PIFS2		s12zbit	PIFS,2
WOMS2		s12zbit	WOMS,2
		endif
		if	__ports_mask&2
PTS1		s12zbit	PTS,1
PTIS1		s12zbit	PTIS,1
DDRS1		s12zbit	DDRS,1
PERS1		s12zbit	PERS,1
PPSS1		s12zbit	PPSS,1
PIES1		s12zbit	PIES,1
PIFS1		s12zbit	PIFS,1
WOMS1		s12zbit	WOMS,1
		endif
		if	__ports_mask&1
PPSS0		s12zbit	PPSS,0
PTS0		s12zbit	PTS,0
PTIS0		s12zbit	PTIS,0
DDRS0		s12zbit	DDRS,0
PERS0		s12zbit	PERS,0
PIES0		s12zbit	PIES,0
PIFS0		s12zbit	PIFS,0
WOMS0		s12zbit	WOMS,0
		endif

		if	__portp_mask
PTP		equ	$02F0		; Port P Data
PTIP		equ	$02F1		; Port P Input Data
DDRP		equ	$02F2		; Port P Data Direction Register
PERP		equ	$02F3		; Port P Pull Enable Register
PPSP		equ	$02F4		; Port P Polarity Select Register
PIEP		equ	$02F6		; Port P Interrupt Enable Register
PIFP		equ	$02F7		; Port P Interrupt Flag Register
OCPEP		equ	$02F9		; Port P Over-Current Protection Enable Register
OCIEP		equ	$02FA		; Port P Over-Current Interrupt Enable Register
OCIFP		equ	$2FB		; Port P Over-Current Interrupt Flag Register
RDRP		equ	$02FD		; Port P Reduced Drive Register
		endif
		if	__portp_mask&128
PTP7		s12zbit	PTP,7
PTIP7		s12zbit	PTIP,7
DDRP7		s12zbit	DDRP,7
PERP7		s12zbit	PERP,7
PPSP7		s12zbit	PPSP,7
PIEP7		s12zbit	PIEP,7
PIFP7		s12zbit	PIFP,7
RDRP7		s12zbit	RDRP,7
		endif
		if	__portp_mask&64
PTP6		s12zbit	PTP,6
PTIP6		s12zbit	PTIP,6
DDRP6		s12zbit	DDRP,6
PERP6		s12zbit	PERP,6
PPSP6		s12zbit	PPSP,6
PIEP6		s12zbit	PIEP,6
PIFP6		s12zbit	PIFP,6
OCPEP6		s12zbit	OCPEP,6
OCIEP6		s12zbit	OCIEP,6
OCIFP6		s12zbit	OCIFP,6
RDRP6		s12zbit	RDRP,6
		endif
		if	__portp_mask&32
PTP5		s12zbit	PTP,5
PTIP5		s12zbit	PTIP,5
DDRP5		s12zbit	DDRP,5
PERP5		s12zbit	PERP,5
PPSP5		s12zbit	PPSP,5
PIEP5		s12zbit	PIEP,5
PIFP5		s12zbit	PIFP,5
OCPEP5		s12zbit	OCPEP,5
OCIEP5		s12zbit	OCIEP,5
OCIFP5		s12zbit	OCIFP,5
RDRP5		s12zbit	RDRP,5
		endif
		if	__portp_mask&16
PTP4		s12zbit	PTP,4
PTIP4		s12zbit	PTIP,4
DDRP4		s12zbit	DDRP,4
PERP4		s12zbit	PERP,4
PPSP4		s12zbit	PPSP,4
PIEP4		s12zbit	PIEP,4
PIFP4		s12zbit	PIFP,4
OCPEP4		s12zbit	OCPEP,4
OCIEP4		s12zbit	OCIEP,4
OCIFP4		s12zbit	OCIFP,4
RDRP4		s12zbit	RDRP,4
		endif
		if	__portp_mask&8
PTP3		s12zbit	PTP,3
PTIP3		s12zbit	PTIP,3
DDRP3		s12zbit	DDRP,3
PERP3		s12zbit	PERP,3
PPSP3		s12zbit	PPSP,3
PIEP3		s12zbit	PIEP,3
PIFP3		s12zbit	PIFP,3
RDRP3		s12zbit	RDRP,3
		endif
		if	__portp_mask&4
PTP2		s12zbit	PTP,2
PTIP2		s12zbit	PTIP,2
DDRP2		s12zbit	DDRP,2
PERP2		s12zbit	PERP,2
PPSP2		s12zbit	PPSP,2
PIEP2		s12zbit	PIEP,2
PIFP2		s12zbit	PIFP,2
OCPEP2		s12zbit	OCPEP,2
OCIEP2		s12zbit	OCIEP,2
OCIFP2		s12zbit	OCIFP,2
RDRP2		s12zbit	RDRP,2
		endif
		if	__portp_mask&2
PTP1		s12zbit	PTP,1
PTIP1		s12zbit	PTIP,1
DDRP1		s12zbit	DDRP,1
PERP1		s12zbit	PERP,1
PPSP1		s12zbit	PPSP,1
PIEP1		s12zbit	PIEP,1
PIFP1		s12zbit	PIFP,1
RDRP1		s12zbit	RDRP,1
		endif
		if	__portp_mask&1
PPSP0		s12zbit	PPSP,0
PTP0		s12zbit	PTP,0
PTIP0		s12zbit	PTIP,0
DDRP0		s12zbit	DDRP,0
PERP0		s12zbit	PERP,0
PIEP0		s12zbit	PIEP,0
PIFP0		s12zbit	PIFP,0
OCPEP0		s12zbit	OCPEP,0
OCIEP0		s12zbit	OCIEP,0
OCIFP0		s12zbit	OCIFP,0
RDRP0		s12zbit	RDRP,0
		endif

		if	__portj_mask
PTJ		equ	$0310		; Port J Data
PTIJ		equ	$0311		; Port J Input Data
DDRJ		equ	$0312		; Port J Data Direction Register
PERJ		equ	$0313		; Port J Pull Enable Register
PPSJ		equ	$0314		; Port J Polarity Select Register
WOMJ		equ	$031F		; Port J Wired-OR Mode Register
		endif
		if	__portj_mask&128
PTJ7		s12zbit	PTJ,7
PTIJ7		s12zbit	PTIJ,7
DDRJ7		s12zbit	DDRJ,7
PERJ7		s12zbit	PERJ,7
PPSJ7		s12zbit	PPSJ,7
WOMJ7		s12zbit	WOMJ,7
		endif
		if	__portj_mask&64
PTJ6		s12zbit	PTJ,6
PTIJ6		s12zbit	PTIJ,6
DDRJ6		s12zbit	DDRJ,6
PERJ6		s12zbit	PERJ,6
PPSJ6		s12zbit	PPSJ,6
WOMJ6		s12zbit	WOMJ,6
		endif
		if	__portj_mask&32
PTJ5		s12zbit	PTJ,5
PTIJ5		s12zbit	PTIJ,5
DDRJ5		s12zbit	DDRJ,5
PERJ5		s12zbit	PERJ,5
PPSJ5		s12zbit	PPSJ,5
WOMJ5		s12zbit	WOMJ,5
		endif
		if	__portj_mask&16
PTJ4		s12zbit	PTJ,4
PTIJ4		s12zbit	PTIJ,4
DDRJ4		s12zbit	DDRJ,4
PERJ4		s12zbit	PERJ,4
PPSJ4		s12zbit	PPSJ,4
WOMJ4		s12zbit	WOMJ,4
		endif
		if	__portj_mask&8
PTJ3		s12zbit	PTJ,3
PTIJ3		s12zbit	PTIJ,3
DDRJ3		s12zbit	DDRJ,3
PERJ3		s12zbit	PERJ,3
PPSJ3		s12zbit	PPSJ,3
WOMJ3		s12zbit	WOMJ,3
		endif
		if	__portj_mask&4
PTJ2		s12zbit	PTJ,2
PTIJ2		s12zbit	PTIJ,2
DDRJ2		s12zbit	DDRJ,2
PERJ2		s12zbit	PERJ,2
PPSJ2		s12zbit	PPSJ,2
WOMJ2		s12zbit	WOMJ,2
		endif
		if	__portj_mask&2
PTJ1		s12zbit	PTJ,1
PTIJ1		s12zbit	PTIJ,1
DDRJ1		s12zbit	DDRJ,1
PERJ1		s12zbit	PERJ,1
PPSJ1		s12zbit	PPSJ,1
WOMJ1		s12zbit	WOMJ,1
		endif
		if	__portj_mask&1
PTJ0		s12zbit	PTJ,0
PTIJ0		s12zbit	PTIJ,0
DDRJ0		s12zbit	DDRJ,0
PERJ0		s12zbit	PERJ,0
PPSJ0		s12zbit	PPSJ,0
WOMJ0		s12zbit	WOMJ,0
		endif

		if	__portl_mask
PTIL		equ	$0331		; Port L Input Data
PTPSL		equ	$0333		; Port L Pull Select Register
PPSL		equ	$0334		; Port L Polarity Select Register
PIEL		equ	$0336		; Port L Interrupt Enable Register
PIFL		equ	$0337		; Port L Interrupt Flag Register
PTABYPL		equ	$033A		; Port L ADC Bypass Register
PTADIRL		equ	$033B		; Port L ADC Direct Register
DIENL		equ	$033C		; Port L Digital Input Enable Register
PTAENL		equ	$033D		; Port L ADC Connection Enable Register
PIRL		equ	$033E		; Port L Input Divider Ratio Selection Register
PTTEL		equ	$033F		; Port L Test Enable Register
		endif
		if	__portl_mask&2
PTIL1		s12zbit	PTIL,1
PTPSL1		s12zbit	PTPSL,1
PPSL1		s12zbit	PPSL,1
PIEL1		s12zbit	PIEL,1
PIFL1		s12zbit	PIFL,1
PTABYPL1	s12zbit	PTABYPL,1
PTADIRL1	s12zbit	PTADIRL,1
DIENL1		s12zbit	DIENL,1
PTAENL1		s12zbit PTAENL,1
PIRL1		s12zbit	PIRL,1
PTTEL1		s12zbit	PTTEL,1
		endif
		if	__portl_mask&1
PTIL0		s12zbit	PTIL,0
PTPSL0		s12zbit	PTPSL,0
PPSL0		s12zbit	PPSL,0
PIEL0		s12zbit	PIEL,0
PIFL0		s12zbit	PIFL,0
PTABYPL0	s12zbit	PTABYPL,0
PTADIRL0	s12zbit	PTADIRL,0
DIENL0		s12zbit	DIENL,0
PTAENL0		s12zbit	PTAENL,0
PIRL0		s12zbit	PIRL,0
PTTEL0		s12zbit	PTTEL,0
		endif

		restore			; re-enable listing

		endif			; __s12z_vcpim_inc
