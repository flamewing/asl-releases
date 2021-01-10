		cpu	eZ8
		page	0

		segment	data

		; first a simple structure
		; to have something to play with:

tPoint		struct
x		db	?
y		db	?
z		db	?
c		db	?
flags		dw	?
valid		defbit	flags,0
		endstruct

		; the simplest case: instantiate a single structure

Point		tPoint

		; nest structures in structures:

tPointPair	struct
p1		tPoint
p2		tPoint
		endstruct

PointPair	tPointPair

		; REPT can also be used if you are inside a structure
		; definition, so you can build arrays by hand - though
		; it's a bit tedious:

tPointManArray	struct
_IDX		set	0
		rept	5
_SIDX		set	"\{_IDX}"
POINTS_{_SIDX}	tPoint
_IDX		set	_IDX+1
		endm
		endstruct

PointManArray	tPointManArray

		; ...but it's a lot simpler with the new array option:

tPointArray	struct
Points		tPoint	[5]
		endstruct

PointArray	tPointArray

		; Instantiating arrays of structures may of course
		; also be done by hand, but it's just as tedious...

_IDX		set	0
		rept	10
_SIDX		set	"\{_IDX}"
POINTS_MAN_{_SIDX}	tPoint
_IDX		set	_IDX+1
		endm

		; ...and the new array option also allows multi-dimensional
                ; arrays:

PointVect	tPoint	[5]
PointMatrix	tPoint	[5],[4]
;PointSpace	tPoint	[5],[4],[12]

		; don't get greedy ;-)

		expect  2221
PointWhatever	tPoint	[5],[4],[12],[3]
		endexpect

		segment	code

		ld	r0,@Point_x

		ld	r0,@PointPair_p1_y

		ld	r0,@PointArray_Points_1_z

		ld	r0,@PointVect_2_z

		ld	r0,@PointMatrix_3_2_c

;		dw	PointSpace_4_3_10_flags
