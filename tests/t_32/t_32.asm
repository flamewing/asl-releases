        cpu     68340
        supmode	on
        include reg683xx.inc
        page    0

        lpstop	#$55aa

        link	a6,#10
        link.l	a6,#10

        bgnd

Test1   tbls.b  (a4),d5
Test2:  tbls.w  30(a6),d1
 Test3: tbls.l  20(a4,d5*1),d6
  Test4: tblsn.b (a4),d5
        tblsn.w 30(a6),d1
        tblsn.l	20(a4,d5*1),d6
        tblu.b	(a4),d5
        tblu.w	30(a6),d1
        tblu.l	20(a4,d5*1),d6
        tblun.b	(a4),d5
        tblun.w	30(a6),d1
        tblun.l	20(a4,d5*1),d6

        tbls.b	d1:d2,d3
        tbls.w	d2:d3,d4
        tbls.l	d3:d4,d5
        tblsn.b	d1:d2,d3
        tblsn.w	d2:d3,d4
        tblsn.l	d3:d4,d5
        tblu.b	d1:d2,d3
        tblu.w	d2:d3,d4
        tblu.l	d3:d4,d5
        tblun.b	d1:d2,d3
        tblun.w	d2:d3,d4
        tblun.l	d3:d4,d5

        move.l	(d0.l),d0
