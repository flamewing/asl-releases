                save
                listing off   ; kein Listing über diesen File

;****************************************************************************
;*                                                                          *
;*   AS 1.41 - Datei REGAVR.INC                                             *
;*                                                                          *
;*   Sinn : enthält SFR- und Bitdefinitionen für die AVR-Prozessoren        *
;*                                                                          *
;*   letzte Änderungen :  6. 7.1996                                         *
;*   letzte Änderungen :  8. 6.1997 Anpassung an die endgültigen Versionen  *
;*                                                                          *
;****************************************************************************

                ifndef  regavrinc       ; verhindert Mehrfacheinbindung

regavrinc       equ     1

                if      (MOMCPUNAME<>"AT90S1200")&&(MOMCPUNAME<>"AT90S2313")&&(MOMCPUNAME<>"AT90S4414")&&(MOMCPUNAME<>"AT90S8515")
                 fatal  "Falscher Prozessortyp eingestellt: nur AT90S1200, AT90S2313, AT90S4414 oder AT90S8515 erlaubt!"
                endif


                if      MOMPASS=1
                 message "Atmel-AVR-SFR-Definitionen (C) 1996 Alfred Arnold"
                endif

;----------------------------------------------------------------------------
; Prozessorkern

sreg            port    $3f             ; Statusregister:
c               equ     0               ;  Carry
z               equ     1               ;  Ergebnis Null
n               equ     2               ;  Ergebnis negativ
v               equ     3               ;  Zweierkomplement-Überlauf
s               equ     4               ;  Vorzeichen
h               equ     5               ;  Halfcarry
t               equ     6               ;  Bitspeicher
i               equ     7               ;  globale Interruptsperre

                if      MOMCPU>=$902313
spl              equ     $3d            ; Stapelzeiger (MSB)
                 if      MOMCPU>=$904414
sph               equ     $3e           ;              (LSB)
                 endif
                endif

;----------------------------------------------------------------------------
; Chip-Konfiguration

mcucr           port    $35             ; CPU-Steuerung:
isc00           equ     0               ;  Flankenwahl INT0
isc01           equ     1               ;  Flanken/Pegeltriggerung INT0
                if      MOMCPU>=$902313
isc10            equ     2              ;  Flankenwahl INT1
isc11            equ     3              ;  Flanken/Pegeltriggerung INT1
                endif
sm              equ     4               ;  Idle/Powerdown-Modus wählen
se              equ     5               ;  Sleep-Modus freigeben
                if      MOMCPU>=$904414
srw              equ     6              ;  Wait-State-Wahl externes SRAM
sre              equ     7              ;  Freigabe externes SRAM
                endif

;----------------------------------------------------------------------------
; Interrupt-Steuerung

gimsk           port    $3b             ; generelle Interrupt-Maske:
int0            equ     6               ;  externer Interrupt 0
                if      MOMCPU>=$902313
int1             equ     7              ;  externer Interrupt 1
                endif

                if      MOMCPU>=$902313
gifr		 port	 $3a            ; generelle Interrupt-Flags:
intf0		 equ	 6		;  externer Interrupt 0
intf1            equ     7              ;  externer Interrupt 1
                endif

timsk           port    $39             ; Timer-Interrupt-Maske:
toie0           equ     1               ;  Überlauf Timer 0
                if      MOMCPU>=$902313
                 if      MOMCPU>=$904414
ocie1b            equ     5             ;  Vergleich B Timer 1
                 endif
ocie1a           equ     6              ;  Vergleich Timer 1
toie1            equ     7              ;  Überlauf Timer 1
ticie1           equ     3              ;  Fang Timer 1
                endif

tifr            port    $38             ; Timer-Interrupt-Flags:
tov0            equ     1               ;  Überlauf Timer 0
                if      MOMCPU>=$902313
                 if      MOMCPU>=$904414
ocf1b             equ     5             ;  Vergleich B Timer 1
                 endif
ocf1a            equ     6              ;  Vergleich A Timer 1
tov1             equ     7              ;  Überlauf Timer 1
icf1             equ     3              ;  Fang Timer 1
                endif

;----------------------------------------------------------------------------
; parallele Ports

                if      MOMCPU>=$904414
porta            port    $1b            ; Datenregister Port A
ddra             port    $1a            ; Datenrichtungsregister Port A
pina             port    $19            ; Leseregister Port A
                endif

portb           port    $18             ; Datenregister Port B
ddrb            port    $17             ; Datenrichtungsregister Port B
pinb            port    $16             ; Leseregister Port B

                if      MOMCPU>=$904414
portc            port    $15            ; Datenregister Port C
ddrc             port    $14            ; Datenrichtungsregister Port C
pinc             port    $13            ; Leseregister Port C
                endif

portd           port    $12             ; Datenregister Port D
ddrd            port    $11             ; Datenrichtungsregister Port D
pind            port    $10             ; Leseregister Port D

;----------------------------------------------------------------------------
; Timer

tccr0           port    $33             ; Steuerregister Timer 0:
cs00            equ     0               ;  Vorteilereinstellung
cs01            equ     1
cs02            equ     2

tcnt0           port    $32             ; Zählregister Timer 0

                if      MOMCPU>=$902313
tccr1a           port    $2f            ; Steuerregister A Timer 1:
pwm10            equ     0              ;  Modus Pulsweitenmodulator
pwm11            equ     1
com1a0           equ     6              ;  Vergleichsmodus A
com1a1           equ     7
                 if      MOMCPU>=$904414
com1b0            equ     4             ;  Vergleichsmodus B
com1b1            equ     5
                 endif

tccr1b           port    $2e            ; Steuerregister B Timer 1:
cs10             equ     0              ; Vorteilereinstellung
cs11             equ     1
cs12             equ     2
ctc1             equ     3              ; nach Gleichheit zurücksetzen ?
ices1            equ     6              ; Flankenwahl Fang
icnc1            equ     7              ; Rauschfilter für Fangfunktion

tcnt1l           port    $2c            ; Zählregister Timer 1 (LSB)
tcnt1h           port    $2d            ;                      (MSB)

                 if      MOMCPU>=$904414
ocr1al            port    $2a           ; Vergleichsregister A Timer 1 (LSB)
ocr1ah            port    $2b           ;                              (MSB)
ocr1bl            port    $28           ; Vergleichsregister B Timer 1 (LSB)
ocr1bh            port    $29           ;                              (MSB)
                 elseif
ocr1l             port    $2a           ; Vergleichsregister Timer 1 (LSB)
ocr1h             port    $2b           ;                            (MSB)
                 endif

icr1l            port    $24            ; Fangwert Timer 1 (LSB)
icr1h            port    $25            ;                  (MSB)
                endif

;----------------------------------------------------------------------------
; Watchdog

wdtcr           port    $21             ; Watchdog-Steuerregister:
wdp0            equ     0               ;  Vorteiler
wdp1            equ     1
wdp2            equ     2
wde             equ     3               ; Freigabe
                if 	MomCPU>=$902313
wdttoe		 equ	 4		; zur Sperre gebraucht
                endif

;----------------------------------------------------------------------------
; serielle Ports

                if      MOMCPU>=$902312
udr              port    $0c            ; Datenregister UART

usr              port    $0b            ; Statusregister UART:
or               equ     3              ;  Empfängerüberlauf
fe               equ     4              ;  Framing-Fehler
udre             equ     5              ;  Datenregister wieder frei
txc              equ     6              ;  Sendung komplett
rxc              equ     7              ;  Empfang komplett

ucr              port    $0a            ; Steuerregister UART:
txb8             equ     0              ;  Sendebit 8
rxb8             equ     1              ;  Empfangsbit 8
chr9             equ     2              ;  auf 9-Bit-Datenwerte umschalten
txen             equ     3              ;  Sender freigeben
rxen             equ     4              ;  Empfänger freigeben
udrie            equ     5              ;  Interrupts bei freiem Datenregister freigeben
txcie            equ     6              ;  Interrupts nach Versand freigeben
rxcie            equ     7              ;  Interrupts nach Empfang freigeben

ubrr             port    $09            ; Baudratengenerator
                endif

                if      MOMCPU>=$904414
spcr             port    $0d            ; SPI Steuerregister:
spr0             equ     0              ;  Wahl Taktfrequenz
spr1             equ     1
cpha             equ     2              ;  Taktphase
cpol             equ     3              ;  Taktpolarität
mstr             equ     4              ;  Master/Slave-Wahl
dord             equ     5              ;  Bitreihenfolge
spe              equ     6              ;  SPI freigeben
spie             equ     7              ;  Interruptfreigabe SPI

spsr             port    $0e            ; SPI Statusregister:
wcol             equ     6              ;  Schreibkollision ?
spif             equ     7              ;  SPI-Interrupt aufgetreten ?

spdr             port    $0f            ; SPI Datenregister
                endif

;----------------------------------------------------------------------------
; Analogkomparator

acsr            port    $08             ; Komparator-Steuer/Statusregister:
acis0           equ     0               ;  Interrupt-Modus
acis1           equ     1
		if	MomCPU>=$902313
acic             equ     2              ;  Komparator als Fangsignal für Timer 1 benutzen
		endif
acie            equ     3               ;  Interrupt freigeben
aci             equ     4               ;  Interrupt aufgetreten ?
aco             equ     5               ;  Komparatorausgang
acd		equ	7		;  Strom abschalten

;----------------------------------------------------------------------------
; EEPROM

		if MomCPU>=$908515
eearl		 port	 $1e		; Adreßregister
eearh		 port	 $1f
		elseif 
eear             port    $1e
		endif

eedr            port    $1d             ; Datenregister

eecr            port    $1c             ; Steuerregister:
eere            equ     0               ;  Lesefreigabe
eewe            equ     1               ;  Schreibfreigabe
		if	MomCPU>=$902313
eemwe		 equ	 2
		endif

;----------------------------------------------------------------------------
; Vektoren
; Leider verschieben sich Vektoren bei den höheren Prozessoren.
; Warum nur, Atmel, warum ?

vec_reset       label   0               ; Reset-Einsprung
vec_int0        label   1               ; Einsprung ext. Interrupt 0
                switch  MOMCPUNAME
                case    "AT90S1200"
vec_tm0ovf       label   2              ; Einsprung Überlauf Timer 0
vec_anacomp      label   3              ; Einsprung Analog-Komparator
                case    "AT90S2313"
vec_int1         label   2              ; Einsprung ext. Interrupt 2
vec_tm1capt      label   3              ; Einsprung Fang Timer 1
vec_tm1comp      label   4              ; Einsprung Vergleich Timer 1
vec_tm1ovf       label   5              ; Einsprung Überlauf Timer 1
vec_tm0ovf       label   6              ; Einsprung Überlauf Timer 0
vec_uartrx       label   7              ; Einsprung UART Empfang komplett
vec_uartudre     label   8              ; Einsprung UART Datenregister leer
vec_uarttx       label   9              ; Einsprung UART Sendung komplett
vec_anacomp      label   10             ; Einsprung Analog-Komparator
                case    "AT90S4414","AT90S8515"
vec_int1         label   2              ; Einsprung ext. Interrupt 2
vec_tm1capt      label   3              ; Einsprung Fang Timer 1
vec_tm1compa     label   4              ; Einsprung Vergleich A Timer 1
vec_tm1compb     label   5              ; Einsprung Vergleich A Timer 1
vec_tm1ovf       label   6              ; Einsprung Überlauf Timer 1
vec_tm0ovf       label   7              ; Einsprung Überlauf Timer 0
vec_spi          label   8              ; Einsprung SPI-Interrupt
vec_uartrx       label   9              ; Einsprung UART Empfang komplett
vec_uartudre     label   10             ; Einsprung UART Datenregister leer
vec_uarttx       label   11             ; Einsprung UART Sendung komplett
vec_anacomp      label   12             ; Einsprung Analog-Komparator
                endcase

;----------------------------------------------------------------------------
; Speicheradressen

eestart         equ     0               ; Startadresse internes EEPROM
iram            equ     96,data         ; Startadresse internes SRAM
                                        ; (hinter gemapptem I/O)
irom            label   0               ; Startadresse internes EPROM

                switch  MOMCPUNAME
                case    "AT90S1200"
eeend            equ     63             ; Endadresse EEPROM
iramend          equ     95,data        ; Endadresse SRAM
iromend          label   1023           ; Endadresse EPROM
                case    "AT90S2313"
eeend            equ     127
iramend          equ     $df,data
iromend          label   2047
                case    "AT90S4414"
eeend            equ     255
iramend          equ     $15f,data
iromend          label   4095
                case    "AT90S8515"
eeend            equ     511
iramend          equ     $25f,data
iromend          label   8191
                endcase

;----------------------------------------------------------------------------

                endif

                restore                 ; wieder erlauben



