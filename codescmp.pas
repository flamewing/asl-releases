{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeSCMP;

{ AS Codegeneratormodul SC/MP }

INTERFACE
        Uses NLS,
             Chunks,AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

CONST
   FixedOrderCnt=21;
   ImmOrderCnt=8;
   RegOrderCnt=3;
   MemOrderCnt=8;
   JmpOrderCnt=4;

TYPE
   FixedOrder=RECORD
               Name:String[5];
               Code:Byte;
              END;

   FixedOrderArray=ARRAY[0..FixedOrderCnt-1] OF FixedOrder;
   ImmOrderArray=ARRAY[0..ImmOrderCnt-1] OF FixedOrder;
   RegOrderArray=ARRAY[0..RegOrderCnt-1] OF FixedOrder;
   MemOrderArray=ARRAY[0..MemOrderCnt-1] OF FixedOrder;
   JmpOrderArray=ARRAY[0..JmpOrderCnt-1] OF FixedOrder;

VAR
   CPUSCMP:CPUVar;

   FixedOrders:^FixedOrderArray;
   ImmOrders:^ImmOrderArray;
   RegOrders:^RegOrderArray;
   MemOrders:^MemOrderArray;
   JmpOrders:^JmpOrderArray;

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

        PROCEDURE AddImm(NName:String; NCode:Byte);
BEGIN
   IF z>=ImmOrderCnt THEN Halt(255);
   WITH ImmOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddReg(NName:String; NCode:Byte);
BEGIN
   IF z>=RegOrderCnt THEN Halt(255);
   WITH RegOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddMem(NName:String; NCode:Byte);
BEGIN
   IF z>=MemOrderCnt THEN Halt(255);
   WITH MemOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddJmp(NName:String; NCode:Byte);
BEGIN
   IF z>=JmpOrderCnt THEN Halt(255);
   WITH JmpOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('LDE' ,$40); AddFixed('XAE' ,$01); AddFixed('ANE' ,$50);
   AddFixed('ORE' ,$58); AddFixed('XRE' ,$60); AddFixed('DAE' ,$68);
   AddFixed('ADE' ,$70); AddFixed('CAE' ,$78); AddFixed('SIO' ,$19);
   AddFixed('SR'  ,$1c); AddFixed('SRL' ,$1d); AddFixed('RR'  ,$1e);
   AddFixed('RRL' ,$1f); AddFixed('HALT',$00); AddFixed('CCL' ,$02);
   AddFixed('SCL' ,$03); AddFixed('DINT',$04); AddFixed('IEN' ,$05);
   AddFixed('CSA' ,$06); AddFixed('CAS' ,$07); AddFixed('NOP' ,$08);

   New(ImmOrders); z:=0;
   AddImm('LDI' ,$c4); AddImm('ANI' ,$d4); AddImm('ORI' ,$dc);
   AddImm('XRI' ,$e4); AddImm('DAI' ,$ec); AddImm('ADI' ,$f4);
   AddImm('CAI' ,$fc); AddImm('DLY' ,$8f);

   New(RegOrders); z:=0;
   AddReg('XPAL',$30); AddReg('XPAH',$34); AddReg('XPPC',$3c);

   New(MemOrders); z:=0;
   AddMem('LD'  ,$c0); AddMem('ST'  ,$c8); AddMem('AND' ,$d0);
   AddMem('OR'  ,$d8); AddMem('XOR' ,$e0); AddMem('DAD' ,$e8);
   AddMem('ADD' ,$f0); AddMem('CAD' ,$f8);

   New(JmpOrders); z:=0;
   AddJmp('JMP' ,$90); AddJmp('JP'  ,$94); AddJmp('JZ'  ,$98);
   AddJmp('JNZ' ,$9c);
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(ImmOrders);
   Dispose(RegOrders);
   Dispose(MemOrders);
   Dispose(JmpOrders);
END;

{-----------------------------------------------------------------------------}

	FUNCTION DecodeReg(Asc:String; VAR Erg:Byte):Boolean;
BEGIN
   DecodeReg:=False;

   IF (Length(Asc)<>2) OR (UpCase(Asc[1])<>'P') THEN Exit;

   CASE UpCase(Asc[2]) OF
   'C':Erg:=0;
   '0'..'3':Erg:=Ord(Asc[2])-AscOfs;
   ELSE Exit;
   END;

   DecodeReg:=True;
END;

	FUNCTION DecodeAdr(Asc:String; MayInc:Boolean; PCDisp:Byte; VAR Arg:Byte):Boolean;
VAR
   Disp:Integer;
   PCVal:Word;
   Reg:Byte;
   OK:Boolean;
BEGIN
   DecodeAdr:=False;

   IF (Length(Asc)>=4) AND (Asc[Length(Asc)]=')') AND (Asc[Length(Asc)-3]='(')
      AND (DecodeReg(Copy(Asc,Length(Asc)-2,2),Arg)) THEN
    BEGIN
     Delete(Asc,Length(Asc)-3,4);
     IF Asc[1]='@' THEN
      BEGIN
       IF NOT MayInc THEN
        BEGIN
         WrError(1350); Exit;
        END;
       Delete(Asc,1,1); Inc(Arg,4);
      END;
     IF NLS_StrCaseCmp(Asc,'E')=0 THEN BAsmCode[1]:=$80
     ELSE IF Arg=0 THEN
      BEGIN
       WrXError(1445,Copy(Asc,Length(Asc)-2,2)); Exit;
      END
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(Asc,SInt8,OK);
       IF NOT OK THEN Exit;
      END;
     DecodeAdr:=True; Exit;
    END;

   PCVal:=(EProgCounter AND $f000)+((EProgCounter+1) AND $fff);
   Disp:=EvalIntExpression(Asc,UInt16,OK)-PCDisp-PCVal;
   IF OK THEN
    IF (NOT SymbolQuestionable) AND ((Disp<-128) OR (Disp>127)) THEN WrError(1370)
    ELSE
     BEGIN
      BAsmCode[1]:=Disp AND $ff; Arg:=0; DecodeAdr:=True;
     END;
END;

{-----------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

        PROCEDURE ChkPage;
BEGIN
   IF ((EProgCounter) AND $f000)<>((EProgCounter+CodeLen) AND $f000) THEN WrError(250);
END;

        PROCEDURE MakeCode_SCMP;
	Far;
VAR
   z:Integer;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

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

   { immediate }

   FOR z:=0 TO ImmOrderCnt-1 DO
    WITH ImmOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         BAsmCode[1]:=EvalIntExpression(ArgStr[1],Int8,OK);
         IF OK THEN
          BEGIN
           BAsmCode[0]:=Code; CodeLen:=2; ChkPage;
          END;
        END;
       Exit;
      END;

   { ein Register }

   FOR z:=0 TO RegOrderCnt-1 DO
    WITH RegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF NOT DecodeReg(ArgStr[1],BAsmCode[0]) THEN WrXError(1445,ArgStr[1])
       ELSE
        BEGIN
         Inc(BAsmCode[0],Code); CodeLen:=1;
        END;
       Exit;
      END;


   { ein Speicheroperand }

   FOR z:=0 TO MemOrderCnt-1 DO
    WITH MemOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF DecodeAdr(ArgStr[1],True,0,BAsmCode[0]) THEN
        BEGIN
         Inc(BAsmCode[0],Code); CodeLen:=2; ChkPage;
        END;
       Exit;
      END;

   FOR z:=0 TO JmpOrderCnt-1 DO
    WITH JmpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF DecodeAdr(ArgStr[1],False,1,BAsmCode[0]) THEN
        BEGIN
         Inc(BAsmCode[0],Code); CodeLen:=2; ChkPage;
        END;
       Exit;
      END;

   IF (Memo('ILD')) OR (Memo('DLD')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF DecodeAdr(ArgStr[1],False,0,BAsmCode[0]) THEN
      BEGIN
       Inc(BAsmCode[0],$a8+(Ord(Memo('DLD')) SHL 4)); CodeLen:=2; ChkPage;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_SCMP:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<$10000;
   ELSE ok:=False;
   END;
   ChkPC_SCMP:=(ok) AND (ProgCounter>=0);
END;


        FUNCTION IsDef_SCMP:Boolean;
	Far;
BEGIN
   IsDef_SCMP:=False;
END;

        PROCEDURE SwitchFrom_SCMP;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_SCMP;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeC; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$6e; NOPCode:=$08;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_SCMP; ChkPC:=ChkPC_SCMP; IsDef:=IsDef_SCMP;
   SwitchFrom:=SwitchFrom_SCMP; InitFields;
END;

BEGIN
   CPUSCMP:=AddCPU('SC/MP',SwitchTo_SCMP);
END.


