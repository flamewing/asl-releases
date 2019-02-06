{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeZ8;

{ AS Codegeneratormodul Z8-Reihe }

INTERFACE
        Uses StringUt,Chunks,NLS,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[4];
	       Code:Byte;
	      END;

   ALU1Order=RECORD
	      Name:String[4];
	      Code:Byte;
	      Is16:Boolean;
	     END;

   Condition=RECORD
	      Name:String[3];
	      Code:Byte;
	     END;

CONST
   WorkOfs=$e0;

   FixedOrderCnt=12;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'CCF' ; Code:$ef),
		(Name:'DI'  ; Code:$8f),
		(Name:'EI'  ; Code:$9f),
		(Name:'HALT'; Code:$7f),
		(Name:'IRET'; Code:$bf),
		(Name:'NOP' ; Code:$ff),
		(Name:'RCF' ; Code:$cf),
		(Name:'RET' ; Code:$af),
		(Name:'SCF' ; Code:$df),
		(Name:'STOP'; Code:$6f),
		(Name:'WDH' ; Code:$4f),
		(Name:'WDT' ; Code:$5f));

   ALU2OrderCnt=10;
   ALU2Orders:ARRAY[1..ALU2OrderCnt] OF FixedOrder=
	      ((Name:'ADD' ; Code:$00),
	       (Name:'ADC' ; Code:$10),
	       (Name:'SUB' ; Code:$20),
	       (Name:'SBC' ; Code:$30),
	       (Name:'OR'  ; Code:$40),
	       (Name:'AND' ; Code:$50),
	       (Name:'TCM' ; Code:$60),
	       (Name:'TM'  ; Code:$70),
	       (Name:'CP'  ; Code:$a0),
	       (Name:'XOR' ; Code:$b0));

   ALU1OrderCnt=14;
   ALU1Orders:ARRAY[1..ALU1OrderCnt] OF ALU1Order=
	      ((Name:'DEC' ; Code:$00; Is16:False),
	       (Name:'RLC' ; Code:$10; Is16:False),
	       (Name:'DA'  ; Code:$40; Is16:False),
	       (Name:'POP' ; Code:$50; Is16:False),
	       (Name:'COM' ; Code:$60; Is16:False),
	       (Name:'PUSH'; Code:$70; Is16:False),
	       (Name:'DECW'; Code:$80; Is16:True ),
	       (Name:'RL'  ; Code:$90; Is16:False),
	       (Name:'INCW'; Code:$a0; Is16:True ),
	       (Name:'CLR' ; Code:$b0; Is16:False),
	       (Name:'RRC' ; Code:$c0; Is16:False),
	       (Name:'SRA' ; Code:$d0; Is16:False),
	       (Name:'RR'  ; Code:$e0; Is16:False),
	       (Name:'SWAP'; Code:$f0; Is16:False));

   CondCnt=20;
   TrueCond=2;
   Conditions:ARRAY[1..CondCnt] OF Condition=
	      ((Name:'F'  ; Code: 0),(Name:'T'  ; Code: 8),
	       (Name:'C'  ; Code: 7),(Name:'NC' ; Code:15),
	       (Name:'Z'  ; Code: 6),(Name:'NZ' ; Code:14),
	       (Name:'MI' ; Code: 5),(Name:'PL' ; Code:13),
	       (Name:'OV' ; Code: 4),(Name:'NOV'; Code:12),
	       (Name:'EQ' ; Code: 6),(Name:'NE' ; Code:14),
	       (Name:'LT' ; Code: 1),(Name:'GE' ; Code: 9),
	       (Name:'LE' ; Code: 2),(Name:'GT' ; Code:10),
	       (Name:'ULT'; Code: 7),(Name:'UGE'; Code:15),
	       (Name:'ULE'; Code: 3),(Name:'UGT'; Code:11));

   ModNone  =-1;
   ModWReg  = 0; MModWReg  = 1 SHL ModWReg;
   ModReg   = 1; MModReg   = 1 SHL ModReg;
   ModIWReg = 2; MModIWReg = 1 SHL ModIWReg;
   ModIReg  = 3; MModIReg  = 1 SHL ModIReg;
   ModImm   = 4; MModImm   = 1 SHL ModImm;
   ModRReg  = 5; MModRReg  = 1 SHL ModRReg;
   ModIRReg = 6; MModIRReg = 1 SHL ModIRReg;
   ModInd   = 7; MModInd   = 1 SHL ModInd;

VAR
   AdrType:ShortInt;
   AdrMode,AdrIndex:Byte;
   AdrWMode:Word;

   CPUZ8601,CPUZ8604,CPUZ8608,CPUZ8630,CPUZ8631:CPUVar;

	PROCEDURE DecodeAdr(Asc:String; Mask:Byte; Is16:Boolean);
LABEL
   AdrFound;
VAR
   OK:Boolean;
   z:Integer;

	FUNCTION IsWReg(Asc:String; VAR Erg:Byte):Boolean;
VAR
   Err:ValErgType;
BEGIN
   IF (Length(Asc)<2) OR (UpCase(Asc[1])<>'R') THEN IsWReg:=False
   ELSE
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,Err);
     IF Err<>0 THEN IsWReg:=False
     ELSE IsWReg:=Erg<=15;
    END;
END;

	FUNCTION IsRReg(VAR Asc:String; VAR Erg:Byte):Boolean;
VAR
   Err:ValErgType;
BEGIN
   IF (Length(Asc)<3) OR (NLS_StrCaseCmp(Copy(Asc,1,2),'RR')<>0) THEN IsRReg:=False
   ELSE
    BEGIN
     Val(Copy(Asc,3,Length(Asc)-2),Erg,Err);
     IF Err<>0 THEN IsRReg:=False
     ELSE IsRReg:=Erg<=15;
    END;
END;

	PROCEDURE CorrMode(Old,New:ShortInt);
BEGIN
   IF (AdrType=Old) AND (Mask AND (1 SHL Old)=0) THEN
    BEGIN
     AdrType:=New; Inc(AdrMode,WorkOfs);
    END;
END;

BEGIN
   AdrType:=ModNone;

   { immediate ? }

   IF Asc[1]='#' THEN
    BEGIN
     AdrMode:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),Int8,OK);
     IF OK THEN AdrType:=ModImm;
     Goto AdrFound;
    END;

   { Register ? }

   IF IsWReg(Asc,AdrMode) THEN
    BEGIN
     AdrType:=ModWReg; Goto AdrFound;
    END;

   IF IsRReg(Asc,AdrMode) THEN
    BEGIN
     IF Odd(AdrMode) THEN WrError(1351) ELSE AdrType:=ModRReg;
     Goto AdrFound;
    END;

   { indirekte Konstrukte ? }

   IF Asc[1]='@' THEN
    BEGIN
     Delete(Asc,1,1);
     IF IsWReg(Asc,AdrMode) THEN AdrType:=ModIWReg
     ELSE IF IsRReg(Asc,AdrMode) THEN
      BEGIN
       IF Odd(AdrMode) THEN WrError(1351) ELSE AdrType:=ModIRReg;
      END
     ELSE
      BEGIN
       AdrMode:=EvalIntExpression(Asc,Int8,OK);
       IF OK THEN
	BEGIN
	 AdrType:=ModIReg; ChkSpace(SegData);
	END;
      END;
     Goto AdrFound;
    END;

   { indiziert ? }

   IF (Asc[Length(Asc)]=')') AND (Length(Asc)>4) THEN
    BEGIN
     z:=Pred(Length(Asc));
     WHILE (z>1) AND (Asc[z]<>'(') DO Dec(z);
     IF Asc[z]<>'(' THEN WrError(1300)
     ELSE IF NOT IsWReg(Copy(Asc,z+1,Length(Asc)-z-1),AdrMode) THEN
      WrXError(1445,Copy(Asc,z+1,Length(Asc)-z-1))
     ELSE
      BEGIN
       AdrIndex:=EvalIntExpression(Copy(Asc,1,z-1),Int8,OK);
       IF OK THEN
        BEGIN
         AdrType:=ModInd; ChkSpace(SegData);
        END;
      END;
     Goto AdrFound;
    END;

   { einfache direkte Adresse ? }

   IF Is16 THEN AdrWMode:=EvalIntExpression(Asc,Int16,OK)
   ELSE AdrMode:=EvalIntExpression(Asc,Int8,OK);
   IF OK THEN
    BEGIN
     AdrType:=ModReg;
     IF Is16 THEN ChkSpace(SegCode) ELSE ChkSpace(SegData);
     Goto AdrFound;
    END;

AdrFound:

   IF NOT Is16 THEN
    BEGIN
     CorrMode(ModWReg,ModReg);
     CorrMode(ModIWReg,ModIReg);
    END;

   IF (AdrType<>ModNone) AND (Mask AND (1 SHL AdrType)=0) THEN
    BEGIN
     WrError(1350); AdrType:=ModNone;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   OK:Boolean;
   AdrByte:Byte;
BEGIN
   DecodePseudo:=True;

   IF Memo('SFR') THEN
    BEGIN
     FirstPassUnknown:=False;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrByte:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF (OK) AND (NOT FirstPassUnknown) THEN
	BEGIN
         PushLocHandle(-1);
	 EnterIntSymbol(LabPart,AdrByte,SegData,False);
         PopLocHandle;
	 IF MakeUseList THEN
	  IF AddChunk(SegChunks[SegData],AdrByte,1,False) THEN WrError(90);
	 ListLine:='='+HexString(AdrByte,2)+'H';
	END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;


	PROCEDURE MakeCode_Z8;
	Far;
VAR
   z,AdrInt:Integer;
   Save:Byte;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(True) THEN Exit;

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
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModWReg+MModIReg+MModIWReg+MModInd,False);
       CASE AdrType OF
       ModReg:
	BEGIN
	 Save:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg+MModWReg+MModIReg+MModImm,False);
         CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$e4;
	   BAsmCode[1]:=AdrMode; BAsmCode[2]:=Save;
	   CodeLen:=3;
	  END;
	 ModWReg:
	  BEGIN
	   BAsmCode[0]:=(AdrMode SHL 4)+9;
	   BAsmCode[1]:=Save;
	   CodeLen:=2;
	  END;
	 ModIReg:
	  BEGIN
	   BAsmCode[0]:=$e5;
	   BAsmCode[1]:=AdrMode; BAsmCode[2]:=Save;
	   CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$e6;
	   BAsmCode[1]:=Save; BAsmCode[2]:=AdrMode;
	   CodeLen:=3;
	  END;
	 END;
	END;
       ModWReg:
	BEGIN
	 Save:=AdrMode;
	 DecodeAdr(ArgStr[2],MModWReg+MModReg+MModIWReg+MModIReg+MModImm+MModInd,False);
	 CASE AdrType OF
	 ModWReg:
	  BEGIN
	   BAsmCode[0]:=(Save SHL 4)+8; BAsmCode[1]:=AdrMode+WorkOfs;
	   CodeLen:=2;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=(Save SHL 4)+8; BAsmCode[1]:=AdrMode;
	   CodeLen:=2;
	  END;
	 ModIWReg:
	  BEGIN
	   BAsmCode[0]:=$e3; BAsmCode[1]:=(Save SHL 4)+AdrMode;
	   CodeLen:=2;
	  END;
	 ModIReg:
	  BEGIN
	   BAsmCode[0]:=$e5;
           BAsmCode[1]:=AdrMode; BAsmCode[2]:=WorkOfs+Save;
	   CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=(Save SHL 4)+12; BAsmCode[1]:=AdrMode;
	   CodeLen:=2;
	  END;
	 ModInd:
	  BEGIN
	   BAsmCode[0]:=$c7;
	   BAsmCode[1]:=(Save SHL 4)+AdrMode; BAsmCode[2]:=AdrIndex;
	   CodeLen:=3;
	  END;
	 END;
	END;
       ModIReg:
	BEGIN
	 Save:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg+MModImm,False);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$f5;
	   BAsmCode[1]:=AdrMode; BAsmCode[2]:=Save;
	   CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$e7;
	   BAsmCode[1]:=Save; BAsmCode[2]:=AdrMode;
	   CodeLen:=3;
	  END;
	 END;
	END;
       ModIWReg:
	BEGIN
	 Save:=AdrMode;
	 DecodeAdr(ArgStr[2],MModWReg+MModReg+MModImm,False);
	 CASE AdrType OF
	 ModWReg:
	  BEGIN
	   BAsmCode[0]:=$f3; BAsmCode[1]:=(Save SHL 4)+AdrMode;
	   CodeLen:=2;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$f5;
	   BAsmCode[1]:=AdrMode; BAsmCode[2]:=WorkOfs+Save;
	   CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$e7;
	   BAsmCode[1]:=WorkOfs+Save; BAsmCode[2]:=AdrMode;
	   CodeLen:=3;
	  END;
	 END;
	END;
       ModInd:
	BEGIN
	 Save:=AdrMode;
	 DecodeAdr(ArgStr[2],MModWReg,False);
	 CASE AdrType OF
	 ModWReg:
	  BEGIN
	   BAsmCode[0]:=$d7;
	   BAsmCode[1]:=(AdrMode SHL 4)+Save; BAsmCode[2]:=AdrIndex;
	   CodeLen:=3;
	  END;
	 END;
	END;
       END;
      END;
     Exit;
    END;

   IF (Memo('LDC')) OR (Memo('LDE')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg+MModIRReg,False);
       CASE AdrType OF
       ModWReg:
	BEGIN
	 Save:=AdrMode; DecodeAdr(ArgStr[2],MModIRReg,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   IF Memo('LDC') THEN BAsmCode[0]:=$c2 ELSE BAsmCode[0]:=$82;
	   BAsmCode[1]:=(Save SHL 4)+AdrMode;
	   CodeLen:=2;
	  END;
	END;
       ModIRReg:
	BEGIN
	 Save:=AdrMode; DecodeAdr(ArgStr[2],MModWReg,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   IF Memo('LDC') THEN BAsmCode[0]:=$d2 ELSE BAsmCode[0]:=$92;
	   BAsmCode[1]:=(AdrMode SHL 4)+Save;
	   CodeLen:=2;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF (Memo('LDCI')) OR (Memo('LDEI')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModIWReg+MModIRReg,False);
       CASE AdrType OF
       ModIWReg:
	BEGIN
	 Save:=AdrMode; DecodeAdr(ArgStr[2],MModIRReg,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   IF Memo('LDCI') THEN BAsmCode[0]:=$c3 ELSE BAsmCode[0]:=$83;
	   BAsmCode[1]:=(Save SHL 4)+AdrMode;
	   CodeLen:=2;
	  END;
	END;
       ModIRReg:
	BEGIN
	 Save:=AdrMode; DecodeAdr(ArgStr[2],MModIWReg,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   IF Memo('LDCI') THEN BAsmCode[0]:=$d3 ELSE BAsmCode[0]:=$93;
	   BAsmCode[1]:=(AdrMode SHL 4)+Save;
	   CodeLen:=2;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=1 TO ALU2OrderCnt DO
    WITH ALU2Orders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg+MModWReg+MModIReg,False);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   Save:=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg+MModIReg+MModImm,False);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     BAsmCode[0]:=Code+4;
	     BAsmCode[1]:=AdrMode;
	     BAsmCode[2]:=Save;
	     CodeLen:=3;
	    END;
	   ModIReg:
	    BEGIN
	     BAsmCode[0]:=Code+5;
	     BAsmCode[1]:=AdrMode;
	     BAsmCode[2]:=Save;
	     CodeLen:=3;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=Code+6;
	     BAsmCode[1]:=Save;
	     BAsmCode[2]:=AdrMode;
	     CodeLen:=3;
	    END;
	   END;
	  END;
	 ModWReg:
	  BEGIN
	   Save:=AdrMode;
	   DecodeAdr(ArgStr[2],MModWReg+MModReg+MModIWReg+MModIReg+MModImm,False);
	   CASE AdrType OF
	   ModWReg:
	    BEGIN
	     BAsmCode[0]:=Code+2;
	     BAsmCode[1]:=(Save SHL 4)+AdrMode;
	     CodeLen:=2;
	    END;
	   ModReg:
	    BEGIN
	     BAsmCode[0]:=Code+4;
	     BAsmCode[1]:=AdrMode;
	     BAsmCode[2]:=WorkOfs+Save;
	     CodeLen:=3;
	    END;
	   ModIWReg:
	    BEGIN
	     BAsmCode[0]:=Code+3;
	     BAsmCode[1]:=(Save SHL 4)+AdrMode;
	     CodeLen:=2;
	    END;
	   ModIReg:
	    BEGIN
	     BAsmCode[0]:=Code+5;
	     BAsmCode[1]:=AdrMode;
	     BAsmCode[2]:=WorkOfs+Save;
	     CodeLen:=3;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=Code+6;
	     BAsmCode[1]:=Save+WorkOfs;
	     BAsmCode[2]:=AdrMode;
	     CodeLen:=3;
	    END;
	   END;
	  END;
	 ModIReg:
	  BEGIN
	   Save:=AdrMode;
	   DecodeAdr(ArgStr[2],MModImm,False);
	   CASE AdrType OF
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=Code+7;
	     BAsmCode[1]:=Save;
	     BAsmCode[2]:=AdrMode;
	     CodeLen:=3;
	    END;
	   END;
	  END;
	 END;
	END;
       Exit;
      END;

   { INC hat eine Optimierungsmîglichkeit }

   IF Memo('INC') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg+MModReg+MModIReg,False);
       CASE AdrType OF
       ModWReg:
	BEGIN
	 BAsmCode[0]:=(AdrMode SHL 4)+$0e; CodeLen:=1;
	END;
       ModReg:
	BEGIN
	 BAsmCode[0]:=$20; BAsmCode[1]:=AdrMode; CodeLen:=2;
	END;
       ModIReg:
	BEGIN
	 BAsmCode[0]:=$21; BAsmCode[1]:=AdrMode; CodeLen:=2;
	END;
       END;
      END;
     Exit;
    END;

   { ...alle anderen nicht }

   FOR z:=1 TO ALU1OrderCnt DO
    WITH ALU1Orders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 IF Is16 THEN DecodeAdr(ArgStr[1],MModRReg+MModReg+MModIReg,False)
	 ELSE DecodeAdr(ArgStr[1],MModReg+MModIReg,False);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=Code; BAsmCode[1]:=AdrMode; CodeLen:=2;
	  END;
	 ModRReg:
	  BEGIN
	   BAsmCode[0]:=Code; BAsmCode[1]:=WorkOfs+AdrMode; CodeLen:=2;
	  END;
	 ModIReg:
	  BEGIN
	   BAsmCode[0]:=Code+1; BAsmCode[1]:=AdrMode; CodeLen:=2;
	  END;
	 END;
	END;
       Exit;
      END;

   { SprÅnge }

   IF Memo('JR') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN z:=TrueCond
       ELSE
	BEGIN
         z:=1; NLS_UpString(ArgStr[1]);
	 WHILE (z<=CondCnt) AND (Conditions[z].Name<>ArgStr[1]) DO Inc(z);
	 IF z>CondCnt THEN WrError(1360);
	END;
       IF z<=CondCnt THEN
	BEGIN
	 AdrInt:=EvalIntExpression(ArgStr[ArgCnt],Int16,OK)-(EProgCounter+2);
	 IF OK THEN
	  IF (AdrInt>127) OR (AdrInt<-128) THEN WrError(1370)
	  ELSE
	   BEGIN
	    ChkSpace(SegCode);
            BAsmCode[0]:=(Conditions[z].Code SHL 4)+$0b;
	    BAsmCode[1]:=Lo(AdrInt);
	    CodeLen:=2;
	   END;
	END;
      END;
     Exit;
    END;

   IF Memo('DJNZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModWReg,False);
       IF AdrType<>ModNone THEN
	BEGIN
	 AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK)-(EProgCounter+2);
	 IF OK THEN
	  IF (AdrInt>127) OR (AdrInt<-128) THEN WrError(1370)
	  ELSE
	   BEGIN
	    BAsmCode[0]:=(AdrMode SHL 4)+$0a;
	    BAsmCode[1]:=Lo(AdrInt);
	    CodeLen:=2;
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
       DecodeAdr(ArgStr[1],MModIRReg+MModIReg+MModReg,True);
       CASE AdrType OF
       ModIRReg:
	BEGIN
	 BAsmCode[0]:=$d4; BAsmCode[1]:=$e0+AdrMode; CodeLen:=2;
	END;
       ModIReg:
	BEGIN
	 BAsmCode[0]:=$d4; BAsmCode[1]:=AdrMode; CodeLen:=2;
	END;
       ModReg:
	BEGIN
	 BAsmCode[0]:=$d6;
	 BAsmCode[1]:=Hi(AdrWMode); BAsmCode[2]:=Lo(AdrWMode);
	 CodeLen:=3;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('JP') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN z:=TrueCond
       ELSE
	BEGIN
         z:=1; NLS_UpString(ArgStr[1]);
	 WHILE (z<=CondCnt) AND (Conditions[z].Name<>ArgStr[1]) DO Inc(z);
	 IF z>CondCnt THEN WrError(1360);
	END;
       IF z<=CondCnt THEN
	BEGIN
	 DecodeAdr(ArgStr[ArgCnt],MModIRReg+MModIReg+MModReg,True);
	 CASE AdrType OF
	 ModIRReg:
	  IF z<>TrueCond THEN WrError(1350)
	  ELSE
	   BEGIN
	    BAsmCode[0]:=$30; BAsmCode[1]:=$e0+AdrMode; CodeLen:=2;
	   END;
	 ModIReg:
	  IF z<>TrueCond THEN WrError(1350)
	  ELSE
	   BEGIN
	    BAsmCode[0]:=$30; BAsmCode[1]:=AdrMode; CodeLen:=2;
	   END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=(Conditions[z].Code SHL 4)+$0d;
	   BAsmCode[1]:=Hi(AdrWMode); BAsmCode[2]:=Lo(AdrWMode);
	   CodeLen:=3;
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   { Sonderbefehle }

   IF Memo('SRP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModImm,False);
       IF AdrType=ModImm THEN
	IF (AdrMode AND 15<>0) OR ((AdrMode>$70) AND (AdrMode<$f0)) THEN WrError(120)
	ELSE
	 BEGIN
	  BAsmCode[0]:=$31; BAsmCode[1]:=AdrMode;
	  CodeLen:=2;
	 END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_Z8:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <$10000;
   SegData  : ok:=ProgCounter <  $100;
   ELSE ok:=False;
   END;
   ChkPC_Z8:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_Z8:Boolean;
	Far;
BEGIN
   IsDef_Z8:=Memo('SFR');
END;

        PROCEDURE SwitchFrom_Z8;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_Z8;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$79; NOPCode:=$ff;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=0;

   MakeCode:=MakeCode_Z8; ChkPC:=ChkPC_Z8; IsDef:=IsDef_Z8;
   SwitchFrom:=SwitchFrom_Z8;
END;

BEGIN
   CPUZ8601:=AddCPU('Z8601',SwitchTo_Z8);
   CPUZ8604:=AddCPU('Z8604',SwitchTo_Z8);
   CPUZ8608:=AddCPU('Z8608',SwitchTo_Z8);
   CPUZ8630:=AddCPU('Z8630',SwitchTo_Z8);
   CPUZ8631:=AddCPU('Z8631',SwitchTo_Z8);
END.

