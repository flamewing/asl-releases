{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
        Unit CodeH8_5;

{ AS - Codegenerator H8/500 }

Interface

        Uses NLS,
             AsmDef,AsmPars,AsmSub,CodePseu;


Implementation

CONST
   FixedOrderCount=6;
   RelOrderCount=21;
   OneOrderCount=13;
   OneRegOrderCount=3;
   RegEAOrderCount=9;
   TwoRegOrderCount=3;
   LogOrderCount=3;
   BitOrderCount=4;

   ModNone=-1;
   ModReg=0;      MModReg=     1 SHL ModReg;
   ModIReg=1;     MModIReg=    1 SHL ModIReg;
   ModDisp8=2;    MModDisp8=   1 SHL ModDisp8;
   ModDisp16=3;   MModDisp16=  1 SHL ModDisp16;
   ModPredec=4;   MModPredec=  1 SHL ModPreDec;
   ModPostInc=5;  MModPostInc= 1 SHL ModPostInc;
   ModAbs8=6;     MModAbs8=    1 SHL ModAbs8;
   ModAbs16=7;    MModAbs16=   1 SHL ModAbs16;
   ModImm=8;      MModImm=     1 SHL ModImm;

   MModAll=MModReg+MModIReg+MModDisp8+MModDisp16+MModPredec
          +MModPostInc+MModAbs8+MModAbs16+MModImm;
   MModNoImm=MModAll-MModImm;

TYPE
   FixedOrder=RECORD
               Name:String[7];
               Code:Word;
              END;
   OneOrder=RECORD
             Name:String[5];
             Code:Word;
             SizeMask:Byte;
             DefSize:ShortInt;
            END;

   FixedOrderArray=ARRAY[1..FixedOrderCount] OF FixedOrder;
   RelOrderArray=ARRAY[1..RelOrderCount] OF FixedOrder;
   OneOrderArray=ARRAY[1..OneOrderCount] OF OneOrder;
   OneRegOrderArray=ARRAY[1..OneRegOrderCount] OF OneOrder;
   RegEAOrderArray=ARRAY[1..RegEAOrderCount] OF OneOrder;
   TwoRegOrderArray=ARRAY[1..OneRegOrderCount] OF OneOrder;
   LogOrderArray=ARRAY[1..LogOrderCount] OF FixedOrder;
   BitOrderArray=ARRAY[1..BitOrderCount] OF FixedOrder;

VAR
   CPU532,CPU534,CPU536,CPU538:CPUVar;
   SaveInitProc:PROCEDURE;

   OpSize:ShortInt;
   Format:String;
   AdrMode:ShortInt;
   AdrByte,AdrCnt,FormatCode:Byte;
   AdrVals:ARRAY[0..2] OF Byte;
   AbsBank:Byte;

   Reg_DP,Reg_EP,Reg_TP,Reg_BR:LongInt;

   FixedOrders:^FixedOrderArray;
   RelOrders:^RelOrderArray;
   OneOrders:^OneOrderArray;
   OneRegOrders:^OneRegOrderArray;
   RegEAOrders:^RegEAOrderArray;
   TwoRegOrders:^TwoRegOrderArray;
   LogOrders:^LogOrderArray;
   BitOrders:^BitOrderArray;

{---------------------------------------------------------------------------}
{ dynamische Belegung/Freigabe Codetabellen }

	PROCEDURE InitFields;
VAR
   z:Word;

   	PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>FixedOrderCount THEN Halt;
   WITH FixedOrders^[z] DO
    BEGIN
     Code:=NCode; Name:=NName;
    END;
END;

   	PROCEDURE AddRel(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>RelOrderCount THEN Halt;
   WITH RelOrders^[z] DO
    BEGIN
     Code:=NCode; Name:=NName;
    END;
END;

   	PROCEDURE AddOne(NName:String; NCode:Word; NMask:Byte; NDef:ShortInt);
BEGIN
   Inc(z); IF z>OneOrderCount THEN Halt;
   WITH OneOrders^[z] DO
    BEGIN
     Code:=NCode; Name:=NName;
     SizeMask:=NMask; DefSize:=NDef;
    END;
END;

   	PROCEDURE AddOneReg(NName:String; NCode:Word; NMask:Byte; NDef:ShortInt);
BEGIN
   Inc(z); IF z>OneRegOrderCount THEN Halt;
   WITH OneRegOrders^[z] DO
    BEGIN
     Code:=NCode; Name:=NName;
     SizeMask:=NMask; DefSize:=NDef;
    END;
END;

   	PROCEDURE AddRegEA(NName:String; NCode:Word; NMask:Byte; NDef:ShortInt);
BEGIN
   Inc(z); IF z>RegEAOrderCount THEN Halt;
   WITH RegEAOrders^[z] DO
    BEGIN
     Code:=NCode; Name:=NName;
     SizeMask:=NMask; DefSize:=NDef;
    END;
END;

   	PROCEDURE AddTwoReg(NName:String; NCode:Word; NMask:Byte; NDef:ShortInt);
BEGIN
   Inc(z); IF z>TwoRegOrderCount THEN Halt;
   WITH TwoRegOrders^[z] DO
    BEGIN
     Code:=NCode; Name:=NName;
     SizeMask:=NMask; DefSize:=NDef;
    END;
END;

   	PROCEDURE AddLog(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>LogOrderCount THEN Halt;
   WITH LogOrders^[z] DO
    BEGIN
     Code:=NCode; Name:=NName;
    END;
END;

   	PROCEDURE AddBit(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>BitOrderCount THEN Halt;
   WITH BitOrders^[z] DO
    BEGIN
     Code:=NCode; Name:=NName;
    END;
END;

BEGIN
   z:=0; New(FixedOrders);
   AddFixed('NOP'  ,$0000); AddFixed('PRTS'   ,$1119);
   AddFixed('RTE'  ,$000a); AddFixed('RTS'    ,$0019);
   AddFixed('SLEEP',$001a); AddFixed('TRAP/VS',$0009);

   z:=0; New(RelOrders);
   AddRel('BRA',$20); AddRel('BT' ,$20); AddRel('BRN',$21);
   AddRel('BF' ,$21); AddRel('BHI',$22); AddRel('BLS',$23);
   AddRel('BCC',$24); AddRel('BHS',$24); AddRel('BCS',$25);
   AddRel('BLO',$25); AddRel('BNE',$26); AddRel('BEQ',$27);
   AddRel('BVC',$28); AddRel('BVS',$29); AddRel('BPL',$2a);
   AddRel('BMI',$2b); AddRel('BGE',$2c); AddRel('BLT',$2d);
   AddRel('BGT',$2e); AddRel('BLE',$2f); AddRel('BSR',$0e);

   z:=0; New(OneOrders);
   AddOne('CLR'  ,$13,3,1); AddOne('NEG'  ,$14,3,1);
   AddOne('NOT'  ,$15,3,1); AddOne('ROTL' ,$1c,3,1);
   AddOne('ROTR' ,$1d,3,1); AddOne('ROTXL',$1e,3,1);
   AddOne('ROTXR',$1f,3,1); AddOne('SHAL' ,$18,3,1);
   AddOne('SHAR' ,$19,3,1); AddOne('SHLL' ,$1a,3,1);
   AddOne('SHLR' ,$1b,3,1); AddOne('TAS'  ,$17,1,0);
   AddOne('TST'  ,$16,3,1);

   z:=0; New(OneRegOrders);
   AddOneReg('EXTS',$11,1,0); AddOneReg('EXTU',$12,1,0);
   AddOneReg('SWAP',$10,1,0);

   z:=0; New(RegEAOrders);
   AddRegEA('ADDS' ,$28,3,1); AddRegEA('ADDX' ,$a0,3,1);
   AddRegEA('AND'  ,$50,3,1); AddRegEA('DIVXU',$b8,3,1);
   AddRegEA('MULXU',$a8,3,1); AddRegEA('OR'   ,$40,3,1);
   AddRegEA('SUBS' ,$38,3,1); AddRegEA('SUBX' ,$b0,3,1);
   AddRegEA('XOR'  ,$60,3,1);

   z:=0; New(TwoRegOrders);
   AddTwoReg('DADD',$a000,1,0); AddTwoReg('DSUB',$b000,1,0);
   AddTwoReg('XCH' ,$90,2,1);

   z:=0; New(LogOrders);
   AddLog('ANDC',$58); AddLog('ORC',$48); AddLog('XORC',$68);

   z:=0; New(BitOrders);
   AddBit('BCLR',$50); AddBit('BNOT',$60);
   AddBit('BSET',$40); AddBit('BTST',$70);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(RelOrders);
   Dispose(OneOrders);
   Dispose(OneRegOrders);
   Dispose(RegEAOrders);
   Dispose(TwoRegOrders);
   Dispose(LogOrders);
   Dispose(BitOrders);
END;

{---------------------------------------------------------------------------}
{ Adre·parsing }

        PROCEDURE SetOpSize(NSize:ShortInt);
BEGIN
   IF OpSize=-1 THEN OpSize:=NSize
   ELSE IF OpSize<>NSize THEN WrError(1132);
END;

        FUNCTION DecodeReg(Asc:String; VAR Erg:Byte):Boolean;
BEGIN
   DecodeReg:=True;
   IF NLS_StrCaseCmp(Asc,'SP')=0 THEN Erg:=7
   ELSE IF NLS_StrCaseCmp(Asc,'FP')=0 THEN Erg:=6
   ELSE IF (Length(Asc)=2) AND (UpCase(Asc[1])='R') AND (Asc[2]>='0') AND (Asc[2]<='7') THEN
    Erg:=Ord(Asc[2])-AscOfs
   ELSE DecodeReg:=False;
END;

	FUNCTION DecodeRegList(Asc:String; VAR Erg:Byte):Boolean;
VAR
   Part:String;
   Reg1,Reg2,z:Byte;
   p:Integer;
BEGIN
   DecodeRegList:=False;

   IF IsIndirect(Asc) THEN Asc:=Copy(Asc,2,Length(Asc)-2);
   KillBlanks(Asc);

   Erg:=0;
   WHILE Asc<>'' DO
    BEGIN
     SplitString(Asc,Part,Asc,QuotPos(Asc,','));
     IF DecodeReg(Part,Reg1) THEN Erg:=Erg OR (1 SHL Reg1)
     ELSE
      BEGIN
       p:=Pos('-',Part);
       IF p=0 THEN Exit;
       IF NOT DecodeReg(Copy(Part,1,p-1),Reg1) THEN Exit;
       IF NOT DecodeReg(Copy(Part,p+1,Length(Part)-p),Reg2) THEN Exit;
       IF Reg1>Reg2 THEN Inc(Reg2,8);
       FOR z:=Reg1 TO Reg2 DO Erg:=Erg OR (1 SHL (z AND 7));
      END;
    END;

   DecodeRegList:=True;
END;

	FUNCTION DecodeCReg(VAR Asc:String; VAR Erg:Byte):Boolean;
BEGIN
   DecodeCReg:=True;
   IF NLS_StrCaseCmp(Asc,'SR')=0 THEN
    BEGIN
     Erg:=0; SetOpSize(1);
    END
   ELSE IF NLS_StrCaseCmp(Asc,'CCR')=0 THEN
    BEGIN
     Erg:=1; SetOpSize(0);
    END
   ELSE IF NLS_StrCaseCmp(Asc,'BR')=0 THEN
    BEGIN
     Erg:=3; SetOpSize(0);
    END
   ELSE IF NLS_StrCaseCmp(Asc,'EP')=0 THEN
    BEGIN
     Erg:=4; SetOpSize(0);
    END
   ELSE IF NLS_StrCaseCmp(Asc,'DP')=0 THEN
    BEGIN
     Erg:=5; SetOpSize(0);
    END
   ELSE IF NLS_StrCaseCmp(Asc,'TP')=0 THEN
    BEGIN
     Erg:=7; SetOpSize(0);
    END
   ELSE DecodeCReg:=False;
END;

	PROCEDURE SplitDisp(VAR Asc:String; VAR Size:ShortInt);
BEGIN
   IF (Length(Asc)>2) AND (Asc[Length(Asc)]='8') AND (Asc[Length(Asc)-1]=':') THEN
    BEGIN
     Dec(Byte(Asc[0]),2); Size:=0;
    END
   ELSE IF (Length(Asc)>3) AND (Asc[Length(Asc)]='6') AND (Asc[Length(Asc)-1]='1') AND (Asc[Length(Asc)-2]=':') THEN
    BEGIN
     Dec(Byte(Asc[0]),3); Size:=1;
    END;
END;

        PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   Found;
VAR
   AdrWord:Word;
   OK,Unknown:Boolean;
   DispAcc:LongInt;
   HReg:Byte;
   DispSize,RegPart:ShortInt;
   Part:String;

   	PROCEDURE DecideAbsolute(Value:LongInt; Size:ShortInt; Unknown:Boolean);
VAR
   Base:LongInt;
BEGIN
   IF Size=-1 THEN
    IF (Value SHR 8=Reg_BR) AND (Mask AND MModAbs8<>0) THEN Size:=0 ELSE Size:=1;

   CASE Size OF
   0:BEGIN
      IF Unknown THEN Value:=(Value AND $ff) OR (Reg_BR SHL 8);
      IF Value SHR 8<>Reg_BR THEN WrError(110);
      AdrMode:=ModAbs8; AdrByte:=$05;
      AdrVals[0]:=Value AND $ff; AdrCnt:=1;
     END;
   1:BEGIN
      IF NOT Maximum THEN Base:=0 ELSE Base:=LongInt(AbsBank) SHL 16;
      IF Unknown THEN Value:=(Value AND $ffff) OR Base;
      IF Value SHR 16<>Base SHR 16 THEN WrError(110);
      AdrMode:=ModAbs16; AdrByte:=$15;
      AdrVals[0]:=(Value SHR 8) AND $ff;
      AdrVals[1]:=Value AND $ff;
      AdrCnt:=2;
     END;
   END;
END;

BEGIN
   AdrMode:=ModNone; AdrCnt:=0;

   { einfaches Register ? }

   IF DecodeReg(Asc,AdrByte) THEN
    BEGIN
     AdrMode:=ModReg; Inc(AdrByte,$a0); Goto Found;
    END;

   { immediate ? }

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OpSize OF
     -1:BEGIN
         OK:=False; WrError(1131);
        END;
     0:AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
     1:BEGIN
        AdrWord:=EvalIntExpression(Asc,Int16,OK);
        AdrVals[0]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
       END;
     END;
     IF OK THEN
      BEGIN
       AdrMode:=ModImm; AdrByte:=$04; AdrCnt:=OpSize+1;
      END;
     Goto Found;
    END;

   { indirekt ? }

   IF Asc[1]='@' THEN
    BEGIN
     Delete(Asc,1,1); IF IsIndirect(Asc) THEN Asc:=Copy(Asc,2,Length(Asc)-2);

     { Predekrement ? }

     IF (Asc[1]='-') AND (DecodeReg(Copy(Asc,2,Length(Asc)-1),AdrByte)) THEN
      BEGIN
       AdrMode:=ModPredec; Inc(AdrByte,$b0);
      END

     { Postinkrement ? }

     ELSE IF (Asc[Length(Asc)]='+') AND (DecodeReg(Copy(Asc,1,Length(Asc)-1),AdrByte)) THEN
      BEGIN
       AdrMode:=ModPostInc; Inc(AdrByte,$c0);
      END

     { zusammengesetzt }

     ELSE
      BEGIN
       DispAcc:=0; DispSize:=-1; RegPart:=-1; OK:=True; Unknown:=False;
       WHILE (Asc<>'') AND (OK) DO
        BEGIN
         SplitString(Asc,Part,Asc,QuotPos(Asc,','));
         IF DecodeReg(Part,HReg) THEN
          IF RegPart<>-1 THEN
	   BEGIN
	    WrError(1350); OK:=False;
	   END
	  ELSE RegPart:=HReg
         ELSE
          BEGIN
           SplitDisp(Part,DispSize);
           IF Part[1]='#' THEN Delete(Part,1,1);
           FirstPassUnknown:=False;
           Inc(DispAcc,EvalIntExpression(Part,Int32,OK));
           IF FirstPassUnknown THEN Unknown:=True;
          END;
        END;
       IF OK THEN
        BEGIN
         IF RegPart=-1 THEN DecideAbsolute(DispAcc,DispSize,Unknown)
         ELSE IF DispAcc=0 THEN
          CASE DispSize OF
          -1:BEGIN
              AdrMode:=ModIReg; AdrByte:=$d0+RegPart;
             END;
           0:BEGIN
              AdrMode:=ModDisp8; AdrByte:=$e0+RegPart;
	      AdrVals[0]:=0; AdrCnt:=1;
             END;
           1:BEGIN
              AdrMode:=ModDisp16; AdrByte:=$f0+RegPart;
	      AdrVals[0]:=0; AdrVals[1]:=0; AdrCnt:=2;
             END;
          END
         ELSE
          BEGIN
           IF DispSize=-1 THEN
            IF (DispAcc>=-128) AND (DispAcc<127) THEN DispSize:=0
            ELSE DispSize:=1;
           CASE DispSize OF
           0:BEGIN
              IF Unknown THEN DispAcc:=DispAcc AND $7f;
              IF ChkRange(DispAcc,-128,127) THEN
               BEGIN
                AdrMode:=ModDisp8; AdrByte:=$e0+RegPart;
                AdrVals[0]:=DispAcc AND $ff; AdrCnt:=1;
               END;
             END;
           1:BEGIN
              IF Unknown THEN DispAcc:=DispAcc AND $7fff;
              IF ChkRange(DispAcc,-$8000,$7fff) THEN
               BEGIN
                AdrMode:=ModDisp16; AdrByte:=$f0+RegPart;
                AdrVals[1]:=DispAcc AND $ff;
                AdrVals[0]:=(DispAcc SHR 8) AND $ff;
		AdrCnt:=2;
               END;
             END;
           END;
          END;
        END;
      END;
     Goto Found;
    END;

   { absolut }

   DispSize:=-1; SplitDisp(Asc,DispSize);
   FirstPassUnknown:=False;
   DispAcc:=EvalIntExpression(Asc,UInt24,OK);
   DecideAbsolute(DispAcc,DispSize,FirstPassUnknown);

Found:
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     WrError(1350); AdrMode:=ModNone; AdrCnt:=0;
    END;
END;

	FUNCTION ImmVal:LongInt;
VAR
   t:LongInt;
BEGIN
   CASE OpSize OF
   0:BEGIN
      t:=AdrVals[0]; IF t>127 THEN Dec(t,256);
     END;
   1:BEGIN
      t:=(Word(AdrVals[0]) SHL 8)+AdrVals[1];
      IF t>$7fff THEN Dec(t,$10000);
     END;
   END;
   ImmVal:=t;
END;

{---------------------------------------------------------------------------}


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

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFFH8_5Count=1;
   ONOFFH8_5s:ARRAY[1..ONOFFH8_5Count] OF ONOFFRec=
              ((Name:'MAXMODE'; Dest:@Maximum   ; FlagName:MaximumName   ));
   ASSUMEH8_5Count=4;
   ASSUMEH8_5s:ARRAY[1..ASSUMEH8_5Count] OF ASSUMERec=
	       ((Name:'DP'; Dest:@Reg_DP; Min:0; Max:$ff; NothingVal:-1),
		(Name:'EP'; Dest:@Reg_EP; Min:0; Max:$ff; NothingVal:-1),
		(Name:'TP'; Dest:@Reg_TP; Min:0; Max:$ff; NothingVal:-1),
		(Name:'BR'; Dest:@Reg_BR; Min:0; Max:$ff; NothingVal:-1));
BEGIN
   DecodePseudo:=True;

   IF CodeONOFF(@ONOFFH8_5s,ONOFFH8_5Count) THEN Exit;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUMEH8_5s,ASSUMEH8_5Count);
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_H8_5;
	Far;
VAR
   z,Num1,AdrInt:Integer;
   OK:Boolean;
   AdrLong:LongInt;
   HReg:Byte;
   Adr2Mode,HSize:ShortInt;
   Adr2Byte,Adr2Cnt:Byte;
   Adr2Vals:ARRAY[0..2] OF Byte;

   	PROCEDURE CopyAdr;
BEGIN
   Adr2Mode:=AdrMode;
   Adr2Byte:=AdrByte;
   Adr2Cnt:=AdrCnt;
   Move(AdrVals,Adr2Vals,AdrCnt);
END;

BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1; AbsBank:=Reg_DP;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

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

   IF AttrPart='' THEN SetOpSize(-1)
   ELSE
    CASE UpCase(AttrPart[1]) OF
    'B':SetOpSize(0);
    'W':SetOpSize(1);
    'L':SetOpSize(2);
    'Q':SetOpSize(3);
    'S':SetOpSize(4);
    'D':SetOpSize(5);
    'X':SetOpSize(6);
    'P':SetOpSize(7);
    ELSE
     BEGIN
      WrError(1107); Exit;
     END;
    END;
   NLS_UpString(Format);

   IF DecodeMoto16Pseudo(OpSize,True) THEN Exit;

   { Anweisungen ohne Argument }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF OpSize<>-1 THEN WrError(1100)
       ELSE IF Format<>' ' THEN WrError(1090)
       ELSE IF Hi(Code)=0 THEN
        BEGIN
         BAsmCode[0]:=Lo(Code); CodeLen:=1;
        END
       ELSE
        BEGIN
         BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
         CodeLen:=2;
        END;
       Exit;
      END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GEIFLS') THEN
      BEGIN
       IF OpSize=-1 THEN
        IF FormatCode=2 THEN SetOpSize(0) ELSE SetOpSize(1);
       IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[2],MModNoImm);
         IF AdrMode<>ModNone THEN
          BEGIN
           CopyAdr;
	   DecodeAdr(ArgStr[1],MModAll);
           IF AdrMode<>ModNone THEN
            BEGIN
             IF FormatCode=0 THEN
              IF (AdrMode=ModImm) AND (Adr2Mode=ModReg) THEN FormatCode:=2+OpSize
              ELSE IF (AdrMode=ModReg) AND (Adr2Byte=$e6) THEN FormatCode:=4
              ELSE IF (Adr2Mode=ModReg) AND (AdrByte=$e6) THEN FormatCode:=4
              ELSE IF (AdrMode=ModReg) AND (Adr2Mode=ModAbs8) THEN FormatCode:=6
              ELSE IF (AdrMode=ModAbs8) AND (Adr2Mode=ModReg) THEN FormatCode:=5
              ELSE FormatCode:=1;
             CASE FormatCode OF
             1:IF AdrMode=ModReg THEN
                BEGIN
                 BAsmCode[0]:=Adr2Byte+(OpSize SHL 3);
                 Move(Adr2Vals,BAsmCode[1],Adr2Cnt);
                 BAsmCode[1+Adr2Cnt]:=$90+(AdrByte AND 7);
                 CodeLen:=2+Adr2Cnt;
                END
               ELSE IF Adr2Mode=ModReg THEN
                BEGIN
                 BAsmCode[0]:=AdrByte+(OpSize SHL 3);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 BAsmCode[1+AdrCnt]:=$80+(Adr2Byte AND 7);
                 CodeLen:=2+AdrCnt;
                END
               ELSE IF AdrMode=ModImm THEN
                BEGIN
                 BAsmCode[0]:=Adr2Byte+(OpSize SHL 3);
                 Move(Adr2Vals,BAsmCode[1],Adr2Cnt);
                 IF (OpSize=0) OR ((ImmVal>=-128) AND (ImmVal<127)) THEN
                  BEGIN
                   BAsmCode[1+Adr2Cnt]:=$06;
                   BAsmCode[2+Adr2Cnt]:=AdrVals[OpSize];
                   CodeLen:=3+Adr2Cnt;
                  END
                 ELSE
                  BEGIN
                   BAsmCode[1+Adr2Cnt]:=$07;
                   Move(AdrVals,BAsmCode[2+Adr2Cnt],AdrCnt);
                   CodeLen:=2+Adr2Cnt+AdrCnt;
                  END;
                END
               ELSE WrError(1350);
             2:IF (AdrMode<>ModImm) OR (Adr2Mode<>ModReg) THEN WrError(1350)
               ELSE IF OpSize<>0 THEN WrError(1130)
	       ELSE
	        BEGIN
                 BAsmCode[0]:=$50+(Adr2Byte AND 7);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 CodeLen:=1+AdrCnt;
		END;
             3:IF (AdrMode<>ModImm) OR (Adr2Mode<>ModReg) THEN WrError(1350)
               ELSE IF OpSize<>1 THEN WrError(1130)
	       ELSE
	        BEGIN
                 BAsmCode[0]:=$58+(Adr2Byte AND 7);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 CodeLen:=1+AdrCnt;
		END;
             4:IF (AdrMode=ModReg) AND (Adr2Byte=$e6) THEN
                BEGIN
                 BAsmCode[0]:=$90+(OpSize SHL 3)+(AdrByte AND 7);
                 Move(Adr2Vals,BAsmCode[1],Adr2Cnt);
                 CodeLen:=1+Adr2Cnt;
                END
               ELSE IF (Adr2Mode=ModReg) AND (AdrByte=$e6) THEN
                BEGIN
                 BAsmCode[0]:=$80+(OpSize SHL 3)+(Adr2Byte AND 7);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 CodeLen:=1+AdrCnt;
                END
               ELSE WrError(1350);
             5:IF (AdrMode<>ModAbs8) OR (Adr2Mode<>ModReg) THEN WrError(1350)
               ELSE
                BEGIN
                 BAsmCode[0]:=$60+(OpSize SHL 3)+(Adr2Byte AND 7);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 CodeLen:=1+AdrCnt;
                END;
             6:IF (Adr2Mode<>ModAbs8) OR (AdrMode<>ModReg) THEN WrError(1350)
               ELSE
                BEGIN
                 BAsmCode[0]:=$70+(OpSize SHL 3)+(AdrByte AND 7);
                 Move(Adr2Vals,BAsmCode[1],Adr2Cnt);
                 CodeLen:=1+Adr2Cnt;
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
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE
      BEGIN
       IF Memo('STC') THEN
        BEGIN
         ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
        END;
       IF NOT DecodeCReg(ArgStr[2],HReg) THEN WrXError(1440,ArgStr[2])
       ELSE
        BEGIN
         IF Memo('LDC') THEN DecodeAdr(ArgStr[1],MModAll)
         ELSE DecodeAdr(ArgStr[1],MModNoImm);
         IF AdrMode<>ModNone THEN
          BEGIN
           BAsmCode[0]:=AdrByte+(OpSize SHL 3);
           Move(AdrVals,BAsmCode[1],AdrCnt);
           BAsmCode[1+AdrCnt]:=$88+(Ord(Memo('STC')) SHL 4)+HReg;
           CodeLen:=2+AdrCnt;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('LDM') THEN
    BEGIN
     IF OpSize=-1 THEN OpSize:=1;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF OpSize<>1 THEN WrError(1130)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE IF NOT DecodeRegList(ArgStr[2],BAsmCode[1]) THEN WrError(1410)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModPostInc);
       IF AdrMode<>ModNone THEN
        IF AdrByte AND 7<>7 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$02; CodeLen:=2;
         END;
      END;
     Exit;
    END;

   IF Memo('STM') THEN
    BEGIN
     IF OpSize=-1 THEN OpSize:=1;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF OpSize<>1 THEN WrError(1130)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE IF NOT DecodeRegList(ArgStr[1],BAsmCode[1]) THEN WrError(1410)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModPreDec);
       IF AdrMode<>ModNone THEN
        IF AdrByte AND 7<>7 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$12; CodeLen:=2;
         END;
      END;
     Exit;
    END;

   IF (Memo('MOVTPE')) OR (Memo('MOVFPE')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       IF Memo('MOVTPE') THEN
        BEGIN
         ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1]; ArgStr[1]:=ArgStr[3];
        END;
       IF OpSize=-1 THEN SetOpSize(0);
       IF OpSize<>0 THEN WrError(1130)
       ELSE IF NOT DecodeReg(ArgStr[2],HReg) THEN WrError(1350)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModNoImm-MModReg);
         IF AdrMode<>ModNone THEN
          BEGIN
           BAsmCode[0]:=AdrByte+(OpSize SHL 3);
           Move(AdrVals,BAsmCode[1],AdrCnt);
           BAsmCode[1+AdrCnt]:=0;
           BAsmCode[2+AdrCnt]:=$80+HReg+(Ord(Memo('MOVTPE')) SHL 4);
           CodeLen:=3+AdrCnt;
          END;
        END;
      END;
     Exit;
    END;

   { Arithmetik mit 2 Operanden }

   IF (Memo('ADD')) OR (Memo('SUB')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GQ') THEN
      BEGIN
       IF OpSize=-1 THEN SetOpSize(1);
       IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[2],MModNoImm);
         IF AdrMode<>ModNone THEN
          BEGIN
           CopyAdr;
	   DecodeAdr(ArgStr[1],MModAll);
           IF AdrMode<>ModNone THEN
            BEGIN
             AdrLong:=ImmVal;
             IF FormatCode=0 THEN
              IF (AdrMode=ModImm) AND (Abs(AdrLong)>=1) AND (Abs(AdrLong)<=2) THEN FormatCode:=2
              ELSE FormatCode:=1;
             CASE FormatCode OF
             1:IF Adr2Mode<>ModReg THEN WrError(1350)
               ELSE
                BEGIN
                 BAsmCode[0]:=AdrByte+(OpSize SHL 3);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 BAsmCode[1+AdrCnt]:=$20+(Ord(Memo('SUB')) SHL 4)+(Adr2Byte AND 7);
                 CodeLen:=2+AdrCnt;
                END;
             2:IF ChkRange(AdrLong,-2,2) THEN
                IF AdrLong=0 THEN WrError(1315)
                ELSE
	         BEGIN
                  IF Memo('SUB') THEN AdrLong:=-AdrLong;
                  BAsmCode[0]:=Adr2Byte+(OpSize SHl 3);
                  Move(Adr2Vals,BAsmCode[1],Adr2Cnt);
                  BAsmCode[1+Adr2Cnt]:=$08+Abs(AdrLong)-1;
                  IF AdrLong<0 THEN Inc(BAsmCode[1+Adr2Cnt],4);
                  CodeLen:=2+Adr2Cnt;
                 END;
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
     ELSE IF CheckFormat('GEI') THEN
      BEGIN
       IF OpSize=-1 THEN
        IF FormatCode=2 THEN SetOpSize(0) ELSE SetOpSize(1);
       IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[2],MModNoImm);
         IF AdrMode<>ModNone THEN
          BEGIN
           CopyAdr;
	   DecodeAdr(ArgStr[1],MModAll);
           IF AdrMode<>ModNone THEN
            BEGIN
             IF FormatCode=0 THEN
              IF (AdrMode=ModImm) AND (Adr2Mode=ModReg) THEN FormatCode:=2+OpSize
              ELSE FormatCode:=1;
	     CASE FormatCode OF
	     1:IF Adr2Mode=ModReg THEN
                BEGIN
                 BAsmCode[0]:=AdrByte+(OpSize SHL 3);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 BAsmCode[1+AdrCnt]:=$70+(Adr2Byte AND 7);
                 CodeLen:=2+AdrCnt;
                END
               ELSE IF AdrMode=ModImm THEN
                BEGIN
                 BAsmCode[0]:=Adr2Byte+(OpSize SHL 3);
                 Move(Adr2Vals,BAsmCode[1],Adr2Cnt);
                 BAsmCode[1+Adr2Cnt]:=$04+OpSize;
                 Move(AdrVals,BAsmCode[2+Adr2Cnt],AdrCnt);
                 CodeLen:=2+AdrCnt+Adr2Cnt;
                END
               ELSE WrError(1350);
	     2:IF (AdrMode<>ModImm) OR (Adr2Mode<>ModReg) THEN WrError(1350)
	       ELSE IF OpSize<>0 THEN WrError(1130)
	       ELSE
	        BEGIN
                 BAsmCode[0]:=$40+(Adr2Byte AND 7);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 CodeLen:=1+AdrCnt;
		END;
	     3:IF (AdrMode<>ModImm) OR (Adr2Mode<>ModReg) THEN WrError(1350)
	       ELSE IF OpSize<>1 THEN WrError(1130)
	       ELSE
	        BEGIN
                 BAsmCode[0]:=$48+(Adr2Byte AND 7);
                 Move(AdrVals,BAsmCode[1],AdrCnt);
                 CodeLen:=1+AdrCnt;
		END;
	     END;
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   FOR z:=1 TO RegEAOrderCount DO
    WITH RegEAOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF CheckFormat('G') THEN
        BEGIN
         IF OpSize=-1 THEN SetOpSize(DefSize);
         IF (1 SHL OpSize) AND SizeMask=0 THEN WrError(1130)
         ELSE IF NOT DecodeReg(ArgStr[2],HReg) THEN WrError(1350)
         ELSE
          BEGIN
           DecodeAdr(ArgStr[1],MModAll);
           IF AdrMode<>ModNone THEN
            BEGIN
             BAsmCode[0]:=AdrByte+(OpSize SHL 3);
             Move(AdrVals,BAsmCode[1],AdrCnt);
             BAsmCode[1+AdrCnt]:=Code+HReg;
             CodeLen:=2+AdrCnt;
            END;
          END
        END;
       Exit;
      END;

   FOR z:=1 TO TwoRegOrderCount DO
    WITH TwoRegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF Format<>' ' THEN WrError(1090)
       ELSE IF NOT DecodeReg(ArgStr[1],HReg) THEN WrError(1350)
       ELSE IF NOT DecodeReg(ArgStr[2],AdrByte) THEN WrError(1350)
       ELSE
        BEGIN
         IF OpSize=-1 THEN SetOpSize(DefSize);
         IF (1 SHL OpSize) AND SizeMask=0 THEN WrError(1130)
	 ELSE
	  BEGIN
           BAsmCode[0]:=$a0+HReg+(OpSize SHL 3);
           IF Hi(Code)<>0 THEN
            BEGIN
             BAsmCode[1]:=Lo(Code); BAsmCode[2]:=Hi(Code)+AdrByte;
             CodeLen:=3;
            END
           ELSE
            BEGIN
             BAsmCode[1]:=Code+AdrByte;
             CodeLen:=2;
            END;
	  END;
        END;
       Exit;
      END;

   FOR z:=1 TO LogOrderCount DO
    WITH LogOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NOT DecodeCReg(ArgStr[2],HReg) THEN WrXError(1440,ArgStr[2])
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModImm);
         IF AdrMode<>ModNone THEN
          BEGIN
           BAsmCode[0]:=AdrByte+(OpSize SHL 3);
           Move(AdrVals,BAsmCode[1],AdrCnt);
           BAsmCode[1+AdrCnt]:=Code+HReg;
           CodeLen:=2+AdrCnt;
          END;
        END;
       Exit;
      END;

   { Arithmetik mit einem Operanden }

   FOR z:=1 TO OneOrderCount DO
    WITH OneOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF CheckFormat('G') THEN
        BEGIN
         IF OpSize=-1 THEN SetOpSize(DefSize);
         IF (1 SHL OpSize) AND SizeMask=0 THEN WrError(1130)
	 ELSE
	  BEGIN
           DecodeAdr(ArgStr[1],MModNoImm);
           IF AdrMode<>ModNone THEN
            BEGIN
             BAsmCode[0]:=AdrByte+(OpSize SHL 3);
             Move(AdrVals,BAsmCode[1],AdrCnt);
             BAsmCode[1+AdrCnt]:=Code;
             CodeLen:=2+AdrCnt;
            END;
	  END;
        END;
       Exit;
      END;

   FOR z:=1 TO OneRegOrderCount DO
    WITH OneRegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF Format<>' ' THEN WrError(1090)
       ELSE IF NOT DecodeReg(ArgStr[1],HReg) THEN WrError(1350)
       ELSE
        BEGIN
         IF OpSize=-1 THEN SetOpSize(DefSize);
         IF (1 SHL OpSize) AND SizeMask=0 THEN WrError(1130)
	 ELSE
	  BEGIN
           BAsmCode[0]:=$a0+HReg+(OpSize SHL 3);
           BAsmCode[1]:=Code;
           CodeLen:=2;
	  END;
        END;
       Exit;
      END;

   { Bitoperationen }

   FOR z:=1 TO BitOrderCount DO
    WITH BitOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         IF OpSize=-1 THEN OpSize:=0;
         IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
         ELSE
          BEGIN
           DecodeAdr(ArgStr[2],MModNoImm);
           IF AdrMode<>ModNone THEN
            BEGIN
             IF DecodeReg(ArgStr[1],HReg) THEN
	      BEGIN
	       OK:=True; Inc(HReg,8);
              END
             ELSE
              BEGIN
               IF ArgStr[1][1]='#' THEN Delete(ArgStr[1],1,1);
               IF OpSize=0 THEN HReg:=EvalIntExpression(ArgStr[1],UInt3,OK)
               ELSE HReg:=EvalIntExpression(ArgStr[1],UInt4,OK);
               IF OK THEN Inc(HReg,$80);
              END;
             IF OK THEN
              BEGIN
               BAsmCode[0]:=AdrByte+(OpSize SHL 3);
               Move(AdrVals,BAsmCode[1],AdrCnt);
               BAsmCode[1+AdrCnt]:=Code+HReg;
               CodeLen:=2+AdrCnt;
              END;
            END;
          END;
        END;
       Exit;
      END;

   { SprÅnge }

   FOR z:=1 TO RelOrderCount DO
    WITH RelOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF Format<>' ' THEN WrError(1090)
       ELSE
        BEGIN
         FirstPassUnknown:=False;
         AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
         IF OK THEN
          IF (AdrLong SHR 16<>EProgCounter SHR 16) AND
	     (NOT FirstPassUnknown) AND (NOT SymbolQuestionable) THEN WrError(1910)
          ELSE IF EProgCounter AND $ffff>=$fffc THEN WrError(1905)
          ELSE
           BEGIN
            Dec(AdrLong,EProgCounter+2);
            IF AdrLong>$7fff THEN Dec(AdrLong,$10000)
            ELSE IF AdrLong<-$8000 THEN Inc(AdrLong,$10000);
            IF OpSize=-1 THEN
             IF (AdrLong<=127) AND (AdrLong>=-128) THEN OpSize:=4
             ELSE OpSize:=2;
            CASE OpSize OF
            2:BEGIN
               Dec(AdrLong);
               BAsmCode[0]:=Code+$10;
               BAsmCode[1]:=(AdrLong SHR 8) AND $ff;
               BAsmCode[2]:=AdrLong AND $ff;
               CodeLen:=3;
              END;
            4:IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
	      ELSE
	       BEGIN
                BAsmCode[0]:=Code;
                BAsmCode[1]:=AdrLong AND $ff;
                CodeLen:=2;
	       END;
            ELSE WrError(1130);
            END;
           END;
        END;
       Exit;
      END;

   IF (Memo('JMP')) OR (Memo('JSR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      BEGIN
       AbsBank:=EProgCounter SHR 16;
       HReg:=Ord(Memo('JSR')) SHL 3;
       DecodeAdr(ArgStr[1],MModIReg+MModReg+MModDisp8+MModDisp16+MModAbs16);
       CASE AdrMode OF
       ModReg,ModIReg:
        BEGIN
         BAsmCode[0]:=$11; BAsmCode[1]:=$d0+HReg+(AdrByte AND 7);
         CodeLen:=2;
        END;
       ModDisp8,ModDisp16:
        BEGIN
         BAsmCode[0]:=$11; BAsmCode[1]:=AdrByte+HReg;
         Move(AdrVals,BAsmCode[2],AdrCnt);
         CodeLen:=2+AdrCnt;
        END;
       ModAbs16:
        BEGIN
         BAsmCode[0]:=$10+HReg; Move(AdrVals,BAsmCode[1],AdrCnt);
         CodeLen:=1+AdrCnt;
        END;
       END;
      END;
     Exit;
    END;

   IF Copy(OpPart,1,4)='SCB/' THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE IF NOT DecodeReg(ArgStr[1],HReg) THEN WrError(1350)
     ELSE
      BEGIN
       ArgStr[3]:=Copy(OpPart,5,Length(OpPart)-4);
       OK:=True; NLS_UpString(ArgStr[3]);
       IF ArgStr[3]='F' THEN BAsmCode[0]:=$01
       ELSE IF ArgStr[3]='NE' THEN BAsmCode[0]:=$06
       ELSE IF ArgStr[3]='EQ' THEN BAsmCode[0]:=$07
       ELSE OK:=False;
       IF NOT OK THEN WrError(1360)
       ELSE
        BEGIN
         FirstPassUnknown:=False;
         AdrLong:=EvalIntExpression(ArgStr[2],UInt24,OK);
         IF OK THEN
          IF (AdrLong SHR 16<>EProgCounter SHR 16) AND
	     (NOT FirstPassUnknown) AND (NOT SymbolQuestionable) THEN WrError(1910)
          ELSE IF EProgCounter AND $ffff>=$fffc THEN WrError(1905)
          ELSE
           BEGIN
            Dec(AdrLong,EProgCounter+3);
            IF (NOT SymbolQuestionable) AND ((AdrLong>127) OR (AdrLong<-128)) THEN WrError(1370)
            ELSE
             BEGIN
              BAsmCode[1]:=$b8+HReg;
              BAsmCode[2]:=AdrLong AND $ff;
              CodeLen:=3;
             END
           END;
        END;
      END;
     Exit;
    END;

   IF (Memo('PJMP')) OR (Memo('PJSR')) THEN
    BEGIN
     z:=Ord(Memo('PJMP'));
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE IF NOT Maximum THEN WrError(1997)
     ELSE
      BEGIN
       IF ArgStr[1][1]='@' THEN Delete(ArgStr[1],1,1);
       IF DecodeReg(ArgStr[1],HReg) THEN
        BEGIN
         BAsmCode[0]:=$11; BAsmCode[1]:=$c0+((1-z) SHL 3)+HReg;
         CodeLen:=2;
        END
       ELSE
        BEGIN
         AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
         IF OK THEN
          BEGIN
           BAsmCode[0]:=$03+(z SHL 4);
           BAsmCode[1]:=(AdrLong SHR 16) AND $ff;
           BAsmCode[2]:=(AdrLong SHR 8) AND $ff;
           BAsmCode[3]:=AdrLong AND $ff;
           CodeLen:=4;
          END;
        END;
      END;
     Exit;
    END;

   IF (Memo('PRTD')) OR (Memo('RTD')) THEN
    BEGIN
     HReg:=Ord(Memo('PRTD'));
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1120)
     ELSE
      BEGIN
       Delete(ArgStr[1],1,1);
       HSize:=-1; SplitDisp(ArgStr[1],HSize);
       IF HSize<>-1 THEN SetOpSize(HSize);
       FirstPassUnknown:=False;
       AdrInt:=EvalIntExpression(ArgStr[1],SInt16,OK);
       IF FirstPassUnknown THEN AdrInt:=AdrInt AND 127;
       IF OK THEN
        BEGIN
         IF OpSize=-1 THEN
          IF (AdrInt<127) AND (AdrInt>-128) THEN OpSize:=0
          ELSE OpSize:=1;
         IF Memo('PRTD') THEN BAsmCode[0]:=$11;
         CASE OpSize OF
         0:IF ChkRange(AdrInt,-128,127) THEN
            BEGIN
             BAsmCode[HReg]:=$14; BAsmCode[1+HReg]:=AdrInt AND $ff;
             CodeLen:=2+HReg;
            END;
         1:BEGIN
            BAsmCode[HReg]:=$1c;
            BAsmCode[1+HReg]:=(AdrInt SHR 8) AND $ff;
            BAsmCode[2+HReg]:=AdrInt AND $ff;
            CodeLen:=3+HReg;
           END;
         ELSE WrError(1130);
         END;
        END;
      END;
     Exit;
    END;

   { SonderfÑlle }

   IF Memo('LINK') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode<>ModNone THEN
        IF AdrByte AND 7<>6 THEN WrError(1350)
        ELSE IF ArgStr[2][1]<>'#' THEN WrError(1120)
        ELSE
         BEGIN
          Delete(ArgStr[2],1,1);
          HSize:=-1; SplitDisp(ArgStr[2],HSize);
          IF HSize<>-1 THEN SetOpSize(HSize);
          FirstPassUnknown:=False;
          AdrInt:=EvalIntExpression(ArgStr[2],SInt16,OK);
          IF FirstPassUnknown THEN AdrInt:=AdrInt AND 127;
          IF OK THEN
           BEGIN
            IF OpSize=-1 THEN
             IF (AdrInt<127) AND (AdrInt>-128) THEN OpSize:=0
             ELSE OpSize:=1;
            CASE OpSize OF
            0:IF ChkRange(AdrInt,-128,127) THEN
               BEGIN
                BAsmCode[0]:=$17; BAsmCode[1]:=AdrInt AND $ff;
                CodeLen:=2;
               END;
            1:BEGIN
               BAsmCode[0]:=$1f;
               BAsmCode[1]:=(AdrInt SHR 8) AND $ff;
               BAsmCode[2]:=AdrInt AND $ff;
               CodeLen:=3;
              END;
            ELSE WrError(1130);
            END;
           END;
         END;
      END;
     Exit;
    END;

   IF Memo('UNLK') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode<>ModNone THEN
        IF AdrByte AND 7<>6 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$0f; CodeLen:=1;
         END;
      END;
     Exit;
    END;

   IF Memo('TRAPA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1120)
     ELSE
      BEGIN
       BAsmCode[1]:=$10+EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt4,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$08; CodeLen:=2;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_H8_5:Boolean;
	Far;
BEGIN
   IF ActPC=SegCode THEN
    IF Maximum THEN ChkPC_H8_5:=ProgCounter<$1000000
    ELSE ChkPC_H8_5:=ProgCounter<$10000
   ELSE ChkPC_H8_5:=False;
END;

	FUNCTION IsDef_H8_5:Boolean;
	Far;
BEGIN
   IsDef_H8_5:=False;
END;

	PROCEDURE SwitchFrom_H8_5;
	Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE InitCode_H8_5;
        Far;
BEGIN
   SaveInitProc;
   Reg_DP:=-1;
   Reg_EP:=-1;
   Reg_TP:=-1;
   Reg_BR:=-1;
END;

	PROCEDURE SwitchTo_H8_5;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$69; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.:';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_H8_5; ChkPC:=ChkPC_H8_5; IsDef:=IsDef_H8_5;
   SwitchFrom:=SwitchFrom_H8_5;

   InitFields;
END;

BEGIN
   CPU532:=AddCPU('HD6475328',SwitchTo_H8_5);
   CPU534:=AddCPU('HD6475348',SwitchTo_H8_5);
   CPU536:=AddCPU('HD6475368',SwitchTo_H8_5);
   CPU538:=AddCPU('HD6475388',SwitchTo_H8_5);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_H8_5;
END.
