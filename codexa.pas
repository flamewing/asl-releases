{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeXA;

{ AS Codegeneratormodul Philips XA }

INTERFACE
        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu,StringUt;


IMPLEMENTATION

CONST
   ModNone=-1;
   ModReg=0;      MModReg=1 SHL ModReg;
   ModMem=1;      MModMem=1 SHL ModMem;
   ModImm=2;      MModImm=1 SHL ModImm;
   ModAbs=3;      MModAbs=1 SHL ModAbs;

   FixedOrderCnt=5;
   JBitOrderCnt=3;
   ALUOrderCnt=8;
   RegOrderCnt=4;
   ShiftOrderCount=4;
   RotateOrderCount=4;
   RelOrderCount=17;
   StackOrderCount=4;

TYPE
   FixedOrder=RECORD
               Name:String[5];
               Code:Word;
              END;

   FixedOrderArray=ARRAY[1..FixedOrderCnt] OF FixedOrder;

   JBitOrderArray=ARRAY[1..JBitOrderCnt] OF FixedOrder;

   StACKOrderArray=ARRAY[1..StackOrderCount] OF FixedOrder;

   ALUOrderArray=ARRAY[0..ALUOrderCnt-1] OF String[4];

   RegOrder=RECORD
             Name:String[4];
             SizeMask:Byte;
             Code:Byte;
            END;

   RegOrderArray=ARRAY[1..RegOrderCnt] OF RegOrder;

   ShiftOrderArray=ARRAY[0..ShiftOrderCount-1] OF String[4];

   RotateOrderArray=ARRAY[1..RotateOrderCount] OF FixedOrder;

   RelOrderArray=ARRAY[1..RelOrderCount] OF FixedOrder;

VAR
   CPUXAG1,CPUXAG2,CPUXAG3:CPUVar;

   FixedOrders:^FixedOrderArray;
   JBitOrders:^JBitOrderArray;
   StackOrders:^StackOrderArray;
   ALUOrders:^ALUOrderArray;
   RegOrders:^RegOrderArray;
   ShiftOrders:^ShiftOrderArray;
   RotateOrders:^RotateOrderArray;
   RelOrders:^RelOrderArray;

   Reg_DS:LongInt;
   SaveInitProc:PROCEDURE;

   AdrMode:ShortInt;
   AdrCnt,AdrPart,MemPart:Byte;
   AdrVals:ARRAY[0..3] OF Byte;
   OpSize:ShortInt;

{---------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>FixedOrderCnt THEN Exit;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddJBit(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>JBitOrderCnt THEN Exit;
   WITH JBitOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddStack(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>StackOrderCount THEN Exit;
   WITH StackOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddReg(NName:String; NMask,NCode:Byte);
BEGIN
   Inc(z); IF z>RegOrderCnt THEN Exit;
   WITH RegOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; SizeMask:=NMask;
    END;
END;

        PROCEDURE AddRotate(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>RotateOrderCount THEN Exit;
   WITH RotateOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

        PROCEDURE AddRel(NName:String; NCode:Word);
BEGIN
   Inc(z); IF z>RelOrderCount THEN Exit;
   WITH RelOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('NOP'  ,$0000);
   AddFixed('RET'  ,$d680);
   AddFixed('RETI' ,$d690);
   AddFixed('BKPT' ,$00ff);
   AddFixed('RESET',$d610);

   New(JBitOrders); z:=0;
   AddJBit('JB'  ,$80);
   AddJBit('JBC' ,$c0);
   AddJBit('JNB' ,$a0);

   New(StackOrders); z:=0;
   AddStack('POP'  ,$1027);
   AddStack('POPU' ,$0037);
   AddStack('PUSH' ,$3007);
   AddStack('PUSHU',$2017);

   New(ALUOrders);
   ALUOrders^[0]:='ADD';  ALUOrders^[1]:='ADDC';
   ALUOrders^[2]:='SUB';  ALUOrders^[3]:='SUBB';
   ALUOrders^[4]:='CMP';  ALUOrders^[5]:='AND';
   ALUOrders^[6]:='OR';   ALUOrders^[7]:='XOR';

   New(RegOrders); z:=0;
   AddReg('NEG' ,3,$0b);
   AddReg('CPL' ,3,$0a);
   AddReg('SEXT',3,$09);
   AddReg('DA'  ,1,$08);

   New(ShiftOrders);
   ShiftOrders^[0]:='LSR'; ShiftOrders^[1]:='ASL';
   ShiftOrders^[2]:='ASR'; ShiftOrders^[3]:='NORM';

   New(RotateOrders); z:=0;
   AddRotate('RR' ,$b0); AddRotate('RL' ,$d3);
   AddRotate('RRC',$b7); AddRotate('RLC',$d7);

   New(RelOrders); z:=0;
   AddRel('BCC',$f0); AddRel('BCS',$f1); AddRel('BNE',$f2);
   AddRel('BEQ',$f3); AddRel('BNV',$f4); AddRel('BOV',$f5);
   AddRel('BPL',$f6); AddRel('BMI',$f7); AddRel('BG' ,$f8);
   AddRel('BL' ,$f9); AddRel('BGE',$fa); AddRel('BLT',$fb);
   AddRel('BGT',$fc); AddRel('BLE',$fd); AddRel('BR' ,$fe);
   AddRel('JZ' ,$ec); AddRel('JNZ',$ee);
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(JBitOrders);
   Dispose(StackOrders);
   Dispose(ALUOrders);
   Dispose(RegOrders);
   Dispose(ShiftOrders);
   Dispose(RotateOrders);
   Dispose(RelOrders);
END;

{---------------------------------------------------------------------------}

	PROCEDURE SetOpSize(NSize:ShortInt);
BEGIN
   IF OpSize=-1 THEN OpSize:=NSize
   ELSE IF OpSize<>NSize THEN
    BEGIN
     AdrMode:=ModNone; AdrCnt:=0; WrError(1131);
    END;
END;

	FUNCTION DecodeReg(Asc:String; VAR NSize:ShortInt; VAR Erg:Byte):Boolean;
BEGIN
   NLS_UpString(Asc);
   IF Asc='SP' THEN
    BEGIN
     Erg:=7; NSize:=1; DecodeReg:=True;
    END
   ELSE IF (Length(Asc)>=2) AND (Asc[1]='R') AND (Asc[2]>='0') AND (Asc[2]<='7') THEN
    IF Length(Asc)=2 THEN
     BEGIN
      Erg:=Ord(Asc[2])-AscOfs;
      IF OpSize=2 THEN
       BEGIN
        IF Odd(Erg) THEN
         BEGIN
          WrError(1760); Dec(Erg);
         END;
        NSize:=2;
       END
      ELSE
       BEGIN
        DecodeReg:=True;
        NSize:=1;
       END;
     END
    ELSE IF (Length(Asc)=3) AND (Asc[3]='L') THEN
     BEGIN
      Erg:=(Ord(Asc[2])-AscOfs) SHL 1; NSize:=0; DecodeReg:=True;
     END
    ELSE IF (Length(Asc)=3) AND (Asc[3]='H') THEN
     BEGIN
      Erg:=((Ord(Asc[2])-AscOfs) SHL 1)+1; NSize:=0; DecodeReg:=True;
     END
    ELSE DecodeReg:=False
   ELSE DecodeReg:=False;
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   Found;
VAR
   NSize:ShortInt;
   DispAcc,DispPart,AdrLong:LongInt;
   FirstFlag,NegFlag,NextFlag,ErrFlag,OK:Boolean;
   PPos,MPos:Integer;
   AdrInt:Word;
   Reg:Byte;
   Part:String;
BEGIN
   AdrMode:=ModNone; AdrCnt:=0; KillBlanks(Asc);

   IF DecodeReg(Asc,NSize,AdrPart) THEN
    BEGIN
     IF Mask AND MModReg<>0 THEN
      BEGIN
       AdrMode:=ModReg; SetOpSize(NSize);
      END
     ELSE
      BEGIN
       AdrMode:=ModMem; MemPart:=1; SetOpSize(NSize);
      END;
     Goto Found;
    END;

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OpSize OF
     -4:BEGIN
         AdrVals[0]:=EvalIntExpression(Asc,UInt5,OK);
         IF OK THEN
          BEGIN
           AdrCnt:=1; AdrMode:=ModImm;
          END;
        END;
     -3:BEGIN
         AdrVals[0]:=EvalIntExpression(Asc,SInt4,OK);
         IF OK THEN
          BEGIN
           AdrCnt:=1; AdrMode:=ModImm;
          END;
        END;
     -2:BEGIN
         AdrVals[0]:=EvalIntExpression(Asc,UInt4,OK);
         IF OK THEN
          BEGIN
           AdrCnt:=1; AdrMode:=ModImm;
          END;
        END;
     -1:WrError(1132);
     0:BEGIN
        AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
        IF OK THEN
         BEGIN
          AdrCnt:=1; AdrMode:=ModImm;
         END;
       END;
     1:BEGIN
        AdrInt:=EvalIntExpression(Asc,Int16,OK);
        IF OK THEN
         BEGIN
          AdrVals[0]:=Hi(AdrInt); AdrVals[1]:=Lo(AdrInt);
          AdrCnt:=2; AdrMode:=ModImm;
         END;
       END;
     2:BEGIN
        AdrLong:=EvalIntExpression(Asc,Int32,OK);
        IF OK THEN
         BEGIN
          AdrVals[0]:=(AdrLong SHR 24) AND $ff;
          AdrVals[1]:=(AdrLong SHR 16) AND $ff;
          AdrVals[2]:=(AdrLong SHR 8) AND $ff;
          AdrVals[3]:=AdrLong AND $ff;
          AdrCnt:=4; AdrMode:=ModImm;
         END;
       END;
     END;
     Goto Found;
    END;

   IF (Asc[1]='[') AND (Asc[Length(Asc)]=']') THEN
    BEGIN
     Asc:=Copy(Asc,2,Length(Asc)-2);
     IF Asc[Length(Asc)]='+' THEN
      BEGIN
       Asc:=Copy(Asc,1,Length(Asc)-1);
       IF NOT DecodeReg(Asc,NSize,AdrPart) THEN WrXError(1445,Asc)
       ELSE IF NSize<>1 THEN WrError(1350)
       ELSE
        BEGIN
         AdrMode:=ModMem; MemPart:=3;
        END;
      END
     ELSE
      BEGIN
       FirstFlag:=False; ErrFlag:=False;
       DispAcc:=0; AdrPart:=$ff; NegFlag:=False;
       WHILE (Asc<>'') AND (NOT ErrFlag) DO
        BEGIN
         PPos:=QuotPos(Asc,'+'); MPos:=QuotPos(Asc,'-');
         IF MPos<PPos THEN
	  BEGIN
	   PPos:=MPos; NextFlag:=True;
          END
         ELSE NextFlag:=False;
         SplitString(Asc,Part,Asc,PPos);
         IF DecodeReg(Part,NSize,Reg) THEN
          IF (NSize<>1) OR (AdrPart<>$ff) OR (NegFlag) THEN
           BEGIN
            WrError(1350); ErrFlag:=True;
           END
          ELSE AdrPart:=Reg
         ELSE
          BEGIN
           FirstPassUnknown:=False;
           DispPart:=EvalIntExpression(Part,Int32,ErrFlag);
           ErrFlag:=NOT ErrFlag;
           IF NOT ErrFlag THEN
            BEGIN
             FirstFlag:=FirstFlag OR FirstPassUnknown;
             IF NegFlag THEN Dec(DispAcc,DispPart)
             ELSE Inc(DispAcc,DispPart);
            END;
          END;
         NegFlag:=NextFlag;
        END;
       IF FirstFlag THEN DispAcc:=DispAcc AND $7fff;
       IF AdrPart=$ff THEN WrError(1350)
       ELSE IF DispAcc=0 THEN
        BEGIN
         AdrMode:=ModMem; MemPart:=2;
        END
       ELSE IF (DispAcc>=-128) AND (DispAcc<127) THEN
        BEGIN
         AdrMode:=ModMem; MemPart:=4;
         AdrVals[0]:=DispAcc AND $ff; AdrCnt:=1;
        END
       ELSE IF ChkRange(DispAcc,-$8000,$7fff) THEN
        BEGIN
         AdrMode:=ModMem; MemPart:=5;
         AdrVals[0]:=(DispAcc SHR 8) AND $ff;
         AdrVals[1]:=DispAcc AND $ff;
         AdrCnt:=2;
        END;
      END;
     Goto Found;
    END;

   FirstPassUnknown:=False;
   AdrLong:=EvalIntExpression(Asc,UInt24,OK);
   IF OK THEN
    BEGIN
     IF FirstPassUnknown THEN
      BEGIN
       IF Mask AND MModAbs=0 THEN AdrLong:=AdrLong AND $3ff;
      END;
     IF (AdrLong AND $ffff>$7ff) THEN WrError(1925)
     ELSE IF (AdrLong AND $ffff<=$3ff) THEN
      BEGIN
       IF AdrLong SHR 16<>Reg_DS THEN WrError(110);
       ChkSpace(SegData);
       AdrMode:=ModMem; MemPart:=6;
       AdrPart:=Hi(AdrLong); AdrVals[0]:=Lo(AdrLong);
       AdrCnt:=1;
      END
     ELSE IF AdrLong>$7ff THEN WrError(1925)
     ELSE
      BEGIN
       ChkSpace(SegIO);
       AdrMode:=ModMem; MemPart:=6;
       AdrPart:=Hi(AdrLong); AdrVals[0]:=Lo(AdrLong);
       AdrCnt:=1;
      END;
    END;

Found:
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     WrError(1350); AdrMode:=ModNone; AdrCnt:=0;
    END;
END;

	FUNCTION DecodeBitAddr(Asc:String; VAR Erg:LongInt):Boolean;
VAR
   p:Integer;
   BPos,Reg:Byte;
   Size,Res:ShortInt;
   AdrLong:LongInt;
   OK:Boolean;
BEGIN
   p:=RQuotPos(Asc,'.'); Res:=0;
   IF p=0 THEN
    BEGIN
     FirstPassUnknown:=False;
     AdrLong:=EvalIntExpression(Asc,UInt24,OK);
     IF FirstPassUnknown THEN AdrLong:=AdrLong AND $3ff;
     Erg:=AdrLong; Res:=1;
    END
   ELSE
    BEGIN
     FirstPassUnknown:=False;
     BPos:=EvalIntExpression(Copy(Asc,p+1,Length(Asc)-p),UInt4,OK);
     IF FirstPassUnknown THEN BPos:=BPos AND 7;
     IF OK THEN
      BEGIN
       Asc:=Copy(Asc,1,p-1);
       IF DecodeReg(Asc,Size,Reg) THEN
        IF (Size=0) AND (BPos>7) THEN WrError(1320)
        ELSE
         BEGIN
          IF Size=0 THEN Erg:=(Reg SHL 3)+BPos
          ELSE Erg:=(Reg SHL 4)+BPos;
          Res:=1;
         END
       ELSE IF BPos>7 THEN WrError(1320)
       ELSE
        BEGIN
         FirstPassUnknown:=False;
         AdrLong:=EvalIntExpression(Asc,UInt24,OK);
         IF (TypeFlag AND (1 SHL SegIO)<>0) THEN
          BEGIN
           ChkSpace(SegIO);
           IF FirstPassUnknown THEN AdrLong:=(AdrLong AND $3f) OR $400;
           IF ChkRange(AdrLong,$400,$43f) THEN
            BEGIN
             Erg:=$200+((AdrLong AND $3f) SHL 3)+BPos;
             Res:=1;
            END
           ELSE Res:=-1
          END
         ELSE
	  BEGIN
           ChkSpace(SegData);
	   IF FirstPassUnknown THEN AdrLong:=(AdrLong AND $00ff003f) OR $20;
	   IF ChkRange(AdrLong AND $ff,$20,$3f) THEN
            BEGIN
             Erg:=$100+((AdrLong AND $1f) SHL 3)+BPos+(AdrLong AND $ff0000);
             Res:=1;
            END
           ELSE Res:=-1
          END;
        END;
      END;
    END;
   DecodeBitAddr:=(Res=1);
   IF Res=0 THEN WrError(1350)
END;

	PROCEDURE ChkBitPage(Adr:LongInt);
BEGIN
   IF Adr SHR 16<>Reg_DS THEN WrError(110);
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUMEXACount=1;
   ASSUMEXAs:ARRAY[1..ASSUMEXACount] OF ASSUMERec=
             ((Name:'DS'; Dest:@Reg_DS; Min:0; Max:$ff; NothingVal:$100));
   ONOFFXACount=1;
   ONOFFXAs:ARRAY[1..ONOFFXACount] OF ONOFFRec=
             ((Name:'SUPMODE' ; Dest:@SupAllowed; FlagName:SupAllowedName));
VAR
   BAdr:LongInt;
   s1,s2,s3:String[40];
BEGIN
   DecodePseudo:=True;

   IF Memo('PORT') THEN
    BEGIN
     CodeEquate(SegIO,$400,$7ff);
     Exit;
    END;

   IF Memo('BIT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF DecodeBitAddr(ArgStr[1],BAdr) THEN
      BEGIN
       EnterIntSymbol(LabPart,BAdr,SegNone,False);
       ListLine:='=';
       CASE ((BAdr AND $3ff) SHR 8) OF
       0:BEGIN
          Str((BAdr SHR 4) AND 15,s1);
          Str(BAdr AND 15,s2);
          ListLine:=ListLine+'R'+s1+'.'+s2;
         END;
       1:BEGIN
          Str(BAdr AND 7,s1);
          ListLine:=ListLine+HexString((BAdr SHR 16) AND 255,0)+':'+HexString((BAdr AND $1f8) SHR 3,0)+'.'+s1;
         END;
       ELSE
        BEGIN
         Str(BAdr AND 7,s1);
         ListLine:=ListLine+'S:'+HexString(((BAdr SHR 3) AND $3f)+$400,0)+'.'+s1;
        END;
       END;
      END;
     Exit;
    END;

   IF CodeONOFF(@ONOFFXAs,ONOFFXACount) THEN Exit;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUMEXAs,ASSUMEXACount);
     Exit;
    END;

   DecodePseudo:=False;
END;

	FUNCTION IsRealDef:Boolean;
BEGIN
   IsRealDef:=(OpPart='PORT') OR (OpPart='BIT');
END;

	PROCEDURE ForceAlign;
BEGIN
   IF Odd(EProgCounter) THEN
    BEGIN
     BAsmCode[0]:=NOPCode; CodeLen:=1;
    END;
END;

	PROCEDURE MakeCode_XA;
	Far;
VAR
   HReg,HMem,HCnt,HPart:Byte;
   HVals:ARRAY[0..2] OF Byte;
   z,i:Integer;
   Mask:Word;
   AdrLong:LongInt;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1;

   { Operandengrî·e }

   IF AttrPart<>'' THEN
   CASE UpCase(AttrPart[1]) OF
   'B':SetOpSize(0);
   'W':SetOpSize(1);
   'D':SetOpSize(2);
   ELSE
    BEGIN
     WrError(1107); Exit;
    END;
   END;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { Labels mÅssen auf geraden Adressen liegen }

   IF (ActPC=SegCode) AND (NOT IsRealDef) AND (LabPart<>'') THEN
    BEGIN
     ForceAlign;
     EnterIntSymbol(LabPart,EProgCounter+CodeLen,ActPC,False);
    END;

   IF DecodeMoto16Pseudo(OpSize,True) THEN Exit;
   IF DecodeIntelPseudo(False) THEN Exit;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Anweisungen ohne Operanden }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF Hi(Code)=0 THEN
        BEGIN
         BAsmCode[CodeLen]:=Lo(Code); Inc(CodeLen);
        END
       ELSE
        BEGIN
         BAsmCode[CodeLen]:=Hi(Code);
	 BAsmCode[CodeLen+1]:=Lo(Code);
	 Inc(CodeLen,2);
         IF (Memo('RETI')) AND (NOT SupAllowed) THEN WrError(50);
        END;
       Exit;
      END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'C')=0 THEN
      BEGIN
       IF DecodeBitAddr(ArgStr[2],AdrLong) THEN
        IF AttrPart<>'' THEN WrError(1100)
        ELSE
         BEGIN
          ChkBitPage(AdrLong);
          BAsmCode[CodeLen]:=$08;
          BAsmCode[CodeLen+1]:=$20+Hi(AdrLong);
          BAsmCode[CodeLen+2]:=Lo(AdrLong);
          Inc(CodeLen,3);
         END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'C')=0 THEN
      BEGIN
       IF DecodeBitAddr(ArgStr[1],AdrLong) THEN
        IF AttrPart<>'' THEN WrError(1100)
        ELSE
         BEGIN
          ChkBitPage(AdrLong);
          BAsmCode[CodeLen]:=$08;
          BAsmCode[CodeLen+1]:=$30+Hi(AdrLong);
          BAsmCode[CodeLen+2]:=Lo(AdrLong);
          Inc(CodeLen,3);
         END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'USP')=0 THEN
      BEGIN
       SetOpSize(1);
       DecodeAdr(ArgStr[2],MModReg);
       IF AdrMode=ModReg THEN
        BEGIN
         BAsmCode[CodeLen]:=$98;
         BAsmCode[CodeLen+1]:=(AdrPart SHL 4)+$0f;
         Inc(CodeLen,2);
        END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'USP')=0 THEN
      BEGIN
       SetOpSize(1);
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode=ModReg THEN
        BEGIN
         BAsmCode[CodeLen]:=$90;
         BAsmCode[CodeLen+1]:=(AdrPart SHL 4)+$0f;
         Inc(CodeLen,2);
        END;
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModMem);
       CASE AdrMode OF
       ModReg:
        IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
        ELSE
         BEGIN
          HReg:=AdrPart;
          DecodeAdr(ArgStr[2],MModMem+MModImm);
          CASE AdrMode OF
          ModMem:
           BEGIN
            BAsmCode[CodeLen]:=$80+(OpSize SHL 3)+MemPart;
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
            Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
            Inc(CodeLen,2+AdrCnt);
            IF (MemPart=3) AND (HReg SHR (1-OpSize)=AdrPart) THEN WrError(140);
           END;
          ModImm:
           BEGIN
            BAsmCode[CodeLen]:=$91+(OpSize SHL 3);
            BAsmCode[CodeLen+1]:=$08+(HReg SHL 4);
            Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
            Inc(CodeLen,2+AdrCnt);
           END;
          END;
         END;
       ModMem:
        BEGIN
         Move(AdrVals,HVals,AdrCnt); HCnt:=AdrCnt; HPart:=MemPart; HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg+MModMem+MModImm);
         CASE AdrMode OF
         ModReg:
          IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE
           BEGIN
            BAsmCode[CodeLen]:=$80+(OpSize SHL 3)+HPart;
            BAsmCode[CodeLen+1]:=(AdrPart SHL 4)+$08+HReg;
            Move(HVals,BAsmCode[CodeLen+2],HCnt);
            Inc(CodeLen,2+HCnt);
            IF (HPart=3) AND (AdrPart SHR (1-OpSize)=HReg) THEN WrError(140);
           END;
         ModMem:
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE IF (HPart=6) AND (MemPart=6) THEN
           BEGIN
            BAsmCode[CodeLen]:=$97+(OpSize SHL 3);
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
            BAsmCode[CodeLen+2]:=HVals[0];
            BAsmCode[CodeLen+3]:=AdrVals[0];
            Inc(CodeLen,4);
           END
          ELSE IF (HPart=6) AND (MemPart=2) THEN
           BEGIN
            BAsmCode[CodeLen]:=$a0+(OpSize SHL 3);
            BAsmCode[CodeLen+1]:=$80+(AdrPart SHL 4)+HReg;
            BAsmCode[CodeLen+2]:=HVals[0];
            Inc(CodeLen,3);
           END
          ELSE IF (HPart=2) AND (MemPart=6) THEN
           BEGIN
            BAsmCode[CodeLen]:=$a0+(OpSize SHL 3);
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
            BAsmCode[CodeLen+2]:=AdrVals[0];
            Inc(CodeLen,3);
           END
          ELSE IF (HPart=3) AND (MemPart=3) THEN
           BEGIN
            BAsmCode[CodeLen]:=$90+(OpSize SHL 3);
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen,2);
            IF HReg=AdrPart THEN WrError(140);
           END
          ELSE WrError(1350);
         ModImm:
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE
           BEGIN
            BAsmCode[CodeLen]:=$90+(OpSize SHL 3)+HPart;
            BAsmCode[CodeLen+1]:=$08+(HReg SHL 4);
            Move(HVals,BAsmCode[CodeLen+2],HCnt);
            Move(AdrVals,BAsmCode[CodeLen+2+HCnt],AdrCnt);
            Inc(CodeLen,2+HCnt+AdrCnt);
           END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('MOVC') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF (AttrPart='') AND (NLS_StrCaseCmp(ArgStr[1],'A')=0) THEN OpSize:=0;
       IF NLS_StrCaseCmp(ArgStr[2],'[A+DPTR]')=0 THEN
        IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
        ELSE IF OpSize<>0 THEN WrError(1130)
        ELSE
         BEGIN
          BAsmCode[CodeLen]:=$90;
          BAsmCode[CodeLen+1]:=$4e;
          Inc(CodeLen,2);
         END
       ELSE IF NLS_StrCaseCmp(ArgStr[2],'[A+PC]')=0 THEN
        IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
        ELSE IF OpSize<>0 THEN WrError(1130)
        ELSE
         BEGIN
          BAsmCode[CodeLen]:=$90;
          BAsmCode[CodeLen+1]:=$4c;
          Inc(CodeLen,2);
         END
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModReg);
         IF AdrMode<>ModNone THEN
          IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE
           BEGIN
            HReg:=AdrPart;
            DecodeAdr(ArgStr[2],MModMem);
            IF AdrMode<>ModNone THEN
             IF MemPart<>3 THEN WrError(1350)
             ELSE
              BEGIN
               BAsmCode[CodeLen]:=$80+(OpSize SHL 3);
               BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
               Inc(CodeLen,2);
               IF (MemPart=3) AND (HReg SHR (1-OpSize)=AdrPart) THEN WrError(140);
              END;
           END;
        END;
      END;
     Exit;
    END;

   IF Memo('MOVX') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModMem);
       IF AdrMode=ModMem THEN
        CASE MemPart OF
        1:IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE
           BEGIN
            HReg:=AdrPart; DecodeAdr(ArgStr[2],MModMem);
            IF AdrMode=ModMem THEN
             IF MemPart<>2 THEN WrError(1350)
             ELSE
              BEGIN
               BAsmCode[CodeLen]:=$a7+(OpSize SHL 3);
               BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
               Inc(CodeLen,2);
              END;
           END;
        2:BEGIN
           HReg:=AdrPart; DecodeAdr(ArgStr[2],MModReg);
           IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
           ELSE
            BEGIN
             BAsmCode[CodeLen]:=$a7+(OpSize SHL 3);
             BAsmCode[CodeLen+1]:=$08+(AdrPart SHL 4)+HReg;
             Inc(CodeLen,2);
            END;
          END;
        ELSE WrError(1350);
        END;
      END;
     Exit;
    END;

   FOR z:=1 TO StackOrderCount DO
    WITH StackOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<1 THEN WrError(1110)
       ELSE
        BEGIN
         HReg:=$ff; OK:=True; Mask:=0;
         FOR i:=1 TO ArgCnt DO
          IF OK THEN
           BEGIN
            DecodeAdr(ArgStr[i],MModMem);
            IF AdrMode=ModNone THEN OK:=False
            ELSE CASE MemPart OF
            1:IF HReg=0 THEN
               BEGIN
                WrError(1350); OK:=False;
               END
              ELSE
               BEGIN
                HReg:=1; Mask:=Mask OR (1 SHL AdrPart);
               END;
            6:IF HReg<>$ff THEN
               BEGIN
                WrError(1350); OK:=False;
               END
              ELSE HReg:=0;
            ELSE
             BEGIN
              WrError(1350); OK:=False;
             END;
            END;
           END;
         IF OK THEN
          IF OpSize=-1 THEN WrError(1132)
          ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
          ELSE IF HReg=0 THEN
           BEGIN
            BAsmCode[CodeLen]:=$87+(OpSize SHL 3);
            BAsmCode[CodeLen+1]:=Hi(Code)+AdrPart;
            BAsmCode[CodeLen+2]:=AdrVals[0];
            Inc(CodeLen,3);
           END
          ELSE IF z<=2 THEN  { POP: obere Register zuerst }
           BEGIN
            IF Hi(Mask)<>0 THEN
             BEGIN
              BAsmCode[CodeLen]:=Lo(Code)+(OpSize SHL 3)+$40;
              BAsmCode[CodeLen+1]:=Hi(Mask);
              Inc(CodeLen,2);
             END;
            IF Lo(Mask)<>0 THEN
             BEGIN
              BAsmCode[CodeLen]:=Lo(Code)+(OpSize SHL 3);
              BAsmCode[CodeLen+1]:=Lo(Mask);
              Inc(CodeLen,2);
             END;
            IF (OpSize=1) AND (Memo('POP')) AND (Mask AND $80<>0) THEN WrError(140);
           END
          ELSE              { PUSH: untere Register zuerst }
           BEGIN
            IF Lo(Mask)<>0 THEN
             BEGIN
              BAsmCode[CodeLen]:=Lo(Code)+(OpSize SHL 3);
              BAsmCode[CodeLen+1]:=Lo(Mask);
              Inc(CodeLen,2);
             END;
            IF Hi(Mask)<>0 THEN
             BEGIN
              BAsmCode[CodeLen]:=Lo(Code)+(OpSize SHL 3)+$40;
              BAsmCode[CodeLen+1]:=Hi(Mask);
              Inc(CodeLen,2);
             END;
           END;
        END;
       Exit;
      END;

   IF Memo('XCH') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModMem);
       IF AdrMode=ModMem THEN
        CASE MemPart OF
        1:BEGIN
           HReg:=AdrPart; DecodeAdr(ArgStr[2],MModMem);
           IF AdrMode=ModMem THEN
            IF (OpSize<>1) AND (OpSize<>0) THEN WrError(1130)
            ELSE CASE MemPart OF
            1:BEGIN
               BAsmCode[CodeLen]:=$60+(OpSize SHL 3);
               BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
               IF HReg=AdrPart THEN WrError(140);
               Inc(CodeLen,2);
              END;
            2:BEGIN
               BAsmCode[CodeLen]:=$50+(OpSize SHL 3);
               BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
               Inc(CodeLen,2);
              END;
            6:BEGIN
               BAsmCode[CodeLen]:=$a0+(OpSize SHL 3);
               BAsmCode[CodeLen+1]:=$08+(HReg SHL 4)+AdrPart;
               BAsmCode[CodeLen+2]:=AdrVals[0];
               Inc(CodeLen,3);
              END;
           ELSE WrError(1350);
           END;
          END;
        2:BEGIN
           HReg:=AdrPart;
           DecodeAdr(ArgStr[2],MModReg);
           IF AdrMode=ModReg THEN
            IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
            ELSE
             BEGIN
              BAsmCode[CodeLen]:=$50+(OpSize SHL 3);
              BAsmCode[CodeLen+1]:=(AdrPart SHL 4)+HReg;
              Inc(CodeLen,2);
             END;
          END;
        6:BEGIN
           HPart:=AdrPart; HVals[0]:=AdrVals[0];
           DecodeAdr(ArgStr[2],MModReg);
           IF AdrMode=ModReg THEN
            IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
            ELSE
             BEGIN
              BAsmCode[CodeLen]:=$a0+(OpSize SHL 3);
              BAsmCode[CodeLen+1]:=$08+(AdrPart SHL 4)+HPart;
              BAsmCode[CodeLen+2]:=HVals[0];
              Inc(CodeLen,3);
             END;
          END;
        ELSE WrError(1350);
        END;
      END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=0 TO ALUOrderCnt-1 DO
    IF Memo(ALUOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[1],MModReg+MModMem);
        CASE AdrMode OF
        ModReg:
         IF OpSize>=2 THEN WrError(1130)
         ELSE IF OpSize=-1 THEN WrError(1132)
         ELSE
          BEGIN
           HReg:=AdrPart;
           DecodeAdr(ArgStr[2],MModMem+MModImm);
           CASE AdrMode OF
           ModMem:
            BEGIN
             BAsmCode[CodeLen]:=(z SHL 4)+(OpSize SHL 3)+MemPart;
             BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
             Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
             Inc(CodeLen,2+AdrCnt);
             IF (MemPart=3) AND (HReg SHR (1-OpSize)=AdrPart) THEN WrError(140);
            END;
           ModImm:
            BEGIN
             BAsmCode[CodeLen]:=$91+(OpSize SHL 3);
             BAsmCode[CodeLen+1]:=(HReg SHL 4)+z;
             Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
             Inc(CodeLen,2+AdrCnt);
            END;
           END;
          END;
        ModMem:
         BEGIN
          HReg:=AdrPart; HMem:=MemPart; HCnt:=AdrCnt;
          Move(AdrVals,HVals,AdrCnt);
          DecodeAdr(ArgStr[2],MModReg+MModImm);
          CASE AdrMode OF
          ModReg:
           IF OpSize=2 THEN WrError(1130)
           ELSE IF OpSize=-1 THEN WrError(1132)
           ELSE
            BEGIN
             BAsmCode[CodeLen]:=(z SHL 4)+(OpSize SHL 3)+HMem;
             BAsmCode[CodeLen+1]:=(AdrPart SHL 4)+8+HReg;
             Move(HVals,BAsmCode[CodeLen+2],HCnt);
             Inc(CodeLen,2+HCnt);
             IF (HMem=3) AND (AdrPart SHR (1-OpSize)=HReg) THEN WrError(140);
            END;
          ModImm:
           IF OpSize=2 THEN WrError(1130)
           ELSE IF OpSize=-1 THEN WrError(1132)
           ELSE
            BEGIN
             BAsmCode[CodeLen]:=$90+HMem+(OpSize SHL 3);
             BAsmCode[CodeLen+1]:=(HReg SHL 4)+z;
             Move(HVals,BAsmCode[CodeLen+2],HCnt);
             Move(AdrVals,BAsmCode[CodeLen+2+HCnt],AdrCnt);
             Inc(CodeLen,2+AdrCnt+HCnt);
            END;
          END;
         END;
        END;
       END;
      Exit;
     END;

   FOR z:=1 TO RegOrderCnt DO
    WITH RegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModReg);
         CASE AdrMode OF
         ModReg:
          IF SizeMask AND (1 SHL OpSize)=0 THEN WrError(1130)
          ELSE
           BEGIN
            BAsmCode[CodeLen]:=$90+(OpSize SHL 3);
            BAsmCode[CodeLen+1]:=(AdrPart SHL 4)+Code;
            Inc(CodeLen,2);
           END;
         END;
        END;
       Exit;
      END;

   IF (Memo('ADDS')) OR (Memo('MOVS')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       HMem:=OpSize; OpSize:=-3;
       DecodeAdr(ArgStr[2],MModImm);
       CASE AdrMode OF
       ModImm:
        BEGIN
         HReg:=AdrVals[0]; OpSize:=HMem;
         DecodeAdr(ArgStr[1],MModMem);
         CASE AdrMode OF
         ModMem:
          IF OpSize=2 THEN WrError(1130)
          ELSE IF OpSize=-1 THEN WrError(1132)
          ELSE
           BEGIN
            BAsmCode[CodeLen]:=$a0+(Ord(Memo('MOVS')) SHL 4)+(OpSize SHL 3)+MemPart;
            BAsmCode[CodeLen+1]:=(AdrPart SHL 4)+(HReg AND $0f);
            Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
            Inc(CodeLen,2+AdrCnt);
           END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('DIV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode=ModReg THEN
        IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
        ELSE
         BEGIN
          HReg:=AdrPart; Dec(OpSize); DecodeAdr(ArgStr[2],MModReg+MModImm);
          CASE AdrMode OF
          ModReg:
           BEGIN
            BAsmCode[CodeLen]:=$e7+(OpSize SHL 3);
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen,2);
           END;
          ModImm:
           BEGIN
            BAsmCode[CodeLen]:=$e8+OpSize;
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+$0b-(OpSize SHL 1);
            Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
            Inc(CodeLen,2+AdrCnt);
           END;
          END;
         END;
      END;
     Exit;
    END;

   IF Memo('DIVU') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode=ModReg THEN
        IF (OpSize=0) AND (Odd(AdrPart)) THEN WrError(1445)
        ELSE
         BEGIN
          HReg:=AdrPart; z:=OpSize; IF OpSize<>0 THEN Dec(OpSize);
	  DecodeAdr(ArgStr[2],MModReg+MModImm);
          CASE AdrMode OF
          ModReg:
           BEGIN
            BAsmCode[CodeLen]:=$e1+(z SHL 2);
            IF z=2 THEN Inc(BAsmCode[CodeLen],4);
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen,2);
           END;
          ModImm:
           BEGIN
            BAsmCode[CodeLen]:=$e8+Ord(z=2);
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+$01+(Ord(z=1) SHL 1);
            Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
            Inc(CodeLen,2+AdrCnt);
           END;
          END;
         END;
      END;
     Exit;
    END;

   IF Memo('MUL') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode=ModReg THEN
        IF OpSize<>1 THEN WrError(1130)
        ELSE IF Odd(AdrPart) THEN WrError(1445)
        ELSE
         BEGIN
          HReg:=AdrPart; DecodeAdr(ArgStr[2],MModReg+MModImm);
          CASE AdrMode OF
          ModReg:
           BEGIN
            BAsmCode[CodeLen]:=$e6;
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen,2);
           END;
          ModImm:
           BEGIN
            BAsmCode[CodeLen]:=$e9;
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+$08;
            Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
            Inc(CodeLen,2+AdrCnt);
           END;
          END;
         END;
      END;
     Exit;
    END;

   IF Memo('MULU') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode=ModReg THEN
        IF Odd(AdrPart) THEN WrError(1445)
        ELSE
         BEGIN
          HReg:=AdrPart;
	  DecodeAdr(ArgStr[2],MModReg+MModImm);
          CASE AdrMode OF
          ModReg:
           BEGIN
            BAsmCode[CodeLen]:=$e0+(OpSize SHL 2);
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen,2);
           END;
          ModImm:
           BEGIN
            BAsmCode[CodeLen]:=$e8+OpSize;
            BAsmCode[CodeLen+1]:=(HReg SHL 4);
            Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
            Inc(CodeLen,2+AdrCnt);
           END;
          END;
         END;
      END;
     Exit;
    END;

   IF Memo('LEA') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       IF AdrMode=ModReg THEN
        IF OpSize<>1 THEN WrError(1130)
        ELSE
         BEGIN
          HReg:=AdrPArt;
          DecodeAdr('['+ArgStr[2]+']',MModMem);
          IF AdrMode=ModMem THEN
           CASE MemPart OF
           4,5:
            BEGIN
             BAsmCode[CodeLen]:=$20+(MemPart SHL 3);
             BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
             Move(AdrVals,BAsmCode[CodeLen+2],AdrCnt);
             Inc(CodeLen,2+AdrCnt);
            END;
           ELSE WrError(1350);
           END;
         END;
      END;
     Exit;
    END;

   { Logik }

   IF (Memo('ANL')) OR (Memo('ORL')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'C')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       IF ArgStr[2][1]='/' THEN
        BEGIN
         OK:=True; Delete(ArgStr[2],1,1);
        END
       ELSE OK:=False;
       IF DecodeBitAddr(ArgStr[2],AdrLong) THEN
        BEGIN
         ChkBitPage(AdrLong);
         BAsmCode[CodeLen]:=$08;
         BAsmCode[CodeLen+1]:=$40+(Ord(Memo('ORL')) SHL 5)+(Ord(OK) SHL 4)+(Hi(AdrLong) AND 3);
         BAsmCode[CodeLen+2]:=Lo(AdrLong);
         Inc(CodeLen,3);
        END;
      END;
     Exit;
    END;

   IF (Memo('CLR')) OR (Memo('SETB')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF DecodeBitAddr(ArgStr[1],AdrLong) THEN
      BEGIN
       ChkBitPage(AdrLong);
       BAsmCode[CodeLen]:=$08;
       BAsmCode[CodeLen+1]:=(Ord(Memo('SETB')) SHL 4)+(Hi(AdrLong) AND 3);
       BAsmCode[CodeLen+2]:=Lo(AdrLong);
       Inc(CodeLen,3);
      END;
     Exit;
    END;

   FOR z:=0 TO ShiftOrderCount-1 DO
    IF Memo(ShiftOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF OpSize>2 THEN WrError(1130)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[1],MModReg);
        CASE AdrMode OF
        ModReg:
         BEGIN
          HReg:=AdrPart; HMem:=OpSize;
          IF ArgStr[2][1]='#' THEN
	   IF HMem=2 THEN OpSize:=-4 ELSE OpSize:=-2
	  ELSE OpSize:=0;
          IF z=3 THEN DecodeAdr(ArgStr[2],MModReg)
          ELSE DecodeAdr(ArgStr[2],MModReg+MModImm);
          CASE AdrMode OF
          ModReg:
           BEGIN
            BAsmCode[CodeLen]:=$c0+((HMem AND 1) SHL 3)+z;
            IF HMem=2 THEN Inc(BAsmCode[CodeLen],12);
            BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen,2);
            IF Memo('NORM') THEN
             IF HMem=2 THEN
              BEGIN
               IF AdrPart SHR 2=HReg SHR 1 THEN WrError(140);
              END
             ELSE IF AdrPart SHR HMem=HReg THEN WrError(140);
           END;
          ModImm:
           BEGIN
            BAsmCode[CodeLen]:=$d0+((HMem AND 1) SHL 3)+z;
            IF HMem=2 THEN
	     BEGIN
              Inc(BAsmCode[CodeLen],12);
              BAsmCode[CodeLen+1]:=((HReg AND 14) SHL 4)+AdrVals[0];
             END
            ELSE BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrVals[0];
            Inc(CodeLen,2);
           END;
	  END;
         END;
        END;
       END;
      Exit;
     END;

   FOR z:=1 TO RotateOrderCount DO
    WITH RotateOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModReg);
         CASE AdrMode OF
         ModReg:
          IF OpSize=2 THEN WrError(1130)
          ELSE
           BEGIN
            HReg:=AdrPart; HMem:=OpSize; OpSize:=-2;
            DecodeAdr(ArgStr[2],MModImm);
            CASE AdrMode OF
            ModImm:
             BEGIN
              BAsmCode[CodeLen]:=Code+(HMem SHL 3);
              BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrVals[0];
              Inc(CodeLen,2);
             END;
            END;
           END;
         END;
        END;
       Exit;
      END;

   { vermischtes }

   IF Memo('TRAP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       OpSize:=-2;
       DecodeAdr(ArgStr[1],MModImm);
       CASE AdrMode OF
       ModImm:
        BEGIN
         BAsmCode[CodeLen]:=$d6;
         BAsmCode[CodeLen+1]:=$30+AdrVals[0];
         Inc(CodeLen,2);
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
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         FirstPassUnknown:=True;
         AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
         IF OK THEN
          BEGIN
           ChkSpace(SegCode);
           IF FirstPassUnknown THEN AdrLong:=AdrLong AND $fffffffe;
           Dec(AdrLong,(EProgCounter+CodeLen+2) AND $fffffe);
           IF (NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256)) THEN WrError(1370)
           ELSE IF Odd(AdrLong) THEN WrError(1325)
           ELSE
            BEGIN
             BAsmCode[CodeLen]:=Code;
             BAsmCode[CodeLen+1]:=(AdrLong SHR 1) AND $ff;
             Inc(CodeLen,2);
            END;
          END;
        END;
       Exit;
      END;

   IF Memo('CALL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPArt<>'' THEN WrError(1100)
     ELSE IF ArgStr[1][1]='[' THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModMem);
       IF AdrMode<>ModNone THEN
        IF MemPart<>2 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[CodeLen]:=$c6;
          BAsmCode[CodeLen+1]:=AdrPart;
          Inc(CodeLen,2);
         END;
      END
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
        BEGIN
         ChkSpace(SegCode);
         IF FirstPassUnknown THEN AdrLong:=AdrLong AND $fffffffe;
         Dec(AdrLong,(EProgCounter+CodeLen+3) AND $fffffe);
         IF (NOT SymbolQuestionable) AND ((AdrLong>65534) OR (AdrLong<-65536)) THEN WrError(1370)
         ELSE IF Odd(AdrLong) THEN WrError(1325)
         ELSE
          BEGIN
           AdrLong:=AdrLong SHR 1;
           BAsmCode[CodeLen]:=$c5;
           BAsmCode[CodeLen+1]:=(AdrLong SHR 8) AND $ff;
           BAsmCode[CodeLen+2]:=AdrLong AND $ff;
           Inc(CodeLen,3);
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('JMP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'[A+DPTR]')=0 THEN
      BEGIN
       BAsmCode[CodeLen]:=$d6;
       BAsmCode[CodeLen+1]:=$46;
       Inc(CodeLen,2);
      END
     ELSE IF Copy(ArgStr[1],1,2)='[[' THEN
      BEGIN
       DecodeAdr(Copy(ArgStr[1],2,Length(ArgStr[1])-2),MModMem);
       IF AdrMode=ModMem THEN
        CASE MemPart OF
        3:BEGIN
           BAsmCode[CodeLen]:=$d6;
           BAsmCode[CodeLen+1]:=$60+AdrPart;
           Inc(CodeLen,2);
          END;
        ELSE WrError(1350);
        END;
      END
     ELSE IF ArgStr[1][1]='[' THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModMem);
       IF AdrMode=ModMem THEN
        CASE MemPart OF
        2:BEGIN
           BAsmCode[CodeLen]:=$d6;
           BAsmCode[CodeLen+1]:=$70+AdrPart;
           Inc(CodeLen,2);
          END;
        ELSE WrError(1350);
        END;
      END
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
        BEGIN
         ChkSpace(SegCode);
         IF FirstPassUnknown THEN AdrLong:=AdrLong AND $fffffffe;
         Dec(AdrLong,(EProgCounter+CodeLen+3) AND $fffffe);
         IF (NOT SymbolQuestionable) AND ((AdrLong>65534) OR (AdrLong<-65536)) THEN WrError(1370)
         ELSE IF Odd(AdrLong) THEN WrError(1325)
         ELSE
          BEGIN
           AdrLong:=AdrLong SHR 1;
           BAsmCode[CodeLen]:=$d5;
           BAsmCode[CodeLen+1]:=(AdrLong SHR 8) AND $ff;
           BAsmCode[CodeLen+2]:=AdrLong AND $ff;
           Inc(CodeLen,3);
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('CJNE') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrLong:=EvalIntExpression(ArgStr[3],UInt24,OK);
       IF FirstPassUnknown THEN AdrLong:=AdrLong AND $fffffe;
       IF OK THEN
        BEGIN
         ChkSpace(SegCode); OK:=False; DecodeAdr(ArgStr[1],MModMem);
         IF AdrMode=ModMem THEN
          CASE MemPart OF
          1:IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
            ELSE
             BEGIN
              HReg:=AdrPart; DecodeAdr(ArgStr[2],MModMem+MModImm);
              CASE AdrMode OF
              ModMem:
               IF MemPart<>6 THEN WrError(1350)
               ELSE
                BEGIN
                 BAsmCode[CodeLen]:=$e2+(OpSize SHL 3);
                 BAsmCode[CodeLen+1]:=(HReg SHL 4)+AdrPart;
                 BAsmCode[CodeLen+2]:=AdrVals[0];
                 HReg:=CodeLen+3;
                 Inc(CodeLen,4); OK:=True;
                END;
              ModImm:
               BEGIN
                BAsmCode[CodeLen]:=$e3+(OpSize SHL 3);
                BAsmCode[CodeLen+1]:=HReg SHL 4;
                HReg:=CodeLen+2;
                Move(AdrVals,BAsmCode[CodeLen+3],AdrCnt);
                Inc(CodeLen,3+AdrCnt); OK:=True;
               END;
              END;
             END;
          2:IF (OpSize<>-1) AND (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
            ELSE
             BEGIN
              HReg:=AdrPart; DecodeAdr(ArgStr[2],MModImm);
              IF AdrMode=ModImm THEN
               BEGIN
                BAsmCode[CodeLen]:=$e3+(OpSize SHL 3);
                BAsmCode[CodeLen+1]:=HReg SHL 4+8;
                HReg:=CodeLen+2;
                Move(AdrVals,BAsmCode[CodeLen+3],AdrCnt);
                Inc(CodeLen,3+AdrCnt); OK:=True;
               END;
             END;
          ELSE WrError(1350);
          END;
         IF OK THEN
          BEGIN
           Dec(AdrLong,(EProgCounter+CodeLen) AND $fffffe); OK:=False;
           IF (NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256)) THEN WrError(1370)
           ELSE IF Odd(AdrLong) THEN WrError(1325)
           ELSE
	    BEGIN
	     BAsmCode[HReg]:=(AdrLong SHR 1) AND $ff; OK:=True;
            END;
          END;
         IF NOT OK THEN CodeLen:=0;
        END;
      END;
     Exit;
    END;

   IF Memo('DJNZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrLong:=EvalIntExpression(ArgStr[2],UInt24,OK);
       IF FirstPassUnknown THEN AdrLong:=AdrLong AND $fffffe;
       IF OK THEN
        BEGIN
         ChkSpace(SegCode);
         DecodeAdr(ArgStr[1],MModMem);
         OK:=False; DecodeAdr(ArgStr[1],MModMem);
         IF AdrMode=ModMem THEN
          CASE MemPart OF
          1:IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
            ELSE
             BEGIN
              BAsmCode[CodeLen]:=$87+(OpSize SHL 3);
              BAsmCode[CodeLen+1]:=(AdrPart SHL 4)+$08;
              HReg:=CodeLen+2;
              Inc(CodeLen,3); OK:=True;
             END;
          6:IF OpSize=-1 THEN WrError(1132)
	    ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
            ELSE
             BEGIN
              BAsmCode[CodeLen]:=$e2+(OpSize SHL 3);
              BAsmCode[CodeLen+1]:=$08+AdrPart;
              BAsmCode[CodeLen+2]:=AdrVals[0];
              HReg:=CodeLen+3;
              Inc(CodeLen,4); OK:=True;
             END;
          ELSE WrError(1350);
          END;
         IF OK THEN
          BEGIN
           Dec(AdrLong,(EProgCounter+CodeLen) AND $fffffe); OK:=False;
           IF (NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256)) THEN WrError(1370)
           ELSE IF Odd(AdrLong) THEN WrError(1325)
           ELSE
	    BEGIN
	     BAsmCode[HReg]:=(AdrLong SHR 1) AND $ff; OK:=True;
            END;
          END;
         IF NOT OK THEN CodeLen:=0;
        END;
      END;
     Exit;
    END;

   IF (Memo('FCALL')) OR (Memo('FJMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPArt<>'' THEN WrError(1100)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF FirstPassUnknown THEN AdrLong:=AdrLong AND $fffffe;
       IF OK THEN
        IF Odd(AdrLong) THEN WrError(1325)
        ELSE
         BEGIN
          BAsmCode[CodeLen]:=$c4+(Ord(Memo('FJMP')) SHL 4);
          BAsmCode[CodeLen+1]:=(AdrLong SHR 8) AND $ff;
          BAsmCode[CodeLen+2]:=AdrLong AND $ff;
          BAsmCode[CodeLen+3]:=(AdrLong SHR 16) AND $ff;
          Inc(CodeLen,4);
         END;
      END;
     Exit;
    END;

   FOR z:=1 TO JBitOrderCnt DO
    WITH JBitOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE IF DecodeBitAddr(ArgStr[1],AdrLong) THEN
        BEGIN
         BAsmCode[CodeLen]:=$97;
         BAsmCode[CodeLen+1]:=Code+Hi(AdrLong);
         BAsmCode[CodeLen+2]:=Lo(AdrLong);
         FirstPassUnknown:=False;
         AdrLong:=EvalIntExpression(ArgStr[2],UInt24,OK);
         IF FirstPassUnknown THEN AdrLong:=AdrLong AND $fffffe;
         Dec(AdrLong,(EProgCounter+CodeLen+4) AND $fffffe);
         IF OK THEN
          IF (NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256)) THEN WrError(1370)
          ELSE IF Odd(AdrLong) THEN WrError(1325)
          ELSE
           BEGIN
            BAsmCode[CodeLen+3]:=(AdrLong SHR 1) AND $ff;
            Inc(CodeLen,4);
           END;
        END;
       Exit;
      END;

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_XA;
        Far;
BEGIN
   SaveInitProc;
   Reg_DS:=0;
END;

	FUNCTION ChkPC_XA:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode,
   SegData  : ok:=ProgCounter < $1000000;
   SegIO    : ok:=(ProgCounter>$3ff) AND (ProgCounter<$800);
   ELSE ok:=False;
   END;
   ChkPC_XA:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_XA:Boolean;
	Far;
BEGIN
   IsDef_XA:=ActPC=SegCode;
END;

        PROCEDURE SwitchFrom_XA;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_XA;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$3c; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.';

   ValidSegs:=[SegCode,SegData,SegIO];
   Grans[SegCode ]:=1; ListGrans[SegCode ]:=1; SegInits[SegCode ]:=0;
   Grans[SegData ]:=1; ListGrans[SegData ]:=1; SegInits[SegData ]:=0;
   Grans[SegIO   ]:=1; ListGrans[SegIO   ]:=1; SegInits[SegIO   ]:=$400;

   MakeCode:=MakeCode_XA; ChkPC:=ChkPC_XA; IsDef:=IsDef_XA;
   SwitchFrom:=SwitchFrom_XA; InitFields;
END;

BEGIN
   CPUXAG1:=AddCPU('XAG1',SwitchTo_XA);
   CPUXAG2:=AddCPU('XAG2',SwitchTo_XA);
   CPUXAG3:=AddCPU('XAG3',SwitchTo_XA);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_XA;
END.
