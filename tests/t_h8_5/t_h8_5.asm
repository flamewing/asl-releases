                cpu     hd6475348
                page    0
                maxmode on
                padding off

                assume  br:0,dp:0

                add.b   r5,sp
                add.w   r2,r4
                add.b   @r4,r2
                add.w   @r4,r2
                add.b   @(20,r4),r2
                add.w   @(20:8,r4),r2
                add.b   @(20:16,r4),r2
                add.w   @(2000,r4),r2
                add.b   @(2000:16,r4),r2
                add.w   @-r4,r2
                add.b   @r4+,r2
                add.w   @20,r2
                add.b   @20:16,r2
                add.w   @2000,r2
                add.b   #5,r2
                add.w   #100,r2
                add.b:g #1,r2		; forced to add:g
                add.w:g #-1,r2		; forced to add:g
                add.b   #1,r2		; optimized to add:q
                add.w   #-1,r2		; optimized to add:q

                adds.w  #$10,r3

                addx.b  @($20,r4),r0

                and.b   @$f8:8,r1

                andc.b  #$fe,ccr

		; :8 and :16 are synonyms for short and long branch

		irp	instr,bra,bt,brn,bf,bhi,bls,bcc,bhs,bcs,blo,bne,beq,bvc,bvs,bpl,bmi,bge,blt,bgt,ble
		instr	*+20
                instr	*+2000
		instr.l	*+20
		instr:16 *+20
		instr.s	*+20
		instr:8	*+20
		endm

                bclr.b  #7,@$ff00

                bnot.w  r0,r1

                bset.b  #0,@r1+

                bsr     *+20
                bsr     *+2000

                btst.b  r0,@$f0:8

		btst	#15,r4
		btst.w	#15,r4
		expect	1320
		btst.b	#15,r4
		endexpect

                clr.w   @($1000,r5)

                cmp:g.b #$aa,@-r3
                cmp.b   #$aa,@-r3
                cmp:e.b #$00,r0
                cmp.b   #$00,r0
                cmp:i.w #$ffff,r1
                cmp.w   #$ffff,r1

                dadd    r0,r1

                divxu.w @r3,r0

                dsub    r2,r3

                exts    r0

                extu    r1

                jmp     @(#$10,r4)

                jsr     @($fff,r3)

                ldc.b   #$01,dp

                ldm     @sp+,(r0,r2-r4)

                link    fp,#-4

		mov:g.b r2,r5
		mov.b   r2,r5
		mov:g.w r2,r5
		mov.w	r2,r5
                mov:g.w r0,@r1
                mov.w   r0,@r1
                mov:e.b #$55,r0
                mov.b   #$55,r0
                mov:f.b @(4,r6),r0
                mov.b   @(4,r6),r0
                mov:f.b r0,@(4,r6)
                mov.b   r0,@(4,r6)
                mov:i.w #$ff00,r5
                mov.w   #$ff00,r5
                mov:l.b @$a0,r0
                mov.b   @$a0,r0
                mov:s.w r0,@$a0:8
                mov.w   r0,@$a0:8

                movfpe  @$f000,r0

                movtpe  r0,@r1

                mulxu.b r0,r1

                neg.w   r0

                nop

                not.b   @($10,r2)

                or.b    @$f0:8,r1

                orc.w   #$0700,sr

                pjmp    @r4

                pjsr    @$010000

                prtd    #8

                prts

                rotl.w  r0

                rotr.b  @r1

                rotxl.w @($02,r1)

                rotxr.b @$fa:8

                rtd     #400

                rte

                rts

                scb/eq  r4,*-20

                shal.b  @r2+

                shar.w  @$ff00

                shll.b  r1

                shlr.w  @-r1

                sleep

                stc.b   br,@-sp

                stm     (r0-r3),@-sp

                sub.w   @r1,r0

                sub.b:g #1,r2		; forced to sub:g
                sub.w:g #-1,r2		; forced to sub:g
                sub.b:q #1,r2		; forced to sub:q, which becomes add:q #-1,...
                sub.w:q #-1,r2		; forced to sub:q, which becomes add:q #1,...
                sub.b   #1,r2		; assembled as sub:g, since there is officially no sub:q
                sub.w   #-1,r2		; assembled as sub:g, since there is officially no sub:q

                subs.w  #2,r2

                subx.w  @r2+,r0

                swap    r0

                tas     @$f000

                trapa   #4

                trap/vs

                tst     @($1000,r1)

                unlk    fp

                xch     r0,r1

                xor.b   @$a0:8,r0

                xorc.b  #$01,ccr

                bra     *+126
                bra     *+127
                bra     *+128
                bra     *+129
                bra     *+130
                bra     *+131
                bra     *+132
                bra     *+133
                bra     *+134
                bra     *-123
                bra     *-124
                bra     *-125
                bra     *-126
                bra     *-127
                bra     *-128
                bra     *-129
                bra     *-130
                bra     *-131

		; register aliases

reg_r0		equ	r0
reg_r1		equ	r1
reg_r2		equ	r2
reg_r3		equ	r3
reg_r4		equ	r4
reg_r5		equ	r5
reg_r6		equ	r6
reg_r7		equ	r7
reg_sp		reg	sp
reg_fp		reg	fp

                mov.b   #$55,r0
		mov.b	#$55,reg_r0
                mov.b   #$55,r1
		mov.b	#$55,reg_r1
                mov.b   #$55,r2
		mov.b	#$55,reg_r2
                mov.b   #$55,r3
		mov.b	#$55,reg_r3
                mov.b   #$55,r4
		mov.b	#$55,reg_r4
                mov.b   #$55,r5
		mov.b	#$55,reg_r5
                mov.b   #$55,r6
		mov.b	#$55,reg_r6
                mov.b   #$55,r7
		mov.b	#$55,reg_r7
		mov.b	#$55,sp
		mov.b	#$55,reg_sp
		mov.b	#$55,fp
		mov.b	#$55,reg_fp
		subx.w	@r0+,r0
		subx.w	@reg_r0+,reg_r0
		mov.w	r0,@-r1
		mov.w	reg_r0,@-reg_r1
		rotxl.w	@($02,r2)
		rotxl.w	@($02,reg_r2)

		; specifying operand size late

		mov.w	#$ff00,r5
		mov	#$ff00:16,r5

		; The very special hex syntax, which requires a quote qualify
                ; callback in the parser and which is only enabled on request.
		; We optionally also allow a terminating ':

		relaxed	on

		mov	#h'aa55:16 ,@h'ffe0  ; comment
		mov	#h'aa55:16 ,@h'ffe0' ; comment
		mov	#h'aa55':16,@h'ffe0  ; comment
		mov	#h'aa55':16,@h'ffe0' ; comment
		mov	#$aa55:16  ,@$ffe0   ; comment

		; For MOV (and not for CMP, though the coding suggests it),
                ; the immediate src's size
                ; may differ from the destination's size.  A sign
                ; extension is then executed and the object code is one
                ; byte shorter.  Earlier versions of AS always did
                ; this optimization, but there is code that explicitly
                ; requests it not being done, by adding a :16 attribute to
                ; the source operand:

		mov.w	#H'fee0,@H'1000
		expect	1320
		mov.w	#H'fee0:8,@H'1000
		endexpect
		mov.w	#H'fee0:16,@H'1000

		mov.w	#H'ffe0,@H'1000
		mov.w	#H'ffe0:8,@H'1000
		mov.w	#H'ffe0:16,@H'1000

		mov.w	#H'0130,@H'1000
		expect	1320
		mov.w	#H'0130:8,@H'1000
		endexpect
		mov.w	#H'0130:16,@H'1000

		mov.w	#H'0030,@H'1000
		mov.w	#H'0030:8,@H'1000
		mov.w	#H'0030:16,@H'1000

		mov.w	#0:16,@r0+
		mov.w	#0:8,@r0+
		mov.w	#0,@r0+

		cmp.w	#H'fee0,@H'1000
		expect	1131
		cmp.w	#H'fee0:8,@H'1000
		endexpect
		cmp.w	#H'fee0:16,@H'1000

		cmp.w	#H'ffe0,@H'1000
		expect	1131
		cmp.w	#H'ffe0:8,@H'1000
		endexpect
		cmp.w	#H'ffe0:16,@H'1000

		cmp.w	#H'0130,@H'1000
		expect	1131
		cmp.w	#H'0130:8,@H'1000
		endexpect
		cmp.w	#H'0130:16,@H'1000

		cmp.w	#H'0030,@H'1000
		expect	1131
		cmp.w	#H'0030:8,@H'1000
		endexpect
		cmp.w	#H'0030:16,@H'1000

		cmp.w	#0:16,@r0+
		expect	1131
		cmp.w	#0:8,@r0+
		endexpect
		cmp.w	#0,@r0+

		; used in more complex expressions

DID		equ	42

		CMP.B	#H'00+DID,R0
		CMP.B	#H'01+DID,R0
		CMP.B	#H'02+DID,R0
		CMP.B	#H'03+DID,R0

		ADD.W	#(H'80*H'20),R6
		CMP.W	#(H'80*H'20),R6
		CMP.W	#(H'7f80+(H'80*H'20)),R6
		SUB.W	#(H'7F*H'80),R6
		ADD.W	#(H'6F*H'80),R6

		; On H8/500, DATA is an alias for DC

		dc.b	10,20,30
		data.b	10,20,30
		dc.w	10,20,30
		data.w	10,20,30
		dc.s	1.0
		data.s	1.0

bitstruct	struct
word		ds.w	1
byte1		ds.b	1
byte2		ds.b	2
msb16		bit.w	#15,word
lsb8		bit	0,byte1
msb8		bit	#7,byte2
		endstruct

mystruct	bitstruct
		btst	mystruct_msb16
		btst	mystruct_msb8
		btst	mystruct_lsb8
