                ifndef  stddef75inc     ; avoid multiple inclusion
stddef75inc     equ     1

                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File STDDEF75.INC                                            *
;*   								            *
;*   Contains SFR Definitions for the 75K0 Family                           *
;* 									    *
;****************************************************************************

;----------------------------------------------------------------------------
; For Comfort

__message       macro   msg,{NoExpand}
                if      MOMPASS=1
                 message msg
                endif
                endm

                __message "uPD75K0 Register Definitions, (C) 1994 Alfred Arnold"

;----------------------------------------------------------------------------
; Die Prozessoren zu Gruppen zusammenfassen

                switch  MOMCPU
                case    480258
                 __message "Including uPD75402 Registers"
__family         equ     400

                case    479236,479238,479240
                 __message "Including uPD750xx Registers"
__family         equ     000

                case     479848
                 __message "Including uPD75268 Registers"
__family         equ     260

                case     480004,480006,480008,480018,480022
                 __message "Including uPD753xx Registers"
__family         equ     300

                case    480040
                 __message "Including uPD75328 Registers"
__family         equ     320

                case    479492,479494,479496,479506,479510
                 __message "Including uPD751xx Registers"
__family         equ     100

                case    479750,479752,479762,479766
                 __message "Including uPD752xx Registers"
__family         equ     200

                case    480530,480534
                 __message "Including uPD755xx Registers"
__family         equ     500

                elsecase
                 fatal   "error: no target from uPD75xxx family selected"
                endcase

;----------------------------------------------------------------------------
; all Interrupt Register have same structure, so use a macro:

__defint        macro   NAME,base,{NoExpand}
__tmpnam        set     "NAME"
I{__tmpnam}     sfr     base
IE{__tmpnam}    bit     base.1
IRQ{__tmpnam}   bit     base.0
                endm

;----------------------------------------------------------------------------
; gemeinsame Register:

SP               sfr     0f80h          ; [8W] Stack Pointer
BTM              sfr     0f85h          ; [4W] Base Timer Mode
BT               sfr     0f86h          ; [8R] Bas Timer Count Value
IM0              sfr     0fb4h          ; [4W] INT0 Mode Register
                 __defint BT,0fb8h      ; [4]  Interrupt BT Enable/Status
PORT0            sfr     0ff0h          ; [4R] Data Register Port 0
PORT1            sfr     0ff1h          ; [4R] Data Register Port 1
PORT2            sfr     0ff2h          ; [4]  Data Register Port 2
PORT3            sfr     0ff3h          ; [4]  Data Register Port 3
PORT5            sfr     0ff5h          ; [4]  Data Register Port 5
PORT6            sfr     0ff6h          ; [4]  Data Register Port 6

RESET            label   0000h          ; Reset Vector
VIBT             label   0002h          ; Interrupt Vector Address INTBT
                                        ; partially also INT4
VI0              label   0004h          ; Interrupt Vector Address INT0
                                        ; partially also INT1

;----------------------------------------------------------------------------

                if      __family=400
IME              bit     0fb2h.3        ; [8]  Interrupt Disable (access via EI/DI)
PCC              sfr     0fb3h          ; [4W] Steuerung Prozessortakt
                 __defint CSI,0fbdh     ; [4]  Interrupt CSI Enable/Status
                 __defint 0,0fbeh       ; [4]  Interrupt 0 Enable/Status
                 __defint 2,0fbeh       ; [4]  Interrupt 2 Enable/Status
CLOM             sfr     0fd0h          ; [4W] Clock Output Mode Register
POGA             sfr     0fdch          ; [8W] Pull-Up Cotrol Port A
CSIM             sfr     0fe0h          ; [8W] Serial Interface Operation Mode
CSIE             bit     CSIM+1.3
COI              bit     CSIM+1.2
WUP              bit     CSIM+1.1
SBIC             sfr     0fe2h          ; [1]  SBI Control
CMDD             bit     SBIC.3
RELD             bit     SBIC.2
CMDT             bit     SBIC.1
RELT             bit     SBIC.0
BSYE             bit     SBIC+1.3
ACKD             bit     SBIC+1.2
ACKE             bit     SBIC+1.1
ACKT             bit     SBIC+1.0
SIO              sfr     0fe4h          ; [8]  SIO Data Register
SVA              sfr     0fe6h          ; [8W] Node Address on Serial Bus
PMGA             sfr     0fe8h          ; [8W] Port Operation Mode
PMGB             sfr     0fech          ; [8W]      "        "

VICSI            label   0008h          ; INTCSI Interrupt Vector Address

RAMEnd           sfr     64             ; RAM Size
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                if      __family=000
WM               sfr     0f98h          ; [8]  Watchdog Mode
TM0              sfr     0fa0h          ; [8]  Timer 0 Mode
TOE0             bit     0fa2h.3        ; [1W] Timer 0 Output Enable
T0               sfr     0fa4h          ; [8R] Timer 0 Count Value
TMOD0            sfr     0fa6h          ; [8W] Timer 0 Modulo Register
PSW              sfr     0fb0h          ; [4]  Processor Status Word
IST0             bit     PSW.2
MBE              bit     PSW.1
IME              bit     0fb2h.3        ; [8]  Interrupt Disable (access via EI/DI)
PCC              sfr     0fb3h          ; [4W] Processor Clock Control
IM1              sfr     0fb5h          ; [4W] INT1 Mode Register
IM2              sfr     0fb6h          ; [4W] INT2 Mode Register
SCC              sfr     0fb7h          ; [1W] System Clock Control
IE4              bit     IBT.3          ;      Enable/Status Interrupt 4
IRQ4             bit     IBT.2
                 __defint W,0fbah       ; [4]  W Interrupt Enable/Status
                 __defint T0,0fbch      ; [4]  T0 Interrupt Enable/Status
                 __defint CSI,0fbdh     ; [4]  CSI Interrupt Enable/Status
                 __defint 0,0fbeh       ; [4]  Interrupt 0 Enable/Status Interrupt 0
IE1              bit     I0.3           ;      Interrupt 1 Enable/Status Interrupt 1
IRQ1             bit     I0.2
                 __defint 2,0fbfh       ; [4]  Interrupt 2 Enable/Status
BSB0             sfr     0fc0h          ; [4]  Bit Sequential Buffers
BSB1             sfr     0fc1h
BSB2             sfr     0fc2h
BSB3             sfr     0fc3h
CLOM             sfr     0fd0h          ; [4W] Clock Output Mode Register
POGA             sfr     0fdch          ; [8W] Port A Pull-Up Control
POGB             sfr     0fdeh          ; [8W] Port B Pull-Up Control
CSIM             sfr     0fe0h          ; [8W] Serial Interface Operation Mode
CSIE             bit     CSIM+1.3
COI              bit     CSIM+1.2
WUP              bit     CSIM+1.1
SBIC             sfr     0fe2h          ; [1]  SBI Control
CMDD             bit     SBIC.3
RELD             bit     SBIC.2
CMDT             bit     SBIC.1
RELT             bit     SBIC.0
BSYE             bit     SBIC+1.3
ACKD             bit     SBIC+1.2
ACKE             bit     SBIC+1.1
ACKT             bit     SBIC+1.0
SIO              sfr     0fe4h          ; [8]  Data Register SIO
SVA              sfr     0fe6h          ; [8W] Node Address on Serial Bus
PMGA             sfr     0fe8h          ; [8W] Port Operation Mode
PM33             bit     PMGA.3
PM32             bit     PMGA.2
PM31             bit     PMGA.1
PM30             bit     PMGA.0
PM63             bit     PMGA+1.3
PM62             bit     PMGA+1.2
PM61             bit     PMGA+1.1
PM60             bit     PMGA+1.0
PMGB             sfr     0fech          ; [8W]      "        "
PM2              bit     PMGB.2
PM4              bit     PMGB+1.0
PM5              bit     PMGB+1.1
PM7              bit     PMGB+1.3
PMGC             sfr     0feeh          ; [8W]      "        "
PM8              bit     PMGC.0
PORT4            sfr     0ff4h          ; [4]  Data Register Port 4
KR0              sfr     PORT6.0
KR1              sfr     PORT6.1
KR2              sfr     PORT6.2
KR3              sfr     PORT6.3
PORT7            sfr     0ff7h          ; [4]  Data Register Port 7
KR4              sfr     PORT7.0
KR5              sfr     PORT7.1
KR6              sfr     PORT7.2
KR7              sfr     PORT7.3
PORT8            sfr     0ff8h          ; [4]  Data Register Port 8

VI1              label   0006h          ; INT1 Vector Address
VICSI            label   0008h          ; INTCSI Vector Address
VIT0             label   000ah          ; INTT0 Vector Address

RAMEnd           sfr     512            ; RAM Size
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                if      __family=260
DSPM             sfr     0f88h          ; [4W] Display Mode
DIMS             sfr     0f89h          ; [4W] Display Dimmer Setting
DIGS             sfr     0f8ah          ; [4]  Display Number of Digits Selection
KSF              bit     DIGS.3
WM               sfr     0f98h          ; [8]  Watchdog Mode
TM0              sfr     0fa0h          ; [8]  Timer 0 Mode
T0               sfr     0fa4h          ; [8R] Timer 0 Count Value
TMOD0            sfr     0fa6h          ; [8W] Timer 0 Modulo Register
PSW              sfr     0fb0h          ; [4]  Processor Status Word
IST0             bit     PSW.2
MBE              bit     PSW.1
IME              bit     0fb2h.3        ; [8]  Interrupt Disable (access via EI/DI)
PCC              sfr     0fb3h          ; [4W] Processor Clock Control
IM1              sfr     0fb5h          ; [4W] INT1 Mode Register
SCC              sfr     0fb7h          ; [1W] System Clock Control
IE4              bit     IBT.3          ;      Interrupt 4 Enable/Status
IRQ4             bit     IBT.2
                 __defint W,0fbah       ; [4]  W Interrupt Enable/Status
IEKS             bit     0fbbh.3        ; [1]  Keyboard Interrupt Enable/Status
IRQKS            bit     0fbbh.2
                 __defint T0,0fbch      ; [4]  T0 Interrupt Enable/Status
                 __defint SIO,0fbdh     ; [4]  SIO Interrupt Enable/Status
                 __defint 0,0fbeh       ; [4]  Interrupt 0 Enable/Status
IE1              bit     I0.3           ;      Interrupt 1 Enable/Status
IRQ1             bit     I0.2
                 __defint 2,0fbfh       ; [4]  Interrupt 2 Enable/Status
SIOM             sfr     0fe0h          ; [8W] Serial Interface Operation Mode
SIO              sfr     0fe4h          ; [8]  SIO Data Register
PMGA             sfr     0fe8h          ; [8W] Port Operation Mode
PM33             bit     PMGA.3
PM32             bit     PMGA.2
PM31             bit     PMGA.1
PM30             bit     PMGA.0
PM63             bit     PMGA+1.3
PM62             bit     PMGA+1.2
PM61             bit     PMGA+1.1
PM60             bit     PMGA+1.0
PMGB             sfr     0fech          ; [8W]      "        "
PM2              bit     PMGB.2
PM4              bit     PMGB+1.0
PM5              bit     PMGB+1.1
PORT4            sfr     0ff4h          ; [4]  Port 4 Data Register
SSTART           sfr     01c0h          ; Start of Display Memory
KS0              sfr     01fch          ; [8]  Keyboard Register
KS1              sfr     01feh          ; [4]
PORTH            sfr     01ffh          ; [4]  Port H Data Register

VI1              label   0006h          ; INT1 Interrupt Vector Address
VISIO            label   0008h          ; INTSIO Interrupt Vector Address
VIT0             label   000ah          ; INTT0 Interrupt Vector Address
VIKS             label   000eh          ; INTKS Interrupt Vector Address

RAMEnd           sfr     512            ; RAM Size
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                if      __family=300
LCDM             sfr     0f8ch          ; [8W] LC-Display Mode
LCDC             sfr     0f8eh          ; [4W] LC-Display Control
WM               sfr     0f98h          ; [8]  Watchdog Mode
TM0              sfr     0fa0h          ; [8]  Timer 0 Mode
TOE0             bit     0fa2h.3        ; [1W] Timer 0 Output Enable
T0               sfr     0fa4h          ; [8R] Timer 0 Count Value
TMOD0            sfr     0fa6h          ; [8W] Timer 0 Modulo Register
PSW              sfr     0fb0h          ; [4]  Processor Status Word
IST0             bit     PSW.2
MBE              bit     PSW.1
IME              bit     0fb2h.3        ; [8]  Interrupt Disable (access via EI/DI)
PCC              sfr     0fb3h          ; [4W] Processor Clock Control
IM1              sfr     0fb5h          ; [4W] INT1 Mode Register
IM2              sfr     0fb6h          ; [4W] INT2 Mode Register
SCC              sfr     0fb7h          ; [1W] System Clock Control
IE4              bit     IBT.3          ;      Interrupt 4 Enable/Status
IRQ4             bit     IBT.2
                 __defint W,0fbah       ; [4]  W Interrupt Enable/Status
                 __defint T0,0fbch      ; [4]  T0 Interrupt Enable/Status
                 __defint CSI,0fbdh     ; [4]  CSI Interrupt Enable/Status
                 __defint 0,0fbeh       ; [4]  Interrupt 0 Enable/Status
IE1              bit     I0.3           ;      Interrupt 1 Enable/Status
IRQ1             bit     I0.2
                 __defint 2,0fbfh       ; [4]  Interrupt 2 Enable/Status
BSB0             sfr     0fc0h          ; [4]  Bit Sequential Buffers
BSB1             sfr     0fc1h
BSB2             sfr     0fc2h
BSB3             sfr     0fc3h
CLOM             sfr     0fd0h          ; [4W] Clock Output Mode Register
POGA             sfr     0fdch          ; [8W] Port A Pull-Up Control
CSIM             sfr     0fe0h          ; [8W] Serial Interface Operation Mode
CSIE             bit     CSIM+1.3
COI              bit     CSIM+1.2
WUP              bit     CSIM+1.1
SBIC             sfr     0fe2h          ; [1]   SBI Control
CMDD             bit     SBIC.3
RELD             bit     SBIC.2
CMDT             bit     SBIC.1
RELT             bit     SBIC.0
BSYE             bit     SBIC+1.3
ACKD             bit     SBIC+1.2
ACKE             bit     SBIC+1.1
ACKT             bit     SBIC+1.0
SIO              sfr     0fe4h          ; [8]  SIO Data Register
SVA              sfr     0fe6h          ; [8W] Node Address on Serial Bus
PMGA             sfr     0fe8h          ; [8W] Port Operation Mode
PM33             bit     PMGA.3
PM32             bit     PMGA.2
PM31             bit     PMGA.1
PM30             bit     PMGA.0
PM63             bit     PMGA+1.3
PM62             bit     PMGA+1.2
PM61             bit     PMGA+1.1
PM60             bit     PMGA+1.0
PMGB             sfr     0fech          ; [8W]      "        "
PM2              bit     PMGB.2
PM4              bit     PMGB+1.0
PM5              bit     PMGB+1.1
PM7              bit     PMGB+1.3
PORT4            sfr     0ff4h          ; [4]  Port 4 Data Register
KR0              sfr     PORT6.0
KR1              sfr     PORT6.1
KR2              sfr     PORT6.2
KR3              sfr     PORT6.3
PORT7            sfr     0ff7h          ; [4]  Port 7 Data Register
KR4              sfr     PORT7.0
KR5              sfr     PORT7.1
KR6              sfr     PORT7.2
KR7              sfr     PORT7.3

VI1              label   0006h          ; INT1 Interrupt Vector Address
VICSI            label   0008h          ; INTCSI Interrupt Vector Address
VIT0             label   000ah          ; INTT0 Interrupt Vector Address

RAMEnd           sfr     512            ; RAM Size
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                if      __family=320
LCDM             sfr     0f8ch          ; [8W] LC-Display Mode
LCDC             sfr     0f8eh          ; [4W] LC-Display Control
WM               sfr     0f98h          ; [8]  Watchdog Mode
TM0              sfr     0fa0h          ; [8]  Timer 0 Mode
TOE0             bit     0fa2h.3        ; [1W] Timer 0 Output Enable
T0               sfr     0fa4h          ; [8R] Timer 0 Count Value
TMOD0            sfr     0fa6h          ; [8W] Timer 0 Modulo Register
PSW              sfr     0fb0h          ; [4]  Processor Status Word
IST0             bit     PSW.2
MBE              bit     PSW.1
IME              bit     0fb2h.3        ; [8]  Interrupt Disable (access via EI/DI)
PCC              sfr     0fb3h          ; [4W] Processor Clock Control
IM1              sfr     0fb5h          ; [4W] INT1 Mode Register
IM2              sfr     0fb6h          ; [4W] INT2 Mode Register
SCC              sfr     0fb7h          ; [1W] System Clock Control
IE4              bit     IBT.3          ;      Interrupt 4 Enable/Status
IRQ4             bit     IBT.2
                 __defint W,0fbah       ; [4]  W Interrupt Enable/Status
                 __defint T0,0fbch      ; [4]  T0 Interrupt Enable/Status
                 __defint CSI,0fbdh     ; [4]  CSI Interrupt Enable/Status
                 __defint 0,0fbeh       ; [4]  Interrupt 0 Enable/Status
IE1              bit     I0.3           ;      Interrupt 1 Enable/Status
IRQ1             bit     I0.2
                 __defint 2,0fbfh       ; [4]  Interrupt 2 Enable/Status
BSB0             sfr     0fc0h          ; [4]  Bit Sequential Buffers
BSB1             sfr     0fc1h
BSB2             sfr     0fc2h
BSB3             sfr     0fc3h
CLOM             sfr     0fd0h          ; [4W] Clock Output Mode Register
ADM              sfr     0fd8h          ; [1]  A/D Converter Control
SOC              sfr     ADM.3
EOC              sfr     ADM.2
SA               sfr     0fdah
POGA             sfr     0fdch          ; [8W] Port A Pull-Up Control
POBG             sfr     0fdeh          ; [8W] Port B Pull-Up Control
CSIM             sfr     0fe0h          ; [8W] Serial Interface Operation Mode
CSIE             bit     CSIM+1.3
COI              bit     CSIM+1.2
WUP              bit     CSIM+1.1
SBIC             sfr     0fe2h          ; [1]   SBI Control
CMDD             bit     SBIC.3
RELD             bit     SBIC.2
CMDT             bit     SBIC.1
RELT             bit     SBIC.0
BSYE             bit     SBIC+1.3
ACKD             bit     SBIC+1.2
ACKE             bit     SBIC+1.1
ACKT             bit     SBIC+1.0
SIO              sfr     0fe4h          ; [8]  SIO Data Register
SVA              sfr     0fe6h          ; [8W] Node Address on Serial Bus
PMGA             sfr     0fe8h          ; [8W] Port Operation Mode
PM33             bit     PMGA.3
PM32             bit     PMGA.2
PM31             bit     PMGA.1
PM30             bit     PMGA.0
PM63             bit     PMGA+1.3
PM62             bit     PMGA+1.2
PM61             bit     PMGA+1.1
PM60             bit     PMGA+1.0
PMGB             sfr     0fech          ; [8W]      "        "
PM2              bit     PMGB.2
PM4              bit     PMGB+1.0
PM5              bit     PMGB+1.1
PM7              bit     PMGB+1.3
PMGC             sfr     0feeh          ; [8W]      "        "
PORT4            sfr     0ff4h          ; [4]  Port 4 Data Register
KR0              sfr     PORT6.0
KR1              sfr     PORT6.1
KR2              sfr     PORT6.2
KR3              sfr     PORT6.3
PORT7            sfr     0ff7h          ; [4]  Port 7 Data Register
KR4              sfr     PORT7.0
KR5              sfr     PORT7.1
KR6              sfr     PORT7.2
KR7              sfr     PORT7.3
PORT8            sfr     0ff8h          ; [4]  Port 8 Data Register

VI1              label   0006h          ; INT1 Interrupt Vector Address
VICSI            label   0008h          ; INTCSI Interrupt Vector Address
VIT0             label   000ah          ; INTT0 Interrupt Vector Address

RAMEnd           sfr     512            ; RAM Size
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                if      __family=100
TM0              sfr     0fa0h          ; [8]  Timer 0 Mode
TOE0             bit     0fa2h.3        ; [1W] Timer 0 Output Enable
TO0              bit     0fa2h.2        ; [1W] Timer 0 Output
TOF0             bit     0fa2h.1        ; [1W] Timer 0 Output Flip Flop
TI0              bit     0fa2h.0        ; [1W] Timer 0 Input
T0               sfr     0fa4h          ; [8R] Timer 0 Count Value
TMOD0            sfr     0fa6h          ; [8W] Timer 0 Modulo Register
TM1              sfr     0fa8h          ; [8]  Timer 1 Mode
TOE1             bit     0faah.3        ; [1W] Enable Timer 1 Output
TO1              bit     0faah.2        ; [1W] Timer 1 Output
TOF1             bit     0faah.1        ; [1W] Timer 1 Output Flip Flop
TI1              bit     0faah.0        ; [1W] Timer 1 Input
T1               sfr     0fach          ; [8R] Timer 1 Counter Value
TMOD1            sfr     0faeh          ; [8W] Timer 1 Modulo Register
PSW              sfr     0fb0h          ; [4]  Processor Status Word
IST1             bit     PSW.3
IST0             bit     PSW.2
MBE              bit     PSW.1
RBE              bit     PSW.0
IPS              sfr     0fb2h          ; [4W] Interrupt Priorities
PCC              sfr     0fb3h          ; [4W] Processor Clock Control
IM1              sfr     0fb5h          ; [4W] INT1 Mode Register
IE4              bit     IBT.3          ;      Interrupt 4 Enable/Status
IRQ4             bit     IBT.2
                 __defint T0,0fbch      ; [4]  T0 Interrupt Enable/Status
IET1             bit     IT0.3
IRQT1            bit     IT0.2
                 __defint SIO,0fbdh     ; [4]  SIO Interrupt Enable/Status
                 __defint 0,0fbeh       ; [4]  Interrupt 0 Enable/Status
IE1              bit     I0.3           ;      Interrupt 1 Enable/Status
IRQ1             bit     I0.2
                 __defint 2,0fbfh       ; [4]  Interrupt 2 Enable/Status
IE3              bit     I2.3           ;      Interrupt 3 Enable/Status
IRQ3             bit     I2.2
BSB0             sfr     0fc0h          ; [4]  Bit Sequential Buffers
BSB1             sfr     0fc1h
BSB2             sfr     0fc2h
BSB3             sfr     0fc3h
CLOM             sfr     0fd0h          ; [4W] Clock Output Mode Register
PONF             bit     0fd1h.0        ; [1]  Power-on-Flag
PTHM             sfr     0fd6h          ; [8]  Threshold Setting
SIOM             sfr     0fe0h          ; [8W] Serial Interface Operation Mode
SIO              sfr     0fe4h          ; [8]  SIO Data Register
PMGA             sfr     0fe8h          ; [8W] Port Operation Mode
PM33             bit     PMGA.3
PM32             bit     PMGA.2
PM31             bit     PMGA.1
PM30             bit     PMGA.0
PM63             bit     PMGA+1.3
PM62             bit     PMGA+1.2
PM61             bit     PMGA+1.1
PM60             bit     PMGA+1.0
PMGB             sfr     0fech          ; [8W]      "        "
PM2              bit     PMGB.2
PM4              bit     PMGB+1.0
PM5              bit     PMGB+1.1
PM7              bit     PMGB+1.3
PMGC             sfr     0feeh          ; [8W]      "        "
PM8              bit     PMGC.0
PM9              bit     PMGC.1
PM12             bit     PMGC+1.0
PM13             bit     PMGC+1.1
PM14             bit     PMGC+1.2
PORT4            sfr     0ff4h          ; [4]  Port 4 Data Register
PORT7            sfr     0ff7h          ; [4]  Port 7 Data Register
PORT8            sfr     0ff8h          ; [4]  Port 8 Data Register
PORT9            sfr     0ff9h          ; [4]  Port 9 Data Register
PORT12           sfr     0ffch          ; [4]  Port 12 Data Register
PORT13           sfr     0ffdh          ; [4]  Port 13 Data Register
PORT14           sfr     0ffeh          ; [4]  Port 14 Data Register

VISIO            label   0006h          ; INTSIO Interrupt Vector Address
VIT0             label   0008h          ; INTT0 Interrupt Vector Address
VIT1             label   000ah          ; INTT1 Interrupt Vector Address

                 if      MOMCPU<75108h   ; RAM Size
RAMEnd            sfr     320
                 elseif
RAMEnd            sfr     512         
                 endif
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                if      __family=200
DSPM             sfr     0f88h          ; [4W] Display Mode
DIMS             sfr     0f89h          ; [4W] Display Dimmer Setting
DIGS             sfr     0f8ah          ; [4]  Display Number of Digits Selection
KSF              bit     DIGS.3
TPGM             sfr     0f90h          ; [8W] Pulse Generator Mode
MODL             sfr     0f94h          ; [8]  Pulsgenerator Modulo Value
MODH             sfr     0f96h          ; [8]
WM               sfr     0f98h          ; [8]  Watchdog Mode
TM0              sfr     0fa0h          ; [8]  Timer 0 Mode
T0               sfr     0fa4h          ; [8R] Timer 0 Count Value
TMOD0            sfr     0fa6h          ; [8W] Timer 0 Modulo Register
PSW              sfr     0fb0h          ; [4]  Processor Status Word
IST1             bit     PSW.3
IST0             bit     PSW.2
MBE              bit     PSW.1
RBE              bit     PSW.0
IPS              sfr     0fb2h          ; [4W] Interrupt Priorities
PCC              sfr     0fb3h          ; [4W] Processor Clock Control
IM1              sfr     0fb5h          ; [4W] INT1 Mode Register
SCC              sfr     0fb7h          ; [1W] System Clock Control
IE4              bit     IBT.3          ;      Interrupt 4 Enable/Status
IRQ4             bit     IBT.2
                 __defint W,0fbah       ; [4]  W Interrupt Enable/Status
                 __defint TPG,0fbbh     ; [4]  TPG Interrupt Enable/Status
IEKS             bit     ITPG.3         ;      KS Interrupt Enable/Status
IRQKS            bit     ITPG.2
                 __defint T0,0fbch      ; [4]  T0 Interrupt Enable/Status
                 __defint SIO,0fbdh     ; [4]  SIO Interrupt Enable/Status
                 __defint 0,0fbeh       ; [4]  Interrupt 0 Enable/Status
IE1              bit     I0.3           ;      Interrupt 1 Enable/Status
IRQ1             bit     I0.2
                 __defint 2,0fbfh       ; [4]  Interrupt 2 Enable/Status
PONF             bit     0fd1h.0        ; [1]  Power-on Flag
SIOM             sfr     0fe0h          ; [8W] Serial Interface Operation Mode
SIO              sfr     0fe4h          ; [8]  SIO Data Register
PMGA             sfr     0fe8h          ; [8W] Port Operation Mode
PM33             bit     PMGA.3
PM32             bit     PMGA.2
PM31             bit     PMGA.1
PM30             bit     PMGA.0
PM63             bit     PMGA+1.3
PM62             bit     PMGA+1.2
PM61             bit     PMGA+1.1
PM60             bit     PMGA+1.0
PMGB             sfr     0fech          ; [8W]      "        "
PM2              bit     PMGB.2
PM4              bit     PMGB+1.0
PM5              bit     PMGB+1.1
PORT4            sfr     0ff4h          ; [4]  Port 4 Data Register
SSTART           sfr     01c0h          ; Start of Display Memory
KS0              sfr     01fch          ; [8]  Keyboard Register
KS1              sfr     01feh          ; [4]
PORTH            sfr     01ffh          ; [4]  Data Register Port H

VI1              label   0006h          ; INT1 Interrupt Vector Address
VISIO            label   0008h          ; Interrupt Vector Address INTSIO
VIT0             label   000ah          ; INTT0 Interrupt Vector Address
VITPG            label   000ch          ; Interrupt Vector Address INTTPG
VIKS             label   000eh          ; Interrupt Vector Address INTKS

                 if      MOMCPU<75108h   ; RAM Size
RAMEnd            sfr     396
                 elseif  MOMCPU<75212h
RAMEnd            sfr     497
                 elseif
RAMEnd            sfr     512         
                 endif
                endif

;- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -

                if      __family=500
TPGM             sfr     0f90h          ; [8W] Pulse Generator Mode
MODL             sfr     0f94h          ; [8]  Pulse Generator Modulo Value
MODH             sfr     0f96h          ; [8]
WM               sfr     0f98h          ; [8]  Watchdog Mode
TM0              sfr     0fa0h          ; [8]  Timer 0 Mode
TOE0             bit     0fa2h.3        ; [1W] Timer 0 Output Enable
T0               sfr     0fa4h          ; [8R] Timer 0 Count Value
TMOD0            sfr     0fa6h          ; [8W] Timer 0 Modulo Register
PSW              sfr     0fb0h          ; [4]  Processor Status Word
IST1             bit     PSW.3
IST0             bit     PSW.2
MBE              bit     PSW.1
RBE              bit     PSW.0
IPS              sfr     0fb2h          ; [4W] Interrupt Priorities
PCC              sfr     0fb3h          ; [4W] Processor Clock Control
IM1              sfr     0fb5h          ; [4W] INT1 Mode Register
IM2              sfr     0fb6h          ; [4W] INT2 Mode Register
SCC              sfr     0fb7h          ; [1W] System Clock Control
IE4              bit     IBT.3          ;      Interrupt 4 Enable/Status
IRQ4             bit     IBT.2
EOT              bit     0fb9h.0
                 __defint W,0fbah       ; [4]  W Interrupt Enable/Status
                 __defint TPG,0fbbh     ; [4]  TPG Interrupt Enable/Status
                 __defint T0,0fbch      ; [4]  T0 Interrupt Enable/Status
                 __defint CSIO,0fbdh    ; [4]  CSIO InterruptEnable/Status
                 __defint 0,0fbeh       ; [4]  Interrupt 0 Enable/Status
IE1              bit     I0.3           ;      Interrupt 1 Enable/Status
IRQ1             bit     I0.2
                 __defint 2,0fbfh       ; [4]  Interrupt 2 Enable/Status
BSB0             sfr     0fc0h          ; [4]  Bit Sequential Buffers
BSB1             sfr     0fc1h
BSB2             sfr     0fc2h
BSB3             sfr     0fc3h
CSIM1            sfr     0fc8h          ; [8W] Serial Interface Operation Mode
CSIE1            bit     CSIM1+1.3
SIO1             sfr     0fcch          ; [8]  SIO Data Register
CLOM             sfr     0fd0h          ; [4W] Clock Output Mode Register
ADM              sfr     0fd8h          ; [1]  A/D Converter Control
SOC              sfr     ADM.3
EOC              sfr     ADM.2
SA               sfr     0fdah
POGA             sfr     0fdch          ; [8W] Port A Pull-Up Control
CSIM0            sfr     0fe0h          ; [8W] Serial Interface Operation Mode
CSIE             bit     CSIM+1.3
COI              bit     CSIM+1.2
WUP              bit     CSIM+1.1
SBIC             sfr     0fe2h          ; [1]   SBI Control
CMDD             bit     SBIC.3
RELD             bit     SBIC.2
CMDT             bit     SBIC.1
RELT             bit     SBIC.0
BSYE             bit     SBIC+1.3
ACKD             bit     SBIC+1.2
ACKE             bit     SBIC+1.1
ACKT             bit     SBIC+1.0
SIO0             sfr     0fe4h          ; [8]  SIO Data Register
SVA              sfr     0fe6h          ; [8W] Node Address on Serial Bus
PMGA             sfr     0fe8h          ; [8W] Port Operation Mode
PM33             bit     PMGA.3
PM32             bit     PMGA.2
PM31             bit     PMGA.1
PM30             bit     PMGA.0
PM63             bit     PMGA+1.3
PM62             bit     PMGA+1.2
PM61             bit     PMGA+1.1
PM60             bit     PMGA+1.0
PMGB             sfr     0fech          ; [8W]      "        "
PM2              bit     PMGB.2
PM4              bit     PMGB+1.0
PM5              bit     PMGB+1.1
PM7              bit     PMGB+1.3
PMGC             sfr     0feeh          ; [8W]      "        "
PM8              bit     PMGC.0
PM9              bit     PMGC.1
PM12             bit     PMGC+1.0
PM13             bit     PMGC+1.1
PM14             bit     PMGC+1.2
PORT4            sfr     0ff4h          ; [4]  Port 4 Data Register
PORT7            sfr     0ff7h          ; [4]  Port 7 Data Register
PORT8            sfr     0ff8h          ; [4R] Port 8 Data Register
PORT9            sfr     0ff9h          ; [4]  Port 9 Data Register
PORT10           sfr     0ffah          ; [4]  Port 10 Data Register
PORT11           sfr     0ffbh          ; [4]  Port 11 Data Register
PORT12           sfr     0ffch          ; [4]  Port 12 Data Register
PORT13           sfr     0ffdh          ; [4]  Port 13 Data Register
PORT14           sfr     0ffeh          ; [4]  Port 14 Data Register
PORT15           sfr     0fffh          ; [4R] Port 15 Data Register

VI1              label   0006h          ; INT1 Interrupt Vector Address
VICSIO           label   0008h          ; INTCSI Interrupt Vector AddressO
VIT0             label   000ah          ; INTT0 Interrupt Vector Address
VITPG            label   000ch          ; Interrupt Vector Address INTTPG

RAMEnd           sfr     512            ; RAM Size
                endif

		restore                 ; re-enable listing

                endif			; stddef75inc
