{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeM16C;

{ AS - Codegeneratormodul Mitsubishi M16C }

INTERFACE

        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

CONST
   ModNone=-1;
   ModGen=0;      MModGen=1 SHL ModGen;
   ModAbs20=1;    MModAbs20=1 SHL ModAbs20;
   ModAReg32=2;   MModAReg32=1 SHL ModAReg32;
   ModDisp20=3;   MModDisp20=1 SHL ModDisp20;
   ModReg32=4;    MModReg32=1 SHL ModReg32;
   ModIReg32=5;   MModIReg32=1 SHL ModIReg32;
   ModImm=6;      MModImm=1 SHL ModImm;
   ModSPRel=7;    MModSPRel=1 SHL ModSPRel;

   FixedOrderCnt=8;

   StringOrderCnt=4;

   Gen1OrderCnt=5;

   Gen2OrderCnt=6;

   DivOrderCnt=3;

   ConditionCnt=18;

   BCDOrderCnt=4;

   DirOrderCnt=4;

   BitOrderCnt=13;

TYPE
   FixedOrder=RECORD
	       Name:String[6];
               Code:Word;
	      END;
   FixedOrderArray=ARRAY[0..FixedOrderCnt-1] OF FixedOrder;
   StringOrderArray=ARRAY[0..StringOrderCnt-1] OF FixedOrder;
   Gen1OrderArray=ARRAY[0..Gen1OrderCnt-1] OF FixedOrder;
   BitOrderArray=ARRAY[0..BitOrderCnt-1] OF FixedOrder;

   Gen2Order=RECORD
	      Name:String[6];
              Code1,Code2,Code3:Byte;
	     END;
   Gen2OrderArray=ARRAY[0..Gen2OrderCnt-1] OF Gen2Order;
   DivOrderArray=ARRAY[0..DivOrderCnt-1] OF Gen2Order;

   Condition=RECORD
              Name:String[3];
              Code:Byte;
             END;
   ConditionArray=ARRAY[0..ConditionCnt-1] OF Condition;

   BCDOrderArray=ARRAY[0..3] OF String[4];
   DirOrderArray=ARRAY[0..3] OF String[5];

VAR
   CPUM16C,CPUM30600M8:CPUVar;

   Format:String;
   FormatCode:Byte;
   OpSize:ShortInt;
   AdrMode:Byte;
   AdrType:ShortInt;
   AdrCnt:Byte;
   AdrVals:ARRAY[0..2] OF Byte;

   FixedOrders:^FixedOrderArray;
   StringOrders:^StringOrderArray;
   Gen1Orders:^Gen1OrderArray;
   Gen2Orders:^Gen2OrderArray;
   DivOrders:^DivOrderArray;
   Conditions:^ConditionArray;
   BCDOrders:^BCDOrderArray;
   DirOrders:^DirOrderArray;
   BitOrders:^BitOrderArray;

{--------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   IF z>=FixedOrderCnt THEN Halt(255);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddString(NName:String; NCode:Word);
BEGIN
   IF z>=StringOrderCnt THEN Halt(255);
   WITH StringOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddGen1(NName:String; NCode:Word);
BEGIN
   IF z>=Gen1OrderCnt THEN Halt(255);
   WITH Gen1Orders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

        PROCEDURE AddGen2(NName:String; NCode1,NCode2,NCode3:Word);
BEGIN
   IF z>=Gen2OrderCnt THEN Halt(255);
   WITH Gen2Orders^[z] DO
    BEGIN
     Name:=NName;
     Code1:=NCode1;
     Code2:=NCode2;
     Code3:=NCode3;
    END;
   Inc(z);
END;

        PROCEDURE AddDiv(NName:String; NCode1,NCode2,NCode3:Word);
BEGIN
   IF z>=DivOrderCnt THEN Halt(255);
   WITH DivOrders^[z] DO
    BEGIN
     Name:=NName;
     Code1:=NCode1;
     Code2:=NCode2;
     Code3:=NCode3;
    END;
   Inc(z);
END;

        PROCEDURE AddCondition(NName:String; NCode:Word);
BEGIN
   IF z>=ConditionCnt THEN Halt(255);
   WITH Conditions^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE AddBCD(NName:String);
BEGIN
   IF z>=BCDOrderCnt THEN Halt(255);
   BCDOrders^[z]:=NName;
   Inc(z);
END;

	PROCEDURE AddDir(NName:String);
BEGIN
   IF z>=DirOrderCnt THEN Halt(255);
   DirOrders^[z]:=NName;
   Inc(z);
END;

        PROCEDURE AddBit(NName:String; NCode:Word);
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
   z:=0; New(FixedOrders);
   AddFixed('BRK'   ,$0000);
   AddFixed('EXITD' ,$7df2);
   AddFixed('INTO'  ,$00f6);
   AddFixed('NOP'   ,$0004);
   AddFixed('REIT'  ,$00fb);
   AddFixed('RTS'   ,$00f3);
   AddFixed('UND'   ,$00ff);
   AddFixed('WAIT'  ,$7df3);

   z:=0; New(StringOrders);
   AddString('RMPA' ,$7cf1);
   AddString('SMOVB',$7ce9);
   AddString('SMOVF',$7ce8);
   AddString('SSTR' ,$7cea);

   z:=0; New(Gen1Orders);
   AddGen1('ABS' ,$76f0);
   AddGen1('ADCF',$76e0);
   AddGen1('NEG' ,$7450);
   AddGen1('ROLC',$76a0);
   AddGen1('RORC',$76b0);

   z:=0; New(Gen2Orders);
   AddGen2('ADC' ,$b0,$76,$60);
   AddGen2('SBB' ,$b8,$76,$70);
   AddGen2('TST' ,$80,$76,$00);
   AddGen2('XOR' ,$88,$76,$10);
   AddGen2('MUL' ,$78,$7c,$50);
   AddGen2('MULU',$70,$7c,$40);

   z:=0; New(DivOrders);
   AddDiv('DIV' ,$e1,$76,$d0);
   AddDiv('DIVU',$e0,$76,$c0);
   AddDiv('DIVX',$e3,$76,$90);

   z:=0; New(Conditions);
   AddCondition('GEU', 0); AddCondition('C'  , 0);
   AddCondition('GTU', 1); AddCondition('EQ' , 2);
   AddCondition('Z'  , 2); AddCondition('N'  , 3);
   AddCondition('LTU', 4); AddCondition('NC' , 4);
   AddCondition('LEU', 5); AddCondition('NE' , 6);
   AddCondition('NZ' , 6); AddCondition('PZ' , 7);
   AddCondition('LE' , 8); AddCondition('O'  , 9);
   AddCondition('GE' ,10); AddCondition('GT' ,12);
   AddCondition('NO' ,13); AddCondition('LT' ,14);

   z:=0; New(BCDOrders);
   AddBCD('DADD'); AddBCD('DSUB'); AddBCD('DADC'); AddBCD('DSBB');

   z:=0; New(DirOrders);
   AddDir('MOVLL'); AddDir('MOVHL'); AddDir('MOVLH'); AddDir('MOVHH');

   z:=0; New(BitOrders);
   AddBit('BAND'  , 4); AddBit('BNAND' , 5);
   AddBit('BNOR'  , 7); AddBit('BNTST' , 3);
   AddBit('BNXOR' ,13); AddBit('BOR'   , 6);
   AddBit('BTSTC' , 0); AddBit('BTSTS' , 1);
   AddBit('BXOR'  ,12); AddBit('BCLR'  , 8);
   AddBit('BNOT'  ,10); AddBit('BSET'  , 9);
   AddBit('BTST'  ,11);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(StringOrders);
   Dispose(Gen1Orders);
   Dispose(Gen2Orders);
   Dispose(DivOrders);
   Dispose(Conditions);
   Dispose(BCDOrders);
   Dispose(DirOrders);
   Dispose(BitOrders);
END;

{--------------------------------------------------------------------------}
{ Adre·parser }

        PROCEDURE SetOpSize(NSize:ShortInt);
BEGIN
   IF OpSize=-1 THEN OpSize:=NSize
   ELSE IF NSize<>OpSize THEN
    BEGIN
     WrError(1131);
     AdrCnt:=0; AdrType:=ModNone;
    END;
END;

        PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   Found;
VAR
   DispAcc:LongInt;
   RegPart:String;
   p:Integer;
   OK:Boolean;
BEGIN
   AdrCnt:=0; AdrType:=ModNone;

   { Datenregister 8 Bit }

   IF (Length(Asc)=3) AND (UpCase(Asc[1])='R') AND (Asc[2]>='0') AND (Asc[2]<='1') AND
      ((UpCase(Asc[3])='L') OR (UpCase(Asc[3])='H')) THEN
    BEGIN
     AdrType:=ModGen;
     AdrMode:=((Ord(Asc[2])-AscOfs) SHL 1)+Ord(UpCase(Asc[3])='H');
     SetOpSize(0);
     Goto Found;
    END;

   { Datenregister 16 Bit }

   IF (Length(Asc)=2) AND (UpCase(Asc[1])='R') AND (Asc[2]>='0') AND (Asc[2]<='3') THEN
    BEGIN
     AdrType:=ModGen;
     AdrMode:=Ord(Asc[2])-AscOfs;
     SetOpSize(1);
     Goto Found;
    END;

   { Datenregister 32 Bit }

   IF NLS_StrCaseCmp(Asc,'R2R0')=0 THEN
    BEGIN
     AdrType:=ModReg32; AdrMode:=0;
     SetOpSize(2);
     Goto Found;
    END;

   IF NLS_StrCaseCmp(Asc,'R3R1')=0 THEN
    BEGIN
     AdrType:=ModReg32; AdrMode:=1;
     SetOpSize(2);
     Goto Found;
    END;

   { Adre·register }

   IF (Length(Asc)=2) AND (UpCase(Asc[1])='A') AND (Asc[2]>='0') AND (Asc[2]<='1') THEN
    BEGIN
     AdrType:=ModGen;
     AdrMode:=Ord(Asc[2])-AscOfs+4;
     Goto Found;
    END;

   { Adre·register 32 Bit }

   IF NLS_StrCaseCmp(Asc,'A1A0')=0 THEN
    BEGIN
     AdrType:=ModAReg32;
     SetOpSize(2);
     Goto Found;
    END;

   { indirekt }

   p:=Pos('[',Asc);
   IF (p<>0) AND (Asc[Length(Asc)]=']') THEN
    BEGIN
     RegPart:=Copy(Asc,p+1,Length(Asc)-p-1);
     IF (NLS_StrCaseCmp(RegPart,'A0')=0) OR (NLS_StrCaseCmp(RegPart,'A1')=0) THEN
      BEGIN
       IF Mask AND MModDisp20=0 THEN
        DispAcc:=EvalIntExpression(Copy(Asc,1,p-1),Int16,OK)
       ELSE
        DispAcc:=EvalIntExpression(Copy(Asc,1,p-1),Int20,OK);
       IF OK THEN
        IF (DispAcc=0) AND (Mask AND MModGen<>0) THEN
         BEGIN
          AdrType:=ModGen;
          AdrMode:=Ord(RegPart[2])-AscOfs+6;
         END
        ELSE IF (DispAcc>=0) AND (DispAcc<=255) AND (Mask AND MModGen<>0) THEN
         BEGIN
          AdrType:=ModGen;
          AdrVals[0]:=DispAcc AND $ff;
          AdrCnt:=1;
          AdrMode:=Ord(RegPart[2])-AscOfs+8;
         END
        ELSE IF (DispAcc>=-32768) AND (DispAcc<=65535) AND (Mask AND MModGen<>0) THEN
         BEGIN
          AdrType:=ModGen;
          AdrVals[0]:=DispAcc AND $ff; AdrVals[1]:=(DispAcc SHR 8) AND $ff;
          AdrCnt:=2;
          AdrMode:=Ord(RegPart[2])-AscOfs+12;
         END
        ELSE IF NLS_StrCaseCmp(RegPart,'A0')<>0 THEN WrError(1350)
        ELSE
         BEGIN
          AdrType:=ModDisp20;
          AdrVals[0]:=DispAcc AND $ff;
          AdrVals[1]:=(DispAcc SHR 8) AND $ff;
          AdrVals[2]:=(DispAcc SHR 16) AND $0f;
          AdrCnt:=3;
          AdrMode:=Ord(RegPart[2])-AscOfs;
         END;
      END
     ELSE IF NLS_StrCaseCmp(RegPart,'SB')=0 THEN
      BEGIN
       DispAcc:=EvalIntExpression(Copy(Asc,1,p-1),Int16,OK);
       IF OK THEN
        IF (DispAcc>=0) AND (DispAcc<=255) THEN
         BEGIN
          AdrType:=ModGen;
          AdrVals[0]:=DispAcc AND $ff;
          AdrCnt:=1;
          AdrMode:=10;
         END
        ELSE
         BEGIN
          AdrType:=ModGen;
          AdrVals[0]:=DispAcc AND $ff; AdrVals[1]:=(DispAcc SHR 8) AND $ff;
          AdrCnt:=2;
          AdrMode:=14;
         END;
      END
     ELSE IF NLS_StrCaseCmp(RegPart,'FB')=0 THEN
      BEGIN
       DispAcc:=EvalIntExpression(Copy(Asc,1,p-1),SInt8,OK);
       IF OK THEN
        BEGIN
         AdrType:=ModGen;
         AdrVals[0]:=DispAcc AND $ff;
         AdrCnt:=1;
         AdrMode:=11;
        END;
      END
     ELSE IF NLS_StrCaseCmp(RegPart,'SP')=0 THEN
      BEGIN
       DispAcc:=EvalIntExpression(Copy(Asc,1,p-1),SInt8,OK);
       IF OK THEN
        BEGIN
         AdrType:=ModSPRel;
         AdrVals[0]:=DispAcc AND $ff;
         AdrCnt:=1;
        END;
      END
     ELSE IF NLS_StrCaseCmp(RegPart,'A1A0')=0 THEN
      BEGIN
       DispAcc:=EvalIntExpression(Copy(Asc,1,p-1),SInt8,OK);
       IF OK THEN
        IF DispAcc<>0 THEN WrError(1320)
        ELSE AdrType:=ModIReg32;
      END;
     Goto Found;
    END;

   { immediate }

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OpSize OF
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
          AdrType:=ModImm;
          AdrVals[0]:=DispAcc AND $ff;
          AdrVals[1]:=(DispAcc SHR 8) AND $ff;
          AdrCnt:=2;
         END;
       END;
     END;
     Goto Found;
    END;

   { dann absolut }

   IF Mask AND MModAbs20=0 THEN
    DispAcc:=EvalIntExpression(Asc,UInt16,OK)
   ELSE
    DispAcc:=EvalIntExpression(Asc,UInt20,OK);
   IF (DispAcc<=$ffff) AND (Mask AND MModGen<>0) THEN
    BEGIN
     AdrType:=ModGen;
     AdrMode:=15;
     AdrVals[0]:=DispAcc AND $ff;
     AdrVals[1]:=(DispAcc SHR 8) AND $ff;
     AdrCnt:=2;
    END
   ELSE
    BEGIN
     AdrType:=ModAbs20;
     AdrVals[0]:=DispAcc AND $ff;
     AdrVals[1]:=(DispAcc SHR 8) AND $ff;
     AdrVals[2]:=(DispAcc SHR 16) AND $0f;
     AdrCnt:=3;
    END;

Found:
   IF (AdrType<>ModNone) AND (Mask AND (1 SHL AdrType)=0) THEN
    BEGIN
     AdrCnt:=0; AdrType:=ModNone; WrError(1350);
    END;
END;

        FUNCTION DecodeReg(Asc:String; VAR Erg:Byte):Boolean;
BEGIN
   DecodeReg:=True; NLS_UpString(Asc);
   IF Asc='FB' THEN Erg:=7
   ELSE IF Asc='SB' THEN Erg:=6
   ELSE IF (Length(Asc)=2) AND (Asc[1]='A') AND
           (Asc[2]>='0') AND (Asc[2]<='1') THEN Erg:=Ord(Asc[2])-AscOfs+4
   ELSE IF (Length(Asc)=2) AND (Asc[1]='R') AND
           (Asc[2]>='0') AND (Asc[2]<='3') THEN Erg:=Ord(Asc[2])-AscOfs
   ELSE DecodeReg:=False;
END;

        FUNCTION DecodeCReg(Asc:String; VAR Erg:Byte):Boolean;
BEGIN
   DecodeCReg:=True; NLS_UpString(Asc);
   IF Asc='INTBL' THEN Erg:=1
   ELSE IF Asc='INTBH' THEN Erg:=2
   ELSE IF Asc='FLG' THEN Erg:=3
   ELSE IF Asc='ISP' THEN Erg:=4
   ELSE IF Asc='SP' THEN Erg:=5
   ELSE IF Asc='SB' THEN Erg:=6
   ELSE IF Asc='FB' THEN Erg:=7
   ELSE
    BEGIN
     WrXError(1440,Asc); DecodeCReg:=False;
    END;
END;

	FUNCTION DecodeBitAdr(MayShort:Boolean):Boolean;
VAR
   DispAcc,DispPart:LongInt;
   OK:Boolean;
   Pos1:Integer;
   Asc,Reg:String;

   	PROCEDURE DecodeDisp(Type1,Type2:IntType);
BEGIN
   IF ArgCnt=2 THEN Inc(DispAcc,EvalIntExpression(Asc,Type2,OK)*8)
   ELSE DispAcc:=EvalIntExpression(Asc,Type1,OK);
END;

BEGIN
   DecodeBitAdr:=False; AdrCnt:=0;

   { Nur 1 oder 2 Argumente zugelassen }

   IF (ArgCnt<1) OR (ArgCnt>2) THEN
    BEGIN
     WrError(1110); Exit;
    END;

   { Ist Teil 1 ein Register ? }

   IF (DecodeReg(ArgStr[ArgCnt],AdrMode)) THEN
    IF AdrMode<6 THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        AdrVals[0]:=EvalIntExpression(ArgStr[1],UInt4,OK);
        IF OK THEN
         BEGIN
          AdrCnt:=1; DecodeBitAdr:=True;
         END;
       END;
      Exit;
     END;

   { Bitnummer ? }

   IF ArgCnt=2 THEN
    BEGIN
     DispAcc:=EvalIntExpression(ArgStr[1],UInt3,OK);
     IF NOT OK THEN Exit;
    END
   ELSE DispAcc:=0;

   { Registerangabe ? }

   Asc:=ArgStr[ArgCnt];
   Pos1:=QuotPos(Asc,'[');

   { nein->absolut }

   IF Pos1>Length(Asc) THEN
    BEGIN
     DecodeDisp(UInt16,UInt13);
     IF OK THEN
      BEGIN
       AdrMode:=15;
       AdrVals[0]:=DispAcc AND $ff;
       AdrVals[1]:=(DispAcc SHR 8) AND $ff;
       AdrCnt:=2;
       DecodeBitAdr:=True;
      END;
     Exit;
    END;

   { Register abspalten }

   IF Asc[Length(Asc)]<>']' THEN
    BEGIN
     WrError(1350); Exit;
    END;
   Reg:=Copy(Asc,Pos1+1,Length(Asc)-Pos1-1);
   Asc:=Copy(Asc,1,Pos1-1);

   IF (Length(Reg)=2) AND (UpCase(Reg[1])='A') AND (Reg[2]>='0') AND (Reg[2]<='1') THEN
    BEGIN
     AdrMode:=Ord(Reg[2])-AscOfs;
     DecodeDisp(UInt13,UInt16);
     IF OK THEN
      BEGIN
       IF DispAcc=0 THEN Inc(AdrMode,6)
       ELSE IF (DispAcc>0) AND (DispAcc<256) THEN
        BEGIN
         Inc(AdrMode,8); AdrVals[0]:=DispAcc AND $ff; AdrCnt:=1;
        END
       ELSE
        BEGIN
         Inc(AdrMode,12);
         AdrVals[0]:=DispAcc AND $ff;
         AdrVals[1]:=(DispAcc SHR 8) AND $ff;
         AdrCnt:=2;
        END;
       DecodeBitAdr:=True;
      END;
    END
   ELSE IF NLS_StrCaseCmp(Reg,'SB')=0 THEN
    BEGIN
     DecodeDisp(UInt13,UInt16);
     IF OK THEN
      BEGIN
       IF (MayShort) AND (DispAcc<$7ff) THEN
        BEGIN
         AdrMode:=16+(DispAcc AND 7);
	 AdrVals[0]:=DispAcc SHR 3; AdrCnt:=1;
        END
       ELSE IF (DispAcc>0) AND (DispAcc<256) THEN
        BEGIN
         AdrMode:=10; AdrVals[0]:=DispAcc AND $ff; AdrCnt:=1;
        END
       ELSE
        BEGIN
         AdrMode:=14;
         AdrVals[0]:=DispAcc AND $ff;
         AdrVals[1]:=(DispAcc SHR 8) AND $ff;
         AdrCnt:=2;
        END;
       DecodeBitAdr:=True;
      END;
    END
   ELSE IF NLS_StrCaseCmp(Reg,'FB')=0 THEN
    BEGIN
     DecodeDisp(SInt5,SInt8);
     IF OK THEN
      BEGIN
       AdrMode:=11; AdrVals[0]:=DispAcc AND $ff; AdrCnt:=1;
       DecodeBitAdr:=True;
      END
    END
   ELSE WrXError(1445,Reg);
END;

{--------------------------------------------------------------------------}

	FUNCTION CheckFormat(FSet:String):Boolean;
BEGIN
   CheckFormat:=True;
   IF Format=' ' THEN FormatCode:=0
   ELSE
    BEGIN
     FormatCode:=Pos(Format,FSet);
     CheckFormat:=FormatCode<>0;
     IF FormatCode=0 THEN WrError(1090);
    END;
END;

	FUNCTION CheckBFieldFormat:Boolean;
BEGIN
   CheckBFieldFormat:=True;
   IF (Format='G:R') OR (Format='R:G') THEN FormatCode:=1
   ELSE IF (Format='G:I') OR (Format='I:G') THEN FormatCode:=2
   ELSE IF (Format='E:R') OR (Format='R:E') THEN FormatCode:=3
   ELSE IF (Format='E:I') OR (Format='I:E') THEN FormatCode:=4
   ELSE
    BEGIN
     CheckBFieldFormat:=False; WrError(1090);
    END;
END;

        FUNCTION ImmVal:Integer;
BEGIN
   IF OpSize=0 THEN ImmVal:=ShortInt(AdrVals[0])
   ELSE ImmVal:=(Integer(AdrVals[1]) SHL 8)+AdrVals[0];
END;

        FUNCTION IsShort(GenMode:Byte; VAR SMode:Byte):Boolean;
BEGIN
   IsShort:=True;
   CASE GenMode OF
   0:SMode:=4;
   1:SMode:=3;
   10:SMode:=5;
   11:SMode:=6;
   15:SMode:=7;
   ELSE IsShort:=False;
   END;
END;

{--------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

        PROCEDURE MakeCode_M16C;
	Far;
VAR
   z,Num1,Num2:Integer;
   AdrLong,Diff:LongInt;
   OK,MayShort:Boolean;
   SMode:Byte;
   AdrMode2,AdrCnt2:Byte;
   AdrType2:ShortInt;
   AdrVals2:ARRAY[0..2] OF Byte;
   OpSize2:ShortInt;

        PROCEDURE CopyAdr;
BEGIN
   AdrType2:=AdrType;
   AdrMode2:=AdrMode;
   AdrCnt2:=AdrCnt;
   Move(AdrVals,AdrVals2,AdrCnt2);
END;

        PROCEDURE CodeGen(GenCode,Imm1Code,Imm2Code:Byte);
BEGIN
   IF AdrType=ModImm THEN
    BEGIN
     BAsmCode[0]:=Imm1Code+OpSize;
     BAsmCode[1]:=Imm2Code+AdrMode2;
     Move(AdrVals2,BAsmCode[2],AdrCnt2);
     Move(AdrVals,BAsmCode[2+AdrCnt2],AdrCnt);
    END
   ELSE
    BEGIN
     BAsmCode[0]:=GenCode+OpSize;
     BAsmCode[1]:=(AdrMode SHL 4)+AdrMode2;
     Move(AdrVals,BAsmCode[2],AdrCnt);
     Move(AdrVals2,BAsmCode[2+AdrCnt],AdrCnt2);
    END;
   CodeLen:=2+AdrCnt+AdrCnt2;
END;

BEGIN
   OpSize:=-1;

   { zu ignorierendes }

   IF Memo('') THEN Exit;

   { Formatangabe abspalten }

   CASE AttrSplit OF
   '.':BEGIN
        Num1:=Pos(':',AttrPart);
        IF Num1<>0 THEN
         BEGIN
          IF Num1<Length(AttrPart) THEN Format:=AttrPart[Num1+1]
          ELSE Format:=' ';
          Delete(AttrPart,Num1,Length(AttrPart)-Num1+1);
         END
        ELSE Format:=' ';
       END;
   ':':BEGIN
        Num1:=Pos('.',AttrPart);
        IF Num1=0 THEN
         BEGIN
          Format:=AttrPart; AttrPart:='';
         END
        ELSE
         BEGIN
          IF Num1=1 THEN Format:=' ' ELSE Format:=Copy(AttrPart,1,Num1-1);
          Delete(AttrPart,1,Num1);
         END;
       END;
   ELSE Format:=' ';
   END;

   { Attribut abarbeiten }

   IF AttrPart='' THEN OpSize:=-1
   ELSE
    CASE UpCase(AttrPart[1]) OF
    'B':OpSize:=0;
    'W':OpSize:=1;
    'L':OpSize:=2;
    'Q':OpSize:=3;
    'S':OpSize:=4;
    'D':OpSize:=5;
    'X':OpSize:=6;
    'A':OpSize:=7;
    ELSE
     BEGIN
      WrError(1107); Exit;
     END;
    END;
   NLS_UpString(Format);

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   { ohne Argument }

   FOR z:=0 TO FixedOrderCnt-1 DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE IF Format<>' ' THEN WrError(1090)
       ELSE IF Hi(Code)=0 THEN
        BEGIN
         BAsmCode[0]:=Lo(Code); CodeLen:=1;
        END
       ELSE
        BEGIN
         BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code); CodeLen:=2;
        END;
       Exit;
      END;

   FOR z:=0 TO StringOrderCnt-1 DO
    WITH StringOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF OpSize=-1 THEN OpSize:=1;
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
       ELSE IF Format<>' ' THEN WrError(1090)
       ELSE IF Hi(Code)=0 THEN
        BEGIN
         BAsmCode[0]:=Lo(Code)+OpSize; CodeLen:=1;
        END
       ELSE
        BEGIN
         BAsmCode[0]:=Hi(Code)+OpSize; BAsmCode[1]:=Lo(Code); CodeLen:=2;
        END;
       Exit;
      END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GSQZ') THEN
      BEGIN
       DecodeAdr(ArgStr[2],MModGen+MModSPRel);
       IF AdrType<>ModNone THEN
        BEGIN
         CopyAdr; DecodeAdr(ArgStr[1],MModGen+MModSPRel+MModImm);
         IF AdrType<>ModNone THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE
           BEGIN
            IF FormatCode=0 THEN
             IF (AdrType2=ModSPRel) OR (AdrType=ModSPRel) THEN FormatCode:=1
             ELSE IF (OpSize=0) AND (AdrType=ModImm) AND (IsShort(AdrMode2,SMode)) THEN
              IF ImmVal=0 THEN FormatCode:=4 ELSE FormatCode:=2
             ELSE IF (AdrType=ModImm) AND (ImmVal>=-8) AND (ImmVal<=7) THEN FormatCode:=3
             ELSE IF (AdrType=ModImm) AND (AdrMode2 AND 14=4) THEN FormatCode:=2
             ELSE IF (OpSize=0) AND (AdrType=ModGen) AND (IsShort(AdrMode,SMode)) AND (AdrMode2 AND 14=4)
                 AND ((AdrMode>=2) OR (Odd(AdrMode XOR AdrMode2))) THEN FormatCode:=2
             ELSE IF (OpSize=0) AND (AdrType=ModGen) AND (AdrMode<=1) AND (IsShort(AdrMode2,SMode))
                 AND ((AdrMode2>=2) OR (Odd(AdrMode XOR AdrMode2))) THEN FormatCode:=2
             ELSE IF (OpSize=0) AND (AdrMode2<=1) AND (AdrType=ModGen) AND (IsShort(AdrMode,SMode))
                 AND ((AdrMode>=2) OR (Odd(AdrMode XOR AdrMode2))) THEN FormatCode:=2
             ELSE FormatCode:=1;
            CASE FormatCode OF
            1:IF AdrType=ModSPRel THEN
               BEGIN
                BAsmCode[0]:=$74+OpSize;
                BAsmCode[1]:=$b0+AdrMode2;
                Move(AdrVals2,BAsmCode[2],AdrCnt2);
                Move(AdrVals,BAsmCode[2+AdrCnt2],AdrCnt);
                CodeLen:=2+AdrCnt+AdrCnt2;
               END
	      ELSE IF AdrType2=ModSPRel THEN
               BEGIN
                BAsmCode[0]:=$74+OpSize;
                BAsmCode[1]:=$30+AdrMode;
                Move(AdrVals,BAsmCode[2],AdrCnt);
                Move(AdrVals2,BAsmCode[2+AdrCnt],AdrCnt2);
                CodeLen:=2+AdrCnt2+AdrCnt;
               END
	      ELSE CodeGen($72,$74,$c0);
            2:IF AdrType=ModImm THEN
               IF AdrType2<>ModGen THEN WrError(1350)
               ELSE IF AdrMode2 AND 14=4 THEN
                BEGIN
                 BAsmCode[0]:=$a2+(OpSize SHL 6)+((AdrMode2 AND 1) SHL 3);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 CodeLen:=1+AdrCnt;
                END
               ELSE IF IsShort(AdrMode2,SMode) THEN
                IF OpSize<>0 THEN WrError(1130)
                ELSE
	         BEGIN
                  BAsmCode[0]:=$c0+SMode;
                  Move(AdrVals,BAsmCode[1],AdrCnt);
                  CodeLen:=1+AdrCnt;
	         END
	       ELSE WrError(1350)
              ELSE IF (AdrType=ModGen) AND (IsShort(AdrMode,SMode)) THEN
               IF AdrType2<>ModGen THEN WrError(1350)
               ELSE IF AdrMode2 AND 14=4 THEN
                IF (AdrMode<=1) AND (NOT Odd(AdrMode XOR AdrMode2)) THEN WrError(1350)
                ELSE
                 BEGIN
                  IF SMode=3 THEN Inc(SMode);
                  BAsmCode[0]:=$30+((AdrMode2 AND 1) SHL 2)+(SMode AND 3);
                  Move(AdrVals,BAsmCode[1],AdrCnt);
                  CodeLen:=1+AdrCnt;
                 END
               ELSE IF AdrMode2 AND 14=0 THEN
                IF (AdrMode<=1) AND (NOT Odd(AdrMode XOR AdrMode2)) THEN WrError(1350)
                ELSE
                 BEGIN
                  IF SMode=3 THEN Inc(SMode);
                  BAsmCode[0]:=$08+((AdrMode2 AND 1) SHL 2)+(SMode AND 3);
                  Move(AdrVals,BAsmCode[1],AdrCnt);
                  CodeLen:=1+AdrCnt;
                 END
               ELSE IF (AdrMode AND 14<>0) OR (NOT IsShort(AdrMode2,SMode)) THEN WrError(1350)
               ELSE IF (AdrMode2<=1) AND (NOT Odd(AdrMode XOR AdrMode2)) THEN WrError(1350)
               ELSE
                BEGIN
                 IF SMode=3 THEN Inc(SMode);
                 BAsmCode[0]:=$00+((AdrMode AND 1) SHL 2)+(SMode AND 3);
                 Move(AdrVals,BAsmCode[1],AdrCnt2);
                 CodeLen:=1+AdrCnt2;
                END
              ELSE WrError(1350);
            3:IF AdrType<>ModImm THEN WrError(1350)
              ELSE
               BEGIN
                Num1:=ImmVal;
                IF ChkRange(Num1,-8,7) THEN
                 BEGIN
                  BAsmCode[0]:=$d8+OpSize;
                  BAsmCode[1]:=(Num1 SHL 4)+AdrMode2;
                  Move(AdrVals2,BAsmCode[2],AdrCnt2);
                  CodeLen:=2+AdrCnt2;
                 END;
               END;
            4:IF OpSize<>0 THEN WrError(1130)
	      ELSE IF AdrType<>ModImm THEN WrError(1350)
              ELSE IF NOT IsShort(AdrMode2,SMode) THEN WrError(1350)
              ELSE
               BEGIN
                Num1:=ImmVal;
                IF ChkRange(Num1,0,0) THEN
                 BEGIN
                  BAsmCode[0]:=$b0+SMode;
                  Move(AdrVals2,BAsmCode[1],AdrCnt2);
                  CodeLen:=1+AdrCnt2;
                 END;
               END;
            END;
           END;
        END;
      END;
     Exit;
    END;

   IF (Memo('LDC')) OR (Memo('STC')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       IF Memo('STC') THEN
        BEGIN
         ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
         z:=1;
        END
       ELSE z:=0;
       IF NLS_StrCaseCmp(ArgStr[2],'PC')=0 THEN
        IF Memo('LDC') THEN WrError(1350)
        ELSE
         BEGIN
          DecodeAdr(ArgStr[1],MModGen+MModReg32+MModAReg32);
          IF AdrType=ModAReg32 THEN AdrMode:=4;
          IF (AdrType=ModGen) AND (AdrMode<6) THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$7c; BAsmCode[1]:=$c0+AdrMode;
            Move(AdrVals,BAsmCode[2],AdrCnt);
            CodeLen:=2+AdrCnt;
           END;
         END
       ELSE IF DecodeCReg(ArgStr[2],SMode) THEN
        BEGIN
         SetOpSize(1);
         IF Memo('LDC') THEN DecodeAdr(ArgStr[1],MModGen+MModImm)
         ELSE DecodeAdr(ArgStr[1],MModGen);
         IF AdrType=ModImm THEN
          BEGIN
           BAsmCode[0]:=$eb; BAsmCode[1]:=SMode SHL 4;
           Move(AdrVals,BAsmCode[2],AdrCnt);
           CodeLen:=2+AdrCnt;
          END
         ELSE IF AdrType=ModGen THEN
          BEGIN
           BAsmCode[0]:=$7a+z; BAsmCode[1]:=$80+(SMode SHL 4)+AdrMode;
           Move(AdrVals,BAsmCode[2],AdrCnt);
           CodeLen:=2+AdrCnt;
          END;
        END;
      END;
     Exit;
    END;

   IF (Memo('LDCTX')) OR (Memo('STCTX')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModGen);
       IF AdrType=ModGen THEN
        IF AdrMode<>15 THEN WrError(1350)
        ELSE
         BEGIN
          Move(AdrVals,BAsmCode[2],AdrCnt);
          DecodeAdr(ArgStr[2],MModAbs20);
          IF AdrType=ModAbs20 THEN
           BEGIN
            Move(AdrVals,BAsmCode[4],AdrCnt);
            BAsmCode[0]:=$7c+Ord(Memo('STCTX'));
            BAsmCode[1]:=$f0;
            CodeLen:=7;
           END;
         END;
      END;
     Exit;
    END;

   IF (Memo('LDE')) OR (Memo('STE')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       IF Memo('LDE') THEN
        BEGIN
         ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
         z:=1;
        END
       ELSE z:=0;
       DecodeAdr(ArgStr[1],MModGen);
       IF AdrType<>ModNone THEN
        IF OpSize=-1 THEN WrError(1132)
        ELSE IF OpSize>1 THEN WrError(1130)
        ELSE
         BEGIN
          CopyAdr; DecodeAdr(ArgStr[2],MModAbs20+MModDisp20+MModIReg32);
          IF AdrType<>ModNone THEN
           BEGIN
            BAsmCode[0]:=$74+OpSize;
            BAsmCode[1]:=(z SHL 7)+AdrMode2;
            CASE AdrType OF
            ModDisp20:Inc(BAsmCode[1],$10);
            ModIReg32:Inc(BAsmCode[1],$20);
            END;
            Move(AdrVals2,BAsmCode[2],AdrCnt2);
            Move(AdrVals,BAsmCode[2+AdrCnt2],AdrCnt);
            CodeLen:=2+AdrCnt2+AdrCnt;
           END;
         END;
      END;
     Exit;
    END;

   IF Memo('MOVA') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModGen);
       IF AdrType<>ModNone THEN
        IF AdrMode<8 THEN WrError(1350)
        ELSE
         BEGIN
          CopyAdr; DecodeAdr(ArgStr[2],MModGen);
          IF AdrType<>ModNone THEN
           IF AdrMode>5 THEN WrError(1350)
           ELSE
            BEGIN
             BAsmCode[0]:=$eb;
             BAsmCode[1]:=(AdrMode SHL 4)+AdrMode2;
             Move(AdrVals2,BAsmCode[2],AdrCnt2);
             CodeLen:=2+AdrCnt2;
            END;
         END;
      END;
     Exit;
    END;

   FOR z:=0 TO DirOrderCnt-1 DO
    IF Memo(DirOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF (OpSize>0) THEN WrError(1130)
      ELSE IF CheckFormat('G') THEN
       BEGIN
        OK:=True;
        IF NLS_StrCaseCmp(ArgStr[2],'R0L')=0 THEN Num1:=0
        ELSE IF NLS_StrCaseCmp(ArgStr[1],'R0L')=0 THEN Num1:=1
        ELSE OK:=False;
        IF NOT OK THEN WrError(1350)
        ELSE
         BEGIN
          DecodeAdr(ArgStr[Num1+1],MModGen);
          IF AdrType<>ModNone THEN
           IF (AdrMode AND 14=4) OR ((AdrMode=0) AND (Num1=1)) THEN WrError(1350)
           ELSE
            BEGIN
             BAsmCode[0]:=$7c; BAsmCode[1]:=(Num1 SHL 7)+(z SHL 4)+AdrMode;
             Move(AdrVals,BAsmCode[2],AdrCnt);
             CodeLen:=2+AdrCnt;
            END;
         END;
       END;
      Exit;
     END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('GS') THEN
      BEGIN
       z:=Ord(Memo('POP'));
       IF Memo('PUSH') THEN DecodeAdr(ArgStr[1],MModImm+MModGen)
       ELSE DecodeAdr(ArgStr[1],MModGen);
       IF AdrType<>ModNone THEN
        IF OpSize=-1 THEN WrError(1132)
        ELSE IF OpSize>1 THEN WrError(1130)
        ELSE
         BEGIN
          IF FormatCode=0 THEN
           IF (AdrType<>ModGen) THEN FormatCode:=1
           ELSE IF (OpSize=0) AND (AdrMode<2) THEN FormatCode:=2
           ELSE IF (OpSize=1) AND (AdrMode AND 14=4) THEN FormatCode:=2
           ELSE FormatCode:=1;
          CASE FormatCode OF
          1:BEGIN
	     IF AdrType=ModImm THEN
              BEGIN
               BAsmCode[0]:=$7c+OpSize;
               BAsmCode[1]:=$e2;
              END
             ELSE
              BEGIN
               BasmCode[0]:=$74+OpSize;
               BAsmCode[1]:=$40+(z*$90)+AdrMode;
              END;
             Move(AdrVals,BAsmCode[2],AdrCnt);
             CodeLen:=2+AdrCnt;
            END;
          2:IF (AdrType<>ModGen) THEN WrError(1350)
            ELSE IF (OpSize=0) AND (AdrMode<2) THEN
             BEGIN
              BAsmCode[0]:=$82+(AdrMode SHL 3)+(z SHL 4);
              CodeLen:=1;
             END
            ELSE IF (OpSize=1) AND (AdrMode AND 14=4) THEN
             BEGIN
              BAsmCode[0]:=$c2+((AdrMode AND 1) SHL 3)+(z SHL 4);
              CodeLen:=1;
             END
            ELSE WrError(1350);
          END;
         END;
      END;
     Exit;
    END;

   IF (Memo('PUSHC')) OR (Memo('POPC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF DecodeCReg(ArgStr[1],SMode) THEN
       BEGIN
        BAsmCode[0]:=$eb;
        BAsmCode[1]:=$02+Ord(Memo('POPC'))+(SMode SHL 4);
        CodeLen:=2;
       END;
     Exit;
    END;

   IF (Memo('PUSHM')) OR (Memo('POPM')) THEN
    BEGIN
     IF ArgCnt<1 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[1]:=0; OK:=True; z:=1;
       WHILE (OK) AND (z<=ArgCnt) DO
        BEGIN
         OK:=DecodeReg(ArgStr[z],SMode);
         IF OK THEN
          BEGIN
           IF Memo('POPM') THEN BAsmCode[1]:=BAsmCode[1] OR (1 SHL SMode)
           ELSE BAsmCode[1]:=BAsmCode[1] OR (1 SHL (7-SMode));
           Inc(z);
          END;
        END;
       IF NOT OK THEN WrXError(1440,ArgStr[z])
       ELSE
        BEGIN
         BAsmCode[0]:=$ec+Ord(Memo('POPM'));
         CodeLen:=2;
        END;
      END;
     Exit;
    END;

   IF Memo('PUSHA') THEN
    BEGIN
     If ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModGen);
       IF AdrType<>ModNone THEN
        IF AdrMode<8 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$7d;
          BAsmCode[1]:=$90+AdrMode;
          Move(AdrVals,BAsmCode[2],AdrCnt);
          CodeLen:=2+AdrCnt;
         END;
      END;
     Exit;
    END;

   IF Memo('XCHG') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModGen);
       IF AdrType<>ModNone THEN
        BEGIN
         CopyAdr; DecodeAdr(ArgStr[2],MModGen);
         IF AdrType<>ModNone THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF OpSize>1 THEN WrError(1130)
          ELSE IF AdrMode<4 THEN
           BEGIN
            BAsmCode[0]:=$7a+OpSize;
            BAsmCode[1]:=(AdrMode SHL 4)+AdrMode2;
            Move(AdrVals2,BAsmCode[2],AdrCnt2);
            CodeLen:=2+AdrCnt2;
           END
          ELSE IF AdrMode2<4 THEN
           BEGIN
            BAsmCode[0]:=$7a+OpSize;
            BAsmCode[1]:=(AdrMode2 SHL 4)+AdrMode;
            Move(AdrVals,BAsmCode[2],AdrCnt);
            CodeLen:=2+AdrCnt;
           END
          ELSE WrError(1350);
        END;
      END;
     Exit;
    END;

   IF (Memo('STZ')) OR (Memo('STNZ')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       IF OpSize=-1 THEN Inc(OpSize);
       DecodeAdr(ArgStr[2],MModGen);
       IF AdrType<>ModNone THEN
        IF NOT IsShort(AdrMode,SMode) THEN WrError(1350)
        ELSE
         BEGIN
          CopyAdr; DecodeAdr(ArgStr[1],MModImm);
          IF AdrType<>ModNone THEN
           BEGIN
            BAsmCode[0]:=$c8+(Ord(Memo('STNZ')) SHL 3)+SMode;
            BAsmCode[1]:=AdrVals[0];
            Move(AdrVals2,BAsmCode[2],AdrCnt2);
            CodeLen:=2+AdrCnt2;
           END;
         END;
      END;
     Exit;
    END;

   IF (Memo('STZX')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       IF OpSize=-1 THEN Inc(OpSize);
       DecodeAdr(ArgStr[3],MModGen);
       IF AdrType<>ModNone THEN
        IF NOT IsShort(AdrMode,SMode) THEN WrError(1350)
        ELSE
         BEGIN
          CopyAdr; DecodeAdr(ArgStr[1],MModImm);
          IF AdrType<>ModNone THEN
           BEGIN
            Num1:=AdrVals[0]; DecodeAdr(ArgStr[2],MModImm);
            IF AdrType<>ModNone THEN
             BEGIN
              BAsmCode[0]:=$d8+SMode;
              BAsmCode[1]:=Num1;
              Move(AdrVals2,BAsmCode[2],AdrCnt2);
              BAsmCode[2+AdrCnt2]:=AdrVals[0];
              CodeLen:=3+AdrCnt2;
             END;
           END;
         END;
      END;
     Exit;
    END;

   { Arithmetik }

   IF Memo('ADD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'SP')=0 THEN
      BEGIN
       IF OpSize=-1 THEN OpSize:=1;
       IF CheckFormat('GQ') THEN
        BEGIN
         DecodeAdr(ArgStr[1],MModImm);
         IF AdrType<>ModNone THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF OpSize>1 THEN WrError(1130)
          ELSE
           BEGIN
            AdrLong:=ImmVal;
            IF FormatCode=0 THEN
             IF (AdrLong>=-8) AND (AdrLong<=7) THEN FormatCode:=2
             ELSE FormatCode:=1;
            CASE FormatCode OF
            1:BEGIN
               BAsmCode[0]:=$7c+OpSize;
               BAsmCode[1]:=$eb;
               Move(AdrVals,BAsmCode[2],AdrCnt);
               CodeLen:=2+AdrCnt;
              END;
            2:IF ChkRange(AdrLong,-8,7) THEN
               BEGIN
                BAsmCode[0]:=$7d;
                BAsmCode[1]:=$b0+(AdrLong AND 15);
                CodeLen:=2;
               END;
            END;
           END;
        END;
      END
     ELSE IF CheckFormat('GQS') THEN
      BEGIN
       DecodeAdr(ArgStr[2],MModGen);
       IF AdrType<>ModNone THEN
        BEGIN
         CopyAdr;
         DecodeAdr(ArgStr[1],MModImm+MModGen);
         IF AdrType<>ModNone THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE
           BEGIN
            IF FormatCode=0 THEN
             IF AdrType=ModImm THEN
              IF (ImmVal>=-8) AND (ImmVal<=7) THEN FormatCode:=2
              ELSE IF (IsShort(AdrMode2,SMode)) AND (OpSize=0) THEN FormatCode:=3
	      ELSE FormatCode:=1
             ELSE
              IF (OpSize=0) AND (IsShort(AdrMode,SMode)) AND (AdrMode2<=1) AND
	         ((AdrMode>1) OR (Odd(AdrMode XOR AdrMode2))) THEN FormatCode:=3
	      ELSE FormatCode:=1;
	    CASE FormatCode OF
            1:CodeGen($a0,$76,$40);
            2:IF AdrType<>ModImm THEN WrError(1350)
              ELSE
               BEGIN
                Num1:=ImmVal;
                IF ChkRange(Num1,-8,7) THEN
                 BEGIN
                  BAsmCode[0]:=$c8+OpSize;
                  BAsmCode[1]:=(Num1 SHL 4)+AdrMode2;
                  Move(AdrVals2,BAsmCode[2],AdrCnt2);
                  CodeLen:=2+AdrCnt2;
                 END;
               END;
            3:IF OpSize<>0 THEN WrError(1130)
              ELSE IF NOT IsShort(AdrMode2,SMode) THEN WrError(1350)
              ELSE IF AdrType=ModImm THEN
               BEGIN
                BAsmCode[0]:=$80+SMode; BAsmCode[1]:=AdrVals[0];
                Move(AdrVals2,BAsmCode[2],AdrCnt2);
                CodeLen:=2+AdrCnt2;
               END
              ELSE IF (AdrMode2>=2) OR (NOT IsShort(AdrMode,SMode)) THEN WrError(1350)
              ELSE IF (AdrMode<2) AND (NOT Odd(AdrMode XOR AdrMode2)) THEN WrError(1350)
              ELSE
               BEGIN
                IF SMode=3 THEN Inc(SMode);
                BAsmCode[0]:=$20+((AdrMode2 AND 1) SHL 3)+(SMode AND 3);
                Move(AdrVals,BAsmCode[1],AdrCnt);
                CodeLen:=1+AdrCnt;
               END;
	    END;
           END;
        END;
      END;
     Exit;
    END;

   IF Memo('CMP') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GQS') THEN
      BEGIN
       DecodeAdr(ArgStr[2],MModGen);
       IF AdrType<>ModNone THEN
        BEGIN
         CopyAdr; DecodeAdr(ArgStr[1],MModImm+MModGen);
         IF AdrType<>ModNone THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE
           BEGIN
            IF FormatCode=0 THEN
             IF AdrType=ModImm THEN
              IF (ImmVal>=-8) AND (ImmVal<=7) THEN FormatCode:=2
              ELSE IF (IsShort(AdrMode2,SMode)) AND (OpSize=0) THEN FormatCode:=3
	      ELSE FormatCode:=1
             ELSE
              IF (OpSize=0) AND (IsShort(AdrMode,SMode)) AND (AdrMode2<=1) AND
	         ((AdrMode>1) OR (Odd(AdrMode XOR AdrMode2))) THEN FormatCode:=3
	      ELSE FormatCode:=1;
	    CASE FormatCode OF
            1:CodeGen($c0,$76,$80);
            2:IF AdrType<>ModImm THEN WrError(1350)
              ELSE
               BEGIN
                Num1:=ImmVal;
                IF ChkRange(Num1,-8,7) THEN
                 BEGIN
                  BAsmCode[0]:=$d0+OpSize;
                  BAsmCode[1]:=(Num1 SHL 4)+AdrMode2;
                  Move(AdrVals2,BAsmCode[2],AdrCnt2);
                  CodeLen:=2+AdrCnt2;
                 END;
               END;
            3:IF OpSize<>0 THEN WrError(1130)
              ELSE IF NOT IsShort(AdrMode2,SMode) THEN WrError(1350)
              ELSE IF AdrType=ModImm THEN
               BEGIN
                BAsmCode[0]:=$e0+SMode; BAsmCode[1]:=AdrVals[0];
                Move(AdrVals2,BAsmCode[2],AdrCnt2);
                CodeLen:=2+AdrCnt2;
               END
              ELSE IF (AdrMode2>=2) OR (NOT IsShort(AdrMode,SMode)) THEN WrError(1350)
              ELSE IF (AdrMode<2) AND (NOT Odd(AdrMode XOR AdrMode2)) THEN WrError(1350)
              ELSE
               BEGIN
                IF SMode=3 THEN Inc(SMode);
                BAsmCode[0]:=$38+((AdrMode2 AND 1) SHL 3)+(SMode AND 3);
                Move(AdrVals,BAsmCode[1],AdrCnt);
                CodeLen:=1+AdrCnt;
               END;
	    END;
           END;
        END;
      END;
     Exit;
    END;

   IF Memo('SUB') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GQS') THEN
      BEGIN
       DecodeAdr(ArgStr[2],MModGen);
       IF AdrType<>ModNone THEN
        BEGIN
         CopyAdr; DecodeAdr(ArgStr[1],MModImm+MModGen);
         IF AdrType<>ModNone THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE
           BEGIN
            IF FormatCode=0 THEN
             IF AdrType=ModImm THEN
              IF (ImmVal>=-7) AND (ImmVal<=8) THEN FormatCode:=2
              ELSE IF (IsShort(AdrMode2,SMode)) AND (OpSize=0) THEN FormatCode:=3
	      ELSE FormatCode:=1
             ELSE
              IF (OpSize=0) AND (IsShort(AdrMode,SMode)) AND (AdrMode2<=1) AND
	         ((AdrMode>1) OR (Odd(AdrMode XOR AdrMode2))) THEN FormatCode:=3
	      ELSE FormatCode:=1;
	    CASE FormatCode OF
            1:CodeGen($a8,$76,$50);
            2:IF AdrType<>ModImm THEN WrError(1350)
              ELSE
               BEGIN
                Num1:=ImmVal;
                IF ChkRange(Num1,-7,8) THEN
                 BEGIN
                  BAsmCode[0]:=$d0+OpSize;
                  BAsmCode[1]:=((-Num1) SHL 4)+AdrMode2;
                  Move(AdrVals2,BAsmCode[2],AdrCnt2);
                  CodeLen:=2+AdrCnt2;
                 END;
               END;
            3:IF OpSize<>0 THEN WrError(1130)
              ELSE IF NOT IsShort(AdrMode2,SMode) THEN WrError(1350)
              ELSE IF AdrType=ModImm THEN
               BEGIN
                BAsmCode[0]:=$88+SMode; BAsmCode[1]:=AdrVals[0];
                Move(AdrVals2,BAsmCode[2],AdrCnt2);
                CodeLen:=2+AdrCnt2;
               END
              ELSE IF (AdrMode2>=2) OR (NOT IsShort(AdrMode,SMode)) THEN WrError(1350)
              ELSE IF (AdrMode<2) AND (NOT Odd(AdrMode XOR AdrMode2)) THEN WrError(1350)
              ELSE
               BEGIN
                IF SMode=3 THEN Inc(SMode);
                BAsmCode[0]:=$28+((AdrMode2 AND 1) SHL 3)+(SMode AND 3);
                Move(AdrVals,BAsmCode[1],AdrCnt);
                CodeLen:=1+AdrCnt;
               END;
	    END;
           END;
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO Gen1OrderCnt-1 DO
    WITH Gen1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF CheckFormat('G') THEN
        BEGIN
         DecodeAdr(ArgStr[1],MModGen);
         IF AdrType<>ModNone THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE
           BEGIN
            BAsmCode[0]:=Hi(Code)+OpSize;
            BAsmCode[1]:=Lo(Code)+AdrMode;
            Move(AdrVals,BAsmCode[2],AdrCnt);
            CodeLen:=2+AdrCnt;
           END;
        END;
       Exit;
      END;

   FOR z:=0 TO Gen2OrderCnt-1 DO
    WITH Gen2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF CheckFormat('G') THEN
        BEGIN
         DecodeAdr(ArgStr[2],MModGen);
         IF AdrType<>ModNone THEN
          BEGIN
           CopyAdr; DecodeAdr(ArgStr[1],MModGen+MModImm);
           IF AdrType<>ModNone THEN
            IF OpSize=-1 THEN WrError(1132)
            ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
            ELSE IF (OpPart[1]='M') AND ((AdrMode2=3) OR (AdrMode2=5) OR (AdrMode2-OpSize=1)) THEN WrError(1350)
            ELSE CodeGen(Code1,Code2,Code3);
          END;
	END;
       Exit;
      END;

   IF (Memo('INC')) OR (Memo('DEC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModGen);
       IF AdrType<>ModNone THEN
        IF OpSize=-1 THEN WrError(1132)
        ELSE IF (OpSize=1) AND (AdrMode AND 14=4) THEN
         BEGIN
          BAsmCode[0]:=$b2+(Ord(Memo('DEC')) SHL 6)+((AdrMode ANd 1) SHL 3);
	  CodeLen:=1;
         END
        ELSE IF NOT IsShort(AdrMode,SMode) THEN WrError(1350)
        ELSE IF OpSize<>0 THEN WrError(1130)
        ELSE
         BEGIN
          BAsmCode[0]:=$a0+(Ord(Memo('DEC')) SHL 3)+SMode;
          Move(AdrVals,BAsmCode[1],AdrCnt);
          CodeLen:=1+AdrCnt;
         END;
      END;
     Exit;
    END;

   FOR z:=0 TO DivOrderCnt-1 DO
    WITH DivOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF CheckFormat('G') THEN
        BEGIN
         DecodeAdr(ArgStr[1],MModImm+MModGen);
         IF AdrType<>ModNone THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE IF AdrType=ModImm THEN
           BEGIN
            BAsmCode[0]:=$7c+OpSize;
            BAsmCode[1]:=Code1;
            Move(AdrVals,BAsmCode[2],AdrCnt);
            CodeLen:=2+AdrCnt;
           END
          ELSE
           BEGIN
            BAsmCode[0]:=Code2+OpSize;
            BAsmCode[1]:=Code3+AdrMode;
            Move(AdrVals,BAsmCode[2],AdrCnt);
            CodeLen:=2+AdrCnt;
           END;
        END;
       Exit;
      END;

   FOR z:=0 TO BCDOrderCnt-1 DO
    IF Memo(BCDOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[2],MModGen);
        IF AdrType<>ModNone THEN
         IF AdrMode<>0 THEN WrError(1350)
         ELSE
          BEGIN
           DecodeAdr(ArgStr[1],MModGen+MModImm);
           IF AdrType<>ModNone THEN
            IF AdrType=ModImm THEN
             BEGIN
              BAsmCode[0]:=$7c+OpSize;
              BAsmCode[1]:=$ec+z;
              Move(AdrVals,BAsmCode[2],AdrCnt);
              CodeLen:=2+AdrCnt;
             END
            ELSE IF AdrMode<>1 THEN WrError(1350)
            ELSE
             BEGIN
              BAsmCode[0]:=$7c+OpSize;
              BAsmCode[1]:=$e4+z;
              CodeLen:=2;
             END;
          END;
       END;
      Exit;
     END;

   IF Memo('EXTS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModGen);
       IF OpSize=-1 THEN OpSize:=0;
       IF AdrType<>ModNone THEN
        IF OpSize=0 THEN
         IF (AdrMode=1) OR ((AdrMode>=3) AND (AdrMode<=5)) THEN WrError(1350)
         ELSE
          BEGIN
           BAsmCode[0]:=$7c;
           BAsmCode[1]:=$60+AdrMode;
           Move(AdrVals,BAsmCode[2],AdrCnt);
           CodeLen:=2+AdrCnt;
          END
	ELSE IF OpSize=1 THEN
         IF AdrMode<>0 THEN WrError(1350)
         ELSE
          BEGIN
           BAsmCode[0]:=$7c; BAsmCode[1]:=$f3;
           CodeLen:=2;
          END
	ELSE WrError(1130);
      END;
     Exit;
    END;

   IF Memo('NOT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('GS') THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModGen);
       IF AdrType<>ModNone THEN
        IF OpSize=-1 THEN WrError(1132)
        ELSE IF (OpSize>1) THEN WrError(1130)
        ELSE
         BEGIN
          IF FormatCode=0 THEN
           IF (OpSize=0) AND (IsShort(AdrMode,SMode)) THEN FormatCode:=2
           ELSE FormatCode:=1;
          CASE FormatCode OF
          1:BEGIN
             BAsmCode[0]:=$74+OpSize;
             BAsmCode[1]:=$70+AdrMode;
             Move(AdrVals,BAsmCode[2],AdrCnt);
             CodeLen:=2+AdrCnt;
            END;
          2:IF OpSize<>0 THEN WrError(1130)
            ELSE IF NOT IsShort(AdrMode,SMode) THEN WrError(1350)
            ELSE
             BEGIN
              BAsmCode[0]:=$b8+SMode;
              Move(AdrVals,BAsmCode[1],AdrCnt);
              CodeLen:=1+AdrCnt;
             END;
          END;
         END;
      END;
     Exit;
    END;

   { Logik }

   IF ((Memo('AND')) OR (Memo('OR'))) THEN
    BEGIN
     z:=Ord(Memo('OR'));
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GQ') THEN
      BEGIN
       DecodeAdr(ArgStr[2],MModGen);
       IF AdrType<>ModNone THEN
        BEGIN
         CopyAdr; DecodeAdr(ArgStr[1],MModGen+MModImm);
         IF AdrType<>ModNone THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF OpSize>1 THEN WrError(1130)
          ELSE
           BEGIN
            IF FormatCode=0 THEN
             IF AdrType=ModImm THEN
              IF (OpSize=0) AND (IsShort(AdrMode2,SMode)) THEN FormatCode:=2
              ELSE FormatCode:=1
             ELSE
              IF (AdrMode2<=1) AND (IsShort(AdrMode,SMode)) AND ((AdrMode>1) OR Odd(AdrMode XOR AdrMode2)) THEN FormatCode:=2
              ELSE FormatCode:=1;
            CASE FormatCode OF
            1:CodeGen($90+(z SHL 3),$76,$20+(z SHL 4));
            2:IF OpSize<>0 THEN WrError(1130)
              ELSE IF AdrType=ModImm THEN
               IF NOT IsShort(AdrMode2,SMode) THEN WrError(1350)
               ELSE
                BEGIN
                 BAsmCode[0]:=$90+(z SHL 3)+SMode;
                 BAsmCode[1]:=ImmVal;
                 Move(AdrVals2,BAsmCode[2],AdrCnt2);
                 CodeLen:=2+AdrCnt2;
                END
              ELSE IF (NOT IsShort(AdrMode,SMode)) OR (AdrMode2>1) THEN WrError(1350)
              ELSE IF (AdrMode<=1) AND (NOT Odd(AdrMode XOR AdrMode2)) THEN WrError(1350)
              ELSE
               BEGIN
                IF SMode=3 THEN Inc(SMode);
                BAsmCode[0]:=$10+(z SHL 3)+((AdrMode2 AND 1) SHL 2)+(SMode AND 3);
                Move(AdrVals,BAsmCode[1],AdrCnt);
                CodeLen:=1+AdrCnt;
               END;
            END;
           END;
        END;
      END;
     Exit;
    END;

   IF Memo('ROT') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       DecodeAdr(ArgStr[2],MModGen);
       IF AdrType<>ModNone THEN
        IF OpSize=-1 THEN WrError(1132)
        ELSE IF OpSize>1 THEN WrError(1130)
        ELSE
         BEGIN
          OpSize2:=OpSize; OpSize:=0; CopyAdr;
          DecodeAdr(ArgStr[1],MModGen+MModImm);
          IF AdrType=ModGen THEN
           IF AdrMode<>3 THEN WrError(1350)
           ELSE IF AdrMode2+2*OpSize2=3 THEN WrError(1350)
           ELSE
            BEGIN
             BAsmCode[0]:=$74+OpSize2;
             BAsmCode[1]:=$60+AdrMode2;
             Move(AdrVals2,BAsmCode[2],AdrCnt2);
             CodeLen:=2+AdrCnt2;
            END
          ELSE IF AdrType=ModImm THEN
           BEGIN
            Num1:=ImmVal;
            IF Num1=0 THEN WrError(1315)
            ELSE IF ChkRange(Num1,-8,8) THEN
             BEGIN
              IF Num1>0 THEN Dec(Num1) ELSE Num1:=-9-Num1;
              BAsmCode[0]:=$e0+OpSize2;
              BAsmCode[1]:=(Num1 SHL 4)+AdrMode2;
              Move(AdrVals2,BAsmCode[2],AdrCnt);
              CodeLen:=2+AdrCnt2;
             END;
           END;
         END;
      END;
     Exit;
    END;

   IF (Memo('SHA')) OR (Memo('SHL')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       z:=Ord(Memo('SHA'));
       DecodeAdr(ArgStr[2],MModGen+MModReg32);
       IF AdrType<>ModNone THEN
        IF OpSize=-1 THEN WrError(1132)
        ELSE IF (OpSize>2) OR ((OpSize=2) AND (AdrType=ModGen)) THEN WrError(1130)
        ELSE
         BEGIN
          CopyAdr; OpSize2:=OpSize; OpSize:=0;
          DecodeAdr(ArgStr[1],MModImm+MModGen);
          IF AdrType=ModGen THEN
           IF AdrMode<>3 THEN WrError(1350)
           ELSE IF AdrMode2*2+OpSize2=3 THEN WrError(1350)
           ELSE
            BEGIN
             IF OpSize2=2 THEN
              BEGIN
               BAsmCode[0]:=$eb;
               BAsmCode[1]:=$01+(AdrMode2 SHL 4)+(z SHL 5);
              END
             ELSE
              BEGIN
               BAsmCode[0]:=$74+OpSize2;
               BAsmCode[1]:=$e0+(z SHL 4)+AdrMode2;
              END;
             Move(AdrVals2,BAsmCode[2],AdrCnt2);
             CodeLen:=2+AdrCnt2;
            END
          ELSE IF AdrType=ModImm THEN
           BEGIN
            Num1:=ImmVal;
            IF Num1=0 THEN WrError(1315)
            ELSE IF ChkRange(Num1,-8,8) THEN
             BEGIN
              IF Num1>0 THEN Dec(Num1) ELSE Num1:=-9-Num1;
              IF OpSize2=2 THEN
               BEGIN
                BAsmCode[0]:=$eb;
                BAsmCode[1]:=$80+(AdrMode2 SHL 4)+(z SHL 5)+(Num1 AND 15);
               END
              ELSE
               BEGIN
                BAsmCode[0]:=$e8+(z SHL 3)+OpSize2;
                BAsmCode[1]:=(Num1 SHl 4)+AdrMode2;
               END;
              Move(AdrVals2,BAsmCode[2],AdrCnt2);
              CodeLen:=2+AdrCnt2;
             END;
           END;
         END;
      END;
     Exit;
    END;

   { Bitoperationen }

   FOR z:=0 TO BitOrderCnt-1 DO
    WITH BitOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       MayShort:=Code AND 12=8;
       IF MayShort THEN OK:=CheckFormat('GS') ELSE OK:=CheckFormat('G');
       IF OK THEN
        IF DecodeBitAdr((FormatCode<>1) AND (MayShort)) THEN
         IF AdrMode>=16 THEN
          BEGIN
           BAsmCode[0]:=$40+((Code-8) SHL 3)+(AdrMode AND 7);
           BAsmCode[1]:=AdrVals[0];
           CodeLen:=2;
          END
         ELSE
          BEGIN
           BAsmCode[0]:=$7e;
           BAsmCode[1]:=(Code SHL 4)+AdrMode;
           Move(AdrVals,BAsmCode[2],AdrCnt);
           CodeLen:=2+AdrCnt;
          END;
       Exit;
      END;

   FOR z:=0 TO ConditionCnt-1 DO
    WITH Conditions^[z] DO
     IF Memo('BM'+Name) THEN
      BEGIN
       IF (ArgCnt=1) AND (NLS_StrCaseCmp(ArgStr[1],'C')=0) THEN
        BEGIN
         BAsmCode[0]:=$7d; BAsmCode[1]:=$d0+Code;
	 CodeLen:=2;
        END
       ELSE IF DecodeBitAdr(False) THEN
        BEGIN
         BAsmCode[0]:=$7e;
         BAsmCode[1]:=$20+AdrMode;
         Move(AdrVals,BAsmCode[2],AdrCnt); z:=Code;
         IF (z>=4) AND (z<12) THEN z:=z XOR 12;
	 IF z>=8 THEN Inc(z,$f0);
         BAsmCode[2+AdrCnt]:=z;
         CodeLen:=3+AdrCnt;
        END;
       Exit;
      END;

   IF (Memo('FCLR')) OR (Memo('FSET')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF Length(ArgStr[1])<>1 THEN WrError(1350)
     ELSE
      BEGIN
       Num1:=Pos(UpCase(ArgStr[1][1]),'CDZSBOIU')-1;
       IF Num1=-1 THEN WrXError(1440,ArgStr[1])
       ELSE
        BEGIN
         BAsmCode[0]:=$eb;
         BAsmCode[1]:=$04+Ord(Memo('FCLR'))+(Num1 SHL 4);
         CodeLen:=2;
        END;
      END;
     Exit;
    END;

   { SprÅnge }

   IF (Memo('JMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt20,OK);
       Diff:=AdrLong-EProgCounter;
       IF OpSize=-1 THEN
        BEGIN
         IF (Diff>=2) AND (Diff<=9) THEN OpSize:=4
         ELSE IF (Diff>=-127) AND (Diff<=128) THEN OpSize:=0
         ELSE IF (Diff>=-32767) AND (Diff<=32768) THEN OpSize:=1
         ELSE OpSize:=7;
        END;
       CASE OpSize OF
       4:IF ((Diff<2) OR (Diff>9)) AND (NOT SymbolQuestionable) THEN WrError(1370)
         ELSE
          BEGIN
           BAsmCode[0]:=$60+((Diff-2) AND 7); CodeLen:=1;
          END;
       0:IF ((Diff<-127) OR (Diff>128)) AND (NOT SymbolQuestionable) THEN WrError(1370)
         ELSE
          BEGIN
           BAsmCode[0]:=$fe;
	   BAsmCode[1]:=(Diff-1) AND $ff;
	   CodeLen:=2;
          END;
       1:IF ((Diff<-32767) OR (Diff>32768)) AND (NOT SymbolQuestionable) THEN WrError(1370)
         ELSE
          BEGIN
           BAsmCode[0]:=$f4; Dec(Diff);
	   BAsmCode[1]:=Diff AND $ff;
           BAsmCode[2]:=(Diff SHR 8) AND $ff;
	   CodeLen:=3;
          END;
       7:BEGIN
          BAsmCode[0]:=$fc;
          BAsmCode[1]:=AdrLong AND $ff;
          BAsmCode[2]:=(AdrLong SHR 8) AND $ff;
          BAsmCode[3]:=(AdrLong SHR 16) AND $ff;
          CodeLen:=4;
         END;
       ELSE WrError(1130);
       END;
      END;
     Exit;
    END;

   IF (Memo('JSR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt20,OK);
       Diff:=AdrLong-EProgCounter;
       IF OpSize=-1 THEN
        BEGIN
         IF (Diff>=-32767) AND (Diff<=32768) THEN OpSize:=1
         ELSE OpSize:=7;
        END;
       CASE OpSize OF
       1:IF ((Diff<-32767) OR (Diff>32768)) AND (NOT SymbolQuestionable) THEN WrError(1370)
         ELSE
          BEGIN
           BAsmCode[0]:=$f5; Dec(Diff);
	   BAsmCode[1]:=Diff AND $ff;
           BAsmCode[2]:=(Diff SHR 8) AND $ff;
	   CodeLen:=3;
          END;
       7:BEGIN
          BAsmCode[0]:=$fd;
          BAsmCode[1]:=AdrLong AND $ff;
          BAsmCode[2]:=(AdrLong SHR 8) AND $ff;
          BAsmCode[3]:=(AdrLong SHR 16) AND $ff;
          CodeLen:=4;
         END;
       ELSE WrError(1130);
       END;
      END;
     Exit;
    END;

   IF (Memo('JMPI')) OR (Memo('JSRI')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       IF OpSize=7 THEN OpSize:=2;
       DecodeAdr(ArgStr[1],MModGen+MModDisp20+MModReg32+MModAReg32);
       IF (AdrType=ModGen) AND (AdrMode AND 14=12) THEN
        BEGIN
         AdrVals[AdrCnt]:=0; Inc(AdrCnt);
        END;
       IF (AdrType=ModGen) AND (AdrMode AND 14=4) THEN
        IF OpSize=-1 THEN OpSize:=1
        ELSE IF OpSize<>1 THEN
	 BEGIN
	  AdrType:=ModNone; WrError(1131);
	 END;
       IF AdrType=ModAReg32 THEN AdrMode:=4;
       IF AdrType<>ModNone THEN
        IF OpSize=-1 THEN WrError(1132)
        ELSE IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
        ELSE
         BEGIN
          BAsmCode[0]:=$7d;
          BAsmCode[1]:=(Ord(Memo('JSRI')) SHL 4)+(Ord(OpSize=1) SHL 5)+AdrMode;
          Move(AdrVals,BAsmCode[2],AdrCnt);
          CodeLen:=2+AdrCnt;
         END;
      END;
     Exit;
    END;

   IF (Memo('JMPS')) OR (Memo('JSRS')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=0;
       FirstPassUnknown:=False;
       DecodeAdr(ArgStr[1],MModImm);
       IF (FirstPassUnknown) AND (AdrVals[0]<18) THEN AdrVals[0]:=18;
       IF AdrType<>ModNone THEN
        IF AdrVals[0]<18 THEN WrError(1315)
        ELSE
         BEGIN
          BAsmCode[0]:=$ee+Ord(Memo('JSRS'));
          BAsmCode[1]:=AdrVals[0];
          CodeLen:=2;
         END;
      END;
     Exit;
    END;

   FOR z:=0 TO ConditionCnt-1 DO
    WITH Conditions^[z] DO
     IF Memo('J'+Name) THEN
      BEGIN
       Num1:=1+Ord(Code>=8);
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF (AttrPart<>'') THEN WrError(1100)
       ELSE IF (Format<>' ') THEN WrError(1090)
       ELSE
        BEGIN
         AdrLong:=EvalIntExpression(ArgStr[1],UInt20,OK)-(EProgCounter+Num1);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrLong>127) OR (AdrLong<-128)) THEN WrError(1370)
          ELSE IF Code>=8 THEN
           BEGIN
            BAsmCode[0]:=$7d; BAsmCode[1]:=$c0+Code;
            BAsmCode[2]:=AdrLong AND $ff;
            CodeLen:=3;
           END
          ELSE
           BEGIN
            BAsmCode[0]:=$68+Code; BAsmCode[1]:=AdrLong AND $ff;
            CodeLen:=2;
           END;
        END;
       Exit;
      END;

   IF (Memo('ADJNZ')) OR (Memo('SBJNZ')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       DecodeAdr(ArgStr[2],MModGen);
       IF AdrType<>ModNone THEN
        IF OpSize=-1 THEN WrError(1132)
        ELSE IF OpSize>1 THEN WrError(1130)
        ELSE
         BEGIN
          CopyAdr; OpSize2:=OpSize; OpSize:=0;
          FirstPassUnknown:=False;
	  DecodeAdr(ArgStr[1],MModImm); Num1:=ImmVal;
          IF FirstPassUnknown THEN Num1:=0;
          IF Memo('SBJNZ') THEN Num1:=-Num1;
          IF ChkRange(Num1,-8,7) THEN
           BEGIN
            AdrLong:=EvalIntExpression(ArgStr[3],UInt20,OK)-(EProgCounter+2);
            IF OK THEN
             IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
             ELSE
              BEGIN
               BAsmCode[0]:=$f8+OpSize2;
               BAsmCode[1]:=(Num1 SHL 4)+AdrMode2;
               Move(AdrVals2,BAsmCode[2],AdrCnt2);
               BAsmCode[2+AdrCnt2]:=AdrLong AND $ff;
               CodeLen:=3+AdrCnt2;
              END;
           END;
         END;
      END;
     Exit;
    END;

   IF Memo('INT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[1]:=$c0+EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1) ,UInt6,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$eb; CodeLen:=2;
        END;
      END;
     Exit;
    END;

   { Miszellaneen }

   IF Memo('ENTER') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[2]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1) ,UInt8,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$7c; BAsmCode[1]:=$f2; CodeLen:=3;
        END;
      END;
     Exit;
    END;

   IF Memo('LDINTB') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1) ,UInt20,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$eb; BAsmCode[1]:=$20;
         BAsmCode[2]:=(AdrLong SHR 16) AND $ff; BAsmCode[3]:=0;
         BAsmCode[4]:=$eb; BAsmCode[5]:=$10;
         BAsmCode[6]:=(AdrLong SHR 8) AND $ff; BAsmCode[7]:=AdrLong AND $ff;
         CodeLen:=8;
        END;
      END;
     Exit;
    END;

   IF Memo('LDIPL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[1]:=$a0+EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1) ,UInt3,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$7d; CodeLen:=2;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_M16C:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=EProgCounter<=$fffff;
   ELSE ok:=False;
   END;
   ChkPC_M16C:=ok AND (EProgCounter>=0);
END;


        FUNCTION IsDef_M16C:Boolean;
	Far;
BEGIN
   IsDef_M16C:=False;
END;

        PROCEDURE SwitchFrom_M16C;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_M16C;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$14; NOPCode:=$04;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.:';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_M16C; ChkPC:=ChkPC_M16C; IsDef:=IsDef_M16C;
   SwitchFrom:=SwitchFrom_M16C; InitFields;
END;

BEGIN
   CPUM16C:=AddCPU('M16C',SwitchTo_M16C);
   CPUM30600M8:=AddCPU('M30600M8',SwitchTo_M16C);
END.


