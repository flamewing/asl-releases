	cpu	68000
	page	0

mul function x,y,x*y

	; basic 68000 modes

	move.l	d3,d7
	move.l	a3,d7
	move.l	(a3),d7
	move.l	(a3)+,d7
	move.l	-(a3),d7
	move.l	mul(20,50)(a3),d7		; "68000 style"
	move.l	(mul(20,50),a3),d7		; "68020 style"
	move.l	mul(10,12)(a3,a4.w),d7		; "68000 style"
	move.l	(mul(10,12),a3,a4.w),d7	; "68020 style"
	move.l	mul(10,12)(a3,a4.l),d7		; "68000 style"
	move.l	(mul(10,12),a3,a4.l),d7	; "68020 style"
	move.l	mul(10,12)(a3,d4.w),d7		; "68000 style"
	move.l	(mul(10,12),a3,d4.w),d7	; "68020 style"
	move.l	mul(10,12)(a3,d4.l),d7		; "68000 style"
	move.l	(mul(10,12),a3,d4.l),d7	; "68020 style"
	move.l	mul(100,100),d7		; "68000 style"
	move.l	mul(100,100).l,d7		; "68000 style"
	move.l	mul(200,500),d7		; "68000 style"
	move.l	(mul(100,100)),d7		; "68020 style"
	move.l	(mul(100,100).l),d7		; "68020 style"
	move.l	(mul(200,500)),d7		; "68020 style"
	move.l	*(pc,a4.w),d7		; "68000 style"
	move.l	(*,pc,a4.w),d7		; "68020 style"
	move.l	*(pc,a4.l),d7		; "68000 style"
	move.l	(*,pc,a4.l),d7		; "68020 style"
	move.l	*(pc,d4.w),d7		; "68000 style"
	move.l	(*,pc,d4.w),d7		; "68020 style"
	move.l	*(pc,d4.l),d7		; "68000 style"
	move.l	(*,pc,d4.l),d7		; "68020 style"
	move.l	#$aa554711,d7

	; extended 68020+ modes

	cpu	68020

	; base displacement
	move.l	(mul(100,100),a3,d4.l*4),d7	; all components
	move.l	(mul(100,100).l,a3,d4.l*4),d7
	move.l	(mul(100,100),a3,d4.l*1),d7	; ->scale field zero
	move.l	(mul(100,100).l,a3,d4.l*1),d7
	move.l	(mul(100,100),a3,d4.w*1),d7	; ->word instead of longword index
	move.l	(mul(100,100).l,a3,d4.w*1),d7
	move.l	(mul(100,100),a3),d7		; no index
	move.l	(mul(100,100).l,a3),d7
	move.l	(mul(100,100),d4.w*1),d7	; no basereg
	move.l	(mul(100,100).l,d4.w*1),d7

	; no index (post/pre-indexed setting in I/IS irrelevant?)
	move.l	([mul(100,100)]),d7
	move.l	([mul(100,100).l]),d7
	move.l	([a3]),d7
	move.l	([a3,mul(100,100)]),d7
	move.l	([a3,mul(100,100).l]),d7
	move.l	([mul(100,100)],mul(100,200)),d7
	move.l	([mul(100,100).l],mul(100,200).l),d7
	move.l	([a3],mul(100,200)),d7
	move.l	([a3],mul(100,200).l),d7
	move.l	([mul(100,100),a3],mul(100,200)),d7
	move.l	([mul(100,100).l,a3],mul(100,200).l),d7

	; postindexed
	move.l	([mul(100,100)],d4.w*1),d7
	move.l	([mul(100,100).l],d4.l*4),d7
	move.l	([a3],d4.w*1),d7
	move.l	([a3],d4.l*4),d7
	move.l	([mul(100,100),a3],d4.w*1),d7
	move.l	([mul(100,100).l,a3],d4.l*4),d7
	move.l	([mul(100,100)],d4.w*1,mul(100,200)),d7
	move.l	([mul(100,100).l],d4.l*4,mul(100,200).l),d7
	move.l	([a3],d4.w*1,mul(100,200)),d7
	move.l	([a3],d4.l*4,mul(100,200).l),d7
	move.l	([mul(100,100),a3],d4.w*1,mul(100,200)),d7
	move.l	([mul(100,100).l,a3],d4.l*4,mul(100,200).l),d7

	; preindexed
	move.l  ([mul(100,100),d4.w*1]),d7
	move.l  ([mul(100,100).l,d4.l*4]),d7
	move.l  ([a3,d4.w*1]),d7
	move.l  ([a3,d4.l*4]),d7
	move.l	([mul(100,100),a3,d4.w*1]),d7
	move.l	([mul(100,100).l,a3,d4.l*4]),d7
	move.l	([mul(100,100),d4.w*1],mul(100,200)),d7
	move.l	([mul(100,100).l,d4.l*4],mul(100,200).l),d7
	move.l	([a3,d4.w*1],mul(100,200)),d7
	move.l	([a3,d4.l*4],mul(100,200).l),d7
	move.l	([mul(100,100),a3,d4.w*1],mul(100,200)),d7
	move.l	([mul(100,100).l,a3,d4.l*4],mul(100,200).l),d7

	; PC with base displacement
	move.l	(*,pc,d4.l*4),d7	; all components
	move.l	(*.l,pc,d4.l*4),d7
	move.l	(*,pc,d4.l*1),d7	; ->scale field zero
	move.l	(*.l,pc,d4.l*1),d7
	move.l	(*,pc,d4.w*1),d7	; ->word instead of longword index
	move.l	(*.l,pc,d4.w*1),d7
	move.l	(*,pc),d7		; no index
	move.l	(*.l,pc),d7

	; PC postindexed
	move.l	([pc],d4.w*1),d7
	move.l	([pc],d4.l*4),d7
	move.l	([*,pc],d4.w*1),d7
	move.l	([*.l,pc],d4.l*4),d7
	move.l	([pc],d4.w*1,mul(100,200)),d7
	move.l	([pc],d4.l*4,mul(100,200).l),d7
	move.l	([*,pc],d4.w*1,mul(100,200)),d7
	move.l	([*.l,pc],d4.l*4,mul(100,200).l),d7

	; PC preindexed
	move.l  ([pc,d4.w*1]),d7
	move.l  ([pc,d4.l*4]),d7
	move.l	([*,pc,d4.w*1]),d7
	move.l	([*.l,pc,d4.l*4]),d7
	move.l	([pc,d4.w*1],mul(100,200)),d7
	move.l	([pc,d4.l*4],mul(100,200).l),d7
	move.l	([*,pc,d4.w*1],mul(100,200)),d7
	move.l	([*.l,pc,d4.l*4],mul(100,200).l),d7
