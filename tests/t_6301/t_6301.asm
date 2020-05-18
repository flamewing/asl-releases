	cpu	6301
	page	0

	; BCLR/BSET/BTGL/BTST are aliases for AIM/OIM/EIM/TIM

	aim	#$fe,$12
	bclr	0,$12
	bclr	#0,$12

	aim	#$fd,34,x
	bclr	1,$34,x
	bclr	#1,$34,x

	oim	#$04,$12
	bset	2,$12
	bset	#2,$12

	oim	#$08,$34,x
	bset	3,$34,x
	bset	#3,$34,x

	eim	#$10,$12
	btgl	4,$12
	btgl	#4,$12

	eim	#$20,$34,x
	btgl	5,$34,x
	btgl	#5,$34,x

	tim	#$40,$12
	btst	6,$12
	btst	#6,$12

	tim	#$80,$34,x
	btst	7,$34,x
	btst	#7,$34,x

	; different opcode than on 68HC11

	xgdx

	slp
