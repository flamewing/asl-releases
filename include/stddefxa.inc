                ifndef  stddefxainc     ; avoid multiple inclusion

stddefxainc     equ     1

                save
                listing off   ; no listing over this file

;****************************************************************************
;*                                                                          *
;*   AS 1.42 - Datei STDDEFXA.INC                                           *
;*   								            *
;*   Sinn : Contains SFR and Bit Definitions for Philips/NXP XA Prozessors  *
;* 									    *
;*   Last Changes : 1996-06-29 Initial Version                              *
;*                  1998-08-18 removed suffix of P_51, F0_51, and F1_51,    *
;*                              since there are no name conflicts           *
;*                                                                          *
;****************************************************************************

                if      (MOMCPUNAME<>"XAG3")&&(MOMCPUNAME<>"XAG1")&&(MOMCPUNAME<>"XAG1")
                 fatal  "wrong target selected: only XAG1, XAG2, or XAG3 allowed"
		endif


                if      MOMPASS=1
                 message "Philips/NXP XA SFR Definitions (C) 1996 Alfred Arnold"
		endif

;----------------------------------------------------------------------------
; Prozessorkern

cs              port    443h            ; Code Segment
ds              port    441h            ; Data Segment
es              port    442h            ; Extra Segment

ssel            port    403h            ; Select whether [R0..R6] use ES or DS
r0seg           bit     ssel.0
r1seg           bit     ssel.1
r2seg           bit     ssel.2
r3seg           bit     ssel.3
r4seg           bit     ssel.4
r5seg           bit     ssel.5
r6seg           bit     ssel.6
eswen           bit     ssel.7          ; Allow/disallow write accesses via ES (user mode)

pswl            port    400h            ; Processor Status (L):
z               bit     pswl.0          ;  Zero Flag
n               bit     pswl.1          ;  Negative Flag
v               bit     pswl.2          ;  Overflow Flag
ac              bit     pswl.6          ;  Half Carry (for DAA)
c               bit     pswl.7          ;  Carry

pswh            port    401h            ; Processor Status (H):
im0             bit     pswh.0          ;  Interrupt Mask Bit 0
im1             bit     pswh.1          ;  Interrupt Mask Bit 1
im2             bit     pswh.2          ;  Interrupt Mask Bit 2
im3             bit     pswh.3          ;  Interrupt Mask Bit 3
rs0             bit     pswh.4          ;  Register Bank Selection Bit 0
rs1             bit     pswh.5          ;  Register Bank Selection Bit 1
tm              bit     pswh.6          ;  CPU in Single Step Mode
sm              bit     pswh.7          ;  CPU in Supervisor Mode

psw51           port    402h            ; Emulated 8051 PSW:
p               bit     psw51.0         ;  Parity Bit
f1              bit     psw51.1         ;  User Flag 1
v_51            bit     psw51.2         ;  Overflow Flag
rs0_51          bit     psw51.3         ;  Register Bank Selection Bit 0
rs1_51          bit     psw51.4         ;  Register Bank Selection Bit 1
f0              bit     psw51.5         ;  User Flag 0
ac_51           bit     psw51.6         ;  Half Carry (for DAA)
c_51            bit     psw51.7         ;  Carry

pcon            port    404h            ; Control Power Modes:
idl             bit     pcon.0          ;  Enable Idle Mode
pd              bit     pcon.1          ;  Enabled Power Down Mode

;----------------------------------------------------------------------------
; Chip Configuration

scr             port    440h            ; System Configuration
;pz             bit     scr.0           ;  Limit Address Spaces to 64K
;cm             bit     scr.1           ;  8051 Compatible Register Mapping
;pt0            bit     scr.2           ;  Prescaler Setting
;pt1            bit     scr.3

bcr             port    46ah            ; Bus Configuration
;bc0            bit     bcr.0           ;  Select 12/16/20/24 Adress Lines
;bc1            bit     bcr.1
;bc2            bit     bcr.2           ;  Select 8 or 16 Data Lines
;busd           bit     bcr.3           ;  Disable Bus entirely
;waitd          bit     bcr.4           ;  Ignore WAIT Pin

btrl            port    468h            ; Configure Bus Timing (L)
;dra0           bit     btrl.0          ;  Total Length of Read Cycle
;dra1           bit     btrl.1
;dr0            bit     btrl.2          ;  Length of Read Cycle without ALE
;dr1            bit     btrl.3
;dwa0           bit     btrl.4          ;  Total Length of Write Cycle
;dwa1           bit     btrl.5
;dw0            bit     btrl.6          ;  Length of Write Cycle without ALE
;dw1            bit     btrl.7

btrh            port    469h            ; Configure Bus Timing (H)
;cra0           bit     btrl.0          ;  Total Length of Instruction Read Cycle
;cra1           bit     btrl.1
;cr0            bit     btrl.2          ;  Length of Instruction Read Cycle without ALE
;cr1            bit     btrl.3
;alew           bit     btrl.5          ;  Length of ALE Pulse
;wm0            bit     btrl.6          ;  Length of WR Pulse
;wm1            bit     btrl.7

;----------------------------------------------------------------------------
; Interrupt Control

iel             port    426h            ; Interrupt Enable (L):
ex0             bit     iel.0           ;  External Interrupt 0
et0             bit     iel.1           ;  Timer 0
ex1             bit     iel.2           ;  External Interrupt 1
et1             bit     iel.3           ;  Timer 1
et2             bit     iel.4           ;  Timer 2
ea              bit     iel.7           ;  Global Enable

ieh             port    427h            ; Interrupt Enable (H):
eri0            bit     ieh.0           ;  UART0 Rx
eti0            bit     ieh.1           ;  UART0 Tx
eri1            bit     ieh.2           ;  UART1 Rx
eti1            bit     ieh.3           ;  UART1 Tx

ipa0            port    4a0h            ; Interrupt Priorities (0):
;px0            bit     ipa0.0          ;  External Interrupt 0 (4 Bits)
;pt0            bit     ipa0.4          ;  Timer 0 (4 Bits)

ipa1            port    4a1h            ; Interrupt Priorities (1):
;px1            bit     ipa1.0          ;  External Interrupt 1 (4 Bits)
;pt1            bit     ipa1.4          ;  Timer 1 (4 Bits)

ipa2            port    4a2h            ; Interrupt Priorities (2):
;pt2            bit     ipa1.0          ;  Timer 2 (4 Bits)

ipa4            port    4a4h            ; Interrupt Priorities (4):
;pri0           bit     ipa4.0          ;  UART0 Rx (4 Bits)
;pti0           bit     ipa4.4          ;  UART0 Tx (4 Bits)

ipa5            port    4a5h            ; Interrupt Priorities (4):
;pri1           bit     ipa5.0          ;  UART1 Rx (4 Bits)
;pti1           bit     ipa5.4          ;  UART1 Tx (4 Bits)

swe             port    403h            ; Enable Software Interrupts
swe1            bit     swe.0
swe2            bit     swe.1
swe3            bit     swe.2
swe4            bit     swe.3
swe5            bit     swe.4
swe6            bit     swe.5
swe7            bit     swe.6

swr             port    42ah            ; Trigger Software Interrupts
swr1            bit     swr.0
swr2            bit     swr.1
swr3            bit     swr.2
swr4            bit     swr.3
swr5            bit     swr.4
swr6            bit     swr.5
swr7            bit     swr.6

;----------------------------------------------------------------------------
; Ports

p0              port    430h            ; Data Register Port 0
ad0             bit     p0.0            ;  Data Bit 0/Address Bit 4
ad1             bit     p0.1            ;  Data Bit 1/Address Bit 5
ad2             bit     p0.2            ;  Data Bit 2/Address Bit 6
ad3             bit     p0.3            ;  Data Bit 3/Address Bit 7
ad4             bit     p0.4            ;  Data Bit 4/Address Bit 8
ad5             bit     p0.5            ;  Data Bit 5/Address Bit 9
ad6             bit     p0.6            ;  Data Bit 6/Address Bit 10
ad7             bit     p0.7            ;  Data Bit 7/Address Bit 11

p1              port    431h            ; Data Register Port 1
a0              bit     p1.0            ;  Address Line 0 (8 Bit Mode)
wrh             bit     p1.0            ;  Write Pulse D8-D15 (16-Bit-Mode)
a1              bit     p1.1            ;  Address Line 1
a2              bit     p1.2            ;  Address Line 2
a3              bit     p1.3            ;  Address Line 3
rxd1            bit     p1.4            ;  UART1 Rx Line
txd1            bit     p1.5            ;  UART1 Tx Line
t2              bit     p1.6            ;  Timer 2 Input/Output
t2ex            bit     p1.7            ;  Trigger Timer 2

p2              port    432h            ; Data Register Port 2
ad8             bit     p2.0            ;  Data Bit 8/Address Bit 12
ad9             bit     p2.1            ;  Data Bit 9/Address Bit 13
ad10            bit     p2.2            ;  Data Bit 10/Address Bit 14
ad11            bit     p2.3            ;  Data Bit 11/Address Bit 15
ad12            bit     p2.4            ;  Data Bit 12/Address Bit 16
ad13            bit     p2.5            ;  Data Bit 13/Address Bit 17
ad14            bit     p2.6            ;  Data Bit 14/Address Bit 18
ad15            bit     p2.7            ;  Data Bit 15/Address Bit 19

p3              port    433h            ; Data Register Port 3
rxd0            bit     p3.0            ;  UART0 Rx Line
txd0            bit     p3.1            ;  UART0 Tx Line
int0            bit     p3.2            ;  External Interrupt 0
int1            bit     p3.3            ;  External Interrupt 1
t0              bit     p3.4            ;  Timer 0 Input/Output
t1              bit     p3.5            ;  Timer 1 Input/Output
wr              bit     p3.6            ;  Write Pulse D0-7
rd              bit     p3.7            ;  Read Pulse D0-15

p0cfga          port    470h            ; Configuration Bits A Port 0
p1cfga          port    471h            ; Configuration Bits A Port 1
p2cfga          port    472h            ; Configuration Bits A Port 2
p3cfga          port    473h            ; Configuration Bits A Port 3

p0cfgb          port    4f0h            ; Configuration Bits B Port 0
p1cfgb          port    4f1h            ; Configuration Bits B Port 1
p2cfgb          port    4f2h            ; Configuration Bits B Port 2
p3cfgb          port    4f3h            ; Configuration Bits B Port 3

;----------------------------------------------------------------------------
; Timer

tl0             port    450h            ; Timer 0 Counter Value
th0             port    451h
rtl0            port    454h            ; Timer 0 Reload Value
rth0            port    455h

tl1             port    452h            ; Timer 1 Counter Value
th1             port    453h
rtl1            port    456h            ; Timer 1 Reload Value
rth1            port    457h

tmod            port    45ch            ; Mode Register Timer 0/1
;t0_m0          bit     tmod.0          ;  Operation Mode Timer 0
;t0_m1          bit     tmod.1
;ct_t0          bit     tmod.2          ;  Timer 0 as Timer or Counter
;t0_gate        bit     tmod.3          ;  Enable Timer 0
;t1_m0          bit     tmod.4          ;  Operation Mode Timer 1
;t1_m1          bit     tmod.5
;ct_t1          bit     tmod.6          ;  Timer 1 as Timer oder Counter
;t1_gate        bit     tmod.7          ;  Enable Timer 1

tcon            port    410h            ; Timer 0/1 Control Register
it0             bit     tcon.0          ;  Edge or Level Triggered Interrupt 0
ie0             bit     tcon.1          ;  Edge Detector Interrupt 0
it1             bit     tcon.2          ;  Edge or Level Triggered Interrupt 1
ie1             bit     tcon.3          ;  Edge Detector Interrupt 1
tr0             bit     tcon.4          ;  Start/Stop Timer 0
tf0             bit     tcon.5          ;  Timer 0 Overflow
tr1             bit     tcon.6          ;  Start/Stop Timer 1
tf1             bit     tcon.7          ;  Timer 1 Overflow

tstat           port    411h            ; Timer 0/1 Status
t0oe            bit     tstat.0         ;  Output Timer 0 Clock on T0
t1oe            bit     tstat.1         ;  Output Timer 1 Clock on T1

tl2             port    458h            ; Timer 2 Value
th2             port    459h

t2capl          port    45ah            ; Timer 2 Capture Value
t2caph          port    45bh

t2mod           port    419h            ; Timer 2 Mode
dcen            bit     t2mod.0         ;  Timer 2 Direction up or Controlled by T2EX
t2oe            bit     t2mod.1         ;  Output Timer 2 Clock on T2
tclk1           bit     t2mod.4         ;  UART1 uses Timer 2 instead of Timer 1 for Tx
rclk1           bit     t2mod.5         ;  UART1 uses Timer 2 instead of Timer 1 for Rx

t2con           port    418h            ; Timer 2 Control Register
cp_rl2          bit     t2con.0         ;  Timer 2 Capture/Reload ?
ct_t2           bit     t2con.1         ;  Timer 2 as Timer or Counter
tr2             bit     t2con.2         ;  Start/Stop Timer 2
exen2           bit     t2con.3         ;  Allow Capture/Reload via T2EX
tclk0           bit     t2con.4         ;  UART0 uses Timer2 instead of Timer 1 for Tx
rclk0           bit     t2con.5         ;  UART0 uses Timer2 instead of Timer 1 for Rx
exf2            bit     t2con.6         ;  Capture/Reload Occured
tf2             bit     t2con.7         ;  Timer 2 Overflow

;----------------------------------------------------------------------------
; Watchdog

wdcon           port    41fh            ; Watchdog Configuration
wdtof           bit     wdcon.1         ;  Timeout occured?
wdrun           bit     wdcon.2         ;  Enable Watchdog
pre0            bit     wdcon.5         ;  Prescaler
pre1            bit     wdcon.6
pre2            bit     wdcon.7

wdl             port    45fh            ; Reload Value

wfeed1          port    45dh            ; Reset Register 1 (a5h)
wfeed2          port    45eh            ; Reset Register 2 (5ah)

;----------------------------------------------------------------------------
; Serial Ports

s0buf           port    460h            ; UART0 Data Register

s0addr          port    461h            ; UART0 Slave Address
s0aden          port    462h            ; UART0 Address Mask

s0stat          port    421h            ; UART0 Status
stint0          bit     s0stat.0        ;  UART0 Interrupt Enable
oe0             bit     s0stat.1        ;  UART0 Receiver Overflow
br0             bit     s0stat.2        ;  UART0 Break Detected
fe0             bit     s0stat.3        ;  UART0 Framing Error

s0con           port    420h            ; UART0 Control
ri_0            bit     s0con.0         ;  UART0 Character Received
ti_0            bit     s0con.1         ;  UART0 Character Transmitted
rb8_0           bit     s0con.2         ;  UART0 9th Bit received
tb8_0           bit     s0con.3         ;  UART0 9th Bit to be snet
ren_0           bit     s0con.4         ;  UART0 Receiver Enable
sm2_0           bit     s0con.5         ;  UART0 Enable Multiprocessor Mode
sm1_0           bit     s0con.6         ;  UART0 Mode Select
sm0_0           bit     s0con.7

s1stat          port    421h            ; UART1 Status
stint1          bit     s1stat.0        ;  UART1 Interrupt Enable
oe1             bit     s1stat.1        ;  UART1 Receiver Overflow
br1             bit     s1stat.2        ;  UART1 Break Detected
fe1             bit     s1stat.3        ;  UART1 Framing Error

s1con           port    424h            ; UART1 Control
ri_1            bit     s1con.0         ;  UART1 Character Received
ti_1            bit     s1con.1         ;  UART1 Character Transmitted
rb8_1           bit     s1con.2         ;  UART1 9th Bit received
tb8_1           bit     s1con.3         ;  UART1 9th Bit to be sent
ren_1           bit     s1con.4         ;  UART1 Receiver Enable
sm2_1           bit     s1con.5         ;  UART1 Enable Multiprocessor Mode
sm1_1           bit     s1con.6         ;  UART1 Mode Select
sm0_1           bit     s1con.7

s1buf           port    464h            ; UART1 Data Register

s1addr          port    465h            ; UART1 Slave Address
s1aden          port    466h            ; UART1 Address Mask


;----------------------------------------------------------------------------
; Vectors

vec_reset       label   0000h           ; Reset Address
vec_bkpt        label   0004h           ; Breakpoint Instruction
vec_trace       label   0008h           ; Single Step
vec_stkover     label   000ch           ; Stack Overflow
vec_divzero     label   0010h           ; Division by 0
vec_reti        label   0014h           ; Privilege Violation (RETI in User Mode)
vec_trap0       label   0040h           ; TRAP 0
vec_trap1       label   0044h           ; TRAP 1
vec_trap2       label   0048h           ; TRAP 2
vec_trap3       label   004ch           ; TRAP 3
vec_trap4       label   0050h           ; TRAP 4
vec_trap5       label   0054h           ; TRAP 5
vec_trap6       label   0058h           ; TRAP 6
vec_trap7       label   005ch           ; TRAP 7
vec_trap8       label   0060h           ; TRAP 8
vec_trap9       label   0064h           ; TRAP 9
vec_trap10      label   0068h           ; TRAP 10
vec_trap11      label   006ch           ; TRAP 11
vec_trap12      label   0070h           ; TRAP 12
vec_trap13      label   0074h           ; TRAP 13
vec_trap14      label   0078h           ; TRAP 14
vec_trap15      label   007ch           ; TRAP 15

vec_ie0         label   0080h           ; External Interrupt 0
vec_tf0         label   0084h           ; Timer 0 Interrupt
vec_ie1         label   0088h           ; External Interrupt 1
vec_tf1         label   008ch           ; Timer 1 Interrupt
vec_tf2         label   0090h           ; Timer 2 Interrupt
vec_ri0         label   00a0h           ; UART0 Reception
vec_ti0         label   00a4h           ; UART0 Transmission
vec_ri1         label   00a8h           ; UART1 Reception
vec_ti1         label   00ach           ; UART1 Transmission

vec_swr1        label   0100h           ; Software Interrupt 1
vec_swr2        label   0104h           ; Software Interrupt 2
vec_swr3        label   0108h           ; Software Interrupt 3
vec_swr4        label   010ch           ; Software Interrupt 4
vec_swr5        label   0110h           ; Software Interrupt 5
vec_swr6        label   0114h           ; Software Interrupt 6
vec_swr7        label   0118h           ; Software Interrupt 7

;----------------------------------------------------------------------------
; Memory Addresses

irom            label   0               ; Start of internal ROM
                switch  MOMCPUNAME      ; End of internal ROM
                case    "XAG3"
iromend          label   7fffh          ; XA-G3: 32K ROM
                case    "XAG2"
iromend          label   3fffh          ; XA-G2: 16K ROM
                case    "XAG1"
iromend          label   1fffh          ; XA-G1:  8K ROM
                endcase

iram            equ     0,data          ; Start of internal RAM
iramend         equ     1ffh,data       ; End of internal RAM: always 512 bytes

                restore                 ; re-allow listing

                endif
