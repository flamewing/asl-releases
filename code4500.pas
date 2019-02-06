{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

{ AS - Codegeneratormodul MELPS4500 }

        UNIT Code4500;

INTERFACE

        Uses Chunks,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[5];
	       Code:Word;
	      END;

   ConstOrder=RECORD
	       Name:String[4];
	       Code:Word;
               Max:IntType;
	      END;

CONST
   FixedOrderCount=79;
   ConstOrderCount=12;

TYPE
   FixedOrderArray=ARRAY[1..FixedOrderCount] OF FixedOrder;
   ConstOrderArray=ARRAY[1..ConstOrderCount] OF ConstOrder;

VAR
   CPU4500:CPUVar;

   FixedOrders:^FixedOrderArray;
   ConstOrders:^ConstOrderArray;

	PROCEDURE InitFields;
VAR
   z:Integer;

	PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   IF z>FixedOrderCount THEN Halt;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE AddConst(NName:String; NCode:Word; NMax:IntType);
BEGIN
   IF z>ConstOrderCount THEN Halt;
   WITH ConstOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Max:=NMax;
    END;
   Inc(z);
END;

BEGIN
   New(FixedOrders); z:=1;
   AddFixed('AM'  ,$00a);  AddFixed('AMC' ,$00b);  AddFixed('AND' ,$018);
   AddFixed('CLD' ,$011);  AddFixed('CMA' ,$01c);  AddFixed('DEY' ,$017);
   AddFixed('DI'  ,$004);  AddFixed('EI'  ,$005);  AddFixed('IAP0',$260);
   AddFixed('IAP1',$261);  AddFixed('IAP2',$262);  AddFixed('IAP3',$263);
   AddFixed('IAP4',$264);  AddFixed('INY' ,$013);  AddFixed('NOP' ,$000);
   AddFixed('OR'  ,$019);  AddFixed('OP0A',$220);  AddFixed('OP1A',$221);
   AddFixed('POF' ,$002);  AddFixed('POF2',$008);  AddFixed('RAR' ,$01d);
   AddFixed('RC'  ,$006);  AddFixed('RC3' ,$2ac);  AddFixed('RC4' ,$2ae);
   AddFixed('RD'  ,$014);  AddFixed('RT'  ,$044);  AddFixed('RTI' ,$046);
   AddFixed('RTS' ,$045);  AddFixed('SC'  ,$007);  AddFixed('SC3' ,$2ad);
   AddFixed('SC4' ,$2af);  AddFixed('SD'  ,$015);  AddFixed('SEAM',$026);
   AddFixed('SNZ0',$038);  AddFixed('SNZP',$003);  AddFixed('SNZT1',$280);
   AddFixed('SNZT2',$281); AddFixed('SNZT3',$282); AddFixed('SPCR',$299);
   AddFixed('STCR',$298);  AddFixed('SZC' ,$02f);  AddFixed('T1R1',$2ab);
   AddFixed('T3AB',$232);  AddFixed('TAB' ,$01e);  AddFixed('TAB3',$272);
   AddFixed('TABE',$02a);  AddFixed('TAD' ,$051);  AddFixed('TAI1',$253);
   AddFixed('TAL1',$24a);  AddFixed('TAMR',$252);  AddFixed('TASP',$050);
   AddFixed('TAV1',$054);  AddFixed('TAW1',$24b);  AddFixed('TAW2',$24c);
   AddFixed('TAW3',$24d);  AddFixed('TAX' ,$052);  AddFixed('TAY' ,$01f);
   AddFixed('TAZ' ,$053);  AddFixed('TBA' ,$00e);  AddFixed('TC1A',$2a8);
   AddFixed('TC2A',$2a9);  AddFixed('TDA' ,$029);  AddFixed('TEAB',$01a);
   AddFixed('TI1A',$217);  AddFixed('TL1A',$20a);  AddFixed('TL2A',$20b);
   AddFixed('TL3A',$20c);  AddFixed('TLCA',$20d);  AddFixed('TMRA',$216);
   AddFixed('TPTA',$2a5);  AddFixed('TPAA',$2aa);  AddFixed('TR1A',$2a6);
   AddFixed('TR1AB',$23f); AddFixed('TV1A',$03f);  AddFixed('TW1A',$20e);
   AddFixed('TW2A',$20f);  AddFixed('TW3A',$210);  AddFixed('TYA' ,$00c);
   AddFixed('WRST',$2a0);

   New(ConstOrders); z:=1;
   AddConst('A'   ,$060,UInt4);  AddConst('LA'  ,$070,UInt4);
   AddConst('LZ'  ,$048,UInt2);  AddConst('RB'  ,$04c,UInt2);
   AddConst('SB'  ,$05c,UInt2);  AddConst('SZB' ,$020,UInt2);
   AddConst('TABP',$080,UInt6);  AddConst('TAM' ,$2c0,UInt4);
   AddConst('TMA' ,$2b0,UInt4);  AddConst('XAM' ,$2d0,UInt4);
   AddConst('XAMD',$2f0,UInt4);  AddConst('XAMI',$2e0,UInt4);
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(ConstOrders);
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   ValOK:Boolean;
   Size,z,z2:Word;
   t:TempResult;
   Ch:Char;
BEGIN
   DecodePseudo:=True;

   IF Memo('SFR') THEN
    BEGIN
     CodeEquate(SegData,0,415);
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
          FirstPassUnknown:=False;
	  EvalExpression(ArgStr[z],t);
          IF (t.Typ=TempInt) AND (FirstPassUnknown) THEN
           IF ActPC=SegData THEN t.Int:=t.Int AND 7 ELSE t.Int:=t.Int AND 511;
	  WITH t DO
	  CASE Typ OF
          TempInt:
           IF ActPC=SegCode THEN
            BEGIN
             IF NOT RangeCheck(Int,Int10) THEN
              BEGIN
               WrError(1320); ValOK:=False;
              END
             ELSE
              BEGIN
               WAsmCode[CodeLen]:=Int AND $3ff; Inc(CodeLen);
              END;
            END
           ELSE
            BEGIN
             IF NOT RangeCheck(Int,Int4) THEN
              BEGIN
               WrError(1320); ValOK:=False;
              END
             ELSE
              BEGIN
               BAsmCode[CodeLen]:=Int AND $0f; Inc(CodeLen);
              END;
            END;
          TempFloat:
           BEGIN
            WrError(1135); ValOK:=False;
           END;
          TempString:
           BEGIN
            FOR z2:=1 TO Length(Ascii) DO
             BEGIN
              Ch:=CharTransTable[Ascii[z2]];
              IF ActPC=SegCode THEN
               BEGIN
                WAsmCode[CodeLen]:=Ord(Ch); Inc(CodeLen);
               END
              ELSE
               BEGIN
                BAsmCode[CodeLen]:=Ord(Ch) SHR 4; Inc(CodeLen);
                BAsmCode[CodeLen]:=Ord(Ch) AND 15; Inc(CodeLen);
               END;
             END;
           END;
	  ELSE ValOK:=False;
	  END;
	 END;
       IF NOT ValOK THEN CodeLen:=0;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_4500;
	Far;
VAR
   z:Integer;
   AdrWord:Word;
   OK:Boolean;

BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
        BEGIN
         CodeLen:=1; WAsmCode[0]:=Code;
	END;
       Exit;
      END;

   IF Memo('SZD') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$024; WAsmCode[1]:=$02b;
      END;
     Exit;
    END;

   FOR z:=1 TO ConstOrderCount DO
    WITH ConstOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         WAsmCode[0]:=EvalIntExpression(ArgStr[1],Max,OK);
         IF OK THEN
          BEGIN
	   CodeLen:=1; Inc(WAsmCode[0],Code);
          END;
	END;
       Exit;
      END;

   IF Memo('SEA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[1]:=EvalIntExpression(ArgStr[1],UInt4,OK);
       IF OK THEN
        BEGIN
         CodeLen:=2; Inc(WAsmCode[1],$070); WAsmCode[0]:=$025;
        END;
      END;
     Exit;
    END;

   IF Memo('B') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],UInt13,OK);
       IF OK THEN
        IF EProgCounter SHR 7<>AdrWord SHR 7 THEN WrError(1910)
        ELSE
         BEGIN
          CodeLen:=1; WAsmCode[0]:=$180+(AdrWord AND $7f);
         END;
      END;
     Exit;
    END;

   IF (Memo('BL')) OR (Memo('BML')) THEN
    BEGIN
     IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN AdrWord:=EvalIntExpression(ArgStr[1],UInt13,OK)
       ELSE
        BEGIN
         AdrWord:=EvalIntExpression(ArgStr[1],UInt6,OK) SHL 7;
         IF OK THEN Inc(AdrWord,EvalIntExpression(ArgStr[2],UInt7,OK));
        END;
       IF OK THEN
        BEGIN
         CodeLen:=2;
         WAsmCode[1]:=$200+(AdrWord AND $7f)+((AdrWord SHR 12) SHL 7);
	 WAsmCode[0]:=$0c0+(Ord(Memo('BL')) SHL 5)+((AdrWord SHR 7) AND $1f);
        END;
      END;
     Exit;
    END;

   IF (Memo('BLA')) OR (Memo('BMLA')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],UInt6,OK);
       IF OK THEN
        BEGIN
         CodeLen:=2;
	 WAsmCode[1]:=$200+(AdrWord AND $0f)+((AdrWord AND $30) SHL 2);
	 WAsmCode[0]:=$010+(Ord(Memo('BMLA')) SHL 5);
        END;
      END;
     Exit;
    END;

   IF Memo('BM') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],UInt13,OK);
       IF OK THEN
	IF AdrWord SHR 7<>2 THEN WrError(1905)
        ELSE
         BEGIN
         CodeLen:=1;
	 WAsmCode[0]:=$100+(AdrWord AND $7f);
        END;
      END;
     Exit;
    END;

   IF Memo('LXY') THEN
    BEGIN
     IF (ArgCnt=0) OR (ArgCnt>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN AdrWord:=EvalIntExpression(ArgStr[1],Int8,OK)
       ELSE
        BEGIN
         AdrWord:=EvalIntExpression(ArgStr[1],Int4,OK) SHL 4;
         IF OK THEN Inc(AdrWord,EvalIntExpression(ArgStr[2],Int4,OK));
	END;
       IF OK THEN
        BEGIN
         CodeLen:=1;
         WAsmCode[0]:=$300+AdrWord;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_4500:Boolean;
        Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode : ok:=ProgCounter < $2000;
   SegData : ok:=ProgCounter < 416;
   ELSE ok:=False;
   END;
   ChkPC_4500:=(ok) AND (ProgCounter>=0);
END;

        FUNCTION IsDef_4500:Boolean;
	Far;
BEGIN
   IsDef_4500:=Memo('SFR');
END;

        PROCEDURE InternSymbol_4500(VAR Asc:String; VAR Erg:TempResult);
        Far;
BEGIN
   Erg.Typ:=TempNone;
END;

        PROCEDURE SwitchFrom_4500;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_4500;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$12; NOPCode:=$000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode ]:=2; ListGrans[SegCode ]:=2; SegInits[SegCode ]:=0;
   Grans[SegData ]:=1; ListGrans[SegData ]:=1; SegInits[SegCode ]:=0;

   MakeCode:=MakeCode_4500; ChkPC:=ChkPC_4500; IsDef:=IsDef_4500;
   SwitchFrom:=SwitchFrom_4500; InternSymbol:=InternSymbol_4500;

   InitFields;
END;

BEGIN
   CPU4500:=AddCPU('MELPS4500' ,SwitchTo_4500);
END.
