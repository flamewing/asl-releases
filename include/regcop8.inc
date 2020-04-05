                ifndef  regcop8inc      ; avoid multiple inclusion
regcop8inc      equ     1

                save
                listing off		; no listing over this file
;****************************************************************************
;*                                                                          *
;*   AS 1.42 - File REGCOP8.INC                                             *
;*   								            *
;*   Contains Register Definitions for COP8 Controllers                     *
;*                                                                          *
;****************************************************************************

                if      MOMPASS=1
                 switch  MOMCPUNAME
                 case    "COP87L84"
                  message "including COP87L84-registers"
                 elsecase
                  fatal   "invalid processor type: only COP87L84 allowed"
                 endcase
                endif

;----------------------------------------------------------------------------
; Processor Core

__REG           set     0
                rept    10
R{"\{__REG}"}   equ __REG+0xf0
__REG           set     __REG+1,data
                endm
                rept    6
R1{"\{__REG-10}"} equ __REG+0xf0
__REG           set     __REG+1,data
                endm

psw             sfr     0xef            ; Flags
gie             equ      0              ; Global Interrupt Enable
exen            equ      1              ; External Interrupt Enable
busy            equ      2              ; Microwire Busy
expnd           equ      3              ; External Interrupt Pending
t1ena           equ      4              ; Timer 1 Interrupt Enable
t1pnda          equ      5              ; Timer 1 Interrupt Pending
c               equ      6              ; Carry
hc              equ      7              ; Half Carry
x               sfr     0xfc            ; X-Register
sp              sfr     0xfd            ; Stack Pointer
b               sfr     0xfe            ; B Register

;----------------------------------------------------------------------------
; Peripheral Control Registers

cntrl           sfr     0xee            ; Globale Control
sl0             equ      0              ; Microwire Clock Divider
sl1             equ      1
iedg            equ      2              ; External Interrupt Edge Select
msel            equ      3              ; Microwire use G4/G5
t1c0            equ      4              ; Timer 1 Interrupt/Start-Stop
t1c1            equ      5              ; Timer 1 Mode Select
t1c2            equ      6
t1c3            equ      7

icntrl          sfr     0xe8            ; Continuation...
t1enb           equ      0              ; Timer 1 Capture Interrupt Enable
t1pndb          equ      1              ; Timer 1 Capture Interrupt Pending
uwen            equ      2              ; MicroWire-Interrupt Enable
uwpnd           equ      3              ; MicroWire-Interrupt Pending
t0en            equ      4              ; Timer 0 Interrupt Enable
t0pndb          equ      5              ; Timer 0 Interrupt Pending
lpen            equ      6              ; Port L-Interrupt Enable

;----------------------------------------------------------------------------
; Timer

t1rblo          sfr     0xe6            ; Timer 1 Reload Value B
t1rbhi          sfr     0xe7

tmr1lo          sfr     0xea            ; Timer 1 Count Value
tmr1hi          sfr     0xeb

t1ralo          sfr     0xec            ; Timer 1 Reload Value B
t1rahi          sfr     0xed

;----------------------------------------------------------------------------
; PWM

pscal           sfr     0xa0            ; PWM Prescaler

rlon            sfr     0xa1            ; PWM Duty Cycle

pwmcon          sfr     0xa2            ; PWM Control
pwen0           equ      0              ; PWM0 Output to I/O Port
pwen1           equ      1              ; PWM1 Output to I/O Port
pwon            equ      2              ; PWM Start/Stop
pwmd            equ      3              ; PWM Mode
pwie            equ      4              ; PWM Interrupt Enable
pwpnd           equ      5              ; PWM Interrupt Pending
esel            equ      6              ; PWM Edge Select

;----------------------------------------------------------------------------
; MicroWire Interface

wkedg           sfr     0xc8
wken            sfr     0xc9
wkpnd           sfr     0xca

sior            sfr     0xe9            ; Shift register

;----------------------------------------------------------------------------
; CAN Interface

txd1            sfr     0xb0            ; Transmit Data (Byte 1,3,5,7,...)
txd2            sfr     0xb1            ; Transmit Data (Byte 2,4,6,8,...)

tdlc            sfr     0xb2            ; Transmit Length/Identifier(L) Register
tdlc0           equ      0              ; Transmit Length
tdlc1           equ      1
tdlc2           equ      2
tdlc3           equ      3
tid0            equ      4              ; Transmit Identification (Bits 0..3)
tid1            equ      5
tid2            equ      6
tid3            equ      7

tid             sfr     0xb3            ; Transmit Identification Register
tid4            equ      0              ; Transmit Identification (Bits 4..10)
tid5            equ      1
tid6            equ      2
tid7            equ      3
tid8            equ      4
tid9            equ      5
tid10           equ      6
trtr            equ      7              ; Transmit Frame Remote

rxd1            sfr     0xb4            ; Receive Data (Byte 1,3,5,7,...)
rxd2            sfr     0xb5            ; Receive Data (Byte 2,4,6,8,...)

ridl            sfr     0xb6            ; Receive Length/Identifier(L) Register
rdlc0           equ      0              ; Receive Length
rdlc1           equ      1
rdlc2           equ      2
rdlc3           equ      3
rid0            equ      4              ; Receive Identification (Bits 0..3)
rid1            equ      5
rid2            equ      6
rid3            equ      7

rid             sfr     0xb7            ; Receive Identification Register
rid4            equ      0              ; Receive Identification (Bits 4..10)
rid5            equ      1
rid6            equ      2
rid7            equ      3
rid8            equ      4
rid9            equ      5
rid10           equ      6

cscal           sfr     0xb8            ; CAN Clock Prescaler

ctim            sfr     0xb9            ; CAN Bus Timing Register
ps0             equ      2              ; Phase Segment
ps1             equ      3
ps2             equ      4
pps0            equ      5              ; Propagation Segment
pps1            equ      6
pps2            equ      7

cbus            sfr     0xba            ; CAN Bus Timing Register
fmod            equ      1              ; Fault Confinement Mode
rxred0          equ      2              ; Apply Reference Voltage to Rx0
rxref1          equ      3              ; Apply Reference Voltage to Tx0
txen0           equ      4              ; Enable TxD Output Drivers
txen1           equ      5
riaf            equ      6              ; Receive Filter Enable

tcntl           sfr     0xbb            ; CAN Bus Control/Status Register
txss            equ      0              ; Transmitter Start/Stop
rie             equ      1              ; Receive Interrupt Enable
tie             equ      2              ; Transmit Interrupt Enable
ceie            equ      3              ; CAN Interrupt Enable
rerr            equ      4              ; Receive Error
terr            equ      5              ; Transmit Error
ns0             equ      6              ; Node State
ns1             equ      7

rtstat          sfr     0xbc            ; CAN Bus Transmitter/Receiver State
rbf             equ      0              ; Receive Buffer Full
rcv             equ      1              ; Reception Running
rfv             equ      2              ; Received Frame Valid
rorn            equ      3              ; Receiver Overflow
rold            equ      4              ; Receiver Frame Overflow
rrtr            equ      5              ; Remote-Bit Set in Received Frame
txpnd           equ      6              ; Transmission Pending
tbe             equ      7              ; Transmit Buffer Empty

tec             sfr     0xbd            ; Transmit Error Counter
rec             sfr     0xbe            ; Receive Error Counter

;----------------------------------------------------------------------------
; Comparators

cmpsl           sfr     0xd3            ; Comparator Control Register
cmp1en          equ      1              ; Comparator 1 Enable
cmp1rd          equ      2              ; Comparator 1 Output Value
cmp1oe          equ      3              ; Comparator 1 Output Enable
cmp2en          equ      4              ; Comparator 2 Enable
cmp2rd          equ      5              ; Comparator 2 Output Value
cmp2oe          equ      6              ; Comparator 2 Output Enable
cmp2sel         equ      7              ; Comparator 2 Output on L3/L5

;----------------------------------------------------------------------------
; Ports

portld          sfr     0xd0            ; Port L Output Register
portlc          sfr     0xd1            ; Port L Configuration Register
portlp          sfr     0xd2            ; Port L Input Register

portgd          sfr     0xd4            ; Port G Output Register
portgc          sfr     0xd5            ; Port G Configuration Register
portgp          sfr     0xd6            ; Port G Input Register

portd           sfr     0xdc            ; Port D Output Register

;----------------------------------------------------------------------------
; Vector Addresses

;----------------------------------------------------------------------------
; Memory Addresses

iram            sfr     0x00            ; Internal RAM Area
iramend         sfr     0x2f

;----------------------------------------------------------------------------

                restore			; re-enable listing

                endif			; regcop8inc


