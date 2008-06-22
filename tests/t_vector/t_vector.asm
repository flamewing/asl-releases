	cpu 	ATARI_VECTOR
	page	0

	org	$000

	jmpl	$001
	labs	(785, 284), s0
	vctr	(0, 0), s7, z0	; Ruhezeit
	vctr	(0, 0), s7, z15	; Schuss
	labs	(700, 359), s14
	vctr	(0, 0), s9, z0	; Ruhezeit
        vctr    (0, 0), s7, z15 ; Schuss
	labs	(641, 413), s15
        vctr	(0, 0), s8, z0	; Ruhezeit
	vctr	(0, 0), s7, z15	; Schuss
        labs    (572, 635), s14
        vctr    (0, 0), s9, z0  ; Ruhezeit
        vctr    (0, 0), s7, z15 ; Schuss
        labs    (294, 708), s15
	vctr	(0, 0), s8, z0	; Ruhezeit
	jsrl	$929		; UFO
	labs	(626, 427), s14
	vctr	(0, 0), s9, z0	; Ruhezeit
        vctr    (-256, 896), s4, z0
	vctr	(-640, -800), s4, z12 ; Schiff
	vctr	(-736, -64), s4, z12
	vctr	(760, -288), s6, z12
	vctr	(-432, 688), s6, z12
	vctr	(64, -736), s4, z12
	labs	(562, 168), s14	; Skalierung -2 (klein)
	vctr	(0, 0), s9, z0	; Ruhezeit
	jsrl	$8f3		; Asteroid (Typ 1)
	labs	(264, 789), s0	; Skalierung 0 (gross)
	vctr	(0, 0), s7, z0	; Ruhezeit
	jsrl	$91a		; Asteroid (Typ 4)
	labs	(319, 790), s14	; Skalierung -2 (klein)
	vctr	(0, 0), s9, z0	; Ruhezeit
	jsrl	$8ff		; Asteroid (Typ 2)
	labs	(17, 474), s14	; Skalierung -2 (klein)
	vctr	(0, 0), s9, z0	; Ruhezeit
	jsrl	$8f3		; Asteroid (Typ 1)
	labs	(819, 538), s15	; Skalierung -1 (mittel)
	vctr	(0, 0), s8, z0	; Ruhezeit
	jsrl	$91a		; Asteroid (Typ 4)
	labs	(477, 463), s14	; Skalierung -2 (klein)
	vctr	(0, 0), s9, z0	; Ruhezeit
	jsrl	$8f3		; Asteroid (Typ 1)
	labs	(413, 643), s14	; Skalierunf -2 (klein)
	vctr	(0, 0), s9, z0	; Ruhezeit
	jsrl	$91a		; Asteroid (Typ 4)
	labs	(306, 295), s15	; Skalierung -1 (mittel)
	vctr	(0, 0), s8, z0	; Ruhezeit
	jsrl	$8ff		; Asteroid (Typ 2)
	labs	(605, 748), s15	; Skalierung -1 (mittel)
	vctr	(0, 0), s8, z0	; Ruhezeit
	jsrl	$8ff		; Asteroid (Typ 2)
	jsrl	$852		; Copyright-Meldung
	labs	(100, 876), s1
	vctr	(0, 0), s7, z0	; Ruhezeit
	jsrl	$b2c		; Leerzeichen
	jsrl	$b2c		; Leerzeichen
	jsrl	$b3a		; 3
	jsrl	$b2e		; 1
	jsrl	$add		; 0
	labs	(160, 852), s14
	jsrl	$a6d		; Schiff
	jsrl	$a6d		; Schiff
	jsrl	$a6d		; Schiff
	labs	(480, 876), s0
	vctr	(0, 0), s5, z0	; Ruhezeit
	jsrl	$b2c		; Leerzeichen
	jsrl	$b2c		; Leerzeichen
	jsrl	$b2c		; Leerzeichen
	jsrl	$add		; 0
	jsrl	$add		; 0
	labs	(768, 876), s1
	vctr	(0, 0), s5, z0	; Ruhezeit
	labs	(508, 508), s1
	halt

	