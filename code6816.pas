{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
	Unit Code6816;

{ AS-Codegeneratormodul 68HC16 }

INTERFACE

        Uses NLS,
             AsmDef,AsmPars,AsmSub,CodePseu;


Implementation

TYPE
   FixedOrder=RECORD
               Name:String[6];
               Code:Word;
              END;

   GenOrder=RECORD
             Name:String[5];
             Size:ShortInt;
             Code,ExtCode:Word;
             AdrMask,ExtShift:Byte;
            END;

   EmuOrder=RECORD
             Name:String[3];
             Code1,Code2:Word;
            END;

CONST
   FixedOrderCnt=140;
   RelOrderCnt=18;
   LRelOrderCnt=3;
   GenOrderCnt=66;
   AuxOrderCnt=12;
   ImmOrderCnt=4;
   ExtOrderCnt=3;
   EmuOrderCnt=6;
   RegCnt=7;

   ModNone=-1;
   ModDisp8=0;    MModDisp8=1 SHL ModDisp8;
   ModDisp16=1;   MModDisp16=1 SHL ModDisp16;
   ModDispE=2;    MModDispE=1 SHL ModDispE;
   ModAbs=3;      MModAbs=1 SHL ModAbs;
   ModImm=4;      MModImm=1 SHL ModImm;
   ModImmExt=5;   MModImmExt=1 SHL ModImmExt;
   ModDisp20=ModDisp16; MModDisp20=MModDisp16;
   ModAbs20=ModAbs; MModAbs20=MModAbs;

TYPE
   FixedOrderArray=ARRAY[1..FixedOrderCnt] OF FixedOrder;
   RelOrderArray=ARRAY[1..RelOrderCnt] OF FixedOrder;
   LRelOrderArray=ARRAY[1..LRelOrderCnt] OF FixedOrder;
   GenOrderArray=ARRAY[1..GenOrderCnt] OF GenOrder;
   AuxOrderArray=ARRAY[1..AuxOrderCnt] OF FixedOrder;
   ImmOrderArray=ARRAY[1..ImmOrderCnt] OF FixedOrder;
   ExtOrderArray=ARRAY[1..ExtOrderCnt] OF FixedOrder;
   EmuOrderArray=ARRAY[1..EmuOrderCnt] OF EmuOrder;
   RegArray=ARRAY[0..RegCnt-1] OF String[3];

VAR
   OpSize:ShortInt;
   AdrMode:ShortInt;
   AdrPart:Byte;
   AdrCnt:Byte;
   AdrVals:ARRAY[0..3] OF Byte;

   Reg_EK:LongInt;
   SaveInitProc:PROCEDURE;

   FixedOrders:^FixedOrderArray;
   RelOrders:^RelOrderArray;
   LRelOrders:^LRelOrderArray;
   GenOrders:^GenOrderArray;
   AuxOrders:^AuxOrderArray;
   ImmOrders:^ImmOrderArray;
   ExtOrders:^ExtOrderArray;
   EmuOrders:^EmuOrderArray;
   Regs:^RegArray;

   CPU6816:CPUVar;

{---------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   IF z=FixedOrderCnt THEN Halt(0);
   Inc(z);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddRel(NName:String; NCode:Word);
BEGIN
   IF z=RelOrderCnt THEN Halt(0);
   Inc(z);
   WITH RelOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddLRel(NName:String; NCode:Word);
BEGIN
   IF z=LRelOrderCnt THEN Halt(0);
   Inc(z);
   WITH LRelOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddGen(NName:String; NSize:ShortInt; NCode,NExtCode:Word; NShift,NMask:Byte);
BEGIN
   IF z=GenOrderCnt THEN Halt(0);
   Inc(z);
   WITH GenOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; ExtCode:=NExtCode;
     Size:=NSize; AdrMask:=NMask; ExtShift:=NShift;
    END;
END;

        PROCEDURE AddAux(NName:String; NCode:Word);
BEGIN
   IF z=AuxOrderCnt THEN Halt(0);
   Inc(z);
   WITH AuxOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddImm(NName:String; NCode:Word);
BEGIN
   IF z=ImmOrderCnt THEN Halt(0);
   Inc(z);
   WITH ImmOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddExt(NName:String; NCode:Word);
BEGIN
   IF z=ExtOrderCnt THEN Halt(0);
   Inc(z);
   WITH ExtOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddEmu(NName:String; NCode1,NCode2:Word);
BEGIN
   IF z=EmuOrderCnt THEN Halt(0);
   Inc(z);
   WITH EmuOrders^[z] DO
    BEGIN
     Name:=NName; Code1:=NCode1; Code2:=NCode2;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('ABA'   ,$370b); AddFixed('ABX'   ,$374f);
   AddFixed('ABY'   ,$375f); AddFixed('ABZ'   ,$376f);
   AddFixed('ACE'   ,$3722); AddFixed('ACED'  ,$3723);
   AddFixed('ADE'   ,$2778); AddFixed('ADX'   ,$37cd);
   AddFixed('ADY'   ,$37dd); AddFixed('ADZ'   ,$37ed);
   AddFixed('AEX'   ,$374d); AddFixed('AEY'   ,$375d);
   AddFixed('AEZ'   ,$376d); AddFixed('ASLA'  ,$3704);
   AddFixed('ASLB'  ,$3714); AddFixed('ASLD'  ,$27f4);
   AddFixed('ASLE'  ,$2774); AddFixed('ASLM'  ,$27b6);
   AddFixed('LSLB'  ,$3714); AddFixed('LSLD'  ,$27f4);
   AddFixed('LSLE'  ,$2774); AddFixed('LSLA'  ,$3704);
   AddFixed('ASRA'  ,$370d); AddFixed('ASRB'  ,$371d);
   AddFixed('ASRD'  ,$27fd); AddFixed('ASRE'  ,$277d);
   AddFixed('ASRM'  ,$27ba); AddFixed('BGND'  ,$37a6);
   AddFixed('CBA'   ,$371b); AddFixed('CLRA'  ,$3705);
   AddFixed('CLRB'  ,$3715); AddFixed('CLRD'  ,$27f5);
   AddFixed('CLRE'  ,$2775); AddFixed('CLRM'  ,$27b7);
   AddFixed('COMA'  ,$3700); AddFixed('COMB'  ,$3710);
   AddFixed('COMD'  ,$27f0); AddFixed('COME'  ,$2770);
   AddFixed('DAA'   ,$3721); AddFixed('DECA'  ,$3701);
   AddFixed('DECB'  ,$3711); AddFixed('EDIV'  ,$3728);
   AddFixed('EDIVS' ,$3729); AddFixed('EMUL'  ,$3725);
   AddFixed('EMULS' ,$3726); AddFixed('FDIV'  ,$372b);
   AddFixed('FMULS' ,$3727); AddFixed('IDIV'  ,$372a);
   AddFixed('INCA'  ,$3703); AddFixed('INCB'  ,$3713);
   AddFixed('LPSTOP',$27f1); AddFixed('LSRA'  ,$370f);
   AddFixed('LSRB'  ,$371f); AddFixed('LSRD'  ,$27ff);
   AddFixed('LSRE'  ,$277f); AddFixed('MUL'   ,$3724);
   AddFixed('NEGA'  ,$3702); AddFixed('NEGB'  ,$3712);
   AddFixed('NEGD'  ,$27f2); AddFixed('NEGE'  ,$2772);
   AddFixed('NOP'   ,$274c); AddFixed('PSHA'  ,$3708);
   AddFixed('PSHB'  ,$3718); AddFixed('PSHMAC',$27b8);
   AddFixed('PULA'  ,$3709); AddFixed('PULB'  ,$3719);
   AddFixed('PULMAC',$27b9); AddFixed('ROLA'  ,$370c);
   AddFixed('ROLB'  ,$371c); AddFixed('ROLD'  ,$27fc);
   AddFixed('ROLE'  ,$277c); AddFixed('RORA'  ,$370e);
   AddFixed('RORB'  ,$371e); AddFixed('RORD'  ,$27fe);
   AddFixed('RORE'  ,$277e); AddFixed('RTI'   ,$2777);
   AddFixed('RTS'   ,$27f7); AddFixed('SBA'   ,$370a);
   AddFixed('SDE'   ,$2779); AddFixed('SWI'   ,$3720);
   AddFixed('SXT'   ,$27f8); AddFixed('TAB'   ,$3717);
   AddFixed('TAP'   ,$37fd); AddFixed('TBA'   ,$3707);
   AddFixed('TBEK'  ,$27fa); AddFixed('TBSK'  ,$379f);
   AddFixed('TBXK'  ,$379c); AddFixed('TBYK'  ,$379d);
   AddFixed('TBZK'  ,$379e); AddFixed('TDE'   ,$277b);
   AddFixed('TDMSK' ,$372f); AddFixed('TDP'   ,$372d);
   AddFixed('TED'   ,$27fb); AddFixed('TEDM'  ,$27b1);
   AddFixed('TEKB'  ,$27bb); AddFixed('TEM'   ,$27b2);
   AddFixed('TMER'  ,$27b4); AddFixed('TMET'  ,$27b5);
   AddFixed('TMXED' ,$27b3); AddFixed('TPA'   ,$37fc);
   AddFixed('TPD'   ,$372c); AddFixed('TSKB'  ,$37af);
   AddFixed('TSTA'  ,$3706); AddFixed('TSTB'  ,$3716);
   AddFixed('TSTD'  ,$27f6); AddFixed('TSTE'  ,$2776);
   AddFixed('TSX'   ,$274f); AddFixed('TSY'   ,$275f);
   AddFixed('TSZ'   ,$276f); AddFixed('TXKB'  ,$37ac);
   AddFixed('TXS'   ,$374e); AddFixed('TXY'   ,$275c);
   AddFixed('TXZ'   ,$276c); AddFixed('TYKB'  ,$37ad);
   AddFixed('TYS'   ,$375e); AddFixed('TYX'   ,$274d);
   AddFixed('TYZ'   ,$276d); AddFixed('TZKB'  ,$37ae);
   AddFixed('TZS'   ,$376e); AddFixed('TZX'   ,$274e);
   AddFixed('TZY'   ,$275e); AddFixed('WAI'   ,$27f3);
   AddFixed('XGAB'  ,$371a); AddFixed('XGDE'  ,$277a);
   AddFixed('XGDX'  ,$37cc); AddFixed('XGDY'  ,$37dc);
   AddFixed('XGDZ'  ,$37ec); AddFixed('XGEX'  ,$374c);
   AddFixed('XGEY'  ,$375c); AddFixed('XGEZ'  ,$376c);
   AddFixed('DES'   ,$3fff); AddFixed('INS'   ,$3f01);
   AddFixed('DEX'   ,$3cff); AddFixed('INX'   ,$3c01);
   AddFixed('DEY'   ,$3dff); AddFixed('INY'   ,$3d01);
   AddFixed('PSHX'  ,$3404); AddFixed('PULX'  ,$3510);
   AddFixed('PSHY'  ,$3408); AddFixed('PULY'  ,$3508);

   New(RelOrders); z:=0;
   AddRel('BCC', 4); AddRel('BCS', 5); AddRel('BEQ', 7);
   AddRel('BGE',12); AddRel('BGT',14); AddRel('BHI', 2);
   AddRel('BLE',15); AddRel('BLS', 3); AddRel('BLT',13);
   AddRel('BMI',11); AddRel('BNE', 6); AddRel('BPL',10);
   AddRel('BRA', 0); AddRel('BRN', 1); AddRel('BVC', 8);
   AddRel('BVS', 9); AddRel('BHS', 4); AddRel('BLO', 5);

   New(LRelOrders); z:=0;
   AddLRel('LBEV',$3791); AddLRel('LBMV',$3790); AddLRel('LBSR',$27f9);

   New(GenOrders); z:=0;
   AddGen('ADCA',0,$43,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ADCB',0,$c3,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ADCD',1,$83,$ffff,$20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ADCE',1,$03,$ffff,$20,          MModImm+           MModDisp16+MModAbs          );
   AddGen('ADDA',0,$41,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ADDB',0,$c1,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ADDD',1,$81,  $fc,$20,MModDisp8+MModImm+MModImmExt+MModDisp16+MModAbs+MModDispE);
   AddGen('ADDE',1,$01,  $7c,$20,          MModImm+MModImmExt+MModDisp16+MModAbs          );
   AddGen('ANDA',0,$46,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ANDB',0,$c6,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ANDD',1,$86,$ffff,$20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ANDE',1,$06,$ffff,$20,          MModImm+           MModDisp16+MModAbs          );
   AddGen('ASL' ,0,$04,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('ASLW',0,$04,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('LSL' ,0,$04,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('LSLW',0,$04,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('ASR' ,0,$0d,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('ASRW',0,$0d,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('BITA',0,$49,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('BITB',0,$c9,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('CLR' ,0,$05,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('CLRW',0,$05,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('CMPA',0,$48,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('CMPB',0,$c8,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('COM' ,0,$00,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('COMW',0,$00,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('CPD' ,1,$88,$ffff,$20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('CPE' ,1,$08,$ffff,$20,          MModImm+           MModDisp16+MModAbs          );
   AddGen('DEC' ,0,$01,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('DECW',0,$01,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('EORA',0,$44,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('EORB',0,$c4,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('EORD',1,$84,$ffff,$20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('EORE',1,$04,$ffff,$20,          MModImm+           MModDisp16+MModAbs          );
   AddGen('INC' ,0,$03,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('INCW',0,$03,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('LDAA',0,$45,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('LDAB',0,$c5,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('LDD' ,1,$85,$ffff,$20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('LDE' ,1,$05,$ffff,$20,          MModImm+           MModDisp16+MModAbs          );
   AddGen('LSR' ,0,$0f,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('LSRW',0,$0f,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('NEG' ,0,$02,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('NEGW',0,$02,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('ORAA',0,$47,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ORAB',0,$c7,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ORD' ,1,$87,$ffff,$20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('ORE' ,1,$07,$ffff,$20,          MModImm+           MModDisp16+MModAbs          );
   AddGen('ROL' ,0,$0c,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('ROLW',0,$0c,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('ROR' ,0,$0e,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('RORW',0,$0e,$ffff,$10,                             MModDisp16+MModAbs          );
   AddGen('SBCA',0,$42,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('SBCB',0,$c2,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('SBCD',1,$82,$ffff,$20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('SBCE',1,$02,$ffff,$20,          MModImm+           MModDisp16+MModAbs          );
   AddGen('STAA',0,$4a,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs+MModDispE);
   AddGen('STAB',0,$ca,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs+MModDispE);
   AddGen('STD' ,1,$8a,$ffff,$20,MModDisp8+                   MModDisp16+MModAbs+MModDispE);
   AddGen('STE' ,1,$0a,$ffff,$20,                             MModDisp16+MModAbs          );
   AddGen('SUBA',0,$40,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('SUBB',0,$c0,$ffff,$00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('SUBD',1,$80,$ffff,$20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen('SUBE',1,$00,$ffff,$20,          MModImm+           MModDisp16+MModAbs          );
   AddGen('TST' ,0,$06,$ffff,$00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen('TSTW',0,$06,$ffff,$10,                             MModDisp16+MModAbs          );

   New(AuxOrders); z:=0;
   AddAux('CPS',$4f); AddAux('CPX',$4c); AddAux('CPY',$4d); AddAux('CPZ',$4e);
   AddAux('LDS',$cf); AddAux('LDX',$cc); AddAux('LDY',$cd); AddAux('LDZ',$ce);
   AddAux('STS',$8f); AddAux('STX',$8c); AddAux('STY',$8d); AddAux('STZ',$8e);

   New(ImmOrders); z:=0;
   AddImm('AIS',$3f); AddImm('AIX',$3c); AddImm('AIY',$3d); AddImm('AIZ',$3e);

   New(ExtOrders); z:=0;
   AddExt('LDED',$2771); AddExt('LDHI',$27b0);AddExt('STED',$2773);

   New(EmuOrders); z:=0;
   AddEmu('CLC',$373a,$feff); AddEmu('CLI',$373a,$ff1f); AddEmu('CLV',$373a,$fdff);
   AddEmu('SEC',$373b,$0100); AddEmu('SEI',$373b,$00e0); AddEmu('SEV',$373b,$0200);

   New(Regs);
   Regs^[0]:='D'; Regs^[1]:='E'; Regs^[2]:='X'; Regs^[3]:='Y';
   Regs^[4]:='Z'; Regs^[5]:='K'; Regs^[6]:='CCR';
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(RelOrders);
   Dispose(LRelOrders);
   Dispose(GenOrders);
   Dispose(AuxOrders);
   Dispose(ImmOrders);
   Dispose(ExtOrders);
   Dispose(EmuOrders);
   Dispose(Regs);
END;

{---------------------------------------------------------------------------}

	PROCEDURE DecodeAdr(Start,Stop:Integer; LongAdr:Boolean; Mask:Byte);
LABEL
   Found;
TYPE
   DispType=(ShortDisp,LongDisp,NoDisp);
VAR
   V16:Integer;
   V32:LongInt;
   OK:Boolean;
   s:String;
   Size:DispType;

   	PROCEDURE SplitSize(VAR Asc:String; VAR Erg:DispType);
BEGIN
   IF Length(Asc)<1 THEN Erg:=NoDisp
   ELSE IF Asc[1]='>' THEN Erg:=LongDisp
   ELSE IF Asc[1]='<' THEN Erg:=ShortDisp
   ELSE Erg:=NoDisp;
END;

BEGIN
   AdrMode:=ModNone; AdrCnt:=0;

   Stop:=Stop-Start+1;
   IF Stop<1 THEN
    BEGIN
     WrError(1110); Exit;
    END;

   { immediate ? }

   IF (Length(ArgStr[Start])>0) AND (ArgStr[Start][1]='#') THEN
    BEGIN
     s:=Copy(ArgStr[Start],2,Length(ArgStr[Start])-1); SplitSize(s,Size);
     CASE OpSize OF
     -1:WrError(1132);
     0:BEGIN
        AdrVals[0]:=EvalIntExpression(s,Int8,OK);
        IF OK THEN
         BEGIN
          AdrCnt:=1; AdrMode:=ModImm;
         END;
       END;
     1:BEGIN
        IF Size=ShortDisp THEN
	 V16:=EvalIntExpression(s,SInt8,OK)
	ELSE
	 V16:=EvalIntExpression(s,Int16,OK);
        IF (Size=NoDisp) AND (V16>=-128) AND (V16<=127) AND (Mask AND MModImmExt<>0) THEN
         Size:=ShortDisp;
        IF OK THEN
         IF Size=ShortDisp THEN
          BEGIN
           AdrVals[0]:=Lo(V16);
           AdrCnt:=1; AdrMode:=ModImmExt;
          END
         ELSE
          BEGIN
           AdrVals[0]:=Hi(V16);
           AdrVals[1]:=Lo(V16);
           AdrCnt:=2; AdrMode:=ModImm;
          END;
       END;
     2:BEGIN
        V32:=EvalIntExpression(s,Int32,OK);
        IF OK THEN
         BEGIN
          AdrVals[0]:=(V32 SHR 24) AND $ff;
          AdrVals[1]:=(V32 SHR 16) AND $ff;
          AdrVals[2]:=(V32 SHR 8) AND $ff;
          AdrVals[3]:=V32 AND $ff;
          AdrCnt:=4; AdrMode:=ModImm;
         END;
       END;
     END;
     Goto Found;
    END;

   { zusammengesetzt ? }

   IF Stop=2 THEN
    BEGIN
     AdrPart:=$ff;
     IF NLS_StrCaseCmp(ArgStr[Start+1],'X')=0 THEN AdrPart:=$00
     ELSE IF NLS_StrCaseCmp(ArgStr[Start+1],'Y')=0 THEN AdrPart:=$10
     ELSE IF NLS_StrCaseCmp(ArgStr[Start+1],'Z')=0 THEN AdrPart:=$20
     ELSE WrXError(1445,ArgStr[Start+1]);
     IF AdrPart<>$ff THEN
      IF NLS_StrCaseCmp(ArgStr[Start],'E')=0 THEN AdrMode:=ModDispE
      ELSE
       BEGIN
        SplitSize(ArgStr[Start],Size);
        IF Size=ShortDisp THEN
	 V32:=EvalIntExpression(ArgStr[Start],UInt8,OK)
        ELSE IF LongAdr THEN
         V32:=EvalIntExpression(ArgStr[Start],SInt20,OK)
        ELSE
         V32:=EvalIntExpression(ArgStr[Start],SInt16,OK);
        IF OK THEN
         BEGIN
          IF Size=NoDisp THEN
           IF (V32>=0) AND (V32<=255) AND (Mask AND MModDisp8<>0) THEN Size:=ShortDisp;
          IF Size=ShortDisp THEN
           BEGIN
            AdrVals[0]:=V32 AND $ff;
	    AdrCnt:=1; AdrMode:=ModDisp8;
           END
          ELSE IF LongAdr THEN
	   BEGIN
            AdrVals[0]:=(V32 SHR 16) AND $0f;
            AdrVals[1]:=(V32 SHR 8) AND $ff;
            AdrVals[2]:=V32 AND $ff;
            AdrCnt:=3; AdrMode:=ModDisp16;
           END
          ELSE
           BEGIN
            AdrVals[0]:=(V32 SHR 8) AND $ff;
            AdrVals[1]:=V32 AND $ff;
            AdrCnt:=2; AdrMode:=ModDisp16;
           END;
         END;
       END;
     Goto Found;
    END

   { absolut ? }

   ELSE
    BEGIN
     SplitSize(ArgStr[Start],Size);
     V32:=EvalIntExpression(ArgStr[Start],UInt20,OK);
     IF OK THEN
      IF LongAdr THEN
       BEGIN
        AdrVals[0]:=(V32 SHR 16) AND $ff;
        AdrVals[1]:=(V32 SHR 8) AND $ff;
        AdrVals[2]:=V32 AND $ff;
        AdrMode:=ModAbs; AdrCnt:=3;
       END
      ELSE
       BEGIN
        IF V32 SHR 16<>Reg_EK THEN WrError(110);
        AdrVals[0]:=(V32 SHR 8) AND $ff;
        AdrVals[1]:=V32 AND $ff;
        AdrMode:=ModAbs; AdrCnt:=2;
       END;
     Goto Found;
    END;

Found:
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     WrError(1350);
     AdrMode:=ModNone; AdrCnt:=0;
    END;
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUME6816Count=1;
   ASSUME6816s:ARRAY[1..ASSUME6816Count] OF ASSUMERec=
	       ((Name:'EK' ; Dest:@Reg_EK ; Min:0; Max:  $ff; NothingVal:  $100));
BEGIN
   DecodePseudo:=True;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME6816s,ASSUME6816Count);
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_6816;
	Far;
VAR
   z,z2:Integer;
   OK:Boolean;
   AdrByte,Mask:Byte;
   AdrLong:LongInt;
BEGIN
   CodeLen:=0; DontPrint:=False; AdrCnt:=0; OpSize:=-1;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeMotoPseudo(True) THEN Exit;

   { Anweisungen ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
        BEGIN
         BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
         CodeLen:=2;
        END;
       Exit
      END;

   FOR z:=1 TO EmuOrderCnt DO
    WITH EmuOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
        BEGIN
         BAsmCode[0]:=Hi(Code1); BAsmCode[1]:=Lo(Code1);
         BAsmCode[2]:=Hi(Code2); BAsmCode[3]:=Lo(Code2);
         CodeLen:=4;
        END;
       Exit
      END;

   { Datentransfer }

   FOR z:=1 TO AuxOrderCnt DO
    WITH AuxOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE
        BEGIN
         OpSize:=1;
         IF Name[1]='S' THEN
	  DecodeAdr(1,ArgCnt,False,MModDisp8+MModDisp16+MModAbs)
         ELSE
	  DecodeAdr(1,ArgCnt,False,MModImm+MModDisp8+MModDisp16+MModAbs);
         CASE AdrMode OF
         ModDisp8:
          BEGIN
           BAsmCode[0]:=Code+AdrPart; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
          END;
         ModDisp16:
          BEGIN
           BAsmCode[0]:=$17;
	   BAsmCode[1]:=Code+AdrPart;
	   Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
         ModAbs:
          BEGIN
           BAsmCode[0]:=$17;
	   BAsmCode[1]:=Code+$30;
	   Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
         ModImm:
	  BEGIN
           BAsmCode[0]:=$37; BAsmCode[1]:=Code+$30;
           IF Name[1]='L' THEN Dec(BAsmCode[1],$40);
	   Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
	  END;
         END;
        END;
       Exit;
      END;

   FOR z:=1 TO ExtOrderCnt DO
    WITH ExtOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         OpSize:=1; DecodeAdr(1,1,False,MModAbs);
         CASE AdrMode OF
         ModAbs:
          BEGIN
           BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
           Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
         END;
        END;
       Exit;
      END;

   IF (Memo('PSHM')) OR (Memo('PULM')) THEN
    BEGIN
     IF ArgCnt<1 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True; Mask:=0;
       FOR z:=1 TO ArgCnt DO
        IF OK THEN
         BEGIN
          z2:=0; NLS_UpString(ArgStr[z]);
	  WHILE (z2<RegCnt) AND (ArgStr[z]<>Regs^[z2]) DO Inc(z2);
          IF z2>=RegCnt THEN
           BEGIN
            WrXError(1445,ArgStr[z]); OK:=False;
           END
          ELSE IF Memo('PSHM') THEN Inc(Mask,1 SHL z2)
          ELSE Inc(Mask,1 SHL (RegCnt-1-z2));
         END;
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$34+Ord(Memo('PULM')); BAsmCode[1]:=Mask;
         CodeLen:=2;
        END;
      END;
     Exit;
    END;

   IF (Memo('MOVB')) OR (Memo('MOVW')) THEN
    BEGIN
     z:=Ord(Memo('MOVW'));
     IF ArgCnt=2 THEN
      BEGIN
       DecodeAdr(1,1,False,MModAbs);
       IF AdrMode=ModAbs THEN
        BEGIN
         Move(AdrVals,BAsmCode[2],2);
         DecodeAdr(2,2,False,MModAbs);
         IF AdrMode=ModAbs THEN
          BEGIN
           Move(AdrVals,BAsmCode[4],2);
           BAsmCode[0]:=$37; BAsmCode[1]:=$fe+z;
           CodeLen:=6;
          END;
        END;
      END
     ELSE IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'X')=0 THEN
      BEGIN
       BAsmCode[1]:=EvalIntExpression(ArgStr[1],SInt8,OK);
       IF OK THEN
        BEGIN
         DecodeAdr(3,3,False,MModAbs);
         IF AdrMode=ModAbs THEN
          BEGIN
           Move(AdrVals,BAsmCode[2],2);
           BAsmCode[0]:=$30+z;
           CodeLen:=4;
          END;
        END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[3],'X')=0 THEN
      BEGIN
       BAsmCode[3]:=EvalIntExpression(ArgStr[2],SInt8,OK);
       IF OK THEN
        BEGIN
         DecodeAdr(1,1,False,MModAbs);
         IF AdrMode=ModAbs THEN
          BEGIN
           Move(AdrVals,BAsmCode[1],2);
           BAsmCode[0]:=$32+z;
           CodeLen:=4;
          END;
        END;
      END
     ELSE WrError(1350);
     Exit;
    END;

   { Arithmetik }

   FOR z:=1 TO GenOrderCnt DO
    WITH GenOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE
        BEGIN
         OpSize:=Size;
         DecodeAdr(1,ArgCnt,False,AdrMask);
         CASE AdrMode OF
         ModDisp8:
          BEGIN
           BAsmCode[0]:=Code+AdrPart; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
          END;
         ModDisp16:
          BEGIN
           BAsmCode[0]:=$17+ExtShift;
	   BAsmCode[1]:=Code+(OpSize SHL 6)+AdrPart;
	   Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
         ModDispE:
          BEGIN
           BAsmCode[0]:=$27; BAsmCode[1]:=Code+AdrPart;
	   CodeLen:=2;
          END;
         ModAbs:
          BEGIN
           BAsmCode[0]:=$17+ExtShift;
	   BAsmCode[1]:=Code+(OpSize SHL 6)+$30;
	   Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
         ModImm:
          IF OpSize=0 THEN
           BEGIN
            BAsmCode[0]:=Code+$30; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
           END
	  ELSE
	   BEGIN
            BAsmCode[0]:=$37; BAsmCode[1]:=Code+$30;
	    Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
	   END;
         ModImmExt:
          BEGIN
           BAsmCode[0]:=ExtCode; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
          END
         END;
        END;
       Exit;
      END;

   FOR z:=1 TO ImmOrderCnt DO
    WITH ImmOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         OpSize:=1; DecodeAdr(1,1,False,MModImm+MModImmExt);
         CASE AdrMode OF
         ModImm:
          BEGIN
           BAsmCode[0]:=$37; BAsmCode[1]:=Code;
           Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
         ModImmExt:
          BEGIN
           BAsmCode[0]:=Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
          END;
         END;
        END;
       Exit;
      END;

   IF (Memo('ANDP')) OR (Memo('ORP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=1; DecodeAdr(1,1,False,MModImm);
       CASE AdrMode OF
       ModImm:
        BEGIN
         BAsmCode[0]:=$37; BAsmCode[1]:=$3a+Ord(Memo('ORP'));
         Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('MAC')) OR (Memo('RMAC')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       z:=EvalIntExpression(ArgStr[1],UInt4,OK);
       IF OK THEN
        BEGIN
         BAsmCode[1]:=EvalIntExpression(ArgStr[2],UInt4,OK);
         IF OK THEN
          BEGIN
           Inc(BAsmCode[1],z SHL 4);
           BAsmCode[0]:=$7b+(Ord(Memo('RMAC')) SHL 7);
           CodeLen:=2;
          END;
        END;
      END;
     Exit;
    END;

   { Bitoperationen }

   IF (Memo('BCLR')) OR (Memo('BSET')) THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=0; z:=Ord(Memo('BSET'));
       DecodeAdr(ArgCnt,ArgCnt,False,MModImm);
       CASE AdrMode OF
       ModImm:
        BEGIN
         Mask:=AdrVals[0];
         DecodeAdr(1,ArgCnt-1,False,MModDisp8+MModDisp16+MModAbs);
         CASE AdrMode OF
         ModDisp8:
          BEGIN
           BAsmCode[0]:=$17; BAsmCode[1]:=$08+z+AdrPart;
           BAsmCode[2]:=Mask; BAsmCode[3]:=AdrVals[0];
           CodeLen:=4;
          END;
         ModDisp16:
          BEGIN
           BAsmCode[0]:=$08+z+AdrPart; BAsmCode[1]:=Mask;
	   Move(AdrVals,BAsmCode[2],AdrCnt);
           CodeLen:=2+AdrCnt;
          END;
         ModAbs:
          BEGIN
           BAsmCode[0]:=$38+z; BAsmCode[1]:=Mask;
	   Move(AdrVals,BAsmCode[2],AdrCnt);
           CodeLen:=2+AdrCnt;
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('BCLRW')) OR (Memo('BSETW')) THEN
    BEGIN
     IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=1; z:=Ord(Memo('BSETW'));
       DecodeAdr(ArgCnt,ArgCnt,False,MModImm);
       CASE AdrMode OF
       ModImm:
        BEGIN
         Move(AdrVals,BAsmCode[2],AdrCnt);
         DecodeAdr(1,ArgCnt-1,False,MModDisp16+MModAbs);
         CASE AdrMode OF
         ModDisp16:
          BEGIN
           BAsmCode[0]:=$27; BAsmCode[1]:=$08+z+AdrPart;
	   Move(AdrVals,BAsmCode[4],AdrCnt);
           CodeLen:=4+AdrCnt;
          END;
         ModAbs:
          BEGIN
           BAsmCode[0]:=$27; BAsmCode[1]:=$38+z;
	   Move(AdrVals,BAsmCode[4],AdrCnt);
           CodeLen:=4+AdrCnt;
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('BRCLR')) OR (Memo('BRSET')) THEN
    BEGIN
     IF (ArgCnt<3) OR (ArgCnt>4) THEN WrError(1110)
     ELSE
      BEGIN
       z:=Ord(Memo('BRSET'));
       OpSize:=0; DecodeAdr(ArgCnt-1,ArgCnt-1,False,MModImm);
       IF AdrMode=ModImm THEN
        BEGIN
         BAsmCode[1]:=AdrVals[0];
         AdrLong:=EvalIntExpression(ArgStr[ArgCnt],UInt20,OK)-EProgCounter-6;
         IF OK THEN
          BEGIN
           OK:=SymbolQuestionable;
           DecodeAdr(1,ArgCnt-2,False,MModDisp8+MModDisp16+MModAbs);
           CASE AdrMode OF
           ModDisp8:
            IF (AdrLong>=-128) AND (AdrLong<127) THEN
             BEGIN
              BAsmCode[0]:=$cb-(z SHL 6)+AdrPart;
              BAsmCode[2]:=AdrVals[0];
              BAsmCode[3]:=AdrLong AND $ff;
              CodeLen:=4;
             END
            ELSE IF (NOT OK) AND ((AdrLong<-$8000) OR (AdrLong>$7fff)) THEN WrError(1370)
            ELSE
             BEGIN
              BAsmCode[0]:=$0a+AdrPart+z;
              BAsmCode[2]:=0;
              BAsmCode[3]:=AdrVals[0];
              BAsmCode[4]:=(AdrLong SHR 8) AND $ff;
	      BAsmCode[5]:=AdrLong AND $ff;
	      CodeLen:=6;
             END;
           ModDisp16:
            IF (NOT OK) AND ((AdrLong<-$8000) OR (AdrLong>$7fff)) THEN WrError(1370)
            ELSE
             BEGIN
              BAsmCode[0]:=$0a+AdrPart+z;
              Move(AdrVals,BAsmCode[2],2);
              BAsmCode[4]:=(AdrLong SHR 8) AND $ff;
	      BAsmCode[5]:=AdrLong AND $ff;
	      CodeLen:=6;
             END;
           ModAbs:
            IF (NOT OK) AND ((AdrLong<-$8000) OR (AdrLong>$7fff)) THEN WrError(1370)
            ELSE
             BEGIN
              BAsmCode[0]:=$3a+z;
              Move(AdrVals,BAsmCode[2],2);
              BAsmCode[4]:=(AdrLong SHR 8) AND $ff;
	      BAsmCode[5]:=AdrLong AND $ff;
	      CodeLen:=6;
             END;
           END;
          END;
        END;
      END;
     Exit;
    END;

   { SprÅnge }

   IF (Memo('JMP')) OR (Memo('JSR')) THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=1;
       DecodeAdr(1,ArgCnt,True,MModAbs20+MModDisp20);
       CASE AdrMode OF
       ModAbs20:
        BEGIN
         BAsmCode[0]:=$7a+(Ord(Memo('JSR')) SHL 7);
         Move(AdrVals,BAsmCode[1],AdrCnt);
         CodeLen:=1+AdrCnt;
        END;
       ModDisp20:
        BEGIN
         IF Memo('JMP') THEN BAsmCode[0]:=$4b
	 ELSE BAsmCode[0]:=$89;
	 Inc(BAsmCode[0],AdrPart);
         Move(AdrVals,BAsmCode[1],AdrCnt);
         CodeLen:=1+AdrCnt;
        END;
       END;
      END;
     Exit;
    END;

   FOR z:=1 TO RelOrderCnt DO
    WITH RelOrders^[z] DO
     IF (Memo(Name)) OR (Memo('L'+Name)) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK)-EProgCounter-6;
         IF Odd(AdrLong) THEN WrError(1325)
         ELSE IF OpPart[1]='L' THEN
          IF (NOT SymbolQuestionable) AND ((AdrLong>$7fff) OR (AdrLong<-$8000)) THEN WrError(1370)
          ELSE
           BEGIN
            BAsmCode[0]:=$37; BAsmCode[1]:=Code+$80;
            BAsmCode[2]:=(AdrLong SHR 8) AND $ff;
            BAsmCode[3]:=AdrLong AND $ff;
            CodeLen:=4;
           END
         ELSE
          IF (NOT SymbolQuestionable) AND ((AdrLong>$7f) OR (AdrLong<-$80)) THEN WrError(1370)
          ELSE
           BEGIN
            BAsmCode[0]:=$b0+Code;
            BAsmCode[1]:=AdrLong AND $ff;
            CodeLen:=2;
           END;
        END;
       Exit;
      END;

   FOR z:=1 TO LRelOrderCnt DO
    WITH LRelOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK)-EProgCounter-6;
         IF Odd(AdrLong) THEN WrError(1325)
         ELSE IF (NOT SymbolQuestionable) AND ((AdrLong>$7fff) OR (AdrLong<-$8000)) THEN WrError(1370)
         ELSE
          BEGIN
           BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
           BAsmCode[2]:=(AdrLong SHR 8) AND $ff;
           BAsmCode[3]:=AdrLong AND $ff;
           CodeLen:=4;
          END;
        END;
       Exit;
      END;

   IF Memo('BSR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK)-EProgCounter-6;
       IF Odd(AdrLong) THEN WrError(1325)
       ELSE IF (NOT SymbolQuestionable) AND ((AdrLong>$7f) OR (AdrLong<-$80)) THEN WrError(1370)
       ELSE
        BEGIN
         BAsmCode[0]:=$36;
         BAsmCode[1]:=AdrLong AND $ff;
         CodeLen:=2;
        END;
      END;
     Exit;
    END;
   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_6816;
	Far;
BEGIN
   SaveInitProc;
   Reg_EK:=0;
END;

	FUNCTION ChkPC_6816:Boolean;
	Far;
BEGIN
   IF ActPC=SegCode THEN ChkPC_6816:=(ProgCounter>=0) AND (ProgCounter<$100000)
   ELSE ChkPC_6816:=False;
END;

	FUNCTION IsDef_6816:Boolean;
	Far;
BEGIN
   IsDef_6816:=False;
END;

        PROCEDURE SwitchFrom_6816;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_6816;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$65; NOPCode:=$274c;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_6816; ChkPC:=ChkPC_6816; IsDef:=IsDef_6816;
   SwitchFrom:=SwitchFrom_6816;

   InitFields;
END;

BEGIN
   CPU6816:=AddCPU('68HC16',SwitchTo_6816);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_6816;
END.
