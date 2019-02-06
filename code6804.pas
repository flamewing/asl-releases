{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

	Unit Code6804;

Interface

        Uses NLS,
             AsmDef,AsmPars,AsmSub,CodePseu;


Implementation

TYPE
   BaseOrder=RECORD
	      Name:String[4];
	      Code:LongInt;
	     END;

CONST
   FixedOrderCnt=19;
   FixedOrders:ARRAY[1..FixedOrderCnt] OF BaseOrder=
	       ((Name:'CLRA'; Code:$00fbff),
		(Name:'CLRX'; Code:$b08000),
		(Name:'CLRY'; Code:$b08100),
		(Name:'COMA'; Code:$0000b4),
		(Name:'ROLA'; Code:$0000b5),
		(Name:'ASLA'; Code:$00faff),
		(Name:'INCA'; Code:$00feff),
		(Name:'INCX'; Code:$0000a8),
		(Name:'INCY'; Code:$0000a9),
		(Name:'DECA'; Code:$00ffff),
		(Name:'DECX'; Code:$0000b8),
		(Name:'DECY'; Code:$0000b9),
		(Name:'TAX' ; Code:$0000bc),
		(Name:'TAY' ; Code:$0000bd),
		(Name:'TXA' ; Code:$0000ac),
		(Name:'TYA' ; Code:$0000ad),
		(Name:'RTS' ; Code:$0000b3),
		(Name:'RTI' ; Code:$0000b2),
		(Name:'NOP' ; Code:$000020));

   RelOrderCnt=6;
   RelOrders:ARRAY[1..RelOrderCnt] OF BaseOrder=
	     ((Name:'BCC' ; Code:$40),
	      (Name:'BHS' ; Code:$40),
	      (Name:'BCS' ; Code:$60),
	      (Name:'BLO' ; Code:$60),
	      (Name:'BNE' ; Code:$00),
	      (Name:'BEQ' ; Code:$20));

   ALUOrderCnt=4;
   ALUOrders:ARRAY[1..ALUOrderCnt] OF BaseOrder=
	     ((Name:'ADD'; Code:$02),
	      (Name:'SUB'; Code:$03),
	      (Name:'CMP'; Code:$04),
	      (Name:'AND'; Code:$05));

   ModNone=-1;
   ModInd=0;    MModInd=1 SHL ModInd;
   ModDir=1;    MModDir=1 SHL ModDir;
   ModImm=2;    MModImm=1 SHL ModImm;

VAR
   AdrMode:ShortInt;
   AdrVal:Byte;

   CPU6804:CPUVar;


	PROCEDURE DecodeAdr(VAR Asc:String; MayImm:Boolean);
LABEL
   Found;
VAR
   OK:Boolean;

BEGIN
   AdrMode:=ModNone;

   IF NLS_StrCaseCmp(Asc,'(X)')=0 THEN
    BEGIN
     AdrMode:=ModInd; AdrVal:=$00; Goto Found;
    END;
   IF NLS_StrCaseCmp(Asc,'(Y)')=0 THEN
    BEGIN
     AdrMode:=ModInd; AdrVal:=$10; Goto Found;
    END;

   IF Asc[1]='#' THEN
    BEGIN
     AdrVal:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),Int8,OK);
     IF OK THEN AdrMode:=ModImm; Goto Found;
    END;

   AdrVal:=EvalIntExpression(Asc,Int8,OK);
   IF OK THEN
    BEGIN
     AdrMode:=ModDir; ChkSpace(SegData);
    END;

Found:
   IF (AdrMode=ModImm) AND (NOT MayImm) THEN
    BEGIN
     WrError(1350); AdrMode:=ModNone;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   z,z2:Integer;
   OK:Boolean;
   HVal8:ShortInt;
   HVal16:Integer;
BEGIN
   DecodePseudo:=True;

   IF Memo('SFR') THEN
    BEGIN
     CodeEquate(SegData,0,$ff);
     Exit;
    END;

   DecodePseudo:=False;
END;

	FUNCTION IsShort(Adr:Byte):Boolean;
BEGIN
   IsShort:=(Adr AND $fc)=$80;
END;

	PROCEDURE MakeCode_6804;
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

   IF DecodeMotoPseudo(True) THEN Exit;

   { Anweisungen ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
	BEGIN
	 IF Code SHR 16<>0 THEN CodeLen:=3
	 ELSE CodeLen:=1+Ord(Hi(Code)<>0);
	 IF CodeLen=3 THEN BAsmCode[0]:=Code SHR 16;
	 IF CodeLen>=2 THEN BAsmCode[CodeLen-2]:=Hi(Code);
	 BAsmCode[CodeLen-1]:=Lo(Code);
	END;
       Exit;
      END;

   { relative/absolute SprÅnge }

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
	    CodeLen:=1; BAsmCode[0]:=Code+(AdrInt AND $1f);
	    ChkSpace(SegCode);
	   END;
	END;
       Exit;
      END;

   IF (Memo('JSR')) OR (Memo('JMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],UInt12,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=2; BAsmCode[1]:=Lo(AdrInt);
	 BAsmCode[0]:=$80+(Ord(Memo('JMP')) SHL 4)+(Hi(AdrInt) AND 15);
         ChkSpace(SegCode);
	END;
      END;
     Exit;
    END;

   { AKKU-Operationen }

   FOR z:=1 TO ALUOrderCnt DO
    WITH ALUOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],True);
	 CASE AdrMode OF
	 ModInd:BEGIN
		 CodeLen:=1; BAsmCode[0]:=$e0+AdrVal+Code;
		END;
	 ModDir:BEGIN
		 CodeLen:=2; BAsmCode[0]:=$f8+Code; BAsmCode[1]:=AdrVal;
		END;
	 ModImm:BEGIN
		 CodeLen:=2; BAsmCode[0]:=$e8+Code; BAsmCode[1]:=AdrVal;
		END;
	 END;
	END;
       Exit;
      END;

   { Datentransfer }

   IF (Memo('LDA')) OR (Memo('STA')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Memo('LDA'));
       AdrInt:=Ord(Memo('STA'));
       CASE AdrMode OF
       ModInd:BEGIN
	       CodeLen:=1; BAsmCode[0]:=$e0+AdrInt+AdrVal;
	      END;
       ModDir:IF IsShort(AdrVal) THEN
	       BEGIN
		CodeLen:=1; BAsmCode[0]:=$ac+(AdrInt SHL 4)+(AdrVal AND 3);
	       END
	      ELSE
	       BEGIN
		CodeLen:=2; BAsmCode[0]:=$f8+AdrInt; BAsmCode[1]:=AdrVal;
	       END;
       ModImm:BEGIN
	       CodeLen:=2; BAsmCode[0]:=$e8+AdrInt; BAsmCode[1]:=AdrVal;
	      END;
       END;
      END;
     Exit;
    END;

   IF (Memo('LDXI')) OR (Memo('LDYI')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[2]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),Int8,OK);
       IF OK THEN
	BEGIN
	 CodeLen:=3; BAsmCode[0]:=$b0;
	 BAsmCode[1]:=$80+Ord(Memo('LDYI'));
	END;
      END;
     Exit;
    END;

   IF Memo('MVI') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF ArgStr[2][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF OK THEN
	BEGIN
	 ChkSpace(SegData);
	 BAsmCode[2]:=EvalIntExpression(Copy(ArgStr[2],2,Length(ArgStr[2])-1),Int8,OK);
	 IF OK THEN
	  BEGIN
	   BAsmCode[0]:=$b0; CodeLen:=3;
	  END;
	END;
      END;
     Exit;
    END;

   { Read/Modify/Write-Operationen }

   IF (Memo('INC')) OR (Memo('DEC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],False);
       AdrInt:=Ord(Memo('DEC'));
       CASE AdrMode OF
       ModInd:BEGIN
	       CodeLen:=1; BAsmCode[0]:=$e6+AdrInt+AdrVal;
	      END;
       ModDir:IF IsShort(AdrVal) THEN
	       BEGIN
		CodeLen:=1; BAsmCode[0]:=$a8+(AdrInt SHL 4)+(AdrVal AND 3);
	       END
	      ELSE
	       BEGIN
		CodeLen:=2; BAsmCode[0]:=$fe+AdrInt; BAsmCode[1]:=AdrVal;
	       END;
       END;
      END;
     Exit;
    END;

   { Bitbefehle }

   IF (Memo('BSET')) OR (Memo('BCLR')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrVal:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[0]:=$d0+(Ord(Memo('BSET')) SHL 3)+AdrVal;
	 BAsmCode[1]:=EvalIntExpression(ArgStr[2],Int8,OK);
	 IF OK THEN
	  BEGIN
	   CodeLen:=2; ChkSpace(SegData);
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('BRSET')) OR (Memo('BRCLR')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       AdrVal:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[0]:=$c0+(Ord(Memo('BRSET')) SHL 3)+AdrVal;
	 BAsmCode[1]:=EvalIntExpression(ArgStr[2],Int8,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegData);
	   AdrInt:=EvalIntExpression(ArgStr[3],Int16,OK)-(EProgCounter+3);
	   IF OK THEN
	    IF (AdrInt<-128) OR (AdrInt>127) THEN WrError(1370)
	    ELSE
	     BEGIN
	      ChkSpace(SegCode); BAsmCode[2]:=AdrInt AND $ff; CodeLen:=3;
	     END;
	  END;
	END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_6804:Boolean;
	Far;
BEGIN
   IF ProgCounter<0 THEN ChkPC_6804:=False
   ELSE
    CASE ActPC OF
    SegCode:ChkPC_6804:=ProgCounter<=$fff;
    SegData:ChkPC_6804:=ProgCounter<=$ff;
    ELSE ChkPC_6804:=False;
    END;
END;

	FUNCTION IsDef_6804:Boolean;
	Far;
BEGIN
   IsDef_6804:=Memo('SFR');
END;

        PROCEDURE SwitchFrom_6804;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_6804;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='PC'; HeaderID:=$64; NOPCode:=$20;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=0;

   MakeCode:=MakeCode_6804; ChkPC:=ChkPC_6804; IsDef:=IsDef_6804;
   SwitchFrom:=SwitchFrom_6804;
END;

BEGIN
   CPU6804:=AddCPU('6804',SwitchTo_6804);
END.
