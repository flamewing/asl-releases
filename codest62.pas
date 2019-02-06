{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeST62;

{ AS Codegeneratormodul ST6210..ST6225 }

INTERFACE
        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[4];
	       Code:Byte;
	      END;

   AccOrder=RECORD
	     Name:String[4];
	     Code:Word;
	    END;

CONST
   FixedOrderCnt=5;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF FixedOrder=
	       ((Name:'NOP' ; Code:$04),
		(Name:'RET' ; Code:$cd),
		(Name:'RETI'; Code:$4d),
		(Name:'STOP'; Code:$6d),
		(Name:'WAIT'; Code:$ed));

   RelOrderCnt=4;
   RelOrders:ARRAY[1..RelOrderCnt] OF FixedOrder=
	       ((Name:'JRZ' ; Code:$04),
		(Name:'JRNZ'; Code:$00),
		(Name:'JRC' ; Code:$06),
		(Name:'JRNC'; Code:$02));

   ALUOrderCnt=4;
   ALUOrders:ARRAY[1..ALUOrderCnt] OF FixedOrder=
	       ((Name:'ADD' ; Code:$47),
		(Name:'AND' ; Code:$a7),
		(Name:'CP'  ; Code:$27),
		(Name:'SUB' ; Code:$c7));

   AccOrderCnt=3;
   AccOrders:ARRAY[1..AccOrderCnt] OF AccOrder=
	       ((Name:'COM' ; Code:$002d),
		(Name:'RLC' ; Code:$00ad),
		(Name:'SLA' ; Code:$ff5f));

ModNone=-1;
ModAcc=0;     MModAcc=1 SHL ModAcc;
ModDir=1;     MModDir=1 SHL ModDir;
ModInd=2;     MModInd=1 SHL ModInd;

VAR
   AdrMode:Byte;
   AdrType:ShortInt;
   AdrVal,AdrCnt:Byte;

   WinAssume:LongInt;

   SaveInitProc:PROCEDURE;

   CPUST6210,CPUST6215,CPUST6220,CPUST6225:CPUVar;


	PROCEDURE DecodeAdr(Asc:String; Mask:Byte);
CONST
   RegCnt=5;
   RegNames:ARRAY[1..RegCnt] OF Char='AVWXY';
   RegCodes:ARRAY[1..RegCnt] OF Byte=($ff,$82,$83,$80,$81);

LABEL
   AdrFnd;

VAR
   Ok:Boolean;
   z,AdrInt:Integer;

	PROCEDURE ResetAdr;
BEGIN
   AdrType:=ModNone; AdrCnt:=0;
END;

BEGIN
   ResetAdr;

   IF (NLS_StrCaseCmp(Asc,'A')=0) AND (Mask AND MModAcc<>0) THEN
    BEGIN
     AdrType:=ModAcc; Goto AdrFnd;
    END;

   FOR z:=1 TO RegCnt DO
    IF NLS_StrCaseCmp(Asc,RegNames[z])=0 THEN
     BEGIN
      AdrType:=ModDir; AdrCnt:=1; AdrVal:=RegCodes[z];
      Goto AdrFnd;
     END;

   IF NLS_StrCaseCmp(Asc,'(X)')=0 THEN
    BEGIN
     AdrType:=ModInd; AdrMode:=0; Goto AdrFnd;
    END;

   IF NLS_StrCaseCmp(Asc,'(Y)')=0 THEN
    BEGIN
     AdrType:=ModInd; AdrMode:=1; Goto AdrFnd;
    END;

   AdrInt:=EvalIntExpression(Asc,Int16,OK);
   IF OK THEN
    IF TypeFlag AND (1 SHL SegCode)<>0 THEN
     BEGIN
      AdrType:=ModDir; AdrVal:=(AdrInt AND $3f)+$40; AdrCnt:=1;
      IF NOT FirstPassUnknown THEN
       IF WinAssume<>AdrInt SHR 6 THEN WrError(110);
     END
    ELSE
     BEGIN
      IF FirstPassUnknown THEN AdrInt:=Lo(AdrInt);
      IF AdrInt>$ff THEN WrError(1320)
      ELSE
       BEGIN
	AdrType:=ModDir; AdrVal:=AdrInt; Goto AdrFnd;
       END;
     END;

AdrFnd:
   IF (AdrType<>ModNone) AND (Mask AND (1 SHL AdrType)=0) THEN
     BEGIN
      ResetAdr; WrError(1350);
     END;
END;

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUME62Count=1;
   ASSUME62s:ARRAY[1..ASSUME62Count] OF ASSUMERec=
	     ((Name:'ROMBASE'; Dest:@WinAssume; Min:0; Max:$3f; NothingVal:$40));
VAR
   HLocHandle:LongInt;
   AdrByte:Byte;
   OK,Flag:Boolean;
   z:Integer;
   s:String;
BEGIN
   DecodePseudo:=True;

   IF Memo('SFR') THEN
    BEGIN
     CodeEquate(SegData,0,$ff);
     Exit;
    END;

   IF (Memo('ASCII')) OR (Memo('ASCIZ')) THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       z:=1; Flag:=Memo('ASCIZ');
       REPEAT
	s:=EvalStringExpression(ArgStr[z],OK);
	IF OK THEN
	 BEGIN
	  TranslateString(s);
	  IF CodeLen+Length(s)+Ord(Flag)>MaxCodeLen THEN
	   BEGIN
	    WrError(1920); OK:=False;
	   END
	  ELSE
	   BEGIN
	    Move(s[1],BAsmCode[CodeLen],Length(s)); Inc(CodeLen,Length(s));
	    IF Flag THEN
	     BEGIN
	      BAsmCode[CodeLen]:=0; Inc(CodeLen);
	     END;
	   END;
	 END;
	Inc(z);
       UNTIL (NOT OK) OR (z>ArgCnt);
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF Memo('BYTE') THEN
    BEGIN
     OpPart:='BYT'; IF DecodeMotoPseudo(False) THEN;
     Exit;
    END;

   IF Memo('WORD') THEN
    BEGIN
     OpPart:='ADR'; IF DecodeMotoPseudo(False) THEN;
     Exit;
    END;

   IF Memo('BLOCK') THEN
    BEGIN
     OpPart:='DFS'; IF DecodeMotoPseudo(False) THEN;
     Exit;
    END;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME62s,ASSUME62Count);
     Exit;
    END;

   DecodePseudo:=False;
END;

	FUNCTION IsReg(Adr:Byte):Boolean;
BEGIN
   IsReg:=Adr AND $fc=$80;
END;

        FUNCTION MirrBit(inp:Byte):Byte;
BEGIN
   MirrBit:=((inp AND 1) SHL 2)+(inp AND 2)+((inp AND 4) SHR 2);
END;

	PROCEDURE MakeCode_ST62;
	Far;
VAR
   z,AdrInt:Integer;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

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
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModDir+MModInd);
       CASE AdrType OF
       ModAcc:
	BEGIN
	 DecodeAdr(ArgStr[2],MModDir+MModInd);
	 CASE AdrType OF
	 ModDir:
	  IF IsReg(AdrVal) THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$35+((AdrVal AND 3) SHL 6);
	   END
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$1f; BAsmCode[1]:=AdrVal;
	   END;
	 ModInd:
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$07+(AdrMode SHL 3);
	  END;
	 END;
	END;
       ModDir:
	BEGIN
	 DecodeAdr(ArgStr[2],MModAcc);
	 IF AdrType<>ModNone THEN
	  IF IsReg(AdrVal) THEN
	   BEGIN
	    CodeLen:=1; BAsmCode[0]:=$3d+((AdrVal AND 3) SHL 6);
	   END
	  ELSE
	   BEGIN
	    CodeLEn:=2; BAsmCode[0]:=$9f; BAsmCode[1]:=AdrVal;
	   END;
	END;
       ModInd:
	BEGIN
	 DecodeAdr(ArgStr[2],MModAcc);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=1; BAsmCode[0]:=$87+(AdrMode SHL 3);
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('LDI') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[2],Int8,OK);
       IF OK THEN
	BEGIN
	 DecodeAdr(ArgStr[1],MModAcc+MModDir);
	 CASE AdrType OF
	 ModAcc:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$17; BAsmCode[1]:=Lo(AdrInt);
	  END;
	 ModDir:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$0d; BAsmCode[1]:=AdrVal;
	   BAsmCode[2]:=Lo(AdrInt);
	  END;
	 END
	END;
      END;
     Exit;
    END;

   { SprÅnge }

   FOR z:=1 TO RelOrderCnt DO
    WITH RelOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK)-(EProgCounter+1);
	 IF OK THEN
	  IF (AdrInt<-16) OR (AdrInt>15) THEN WrError(1370)
	  ELSE
	   BEGIN
	    CodeLen:=1;
	    BAsmCode[0]:=Code+((AdrInt SHL 3) AND $f8);
	   END;
	END;
       Exit;
      END;

   IF (Memo('JP')) OR (Memo('CALL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
        IF (AdrInt<0) OR (AdrInt>$fff) THEN WrError(1925)
        ELSE
         BEGIN
          CodeLen:=2;
          BAsmCode[0]:=$01+(Ord(Memo('JP')) SHL 3)+((AdrInt AND $00f) SHL 4);
          BAsmCode[1]:=AdrInt SHR 4;
         END;
      END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=1 TO ALUOrderCnt DO
    WITH ALUOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModAcc);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   DecodeAdr(ArgStr[2],MModDir+MModInd);
	   CASE AdrType OF
	   ModDir:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=Code+$18; BAsmCode[1]:=AdrVal;
	    END;
	   ModInd:
	    BEGIN
	     CodeLen:=1; BAsmCode[0]:=Code+(AdrMode SHL 3);
	    END;
	   END;
	  END;
	END;
       Exit;
      END
     ELSE IF Memo(Name+'I') THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModAcc);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[1]:=EvalIntExpression(ArgStr[2],Int8,OK);
	   IF OK THEN
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=Code+$10;
	    END;
	  END;
	END;
       Exit;
      END;

   IF Memo('CLR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModDir);
       CASE AdrType OF
       ModAcc:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$df; BAsmCode[1]:=$ff;
	END;
       ModDir:
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$0d; BAsmCode[1]:=AdrVal; BAsmCode[2]:=0;
	END;
       END;
      END;
     Exit;
    END;

   FOR z:=1 TO AccOrderCnt DO
    WITH AccOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModAcc);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   OK:=Hi(Code)<>0;
	   CodeLen:=1+Ord(OK);
	   BAsmCode[0]:=Lo(Code);
	   IF OK THEN BAsmCode[1]:=Hi(Code);
	  END;
	END;
       Exit;
      END;

   IF (Memo('INC')) OR (Memo('DEC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModDir+MModInd);
       CASE AdrType OF
       ModDir:
	IF IsReg(AdrVal) THEN
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$15+((AdrVal AND 3) SHL 6);
	  IF Memo('DEC') THEN Inc(BAsmCode[0],8);
	 END
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$7f+(Ord(Memo('DEC')) SHL 7);
	  BAsmCode[1]:=AdrVal;
	 END;
       ModInd:
	BEGIN
	 CodeLen:=1;
	 BAsmCode[0]:=$67+(AdrMode SHL 3)+(Ord(Memo('DEC')) SHL 7);
	END;
       END;
      END;
     Exit;
    END;

   { Bitbefehle }

   IF (Memo('SET')) OR (Memo('RES')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[0]:=MirrBit(EvalIntExpression(ArgStr[1],UInt3,OK));
       IF OK THEN
	BEGIN
	 DecodeAdr(ArgStr[2],MModDir);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=2;
	   BAsmCode[0]:=(BAsmCode[0] SHL 5)+(Ord(Memo('SET')) SHL 4)+$0b;
	   BAsmCode[1]:=AdrVal;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('JRR')) OR (Memo('JRS')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[0]:=MirrBit(EvalIntExpression(ArgStr[1],UInt3,OK));
       IF OK THEN
	BEGIN
	 BAsmCode[0]:=(BAsmCode[0] SHL 5)+3+(Ord(Memo('JRS')) SHL 4);
	 DecodeAdr(ArgStr[2],MModDir);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[1]:=AdrVal;
           AdrInt:=EvalIntExpression(ArgStr[3],Int16,OK)-(EProgCounter+3);
	   IF OK THEN
	    IF (AdrInt>127) OR (AdrInt<-128) THEN WrError(1370)
	    ELSE
	     BEGIN
	      CodeLen:=3; BAsmCode[2]:=Lo(AdrInt);
	     END;
	  END;
	END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_ST62;
	Far;
BEGIN
   SaveInitProc;
   WinAssume:=$40;
END;

	FUNCTION ChkPC_ST62:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : IF MomCPU<CPUST6220 THEN ok:=ProgCounter<$1000
				  ELSE ok:=ProgCounter< $800;
   SegData  : ok:=ProgCounter < $100;
   ELSE ok:=False;
   END;
   ChkPC_ST62:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_ST62:Boolean;
	Far;
BEGIN
   IsDef_ST62:=Memo('SFR');
END;

        PROCEDURE SwitchFrom_ST62;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_ST62;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=True;

   PCSymbol:='PC'; HeaderID:=$78; NOPCode:=$04;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=0;

   MakeCode:=MakeCode_ST62; ChkPC:=ChkPC_ST62; IsDef:=IsDef_ST62;
   SwitchFrom:=SwitchFrom_ST62;
END;

BEGIN
   CPUST6210:=AddCPU('ST6210',SwitchTo_ST62);
   CPUST6215:=AddCPU('ST6215',SwitchTo_ST62);
   CPUST6220:=AddCPU('ST6220',SwitchTo_ST62);
   CPUST6225:=AddCPU('ST6225',SwitchTo_ST62);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_ST62;
END.

