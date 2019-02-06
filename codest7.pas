{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeST7;

{ AS Codegeneratormodul ST7 }

INTERFACE
        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
               Name:String[5];
	       Code:Word;
	      END;

   AriOrder=RECORD
             Name:String[5];
	     Code:Word;
             MayImm:Boolean;
	    END;

CONST
   FixedOrderCnt=11;
   AriOrderCnt=8;
   RMWOrderCnt=13;
   RelOrderCnt=20;

   ModNone=-1;
   ModImm=0;       MModImm=1 SHL ModImm;
   ModAbs8=1;      MModAbs8=1 SHL ModAbs8;
   ModAbs16=2;     MModAbs16=1 SHL ModAbs16;
   ModIX=3;        MModIX=1 SHL ModIX;
   ModIX8=4;       MModIX8=1 SHL ModIX8;
   ModIX16=5;      MModIX16=1 SHL ModIX16;
   ModIY=6;        MModIY=1 SHL ModIY;
   ModIY8=7;       MModIY8=1 SHL ModIY8;
   ModIY16=8;      MModIY16=1 SHL ModIY16;
   ModIAbs8=9;     MModIAbs8=1 SHL ModIAbs8;
   ModIAbs16=10;   MModIAbs16=1 SHL ModIAbs16;
   ModIXAbs8=11;   MModIXAbs8=1 SHL ModIXAbs8;
   ModIXAbs16=12;  MModIXAbs16=1 SHL ModIXAbs16;
   ModIYAbs8=13;   MModIYAbs8=1 SHL ModIYAbs8;
   ModIYAbs16=14;  MModIYAbs16=1 SHL ModIYAbs16;
   ModA=15;        MModA=1 SHL ModA;
   ModX=16;        MModX=1 SHL ModX;
   ModY=17;        MModY=1 SHL ModY;
   ModS=18;        MModS=1 SHL ModS;
   ModCCR=19;      MModCCR=1 SHL ModCCR;

TYPE
   FixedOrderArray=ARRAY[0..FixedOrderCnt-1] OF FixedOrder;
   AriOrderArray=ARRAY[0..AriOrderCnt-1] OF AriOrder;
   RMWOrderArray=ARRAY[0..RMWOrderCnt-1] OF FixedOrder;
   RelOrderArray=ARRAY[0..RelOrderCnt-1] OF FixedOrder;

VAR
   CPUST7:CPUVar;

   FixedOrders:^FixedOrderArray;
   AriOrders:^AriOrderArray;
   RMWOrders:^RMWOrderArray;
   RelOrders:^RelOrderArray;

   AdrType,AbsSeg:ShortInt;
   AdrPart,AdrCnt,OpSize,PrefixCnt:Byte;
   AdrVals:ARRAY[0..2] OF Byte;

{----------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

   	PROCEDURE AddFixed(NName:String; NCode:Byte);
BEGIN
   IF z>=FixedOrderCnt THEN Halt(255);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

   	PROCEDURE AddAri(NName:String; NCode:Byte; NMay:Boolean);
BEGIN
   IF z>=AriOrderCnt THEN Halt(255);
   WITH AriOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MayImm:=NMay;
    END;
   Inc(z);
END;

   	PROCEDURE AddRMW(NName:String; NCode:Byte);
BEGIN
   IF z>=RMWOrderCnt THEN Halt(255);
   WITH RMWOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddRel(NName:String; NCode:Byte);
BEGIN
   IF z>=RelOrderCnt THEN Halt(255);
   WITH RelOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   z:=0; New(FixedOrders);
   AddFixed('HALT' ,$8e); AddFixed('IRET' ,$80); AddFixed('NOP'  ,$9d);
   AddFixed('RCF'  ,$98); AddFixed('RET'  ,$81); AddFixed('RIM'  ,$9a);
   AddFixed('RSP'  ,$9c); AddFixed('SCF'  ,$99); AddFixed('SIM'  ,$9b);
   AddFixed('TRAP' ,$83); AddFixed('WFI'  ,$8f);

   z:=0; New(AriOrders);
   AddAri('ADC' ,$09,True ); AddAri('ADD' ,$0b,True ); AddAri('AND' ,$04,True );
   AddAri('BCP' ,$05,True ); AddAri('OR'  ,$0a,True ); AddAri('SBC' ,$02,True );
   AddAri('SUB' ,$00,True ); AddAri('XOR' ,$08,True );

   z:=0; New(RMWOrders);
   AddRMW('CLR' ,$0f); AddRMW('CPL' ,$03); AddRMW('DEC' ,$0a);
   AddRMW('INC' ,$0c); AddRMW('NEG' ,$00); AddRMW('RLC' ,$09);
   AddRMW('RRC' ,$06); AddRMW('SLA' ,$08); AddRMW('SLL' ,$08);
   AddRMW('SRA' ,$07); AddRMW('SRL' ,$04); AddRMW('SWAP',$0e);
   AddRMW('TNZ' ,$0d);

   z:=0; New(RelOrders);
   AddRel('CALLR',$ad); AddRel('JRA'  ,$20); AddRel('JRC'  ,$25);
   AddRel('JREQ' ,$27); AddRel('JRF'  ,$21); AddRel('JRH'  ,$29);
   AddRel('JRIH' ,$2f); AddRel('JRIL' ,$2e); AddRel('JRM'  ,$2d);
   AddRel('JRMI' ,$2b); AddRel('JRNC' ,$24); AddRel('JRNE' ,$26);
   AddRel('JRNH' ,$28); AddRel('JRNM' ,$2c); AddRel('JRPL' ,$2a);
   AddRel('JRT'  ,$20); AddRel('JRUGE',$24); AddRel('JRUGT',$22);
   AddRel('JRULE',$23); AddRel('JRULT',$25);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(AriOrders);
   Dispose(RMWOrders);
   Dispose(RelOrders);
END;

{----------------------------------------------------------------------------}

	PROCEDURE AddPrefix(Pref:Byte);
BEGIN
   BAsmCode[PrefixCnt]:=Pref; Inc(PrefixCnt);
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:LongInt);
LABEL
   Found;
VAR
   OK,YReg:Boolean;
   Asc2:String;
   p:Integer;

        PROCEDURE DecideSize(Asc:String; Type1,Type2:LongInt; Part1,Part2:Byte);
VAR
   Size:(None,I8,I16);
   Value:Word;
   OK:Boolean;
BEGIN
   IF (Length(Asc)>=3) AND (Asc[Length(Asc)-1]='.') THEN
    BEGIN
     IF UpCase(Asc[Length(Asc)])='B' THEN
      BEGIN
       Size:=I8; Delete(Asc,Length(Asc)-1,2);
      END
     ELSE IF UpCase(Asc[Length(Asc)])='W' THEN
      BEGIN
       Size:=I16; Delete(Asc,Length(Asc)-1,2);
      END
     ELSE Size:=None;
    END
   ELSE Size:=None;

   IF Size=I8 THEN Value:=EvalIntExpression(Asc,UInt8,OK)
   ELSE Value:=EvalIntExpression(Asc,Int16,OK);

   IF OK THEN
    IF (Size=I8) OR ((Mask AND (1 SHL Type1)<>0) AND (Size=None) AND (Hi(Value)=0)) THEN
     BEGIN
      AdrVals[0]:=Lo(Value); AdrCnt:=1;
      AdrPart:=Part1; AdrType:=Type1;
     END
    ELSE
     BEGIN
      AdrVals[0]:=Hi(Value); AdrVals[1]:=Lo(Value); AdrCnt:=2;
      AdrPart:=Part2; AdrType:=Type2;
     END;
END;

        PROCEDURE DecideASize(Asc:String; Type1,Type2:LongInt; Part1,Part2:Byte);
VAR
   I16:Boolean;
   OK:Boolean;
BEGIN
   IF (Length(Asc)>=3) AND (Asc[Length(Asc)-1]='.') AND (UpCase(Asc[Length(Asc)])='W') THEN
    BEGIN
     I16:=True; Delete(Asc,Length(Asc)-1,2);
    END
   ELSE IF (Mask AND (1 SHL Type1))=0 THEN I16:=True
   ELSE I16:=False;

   AdrVals[0]:=EvalIntExpression(Asc,UInt8,OK);
   IF OK THEN
    BEGIN
     AdrCnt:=1;
     IF (I16) THEN
      BEGIN
       AdrPart:=Part2; AdrType:=Type2;
      END
     ELSE
      BEGIN
       AdrPart:=Part1; AdrType:=Type1;
      END;
    END;
END;

BEGIN
   AdrType:=ModNone; AdrCnt:=0;

   { Register ? }

   IF NLS_StrCaseCmp(Asc,'A')=0 THEN
    BEGIN
     AdrType:=ModA; Goto Found;
    END;

   IF NLS_StrCaseCmp(Asc,'X')=0 THEN
    BEGIN
     AdrType:=ModX; Goto Found;
    END;

   IF NLS_StrCaseCmp(Asc,'Y')=0 THEN
    BEGIN
     AdrType:=ModY; AddPrefix($90); Goto Found;
    END;

   IF NLS_StrCaseCmp(Asc,'S')=0 THEN
    BEGIN
     AdrType:=ModS; Goto Found;
    END;

   IF NLS_StrCaseCmp(Asc,'CC')=0 THEN
    BEGIN
     AdrType:=ModCCR; Goto Found;
    END;

   { immediate ? }

   IF Asc[1]='#' THEN
    BEGIN
     AdrVals[0]:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),Int8,OK);
     IF OK THEN
      BEGIN
       AdrType:=ModImm; AdrPart:=$a; AdrCnt:=1;
      END;
     Goto Found;
    END;

   { speicherindirekt ? }

   IF (Asc[1]='[') AND (Asc[Length(Asc)]=']') THEN
    BEGIN
     Asc:=Copy(Asc,2,Length(Asc)-2);
     DecideASize(Asc,ModIAbs8,ModIAbs16,$b,$c);
     IF AdrType<>ModNone THEN AddPrefix($92);
     Goto Found;
    END;

   { sonstwie indirekt ? }

   IF IsIndirect(Asc) THEN
    BEGIN
     Asc:=Copy(Asc,2,Length(Asc)-2);

     { ein oder zwei Argumente ? }

     p:=QuotPos(Asc,',');
     IF p>Length(Asc) THEN
      BEGIN
       AdrPart:=$f;
       IF NLS_StrCaseCmp(Asc,'X')=0 THEN AdrType:=ModIX
       ELSE IF NLS_StrCaseCmp(Asc,'Y')=0 THEN
        BEGIN
	 AdrType:=ModIY; AddPrefix($90);
        END
       ELSE WrXError(1445,Asc);
       Goto Found;
      END;

     Asc2:=Copy(Asc,p+1,Length(Asc)-p); Delete(Asc,p,Length(Asc)-p+1);

     IF NLS_StrCaseCmp(Asc,'X')=0 THEN
      BEGIN
       Asc:=Asc2; YReg:=False;
      END
     ELSE IF NLS_StrCaseCmp(Asc2,'X')=0 THEN YReg:=False
     ELSE IF NLS_StrCaseCmp(Asc,'Y')=0 THEN
      BEGIN
       Asc:=Asc2; YReg:=True;
      END
     ELSE IF NLS_StrCaseCmp(Asc2,'Y')=0 THEN YReg:=True
     ELSE
      BEGIN
       WrError(1350); Exit;
      END;

     { speicherindirekt ? }

     IF (Asc[1]='[') AND (Asc[Length(Asc)]=']') THEN
      BEGIN
       Asc:=Copy(Asc,2,Length(Asc)-2);
       IF YReg THEN
        BEGIN
         DecideASize(Asc,ModIYAbs8,ModIYAbs16,$e,$d);
         IF AdrType<>ModNone THEN AddPrefix($91);
        END
       ELSE
        BEGIN
         DecideASize(Asc,ModIXAbs8,ModIXAbs16,$e,$d);
         IF AdrType<>ModNone THEN AddPrefix($92);
        END;
      END
     ELSE
      BEGIN
       IF YReg THEN DecideSize(Asc,ModIY8,ModIY16,$e,$d)
       ELSE DecideSize(Asc,ModIX8,ModIX16,$e,$d);
       IF (AdrType<>ModNone) AND (YReg) THEN AddPrefix($90);
      END;

     Goto Found;
    END;

   { dann absolut }

   DecideSize(Asc,ModAbs8,ModAbs16,$b,$c);

Found:
   IF (AdrType<>ModNone) AND (Mask AND (LongInt(1) SHL AdrType)=0) THEN
    BEGIN
     WrError(1350); AdrType:=ModNone; AdrCnt:=0;
    END;
END;

{----------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_ST7;
	Far;
VAR
   z,AdrInt:Integer;
   Mask:LongInt;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=1; PrefixCnt:=0;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Attribut verarbeiten }

   IF AttrPart<>'' THEN
    CASE UpCase(AttrPart[1]) OF
    'B':OpSize:=0;
    'W':OpSize:=1;
    'L':OpSize:=2;
    'Q':OpSize:=3;
    'S':OpSize:=4;
    'D':OpSize:=5;
    'X':OpSize:=6;
    'P':OpSize:=7;
    ELSE
     BEGIN
      WrError(1107); Exit;
     END;
    END;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeMotoPseudo(True) THEN Exit;
   IF DecodeMoto16Pseudo(OpSize,True) THEN Exit;

   { ohne Argument}

   FOR z:=0 TO FixedOrderCnt-1 DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         BAsmCode[PrefixCnt]:=Code; CodeLen:=PrefixCnt+1;
        END;
       Exit;
      END;

   { Datentransfer }

   IF Memo('LD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModA+MModX+MModY+MModS+
                 MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
                 MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
                 MModIYAbs8+MModIYAbs16);
       CASE AdrType OF
       ModA:
        BEGIN
         DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
                   MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
                   MModIYAbs8+MModIYAbs16+MModX+MModY+MModS);
         CASE AdrType OF
         ModX,ModY:
          BEGIN
           BAsmCode[PrefixCnt]:=$9f; CodeLen:=PrefixCnt+1;
          END;
         ModS:
          BEGIN
           BAsmCode[PrefixCnt]:=$9e; CodeLen:=PrefixCnt+1;
          END;
         ELSE IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[PrefixCnt]:=$06+(AdrPart SHL 4);
           Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
           CodeLen:=PrefixCnt+1+AdrCnt;
          END;
         END;
        END;
       ModX:
        BEGIN
         DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+
                   MModIX16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
                   MModA+MModY+MModS);
         CASE AdrType OF
         ModA:
          BEGIN
           BAsmCode[PrefixCnt]:=$97; CodeLen:=PrefixCnt+1;
          END;
         ModY:
          BEGIN
           BAsmCode[0]:=$93; CodeLen:=1;
          END;
         ModS:
          BEGIN
           BAsmCode[PrefixCnt]:=$96; CodeLen:=PrefixCnt+1;
          END;
         ELSE IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[PrefixCnt]:=$0e+(AdrPart SHL 4);
           Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
           CodeLen:=PrefixCnt+1+AdrCnt;
          END;
         END;
        END;
       ModY:
        BEGIN
         PrefixCnt:=0;
         DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIY+MModIY8+
                   MModIY16+MModIAbs8+MModIAbs16+MModIYAbs8+MModIYAbs16+
                   MModA+MModX+MModS);
         CASE AdrType OF
         ModA:
          BEGIN
           AddPrefix($90); BAsmCode[PrefixCnt]:=$97; CodeLen:=PrefixCnt+1;
          END;
         ModX:
          BEGIN
           AddPrefix($90); BAsmCode[PrefixCnt]:=$93; CodeLen:=PrefixCnt+1;
          END;
         ModS:
          BEGIN
           AddPrefix($90); BAsmCode[PrefixCnt]:=$96; CodeLen:=PrefixCnt+1;
          END;
         ELSE IF AdrType<>ModNone THEN
          BEGIN
           IF PrefixCnt=0 THEN AddPrefix($90);
           IF BAsmCode[0]=$92 THEN Dec(BAsmCode[0]);
           BAsmCode[PrefixCnt]:=$0e+(AdrPart SHL 4);
           Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
           CodeLen:=PrefixCnt+1+AdrCnt;
          END;
         END;
        END;
       ModS:
        BEGIN
         DecodeAdr(ArgStr[2],MModA+MModX+MModY);
         CASE AdrType OF
         ModA:
          BEGIN
           BAsmCode[PrefixCnt]:=$95; CodeLen:=PrefixCnt+1;
          END;
         ModX,ModY:
          BEGIN
           BAsmCode[PrefixCnt]:=$94; CodeLen:=PrefixCnt+1;
          END;
         END;
        END;
       ELSE IF AdrType<>ModNone THEN
        BEGIN
         PrefixCnt:=0; DecodeAdr(ArgStr[2],MModA+MModX+MModY);
         CASE AdrType OF
         ModA:
          BEGIN
           Mask:=MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
                 MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
                 MModIYAbs8+MModIYAbs16;
           DecodeAdr(ArgStr[1],Mask);
           IF AdrType<>ModNone THEN
            BEGIN
             BAsmCode[PrefixCnt]:=$07+(AdrPart SHL 4);
             Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
             CodeLen:=PrefixCnt+1+AdrCnt;
            END;
          END;
         ModX:
          BEGIN
           DecodeAdr(ArgStr[1],MModAbs8+MModAbs16+MModIX+MModIX8+
                     MModIX16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16);
           IF AdrType<>ModNone THEN
            BEGIN
             BAsmCode[PrefixCnt]:=$0f+(AdrPart SHL 4);
             Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
             CodeLen:=PrefixCnt+1+AdrCnt;
            END;
          END;
         ModY:
          BEGIN
           PrefixCnt:=0;
           DecodeAdr(ArgStr[1],MModAbs8+MModAbs16+MModIY+MModIY8+
                     MModIY16+MModIAbs8+MModIAbs16+MModIYAbs8+MModIYAbs16);
           IF AdrType<>ModNone THEN
            BEGIN
             IF PrefixCnt=0 THEN AddPrefix($90);
             IF BAsmCode[0]=$92 THEN Dec(BAsmCode[0]);
             BAsmCode[PrefixCnt]:=$0f+(AdrPart SHL 4);
             Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
             CodeLen:=PrefixCnt+1+AdrCnt;
            END;
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModA+MModX+MModY+MModCCR);
       IF AdrType<>ModNone THEN
        BEGIN
         CASE AdrType OF
         ModA:BAsmCode[PrefixCnt]:=$84;
         ModX,ModY:BAsmCode[PrefixCnt]:=$85;
         ModCCR:BAsmCode[PrefixCnt]:=$86;
         END;
         IF Memo('PUSH') THEN Inc(BAsmCode[PrefixCnt],4);
         CodeLen:=PrefixCnt+1;
        END;
      END;
     Exit;
    END;

   { Arithmetik }

   IF Memo('CP') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModA+MModX+MModY);
       CASE AdrType OF
       ModA:
        BEGIN
         Mask:=MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
               MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
               MModIYAbs8+MModIYAbs16;
         DecodeAdr(ArgStr[2],Mask);
         IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[PrefixCnt]:=$01+(AdrPart SHL 4);
           Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
           CodeLen:=PrefixCnt+1+AdrCnt;
          END;
        END;
       ModX:
        BEGIN
         DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIX+MModIX8+
                   MModIX16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16);
         IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[PrefixCnt]:=$03+(AdrPart SHL 4);
           Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
           CodeLen:=PrefixCnt+1+AdrCnt;
          END;
        END;
       ModY:
        BEGIN
         PrefixCnt:=0;
         DecodeAdr(ArgStr[2],MModImm+MModAbs8+MModAbs16+MModIY+MModIY8+
                   MModIY16+MModIAbs8+MModIAbs16+MModIYAbs8+MModIYAbs16);
         IF AdrType<>ModNone THEN
          BEGIN
           IF PrefixCnt=0 THEN AddPrefix($90);
           IF BAsmCode[0]=$92 THEN Dec(BAsmCode[0]);
           BAsmCode[PrefixCnt]:=$03+(AdrPart SHL 4);
           Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
           CodeLen:=PrefixCnt+1+AdrCnt;
          END;
        END;
       END;
      END;
     Exit;
    END;

   FOR z:=0 TO AriOrderCnt-1 DO
    WITH AriOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModA);
         IF AdrType=ModA THEN
          BEGIN
           Mask:=MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
                 MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
                 MModIYAbs8+MModIYAbs16;
           IF MayImm THEN Inc(Mask,MModImm);
           DecodeAdr(ArgStr[2],Mask);
           IF AdrType<>ModNone THEN
            BEGIN
             BAsmCode[PrefixCnt]:=Code+(AdrPart SHL 4);
             Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
             CodeLen:=PrefixCnt+1+AdrCnt;
            END;
          END;
        END;
       Exit;
      END;

   FOR z:=0 TO RMWOrderCnt-1 DO
    WITH RMWOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModA+MModX+MModY+MModAbs8+MModIX+MModIX8+
                             MModIY+MModIY8+MModIAbs8+MModIXAbs8+MModIYAbs8);
         CASE AdrType OF
         ModA:
          BEGIN
           BAsmCode[PrefixCnt]:=$40+Code; CodeLen:=PrefixCnt+1;
          END;
         ModX,ModY:
          BEGIN
           BAsmCode[PrefixCnt]:=$50+Code; CodeLen:=PrefixCnt+1;
          END;
         ELSE IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[PrefixCnt]:=Code+((AdrPart-8) SHL 4);
           Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
           CodeLen:=PrefixCnt+1+AdrCnt;
          END;
         END;
        END;
       Exit;
      END;

   IF Memo('MUL') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModA);
       IF AdrType<>ModNone THEN
        BEGIN
         DecodeAdr(ArgStr[1],MModX+MModY);
         IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[PrefixCnt]:=$42; CodeLen:=PrefixCnt+1;
          END;
        END;
      END;
     Exit;
    END;

   { Bitbefehle }

   IF (Memo('BRES')) OR (Memo('BSET')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgStr[2][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       z:=EvalIntExpression(Copy(ArgStr[2],2,Length(ArgStr[2])-1),UInt3,OK);
       IF OK THEN
        BEGIN
         DecodeAdr(ArgStr[1],MModAbs8+MModIAbs8);
         IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[PrefixCnt]:=$10+Ord(Memo('BRES'))+(z SHL 1);
           Move(AdrVals,BAsmCode[1+PrefixCnt],AdrCnt);
           CodeLen:=PrefixCnt+1+AdrCnt;
          END;
        END;
      END;
     Exit;
    END;

   IF (Memo('BTJF')) OR (Memo('BTJT')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgStr[2][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       z:=EvalIntExpression(Copy(ArgStr[2],2,Length(ArgStr[2])-1),UInt3,OK);
       IF OK THEN
        BEGIN
         DecodeAdr(ArgStr[1],MModAbs8+MModIAbs8);
         IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[PrefixCnt]:=$00+Ord(Memo('BTJF'))+(z SHL 1);
           Move(AdrVals,BAsmCode[1+PrefixCnt],AdrCnt);
           AdrInt:=EvalIntExpression(ArgStr[3],UInt16,OK)-(EProgCounter+PrefixCnt+1+AdrCnt);
           IF OK THEN
            IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
            ELSE
             BEGIN
              BAsmCode[PrefixCnt+1+AdrCnt]:=AdrInt AND $ff;
              CodeLen:=PrefixCnt+1+AdrCnt+1;
             END;
          END;
        END;
      END;
     Exit;
    END;

   { SprÅnge }

   IF (Memo('JP')) OR (Memo('CALL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       Mask:=MModAbs8+MModAbs16+MModIX+MModIX8+MModIX16+MModIY+
             MModIY8+MModIY16+MModIAbs8+MModIAbs16+MModIXAbs8+MModIXAbs16+
             MModIYAbs8+MModIYAbs16;
       DecodeAdr(ArgStr[1],Mask);
       IF AdrType<>ModNone THEN
        BEGIN
         BAsmCode[PrefixCnt]:=$0c+Ord(Memo('CALL'))+(AdrPart SHL 4);
         Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
         CodeLen:=PrefixCnt+1+AdrCnt;
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO RelOrderCnt-1 DO
    WITH RelOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF AttrPArt<>'' THEN WrError(1100)
       ELSE IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF ArgStr[1][1]='[' THEN
        BEGIN
         DecodeAdr(ArgStr[1],MModIAbs8);
         IF AdrType<>ModNone THEN
          BEGIN
           BAsmCode[PrefixCnt]:=Code;
           Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
           CodeLen:=PrefixCnt+1+AdrCnt;
          END;
        END
       ELSE
        BEGIN
         AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+2);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
          ELSE
           BEGIN
            BAsmCode[0]:=Code; BAsmCode[1]:=AdrInt AND $ff;
            CodeLen:=2;
           END;
        END;
       Exit;
      END;

   { nix gefunden }

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_ST7:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode: ok:=ProgCounter<$10000;
   ELSE ok:=False;
   END;
   ChkPC_ST7:=(ok) AND (ProgCounter>=0);
END;


        FUNCTION IsDef_ST7:Boolean;
	Far;
BEGIN
   IsDef_ST7:=False;
END;

        PROCEDURE SwitchFrom_ST7;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_ST7;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='PC'; HeaderID:=$33; NOPCode:=$9d;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_ST7; ChkPC:=ChkPC_ST7; IsDef:=IsDef_ST7;
   SwitchFrom:=SwitchFrom_ST7; InitFields;

   SetFlag(DoPadding,DoPaddingName,False);
END;

BEGIN
   CPUST7:=AddCPU('ST7',SwitchTo_ST7);
END.


