	        cpu     6301

; first, we define a structure.  This one contains three fields, occupying 1, 2, and 12 bytes:

Record   	STRUCT
val8    	rmb 	1
val16		rmb 	2
val96		rmb 	12
	        ENDSTRUCT

; this macro allows to define an array of structures.  They will have the names
; <name>_0 to <name>_<cnt-1>.  Note however that the indices are written in hex!

NStruct		macro	name,cnt,{GLOBALSYMBOLS}
z		set	0
		rept	cnt,{GLOBALSYMBOLS}
z_str		set	"\{z}"
name_{z_str}	Record
z		set	z+1
		endm
		endm

	        org 	$1000

; define the structures Array_0 to Array_4

		NStruct	Array,5

; another way to define a list of structures

		irp	name,{GLOBALSYMBOLS},Rec1,Rec2,Rec3
name		Record
		endm

; defines Rec_a to Rec_c

		irpc	name,{GLOBALSYMBOLS},"abc"
Rec_name        Record
                endm

; an alternative way to define an array of structures, using the WHILE construct

z		set	1
		while	z<5,{GLOBALSYMBOLS}
z_str           set     "\{z}"
Array2_{z_str}  Record
z               set     z+1
                endm

; and now let's access the records, just to generate a bit of code...

		ldaa	Array_0_val8
		staa	Array_4_val8
		ldx	Rec1_val16
		stx	Rec2_val16
