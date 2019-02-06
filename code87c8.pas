{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
        UNIT Code87C8;

INTERFACE

        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;



IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[4];
	       Code:Word;
	      END;

   CondRec=RECORD
	    Name:String[2];
	    Code:Byte;
	   END;

CONST
   FixedOrderCnt=7;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'DI'  ; Code:$483a),
		(Name:'EI'  ; Code:$403a),
		(Name:'RET' ; Code:$0005),
		(Name:'RETI'; Code:$0004),
		(Name:'RETN'; Code:$e804),
		(Name:'SWI' ; Code:$00ff),
		(Name:'NOP' ; Code:$0000));

   ConditionCnt=12;
   Conditions:ARRAY[1..ConditionCnt] OF CondRec=
	      ((Name:'EQ'; Code:0),(Name:'Z' ; Code:0),
	       (Name:'NE'; Code:1),(Name:'NZ'; Code:1),
	       (Name:'CS'; Code:2),(Name:'LT'; Code:2),
	       (Name:'CC'; Code:3),(Name:'GE'; Code:3),
	       (Name:'LE'; Code:4),(Name:'GT'; Code:5),
	       (Name:'T' ; Code:6),(Name:'F' ; Code:7));

   RegOrderCnt=7;
   RegOrders:ARRAY[1..RegOrderCnt] OF FixedOrder=
	     ((Name:'DAA' ; Code:$0a),(Name:'DAS' ; Code:$0b),
	      (Name:'SHLC'; Code:$1c),(Name:'SHRC'; Code:$1d),
	      (Name:'ROLC'; Code:$1e),(Name:'RORC'; Code:$1f),
	      (Name:'SWAP'; Code:$01));

   ALUOrderCnt=7;
   ALUOrders:ARRAY[0..ALUOrderCnt] OF String[4]=
	     ('ADDC','ADD','SUBB','SUB','AND','XOR','OR','CMP');

   ModNone=-1;
   ModReg8=0;    MModReg8=1 SHL ModReg8;
   ModReg16=1;   MModReg16=1 SHL ModReg16;
   ModImm=2;     MModImm=1 SHL ModImm;
   ModAbs=3;     MModAbs=1 SHL ModAbs;
   ModMem=4;     MModMem=1 SHL ModMem;

   AccReg=0; WAReg=0;

   Reg8Cnt=7;
   Reg8Names:ARRAY[0..Reg8Cnt] OF Char='AWCBEDLH';

VAR
   CPU87C00,CPU87C20,CPU87C40,CPU87C70:CPUVar;
   OpSize:ShortInt;
   AdrCnt:Byte;
   AdrVals:ARRAY[0..3] OF Byte;
   AdrType:ShortInt;
   AdrMode:Byte;


	PROCEDURE DecodeAdr(Asc:String; Erl:Byte);
CONST
   Reg16Cnt=3;
   Reg16Names:ARRAY[0..Reg16Cnt] OF String[2]=('WA','BC','DE','HL');
   AdrRegCnt=4;
   AdrRegs:ARRAY[0..AdrRegCnt] OF String[2]=('HL','DE','C','PC','A');
LABEL
   Found;
VAR
   z:Integer;
   RegFlag:Byte;
   DispAcc,DispPart:LongInt;
   AdrPart:String;
   OK,NegFlag,NNegFlag,FirstFlag:Boolean;
   PPos,NPos,EPos:Integer;
BEGIN
   AdrType:=ModNone; AdrCnt:=0;

   FOR z:=0 TO Reg8Cnt DO
    IF NLS_StrCaseCmp(Asc,Reg8Names[z])=0 THEN
     BEGIN
      AdrType:=ModReg8; OpSize:=0; AdrMode:=z;
      Goto Found;
     END;

   FOR z:=0 TO Reg16Cnt DO
    IF NLS_StrCaseCmp(Asc,Reg16Names[z])=0 THEN
     BEGIN
      AdrType:=ModReg16; OpSize:=1; AdrMode:=z;
      Goto Found;
     END;

   IF IsIndirect(Asc) THEN
    BEGIN
     Asc:=Copy(Asc,2,Length(Asc)-2);
     IF NLS_StrCaseCmp(Asc,'-HL')=0 THEN
      BEGIN
       AdrType:=ModMem; AdrMode:=7; Goto Found;
      END;
     IF NLS_StrCaseCmp(Asc,'HL+')=0 THEN
      BEGIN
       AdrType:=ModMem; AdrMode:=6; Goto Found;
      END;
     RegFlag:=0; DispAcc:=0; NegFlag:=False; OK:=True; FirstFlag:=False;
     WHILE (OK) AND (Asc<>'') DO
      BEGIN
       PPos:=QuotPos(Asc,'+'); NPos:=QuotPos(Asc,'-');
       NNegFlag:=NPos<PPos;
       IF NNegFlag THEN EPos:=NPos ELSE EPos:=PPos;
       AdrPart:=Copy(Asc,1,EPos-1); Delete(Asc,1,EPos);
       z:=0;
       WHILE (z<=AdrRegCnt) AND (NLS_StrCaseCmp(AdrRegs[z],AdrPart)<>0) DO Inc(z);
       IF z>AdrRegCnt THEN
	BEGIN
         FirstPassUnknown:=False;
	 DispPart:=EvalIntExpression(AdrPart,Int32,OK);
         IF FirstPassUnknown THEN FirstFlag:=True;
	 IF NegFlag THEN Dec(DispAcc,DispPart)
		    ELSE Inc(DispAcc,DispPart);
	END
       ELSE IF (NegFlag) OR (RegFlag AND (1 SHL z)<>0) THEN
	BEGIN
	 WrError(1350); OK:=False;
	END
       ELSE Inc(RegFlag,1 SHL z);
       NegFlag:=NNegFlag;
      END;
     IF DispAcc<>0 THEN Inc(RegFlag,1 SHL (1+AdrRegCnt));
     IF OK THEN
      CASE RegFlag OF
      $20:
       BEGIN
        IF FirstFlag THEN DispAcc:=DispAcc AND $ff;
        IF DispAcc>$ff THEN WrError(1320)
        ELSE
         BEGIN
          AdrType:=ModAbs; AdrMode:=0;
          AdrCnt:=1; AdrVals[0]:=DispAcc AND $ff;
         END;
       END;
      $02:BEGIN
           AdrType:=ModMem; AdrMode:=2;
          END;
      $01:BEGIN
           AdrType:=ModMem; AdrMode:=3;
          END;
      $21:
       BEGIN
        IF FirstFlag THEN DispAcc:=DispAcc AND $7f;
        IF DispAcc>127 THEN WrError(1320)
        ELSE IF DispAcc<-128 THEN WrError(1315)
        ELSE
         BEGIN
          AdrType:=ModMem; AdrMode:=4;
          AdrCnt:=1; AdrVals[0]:=DispAcc AND $ff;
         END;
       END;
      $05:BEGIN
           AdrType:=ModMem; AdrMode:=5;
          END;
      $18:BEGIN
           AdrType:=ModMem; AdrMode:=1;
          END;
      ELSE WrError(1350);
      END;
     Goto Found;
    END
   ELSE CASE OpSize OF
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
	AdrVals[0]:=DispAcc AND $ff;
	AdrVals[1]:=(DispAcc SHR 8) AND $ff;
       END;
     END;
   END;

Found:
   IF (AdrType<>ModNone) AND ((1 SHL AdrType) AND Erl=0) THEN
    BEGIN
     WrError(1350); AdrCnt:=0; AdrType:=ModNone;
    END;
END;

	FUNCTION SplitBit(VAR Asc:String; VAR Erg:Byte):Boolean;
VAR
   p:Byte;
   Part:String;
BEGIN
   SplitBit:=False;
   p:=RQuotPos(Asc,'.');
   IF p=0 THEN Exit;
   Part:=Copy(Asc,p+1,Length(Asc)-p); Byte(Asc[0]):=Pred(p);
   IF Length(Part)=1 THEN
    IF (Part[1]>='0') AND (Part[1]<='7') THEN
     BEGIN
      Erg:=Ord(Part[1])-AscOfs; SplitBit:=True;
     END
    ELSE
     BEGIN
      Erg:=0;
      WHILE (Erg<=Reg8Cnt) AND (UpCase(Part[1])<>Reg8Names[Erg]) DO Inc(Erg);
      IF Erg<=Reg8Cnt THEN
       BEGIN
	Inc(Erg,8); SplitBit:=True;
       END;
     END;
END;

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

	PROCEDURE CodeMem(Entry,Opcode:Byte);
BEGIN
   BAsmCode[0]:=Entry+AdrMode;
   Move(AdrVals,BAsmCode[1],AdrCnt);
   BAsmCode[1+AdrCnt]:=OpCode;
END;

	PROCEDURE MakeCode_87C800;
	Far;
VAR
   z,AdrInt,Condition:Integer;
   HReg,HCnt,HMode,HVal:Byte;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1;

   { zu ignorierendes }

   IF Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF Hi(Code)=0 THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=Lo(Code);
	END
       ELSE
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
	END;
       Exit;
      END;

   { Datentransfer }

   IF Memo('LD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'SP')=0 THEN
      BEGIN
       OpSize:=1;
       DecodeAdr(ArgStr[2],MModImm+MModReg16);
       CASE AdrType OF
       ModReg16:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$fa;
	END;
       ModImm:
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$fa; Move(AdrVals,BAsmCode[1],AdrCnt);
	END;
       END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'SP')=0 THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       CASE AdrType OF
       ModReg16:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$fb;
	END;
       END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'RBS')=0 THEN
      BEGIN
       BAsmCode[1]:=EvalIntExpression(ArgStr[2],Int4,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$0f;
	END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CF')=0 THEN
      BEGIN
       IF NOT SplitBit(ArgStr[2],HReg) THEN WrError(1510)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModMem);
	 CASE AdrType OF
	 ModReg8:
	  IF HReg>=8 THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$d8+HReg;
	   END;
	 ModAbs:
	  IF HReg>=8 THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$d8+HReg; BAsmCode[1]:=AdrVals[0];
	   END;
	 ModMem:
	  IF HReg<8 THEN
	   BEGIN
	    CodeLen:=2+AdrCnt;
	    CodeMem($e0,$d8+HReg);
	   END
	  ELSE IF (AdrMode<>2) AND (AdrMode<>3) THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$e0+HReg; BAsmCode[1]:=$9c+AdrMode;
	   END;
	 END;
	END;
      END
     ELSE IF NLS_StrCmp(ArgStr[2],'CF')=0 THEN
      BEGIN
       IF NOT SplitBit(ArgStr[1],HReg) THEN WrError(1510)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg8+MModAbs+MModMem);
	 CASE AdrType OF
	 ModReg8:
	  IF HReg>=8 THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$c8+HReg;
	   END;
	 ModAbs,ModMem:
	  IF HReg<8 THEN
	   BEGIN
	    CodeLen:=2+AdrCnt; CodeMem($e0,$c8+HReg);
	   END
	  ELSE IF (AdrMode<>2) AND (AdrMode<>3) THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$e0+HReg; BAsmCode[1]:=$98+AdrMode;
	   END;
	 END;
	END;
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModAbs+MModMem);
       CASE AdrType OF
       ModReg8:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModMem+MModImm);
	 CASE AdrType OF
	 ModReg8:
	  IF HReg=AccReg THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$50+AdrMode;
	   END
	  ELSE IF AdrMode=AccReg THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$58+HReg;
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$58+HReg;
	   END;
	 ModAbs:
	  IF HReg=AccReg THEN
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$22; BAsmCode[1]:=AdrVals[0];
	   END
	  ELSE
	   BEGIN
	    CodeLen:=3; BAsmCode[0]:=$e0; BAsmCode[1]:=AdrVals[0];
	    BAsmCode[2]:=$58+HReg;
	   END;
	 ModMem:
	  IF (HReg=AccReg) AND (AdrMode=3) THEN   { A,(HL) }
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$23;
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2+AdrCnt; CodeMem($e0,$58+HReg);
	    IF (HReg>=6) AND (AdrMode=6) THEN WrError(140);
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
	 DecodeAdr(ArgStr[2],MModReg16+MModAbs+MModMem+MModImm);
	 CASE AdrType OF
	 ModReg16:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$14+HReg;
	  END;
	 ModAbs:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$e0; BAsmCode[1]:=AdrVals[0];
	   BAsmCode[2]:=$14+HReg;
	  END;
	 ModMem:
	  IF AdrMode>5 THEN WrError(1350)   { (-HL),(HL+) }
	  ELSE
	   BEGIN
	    CodeLen:=2+AdrCnt; BAsmCode[0]:=$e0+AdrMode;
	    Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=$14+HReg;
	   END;
	 ModImm:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$14+HReg; Move(AdrVals,BAsmCode[1],2);
	  END;
	 END;
	END;
       ModAbs:
	BEGIN
	 HReg:=AdrVals[0]; OpSize:=0;
	 DecodeAdr(ArgStr[2],MModReg8+MModReg16+MModAbs+MModMem+MModImm);
	 CASE AdrType OF
	 ModReg8:
	  IF AdrMode=AccReg THEN
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$2a; BAsmCode[1]:=HReg;
	   END
	  ELSE
	   BEGIN
	    CodeLen:=3; BAsmCode[0]:=$f0; BAsmCode[1]:=HReg;
	    BAsmCode[2]:=$50+AdrMode;
	   END;
	 ModReg16:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$f0; BAsmCode[1]:=HReg;
	   BAsmCode[2]:=$10+AdrMode;
	  END;
	 ModAbs:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$26; BAsmCode[1]:=AdrVals[0];
	   BAsmCode[2]:=HReg;
	  END;
	 ModMem:
	  IF AdrMode>5 THEN WrError(1350)      { (-HL),(HL+) }
	  ELSE
	   BEGIN
	    CodeLen:=3+AdrCnt; BAsmCode[0]:=$e0+AdrMode;
	    Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=$26;
	    BAsmCode[2+AdrCnt]:=HReg;
	   END;
	 ModImm:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$2c; BAsmCode[1]:=HReg;
	   BAsmCode[2]:=AdrVals[0];
	  END;
	 END;
	END;
       ModMem:
	BEGIN
	 HVal:=AdrVals[0]; HCnt:=AdrCnt; HMode:=AdrMode; OpSize:=0;
	 DecodeAdr(ArgStr[2],MModReg8+MModReg16+MModAbs+MModMem+MModImm);
	 CASE AdrType OF
	 ModReg8:
	  IF (HMode=3) AND (AdrMode=AccReg) THEN   { (HL),A }
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$2b;
	   END
	  ELSE IF (HMode=1) OR (HMode=5) THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=2+HCnt; BAsmCode[0]:=$f0+HMode;
	    Move(HVal,BAsmCode[1],HCnt); BAsmCode[1+HCnt]:=$50+AdrMode;
	    IF (HMode=6) AND (AdrMode>=6) THEN WrError(140);
	   END;
	 ModReg16:
	  IF (HMode<2) OR (HMode>4) THEN WrError(1350)  { (HL),(DE),(HL+d) }
	  ELSE
	   BEGIN
	    CodeLen:=2+HCnt; BAsmCode[0]:=$f0+HMode;
	    Move(HVal,BAsmCode[1],HCnt); BAsmCode[1+HCnt]:=$10+AdrMode;
	   END;
	 ModAbs:
	  IF HMode<>3 THEN WrError(1350)  { (HL) }
	  ELSE
	   BEGIN
	    CodeLen:=3; BAsmCode[0]:=$e0; BAsmCode[1]:=AdrVals[0];
	    BAsmCode[2]:=$27;
	   END;
	 ModMem:
	  IF HMode<>3 THEN WrError(1350)         { (HL) }
	  ELSE IF AdrMode>5 THEN WrError(1350)   { (-HL),(HL+) }
	  ELSE
	   BEGIN
	    CodeLen:=2+AdrCnt; BAsmCode[0]:=$e0+AdrMode;
	    Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=$27;
	   END;
	 ModImm:
	  IF (HMode=1) OR (HMode=5) THEN WrError(1350)  { (HL+C),(PC+A) }
	  ELSE IF HMode=3 THEN               { (HL) }
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$2d; BAsmCode[1]:=AdrVals[0];
	   END
	  ELSE
	   BEGIN
	    CodeLen:=3+HCnt; BAsmCode[0]:=$f0+HMode;
	    Move(HVal,BAsmCode[1],HCnt); BAsmCode[1+HCnt]:=$2c;
	    BAsmCode[2+HCnt]:=AdrVals[0];
	   END;
	 END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('XCH') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModAbs+MModMem);
       CASE AdrType OF
       ModReg8:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModMem);
	 CASE AdrType OF
	 ModReg8:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$a8+HReg;
	  END;
	 ModAbs,ModMem:
	  BEGIN
	   CodeLen:=2+AdrCnt; CodeMem($e0,$a8+HReg);
	   IF (HReg>=6) AND (AdrMode=6) THEN WrError(140);
	  END;
	 END;
	END;
       ModReg16:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg16);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$10+HReg;
	  END;
	END;
       ModAbs:
	BEGIN
	 BAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[2],MModReg8);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$e0; BAsmCode[2]:=$a8+AdrMode;
	  END;
	END;
       ModMem:
	BEGIN
	 BAsmCode[0]:=$e0+AdrMode; Move(AdrVals,BAsmCode[1],AdrCnt);
	 HReg:=AdrCnt;
	 DecodeAdr(ArgStr[2],MModReg8);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=2+HReg; BAsmCode[1+HReg]:=$a8+AdrMode;
	   IF (AdrMode>=6) AND (BAsmCode[0] AND $0f=6) THEN WrError(140);
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('CLR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CF')=0 THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$0c;
      END
     ELSE IF SplitBit(ArgStr[1],HReg) THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModAbs+MModMem);
       CASE AdrType OF
       ModReg8:
	IF HReg>=8 THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$48+HReg;
	 END;
       ModAbs:
	IF HReg>=8 THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$48+HReg; BAsmCode[1]:=AdrVals[0];
	 END;
       ModMem:
	IF HReg<=8 THEN
	 BEGIN
	  CodeLen:=2+AdrCnt; CodeMem($e0,$48+HReg);
	 END
	ELSE IF (AdrMode<>2) AND (AdrMode<>3) THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$e0+HReg; BAsmCode[1]:=$88+AdrMode
	 END;
       END;
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModAbs+MModMem);
       CASE AdrType OF
       ModReg8:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$30+AdrMode; BAsmCode[1]:=0;
	END;
       ModReg16:
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$14+AdrMode; BAsmCode[1]:=0; BAsmCode[2]:=0;
	END;
       ModAbs:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$2e; BAsmCode[1]:=AdrVals[0];
	END;
       ModMem:
	IF (AdrMode=5) OR (AdrMode=1) THEN WrError(1350)  { (PC+A, HL+C) }
	ELSE IF AdrMode=3 THEN     { (HL) }
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$2f;
	 END
	ELSE
	 BEGIN
	  CodeLen:=3+AdrCnt; BAsmCode[0]:=$f0+AdrMode;
	  Move(AdrVals,BAsmCode[1],AdrCnt);
	  BAsmCode[1+AdrCnt]:=$2c; BAsmCode[2+AdrCnt]:=0;
	 END;
       END;
      END;
     Exit;
    END;

   IF Memo('LDW') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK);
       IF OK THEN
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg16+MModAbs+MModMem);
	 CASE AdrType OF
	 ModReg16:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$14+AdrMode;
	   BAsmCode[1]:=AdrInt AND $ff; BAsmCode[2]:=AdrInt SHR 8;
	  END;
	 ModAbs:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$24; BAsmCode[1]:=AdrVals[0];
	   BAsmCode[2]:=AdrInt AND $ff; BAsmCode[3]:=AdrInt SHR 8;
	  END;
	 ModMem:
	  IF AdrMode<>3 THEN WrError(1350)  { (HL) }
	  ELSE
	   BEGIN
	    CodeLen:=3; BAsmCode[0]:=$25;
	    BAsmCode[1]:=AdrInt AND $ff; BAsmCode[2]:=AdrInt SHR 8;
	   END;
	 END;
	END;
      END;
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     HReg:=Ord(Memo('PUSH'))+6;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'PSW')=0 THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=HReg;
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=HReg;
	END;
      END;
     Exit;
    END;

   IF (Memo('TEST')) OR (Memo('CPL')) OR (Memo('SET')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CF')=0 THEN
      BEGIN
       IF Memo('TEST') THEN WrError(1350)
       ELSE
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$0d+Ord(Memo('CPL'));
	END;
      END
     ELSE IF NOT SplitBit(ArgStr[1],HReg) THEN WrError(1510)
     ELSE
      BEGIN
       IF Memo('TEST') THEN HVal:=$d8
       ELSE IF Memo('SET') THEN HVal:=$40
       ELSE HVal:=$c0;
       DecodeAdr(ArgStr[1],MModReg8+MModAbs+MModMem);
       CASE AdrType OF
       ModReg8:
	IF HReg>=8 THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=HVal+HReg;
	 END;
       ModAbs:
	IF HReg>=8 THEN WrError(1350)
	ELSE IF Memo('CPL') THEN
	 BEGIN
	  CodeLen:=3; CodeMem($e0,HVal+HReg);
	 END
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=HVal+HReg; BAsmCode[1]:=AdrVals[0];
	 END;
       ModMem:
	IF HReg<8 THEN
	 BEGIN
	  CodeLen:=2+AdrCnt; CodeMem($e0,HVal+HReg);
	 END
	ELSE IF (AdrMode<>2) AND (AdrMode<>3) THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$e0+HReg;
	  BAsmCode[1]:=((HVal AND $18) SHR 1)+((HVal AND $80) SHR 3)+$80+AdrMode;
	 END;
       END;
      END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=1 TO RegOrderCnt DO
    WITH RegOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg8);
	 IF AdrType<>ModNone THEN
	  IF AdrMode=AccReg THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=Code;
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=Code;
	   END;
	END;
       Exit;
      END;

   FOR z:=0 TO ALUOrderCnt DO
    IF Memo(ALUOrders[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF NLS_StrCaseCmp(ArgStr[1],'CF')=0 THEN
       BEGIN
	IF NOT Memo('XOR') THEN WrError(1350)
	ELSE IF NOT SplitBit(ArgStr[2],HReg) THEN WrError(1510)
	ELSE IF HReg>=8 THEN WrError(1350)
	ELSE
	 BEGIN
	  DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModMem);
	  CASE AdrType OF
	  ModReg8:
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$d0+HReg;
	   END;
	  ModAbs,ModMem:
	   BEGIN
	    CodeLen:=2+AdrCnt; CodeMem($e0,$d0+HReg);
	   END;
	  END;
	 END;
       END
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModMem+MModAbs);
	CASE AdrType OF
	ModReg8:
	 BEGIN
	  HReg:=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg8+MModMem+MModAbs+MModImm);
	  CASE AdrType OF
	  ModReg8:
	   IF HReg=AccReg THEN
	    BEGIN
	     CodeLen:=2;
	     BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$60+z;
	    END
	   ELSE IF AdrMode=AccReg THEN
	    BEGIN
	     CodeLen:=2;
	     BAsmCode[0]:=$e8+HReg; BAsmCode[1]:=$68+z;
	    END
	   ELSE WrError(1350);
	  ModMem:
	   IF HReg<>AccReg THEN WrError(1350)
	   ELSE
	    BEGIN
	     CodeLen:=2+AdrCnt; BAsmCode[0]:=$e0+AdrMode;
	     Move(AdrVals,BAsmCode[1],AdrCnt);
	     BAsmCode[1+AdrCnt]:=$78+z;
	    END;
	  ModAbs:
	   IF HReg<>AccReg THEN WrError(1350)
	   ELSE
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$78+z; BAsmCode[1]:=AdrVals[0];
	    END;
	  ModImm:
	   IF HReg=AccReg THEN
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$70+z; BAsmCode[1]:=AdrVals[0];
	    END
	   ELSE
	    BEGIN
	     CodeLen:=3; BAsmCode[0]:=$e8+HReg;
	     BAsmCode[1]:=$70+z; BAsmCode[2]:=AdrVals[0];
	    END;
	  END;
	 END;
	ModReg16:
	 BEGIN
	  HReg:=AdrMode; DecodeAdr(ArgStr[2],MModImm+MModReg16);
	  CASE AdrType OF
	  ModImm:
	   BEGIN
	    CodeLen:=4; BAsmCode[0]:=$e8+HReg; BAsmCode[1]:=$38+z;
	    Move(AdrVals,BAsmCode[2],AdrCnt);
	   END;
	  ModReg16:
	   IF HReg<>WAReg THEN WrError(1350)
	   ELSE
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=$30+z;
	    END;
	  END;
	 END;
	ModAbs:
	 BEGIN
          IF NLS_StrCaseCmp(ArgStr[2],'(HL)')=0 THEN
	   BEGIN
	    CodeLen:=3; BAsmCode[0]:=$e0;
	    BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=$60+z;
	   END
	  ELSE
	   BEGIN
	    BAsmCode[3]:=EvalIntExpression(ArgStr[2],Int8,OK);
	    IF OK THEN
	     BEGIN
	      CodeLen:=4; BAsmCode[0]:=$e0;
	      BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=$70+z;
	     END;
	   END;
	 END;
	ModMem:
	 BEGIN
          IF NLS_StrCaseCmp(ArgStr[2],'(HL)')=0 THEN
	   BEGIN
	    CodeLen:=2+AdrCnt; BAsmCode[0]:=$e0+AdrMode;
	    Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=$60+z;
	   END
	  ELSE
	   BEGIN
	    BAsmCode[2+AdrCnt]:=EvalIntExpression(ArgStr[2],Int8,OK);
	    IF OK THEN
	     BEGIN
	      CodeLen:=3+AdrCnt; BAsmCode[0]:=$e0+AdrMode;
	      Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=$70+z;
	     END;
	   END;
	 END;
	END;
       END;
      Exit;
     END;

   IF Memo('MCMP') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       HReg:=EvalIntExpression(ArgStr[2],Int8,OK);
       IF OK THEN
	BEGIN
	 DecodeAdr(ArgStr[1],MModMem+MModAbs);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=3+AdrCnt; CodeMem($e0,$2f); BAsmCode[2+AdrCnt]:=HReg;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('DEC')) OR (Memo('INC')) THEN
    BEGIN
     HReg:=Ord(Memo('DEC')) SHL 3;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8+MModReg16+MModAbs+MModMem);
       CASE AdrType OF
       ModReg8:
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$60+HReg+AdrMode;
	END;
       ModReg16:
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$10+HReg+AdrMode;
	END;
       ModAbs:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$20+HReg; BAsmCode[1]:=AdrVals[0];
	END;
       ModMem:
	IF AdrMode=3 THEN     { (HL) }
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$21+HReg;
	 END
	ELSE
	 BEGIN
	  CodeLen:=2+AdrCnt; BAsmCode[0]:=$e0+AdrMode;
	  Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=$20+HReg;
	 END;
       END;
      END;
     Exit;
    END;

   IF Memo('MUL') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8);
       IF AdrType=ModReg8 THEN
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg8);
	 IF AdrType=ModReg8 THEN
	  IF HReg XOR AdrMode<>1 THEN WrError(1760)
	  ELSE
	   BEGIN
	    HReg:=HReg SHR 1;
	    IF HReg=0 THEN
	     BEGIN
	      CodeLen:=1; BAsmCode[0]:=$02;
	     END
	    ELSE
	     BEGIN
	      CodeLen:=2; BAsmCode[0]:=$e8+HReg; BAsmCode[1]:=$02;
	     END;
	   END;
	END;
      END;
     Exit;
    END;

   IF Memo('DIV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg16);
       IF AdrType=ModReg16 THEN
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg8);
	 IF AdrType=ModReg8 THEN
	  IF AdrMode<>2 THEN WrError(1350)  { C }
	  ELSE IF HReg=0 THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$03;
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$e8+HReg; BAsmCode[1]:=$03;
	    IF HReg=1 THEN WrError(140);
	   END;
	END;
      END;
     Exit;
    END;

   IF (Memo('ROLD')) OR (Memo('RORD')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       HReg:=Ord(Memo('RORD'))+8;
       DecodeAdr(ArgStr[2],MModAbs+MModMem);
       IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=2+AdrCnt; CodeMem($e0,HReg);
	 IF AdrMode=1 THEN WrError(140);
	END;
      END;
     Exit;
    END;

   { SprÅnge }

   IF Memo('JRS') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       NLS_UpString(ArgStr[1]);
       Condition:=ConditionCnt-2;
       WHILE (Condition<=ConditionCnt) AND
             (Conditions[Condition].Name<>ArgStr[1]) DO Inc(Condition);
       IF Condition>ConditionCnt THEN WrXError(1360,ArgStr[1])
       ELSE
	BEGIN
	 AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK)-(EProgCounter+2);
	 IF OK THEN
	  IF (AdrInt<-16) OR (AdrInt>15) THEN WrError(1370)
	  ELSE
	   BEGIN
	    CodeLen:=1;
	    BAsmCode[0]:=((Conditions[Condition].Code-2) SHL 5)+(AdrInt AND $1f);
	   END;
	END;
      END;
     Exit;
    END;

   IF Memo('JR') THEN
    BEGIN
     IF (ArgCnt<>2) AND (ArgCnt<>1) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN Condition:=-1
       ELSE
	BEGIN
	 Condition:=1;
         NLS_UpString(ArgStr[1]);
         WHILE (Condition<=ConditionCnt) AND
	       (Conditions[Condition].Name<>ArgStr[1]) DO Inc(Condition);
	END;
       IF Condition>ConditionCnt THEN WrXError(1360,ArgStr[1])
       ELSE
	BEGIN
	 AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK)-(EProgCounter+2);
	 IF OK THEN
	  IF (AdrInt<-128) OR (AdrInt>127) THEN WrError(1370)
	  ELSE
	   BEGIN
	    CodeLen:=2;
	    IF Condition=-1 THEN BAsmCode[0]:=$fb
	    ELSE BAsmCode[0]:=$d0+Conditions[Condition].Code;
	    BAsmCode[1]:=AdrInt AND $ff;
	   END;
	END;
      END;
     Exit;
    END;

   IF (Memo('JP')) OR (Memo('CALL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=1; HReg:=$fc+2*Ord(Memo('JP'));
       DecodeAdr(ArgStr[1],MModReg16+MModAbs++MModMem+MModImm);
       CASE AdrType OF
       ModReg16:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$e8+AdrMode; BAsmCode[1]:=HReg;
	END;
       ModAbs:
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$e0; BAsmCode[1]:=AdrVals[0];
	 BAsmCode[2]:=HReg;
	END;
       ModMem:
	IF (AdrMode>5) THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=2+AdrCnt; BAsmCode[0]:=$e0+AdrMode;
	  Move(AdrVals,BAsmCode[1],AdrCnt); BAsmCode[1+AdrCnt]:=HReg;
	 END;
       ModImm:
	IF (AdrVals[1]=$ff) AND (Memo('CALL')) THEN
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$fd; BAsmCode[1]:=AdrVals[0];
	 END
	ELSE
	 BEGIN
	  CodeLen:=3; BAsmCode[0]:=HReg; Move(AdrVals,BAsmCode[1],AdrCnt);
	 END;
       END;
      END;
     Exit;
    END;

   IF Memo('CALLV') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       HVal:=EvalIntExpression(ArgStr[1],Int4,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$c0+(HVal AND 15);
	END;
      END;
     Exit;
    END;

   IF Memo('CALLP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF (Hi(AdrInt)<>$ff) AND (Hi(AdrInt)<>0) THEN WrError(1320)
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$fd; BAsmCode[1]:=Lo(AdrInt);
	 END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_87C800:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<=$ffff;
   ELSE ok:=False;
   END;
   ChkPC_87C800:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_87C800:Boolean;
	Far;
BEGIN
   IsDef_87C800:=False;
END;

        PROCEDURE SwitchFrom_87C800;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_87C800;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=True;

   PCSymbol:='$'; HeaderID:=$54; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_87C800; ChkPC:=ChkPC_87C800; IsDef:=IsDef_87C800;
   SwitchFrom:=SwitchFrom_87C800;
END;

BEGIN
   CPU87C00:=AddCPU('87C00',SwitchTo_87C800);
   CPU87C20:=AddCPU('87C20',SwitchTo_87C800);
   CPU87C40:=AddCPU('87C40',SwitchTo_87C800);
   CPU87C70:=AddCPU('87C70',SwitchTo_87C800);
END.
