; CONOUT.INC
;******************************************************************************
;* Konsolenausgabe fÅr Micro-ICE TLCS900                                      *
;*                                                                            *
;* Routinen CONOUT, PSTR, A_ASC von Oliver Sellke                             *
;* Routine WriteDec von Alfred Arnold, Oktober 1993                           *
;* 20.11.1993 StrTerm C-konform auf 00h geÑndert                              *
;*            TXTAUS hinzugefÅgt                                              *
;*                                                                            *
;*  Routine     Funktion         Eingabe                    Stack             *
;*                                                                            *
;*  CONOUT   Zeichen ausgeben      A                        1..3 Byte         *
;*  TXTAUS   String ausgeben       (XHL)                                      *
;*  PSTR     String ausgeben    hinter CALL+StrTerm als Ende 6 Byte           *
;*  A_ASC    Byte hex ausgeben     A                         2 Byte           *
;*  WriteDec Wort dez ausgeben     WA                        6 Byte           *
;*                                                                            *
;*  - Da die Routinen lokale Labels verwenden, ist mindestens AS 1.39 erfor-  *
;*    derlich                                                                 *
;*  - Der Prozessor mu· sich im Maximum-Modus befinden                        *
;*  - Symbol SIODelay setzen, falls langsames Terminal vorhanden              *
;*  - MACROS.INC mu· vorher eingebunden werden                                *
;******************************************************************************

;------------------------------------------------------------------------------
; gemeinsamer Anfang, Definitionen

                section ConOut

                public  StrTerm
StrTerm         equ     0

SC0BUF          EQU     050H
INTES0          EQU     077H

;******************************************************************************
; (7 bit ASCII) Ausgabe Åber SIO-Kanal 0

                proc    CONOUT
                PUSH    A               ;sichern
                RES     7,A             ;MSB ausblenden (nur ASCII!)
CONOUT1:        BIT     7,(INTES0)      ;ist der Buffer frei?
                JR      Z,CONOUT1       ;Wenn nicht, dann weiter prÅfen
                LD      (INTES0),0Dh    ;Flag rÅcksetzen
                LD      (SC0BUF),A      ;Zeichen in Transmit-Buffer
                ifdef   SIODelay
                 PUSH    HL             ;sichern
                 LD      HL,80H         ;Verzîgerungszeit laden
WAIT:            DEC     1,HL           ;ZÑhler - 1
                 CP      HL,0           ;Ist ZÑhler = 0?
                 JR      NZ,WAIT        ;Wenn nicht, dann weiter DEC
                 POP     HL             ;restaurieren
                endif
                POP     A               ;restaurieren
                RET                     ;ZurÅck zum Aufruf
                endp

;******************************************************************************
; ASCIIZ-String ab (XHL) ausgeben:

                proc    TXTAUS
                push    xhl             ; Register retten
                push    a
Schl:           ld      a,(xhl+)        ; ein Zeichen holen
                cp      a,StrTerm       ; Endemarkierung ?
                jr      eq,Ende
                call    CONOUT          ; nein, ausgeben
                jr      Schl            ; Go on...
Ende:           pop     a               ; Register zurÅck
                pop     xhl
                ret
                endp

;******************************************************************************
; String der dem Aufruf folgt bis zur Endemarkierung ausgeben
; Siehe auch TXTAUS:

                proc    PSTR
                PUSH    XHL             ;sichern
                LD      XHL,(XSP+4)     ;Datenadresse lesen
                PUSH    F               ;sichern
                PUSH    A               ;dito
PSTR1:          LD      A,(XHL)         ;Zeichen holen
                INC     1,C             ;ZeichenzÑhler + 1
                INC     1,XHL           ;Zeichenzeiger + 1
                CP      A,StrTerm       ;Endemarkierung erwischt?
                JR      Z,PSTR2         ;Wenn ja, dann Ende
                AND     A,A             ;Null-Zeichen erwischt?
                JR      Z,PSTR1         ;Wenn ja, dann nicht ausgeben
                CALL    CONOUT          ;Zeichen ausgeben
                AND     A,A             ;War das 8. Bit gesetzt?
                JR      P,PSTR1         ;Wenn nicht, dann nÑchstes Zeichen
PSTR2:          POP     A               ;restaurieren
                POP     F               ;dito
                LD      (XSP+4),XHL     ;RÅcksprungadresse restaurieren
                POP     XHL             ;RÅcksprungadresse abspeichern
                RET                     ;ZurÅck zum Aufruf
                endp

;******************************************************************************
; "A" als ASCII Text ausgeben

                proc    A_ASC
                PUSH    F               ;Sichern
                PUSH    A               ;dito
                RR      4,A             ;high nibble an die richtige
                CALL    A_ASC00         ;Stelle und ausgeben
                POP     A               ;restaurieren bzw.
                POP     F               ;low nibble laden
A_ASC00:        AND     A,00FH          ;low nibble maskieren
                CP      A,00AH          ;eine Zahl ?
                JR      C,A_ASC01       ;ja
                ADD     A,007H          ;Offset A-F
A_ASC01:        ADD     A,'0'           ;Offset ASCII
                JP      CONOUT          ;Ausgabe und Ende UP
                endp

;******************************************************************************
; Zahl in WA dezimal ausgeben
; C = minimale Stellenzahl
; B = FÅllzeichen (z.B. ' ', '0')

                proc    WriteDec
                push    xwa             ; Register retten
                push    de

                sub     de,de           ; E zÑhlt echte Stellenzahl, D=1 falls Null
InLoop:         extz    xwa             ; fÅr Division vorbereiten
                div     xwa,10          ; letzte Stelle ausmaskieren
                add     qa,'0'          ; ASCII-Offset
                cp      d,1             ; schon im Nullbereich ?
                jr      ne,NormVal
                ld      qa,b            ; ja, dann gewÅnschtes Leerzeichen
NormVal:        push    qa              ; ein Zeichen mehr
                inc     e
                or      wa,wa           ; schon Null ?
                scc     z,d
                jr      nz,InLoop       ; wenn nicht Null, auf jeden Fall weiter
                cp      e,c             ; noch nicht genug Zeichen ?
                jr      ult,InLoop      ; dann auch weiter

OutLoop:        pop     a               ; umgekehrt ausgeben
                call    ConOut
                djnz    e,OutLoop

                pop     de
                pop     xwa
                ret
                endp

;******************************************************************************
; Modulende

                endsection
