	cpu		z80

	enum		e1_v1=1,e1_v2
	nextenum	e1_v3,e1_v4

	enum		e2_v0,e2_v1,e2_v2

	db		e1_v1,e1_v2,e1_v3,e1_v4
	db		e2_v0,e2_v1,e2_v2

	enumconf	2,code
	enum		vec_reset,vec_int0,vec_int1,vec_int2
	nextenum	vec_pcint0,vec_pcint1

	dw		vec_reset,vec_int0,vec_int1,vec_int2,vec_pcint0,vec_pcint1
