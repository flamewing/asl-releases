; FLOAT.INC
;******************************************************************************
;* Gleitkommabibliothek fÅr TLCS 900                                          *
;*                                                                            *
;* Originale fÅr den 68000 aus mc, bis auf die Quadratwurzel aus c't          *
;* Portierung auf TLCS 900 von Alfred Arnold, Oktober 1993                    *
;*                                                                            *
;*  Routine  Funktion             Eingabe Ausgabe  Stack    LÑnge  Zeit/14MHz *
;*                                                                            *
;*  fadd     Addition             XWA+XHL    XWA  12 Byte 194 Byte    60 us   *
;*  fsub     Subtraktion          XWA-XHL    XWA  12 Byte   7 Byte    65 us   *
;*  fmul     Multiplikation       XWA*XHL    XWA  20 Byte 218 Byte    70 us   *
;*  fdiv     Division             XWA/XHL    XWA  20 Byte 147 Byte   300 us   *
;*  fmul2    Mult. mit 2er-Potenz XWA*(2^BC) XWA   6 Byte  99 Byte    20 us   *
;*  fitof    Int-->Float          XWA        XWA   4 Byte  41 Byte    90 us   *
;*  fftoi    Float-->Int          XWA        XWA   2 Byte  72 Byte    20 us   *
;*  fsqrt    Quadratwurzel        XWA        XWA  16 Byte 192 Byte   220 us   *
;*  fftoa    Float-->ASCII        XWA   (XHL),BC ~38 Byte 228 Byte ~4500 us   *
;*  fatof    ASCII-->Float     (XHL),BC XWA,[BC] ~40 Byte 260 Byte ~2300 us   *
;*                                                                            *
;*  - Wollen Sie einzelne Routinen entfernen, so beachten Sie, da· fsub Teile *
;*    aus fadd und fdiv Teile aus fmul verwendet !                            *
;*  - Gleitkommaformat ist IEEE Single (32 Bit)                               *
;*  - Integerwerte bei fmul2, fitof und fftoi sind vorzeichenbehaftet         *
;*  - Der Prozessor mu· sich im Maximum-Modus befinden                        *
;*  - Da die Routinen lokale Labels verwenden, ist mindestens AS 1.39 erfor-  *
;*    derlich                                                                 *
;*  - Die AusfÅhrungszeiten kînnen je nach Operand streuen, insbesondere bei  *
;*    den Konvertierungsfunktionen                                            *
;*  - MACROS.INC mu· vorher eingebunden werden                                *
;******************************************************************************

;------------------------------------------------------------------------------
; gemeinsamer Anfang, Makros

shifta          macro   op,dest         ; Schieben, falls evtl. A>=16
                push    a               ; A wird zerschossen
                bit     4,a             ; Akku im Bereich 16..31 ?
                jr      z,smaller
                op      16,dest         ; dann einmal gro· schieben
smaller:        push    f               ; Carry erhalten
                and     a,15            ; obere Bits plÑtten
                jr      z,fertig
                pop     f               ; evtl. Rest verarbeiten
                op      a,dest
                jr      ende            ; Carry schon gut
fertig:         pop     f
ende:           pop     a               ; A zurÅck
                endm

                section FloatLib

;------------------------------------------------------------------------------
; Konstanten

Ld10:           dd      ld(10)          ; Konversionskonstanten
One:            dd      1.0
Half:           dd      0.5
Ten:            dd      10.0
Tenth:          dd      3dcccccdh       ; =0.1, aber die Rundung auf manchen
					; Systemen variiert (damit Test nicht
					; scheitert)

Bias            equ     127
MaxExpo         equ     255
Comma           equ     '.'

;------------------------------------------------------------------------------
; Addition: XWA=XWA+XHL

                proc    fadd

                push    xbc             ; andere Register retten
                push    xde
                push    xhl

                ld      xde,xwa         ; Operand 1 nach XDE verschieben
                rlc     1,xde           ; Vorzeichen der Operanden nach Bit 0
                rlc     1,xhl
                ld      xbc,xde         ; Differenz bilden
                sub     xbc,xhl
                jr      nc,NoSwap       ; evtl. vertauschen, so da·
                ld      xwa,xhl         ; grî·ere in XDE
                ld      xhl,xde
                ld      xde,xwa
NoSwap:         ld      qa,e            ; Vorzeichen 1 ausmaskieren
                and     qa,1            ; (=Ergebnis Vorzeichen)
                bit     0,c             ; gleiches Vorzeichen ?
                jr      z,NoSub
                set     1,qa            ; dann Subtraktion vormerken

NoSub:          sub     xbc,xbc         ; XBC initialisieren
                rlc     8,xde           ; Exponent 1 rausholen
                ld      c,e
                or      e,e             ; falls <>0, implizite 1 einbauen
                scc     nz,e
                rrc     1,xde
                ld      e,0             ; Bit 0..7 wieder plÑtten

                rlc     8,xhl           ; dito Exponent 2 extrahieren
                ld      qc,l
                or      l,l
                scc     nz,l
                rrc     1,xhl
                ld      l,0

; Zwischenstand:
;  - Mantissen linksbÅndig inkl. impliziter Eins in XDE und XHL
;  - Exponent 1 in BC, Exponent 2 in QBC
;  - Ergebnisvorzeichen in QA, Bit 0
;  - Subtraktionsflag in QA, Bit 1

                ld      wa,bc           ; Exponentendifferenz berechnen
                sub     wa,qbc
                cp      wa,24           ; >24, d.h. Zahl 2 vernachlÑssigbar gegen Zahl 1
                jr      gt,Round        ; ja, Ergebnis ist grî·ere Zahl
                shifta  srl,xhl         ; ansonsten Mantisse 2 entspr. anpassen

Add:            bit     1,qa            ; subtrahieren ?
                jr      nz,Subtract     ; ja-->
                add     xde,xhl         ; nein, Mantissen addieren
                jr      nc,Round        ; kein öberlauf, runden
                rr      1,xde           ; ansonsten öberlauf einschieben...
                inc     bc              ; ...und Exponent korrigieren
                jr      Round           ; normal weiter runden

Subtract:       sub     xde,xhl         ; Mantissen subtrahieren
                jr      z,Zero          ; falls Null, Gesamtergebnis 0
                jr      m,Round         ; fÅhrende 1 noch da: zum Runden
Normalize:      or      bc,bc           ; Exponent bereits Null ?
                jr      z,Round         ; dann denormales Ergebnis
                dec     bc              ; ansonsten Mantisse eins rauf, Exponent
                sll     1,xde           ; eins runter
                jr      p,Normalize     ; solange, bis Eins auftaucht

Round:          add     xde,80h         ; Rundung auf Bit hinter Mantisse
                jr      nc,NoOver
                rr      1,xde           ; Bei öberlauf korrigieren
                inc     bc
NoOver:         ld      e,0             ; Mantissenrest plÑtten
                or      xde,xde         ; insgesamt 0 ?
                jr      z,Zero          ; dann Ergebnis 0
                cp      bc,MaxExpo      ; ExponentenÅberlauf ?
                jr      lt,NoEOver
                ld      bc,MaxExpo      ; ja: Unendlich: Exponent=Maximum
                sub     xde,xde         ;                Mantisse=0
                jr      Denormal

NoEOver:        or      bc,bc           ; Exponent 0 ?
                jr      z,Denormal      ; ja, denormal
                sll     1,xde           ; fÅhrende Eins nicht speichern
Denormal:       ld      e,c             ; Exponenten einbauen
                rrc     8,xde           ; nach oben schieben
                rr      1,qa            ; Vorzeichen einbauen
                rr      1,xde

Zero:           ld      xwa,xde         ; Ergebnis in Akku

                pop     xhl             ; Register zurÅck
                pop     xde
                pop     xbc

                ret

                endp

;------------------------------------------------------------------------------
; Subtraktion: XWA=XWA-XHL

                proc    fsub

                xor     qh,80h          ; Vorzeichen 2 drehen
                jp      fadd            ; ansonsten wie Addition

                endp

;------------------------------------------------------------------------------
; Multiplikation: XWA=XWA*XHL

                proc    fmul

		public  MulRound:Parent,MulZero:Parent,MulResult:Parent
		public	DivError:Parent

		push    xbc             ; Register retten
		push    xde
		push    xhl
		push    xix
		push    xiy

		ld      xiy,xwa         ; Op1 kopieren
                xor     xiy,xhl         ; Ergebnisvorzeichen bestimmen

                ex      wa,qwa          ; RegisterhÑlften Op1 vertauschen
                ld      xde,xwa         ; Op1 ab sofort in XDE
                and     de,7fh          ; Exponent und Vz. behandeln
                and     wa,7f80h        ; Exponent maskieren
                jr      z,Denorm1       ; gleich Null-->Op1 denormal
                set     7,de            ; ansonsten implizite Eins einbauen
                sub     wa,80h          ; Bias kompensieren
Denorm1:
                ex      hl,qhl          ; Op2 genauso behandeln
                ld      xbc,xhl
                and     hl,7fh
                and     bc,7f80h
                jr      z,Denorm2
                set     7,hl
                sub     bc,80h
Denorm2:
                add     bc,wa           ; Exponenten addieren
                srl     7,bc            ; richtig positionieren
                sub     bc,Bias-3       ; Bias-3 abziehen
                cp      bc,-24          ; totaler Unterlauf ?
		jr      lt,MulZero      ; dann Ergebnis 0

                ld      wa,de           ; beide oberen HÑlften multiplizieren
                mul     xwa,hl
                ex      wa,qwa          ; Ergebnis in oberer HÑlfte lagern
                ld      wa,de           ; obere HÑlfte Op1 retten
                ex      de,qde          ; untere HÑlfte Op1 holen
                ld      ix,hl           ; untere HÑlfte Op1 * obere Op2
                mul     xix,de
                ex      hl,qhl          ; untere Op1 * untere Op2
                mul     xde,hl
                ex      de,qde          ; obere Op1 * untere Op2
                mul     xhl,wa

                ld      wa,de           ; Teile aufaddieren
                add     xwa,xix
                add     xwa,xhl
		jr      z,MulResult     ; Mantisse Null, Ergebnis Null
                jr      m,MulRound

                or      bc,bc           ; Exponent negativ ?
                jr      m,Unterlauf     ; ggfs. Unterlauf behandeln

Nor:            or      bc,bc           ; Exponent Null ?
                jr      z,MulRound      ; ja-->zum Runden
                rl      1,xde           ; nein, Mantisse eins nachschieben
                rl      1,xwa
                dec     bc              ; und Exponent runter
                or      xwa,xwa         ; fÅhrende Eins da ?
                jr      p,Nor           ; nein, weiterschieben

MulRound:       add     xwa,80h         ; Rundung
                jr      nc,NoROver      ; dabei öberlauf ?
                rr      1,xwa           ; ja: Mantisse & Exponent korrigieren
                inc     bc
NoROver:        cp      bc,MaxExpo      ; ExponentenÅberlauf ?
                jr      lt,NoEOver
DivError:       ld      bc,MaxExpo      ; dann unendlich einstellen
                sub     xwa,xwa
                jr      Denormal

NoEOver:        or      bc,bc           ; Exponent 0 ?
                jr      z,Denormal
                sll     1,xwa           ; fÅhrende 1 lîschen

Denormal:       ld      a,c             ; Exponent einbauen
                rrc     8,xwa           ; hochschieben
                rl      1,xiy           ; Vorzeichen einbauen
                rr      1,xwa

MulResult:      pop     xiy
                pop     xix
                pop     xhl
                pop     xde
                pop     xbc

                ret

MulZero:        sub     xwa,xwa         ; Null erzeugen
		jr      MulResult

Unterlauf:      cp      bc,-24          ; totaler Unterlauf ?
		jr      le,MulZero      ; dann Null
		neg     bc              ; sonst umbauen
		ld      xde,xwa         ; dazu Akku freimachen
		sub     wa,wa           ; Endexponent
                ex      wa,bc           ; ist 0
		shifta  srl,xde         ; Mantisse herunterschieben
                ld      xwa,xde         ; Ergebnis zurÅck nach XWA
                jr      MulRound        ; zurÅck mit Exponent 0

                endp

;------------------------------------------------------------------------------
; Division: XWA=XWA/XHL

                proc    fdiv

                push    xbc             ; Register retten (mu· gleich zu fmul sein)
                push    xde
                push    xhl
                push    xix
                push    xiy

                ld      xiy,xwa         ; Op1 kopieren
                xor     xiy,xhl         ; Ergebnisvorzeichen bestimmen

                ex      wa,qwa          ; Vorbehandlung wie bei fmul
                ld      xde,xwa
                and     de,7fh
                and     wa,7f80h
                jr      z,Denorm1
                set     7,de
                sub     wa,80h
Denorm1:
                ex      hl,qhl
                ld      xbc,xhl
                and     hl,7fh
                and     bc,7f80h
                jr      z,Denorm2
                set     7,hl
                sub     bc,80h
Denorm2:
                sub     wa,bc           ; Exponentendifferenz bilden
                ld      bc,wa           ; mu· in BC liegen
                sra     7,bc            ; richtig positionieren
                add     bc,Bias         ; Bias addieren
                cp      bc,-24          ; totaler Unterlauf ?
                jr      lt,MulZero      ; ja, Ergebnis Null

                ex      hl,qhl          ; Format 0fff ... ffff 0000 0000
		or      xhl,xhl         ; Ergebnis unendlich ?
		jrl     z,DivError
                sll     7,xhl
                ex      de,qde          ; dito Divident
                or      xde,xde         ; falls Null, Ergebnis Null
                jrl     z,MulZero
                sll     7,xde

NormLoop:       bit     14,qhl          ; Divisor normalisiert ?
                jr      nz,Normal
                inc     bc              ; nein, Exponent RAUF (ist Ergebnisexponent)
                sll     1,xhl
                jr      NormLoop

Normal:         sub     xwa,xwa         ; Ergebnisquotient vorbesetzen
                add     bc,25           ; Exponent nicht grî·er als 0

Loop:           ld      xix,xde         ; Divident zwischenspeichern
                sub     xde,xhl         ; probeweise abziehen
                ccf                     ; Carry drehen
                jr      c,IsOne         ; ungedrehter Carry=1: Divisor pa·t
                ld      xde,xix         ; ansonsten zurÅckkopieren
IsOne:          rl      1,xwa           ; Ergebnisbit einschieben
                sll     1,xde           ; Divident verschieben
                dec     bc              ; Exponent runter
                or      bc,bc
                jr      z,Denorm        ; falls Null, denormalisieren
                bit     8,qwa           ; fÅhrende Eins da ?
                jr      z,Loop          ; nein, weiterrechnen

Denorm:         sll     7,xwa           ; Mantisse positionieren
                jrl     z,MulResult     ; Ergebnis 0 ?
                jrl     MulRound        ; ansonsten zum Runden

                endp

;-----------------------------------------------------------------------------
; Multiplikation mit Zweierpotenz: XWA=XWA*2^BC

                proc    fmul2

                push    bc              ; Register retten
                push    xde

                ld      xde,xwa         ; Vorzeichen merken
                sll     1,xwa           ; Vorzeichen rausschieben
                jr      z,Zero          ; falls Null, Ergebnis Null
                rlc     8,xwa           ; Exponent nach unten...
                sub     de,de           ; und in DE packen
                add     e,a
                jr      z,Denorm        ; falls denormalisiert..
                or      bc,bc           ; Multiplikation oder Division ?
                jr      m,Divide        ; (neg. Exponent=Division)

                add     de,bc           ; Exponent addieren
                cp      de,MaxExpo      ; öberlauf ?
                jr      ge,Over         ; ja, Ergebnis unendlich
Result:         ld      a,e             ; Ergebnisexponent einbauen
                rrc     8,xwa           ; Exponent nach oben
                rl      1,xde           ; Vorzeichen einschieben
                rr      1,xwa

Zero:           pop     xde             ; Register zurÅck
                pop     bc
                ret

Over:           ld      de,MaxExpo      ; Ergebnis unendlich
                sub     xwa,xwa
                jr      Result

Divide:         add     de,bc           ; Exponentensumme bilden
                jr      gt,Result       ; >0, keine Sonderbehandlung
                scf                     ; ansonsten 1 explizit fÅr
                rr      1,xwa           ; denormale Zahl machen
DDenorm:        or      de,de           ; Exponent=0 ?
                jr      z,Result        ; ja, Ergebnis einfach denormal
                srl     1,xwa           ; ansonsten weiter denormalisieren
                jr      z,Zero          ; dabei totaler Unterlauf->Null
                inc     de              ; Exponent korrigieren
                jr      DDenorm
DDDenorm:       add     de,bc           ; Exponentensumme bilden
                jr      DDenorm

Denorm:         or      bc,bc           ; Multiplikation oder Division ?
                jr      m,DDDenorm
                sub     a,a             ; alten Exponenten lîschen
Norm:           sll     1,xwa           ; normalisieren...
                jr      c,Stop          ; bis fÅhrende Eins da
                dec     bc              ; oder 2. Exponent 0
                or      bc,bc
                jr      nz,Norm
                jr      Result          ; Multiplikator kompl. fÅr Normalisierung draufgegangen
Stop:           add     de,bc           ; Rest addieren
                jr      Result          ; alles andere schon o.k.

                endp

;------------------------------------------------------------------------------
; LongInt-->Float : XWA-->XWA

                proc    fitof

                push    xbc             ; Register retten

                or      xwa,xwa         ; Null ?
                jr      z,Result        ; dann Ergebnis Null
                scc     m,qc            ; Vorzeichen nach QC, Bit 0
                jr      p,Positive
                cpl     wa              ; falls negativ,drehen
                cpl     qwa
                inc     xwa
Positive:       ld      bc,Bias+32      ; Exponent vorbesetzen
Shift:          dec     bc              ; Mantisse verschieben
                sll     1,xwa
                jr      nc,Shift
                ld      a,c             ; Exponent einbauen
                rrc     8,xwa           ; Exponent nach oben
                rr      1,qc            ; Vorzeichen einbauen
                rr      1,xwa

Result:         pop     xbc             ; Register zurÅck
                ret

                endp

;------------------------------------------------------------------------------
; Float-->LongInt : XWA-->XWA

                proc    fftoi

                push    bc              ; Register retten

                rl      1,xwa           ; Vorzeichen in Carry
                scc     c,b             ; in B merken

                rlc     8,xwa           ; Exponent nach unten
                ld      c,a             ; in C legen
                sub     c,Bias          ; Bias abziehen

                jr      m,Zero          ; neg. Exponent -> Zahl<0 -> Ergebnis 0
                cp      c,31            ; öberlauf ?
                jr      ge,Over

                scf                     ; fÅhrende Eins einschieben
                rr      1,xwa
                sub     a,a             ; Exponent lîschen

Shift:          srl     1,xwa           ; jetzt schieben, bis Ergebnis stimmt
                inc     c
                cp      c,31
                jr      ne,Shift

                srl     1,b             ; negieren ?
                jr      nc,Positive
                cpl     wa              ; ggfs. negieren
                cpl     qwa
                inc     xwa

Positive:       pop     bc              ; Register zurÅck
                ret

Zero:           sub     xwa,xwa         ; Ergebnis 0
                jr      Positive

Over:           ld      xwa,7fffffffh   ; öberlauf: Maxint zurÅckgeben
                srl     1,b             ; negativ ?
                jr      nc,Positive
                cpl     wa              ; ja, neg. Maximum zurÅckgeben
                cpl     qwa
                jr      Positive

                endp

;------------------------------------------------------------------------------
; Quadratwurzel: XWA=SQRT(XWA)

                proc    fsqrt

                push    xbc             ; Register retten
                push    xde
                push    xhl
                push    xix

                ld      xix,xwa         ; Argument retten
                or      xix,xix         ; Zahl negativ ?
                jrl     m,DomainError   ; dann geht es nicht

                ex      ix,qix          ; MSW holen
                and     xix,7f80h       ; Exponent isolieren
                jrl     z,Zero          ; keine Behandlung denormaler Zahlen

                and     xwa,7fffffh     ; Mantisse isolieren
                sub     ix,7fh*80h      ; Bias vom Exponenten entfernen
                bit     7,ix            ; Exponent ungerade ?
                res     7,ix
                jr      z,EvenExp
                add     xwa,xwa         ; ja: Mantisse verdoppeln
                add     xwa,1000000h-800000h ; impl. Eins dazu
EvenExp:                                ; erste Iteration ohne impl. Eins
                sra     1,ix            ; Exponent/2 mit Vorzeichen
                add     ix,7fh*80h      ; Bias wieder dazu
                ex      ix,qix          ; neuen Exponenten in QIX aufheben
                sll     7,xwa           ; x ausrichten
                ld      xde,40000000h   ; xroot nach erster Iteration
                ld      xhl,10000000h   ; m2=2 << (MaxBit-1)
Loop10:         ld      xbc,xwa         ; xx2 = x
Loop11:         sub     xbc,xde         ; xx2 -= xroot
                srl     1,xde           ; xroot = xroot/2
                sub     xbc,xhl         ; x2 -= m2
                jr      m,DontSet1
                ld      xwa,xbc         ; x = xx2
                or      xde,xhl         ; xroot += m2
                srl     2,xhl           ; m2 = m2/4
                jr      nz,Loop11
                jr      WABCSame
DontSet1:       srl     2,xhl           ; m2 = m2/4
                jr      nz,Loop10       ; 15* abarbeiten
                                        ; Bit 22..8
                ld      xbc,xwa         ; 17. Iteration separat
WABCSame:       sub     xbc,xde
                rrc     1,xde           ; mitsamt Carry...
                ex      de,qde          ; auf neues Alignment umstellen
                sub     xbc,1           ; Carry von 0-$4000: x2 -= m2
                jr      m,DontSet7
                or      xbc,-40000000h  ; 0-$4000: x2 -= m2, Teil 2
                ld      xwa,xbc
                or      de,4000h        ; xroot += m2
DontSet7:       ex      wa,qwa          ; x auf neues Alignment umstellen
                ld      hl,1000h        ; m2 - obere HÑlfte schon 0
Loop20:         ld      xbc,xwa         ; xx2 = x
Loop21:         sub     xbc,xde         ; xx2 -= xroot
                srl     1,xde           ; xroot = xroot/2
                sub     xbc,xhl         ; x2 -= m2
                jr      m,DontSet2
                ld      xwa,xbc         ; x = xx2
                or      xde,xhl         ; xroot += m2
                srl     2,xhl           ; m2 = m2/4
                jr      nz,Loop21
                jr      Finish
DontSet2:       srl     2,xhl           ; m2 = m2/4
                jr      nz,Loop20       ; 7* abarbeiten

Finish:         sub     xwa,xde         ; Aufrunden notwendig ?
                jr      ule,NoInc
                inc     xde             ; wenn ja, durchfÅhren
NoInc:          res     7,qde           ; impl. Eins lîschen
                or      xde,xix
                ld      xwa,xde         ; Ergebnis in XWA
                jr      End

DomainError:    ld      xwa,0ffc00000h  ; -NAN zurÅckgeben
                jr      End

Zero:           sub     xwa,xwa         ; Ergebnis 0

End:            pop     xix             ; Register zurÅck
                pop     xhl
                pop     xde
                pop     xbc
                ret

                endp

;------------------------------------------------------------------------------
; Unterroutine Zehnerpotenz bilden: XWA=10.0^BC

                section fPot10          ; nicht mit proc, da private Funktion
                public  fPot10:Parent
fPot10:

                push    xbc             ; Register retten
                push    xhl

                ld      xwa,(One)       ; Ausgangspunkt fÅrs Multiplizieren
                ld      xhl,(Ten)       ; zu benutzende Potenz
                or      bc,bc           ; negative Potenz ?
                jr      p,IsPos
                ld      xhl,(Tenth)     ; dann eben Zehntel multiplizieren
                neg     bc              ; fÅr Schleife immer positiv
IsPos:
                or      bc,bc           ; Noch weiter multiplizieren ?
                jr      z,End
                bit     0,bc            ; Restpotenz ungerade ?
                jr      z,IsEven
                call    fmul            ; ja: einzeln multiplizieren
IsEven:         srl     1,bc            ; nÑchste Stelle
                push    xwa             ; neue Potenz berechnen
                ld      xwa,xhl
                call    fmul            ; durch quadrieren
                ld      xhl,xwa
                pop     xwa
                jr      IsPos           ; weiter nach Einsen suchen

End:            pop     xhl             ; Register zurÅck
                pop     xbc
                ret

                endsection

;------------------------------------------------------------------------------
; Unterroutine Zahl dezimal wandeln

                section fOutDec
                public  fOutDec:Parent
fOutDec:

                push    xwa             ; Register retten
                push    xbc
                push    de
                push    xhl

                bit     15,qwa          ; negativ ?
                jr      z,IsPos
                ld      (xix+),'-'      ; ja: vermerken...
                cpl     wa              ; ...und Zweierkomplement
                cpl     qwa
                inc     xwa
                jr      GoOn
IsPos:          bit     7,c             ; Pluszeichen ausgeben ?
                jr      nz,GoOn
                ld      (xix+),'+'
GoOn:           res     7,c             ; Plusflag lîschen
                ld      qbc,0           ; Nullflag und ZÑhler lîschen

InLoop:         ld      xhl,0           ; Division vorbereiten
                ld      e,32            ; 32 Bit-Division
DivLoop:        sll     1,xwa           ; eins weiterschieben
                rl      1,xhl
                srl     1,xwa           ; fÅr nachher
                sub     xhl,10          ; pa·t Divisor hinein ?
                jr      nc,DivOK
                add     xhl,10          ; nein, zurÅcknehmen...
                scf                     ; im Ergebnis 0 einschieben
DivOK:          ccf                     ; neues Ergebnisbit
                rl      1,xwa           ; Ergebnis in XWA einschieben...
                djnz    e,DivLoop

                add     l,'0'           ; ASCII-Offset addieren
                bit     1,qb            ; schon im Nullbereich ?
                jr      z,NormVal
                ld      l,b             ; ja, dann gewÅnschtes Leerzeichen
NormVal:        push    l               ; auf LIFO legen
                inc     qc              ; ein Zeichen mehr
                or      xwa,xwa         ; Quotient Null ?
                scc     z,qb
                jr      nz,InLoop       ; wenn nicht Null, auf jeden Fall weiter
                cp      c,qc            ; ansonsten nur, falls min. Stellenzahl
                jr      ugt,InLoop      ; noch nicht erreicht

OutLoop:        pop     a               ; jetzt Zeichen umgekehrt ablegen
                ld      (xix+),a
                djnz    qc,OutLoop

                pop     xhl             ; Register zurÅck
                pop     de
                pop     xbc
                pop     xwa

                ret

                endsection

;------------------------------------------------------------------------------
; Gleitkomma nach ASCII wandeln:
; In:  Zahl in XWA
;      Zeiger auf Speicher in XHL
;      max. Anzahl Nachkommastellen in C
;      B/Bit 0 setzen, falls Mantissen-Pluszeichen unerwÅnscht
;      B/Bit 1 setzen, falls Exponenten-Pluszeichen unerwÅnscht
;      B/Bit 2..4 = Stellenzahl Exponent
;      B/Bit 5 setzen, falls Nullen am Ende der Mantisse unerwÅnscht
; Out: Zahl abgelegter Zeichen (exkl. NUL am Ende) in BC
;      (XHL) = gebildeter String

                proc    fftoa

                push    xix             ; Register retten
                push    xhl
                push    de
                push    xbc
                push    xwa

                ld      xix,xhl         ; Zeiger auf Speicher kopieren
                ld      de,bc           ; Parameter sichern

                ld      xhl,xwa         ; Zahl auf die Zerlegebank bringen
                res     15,qwa          ; Vorzeichen hier nicht mehr gebraucht

                ld      c,'+'           ; Annahme positiv
                sll     1,xhl           ; Vorzeichen in Carry bringen
                jr      c,IsNeg         ; Minuszeichen immer erforderlich...
                bit     0,d             ; ...Pluszeichen optional
                jr      nz,NoMantSgn
                jr      WrMantSgn
IsNeg:          ld      c,'-'           ; ja
WrMantSgn:      ld      (xix+),c        ; Mantissenvorzeichen ablegen
NoMantSgn:
                ld      c,qh            ; Exponenten herausholen...
                extz    bc              ; ...auf 16 Bit erweitern...
                sll     8,xhl           ; ...und in Quelle lîschen

                cp      bc,MaxExpo      ; Sonderwert (INF/NAN) ?
                jrl     z,SpecialVals   ; ja-->

                or      bc,bc           ; Zahl denormal ?
                jr      nz,IsNormal     ; nein, normal weiter
                or      xhl,xhl         ; bei kompl. Null auch Åberspringen
                jr      z,IsNull
Normalize:      sll     1,xhl           ; ja: solange zÑhlen, bis 1 erscheint
                jr      c,IsNormal
                dec     bc
                jr      Normalize
IsNormal:       sub     bc,Bias         ; Bias abziehen
IsNull:
                push    xwa             ; fÅr folgendes Zahl retten
                ld      wa,bc           ; Zweierexponenten in Float wandeln
                exts    xwa
                call    fitof
                ld      xhl,(Ld10)      ; in Dezimalexponenten wandeln
                call    fdiv
                or      xwa,xwa         ; Zahl negativ ?
                jr      p,NoCorr
                ld      xhl,(One)       ; dann nocheinmal korrigieren wg.
                call    fsub            ; unterer Gau·klammer
NoCorr:         call    fftoi           ; Den Ausflug in Float beenden
                ld      qbc,wa          ; den Zehnerexponenten retten
                ld      bc,wa
                call    fPot10          ; von diesem Exponenten Zehnerpotenz
                ld      xhl,xwa         ; bilden
                pop     xwa             ; alte Zahl zurÅck
                call    fdiv            ; Teilen: Ergebnis ist Zahl zwischen
Again:          ld      xhl,xwa         ; 1.0 und 9.9999..., diese retten
                call    fftoi           ; Vorkommastelle berechnen
                cp      a,10            ; doch etwas drÅber ?
                jr      ult,NoRoundErr
                ld      xwa,xhl         ; ja, dann noch einmal zehnteln
                ld      xhl,(Tenth)
                call    fmul
                inc     qbc
                jr      Again
NoRoundErr:     add     a,'0'           ; diese nach ASCII wandeln...
                ld      (xix+),a        ; ...und ablegen
                sub     a,'0'           ; wieder rÅckgÑngig machen
                cp      e,0             ; gar keine Nachkommastellen ?
                jr      eq,NoComma
                ld      (xix+),Comma    ; Dezimalpunkt ausgeben
                call    fitof           ; in ganze Gleitkommazahl wandeln
                call    fsub            ; Differenz bilden
                chg     15,qwa          ; war verkehrtherum...
                ld      xhl,xwa         ; nach XHL verschieben, weil XWA gebraucht
                ld      c,e             ; Zehnerpotenz fÅr Skalierung ausrechnen
                extz    bc              ; auf 16 Bit aufblasen
                call    fPot10          ; Skalierungswert berechnen
                call    fmul            ; hochmultiplizieren
                ld      xhl,(Half)      ; Rundung
                call    fadd
                call    fftoi           ; diese herausziehen
                ld      b,'0'           ; n-stellig mit Vornullen ausgeben
                ld      c,e
                set     7,c             ; kein Pluszeichen!
                call    fOutDec
                bit     5,d             ; Nullen am Ende abrÑumen ?
                jr      nz,CleanZeros
NoComma:
                ld      a,d             ; falls Minimalstellenzahl Exponent=0
                and     a,00011100b     ; und Exponent=0, vergessen
                or      a,qb
                or      a,qc
                jr      z,End

                ld      (xix+),'E'      ; Exponenten ausgeben
                ld      wa,qbc
                exts    xwa
                ld      b,'0'           ; evtl. vornullen...
                ld      c,d             ; Bit 1-->Bit 7
                rrc     2,c
                and     c,87h           ; Bits ausmaskieren
                call    fOutDec

End:            pop     xwa             ; Register zurÅck
                pop     xbc
                pop     de
                pop     xhl
                ld      (xix),0         ; NUL-Zeichen im String nicht vergessen
                sub     xix,xhl         ; StringlÑnge berechnen
                ld      bc,ix
                pop     xix

                ret

SpecialVals:    or      xde,xde         ; Ist die Mantisse Null ?
                jr      nz,IsNAN
                ldw     (xix+),'NI'     ; ja: INF einschreiben
                ld      (xix+),'F'
                jr      End
IsNAN:          ldw     (xix+),'AN'     ; nein: NAN einschreiben
                ld      (xix+),'N'
                jr      End

CleanZeros:     cp      (xix-1),'0'     ; steht da eine Null am Ende ?
                jr      nz,CleanNoZero  ; nein, Ende
                dec     xix             ; ja: ZÑhler runter, so da· Åber-
                jr      CleanZeros      ; schrieben wird und neuer Versuch
CleanNoZero:    cp      (xix-1),Comma   ; evtl. Komma entfernbar ?
                jr      nz,NoComma      ; nein-->
                dec     xix             ; ja: noch ein Zeichen weniger
                jr      NoComma

                endp

;------------------------------------------------------------------------------
; ASCII nach Gleitkomma wandeln:
; In:  Zeiger auf String (ASCIIZ) in XHL
; Out: XWA = Ergebnis bzw. fehlerhafte Stelle
;      CY = 0, falls fehlerfrei

                proc    fatof

                push    xbc             ; Register retten
                push    xde
                push    xhl
                push    xix

                ld      xix,xhl         ; Zeiger nach XIX
                ld      qbc,01          ; Phase 1 (Mantisse), noch kein Vorzeichen
                ld      xde,(Ten)       ; in der Mantisse mit 10 hochmultiplizieren
                ld      xhl,0           ; Mantisse vorbelegen
                ld      bc,0            ; Exponent vorbelegen

ReadLoop:       ld      a,(xix+)        ; ein neues Zeichen holen
                extz    wa              ; auf 32 Bit aufblasen
                extz    xwa

                cp      a,0             ; Endezeichen ?
                jrl     eq,Combine      ; ja, alles zusammen

                cp      a,' '           ; Leerzeichen ignorieren
                jr      eq,ReadLoop

                cp      a,'+'           ; Pluszeichen gnadenhalber zugelassen
                jr      ne,NoPlus       ; ist aber nur ein Dummy
                bit     0,qb            ; schon ein Vorzeichen dagewesen ?
                jrl     nz,Error        ; dann Fehler
                set     0,qb            ; ansonsten einfach setzen
                jr      ReadLoop
NoPlus:
                cp      a,'-'           ; Minuszeichen bewirkt schon eher etwas
                jr      ne,NoMinus
                bit     0,qb            ; darf auch nur einmal auftreten
                jrl     nz,Error
                set     0,qb
                cp      qc,1            ; je nach Phase anderes Flag setzen
                jr      ne,MinPhase3
                set     1,qb            ; bei Mantisse Bit 1...
                jr      ReadLoop
MinPhase3:      set     2,qb            ; bei Exponent Bit 2
                jr      ReadLoop
NoMinus:
                cp      a,'.'           ; Umschaltung zu Phase 2 (Nachkomma) ?
                jr      ne,NoPoint
                cp      qc,1            ; bish. Phase mu· eins sein
                jrl     ne,Error
                ld      qc,2            ; neue Phase eintragen
                set     0,qb            ; Nachkomma darf kein Vorzeichen haben
                ld      xde,(Tenth)     ; im Nachkomma durch 10 teilen
                jr      ReadLoop
NoPoint:
                cp      a,'e'           ; kleines und gro·es E zulassen
                jr      eq,IsE
                cp      a,'E'
                jr      ne,NoE
IsE:            cp      qc,3            ; vorherige Phase mu· 1 oder 2 sein
                jr      eq,Error
                ld      qc,3            ; vermerken
                res     0,qb            ; Vorzeichen wieder zugelassen
                jr      ReadLoop
NoE:
                sub     a,'0'           ; jetzt nur noch 0..9 zugelassen
                jr      c,Error
                cp      a,9
                jr      ugt,Error
                set     0,qb            ; nach Ziffern kein Vorzeichen mehr zulassen

                cp      qc,1            ; Phase 1 (Mantisse) :
                jr      ne,NoPhase1
                push    xwa             ; Zeichen retten
                ld      xwa,xde         ; bish. Mantisse * 10
                call    fmul
                ld      xhl,xwa
                pop     xwa             ; Zahl nach Float wandeln
                call    fitof
                call    fadd            ; dazuaddieren
                ld      xhl,xwa         ; Mantisse zurÅcklegen
                jrl     ReadLoop
NoPhase1:
                cp      qc,2            ; Phase 2 (Nachkomma) :
                jr      ne,NoPhase2
                call    fitof           ; Stelle nach Float wandeln
                push    xhl             ; Mantisse retten
                ld      xhl,xde         ; Stelle mit Zehnerpotenz skalieren
                call    fmul
                pop     xhl             ; zur Mantisse addieren
                call    fadd
                push    xwa             ; Zwischenergebnis retten
                ld      xwa,xde         ; nÑchste Skalierungspotenz ausrechnen
                ld      xhl,(Tenth)
                call    fmul
                ld      xde,xwa         ; alles wieder zurÅck
                pop     xhl
                jrl     ReadLoop
NoPhase2:
                mul     bc,10           ; Exponent heraufmultiplizieren
                add     bc,wa
                cp      bc,45           ; Minimum ist 1e-45
                jr      ugt,Error
                jrl     ReadLoop

Combine:        bit     2,qb            ; Exponent negativ ?
                jr      z,ExpPos
                neg     bc
ExpPos:         call    fPot10          ; Zehnerpotenz des Exponenten bilden
                call    fmul            ; mit Mantisse kombinieren
                bit     1,qb            ; Mantisse negativ ?
                jr      z,ManPos
                set     15,qwa
ManPos:         rcf                     ; Ende ohne Fehler

                pop     xix             ; Register zurÅck
                pop     xhl
                pop     xde
                pop     xbc
                ret

Error:          ld      xwa,xix         ; Endzeiger laden
                pop     xix
                pop     xhl
                sub     xwa,xhl         ; rel. Position des fehlerhaften Zeichens berechnen
                pop     xde
                pop     xbc
                scf                     ; Ende mit Fehler
                ret

                endp

;------------------------------------------------------------------------------
; gemeinsames Ende

                endsection

