	cpu 68000
	page	0

TestTemporaryOverTemporary:
	bra.s	$$dest
$$over:
	nop
$$dest:

TestTemporaryOverComposite:
	bra.s	$$dest
.over:
	nop
$$dest:

TestTemporaryOverNameless:
	bra.s	$$dest
+
	nop
$$dest:

TestTemporaryOverLabel:
	if mompass>1
		expect 1010
	endif
	bra.s	$$dest
	if mompass>1
		endexpect
	endif
Over1:
	nop
$$dest:

TestCompositeOverTemporary:
	bra.s	.dest
$$over:
	nop
.dest:

TestCompositeOverComposite:
	bra.s	.dest
.over:
	nop
.dest:

TestCompositeOverNameless:
	bra.s	.dest
+
	nop
.dest:

TestCompositeOverLabel1:
	if mompass>1
		expect 1010
	endif
	bra.s	.dest
	if mompass>1
		endexpect
	endif
Over2:
	nop
.dest:

TestCompositeOverLabel2:
	bra.s	Over2.dest
Over3:
	nop
.dest:

TestNamelessOverTemporary:
	bra.s	+
$$over:
	nop
+

TestNamelessOverComposite:
	bra.s	+
.over:
	nop
+

TestNamelessOverNameless:
	bra.s	++
+
	nop
+

TestNamelessOverLabel:
	bra.s	+
Over4:
	nop
+

	END
