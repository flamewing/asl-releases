; FLOAT.I68
;-----------------------------------------------------------------------------
;  Fliesskommaroutinen fuer den PC-PAR 68000, Version ohne 68881
;  entnommen mc 11/88, c't...

;-----------------------------------------------------------------------------
; Definitionen

vorz		equ		0
subflag		equ		1
maxexpo		equ		255
bias		equ		127
extend          equ             $10
e               equ             $402df854       ; exp(1)
ln2             equ             $3f317218       ; ln(2)
ln10            equ             $40135d8e       ; ln(10)
eins            equ             $3f800000       ; 1.0
zwei            equ             $40000000       ; 2.0
pi2             equ             $40c90fdb       ; Pi*2
pi              equ             $40490fdb       ; Pi
pihalf          equ             $3fc90fdb       ; Pi/2

;-----------------------------------------------------------------------------
; Librarykopf:


S_FloatLib:     dc.l            S_floatlibend-S_floatlibstart ; Laenge
S_floatlibstart:
                dc.l            -1              ; Speicher fuer Zeiger
		dc.b            "FLOAT",0       ; Name
                ds              0



;-----------------------------------------------------------------------------
; Sprungtabelle:

		bra.l		S_fadd
		bra.l		S_fsub
		bra.l		S_fmul
		bra.l		S_fdiv
		bra.l		S_fmul2
		bra.l		S_fsqrt
                bra.l           S_fabs
                bra.l           S_floatlibnop
		bra.l		S_fcmp
		bra.l		S_fitof
		bra.l		S_fftoi
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
		bra.l		S_fexp
		bra.l		S_fsinh
		bra.l		S_fcosh
		bra.l		S_ftanh
		bra.l		S_fcoth
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
		bra.l		S_fln
		bra.l		S_flog
		bra.l		S_fasinh
		bra.l		S_facosh
		bra.l		S_fatanh
		bra.l		S_facoth
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
		bra.l		S_fsin
		bra.l		S_fcos
		bra.l		S_ftan
		bra.l		S_fcot
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
                bra.l           S_floatlibnop
		bra.l		S_fasin
		bra.l		S_facos
		bra.l		S_fatan
		bra.l		S_facot


;-----------------------------------------------------------------------------
; Konstanten :

S_Const1        dc.s            1.0

;-----------------------------------------------------------------------------
; Nullprozedur :

S_floatlibnop:  rts

;-----------------------------------------------------------------------------
; Addition :  D0.S = D0.S + D1.S

		ds		0
S_fadd:
		addq.l		#1,_fadd_cnt.w
                movem.l		d1-d5,-(a7)	; benoetigte Register retten
		rol.l		#1,d0		; Operanden rotieren und in Form
		rol.l		#1,d1		; eeee eeee ffff ... fffs bringen
		move.l		d0,d2
		sub.l		d1,d2		; Differenz beider Zahlen bilden
		bcc.s		fadd_1
		exg		d0,d1		; ggf. vertauschen, so dass der
fadd_1:		move.b		d0,d3		; kleinere in Register D1 steht
		and.b		#1,d3     	; maskiere das Vorzeichenbit
		btst		#vorz,d2	; haben beide gleiches Vorzeichen ?
		beq.s		fadd_2		; bei verschiedenen Vorzeichen
		bset		#subflag,d3	; Flag fuer Subtraktion setzen
fadd_2:		rol.l		#8,d0		; Form: ffff ... fffs eeee eeee
		clr.w		d4		; Exponent der ersten Zahl
		move.b		d0,d4		; wird im Register D4 aufgebaut
		sne		d0		; falls ungleich Null, dann
		ror.l		#1,d0		; implizite Eins, sonst implizite
		clr.b		d0		; Null erzeugen, neu positionieren

		rol.l		#8,d1		; jetzt das gleiche fuer den
		clr.w		d5		; zweiten Operanden, der Exponent
		move.b		d1,d5		; kommt ins Register D5
		sne		d1
		ror.l		#1,d1
		clr.b		d1

; In den Registern D0 und D1 stehen jetzt nur noch die Mantissen
; im Format ffff ... ffff 0000 0000, also linksbuendig, wobei die
; ehemals implizite Null bzw. Eins nun explizit an erster Stelle steht.
; In den Registern D4 und D5 stehen die Exponenten der beiden Zahlen.
; Das Vorzeichen des Ergebnisses sowie die Subtraktionsflags sind im
; Register D3 zwischengespeichert.

		move.w		d4,d2		; Jetzt Differenz der Exponenten
		sub.w		d5,d2		; berechnen
		cmp.w		#24,d2		; groesser als 24 ?
		bgt.s		fadd_rnd	; ja, --> Ergebnis ist groessere Zahl
		lsr.l		d2,d1		; Mantisse um (D2)-Bits verschieben
		btst		#subflag,d3	; Subtraktion oder Addition ?
		bne.s		fadd_subtr	; ggf. zur Subtraktion springen
		add.l		d1,d0		; die beiden Mantissen addieren
		bcc.s		fadd_rnd	; kein Ueberlauf --> zum Runden
		roxr.l		#1,d0		; Ueberlauf einschieben
		addq.w		#1,d4		; Exponent korrigieren
		bra.s		fadd_rnd	; und zum Runden

fadd_subtr:	sub.l		d1,d0		; die beiden Mantissen subtrahieren
		beq.s		fadd_zero	; bei Null ist das Gesamtergebnis Null
		bmi.s		fadd_rnd	; bei fuehrender Eins zum Runden
fadd_nrm:	tst.w		d4		; Exponent ist schon Null ?
		beq.s		fadd_rnd	; dann ist Ergebnis denormalisiert
		subq.w		#1,d4		; Exponent erniedrigen
		lsl.l		#1,d0		; Mantisse normalisieren bis
		bpl.s		fadd_nrm	; fuehrende Eins auftaucht

fadd_rnd:	add.l		#$80,d0		; jetzt Runden auf Bit hinter
		bcc.s		fadd_nov	; Mantisse
		roxr.l		#1,d0		; bei Ueberlauf Mantisse normalisieren
		addq.w		#1,d4		; und Exponent korrigieren
fadd_nov:	clr.b		d0		; Rest-Mantisse loeschen
		tst.l		d0		; Ist die Mantisse komplett Null ?
		beq.s		fadd_zero	; ja, dann ist Ergebnis auch Null
		cmp.w		#maxexpo,d4	; Exponent-Ueberlauf ?
		blt.s		fadd_nue
		move.w		#maxexpo,d4	; Unendlich Exponent = maxexpo
		clr.l		d0		; 	    Mantisse = Null
		bra.s		fadd_den

fadd_nue:	tst.w		d4		; Exponent Null ( Zahl denormalisiert? )
		beq.s		fadd_den	; ja -->
		lsl.l		#1,d0		; fuehrendes Bit wird nicht gespeichert
fadd_den:	move.b		d4,d0		; Exponent einsetzen
		ror.l		#8,d0		; Form: eeee eeee ffff ... fffx
		roxr.b		#1,d3		; Vorzeichen in Carry schieben
		roxr.l		#1,d0		; Form: seee eeee efff ... ffff

fadd_zero:
		movem.l		(a7)+,d1-d5	; Register restaurieren
		rts				; Ende, Ergebnis steht in D0.L

;-----------------------------------------------------------------------------
; Subtraktion :  D0.S = D0.S - D1.S

		ds		0
S_fsub:
		bchg		#31,d1		; Vorzeichen des zweiten Operanden
                bra             S_fadd          ; invertieren und zur Addition springen


;-----------------------------------------------------------------------------
; Multiplikation : D0.S = D0.S * D1.S

		ds		0
S_fmul:
		addq.l		#1,_fmul_cnt.w
		movem.l		d1-d5,-(a7)	; benoetigte Register retten
		move.l		d0,d2		; Operand 1 kopieren
		eor.l		d1,d2		; EXOR um Vorzeichen zu bestimmen

		swap		d0		; Registerhaelften Operand 1 vertauschen
		move.l		d0,d3		; Operand 1 ab jetzt in Register D3
		and.w		#$7f,d3		; Exponent und Vorzeichen loeschen
		and.w		#$7f80,d0	; Exponent maskieren
		beq.s		fmul_dn1	; gleich Null: Zahl ist denormalisiert
		bset		#7,d3		; implizite Eins einsetzen
		sub.w		#$0080,d0	; Bias kompensieren

fmul_dn1:	swap		d1		; jetzt Operand 2 behandeln
		move.w		d1,d4
		and.w		#$7f,d1
		and.w		#$7f80,d4
		beq.s		fmul_dn2
		bset		#7,d1
		sub.w		#$0080,d4       ; Bias kompensieren

fmul_dn2:	add.w		d0,d4		; Exponenten addieren
		lsr.w		#7,d4		; richtig positionieren
		sub.w		#bias-3,d4	; Bias-3 subtrahieren
		cmp.w		#-24,d4		; totaler Unterlauf ?
		blt.s		fmul_zero	; ja, dann ist Ergebnis Null

		move.w		d3,d0		; oberes Mantissenwort von Operand 1
		mulu		d1,d0		; mal oberem Mantissenwort von Op2
		swap		d0		; entspricht Verschiebung um 16 Bit

; Das obere Wort von D0 ist nach der Multiplikation auf jeden Fall Null,
; da die oberen Mantissenworte nur im Bereich 0 ... 255 liegen.
; Das groete moegliche Ergebnis ist also 255 x 255 = 65025 = 0000FE01.
; Nach der Vertauschung erhalten wir also eine Zahl der xxxx 0000.
; Die untere Registerhaelfte von D0 koennen wir kurzzeitig als Zwischen-
; speicher verwenden.

		move.w		d3,d0		; oberes Wort von Operand 1 merken
		swap		d3		; jetzt unteres Wort Op1 mal oberes Op2
		move.w		d1,d5
		mulu		d3,d5		; Ergebnis steht im D5
		swap		d1		; jetzt unteres Wort Op1 mal unteres Op2
		mulu		d1,d3		; Ergebnis steht im D3
		swap		d3		; entspricht Verschiebung um 16 Bit
		mulu		d0,d1		; jetzt oberes Wort Op1 mal unteres Op2

		move.w		d3,d0		; zum ersten Zwischenergebnis dazu
		add.l		d5,d0		; jetzt alles aufaddieren
		add.l		d1,d0
		beq.s		fmul_res	; falls Mantisse Null auch Ergebnis Null
		bmi.s		fmul_rnd	; fuehrende Eins? dann zum Runden

; Im Register D0.L befinden sich die oberen 32 Bit des Produktes,
; im oberen Wort von D3 die restlichen 16 Bit.

		tst.w		d4		; Exponent ist negativ ?
		bmi.s		fmul_unt	; ggf. Unterlauf behandeln

fmul_nor:	tst.w		d4		; Exponent = Null ?
		beq.s		fmul_rnd	; falls Null, dann zum Runden
		roxl.l		#1,d3		; Im oberen Wort von D3 sind die
		roxl.l		#1,d0		; niedrigsten Bits des Produktes
		subq.w		#1,d4		; Exponent korrigieren
		tst.l		d0		; Mantisse testen
		bpl.s		fmul_nor	; bis fuehrende Eins auftaucht

fmul_rnd:	add.l		#$80,d0		; Rundung
		bcc.s		fmul_nov
		roxr.l		#1,d0		; Ueberlauf einschieben
		addq.w		#1,d4           ; Exponent korrigieren
fmul_nov:	cmp.w		#maxexpo,d4	; Exponent-Ueberlauf ?
		blt.s		fmul_nue
fdiv_err:	move.w		#maxexpo,d4	; Ueberlauf: Exponent = Maxexpo
		clr.l		d0		;           Mantisse = Null
		bra.s		fmul_den

fmul_nue:	tst.w		d4		; Exponent = Null ?
		beq.s		fmul_den	; falls Null, dann denormalisiert
		lsl.l		#1,d0		; fuehrende Eins wird nicht abgespeichert

fmul_den:       move.b		d4,d0		; Exponent einsetzen
		ror.l		#8,d0		; Form: eeee eeee ffff ... fffx
		roxl.l		#1,d2		; Vorzeichen in Carry schieben
		roxr.l		#1,d0		; und ins Ergebnis einsetzen

fmul_res:	movem.l		(a7)+,d1-d5	; Register restaurieren
		rts

fmul_zero:	clr.l		d0		; Null erzeugen
		bra.s		fmul_res	; Ende, Ergebnis steht in D0.L

fmul_unt:	cmp.w		#-24,d4		; totaler Unterlauf ?
		ble.s		fmul_zero	; Dann ist das Ergebnis auf jeden Fall Null
		neg.w		d4		; sonst Shift-Zaehler erzeugen
		lsr.l		d4,d0		; und Zahl denormalisieren
		clr.w		d4		; Exponent ist Null als Kennzeichen
		bra.s		fmul_rnd	; fuer eine denormalisierte Zahl

;-----------------------------------------------------------------------------
; Division : D0.S = D0.S / D1.S

		ds		0
S_fdiv:
		addq.l		#1,_fdiv_cnt.w
		movem.l		d1-d5,-(a7)	; benoetigte Register retten
		move.l		d0,d2		; Operand 1 kopieren
		eor.l		d1,d2		; EXOR um Vorzeichen zu bestimmen

		swap		d0		; Registerhaelften Operand 1 vertauschen
		move.l		d0,d3		; Operand 1 ab jetzt in Register D3
		and.w		#$7f,d3		; Exponent und Vorzeichen loeschen
		and.w		#$7f80,d0	; Exponent maskieren
		beq.s		fdiv_dn1	; gleich Null: Zahl ist denormalisiert
		bset		#7,d3		; implizite Eins einsetzen
		sub.w		#$0080,d0	; Bias kompensieren

fdiv_dn1:	swap		d1		; jetzt Operand 2 behandeln
		move.w		d1,d4
		and.w		#$7f,d1
		and.w		#$7f80,d4
		beq.s		fdiv_dn2
		bset		#7,d1
		sub.w		#$0080,d4

fdiv_dn2:	sub.w		d4,d0		; Exponenten subtrahieren
		move.w		d0,d4		; Exponent nach D4 kopieren
		asr.w		#7,d4		; richtig positionieren
		add.w		#bias,d4	; Bias addieren
		cmp.w		#-24,d4		; totaler Ueberlauf ?
		blt.s		fmul_zero	; ja, dann ist Ergebnis Null

		swap		d1		; Form: 0fff ... ffff 0000 0000
		beq.s		fdiv_err	; falls Divisor Null, dann wird
		lsl.l		#7,d1		; als Ergebnis unendlich ausgegeben
		swap		d3
		beq.s		fmul_zero	; falls Divident Null --> Ergebnis Null
		lsl.l		#7,d3

fdiv_nlp:	btst		#30,d1		; ist der Divisor normalisiert ?
		bne.s		fdiv_nor	; ja, -->
		addq.w		#1,d4		; nein, Exponent erhoehen
		lsl.l		#1,d1		; Divisor verschieben bis Form 01ff ..
		bra.s		fdiv_nlp

fdiv_nor:	clr.l		d0		; Ergebnis vorbesetzen
		add.w		#25,d4		; Exponent ist nicht groesser als Null

fdiv_lop:	move.l		d3,d5		; Divident zwischenspeichern
		sub.l		d1,d3		; Divisor abziehen
		eori		#extend,ccr	; X-Bit invertieren
		bcc.s		fdiv_one	; kein Carry: Divisor passt
		move.l		d5,d3           ; zurueckkopieren (X-Bit unveraendert!)
fdiv_one:	roxl.l		#1,d0		; Ergebnis aufbauen
		lsl.l		#1,d3		; Divident verschieben
		subq.w		#1,d4		; Exponent erniedrigen
		beq.s		fdiv_den	; falls Null, dann denormalisiert
		btst		#24,d0		; fuehrende Eins in Ergebnis-Mantisse?
		beq.s		fdiv_lop	; nein, weiter rechnen

fdiv_den:	lsl.l		#7,d0		; Mantisse positionieren
		beq		fmul_res	; Null ?
		bra		fmul_rnd	; zum Runden
;-----------------------------------------------------------------------------
; Multiplikation mit einer Zweierpotenz: D0.S=D0.S * 2^(D1.W)

		ds		0
S_fmul2:
		addq.l		#1,_fmul_cnt.w
		movem.l         d1-d2,-(a7)	; Register retten
		move.l          d0,d2		; Vorzeichen in D2 Bit 31 merken
		lsl.l           #1,d0		; Vorzeichen rausschieben
		beq.s           fmul2_zero	; falls Null, dann ist Ergebnis Null
		rol.l           #8,d0		; Form: ffff ... fff0 eeee eeee
		clr.w           d2		; auf Wort vorbereiten
		move.b          d0,d2		; Exponent in D2
                beq.s           fmul2_den
		tst.w           d1		; Multiplikation oder Division?
		bmi.s           fmul2_div	; (neg. Exponent entspr. Div.)

		add.w           d1,d2		; Summe der Exponenten bilden
		cmp.w           #maxexpo,d2	; Ueberlauf?
		bge.s           fmul2_over	; ja, Ergebnis ist unendlich
fmul2_res:	move.b          d2,d0		; Ergebnisexponent einsetzen
		ror.l           #8,d0		; Form: eeee eeee ffff ... fffx
		roxl.l          #1,d2		; Vorzeichen ins X-Bit
		roxr.l          #1,d0		; und ins Ergebnis einschieben
fmul2_zero:	movem.l         (a7)+,d1-d2	; Register restaurieren
                rts

fmul2_over:	move.w          #maxexpo,d2	; Unendlich: Exponent = maxexpo
		clr.l           d0		;            Mantisse = Null
                bra.s           fmul2_res

fmul2_div:	add.w           d1,d2		; Summe der Exponenten bilden
		bgt.s           fmul2_res	; Unterlauf? nein --> Ergebnis
		ori             #Extend,ccr	; implizite Eins real machen
		roxr.l          #1,d0		; Form: 1fff ... ffff xxxx xxxx
fmul2_dnr:	tst.w           d2		; Exponent = Null ?
		beq.s           fmul2_res	; ja, Ergebnis ist denormalisiert
		lsr.l           #1,d0		; Mantisse denormalisieren
		beq.s           fmul2_zero	; totaler Unterlauf: Ergebnis ist Null
		addq.w          #1,d2		; Exponent korrigieren
                bra.s           fmul2_dnr
fmul2_ddd:	add.w           d1,d2		; Summe der Exponenten bilden
		bra.s           fmul2_dnr	; mit denormalisiereter Eingabe bearbeiten

fmul2_den:	tst.w           d1		; Multiplikation oder Division
                bmi.s           fmul2_ddd
		clr.b           d0		; Form: ffff ... fff0 0000 0000
fmul2_nor:	lsl.l           #1,d0		; Mantisse nach links schieben
		bcs.s           fmul2_stp	; bis fuehrende Eins auftaucht
		subq.w          #1,d1		; oder zweiter Exponent Null wird
                bne.s           fmul2_nor
		bra.s           fmul2_res	; Ergebnis abliefern
fmul2_stp:	add.w           d1,d2		; Rest zum Exponenten addieren
		bra.s           fmul2_res	; Bias stimmt auch ( jetzt 127 statt 126)

;-----------------------------------------------------------------------------
; Vergleich zweier Zahlen: cmp d0,d1

S_fcmp:
		bclr		#31,d0		; Zahl 1 >=0 ?
		bne.s		fcmp_2
fcmp_1:
		bclr		#31,d1		; Zahl 2 >=0 ?
		bne.s		fcmp_12
fcmp_11:
		cmp.l		d1,d0		; beide Zahlen >=0
		rts				; dann Betraege vergleichen
fcmp_12:
		moveq.l		#1,d0		; Zahl 1 >=0 und Zahl 2 <0
                cmp.l           #-1,d0
		rts
fcmp_2:
		bclr		#31,d1		; Zahl 2 >=0 ?
		bne.s		fcmp_22
fcmp_21:
                moveq.l         #-1,d0          ; Zahl 1 <0 und Zahl 2 >=0
		cmp.w		#1,d0		; dann kleiner
		rts
fcmp_22:
                neg.l           d0
                neg.l           d1
                cmp.l           d1,d0           ; beide Zahlen <0, dann ver-
		rts				; kehrtherum vergleichen


;-----------------------------------------------------------------------------
; Longint-->Gleitkomma
; D0.L --> D0.S

S_fitof:
		movem.l		d1-d2,-(a7)	; Register retten
		tst.l		d0		; Integer ist Null ?
		beq.s		fitof_res; Ergebnis ist auch Null
		smi		d1		; Vorzeichen in D1 merken
		bpl.s           fitof_pos
		neg.l		d0		; ggf. Integer negieren
fitof_pos:	move.w		#bias+32,d2	; Exponent vorbesetzen
fitof_shift:	subq.w		#1,d2		; Mantisse verschieben
		lsl.l		#1,d0		; bis fuehrende Eins rausfliegt
		bcc.s		fitof_shift
		move.b		d2,d0		; Exponent einsetzen
		ror.l		#8,d0		; Zahl positionieren
		roxr.b		#1,d1		; Vorzeichen in X-Bit
		roxr.l		#1,d0		; und ins Ergebnis
fitof_res:	movem.l		(a7)+,d1-d2	; fertig
		rts

;-----------------------------------------------------------------------------
; Gleitkomma --> Longint:
; D0.S --> D0.L

S_fftoi:
		movem.l		d1-d2,-(a7)	; Register retten
		roxl.l		#1,d0		; Vorzeichen in Carry
		scs		d1		; in D1 merken
		rol.l		#8,d0		; Form: ffff ... fffx eeee eeee
		move.b		d0,d2		; Exponent extrahieren
		sub.b		#bias,d2	; Bias subtrahieren
		bmi.s		fftoi_zero	; kleiner Null -> Ergebnis = Null
		cmp.b		#31,d2		; Ueberlauf?
		bge.s		fftoi_over
		ori		#extend,ccr	; Implizite Eins explizit machen
		roxr.l		#1,d0
		clr.b		d0		; Form: 1fff ... ffff 0000 0000
fftoi_shft:
		lsr.l		#1,d0		; jetzt Verschiebung bis
		addq.b		#1,d2		; Exponent stimmt
		cmp.b		#31,d2
		bne.s		fftoi_shft
		tst.b		d1		; Zahl negativ ?
		bpl.s		fftoi_pos
		neg.l		d0       	; ggf. Ergebnis negieren
fftoi_pos:
		movem.l		(a7)+,d1-d2	; Register wieder holen
		rts
fftoi_zero:
		clr.l		d0		; Unterlauf; Ergebnis ist Null
		bra.s		fftoi_pos
fftoi_over:
		move.l		#$7fffffff,d0	; Ueberlauf: Maxint zurueckgeben
		tst.b		d1		; positiv oder negativ ?
		bpl.s		fftoi_pos
		not.l		d0		; Einser-Komplement erzeugt Minint
		bra.s		fftoi_pos

;-----------------------------------------------------------------------------
; Quadratwurzel : D0.S-->D0.S

		ds		0
fsqrt_domainerror:
		move.l		#$ffc00000,d0	; -NAN zurueckgeben
		movem.l		(a7)+,d1-d4
		rts
fsqrt_sq0:
		clr.l		d0
		movem.l		(a7)+,d1-d4
		rts
S_fsqrt:
		addq.l          #1,_fsqrt_cnt.w
		movem.l		d1-d4,-(a7)	; D1-D4 werden sonst zerstoert
		move.l		d0,d4
		bmi.s		fsqrt_domainerror ; Fehler bei negativem Argument
		swap		d4		; MSW des Arguments
		and.l		#$7f80,d4	; Exponent isolieren
		beq.s		fsqrt_sq0	; Zahl ist 0, wenn Exponent 0
		and.l		#$007fffff,d0	; Mantisse isolieren
		sub.w		#$7f*$80,d4	; Exponent im Zweierkomplement
		bclr		#7,d4		; Exponent ungerade? (und LSB auf 0)
		beq.s		fsqrt_evenexp
		add.l		d0,d0		; ja: Mantisse * 2
		add.l		#$01000000-$00800000,d0 ; Hidden Bit setzen, 1.Iteration

fsqrt_evenexp:
		; 1. Iteration fuer geraden Exponenten: Hidden Bit nicht setzen
		asr.w		#1,d4		; Exponent/2 mit Vorzeichen
		add.w		#$7f*$80,d4	; Exponent wieder in Offset-Darst.
		swap		d4		; neuen Exponenten im MSW aufheben
		lsl.l		#7,d0		; x ausrichten
		move.l		#$40000000,d2	; xroot nach erster Iteration
		move.l		#$10000000,d3	; m2=2 << (MaxBit-1);
fsqrt_loop10:
		move.l		d0,d1		; xx2 = x
fsqrt_loop11:
		sub.l		d2,d1		; xx2 -= root
		lsr.l		#1,d2		; xroot >>= 1
		sub.l		d3,d1		; x2 -= m2
		bmi.s		fsqrt_dontset1
		move.l		d1,d0		; x = xx2
		or.l		d3,d2		; xroot += m2
		lsr.l		#2,d3		; m2 >>= 2
		bne.s		fsqrt_loop11
		bra.s		fsqrt_d0d1same
fsqrt_dontset1:
		lsr.l		#2,d3		; m2 >>= 2
		bne.s		fsqrt_loop10	; Schleife 15* abarbeiten
						; Bit 22..8
		; 17. Iteration (Bit 7) mit separatem Code durchfuehren:
		move.l		d0,d1		; xx2 = x
fsqrt_d0d1same:
		sub.l		d2,d1		; xx2 -= root
		ror.l		#1,d2		; xroot >>= 1 mitsamt Carry...
		swap		d2		; auf neues Alignment umstellen
		subq.l		#1,d1		; Carry von 0-0x4000: x2 -= m2
						; Teil 1
		bmi.s		fsqrt_dontset7
		or.l		#-$40000000,d1	; 0 - 0x4000: x2 -= m2, Teil 2
		move.l		d1,d0		; x = xx2
		or.w		#$4000,d2	; xroot += m2
fsqrt_dontset7:
		swap		d0		; x auf neues Alignment umstellen

		move.w		#$1000,d3	; m2 - Bit 16..31 bereits 0
fsqrt_loop20:
		move.l		d0,d1		; xx2 = x
fsqrt_loop21:
		sub.l		d2,d1		; xx2 -= xroot
		lsr.l		#1,d2		; xroot >>= 1
		sub.l		d3,d1		; x2 -= m2
		bmi.s		fsqrt_dontset2
		move.l		d1,d0		; x = xx2
		or.w		d3,d2		; xroot += m2
		lsr.w		#2,d3		; m2 >>= 2
		bne.s		fsqrt_loop21
		bra.s		fsqrt_finish
fsqrt_dontset2:
		lsr.w           #2,d3		; m2 >>= 2
		bne.s		fsqrt_loop20	; Schleife 7 * abarbeiten (n=6..0)
fsqrt_finish:
		sub.l		d2,d0		; Aufrunden notwendig ?
                bls.s           fsqrt_noinc
		addq.l		#1,d2		; wenn ja, durchfuehren
fsqrt_noinc:
		bclr		#23,d2		; Hidden Bit loeschen
		or.l		d4,d2		; Exponent und Mantisse kombinieren
		move.l		d2,d0		; Ergebnis
		movem.l		(a7)+,d1-d4
		rts				; Z-,S-, und V-Flag o.k.

;-----------------------------------------------------------------------------
; Absolutbetrag: D0.S--> D0.S

                ds      0

S_fabs:         bclr    #31,d0                  ; ganz einfach...
                rts

;-----------------------------------------------------------------------------
; Exponentialfunktion: D0.S--> D0.S

; Die "krummen" Konstanten legen wir als hex ab, damit es keine Vergleichs-
; fehler durch Rundungsvarianzen gibt.

S_fexp_Const0:  dc.l $3FB8AA3B  ; ld(exp(1.0))    = ld(e) = 1/ln(2)
S_fexp_ConstA:  dc.l $3D0DF4E0  ; 0.034657359038  Polynomkonstanten
S_fexp_ConstB:  dc.l $411F4606  ; 9.9545957821
S_fexp_ConstC:  dc.l $441A7E3A  ; 617.97226953
S_fexp_ConstD:  dc.l $42AED5C2  ; 87.417498202

		ds		0
S_fexp:         movem.l         d1-d5,-(sp)

                bclr            #31,d0          ; Vorzeichen loeschen und nach D2 retten
                sne             d2

                move.l          S_fexp_Const0(pc),d1 ; auf 2erpotenz umrechnen
                bsr             S_fmul

                move.l          d0,d3           ; in Ganzzahlanteil und Nach-
                bsr             S_fftoi         ; kommastellen (z) zerlegen
                move.l          d0,d4           ; Ganzzahlanteil nach D4
                bsr             S_fitof
                move.l          d0,d1
                move.l          d3,d0
                bsr             S_fsub
                move.l          d0,d3

                move.l          d0,d1           ; z^2 berechnen
                bsr             S_fmul
                move.l          d0,d5           ; noch zu gebrauchen

                move.l          S_fexp_ConstD(pc),d1 ; --> D+z^2
                bsr             S_fadd
                move.l          d0,d1           ; --> C/(..)
                move.l          S_fexp_ConstC(pc),d0
                bsr             S_fdiv
                move.l          d0,d1           ; --> B-(..)
                move.l          S_fexp_ConstB(pc),d0
                bsr             S_fsub
                move.l          d3,d1           ; --> (..)-z
                bsr             S_fsub
                exg             d0,d5           ; Ergebnis retten
                move.l          S_fexp_ConstA(pc),d1 ; A*z^2 berechnen
                bsr             S_fmul
                move.l          d5,d1           ; ergibt Nenner
                bsr             S_fadd
                move.l          d0,d1           ; Quotient bilden
                move.l          d3,d0
                bsr             S_fdiv
                moveq           #1,d1           ; verdoppeln
                bsr             S_fmul2
                move.l          S_Const1(pc),d1 ; 1 addieren
                bsr             S_fadd
                move.l          d4,d1           ; Potenzieren
                bsr             S_fmul2

                tst.b           d2              ; war Argument negativ ?
                beq.s           S_fexp_ArgPos
                move.l          d0,d1           ; dann Kehrwert bilden
                move.l          S_Const1(pc),d0
                bsr             S_fdiv

Terminate:
S_fexp_ArgPos:  movem.l         (sp)+,d1-d5

		rts

;------------------------------------------------------------------------------
; Sinus hyperbolicus: D0.S-->D0.S

S_fsinh:
		movem.l		d1-d2,-(a7)	; Register retten
		bsr		S_fexp		; exp(x) berechnen
		move.l		d0,d2		; in D2 merken
		move.l		d0,d1		; exp(-x)=1/exp(x) berechnen
		move.l		#eins,d0
		bsr		S_fdiv
		move.l		d0,d1           ; Teilergebnisse subtrahieren
		move.l		d2,d0
		bsr		S_fsub
		move.w		#-1,d1		; halbieren
		bsr		S_fmul2
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts

;------------------------------------------------------------------------------
; Cosinus hyperbolicus: D0.S-->D0.S

S_fcosh:
		movem.l		d1-d2,-(a7)	; Register retten
		bsr		S_fexp		; exp(x) berechnen
		move.l		d0,d2		; in D2 merken
		move.l		d0,d1		; exp(-x)=1/exp(x) berechnen
		move.l		#eins,d0
		bsr		S_fdiv
		move.l		d2,d1           ; Teilergebnisse addieren
		bsr		S_fadd
		move.w		#-1,d1		; halbieren
		bsr		S_fmul2
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Tangens hyperbolicus: D0.S-->D0.S

S_ftanh:
		movem.l		d1-d3,-(a7)     ; Register sichern
		bsr		S_fexp		; exp(x) berechnen
		move.l		d0,d2		; in D2 merken
		move.l		d0,d1		; exp(-x)=1/exp(x) berechnen
		move.l		#eins,d0
		bsr		S_fdiv
		move.l		d0,d3           ; in D3 merken
		move.l		d2,d1		; Summe=Nenner berechnen
		bsr		S_fadd
		exg		d0,d2		; jetzt exp(x) in D0, Nenner
		move.l		d3,d1		; in D2
		bsr		S_fsub		; Zaehler berechnen
		move.l		d2,d1		; Quotient berechnen
		bsr		S_fdiv
		movem.l		(a7)+,d1-d3	; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Cotangens hyperbolicus: D0.S-->D0.S

S_fcoth:
		tst.l		d0		; Argument Null ?
		beq.s		S_fcoth_valerr	; dann zur Fehlerroutine
		movem.l		d1-d3,-(a7)	; Register sichern
		bsr		S_fexp		; exp(x) berechnen
		move.l		d0,d2		; in D2 merken
		move.l		d0,d1		; exp(-x)=1/exp(x) berechnen
		move.l		#eins,d0
		bsr		S_fdiv
		move.l		d0,d3           ; in D3 merken
		move.l		d0,d1		; Differenz=Nenner berechnen
		move.l		d2,d0
		bsr		S_fsub
		exg		d0,d2		; jetzt exp(x) in D0, Nenner
		move.l		d3,d1		; in D2
		bsr		S_fadd		; Zaehler berechnen
		move.l		d2,d1		; Quotient berechnen
		bsr		S_fdiv
		movem.l		(a7)+,d1-d3	; Register zurueck
		rts
S_fcoth_valerr:
		move.l		#$7f800000,d0	; +INF zurueckgeben
		rts

;-----------------------------------------------------------------------------
; nat. Logarithmus: D0.S-->D0.S

		ds		0
S_fln:
		tst.l		d0		; Argument <=0 ?
		ble		S_fln_errval
                movem.l         d1-d7,-(a7)     ; Register retten
		move.l		d0,d3		; Argument sichern

		move.l		#eins,d1	; Zahl>1?
		bsr		S_fsub		; ( dies ist sinnvoll bei
		tst.l           d0		;   Zahlen <<1 );
                smi             d7              ; und die Vorzeichenumkehr merken
		bpl.s		S_fln_gr1	; ja-->o.k.
		move.l		d3,d1		; ansonsten Kehrwert bilden
		move.l		#eins,d0
		bsr		S_fdiv
		move.l		d0,d3

S_fln_gr1:
		clr.l		d2		; Merker = Null
S_fln_nrm:
		move.l		d3,d0		; Zahl > 1 ?
		move.l		#eins,d1
		bsr		S_fsub
		bmi.s		S_fln_isok
                beq.s           S_fln_isok
		sub.l		#$00800000,d3   ; ja-->Zahl durch 2 teilen...
		addq.w		#1,d2		; ...und Merker erhoehen
		bra.s		S_fln_nrm	; nochmal probieren
S_fln_isok:
		move.l		d0,d3		; Zahl um Eins erniedrigt abspeichern
		move.l		d0,d4		; yz:=y
		moveq.l		#1,d6		; zaehler:=1
		clr.l		d5              ; Summe:=0
                bchg            #31,d3          ; Multiplikator negativ
S_fln_loop:
		move.l          d6,d0		; Zaehler in Gleitkomma wandeln
		bsr		S_fitof
		move.l		d0,d1		; s:=s+yz/zaehler*vz
		move.l		d4,d0
		bsr		S_fdiv
		move.l		d5,d1
		bsr		S_fadd
		cmp.l           d5,d0		; noch signifikant ?
		beq.s		S_fln_loopend
		move.l		d0,d5
		addq.w		#1,d6		; zaehler:=zaehler+1
		cmp.w		#10,d6		; Schleife fertig ?
		beq.s		S_fln_loopend
		move.l		d4,d0		; yz:=yz*y
		move.l		d3,d1
		bsr		S_fmul
		move.l		d0,d4
		bra.s		S_fln_loop
S_fln_loopend:
		move.l		d2,d0		; Merker in Gleitkomma
		bsr		S_fitof
		move.l		#ln2,d1		; * ln(2)
		bsr 		S_fmul
		move.l		d5,d1		; s:=s+merker
		bsr		S_fadd

                tst.b           d7              ; noch Vorzeichen tauschen ?
		beq.s		S_fln_end
                bchg            #31,d0
S_fln_end:
                movem.l         (a7)+,d1-d7     ; Register zurueck
		rts
S_fln_errval:
		move.l		#$ffc00000,d0	; -NAN zurueckgeben
		rts

;-----------------------------------------------------------------------------
; 10er-Logarithmus : D0.S --> D0.S

S_flog:
		tst.l		d0		; Argument <=0 ?
                ble.s           S_flog_errval
		bsr		S_fln		; nat. Logarithmus bilden
		move.l		#ln10,d1	; umrechnen
		bsr		S_fdiv
		rts
S_flog_errval:
		move.l		#$ffc00000,d0	; -NAN zurueckgeben
		rts

;-----------------------------------------------------------------------------
; Areasinus hyperbolicus: D0.S-->D0.S    == ln[x+sqrt(x*x+1)]

S_fasinh:
		movem.l		d1-d2,-(a7)
		move.l		d0,d2		; Argument sichern
		move.l		d0,d1		; quadrieren
		bsr		S_fmul
		move.l		#eins,d1	; 1 addieren
		bsr		S_fadd
		bsr		S_fsqrt		; Wurzel ziehen
		move.l		d2,d1		; Argument addieren
		bsr		S_fadd
		bsr		S_fln		; Logarithmus des ganzen
		movem.l		(a7)+,d1-d2
		rts

;-----------------------------------------------------------------------------
; Areacosinus hyperbolicus: D0.S-->D0.S    == ln[x+sqrt(x*x-1)]

S_facosh:
		movem.l		d1-d2,-(a7)	; Register sichern
		move.l		d0,d2		; Argument sichern
		move.l		#eins,d1	; Argument <1 ?
		bsr		S_fcmp
		bmi.s		S_facosh_errval
		move.l		d2,d0		; Argument zurueck
		move.l		d0,d1		; quadrieren
		bsr		S_fmul
		move.l		#eins,d1	; 1 abziehen
		bsr		S_fsub
		bsr		S_fsqrt		; Wurzel ziehen
		move.l		d2,d1		; Argument addieren
		bsr		S_fadd
		bsr		S_fln		; Logarithmus des ganzen
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts
S_facosh_errval:
		movem.l		(a7)+,d1-d2	; Register zurueck
		move.l		#$ffc00000,d0	; NAN zurueckgeben
		rts

;-----------------------------------------------------------------------------
; Areatangens hyperbolicus: D0.S-->D0.S   == 0.5*ln((1+x)/(1-x))

S_fatanh:
		movem.l		d1-d2,-(a7)	; Register sichern
		move.l		d0,d2		; Argument sichern
		bclr		#31,d0		; Vorzeichen weg
		cmp.l		#eins,d0
		beq.s		S_fatanh_inf	; =1-->INF
		bhi.s		S_fatanh_errval	; >1-->NAN
		move.l		d2,d1		; Nenner berechnen
		move.l		#eins,d0
		bsr		S_fsub
		exg		d0,d2   	; Zaehler berechnen
		move.l		#eins,d1
		bsr		S_fadd
		move.l		d2,d1		; Quotient daraus
		bsr		S_fdiv
		bsr		S_fln		; logarithmieren
		move.w		#-1,d1		; halbieren
		bsr		S_fmul2
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts
S_fatanh_inf:
		move.l		#$ff000000,d0	; vorzeichenbehaftete Unend-
		roxr.l		#1,d0		; lichkeit
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts
S_fatanh_errval:
		move.l		#$7fc00000,d0	; NAN geben
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Areakotangens hyperbolicus: D0.S--> D0.S  ==  0.5*ln((1+x)/(x-1))

S_facoth:
		movem.l		d1-d2,-(a7)	; Register sichern
		move.l		d0,d2		; Argument sichern
		roxl.l		#1,d0		; Vorzeichen in X-Flag
		cmp.l		#eins*2,d0
		beq.s		S_facoth_inf	; =1-->INF
		bmi.s		S_facoth_errval	; <1-->NAN
		move.l		d2,d0		; Nenner berechnen
		move.l		#eins,d1
		bsr		S_fsub
		exg		d0,d2   	; Zaehler berechnen
		move.l		#eins,d1
		bsr		S_fadd
		move.l		d2,d1		; Quotient daraus
		bsr		S_fdiv
		bsr		S_fln		; logarithmieren
		move.w		#-1,d1		; halbieren
		bsr		S_fmul2
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts
S_facoth_inf:
		move.l		#$ff000000,d0	; vorzeichenbehaftete Unend-
		roxr.l		#1,d0		; lichkeit
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts
S_facoth_errval:
		move.l		#$7fc00000,d0	; NAN geben
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Kosinusfunktion: D0.S--> D0.S

		ds		0
S_fcos:
		movem.l		d1-d6,-(a7)	; Register retten
		bclr		#31,d0		; cos(-x)=cos(x)

		move.l		#pi2,d1		; auf Bereich 0..2*Pi reduzieren
S_fcos_subtr:
		cmp.l		d1,d0           ; x>=2*Pi ?
		blo.s		S_fcos_subend     ; ja-->Ende
		bchg		#31,d1		; fuer Subtraktion
		bsr		S_fadd		; reduzieren
		bchg		#31,d1		; Subtrahend wieder richtig
		bra.s		S_fcos_subtr
S_fcos_subend:
		cmp.l		#pi,d0		; x>Pi ?
		blo.s		S_fcos_nosub
		exg		d0,d1		; ja-->cos(x)=cos(2*Pi-x)
		bsr		S_fsub
S_fcos_nosub:
		move.l		d0,d1		; wir brauchen nur x^2
		bsr		S_fmul
		bset		#31,d0
		move.l		d0,d3           ; -x^2 in D3
		move.l		d0,d4		; D4 enthaelt laufende Potenz von x^2
						; inkl. Vorzeichen
		move.l		#zwei,d5	; D5 enthaelt laufende Fakultaet
		move.l		#eins,d2	; D2 enthaelt Summe
		moveq.l		#2,d6		; D6 enthaelt Zaehler
S_fcos_loop:
		move.l		d5,d1		; s:=s+yz/zaehler
		move.l		d4,d0
		bsr		S_fdiv
		move.l		d2,d1
		bsr		S_fadd
		cmp.l		d2,d0		; Veraendert sich Summe noch ?
		beq.s		S_fcos_end
		move.l		d0,d2
		addq.b		#2,d6		; i:=i+1
		cmp.b		#22,d6		; i=11 ?
		beq.s		S_fcos_end
		move.w		d6,d0		; Fakultaet erhhen: *(2n-1)*(2n)
		mulu.w		d6,d0		; =4*n^2-2*n
		sub.w		d6,d0
		bsr		S_fitof		; dazumultiplizieren
		move.l		d5,d1
		bsr		S_fmul
		move.l		d0,d5
		move.l		d4,d0		; yz:=yz*y
		move.l		d3,d1
		bsr		S_fmul
		move.l		d0,d4
		bra.s           S_fcos_loop
S_fcos_end:
						; Ergebnis bereits in D0
		movem.l         (a7)+,d1-d6	; Register zurueck
		rts

;----------------------------------------------------------------------------
; Sinus : D0.S-->D0.S

S_fsin:
		move.l		d1,-(a7)	; Register retten
		move.l		#pihalf,d1	; sin(x)=cos(x-pi/2)
		bsr		S_fsub
		bsr		S_fcos
		move.l		(a7)+,d1	; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Tangens: D0.S-->D0.S

S_ftan:
                movem.l         d1-d4,-(a7)     ; Register retten
		tst.l		d0		; Vorzeichen merken
                smi             d4
		bclr		#31,d0
		move.l		#pi,d1		; auf Bereich 0..Pi reduzieren
S_ftan_subtr:
		cmp.l		d1,d0           ; x>=Pi ?
		blo.s		S_ftan_subend     ; ja-->Ende
		bchg		#31,d1		; fuer Subtraktion
		bsr		S_fadd		; reduzieren
		bchg		#31,d1		; Subtrahend wieder richtig
		bra.s		S_ftan_subtr
S_ftan_subend:
		move.l		d0,d2		; Argument merken
		bsr		S_fcos		; Nenner rechnen
		move.l		d0,d3		; Nenner merken
		move.l		d0,d1		; sqr(1-x^2) rechnen
		bsr		S_fmul
		move.l		d0,d1
		move.l		#eins,d0
		bsr		S_fsub
		bsr		S_fsqrt
		move.l		d3,d1
		bsr		S_fdiv		; Quotient
                tst.b           d4              ; Vorzeichen dazu
		beq.s		S_ftan_noneg
		bchg		#31,d0
S_ftan_noneg:
                movem.l         (a7)+,d1-d4     ; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Kotangens: D0.S-->D0.S

S_fcot:
                movem.l         d1-d4,-(a7)     ; Register retten
		tst.l		d0		; Vorzeichen merken
                smi             d4
		bclr		#31,d0
		move.l		#pi,d1		; auf Bereich 0..Pi reduzieren
S_fcot_subtr:
		cmp.l		d1,d0           ; x>=Pi ?
		blo.s		S_fcot_subend     ; ja-->Ende
		bchg		#31,d1		; fuer Subtraktion
		bsr		S_fadd		; reduzieren
		bchg		#31,d1		; Subtrahend wieder richtig
		bra.s		S_fcot_subtr
S_fcot_subend:
		move.l		d0,d2		; Argument merken
		bsr		S_fcos		; Zaehler rechnen
		move.l		d0,d3		; Zaehler merken
		move.l		d0,d1		; sqr(1-x^2) rechnen
		bsr		S_fmul
		move.l		d0,d1
		move.l		#eins,d0
		bsr		S_fsub
		bsr		S_fsqrt
		move.l		d0,d1
		move.l		d3,d0
		bsr		S_fdiv		; Quotient
                tst.b           d4              ; Vorzeichen dazu
		beq.s		S_fcot_noneg
		bchg		#31,d0
S_fcot_noneg:
                movem.l         (a7)+,d1-d4     ; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Arcustangens: D0.S-->D0.S

S_fatan:
		movem.l		d1-d6,-(a7)	; Register sichern
                subq.l          #2,a7           ; Platz fuer Hilfsvariablen
		tst.l		d0		; Vorzeichen merken...
                smi             (a7)
		bclr		#31,d0		; ...und loeschen
		cmp.l		#eins,d0	; Argument>1 ?
                shi             1(a7)           ; ja :
		bls.s		S_fatan_no1	; nein :
		move.l		d0,d1		; ja : Kehrwert bilden
		move.l		#eins,d0
		bsr		S_fdiv
S_fatan_no1:
		move.l		#3,d2		; Zaehler initialisieren
		move.l		d0,d5		; Summe initialisieren
		move.l		d0,d4		; laufende Potenz = x^1
		move.l		d0,d1		; x^2 berechnen
		bsr		S_fmul
		move.l		d0,d3
		bset		#31,d3		; immer mit -x^2 multiplizieren
S_fatan_loop:
		move.l		d4,d0		; naechste Potenz ausrechnen
		move.l		d3,d1
		bsr		S_fmul
		move.l		d0,d4
		move.l		d2,d0		; Nenner in Gleitkomma
		bsr		S_fitof
		move.l		d0,d1		; Division ausfuehren
		move.l		d4,d0
		bsr		S_fdiv
		move.l		d5,d1		; zur Summe addieren
		bsr		S_fadd
		cmp.l		d0,d5		; noch signifikant ?
		beq.s		S_fatan_endloop	; nein-->Ende
		move.l		d0,d5
		addq.l		#2,d2		; Zaehler erhoehen
		cmp.l		#61,d2		; fertig ?
		bne.s		S_fatan_loop
S_fatan_endloop:
		move.l          d5,d0		; Ergebnis in D0 bringen
                tst.b           1(a7)           ; war Argument < 1 ?
		beq.s		S_fatan_not2
		bchg		#31,d0		; ja : Erg.=Pi/2-Erg
		move.l		#pihalf,d1
		bsr		S_fadd
S_fatan_not2:
                tst.b           (a7)            ; Vorzeichen dazu
		beq.s		S_fatan_not1
		bset		#31,d0
S_fatan_not1:
                addq.l          #2,a7           ; Hilfsvar. abraeumen
		movem.l		(a7)+,d1-d6	; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Arcuskotangens: D0.S-->D0.S

S_facot:
		move.l		d1,-(a7)	; Register sichern
		bsr		S_fatan		; acot(x)=pi/2-atan(x)
		bchg		#31,d0
		move.l		#pihalf,d1
		bsr		S_fadd
		move.l		(a7)+,d1	; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Arcussinus: D0.S --> D0.S

S_fasin:
		movem.l		d1-d2,-(a7)	; Register retten
		move.l		d0,d2		; Argument merken
		move.l		d0,d1		; Quadrat berechnen
		bsr		S_fmul
		bset		#31,d0		; 1-x^2 bilden
		move.l		#eins,d1
		bsr		S_fadd
		tst.l		d0		; Sonderfaelle abfangen
		bmi.s		S_fasin_errval	; <0 geht nicht
		beq.s		S_fasin_inf	; Endwerte
		bsr		S_fsqrt		; Wurzel ...
		move.l		d0,d1		; ... und Quotient
		move.l		d2,d0
		bsr             S_fdiv
		bsr		S_fatan		; zuletzt das wichtigste
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts
S_fasin_errval:
		move.l		#$7fc00000,d0	; NAN zurueckgeben
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts
S_fasin_inf:
		move.l		#pihalf,d0	; +- pi/2 zurueckgeben
		and.l		#$80000000,d2	; Vorzeichen dazu
		or.l		d2,d0
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts

;-----------------------------------------------------------------------------
; Arcuskosinus: D0.S --> D0.S

S_facos:
		tst.l		d0		; Argument=0 ?
		beq.s		S_facos_inf
		move.l		d0,d2		; Argument merken
		move.l		d0,d1		; Quadrat berechnen
		bsr		S_fmul
		bset		#31,d0		; 1-x^2 bilden
		move.l		#eins,d1
		bsr		S_fadd
		tst.l		d0		; Sonderfaelle abfangen
		bmi.s		S_facos_errval	; <0 geht nicht
		bsr		S_fsqrt		; Wurzel ...
		move.l		d2,d1		; ... und Quotient
		bsr             S_fdiv
		bsr		S_fatan		; zuletzt das wichtigste
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts
S_facos_errval:
		move.l		#$7fc00000,d0	; NAN zurueckgeben
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts
S_facos_inf:
		move.l		#pihalf,d0	; +- pi/2 zurueckgeben
		and.l		#$80000000,d2	; Vorzeichen dazu
		or.l		d2,d0
		movem.l		(a7)+,d1-d2	; Register zurueck
		rts

S_floatlibend:
