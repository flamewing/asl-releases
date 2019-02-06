{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

	UNIT CodeAVR;

{ AS - Codegenerator fÅr die Atmel AVR-Reihe }

INTERFACE
	Uses Chunks,NLS,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[5];
	       Code:Word;
	      END;

   ArchOrder=RECORD
	       Name:String[5];
	       MinCPU:CPUvar;
	       Code:Word;
	      END;

CONST
   FixedOrderCnt=24;
   Reg1OrderCnt=10;
   Reg2OrderCnt=12;
   Reg3OrderCnt=4;
   ImmOrderCnt=7;
   RelOrderCnt=18;
   BitOrderCnt=4;
   PBitOrderCnt=4;

TYPE
   FixedOrderArray=ARRAY[1..FixedOrderCnt] OF ArchOrder;
   Reg1OrderArray=ARRAY[1..Reg1OrderCnt] OF ArchOrder;
   Reg2OrderArray=ARRAY[1..Reg2OrderCnt] OF ArchOrder;
   Reg3OrderArray=ARRAY[1..Reg3OrderCnt] OF FixedOrder;
   ImmOrderArray=ARRAY[1..ImmOrderCnt] OF FixedOrder;
   RelOrderArray=ARRAY[1..RelOrderCnt] OF FixedOrder;
   BitOrderArray=ARRAY[1..BitOrderCnt] OF FixedOrder;
   PBitOrderArray=ARRAY[1..PBitOrderCnt] OF FixedOrder;

VAR
   CPU90S1200,CPU90S2313,CPU90S4414,CPU90S8515:CPUVar;

   FixedOrders:^FixedOrderArray;
   Reg1Orders:^Reg1OrderArray;
   Reg2Orders:^Reg2OrderArray;
   Reg3Orders:^Reg3OrderArray;
   ImmOrders:^ImmOrderArray;
   RelOrders:^RelOrderArray;
   BitOrders:^BitOrderArray;
   PBitOrders:^PBitOrderArray;

{-----------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

	PROCEDURE AddFixed(NName:String; NMin:CPUVar; NCode:Word);
BEGIN
   Inc(z); IF z>FixedOrderCnt THEN Halt(255);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; MinCPU:=NMin; Code:=NCode;
    END;
END;

	PROCEDURE AddReg1(NName:String; NMin:CPUVar; NCode:Word);
BEGIN
   Inc(z); IF z>Reg1OrderCnt THEN Halt(255);
   WITH Reg1Orders^[z] DO
    BEGIN
     Name:=NName; MinCPU:=NMin; Code:=NCode;
    END;
END;

	PROCEDURE AddReg2(NName:String; NMin:CPUVar; NCode:Word);
BEGIN
   Inc(z); IF z>Reg2OrderCnt THEN Halt(255);
   WITH Reg2Orders^[z] DO
    BEGIN
     Name:=NName; MinCPU:=NMin; Code:=NCode;
    END;
END;

	PROCEDURE AddReg3(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>Reg3OrderCnt THEN Halt(255);
   WITH Reg3Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddImm(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>ImmOrderCnt THEN Halt(255);
   WITH ImmOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddRel(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>RelOrderCnt THEN Halt(255);
   WITH RelOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddBit(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>BitOrderCnt THEN Halt(255);
   WITH BitOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddPBit(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>PBitOrderCnt THEN Halt(255);
   WITH PBitOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('IJMP' ,CPU90S2313,$9409); AddFixed('ICALL',CPU90S2313,$9509);
   AddFixed('RET'  ,CPU90S1200,$9508); AddFixed('RETI' ,CPU90S1200,$9518);
   AddFixed('LPM'  ,CPU90S2313,$95c8); AddFixed('SEC'  ,CPU90S1200,$9408);
   AddFixed('CLC'  ,CPU90S1200,$9488); AddFixed('SEN'  ,CPU90S1200,$9428);
   AddFixed('CLN'  ,CPU90S1200,$94a8); AddFixed('SEZ'  ,CPU90S1200,$9418);
   AddFixed('CLZ'  ,CPU90S1200,$9498); AddFixed('SEI'  ,CPU90S1200,$9478);
   AddFixed('CLI'  ,CPU90S1200,$94f8); AddFixed('SES'  ,CPU90S1200,$9448);
   AddFixed('CLS'  ,CPU90S1200,$94c8); AddFixed('SEV'  ,CPU90S1200,$9438);
   AddFixed('CLV'  ,CPU90S1200,$94b8); AddFixed('SET'  ,CPU90S1200,$9468);
   AddFixed('CLT'  ,CPU90S1200,$94e8); AddFixed('SEH'  ,CPU90S1200,$9458);
   AddFixed('CLH'  ,CPU90S1200,$94d8); AddFixed('NOP'  ,CPU90S1200,$0000);
   AddFixed('SLEEP',CPU90S1200,$9588); AddFixed('WDR'  ,CPU90S1200,$95a8);

   New(Reg1Orders); z:=0;
   AddReg1('COM'  ,CPU90S1200,$9400); AddReg1('NEG'  ,CPU90S1200,$9401);
   AddReg1('INC'  ,CPU90S1200,$9403); AddReg1('DEC'  ,CPU90S1200,$940a);
   AddReg1('PUSH' ,CPU90S2313,$920f); AddReg1('POP'  ,CPU90S2313,$900f);
   AddReg1('LSR'  ,CPU90S1200,$9406); AddReg1('ROR'  ,CPU90S1200,$9407);
   AddReg1('ASR'  ,CPU90S1200,$9405); AddReg1('SWAP' ,CPU90S1200,$9402);

   New(Reg2Orders); z:=0;
   AddReg2('ADD'  ,CPU90S1200,$0c00); AddReg2('ADC'  ,CPU90S1200,$1c00);
   AddReg2('SUB'  ,CPU90S1200,$1800); AddReg2('SBC'  ,CPU90S1200,$0800);
   AddReg2('AND'  ,CPU90S1200,$2000); AddReg2('OR'   ,CPU90S1200,$2800);
   AddReg2('EOR'  ,CPU90S1200,$2400); AddReg2('CPSE' ,CPU90S1200,$1000);
   AddReg2('CP'   ,CPU90S1200,$1400); AddReg2('CPC'  ,CPU90S1200,$0400);
   AddReg2('MOV'  ,CPU90S1200,$2c00); AddReg2('MUL'  ,CPU90S8515+1,$9c00);

   New(Reg3Orders); z:=0;
   AddReg3('CLR'  ,$2400); AddReg3('TST'  ,$2000); AddReg3('LSL'  ,$0c00);
   AddReg3('ROL'  ,$1c00);

   New(ImmOrders); z:=0;
   AddImm('SUBI' ,$5000); AddImm('SBCI' ,$4000); AddImm('ANDI' ,$7000);
   AddImm('ORI'  ,$6000); AddImm('SBR'  ,$6000); AddImm('CPI'  ,$3000);
   AddImm('LDI'  ,$e000);

   New(RelOrders); z:=0;
   AddRel('BRCC' ,$f400); AddRel('BRCS' ,$f000); AddRel('BREQ' ,$f001);
   AddRel('BRGE' ,$f404); AddRel('BRSH' ,$f400); AddRel('BRID' ,$f407);
   AddRel('BRIE' ,$f007); AddRel('BRLO' ,$f000); AddRel('BRLT' ,$f004);
   AddRel('BRMI' ,$f002); AddRel('BRNE' ,$f401); AddRel('BRHC' ,$f405);
   AddRel('BRHS' ,$f005); AddRel('BRPL' ,$f402); AddRel('BRTC' ,$f406);
   AddRel('BRTS' ,$f006); AddRel('BRVC' ,$f403); AddRel('BRVS' ,$f003);

   New(BitOrders); z:=0;
   AddBit('BLD'  ,$f800); AddBit('BST'  ,$fa00);
   AddBit('SBRC' ,$fc00); AddBit('SBRS' ,$fe00);

   New(PBitOrders); z:=0;
   AddPBit('CBI' ,$9800); AddPBit('SBI' ,$9a00);
   AddPBit('SBIC',$9900); AddPBit('SBIS',$9b00);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(Reg1Orders);
   Dispose(Reg2Orders);
   Dispose(Reg3Orders);
   Dispose(ImmOrders);
   Dispose(RelOrders);
   Dispose(BitOrders);
   Dispose(PBitOrders);
END;

{-----------------------------------------------------------------------------}

	FUNCTION DecodeReg(VAR Asc:String; VAR Erg:Word):Boolean;
VAR
   io:Integer;
   s:StringPtr;
BEGIN
   IF FindRegDef(Asc,s) THEN Asc:=s^;
   IF (Length(Asc)<2) OR (Length(Asc)>3) OR (UpCase(Asc[1])<>'R') THEN DecodeReg:=False
   ELSE
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,io);
     DecodeReg:=(io=0) AND (Erg<32);
    END;
END;

	FUNCTION DecodeMem(VAR Asc:String; VAR Erg:Word):Boolean;
BEGIN
   DecodeMem:=True;
   IF NLS_StrCaseCmp(Asc,'X')=0 THEN Erg:=$1c
   ELSE IF NLS_StrCaseCmp(Asc,'X+')=0 THEN Erg:=$1d
   ELSE IF NLS_StrCaseCmp(Asc,'-X')=0 THEN Erg:=$1e
   ELSE IF NLS_StrCaseCmp(Asc,'Y')=0 THEN Erg:=$08
   ELSE IF NLS_StrCaseCmp(Asc,'Y+')=0 THEN Erg:=$19
   ELSE IF NLS_StrCaseCmp(Asc,'-Y')=0 THEN Erg:=$1a
   ELSE IF NLS_StrCaseCmp(Asc,'Z')=0 THEN Erg:=$00
   ELSE IF NLS_StrCaseCmp(Asc,'Z+')=0 THEN Erg:=$11
   ELSE IF NLS_StrCaseCmp(Asc,'-Z')=0 THEN Erg:=$12
   ELSE DecodeMem:=False;
END;

{-----------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
VAR
   Size,z,z2:Integer;
   ValOK:Boolean;
   t:TempResult;
BEGIN
   DecodePseudo:=True;

   IF (Memo('PORT')) THEN
    BEGIN
     CodeEquate(SegIO,0,$3f);
     Exit;
    END;

   IF Memo('RES') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Size:=EvalIntExpression(ArgStr[1],Int16,ValOK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (ValOK) AND (NOT FirstPassUnknown) THEN
	BEGIN
	 DontPrint:=True;
	 CodeLen:=Size;
	 IF MakeUseList THEN
	  IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
	END;
      END;
     Exit;
    END;

   IF Memo('DATA') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       ValOK:=True;
       FOR z:=1 TO ArgCnt DO
	IF ValOK THEN
	 BEGIN
	  EvalExpression(ArgStr[z],t);
	  WITH t DO
	  CASE Typ OF
	  TempInt:   IF (Int<-32768) OR (Int>$ffff) THEN
		      BEGIN
		       WrError(1320); ValOK:=False;
		      END
		     ELSE
		      BEGIN
		       WAsmCode[CodeLen]:=Int; Inc(CodeLen);
		      END;
	  TempFloat: BEGIN
		      WrError(1135); ValOK:=False;
		     END;
	  TempString:BEGIN
		      FOR z2:=1 TO Length(Ascii) DO
		       BEGIN
			IF Odd(z2) THEN
			 WAsmCode[CodeLen]:=Ord(CharTransTable[Ascii[z2]])
			ELSE
			 BEGIN
			  Inc(WAsmCode[CodeLen],Word(Ord(CharTransTable[Ascii[z2]])) SHL 8);
			  Inc(CodeLen);
			 END;
		       END;
		      IF Odd(Length(Ascii)) THEN Inc(CodeLen);
		     END;
	  ELSE ValOK:=False;
	  END;
	 END;
       IF NOT ValOK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF Memo('REG') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE AddRegDef(LabPart,ArgStr[1]);
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_AVR;
	Far;
VAR
   z:Integer;
   AdrInt:LongInt;
   Reg1,Reg2:Word;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { kein Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrXError(1500,OpPart)
       ELSE
	BEGIN
	 WAsmCode[0]:=Code; CodeLen:=1;
	END;
       Exit;
      END;

   { nur Register }

   FOR z:=1 TO Reg1OrderCnt DO
    WITH Reg1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
       ELSE
	BEGIN
	 WAsmCode[0]:=Code+(Reg1 SHL 4); CodeLen:=1;
	END;
       Exit;
      END;

   FOR z:=1 TO Reg2OrderCnt DO
    WITH Reg2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrXError(1500,OpPart)
       ELSE IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
       ELSE IF NOT DecodeReg(ArgStr[2],Reg2) THEN WrXError(1445,ArgStr[2])
       ELSE
	BEGIN
	 WAsmCode[0]:=Code+(Reg2 AND 15)+(Reg1 SHL 4)+
		      ((Reg2 AND 16) SHL 5);
	 CodeLen:=1;
	END;
       Exit;
      END;

   FOR z:=1 TO Reg3OrderCnt DO
    WITH Reg3Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
       ELSE
	BEGIN
	 WAsmCode[0]:=Code+(Reg1 AND 15)+(Reg1 SHL 4)+
		      ((Reg1 AND 16) SHL 5);
	 CodeLen:=1;
	END;
       Exit;
      END;

   { immediate mit Register }

   FOR z:=1 TO ImmOrderCnt DO
    WITH ImmOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
       ELSE IF Reg1<16 THEN WrXError(1445,ArgStr[1])
       ELSE
	BEGIN
	 Reg2:=EvalIntExpression(ArgStr[2],Int8,OK);
	 IF OK THEN
	  BEGIN
	   WAsmCode[0]:=Code+((Reg2 AND $f0) SHL 4)+(Reg2 AND $0f)+
			((Reg1 AND $0f) SHL 4);
	   CodeLen:=1;
	  END;
	END;
       Exit;
      END;

   IF (Memo('ADIW')) OR (Memo('SBIW')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU90S2313 THEN WrError(1500)
     ELSE IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
     ELSE IF (Reg1<24) OR (Odd(Reg1)) THEN WrXError(1445,ArgStr[1])
     ELSE
      BEGIN
       Reg2:=EvalIntExpression(ArgStr[2],UInt6,OK);
       IF OK THEN
	BEGIN
	 WAsmCode[0]:=$9600+(Ord(Memo('SBIW')) SHL 8)+((Reg1 AND 6) SHL 3)+
		      (Reg2 AND 15)+((Reg2 AND $30) SHL 2);
	 CodeLen:=1;
	END;
      END;
     Exit;
    END;

   { Transfer }

   IF (Memo('LD')) OR (Memo('ST')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF (Memo('ST')) THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	 z:=$200;
	END
       ELSE z:=0;
       IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
       ELSE IF NOT DecodeMem(ArgStr[2],Reg2) THEN WrError(1350)
       ELSE IF (MomCPU=CPU90S2313) AND (Reg2<>0) THEN WrError(1351)
       ELSE
	BEGIN
	 WAsmCode[0]:=$8000+z+(Reg1 SHL 4)+(Reg2 AND $0f)+((Reg2 AND $10) SHL 8);
	 CodeLen:=1;
	END;
      END;
     Exit;
    END;

   IF (Memo('LDD')) OR (Memo('STD')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU90S2313 THEN WrXError(1500,OpPart)
     ELSE
      BEGIN
       IF (Memo('STD')) THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	 z:=$200;
	END
       ELSE z:=0;
       OK:=True;
       IF UpCase(ArgStr[2][1])='Y' THEN Inc(z,8)
       ELSE IF UpCase(ArgStr[2][1])='Z' THEN
       ELSE OK:=False;
       IF NOT OK THEN WrError(1350)
       ELSE IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
       ELSE
	BEGIN
         ArgStr[2][1]:='0';
	 Reg2:=EvalIntExpression(ArgStr[2],UInt6,OK);
	 IF OK THEN
	  BEGIN
	   WAsmCode[0]:=$8000+z+(Reg1 SHL 4)+(Reg2 AND 7)+((Reg2 AND $18) SHL 7)+((Reg2 AND $20) SHL 8);
	   CodeLen:=1;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('IN')) OR (Memo('OUT')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF (Memo('OUT')) THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	 z:=$800;
	END
       ELSE z:=0;
       IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
       ELSE
	BEGIN
	 Reg2:=EvalIntExpression(ArgStr[2],UInt6,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegIO);
	   WAsmCode[0]:=$b000+z+(Reg1 SHL 4)+(Reg2 AND $0f)+((Reg2 AND $f0) SHL 5);
	   CodeLen:=1;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('LDS')) OR (Memo('STS')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU90S2313 THEN WrError(1500)
     ELSE
      BEGIN
       IF (Memo('STS')) THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	 z:=$200;
	END
       ELSE z:=0;
       IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
       ELSE
	BEGIN
	 WAsmCode[1]:=EvalIntExpression(ArgStr[2],UInt16,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegData);
	   WAsmCode[0]:=$9000+z+(Reg1 SHL 4);
	   CodeLen:=2;
	  END;
	END;
      END;
     Exit;
    END;

   { Bitoperationen }

   IF (Memo('BCLR')) OR (Memo('BSET')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       Reg1:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF OK THEN
	BEGIN
	 WAsmCode[0]:=$9408+(Reg1 SHL 4)+(Ord(Memo('BCLR')) SHL 7);
	 CodeLen:=1;
	END;
      END;
     Exit;
    END;

   FOR z:=1 TO BitOrderCnt DO
    WITH BitOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
       ELSE
	BEGIN
	 Reg2:=EvalIntExpression(ArgStr[2],UInt3,OK);
	 IF OK THEN
	  BEGIN
	   WAsmCode[0]:=Code+(Reg1 SHL 4)+Reg2;
	   CodeLen:=1;
	  END;
	END;
       Exit;
      END;

   IF Memo('CBR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
     ELSE IF Reg1<16 THEN WrXError(1445,ArgStr[1])
     ELSE
      BEGIN
       Reg2:=EvalIntExpression(ArgStr[2],Int8,OK) XOR $ff;
       IF OK THEN
	BEGIN
	 WAsmCode[0]:=$7000+((Reg2 AND $f0) SHL 4)+(Reg2 AND $0f)+
		      ((Reg1 AND $0f) SHL 4);
	 CodeLen:=1;
	END;
      END;
     Exit;
    END;

   IF Memo('SER') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[1],Reg1) THEN WrXError(1445,ArgStr[1])
     ELSE IF Reg1<16 THEN WrXError(1445,ArgStr[1])
     ELSE
      BEGIN
       WAsmCode[0]:=$ef0f+((Reg1 AND $0f) SHL 4);
       CodeLen:=1;
      END;
     Exit;
    END;

   FOR z:=1 TO PBitOrderCnt DO
    WITH PBitOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 Reg1:=EvalIntExpression(ArgStr[1],UInt5,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegIO);
	   Reg2:=EvalIntExpression(ArgStr[2],UInt3,OK);
	   IF OK THEN
	    BEGIN
	     WAsmCode[0]:=Code+Reg2+(Reg1 SHL 3);
	     CodeLen:=1;
	    END;
	  END;
	END;
       Exit;
      END;

   { SprÅnge }

   FOR z:=1 TO RelOrderCnt DO
    WITH RelOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+1);
	 IF OK THEN
	  IF (NOT SymbolQuestionable) AND ((AdrInt<-64) OR (AdrInt>63)) THEN WrError(1370)
	  ELSE
	   BEGIN
	    ChkSpace(SegCode);
	    WAsmCode[0]:=Code+((AdrInt AND $7f) SHL 3);
	    CodeLen:=1;
	   END;
	END;
       Exit;
      END;

   IF (Memo('BRBC')) OR (Memo('BRBS')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       Reg1:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF OK THEN
	BEGIN
	 AdrInt:=EvalIntExpression(ArgStr[2],UInt16,OK)-(EProgCounter+1);
	 IF OK THEN
	  IF (NOT SymbolQuestionable) AND ((AdrInt<-64) OR (AdrInt>63)) THEN WrError(1370)
	  ELSE
	   BEGIN
	    ChkSpace(SegCode);
	    WAsmCode[0]:=$f000+(Ord(Memo('BRBC')) SHL 10)+((AdrInt AND $7f) SHL 3)+Reg1;
	    CodeLen:=1;
	   END;
	END;
      END;
     Exit;
    END;

   IF (Memo('JMP')) OR (Memo('CALL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU90S2313 THEN WrError(1500)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],UInt22,OK);
       IF OK THEN
	BEGIN
	 ChkSpace(SegCode);
	 WAsmCode[0]:=$940c+(Ord(Memo('CALL')) SHL 1)+((AdrInt AND $3e0000) SHR 13)+((AdrInt AND $10000) SHR 16);
	 WAsmCode[1]:=AdrInt AND $ffff;
	 CodeLen:=2;
	END;
      END;
     Exit;
    END;

   IF (Memo('RJMP')) OR (Memo('RCALL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],UInt22,OK)-(EProgCounter+1);
       IF OK THEN
	IF (NOT SymbolQuestionable) AND ((AdrInt<-2048) OR (AdrInt>2047)) THEN WrError(1370)
	ELSE
	 BEGIN
	  ChkSpace(SegCode);
	  WAsmCode[0]:=$c000+(Ord(Memo('RCALL')) SHL 12)+(AdrInt AND $fff);
	  CodeLen:=1;
	 END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_AVR:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  :
    IF MomCPU=CPU90S1200 THEN ok:=ProgCounter<=$03ff
    ELSE IF MomCPU=CPU90S2313 THEN ok:=ProgCounter<=$07ff
    ELSE IF MomCPU=CPU90S4414 THEN ok:=ProgCounter<=$0fff
    ELSE ok:=ProgCounter<=$1fff;
   SegData  :
    IF MomCPU=CPU90S1200 THEN ok:=ProgCounter <=$5f
    ELSE IF MomCPU=CPU90S2313 THEN ok:=ProgCounter<=$df
    ELSE ok:=ProgCounter<$ffff;
   SegIO    :
    ok:=ProgCounter<$3f;
   ELSE ok:=False;
   END;
   ChkPC_AVR:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_AVR:Boolean;
	Far;
BEGIN
   IsDef_AVR:=Memo('PORT') OR Memo('REG');
END;

	PROCEDURE SwitchFrom_AVR;
	Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_AVR;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=True;

   PCSymbol:='*'; HeaderID:=$3b; NOPCode:=$0000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData,SegIO];
   Grans[SegCode]:=2; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=32;
   Grans[SegIO  ]:=1; ListGrans[SegIO  ]:=1; SegInits[SegIO  ]:=0;

   MakeCode:=MakeCode_AVR; ChkPC:=ChkPC_AVR; IsDef:=IsDef_AVR;
   SwitchFrom:=SwitchFrom_AVR; InitFields;
END;

BEGIN
   CPU90S1200:=AddCPU('AT90S1200',SwitchTo_AVR);
   CPU90S2313:=AddCPU('AT90S2313',SwitchTo_AVR);
   CPU90S4414:=AddCPU('AT90S4414',SwitchTo_AVR);
   CPU90S8515:=AddCPU('AT90S8515',SwitchTo_AVR);
END.

