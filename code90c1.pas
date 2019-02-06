{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
        UNIT Code90C1;

INTERFACE

        Uses NLS,
             AsmDef,AsmSub,AsmPars,InstTree,CodePseu;



IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Code:Byte;
	      END;

   ShiftOrder=RECORD
	       Code:Byte;
	       MayReg:Boolean;
	      END;

   Condition=RECORD
	      Name:String[3];
	      Code:Byte;
	     END;

CONST
   AccReg=6;
   HLReg=2;

   FixedOrderCnt=18;

   MoveOrderCnt=8;

   ShiftOrderCnt=10;

   BitOrderCnt=4;

   AccOrderCnt=3;

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
   ModReg8   = 0;  MModReg8   = 1 SHL ModReg8;
   ModReg16  = 1;  MModReg16  = 1 SHL ModReg16;
   ModIReg16 = 2;  MModIReg16 = 1 SHL ModIReg16;
   ModIndReg = 3;  MModIndReg = 1 SHL ModIndReg;
   ModIdxReg = 4;  MModIdxReg = 1 SHL ModIdxReg;
   ModDir    = 5;  MModDir    = 1 SHL ModDir;
   ModMem    = 6;  MModMem    = 1 SHL ModMem;
   ModImm    = 7;  MModImm    = 1 SHL ModImm;

   ALU2OrderCnt=8;
   ALU2Orders:ARRAY[1..ALU2OrderCnt] OF String[3]=
	      ('ADD','ADC','SUB','SBC','AND','XOR','OR','CP');

TYPE
   FixedOrderArray=ARRAY[1..FixedOrderCnt] OF FixedOrder;
   MoveOrderArray=ARRAY[1..MoveOrderCnt] OF FixedOrder;
   ShiftOrderArray=ARRAY[1..ShiftOrderCnt] OF ShiftOrder;
   BitOrderArray=ARRAY[1..BitOrderCnt] OF FixedOrder;
   AccOrderArray=ARRAY[1..AccOrderCnt] OF FixedOrder;

VAR
   AdrType:ShortInt;
   AdrMode:Byte;
   AdrCnt:Byte;
   OpSize:ShortInt;
   AdrVals:ARRAY[0..9] OF Byte;
   MinOneIs0:Boolean;

   FixedOrders:^FixedOrderArray;
   MoveOrders:^MoveOrderArray;
   ShiftOrders:^ShiftOrderArray;
   BitOrders:^BitOrderArray;
   AccOrders:^AccOrderArray;
   ITree:PInstTreeNode;

   CPU90C141:CPUVar;

	PROCEDURE DecodeAdr(Asc:String; Erl:Byte);
LABEL
   AdrFnd;

CONST
   Reg8Cnt=7;
   Reg8Names:ARRAY[1..Reg8Cnt] OF Char='BCDEHLA';
   Reg16Cnt=7;
   Reg16Names:ARRAY[1..Reg16Cnt] OF String[2]=('BC','DE','HL',#0#0,'IX','IY','SP');
   IReg16Cnt=3;
   IReg16Names:ARRAY[1..IReg16Cnt] OF String[2]=('IX','IY','SP');

VAR
   z:Integer;
   p,ppos,mpos:Integer;
   DispAcc,DispVal:LongInt;
   OccFlag,BaseReg:Byte;
   ok,fnd,NegFlag,NNegFlag,Unknown:Boolean;
   Part:String;


	PROCEDURE SetOpSize(New:ShortInt);
BEGIN
   IF OpSize=-1 THEN OpSize:=New
   ELSE IF OpSize<>New THEN
    BEGIN
     WrError(1131); AdrType:=ModNone; AdrCnt:=0;
    END;
END;

BEGIN
   AdrType:=ModNone; AdrCnt:=0;

   { 1. 8-Bit-Register }

   FOR z:=1 TO Reg8Cnt DO
    IF NLS_StrCaseCmp(Asc,Reg8Names[z])=0 THEN
     BEGIN
      AdrType:=ModReg8; AdrMode:=z-1; SetOpSize(0);
      Goto AdrFnd;
     END;

   { 2. 16-Bit-Register, indiziert }

   IF Erl AND MModIReg16<>0 THEN
    FOR z:=1 TO IReg16Cnt DO
     IF NLS_StrCaseCmp(Asc,IReg16Names[z])=0 THEN
      BEGIN
       AdrType:=ModIReg16; AdrMode:=z-1; SetOpSize(1);
       Goto AdrFnd;
      END;

   { 3. 16-Bit-Register, normal }

   FOR z:=1 TO Reg16Cnt DO
    IF NLS_StrCaseCmp(Asc,Reg16Names[z])=0 THEN
     BEGIN
      AdrType:=ModReg16; AdrMode:=z-1; SetOpSize(1);
      Goto AdrFnd;
     END;

   { Speicheradresse }

   IF IsIndirect(Asc) THEN
    BEGIN
     OccFlag:=0; DispAcc:=0; ok:=True; NegFlag:=False; Unknown:=False;
     Asc:=Copy(Asc,2,Length(Asc)-2);
     REPEAT
      ppos:=QuotPos(Asc,'+');
      mpos:=QuotPos(Asc,'-');
      NNegFlag:=mpos<ppos; 
      if NNegFlag THEN p:=mpos ELSE p:=ppos;
      SplitString(Asc,Part,Asc,p);
      fnd:=False;
      IF NLS_StrCaseCmp(Part,'A')=0 THEN
       BEGIN
	fnd:=True;
	ok:=(NOT NegFlag) AND (OccFlag AND 1=0);
	IF ok THEN Inc(OccFlag,1) ELSE WrError(1350);
       END;
      IF NOT fnd THEN
       FOR z:=1 TO Reg16Cnt DO
  IF NLS_StrCaseCmp(Part,Reg16Names[z])=0 THEN
	 BEGIN
	  fnd:=True; BaseReg:=z-1;
	  ok:=(NOT NegFlag) AND (OccFlag AND 2=0);
	  IF OK THEN Inc(OccFlag,2) ELSE WrError(1350);
	 END;
      IF NOT fnd THEN
       BEGIN
	FirstPassUnknown:=False;
	DispVal:=EvalIntExpression(Part,Int32,ok);
	IF ok THEN
	 BEGIN
	  IF NegFlag THEN Dec(DispAcc,DispVal) ELSE Inc(DispAcc,DispVal);
	  IF FirstPassUnknown THEN Unknown:=True;
	 END;
       END;
      NegFlag:=NNegFlag;
     UNTIL (Asc='') OR (NOT ok);
     IF NOT ok THEN Exit;
     IF Unknown THEN DispAcc:=DispAcc AND $7f;
     CASE OccFlag OF
     1:WrError(1350);
     3:IF (BaseReg<>2) OR (DispAcc<>0) THEN WrError(1350)
       ELSE
	BEGIN
	 AdrType:=ModIdxReg; AdrMode:=3;
	END;
     2:IF (DispAcc>127) OR (DispAcc<-128) THEN WrError(1320)
       ELSE IF DispAcc=0 THEN
	BEGIN
	 AdrType:=ModIndReg; AdrMode:=BaseReg;
	END
       ELSE IF BaseReg<4 THEN WrError(1350)
       ELSE
	BEGIN
	 AdrType:=ModIdxReg; AdrMode:=BaseReg-4;
	 AdrCnt:=1; AdrVals[0]:=DispAcc AND $ff;
	END;
     0:IF (DispAcc>$ffff) THEN WrError(1925)
       ELSE IF (Hi(DispAcc)=$ff) AND (Erl AND MModDir<>0) THEN
	BEGIN
	 AdrType:=ModDir; AdrCnt:=1; AdrVals[0]:=Lo(DispAcc);
	END
       ELSE
	BEGIN
	 AdrType:=ModMem; AdrCnt:=2; AdrVals[0]:=Lo(DispAcc);
	 AdrVals[1]:=Hi(DispAcc);
	END;
     END;
    END

   { immediate }

   ELSE
    BEGIN
     IF (OpSize=-1) AND (MinOneIs0) THEN OpSize:=0;
     CASE OpSize OF
     -1:WrError(1130);
     0:BEGIN
	AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
	IF OK THEN
	 BEGIN
	  AdrType:=ModImm; AdrCnt:=1;
	 END;
       END;
     1:BEGIN
	DispVal:=EvalIntExpression(Asc,Int16,OK);
	IF OK THEN
	 BEGIN
	  AdrType:=ModImm; AdrCnt:=2;
	  AdrVals[0]:=Lo(DispVal); AdrVals[1]:=Hi(DispVal);
	 END;
       END;
     END;
    END;

   { gefunden }

AdrFnd:
   IF AdrType<>ModNone THEN
    IF (1 SHL AdrType) AND Erl=0 THEN
     BEGIN
      WrError(1350); AdrType:=ModNone;
     END;
END;

{-----------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
VAR
   HLocHandle:LongInt;
   t:TempResult;
   OK:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

	FUNCTION WMemo(Name:String):Boolean;
BEGIN
   IF Memo(Name) THEN WMemo:=True
   ELSE
    BEGIN
     Name:=Name+'W';
     IF Memo(Name) THEN
      BEGIN
       OpSize:=1; WMemo:=True;
      END
     ELSE WMemo:=False;
    END;
END;

	FUNCTION ArgPair(Arg1,Arg2:String):Boolean;
BEGIN
   ArgPair:=((NLS_StrCaseCmp(ArgStr[1],Arg1)=0) AND (NLS_StrCaseCmp(ArgStr[2],Arg2)=0))
   OR ((NLS_StrCaseCmp(ArgStr[1],Arg2)=0) AND (NLS_StrCaseCmp(ArgStr[2],Arg1)=0));
END;

{---------------------------------------------------------------------------}

{ ohne Argument }

	PROCEDURE CodeFixed(Index:Word);
        Far;
BEGIN
   WITH FixedOrders^[Index] DO
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=Code;
      END;
    END;
END;

        PROCEDURE CodeMove(Index:Word);
        Far;
BEGIN
   WITH MoveOrders^[Index] DO
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; BAsmCode[0]:=$fe; BAsmCode[1]:=Code;
      END;
    END;
END;

        PROCEDURE CodeShift(Index:Word);
        Far;
BEGIN
   WITH ShiftOrders^[Index] DO
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF MayReg THEN DecodeAdr(ArgStr[1],MModReg8+MModIndReg+MModIdxReg+MModMem+MModDir)
       ELSE DecodeAdr(ArgStr[1],MModIndReg+MModIdxReg+MModMem+MModDir);
       CASE AdrType OF
       ModReg8:
        BEGIN
         CodeLen:=2; BAsmCode[0]:=$f8+AdrMode; BAsmCode[1]:=Code;
         IF AdrMode=AccReg THEN WrError(10);
        END;
       ModIndReg:
        BEGIN
         CodeLen:=2; BAsmCode[0]:=$e0+AdrMode; BAsmCode[1]:=Code;
        END;
       ModIdxReg:
        BEGIN
         CodeLen:=2+AdrCnt; BAsmCode[0]:=$f0+AdrMode;
         Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=Code;
        END;
       ModDir:
        BEGIN
         CodeLen:=3; BAsmCode[0]:=$e7; BAsmCode[1]:=AdrVals[0];
         BAsmCode[2]:=Code;
        END;
       ModMem:
        BEGIN
         CodeLen:=4; BAsmCode[0]:=$e3;
         Move(AdrVals,BAsmCode[1],AdrCnt);
         BAsmCode[3]:=Code;
        END;
       END;
      END;
    END;
END;

{ Logik }

        PROCEDURE CodeBit(Index:Word);
        Far;
VAR
   HReg:Byte;
   OK:Boolean;
BEGIN
   WITH BitOrders^[Index] DO
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       HReg:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF OK THEN
        BEGIN
         DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModMem+MModDir);
         CASE AdrType OF
         ModReg8:
          BEGIN
           CodeLen:=2;
           BAsmCode[0]:=$f8+AdrMode; BAsmCode[1]:=Code+HReg;
          END;
         ModIndReg:
          BEGIN
           CodeLen:=2;
           BAsmCode[0]:=$e0+AdrMode; BAsmCode[1]:=Code+HReg;
          END;
         ModIdxReg:
          BEGIN
           CodeLen:=2+AdrCnt; Move(AdrVals,BAsmCode[1],AdrCnt);
           BAsmCode[0]:=$f0+AdrMode; BAsmCode[1+AdrCnt]:=Code+HReg;
          END;
         ModMem:
          BEGIN
           CodeLen:=4; Move(AdrVals,BAsmCode[1],AdrCnt);
           BAsmCode[0]:=$e3; BAsmCode[1+AdrCnt]:=Code+HReg;
          END;
         ModDir:
          BEGIN
           BAsmCode[1]:=AdrVals[0];
           IF Index=4 THEN
            BEGIN
             BAsmCode[0]:=$e7; BAsmCode[2]:=Code+HReg; CodeLen:=3;
            END
           ELSE
            BEGIN
             BAsmCode[0]:=Code+HReg; CodeLen:=2;
            END;
          END;
         END;
        END;
      END;
    END;
END;

        PROCEDURE CodeAcc(Index:Word);
        Far;
BEGIN
   WITH AccOrders^[Index] DO
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=Code;
      END;
    END;
END;

        PROCEDURE MakeCode_90C141;
	Far;
VAR
   z:Integer;
   AdrInt:Integer;
   OK:Boolean;
   HReg:Byte;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1;

   { zu ignorierendes }

   IF Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   IF SearchInstTree(ITree) THEN Exit;


   { Datentransfer }

   IF WMemo('LD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem);
       CASE AdrType OF
       ModReg8:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
	 CASE AdrType OF
	 ModReg8:
	  IF HReg=AccReg THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$20+AdrMode;
	   END
	  ELSE IF AdrMode=AccReg THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$28+HReg;
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$f8+AdrMode; BAsmCode[1]:=$30+HReg;
	   END;
	 ModIndReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$e0+AdrMode; BAsmCode[1]:=$28+HReg;
	  END;
	 ModIdxReg:
	  BEGIN
	   CodeLen:=2+AdrCnt; BAsmCode[0]:=$f0+AdrMode;
	   Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=$28+HReg;
	  END;
	 ModDir:
	  IF HReg=AccReg THEN
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$27; BAsmCode[1]:=AdrVals[0];
	   END
	  ELSE
	   BEGIN
	    CodeLen:=3; BAsmCode[0]:=$e7; BAsmCode[1]:=AdrVals[0];
	    BAsmCode[2]:=$28+HReg;
	   END;
	 ModMem:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$e3;
	   Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[3]:=$28+HReg;
	  END;
	 ModImm:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$30+HReg; BAsmCode[1]:=AdrVals[0];
	  END;
	 END;
	END;
       ModReg16:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
	 CASE AdrType OF
	 ModReg16:
	  IF HReg=HLReg THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$40+AdrMode;
	   END
	  ELSE IF AdrMode=HLReg THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$48+HReg;
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$f8+AdrMode; BAsmCode[1]:=$38+HReg;
	   END;
	 ModIndReg:
	  BEGIN
	   CodeLen:=2;
	   BAsmCode[0]:=$e0+AdrMode; BAsmCode[1]:=$48+HReg;
	  END;
	 ModIdxReg:
	  BEGIN
	   CodeLen:=2+AdrCnt; BAsmCode[0]:=$f0+AdrMode;
	   Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=$48+HReg;
	  END;
	 ModDir:
	  IF HReg=HLReg THEN
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$47; BAsmCode[1]:=AdrVals[0];
	   END
	  ELSE
	   BEGIN
	    CodeLen:=3; BAsmCode[0]:=$e7; BAsmCode[1]:=AdrVals[0];
	    BAsmCode[2]:=$48+HReg;
	   END;
	 ModMem:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$e3; BAsmCode[3]:=$48+HReg;
	   Move(AdrVals,BAsmCode[1],AdrCnt);
	  END;
         ModImm:
          BEGIN
           CodeLen:=3; BAsmCode[0]:=$38+HReg;
           Move(AdrVals,BAsmCode[1],AdrCnt);
          END;
	 END;
	END;
       ModIndReg,ModIdxReg,ModDir,ModMem:
	BEGIN
	 MinOneIs0:=True; HReg:=AdrCnt; Move(AdrVals,BAsmCode[1],AdrCnt);
	 CASE AdrType OF
	 ModIndReg:BAsmCode[0]:=$e8+AdrMode;
	 ModIdxReg:BAsmCode[0]:=$f4+AdrMode;
	 ModMem:BAsmCode[0]:=$eb;
	 ModDir:BAsmCode[0]:=$0f;
	 END;
	 DecodeAdr(ArgStr[2],MModReg16+MModReg8+MModImm);
	 IF BAsmCode[0]=$0f THEN
	  CASE AdrType OF
	  ModReg8:
	   IF AdrMode=AccReg THEN
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$2f;
	    END
	   ELSE
	    BEGIN
	     CodeLen:=3; BAsmCode[0]:=$ef; BAsmCode[2]:=$20+AdrMode;
	    END;
	  ModReg16:
	   IF AdrMode=HLReg THEN
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$4f;
	    END
	   ELSE
	    BEGIN
	     CodeLen:=3; BAsmCode[0]:=$ef; BAsmCode[2]:=$40+AdrMode;
	    END;
	  ModImm:
	   BEGIN
	    CodeLen:=3+OpSize; BAsmCode[0]:=$37+(OpSize SHL 3);
	    Move(AdrVals,BAsmCode[2],AdrCnt);
	   END;
	  END
	 ELSE
	  BEGIN
	   CASE AdrType OF
	   ModReg8:BAsmCode[1+HReg]:=$20+AdrMode;
	   ModReg16:BAsmCode[1+HReg]:=$40+AdrMode;
	   ModImm:BAsmCode[1+HReg]:=$37+(OpSize SHL 3);
	   END;
	   Move(AdrVals,BAsmCode[2+HReg],AdrCnt);
	   CodeLen:=1+HReg+1+AdrCnt;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       HReg:=Ord(Memo('POP')) SHL 3;
       IF NLS_StrCaseCmp(ArgStr[1],'AF')=0 THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$56+HReg;
	END
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg16);
	 IF AdrType=ModReg16 THEN
	  IF AdrMode=6 THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$50+HReg+AdrMode;
	   END;
	END;
      END;
     Exit;
    END;

   IF Memo('LDA') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       IF AdrType=ModReg16 THEN
	BEGIN
	 HReg:=$38+AdrMode;
	 DecodeAdr(ArgStr[2],MModIndReg+MModIdxReg);
	 CASE AdrType OF
	 ModIndReg:
	  IF AdrMode<4 THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=3; BAsmCode[0]:=$f0+AdrMode;
	    BAsmCode[1]:=0; BAsmCode[2]:=HReg;
	   END;
	 ModIdxReg:
	  BEGIN
	   CodeLen:=2+AdrCnt; BAsmCode[0]:=$f4+AdrMode;
	   Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=HReg;
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   IF Memo('LDAR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'HL')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK)-(EProgCounter+2);
       IF OK THEN
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$17;
	 BAsmCode[1]:=Lo(AdrInt); BAsmCode[2]:=Hi(AdrInt);
	END;
      END;
     Exit;
    END;

   IF Memo('EX') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF ArgPair('DE','HL') THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$08;
      END
     ELSE IF (ArgPair('AF','AF''') OR ArgPair('AF','AF`')) THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$09;
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16+MModIndReg+MModIdxReg+MModMem+MModDir);
       CASE AdrType OF
       ModReg16:
	BEGIN
	 HReg:=$50+AdrMode;
	 DecodeAdr(ArgStr[2],MModIndReg+MModIdxReg+MModMem+MModDir);
	 CASE AdrType OF
	 ModIndReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$e0+AdrMode; BAsmCode[1]:=HReg;
	  END;
	 ModIdxReg:
	  BEGIN
	   CodeLen:=2+AdrCnt; BAsmCode[0]:=$f0+AdrMode;
	   Move(AdrVals,BAsmCode[1],AdrCnt);
	   BAsmCode[1+AdrCnt]:=HReg;
	  END;
	 ModDir:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$e7; BAsmCode[1]:=AdrVals[0];
	   BAsmCode[2]:=HReg;
	  END;
	 ModMem:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$e3; Move(AdrVals,BAsmCode[1],AdrCnt);
	   BAsmCode[3]:=HReg;
	  END;
	 END;
	END;
       ModIndReg,ModIdxReg,ModDir,ModMem:
	BEGIN
	 CASE AdrType OF
	 ModIndReg:BAsmCode[0]:=$e0+AdrMode;
	 ModIdxReg:BAsmCode[0]:=$f0+AdrMode;
	 ModDir:BAsmCode[0]:=$e7;
	 ModMem:BAsmCode[0]:=$e3;
	 END;
	 Move(AdrVals,BAsmCode[1],AdrCnt); HReg:=2+AdrCnt;
	 DecodeAdr(ArgStr[2],MModReg16);
	 IF AdrType=ModReg16 THEN
	  BEGIN
	   BAsmCode[HReg-1]:=$50+AdrMode; CodeLen:=HReg;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=0 TO ALU2OrderCnt-1 DO
    IF Memo(ALU2Orders[z+1]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModIdxReg+MModIndReg+MModDir+MModMem);
	CASE AdrType OF
	ModReg8:
	 BEGIN
	  HReg:=AdrMode;
	  IF HReg=AccReg THEN DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm)
	  ELSE DecodeAdr(ArgStr[2],MModImm);
	  CASE AdrType OF
	  ModReg8:
	   BEGIN
	    CodeLen:=2;
	    BAsmCode[0]:=$f8+AdrMode; BAsmCode[1]:=$60+z;
	   END;
	  ModIndReg:
	   BEGIN
	    CodeLen:=2;
	    BAsmCode[0]:=$e0+AdrMode; BAsmCode[1]:=$60+z;
	   END;
	  ModIdxReg:
	   BEGIN
	    CodeLen:=2+AdrCnt;
	    BAsmCode[0]:=$f0+AdrMode;
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	    BAsmCode[1+AdrCnt]:=$60+z;
	   END;
	  ModDir:
	   BEGIN
	    CodeLen:=2;
	    BAsmCode[0]:=$60+z;
	    BAsmCode[1]:=AdrVals[0];
	   END;
	  ModMem:
	   BEGIN
	    CodeLen:=4;
	    BAsmCode[0]:=$e3; BAsmCode[3]:=$60+z;
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	   END;
	  ModImm:
	   IF HReg=AccReg THEN
	    BEGIN
	     CodeLen:=2;
	     BAsmCode[0]:=$68+z; BAsmCode[1]:=AdrVals[0];
	    END
	   ELSE
	    BEGIN
	     CodeLen:=3;
	     BAsmCode[0]:=$f8+HReg; BAsmCode[1]:=$68+z;
	     BAsmCode[2]:=AdrVals[0];
	    END;
	  END;
	 END;
	ModReg16:
	 IF (AdrMode=2) OR ((z=0) AND (AdrMode>=4)) THEN
	  BEGIN
	   HReg:=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
	   CASE AdrType OF
	   ModReg16:
	    BEGIN
	     CodeLen:=2;
	     BAsmCode[0]:=$f8+AdrMode;
	     IF HReg>=4 THEN BAsmCode[1]:=$14+HReg-4 ELSE BAsmCode[1]:=$70+z;
	    END;
	   ModIndReg:
	    BEGIN
	     CodeLen:=2;
	     BAsmCode[0]:=$e0+AdrMode;
	     IF HReg>=4 THEN BAsmCode[1]:=$14+HReg-4 ELSE BAsmCode[1]:=$70+z;
	    END;
	   ModIdxReg:
	    BEGIN
	     CodeLen:=2+AdrCnt;
	     BAsmCode[0]:=$f0+AdrMode;
	     Move(AdrVals,BAsmCode[1],AdrCnt);
	     IF HReg>=4 THEN BAsmCode[1+AdrCnt]:=$14+HReg-4
			ELSE BAsmCode[1+AdrCnt]:=$70+z;
	    END;
	   ModDir:
	    IF HReg>=4 THEN
	     BEGIN
	      CodeLen:=3;
	      BAsmCode[0]:=$e7;
	      BAsmCode[1]:=AdrVals[0];
	      BAsmCode[2]:=$10+HReg;
	     END
	    ELSE
	     BEGIN
	      CodeLen:=2;
	      BAsmCode[0]:=$70+z; BAsmCode[1]:=AdrVals[0];
	     END;
	   ModMem:
	    BEGIN
	     CodeLen:=4;
	     BAsmCode[0]:=$e3;
	     Move(AdrVals,BAsmCode[1],2);
	     IF HReg>=4 THEN BAsmCode[3]:=$14+HReg-4
			ELSE BAsmCode[3]:=$70+z;
	    END;
	   ModImm:
	    BEGIN
	     CodeLen:=3;
	     IF HReg>=4 THEN BAsmCode[0]:=$14+HReg-4
			ELSE BAsmCode[0]:=$78+z;
	     Move(AdrVals,BAsmCode[1],AdrCnt);
	    END;
	   END;
	  END
	 ELSE WrError(1350);
	ModIndReg,ModIdxReg,ModDir,ModMem:
	 BEGIN
	  OpSize:=0;
	  CASE AdrType OF
	  ModIndReg:
	   BEGIN
	    HReg:=3;
	    BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$68+z;
	   END;
	  ModIdxReg:
	   BEGIN
	    HReg:=3+AdrCnt;
	    BAsmCode[0]:=$f4+AdrMode; BAsmCode[1+AdrCnt]:=$68+z;
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	   END;
	  ModDir:
	   BEGIN
	    HReg:=4;
	    BAsmCode[0]:=$ef; BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=$68+z;
	   END;
	  ModMem:
	   BEGIN
	    HReg:=5;
	    BAsmCode[0]:=$eb; Move(AdrVals,BAsmCode[1],2); BAsmCode[3]:=$68+z;
	   END;
	  END;
	  DecodeAdr(ArgStr[2],MModImm);
	  IF AdrType=ModImm THEN
	   BEGIN
	    BAsmCode[HReg-1]:=AdrVals[0]; CodeLen:=HReg;
	   END;
	 END;
	END;
       END;
      Exit;
     END;

   IF (WMemo('INC')) OR (WMemo('DEC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       HReg:=Ord(WMemo('DEC')) SHL 3;
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModIndReg+MModIdxReg+MModDir+MModMem);
       IF OpSize=-1 THEN OpSize:=0;
       CASE AdrType OF
       ModReg8:
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$80+HReg+AdrMode;
	END;
       ModReg16:
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$90+HReg+AdrMode;
	END;
       ModIndReg:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$e0+AdrMode;
	 BAsmCode[1]:=$87+(OpSize SHL 4)+HReg;
	END;
       ModIdxReg:
	BEGIN
	 CodeLen:=2+AdrCnt; BAsmCode[0]:=$f0+AdrMode;
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 BAsmCode[1+AdrCnt]:=$87+(OpSize SHL 4)+HReg;
	END;
       ModDir:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$87+(OpSize SHL 4)+HReg;
	 BAsmCode[1]:=AdrVals[0];
	END;
       ModMem:
	BEGIN
	 CodeLen:=4; BAsmCode[0]:=$e3;
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 BAsmCode[3]:=$87+(OpSize SHL 4)+HReg;
	 BAsmCode[1]:=AdrVals[0];
	END;
       END;
      END;
     Exit;
    END;

   IF (Memo('INCX')) OR (Memo('DECX')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModDir);
       IF AdrType=ModDir THEN
	BEGIN
	 CodeLen:=2;
	 BAsmCode[0]:=$07+(Ord(Memo('DECX')) SHL 3);
	 BAsmCOde[1]:=AdrVals[0];
	END;
      END;
     Exit;
    END;

   IF (Memo('MUL')) OR (Memo('DIV')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'HL')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       HReg:=$12+Ord(Memo('DIV')); OpSize:=0;
       DecodeAdr(ArgStr[2],MModReg8+MModIndReg+MModIdxReg+MModDir+MModMem+MModImm);
       CASE AdrType OF
       ModReg8:
	BEGIN
	 CodeLen:=2;
	 BAsmCode[0]:=$f8+AdrMode; BAsmCode[1]:=HReg;
	END;
       ModIndReg:
	BEGIN
	 CodeLen:=2;
	 BAsmCode[0]:=$e0+AdrMode; BAsmCode[1]:=HReg;
	END;
       ModIdxReg:
	BEGIN
	 CodeLen:=2+AdrCnt;
	 BAsmCode[0]:=$f0+AdrMode; BAsmCode[1+AdrCnt]:=HReg;
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	END;
       ModDir:
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$e7;
	 BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=HReg;
	END;
       ModMem:
	BEGIN
	 CodeLen:=4; BAsmCode[0]:=$e3; BAsmCode[3]:=HReg;
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	END;
       ModImm:
        BEGIN
         CodeLen:=3; BAsmCode[0]:=$e7; BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=HReg;
        END;
       END;
      END;
     Exit;
    END;

   { Sprnge }

   IF Memo('JR') THEN
    BEGIN
     IF (ArgCnt=0) OR (ArgCnt>2) THEN WrError(1110)
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
	 AdrInt:=EvalIntExpression(ArgStr[ArgCnt],Int16,OK)-(EProgCounter+2);
	 IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrInt>127) OR (AdrInt<-128)) THEN WrError(1370)
	  ELSE
	   BEGIN
	    CodeLen:=2;
	    BAsmCode[0]:=$c0+Conditions[z].Code;
	    BAsmCode[1]:=AdrInt AND $ff;
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
         WHILE (z<=ConditionCnt) AND (ArgStr[1]<>Conditions[z].Name) DO Inc(z);
	 IF z>ConditionCnt THEN WrError(1360);
	END;
       IF z<=ConditionCnt THEN
	BEGIN
	 OpSize:=1; HReg:=Ord(Memo('CALL'));
	 DecodeAdr(ArgStr[ArgCnt],MModIndReg+MModIdxReg+MModMem+MModImm);
	 IF AdrType=ModImm THEN AdrType:=ModMem;
	 CASE AdrType OF
	 ModIndReg:
	  BEGIN
	   CodeLen:=2;
	   BAsmCode[0]:=$e8+AdrMode;
	   BAsmCode[1]:=$c0+(HReg SHL 4)+Conditions[z].Code;
	  END;
	 ModIdxReg:
	  BEGIN
	   CodeLen:=2+AdrCnt;
	   BAsmCode[0]:=$f4+AdrMode;
	   Move(AdrVals,BAsmCode[1],AdrCnt);
	   BAsmCode[1+AdrCnt]:=$c0+(HReg SHL 4)+Conditions[z].Code;
	  END;
	 ModMem:
	  IF z=DefaultCondition THEN
	   BEGIN
	    CodeLen:=3;
	    BAsmCode[0]:=$1a+(HReg SHL 1);
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	   END
	  ELSE
	   BEGIN
	    CodeLen:=4;
	    BAsmCode[0]:=$eb;
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	    BAsmCode[3]:=$c0+(HReg SHL 4)+Conditions[z].Code;
	   END;
	 END;
	END;
      END;
     Exit;
    END;

   IF Memo('RET') THEN
    BEGIN
     IF (ArgCnt<>0) AND (ArgCnt<>1) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=0 THEN z:=DefaultCondition
       ELSE
	BEGIN
         z:=1; NLS_UpString(ArgStr[1]);
         WHILE (z<=ConditionCnt) AND (ArgStr[1]<>Conditions[z].Name) DO Inc(z);
	 IF z>ConditionCnt THEN WrError(1360);
	END;
       IF z=DefaultCondition THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$1e;
	END
       ELSE IF z<=ConditionCnt THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$fe;
	 BAsmCode[1]:=$d0+Conditions[z].Code;
	END;
      END;
     Exit;
    END;

   IF Memo('DJNZ') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 AdrType:=ModReg8; AdrMode:=0; OpSize:=0;
	END
       ELSE DecodeAdr(ArgStr[1],MModReg8+MModReg16);
       IF AdrType<>ModNone THEN
	IF AdrMode<>0 THEN WrError(1350)
	ELSE
	 BEGIN
	  AdrInt:=EvalIntExpression(ArgStr[ArgCnt],Int16,OK)-(EProgCounter+2);
	  IF OK THEN
           IF (NOT SymbolQuestionable) AND ((AdrInt>127) OR (AdrInt<-128)) THEN WrError(1370)
	   ELSE
	    BEGIN
	     CodeLen:=2;
	     BAsmCode[0]:=$18+OpSize;
	     BAsmCode[1]:=AdrInt AND $ff;
	    END;
	 END;
      END;
     Exit;
    END;

   IF (Memo('JRL')) OR (Memo('CALR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK)-(EProgCounter+2);
       IF OK THEN
	BEGIN
	 CodeLen:=3;
	 IF Memo('JRL') THEN
	  BEGIN
	   BAsmCode[0]:=$1b;
           IF (NOT SymbolQuestionable) AND ((AdrInt>=-128) AND (AdrInt<=127)) THEN WrError(20);
	  END
	 ELSE BAsmCode[0]:=$1d;
	 BAsmCode[1]:=Lo(AdrInt); BAsmCode[2]:=Hi(AdrInt);
	END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

{---------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

	PROCEDURE AddFixed(NName:String; NCode:Byte);
BEGIN
   Inc(z); IF z>FixedOrderCnt THEN Halt;
   WITH FixedOrders^[z] DO
    BEGIN
     Code:=NCode;
    END;
   AddInstTree(ITree,NName,CodeFixed,z);
END;

        PROCEDURE AddMove(NName:String; NCode:Byte);
BEGIN
   Inc(z); IF z>MoveOrderCnt THEN Halt;
   WITH MoveOrders^[z] DO
    BEGIN
     Code:=NCode;
    END;
   AddInstTree(ITree,NName,CodeMove,z);
END;

        PROCEDURE AddShift(NName:String; NCode:Byte; NMay:Boolean);
BEGIN
   Inc(z); IF z>ShiftOrderCnt THEN Halt;
   WITH ShiftOrders^[z] DO
    BEGIN
     Code:=NCode; MayReg:=NMay;
    END;
   AddInstTree(ITree,NName,CodeShift,z);
END;

        PROCEDURE AddBit(NName:String; NCode:Byte);
BEGIN
   Inc(z); IF z>BitOrderCnt THEN Halt;
   WITH BitOrders^[z] DO
    BEGIN
     Code:=NCode;
    END;
   AddInstTree(ITree,NName,CodeBit,z);
END;

        PROCEDURE AddAcc(NName:String; NCode:Byte);
BEGIN
   Inc(z); IF z>AccOrderCnt THEN Halt;
   WITH AccOrders^[z] DO
    BEGIN
     Code:=NCode;
    END;
   AddInstTree(ITree,NName,CodeAcc,z);
END;

BEGIN
   ITree:=Nil;

   New(FixedOrders); z:=0;
   AddFixed('EXX' ,$0a); AddFixed('CCF' ,$0e);
   AddFixed('SCF' ,$0d); AddFixed('RCF' ,$0c);
   AddFixed('NOP' ,$00); AddFixed('HALT',$01);
   AddFixed('DI'  ,$02); AddFixed('EI'  ,$03);
   AddFixed('SWI' ,$ff); AddFixed('RLCA',$a0);
   AddFixed('RRCA',$a1); AddFixed('RLA' ,$a2);
   AddFixed('RRA' ,$a3); AddFixed('SLAA',$a4);
   AddFixed('SRAA',$a5); AddFixed('SLLA',$a6);
   AddFixed('SRLA',$a7); AddFixed('RETI',$1f);

   New(MoveOrders); z:=0;
   AddMove('LDI' ,$58);
   AddMove('LDIR',$59);
   AddMove('LDD' ,$5a);
   AddMove('LDDR',$5b);
   AddMove('CPI' ,$5c);
   AddMove('CPIR',$5d);
   AddMove('CPD' ,$5e);
   AddMove('CPDR',$5f);

   New(ShiftOrders); z:=0;
   AddShift('RLC',$a0,True );
   AddShift('RRC',$a1,True );
   AddShift('RL' ,$a2,True );
   AddShift('RR' ,$a3,True );
   AddShift('SLA',$a4,True );
   AddShift('SRA',$a5,True );
   AddShift('SLL',$a6,True );
   AddShift('SRL',$a7,True );
   AddShift('RLD',$10,False);
   AddShift('RRD',$11,False);

   New(BitOrders); z:=0;
   AddBit('BIT' ,$a8);
   AddBit('SET' ,$b8);
   AddBit('RES' ,$b0);
   AddBit('TSET',$18);

   New(AccOrders); z:=0;
   AddAcc('DAA',$0b);
   AddAcc('CPL',$10);
   AddAcc('NEG',$11);
END;

	PROCEDURE DeinitFields;
BEGIN
   ClearInstTree(ITree);

   Dispose(FixedOrders);
   Dispose(MoveOrders);
   Dispose(ShiftOrders);
   Dispose(BitOrders);
   Dispose(AccOrders);
END;

{---------------------------------------------------------------------------}

	FUNCTION ChkPC_90C141:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<=$ffff;
   ELSE ok:=False;
   END;
   ChkPC_90C141:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_90C141:Boolean;
	Far;
BEGIN
   IsDef_90C141:=False;
END;

        PROCEDURE SwitchFrom_90C141;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_90C141;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=True;

   PCSymbol:='$'; HeaderID:=$53; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_90C141; ChkPC:=ChkPC_90C141; IsDef:=IsDef_90C141;
   SwitchFrom:=SwitchFrom_90C141; InitFields;
END;

BEGIN
   CPU90C141:=AddCPU('90C141',SwitchTo_90C141);
END.
