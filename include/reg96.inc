		save
                listing off   ; kein Listing Åber diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.40 - Datei REG96.INC                                              *
;*   								            *
;*   Sinn : enthÑlt Makro-, SFR- und Adre·definitionen fÅr die              *
;*          MCS-96-Prozessoren                                              *
;*          OK, der Name fÑllt aus dem Rahmen, aber STDDEF96.INC war schon  *
;*          belegt...                                                       *
;* 									    *
;*   letzte énderungen : 20. 2.1994                                         *
;*                       28. 6.1994 80196 (KR) hinzugefÅgt                  *
;*                                                                          *
;****************************************************************************

		ifndef  reg96inc     	; verhindert Mehrfacheinbindung

reg96inc     	equ     1

		if      (MomCPU<>32918)&&(MomCPU<>524694)
		 fatal  "Falscher Prozessortyp eingestellt: nur 8096 oder 80196 erlaubt!"
		endif


		if      MomPass=1
		 message "MCS-96-SFR-Definitionen (C) 1994 Alfred Arnold"
		 message "binde \{MOMCPU}-SFRs ein"
		endif

;----------------------------------------------------------------------------
; Spezialregister:

R0		equ	0		; Nullregister
SP    		equ	18h		; Stackpointer
CCB		equ	2018h		; Chip-Konfiguration
                if      MomCPU=80196h
CCB1             equ     201ah          ; erw. Chip-Konfiguration
WSR              equ     14h            ; RAM-Window in Registerbank
USFR             equ     1ff6h          ; ???
                endif

;----------------------------------------------------------------------------
; SFRs:

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; A/D-Wandler
                if      MomCPU=80196h
AD_Result        equ     1faah          ; Ergebnis Wandlung (wortweise lesbar)
AD_Command       equ     1fach          ; Steuerung AD-Wandler
AD_Test          equ     1faeh          ; Justage
AD_Time          equ     1fafh          ; Wandlungsgeschwindigkeit
                elseif
AD_Command       equ     2              ; A/D-Wandler Steuerregister
AD_Result_Lo     equ     2              ; Ergebnis A/D-Wandler
AD_Result_Hi     equ     3              ; NUR byteweise lesen!!
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; HSI/HSO/EPA

                if      MomCPU=80196h
EPA_Mask         equ     1fa0h          ; (w) EPA Interruptfreigabe 4..9, 0..1, overrun 0..9/0/1
EPA_Mask1        equ     1fa4h
EPA_Pend         equ     1fa2h          ; (w) EPA Interrupt aufgetreten 4..9, 0..1, overrun 0..9/0/1
EPA_Pend1        equ     1fa6h
EPAIpv           equ     1fa8h          ; PrioritÑt EPA-Interrupt

EPA_Control0     equ     1f60h          ; Steuerung Capture/Compare Kanal 0..9
EPA_Time0        equ     1f62h          ; (w) gespeicherter Zeitpunkt Kanal 0..9
EPA_Control1     equ     1f64h
EPA_Time1        equ     1f66h
EPA_Control2     equ     1f68h
EPA_Time2        equ     1f6ah
EPA_Control3     equ     1f6ch
EPA_Time3        equ     1f6eh
EPA_Control4     equ     1f70h
EPA_Time4        equ     1f72h
EPA_Control5     equ     1f74h
EPA_Time5        equ     1f76h
EPA_Control6     equ     1f78h
EPA_Time6        equ     1f7ah
EPA_Control7     equ     1f7ch
EPA_Time7        equ     1f7eh
EPA_Control8     equ     1f80h
EPA_Time8        equ     1f82h
EPA_Control9     equ     1f84h
EPA_Time9        equ     1f86h
Comp_Control0    equ     1f88h          ; Steuerung Compare-Kanal 0..1
Comp_Time0       equ     1f8ah          ; Vergleichswert Compare-Kanal 0..1
Comp_Control1    equ     1f8ch
Comp_Time1       equ     1f8eh
                elseif
HSI_Time         equ     4              ; HSI Triggerzeitpunkt (nur Wort)
HSI_Mode         equ     3              ; HSI Modusregister
HSI_Status       equ     6              ; HSI Statusregister

HSO_Time         equ     4              ; HSO Zeiteinstellung (nur Wort)
HSO_Command      equ     6              ; HSO Steuerregister
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; serielles

                if      MomCPU=80196h
SSIO_StB0        equ     1fb0h          ; Datenpuffer SSIO Kanal 0
SSIO_StCr0       equ     1fb1h          ; Steuerung Sender/EmpfÑnger SSIO Kanal 0
SSIO_StB1        equ     1fb2h          ; Datenpuffer SSIO Kanal 1
SSIO_StCr1       equ     1fb3h          ; Steuerung Sender/EmpfÑnger SSIO Kanal 1
SSIO_Baud        equ     1fb4h          ; Baudrate SSIO
SBuf_RX          equ     1fb8h          ; Empfangsdaten
SP_Stat          equ     1fb9h          ; Zustand SIO
SBuf_TX          equ     1fbah          ; Sendedaten
SP_Con           equ     1fbbh          ; Steuerung SIO
SP_Baud          equ     1fbch          ; (w) Baudrate SIO
                elseif
SBUF             equ     7              ; UART Datenregister
SPStat           equ     11h            ; UART Statusregister
SPCon            equ     11h            ; UART Steuerregister
Baud_Reg         equ     0eh            ; UART Datenrate
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Interruptsteuerung

                if      MomCPU=80196h
Int_Mask_Lo      equ     08h            ; Interruptfreigabe
Int_Mask_Hi      equ     13h
Int_Pend_Lo      equ     09h            ; wartende Interrupts
Int_Pend_Hi      equ     12h
PTS_Srv          equ     06h            ; w PTS-Interrupt bedient
PTS_Select       equ     04h            ; w Quellen PTS freigeben
                elseif
INT_Mask         equ     8              ; Interruptfreigabe
INT_Pending      equ     9              ; Interrupts aufgetreten ?
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Timer

Watchdog	equ	0ah		; Watchdog zurÅcksetzen

                if      MomCPU=80196h
Timer1_Control   equ     1f98h          ; Steuerung Timer 1
Timer1           equ     1f9ah          ; (w) Daten Timer 1
Timer2_Control   equ     1f9ch          ; Steuerung Timer 2
Timer2           equ     1f9eh          ; (w) Daten Timer 2
                elseif
Timer1           equ     0ah            ; Timer 1 (nur Wort)
Timer2           equ     0ch            ; Timer 2 (nur Wort)
                endif

                if      MomCPU=8096h
PWM_Control      equ     17h            ; Einstellung Pulsweite PWM
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Ports

                if      MomCPU=80196h
P0PIn            equ     1fdah          ; Daten Port 0

P1SSel           equ     1fd0h          ; Spezialfunktionen auf Port 1 ?
P1IO             equ     1fd2h          ; Port 1 Datenrichtungsregister
P1Reg            equ     1fd4h          ; Port 1 Datenausgabe
P1PIn            equ     1fd6h          ; Eingangszustand Port 1

P2SSel           equ     1fc9h          ; Rest sinngemÑ·
P2IO             equ     1fcbh
P2Reg            equ     1fcdh
P2PIn            equ     1fcfh

P3Reg            equ     1ffch
P3PIn            equ     1ffeh

P4Reg            equ     1ffdh
P4PIn            equ     1fffh

P5SSel           equ     1ff1h
P5IO             equ     1ff3h
P5Reg            equ     1ff5h
P5PIn            equ     1ff7h

P6SSel           equ     1fd1h
P6IO             equ     1fd3h
P6Reg            equ     1fd5h
P6PIn            equ     1fd7h
                elseif
Port0            equ     0eh            ; Port 0
Port1            equ     0fh            ; Port 1
Port2            equ     10h            ; Port 2
Port3            equ     1ffeh          ; Port 3 (falls kein ext. Speicher)
Port4            equ     1fffh          ; Port 4 (falls kein ext. Speicher)

IOS0             equ     15h            ; I/O-Statusregister 0
IOS1             equ     16h            ; I/O-Statusregister 1
IOC0             equ     15h            ; I/O-Statusregister 0
IOC1             equ     16h            ; I/O-Statusregister 1
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -
; Slave-Port

                if      MomCPU=8096h
SlpStat          equ     1ff8h          ; Status Slave-Port
SlpCmd           equ     1ffah          ; Kommandoregister Slave-Port
SlpFunReg        equ     1ffbh          ; Slave-Port Steuerung
                endif

;----------------------------------------------------------------------------
; Vektoren/Sonderadressen

                if      MomCPU=80196h
EPAIntX_Vec      equ     2000h          ; Kanal 4...9/0/1 EPA
EPAInt3_Vec      equ     2002h          ; Kanal 3 EPA
EPAInt2_Vec      equ     2004h          ; Kanal 2 EPA
EPAInt1_Vec      equ     2006h          ; Kanal 1 EPA
EPAInt0_Vec      equ     2008h          ; Kanal 0 EPA
AD_Complete_Vec  equ     200ah          ; A/D-Wandlung abgeschlossen
OBE_Slp_Vec      equ     200ch          ; Ausgabepuffer leer
IBF_Slp_Vec      equ     200eh          ; Eingabepuffer voll
Trap_Vec         equ     2010h          ; Software-Trap
Ill_Opcode_Vec   equ     2012h          ; undefinierter Opcode
CMD_Full_Vec     equ     2030h          ; Kommandopuffer voll
SSIO0_Trans_Vec  equ     2032h          ; Transfer SSIO0 abgeschlossen
SSIO1_Trans_Vec  equ     2034h          ; Transfer SSIO1 abgeschlossen
SIO_TrInt_Vec    equ     2036h          ; SIO hat Zeichen versandt
SIO_RcInt_Vec    equ     2038h          ; SIO hat Zeichen empfangen
ExtInt_Vec       equ     203ch          ; externer Interrupt
NMI_Vec          equ     203eh          ; nicht mask. Interrupt
                elseif
TOverInt_Vec     equ     2000h          ; Vektor TimerÅberlauf
ADInt_Vec        equ     2002h          ; Vektor A/D-Wandlung beendet
HSIDataInt_Vec   equ     2004h          ; Vektor HSI-Daten verfÅgbar
HSOInt_Vec       equ     2006h          ; Vektor HSO
HSI0Int_Vec      equ     2008h          ; Vektor HSI.0
TimeInt_Vec      equ     200ah          ; Vektor fÅr Timer
SerialInt_Vec    equ     200ch          ; Vektor serieller Interrupt
ExtInt_Vec       equ     200eh          ; Vektor externer Interrupt
SoftInt_Vec      equ     2010h          ; TRAP Interruptvektor
Self_Jump        equ     201ah          ; Endlosschleife
                endif

Security_Key    equ     2020h           ; SchlÅssel
Security_Key_End equ	Security_Key+15

Reset_Location	equ	2080h		; Startadresse nach Reset

;----------------------------------------------------------------------------
; Speicherbereiche

IRAM		equ	1ah		; Beginn internes RAM
                if      MomCPU=80196h
IRAMEND          equ     1ffh           ; Ende internes RAM
PDRAM            equ     IRAM           ; Beginn Power-Down-RAM
PDRAMEND         equ     1ffh           ; Ende Power-Down-RAM
CODERAM          equ     400h           ; Beginn Code-RAM (KEIN Windowing!)
CODERAMEND       equ     4ffh           ; Ende Code-RAM
                elseif
IRAMEND          equ     0ffh           ; Ende internes RAM
PDRAM            equ     0f0h           ; Beginn Power-Down-RAM
PDRAMEND         equ     0ffh           ; Ende Power-Down-RAM
                endif
IROM            equ     2080h           ; Beginn internes Programm-ROM
IROMEND		equ	3fffh		; Ende internes Programm-ROM
EXTMEM1		equ	100h		; Beginn externer Speicher Bereich 1
EXTMEM1END	equ	1ffdh		; Ende externer Speicher Bereich 1
EXTMEM2		equ	4000h		; Beginn externer Speicher Bereich 2
EXTMEM2END	equ	0ffffh		; Ende externer Speicher Bereich 2

		endif

;--------------------------------------------------------------------------
; lange SprÅnge

__DefLongJmp    macro   new,old
new             macro   Adr,{NoExpand}
                old     Skip
                ljmp    Adr
Skip:
                endm
                endm

                __DefLongJmp bc,jnc
                __DefLongJmp be,jne
                __DefLongJmp bge,jlt
                __DefLongJmp bgt,jle
                __DefLongJmp bh,jnh
                __DefLongJmp ble,jgt
                __DefLongJmp blt,jge
                __DefLongJmp bnc,jc
                __DefLongJmp bne,je
                __DefLongJmp bnh,jh
                __DefLongJmp bnst,jst
                __DefLongJmp bnv,jv
                __DefLongJmp bnvt,jvt
                __DefLongJmp bst,jnst
                __DefLongJmp bv,jnv
                __DefLongJmp bvt,jnvt

		restore                 ; wieder erlauben

