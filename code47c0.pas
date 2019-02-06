{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code47C0;

INTERFACE
        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[4]; Code:Byte;
	      END;

CONST
   FixedOrderCnt=3;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'RET' ; Code:$2a),
		(Name:'RETI'; Code:$2b),
		(Name:'NOP' ; Code:$00));

   BitOrderCnt=4;
   BitOrders:ARRAY[0..3] OF String[5]=('SET','CLR','TEST','TESTP');

   ModNone=-1;
   ModAcc=0;      MModAcc=1 SHL ModAcc;
   ModL=1;        MModL=1 SHL ModL;
   ModH=2;        MModH=1 SHL ModH;
   ModHL=3;       MModHL=1 SHL ModHL;
   ModIHL=4;      MModIHL=1 SHL ModIHL;
   ModAbs=5;      MModAbs=1 SHL ModAbs;
   ModPort=6;     MModPort=1 SHL ModPort;
   ModImm=7;      MModImm=1 SHL ModImm;
   ModSAbs=8;     MModSAbs=1 SHL ModSAbs;

VAR
   CPU47C00,CPU470C00,CPU470AC00:CPUVar;
   AdrType,OpSize:ShortInt;
   AdrVal:Byte;
   DMBAssume:LongInt;
   SaveInitProc:PROCEDURE;


	FUNCTION RAMEnd:Word;
BEGIN
   IF MomCPU=CPU47C00 THEN RAMEnd:=$ff
   ELSE IF MomCPU=CPU470C00 THEN RAMENd:=$1ff
   ELSE RAMEnd:=$3ff;
END;

	FUNCTION ROMEnd:Word;
BEGIN
   IF MomCPU=CPU47C00 THEN ROMEnd:=$fff
   ELSE IF MomCPU=CPU470C00 THEN ROMENd:=$1fff
   ELSE ROMEnd:=$3fff;
END;

	FUNCTION PortEnd:Word;
BEGIN
   IF MomCPU=CPU47C00 THEN PortEnd:=$0f
   ELSE PortEnd:=$1f;
END;

	PROCEDURE SetOpSize(NewSize:ShortInt);
BEGIN
   IF OpSize=-1 THEN OpSize:=NewSize
   ELSE IF OpSize<>NewSize THEN
    BEGIN
     WrError(1131); AdrType:=ModNone;
    END;
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   Found;
CONST
   RegNames:ARRAY[ModAcc..ModIHL] OF String[3]=('A','L','H','HL','@HL');
VAR
   z:Byte;
   AdrWord:Word;
   OK:Boolean;
BEGIN
   AdrType:=ModNone;

   FOR z:=ModAcc TO ModIHL DO
    IF NLS_StrCaseCmp(Asc,RegNames[z])=0 THEN
     BEGIN
      AdrType:=z;
      IF z<>ModIHL THEN SetOpSize(Ord(z=ModHL));
      Goto Found;
     END;

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OpSize OF
     -1:WrError(1132);
     2:BEGIN
	AdrVal:=EvalIntExpression(Asc,UInt2,OK) AND 3;
	IF OK THEN AdrType:=ModImm;
       END;
     0:BEGIN
	AdrVal:=EvalIntExpression(Asc,Int4,OK) AND 15;
	IF OK THEN AdrType:=ModImm;
       END;
     1:BEGIN
	AdrVal:=EvalIntExpression(Asc,Int8,OK);
	IF OK THEN AdrType:=ModImm;
       END;
     END;
     Goto Found;
    END;

   IF Asc[1]='%' THEN
    BEGIN
     AdrVal:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),Int5,OK);
     IF OK THEN
      BEGIN
       AdrType:=ModPort; ChkSpace(SegIO);
      END;
     Goto Found;
    END;

   FirstPassUnknown:=False;
   AdrWord:=EvalIntExpression(Asc,Int16,OK);
   IF OK THEN
    BEGIN
     ChkSpace(SegData);

     IF FirstPassUnknown THEN AdrWord:=AdrWord AND RAMEnd
     ELSE IF Hi(AdrWord)<>DMBAssume THEN WrError(110);

     AdrVal:=Lo(AdrWord);
     IF FirstPassUnknown THEN AdrVal:=AdrVal AND 15;

     IF (Mask AND MModSAbs<>0) AND (AdrVal<16) THEN
      AdrType:=ModSAbs
     ELSE AdrType:=ModAbs
    END;

Found:
   IF (AdrType<>ModNone) AND ((1 SHL AdrType) AND Mask=0) THEN
    BEGIN
     WrError(1350); AdrType:=ModNone;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUME47Count=1;
   ASSUME47s:ARRAY[1..ASSUME47Count] OF ASSUMERec=
	     ((Name:'DMB'; Dest:@DMBAssume; Min:0; Max:3; NothingVal:4));
BEGIN
   DecodePseudo:=True;

   IF Memo('PORT') THEN
    BEGIN
     CodeEquate(SegIO,0,$1f);
     Exit;
    END;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME47s,ASSUME47Count);
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE ChkCPU(Mask:Byte);
BEGIN
   IF Mask AND (1 SHL (MomCPU-CPU47C00))=0 THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    END;
END;

	FUNCTION DualOp(s1,s2:String):Boolean;
BEGIN
   DualOp:=((NLS_StrCaseCmp(ArgStr[1],s1)=0) AND (NLS_StrCaseCmp(ArgStr[2],s2)=0))
        OR ((NLS_StrCaseCmp(ArgStr[2],s1)=0) AND (NLS_StrCaseCmp(ArgStr[1],s2)=0));
END;

	PROCEDURE MakeCode_47C00;
	Far;
VAR
   OK:Boolean;
   AdrWord:Word;
   z:Integer;
   HReg:Byte;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodeIntelPseudo(False) THEN Exit;

   IF DecodePseudo THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=Code;
	END;
       Exit;
      END;

   { Datentransfer }

   IF Memo('LD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'DMB')=0 THEN
      BEGIN
       SetOpSize(2); DecodeAdr(ArgStr[2],MModImm+MModIHL);
       CASE AdrType OF
       ModIHL:
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$03; BAsmCode[1]:=$3a; BAsmCode[2]:=$e9;
	 ChkCPU(4);
	END;
       ModImm:
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$03;
	 BAsmCode[1]:=$2c; BAsmCode[2]:=$09+(AdrVal SHL 4);
	 ChkCPU(4);
	END;
       END;
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModHL+MModH+MModL);
       CASE AdrType OF
       ModAcc:
	BEGIN
	 DecodeAdr(ArgStr[2],MModIHL+MModAbs+MModImm);
	 CASE AdrType OF
	 ModIHL:
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$0c;
	  END;
	 ModAbs:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$3c; BAsmCode[1]:=AdrVal;
	  END;
	 ModImm:
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$40+AdrVal;
	  END;
	 END;
	END;
       ModHL:
	BEGIN
	 DecodeAdr(ArgStr[2],MModAbs+MModImm);
	 CASE AdrType OF
	 ModAbs:
	  IF AdrVal AND 3<>0 THEN WrError(1325)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$28; BAsmCode[1]:=AdrVal;
	   END;
	 ModImm:
	  BEGIN
	   CodeLen:=2;
	   BAsmCode[0]:=$c0+(AdrVal SHR 4);
	   BAsmCode[1]:=$e0+(AdrVal AND 15);
	  END;
	 END;
	END;
       ModH,ModL:
	BEGIN
         BAsmCode[0]:=$c0+(Ord(AdrType=ModL) SHL 5);
	 DecodeAdr(ArgStr[2],MModImm);
	 IF AdrType<>ModNone THEN
	  BEGIN
           CodeLen:=1; Inc(BAsmCode[0],AdrVal);
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('LDL') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'A')<>0) OR (NLS_StrCaseCmp(ArgStr[2],'@DC')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$33;
      END;
     Exit;
    END;

   IF Memo('LDH') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'A')<>0) OR (NLS_StrCaseCmp(ArgStr[2],'@DC+')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$32;
      END;
     Exit;
    END;

   IF Memo('ST') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'DMB')=0 THEN
      BEGIN
       DecodeAdr(ArgStr[2],MModIHL);
       IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$03; BAsmCode[1]:=$3a; BAsmCode[2]:=$69;
	 ChkCPU(4);
	END;
      END
     ELSE
      BEGIN
       OpSize:=0; DecodeAdr(ArgStr[1],MModImm+MModAcc);
       CASE AdrType OF
       ModAcc:
        IF NLS_StrCaseCmp(ArgStr[2],'@HL+')=0 THEN
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$1a;
	 END
        ELSE IF NLS_StrCaseCmp(ArgStr[2],'@HL-')=0 THEN
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$1b;
	 END
	ELSE
	 BEGIN
	  DecodeAdr(ArgStr[2],MModAbs+MModIHL);
	  CASE AdrType OF
	  ModAbs:
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$3f; BAsmCode[1]:=AdrVal;
	   END;
	  ModIHL:
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$0f;
	   END;
	  END;
	 END;
       ModImm:
	BEGIN
	 HReg:=AdrVal;
         IF NLS_StrCaseCmp(ArgStr[2],'@HL+')=0 THEN
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$f0+HReg;
	  END
	 ELSE
	  BEGIN
	   DecodeAdr(ArgStr[2],MModSAbs);
	   IF AdrType<>ModNone THEN
	    IF AdrVal>$0f THEN WrError(1320)
	    ELSE
	     BEGIN
	      CodeLen:=2; BAsmCode[0]:=$2d;
	      BAsmCode[1]:=HReg SHL 4+AdrVal;
	     END;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'A')=0) AND (NLS_StrCaseCmp(ArgStr[2],'DMB')=0) THEN
      BEGIN
       CodeLen:=3; BAsmCode[0]:=$03; BAsmCode[1]:=$3a; BAsmCode[2]:=$a9;
       ChkCPU(4);
      END
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'DMB')=0) AND (NLS_StrCaseCmp(ArgStr[2],'A')=0) THEN
      BEGIN
       CodeLen:=3; BAsmCode[0]:=$03; BAsmCode[1]:=$3a; BAsmCode[2]:=$29;
       ChkCPU(4);
      END
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'A')=0) AND (NLS_StrCaseCmp(ArgStr[2],'SPW13')=0) THEN
      BEGIN
       CodeLen:=2; BAsmCode[0]:=$3a; BAsmCode[1]:=$84;
       ChkCPU(4);
      END
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'STK13')=0) AND (NLS_StrCaseCmp(ArgStr[2],'A')=0) THEN
      BEGIN
       CodeLen:=2; BAsmCode[0]:=$3a; BAsmCode[1]:=$04;
       ChkCPU(4);
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'A')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModH+MModL);
       IF AdrType<>ModNone THEN
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$10+Ord(AdrType=ModL);
	END;
      END;
     Exit;
    END;

   IF Memo('XCH') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF DualOp('A','EIR') THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$13;
      END
     ELSE IF DualOp('A','@HL') THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$0d;
      END
     ELSE IF DualOp('A','H') THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$30;
      END
     ELSE IF DualOp('A','L') THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$31;
      END
     ELSE
      BEGIN
       IF (NLS_StrCaseCmp(ArgStr[1],'A')<>0) AND (NLS_StrCaseCmp(ArgStr[1],'HL')<>0) THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       IF (NLS_StrCaseCmp(ArgStr[1],'A')<>0) AND (NLS_StrCaseCmp(ArgStr[1],'HL')<>0) THEN WrError(1350)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2],MModAbs);
	 IF AdrType<>ModNone THEN
          IF (NLS_StrCaseCmp(ArgStr[1],'HL')=0) AND (AdrVal AND 3<>0) THEN WrError(1325)
	  ELSE
	   BEGIN
	    CodeLen:=2;
            BAsmCode[0]:=$29+($14*Ord(NLS_StrCaseCmp(ArgStr[1],'A')=0));
	    BAsmCode[1]:=AdrVal;
	   END;
	END;
      END;
     Exit;
    END;

   IF Memo('IN') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModPort);
       IF AdrType<>ModNone THEN
	BEGIN
	 HReg:=AdrVal;
	 DecodeAdr(ArgStr[2],MModAcc+MModIHL);
	 CASE AdrType OF
	 ModAcc:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$3a;
	   BAsmcode[1]:=HReg AND $0f+(((HReg AND $10) XOR $10) SHL 1);
	  END;
	 ModIHL:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$3a;
	   BAsmcode[1]:=$40+HReg AND $0f+(((HReg AND $10) XOR $10) SHL 1);
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   IF Memo('OUT') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModPort);
       IF AdrType<>ModNone THEN
	BEGIN
	 HReg:=AdrVal; OpSize:=0;
	 DecodeAdr(ArgStr[1],MModAcc+MModIHL+MModImm);
	 CASE AdrType OF
	 ModAcc:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$3a;
	   BAsmCode[1]:=$80+((HReg AND $10) SHL 1)+((HReg AND $0f) XOR 4);
	  END;
	 ModIHL:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$3a;
	   BAsmCode[1]:=$c0+((HReg AND $10) SHL 1)+((HReg AND $0f) XOR 4);
	  END;
	 ModImm:
	  IF HReg>$0f THEN WrError(1110)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$2c;
	    BAsmCode[1]:=(AdrVal SHL 4)+HReg;
	   END;
	 END;
	END;
      END;
     Exit;
    END;

   IF Memo('OUTB') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'@HL')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$12;
      END;
     Exit;
    END;

   { Arithmetik }

   IF Memo('CMPR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModSAbs+MModH+MModL);
       CASE AdrType OF
       ModAcc:
	BEGIN
	 DecodeAdr(ArgStr[2],MModIHL+MModAbs+MModImm);
	 CASE AdrType OF
	 ModIHL:
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$16;
	  END;
	 ModAbs:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$3e; BAsmCode[1]:=AdrVal;
	  END;
	 ModImm:
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$d0+AdrVal;
	  END;
	 END;
	END;
       ModSAbs:
	BEGIN
	 OpSize:=0; HReg:=AdrVal; DecodeAdr(ArgStr[2],MModImm);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$2e;
	   BAsmCode[1]:=AdrVal SHL 4+HReg;
	  END;
	END;
       ModH,ModL:
	BEGIN
	 HReg:=AdrType;
	 DecodeAdr(ArgStr[2],MModImm);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$38;
	   BAsmCode[1]:=$90+(Ord(HReg=ModH) SHL 6)+AdrVal;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('ADD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModIHL+MModSAbs+MModL+MModH);
       CASE AdrType OF
       ModAcc:
	BEGIN
	 DecodeAdr(ArgStr[2],MModIHL+MModImm);
	 CASE AdrType OF
	 ModIHL:
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$17;
	  END;
	 ModImm:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$38; BAsmCode[1]:=AdrVal;
	  END;
	 END;
	END;
       ModIHL:
	BEGIN
	 OpSize:=0; DecodeAdr(ArgStr[2],MModImm);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$38; BAsmCode[1]:=$40+AdrVal;
	  END;
	END;
       ModSAbs:
	BEGIN
	 HReg:=AdrVal; OpSize:=0; DecodeAdr(ArgStr[2],MModImm);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$2f; BAsmCode[1]:=AdrVal SHL 4+HReg;;
	  END;
	END;
       ModH,ModL:
	BEGIN
	 HReg:=Ord(AdrType=ModH); DecodeAdr(ArgStr[2],MModImm);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$38; BAsmCode[1]:=$80+(HReg SHL 6)+AdrVal;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('ADDC') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'A')<>0) OR (NLS_StrCaseCmp(ArgStr[2],'@HL')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$15;
      END;
     Exit;
    END;

   IF Memo('SUBRC') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'A')<>0) OR (NLS_StrCaseCmp(ArgStr[2],'@HL')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$14;
      END;
     Exit;
    END;

   IF Memo('SUBR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=0; DecodeAdr(ArgStr[2],MModImm);
       IF AdrType<>ModNone THEN
	BEGIN
	 HReg:=AdrVal;
	 DecodeAdr(ArgStr[1],MModAcc+MModIHL);
	 CASE AdrType OF
	 ModAcc:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$38; BAsmCode[1]:=$10+HReg;
	  END;
	 ModIHL:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$38; BAsmCode[1]:=$50+HReg;
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   IF (Memo('INC')) OR (Memo('DEC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       HReg:=Ord(Memo('DEC'));
       DecodeAdr(ArgStr[1],MModAcc+MModIHL+MModL);
       CASE AdrType OF
       ModAcc:
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$08+HReg;
	END;
       ModL:
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$18+HReg;
	END;
       ModIHL:
	BEGIN
	 CodeLen:=1; BAsmCode[0]:=$0a+HReg;
	END;
       END;
      END;
     Exit;
    END;

   { Logik }

   IF (Memo('AND')) OR (Memo('OR')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       HReg:=Ord(Memo('OR'));
       DecodeAdr(ArgStr[1],MModAcc+MModIHL);
       CASE AdrType OF
       ModAcc:
	BEGIN
	 DecodeAdr(ArgStr[2],MModImm+MModIHL);
	 CASE AdrType OF
	 ModIHL:
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$1e-HReg;
	  END;
	 ModImm:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$38; BAsmCode[1]:=$30-(HReg SHL 4)+AdrVal;
	  END;
	 END;
	END;
       ModIHL:
	BEGIN
	 SetOpSize(0); DecodeAdr(ArgStr[2],MModImm);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$38; BAsmCode[1]:=$70-(HReg SHL 4)+AdrVal;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('XOR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'A')<>0) OR (NLS_StrCaseCmp(ArgStr[2],'@HL')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$1f;
      END;
     Exit;
    END;

   IF (Memo('ROLC')) OR (Memo('RORC')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 HReg:=1; OK:=True;
	END
       ELSE HReg:=EvalIntExpression(ArgStr[2],Int8,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[0]:=$05+Ord(Memo('RORC')) SHL 1;
	 FOR z:=1 TO Pred(HReg) DO BAsmCode[z]:=BAsmCode[0];
	 CodeLen:=HReg;
	 IF HReg>=4 THEN WrError(160);
	END;
      END;
     Exit;
    END;

   FOR z:=0 TO BitOrderCnt-1 DO
    IF Memo(BitOrders[z]) THEN
     BEGIN
      IF ArgCnt=1 THEN
       IF NLS_StrCaseCmp(ArgStr[1],'@L')=0 THEN
	BEGIN
	 IF Memo('TESTP') THEN WrError(1350)
	 ELSE
	  BEGIN
	   IF z=2 THEN z:=3; CodeLen:=1; BAsmCode[0]:=$34+z;
	  END;
	END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'CF')=0 THEN
	BEGIN
	 IF z<2 THEN WrError(1350)
	 ELSE
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=10-2*z;
	  END;
	END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'ZF')=0 THEN
	BEGIN
	 IF z<>3 THEN WrError(1350)
	 ELSE
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$0e;
	  END;
	END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'GF')=0 THEN
	BEGIN
	 IF z=2 THEN WrError(1350)
	 ELSE
	  BEGIN
	   CodeLen:=1; IF z=3 THEN BAsmCode[0]:=1 ELSE BAsmCode[0]:=3-z;
	   ChkCPU(1);
	  END;
	END
       ELSE IF (NLS_StrCaseCmp(ArgStr[1],'DMB')=0) OR (NLS_StrCaseCmp(ArgStr[1],'DMB0')=0) THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$3b; BAsmCode[1]:=$39+(z SHL 6);
         ChkCPU(1 SHL (1+Ord(NLS_StrCaseCmp(ArgStr[1],'DMB0')=0)));
	END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'DMB1')=0 THEN
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=3; BAsmCode[1]:=$3b;
	 BAsmCode[2]:=$19+(z SHL 6);
	 ChkCPU(4);
	END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'STK13')=0
        THEN
	BEGIN
	 IF z>1 THEN WrError(1350)
	 ELSE
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=3-z; BAsmCode[1]:=$3a; BAsmCode[2]:=$84;
	   ChkCPU(4);
	  END;
	END
       ELSE WrError(1350)
      ELSE IF ArgCnt=2 THEN
       IF NLS_StrCaseCmp(ArgStr[1],'IL')=0 THEN
	BEGIN
	 IF z<>1 THEN WrError(1350)
	 ELSE
	  BEGIN
	   HReg:=EvalIntExpression(ArgStr[2],UInt6,OK);
	   IF OK THEN
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$36; BAsmCode[1]:=$c0+HReg;
	    END;
	  END;
	END
       ELSE
	BEGIN
	 HReg:=EvalIntExpression(ArgStr[2],UInt2,OK);
	 IF OK THEN
	  BEGIN
	   DecodeAdr(ArgStr[1],MModAcc+MModIHL+MModPort+MModSAbs);
	   CASE AdrType OF
	   ModAcc:
	    IF z<>2 THEN WrError(1350)
	    ELSE
	     BEGIN
	      CodeLen:=1; BAsmCode[0]:=$5c+HReg;
	     END;
	   ModIHL:
	    IF z=3 THEN WrError(1350)
	    ELSE
	     BEGIN
	      CodeLen:=1; BAsmCode[0]:=$50+HReg+(z SHL 2);
	     END;
	   ModPort:
	    IF AdrVal>15 THEN WrError(1320)
	    ELSE
	     BEGIN
	      CodeLen:=2; BAsmCode[0]:=$3b;
	      BAsmCode[1]:=(z SHL 6)+(HReg SHL 4)+AdrVal;
	     END;
	   ModSAbs:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$39;
	     BAsmCode[1]:=(z SHL 6)+(HReg SHL 4)+AdrVal;
	    END;
	   END;
	  END;
	END
      ELSE WrError(1110);
      Exit;
     END;

   IF (Memo('EICLR')) OR (Memo('DICLR')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'IL')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(ArgStr[2],UInt6,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$36;
	 Inc(BAsmCode[1],$40*(1+Ord(Memo('DICLR'))));
	END;
      END;
     Exit;
    END;

   { SprÅnge }

   IF Memo('BSS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF AdrWord SHR 6<>(EProgCounter+1) SHR 6 THEN WrError(1910)
	ELSE
	 BEGIN
	  ChkSpace(SegCode);
	  CodeLen:=1; BAsmCode[0]:=$80+(AdrWord AND $3f);
	 END;
      END;
     Exit;
    END;

   IF Memo('BS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF AdrWord SHR 12<>(EProgCounter+2) SHR 12 THEN WrError(1910)
	ELSE
	 BEGIN
	  ChkSpace(SegCode);
	  CodeLen:=2; BAsmCode[0]:=$60+(Hi(AdrWord) AND 15);
	  BAsmCode[1]:=Lo(AdrWord);
	 END;
      END;
     Exit;
    END;

   IF Memo('BSL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF AdrWord>ROMEnd THEN WrError(1320)
	ELSE
	 BEGIN
	  ChkSpace(SegCode);
	  CodeLen:=3;
	  CASE AdrWord SHR 12 OF
	  0:BAsmCode[0]:=$02;
	  1:BAsmCode[0]:=$03;
	  2:BAsmCode[0]:=$1c;
	  3:BAsmCode[0]:=$01;
	  END;
	  BAsmCode[1]:=$60+(Hi(AdrWord) AND $0f); BAsmCode[2]:=Lo(AdrWord);
	 END;
      END;
     Exit;
    END;

   IF Memo('B') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF AdrWord>ROMEnd THEN WrError(1320)
	ELSE
	 BEGIN
	  ChkSpace(SegCode);
	  IF AdrWord SHR 6=(EProgCounter+1) SHR 6 THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$80+(AdrWord AND $3f);
	   END
	  ELSE IF AdrWord SHR 12=(EProgCounter+2) SHR 12 THEN
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$60+(Hi(AdrWord) AND $0f);
	    BAsmCode[1]:=Lo(AdrWord);
	   END
	  ELSE
	   BEGIN
	    CodeLen:=3;
	    CASE AdrWord SHR 12 OF
	    0:BAsmCode[0]:=$02;
	    1:BAsmCode[0]:=$03;
	    2:BAsmCode[0]:=$1c;
	    3:BAsmCode[0]:=$01;
	    END;
	    BAsmCode[1]:=$60+(Hi(AdrWord) AND $0f); BAsmCode[2]:=Lo(AdrWord);
	   END;
	 END;
      END;
     Exit;
    END;

   IF Memo('CALLS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	BEGIN
	 IF AdrWord=$86 THEN AdrWord:=$06;
	 IF AdrWord AND $ff87<>6 THEN WrError(1135)
	 ELSE
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=(AdrWord SHR 3)+$70;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('CALL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
	IF (AdrWord XOR EProgCounter) AND $3800<>0 THEN WrError(1910)
	ELSE
	 BEGIN
	  ChkSpace(SegCode);
	  CodeLen:=2; BAsmCode[0]:=$20+(Hi(AdrWord) AND 7);
	  BAsmCode[1]:=Lo(AdrWord);
	 END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_47C00:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <=ROMEnd;
   SegData  : ok:=ProgCounter <=RAMEnd;
   SegIO    : ok:=ProgCounter <=PortEnd;
   ELSE ok:=False;
   END;
   ChkPC_47C00:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_47C00:Boolean;
	Far;
BEGIN
   IsDef_47C00:=Memo('PORT');
END;

        PROCEDURE SwitchFrom_47C00;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_47C00;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=True;

   PCSymbol:='$'; HeaderID:=$55; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData,SegIO];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=0;
   Grans[SegIO  ]:=1; ListGrans[SegIO  ]:=1; SegInits[SegIO  ]:=0;

   MakeCode:=MakeCode_47C00; ChkPC:=ChkPC_47C00; IsDef:=IsDef_47C00;
   SwitchFrom:=SwitchFrom_47C00;
END;

	PROCEDURE InitCode_47C00;
	Far;
BEGIN
   SaveInitProc;

   DMBAssume:=0;
END;

BEGIN
   CPU47C00:=AddCPU('47C00',SwitchTo_47C00);
   CPU470C00:=AddCPU('470C00',SwitchTo_47C00);
   CPU470AC00:=AddCPU('470AC00',SwitchTo_47C00);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_47C00;
END.
