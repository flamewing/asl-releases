{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeCOP8;

{ AS Codegeneratormodul COP8... }

INTERFACE
        Uses NLS,
             Chunks,AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

CONST
   ModNone=-1;
   ModAcc=0;      MModAcc=1 SHL ModAcc;
   ModBInd=1;     MModBInd=1 SHL ModBInd;
   ModBInc=2;     MModBInc=1 SHL ModBInc;
   ModBDec=3;     MModBDec=1 SHL ModBDec;
   ModXInd=4;     MModXInd=1 SHL ModXInd;
   ModXInc=5;     MModXInc=1 SHL ModXInc;
   ModXDec=6;     MModXDec=1 SHL ModXDec;
   ModDir=7;      MModDir=1 SHL ModDir;
   ModImm=8;      MModImm=1 SHL ModImm;

   DirPrefix=$bd;
   BReg=$fe;

   FixedOrderCnt=13;

   AccOrderCnt=9;

   AccMemOrderCnt=7;

   BitOrderCnt=3;

TYPE
   FixedOrder=RECORD
               Name:String[5];
               Code:Byte;
              END;

   FixedOrderArray=ARRAY[0..FixedOrderCnt-1] OF FixedOrder;
   AccOrderArray=ARRAY[0..AccOrderCnt-1] OF FixedOrder;
   AccMemOrderArray=ARRAY[0..AccMemOrderCnt-1] OF FixedOrder;
   BitOrderArray=ARRAY[0..BitOrderCnt-1] OF FixedOrder;

VAR
   CPUCOP87L84:CPUVar;

   FixedOrders:^FixedOrderArray;
   AccOrders:^AccOrderArray;
   AccMemOrders:^AccOrderArray;
   BitOrders:^BitOrderArray;

   AdrMode:ShortInt;
   AdrVal:Byte;
   BigFlag:Boolean;

{-----------------------------------------------------------------------------}

        PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddFixed(NName:String; NCode:Byte);
BEGIN
   IF z>=FixedOrderCnt THEN Halt(255);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddAcc(NName:String; NCode:Byte);
BEGIN
   IF z>=AccOrderCnt THEN Halt(255);
   WITH AccOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddAccMem(NName:String; NCode:Byte);
BEGIN
   IF z>=AccMemOrderCnt THEN Halt(255);
   WITH AccMemOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddBit(NName:String; NCode:Byte);
BEGIN
   IF z>=BitOrderCnt THEN Halt(255);
   WITH BitOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('LAID' ,$a4);  AddFixed('SC'   ,$a1);  AddFixed('RC'   ,$a0);
   AddFixed('IFC'  ,$88);  AddFixed('IFNC' ,$89);  AddFixed('VIS'  ,$b4);
   AddFixed('JID'  ,$a5);  AddFixed('RET'  ,$8e);  AddFixed('RETSK',$8d);
   AddFixed('RETI' ,$8f);  AddFixed('INTR' ,$00);  AddFixed('NOP'  ,$b8);
   AddFixed('RPND' ,$b5);

   New(AccOrders); z:=0;
   AddAcc('CLR'  ,$64);  AddAcc('INC'  ,$8a);  AddAcc('DEC'  ,$8b);
   AddAcc('DCOR' ,$66);  AddAcc('RRC'  ,$b0);  AddAcc('RLC'  ,$a8);
   AddAcc('SWAP' ,$65);  AddAcc('POP'  ,$8c);  AddAcc('PUSH' ,$67);

   New(AccMemOrders); z:=0;
   AddAccMem('ADD'  ,$84);  AddAccMem('ADC'  ,$80);  AddAccMem('SUBC' ,$81);
   AddAccMem('AND'  ,$85);  AddAccMem('OR'   ,$87);  AddAccMem('XOR'  ,$86);
   AddAccMem('IFGT' ,$83);

   New(BitOrders); z:=0;
   AddBit('IFBIT',$70); AddBit('SBIT',$78); AddBit('RBIT',$68);
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(AccOrders);
   Dispose(AccMemOrders);
   Dispose(BitOrders);
END;

{-----------------------------------------------------------------------------}

        PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   Found;
CONST
   ModStrings:ARRAY[ModAcc..ModXDec] OF String[4]=
              ('A','[B]','[B+]','[B-]','[X]','[X+]','[X-]');
VAR
   z:Integer;
   OK:Boolean;
BEGIN
   AdrMode:=ModNone;

   { indirekt/Akku }

   FOR z:=ModAcc TO ModXDec DO
    IF NLS_StrCaseCmp(Asc,ModStrings[z])=0 THEN
     BEGIN
      AdrMode:=z; Goto Found;
     END;

   { immediate }

   IF Asc[1]='#' THEN
    BEGIN
     AdrVal:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),Int8,OK);
     IF OK THEN AdrMode:=ModImm;
     Goto Found;
    END;

   { direkt }

   AdrVal:=EvalIntExpression(Asc,Int8,OK);
   IF OK THEN
    BEGIN
     AdrMode:=ModDir; ChkSpace(SegData);
    END;

Found:
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     AdrMode:=ModNone; WrError(1350);
    END;
END;

{-----------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
VAR
   ValOK:Boolean;
   Size,Value,t,z:Word;
BEGIN
   DecodePseudo:=True;

   IF Memo('SFR') THEN
    BEGIN
     CodeEquate(SegData,0,$ff);
     Exit;
    END;

   IF Memo('ADDR') THEN
    BEGIN
     OpPart:='DB'; BigFlag:=True;
    END;

   IF Memo('ADDRW') THEN
    BEGIN
     OpPart:='DW'; BigFlag:=True;
    END;

   IF Memo('BYTE') THEN OpPart:='DB';

   IF Memo('WORD') THEN OpPart:='DW';

   IF (Memo('DSB')) OR (Memo('DSW')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Size:=EvalIntExpression(ArgStr[1],UInt16,ValOK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (ValOK) AND (NOT FirstPassUnknown) THEN
	BEGIN
	 DontPrint:=True;
         IF Memo('DSW') THEN Inc(Size,Size);
         CodeLen:=Size;
	 IF MakeUseList THEN
	  IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
	END;
      END;
     Exit;
    END;

   IF Memo('FB') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Size:=EvalIntExpression(ArgStr[1],UInt16,ValOK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (ValOK) AND (NOT FirstPassUnknown) THEN
        IF Size>MaxCodeLen THEN WrError(1920)
	ELSE
	 BEGIN
          BAsmCode[0]:=EvalIntExpression(ArgStr[2],Int8,ValOK);
          IF ValOK THEN
           BEGIN
            CodeLen:=Size;
            FillChar(BAsmCode[1],Size-1,BAsmCode[0]);
           END;
	 END;
      END;
     Exit;
    END;

   IF Memo('FW') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Size:=EvalIntExpression(ArgStr[1],UInt16,ValOK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (ValOK) AND (NOT FirstPassUnknown) THEN
        IF Size SHL 1>MaxCodeLen THEN WrError(1920)
	ELSE
	 BEGIN
          Value:=EvalIntExpression(ArgStr[2],Int16,ValOK);
          IF ValOK THEN
           BEGIN
            CodeLen:=Size SHL 1; t:=0;
            FOR z:=0 TO Size-1 DO
             BEGIN
              BAsmCode[t]:=Lo(Value); BAsmCode[t+1]:=Hi(Value);
              Inc(t,2);
             END;
           END;
	 END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_COP8;
	Far;
VAR
   z,AdrInt:Integer;
   HReg:Byte;
   OK:Boolean;
   AdrWord:Word;
BEGIN
   CodeLen:=0; DontPrint:=False; BigFlag:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(BigFlag) THEN Exit;

   { ohne Argument }

   FOR z:=0 TO FixedOrderCnt-1 DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
        BEGIN
         BAsmCode[0]:=Code; CodeLen:=1;
        END;
       Exit;
      END;

   { Datentransfer }

   IF Memo('LD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModDir+MModBInd+MModBInc+MModBDec);
       CASE AdrMode OF
       ModAcc:
        BEGIN
         DecodeAdr(ArgStr[2],MModDir+MModImm+MModBInd+MModXInd+MModBInc+MModXInc+MModBDec+MModXDec);
         CASE AdrMode OF
         ModDir:
          BEGIN
           BAsmCode[0]:=$9d; BAsmCode[1]:=AdrVal; CodeLen:=2;
          END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$98; BAsmCode[1]:=AdrVal; CodeLen:=2;
          END;
         ModBInd:
          BEGIN
           BAsmCode[0]:=$ae; CodeLen:=1;
          END;
         ModXInd:
          BEGIN
           BAsmCode[0]:=$be; CodeLen:=1;
          END;
         ModBInc:
          BEGIN
           BAsmCode[0]:=$aa; CodeLen:=1;
          END;
         ModXInc:
          BEGIN
           BAsmCode[0]:=$ba; CodeLen:=1;
          END;
         ModBDec:
          BEGIN
           BAsmCode[0]:=$ab; CodeLen:=1;
          END;
         ModXDec:
          BEGIN
           BAsmCode[0]:=$bb; CodeLen:=1;
          END;
         END;
        END;
       ModDir:
        BEGIN
         HReg:=AdrVal; DecodeAdr(ArgStr[2],MModImm);
         IF Adrmode=ModImm THEN
          IF HReg=BReg THEN
           IF AdrVal<=15 THEN
            BEGIN
             BAsmCode[0]:=$5f-AdrVal; CodeLen:=1;
            END
           ELSE
            BEGIN
             BAsmCode[0]:=$9f; BAsmCode[1]:=AdrVal; CodeLen:=2;
            END
          ELSE IF HReg>=$f0 THEN
           BEGIN
            BAsmCode[0]:=HReg-$20; BAsmCode[1]:=AdrVal; CodeLen:=2;
           END
          ELSE
           BEGIN
            BAsmCode[0]:=$bc; BAsmCode[1]:=HReg; BAsmCode[2]:=AdrVal; CodeLen:=3;
           END;
        END;
       ModBInd:
        BEGIN
         DecodeAdr(ArgStr[2],MModImm);
         IF AdrMode<>ModNone THEN
          BEGIN
           BAsmCode[0]:=$9e; BAsmCode[1]:=AdrVal; CodeLen:=2;
          END;
        END;
       ModBInc:
        BEGIN
         DecodeAdr(ArgStr[2],MModImm);
         IF AdrMode<>ModNone THEN
          BEGIN
           BAsmCode[0]:=$9a; BAsmCode[1]:=AdrVal; CodeLen:=2;
          END;
        END;
       ModBDec:
        BEGIN
         DecodeAdr(ArgStr[2],MModImm);
         IF AdrMode<>ModNone THEN
          BEGIN
           BAsmCode[0]:=$9b; BAsmCode[1]:=AdrVal; CodeLen:=2;
          END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('X') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN
        BEGIN
         ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
        END;
       DecodeAdr(ArgStr[1],MModAcc);
       IF AdrMode<>ModNone THEN
        BEGIN
         DecodeAdr(ArgStr[2],MModDir+MModBInd+MModXInd+MModBInc+MModXInc+MModBDec+MModXDec);
         CASE AdrMode OF
         ModDir:
          BEGIN
           BAsmCode[0]:=$9c; BAsmCode[1]:=AdrVal; CodeLen:=2;
          END;
         ModBInd:
          BEGIN
           BAsmCode[0]:=$a6; CodeLen:=1;
          END;
         ModBInc:
          BEGIN
           BAsmCode[0]:=$a2; CodeLen:=1;
          END;
         ModBDec:
          BEGIN
           BAsmCode[0]:=$a3; CodeLen:=1;
          END;
         ModXInd:
          BEGIN
           BAsmCode[0]:=$b6; CodeLen:=1;
          END;
         ModXInc:
          BEGIN
           BAsmCode[0]:=$b2; CodeLen:=1;
          END;
         ModXDec:
          BEGIN
           BAsmCode[0]:=$b3; CodeLen:=1;
          END;
         END;
        END;
      END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=0 TO AccOrderCnt-1 DO
    WITH AccOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModAcc);
         IF AdrMode<>ModNone THEN
          BEGIN
           BAsmCode[0]:=Code; CodeLen:=1;
          END;
        END;
       Exit;
      END;

   FOR z:=0 TO AccMemOrderCnt-1 DO
    WITH AccMemOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModAcc);
         IF AdrMode<>ModNone THEN
          BEGIN
           DecodeAdr(ArgStr[2],MModDir+MModImm+MModBInd);
           CASE AdrMode OF
           ModBInd:
            BEGIN
             BAsmCode[0]:=Code; CodeLen:=1;
            END;
           ModImm:
            BEGIN
             BAsmCode[0]:=Code+$10; BAsmCode[1]:=AdrVal;
             CodeLen:=2;
            END;
           ModDir:
            BEGIN
             BAsmCode[0]:=DirPrefix; BAsmCode[1]:=AdrVal; BAsmCode[2]:=Code;
             CodeLen:=3;
            END;
           END;
          END;
        END;
       Exit;
      END;

   IF Memo('ANDSZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc);
       IF AdrMode<>ModNone THEN
        BEGIN
         DecodeAdr(ArgStr[2],MModImm);
         IF AdrMode=ModImm THEN
          BEGIN
           BAsmCode[0]:=$60; BAsmCode[1]:=AdrVal; CodeLen:=2;
          END;
        END;
      END;
     Exit;
    END;

   { Bedingungen }

   IF Memo('IFEQ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModDir);
       CASE AdrMode OF
       ModAcc:
        BEGIN
         DecodeAdr(ArgStr[2],MModDir+MModBInd+MModImm);
         CASE AdrMode OF
         ModDir:
          BEGIN
           BAsmCode[0]:=DirPrefix; BAsmCode[1]:=AdrVal; BAsmCode[2]:=$82; CodeLen:=3;
          END;
         ModBInd:
          BEGIN
           BAsmCode[0]:=$82; CodeLen:=1;
          END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$92; BAsmCode[1]:=AdrVal; CodeLen:=2;
          END;
         END;
        END;
       ModDir:
        BEGIN
         BAsmCode[1]:=AdrVal;
         DecodeAdr(ArgStr[2],MModImm);
         IF AdrMode=ModImm THEN
          BEGIN
           BAsmCode[0]:=$a9; BAsmCode[2]:=AdrVal; CodeLen:=3;
          END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('IFNE') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc);
       CASE AdrMode OF
       ModAcc:
        BEGIN
         DecodeAdr(ArgStr[2],MModDir+MModBInd+MModImm);
         CASE AdrMode OF
         ModDir:
          BEGIN
           BAsmCode[0]:=DirPrefix; BAsmCode[1]:=AdrVal; BAsmCode[2]:=$b9; CodeLen:=3;
          END;
         ModBInd:
          BEGIN
           BAsmCode[0]:=$b9; CodeLen:=1;
          END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$99; BAsmCode[1]:=AdrVal; CodeLen:=2;
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('IFBNE') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[0]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt4,OK);
       IF OK THEN
        BEGIN
         Inc(BAsmCode[0],$40); CodeLen:=1;
        END;
      END;
     Exit;
    END;

   { Bitbefehle }

   FOR z:=0 TO BitOrderCnt DO
    WITH BitOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         HReg:=EvalIntExpression(ArgStr[1],UInt3,OK);
         IF OK THEN
          BEGIN
           DecodeAdr(ArgStr[2],MModDir+MModBInd);
           CASE AdrMode OF
           ModDir:
            BEGIN
             BAsmCode[0]:=DirPrefix; BAsmCode[1]:=AdrVal; BAsmCode[2]:=Code+HReg; CodeLen:=3;
            END;
           ModBInd:
            BEGIN
             BAsmCode[0]:=Code+HReg; CodeLen:=1;
            END;
           END;
          END;
        END;
       Exit;
      END;

   { SprÅnge }

   IF (Memo('JMP')) OR (Memo('JSR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],UInt16,OK);
       IF OK THEN
        IF ((EProgCounter+2) SHR 12)<>(AdrWord SHR 12) THEN WrError(1910)
        ELSE
         BEGIN
          ChkSpace(SegCode);
          BAsmCode[0]:=$20+(Ord(Memo('JSR')) SHL 4)+((AdrWord SHR 8) AND 15);
          BAsmCode[1]:=Lo(AdrWord);
          CodeLen:=2;
         END;
      END;
     Exit;
    END;

   IF (Memo('JMPL')) OR (Memo('JSRL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(ArgStr[1],UInt16,OK);
       IF OK THEN
        BEGIN
         ChkSpace(SegCode);
         BAsmCode[0]:=$ac+Ord(Memo('JSRL'));
         BAsmCode[1]:=Hi(AdrWord);
         BAsmCode[2]:=Lo(AdrWord);
         CodeLen:=3;
        END;
      END;
     Exit;
    END;

   IF (Memo('JP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+1);
       IF OK THEN
        IF AdrInt=0 THEN
         BEGIN
          BAsmCode[0]:=NOPCode; CodeLen:=1; WrError(60);
         END
        ELSE IF ((AdrInt>31) OR (AdrInt<-32)) AND (NOT SymbolQuestionable) THEN WrError(1370)
        ELSE
         BEGIN
          BAsmCode[0]:=AdrInt AND $ff; CodeLen:=1;
         END;
      END;
     Exit;
    END;

   IF Memo('DRSZ') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       DecodeAdr(ArgStr[1],MModDir);
       IF FirstPassUnknown THEN AdrVal:=AdrVal OR $f0;
       IF AdrVal<$f0 THEN WrError(1315)
       ELSE
        BEGIN
         BAsmCode[0]:=AdrVal-$30; CodeLen:=1;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_COP8:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<$2000;
   SegData  : ok:=ProgCounter< $100;
   ELSE ok:=False;
   END;
   ChkPC_COP8:=(ok) AND (ProgCounter>=0);
END;


        FUNCTION IsDef_COP8:Boolean;
	Far;
BEGIN
   IsDef_COP8:=Memo('SFR');
END;

        PROCEDURE SwitchFrom_COP8;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_COP8;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeC; SetIsOccupied:=False;

   PCSymbol:='.'; HeaderID:=$6f; NOPCode:=$b8;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=0;

   MakeCode:=MakeCode_COP8; ChkPC:=ChkPC_COP8; IsDef:=IsDef_COP8;
   SwitchFrom:=SwitchFrom_COP8; InitFields;
END;

BEGIN
   CPUCOP87L84:=AddCPU('COP87L84',SwitchTo_COP8);
END.


