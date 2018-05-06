; -------------------------------------------------------------------------
; Part ID

PARTID0		equ	$0000		; Part ID Bit 31..24
PARTID1		equ	$0001		; Part ID Bit 23..16
PARTID2		equ	$0002		; Part ID Bit 15..8
PARTID3		equ	$0003		; Part ID Bit 7..0

; -------------------------------------------------------------------------
; S12ZINT

IVBR		equ	$0010		; Interrupt Vector Base Register
INT_XGPRIO	equ	$0016
INT_CFADDR	equ	$0017		; Interrupt Request Configuration Address Register
INT_CFDATA0	equ	$0018		; Interrupt Request Configuration Data Register 0
INT_CFDATA1	equ	$0019		; Interrupt Request Configuration Data Register 1
INT_CFDATA2	equ	$001a		; Interrupt Request Configuration Data Register 2
INT_CFDATA3	equ	$001b		; Interrupt Request Configuration Data Register 3
INT_CFDATA4	equ	$001c		; Interrupt Request Configuration Data Register 4
INT_CFDATA5	equ	$001d		; Interrupt Request Configuration Data Register 5
INT_CFDATA6	equ	$001e		; Interrupt Request Configuration Data Register 6
INT_CFDATA7	equ	$001f		; Interrupt Request Configuration Data Register 7

; -------------------------------------------------------------------------
; S12ZMMC

MODE		equ	$0070		; Mode Register
MODC		s12zbit	MODE,7		;  Mode Select Bit
MMCECH		equ	$0080		; Error Code Register MSB
ITR		s12zfld	MMCECH,4,7	;  Initiator Field
TGT		s12zfld	MMCECH,0,3	;  Target Field
MMCECL		equ	$0081		; Error Code Register LSB
ACC		s12zfld	MMCECL,4,7	;  Access Type Field
ERR		s12zfld	MMCECL,0,3	;  Error Type Field
MMCCCRH		equ	$0082		; Captured S12ZCPU Condition Code Register MSB
CPUU		s12zbit	MMCCCRH,7	;  S12ZCPU User State Flag
MMCCCRL		equ	$0083		; Captured S12ZCPU Condition Code Register LSB
CPUX		s12zbit	MMCCCRL,6	;  S12ZCPU X-Interrupt Mask
CPUI		s12zbit	MMCCCRL,4	;  S12ZCPU I-Interrupt Mask
MMCPCH		equ	$0085		; Captured S12ZCPU Program Counter MSB
MMCPCM		equ	$0086
MMCPCL		equ	$0087		; Captured S12ZCPU Program Counter LSB

; -------------------------------------------------------------------------
; S12ZDBG

DBGC1		equ	$0100		; Debug Control Register 1
ARM		s12zbit	DBGC1,7		;  Arm Bit
TRIG		s12zbit	DBGC1,6		;  Immediate Trigger Request Bit
BDMBP		s12zbit	DBGC1,4		;  Background Debug Mode Enable
BRKCPU		s12zbit	DBGC1,3		;  CPU Breakpoint Enable
EEVE1		s12zbit	DBGC1,1		;  External Event Enable
DBGC2		equ	$0101		; Debug Control Register2
ABCM		s12zfld DBGC2,0,1		;  A and B Comparator Match Control
DBGSCR1		equ	$0107		; Debug State Control Register 1
C3SC1		s12zbit	DBGSCR1,7	;  Channel 3 State Control
C3SC0		s12zbit	DBGSCR1,6
C1SC1		s12zbit	DBGSCR1,3	;  Channel 1 State Control
C1SC0		s12zbit	DBGSCR1,2
C0SC1		s12zbit	DBGSCR1,1	;  Channel 0 State Control
C0SC0		s12zbit	DBGSCR1,0
DBGSCR2		equ	$0108		; Debug State Control Register 2
DBGSCR3		equ	$0109		; Debug State Control Register 3
DBGEFR		equ	$010A		; Debug Event Flag Register
TRIGF		s12zbit	DBGEFR,6	;  TRIG Flag
EEVF		s12zbit	DBGEFR,4	;  External Event Flag
ME3		s12zbit	DBGEFR,3	;  Match Event
ME1		s12zbit	DBGEFR,1
ME0		s12zbit	DBGEFR,0
DBGSR		equ	$010B		; Debug Status Register
SSF2		s12zbit	DBGSR,2		;  State Sequencer Flag Bits
SSF1		s12zbit	DBGSR,1
SSF0		s12zbit	DBGSR,0
DBGACTL		equ	$0110		; Debug Comparator A Control Register
NDB		s12zbit	DBGACTL,6	;  Not Data Bus
INST		s12zbit	DBGACTL,5	;  Instruction Select
RW		s12zbit	DBGACTL,3	;  Read/Write Comparator Value Bit
RWE		s12zbit	DBGACTL,2	;  Read/Write Enable Bit
COMPE		s12zbit	DBGACTL,0	;  Enable Bit
DBGAAH		equ	$0115		; Debug Comparator A Address Register MSB
DBGAAM		equ	$0116
DBGAAL		equ	$0117		; Debug Comparator A Address Register LSB
DBGAD0		equ	$0118		; Debug Comparator A Data Register MSB
DBGAD1		equ	$0119
DBGAD2		equ	$011A
DBGAD3		equ	$011B		; Debug Comparator A Data Register LSB
DBGADM0		equ	$011C		; Debug Comparator A Data Mask Register MSB
DBGADM1		equ	$011D
DBGADM2		equ	$011E
DBGADM3		equ	$011F		; Debug Comparator A Data Mask Register LSB
DBGBCTL		equ	$0120		; Debug Comparator B Control Register (same bits as in DBGACTL)
DBGBAH		equ	$0125		; Debug Comparator B Address Register MSB
DBGBAM		equ	$0126
DBGBAL		equ	$0127		; Debug Comparator B Address Register LSB
DBGDCTL		equ	$0140		; Debug Comparator D Control Register (same bits as in DBGACTL)
DBGDAH		equ	$0145		; Debug Comparator D Address Register MSB
DBGDAM		equ	$0146
DBGDAL		equ	$0147		; Debug Comparator D Address Register LSB

; -------------------------------------------------------------------------
; S12ZVCPIM

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
PTE		equ	$0260		; Port E Data
PTE7		s12zbit	PTE,7
PTE6		s12zbit	PTE,6
PTE5		s12zbit	PTE,5
PTE4		s12zbit	PTE,4
PTE3		s12zbit	PTE,3
PTE2		s12zbit	PTE,2
PTE1		s12zbit	PTE,1
PTE0		s12zbit	PTE,0
PTIE		equ	$0262		; Port E Input Data
PTIE7		s12zbit	PTIE,7
PTIE6		s12zbit	PTIE,6
PTIE5		s12zbit	PTIE,5
PTIE4		s12zbit	PTIE,4
PTIE3		s12zbit	PTIE,3
PTIE2		s12zbit	PTIE,2
PTIE1		s12zbit	PTIE,1
PTIE0		s12zbit	PTIE,0
DDRE		equ	$0264		; Port E Data Direction Register
DDRE7		s12zbit	DDRE,7
DDRE6		s12zbit	DDRE,6
DDRE5		s12zbit	DDRE,5
DDRE4		s12zbit	DDRE,4
DDRE3		s12zbit	DDRE,3
DDRE2		s12zbit	DDRE,2
DDRE1		s12zbit	DDRE,1
DDRE0		s12zbit	DDRE,0
PERE		equ	$0266		; Port E Pull Enable Register
PERE7		s12zbit	PERE,7
PERE6		s12zbit	PERE,6
PERE5		s12zbit	PERE,5
PERE4		s12zbit	PERE,4
PERE3		s12zbit	PERE,3
PERE2		s12zbit	PERE,2
PERE1		s12zbit	PERE,1
PERE0		s12zbit	PERE,0
PPSE		equ	$0268		; Port E Polarity Select Register
PPSE7		s12zbit	PPSE,7
PPSE6		s12zbit	PPSE,6
PPSE5		s12zbit	PPSE,5
PPSE4		s12zbit	PPSE,4
PPSE3		s12zbit	PPSE,3
PPSE2		s12zbit	PPSE,2
PPSE1		s12zbit	PPSE,1
PPSE0		s12zbit	PPSE,0
PTADH		equ	$0280		; Port AD Data MSB
PTAD7		s12zbit	PTADH,7
PTAD6		s12zbit	PTADH,6
PTAD5		s12zbit	PTADH,5
PTAD4		s12zbit	PTADH,4
PTAD3		s12zbit	PTADH,3
PTAD2		s12zbit	PTADH,2
PTAD1		s12zbit	PTADH,1
PTAD0		s12zbit	PTADH,0
PTADL		equ	$0281		; Port AD Data LSB
PTADL7		s12zbit	PTADL,7
PTADL6		s12zbit	PTADL,6
PTADL5		s12zbit	PTADL,5
PTADL4		s12zbit	PTADL,4
PTADL3		s12zbit	PTADL,3
PTADL2		s12zbit	PTADL,2
PTADL1		s12zbit	PTADL,1
PTADL0		s12zbit	PTADL,0
PTIADH		equ	$0282		; Port AD Input Data MSB
PTIADH7		s12zbit	PTIADH,7
PTIADH6		s12zbit	PTIADH,6
PTIADH5		s12zbit	PTIADH,5
PTIADH4		s12zbit	PTIADH,4
PTIADH3		s12zbit	PTIADH,3
PTIADH2		s12zbit	PTIADH,2
PTIADH1		s12zbit	PTIADH,1
PTIADH0		s12zbit	PTIADH,0
PTIADL		equ	$0283		; Port AD Input Data LSB
PTIADL7		s12zbit	PTIADL,7
PTIADL6		s12zbit	PTIADL,6
PTIADL5		s12zbit	PTIADL,5
PTIADL4		s12zbit	PTIADL,4
PTIADL3		s12zbit	PTIADL,3
PTIADL2		s12zbit	PTIADL,2
PTIADL1		s12zbit	PTIADL,1
PTIADL0		s12zbit	PTIADL,0
DDRADH		equ	$0284		; Port AD Data Direction Register MSB
DDRADH7		s12zbit	DDRADH,7
DDRADH6		s12zbit	DDRADH,6
DDRADH5		s12zbit	DDRADH,5
DDRADH4		s12zbit	DDRADH,4
DDRADH3		s12zbit	DDRADH,3
DDRADH2		s12zbit	DDRADH,2
DDRADH1		s12zbit	DDRADH,1
DDRADH0		s12zbit	DDRADH,0
DDRADL		equ	$0285		; Port AD Data Direction Register LSB
DDRADL7		s12zbit	DDRADL,7
DDRADL6		s12zbit	DDRADL,6
DDRADL5		s12zbit	DDRADL,5
DDRADL4		s12zbit	DDRADL,4
DDRADL3		s12zbit	DDRADL,3
DDRADL2		s12zbit	DDRADL,2
DDRADL1		s12zbit	DDRADL,1
DDRADL0		s12zbit	DDRADL,0
PERADH		equ	$0286		; Port AD Pull Enable Register MSB
PERADH7		s12zbit	PERADH,7
PERADH6		s12zbit	PERADH,6
PERADH5		s12zbit	PERADH,5
PERADH4		s12zbit	PERADH,4
PERADH3		s12zbit	PERADH,3
PERADH2		s12zbit	PERADH,2
PERADH1		s12zbit	PERADH,1
PERADH0		s12zbit	PERADH,0
PERADL		equ	$0287		; Port AD Pull Enable Register LSB
PERADL7		s12zbit	PERADL,7
PERADL6		s12zbit	PERADL,6
PERADL5		s12zbit	PERADL,5
PERADL4		s12zbit	PERADL,4
PERADL3		s12zbit	PERADL,3
PERADL2		s12zbit	PERADL,2
PERADL1		s12zbit	PERADL,1
PERADL0		s12zbit	PERADL,0
PPSADH		equ	$0288		; Port AD Polarity Select Register MSB
PPSADH7		s12zbit	PPSADH,7
PPSADH6		s12zbit	PPSADH,6
PPSADH5		s12zbit	PPSADH,5
PPSADH4		s12zbit	PPSADH,4
PPSADH3		s12zbit	PPSADH,3
PPSADH2		s12zbit	PPSADH,2
PPSADH1		s12zbit	PPSADH,1
PPSADH0		s12zbit	PPSADH,0
PPSADL		equ	$0289		; Port AD Polarity Select Register LSB
PPSADL7		s12zbit	PPSADL,7
PPSADL6		s12zbit	PPSADL,6
PPSADL5		s12zbit	PPSADL,5
PPSADL4		s12zbit	PPSADL,4
PPSADL3		s12zbit	PPSADL,3
PPSADL2		s12zbit	PPSADL,2
PPSADL1		s12zbit	PPSADL,1
PPSADL0		s12zbit	PPSADL,0
PIEADH		equ	$028C		; Port AD Interrupt Enable Register MSB
PIEADH7		s12zbit	PIEADH,7
PIEADH6		s12zbit	PIEADH,6
PIEADH5		s12zbit	PIEADH,5
PIEADH4		s12zbit	PIEADH,4
PIEADH3		s12zbit	PIEADH,3
PIEADH2		s12zbit	PIEADH,2
PIEADH1		s12zbit	PIEADH,1
PIEADH0		s12zbit	PIEADH,0
PIEADL		equ	$028D		; Port AD Interrupt Enable Register LSB
PIEADL7		s12zbit	PIEADL,7
PIEADL6		s12zbit	PIEADL,6
PIEADL5		s12zbit	PIEADL,5
PIEADL4		s12zbit	PIEADL,4
PIEADL3		s12zbit	PIEADL,3
PIEADL2		s12zbit	PIEADL,2
PIEADL1		s12zbit	PIEADL,1
PIEADL0		s12zbit	PIEADL,0
PIFADH		equ	$028E		; Port AD Interrupt Flag Register MSB
PIFADH7		s12zbit	PIFADH,7
PIFADH6		s12zbit	PIFADH,6
PIFADH5		s12zbit	PIFADH,5
PIFADH4		s12zbit	PIFADH,4
PIFADH3		s12zbit	PIFADH,3
PIFADH2		s12zbit	PIFADH,2
PIFADH1		s12zbit	PIFADH,1
PIFADH0		s12zbit	PIFADH,0
PIFADL		equ	$028F		; Port AD Interrupt Flag Register LSB
PIFADL7		s12zbit	PIFADL,7
PIFADL6		s12zbit	PIFADL,6
PIFADL5		s12zbit	PIFADL,5
PIFADL4		s12zbit	PIFADL,4
PIFADL3		s12zbit	PIFADL,3
PIFADL2		s12zbit	PIFADL,2
PIFADL1		s12zbit	PIFADL,1
PIFADL0		s12zbit	PIFADL,0
DIENADH		equ	$0298		; Port AD Digital Input Enable Register MSB
DIENADH		s12zbit	DIENADH,7
DIENADH		s12zbit	DIENADH,6
DIENADH		s12zbit	DIENADH,5
DIENADH		s12zbit	DIENADH,4
DIENADH		s12zbit	DIENADH,3
DIENADH		s12zbit	DIENADH,2
DIENADH		s12zbit	DIENADH,1
DIENADH		s12zbit	DIENADH,0
DIENADL		equ	$0299		; Port AD Digital Input Enable Register LSB
DIENADL7	s12zbit	DIENADL,7
DIENADL6	s12zbit	DIENADL,6
DIENADL5	s12zbit	DIENADL,5
DIENADL4	s12zbit	DIENADL,4
DIENADL3	s12zbit	DIENADL,3
DIENADL2	s12zbit	DIENADL,2
DIENADL1	s12zbit	DIENADL,1
DIENADL0	s12zbit	DIENADL,0
PTT		equ	$02C0		; Port T Data
PTT7		s12zbit	PTT,7
PTT6		s12zbit	PTT,6
PTT5		s12zbit	PTT,5
PTT4		s12zbit	PTT,4
PTT3		s12zbit	PTT,3
PTT2		s12zbit	PTT,2
PTT1		s12zbit	PTT,1
PTT0		s12zbit	PTT,0
PTIT		equ	$02C1		; Port T Input Data
PTIT7		s12zbit	PTIT,7
PTIT6		s12zbit	PTIT,6
PTIT5		s12zbit	PTIT,5
PTIT4		s12zbit	PTIT,4
PTIT3		s12zbit	PTIT,3
PTIT2		s12zbit	PTIT,2
PTIT1		s12zbit	PTIT,1
PTIT0		s12zbit	PTIT,0
DDRT		equ	$02C2		; Port T Data Direction Register
DDRT7		s12zbit	DDRT,7
DDRT6		s12zbit	DDRT,6
DDRT5		s12zbit	DDRT,5
DDRT4		s12zbit	DDRT,4
DDRT3		s12zbit	DDRT,3
DDRT2		s12zbit	DDRT,2
DDRT1		s12zbit	DDRT,1
DDRT0		s12zbit	DDRT,0
PERT		equ	$02C3		; Port T Pull Enable Register
PERT7		s12zbit	PERT,7
PERT6		s12zbit	PERT,6
PERT5		s12zbit	PERT,5
PERT4		s12zbit	PERT,4
PERT3		s12zbit	PERT,3
PERT2		s12zbit	PERT,2
PERT1		s12zbit	PERT,1
PERT0		s12zbit	PERT,0
PPST		equ	$02C4		; Port T Polarity Select Register
PPST7		s12zbit	PPST,7
PPST6		s12zbit	PPST,6
PPST5		s12zbit	PPST,5
PPST4		s12zbit	PPST,4
PPST3		s12zbit	PPST,3
PPST2		s12zbit	PPST,2
PPST1		s12zbit	PPST,1
PPST0		s12zbit	PPST,0
PTS		equ	$02D0		; Port S Data
PTS7		s12zbit	PTS,7
PTS6		s12zbit	PTS,6
PTS5		s12zbit	PTS,5
PTS4		s12zbit	PTS,4
PTS3		s12zbit	PTS,3
PTS2		s12zbit	PTS,2
PTS1		s12zbit	PTS,1
PTS0		s12zbit	PTS,0
PTIS		equ	$02D1		; Port S Input Data
PTIS7		s12zbit	PTIS,7
PTIS6		s12zbit	PTIS,6
PTIS5		s12zbit	PTIS,5
PTIS4		s12zbit	PTIS,4
PTIS3		s12zbit	PTIS,3
PTIS2		s12zbit	PTIS,2
PTIS1		s12zbit	PTIS,1
PTIS0		s12zbit	PTIS,0
DDRS		equ	$02D2		; Port S Data Direction Register
DDRS7		s12zbit	DDRS,7
DDRS6		s12zbit	DDRS,6
DDRS5		s12zbit	DDRS,5
DDRS4		s12zbit	DDRS,4
DDRS3		s12zbit	DDRS,3
DDRS2		s12zbit	DDRS,2
DDRS1		s12zbit	DDRS,1
DDRS0		s12zbit	DDRS,0
PERS		equ	$02D3		; Port S Pull Enable Register
PERS7		s12zbit	PERS,7
PERS6		s12zbit	PERS,6
PERS5		s12zbit	PERS,5
PERS4		s12zbit	PERS,4
PERS3		s12zbit	PERS,3
PERS2		s12zbit	PERS,2
PERS1		s12zbit	PERS,1
PERS0		s12zbit	PERS,0
PPSS		equ	$02D4		; Port S Polarity Select Register
PPSS7		s12zbit	PPSS,7
PPSS6		s12zbit	PPSS,6
PPSS5		s12zbit	PPSS,5
PPSS4		s12zbit	PPSS,4
PPSS3		s12zbit	PPSS,3
PPSS2		s12zbit	PPSS,2
PPSS1		s12zbit	PPSS,1
PPSS0		s12zbit	PPSS,0
PIES		equ	$02D6		; Port S Interrupt Enable Register
PIES7		s12zbit	PIES,7
PIES6		s12zbit	PIES,6
PIES5		s12zbit	PIES,5
PIES4		s12zbit	PIES,4
PIES3		s12zbit	PIES,3
PIES2		s12zbit	PIES,2
PIES1		s12zbit	PIES,1
PIES0		s12zbit	PIES,0
PIFS		equ	$02D7		; Port S Interrupt Flag Register
PIFS7		s12zbit	PIFS,7
PIFS6		s12zbit	PIFS,6
PIFS5		s12zbit	PIFS,5
PIFS4		s12zbit	PIFS,4
PIFS3		s12zbit	PIFS,3
PIFS2		s12zbit	PIFS,2
PIFS1		s12zbit	PIFS,1
PIFS0		s12zbit	PIFS,0
WOMS		equ	$02DF		; Port S Wired-OR Mode Register
WOMS7		s12zbit	WOMS,7
WOMS6		s12zbit	WOMS,6
WOMS5		s12zbit	WOMS,5
WOMS4		s12zbit	WOMS,4
WOMS3		s12zbit	WOMS,3
WOMS2		s12zbit	WOMS,2
WOMS1		s12zbit	WOMS,1
WOMS0		s12zbit	WOMS,0
PTP		equ	$02F0		; Port P Data
PTP7		s12zbit	PTP,7
PTP6		s12zbit	PTP,6
PTP5		s12zbit	PTP,5
PTP4		s12zbit	PTP,4
PTP3		s12zbit	PTP,3
PTP2		s12zbit	PTP,2
PTP1		s12zbit	PTP,1
PTP0		s12zbit	PTP,0
PTIP		equ	$02F1		; Port P Input Data
PTIP7		s12zbit	PTIP,7
PTIP6		s12zbit	PTIP,6
PTIP5		s12zbit	PTIP,5
PTIP4		s12zbit	PTIP,4
PTIP3		s12zbit	PTIP,3
PTIP2		s12zbit	PTIP,2
PTIP1		s12zbit	PTIP,1
PTIP0		s12zbit	PTIP,0
DDRP		equ	$02F2		; Port P Data Direction Register
DDRP7		s12zbit	DDRP,7
DDRP6		s12zbit	DDRP,6
DDRP5		s12zbit	DDRP,5
DDRP4		s12zbit	DDRP,4
DDRP3		s12zbit	DDRP,3
DDRP2		s12zbit	DDRP,2
DDRP1		s12zbit	DDRP,1
DDRP0		s12zbit	DDRP,0
PERP		equ	$02F3		; Port P Pull Enable Register
PERP7		s12zbit	PERP,7
PERP6		s12zbit	PERP,6
PERP5		s12zbit	PERP,5
PERP4		s12zbit	PERP,4
PERP3		s12zbit	PERP,3
PERP2		s12zbit	PERP,2
PERP1		s12zbit	PERP,1
PERP0		s12zbit	PERP,0
PPSP		equ	$02F4		; Port P Polarity Select Register
PPSP7		s12zbit	PPSP,7
PPSP6		s12zbit	PPSP,6
PPSP5		s12zbit	PPSP,5
PPSP4		s12zbit	PPSP,4
PPSP3		s12zbit	PPSP,3
PPSP2		s12zbit	PPSP,2
PPSP1		s12zbit	PPSP,1
PPSP0		s12zbit	PPSP,0
PIEP		equ	$02F6		; Port P Interrupt Enable Register
PIEP7		s12zbit	PIEP,7
PIEP6		s12zbit	PIEP,6
PIEP5		s12zbit	PIEP,5
PIEP4		s12zbit	PIEP,4
PIEP3		s12zbit	PIEP,3
PIEP2		s12zbit	PIEP,2
PIEP1		s12zbit	PIEP,1
PIEP0		s12zbit	PIEP,0
PIFP		equ	$02F7		; Port P Interrupt Flag Register
PIFP7		s12zbit	PIFP,7
PIFP6		s12zbit	PIFP,6
PIFP5		s12zbit	PIFP,5
PIFP4		s12zbit	PIFP,4
PIFP3		s12zbit	PIFP,3
PIFP2		s12zbit	PIFP,2
PIFP1		s12zbit	PIFP,1
PIFP0		s12zbit	PIFP,0
OCPEP		equ	$02F9		; Port P Over-Current Protection Enable Register
OCPEP6		s12zbit	OCPEP,6
OCPEP5		s12zbit	OCPEP,5
OCPEP4		s12zbit	OCPEP,4
OCPEP2		s12zbit	OCPEP,2
OCPEP0		s12zbit	OCPEP,0
OCIEP		equ	$02FA		; Port P Over-Current Interrupt Enable Register
OCIEP6		s12zbit	OCIEP,6
OCIEP5		s12zbit	OCIEP,5
OCIEP4		s12zbit	OCIEP,4
OCIEP2		s12zbit	OCIEP,2
OCIEP0		s12zbit	OCIEP,0
OCIFP		equ	$2FB		; Port P Over-Current Interrupt Flag Register
OCIFP6		s12zbit	OCIFP,6
OCIFP5		s12zbit	OCIFP,5
OCIFP4		s12zbit	OCIFP,4
OCIFP2		s12zbit	OCIFP,2
OCIFP0		s12zbit	OCIFP,0
RDRP		equ	$02FD		; Port P Reduced Drive Register
RDRP7		s12zbit	RDRP,7
RDRP6		s12zbit	RDRP,6
RDRP5		s12zbit	RDRP,5
RDRP4		s12zbit	RDRP,4
RDRP3		s12zbit	RDRP,3
RDRP2		s12zbit	RDRP,2
RDRP1		s12zbit	RDRP,1
RDRP0		s12zbit	RDRP,0
PTJ		equ	$0310		; Port J Data
PTJ7		s12zbit	PTJ,7
PTJ6		s12zbit	PTJ,6
PTJ5		s12zbit	PTJ,5
PTJ4		s12zbit	PTJ,4
PTJ3		s12zbit	PTJ,3
PTJ2		s12zbit	PTJ,2
PTJ1		s12zbit	PTJ,1
PTJ0		s12zbit	PTJ,0
PTIJ		equ	$0311		; Port J Input Data
PTIJ7		s12zbit	PTIJ,7
PTIJ6		s12zbit	PTIJ,6
PTIJ5		s12zbit	PTIJ,5
PTIJ4		s12zbit	PTIJ,4
PTIJ3		s12zbit	PTIJ,3
PTIJ2		s12zbit	PTIJ,2
PTIJ1		s12zbit	PTIJ,1
PTIJ0		s12zbit	PTIJ,0
DDRJ		equ	$0312		; Port J Data Direction Register
DDRJ7		s12zbit	DDRJ,7
DDRJ6		s12zbit	DDRJ,6
DDRJ5		s12zbit	DDRJ,5
DDRJ4		s12zbit	DDRJ,4
DDRJ3		s12zbit	DDRJ,3
DDRJ2		s12zbit	DDRJ,2
DDRJ1		s12zbit	DDRJ,1
DDRJ0		s12zbit	DDRJ,0
PERJ		equ	$0313		; Port J Pull Enable Register
PERJ7		s12zbit	PERJ,7
PERJ6		s12zbit	PERJ,6
PERJ5		s12zbit	PERJ,5
PERJ4		s12zbit	PERJ,4
PERJ3		s12zbit	PERJ,3
PERJ2		s12zbit	PERJ,2
PERJ1		s12zbit	PERJ,1
PERJ0		s12zbit	PERJ,0
PPSJ		equ	$0314		; Port J Polarity Select Register
PPSJ7		s12zbit	PPSJ,7
PPSJ6		s12zbit	PPSJ,6
PPSJ5		s12zbit	PPSJ,5
PPSJ4		s12zbit	PPSJ,4
PPSJ3		s12zbit	PPSJ,3
PPSJ2		s12zbit	PPSJ,2
PPSJ1		s12zbit	PPSJ,1
PPSJ0		s12zbit	PPSJ,0
WOMJ		equ	$031F		; Port J Wired-OR Mode Register
WOMJ7		s12zbit	WOMJ,7
WOMJ6		s12zbit	WOMJ,6
WOMJ5		s12zbit	WOMJ,5
WOMJ4		s12zbit	WOMJ,4
WOMJ3		s12zbit	WOMJ,3
WOMJ2		s12zbit	WOMJ,2
WOMJ1		s12zbit	WOMJ,1
WOMJ0		s12zbit	WOMJ,0
PTIL		equ	$0331		; Port L Input Data
PTIL1		s12zbit	PTIL,1
PTIL0		s12zbit	PTIL,0
PTPSL		equ	$0333		; Port L Pull Select Register
PTPSL1		s12zbit	PTPSL,1
PTPSL0		s12zbit	PTPSL,0
PPSL		equ	$0334		; Port L Polarity Select Register
PPSL1		s12zbit	PPSL,1
PPSL0		s12zbit	PPSL,0
PIEL		equ	$0336		; Port L Interrupt Enable Register
PIEL1		s12zbit	PIEL,1
PIEL0		s12zbit	PIEL,0
PIFL		equ	$0337		; Port L Interrupt Flag Register
PIFL1		s12zbit	PIFL,1
PIFL0		s12zbit	PIFL,0
PTABYPL		equ	$033A		; Port L ADC Bypass Register
PTABYPL0	s12zbit	PTABYPL,1
PTABYPL1	s12zbit	PTABYPL,0
PTADIRL		equ	$033B		; Port L ADC Direct Register
PTADIRL1	s12zbit	PTADIRL,1
PTADIRL0	s12zbit	PTADIRL,0
DIENL		equ	$033C		; Port L Digital Input Enable Register
DIENL1		s12zbit	DIENL,1
DIENL0		s12zbit	DIENL,0
PTAENL		equ	$033D		; Port L ADC Connection Enable Register
PTAENL1		s12zbit PTAENL,1
PTAENL0		s12zbit	PTAENL,0
PIRL		equ	$033E		; Port L Input Divider Ratio Selection Register
PIRL1		s12zbit	PIRL,1
PIRL0		s12zbit	PIRL,0
PTTEL		equ	$033F		; Port L Test Enable Register
PTTEL1		s12zbit	PTTEL,1
PTTEL0		s12zbit	PTTEL,0

; -------------------------------------------------------------------------
; FTMRZ192K2K

FCLKDIV		equ	$0380		; Flash Clock Divider Register
FDIVLD		s12zbit	FCLKDIV,7	;  Clock Divider Loaded
FDIVLCK		s12zbit	FCLKDIV,6	;  Clock Divider Locked
FDIV		s12zfld	FCLKDIV,0,5	;  Clock Divider Bits
FSEC		equ	$0381		; Flash Security Register
KEYEN		s12zfld	FSEC,6,7	;  Backdoor Key Security Enable Bits
RNV		s12zfld	FSEC,2,5	;  Reserved Nonvolatile Bits
SEC		s12zfld	FSEC,0,1	;  Flash Security Bits
FCCOBIX		equ	$0382		; Flash CCOB Index Register
CCOBIX		s12zfld	FCCOBIX,0,2	;  Common Command Register Index
FPSTAT		equ	$0383		; Flash Protection Status Register
FPOVRD		s12zbit	FPSTAT,7	;  Flash Protection Override Status
WSTATACK	s12zbit	FPSTAT,0	;  Wait-State Switch Acknowledge
FCNFG		equ	$0384		; Flash Configuration Register
CCIE		s12zbit	FCNFG,7		;  Command Complete Interrupt Enable
ERSAREQ		s12zbit	FCNFG,5		;  Erase All Request
IGNSF		s12zbit	FCNFG,4		;  Ignore Single Bit Fault
WSTAT		s12zfld	FCNFG,2,3	;  Wait State control bits
FDFD		s12zbit	FCNFG,1		;  Force Double Bit Fault Detect
FSFD		s12zbit	FCNFG,0		;  Force Single Bit Fault Detect
FERCNFG		equ	$0385		; Flash Error Configuration Register
SFDIE		s12zbit	FERCNFG,0	;  Single Bit Fault Detect Interrupt Enable
FSTAT		equ	$0386		; Flash Status Register
CCIF		s12zbit	FSTAT,7		;  Command Complete Interrupt Flag
ACCERR		s12zbit	FSTAT,5		;  Flash Access Error Flag
FPVIOL		s12zbit	FSTAT,6		;  Flash Protection Violation Flag
MGBUSY		s12zbit	FSTAT,3		;  Memory Controller Busy Flag
MGSTAT		s12zfld	FSTAT,0,1	;  Memory Controller Command Completion Status Flag
FERSTAT		equ	$0387		; Flash Error Status Register
DFDF		s12zbit	FERSTAT,1	;  Double Bit Fault Detect Flag
SFDIF		s12zbit	FERSTAT,0	;  Single Bit Fault Detect Interrupt Flag
FPROT		equ	$0388		; P-Flash Protection Register
FPOPEN		s12zbit	FPROT,7		;  Flash Protection Operation Enable
RNV6		s12zbit	FPROT,6		;  Reserved Nonvolatile Bit
FPHDIS		s12zbit	FPROT,5		;  Flash Protection Higher Address Range Disable
FPHS		s12zfld	FPROT,3,4	;  Flash Protection Higher Address Size
FPLDIS		s12zbit	FPROT,2		;  Flash Protection Lower Address Range Disable
FPLS		s12zfld	FPROT,0,1	;  Flash Protection Lower Address Size
DFPROT		equ	$0389		; EEPROM Protection Register
DPOPEN		s12zbit	DFPROT,7	;  EEPROM Protection Control
DPS		s12zfld	DFPROT,0,5	;  EEPROM Protection Size
FOPT		equ	$038A		; Flash Option Register
NV		s12zfld	FOPT,0,7	;  Nonvolatile Bits
FRSV1		equ	$038B		; Flash Reserved1 Register
FCCOB0HI	equ	$038C		; Flash Common Command Object Register 0 MSB
FCCOB0LO	equ	$038D           ; Flash Common Command Object Register 0 LSB
FCCOB1HI	equ	$038E           ; Flash Common Command Object Register 1 MSB
FCCOB1LO	equ	$038F           ; Flash Common Command Object Register 1 LSB
FCCOB2HI	equ	$0390           ; Flash Common Command Object Register 2 MSB
FCCOB2LO	equ	$0391           ; Flash Common Command Object Register 2 LSB
FCCOB3HI	equ	$0392           ; Flash Common Command Object Register 3 MSB
FCCOB3LO	equ	$0393           ; Flash Common Command Object Register 3 LSB
FCCOB4HI	equ	$0394           ; Flash Common Command Object Register 4 MSB
FCCOB4LO	equ	$0395           ; Flash Common Command Object Register 4 LSB
FCCOB5HI	equ	$0396           ; Flash Common Command Object Register 5 MSB
FCCOB5LO	equ	$0397           ; Flash Common Command Object Register 5 LSB

; -------------------------------------------------------------------------
; SRAM_ECC_32D7P

ECCSTAT		equ	$03C0		; ECC Status Register
RDY		s12zbit	ECCSTAT,0	;  ECC Ready
ECCIE		equ	$03C1		; ECC Interrupt Enable Register
SBEEIE		s12zbit	ECCIE,0		;  Single bit ECC Error Interrupt Enable
ECCIF		equ	$03C2		; ECC Interrupt Flag Register
SBEEIF		s12zbit	ECCIF,0		;  Single bit ECC Error Interrupt Flag
ECCDPTRH	equ	$03C7		; ECC Debug Pointer Register MSB
ECCDPTRM	equ	$03C8
ECCDPTRL	equ	$03C9		; ECC Debug Pointer Register LSB
ECCDDH		equ	$03CC		; ECC Debug Data MSB
ECCDDL		equ	$03CD		; ECC Debug Data LSB
ECCDE		equ	$03CE		; ECC Debug ECC
DECC		s12zfld	ECCDE,0,5	;  ECC Debug ECC
ECCDCMD		equ	$03CF		; ECC Debug Command
ECCDRR		s12zbit	ECCDCMD,7	;  ECC Disable Read Repair Function
ECCDW		s12zbit	ECCDCMD,1	;  ECC Debug Write Command
ECCDR		s12zbit	ECCDCMD,0	;  ECC Debug Read Command

; -------------------------------------------------------------------------
; TIM1

TIM1TIOS	equ	$0400		; Timer 1 Input Capture/Output Compare Select
IOS7		s12zbit	TIM1TIOS,7	;  Timer 1 Channel n Input Capture or Output Compare Channel Configuration
IOS6		s12zbit	TIM1TIOS,6
IOS5		s12zbit	TIM1TIOS,5
IOS4		s12zbit	TIM1TIOS,4
IOS3		s12zbit	TIM1TIOS,3
IOS2		s12zbit	TIM1TIOS,2
IOS1		s12zbit	TIM1TIOS,1
IOS0		s12zbit	TIM1TIOS,0
TIM1CFORC	equ	$0401		; Timer 1 Compare Force Register
FOC7		s12zbit	TIM1CFORC,7	;  Timer 1 Channel n Force Output Compare Action
FOC6		s12zbit	TIM1CFORC,6
FOC5		s12zbit	TIM1CFORC,5
FOC4		s12zbit	TIM1CFORC,4
FOC3		s12zbit	TIM1CFORC,3
FOC2		s12zbit	TIM1CFORC,2
FOC1		s12zbit	TIM1CFORC,1
FOC0		s12zbit	TIM1CFORC,0
TIM1OC7M	equ	$0402		; Timer 1 Output Compare 7 Mask Register
OC7M		s12zfld	TIM1OC7M,0,7	;  Timer 1 Output Compare 7 Mask
TIM1OC7D	equ	$0403		; Timer 1 Output Compare 7 Data Register
OC7D		s12zfld	TIM1OC7D,0,7	;  Timer 1 Output Compare 7 Data
TIM1TCNTH	equ	$0404		; Timer 1 Count Register MSB
TIM1TCNTL	equ	$0405		; Timer 1 Count Register LSB
TIM1TSCR1	equ	$0406		; Timer 1 System Control Register 1
TEN		s12zbit	TIM1TSCR1,7	;  Timer Enable
TSWAI		s12zbit	TIM1TSCR1,6	;  Timer Module Stops While in Wait
TSFRZ		s12zbit	TIM1TSCR1,5	;  Timer Stops While in Freeze Mode
TFFCA		s12zbit	TIM1TSCR1,4	;  Timer Fast Flag Clear All
PRNT		s12zbit	TIM1TSCR1,3	;  Precision Timer
TIM1TTOV	equ	$0047		; Timer 1 Toggle On Overflow Register 1
TOV7		s12zbit	TIM1TTOV,7	;  Toggle On Overflow Bits
TOV6		s12zbit	TIM1TTOV,6
TOV5		s12zbit	TIM1TTOV,5
TOV4		s12zbit	TIM1TTOV,4
TOV3		s12zbit	TIM1TTOV,3
TOV2		s12zbit	TIM1TTOV,2
TOV1		s12zbit	TIM1TTOV,1
TOV0		s12zbit	TIM1TTOV,0
TIM1TCTL1	equ	$0408		; Timer 1 Control Register 1
OM7		s12zbit	TIM1TCTL1,7	;  Output Mode n
OL7		s12zbit	TIM1TCTL1,6	;  Output Level n
OM6		s12zbit	TIM1TCTL1,5
OL6		s12zbit	TIM1TCTL1,4
OM5		s12zbit	TIM1TCTL1,3
OL5		s12zbit	TIM1TCTL1,2
OM4		s12zbit	TIM1TCTL1,1
OL4		s12zbit	TIM1TCTL1,0
TIM1TCTL2	equ	$0409		; Timer 1 Control Register 2
OM3		s12zbit	TIM1TCTL2,7
OL3		s12zbit	TIM1TCTL2,6
OM2		s12zbit	TIM1TCTL2,5
OL2		s12zbit	TIM1TCTL2,4
OM1		s12zbit	TIM1TCTL2,3
OL1		s12zbit	TIM1TCTL2,2
OM0		s12zbit	TIM1TCTL2,1
OL0		s12zbit	TIM1TCTL2,0
TIM1TCTL3	equ	$040A		; Timer 1 Control Register 3
EDG7B		s12zbit	TIM1TCTL3,7	;  Input Capture Edge Control n
EDG7A		s12zbit	TIM1TCTL3,6
EDG6B		s12zbit	TIM1TCTL3,5
EDG6A		s12zbit	TIM1TCTL3,4
EDG5B		s12zbit	TIM1TCTL3,3
EDG5A		s12zbit	TIM1TCTL3,2
EDG4B		s12zbit	TIM1TCTL3,1
EDG4A		s12zbit	TIM1TCTL3,0
TIM1TCTL4	equ	$040B		; Timer 1 Control Register 4
EDG3B		s12zbit	TIM1TCTL4,7
EDG3A		s12zbit	TIM1TCTL4,6
EDG2B		s12zbit	TIM1TCTL4,5
EDG2A		s12zbit	TIM1TCTL4,4
EDG1B		s12zbit	TIM1TCTL4,3
EDG1A		s12zbit	TIM1TCTL4,2
EDG0B		s12zbit	TIM1TCTL4,1
EDG0A		s12zbit	TIM1TCTL4,0
TIM1TIE		equ	$040C		; Timer 1 Interrupt Enable Register
C7I		s12zbit	TIM1TIE,7	;  Input Capture/Output Compare n Interrupt Enable
C6I		s12zbit	TIM1TIE,6
C5I		s12zbit	TIM1TIE,5
C4I		s12zbit	TIM1TIE,4
C3I		s12zbit	TIM1TIE,3
C2I		s12zbit	TIM1TIE,2
C1I		s12zbit	TIM1TIE,1
C0I		s12zbit	TIM1TIE,0
TIM1TSCR2	equ	$040D		; Timer 1 System Control Register 2
TOI		s12zbit	TIM1TSCR2,7	;  Timer Overflow Interrupt Enable
TCRE		s12zbit	TIM1TSCR2,3	;  Timer Counter Reset Enable
PR		s12zfld	TIM1TSCR2,0,2	;  Timer Prescaler Select
TIM1TFLG1	equ	$040E		; Timer 1 Main Interrupt Flag 1
C7F		s12zbit	TIM1TFLG1,7	;  Input Capture/Output Compare Channel n Flag
C6F		s12zbit	TIM1TFLG1,6
C5F		s12zbit	TIM1TFLG1,5
C4F		s12zbit	TIM1TFLG1,4
C3F		s12zbit	TIM1TFLG1,3
C2F		s12zbit	TIM1TFLG1,2
C1F		s12zbit	TIM1TFLG1,1
C0F		s12zbit	TIM1TFLG1,0
TIM1TFLG2	equ	$040F		; Timer 1 Main Interrupt Flag  2
TOF		s12zbit	TIM1TFLG2,7	;  Timer Overflow Flag
TIM1TC0H	equ	$0410		; Timer 1 Input Capture/Output Compare Register 0 MSB
TIM1TC0L	equ	$0411		; Timer 1 Input Capture/Output Compare Register 0 LSB
TIM1TC1H	equ	$0412		; Timer 1 Input Capture/Output Compare Register 1 MSB
TIM1TC1L	equ	$0413		; Timer 1 Input Capture/Output Compare Register 1 LSB
TIM1TC2H	equ	$0414		; Timer 1 Input Capture/Output Compare Register 2 MSB
TIM1TC2L	equ	$0415		; Timer 1 Input Capture/Output Compare Register 2 LSB
TIM1TC3H	equ	$0416		; Timer 1 Input Capture/Output Compare Register 3 MSB
TIM1TC3L	equ	$0417		; Timer 1 Input Capture/Output Compare Register 3 LSB
TIM1TC4H	equ	$0418		; Timer 1 Input Capture/Output Compare Register 4 MSB
TIM1TC4L	equ	$0419		; Timer 1 Input Capture/Output Compare Register 4 LSB
TIM1TC5H	equ	$041a		; Timer 1 Input Capture/Output Compare Register 5 MSB
TIM1TC5L	equ	$041b		; Timer 1 Input Capture/Output Compare Register 5 LSB
TIM1TC6H	equ	$041c		; Timer 1 Input Capture/Output Compare Register 6 MSB
TIM1TC6L	equ	$041d		; Timer 1 Input Capture/Output Compare Register 6 LSB
TIM1TC7H	equ	$041e		; Timer 1 Input Capture/Output Compare Register 7 MSB
TIM1TC7L	equ	$041f		; Timer 1 Input Capture/Output Compare Register 8 LSB
TIM1PACTL	equ	$0420		; Timer 1 16-Bit Pulse Accumulator Control Register
PAEN		s12zbit	TIM1PACTL,6	;  Pulse Accumulator System Enable
PAMOD		s12zbit	TIM1PACTL,5	;  Pulse Accumulator Mode
PEDGE		s12zbit	TIM1PACTL,4	;  Pulse Accumulator Edge Control
CLK		s12zfld	TIM1PACTL,2,3	;  Clock Select Bits
PAOVI		s12zbit	TIM1PACTL,1	;  Pulse Accumulator Overflow Interrupt Enable
PAI		s12zbit	TIM1PACTL,0	;  Pulse Accumulator Input Interrupt Enable
TIM1PAFLG	equ	$0421		; Timer 1 Pulse Accumulator Flag Register
PAOVF		s12zbit	TIM1PAFLG,1	;  Pulse Accumulator Overflow Flag
PAIF		s12zbit	TIM1PAFLG,0	;  Pulse Accumulator Input edge Flag
TIM1PACNTH	equ	$0422		; Timer 1 Pulse Accumulators Count Register MSB
TIM1PACNTL	equ	$0423
TIM1OCPD	equ	$042C		; Timer 1 Output Compare Pin Disconnect Register
OCPD7		s12zbit	TIM1OCPD,7	;  Output Compare Pin Disconnect n
OCPD6		s12zbit	TIM1OCPD,6
OCPD5		s12zbit	TIM1OCPD,5
OCPD4		s12zbit	TIM1OCPD,4
OCPD3		s12zbit	TIM1OCPD,3
OCPD2		s12zbit	TIM1OCPD,2
OCPD1		s12zbit	TIM1OCPD,1
OCPD0		s12zbit	TIM1OCPD,0
TIM1PTPSR	equ	$042E		; Timer 1 Precision Timer Prescaler Select Register
PTPS		s12zfld	TIM1PTPSR,0,7	;  Precision Timer Prescaler Select

; -------------------------------------------------------------------------
; PWM0

PWM0E		equ	$0480		; PWM0 Enable Register
PWME7		s12zbit	PWM0E,7		;  Enable Pulse Width Channel n
PWME6		s12zbit	PWM0E,6
PWME5		s12zbit	PWM0E,5
PWME4		s12zbit	PWM0E,4
PWME3		s12zbit	PWM0E,3
PWME2		s12zbit	PWM0E,2
PWME1		s12zbit	PWM0E,1
PWME0		s12zbit	PWM0E,0
PWM0POL		equ	$0481		; PWM Polarity Register
PPOL7		s12zbit	PWM0POL,7	;  Pulse Width Channel 7-0 n
PPOL6		s12zbit	PWM0POL,6
PPOL5		s12zbit	PWM0POL,5
PPOL4		s12zbit	PWM0POL,4
PPOL3		s12zbit	PWM0POL,3
PPOL2		s12zbit	PWM0POL,2
PPOL1		s12zbit	PWM0POL,1
PPOL0		s12zbit	PWM0POL,0
PWM0CLK		equ	$0482		; PWM Clock Select Register
PCLK7		s12zbit	PWM0CLK,7	;  Pulse Width Channel n Clock Select
PCLK6		s12zbit	PWM0CLK,6
PCLK5		s12zbit	PWM0CLK,5
PCLK4		s12zbit	PWM0CLK,4
PCLK3		s12zbit	PWM0CLK,3
PCLK2		s12zbit	PWM0CLK,2
PCLK1		s12zbit	PWM0CLK,1
PCLK0		s12zbit	PWM0CLK,0
PWM0PRCLK	equ	$0483		; PWM Prescale Clock Select Register
PCKB		s12zfld	PWM0PRCLK,4,6	;  Prescaler Select for Clock B
PCKA		s12zfld	PWM0PRCLK,0,2	;  Prescaler Select for Clock A
PWM0CAE		equ	$0484		; PWM Center Align Enable Register
CAE7		s12zbit	PWM0CAE,7	;  Center Aligned Output Modes on Channel n
CAE6		s12zbit	PWM0CAE,6
CAE5		s12zbit	PWM0CAE,5
CAE4		s12zbit	PWM0CAE,4
CAE3		s12zbit	PWM0CAE,3
CAE2		s12zbit	PWM0CAE,2
CAE1		s12zbit	PWM0CAE,1
CAE0		s12zbit	PWM0CAE,0
PWM0CTL		equ	$0485		; PWM Control Register
CON67		s12zbit	PWM0CTL,7	;  Concatenate Channels 6 and 7
CON45		s12zbit	PWM0CTL,6	;  Concatenate Channels 4 and 5
CON23		s12zbit	PWM0CTL,5	;  Concatenate Channels 2 and 3
CON01		s12zbit	PWM0CTL,4	;  Concatenate Channels 0 and 1
PSWAI		s12zbit	PWM0CTL,3	;  PWM Stops in Wait Mode
PFRZ		s12zbit	PWM0CTL,2	;  PWM Counters Stop in Freeze Mode
PWM0CLKAB	equ	$0486		; PWM Clock A/B Select Register
PCLKAB7		s12zbit	PWM0CLKAB,7	;  Pulse Width Channel n Clock A/B Select
PCLKAB6		s12zbit	PWM0CLKAB,6
PCLKAB5		s12zbit	PWM0CLKAB,5
PCLKAB4		s12zbit	PWM0CLKAB,4
PCLKAB3		s12zbit	PWM0CLKAB,3
PCLKAB2		s12zbit	PWM0CLKAB,2
PCLKAB1		s12zbit	PWM0CLKAB,1
PCLKAB0		s12zbit	PWM0CLKAB,0
PWM0SCLA	equ	$0488		; PWM Scale A Register
PWM0SCLB	equ	$0489		; PWM Scale B Register
PWM0CNT0	equ	$048C		; PWM Channel 0 Counter Register
PWM0CNT1	equ	$048D		; PWM Channel 1 Counter Register
PWM0CNT2	equ	$048E		; PWM Channel 2 Counter Register
PWM0CNT3	equ	$048F		; PWM Channel 3 Counter Register
PWM0CNT4	equ	$0490		; PWM Channel 4 Counter Register
PWM0CNT5	equ	$0491		; PWM Channel 5 Counter Register
PWM0CNT6	equ	$0492		; PWM Channel 6 Counter Register
PWM0CNT7	equ	$0493		; PWM Channel 7 Counter Register
PWM0PER0	equ	$0494		; PWM Channel 0 Period Register
PWM0PER1	equ	$0495		; PWM Channel 1 Period Register
PWM0PER2	equ	$0496		; PWM Channel 2 Period Register
PWM0PER3	equ	$0497		; PWM Channel 3 Period Register
PWM0PER4	equ	$0498		; PWM Channel 4 Period Register
PWM0PER5	equ	$0499		; PWM Channel 5 Period Register
PWM0PER6	equ	$049A		; PWM Channel 6 Period Register
PWM0PER7	equ	$049B		; PWM Channel 7 Period Register
PWM0DTY0	equ	$049C		; PWM Channel 0 Duty Register
PWM0DTY1	equ	$049D		; PWM Channel 1 Duty Register
PWM0DTY2	equ	$049E		; PWM Channel 2 Duty Register
PWM0DTY3	equ	$049F		; PWM Channel 3 Duty Register
PWM0DTY4	equ	$04A0		; PWM Channel 4 Duty Register
PWM0DTY5	equ	$04A1		; PWM Channel 5 Duty Register
PWM0DTY6	equ	$04A2		; PWM Channel 6 Duty Register
PWM0DTY7	equ	$04A3		; PWM Channel 7 Duty Register

; -------------------------------------------------------------------------
; PWM1

PWM1E		equ	$0500
PWM1POL		equ	$0501
PWM1CLK		equ	$0502
PWM1PRCLK	equ	$0503
PWM1CAE		equ	$0504
PWM1CTL		equ	$0505
PWM1CLKAB	equ	$0506
PWM1SCLA	equ	$0508
PWM1SCLB	equ	$0509
PWM1CNT0	equ	$050C
PWM1CNT1	equ	$050D
PWM1CNT2	equ	$050E
PWM1CNT3	equ	$050F
PWM1CNT4	equ	$0510
PWM1CNT5	equ	$0511
PWM1CNT6	equ	$0512
PWM1CNT7	equ	$0513
PWM1PER0	equ	$0514
PWM1PER1	equ	$0515
PWM1PER2	equ	$0516
PWM1PER3	equ	$0517
PWM1PER4	equ	$0518
PWM1PER5	equ	$0519
PWM1PER6	equ	$051A
PWM1PER7	equ	$051B
PWM1DTY0	equ	$051C
PWM1DTY1	equ	$051D
PWM1DTY2	equ	$051E
PWM1DTY3	equ	$051F
PWM1DTY4	equ	$0520
PWM1DTY5	equ	$0521
PWM1DTY6	equ	$0522
PWM1DTY7	equ	$0523

; -------------------------------------------------------------------------
; TIM0

TIM0TIOS	equ	$05C0
TIM0CFORC       equ	$05C1
TIM0TCNTH       equ	$05C4
TIM0TCNTL       equ	$05C5
TIM0TSCR1       equ	$05C6
TIM0TTOV        equ	$05C7
TIM0TCTL1       equ	$05C8
TIM0TCTL2       equ	$05C9
TIM0TCTL3       equ	$05CA
TIM0TCTL4       equ	$05CB
TIM0TIE         equ	$05CC
TIM0TSCR2       equ	$05CD
TIM0TFLG1       equ	$05CE
TIM0TFLG2       equ	$05CF
TIM0TC0H        equ	$05D0
TIM0TC0L        equ	$05D1
TIM0TC1H        equ	$05D2
TIM0TC1L        equ	$05D3
TIM0TC2H        equ	$05D4
TIM0TC2L        equ	$05D5
TIM0TC3H        equ	$05D6
TIM0TC3L        equ	$05D7
TIM0OCPD        equ	$05EC
TIM0PTPSR       equ	$05EE

; -------------------------------------------------------------------------
; ADC0

ADC0CTL_0	equ	0x0600		; ADC Control Register 0
ADC_EN		s12zbit	ADC0CTL_0,7	;  ADC Enable Bit
ADC_SR		s12zbit	ADC0CTL_0,6	;  ADC Soft-Reset
FRZ_MOD		s12zbit	ADC0CTL_0,5	;  Freeze Mode Configuration
SWAI		s12zbit	ADC0CTL_0,4	;  Wait Mode Configuration
ACC_CFG		s12zfld	ADC0CTL_0,2,3	;  ADCFLWCTL Register Access Configuration
STR_SEQA	s12zbit	ADC0CTL_0,1	;  Control Of Conversion Result Storage and RSTAR_EIF flag setting at Sequence Abort or Restart Event
MOD_CFG		s12zbit	ADC0CTL_0,0	;  (Conversion Flow Control) Mode Configuration
ADC0CTL_1	equ	0x0601		; ADC Control Register 1
CSL_BMOD	s12zbit	ADC0CTL_1,7	;  CSL Buffer Mode Select Bit
RVL_BMOD	s12zbit	ADC0CTL_1,6	;  RVL Buffer Mode Select Bit
SMOD_ACC	s12zbit	ADC0CTL_1,5	;  Special Mode Access Control Bit
AUT_RSTA	s12zbit	ADC0CTL_1,4	;  Automatic Restart Event after exit from MCU Stop and Wait Mode (SWAI set)
ADC0STS		equ	0x0602		; ADC Status Register
CSL_SEL		s12zbit	ADC0STS,7	;  Command Sequence List Select bit
RVL_SEL		s12zbit	ADC0STS,6	;  Result Value List Select Bit
DBECC_ERR	s12zbit	ADC0STS,5	;  Double Bit ECC Error Flag
READY		s12zbit	ADC0STS,3	;  Ready For Restart Event Flag
ADC0TIM		equ	0x0603		; ADC Timing Register
PRS		s12zfld	ADC0TIM,0,6	;  ADC Clock Prescale
ADC0FMT		equ	0x0604		; ADC Format Register
DJM		s12zbit	ADC0FMT,7	;  Result Register Data Justification
SRES		s12zfld	ADC0FMT,0,2	;  ADC Resolution Select
ADC0FLWCTL	equ	0x0605		; ADC Conversion Flow Control Register
SEQA		s12zbit	ADC0FLWCTL,7	;  Conversion Sequence Abort Event
TRIG		s12zbit	ADC0FLWCTL,6	;  Conversion Sequence Trigger Bit
RSTA		s12zbit	ADC0FLWCTL,5	;  Restart Event (Restart from Top of Command Sequence List)
LDOK		s12zbit	ADC0FLWCTL,4	;  Load OK for alternative Command Sequence List
ADC0EIE		equ	0x0606		; ADC Error Interrupt Enable Register
IA_EIE		s12zbit	ADC0EIE,7	;  Illegal Access Error Interrupt Enable Bit
CMD_EIE		s12zbit	ADC0EIE,6	;  Command Value Error Interrupt Enable Bit
EOL_EIE		s12zbit	ADC0EIE,5	;  "End Of List" Error Interrupt Enable Bit
TRIG_EIE	s12zbit	ADC0EIE,3	;  Conversion Sequence Trigger Error Interrupt Enable Bit
RSTAR_EIE	s12zbit	ADC0EIE,2	;  Restart Request Error Interrupt Enable Bit
LDOK_EIE	s12zbit	ADC0EIE,1	;  Load OK Error Interrupt Enable Bit
ADC0IE		equ	0x0607		; ADC Interrupt Enable Register
SEQAD_IE	s12zbit	ADC0IE,7	;  Conversion Sequence Abort Done Interrupt Enable Bit
CONIF_OIE	s12zbit	ADC0IE,6	;  ADCCONIF Register Flags Overrun Interrupt Enable
ADC0EIF		equ	0x0608		; ADC Error Interrupt Flag Register
IA_EIF		s12zbit	ADC0EIF,7	;  Illegal Access Error Interrupt Flag
CMD_EIF		s12zbit	ADC0EIF,6	;  Command Value Error Interrupt Flag
EOL_EIF		s12zbit	ADC0EIF,5	;  "End Of List" Error Interrupt Flag -
TRIG_EIF	s12zbit	ADC0EIF,3	;  Trigger Error Interrupt Flag
RSTAR_EIF	s12zbit	ADC0EIF,2	;  Restart Request Error Interrupt Flag
LDOK_EIF	s12zbit	ADC0EIF,1	;  Load OK Error Interrupt Flag
ADC0IF		equ	0x0609		; ADC Interrupt Flag Register
SEQAD_IF	s12zbit	ADC0IF,7	;  Conversion Sequence Abort Done Interrupt Flag
CONIF_OIF	s12zbit	ADC0IF,6	;  ADCCONIF Register Flags Overrun Interrupt Flag
ADC0CONIE_0	equ	0x060A		; ADC Conversion Interrupt Enable Register MSB
CON_IE15	s12zbit	ADC0CONIE_0,7	;  Conversion Interrupt Enable Bits
CON_IE14	s12zbit	ADC0CONIE_0,6
CON_IE13	s12zbit	ADC0CONIE_0,5
CON_IE12	s12zbit	ADC0CONIE_0,4
CON_IE11	s12zbit	ADC0CONIE_0,3
CON_IE10	s12zbit	ADC0CONIE_0,2
CON_IE9		s12zbit	ADC0CONIE_0,1
CON_IE8		s12zbit	ADC0CONIE_0,0
ADC0CONIE_1	equ	0x060B		; ADC Conversion Interrupt Enable Register LSB
CON_IE7		s12zbit	ADC0CONIE_1,7
CON_IE6		s12zbit	ADC0CONIE_1,6
CON_IE5		s12zbit	ADC0CONIE_1,5
CON_IE4		s12zbit	ADC0CONIE_1,4
CON_IE3		s12zbit	ADC0CONIE_1,3
CON_IE2		s12zbit	ADC0CONIE_1,2
CON_IE1		s12zbit	ADC0CONIE_1,1
EOL_IE		s12zbit	ADC0CONIE_1,0	;  End Of List Interrupt Enable Bit
ADC0CONIF_0	equ	0x060C		; ADC Conversion Interrupt Flag Register MSB
CON_IF15	s12zbit	ADC0CONIF_0,7	;  Conversion Interrupt Flags
CON_IF15	s12zbit	ADC0CONIF_0,6
CON_IF15	s12zbit	ADC0CONIF_0,5
CON_IF15	s12zbit	ADC0CONIF_0,4
CON_IF15	s12zbit	ADC0CONIF_0,3
CON_IF15	s12zbit	ADC0CONIF_0,2
CON_IF15	s12zbit	ADC0CONIF_0,1
CON_IF15	s12zbit	ADC0CONIF_0,0
ADC0CONIF_1	equ	0x060D		; ADC Conversion Interrupt Flag Register LSB
CON_IF15	s12zbit	ADC0CONIF_1,7
CON_IF15	s12zbit	ADC0CONIF_1,6
CON_IF15	s12zbit	ADC0CONIF_1,5
CON_IF15	s12zbit	ADC0CONIF_1,4
CON_IF15	s12zbit	ADC0CONIF_1,3
CON_IF15	s12zbit	ADC0CONIF_1,2
CON_IF15	s12zbit	ADC0CONIF_1,1
EOL_IF		s12zbit	ADC0CONIF_1,0	;  End Of List Interrupt Flag
ADC0IMDRI_0	equ	0x060E		; ADC Intermediate Result Information Register MSB
CSL_IMD		s12zbit	ADC0IMDRI_0,7	;  Active CSL At Intermediate Event
RVL_IMD		s12zbit	ADC0IMDRI_0,6	;  Active RVL At Intermediate Event
ADC0IMDRI_1	equ	0x060F		; ADC Intermediate Result Information Register LSB
RIDX_IMD5	s12zbit	ADC0IMDRI_1,5	;  RES_IDX Value At Intermediate Event
RIDX_IMD4	s12zbit	ADC0IMDRI_1,4
RIDX_IMD3	s12zbit	ADC0IMDRI_1,3
RIDX_IMD2	s12zbit	ADC0IMDRI_1,2
RIDX_IMD1	s12zbit	ADC0IMDRI_1,1
RIDX_IMD0	s12zbit	ADC0IMDRI_1,0
ADC0EOLRI	equ	0x0610		; ADC End Of List Result Information Register
CSL_EOL		s12zbit	ADC0EOLRI,7	;  Active CSL When "End Of List" Command Type Executed
RVL_EOL		s12zbit	ADC0EOLRI,6	;  Active RVL When "End Of List" Command Type Executed
ADC0CMD_0	equ	0x0614		; ADC Command Register 0
CMD_SEL		s12zfld	ADC0CMD_0,6,7	;  Conversion Command Select Bits
INTFLG_SEL	s12zfld	ADC0CMD_0,0,3	;  Conversion Interrupt Flag Select Bits
ADC0CMD_1	equ	0x0615		; ADC Command Register 1
VRH_SEL		s12zbit	ADC0CMD_1,7	;  Reference High Voltage Select Bit
VRL_SEL		s12zbit	ADC0CMD_1,6	;  Reference Low Voltage Select Bit
CH_SEL		s12zfld	ADC0CMD_1,0,5	;  ADC Input Channel Select Bits
ADC0CMD_2	equ	0x0616		; ADC Command Register 2
SMP		s12zfld	ADC0CMD_2,3,7	;  Sample Time Select Bits
ADC0CMD_3	equ	0x0617		; ADC Command Register 3
ADC0CIDX	equ	0x061C		; ADC Command Index Register
CMD_IDX		s12zfld	ADC0CIDX,0,5	;  ADC Command Index Bits
ADC0CBP_0	equ	0x061D		; ADC Command Base Pointer Register MSB
ADC0CBP_1	equ	0x061E
ADC0CBP_2	equ	0x061F		; ADC Command Base Pointer Register LSB
ADC0RIDX	equ	0x0620		; ADC Result Index Register
RES_IDX		s12zfld	ADC0RIDX	;  ADC0RIDX,0,5
ADC0RBP_0	equ	0x0621		; ADC Result Base Pointer Register MSB
ADC0RBP_1	equ	0x0622
ADC0RBP_2	equ	0x0623		; ADC Result Base Pointer Register LSB
ADC0CROFF0	equ	0x0624		; ADC Command and Result Offset Register 0
CMDRES_OFF0	s12zfld	ADC0CROFF0,0,6	;  ADC Command and Result Offset Value 0
ADC0CROFF1	equ	0x0625		; ADC Command and Result Offset Register 1
CMDRES_OFF1	s12zfld	ADC0CROFF1,0,6	;  ADC Command and Result Offset Value 1

; -------------------------------------------------------------------------
; DAC8B5V

DACCTL		equ	$0680		; Control Register
FVR		s12zbit	DACCTL,7	;  Full Voltage Range
DRIVE		s12zbit	DACCTL,6	;  Drive Select
DACM		s12zfld	DACCTL,0,2	;  Mode Select
DACVOL		equ	$0682		; Analog Output Voltage Level Register
VOLTAGE		s12zfld	DACVOL,0,7	;  analog output voltage
DACDEBUG	equ	$0687		; reserved

; -------------------------------------------------------------------------
; ACMP0

ACM0PC0		equ	$0690		; ACMP Control Register 0
ACE		s12zbit	ACM0PC0,7	;  ACMP Enable
ACOPE		s12zbit	ACM0PC0,6	;  ACMP Output Pin Enable
ACOPS		s12zbit	ACM0PC0,5	;  ACMP Output Polarity Select
ACDLY		s12zbit	ACM0PC0,4	;  ACMP Input Filter Select for Inputs ACMP_0 and ACMP_1
ACHYS		s12zfld	ACM0PC0,2,3	;  ACMP Hysteresis
ACMOD		s12zfld	ACM0PC0,0,1	;  ACMP Mode
ACM0PC1		equ	$0691		; ACMP Control Register 1
ACPSEL		s12zfld	ACM0PC1,4,5	;  ACMP Positive Input Select
ACNSEL		s12zfld	ACM0PC1,0,1	;  ACMP Negative Input Select
ACM0PC2		equ	$0692		; ACMP Control Register 2
ACIE		s12zbit	ACM0PC2,0	;  ACMP Interrupt Enable
ACM0PS		equ	$0693		; ACMP Status Register
ACIF		s12zbit	ACM0PS,0	;  ACMP Interrupt Flag

; -------------------------------------------------------------------------
; ACMP1

ACM1PC0		equ	$0698
ACM1PC1		equ	$0699
ACM1PC2		equ	$069a
ACM1PS		equ	$069b

; -------------------------------------------------------------------------
; CPMU

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
VCOFRQ		s12zfld	CPMUSYNR,6,7	;  VCO gain
SYNDIV		s12zfld	CPMUSYNR,0,5	;  Divider
CPMUREFDIV	equ	$06C5		; Reference Divider Register
REFFRQ		s12zfld	CPMUREFDIV,6,7	;  REFFRQ
REFDIV		s12zfld	CPMUREFDIV,0,3	;  Divider
CPMUPOSTDIV	equ	$06C6		; Post Divider Register
POSTDIV		s12zfld	CPMUPOSTDIV,0,4	; Divider
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
RTR		s12zfld	CPMURTI,0,6	;  Real Time Interrupt Prescale Rate Select Bits (4..6)
					;  Real Time Interrupt Modulus Counter Select Bits (0..3)
CPMUCOP		equ	$06CC		; COP Control Register
WCOP		s12zbit	CPMUCOP,7	;  Window COP Mode Bit
RSBCK		s12zbit	CPMUCOP,6	;  COP and RTI Stop in Active BDM Mode Bit
WRTMASK		s12zbit	CPMUCOP,5	;  Write Mask for WCOP and CR[2:0] Bit
CR		s12zfld	CPMUCOP,0,2	;  COP Watchdog Timer Rate Select
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
ACLKTR		s12zfld	CPMUACLKTR,2,7	;  Autonomous Clock Period Trimming Bits
CPMUAPIRH	equ	$06D4		; Autonomous Periodical Interrupt Rate MSB
CPMUAPIRL	equ	$06D5		; Autonomous Periodical Interrupt Rate LSB
CPMUTEST3	equ	$06D6
CPMUHTTR	equ	$06D7		; High Temperature Trimming Register
HTOE		s12zbit	CPMUHTTR,7	;  High Temperature Offset Enable Bit
HTTR		s12zfld	CPMUHTTR,0,3	;  High Temperature Trimming Bits
CPMUIRCTRIMH	equ	$06D8		; IRC1M Trim Register MSB
TCTRIM		s12zfld	CPMUIRCTRIMH,3,7;  IRC1M temperature coefficient Trim Bits
IRCTRIM		s12zfld	CPMUIRCTRIMH,9,0;  IRC1M Frequency Trim Bits
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

; -------------------------------------------------------------------------
; BATS

BATE		equ	$06F0		; BATS Module Enable Register
BVHS		s12zbit	BATE,6		;  BATS Voltage High Select
BVLS		s12zfld	BATE,4,5	;  BATS Voltage Low Select
BSUAE		s12zbit	BATE,3		;  BATS VSUP ADC Connection Enable
BSUSE		s12zbit	BATE,2		;  BATS VSUP Level Sense Enable
BATSR		equ	$06F1		; BATS Module Status Register
BVHC		s12zbit	BATSR,1		;  BATS Voltage Sense High Condition Bit
BVLC		s12zbit	BATSR,0		;  BATS Voltage Sense Low Condition Bit
BATIE		equ	$06F2		; BATS Interrupt Enable Register
BVHIE		s12zbit	BATIE,1		;  BATS Interrupt Enable High
BVLIE		s12zbit	BATIE,0		;  BATS Interrupt Enable Low
BATIF		equ	$06F3		; BATS Interrupt Flag Register
BVHIF		s12zbit	BATIF,1		;  BATS Interrupt Flag High Detect
BVLIF		s12zbit	BATIF,0		;  BATS Interrupt Flag Low Detect

; -------------------------------------------------------------------------
; SCI0

SCI0BDH		equ	$0700		; SCI Baud Rate Register MSB
SCI0BDL		equ	$0701		; SCI Baud Rate Register LSB
SCI0CR1		equ	$0702		; SCI Control Register 1
LOOPS		s12zbit	SCI0CR1,7	;  Loop Select Bit
SCISWAI		s12zbit	SCI0CR1,6	;  SCI Stop in Wait Mode Bit
RSRC		s12zbit	SCI0CR1,5	;  Receiver Source Bit
M		s12zbit	SCI0CR1,4	;  Data Format Mode Bit
WAKE		s12zbit	SCI0CR1,3	;  Wakeup Condition Bit
ILT		s12zbit	SCI0CR1,2	;  Idle Line Type Bit
PE		s12zbit	SCI0CR1,1	;  Parity Enable Bit
PT		s12zbit	SCI0CR1,0	;  Parity Type Bit
SCI0ASR1	equ	$0700		; SCI Alternative Status Register 1
RXEDGIF		s12zbit	SCI0ASR1,7	;  Receive Input Active Edge Interrupt Flag
BERRV		s12zbit	SCI0ASR1,2	;  Bit Error Value
BERRIF		s12zbit	SCI0ASR1,1	;  Bit Error Interrupt Flag
BKDIF		s12zbit	SCI0ASR1,0	;  Break Detect Interrupt Flag
SCI0ACR1	equ	$0701		; SCI Alternative Control Register 1
RXEDGIE		s12zbit	SCI0ACR1,7	;  Receive Input Active Edge Interrupt Enable
BERRIE		s12zbit	SCI0ACR1,1	;  Bit Error Interrupt Enable
BKDIE		s12zbit	SCI0ACR1,0	;  Break Detect Interrupt Enable
SCI0ACR2	equ	$0702		; SCI Alternative Control Register 2
IREN		s12zbit	SCI0ACR2,7	;  Infrared Enable Bit
TNP		s12zfld	SCI0ACR2,5,6	;  Transmitter Narrow Pulse Bits
BERRM		s12zfld	SCI0ACR2,1,2	;  Bit Error Mode
BKDFE		s12zbit	SCI0ACR2,0	;  Break Detect Feature Enable
SCI0CR2		equ	$0703		; SCI Control Register 2
TIE		s12zbit	SCI0CR2,7	;  Transmitter Interrupt Enable Bit
TCIE		s12zbit	SCI0CR2,6	;  Transmission Complete Interrupt Enable Bit
RIE		s12zbit	SCI0CR2,5	;  Receiver Full Interrupt Enable Bit
ILIE		s12zbit	SCI0CR2,4	;  Idle Line Interrupt Enable Bit
TE		s12zbit	SCI0CR2,3	;  Transmitter Enable Bit
RE		s12zbit	SCI0CR2,2	;  Receiver Enable Bit
RWU		s12zbit	SCI0CR2,1	;  Receiver Wakeup Bit
SBK		s12zbit	SCI0CR2,0	;  Send Break Bit
SCI0SR1		equ	$0704		; SCI Status Register 1
TDRE		s12zbit	SCI0SR1,7	;  Transmit Data Register Empty Flag
TC		s12zbit	SCI0SR1,6	;  Transmit Complete Flag
RDRF		s12zbit	SCI0SR1,5	;  Receive Data Register Full Flag
IDLE		s12zbit	SCI0SR1,4	;  Idle Line Flag
OR		s12zbit	SCI0SR1,3	;  Overrun Flag
NF		s12zbit	SCI0SR1,2	;  Noise Flag
FE		s12zbit	SCI0SR1,1	;  Framing Error Flag
PF		s12zbit	SCI0SR1,0	;  Parity Error Flag
SCI0SR2		equ	$0705		; SCI Status Register 2
AMAP		s12zbit	SCI0SR2,7	;  Alternative Map
TXPOL		s12zbit	SCI0SR2,4	;  Transmit Polarity
RXPOL		s12zbit	SCI0SR2,3	;  Receive Polarity
BRK13		s12zbit	SCI0SR2,2	;  Break Transmit Character Length
TXDIR		s12zbit	SCI0SR2,1	;  Transmitter Pin Data Direction in Single-Wire Mode
RAF		s12zbit	SCI0SR2,0	;  Receiver Active Flag
SCI0DRH		equ	$0706		; SCI Data Register MSB
SCI0DRL		equ	$0707		; SCI Data Register LSB

; -------------------------------------------------------------------------
; SCI1

SCI1BDH		equ	$0710
SCI1BDL		equ	$0711
SCI1CR1		equ	$0712
SCI1ASR1	equ	$0710
SCI1ACR1	equ	$0711
SCI1ACR2	equ	$0712
SCI1CR2		equ	$0713
SCI1SR1		equ	$0714
SCI1SR2		equ	$0715
SCI1DRH		equ	$0716
SCI1DRL		equ	$0717

; -------------------------------------------------------------------------
; SPI0

SPI0CR1		equ	$0780		; SPI Control Register 1
SPIE		s12zbit	SPI0CR1,7	;  SPI Interrupt Enable Bit
SPE		s12zbit	SPI0CR1,6	;  SPI System Enable Bit
SPTIE		s12zbit	SPI0CR1,5	;  SPI Transmit Interrupt Enable
MSTR		s12zbit	SPI0CR1,4	;  SPI Master/Slave Mode Select Bit
CPOL		s12zbit	SPI0CR1,3	;  SPI Clock Polarity Bit
CPHA		s12zbit	SPI0CR1,2	;  SPI Clock Phase Bit
SSOE		s12zbit	SPI0CR1,1	;  Slave Select Output Enable
LSBFE		s12zbit	SPI0CR1,0	;  LSB-First Enable
SPI0CR2		equ	$0781		; SPI Control Register 2
XFRW		s12zbit	SPI0CR2,6	;  Transfer Width
MODFEN		s12zbit	SPI0CR2,4	;  Mode Fault Enable Bit
BIDIROE		s12zbit	SPI0CR2,3	;  Output Enable in the Bidirectional Mode of Operation
SPISWAI		s12zbit	SPI0CR2,1	;  SPI Stop in Wait Mode Bit
SPC0		s12zbit	SPI0CR2,0	;  Serial Pin Control Bit 0
SPI0BR		equ	$0782		; SPI Baud Rate Register
SPPR		s12zfld	SPI0BR,4,6	;  SPI Baud Rate Preselection Bits
SPR		s12zfld	SPI0BR,0,2	;  SPI Baud Rate Selection Bits
SPI0SR		equ	$0783		; SPI Status Register
SPIF		s12zbit	SPI0SR,7	;  SPIF Interrupt Flag
SPTEF		s12zbit	SPI0SR,5	;  SPI Transmit Empty Interrupt Flag
MODF		s12zbit	SPI0SR,4	;  Mode Fault Flag
SPI0DRH		equ	$0784		; SPI Data Register MSB
SPI0DRL		equ	$0785		; SPI Data Register LBS

; -------------------------------------------------------------------------
; SPI1

SPI1CR1		equ	$0790
SPI1CR2		equ	$0791
SPI1BR		equ	$0792
SPI1SR		equ	$0793
SPI1DRH		equ	$0794
SPI1DRL		equ	$0795

; -------------------------------------------------------------------------
; IIC0

IBAD		equ	$07C0		; IIC Address Register (1..7)
IBFD		equ	$07C1		; IIC Frequency Divider Register
IBCR		equ	$07C2		; IIC Control Register
IBEN		s12zbit	IBCR,7		;  I-Bus Enable
IBIE		s12zbit	IBCR,6		;  I-Bus Interrupt Enable
MS_SL		s12zbit	IBCR,5		;  Master/Slave Mode Select Bit
Tx_Rx		s12zbit	IBCR,4		;  Transmit/Receive Mode Select Bit
TXAK		s12zbit	IBCR,3		;  Transmit Acknowledge Enable
RSTA		s12zbit	IBCR,2		;  Repeat Start
IBSWAI		s12zbit	IBCR,0		;  I Bus Interface Stop in Wait Mode
IBSR		equ	$07C3		; IIC Status Register
TCF		s12zbit	IBSR,7		;  Data Transferring Bit
IAAS		s12zbit	IBSR,6		;  Addressed as a Slave Bit
IBB		s12zbit	IBSR,5		;  Bus Busy Bit
IBAL		s12zbit	IBSR,4		;  Arbitration Lost
SRW		s12zbit	IBSR,8		;  Slave Read/Write
IBIF		s12zbit	IBSR,9		;  I-Bus Interrupt
RXAK		s12zbit	IBSR,0		;  Received Acknowledge
IBDR		equ	$07C4		; IIC Data I/O Register
IBCR2		equ	$07C5		; IIC Control Register 2
GCEN		s12zbit	IBCR2,7		;  General Call Enable
ADTYPE		s12zbit	IBCR2,6		;  Address Type
ADRHI		s12zfld	IBCR2,0,2	;  Slave Address MSB

; -------------------------------------------------------------------------
; CAN0

CANCTL0		equ	$0800		; MSCAN Control Register 0
RXFRM		s12zbit	CANCTL0,7	;  Received Frame Flag
RXACT		s12zbit	CANCTL0,6	;  Receiver Active Status
CSWAI		s12zbit	CANCTL0,5	;  CAN Stops in Wait Mode
SYNCH		s12zbit	CANCTL0,4	;  Synchronized Status
TIME		s12zbit	CANCTL0,3	;  Timer Enable
WUPE		s12zbit	CANCTL0,2	;  Wake-Up Enable
SLPRQ		s12zbit	CANCTL0,1	;  Sleep Mode Request
INITRQ		s12zbit	CANCTL0,0	;  Initialization Mode Request
CANCTL1		equ	$0801		; MSCAN Control Register 1
CANE		s12zbit	CANCTL1,7	;  MSCAN Enable
CLKSRC		s12zbit	CANCTL1,6	;  MSCAN Clock Source
LOOPB		s12zbit	CANCTL1,5	;  Loopback Self Test Mode
LISTEN		s12zbit	CANCTL1,4	;  Listen Only Mode
BORM		s12zbit	CANCTL1,3	;  Bus-Off Recovery Mode
WUPM		s12zbit	CANCTL1,2	;  Wake-Up Mode
SLPAK		s12zbit	CANCTL1,1	;  Sleep Mode Acknowledge
INITAK		s12zbit	CANCTL1,0	;  Initialization Mode Acknowledge
CANBTR0		equ	$0802		; MSCAN Bus Timing Register 0
SJW		s12zfld	CANBTR0,6,7	;  Synchronization Jump Width
BRP		s12zfld	CANBTR0,0,5	;  Baud Rate Prescaler
CANBTR1		equ	$0803		; MSCAN Bus Timing Register 1
SAMP		s12zbit	CANBTR1,7	;  Sampling
TSEG2		s12zfld	CANBTR1,4,6	;  Time Segment 2
TSEG1		s12zfld	CANBTR1,0,3	;  Time Segment 1
CANRFLG		equ	$0804		; MSCAN Receiver Flag Register
WUPIF		s12zbit	CANRFLG,7	;  Wake-Up Interrupt Flag
CSCIF		s12zbit	CANRFLG,6	;  CAN Status Change Interrupt Flag
RSTAT		s12zfld	CANRFLG,4,5	;  Receiver Status
TSTAT		s12zfld	CANRFLG,2,3	;  Transmitter Status
OVRIF		s12zbit	CANRFLG,1	;  Overrun Interrupt Flag
RXF		s12zbit	CANRFLG,0	;  Receive Buffer Full Flag
CANRIER		equ	$0805		; MSCAN Receiver Interrupt Enable Register
WUPIE		s12zbit	CANRIER,7	;  Wake-Up Interrupt Enable
CSCIE		s12zbit	CANRIER,6	;  CAN Status Change Interrupt Enable
RSTATE		s12zfld	CANRIER,4,5	;  Receiver Status Change Enable
TSTATE		s12zfld	CANRIER,2,3	;  Transmitter Status Change Enable
OVRIE		s12zbit	CANRIER,1	;  Overrun Interrupt Enable
RXFIE		s12zbit	CANRIER,0	;  Receiver Full Interrupt Enable
CANTFLG		equ	$0806		; MSCAN Transmitter Flag Register
TXE2		s12zbit	CANTFLG,2	;  Transmitter Buffer n Empty
TXE1		s12zbit	CANTFLG,1
TXE0		s12zbit	CANTFLG,0
CANTIER		equ	$0807		; MSCAN Transmitter Interrupt Enable Register
TXEIE2		s12zbit	CANTIER,2	;  Transmitter n Empty Interrupt Enable
TXEIE1		s12zbit	CANTIER,1
TXEIE0		s12zbit	CANTIER,0
CANTARQ		equ	$0808		; MSCAN Transmitter Message Abort Request Register
ABTRQ2		s12zbit	CANTARQ,2	;  Abort Request n
ABTRQ1		s12zbit	CANTARQ,1
ABTRQ0		s12zbit	CANTARQ,0
CANTAAK		equ	$0809		; MSCAN Transmitter Message Abort Acknowledge Register
ABTAK2		s12zbit	CANTAAK,2	;  Abort Acknowledge n
ABTAK1		s12zbit	CANTAAK,1
ABTAK0		s12zbit	CANTAAK,0
CANTBSEL	equ	$080A		; MSCAN Transmit Buffer Selection Register
TX2		s12zbit	CANTBSEL,2	;  Transmit Buffer Select n
TX1		s12zbit	CANTBSEL,1
TX0		s12zbit	CANTBSEL,0
CANIDAC		equ	$080B		; MSCAN Identifier Acceptance Control Register
IDAM		s12zfld	CANIDAC,4,5	;  Identifier Acceptance Mode
IDHIT		s12zfld	CANIDAC,0,2	;  Identifier Acceptance Hit Indicator
CANMISC		equ	$080D		; MSCAN Miscellaneous Register
BOHOLD		s12zbit	CANMISC,0	;  Bus-off State Hold Until User Request
CANRXERR	equ	$080E		; MSCAN Receive Error Counter
CANTXERR	equ	$080F		; MSCAN Transmit Error Counter
CANIDAR0	equ	$0810		; MSCAN Identifier Acceptance Register 0
CANIDAR1	equ	$0812		; MSCAN Identifier Acceptance Register 1
CANIDAR2	equ	$0813		; MSCAN Identifier Acceptance Register 2
CANIDAR3	equ	$0813		; MSCAN Identifier Acceptance Register 3
CANIDMR0	equ	$0814		; MSCAN Identifier Mask Register 0
CANIDMR1	equ	$0815		; MSCAN Identifier Mask Register 1
CANIDMR2	equ	$0816		; MSCAN Identifier Mask Register 2
CANIDMR3	equ	$0817		; MSCAN Identifier Mask Register 3
CANIDAR4	equ	$0818		; MSCAN Identifier Acceptance Register 4
CANIDAR5	equ	$0819		; MSCAN Identifier Acceptance Register 5
CANIDAR6	equ	$081A		; MSCAN Identifier Acceptance Register 6
CANIDAR7	equ	$081B		; MSCAN Identifier Acceptance Register 7
CANIDMR4	equ	$081C		; MSCAN Identifier Mask Register 4
CANIDMR5	equ	$081D		; MSCAN Identifier Mask Register 5
CANIDMR6	equ	$081E		; MSCAN Identifier Mask Register 6
CANIDMR7	equ	$081F		; MSCAN Identifier Mask Register 7

; -------------------------------------------------------------------------
; CANPHY

CPDR		equ	$0990		; Port CP Data Register
CPDR7		s12zbit	CPDR,7		;  Port CP Data Bit 7
CPDR1		s12zbit	CPDR,1		;  Port CP Data Bit 1
CPDR0		s12zbit	CPDR,0		;  Port CP Data Bit 0
CPCR		equ	$0991		; CAN Physical Layer Control Register
CPE		s12zbit	CPCR,7		;  CAN Physical Layer Enable
SPE		s12zbit	CPCR,6		;  Split Enable
WUPE		s12zfld	CPCR,4,5	;  Wake-Up Receiver Enable and Filter Select
SLR		s12zfld	CPCR,0,2	;  Slew Rate
CPSR		equ	$0993		; CAN Physical Layer Status Register
CPCHVH		s12zbit	CPSR,7		;  CANH Voltage Failure High Status Bit
CPCHVL		s12zbit	CPSR,6		;  CANH Voltage Failure Low Status Bit
CPCLVH		s12zbit	CPSR,5		;  CANL Voltage Failure High Status Bit
CPCLVL		s12zbit	CPSR,4		;  CANL Voltage Failure Low Status Bit
CPDT		s12zbit	CPSR,3		;  CPTXD-Dominant Timeout Status Bit
CPIE		equ	$0996		; CAN Physical Layer Interrupt Enable Register
CPVFIE		s12zbit	CPIE,4		;  CAN Physical Layer Voltage-Failure Interrupt Enable
CPDTIE		s12zbit	CPIE,3		;  CPTXD-Dominant Timeout Interrupt Enable
CPOCIE		s12zbit	CPIE,0		;  CAN Physical Layer Over-current Interrupt Enable
CPIF		equ	$0997		; CAN Physical Layer Interrupt Flag Register
CHVHIF		s12zbit	CPIF,7		;  CANH Voltage Failure High Interrupt Flag
CHVLIF		s12zbit	CPIF,6		;  CANH Voltage Failure Low Interrupt Flag
CLVHIF		s12zbit	CPIF,5		;  CANL Voltage Failure High Interrupt Flag
CLVLIF		s12zbit	CPIF,4		;  CANL Voltage Failure Low Interrupt Flag
CPDTIF		s12zbit	CPIF,3		;  CAN CPTXD-Dominant Timeout Interrupt Flag
CHOCIF		s12zbit	CPIF,1		;  CANH Over-Current Interrupt Flag
CLOCIF		s12zbit	CPIF,0		;  CANL Over-Current Interrupt Flag

; -------------------------------------------------------------------------
; SENTTX

STTICKRATE	equ	$09A0		; SENT Transmitter Tick Rate Register (16 bit)
PRE		s12zfld	STTICKRATE,0,13	;  SENTTX Tick Rate Bits
STPPULSE	equ	$09A2		; SENT Transmitter Pause-Pulse Register (16 bit)
PPEN		s12zbit	STPPULSE,15	;  SENTTX Pause Pulse Enable
PPFIXED		s12zbit	STPPULSE,14	;  SENTTX Fixed Length Pause Pulse
PPCOUNT		s12zfld	STPPULSE,0,10	;  SENTTX Pause Pulse Count Ticks
STCONFIG	equ	$09A4		; SENT Transmitter Configuration Register (16 bit)
TXINIT		s12zbit	STCONFIG,15	;  SENTTX Initialization Enable
TXEN		s12zbit	STCONFIG,14	;  SENTTX Pin Enable
DNIBBLECOUNT	s12zfld	STCONFIG,8,10	;  SENTTX Data Nibble Count
OPTEDGE		s12zbit	STCONFIG,4	;  SENTTX Optimized Rising Edge Position
SINGLE		s12zbit	STCONFIG,3	;  SENTTX Single Shot Operation
CRCSCN		s12zbit	STCONFIG,2	;  SENTTX CRC includes Status- and Communication Nibble
CRCLEG		s12zbit	STCONFIG,1	;  SENTTX CRC Legacy Algorithm Enable
CRCBYP		s12zbit	STCONFIG,0	;  SENTTX Automatic CRC generation bypass
STINTEN		equ	$09A6		; SENT Transmitter Interrupt Enable Register
PPREIE		s12zbit	STINTEN,4	;  SENTTX Pause Pulse Rising-Edge Interrupt Enable
TUIE		s12zbit	STINTEN,3	;  SENTTX Transmitter Under-run Interrupt Enable
CSIE		s12zbit	STINTEN,2	;  SENTTX Calibration Pulse Start Interrupt Enable
TCIE		s12zbit	STINTEN,1	;  SENTTX Transmission Complete Interrupt Enable
TBEIE		s12zbit	STINTEN,0	;  SENTTX Transmit-Buffer Empty Interrupt Enable
STINTFLG	equ	$09A7		; SENT Transmitter Interrupt Flag Register
PPRE		s12zbit	STINTFLG,4	;  SENTTX Pause Pulse Rising-Edge Flag
TU		s12zbit	STINTFLG,3	;  SENTTX Transmitter Under-run Flag
CS		s12zbit	STINTFLG,2	;  SENTTX Calibration Pulse Start Flag
TC		s12zbit	STINTFLG,1	;  SENTTX Transmission Complete Flag
TBE		s12zbit	STINTFLG,0	;  SENTTX Transmit-Buffer Empty Flag
STTXBUF		equ	$09A8		; SENT Transmit Buffer (32 bit)
STATCONF	s12zfld	STTXBUF,28,31	;  SENTTX Status and Configuration Nibble
CRC		s12zfld	STTXBUF,24,27	;  SENTTX CRC Nibble
DATA0		s12zfld	STTXBUF,20,23	;  SENTTX Data Nibble 0
DATA1		s12zfld	STTXBUF,16,19	;  SENTTX Data Nibble 1
DATA2		s12zfld	STTXBUF,12,15	;  SENTTX Data Nibble 2
DATA3		s12zfld	STTXBUF,8,11	;  SENTTX Data Nibble 3
DATA4		s12zfld	STTXBUF,4,7	;  SENTTX Data Nibble 4
DATA5		s12zfld	STTXBUF,0,3	;  SENTTX Data Nibble 5
