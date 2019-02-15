	cpu	96c141
	page	0
	maxmode	on

	; ordinary case with dest=reg and src=mem:
	; operand & default autoinc/dec amount is given by register

	;  8 bit:

	LD	A,(XBC+)	; hex = C5 E4 21
	LD	A,(XBC+:1)
	LD	A,(XBC++1)
	LD	A,(XBC+:2)	; disassembler 1 : hex = C5 E5 21
	LD	A,(XBC++2)	; disassembler 2 : hex = C5 E5 21
	LD	A,(XBC+:4)	; hex = C5 E6 21
	LD	A,(XBC++4)	; hex = C5 E6 21

	LD	A,(-XBC)	; equal to 1--
	LD	A,(-XBC:1)
	LD	A,(1--XBC)
	LD	A,(-XBC:2)
	LD	A,(2--XBC)
	LD	A,(-XBC:4)
	LD	A,(4--XBC)

	; 16 bit:

	LD	WA,(XBC+)	; equal to ++2
	LD	WA,(XBC+:1)
	LD	WA,(XBC++1)
	LD	WA,(XBC+:2)
	LD	WA,(XBC++2)
	LD	WA,(XBC+:4)
	LD	WA,(XBC++4)

	LD	WA,(-XBC)	; equal to 2--
	LD	WA,(-XBC:1)
	LD	WA,(1--XBC)
	LD	WA,(-XBC:2)
	LD	WA,(2--XBC)
	LD	WA,(-XBC:4)
	LD	WA,(4--XBC)

	; 32 bit:

	LD	XWA,(XBC+)	; equal to ++4
	LD	XWA,(XBC+:1)
	LD	XWA,(XBC++1)
	LD	XWA,(XBC+:2)
	LD	XWA,(XBC++2)
	LD	XWA,(XBC+:4)
	LD	XWA,(XBC++4)

	LD	XWA,(-XBC)	; equal to 4--
	LD	XWA,(-XBC:1)
	LD	XWA,(1--XBC)
	LD	XWA,(-XBC:2)
	LD	XWA,(2--XBC)
	LD	XWA,(-XBC:4)
	LD	XWA,(4--XBC)

	; reverse case dest=mem and src=reg:
	; operand size & autoinc/dec default amount is given by src register
	; if inc/dec amount no specified explicitly, fixed up later after
        ; src register has been parsed

	;  8 bit:

	LD	(XBC+),A
	LD	(XBC+:1),A
	LD	(XBC++1),A
	LD	(XBC+:2),A
	LD	(XBC++2),A
	LD	(XBC+:4),A
	LD	(XBC++4),A

	LD	(-XBC),A
	LD	(-XBC:1),A
	LD	(1--XBC),A
	LD	(-XBC:2),A
	LD	(2--XBC),A
	LD	(-XBC:4),A
	LD	(4--XBC),A

	;  16 bit:

	LD	(XBC+),WA
	LD	(XBC+:1),WA
	LD	(XBC++1),WA
	LD	(XBC+:2),WA
	LD	(XBC++2),WA
	LD	(XBC+:4),WA
	LD	(XBC++4),WA

	LD	(-XBC),WA
	LD	(-XBC:1),WA
	LD	(1--XBC),WA
	LD	(-XBC:2),WA
	LD	(2--XBC),WA
	LD	(-XBC:4),WA
	LD	(4--XBC),WA

	;  32 bit:

	LD	(XBC+),XWA
	LD	(XBC+:1),XWA
	LD	(XBC++1),XWA
	LD	(XBC+:2),XWA
	LD	(XBC++2),XWA
	LD	(XBC+:4),XWA
	LD	(XBC++4),XWA

	LD	(-XBC),XWA
	LD	(-XBC:1),XWA
	LD	(1--XBC),XWA
	LD	(-XBC:2),XWA
	LD	(2--XBC),XWA
	LD	(-XBC:4),XWA
	LD	(4--XBC),XWA

	; LDA case: in principle, the "operand size" is unknown, since
        ; no actual data transfer takes place.  However, we assume 16/32 bits,
        ; deduced from the 16/32 bit destination register taking the effective address

	LDA	XWA,XSP
	LDA	XWA,(XSP)
	LDA	XIX,XIY+33h
	LDA	XIX,(XIY+33h)
	LDA	XIX,XIY+
	LDA	XIX,(XIY+)
	LDA	XIX,XIY+:1
	LDA	XIX,(XIY+:1)
	LDA	XIX,XIY++1
	LDA	XIX,(XIY++1)
	LDA	XIX,XIY+:2
	LDA	XIX,(XIY+:2)
	LDA	XIX,XIY++2
	LDA	XIX,(XIY++2)
	LDA	XIX,XIY+:4
	LDA	XIX,(XIY+:4)
	LDA	XIX,XIY++4
	LDA	XIX,(XIY++4)
	LDA	XIX,-XIY
	LDA	XIX,(-XIY)
	LDA	XIX,-XIY:1
	LDA	XIX,(-XIY:1)
	LDA	XIX,1--XIY
	LDA	XIX,(1--XIY)
	LDA	XIX,-XIY:2
	LDA	XIX,(-XIY:2)
	LDA	XIX,2--XIY
	LDA	XIX,(2--XIY)
	LDA	XIX,-XIY:4
	LDA	XIX,(-XIY:4)
	LDA	XIX,4--XIY
	LDA	XIX,(4--XIY)
