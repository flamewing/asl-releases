; FLOAT.INC
;******************************************************************************
;* Gleitkommabibliothek fÅr TLCS 90                                           *
;*                                                                            *
;* Originale fÅr den Z80 aus mc 12/88,1/89                                    *
;* Portierung auf TLCS 90 von Alfred Arnold, Dezember 1993                    *
;*                                                                            *
;*  Routine  Funktion             Eingabe Ausgabe  Stack    LÑnge  Zeit/10MHz *
;*                                                                            *
;*  fadd     Addition             2*Stack  BC-DE  14 Byte 347 Byte   248 us   *
;*  fsub     Subtraktion          2*Stack  BC-DE  14 Byte  12 Byte   255 us   *
;*  fmul     Multiplikation       2*Stack  BC-DE  20 Byte 356 Byte   936 us   *
;*  fdiv     Division             2*Stack  BC-DE  22 Byte 303 Byte  1081 us   *
;*  fmul2    Mult. mit 2er-Potenz Stack,A  BC-DE  10 Byte 162 Byte    28 us   *
;*  fsqrt    Quadratwurzel        Stack    BC-DE  22 Byte 621 Byte  1900 us   *
;*  fitof    Int-->Float          Stack    BC-DE  10 Byte  84 Byte  160 us *) *
;*  fftoi    Float-->Int          Stack    BC-DE  10 Byte 104 Byte  170 us *) *
;*  fftoa    Float-->ASCII        3*Stack  -----  40 Byte 451 Byte     *)     *
;*  fatof    ASCII-->Float        Stack  C,BC-DE  42 Byte 396 Byte     *)     *
;*                                                                            *
;*  *) Die AusfÅhrungszeiten streuen je nach Operand sehr stark und kînnen    *
;*     bei den ASCII-Funktionen bei vielen Millisekunden liegen.              *
;*                                                                            *
;*  - Parametereingabe Åber den Stack bedeutet, da· die Parameter mittels     *
;*    PUSH vor dem Aufruf auf den Stack gelegt werden mÅssen.  Diese Werte    *
;*    werden von den Unterroutinen am Ende automatisch vom Stack entfernt.    *
;*    Der zur öbergabe benîtigte Platz ist bei den Angaben zur Stackbelastung *
;*    eingerechnet!                                                           *
;*  - Wollen Sie einzelne Routinen entfernen, so beachten Sie, da· fsub Teile *
;*    aus fadd, fdiv Teile aus fmul sowie fftoi Teile aus fitof verwendet !   *
;*  - Gleitkommaformat ist IEEE Single (32 Bit)                               *
;*  - Integerwerte bei fmul2, fitof und fftoi sind vorzeichenbehaftet         *
;*  - Da die Routinen lokale Labels verwenden, ist mindestens AS 1.39 erfor-  *
;*    derlich                                                                 *
;*  - MACROS.INC mu· vorher eingebunden werden                                *
;******************************************************************************

                section float

;------------------------------------------------------------------------------
; modulglobale Konstanten

MaxExpo         equ     255
Bias            equ     127

OpSize          equ     4               ; Grî·e eines Operanden

fIX_alt         equ     0               ; Top of Stack liegt IX
FAdr            equ     2               ; RÅcksprungadresse
Op2             equ     4               ; Adresse Operand 2
Op1             equ     Op2+OpSize      ; Adresse Operand 1

Ld10:           dd      ld(10)
One:            dd      1.0
Ten:            dd      10.0
Tenth:          dd      3dcccccdh       ; =0.1, aber die Rundung auf manchen
					; Systemen variiert (damit Test nicht
					; scheitert)
Half:           dd      0.5

cpsh            macro   reg,op,{NoExpand}
                ld      reg,(op+2)
                push    reg
                ld      reg,(op)
                push    reg
                endm

;------------------------------------------------------------------------------
; Addition

                proc    fadd
                link    ix,0            ; Eintritt

                push    af              ; Register retten
                push    hl

                public  AddSub:Parent   ; Einsprung fÅr fsub

AddSub:         ld      a,(ix+Op1+3)    ; Vorzeichen Operand 1 laden
                ld      e,a             ; Ergebnisvorzeichen in E, Bit 7
                xor     a,(ix+Op2+3)    ; mit Vorzeichen von Op2 verknÅpfen
                ld      d,a             ; Subtraktionsflag in D, Bit 7
                res     7,(ix+Op1+3)    ; Vorzeichen in Mantisse 1 lîschen
                res     7,(ix+Op2+3)    ; Vorzeichen in Mantisse 2 lîschen

; Die Operanden sind jetzt in der Form 0eee eeee efff ... ffff

                ld      hl,(ix+Op1)     ; Differenz Op1-Op2 bilden
                sub     hl,(ix+Op2)
                ld      hl,(ix+Op1+2)
                sbc     hl,(ix+Op2+2)
                jr      nc,Ad_1         ; Sprung falls Op1>Op2
                ld      bc,(ix+Op1)     ; ansonsten Operanden vertauschen
                ex      bc,(ix+Op2)
                ld      (ix+Op1),bc
                ld      bc,(ix+Op1+2)
                ex      bc,(ix+Op2+2)
                ld      (ix+Op1+2),bc
                ld      a,e             ; Ergebnisvorzeichen neu berechnen
                xor     a,d
                ld      e,a

Ad_1:           ld      a,(ix+Op1+2)    ; Exponent der grî·eren Zahl laden
                ld      c,(ix+Op1+3)
                slaa
                rl      c
                jr      z,Den1
                set     7,(ix+Op1+2)    ; implizite Eins erzeugen
Den1:           ld      a,(ix+Op2+2)    ; dito Zahl 2
                ld      b,(ix+Op2+3)
                slaa
                rl      b
                jr      z,Den2
                set     7,(ix+Op2+2)

Den2:           push    bc              ; jetzt die Register fÅr den
                push    de              ; Blocktransferbefehl retten
                ld      bc,2*OpSize-1   ; beide Operanden verschieben
                ld      hl,ix           ; HL zeigt auf letztes Byte
                add     hl,Op2+2*OpSize-1
                ld      de,hl           ; HL nach DE kopieren
                dec     hl              ; HL zeigt auf vorletztes Byte
                lddr                    ; Verschiebung beider Mantissen
                pop     de              ; um 8 Bit nach links
                pop     bc

                xor     a,a
                ld      (ix+Op1),a      ; Form: ffff ... ffff 0000 0000
                ld      (ix+Op2),a
                ld      a,c             ; Differenz der Exponenten berechnen
                sub     a,b
                ld      b,a             ; Differenz nach B fÅr LOOP-Befehl
                jr      z,N_Anp         ; falls Null, keine Anpassung
                cp      a,25            ; mehr als 24? (Abfrage mit Carry
                jp      c,Anp           ; erfordert Vergleich mit 25)
                ld      b,0             ; !!!!
                jp      Round

Anp:            srl     (ix+Op2+3)      ; Anpassung der zweiten Mantisse
                rr      (ix+Op2+2)      ; durch Verschiebung nach rechts
                rr      (ix+Op2+1)
                rr      (ix+Op2)
                djnz    Anp             ; bis B=0

N_Anp:          bit     7,d             ; Addition oder Subtraktion ?
                jr      nz,Subtract     ; ggfs. zur Subtraktion springen
                ld      hl,(ix+Op1)     ; jetzt werden die beiden Mantissen
                add     hl,(ix+Op2)     ; zueinander addiert
                ld      (ix+Op1),hl
                ld      hl,(ix+Op1+2)
                adc     hl,(ix+Op2+2)
                ld      (ix+Op1+2),hl
                jr      nc,Round        ; kein öberlauf-->zum Runden
                rr      (ix+Op1+3)      ; öberlauf einschieben
                rr      (ix+Op1+2)
                rr      (ix+Op1+1)
                rr      (ix+Op1)
                inc     bc              ; Exponent erhîhen (B ist 0 durch
                jr      Round           ; Schleife), zum Runden

Subtract:       ld      hl,(ix+Op1)     ; beide Mantissen werden voneinander
                sub     hl,(ix+Op2)     ; subtrahiert
                ld      (ix+Op1),hl
                ld      hl,(ix+Op1+2)
                sbc     hl,(ix+Op2+2)
                ld      (ix+Op1+2),hl
                jr      m,Round         ; bei fÅhrender Eins zum Runden
                jr      nz,Norm         ; ungleich 0 ?  Dann zum Normalisieren
                cp      hl,(ix+Op1)     ; Rest der Mantisse auch Null ?
                jr      eq,Zero         ; alles Null --> Ergebnis ist Null

Norm:           ld      a,b             ; Exponent noch nicht Null ?
                or      a,c
                jr      z,Round
                dec     bc              ; Exponent erniedrigen
                sla     (ix+Op1)        ; Mantisse normalisieren, bis
                rl      (ix+Op1+1)      ; fÅhrende Eins auftaucht
                rl      (ix+Op1+2)
                rl      (ix+Op1+3)
                jr      p,Norm          ; noch keine Eins-->weitermachen

Round:          add     (ix+Op1),80h    ; jetzt Runden auf Bit hinter Mantisse
                jr      nc,NoOver       ; kein öbertrag ?
                inc     (ix+Op1+1)      ; doch, nÑchstes Mantissenbyte
                jr      nz,NoOver       ; behandeln, jetzt auf Null prÅfen,
                inc     (ix+Op1+2)      ; da der INC-Befehl kein Carry liefert
                jr      nz,NoOver
                inc     (ix+Op1+3)
                jr      nz,NoOver
                scf                     ; fÅhrende Eins erzeugen
                rr      (ix+Op1+3)      ; bei öberlauf Mantisse durch
                rr      (ix+Op1+2)      ; Rechtsschieben wieder normalisieren
                rr      (ix+Op1+1)      ; (nur fÅr 24 Bit notwendig)
                inc     bc              ; und Exponent korrigieren

NoOver:         xor     a,a             ; A = 0
                cp      a,(ix+Op1+3)    ; Mantisse auf Null prÅfen
                jr      nz,NoZero
                cp      a,(ix+Op1+2)
                jr      nz,NoZero
                cp      a,(ix+Op1+1)    ; alle Mantissenbytes Null ?
                jr      nz,NoZero       ; dann ist auch das Ergebnis Null

Zero:           ld      b,a             ; Null-Ergebnis aufbauen
                ld      c,a
                ld      de,bc
                jr      Exit            ; dann Routine verlassen

NoZero:         cp      a,b             ; A ist Null
                ld      a,MaxExpo       ; Exponent oberes Byte ungleich Null ?
                jr      nz,Over         ; dann ist öberlauf eingetreten
                cp      a,c             ; oder genau MaxExpo erreicht ?
                jr      nz,NoUe
Over:           ld      c,a             ; Exponent auf MaxExpo setzen
                xor     a,a             ; und Mantisse auf Null
                ld      (ix+Op1+3),a
                ld      (ix+Op1+2),a
                ld      (ix+Op1+1),a
                jr      DeNorm

NoUe:           xor     a,a             ; A = 0
                cp      a,c             ; Exponent Null (Zahl denormalisiert ?
                jr      z,DeNorm        ; ja -->
                sla     (ix+Op1+1)      ; fÅhrendes Bit wird nicht gespeichert
                rl      (ix+Op1+2)      ; daher Mantisse um 1 Bit nach links
                rl      (ix+Op1+3)

DeNorm:         ld      b,c             ; Ergebnis aufbauen: Exponent in B
                ld      c,(ix+Op1+3)    ; Mantisse oberstes Byte
                ld      d,(ix+Op1+2)
                sla     e               ; Vorzeichen aus E in Carry schieben
                ld      e,(ix+Op1+1)
                rr      b               ; Vorzeichen in Ergebnis einschieben
                rr      c
                rr      d
                rr      e

Exit:           pop     hl              ; Register restaurieren
                pop     af

                unlk    ix              ; Austritt
                retd    2*OpSize        ; Parameter abrÑumen
                endp

;------------------------------------------------------------------------------
; Subtraktion

                proc    fsub

                link    ix,0            ; Eintritt

                push    af              ; Register retten
                push    hl

                xor     (ix+Op2+3),80h  ; Vorzeichen Operand 2 kippen

                jrl     AddSub          ; weiter wie Addition

                endp

;------------------------------------------------------------------------------
; Multiplikation

                proc    fmul

                DefLocal temp,6         ; Platz TemporÑrvariable

                link    ix,LocalSize    ; Platz auf Stack reservieren

                push    af              ; Register retten
                push    hl

                ld      a,(ix+Op1+3)    ; Ergebnisvorzeichen bestimmen
                xor     a,(ix+Op2+3)
                ld      c,a             ; in C merken

                ld      d,0             ; Exponent 1 laden
                ld      e,(ix+Op1+3)
                ld      a,(ix+Op1+2)
                slaa                    ; Exponent unterstes Bit in Carry
                rl      e               ; und in E einschieben
                srla                    ; ergibt Bit 7=0
                ld      (ix+Op1+3),a    ; impl. Null vorbesetzen+um 8 Bit schieben
                cp      e,0
                jr      z,Den1          ; falls Null, dann denormalisiert
                set     7,(ix+Op1+3)    ; ansonsten impl. Eins erzeugen
                dec     de              ; Bias kompensieren
Den1:           ld      hl,(ix+Op1)     ; jetzt restliche Bytes verschieben
                ld      (ix+Op1+1),hl
                xor     hl,hl           ; unterste Mantissenbits lîschen
                ld      (ix+Op1),h      ; Form: ffff ... ffff 0000 0000

                ld      (ix+temp+4),hl  ; lokale Variable mit Null vorbesetzen
                ld      (ix+temp+2),hl
                ld      (ix+temp),hl

                ld      l,(ix+Op2+3)    ; Exponent 2 in HL aufbauen
                ld      a,(ix+Op2+2)
                res     7,(ix+Op2+2)    ; gleiches Verfahren wie Op1
                slaa
                rl      l
                jr      z,Den2
                set     7,(ix+Op2+2)
                dec     hl
Den2:
                add     hl,de           ; Exponenten aufaddieren
                sub     hl,Bias-3       ; Bias-3 subtrahieren
                jp      p,NoZero        ; positiv-->kein Unterlauf
                ld      a,l             ; Exponent <-24 ?
                cp      a,-24
                jr      nc,NoZero
                jp      MulZero         ; ja, dann ist Ergebnis Null

NoZero:         ld      b,24            ; SchleifenzÑhler Multiplikation
                ld      de,0            ; Hilfsregister Multiplikand
                push    hl              ; HL zum Addieren benutzen
Multiply:       srl     (ix+Op1+3)      ; Multiplikand nach rechts schieben
                rr      (ix+Op1+2)
                rr      (ix+Op1+1)
                rr      (ix+Op1)
                rr      d               ; DE als VerlÑngerung von Operand 1
                rr      e
                sla     (ix+Op2)        ; Multiplikator nach links schieben
                rl      (ix+Op2+1)
                rl      (ix+Op2+2)      ; falls fÅhrendes Bit 0, nicht addieren
                jr      nc,NoAdd
                ld      hl,(ix+temp)    ; sonst aufaddieren
                add     hl,de
                ld      (ix+temp),hl
                ld      hl,(ix+temp+2)
                adc     hl,(ix+Op1)
                ld      (ix+temp+2),hl
                ld      hl,(ix+temp+4)
                adc     hl,(ix+Op1+2)
                ld      (ix+temp+4),hl
NoAdd:          djnz    Multiply        ; Schleife durchlaufen
                pop     hl
                ld      a,(ix+temp+5)
                or      a,a             ; Flags setzen
                jp      m,MulRound      ; bei fÅhrender Eins zum Runden
                jr      nz,Normalize    ; ansonsten normalisieren
                cp      a,(ix+temp+4)
                jr      nz,Normalize
                cp      a,(ix+temp+3)
                jr      nz,Normalize
                cp      a,(ix+temp+2)
                jr      Normalize
                jp      MulZero         ; komplett Null-->Ergebnis Null

Normalize:      bit     7,h             ; Exponent negativ ?
                jp      nz,Underrun     ; ggf. Unterlauf behandlen

Norm1:          cp      hl,0            ; Exponent=0 ?
                jr      z,MulRound
                dec     hl              ; Exponent erniedrigen,
                sla     (ix+temp)       ; Mantisse verschieben...
                rl      (ix+temp+1)
                rl      (ix+temp+2)
                rl      (ix+temp+3)
                rl      (ix+temp+4)
                rl      (ix+temp+5)
                jp      p,Norm1         ; ...bis fÅhrende Eins auftaucht

                public  MulRound:Parent ; Einsprung fÅr Division
MulRound:       ld      a,(ix+temp+2)   ; jetzt Runden auf Bit hinter Mantisse
                add     a,80h
                jr      nc,NoOver       ; kein öbertrag
                inc     (ix+temp+3)     ; doch, nÑchstes Mantissenbyte
                jr      nz,NoOver       ; behandeln, jetzt auf Null prÅfen
                inc     (ix+temp+4)     ; da INC kein Carry liefert
                jr      nz,NoOver
                inc     (ix+temp+5)
                jr      nz,NoOver
                scf                     ; Eins erzeugen
                rr      (ix+temp+5)     ; bei öberlauf Mantisse durch
                rr      (ix+temp+4)     ; Rechtsschieben wieder normalisieren
                rr      (ix+temp+3)
                inc     hl              ; und Exponent korrigieren

NoOver:         cp      hl,MaxExpo      ; Exponent prÅfen
                jr      ult,NoUeber     ; kein öberlauf

                public  MulOver:Parent  ; Einsprung fÅr fdiv
MulOver:        ld      hl,MaxExpo      ; öberlauf: Exponent=MaxExpo
                ld      (ix+temp+5),h
                ld      (ix+temp+4),h
                ld      (ix+temp+3),h
                jr      DeNorm

NoUeber:        xor     a,a             ; A=0
                cp      a,l             ; Exponent ist Null ?
                jr      z,DeNorm        ; ja, Ergebnis ist denormalisiert
                sla     (ix+temp+3)     ; nein, fÅhrende=implizite Eins
                rl      (ix+temp+4)     ; rausschieben
                rl      (ix+temp+5)

DeNorm:         sla     c               ; Vorzeichen in Carry schieben
                ld      b,l             ; Exponent einsetzen
                ld      c,(ix+temp+5)
                ld      d,(ix+temp+4)
                ld      e,(ix+temp+3)
                rr      b               ; und Vorzeichen einschieben
                rr      c
                rr      d
                rr      e               ; Form: seee eeee efff ffff ... ffff

Result:         pop     hl              ; Register zurÅck
                pop     af

                unlk    ix              ; Stackrahmen abbauen
                retd    2*OpSize        ; Operanden abrÑumen

                public  MulZero:Parent  ; Einsprung fÅr fdiv
MulZero:        xor     a,a             ; Ergebnis ist Null
                ld      b,a
                ld      c,a
                ld      d,a
                ld      e,a
                jr      Result

Underrun:       ld      a,l             ; Exponent in A
                neg     a               ; negieren fÅr SchleifenzÑhler
                cp      a,24            ; totaler Unterlauf ?
                jr      nc,MulZero      ; ja, dann ist Ergebnis Null
                ld      b,a             ; Mantisse denormalisieren
Shr:            srl     (ix+temp+5)     ; bis Exponent Null ist
                rr      (ix+temp+4)
                rr      (ix+temp+3)
                djnz    Shr
                ld      l,b             ; Exponent in Register L=B=0
                jp      Denorm          ; denormalisiertes Ergebnis erzeugen

                endp

;------------------------------------------------------------------------------
; Division

                proc    fdiv

                DefLocal temp,6         ; Platz TemporÑrvariable

                link    ix,LocalSize    ; 6 Byte Platz auf Stack reservieren

                push    af              ; Register retten
                push    hl

                ld      a,(ix+Op1+3)    ; Ergebnisvorzeichen bestimmen
                xor     a,(ix+Op2+3)
                ld      c,a             ; Vorzeichen in C Bit 7 merken
                push    bc              ; Vorzeichen retten

                ld      h,0             ; Exponent 1 laden
                ld      l,(ix+Op1+3)
                ld      a,(ix+Op1+2)
                res     7,(ix+Op1+2)    ; impl. Null vorbesetzen
                slaa                    ; Exponent unterstes Bit in Carry
                rl      l               ; und in L einschieben
                jr      z,Den1          ; falls Null, dann Op1 denormalisiert
                set     7,(ix+Op1+2)    ; implizite Eins erzeugen
                dec     hl              ; Bias kompensieren
Den1:
                ld      d,0             ; Exponent 2 in DE aufbauen
                ld      e,(ix+Op2+3)
                ld      a,(ix+Op2+2)
                ld      (ix+Op2+3),a    ; Verfahren wie oben
                res     7,(ix+Op2+3)
                slaa
                rl      e
                jr      z,Den2
                set     7,(ix+Op2+3)
                dec     de
Den2:
                ld      bc,(ix+Op2)     ; jetzt restliche Bytes kopieren
                ld      (ix+Op2+1),bc
                xor     a,a             ; A=0
                ld      (ix+Op2),a      ; Form: ffff ... ffff 0000 0000
                srl     (ix+Op2+3)
                rr      (ix+Op2+2)
                rr      (ix+Op2+1)
                rr      (ix+Op2)        ; Form: 0fff ... ffff f000 0000
                jr      nz,NoZero1      ; Mantisse 2 auf Null prÅfen
                cp      a,(ix+Op2+1)
                jr      nz,NoZero1
                cp      a,(ix+Op2+2)
                jr      nz,NoZero1
                cp      a,(ix+Op2+3)
                jr      nz,NoZero1
                jp      MulOver

NoZero1:        xor     a,a             ; Carry-Flag lîschen
                sbc     hl,de           ; Exponenten subtrahieren
                add     hl,Bias         ; Bias addieren
                jr      p,NoZero        ; Exponent negativ ?
                cp      l,-24           ; Exponent kleiner als -24 ?
                jr      nc,NoZero
                jp      MulZero         ; ja, dann ist das Ergebnis Null
NoZero:
                add     hl,25           ; Exponent um 25 erhîhen; jetzt ist er sicher grî·er als Null
                xor     a,a             ; A=0
                ld      bc,(ix+Op1+1)   ; Divident in Register kopieren
                ld      d,(ix+Op1)
                ld      e,a             ; die untersten Bits sind Null
                cp      a,d             ; ist Divident Null ?
                jr      nz,NoZero2
                cp      a,c
                jr      nz,NoZero2
                cp      a,b
                jr      nz,NoZero2
                pop     bc              ; Stack bereinigen (Vorzeichen laden)
                jp      MulZero         ; und Null als Ergebnis ausgeben
NoZero2:
                ld      (ix+temp+5),a   ; Ergebnis vorbesetzen
                ld      (ix+temp+4),a
                ld      (ix+temp+3),a
                ld      (ix+temp+2),a

NormLoop:       bit     6,(ix+Op2+3)    ; ist der Divisor normalisiert ?
                jr      nz,Norm         ; ja-->
                inc     hl              ; nein, Exponent erhîhen
                sla     (ix+Op2)        ; Divisor verschieben bis in
                rl      (ix+Op2+1)      ; Form 01ff ...
                rl      (ix+Op2+2)
                rl      (ix+Op2+3)
                jr      NormLoop
Norm:           srl     b
                rr      c
                rr      d
                rr      e               ; Form: 0fff ... ffff f000 0000

                push    iy              ; Exponent nach IY
                ld      iy,hl
Loop:           ld      (ix+Op1+2),bc   ; Divident zwischenspeichern
                                        ; die SpeicherplÑtze von Op1
                ld      (ix+Op1),de     ; stehen zur VerfÅgung, da wir Op1
                                        ; in die Register BC-DE kopiert haben
                ld      hl,de           ; jetzt Divisor abziehen
                sub     hl,(ix+Op2)
                ld      de,hl
                ld      hl,bc
                sbc     hl,(ix+Op2+2)
                ld      bc,hl
                jr      nc,IsOne        ; kein Carry: Divisor pa·t
                ld      de,(ix+Op1)     ; ansonsten zurÅckkopieren
                ld      bc,(ix+Op1+2)   ; Carry bleibt erhalten!
IsOne:          ccf                     ; Carry-Flag umdrehen
                rl      (ix+temp+2)     ; Ergebnis aufbauen
                rl      (ix+temp+3)
                rl      (ix+temp+4)
                rl      (ix+temp+5)
                sla     e               ; Divident verschieben
                rl      d
                rl      c
                rl      b

                add     iy,-1           ; Exponent erniedrigen
                jr      z,DeNorm        ; falls Null, dann denormalisiert
                bit     0,(ix+temp+5)   ; fÅhrende Eins in Ergebnis-Mantisse ?
                jr      z,Loop          ; nein, weiter rechnen

DeNorm:         ld      hl,iy           ; Exponent zurÅck
                ld      b,(ix+temp+5)   ; hîchstes Bit merken
                ld      a,(ix+temp+4)
                ld      (ix+temp+5),a   ; Mantisse in Form
                ld      iy,(ix+temp+2)  ; ffff ... ffff 0000 0000
                ld      (ix+temp+3),iy
                pop     iy              ; IY erst jetzt freigeben
                rr      b               ; hîchstes Bit einschieben
                rr      (ix+temp+5)
                rr      (ix+temp+4)
                rr      (ix+temp+3)
                rr      (ix+temp+2)

                pop     bc              ; Vorzeichen wieder laden
                xor     a,a             ; A=0
                cp      a,(ix+temp+5)   ; Mantisse ist Null ?
                jr      nz,NoZero3
                cp      a,(ix+temp+4)
                jr      nz,NoZero3
                cp      a,(ix+temp+3)
                jr      nz,NoZero3
                cp      a,(ix+temp+2)
                jp      z,MulZero
NoZero3:
                jp      MulRound

                endp

;------------------------------------------------------------------------------
; Wandlung Integer-->Gleitkomma

                proc    fitof

                link    ix,0            ; Stackrahmen aufbauen
                push    af              ; Register retten
                push    hl

                ld      bc,(ix+Op2+2)   ; Operanden hereinholen
                ld      de,(ix+Op2)     ; Reihenfolge: BCDE

                ld      hl,bc           ; Operand = 0 ?
                or      hl,de
                jr      z,ItofResult    ; dann Ergebnis Null

                bit     7,b             ; Zahl positiv ?
                jr      z,Positive
                ld      hl,bc           ; dann Zahl negieren
                xor     hl,-1
                ld      bc,hl
                ld      hl,de
                xor     hl,-1
                inc     hl
                or      hl,hl
                ld      de,hl
                jr      nz,Positive
                inc     bc

Positive:       ld      l,Bias+32       ; Exponent vorbesetzen
Shift:          dec     l
                sla     e               ; Mantisse verschieben, bis fÅhrende
                rl      d               ; Eins auftaucht
                rl      c
                rl      b
                jr      nc,Shift
                ld      e,d             ; Exponent einsetzen
                ld      d,c
                ld      c,b
                ld      b,l
                sla     (ix+Op2+3)      ; Vorzeichen in Carry
                rr      b               ; ins Ergebnis einschieben
                rr      c
                rr      d
                rr      e

                public  ItofResult:Parent
ItofResult:     pop     hl              ; Register zurÅck
                pop     af
                unlk    ix              ; abbauen
                retd    4               ; Ende

                endp

;------------------------------------------------------------------------------
; Wandlung Gleitkomma-->Integer

                proc    fftoi

                link    ix,0            ; Stackrahmen aufbauen

                push    af              ; Register retten
                push    hl

                ld      d,(ix+Op2)      ; Operand in Register laden
                ld      bc,(ix+Op2+1)   ; Reihenfolge: EBCD
                ld      e,(ix+Op2+3)    ; erspart spÑter Vertauschungen

                ld      h,e             ; Vorzeichen in H, Bit 7
                ld      a,e             ; Exponent in A aufbauen
                sla     b               ; LSB aus B holen
                rla
                scf                     ; impl. Eins einschieben
                rr      b
                sub     a,Bias
                ld      l,a             ; Exponent nach L kopieren
                jp      m,Zero          ; falls keiner Null, Ergebnis Null
                ld      a,30
                cp      a,l             ; grî·er 30 ?
                jr      c,Over          ; dann öberlauf
                ld      e,0             ; Zahl jetzt in BCDE in der Form
                inc     a               ; 1fff ... ffff 0000 0000

Shift:          srl     b               ; jetzt Mantisse verschieben
                rr      c
                rr      d
                rr      e
                inc     l
                cp      a,l             ; bis Exponent stimmt
                jr      nz,Shift
                bit     7,h             ; Zahl negativ ?
                jr      z,ItofResult    ; nein, fertig

                ld      hl,de           ; Zahl negieren
                xor     hl,-1
                ld      de,hl
                ld      hl,bc
                xor     hl,-1
                ld      bc,hl
                inc     de
                jr      nz,ItofResult
                inc     bc
                jr      nz,ItofResult

Zero:           ld      bc,0
                ld      de,bc
                jp      ItofResult      ; Ergebnis Null

Over:           bit     7,h             ; Ergebnis positiv ?
                jr      z,OpPos
                ld      b,80h           ; MININT laden
                xor     a,a             ; A=0
                ld      c,a
                ld      d,a
                ld      e,a
                jp      ItofResult
OpPos:          ld      b,7fh           ; MAXINT laden
                ld      a,0ffh
                ld      c,a
                ld      d,a
                ld      e,a
                jp      ItofResult

                endp

;------------------------------------------------------------------------------
; Multiplikation mit Zweierpotenz (in A)

                proc    fmul2

                link    ix,0            ; Stackrahmen aufbauen

                push    af              ; Register retten
                push    hl

                ld      de,(ix+Op2)     ; Operand 1 in Register laden
                ld      bc,(ix+Op2+2)

                ld      h,a             ; Operand 2 nach H kopieren
                ld      l,b             ; Vorzeichen nach L, Bit 7
                xor     a,a             ; A=0
                cp      a,b             ; Operand 1 = Null ?
                jr      nz,NoZero
                cp      a,c
                jr      nz,NoZero
                cp      a,d
                jr      nz,NoZero
                cp      a,e
                jr      z,Zero

NoZero:         sla     e               ; Operand 1 verschieben
                rl      d
                rl      c
                rl      b               ; Form: eeee eeee ffff ... fff0
                jr      z,Den           ; Falls Exponent Null -->denormal

                add     a,h             ; A=0+H
                jr      m,Div           ; Falls Op2<0-->Division
                add     a,b             ; A=Summe der Exponenten
                ld      b,a             ; zurÅck nach B
                jr      c,Over          ; bei öberlauf-->
                cp      a,MaxExpo       ; oder genau MaxExpo
                jr      z,Over

Result:         sla     l               ; Vorzeichen in Carry schieben
                rr      b
                rr      c
                rr      d
                rr      e               ; Ergebnis zusammensetzen

Zero:           pop     hl              ; Register zurÅck
                pop     af

                unlk    ix              ; Stackrahmen abbauen
                retd    4               ; Ende

Over:           ld      b,MaxExpo       ; öberlauf: Exponent=MaxExpo
                xor     a,a             ;           Mantisse=0
                ld      c,a
                ld      d,a
                ld      e,a
                jr      Result

Div:            add     a,b             ; A = Summe der Exponenten
                ld      b,a             ; zurÅck nach B
                jr      z,Div2
                jr      p,Result        ; falls >0, Ergebnis abliefern
Div2:           scf                     ; implizite Eins real machen
                rr      c
                rr      d
                rr      e               ; Form: eeee eeee 1fff ... ffff

Denorm:         xor     a,a             ; A = 0
                cp      a,b             ; Exponent Null ?
                jr      z,Result        ; ja, ergebnis abliefern
                srl     c
                rr      d
                rr      e               ; Mantisse denormalisieren
                jr      nz,NoZero2
                cp      a,d
                jr      nz,NoZero2
                cp      a,c
                jr      nz,NoZero2
                ld      b,a             ; totaler Unterlauf, Ergebnis = Null
                jr      Zero

NoZero2:        inc     b               ; Exponent erhîhen
                jr      Denorm          ; weiter denormalisieren

DDD:            add     a,b             ; Summe der Exponenten bilden
                ld      b,a             ; zurÅck nach B
                jr      Denorm

Den:            add     a,h             ; A=0+H
                jr      m,DDD           ; bei Division verzweigen
NoOver:         sla     e               ; Multiplikation: Eine
                rl      d               ; denormalisierte Mantisse
                rl      c               ; wird wieder normalisiert
                jr      c,Stop          ; bis fÅhrende Eins rausfliegt
                dec     h               ; oder Operand 2 = Null
                jr      nz,NoOver
                jr      Result

Stop:           ld      a,h             ; Summe der Exponenten bilden
                add     a,b
                ld      b,a             ; zurÅck nach B
                jr      Result

                endp

;------------------------------------------------------------------------------
; Quadratwurzel ziehen

                proc    fsqrt

Op              equ     4               ; Lage Parameter
                DefLocal XRoot,4        ; Iterationsvariablen
                DefLocal m2,4
                DefLocal xx2,4

                link    ix,LocalSize    ; Stackrahmen aufbauen

                push    af              ; Register retten
                push    hl
                push    iy

                bit     7,(ix+Op+3)     ; negatives Argument ?
                jp      nz,DomainError  ; dann Fehler

                ld      hl,(ix+Op+2)    ; Exponent isolieren
                and     hl,07f80h
                jp      z,Zero          ; keine Behandlung denormaler Zahlen

                ld      (ix+Op+3),0     ; Mantisse isolieren
                and     (ix+Op+2),7fh
                sub     hl,7fh*80h      ; Bias vom Exponenten entfernen
                ld      bc,hl
                bit     7,c             ; Exponent ungerade ?
                res     7,c
                jr      z,EvenExp
                ld      hl,(ix+Op)      ; ja: Mantisse verdoppeln
                add     hl,hl
                ld      (ix+Op),hl
                ld      hl,(ix+Op+2)
                adc     hl,hl
                add     hl,100h-80h     ; impl. Eins dazu
                ld      (ix+Op+2),hl
EvenExp:
                sra     b               ; Exponent/2 mit Vorzeichen
                rr      c
                ld      hl,7fh*80h      ; Bias wieder dazu
                add     hl,bc
                ld      iy,hl           ; Exponent in IY aufheben
                ld      de,(ix+Op+1)    ; x ausrichten (um 7 nach links)
                ld      a,(ix+Op+3)     ; oberstes Byte merken
                ld      (ix+Op+2),de    ; da wir hier eins zuviel schieben
                ld      d,(ix+Op)
                ld      e,0
                ld      (ix+Op),de
                srla                    ; dieses Bit einschieben
                rr      (ix+Op+3)
                rr      (ix+Op+2)
                rr      (ix+Op+1)
                rr      (ix+Op)
                ld      de,0            ; vorbelegen
                ld      (ix+XRoot),de
                ld      (ix+m2),de
                ld      d,40h
                ld      (ix+XRoot+2),de
                ld      d,10h
                ld      (ix+m2+2),de
Loop10:         ld      de,(ix+Op)      ; xx2 = x
                ld      (ix+xx2),de
                ld      de,(ix+Op+2)
                ld      (ix+xx2+2),de
Loop11:         ld      hl,(ix+xx2)     ; xx2 -= xroot
                sub     hl,(ix+XRoot)
                ld      (ix+xx2),hl
                ld      hl,(ix+xx2+2)
                sbc     hl,(ix+XRoot+2)
                ld      (ix+xx2+2),hl
                srl     (ix+XRoot+3)    ; xroot /= 2
                rr      (ix+XRoot+2)
                rr      (ix+XRoot+1)
                rr      (ix+XRoot)
                ld      hl,(ix+xx2)     ; xx2 -= m2
                sub     hl,(ix+m2)
                ld      (ix+xx2),hl
                ld      hl,(ix+xx2+2)
                sbc     hl,(ix+m2+2)
                ld      (ix+xx2+2),hl
                jr      m,DontSet1
                ld      hl,(ix+xx2)     ; x = xx2
                ld      (ix+Op),hl
                ld      hl,(ix+xx2+2)
                ld      (ix+Op+2),hl
                ld      hl,(ix+XRoot)   ; xroot += m2
                or      hl,(ix+m2)
                ld      (ix+XRoot),hl
                ld      hl,(ix+XRoot+2)
                or      hl,(ix+m2+2)
                ld      (ix+XRoot+2),hl
                ld      hl,(ix+m2)      ; m2 /= 4
                ld      de,(ix+m2+2)
                rept    2
                 srl    d
                 rr     e
                 rr     h
                 rr     l
                endm
                ld      (ix+m2),hl
                ld      (ix+m2+2),de
                or      hl,de
                jr      nz,Loop11
                jr      IsSame
DontSet1:       ld      hl,(ix+m2)      ; m2 /= 4
                ld      de,(ix+m2+2)
                rept    2
                 srl    d
                 rr     e
                 rr     h
                 rr     l
                endm
                ld      (ix+m2),hl
                ld      (ix+m2+2),de
                or      hl,de
                jp      nz,Loop10       ; 15* abarbeiten
                                        ; Bit 22..8
                ld      hl,(ix+Op)      ; 17. Iteration separat
                ld      (ix+xx2),hl
                ld      hl,(ix+Op+2)
                ld      (ix+xx2+2),hl
IsSame:         ld      hl,(ix+xx2)
                sub     hl,(ix+XRoot)
                ld      (ix+xx2),hl
                ld      hl,(ix+xx2+2)
                sbc     hl,(ix+XRoot+2)
                ld      (ix+xx2+2),hl
                ld      de,(ix+XRoot+2) ; mitsamt Carry...
                ld      hl,(ix+XRoot)
                srl     d
                rr      e
                rr      h
                rr      l
                jr      nc,NoC1
                set     7,d
NoC1:           ld      (ix+XRoot+2),hl ; auf neues Alignment umstellen
                ld      (ix+XRoot),de
                decw    (ix+xx2)        ; Carry von 0-$4000: xx2 -= m2
                jr      nz,NoC2
                decw    (ix+xx2+2)
NoC2:           bit     7,(ix+xx2+3)
                jr      nz,DontSet7
                or      (ix+xx2+3),0c0h ; 0-$4000: x2 -= m2, Teil 2
                ld      hl,(ix+xx2)
                ld      (ix+Op),hl
                ld      hl,(ix+xx2+2)
                ld      (ix+Op+2),hl
                or      (ix+XRoot+1),40h; xroot += m2
DontSet7:       ld      hl,(ix+Op)      ; x auf neues Alignment umstellen
                ld      de,(ix+Op+2)
                ld      (ix+Op),de
                ld      (ix+Op+2),hl
                ld      hl,1000h        ; m2 - obere HÑlfte schon 0
                ld      (ix+m2),hl
Loop20:         ld      hl,(ix+Op)      ; xx2 = x
                ld      (ix+xx2),hl
                ld      hl,(ix+Op+2)
                ld      (ix+xx2+2),hl
Loop21:         ld      hl,(ix+xx2)     ; xx2 -= xroot
                sub     hl,(ix+XRoot)
                ld      (ix+xx2),hl
                ld      hl,(ix+xx2+2)
                sbc     hl,(ix+XRoot+2)
                ld      (ix+xx2+2),hl
                srl     (ix+XRoot+3)    ; XRoot = XRoot/2
                rr      (ix+XRoot+2)
                rr      (ix+XRoot+1)
                rr      (ix+XRoot)
                ld      hl,(ix+xx2)     ; x2 -= m2
                sub     hl,(ix+m2)
                ld      (ix+xx2),hl
                ld      hl,(ix+xx2+2)
                sbc     hl,(ix+m2+2)
                ld      (ix+xx2+2),hl
                jr      m,DontSet2
                ld      hl,(ix+xx2)     ; x = xx2
                ld      (ix+Op),hl
                ld      hl,(ix+xx2+2)
                ld      (ix+Op+2),hl
                ld      hl,(ix+XRoot)   ; xroot += m2
                or      hl,(ix+m2)
                ld      (ix+XRoot),hl
                ld      hl,(ix+XRoot+2)
                or      hl,(ix+m2+2)
                ld      (ix+XRoot+2),hl
                ld      hl,(ix+m2)      ; m2 /= 4
                ld      de,(ix+m2+2)
                rept    2
                 srl    d
                 rr     e
                 rr     h
                 rr     l
                endm
                ld      (ix+m2),hl
                ld      (ix+m2+2),de
                or      hl,de
                jr      nz,Loop21
                jr      Finish
DontSet2:       ld      hl,(ix+m2)      ; m2 /= 4
                ld      de,(ix+m2+2)
                rept    2
                 srl    d
                 rr     e
                 rr     h
                 rr     l
                endm
                ld      (ix+m2),hl
                ld      (ix+m2+2),de
                or      hl,de
                jp      nz,Loop20       ; 7* abarbeiten

Finish:         ld      hl,(ix+Op)      ; Aufrunden notwendig ?
                sub     hl,(ix+XRoot)
                ld      (ix+Op),hl
                ld      hl,(ix+Op+2)
                sub     hl,(ix+XRoot+2)
                ld      (ix+Op+2),hl
                jr      ule,NoInc
                incw    (ix+XRoot)      ; wenn ja, durchfÅhren
                jr      nz,NoInc
                incw    (ix+XRoot)
NoInc:          res     7,(ix+XRoot+2)  ; impl. Eins lîschen
                ld      hl,(ix+XRoot+2) ; Exponent einbauen
                or      hl,iy
                ld      bc,hl           ; Ergebnis in BC-DE
                ld      de,(ix+XRoot)
                jr      End

DomainError:    ld      bc,0ffc0h       ; - NAN zuÅckgeben
                ld      de,0
                jr      End

Zero:           ld      bc,0            ; Ergebnis 0
                ld      de,bc

End:            pop     iy              ; Register zurÅck
                pop     hl
                pop     af

                unlk    ix              ; Stackrahmen abbauen
                retd    4               ; Ende

                endp

;------------------------------------------------------------------------------
; Zehnerpotenz bilden

                subproc fPot10

                push    ix              ; Register retten
                push    iy
                push    hl

                ld      bc,(One+2)      ; Ausgangspunkt fÅrs Multiplizieren
                ld      de,(One)
                ld      ix,(Ten+2)      ; zu benutzende Potenz
                ld      iy,(Ten)
                or      hl,hl           ; negative Potenz?
                jr      p,IsPos
                ld      ix,(Tenth+2)    ; dann eben mit Zehntel
                ld      iy,(Tenth)
                xor     hl,-1           ; Zweierkomplement
                inc     hl
IsPos:
                or      hl,hl           ; weiter multiplizieren ?
                jr      z,End           ; nein, Ende
                bit     0,l             ; Restpotenz ungerade ?
                jr      z,IsEven
                push    bc              ; ja: einzeln multiplizieren
                push    de
                push    ix
                push    iy
                call    fmul
IsEven:         srl     h
                rr      l
                push    bc              ; nÑchste Potenz berechnen
                push    de
                push    ix              ; durch quadrieren
                push    iy
                push    ix
                push    iy
                call    fmul
                ld      ix,bc
                ld      iy,de
                pop     de
                pop     bc
                jr      IsPos           ; weitersuchen
End:
                pop     hl              ; Register zurÅck
                pop     iy
                pop     ix

                ret                     ; Ende

                endp

;------------------------------------------------------------------------------

                subproc fOutDec

Op              equ     6               ; Adresse Operand
Format          equ     4               ; Formatdeskriptor
                DefLocal Temp,4         ; 64-Bit-Erweiterung Divident

                link    ix,LocalSize

                push    af              ; Register retten
                push    bc
                push    de
                push    hl

                bit     7,(ix+Op+3)     ; negativ ?
                jr      z,IsPos
                ld      (iy),'-'        ; ja: vermerken...
                inc     iy
                ld      hl,(ix+Op)      ; ...und Zweierkomplement
                xor     hl,-1
                ld      (ix+Op),hl
                ld      hl,(ix+Op+2)
                xor     hl,-1
                ld      (ix+Op+2),hl
                incw    (ix+Op)
                jr      nz,GoOn
                incw    (ix+Op+2)
                jr      GoOn
IsPos:          bit     7,(ix+Format+1) ; Pluszeichen ausgeben ?
                jr      nz,GoOn
                ld      (iy),'+'
                inc     iy
GoOn:           res     7,(ix+Format+1) ; Plusflag lîschen
                ld      de,0            ; Nullflag & ZÑhler lîschen

InLoop:         ld      hl,0            ; Division vorbereiten
                ld      (ix+Temp),hl    ; dazu auf 64 Bit erweitern
                ld      (ix+Temp+2),hl
                ld      b,32            ; 32-Bit-Division
DivLoop:        sll     (ix+Op)         ; eins weiterschieben
                rl      (ix+Op+1)
                rl      (ix+Op+2)
                rl      (ix+Op+3)
                rl      (ix+Temp)
                rl      (ix+Temp+1)
                rl      (ix+Temp+2)
                rl      (ix+Temp+3)
                srl     (ix+Op)         ; fÅr nachher
                ld      hl,(ix+Temp)    ; probeweise abziehen
                sub     hl,10
                ld      (ix+Temp),hl
                ld      hl,(ix+Temp+2)
                sbc     hl,0
                ld      (ix+Temp+2),hl
                jr      nc,DivOK        ; pa·t es ?
                ld      hl,(ix+Temp)    ; nein, zurÅcknehmen
                add     hl,10
                ld      (ix+Temp),hl
                ld      hl,(ix+Temp+2)
                adc     hl,0
                ld      (ix+Temp+2),hl
                scf                     ; ins Ergebnis 0 einschieben
DivOK:          ccf                     ; neues Ergebnisbit
                rl      (ix+Op)         ; von unten einschieben
                djnz    DivLoop

                ld      a,(ix+Temp)     ; ASCII-Offset addieren
                add     a,'0'
                bit     0,d             ; schon im Nullbereich ?
                jr      z,NormVal
                ld      a,(ix+Format)   ; ja, dann gewÅnschtes Leerzeichen
NormVal:        push    af              ; auf LIFO legen
                inc     e               ; ein Zeichen mehr
                ld      a,(ix+Op)       ; Quotient Null ?
                or      a,(ix+Op+1)
                or      a,(ix+Op+2)
                or      a,(ix+Op+3)
                ld      d,0             ; Annahme: nicht Null
                jr      nz,InLoop       ; falls <>0, auf jeden Fall weiter
                ld      d,0ffh          ; Flag auf True setzen
                ld      a,e             ; ansonsten nur weiter, falls minimale
                cp      a,(ix+Format+1) ; Zahl noch nicht erreicht
                jr      ult,InLoop

                ld      b,e             ; jetzt Zeichen ausgeben
OutLoop:        pop     af
                ld      (iy),a
                inc     iy
                djnz    OutLoop

                pop     hl              ; Register zurÅck
                pop     de
                pop     bc
                pop     af

                unlk    ix
                retd    6

                endp

;------------------------------------------------------------------------------
; Wandlung Float-->ASCII

                proc    fftoa

Op              equ     8               ; Lage Eingabe auf Stack
Format          equ     6               ; Lage Formatdeskriptor auf Stack
Buffer          equ     4               ; Pufferadresse
                DefLocal Copy,4         ; TemporÑrkopie der Zahl
                DefLocal ExpSave,2      ; berechneter Exponent

                link    ix,LocalSize    ; Platz fÅr Exponenten/Kopie der Zahl

                push    af              ; Register retten
                push    de
                push    iy
                push    hl

                ld      iy,(ix+Buffer)  ; Pufferadresse holen

                ld      hl,(ix+Op)      ; Zahl kopieren
                ld      (ix+Copy),hl
                ld      hl,(ix+Op+2)
                res     7,h             ; dabei Vorzeichen lîschen
                ld      (ix+Copy+2),hl

                ld      a,'+'           ; Annahme positiv
                sll     (ix+Op)         ; Vorzeichen herausschieben
                rl      (ix+Op+1)       ; und in Carry bringen
                rl      (ix+Op+2)
                rl      (ix+Op+3)
                jr      c,IsNeg         ; Minuszeichen immer erforderlich
                bit     0,(ix+Format+1) ; Pluszeichen dagegen optional
                jr      nz,NoMantSgn
                jr      WrMantSgn
IsNeg:          ld      a,'-'           ; negative Zahl
WrMantSgn:      ld      (iy),a          ; Vorzeichen ablegen
                inc     iy
NoMantSgn:
                ld      l,(ix+Op+3)     ; Exponent herausholen...
                ld      h,0             ; ...auf 16 Bit erweitern...
                ld      bc,(ix+Op+1)    ; ...und in Quelle lîschen
                ld      (ix+Op+2),bc
                ld      b,(ix+Op)
                ld      c,0
                ld      (ix+Op),bc

                cp      hl,MaxExpo      ; Sonderwerte ?
                jp      z,SpecialVals   ; ja-->

                or      hl,hl           ; Zahl denormal ?
                jr      nz,IsNormal     ; nein, normal weiter
                ld      a,(ix+Op+3)     ; falls Mantisse Null,
                or      a,(ix+Op+2)     ; nicht normalisieren
                or      a,(ix+Op+1)
                jr      z,IsNull
Normalize:      sll     (ix+Op+1)       ; ansonsten schieben, bis fÅhrende
                rl      (ix+Op+2)       ; Eins da
                rl      (ix+Op+3)
                jr      c,IsNormal
                dec     hl
                jr      Normalize
IsNormal:       sub     hl,Bias         ; Bias abziehen
IsNull:
                ld      b,h             ; Zweierexponenten in Float wandeln
                ld      c,h
                push    bc
                push    hl
                call    fitof
                push    bc              ; in Dezimalexponenten wandeln
                push    de
                cpsh    bc,Ld10
                call    fdiv
                bit     7,b             ; Zahl negativ ?
                jr      z,NoCorr
                push    bc              ; dann noch eins abziehen wegen
                push    de              ; unterer Gau·klammer
                cpsh    bc,One
                call    fsub
NoCorr:         push    bc              ; den Ausflug in Float beenden
                push    de
                call    fftoi
                ld      (ix+ExpSave),de ; Exponenten retten

                ld      bc,(ix+Copy+2)  ; Originalzahl
                push    bc
                ld      bc,(ix+Copy)
                push    bc
                ld      hl,de           ; durch die Zehnerpotenz
                call    fPot10          ; des Exponenten
                push    bc
                push    de
                call    fdiv            ; teilen
Again:          ld      (ix+Copy),de    ; Ergebnis zwischen 1...9,999 retten
                ld      (ix+Copy+2),bc
                push    bc              ; Vorkommastelle berechnen
                push    de
                call    fftoi
                cp      e,10            ; doch etwas drÅber ?
                jr      ult,NoRoundErr
                ld      bc,(ix+Copy+2)  ; dann nocheinmal zehnteln
                push    bc
                ld      bc,(ix+Copy)
                push    bc
                cpsh    bc,Tenth
                call    fmul
                incw    (ix+ExpSave)
                jr      Again
NoRoundErr:     add     e,'0'           ; Vorkommastelle nach ASCII
                ld      (iy),e          ; ablegen
                inc     iy
                sub     e,'0'           ; wieder rÅckgÑngig machen
                cp      (ix+Format),0   ; gar keine Nachkommastellen ?
                jr      eq,NoComma
                ld      (iy),'.'        ; Dezimalpunkt ausgeben
                inc     iy
                push    bc              ; Vorkomma nach Float wandeln
                push    de
                call    fitof
                push    bc
                push    de
                cpsh    bc,ix+Copy      ; von alter Zahl abziehen
                call    fsub
                xor     b,80h           ; war verkehrtherum
                push    bc              ; zum Skalieren auf Stack
                push    de
                ld      l,(ix+Format)   ; passende Skalierungskonstante ausrechnen
                ld      h,0
                call    fPot10
                push    bc
                push    de
                call    fmul            ; hochskalieren
                push    bc              ; Rundung
                push    de
                cpsh    bc,Half
                call    fadd
                push    bc              ; Stellen nach Integer
                push    de
                call    fftoi
                push    bc              ; entspr. ausgeben
                push    de
                ld      b,(ix+Format)   ; Format fÅr fOutDec aufbauen
                set     7,b             ; kein Pluszeichen
                ld      c,'0'           ; FÅllzeichen Nullen
                push    bc
                call    fOutDec
                bit     5,(ix+Format+1) ; Nullen am Ende abrÑumen ?
                jr      nz,CleanZeros
NoComma:
                ld      a,(ix+Format+1) ; falls Minimalstellenzahl Exponent=0
                and     a,00011100b     ; und Exponent=0, vergessen
                or      a,(ix+ExpSave)
                or      a,(ix+ExpSave+1)
                jr      z,End

                ld      (iy),'E'        ; Exponenten ausgeben
                inc     iy
                ld      hl,(ix+ExpSave)
                ld      b,h
                ld      c,h
                push    bc
                push    hl
                ld      c,'0'           ; evtl. vornullen
                ld      b,(ix+Format+1)
                rrc     b               ; Bit 1-->Bit 7
                rrc     b
                and     b,87h
                push    bc
                call    fOutDec

End:            ld      (iy),0          ; NUL-Zeichen als Terminierer
                ld      de,iy           ; Endezeiger nach DE
                pop     hl              ; Register zurÅck
                pop     iy
                ex      de,hl           ; zur Subtraktion tauschen
                sub     hl,de           ; = Zahl geschriebener Zeichen
                ex      de,hl           ; HL wieder original
                ld      bc,de           ; Ergebnis nach BC
                pop     de
                pop     af

                unlk    ix              ; Stackrahmen abbauen
                retd    8               ; Ende

SpecialVals:    ld      a,(ix+Op+3)     ; Mantisse Null ?
                or      a,(ix+Op+2)
                or      a,(ix+Op+1)
                jr      nz,IsNAN
                ld      (iy),'I'        ; ja: Unendlichkeit
                ld      (iy+1),'N'
                ld      (iy+2),'F'
                add     iy,3
                jr      End
IsNAN:          ld      (iy),'N'        ; nein: NAN
                ld      (iy+1),'A'
                ld      (iy+2),'N'
                add     iy,3
                jr      End

CleanZeros:     cp      (iy-1),'0'      ; Null am Ende ?
                jr      nz,CleanNoZero  ; nein, Ende
                dec     iy              ; ja: ZÑhler runter, so da· Åber-
                jr      CleanZeros      ; schrieben wird und neuer Versuch
CleanNoZero:    cp      (iy-1),'.'      ; evtl. Komma entfernbar ?
                jr      nz,Ready        ; nein-->
                dec     iy              ; ja: noch ein Zeichen weniger
Ready:          jrl     NoComma

                endp

;------------------------------------------------------------------------------
; Wandlung ASCII-->Float

                proc    fatof

SrcAddr         equ     4               ; Lage Parameter auf Stack
                DefLocal Flags,2        ; Steuerflags
                DefLocal Exp,2          ; Speicher Exponent
                DefLocal Mant,4         ; Speicher fÅr Mantissenzwischenwert
                DefLocal Factor,4       ; Speicher fÅr Zehnerpotenz

                link    ix,LocalSize    ; Stackrahmen aufbauen

                push    af              ; Register retten
                push    hl
                push    iy

                ld      iy,(ix+SrcAddr) ; Zeigeradresse laden
                ld      (ix+Flags),01h  ; Phase 1 (Mantisse), noch kein Vorzeichen
                ld      (ix+Flags+1),0
                ld      bc,(Ten)        ; in der Mantisse mit 10 hochmultiplizieren
                ld      (ix+Factor),bc
                ld      bc,(Ten+2)
                ld      (ix+Factor+2),bc
                ld      bc,0            ; Exponent mit 0 vorbelegen
                ld      (ix+Exp),bc
                ld      (ix+Mant),bc    ; Mantisse auch
                ld      (ix+Mant+2),bc

ReadLoop:       ld      a,(iy)          ; ein neues Zeichen holen
                inc     iy

                cp      a,0             ; Endezeichen ?
                jp      eq,Combine      ; ja, zusammenbauen

                cp      a,' '           ; Leerzeichen ignorieren
                jr      eq,ReadLoop

                cp      a,'+'           ; Pluszeichen gnadenhalber zulassen
                jr      ne,NoPlus       ; ist aber nur ein Dummy
                bit     0,(ix+Flags+1)  ; schon ein Vorzeichen dagewesen ?
                jp      nz,Error        ; dann Fehler
                set     0,(ix+Flags+1)  ; ansonsten einfach setzen
                jr      ReadLoop
NoPlus:
                cp      a,'-'           ; Minuszeichen bewirkt schon eher etwas
                jr      ne,NoMinus
                bit     0,(ix+Flags+1)  ; darf auch nur einmal auftreten
                jp      nz,Error
                set     0,(ix+Flags+1)
                cp      (ix+Flags),1    ; je nach Phase anderes Flag setzen
                jr      ne,MinPhase3
                set     1,(ix+Flags+1)  ; bei Mantisse Bit 1...
                jr      ReadLoop
MinPhase3:      set     2,(ix+Flags+1)  ; ...bei Exponent Bit 2
                jr      ReadLoop
NoMinus:
                cp      a,'.'           ; Umschaltung Phase 2 (Nachkomma) ?
                jr      ne,NoPoint
                cp      (ix+Flags),1    ; bish. Phase mu· Eins sein
                jp      ne,Error
                ld      (ix+Flags),2    ; neue Phase eintragen
                set     0,(ix+Flags+1)  ; Nachkomma darf kein Vorzeichen haben
                ld      bc,(Tenth)      ; im Nachkomma durch 10 teilen
                ld      (ix+Factor),bc
                ld      bc,(Tenth+2)
                ld      (ix+Factor+2),bc
                jr      ReadLoop
NoPoint:
                cp      a,'e'           ; kleines & gro·es E zulassen
                jr      eq,IsE
                cp      a,'E'
                jr      ne,NoE
IsE:            cp      (ix+Flags),3    ; vorh. Phase mu· 1 oder 2 sein
                jp      eq,Error
                ld      (ix+Flags),3    ; vermerken
                res     0,(ix+Flags+1)  ; Vorzeichen wieder zulassen
                jr      ReadLoop
NoE:
                sub     a,'0'           ; jetzt nur noch 0..9 zugelassen
                jp      c,Error
                cp      a,9
                jp      ugt,Error
                set     0,(ix+Flags+1)  ; nach Ziffern keine Vorzeichen mehr zulassen

                cp      (ix+Flags),1    ; Phase 1 (Mantisse) :
                jr      ne,NoPhase1
                cpsh    bc,ix+Mant      ; bish. Mantisse * 10
                cpsh    bc,ix+Factor
                call    fmul
                push    bc              ; Ziffer dazuaddieren
                push    de
                ld      e,a
                ld      d,0
                ld      bc,0
                push    bc
                push    de
                call    fitof
                push    bc
                push    de
                call    fadd
                ld      (ix+Mant),de    ; Mantisse zurÅcklegen
                ld      (ix+Mant+2),bc
                jrl     ReadLoop
NoPhase1:
                cp      (ix+Flags),2    ; Phase 2 (Nachkomma) :
                jr      nz,NoPhase2
                ld      e,a             ; Stelle nach Float
                ld      d,0
                ld      bc,0
                push    bc
                push    de
                call    fitof
                push    bc              ; mit Zehnerpotenz skalieren
                push    de
                cpsh    bc,ix+Factor
                call    fmul
                push    bc              ; zur Mantisse addieren
                push    de
                cpsh    bc,ix+Mant
                call    fadd
                ld      (ix+Mant),de    ; Mantisse zurÅcklegen
                ld      (ix+Mant+2),bc
                cpsh    bc,ix+Factor    ; Faktor * 1/10
                cpsh    bc,Tenth
                call    fmul
                ld      (ix+Factor),de
                ld      (ix+Factor+2),bc
                jrl     ReadLoop
NoPhase2:
                ld      hl,(ix+Exp)
                mul     hl,10           ; Exponent heraufmultiplizieren
                add     a,l
                ld      l,a
                ld      a,0
                adc     h,0
                cp      hl,45           ; Minimum ist 1E-45
                jr      ugt,Error
                ld      (ix+Exp),hl
                jrl     ReadLoop

Combine:        ld      hl,(ix+Exp)
                bit     2,(ix+Flags+1)  ; Exponent negativ ?
                jr      z,ExpPos
                xor     hl,-1
                inc     hl
ExpPos:         call    fPot10          ; Zehnerpotenz des Exponenten bilden
                push    bc
                push    de
                cpsh    bc,ix+Mant      ; mit Mantisse kombinieren
                call    fmul
                bit     1,(ix+Flags+1)  ; Mantisse negativ ?
                jr      z,ManPos
                set     7,b
ManPos:         rcf                     ; Ende ohne Fehler

End:            pop     iy              ; Register zurÅck
                pop     hl
                pop     af

                unlk    ix              ; Rahmen abbauen
                retd    2               ; Ende

Error:          ld      hl,iy           ; rel. Zeichenposition ermitteln
                sub     hl,(ix+SrcAddr)
                ld      bc,hl
                scf                     ; Ende mit Fehler
                jr      End

                endp

;------------------------------------------------------------------------------
; gemeinsames Ende

                endsection

