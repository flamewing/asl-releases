		ifndef  reg166inc       ; avoid multiple inclusion
reg166inc       equ     1

                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - Datei REG166.INC                                             *
;*   								            *
;*   Contains SFR, Macro and Adress Definitions for 80C166/167              *
;*                                                                          *
;****************************************************************************

		switch	MOMCPUNAME
     		case   	"80C166"
_n   		equ	166
     		case   	"80C167"
_n  	 	equ    	167
     		case   	"80C167CS"
_n   		equ    	168
     		elsecase
     		fatal  "wrong target: only 80C166, 80C167, or 80C167CS are supported!"
     		endcase

                if      MOMPASS=1
		 message "80C166/167 SFR Definitions (C) 1994 Alfred Arnold"
		 message "Extensions for  80C167CS       2003 Marc Reibel"
		 message "including SFPRs for \{MOMCPUNAME}"
		endif

; Make things a bit shorter:

DefIntBits      macro   BASE,adr,{NoExpand} ; Define Interrupt Register + its Bits
tmpstr166	set	"BASE"
{tmpstr166}IC   equ     adr
{tmpstr166}_GLVL bit    {tmpstr166}IC.0 ; Group Priority
{tmpstr166}_ILVL bit    {tmpstr166}IC.2 ; Interrupt Priority
{tmpstr166}IE   bit     {tmpstr166}IC.6 ; Interrupt Enable
{tmpstr166}IR   bit     {tmpstr166}IC.7 ; Interrupt Flag
		endm

;----------------------------------------------------------------------------
; Special Registers:
; Bit Addresses only for bit-addressable registers!
; Only start bit for bit fields!

                if      _n>=167
SYSCON           equ     0ff12h         ; System Configuration
XPERSHARE        bit     SYSCON.0       ; XBUS Peripherals also on External BUS
VISIBLE          bit     SYSCON.1       ; XBUS Peripherals on Port Pins
XPEN             bit     SYSCON.2       ; Enable XBUS Peripherals
BDRSTEN          bit     SYSCON.3       ; Bidirectional /RSTIN
OWDDIS           bit     SYSCON.4       ; Disable Oscillator Watchdog
CSCFG            bit     SYSCON.6       ; Chip Select Mode (1=non-latched)
WRCFG            bit     SYSCON.7       ; /WRL and /WRH instead of /WR and /BHE
CLKEN            bit     SYSCON.8       ; Emit System Clock on Pin
BYTDIS           bit     SYSCON.9       ; Block /BHE Pin
ROMENT           bit     SYSCON.10      ; Enable Internal Rom
SGTDIS           bit     SYSCON.11      ; Disable Memory Segmentation
ROMS1            bit     SYSCON.12      ; Position of Internal ROM


		if	 _n>=168
SYSCON1		 equ	 0f1dch		; Sleep Mode for IDLE Instruction
					; (Bits 1 and 0)
					; 00 Normal IDLE Mode
					; 01 SLEEP Mode
					; 11 SLEEP Mode, RTC and Oscillator off

SYSCON2		 equ	 0f1d0h		; Clock Generieration
RCS		 bit	 SYSCON2.6	; RTC Clock Source Main Oscillator
SCS		 bit	 SYSCON2.7	; SDD Clock Source Main Oscillator
CLKLOCK		 bit	 SYSCON2.15	; Main Oscillator/PLL stable?

RSTCON		 equ	 0f1e0h		; Reset Control Register

IDCHIP		 equ	 0f07ch		; Chip Identification Register
IDMANUF		 equ	 0f07eh	
IDMEM2		 equ	 0f076h
IDMEM		 equ	 0f07ah
IDPROG		 equ	 0f078h

FOCON		 equ	 0ffaah		; Clock Output Control Register

SYSCON3		 equ	 0f1d4h		; Peripheral Management
ADCDIS		 bit	 SYSCON3.0	; Disable ADC
ASC0DIS		 bit	 SYSCON3.1	; Disable Asynchronous Serial Port
SSCDIS		 bit	 SYSCON3.2	; Disable Synchronous Serial Port
GPTDIS		 bit	 SYSCON3.3	; Disable GPT Disables
DFMDIS		 bit	 SYSCON3.4	; Disable DFlash (only Variants with Flash)
PFMDIS		 bit	 SYSCON3.5	; Disable PFlash (only Variants with Flash)
CC1DIS		 bit	 SYSCON3.6	; Disable Capcom1
CC2DIS		 bit	 SYSCON3.7	; Disable Capcom2
PWMDIS		 bit	 SYSCON3.9	; Disable PWM
CAN1DIS		 bit	 SYSCON3.13	; Disable CAN1
CAN2DIS		 bit	 SYSCON3.14	; Disable CAN2
PCDDIS		 bit	 SYSCON3.15	; Disable Peripheral Clock Source
		endif

                elseif
SYSCON           equ     0ff0ch
MCTC             bit     SYSCON.0       ; Memory Cycle Length
RWDC             bit     SYSCON.4       ; Write/Read Delay
MTTC             bit     SYSCON.5       ; Tri State Time
BTYP             bit     SYSCON.6       ; State of EBC-Pins
RDYEN            bit     SYSCON.12      ; Enable /READY
                endif
ROMEN           bit     SYSCON.10       ; Enable Internal ROM
STKSZ           bit     SYSCON.13       ; Maximum Size of Stack

                if      _n>=167
RP0H             equ     0f108h         ; System Startup Register (Read Only)
CSSEL            bit     RP0H.1         ; Number of CS Lines
SALSEL           bit     RP0H.3         ; Number of Address Lines, starting at A16
                endif

PSW             equ     0ff10h          ; Flag Register
N               bit     PSW.0           ; Negatives Result
C               bit     PSW.1           ; Carry
V               bit     PSW.2           ; Overflow
Z               bit     PSW.3           ; Zero Result
E               bit     PSW.4           ; End of Table
MULIP           bit     PSW.5           ; Multiplication/Division interrupted ?
USR0            bit     PSW.6           ; User Defined
		if      _n>=167
HLDEN		 bit	 PSW.10		; Enable Hold
		endif
IEN             bit     PSW.11          ; Enable Interrupts
ILVL            bit     PSW.12          ; Interrupt Level currently being served

TFR             equ     0ffach          ; Trap Flag Register
ILLBUS          bit     TFR.0           ; Externer Bus used, but not configured
ILLINA          bit     TFR.1           ; Code Access to Odd Address
ILLOPA          bit     TFR.2           ; Word Access to Odd Address
PRTFLT          bit     TFR.3           ; Protection Violation
UNDOPC          bit     TFR.7           ; Undefined Maschine Instruction
STKUF           bit     TFR.13          ; Stack Underflow
STKOF           bit     TFR.14          ; Stack Overflow
NMI             bit     TFR.15          ; Non-Maskable Interrupt
                if      _n>=167
EXICON           equ     0f1c0h         ; External Interrupt Controller Control
		if	_n>=168
EXISEL		 equ	 0f1dah		; External Interrupt Source Register
		endif
EXI0ES           bit     EXICON.0       ; Edge-Triggered Interrupt 0
EXI1ES           bit     EXICON.2       ; Edge-Triggered Interrupt 1
EXI2ES           bit     EXICON.4       ; Edge-Triggered Interrupt 2
EXI3ES           bit     EXICON.6       ; Edge-Triggered Interrupt 3
EXI4ES           bit     EXICON.8       ; Edge-Triggered Interrupt 4
EXI5ES           bit     EXICON.10      ; Edge-Triggered Interrupt 5
EXI6ES           bit     EXICON.12      ; Edge-Triggered Interrupt 6
EXI7ES           bit     EXICON.14      ; Edge-Triggered Interrupt 7
                endif

DPP0            equ     0fe00h          ; Paging Register
DPP1            equ     0fe02h
DPP2            equ     0fe04h
DPP3            equ     0fe06h

CSP             equ     0fe08h          ; Bits 16.. of Program Counter

MDH             equ     0fe0ch          ; Multiplier/Divider Register
MDL             equ     0fe0eh
MDC             equ     0ff0eh          ; Multiplier/Divider Control
MDRIU           bit     MDC.4           ; Multiplier in Use

CP              equ     0fe10h          ; Register Context Pointer

SP              equ     0fe12h          ; Stack Pointer
STKOV           equ     0fe14h          ; Upper Bound Stack Pointer
STKUN           equ     0fe16h          ; Lower Bound Stack Pointer

ZEROS           equ     0ff1ch          ; Constant All-Zeros
ONES            equ     0ff1eh          ; Constant All-Ones

;----------------------------------------------------------------------------
; Memory Interface

                if      _n>=167
ADDRSEL1         equ     0fe18h         ; Address Decoder Register 1..4
ADDRSEL2         equ     0fe1ah
ADDRSEL3         equ     0fe1ch
ADDRSEL4         equ     0fe1eh

BUSCON0          equ     0ff0ch         ; Bus Configuration Register 0..4
MCTC0            bit     BUSCON0.0      ; Memory Cycle Length
RWDC0            bit     BUSCON0.4      ; Write/Read Delay
MTTC0            bit     BUSCON0.5      ; Tri State Zeit
BTYP0            bit     BUSCON0.6      ; State of EBC Pins
ALECTL0          bit     BUSCON0.9      ; ALE Extension
BUSACT0          bit     BUSCON0.10     ; Bus Active
RDYEN0           bit     BUSCON0.12     ; Enable /READY
BUSCON1          equ     0ff14h
MCTC1            bit     BUSCON1.0      ; Memory Cycle Length
RWDC1            bit     BUSCON1.4      ; Write/Read Delay
MTTC1            bit     BUSCON1.5      ; Tri State Zeit
BTYP1            bit     BUSCON1.6      ; State of EBC Pins
ALECTL1          bit     BUSCON1.9      ; ALE Extension
BUSACT1          bit     BUSCON1.10     ; Bus Active
RDYEN1           bit     BUSCON1.12     ; Enable /READY
CSREN1           bit     BUSCON1.14     ; CS Pins Operating Mode
CSWEN1           bit     BUSCON1.15
BUSCON2          equ     0ff16h
MCTC2            bit     BUSCON2.0      ; see above
RWDC2            bit     BUSCON2.4
MTTC2            bit     BUSCON2.5
BTYP2            bit     BUSCON2.6
ALECTL2          bit     BUSCON2.9
BUSACT2          bit     BUSCON2.10
RDYEN2           bit     BUSCON2.12
CSREN2           bit     BUSCON2.14
CSWEN2           bit     BUSCON2.15
BUSCON3          equ     0ff18h
MCTC3            bit     BUSCON3.0
RWDC3            bit     BUSCON3.4
MTTC3            bit     BUSCON3.5
BTYP3            bit     BUSCON3.6
ALECTL3          bit     BUSCON3.9
BUSACT3          bit     BUSCON3.10
RDYEN3           bit     BUSCON3.12
CSREN3           bit     BUSCON3.14
CSWEN3           bit     BUSCON3.15
BUSCON4          equ     0ff1ah
MCTC4            bit     BUSCON4.0
RWDC4            bit     BUSCON4.4
MTTC4            bit     BUSCON4.5
BTYP4            bit     BUSCON4.6
ALECTL4          bit     BUSCON4.9
BUSACT4          bit     BUSCON4.10
RDYEN4           bit     BUSCON4.12
CSREN4           bit     BUSCON4.14
CSWEN4		 bit     BUSCON4.15

		if	 _n>=168
XPERCON		 equ	 0f024h		; XPeripherals
XPER0_M		 equ	 0001h		; CAN1 (on after Reset)
XPER1_M		 equ	 0002h		; CAN2 (off after Reset)
XPER10_M	 equ	 0400h		; XRAM 2 KByte (on after Reset)
XPER11_M	 equ	 0800h		; XRAM 6 KByte (off after Reset)!!!
XPER14_M	 equ	 4000h		; 4KByte DFlash (off after Reset)!!!

ISNC		 equ	 0f1deh		; Interrupt Sub Node Control Register
RTCIR		 bit	 ISNC.0		; RTC Interrupt Request Flag
RTCIE		 bit	 ISNC.1		; Enable RTC Interrupt
PLLIR		 bit	 ISNC.2		; PLL Interrupt Request Flag
PLLIE		 bit	 ISNC.3		; Enable PLL Interrupt
		endif

                 DefIntBits XP0,0f186h  ; Interrupt Control Peripheral Device 0
                 DefIntBits XP1,0f18eh  ; Interrupt Control Peripheral Device 1
                 DefIntBits XP2,0f196h  ; Interrupt Control Peripheral Device 2
					; Flash Termination (Flash Variants) 
                 DefIntBits XP3,0f19eh  ; Interrupt Control Peripheral Device 3
                endif

;----------------------------------------------------------------------------
; Ports:

                if      _n>=167
PICON		 equ	 0f1c4h
                endif

                if      _n>=167
P0L              equ     0ff00h         ; Port 0 Data Register
P0H              equ     0ff02h
DP0L             equ     0f100h         ; Port 0 Data Direction Register
DP0H             equ     0f102h
		if	 _n>=168
POCON0H		 equ	 0f082h
POCON0L		 equ	 0f080h
		endif
                elseif
P0               equ     0ff00h
DP0              equ     0ff02h
                endif

                if      _n>=167
P1L              equ     0ff04h         ; Port 1 Data Register
P1H              equ     0ff06h
DP1L             equ     0f104h         ; Port 1 Data Direction Register
DP1H             equ     0f106h
		if	 _n>=168
P1DIDIS		 equ	 0fea4h		; Port 1 Disable Digital-Input
		endif
		if	 _n>=168
POCON1H		 equ	 0f086h		; Port 1 Output Drivers
POCON1L		 equ	 0f084h
		endif
                elseif
P1               equ     0ff04h 
DP1              equ     0ff06h
                endif

P2              equ     0ffc0h          ; Port 2 Data Register
DP2             equ     0ffc2h          ; Port 2 Data Direction Register
                if      _n>=167
ODP2             equ     0f1c2h         ; Port 2 Open Drain Setting
P2LIN		 bit	 PICON.0	; Port 2 Input Threshold Bits 0..7
P2HIN		 bit	 PICON.1	; Port 2 Input Threshold Bits 8..15
		if	 _n>=168
POCON20		 equ	 0f0aah		; Port 2 Dedicated Pin Control Register
					; /RSTOUT,CLKOUT/FOUT,ALE,
					; /WH,/RD,/BHE,/WH		
POCON2		 equ	 0f088h		; Port 2 Output Drivers
		endif
                endif

P3              equ     0ffc4h          ; Port 3 Data Register
DP3             equ     0ffc6h          ; Port 3 Data Direction Register
                if      _n>=167
ODP3             equ     0f1c6h         ; Port 3 Open Drain Setting
P3LIN		 bit	 PICON.2	; Port 3 Input Threshold Bits 0..7
P3HIN		 bit	 PICON.3	; Port 3 Input Threshold Bits 8..15
		if	 _n>=168
POCON3		 equ	 0f08ah		; Port 3 Output Drivers
		endif
                endif

                if      _n>=167
P4               equ     0ffc8h         ; Port 4 Data Register
DP4              equ     0ffcah         ; Port 4 Data Direction Register
ODP4		 equ	 0f1cah		; Port 4 Open Drain Setting
P4LIN		 bit	 PICON.4	; Port 4 Input Threshold
		if	 _n>=168
POCON4		 equ	 0f08ch		; Port 4 Output Drivers
		endif
                elseif
P4               equ     0ff08h
DP4              equ     0ff0ah
                endif

P5              equ     0ffa2h          ; Port 5 Data Register (input-only)

                if      _n>=167
P5DIDIS		 equ	 0ffa4h
		endif

                if      _n>=167
P6               equ     0ffcch         ; Port 6 Data Register
DP6              equ     0ffceh         ; Port 6 Data Direction Register
ODP6             equ     0f1ceh         ; Port 6 Open Drain Setting
P6LIN		 bit	 PICON.5	; Port 6 Input Threshold
		if	 _n>=168
POCON6		 equ	 0f08eh		; Port 6 Output Drivers
		endif
P7               equ     0ffd0h         ; Port 7 Data Register
DP7              equ     0ffd2h         ; Port 7 Data Direction Register
ODP7             equ     0f1d2h         ; Port 7 Open Drain Setting
P7LIN		 bit	 PICON.6	; Port 7 Input Threshold
		if	 _n>=168
POCON7		 equ	 0f090h		; Port 7 Output Drivers
		endif
P8               equ     0ffd4h         ; Port 8 Data Register
DP8              equ     0ffd6h         ; Port 8 Data Direction Register
ODP8             equ     0f1d6h         ; Port 8 Open Drain Setting
P8LIN		 bit	 PICON.7	; Port 8 Input Threshold
		if	 _n>=168
POCON8		 equ	 0f092h		; Port 8 Output Drivers
		endif
                endif

;----------------------------------------------------------------------------
; Timers:

T0              equ     0fe50h          ; CAPCOM Timer 0
T0REL           equ     0fe54h          ; Timer 0 Reload Register
T01CON          equ     0ff50h          ; Timer 0+1 Control Register
T0I             bit     T01CON.0        ; Timer 0 Input Selection
T0M             bit     T01CON.3        ; Timer 0 Mode
T0R             bit     T01CON.6        ; Timer 0 Start/Stop
T1I             bit     T01CON.8        ; Timer 1 Input Selection
T1M             bit     T01CON.11       ; Timer 1 Mode
T1R             bit     T01CON.14       ; Timer 1 Start/Stop
                DefIntBits T0,0ff9ch    ; Timer 0 Interrupt Control

T1              equ     0fe52h          ; CAPCOM Timer 1
T1REL           equ     0fe56h          ; Timer 1 Reload Register
                DefIntBits T1,0ff9eh    ; Timer 1 Interrupt Control

T2              equ     0fe40h          ; Timer 2
T2CON           equ     0ff40h          ; Timer 2 Control Register
T2I             bit     T2CON.0         ; Timer 2 Input Selection
T2M             bit     T2CON.3         ; Timer 2 Mode
T2R             bit     T2CON.6         ; Timer 2 Start/Stop
T2UD            bit     T2CON.7         ; Timer 2 Direction
                if      _n>=167
T2UDE            bit    T2CON.8         ; Timer 2 External Count Direction Selection
                endif
		DefIntBits T2,0ff60h    ; Timer 2 Interrupt Control

T3              equ     0fe42h          ; Timer 3
T3CON           equ     0ff42h          ; Timer 3 Control Register
T3I             bit     T3CON.0         ; Timer 3 Input Selection
T3M             bit     T3CON.3         ; Timer 3 Mode
T3R             bit     T3CON.6         ; Timer 3 Start/Stop
T3UD            bit     T3CON.7         ; Timer 3 Direction
T3UDE           bit     T3CON.8         ; Timer 3 External Count Direction Selection
T3OE            bit     T3CON.9         ; Timer 3 Enable Output
T3OTL           bit     T3CON.10        ; Timer 3 Overflow
		DefIntBits T3,0ff62h    ; Timer 3 Interrupt Control

T4              equ     0fe44h          ; Timer 4
T4CON           equ     0ff44h          ; Timer 4 Control Register
T4I             bit     T4CON.0         ; Timer 4 Eingangssignal
T4M             bit     T4CON.3         ; Timer 4 Mode
T4R             bit     T4CON.6         ; Timer 4 Start/Stop
T4UD            bit     T4CON.7         ; Timer 4 Direction
                if      _n>=167
T4UDE            bit    T4CON.8         ; Timer 4 External Count Direction Selection
                endif
                DefIntBits T4,0ff64h    ; Timer 4 Interrupt Control

T5              equ     0fe46h          ; Timer 5
T5CON           equ     0ff46h          ; Timer 5 Control Register
T5I             bit     T5CON.0         ; Timer 5 Input Selection
T5M             bit     T5CON.3         ; Timer 5 Mode
T5R             bit     T5CON.6         ; Timer 5 Start/Stop
T5UD            bit     T5CON.7         ; Timer 5 Direction
                if      _n>=167
T5UDE            bit    T5CON.8         ; Timer 5 External Count Direction Selection
                endif
CI              bit     T5CON.12        ; CAPREL Input Selection
T5CLR           bit     T5CON.14        ; Timer 5 Clear upon Capture
T5SC            bit     T5CON.15        ; Timer 5 Capture Enable
		DefIntBits T5,0ff66h    ; Timer 5 Interrupt Control

T6              equ     0fe48h          ; Timer 6
T6CON           equ     0ff48h          ; Control Register Timer 6
T6I             bit     T6CON.0         ; Timer 6 Input Selection
                if      _n>=167
T6M              bit    T6CON.3         ; Timer 6 Mode
                endif
T6R             bit     T6CON.6         ; Timer 6 Start/Stop
T6UD            bit     T6CON.7         ; Timer 6 Count Direction
                if      _n>=167
T6UDE            bit    T6CON.8         ; Timer 6 External Count Direction Selection
                endif
T6OE            bit     T6CON.9         ; Timer 6 Enable Output
T6OTL           bit     T6CON.10        ; Timer 6 Overflow
T6SR            bit     T6CON.15        ; Timer 6 Reload
		DefIntBits T6,0ff68h    ; Timer 6 Interrupt Control

                if      _n>=167
T78CON           equ     0ff20h         ; Timer 7+8 Control Register

T7               equ     0f050h         ; Timer 7
T7REL            equ     0f054h         ; Timer 7 Reload Register
T7M		 bit	 T78CON.3	; Timer 7 Timer/Counter Mode
                 DefIntBits T7,0f17ah   ; Timer 7 Interrupt Control

T8               equ     0f052h         ; CAPCOM Timer 8
T8REL            equ     0f056h         ; Timer 8 Reload Register
T8M		 bit	 T78CON.11	; Timer 8 Timer/Counter Mode
                 DefIntBits T8,0f17ch   ; Timer 8 Interrupt Control
		if	 _n>=168
T14		 equ	 0f0d2h		; RTC Timer 14 
T14REL		 equ	 0f0d0h		; RTC Timer 14 Reload Register
RTCL		 equ	 0f0d4h		; RTC Low Word
RTCH		 equ	 0f0d6h		; RTC High Word
		endif
                endif

;-----------------------------------------------------------------------
; Capture/Compare Unit:

CAPREL          equ     0fe4ah		; Capture/Reload Register
CC0             equ     0fe80h          ; CAPCOM Registers
CC1             equ     0fe82h
CC2             equ     0fe84h
CC3             equ     0fe86h
CC4             equ     0fe88h
CC5             equ     0fe8ah
CC6             equ     0fe8ch
CC7             equ     0fe8eh
CC8             equ     0fe90h
CC9             equ     0fe92h
CC10            equ     0fe94h
CC11            equ     0fe96h
CC12            equ     0fe98h
CC13            equ     0fe9ah
CC14            equ     0fe9ch
CC15            equ     0fe9eh
                if      _n>=167
CC16             equ     0fe60h         ; Further CAPCOM Registers
CC17             equ     0fe62h
CC18             equ     0fe64h
CC19             equ     0fe66h
CC20             equ     0fe68h
CC21             equ     0fe6ah
CC22             equ     0fe6ch
CC23             equ     0fe6eh
CC24             equ     0fe70h
CC25             equ     0fe72h
CC26             equ     0fe74h
CC27             equ     0fe76h
CC28             equ     0fe78h
CC29             equ     0fe7ah
CC30             equ     0fe7ch
CC31             equ     0fe7eh
                endif

CCM0            equ     0ff52h          ; CAPCOM Mode Register
CCMOD0          bit     CCM0.0          ; CC0 Mode Selection
ACC0            bit     CCM0.3          ; CC0 assigned to Timer 0/1
CCMOD1          bit     CCM0.4          ; CC1 Mode Selection
ACC1            bit     CCM0.7          ; CC1 assigned to Timer 0/1
CCMOD2          bit     CCM0.8          ; CC2 Mode Selection
ACC2            bit     CCM0.11         ; CC2 assigned to Timer 0/1
CCMOD3          bit     CCM0.12         ; CC3 Mode Selection
ACC3            bit     CCM0.15         ; CC3 assigned to Timer 0/1

CCM1            equ     0ff54h
CCMOD4          bit     CCM1.0          ; CC4 Mode Selection
ACC4            bit     CCM1.3          ; CC4 assigned to Timer 0/1
CCMOD5          bit     CCM1.4          ; CC5 Mode Selection
ACC5            bit     CCM1.7          ; CC5 assigned to Timer 0/1
CCMOD6          bit     CCM1.8          ; CC6 Mode Selection
ACC6            bit     CCM1.11         ; CC6 assigned to Timer 0/1
CCMOD7          bit     CCM1.12         ; CC7 Mode Selection
ACC7            bit     CCM1.15         ; CC7 assigned to Timer 0/1

CCM2            equ     0ff56h
CCMOD8          bit     CCM2.0          ; CC8 Mode Selection
ACC8            bit     CCM2.3          ; CC8 assigned to Timer 0/1
CCMOD9          bit     CCM2.4          ; CC9 Mode Selection
ACC9            bit     CCM2.7          ; CC9 assigned to Timer 0/1
CCMOD10         bit     CCM2.8          ; CC10 Mode Selection
ACC10           bit     CCM2.11         ; CC10 assigned to Timer 0/1
CCMOD11         bit     CCM2.12         ; CC11 Mode Selection
ACC11           bit     CCM2.15         ; CC11 assigned to Timer 0/1

CCM3            equ     0ff58h
CCMOD12         bit     CCM3.0          ; CC12 Mode Selection
ACC12           bit     CCM3.3          ; CC12 assigned to Timer 0/1
CCMOD13         bit     CCM3.4          ; CC13 Mode Selection
ACC13           bit     CCM3.7          ; CC13 assigned to Timer 0/1
CCMOD14         bit     CCM3.8          ; CC14 Mode Selection
ACC14           bit     CCM3.11         ; CC14 assigned to Timer 0/1
CCMOD15         bit     CCM3.12         ; CC15 Mode Selection
ACC15           bit     CCM3.15         ; CC15 assigned to Timer 0/1

CCM4            equ     0ff22h
CCMOD16         bit     CCM4.0          ; CC16 Mode Selection
ACC16           bit     CCM4.3          ; CC16 assigned to Timer 0/1
CCMOD17         bit     CCM4.4          ; CC17 Mode Selection
ACC17           bit     CCM4.7          ; CC17 assigned to Timer 0/1
CCMOD18         bit     CCM4.8          ; CC18 Mode Selection
ACC18           bit     CCM4.11         ; CC18 assigned to Timer 0/1
CCMOD19         bit     CCM4.12         ; CC19 Mode Selection
ACC19           bit     CCM4.15         ; CC19 assigned to Timer 0/1

CCM5            equ     0ff24h
CCMOD20         bit     CCM5.0          ; CC20 Mode Selection
ACC20           bit     CCM5.3          ; CC20 assigned to Timer 0/1
CCMOD21         bit     CCM5.4          ; CC21 Mode Selection
ACC21           bit     CCM5.7          ; CC21 assigned to Timer 0/1
CCMOD22         bit     CCM5.8          ; CC22 Mode Selection
ACC22           bit     CCM5.11         ; CC22 assigned to Timer 0/1
CCMOD23         bit     CCM5.12         ; CC23 Mode Selection
ACC23           bit     CCM5.15         ; CC23 assigned to Timer 0/1

CCM6            equ     0ff26h
CCMOD24         bit     CCM6.0          ; CC24 Mode Selection
ACC24           bit     CCM6.3          ; CC24 assigned to Timer 0/1
CCMOD25         bit     CCM6.4          ; CC25 Mode Selection
ACC25           bit     CCM6.7          ; CC25 assigned to Timer 0/1
CCMOD26         bit     CCM6.8          ; CC26 Mode Selection
ACC26           bit     CCM6.11         ; CC26 assigned to Timer 0/1
CCMOD27         bit     CCM6.12         ; CC27 Mode Selection
ACC27           bit     CCM6.15         ; CC27 assigned to Timer 0/1

CCM7            equ     0ff28h
CCMOD28         bit     CCM7.0          ; CC28 Mode Selection
ACC28           bit     CCM7.3          ; CC28 assigned to Timer 0/1
CCMOD29         bit     CCM7.4          ; CC29 Mode Selection
ACC29           bit     CCM7.7          ; CC29 assigned to Timer 0/1
CCMOD30         bit     CCM7.8          ; CC30 Mode Selection
ACC30           bit     CCM7.11         ; CC30 assigned to Timer 0/1
CCMOD31         bit     CCM7.12         ; CC31 Mode Selection
ACC31           bit     CCM7.15         ; CC31 assigned to Timer 0/1

		DefIntBits CR,0ff6ah    ; CAPREL Interrupt Control Register

		DefIntBits CC0,0ff78h   ; Interrupt Control CAPCOM Channels
		DefIntBits CC1,0ff7ah
                DefIntBits CC2,0ff7ch
                DefIntBits CC3,0ff7eh
                DefIntBits CC4,0ff80h
                DefIntBits CC5,0ff82h
                DefIntBits CC6,0ff84h
                DefIntBits CC7,0ff86h
                DefIntBits CC8,0ff88h
                DefIntBits CC9,0ff8ah
		DefIntBits CC10,0ff8ch
		DefIntBits CC11,0ff8eh
		DefIntBits CC12,0ff90h
		DefIntBits CC13,0ff92h
		DefIntBits CC14,0ff94h
		DefIntBits CC15,0ff96h
                if      _n>=167
                 DefIntBits CC16,0f160h ; Interrupt Control futher CAPCOM Channels
                 DefIntBits CC17,0f162h
                 DefIntBits CC18,0f164h
                 DefIntBits CC19,0f166h
                 DefIntBits CC20,0f168h
                 DefIntBits CC21,0f16ah
                 DefIntBits CC22,0f16ch
                 DefIntBits CC23,0f16eh
                 DefIntBits CC24,0f170h
                 DefIntBits CC25,0f172h
                 DefIntBits CC26,0f174h
                 DefIntBits CC27,0f176h
                 DefIntBits CC28,0f178h
                 DefIntBits CC29,0f184h
                 DefIntBits CC30,0f18ch
                 DefIntBits CC31,0f194h
                endif

PECC0           equ     0fec0h          ; PEC Channel Control Register
PECC1           equ     0fec2h
PECC2           equ     0fec4h
PECC3           equ     0fec6h
PECC4           equ     0fec8h
PECC5           equ     0fecah
PECC6           equ     0fecch
PECC7           equ     0feceh

SRCP0           equ    0fce0h           ; PEC Source Register
SRCP1           equ    0fce4h
SRCP2           equ    0fce8h
SRCP3           equ    0fcech
SRCP4           equ    0fcf0h
SRCP5           equ    0fcf4h
SRCP6           equ    0fcf8h
SRCP7           equ    0fcfch
DSTP0           equ    0fce2h		; PEC Destination Register
DSTP1           equ    0fce6h
DSTP2           equ    0fceah
DSTP3           equ    0fceeh
DSTP4           equ    0fcf2h
DSTP5           equ    0fcf6h
DSTP6           equ    0fcfah
DSTP7           equ    0fcfeh

;----------------------------------------------------------------------------
; Pulse Width Modulator:

                if      _n>=167
PP0              equ     0f038h         ; PWM Period 0
PT0              equ     0f030h         ; PWM Counter 0
PW0              equ     0fe30h         ; PWM Pulse Width 0
PP1              equ     0f03ah         ; PWM Period 1
PT1              equ     0f032h         ; PWM Counter 1
PW1              equ     0fe30h         ; PWM Pulse Width 1
PP2              equ     0f03ch         ; PWM Period 2
PT2              equ     0f034h         ; PWM Counter 2
PW2              equ     0fe30h         ; PWM Pulse Width 2
PP3              equ     0f03eh         ; PWM Period 3
PT3              equ     0f036h         ; PWM Counter 3
PW3              equ     0fe30h         ; PWM Pulse Width 3
PWMCON0          equ     0ff30h         ; PWM Control Register 0
PTR0             bit     PWMCON0.0      ; PT0 Run
PTR1             bit     PWMCON0.1      ; PT1 Run
PTR2             bit     PWMCON0.2      ; PT2 Run
PTR3             bit     PWMCON0.3      ; PT3 Run
PTI0             bit     PWMCON0.4      ; PT0 Input Selection
PTI1             bit     PWMCON0.5      ; PT1 Input Selection
PTI2             bit     PWMCON0.6      ; PT2 Input Selection
PTI3             bit     PWMCON0.7      ; PT3 Input Selection
PIE0             bit     PWMCON0.8      ; PT0 Enable Interrups
PIE1             bit     PWMCON0.9      ; PT1 Enable Interrups
PIE2             bit     PWMCON0.10     ; PT2 Enable Interrups
PIE3             bit     PWMCON0.11     ; PT3 Enable Interrups
PIR0             bit     PWMCON0.12     ; PT0 Interrupt Flag
PIR1             bit     PWMCON0.13     ; PT1 Interrupt Flag
PIR2             bit     PWMCON0.14     ; PT2 Interrupt Flag
PIR3             bit     PWMCON0.15     ; PT3 Interrupt Flag
PWMCON1          equ     0ff32h         ; PWM Control Register 1
PEN0             bit     PWMCON1.0      ; PT0 Enable Output
PEN1             bit     PWMCON1.1      ; PT1 Enable Output
PEN2             bit     PWMCON1.2      ; PT2 Enable Output
PEN3             bit     PWMCON1.3      ; PT3 Enable Output
PM0              bit     PWMCON1.4      ; PT0 Mode
PM1              bit     PWMCON1.5      ; PT1 Mode
PM2              bit     PWMCON1.6      ; PT2 Mode
PM3              bit     PWMCON1.7      ; PT3 Mode
PB01             bit     PWMCON1.12     ; PT0&1 Burst Mode
PS2              bit     PWMCON1.14     ; PT2 Single Shot Mode
PS3              bit     PWMCON1.15     ; PT3 Single Shot Mode
                 DefIntBits PWM,0f17eh  ; PWM Interrupt Control
                endif

;----------------------------------------------------------------------------
; A/D-Converter:

ADDAT           equ     0fea0h          ; A/D-Converter Ergebnis
                if      _n>=167
ADDAT2           equ    0f0a0h          ; A/D-Converter Ergebnis 2
                endif
		DefIntBits ADC,0ff98h   ; A/D-Converter Interrupt Control for End of Conversion
		DefIntBits ADE,0ff9ah   ; A/D-Converter Interrupt Control for Errors
ADCON           equ     0ffa0h          ; A/D-Converter Control Register
ADCH            bit     ADCON.0         ; A/D-Converter Channel Selection
ADM             bit     ADCON.4         ; A/D-Converter Mode
ADST            bit     ADCON.7         ; Start A/D Conversion
ADBSY           bit     ADCON.8         ; A/D-Converter Busy
                if      _n>=167
ADWR             bit     ADCON.9        ; do not restart A/D-Converter before being read
ADCIN            bit     ADCON.10       ; A/D-Converter Channel Injection (??...)
ADCRQ            bit     ADCON.11       ; A/D-Converter Channel Injection Requested
                endif

;----------------------------------------------------------------------------
; Watchdog:

WDT             equ     0feaeh          ; Watchdog Counter
WDTCON          equ     0ffaeh          ; Watchdog Control Register
WDTIN           bit     WDTCON.0        ; Watchdog Input frequency
WDTR            bit     WDTCON.1        ; Reset by Watchdog
		if	_n>=167
SWR		 bit	 WDTCON.2	; Software Reset
SHWR		 bit	 WDTCON.3	; Short Hardware-Reset
LHWR		 bit	 WDTCON.4	; Long Hardware-Reset
PONR		 bit	 WDTCON.5 	; Power On Reset (Flash Variants)
		endif
WDTREL          bit     WDTCON.8        ; Reload Value Watchdog

;----------------------------------------------------------------------------
; Serielles:

S0TBUF          equ     0feb0h          ; SIO0 Transmit Register
S0RBUF          equ     0feb2h          ; SIO0 Receive Register
S0BG            equ     0feb4h          ; SIO0 Baud Rate Selection
		DefIntBits S0T,0ff6ch   ; SIO0 Interrupt Control Transmitter
		DefIntBits S0R,0ff6eh   ; SIO0 Interrupt Control Receiver 
		DefIntBits S0E,0ff70h   ; SIO0 Interrupt Control Errors

                if      _n>=167
		 DefIntBits  S0TB,0f19ch ; Interrupt Control Transmit Buffer
		endif

S0CON           equ     0ffb0h          ; SIO0 Control Register
S0M             bit     S0CON.0         ; SIO0 Mode
S0STP           bit     S0CON.3         ; SIO0 Number of Stopbits
S0REN           bit     S0CON.4         ; SIO0 Enable Receiver
S0PEN           bit     S0CON.5         ; SIO0 Enable Parity
S0FEN           bit     S0CON.6         ; SIO0 Enable Frame Check
S0OEN           bit     S0CON.7         ; SIO0 Enable Overflow Check
S0PE            bit     S0CON.8         ; SIO0 Parity Error
S0FE            bit     S0CON.9         ; SIO0 Framing Error
S0OE            bit     S0CON.10        ; SIO0 Overflow
                if      _n>=167
S0ODD            bit     S0CON.12       ; SIO0 Odd Parity
S0BRS            bit     S0CON.13       ; SIO0 Baud Rate Factor
                endif
S0LB            bit     S0CON.14        ; SIO0 Loopback
S0R             bit     S0CON.15        ; SIO0 Enable Baud Rate Generator

                if      _n>=167
SSCTB            equ     0f0b0h         ; SSC Transmit Register
SSCRB            equ     0f0b2h         ; SSC Receive Register
SSCBR            equ     0f0b4h         ; SSC Baud Rate Selection
                 DefIntBits SSCT,0ff72h ; SSC Interrupt Control Transmitter
                 DefIntBits SSCR,0ff74h ; SSC Interrupt Control Receiver
                 DefIntBits SSCE,0ff76h ; SSC Interrupt Control Errors
SSCCON           equ     0ffb2h         ; SSC Control Register
SSCBM            bit     SSCCON.0       ; Word Width
SSCBC            bit     SSCCON.0       ; Number of Bits
SSCHB            bit     SSCCON.4       ; Start Bit Control
SSCPH            bit     SSCCON.5       ; Clock Phase
SSCP0            bit     SSCCON.6       ; Clock Polarity
SSCTEN           bit     SSCCON.8       ; Enable Transmit Errors
SSCTE            bit     SSCCON.8       ; Transmit Error Flag
SSCREN           bit     SSCCON.9       ; Enable Receive Errors
SSCRE            bit     SSCCON.9       ; Receive Errot Flag
SSCPEN           bit     SSCCON.10      ; Enable Phase Errors
SSCPE            bit     SSCCON.10      ; Phase Error Flag
SSCBEN           bit     SSCCON.11      ; Enable Baud Rate Errors
SSCBE            bit     SSCCON.11      ; Baud Rate Errors Flag
SSCBSY           bit     SSCCON.12      ; SSC Busy
SSCMS            bit     SSCCON.14      ; SSC Operate as Master
SSCEN            bit     SSCCON.15      ; SSC Enable
                elseif
S1TBUF           equ     0feb8h         ; SIO1 Transmit Register
S1RBUF           equ     0febah         ; SIO1 Receive Register
S1BG             equ     0febch         ; SIO1 Baud Rate Selection
                 DefIntBits S1T,0ff72h  ; SIO1 Interrupt Control Transmitter
                 DefIntBits S1R,0ff74h  ; SIO1 Interrupt Control Receiver
                 DefIntBits S1E,0ff76h  ; SIO1 Interrupt Control Errors
S1CON            equ     0ffb8h         ; SIO1 Control Register
S1M              bit     S1CON.0        ; SIO1 Mode
S1STP            bit     S1CON.3        ; SIO1 Number of Stopbits
S1REN            bit     S1CON.4        ; SIO1 Enable Receiver
S1PEN            bit     S1CON.5        ; SIO1 Enable Parity
S1FEN            bit     S1CON.6        ; SIO1 Enable Frame Check
S1OEN            bit     S1CON.7        ; SIO1 Enable Overflow Check
S1PE             bit     S1CON.8        ; SIO1 Parity Error
S1FE             bit     S1CON.9        ; SIO1 Framing Error
S1OE             bit     S1CON.10       ; SIO1 Overflow
S1LB             bit     S1CON.14       ; SIO1 Loopback
S1R              bit     S1CON.15       ; SIO1 Enable Baud Rate Generator
                endif

;----------------------------------------------------------------------------
; Canbus 1 und 2

                if      _n>=167

;bitram;	 DefIntBits C1, 0ef02h	; CAN1 Interrupt Control Register 
C1BTR		 equ	0ef04h		; CAN1 Bit Timing Register
C1CSR		 equ	0ef00h		; CAN1 Control/Status Register
C1GMS		 equ	0ef06h		; CAN1 Global Mask Short
C1LAR1		 equ	0ef14h		; CAN1 Lower Arbitration Register
C1LAR2		 equ	0ef24h
C1LAR3		 equ	0ef34h
C1LAR4		 equ	0ef44h
C1LAR5		 equ	0ef54h
C1LAR6		 equ	0ef64h
C1LAR7		 equ	0ef74h
C1LAR8		 equ	0ef84h
C1LAR9		 equ	0ef94h
C1LAR10		 equ	0efa4h
C1LAR11		 equ	0efb4h
C1LAR12		 equ	0efc4h
C1LAR13		 equ	0efd4h
C1LAR14		 equ	0efe4h
C1LAR15		 equ	0eff4h
C1LGML		 equ	0ef0ah		; CAN1 Lower Global Mask Support Long
C1LMLM		 equ	0ef0eh		; CAN1 Lower Mask Of Last Message
C1MCFG1		 equ	0ef16h		; CAN1 Message Configuration Register
C1MCFG2		 equ	0ef26h
C1MCFG3		 equ	0ef36h
C1MCFG4		 equ	0ef46h
C1MCFG5		 equ	0ef56h
C1MCFG6		 equ	0ef66h
C1MCFG7		 equ	0ef76h
C1MCFG8		 equ	0ef86h
C1MCFG9		 equ	0ef96h
C1MCFG10	 equ	0efa6h		
C1MCFG11	 equ	0efb6h
C1MCFG12	 equ	0efc6h
C1MCFG13	 equ	0efd6h
C1MCFG14	 equ	0efe6h
C1MCFG15	 equ	0eff6h
C1MCR1		 equ	0ef10h		; CAN1 Message Control Register 
C1MCR2		 equ	0ef20h
C1MCR3		 equ	0ef30h
C1MCR4		 equ	0ef40h
C1MCR5		 equ	0ef50h
C1MCR6		 equ	0ef60h
C1MCR7		 equ	0ef70h
C1MCR8		 equ	0ef80h
C1MCR9		 equ	0ef90h
C1MCR10		 equ	0efa0h
C1MCR11		 equ	0efb0h
C1MCR12		 equ	0efc0h
C1MCR13		 equ	0efd0h
C1MCR14		 equ	0efe0h
C1MCR15		 equ	0eff0h
C1PCIR		 equ	0ef02h		; CAN1 Port Control and Interrupt Register 
C1UAR1		 equ	0ef12h		; CAN1 Upper Arbitration Register
C1UAR2		 equ	0ef22h
C1UAR3		 equ	0ef32h
C1UAR4		 equ	0ef42h
C1UAR5		 equ	0ef52h
C1UAR6		 equ	0ef62h
C1UAR7		 equ	0ef72h
C1UAR8		 equ	0ef82h
C1UAR9		 equ	0ef92h
C1UAR10		 equ	0efa2h
C1UAR11		 equ	0efb2h
C1UAR12		 equ	0efc2h
C1UAR13		 equ	0efd2h
C1UAR14		 equ	0efe2h
C1UAR15		 equ	0eff2h
C1UGML		 equ	0ef08h		; CAN1 Upper Global Mask Long
C1UMLM		 equ	0ef0ch		; CAN1 Upper Mask Of Last Message
		endif
		if	_n>=168

;bitram;	 DefIntBits C2, 0ee02h	; CAN2 Interrupt Control Register 
C2BTR		 equ	0ee04h		; CAN2 Bit Timing Register
C2CSR		 equ	0ee00h		; CAN2 Control Status Register
C2GMS		 equ	0ee06h		; CAN2 Global Mask Support
C2LAR1		 equ	0ee14h		; CAN2 Lower Arbitration Register
C2LAR2		 equ	0ee24h
C2LAR3		 equ	0ee34h
C2LAR4		 equ	0ee44h
C2LAR5		 equ	0ee54h
C2LAR6		 equ	0ee64h
C2LAR7		 equ	0ee74h
C2LAR8		 equ	0ee84h
C2LAR9		 equ	0ee94h
C2LAR10		 equ	0eea4h
C2LAR11		 equ	0eeb4h
C2LAR12		 equ	0eec4h
C2LAR13		 equ	0eed4h
C2LAR14		 equ	0eee4h
C2LAR15		 equ	0eef4h
C2LGML		 equ	0ee0ah		; CAN2 Lower Global Mask Support
C2LMLM		 equ	0ee0eh		; CAN2 Lower Mask Of Last Message
C2MCFG1		 equ	0ee16h		; CAN2 Message Configuration Register
C2MCFG2		 equ	0ee26h
C2MCFG3		 equ	0ee36h
C2MCFG4		 equ	0ee46h
C2MCFG5		 equ	0ee56h
C2MCFG6		 equ	0ee66h
C2MCFG7		 equ	0ee76h
C2MCFG8		 equ	0ee86h
C2MCFG9		 equ	0ee96h
C2MCFG10	 equ	0eea6h
C2MCFG11	 equ	0eeb6h
C2MCFG12	 equ	0eec6h
C2MCFG13	 equ	0eed6h
C2MCFG14	 equ	0eee6h
C2MCFG15	 equ	0eef6h
C2MCR1		 equ	0ee10h		; CAN2 Message Control Register
C2MCR2		 equ	0ee20h
C2MCR3		 equ	0ee30h
C2MCR4		 equ	0ee40h
C2MCR5		 equ	0ee50h
C2MCR6		 equ	0ee60h
C2MCR7		 equ	0ee70h
C2MCR8		 equ	0ee80h
C2MCR9		 equ	0ee90h
C2MCR10		 equ	0eea0h
C2MCR11		 equ	0eeb0h
C2MCR12		 equ	0eec0h
C2MCR13		 equ	0eed0h
C2MCR14		 equ	0eee0h
C2MCR15		 equ	0eef0h
C2PCIR		 equ	0ee02h		; CAN2 Port Control And Interrupt Register
C2UAR1		 equ	0ee12h		; CAN2 Upper Arbitration Register
C2UAR2		 equ	0ee22h
C2UAR3		 equ	0ee32h
C2UAR4		 equ	0ee42h
C2UAR5		 equ	0ee52h
C2UAR6		 equ	0ee62h
C2UAR7		 equ	0ee72h
C2UAR8		 equ	0ee82h
C2UAR9		 equ	0ee92h
C2UAR10		 equ	0eea2h
C2UAR11		 equ	0eeb2h
C2UAR12		 equ	0eec2h
C2UAR13		 equ	0eed2h
C2UAR14		 equ	0eee2h
C2UAR15		 equ	0eef2h
C2UGML		 equ	0ee08h		; CAN2 Upper Global Mask Support
C2UMLM		 equ	0ee0ch		; CAN2 Upper Mask Of Last Message
		endif


;----------------------------------------------------------------------------
; Vectors / Special Addresses

RESET           equ     000h            ; Reset Entry
NMITRAP         equ     008h            ; NMI Entry
STOTRAP         equ     010h            ; Entry Stack Overflow
STUTRAP         equ     018h            ; Entry Stack Underflow
BTRAP           equ     028h            ; Undefined Opcode, Protection Fault,
                                        ; Invalid Word Access, Invalid Instruction Address,
                                        ; Invalid Bus Access
CC0INT          equ     040h            ; Interrupt Entry CAPCOM
CC1INT          equ     044h
CC2INT          equ     048h
CC3INT          equ     04ch
CC4INT          equ     050h
CC5INT          equ     054h
CC6INT          equ     058h
CC7INT          equ     05ch
CC8INT          equ     060h
CC9INT          equ     064h
CC10INT         equ     068h
CC11INT         equ     06ch
CC12INT         equ     070h
CC13INT         equ     074h
CC14INT         equ     078h
CC15INT         equ     07ch
                if      _n>=167
CC16INT          equ     0c0h
CC17INT          equ     0c4h
CC18INT          equ     0c8h
CC19INT          equ     0cch
CC20INT          equ     0d0h
CC21INT          equ     0d4h
CC22INT          equ     0d8h
CC23INT          equ     0dch
CC24INT          equ     0e0h
CC25INT          equ     0e4h
CC26INT          equ     0e8h
CC27INT          equ     0ech
CC28INT          equ     0f0h
CC29INT          equ    0110h
CC30INT          equ    0114h
CC31INT          equ    0118h
		endif


T0INT           equ     080h            ; Interrupt Entry Timer 0
T1INT           equ     084h            ; Interrupt Entry Timer 1
T2INT           equ     088h            ; Interrupt Entry Timer 2
T3INT           equ     08ch            ; Interrupt Entry Timer 3
T4INT           equ     090h            ; Interrupt Entry Timer 4
T5INT           equ     094h            ; Interrupt Entry Timer 5
T6INT           equ     098h            ; Interrupt Entry Timer 6
                if      _n>=167
T7INT            equ	 0f4h            ; Interrupt Entry Timer 7
T8INT            equ	 0f8h            ; Interrupt Entry Timer 8
		endif


CRINT           equ     09ch            ; Interrupt Entry CAPREL
ADCINT          equ     0a0h            ; Interrupt Entry A/D Conversion Complete
ADEINT          equ     0a4h            ; Interrupt Entry A/D Converter Overflow
S0TINT          equ     0a8h            ; Interrupt Entry SIO0 Ready to Send
S0RINT          equ     0ach            ; Interrupt Entry SIO0 Character Reception
S0EINT          equ     0b0h            ; Interrupt Entry SIO0 Error
S1TINT          equ     0b4h            ; Interrupt Entry SIO1 Ready to Send
S1RINT          equ     0b8h            ; Interrupt Entry SIO1 Character Reception
S1EINT          equ     0bch            ; Interrupt Entry SIO1 Error
                if      _n>=167
S0TBINT		 equ	011ch		; ASC0 Transmit Buffer
PWMINT		 equ	0fch		; PWM Channels 0..3
XP0INT		 equ	0100h		; CAN1
XP1INT		 equ	0104h		; CAN2
XP2INT		 equ	0108h		; Unassigned Node
XP3INT		 equ	010ch		; PLL,OWD,RTC
		endif


;----------------------------------------------------------------------------
; Memory Ranges

		switch 	 _n
		case	168
IRAM            equ     0f200h		; Start of Internal RAM
		case	167
IRAM            equ     0f600h
		elsecase
IRAM            equ     0fa00h
		endcase	
IRAMEND         equ     0fdffh          ; End   "   "      "


BITRAM          equ     0fd00h          ; Start of Bit Addressable IRAM Area
BITRAMEND       equ     0fdffh          ; End   "   "       "        "   "

		if	_n>=167
BITRAM1           equ     0f100h         ; Start of Bit Addressable ESFR Area
BITRAM1END        equ     0f1ffh         ; End   "   "     "          "   "
BITRAM2           equ     0ff00h         ; Start of Bit Addressable SFR Area
BITRAM2END        equ     0ffffh         ; End   "   "     "          "   "
		endif


                if      _n>=167
PECPTR           equ     0fce0h         ; Start of PEC-Vektoren (optional)
PECPTREND        equ     0fcffh         ; End   "     "
                elseif
PECPTR           equ     0fde0h
PECPTREND        equ     0fdffh
                endif

SFRSTART         equ     0fe00h         ; Start of SFR-Bereich
SFREND           equ     0ffffh         ; End SFR-Bereich
	
                if      _n>=167
ESFRSTART        equ     0f000h         ; Start of Extended SFR Area
ESFREND          equ     0f1ffh         ; End of Extended SFR Area
                endif

IROM            equ     0               ; Start of Internal ROM
                if      _n>=167
IROMEND          equ     07fffh         ; End of Internal ROM (not fully populated)
                elseif
IROMEND          equ     01fffh
                endif
		
		if	_n>=168
EEPROM		 equ	 08000h		; Start of Internal 4 KByte EEprom
EEPROMEND	 equ	 08fffh		; End of Internal EEprom
		endif
		if	_n>=167
XRAM1		 equ	 0e000h		; Internal XRAM 2 KByte Start
XRAM1END	 equ	 0e7ffh		; Internal XRAM 2 KByte End
		endif
		if	_n>=168
XRAM2		 equ	 0c000h		; Internal XRAM 6 KByte Start
XRAM2END	 equ	 0d7ffh		; Internal XRAM 6 KByte End
		endif
                

;----------------------------------------------------------------------------
; Bequemlichkeitsmakros

clr             macro   op              ; Set Operand to 0
                and     op,#0
                endm

bchg            macro   op              ; Invert Bit
                bmovn   op,op
                endm

dec             macro   op              ; Decrement
                sub     op,#1
                endm

inc             macro   op              ; Increment
                add     op,#1
                endm

swapb           macro   op              ; Swap Bytes
                ror     op,#8
                endm

;----------------------------------------------------------------------------

		restore                 ; enable listing again

		endif			; reg166inc
