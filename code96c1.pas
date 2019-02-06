{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code96C1;

INTERFACE

        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;



IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[6];
	       Code:Word;
	       CPUFlag:Byte;
	       InSup:Boolean;
	      END;

   ImmOrder=RECORD
	     Name:String[4];
	     Code:Word;
	     InSup:Boolean;
	     MinMax,MaxMax:Byte;
	     Default:ShortInt;
	    END;

   RegOrder=RECORD
	     Name:String[4];
	     Code:Word;
	     OpMask:Byte;
	    END;

   ALU2Order=RECORD
	      Name:String[5];
	      Code:Byte;
	     END;

   Condition=RECORD
	      Name:String[3];
	      Code:Byte;
	     END;


CONST
   FixedOrderCnt=13;

   ImmOrderCnt=3;
   ImmOrders:ARRAY[1..ImmOrderCnt] OF ImmOrder=
	     ((Name:'EI'  ; Code:$0600; InSup:True;  MinMax:7; MaxMax:7; Default: 0),
	      (Name:'LDF' ; Code:$1700; InSup:False; MinMax:7; MaxMax:3; Default:-1),
	      (Name:'SWI' ; Code:$00f8; InSup:False; MinMax:7; MaxMax:7; Default:7));

   RegOrderCnt=8;

   ALU2OrderCnt=8;
   ALU2Orders:ARRAY[1..ALU2OrderCnt] OF ALU2Order=
	      ((Name:'ADC' ; Code:1),
	       (Name:'ADD' ; Code:0),
	       (Name:'AND' ; Code:4),
	       (Name:'OR'  ; Code:6),
	       (Name:'SBC' ; Code:3),
	       (Name:'SUB' ; Code:2),
	       (Name:'XOR' ; Code:5),
	       (Name:'CP'  ; Code:7));

   BitCFOrderCnt=5;
   BitCFOrders:ARRAY[1..BitCFOrderCnt] OF ALU2Order=
	       ((Name:'ANDCF' ; Code:0),
		(Name:'LDCF'  ; Code:3),
		(Name:'ORCF'  ; Code:1),
		(Name:'STCF'  ; Code:4),
		(Name:'XORCF' ; Code:2));

   BitOrderCnt=5;
   BitOrders:ARRAY[0..BitOrderCnt-1] OF String[4]=('RES','SET','CHG','BIT','TSET');

   MulDivOrderCnt=4;
   MulDivOrders:ARRAY[0..MulDivOrderCnt-1] OF String[4]=('MUL','MULS','DIV','DIVS');

   ShiftOrderCnt=8;
   ShiftOrders:ARRAY[0..ShiftOrderCnt-1] OF String[3]=('RLC','RRC','RL','RR','SLA','SRA','SLL','SRL');

   ConditionCnt=24;
   Conditions:ARRAY[1..ConditionCnt] OF Condition=
	      ((Name:'F'   ; Code: 0),(Name:'T'   ; Code: 8),
	       (Name:'Z'   ; Code: 6),(Name:'NZ'  ; Code:14),
	       (Name:'C'   ; Code: 7),(Name:'NC'  ; Code:15),
	       (Name:'PL'  ; Code:13),(Name:'MI'  ; Code: 5),
	       (Name:'P'   ; Code:13),(Name:'M'   ; Code: 5),
	       (Name:'NE'  ; Code:14),(Name:'EQ'  ; Code: 6),
	       (Name:'OV'  ; Code: 4),(Name:'NOV' ; Code:12),
	       (Name:'PE'  ; Code: 4),(Name:'PO'  ; Code:12),
	       (Name:'GE'  ; Code: 9),(Name:'LT'  ; Code: 1),
	       (Name:'GT'  ; Code:10),(Name:'LE'  ; Code: 2),
	       (Name:'UGE' ; Code:15),(Name:'ULT' ; Code: 7),
	       (Name:'UGT' ; Code:11),(Name:'ULE' ; Code: 3));
   DefaultCondition=2;

   ModNone=-1;
   ModReg=0;    MModReg=1  SHL ModReg;
   ModXReg=1;   MModXReg=1 SHL ModXReg;
   ModMem=2;    MModMem=1  SHL ModMem;
   ModImm=3;    MModImm=1  SHL ModImm;
   ModCReg=4;   MModCReg=1 SHL ModCReg;

TYPE
   FixedOrderArray=ARRAY[1..FixedOrderCnt] OF FixedOrder;
   RegOrderArray=ARRAY[1..RegOrderCnt] OF RegOrder;

VAR
   FixedOrders:^FixedOrderArray;
   RegOrders:^RegOrderArray;

   Maximum:Boolean;
   AdrType:ShortInt;
   OpSize:ShortInt;        { -1/0/1/2 = nix/Byte/Word/Long }
   AdrMode:Byte;
   AdrCnt:Byte;
   AdrVals:ARRAY[0..9] OF Byte;
   MinOneIs0:Boolean;

   CPU96C141,CPU93C141:CPUVar;


{-----------------------------------------------------------------------------}

        PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddFixed(NName:String; NCode:Word; NFlag:Byte; NSup:Boolean);
BEGIN
   Inc(z); IF z>FixedOrderCnt THEN Halt;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; CPUFlag:=NFlag; InSup:=NSup;
    END;
END;

        PROCEDURE AddReg(NName:String; NCode:Word; NMask:Byte);
BEGIN
   Inc(z); IF z>RegOrderCnt THEN Halt;
   WITH RegOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; OpMask:=NMask;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('CCF'   , $0012, 3, False);
   AddFixed('DECF'  , $000d, 3, False);
   AddFixed('DI'    , $0607, 3, True );
   AddFixed('HALT'  , $0005, 3, True );
   AddFixed('INCF'  , $000c, 3, False);
   AddFixed('MAX'   , $0004, 1, True );
   AddFixed('MIN'   , $0004, 2, True );
   AddFixed('NOP'   , $0000, 3, False);
   AddFixed('NORMAL', $0001, 1, True );
   AddFixed('RCF'   , $0010, 3, False);
   AddFixed('RETI'  , $0007, 3, True );
   AddFixed('SCF'   , $0011, 3, False);
   AddFixed('ZCF'   , $0013, 3, False);

   New(RegOrders); z:=0;
   AddReg('CPL' , $c006, 3);
   AddReg('DAA' , $c010, 1);
   AddReg('EXTS', $c013, 6);
   AddReg('EXTZ', $c012, 6);
   AddReg('MIRR', $c016, 2);
   AddReg('NEG' , $c007, 3);
   AddReg('PAA' , $c014, 6);
   AddReg('UNLK', $c00d, 4);
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(RegOrders);
END;

{-----------------------------------------------------------------------------}

	FUNCTION IsRegBase(No,Size:Byte):Boolean;
BEGIN
   IsRegBase:=(Size=2) OR ((Size=1) AND (No<$f0) AND (NOT Maximum) AND (No AND 3=0));
END;

	FUNCTION IsRegCurrent(No,Size:Byte; VAR Erg:Byte):Boolean;
BEGIN
   IsRegCurrent:=False;
   CASE Size OF
   0:IF No AND $f2=$e0 THEN
      BEGIN
       IsRegCurrent:=True;
       Erg:=((No AND $0c) SHR 1)+((No AND 1) XOR 1);
      END;
   1,2:IF No AND $e3=$e0 THEN
      BEGIN
       IsRegCurrent:=True;
       Erg:=((No AND $1c) SHR 2);
      END;
   END;
END;

	FUNCTION CodeEReg(Asc:String; VAR ErgNo:Byte; VAR ErgSize:Byte):Byte;
CONST
   RegCnt=8;
   Reg8Names :ARRAY[0..RegCnt-1] OF String[1]=
	      ('A'  ,'W'  ,'C'  ,'B'  ,'E'  ,'D'  ,'L'  ,'H'  );
   Reg16Names:ARRAY[0..RegCnt-1] OF String[2]=
	      ('WA' ,'BC' ,'DE' ,'HL' ,'IX' ,'IY' ,'IZ' ,'SP' );
   Reg32Names:ARRAY[0..RegCnt-1] OF String[3]=
	      ('XWA','XBC','XDE','XHL','XIX','XIY','XIZ','XSP');

VAR
   z:Integer;
   HAsc:String;

	PROCEDURE ChkMaximum(MustMax:Boolean);
BEGIN
   IF Maximum<>MustMax THEN
    BEGIN
     CodeEReg:=1;
     IF MustMax THEN WrError(1997)
     ELSE WrError(1996);
    END;
END;

	FUNCTION IsQuot(Ch:Char):Boolean;
BEGIN
   IsQuot:=(Ch='''') OR (Ch='`');
END;

BEGIN
   NLS_UpString(Asc);

   CodeEReg:=2;

   { mom. Bank ? }

   FOR z:=0 TO RegCnt-1 DO
    BEGIN
     IF Asc=Reg8Names[z] THEN
      BEGIN
       ErgNo:=$e0+((z AND 6) SHL 1)+(z AND 1); ErgSize:=0; Exit;
      END;
     IF Asc=Reg16Names[z] THEN
      BEGIN
       ErgNo:=$e0+(z SHL 2); ErgSize:=1; Exit;
      END;
     IF Asc=Reg32Names[z] THEN
      BEGIN
       ErgNo:=$e0+(z SHL 2); ErgSize:=2;
       IF z<4 THEN ChkMaximum(True);
       Exit;
      END;
    END;

   { Bankregister, 8 Bit ? }

   IF (Length(Asc)=3) AND ((Asc[1]='Q') OR (Asc[1]='R')) AND ((Asc[3]>='0') AND (Asc[3]<='7')) THEN
    FOR z:=0 TO RegCnt-1 DO
     IF Asc[2]=Reg8Names[z] THEN
      BEGIN
       ErgNo:=((Ord(Asc[3])-AscOfs) SHL 4)+((z AND 6) SHL 1)+(z AND 1);
       IF Asc[1]='Q' THEN
	BEGIN
	 Inc(ErgNo,2); ChkMaximum(True);
	END;
       IF ((Asc[1]='Q') OR (Maximum)) AND (Asc[3]>'3') THEN
	BEGIN
	 WrError(1320); CodeEReg:=1;
	END;
       ErgSize:=0; Exit;
      END;

   { Bankregister, 16 Bit ? }

   IF (Length(Asc)=4) AND ((Asc[1]='Q') OR (Asc[1]='R')) AND ((Asc[4]>='0') AND (Asc[4]<='7')) THEN
    BEGIN
     HAsc:=Copy(Asc,2,2);
     FOR z:=0 TO (RegCnt DIV 2)-1 DO
      IF HAsc=Reg16Names[z] THEN
       BEGIN
	ErgNo:=((Ord(Asc[4])-AscOfs) SHL 4)+(z SHL 2);
	IF Asc[1]='Q' THEN
	 BEGIN
	  Inc(ErgNo,2); ChkMaximum(True);
	 END;
	IF ((Asc[1]='Q') OR (Maximum)) AND (Asc[4]>'3') THEN
	 BEGIN
	  WrError(1320); CodeEReg:=1;
	 END;
	ErgSize:=1; Exit;
       END;
    END;

   { Bankregister, 32 Bit ? }

   IF (Length(Asc)=4) AND ((Asc[4]>='0') AND (Asc[4]<='7')) THEN
    BEGIN
     HAsc:=Copy(Asc,1,3);
     FOR z:=0 TO (RegCnt DIV 2)-1 DO
      IF HAsc=Reg32Names[z] THEN
       BEGIN
	ErgNo:=((Ord(Asc[4])-AscOfs) SHL 4)+(z SHL 2);
	ChkMaximum(True);
	IF Asc[4]>'3' THEN
	 BEGIN
	  WrError(1320); CodeEReg:=1;
	 END;
	ErgSize:=2; Exit;
       END;
    END;

   { obere 8-Bit-H„lften momentaner Bank ? }

   IF (Length(Asc)=2) AND (Asc[1]='Q') THEN
    FOR z:=0 TO RegCnt-1 DO
     IF Asc[2]=Reg8Names[z] THEN
      BEGIN
       ErgNo:=$e2+((z AND 6) SHL 1)+(z AND 1);
       ChkMaximum(True);
       ErgSize:=0; Exit;
      END;

   { obere 16-Bit-H„lften momentaner Bank und von XIX..XSP ? }

   IF (Length(Asc)=3) AND (Asc[1]='Q') THEN
    BEGIN
     HAsc:=Copy(Asc,2,2);
     FOR z:=0 TO RegCnt-1 DO
      IF HAsc=Reg16Names[z] THEN
       BEGIN
	ErgNo:=$e2+(z SHL 2);
	IF z<4 THEN ChkMaximum(True);
	ErgSize:=1; Exit;
       END;
    END;

   { 8-Bit-Teile von XIX..XSP ? }

   IF ((Length(Asc)=3) OR ((Length(Asc)=4) AND (Asc[1]='Q')))
   AND ((Asc[Length(Asc)]='L') OR (Asc[Length(Asc)]='H')) THEN
    BEGIN
     HAsc:=Copy(Asc,Length(Asc)-2,2);
     FOR z:=0 TO (RegCnt DIV 2)-1 DO
      IF HAsc=Reg16Names[z+4] THEN
       BEGIN
	ErgNo:=$f0+(z SHL 2)+((Length(Asc)-3) SHL 1)+(Ord(Asc[Length(Asc)]='H'));
	ErgSize:=0; Exit;
       END;
    END;

   { 8-Bit-Teile vorheriger Bank ? }

   IF ((Length(Asc)=2) OR ((Length(Asc)=3) AND (Asc[1]='Q'))) AND (IsQuot(Asc[Length(Asc)])) THEN
    FOR z:=0 TO RegCnt-1 DO
     IF Asc[Length(Asc)-1]=Reg8Names[z] THEN
      BEGIN
       ErgNo:=$d0+((z AND 6) SHL 1)+((Length(Asc)-2) SHL 1)+(z AND 1);
       IF Length(Asc)=3 THEN ChkMaximum(True);
       ErgSize:=0; Exit;
      END;

   { 16-Bit-Teile vorheriger Bank ? }

   IF ((Length(Asc)=3) OR ((Length(Asc)=4) AND (Asc[1]='Q'))) AND (IsQuot(Asc[Length(Asc)])) THEN
    BEGIN
     HAsc:=Copy(Asc,Length(Asc)-2,2);
     FOR z:=0 TO (RegCnt DIV 2)-1 DO
      IF HAsc=Reg16Names[z] THEN
       BEGIN
	ErgNo:=$d0+(z SHL 2)+((Length(Asc)-3) SHL 1);
	IF Length(Asc)=4 THEN ChkMaximum(True);
	ErgSize:=1; Exit;
       END;
    END;

   { 32-Bit-Register vorheriger Bank ? }

   IF (Length(Asc)=4) AND (IsQuot(Asc[4])) THEN
    BEGIN
     HAsc:=Copy(Asc,1,3);
     FOR z:=0 TO (RegCnt DIV 2)-1 DO
      IF HAsc=Reg32Names[z] THEN
       BEGIN
	ErgNo:=$d0+(z SHL 2);
	ChkMaximum(True);
	ErgSize:=2; Exit;
       END;
    END;

   CodeEReg:=0;
END;

	FUNCTION CodeCReg(Asc:String; VAR ErgNo:Byte; VAR ErgSize:Byte):Byte;

	PROCEDURE ChkL(Must:CPUVar);
BEGIN
   IF MomCPU<>Must THEN
    BEGIN
     WrError(1440); CodeCReg:=0;
    END;
END;

BEGIN
   NLS_UpString(Asc);
   CodeCReg:=2;
   IF Asc='NSP' THEN
    BEGIN
     ErgNo:=$3c; ErgSize:=1;
     ChkL(CPU96C141);
     Exit;
    END;
   IF Asc='XNSP' THEN
    BEGIN
     ErgNo:=$3c; ErgSize:=2;
     ChkL(CPU96C141);
     Exit;
    END;
   IF Asc='INTNEST' THEN
    BEGIN
     ErgNo:=$3c; ErgSize:=1;
     ChkL(CPU93C141);
     Exit;
    END;
   IF (Length(Asc)=5) AND (Copy(Asc,1,3)='DMA') AND (Asc[5]>='0') AND (Asc[5]<='3') THEN
   CASE Asc[4] OF
   'S':BEGIN
	ErgNo:=(Ord(Asc[5])-AscOfs)*4; ErgSize:=2; Exit;
       END;
   'D':BEGIN
	ErgNo:=(Ord(Asc[5])-AscOfs)*4+$10; ErgSize:=2; Exit;
       END;
   'M':BEGIN
	ErgNo:=(Ord(Asc[5])-AscOfs)*4+$22; ErgSize:=0; Exit;
       END;
   'C':BEGIN
	ErgNo:=(Ord(Asc[5])-AscOfs)*4+$20; ErgSize:=1; Exit;
       END;
   END;

   CodeCReg:=0;
END;

	PROCEDURE DecodeAdr(Asc:String; Erl:Byte);
LABEL
   AdrFnd;

TYPE
   RegDesc=RECORD
	    Name:String[4];
	    Num:Byte;
	    InMax,InMin:Boolean;
	   END;

VAR
   z:Integer;
   HAsc:String;
   HNum,HSize:Byte;
   OK,NegFlag,NNegFlag,MustInd,FirstFlag:Boolean;
   BaseReg,BaseSize:Byte;
   IndReg,IndSize:Byte;
   PartMask:Byte;
   DispPart,DispAcc:LongInt;
   MPos,PPos,EPos:Integer;

	PROCEDURE SetOpSize(NewSize:Byte);
BEGIN
   IF OpSize=-1 THEN OpSize:=NewSize
   ELSE IF OpSize<>NewSize THEN
    BEGIN
     WrError(1131); AdrType:=ModNone;
    END;
END;

	PROCEDURE ChkMaximum(MustMax:Boolean);
BEGIN
   IF Maximum<>MustMax THEN
    BEGIN
     AdrType:=ModNone;
     IF MustMax THEN WrError(1997)
     ELSE WrError(1996);
    END;
END;

	FUNCTION IsRegCurrent(No,Size:Byte; VAR Erg:Byte):Boolean;
BEGIN
   IsRegCurrent:=False;
   CASE Size OF
   0:IF No AND $f2=$e0 THEN
      BEGIN
       IsRegCurrent:=True;
       Erg:=((No AND $0c) SHR 1)+((No AND 1) XOR 1);
      END;
   1,2:IF No AND $e3=$e0 THEN
      BEGIN
       IsRegCurrent:=True;
       Erg:=((No AND $1c) SHR 2);
      END;
   END;
END;

BEGIN
   AdrType:=ModNone;

   { Register ? }

   CASE CodeEReg(Asc,HNum,HSize) OF
   1:Goto AdrFnd;
   2:BEGIN
      IF IsRegCurrent(HNum,HSize,AdrMode) THEN
       AdrType:=ModReg
      ELSE
       BEGIN
	AdrType:=ModXReg; AdrMode:=HNum;
       END;
      SetOpSize(HSize);
      Goto AdrFnd;
     END;
   END;

   { Steuerregister ? }

   IF CodeCReg(Asc,HNum,HSize)=2 THEN
    BEGIN
     AdrType:=ModCReg; AdrMode:=HNum;
     SetOpSize(HSize);
     Goto AdrFnd;
    END;

   { Predekrement ? }

   IF (Length(Asc)>4) AND (Asc[Length(Asc)]=')') AND (Copy(Asc,1,2)='(-') THEN
    BEGIN
     IF CodeEReg(Copy(Asc,3,Length(Asc)-3),HNum,HSize)<>2 THEN WrError(1350)
     ELSE IF NOT IsRegBase(HNum,HSize) THEN WrError(1350)
     ELSE
      BEGIN
       AdrType:=ModMem; AdrMode:=$44;
       AdrCnt:=1; AdrVals[0]:=HNum; IF OpSize<>-1 THEN Inc(AdrVals[0],OpSize);
      END;
     Goto AdrFnd;
    END;

   { Postinkrement ? }

   IF (Length(Asc)>4) AND (Asc[1]='(') AND (Copy(Asc,Length(Asc)-1,2)='+)') THEN
    BEGIN
     IF CodeEReg(Copy(Asc,2,Length(Asc)-3),HNum,HSize)<>2 THEN WrError(1350)
     ELSE IF NOT IsRegBase(HNum,HSize) THEN WrError(1350)
     ELSE
      BEGIN
       AdrType:=ModMem; AdrMode:=$45;
       AdrCnt:=1; AdrVals[0]:=HNum; IF OpSize<>-1 THEN Inc(AdrVals[0],OpSize);
      END;
     Goto AdrFnd;
    END;

   { Speicheroperand ? }

   IF IsIndirect(Asc) THEN
    BEGIN
     NegFlag:=False; NNegFlag:=False; FirstFlag:=False;
     PartMask:=0; DispAcc:=0;
     Asc:=Copy(Asc,2,Length(Asc)-2);

     REPEAT
      MPos:=QuotPos(Asc,'-'); PPos:=QuotPos(Asc,'+');
      IF PPos<MPos THEN
       BEGIN
	EPos:=PPos; NNegFlag:=False;
       END
      ELSE IF MPos<PPos THEN
       BEGIN
	EPos:=MPos; NNegFlag:=True;
       END
      ELSE EPos:=Length(Asc)+1;
      IF (EPos=1) OR (EPos=Length(Asc)) THEN
       BEGIN
        WrError(1350); Goto AdrFnd;
       END;
      HAsc:=Copy(Asc,1,EPos-1);
      Delete(Asc,1,EPos);

      CASE CodeEReg(HAsc,HNum,HSize) OF
      0:BEGIN
         FirstPassUnknown:=False;
	 DispPart:=EvalIntExpression(HAsc,Int32,OK);
         IF FirstPassUnknown THEN FirstFlag:=True;
	 IF NOT OK THEN Goto AdrFnd;
	 IF NegFlag THEN Dec(DispAcc,DispPart) ELSE Inc(DispAcc,DispPart);
	 PartMask:=PartMask OR 1;
	END;
      1:Goto AdrFnd;
      2:IF NegFlag THEN
	 BEGIN
	  WrError(1350); Goto AdrFnd;
	 END
	ELSE
	 BEGIN
	  IF HSize=0 THEN MustInd:=True
	  ELSE IF HSize=2 THEN MustInd:=False
	  ELSE IF NOT IsRegBase(HNum,HSize) THEN MustInd:=True
	  ELSE IF PartMask AND 4<>0 THEN MustInd:=True
	  ELSE MustInd:=False;
	  IF MustInd THEN
	   IF PartMask AND 2<>0 THEN
	    BEGIN
	     WrError(1350); Goto AdrFnd;
	    END
	   ELSE
	    BEGIN
	     IndReg:=HNum; PartMask:=PartMask OR 2;
	     IndSize:=HSize;
	    END
	  ELSE
	   IF PartMask AND 4<>0 THEN
	    BEGIN
	     WrError(1350); Goto AdrFnd;
	    END
	   ELSE
	    BEGIN
	     BaseReg:=HNum; PartMask:=PartMask OR 4;
	     BaseSize:=HSize;
	    END;
	 END;
      END;

      NegFlag:=NNegFlag; NNegFlag:=False;
     UNTIL Asc='';

     IF (DispAcc=0) AND (PartMask<>1) THEN PartMask:=PartMask AND 6;
     IF (PartMask=5) AND (FirstFlag) THEN DispAcc:=DispAcc AND $7fff;

     CASE PartMask OF
     0,2,3,
     7:WrError(1350);
     1:IF DispAcc<=$ff THEN
	BEGIN
	 AdrType:=ModMem; AdrMode:=$40; AdrCnt:=1;
	 AdrVals[0]:=DispAcc;
	END
       ELSE IF DispAcc<$ffff THEN
	BEGIN
	 AdrType:=ModMem; AdrMode:=$41; AdrCnt:=2;
	 AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc);
	END
       ELSE IF DispAcc<$ffffff THEN
	BEGIN
	 AdrType:=ModMem; AdrMode:=$42; AdrCnt:=3;
	 AdrVals[0]:=DispAcc AND $ff;
	 AdrVals[1]:=(DispAcc SHR 8) AND $ff;
	 AdrVals[2]:=(DispAcc SHR 16) AND $ff;
	END
       ELSE WrError(1925);
     4:IF IsRegCurrent(BaseReg,BaseSize,AdrMode) THEN
	BEGIN
	 AdrType:=ModMem; AdrCnt:=0;
	END
       ELSE
	BEGIN
	 AdrType:=ModMem; AdrMode:=$43; AdrCnt:=1;
	 AdrVals[0]:=BaseReg;
	END;
     5:IF (DispAcc<=127) AND (DispAcc>=-128) AND (IsRegCurrent(BaseReg,BaseSize,AdrMode)) THEN
	BEGIN
	 AdrType:=ModMem; Inc(AdrMode,8); AdrCnt:=1;
	 AdrVals[0]:=DispAcc AND $ff;
	END
       ELSE IF (DispAcc<=32767) AND (DispAcc>=-32768) THEN
	BEGIN
	 AdrType:=ModMem; AdrMode:=$43; AdrCnt:=3;
	 AdrVals[0]:=BaseReg+1;
	 AdrVals[1]:=DispAcc AND $ff;
	 AdrVals[2]:=(DispAcc SHR 8) AND $ff;
	END
       ELSE WrError(1320);
     6:BEGIN
	AdrType:=ModMem; AdrMode:=$43; AdrCnt:=3;
	AdrVals[0]:=3+(IndSize SHL 2);
	AdrVals[1]:=BaseReg;
	AdrVals[2]:=IndReg;
       END;
     END;
     Goto AdrFnd;
    END;

   { bleibt nur noch immediate... }

   IF (MinOneIs0) AND (OpSize=-1) THEN SetOpSize(0);
   CASE OpSize OF
   -1:WrError(1132);
   0:BEGIN
      AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
      IF OK THEN
       BEGIN
	AdrType:=ModImm; AdrCnt:=1;
       END;
     END;
   1:BEGIN
      DispAcc:=EvalIntExpression(Asc,Int16,OK);
      IF OK THEN
       BEGIN
	AdrType:=ModImm; AdrCnt:=2;
	AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc);
       END;
     END;
   2:BEGIN
      DispAcc:=EvalIntExpression(Asc,Int32,OK);
      IF OK THEN
       BEGIN
	AdrType:=ModImm; AdrCnt:=4;
	AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc);
	AdrVals[2]:=Lo(DispAcc SHR 16); AdrVals[3]:=Hi(DispAcc SHR 16);
       END;
     END;
   END;

   IF AdrType=ModNone THEN
    BEGIN
     WrError(1350); Exit;
    END;

   { gefunden }

AdrFnd:
   IF AdrType<>ModNone THEN
    IF (1 SHL AdrType) AND Erl=0 THEN
     BEGIN
      WrError(1350); AdrType:=ModNone;
     END;
END;

	PROCEDURE CheckSup;
BEGIN
   IF MomCPU=CPU96C141 THEN
    IF NOT SupAllowed THEN WrError(50);
END;

	FUNCTION WMemo(Asc:String):Boolean;
BEGIN
   IF Memo(Asc) THEN
    BEGIN
     WMemo:=True; Exit;
    END;

   Asc:=Asc+'W';
   IF Memo(Asc) THEN
    BEGIN
     OpSize:=1; WMemo:=True; Exit;
    END;

   Asc[Length(Asc)]:='L';
   IF Memo(Asc) THEN
    BEGIN
     OpSize:=2; WMemo:=True; Exit;
    END;

   Asc[Length(Asc)]:='B';
   IF Memo(Asc) THEN
    BEGIN
     OpSize:=0; WMemo:=True; Exit;
    END;

   WMemo:=False;
END;

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFF96C1Count=2;
   ONOFF96C1s:ARRAY[1..ONOFF96C1Count] OF ONOFFRec=
	     ((Name:'MAXMODE'; Dest:@Maximum   ; FlagName:MaximumName   ),
	      (Name:'SUPMODE'; Dest:@SupAllowed; FlagName:SupAllowedName));
BEGIN
   DecodePseudo:=True;

   IF CodeONOFF(@ONOFF96C1s,ONOFF96C1Count) THEN Exit;

   DecodePseudo:=False;
END;

	PROCEDURE CorrMode(Ref,Adr:Byte);
BEGIN
   IF BAsmCode[Ref] AND $4e=$44 THEN
    BAsmCode[Adr]:=(BAsmCode[Adr] AND $fc) OR OpSize;
END;

	FUNCTION ArgPair(Val1,Val2:String):Boolean;
BEGIN
   ArgPair:=((NLS_StrCaseCmp(ArgStr[1],Val1)=0) AND (NLS_StrCaseCmp(ArgStr[2],Val2)=0)) OR
            ((NLS_StrCaseCmp(ArgStr[1],Val2)=0) AND (NLS_StrCaseCmp(ArgStr[2],Val1)=0));
END;

	FUNCTION ImmVal:LongInt;
VAR
   tmp:LongInt;
BEGIN
   tmp:=AdrVals[0];
   IF (OpSize>=1) THEN Inc(tmp,LongInt(AdrVals[1]) SHL 8);
   IF OpSize=2 THEN
    BEGIN
     Inc(tmp,LongInt(AdrVals[2]) SHL 16);
     Inc(tmp,LongInt(AdrVals[3]) SHL 24);
    END;
   ImmVal:=tmp;
END;

	FUNCTION IsPwr2(Inp:LongInt; VAR Erg:Byte):Boolean;
VAR
   Shift:LongInt;
BEGIN
   Shift:=1; Erg:=0; IsPwr2:=True;
   REPEAT
    IF Inp=Shift THEN Exit;
    Inc(Shift,Shift); Inc(Erg);
   UNTIL Shift=0;
   IsPwr2:=False;
END;

	FUNCTION IsShort(Code:Byte):Boolean;
BEGIN
   IsShort:=(Code AND $4e)=$40;
END;

	PROCEDURE MakeCode_96C141;
	Far;
VAR
   z:Integer;
   AdrWord:Word;
   AdrLong:LongInt;
   OK,ShSrc,ShDest:Boolean;
   HReg:Byte;
   CmpStr:String;
   LChar:Char;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1; MinOneIs0:=False;

   { zu ignorierendes }

   IF Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   IF WMemo('LD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
       CASE AdrType OF
       ModReg:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem+MModImm);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=2;
	   BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	   BAsmCode[1]:=$88+HReg;
	  END;
	 ModXReg:
	  BEGIN
	   CodeLen:=3;
	   BAsmCode[0]:=$c7+(OpSize SHL 4);
	   BAsmCode[1]:=AdrMode;
	   BAsmCode[2]:=$88+HReg;
	  END;
	 ModMem:
	  BEGIN
	   CodeLen:=2+AdrCnt;
	   BAsmCode[0]:=$80+(OpSize SHL 4)+AdrMode;
	   Move(AdrVals,BAsmCode[1],AdrCnt);
	   BAsmCode[1+AdrCnt]:=$20+HReg;
	  END;
	 ModImm:
	  IF (ImmVal<=7) AND (ImmVal>=0) THEN
	   BEGIN
	    CodeLen:=2;
	    BAsmCode[0]:=$c8+(OpSize SHL 4)+HReg;
	    BAsmCode[1]:=$a8+AdrVals[0];
	   END
	  ELSE
	   BEGIN
	    CodeLen:=1+AdrCnt;
	    BAsmCode[0]:=(OpSize+2) SHL 4+HReg;
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	   END;
	 END;
	END;
       ModXReg:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg+MModImm);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=3;
	   BAsmCode[0]:=$c7+(OpSize SHL 4);
	   BAsmCode[1]:=HReg;
	   BAsmCode[2]:=$98+AdrMode;
	  END;
	 ModImm:
	  IF (ImmVal<=7) AND (ImmVal>=0) THEN
	   BEGIN
	    CodeLen:=3;
	    BAsmCode[0]:=$c7+(OpSize SHL 4);
	    BAsmCode[1]:=HReg;
	    BAsmCode[2]:=$a8+AdrVals[0];
	   END
	  ELSE
	   BEGIN
	    CodeLen:=3+AdrCnt;
	    BAsmCode[0]:=$c7+(OpSize SHL 4);
	    BAsmCode[1]:=HReg;
	    BAsmCode[2]:=3;
	    Move(AdrVals,BAsmCode[3],AdrCnt);
	   END;
	 END;
	END;
       ModMem:
	BEGIN
	 BAsmCode[0]:=AdrMode;
	 HReg:=AdrCnt; MinOneIs0:=True;
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 DecodeAdr(ArgStr[2],MModReg+MModMem+MModImm);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=2+HReg;
	   Inc(BAsmCode[0],$b0); CorrMode(0,1);
	   BAsmCode[1+HReg]:=$40+(OpSize SHL 4)+AdrMode;
	  END;
	 ModMem:
	  BEGIN
	   IF OpSize=-1 THEN OpSize:=0;
	   ShDest:=IsShort(BAsmCode[0]); ShSrc:=IsShort(AdrMode);

	   IF NOT (ShDest OR ShSrc) THEN WrError(1350)
	   ELSE
	    BEGIN
	     IF (ShDest AND (NOT ShSrc)) THEN OK:=True
	     ELSE IF (ShSrc AND (NOT ShDest)) THEN OK:=False
	     ELSE IF AdrMode=$40 THEN OK:=True
	     ELSE OK:=False;

	     IF OK  THEN  { dest=(dir8/16) }
	      BEGIN
	       CodeLen:=4+AdrCnt; HReg:=BAsmCode[0];
	       IF BAsmCode[0]=$40 THEN BAsmCode[3+AdrCnt]:=0
				  ELSE BAsmCode[3+AdrCnt]:=BAsmCode[2];
	       BAsmCode[2+AdrCnt]:=BAsmCode[1];
	       BAsmCode[0]:=$80+(OpSize SHL 4)+AdrMode;
	       AdrMode:=HReg; CorrMode(0,1);
	       Move(AdrVals,BAsmCode[1],AdrCnt);
	       BAsmCode[1+AdrCnt]:=$19;
	      END
	     ELSE
	      BEGIN
	       CodeLen:=4+HReg;
	       BAsmCode[2+HReg]:=AdrVals[0];
	       IF AdrMode=$40 THEN BAsmCode[3+HReg]:=0
			      ELSE BAsmCode[3+HReg]:=AdrVals[1];
	       Inc(BAsmCode[0],$b0); CorrMode(0,1);
	       BAsmCode[1+HReg]:=$14+(OpSize SHL 1);
	      END;
	    END;
	  END;
	 ModImm:
	  IF BAsmCode[0]=$40 THEN
	   BEGIN
	    CodeLen:=2+AdrCnt;
	    BAsmCode[0]:=$08+(OpSize SHL 1);
	    Move(AdrVals,BAsmCode[2],AdrCnt);
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2+HReg+AdrCnt;
	    Inc(BAsmCode[0],$b0);
	    BAsmCode[1+HReg]:=OpSize SHL 1;
	    Move(AdrVals,BAsmCode[2+HReg],AdrCnt);
	   END;
	 END;
	END;
       END;
      END;
     Exit;
    END;

   IF (WMemo('POP')) OR (WMemo('PUSH')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'F')=0 THEN
      BEGIN
       CodeLen:=1;
       BAsmCode[0]:=$18+Ord(Memo('POP'));
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN
      BEGIN
       CodeLen:=1;
       BAsmCode[0]:=$14+Ord(Memo('POP'));
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'SR')=0 THEN
      BEGIN
       CodeLen:=1;
       BAsmCode[0]:=$02+Ord(Memo('POP'));
       CheckSup;
      END
     ELSE
      BEGIN
       MinOneIs0:=True;
       IF WMemo('PUSH') THEN DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem+MModImm)
       ELSE DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
       CASE AdrType OF
       ModReg:
	IF OpSize=0 THEN
	 BEGIN
	  CodeLen:=2;
	  BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	  BAsmCode[1]:=$04+Ord(Memo('POP'));
	 END
	ELSE
	 BEGIN
	  CodeLen:=1;
	  BAsmCode[0]:=$28+(Ord(Memo('POP')) SHL 5)+((OpSize-1) SHL 4)+AdrMode;
	 END;
       ModXReg:
	BEGIN
	 CodeLen:=3;
	 BAsmCode[0]:=$c7+(OpSize SHL 4);
	 BAsmCode[1]:=AdrMode;
	 BAsmCode[2]:=$04+Ord(Memo('POP'));
	END;
       ModMem:
	BEGIN
	 IF OpSize=-1 THEN OpSize:=0;
	 CodeLen:=2+AdrCnt;
	 IF Copy(OpPart,1,3)='POP'
	  THEN BAsmCode[0]:=$b0+AdrMode
	  ELSE BAsmCode[0]:=$80+(OpSize SHL 4)+AdrMode;
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 IF Copy(OpPart,1,3)='POP'
	  THEN BAsmCode[1+AdrCnt]:=$04+(OpSize SHL 1)
	  ELSE BAsmCode[1+AdrCnt]:=$04;
	END;
       ModImm:
        BEGIN
         IF OpSize=-1 THEN OpSize:=0;
         BAsmCode[0]:=9+(OpSize SHL 1);
         Move(AdrVals,BAsmCode[1],AdrCnt);
         CodeLen:=1+AdrCnt;
        END;
       END;
      END;
     Exit;
    END;

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF CPUFlag AND (1 SHL (Ord(MomCPU)-Ord(CPU96C141)))=0 THEN WrError(1500)
       ELSE
	BEGIN
	 IF Hi(Code)=0 THEN
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=Lo(Code);
	  END
	 ELSE
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
	  END;
	 IF InSup THEN CheckSup;
	END;
       Exit;
      END;

   FOR z:=1 TO ImmOrderCnt DO
    WITH ImmOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt>1) OR ((Default=-1) AND (ArgCnt=0)) THEN WrError(1110)
       ELSE
	BEGIN
	 IF ArgCnt=0 THEN
	  BEGIN
	   AdrWord:=Default; OK:=True;
	  END
	 ELSE AdrWord:=EvalIntExpression(ArgStr[1],Int8,OK);
	 IF OK THEN
	  IF ((Maximum) AND (AdrWord>MaxMax)) OR ((NOT Maximum) AND (AdrWord>MinMax)) THEN WrError(1320)
	  ELSE IF Hi(Code)=0 THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=Lo(Code)+AdrWord;
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=Hi(Code);
	    BAsmCode[1]:=Lo(Code)+AdrWord;
	   END;
	 IF InSup THEN CheckSup;
	END;
       Exit;
      END;

   FOR z:=1 TO RegOrderCnt DO
    WITH RegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg+MModXReg);
	 IF AdrType<>ModNone THEN
	  IF (1 SHL OpSize) AND OpMask=0 THEN WrError(1130)
	  ELSE IF AdrType=ModReg THEN
	   BEGIN
	    BAsmCode[0]:=Hi(Code)+8+(OpSize SHL 4)+AdrMode;
	    BAsmCode[1]:=Lo(Code);
	    CodeLen:=2;
	   END
	  ELSE
	   BEGIN
	    BAsmCode[0]:=Hi(Code)+7+(OpSize SHL 4);
	    BAsmCode[1]:=AdrMode;
	    BAsmCode[2]:=Lo(Code);
	    CodeLen:=3;
	   END;
	END;
       Exit;
      END;

   FOR z:=1 TO ALU2OrderCnt DO
    WITH ALU2Orders[z] DO
     IF WMemo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
         CASE AdrType OF
         ModReg:
	  BEGIN
	   HReg:=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     CodeLen:=2;
	     BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	     BAsmCode[1]:=$80+(Code SHL 4)+HReg;
	    END;
	   ModXReg:
	    BEGIN
	     CodeLen:=3;
	     BAsmCode[0]:=$c7+(OpSize SHL 4);
	     BAsmCode[1]:=AdrMode;
	     BAsmCode[2]:=$80+(Code SHL 4)+HReg;
	    END;
	   ModMem:
	    BEGIN
	     CodeLen:=2+AdrCnt;
	     BAsmCode[0]:=$80+AdrMode+(OpSize SHL 4);
	     Move(AdrVals,BAsmCode[1],AdrCnt);
	     BAsmCode[1+AdrCnt]:=$80+HReg+(Code SHL 4);
	    END;
	   ModImm:
	    IF (Code=7) AND (OpSize<>2) AND (ImmVal<=7) AND (ImmVal>=0) THEN
	     BEGIN
	      CodeLen:=2;
	      BAsmCode[0]:=$c8+(OpSize SHL 4)+HReg;
	      BAsmCode[1]:=$d8+AdrVals[0];
	     END
	    ELSE
	     BEGIN
	      CodeLen:=2+AdrCnt;
	      BAsmCode[0]:=$c8+(OpSize SHL 4)+HReg;
	      BAsmCode[1]:=$c8+Code;
	      Move(AdrVals,BAsmCode[2],AdrCnt);
	     END;
	   END;
	  END;
         ModXReg:
	  BEGIN
	   HReg:=AdrMode;
	   DecodeAdr(ArgStr[2],MModImm);
	   CASE AdrType OF
	   ModImm:
	    IF (Code=7) AND (OpSize<>2) AND (ImmVal<=7) AND (ImmVal>=0) THEN
	     BEGIN
	      CodeLen:=3;
	      BAsmCode[0]:=$c7+(OpSize SHL 4);
	      BAsmCode[1]:=HReg;
	      BAsmCode[2]:=$d8+AdrVals[0];
	     END
	    ELSE
	     BEGIN
	      CodeLen:=3+AdrCnt;
	      BAsmCode[0]:=$c7+(OpSize SHL 4);
	      BAsmCode[1]:=HReg;
	      BAsmCode[2]:=$c8+Code;
	      Move(AdrVals,BAsmCode[3],AdrCnt);
	     END;
	   END;
	  END;
         ModMem:
	  BEGIN
	   MinOneIs0:=True;
	   HReg:=AdrCnt; BAsmCode[0]:=AdrMode;
	   Move(AdrVals,BAsmCode[1],AdrCnt);
	   DecodeAdr(ArgStr[2],MModReg+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     CodeLen:=2+HReg; CorrMode(0,1);
	     Inc(BAsmCode[0],$80+(OpSize SHL 4));
	     BAsmCode[1+HReg]:=$88+(Code SHL 4)+AdrMode;
	    END;
	   ModImm:
	    BEGIN
	     CodeLen:=2+HReg+AdrCnt;
	     Inc(BAsmCode[0],$80+(OpSize SHL 4));
	     BAsmCode[1+HReg]:=$38+Code;
	     Move(AdrVals,BAsmCode[2+HReg],AdrCnt);
	    END;
	   END;
	  END;
         END;
        END;
       Exit;
      END;

   FOR z:=0 TO ShiftOrderCnt-1 DO
    IF WMemo(ShiftOrders[z]) THEN
     BEGIN
      IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
      ELSE
       BEGIN
	OK:=True;
	IF ArgCnt=1 THEN HReg:=1
        ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN HReg:=$ff
	ELSE
	 BEGIN
          FirstPassUnknown:=False;
	  HReg:=EvalIntExpression(ArgStr[1],Int8,OK);
	  IF OK THEN
	   IF FirstPassUnknown THEN HReg:=HReg AND $0f
           ELSE
            IF (HReg=0) OR (HReg>16) THEN
             BEGIN
              WrError(1320); OK:=False;
             END
            ELSE HReg:=HReg AND $0f;
	 END;
	IF OK THEN
	 BEGIN
	  IF HReg=$ff THEN DecodeAdr(ArgStr[ArgCnt],MModReg+MModXReg)
	  ELSE DecodeAdr(ArgStr[ArgCnt],MModReg+MModXReg+MModMem);
	  CASE AdrType OF
	  ModReg:
	   BEGIN
	    CodeLen:=2+Ord(HReg<>$ff);
	    BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	    BAsmCode[1]:=$e8+z;
	    IF HReg=$ff THEN Inc(BAsmCode[1],$10)
	    ELSE BAsmCode[2]:=HReg;
	   END;
	  ModXReg:
	   BEGIN
	    CodeLen:=3+Ord(HReg<>$ff);
	    BAsmCode[0]:=$c7+(OpSize SHL 4);
	    BAsmCode[1]:=AdrMode;
	    BAsmCode[2]:=$e8+z;
	    IF HReg=$ff THEN Inc(BAsmCode[2],$10)
	    ELSE BAsmCode[3]:=HReg;
	   END;
	  ModMem:
	   IF HReg<>1 THEN WrError(1350)
	   ELSE
	    BEGIN
	     IF OpSize=-1 THEN OpSize:=0;
	     CodeLen:=2+AdrCnt;
	     BAsmCode[0]:=$80+(OpSize SHL 4)+AdrMode;
	     Move(AdrVals,BAsmCode[1],AdrCnt);
	     BAsmCode[1+AdrCnt]:=$78+z;
	    END;
	  END;
	 END;
       END;
      Exit;
     END;

   FOR z:=0 TO MulDivOrderCnt-1 DO
    IF Memo(MulDivOrders[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],MModReg+MModXReg);
	IF OpSize=0 THEN WrError(1130)
	ELSE
	 BEGIN
	  IF (AdrType=ModReg) AND (OpSize=1) THEN
	   IF AdrMode>3 THEN
	    BEGIN
	     AdrType:=ModXReg; AdrMode:=$e0+(AdrMode SHL 2);
	    END
	   ELSE Inc(AdrMode,1+AdrMode);
	  Dec(OpSize);
	  HReg:=AdrMode;
	  CASE AdrType OF
	  ModReg:
	   BEGIN
	    DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem+MModImm);
	    CASE AdrType OF
	    ModReg:
	     BEGIN
	      CodeLen:=2;
	      BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	      BAsmCode[1]:=$40+(z SHL 3)+HReg;
	     END;
	    ModXReg:
	     BEGIN
	      CodeLen:=3;
	      BAsmCode[0]:=$c7+(OpSize SHL 4);
	      BAsmCode[1]:=AdrMode;
	      BAsmCode[2]:=$40+(z SHL 3)+HReg;
	     END;
	    ModMem:
	     BEGIN
	      CodeLen:=2+AdrCnt;
	      BAsmCode[0]:=$80+(OpSize SHL 4)+AdrMode;
	      Move(AdrVals,BAsmCode[1],AdrCnt);
	      BAsmCode[1+AdrCnt]:=$40+(z SHL 3)+HReg;
	     END;
	    ModImm:
	     BEGIN
	      CodeLen:=2+AdrCnt;
	      BAsmCode[0]:=$c8+(OpSize SHL 4)+HReg;
	      BAsmCode[1]:=$08+z;
	      Move(AdrVals,BAsmCode[2],AdrCnt);
	     END;
	    END;
	   END;
	  ModXReg:
	   BEGIN
	    DecodeAdr(ArgStr[2],MModImm);
	    IF AdrType=ModImm THEN
	     BEGIN
	      CodeLen:=3+AdrCnt;
	      BAsmCode[0]:=$c7+(OpSize SHL 4);
	      BAsmCode[1]:=HReg;
	      BAsmCode[2]:=$08+z;
	      Move(AdrVals,BAsmCode[3],AdrCnt);
	     END;
	   END;
	  END;
	 END;
       END;
      Exit;
     END;

   IF Memo('MULA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModXReg);
       IF (AdrType<>ModNone) AND (OpSize<>2) THEN WrError(1130)
       ELSE CASE AdrType OF
       ModReg:
	BEGIN
	 CodeLen:=2;
	 BAsmCode[0]:=$d8+AdrMode;
	 BAsmCode[1]:=$19;
	END;
       ModXReg:
	BEGIN
	 CodeLen:=3;
	 BAsmCode[0]:=$d7;
	 BAsmCode[1]:=AdrMode;
	 BAsmCode[2]:=$19;
	END;
       END;
      END;
     Exit;
    END;

   FOR z:=1 TO BitCFOrderCnt DO
    WITH BitCFOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem);
	 IF AdrType<>ModNone THEN
	  IF OpSize=2 THEN WrError(1130)
	  ELSE
	   BEGIN
	    IF AdrType=ModMem THEN OpSize:=0;
            IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN
	     BEGIN
	      HReg:=$ff; OK:=True;
	     END
	    ELSE
	     BEGIN
	      FirstPassUnknown:=False;
	      IF OpSize=0 THEN
	       HReg:=EvalIntExpression(ArgStr[1],UInt3,OK)
	      ELSE
	       HReg:=EvalIntExpression(ArgStr[1],Int4,OK);
	     END;
	    IF OK THEN
	     CASE AdrType OF
	     ModReg:
	      BEGIN
	       CodeLen:=2+Ord(HReg<>$ff);
	       BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	       BAsmCode[1]:=$20+(Ord(HReg=$ff) SHL 3)+Code;
	       IF HReg<>$ff THEN BAsmCode[2]:=HReg;
	      END;
	     ModXReg:
	      BEGIN
	       CodeLen:=3+Ord(HReg<>$ff);
	       BAsmCode[0]:=$c7+(OpSize SHL 4);
	       BAsmCode[1]:=AdrMode;
	       BAsmCode[2]:=$20+(Ord(HReg=$ff) SHL 3)+Code;
	       IF HReg<>$ff THEN BAsmCode[3]:=HReg;
	      END;
	     ModMem:
	      BEGIN
	       CodeLen:=2+AdrCnt;
	       BAsmCode[0]:=$b0+AdrMode;
	       Move(AdrVals,BAsmCode[1],AdrCnt);
	       IF HReg=$ff THEN BAsmCode[1+AdrCnt]:=$28+Code
	       ELSE BAsmCode[1+AdrCnt]:=$80+(Code SHL 3)+HReg;
	      END;
	     END;
	   END;
	END;
       Exit;
      END;

   FOR z:=0 TO BitOrderCnt-1 DO
    IF Memo(BitOrders[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem);
	IF AdrType=ModMem THEN OpSize:=0;
	IF AdrType<>ModNone THEN
	 IF OpSize=2 THEN WrError(1130)
	 ELSE
	  BEGIN
	   IF OpSize=0 THEN
	    HReg:=EvalIntExpression(ArgStr[1],UInt3,OK)
	   ELSE
	    HReg:=EvalIntExpression(ArgStr[1],Int4,OK);
	   IF OK THEN
	    CASE AdrType OF
	    ModReg:
	     BEGIN
	      CodeLen:=3;
	      BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	      BAsmCode[1]:=$30+z;
	      BAsmCode[2]:=HReg;
	     END;
	    ModXReg:
	     BEGIN
	      CodeLen:=4;
	      BAsmCode[0]:=$c7+(OpSize SHL 4);
	      BAsmCode[1]:=AdrMode;
	      BAsmCode[2]:=$30+z;
	      BAsmCode[3]:=HReg;
	     END;
	    ModMem:
	     BEGIN
	      CodeLen:=2+AdrCnt;
	      IF z=4 THEN z:=0 ELSE Inc(z);
	      BAsmCode[0]:=$b0+AdrMode;
	      Move(AdrVals,BAsmCode[1],AdrCnt);
	      BAsmCode[1+AdrCnt]:=$a8+(z SHL 3)+HReg;
	     END;
	    END;
	  END;
       END;
      Exit;
     END;

   IF (Memo('CALL')) OR (Memo('JP')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN z:=DefaultCondition
       ELSE
	BEGIN
         z:=1; NLS_UpString(ArgStr[1]);
	 WHILE (z<=ConditionCnt) AND (Conditions[z].Name<>ArgStr[1]) DO Inc(z);
	END;
       IF z>ConditionCnt THEN WrError(1360)
       ELSE
	BEGIN
	 OpSize:=2;
	 DecodeAdr(ArgStr[ArgCnt],MModMem+MModImm);
	 IF AdrType=ModImm THEN
	  IF AdrVals[3]<>0 THEN
	   BEGIN
	    WrError(1320); AdrType:=ModNone;
	   END
	  ELSE IF AdrVals[2]<>0 THEN
	   BEGIN
	    AdrType:=ModMem; AdrMode:=$42; AdrCnt:=3;
	   END
	  ELSE
	   BEGIN
	    AdrType:=ModMem; AdrMode:=$41; AdrCnt:=2;
	   END;
	 IF AdrType=ModMem THEN
	  IF (z=DefaultCondition) AND ((AdrMode=$41) OR (AdrMode=$42)) THEN
	   BEGIN
	    CodeLen:=1+AdrCnt;
	    BAsmCode[0]:=$1a+2*Ord(Memo('CALL'))+(AdrCnt-2);
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2+AdrCnt;
	    BAsmCode[0]:=$b0+AdrMode;
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	    BAsmCode[1+AdrCnt]:=$d0+(Ord(Memo('CALL')) SHL 4)+(Conditions[z].Code);
	   END;
	END;
      END;
     Exit;
    END;

   IF (Memo('JR')) OR (Memo('JRL')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN z:=DefaultCondition
       ELSE
	BEGIN
         z:=1; NLS_UpString(ArgStr[1]);
	 WHILE (z<=ConditionCnt) AND (Conditions[z].Name<>ArgStr[1]) DO Inc(z);
	END;
       IF z>ConditionCnt THEN WrError(1360)
       ELSE
	BEGIN
	 AdrLong:=EvalIntExpression(ArgStr[ArgCnt],Int32,OK);
	 IF OK THEN
	  IF Memo('JRL') THEN
	   BEGIN
	    Dec(AdrLong,EProgCounter+3);
            IF ((AdrLong>32767) OR (AdrLong<-32768)) AND (NOT SymbolQuestionable) THEN WrError(1330)
	    ELSE
	     BEGIN
	      CodeLen:=3; BAsmCode[0]:=$70+Conditions[z].Code;
	      BAsmCode[1]:=Lo(AdrLong); BAsmCode[2]:=Hi(AdrLong);
	      IF NOT FirstPassUnknown THEN
	       BEGIN
		Inc(AdrLong);
		IF (AdrLong>=-128) AND (AdrLong<=127) THEN WrError(20);
	       END;
	     END;
	   END
	  ELSE
	   BEGIN
	    Dec(AdrLong,EProgCounter+2);
            IF ((AdrLong>127) OR (AdrLong<-128)) AND (NOT SymbolQuestionable) THEN WrError(1330)
	    ELSE
	     BEGIN
	      CodeLen:=2; BAsmCode[0]:=$60+Conditions[z].Code;
	      BAsmCode[1]:=Lo(AdrLong);
	     END;
	   END;
	END;
      END;
     Exit;
    END;

   IF Memo('CALR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],Int32,OK)-(EProgCounter+3);
       IF OK THEN
        IF ((AdrLong<-32768) OR (AdrLong>32767)) AND (NOT SymbolQuestionable) THEN WrError(1330)
	ELSE
	 BEGIN
	  CodeLen:=3; BAsmCode[0]:=$1e;
	  BAsmCode[1]:=Lo(AdrLong); BAsmCode[2]:=Hi(AdrLong);
	 END;
      END;
     Exit;
    END;

   IF Memo('RET') THEN
    BEGIN
     IF ArgCnt>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=0 THEN z:=DefaultCondition
       ELSE
	BEGIN
         z:=1; NLS_UpString(ArgStr[1]);
	 WHILE (z<=ConditionCnt) AND (Conditions[z].Name<>ArgStr[1]) DO Inc(z);
	END;
       IF z>ConditionCnt THEN WrError(1360)
       ELSE IF z=DefaultCondition THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$0e;
	END
       ELSE
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$b0;
	 BAsmCode[1]:=$f0+Conditions[z].Code;
	END;
      END;
     Exit;
    END;

   IF Memo('RETD') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$0f;
	 BAsmCode[1]:=Lo(AdrWord); BAsmCode[2]:=Hi(AdrWord);
	END;
      END;
     Exit;
    END;

   IF Memo('DJNZ') THEN
    BEGIN
     IF (ArgCnt<>2) AND (ArgCnt<>1) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 AdrType:=ModReg; AdrMode:=2; OpSize:=0;
	END
       ELSE DecodeAdr(ArgStr[1],MModReg+MModXReg);
       IF AdrType<>ModNone THEN
	IF OpSize=2 THEN WrError(1130)
	ELSE
	 BEGIN
	  AdrLong:=EvalIntExpression(ArgStr[ArgCnt],Int32,OK)-(EProgCounter+3+Ord(AdrType=ModXReg));
	  IF OK THEN
           IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
	   ELSE CASE AdrType OF
	   ModReg:
	    BEGIN
	     CodeLen:=3;
	     BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	     BAsmCode[1]:=$1c;
	     BAsmCode[2]:=AdrLong AND $ff;
	    END;
	   ModXReg:
	    BEGIN
	     CodeLen:=4;
	     BAsmCode[0]:=$c7+(OpSize SHL 4);
	     BAsmCode[1]:=AdrMode;
	     BAsmCode[2]:=$1c;
	     BAsmCode[3]:=AdrLong AND $ff;
	    END;
	   END;
	 END;
      END;
     Exit;
    END;

   IF Memo('EX') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (ArgPair('F','F''')) OR (ArgPair('F`','F')) THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$16;
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModXReg+MModMem);
       IF OpSize=2 THEN WrError(1130)
       ELSE CASE AdrType OF
       ModReg:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg+MModXReg+MModMem);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=2;
	   BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	   BAsmCode[1]:=$b8+HReg;
	  END;
	 ModXReg:
	  BEGIN
	   CodeLen:=3;
	   BAsmCode[0]:=$c7+(OpSize SHL 4);
	   BAsmCode[1]:=AdrMode;
	   BAsmCode[2]:=$b8+HReg;
	  END;
	 ModMem:
	  BEGIN
	   CodeLen:=2+AdrCnt;
	   BAsmCode[0]:=$80+(OpSize SHL 4)+AdrMode;
	   Move(AdrVals,BAsmCode[1],AdrCnt);
	   BAsmCode[1+AdrCnt]:=$30+HReg;
	  END;
	 END;
	END;
       ModXReg:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg);
	 IF AdrType=ModReg THEN
	  BEGIN
	   CodeLen:=3;
	   BAsmCode[0]:=$c7+(OpSize SHL 4);
	   BAsmCode[1]:=HReg;
	   BAsmCode[2]:=$b8+AdrMode;
	  END;
	END;
       ModMem:
	BEGIN
	 MinOneIs0:=True;
	 HReg:=AdrCnt; BAsmCode[0]:=AdrMode;
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 DecodeAdr(ArgStr[2],MModReg);
	 IF AdrType=ModReg THEN
	  BEGIN
	   CodeLen:=2+HReg; CorrMode(0,1);
	   Inc(BAsmCode[0],$80+(OpSize SHL 4));
	   BAsmCode[1+HReg]:=$30+AdrMode;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF (WMemo('INC')) OR (WMemo('DEC')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 HReg:=1; OK:=True;
	END
       ELSE HReg:=EvalIntExpression(ArgStr[1],Int4,OK);
       IF OK THEN
	IF FirstPassUnknown THEN HReg:=HReg AND 7
	ELSE IF (HReg<1) OR (HReg>8) THEN
	 BEGIN
	  WrError(1320); OK:=False;
	 END;
       IF OK THEN
	BEGIN
	 HReg:=HReg AND 7;    { 8-->0 }
	 DecodeAdr(ArgStr[ArgCnt],MModReg+MModXReg+MModMem);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=2;
	   BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
           BAsmCode[1]:=$60+(Ord(WMemo('DEC')) SHL 3)+HReg;
	  END;
	 ModXReg:
	  BEGIN
	   CodeLen:=3;
	   BAsmCode[0]:=$c7+(OpSize SHL 4);
	   BAsmCode[1]:=AdrMode;
           BAsmCode[2]:=$60+(Ord(WMemo('DEC')) SHL 3)+HReg;
	  END;
	 ModMem:
	  BEGIN
           IF OpSize=-1 THEN OpSize:=0;
	   CodeLen:=2+AdrCnt;
           BAsmCode[0]:=$80+AdrMode+(OpSize SHL 4);
	   Move(AdrVals,BAsmCode[1],AdrCnt);
           BAsmCode[1+AdrCnt]:=$60+(Ord(WMemo('DEC')) SHL 3)+HReg;
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   IF (Memo('BS1B')) OR (Memo('BS1F')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1135)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg+MModXReg);
       IF OpSize<>1 THEN WrError(1130)
       ELSE CASE AdrType OF
       ModReg:
	BEGIN
	 CodeLen:=2;
	 BAsmCode[0]:=$d8+AdrMode;
	 BAsmCode[1]:=$0e+Ord(Memo('BS1B'));
	END;
       ModXReg:
	BEGIN
	 CodeLen:=3;
	 BAsmCode[0]:=$d7;
	 BAsmCode[1]:=AdrMode;
	 BAsmCode[2]:=$0e+Ord(Memo('BS1B'));
	END;
       END;
      END;
     Exit;
    END;

   IF (Memo('CPD')) OR (Memo('CPDR')) OR (Memo('CPI')) OR (Memo('CPIR')) THEN
    BEGIN
     IF (ArgCnt<>0) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=0 THEN
	BEGIN
	 OK:=True; OpSize:=0; AdrMode:=3;
	END
       ELSE
	BEGIN
	 OK:=True;
         IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN OpSize:=0
         ELSE IF NLS_StrCaseCmp(ArgStr[1],'WA')=0 THEN OpSize:=1;
	 IF OpPart[3]='I' THEN CmpStr:='+)' ELSE CmpStr:='-)';
	 IF OpSize=-1 THEN OK:=False
         ELSE IF (ArgStr[2][1]<>'(') OR (NLS_StrCaseCmp(Copy(ArgStr[2],Length(ArgStr[2])-1,2),CmpStr)<>0) THEN OK:=False
	 ELSE IF CodeEReg(Copy(ArgStr[2],2,Length(ArgStr[2])-3),AdrMode,HReg)<>2 THEN OK:=False
	 ELSE IF NOT IsRegBase(AdrMode,HReg) THEN OK:=False
	 ELSE IF NOT IsRegCurrent(AdrMode,HReg,AdrMode) THEN OK:=False;
	 IF NOT OK THEN WrError(1135);
	END;
       IF OK THEN
	BEGIN
	 CodeLen:=2;
	 BAsmCode[0]:=$80+(OpSize SHL 4)+AdrMode;
	 BAsmCode[1]:=$14+(Ord(OpPart[3]='D') SHL 1)+(Length(OpPart)-3);
	END;
      END;
     Exit;
    END;

   IF (WMemo('LDD')) OR (WMemo('LDDR')) OR (WMemo('LDI')) OR (WMemo('LDIR')) THEN
    BEGIN
     IF OpSize=-1 THEN OpSize:=0;
     IF OpSize=2 THEN WrError(1130)
     ELSE IF (ArgCnt<>0) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=0 THEN
	BEGIN
	 OK:=True; HReg:=0;
	END
       ELSE
	BEGIN
	 OK:=True;
	 IF OpPart[3]='I' THEN CmpStr:='+)' ELSE CmpStr:='-)';
	 IF (ArgStr[1][1]<>'(') OR (ArgStr[2][1]<>'(') OR
            (NLS_StrCaseCmp(Copy(ArgStr[1],Length(ArgStr[1])-1,2),CmpStr)<>0) OR
            (NLS_StrCaseCmp(Copy(ArgStr[2],Length(ArgStr[2])-1,2),CmpStr)<>0) THEN OK:=False
	 ELSE
	  BEGIN
	   ArgStr[1]:=Copy(ArgStr[1],2,Length(ArgStr[1])-3);
	   ArgStr[2]:=Copy(ArgStr[2],2,Length(ArgStr[2])-3);
           IF (NLS_StrCaseCmp(ArgStr[1],'XIX')=0) AND (NLS_StrCaseCmp(ArgStr[2],'XIY')=0) THEN HReg:=2
           ELSE IF (Maximum) AND (NLS_StrCaseCmp(ArgStr[1],'XDE')=0) AND (NLS_StrCaseCmp(ArgStr[2],'XHL')=0) THEN HReg:=0
           ELSE IF (NOT Maximum) AND (NLS_StrCaseCmp(ArgStr[1],'DE')=0) AND (NLS_StrCaseCmp(ArgStr[2],'HL')=0) THEN HReg:=0
	   ELSE OK:=False;
	  END;
	END;
       IF NOT OK THEN WrError(1350)
       ELSE
	BEGIN
	 CodeLen:=2;
	 BAsmCode[0]:=$83+(OpSize SHL 4)+HReg;
	 BAsmCode[1]:=$10+(Ord(OpPart[3]='D') SHL 1)+Ord(Pos('R',OpPart)<>0);
	END;
      END;
     Exit;
    END;

   IF Memo('LDA') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrType<>ModNone THEN
	IF OpSize<1 THEN WrError(1130)
	ELSE
	 BEGIN
	  HReg:=AdrMode;
	  DecodeAdr(ArgStr[2],MModMem);
	  IF AdrType<>ModNone THEN
	   BEGIN
	    CodeLen:=2+AdrCnt;
	    BAsmCode[0]:=$b0+AdrMode;
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	    BAsmCode[1+AdrCnt]:=$20+((OpSize-1) SHL 4)+HReg;
	   END;
	 END;
      END;
     Exit;
    END;

   IF Memo('LDAR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[2],Int32,OK)-(EProgCounter+4);
       IF OK THEN
        IF ((AdrLong<-32768) OR (AdrLong>32767)) AND (NOT SymbolQuestionable) THEN WrError(1330)
	ELSE
	 BEGIN
	  DecodeAdr(ArgStr[1],MModReg);
	  IF AdrType<>ModNone THEN
	   IF OpSize<1 THEN WrError(1130)
	   ELSE
	    BEGIN
	     CodeLen:=5;
	     BAsmCode[0]:=$f3; BAsmCode[1]:=$13;
	     BAsmCode[2]:=Lo(AdrLong); BAsmCode[3]:=Hi(AdrLong);
	     BAsmCode[4]:=$20+((OpSize-1) SHL 4)+AdrMode;
	    END;
	 END;
      END;
     Exit;
    END;

   IF Memo('LDC') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModXReg+MModCReg);
       HReg:=AdrMode;
       CASE AdrType OF
       ModReg:
	BEGIN
	 DecodeAdr(ArgStr[2],MModCReg);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=3;
	   BAsmCode[0]:=$c8+(OpSize SHL 4)+HReg;
	   BAsmCode[1]:=$2f;
	   BAsmCode[2]:=AdrMode;
	  END;
	END;
       ModXReg:
	BEGIN
	 DecodeAdr(ArgStr[2],MModCReg);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=4;
	   BAsmCode[0]:=$c7+(OpSize SHL 4);
	   BAsmCode[1]:=HReg;
	   BAsmCode[2]:=$2f;
	   BAsmCode[3]:=AdrMode;
	  END;
	END;
       ModCReg:
	BEGIN
	 DecodeAdr(ArgStr[2],MModReg+MModXReg);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=3;
	   BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	   BAsmCode[1]:=$2e;
	   BAsmCode[2]:=HReg;
	  END;
	 ModXReg:
	  BEGIN
	   CodeLen:=4;
	   BAsmCode[0]:=$c7+(OpSize SHL 4);
	   BAsmCode[1]:=AdrMode;
	   BAsmCode[2]:=$2e;
	   BAsmCode[3]:=HReg;
	  END;
	 END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('LDX') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModMem);
       IF AdrType<>ModNone THEN
	IF AdrMode<>$40 THEN WrError(1350)
	ELSE
	 BEGIN
	  BAsmCode[4]:=EvalIntExpression(ArgStr[2],Int8,OK);
	  IF OK THEN
	   BEGIN
	    CodeLen:=6;
	    BAsmCode[0]:=$f7; BAsmCode[1]:=0;
	    BAsmCode[2]:=AdrVals[0]; BAsmCode[3]:=0;
	    BAsmCode[5]:=0;
	   END;
	 END;
      END;
     Exit;
    END;

   IF Memo('LINK') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[2],Int16,OK);
       IF OK THEN
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg+MModXReg);
	 IF (AdrType<>ModNone) AND (OpSize<>2) THEN WrError(1130)
	 ELSE CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=4;
	   BAsmCode[0]:=$e8+AdrMode;
	   BAsmCode[1]:=$0c;
	   BAsmCode[2]:=Lo(AdrWord);
	   BAsmCode[3]:=Hi(AdrWord);
	  END;
	 ModXReg:
	  BEGIN
	   CodeLen:=5;
	   BAsmCode[0]:=$e7;
	   BAsmCode[1]:=AdrMode;
	   BAsmCode[2]:=$0c;
	   BAsmCode[3]:=Lo(AdrWord);
	   BAsmCode[4]:=Hi(AdrWord);
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   LChar:=OpPart[Length(OpPart)];
   IF ((Copy(OpPart,1,4)='MDEC') OR (Copy(OpPart,1,4)='MINC')) AND (LChar>='1') AND (LChar<='4') THEN
    BEGIN
     IF LChar='3' THEN WrError(1135)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF NOT IsPwr2(AdrWord,HReg) THEN WrError(1135)
	ELSE IF (HReg=0) OR ((LChar='2') AND (HReg<2)) OR ((LChar='4') AND (HReg<3)) THEN WrError(1135)
	ELSE
	 BEGIN
	  Dec(AdrWord,Ord(LChar)-AscOfs);
	  IF IsPwr2(Ord(LChar)-AscOfs,HReg) THEN;
	  DecodeAdr(ArgStr[2],MModReg+MModXReg);
	  IF (AdrType<>ModNone) AND (OpSize<>1) THEN WrError(1130)
	  ELSE CASE AdrType OF
	  ModReg:
	   BEGIN
	    CodeLen:=4;
	    BAsmCode[0]:=$d8+AdrMode;
	    BAsmCode[1]:=$38+(Ord(OpPart[2]='D') SHL 2)+HReg;
	    BAsmCode[2]:=Lo(AdrWord);
	    BAsmCode[3]:=Hi(AdrWord);
	   END;
	  ModXReg:
	   BEGIN
	    CodeLen:=5;
	    BAsmCode[0]:=$d7;
	    BAsmCode[1]:=AdrMode;
	    BAsmCode[2]:=$38+(Ord(OpPart[2]='D') SHL 2)+HReg;
	    BAsmCode[3]:=Lo(AdrWord);
	    BAsmCode[4]:=Hi(AdrWord);
	   END;
	  END;
	 END;
      END;
     Exit;
    END;

   IF (Memo('RLD')) OR (Memo('RRD')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE IF (ArgCnt=2) AND (NLS_StrCaseCmp(ArgStr[1],'A')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[ArgCnt],MModMem);
       IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=2+AdrCnt;
	 BAsmCode[0]:=$80+AdrMode;
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 BAsmCode[1+AdrCnt]:=$06+Ord(Memo('RRD'));
	END;
      END;
     Exit;
    END;

   IF Memo('SCC') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       z:=1; NLS_UpString(ArgStr[1]);
       WHILE (z<=ConditionCnt) AND (Conditions[z].Name<>ArgStr[1]) DO Inc(z);
       IF z>ConditionCnt THEN WrError(1360)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2],MModReg+MModXReg);
	 IF OpSize>1 THEN WrError(1110)
	 ELSE CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=2;
	   BAsmCode[0]:=$c8+(OpSize SHL 4)+AdrMode;
	   BAsmCode[1]:=$70+Conditions[z].Code;
	  END;
	 ModXReg:
	  BEGIN
	   CodeLen:=3;
	   BAsmCode[0]:=$c7+(OpSize SHL 4);
	   BAsmCode[1]:=AdrMode;
	   BAsmCode[2]:=$70+Conditions[z].Code;
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_96C141:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : IF Maximum THEN ok:=ProgCounter<=$ffffff
	      ELSE ok:=ProgCounter<=$ffff;
   ELSE ok:=False;
   END;
   ChkPC_96C141:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_96C141:Boolean;
	Far;
BEGIN
   IsDef_96C141:=False;
END;

        PROCEDURE SwitchFrom_96C141;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_96C141;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=True;

   PCSymbol:='$'; HeaderID:=$52; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_96C141; ChkPC:=ChkPC_96C141; IsDef:=IsDef_96C141;
   SwitchFrom:=SwitchFrom_96C141;

   InitFields;
END;

BEGIN
   CPU96C141:=AddCPU('96C141',SwitchTo_96C141);
   CPU93C141:=AddCPU('93C141',SwitchTo_96C141);
END.
