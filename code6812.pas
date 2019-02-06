{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
        Unit Code6812;

Interface

        Uses NLS,
             AsmDef,AsmPars,AsmSub,CodePseu;


Implementation

TYPE
   FixedOrder=RECORD
               Name:String[5];
               Code:Word;
              END;

   GenOrder=RECORD
             Name:String[5];
             Code:Word;
             MayImm,MayDir,MayExt:Boolean;
             ThisOpSize:ShortInt;
            END;

   JmpOrder=RECORD
             Name:String[4];
             Code:Word;
             MayDir:Boolean;
            END;


CONST
  ModNone=-1;
  ModImm=0;    MModImm=1 SHL ModImm;
  ModDir=1;    MModDir=1 SHL ModDir;
  ModExt=2;    MModExt=1 SHL ModExt;
  ModIdx=3;    MModIdx=1 SHL ModIdx;
  ModIdx1=4;   MModIdx1=1 SHL ModIdx1;
  ModIdx2=5;   MModIdx2=1 SHL ModIdx2;
  ModDIdx=6;   MModDIdx=1 SHL ModDIdx;
  ModIIdx2=7;  MModIIdx2=1 SHL ModIIdx2;

  MModAllIdx=MModIdx OR MModIdx1 OR MModIdx2 OR MModDIdx OR MModIIdx2;

  FixedOrderCount=87;
  BranchOrderCount=20;
  GenOrderCount=56 ;
  LoopOrderCount=6;
  LEAOrderCount=3;
  JmpOrderCount=2;


TYPE
   FixedOrderField=ARRAY[1..FixedOrderCount] OF FixedOrder;
   BranchOrderField=ARRAY[1..BranchOrderCount] OF FixedOrder;
   GenOrderField=ARRAY[1..GenOrderCount] OF GenOrder;
   LoopOrderField=ARRAY[1..LoopOrderCount] OF FixedOrder;
   LEAOrderField=ARRAY[1..LEAOrderCount] OF FixedOrder;
   JmpOrderField=ARRAY[1..JmpOrderCount] OF JmpOrder;

VAR
   OpSize:ShortInt;
   AdrMode:ShortInt;
   AdrCnt:Byte;
   ExPos:ShortInt;
   AdrVals:ARRAY[0..3] OF Byte;
   CPU6812:CPUVar;

   FixedOrders:^FixedOrderField;
   BranchOrders:^BranchOrderField;
   GenOrders:^GenOrderField;
   LoopOrders:^LoopOrderField;
   LEAOrders:^LEAOrderField;
   JmpOrders:^JmpOrderField;

{-----------------------------------------------------------------------------}

        PROCEDURE InitFields;
VAR
   z:Integer;

        PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   Inc(z); IF (z>FixedOrderCount) THEN Halt(255);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
END;

        PROCEDURE AddBranch(NName:String; NCode:Word);
BEGIN
   Inc(z); IF (z>BranchOrderCount) THEN Halt(255);
   WITH BranchOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
END;

        PROCEDURE AddGen(NName:String; NCode:Word; NMayI,NMayD,NMayE:Boolean; NSize:ShortInt);
BEGIN
   Inc(z); IF (z>GenOrderCount) THEN Halt(255);
   WITH GenOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
     MayImm:=NMayI;
     MayDir:=NMayD;
     MayExt:=NMayE;
     ThisOpSize:=NSize;
    END;
END;

        PROCEDURE AddLoop(NName:String; NCode:Word);
BEGIN
   Inc(z); IF (z>LoopOrderCount) THEN Halt(255);
   WITH LoopOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
END;

        PROCEDURE AddLEA(NName:String; NCode:Word);
BEGIN
   Inc(z); IF (z>LEAOrderCount) THEN Halt(255);
   WITH LEAOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
    END;
END;

        PROCEDURE AddJmp(NName:String; NCode:Word; NDir:Boolean);
BEGIN
   Inc(z); IF (z>JmpOrderCount) THEN Halt(255);
   WITH JmpOrders^[z] DO
    BEGIN
     Name:=NName;
     Code:=NCode;
     MayDir:=NDir;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('ABA'  ,$1806); AddFixed('ABX'  ,$1ae5);
   AddFixed('ABY'  ,$19ed); AddFixed('ASLA' ,$0048);
   AddFixed('ASLB' ,$0058); AddFixed('ASLD' ,$0059);
   AddFixed('ASRA' ,$0047); AddFixed('ASRB' ,$0057);
   AddFixed('BGND' ,$0000); AddFixed('CBA'  ,$1817);
   AddFixed('CLC'  ,$10fe); AddFixed('CLI'  ,$10ef);
   AddFixed('CLRA' ,$0087); AddFixed('CLRB' ,$00c7);
   AddFixed('CLV'  ,$10fd); AddFixed('COMA' ,$0041);
   AddFixed('COMB' ,$0051); AddFixed('DAA'  ,$1807);
   AddFixed('DECA' ,$0043); AddFixed('DECB' ,$0053);
   AddFixed('DES'  ,$1b9f); AddFixed('DEX'  ,$0009);
   AddFixed('DEY'  ,$0003); AddFixed('EDIV' ,$0011);
   AddFixed('EDIVS',$1814); AddFixed('EMUL' ,$0013);
   AddFixed('EMULS',$1813); AddFixed('FDIV' ,$1811);
   AddFixed('IDIV' ,$1810); AddFixed('IDIVS',$1815);
   AddFixed('INCA' ,$0042); AddFixed('INCB' ,$0052);
   AddFixed('INS'  ,$1b81); AddFixed('INX'  ,$0008);
   AddFixed('INY'  ,$0002); AddFixed('LSLA' ,$0048);
   AddFixed('LSLB' ,$0058); AddFixed('LSLD' ,$0059);
   AddFixed('LSRA' ,$0044); AddFixed('LSRB' ,$0054);
   AddFixed('LSRD' ,$0049); AddFixed('MEM'  ,$0001);
   AddFixed('MUL'  ,$0012); AddFixed('NEGA' ,$0040);
   AddFixed('NEGB' ,$0050); AddFixed('NOP'  ,$00a7);
   AddFixed('PSHA' ,$0036); AddFixed('PSHB' ,$0037);
   AddFixed('PSHC' ,$0039); AddFixed('PSHD' ,$003b);
   AddFixed('PSHX' ,$0034); AddFixed('PSHY' ,$0035);
   AddFixed('PULA' ,$0032); AddFixed('PULB' ,$0033);
   AddFixed('PULC' ,$0038); AddFixed('PULD' ,$003a);
   AddFixed('PULX' ,$0030); AddFixed('PULY' ,$0031);
   AddFixed('REV'  ,$183a); AddFixed('REVW' ,$183b);
   AddFixed('ROLA' ,$0045); AddFixed('ROLB' ,$0055);
   AddFixed('RORA' ,$0046); AddFixed('RORB' ,$0056);
   AddFixed('RTC'  ,$000a); AddFixed('RTI'  ,$000b);
   AddFixed('RTS'  ,$003d); AddFixed('SBA'  ,$1816);
   AddFixed('SEC'  ,$1401); AddFixed('SEI'  ,$1410);
   AddFixed('SEV'  ,$1402); AddFixed('STOP' ,$183e);
   AddFixed('SWI'  ,$003f); AddFixed('TAB'  ,$180e);
   AddFixed('TAP'  ,$b702); AddFixed('TBA'  ,$180f);
   AddFixed('TPA'  ,$b720); AddFixed('TSTA' ,$0097);
   AddFixed('TSTB' ,$00d7); AddFixed('TSX'  ,$b775);
   AddFixed('TSY'  ,$b776); AddFixed('TXS'  ,$b757);
   AddFixed('TYS'  ,$b767); AddFixed('WAI'  ,$003e);
   AddFixed('WAV'  ,$183c); AddFixed('XGDX' ,$b7c5);
   AddFixed('XGDY' ,$b7c6);

   New(BranchOrders); z:=0;
   AddBranch('BGT',$2e);    AddBranch('BGE',$2c);
   AddBranch('BEQ',$27);    AddBranch('BLE',$2f);
   AddBranch('BLT',$2d);    AddBranch('BHI',$22);
   AddBranch('BHS',$24);    AddBranch('BCC',$24);
   AddBranch('BNE',$26);    AddBranch('BLS',$23);
   AddBranch('BLO',$25);    AddBranch('BCS',$25);
   AddBranch('BMI',$2b);    AddBranch('BVS',$29);
   AddBranch('BRA',$20);    AddBranch('BPL',$2a);
   AddBranch('BGT',$2e);    AddBranch('BRN',$21);
   AddBranch('BVC',$28);    AddBranch('BSR',$07);

   New(GenOrders); z:=0;
   AddGen('ADCA' ,$0089,True ,True ,True , 0);
   AddGen('ADCB' ,$00c9,True ,True ,True , 0);
   AddGen('ADDA' ,$008b,True ,True ,True , 0);
   AddGen('ADDB' ,$00cb,True ,True ,True , 0);
   AddGen('ADDD' ,$00c3,True ,True ,True , 1);
   AddGen('ANDA' ,$0084,True ,True ,True , 0);
   AddGen('ANDB' ,$00c4,True ,True ,True , 0);
   AddGen('ASL'  ,$0048,False,False,True ,-1);
   AddGen('ASR'  ,$0047,False,False,True ,-1);
   AddGen('BITA' ,$0085,True ,True ,True , 0);
   AddGen('BITB' ,$00c5,True ,True ,True , 0);
   AddGen('CLR'  ,$0049,False,False,True ,-1);
   AddGen('CMPA' ,$0081,True ,True ,True , 0);
   AddGen('CMPB' ,$00c1,True ,True ,True , 0);
   AddGen('COM'  ,$0041,False,False,True ,-1);
   AddGen('CPD'  ,$008c,True ,True ,True , 1);
   AddGen('CPS'  ,$008f,True ,True ,True , 1);
   AddGen('CPX'  ,$008e,True ,True ,True , 1);
   AddGen('CPY'  ,$008d,True ,True ,True , 1);
   AddGen('DEC'  ,$0043,False,False,True ,-1);
   AddGen('EMAXD',$18fa,False,False,False,-1);
   AddGen('EMAXM',$18fe,False,False,False,-1);
   AddGen('EMIND',$18fb,False,False,False,-1);
   AddGen('EMINM',$18ff,False,False,False,-1);
   AddGen('EORA' ,$0088,True ,True ,True , 0);
   AddGen('EORB' ,$00c8,True ,True ,True , 0);
   AddGen('INC'  ,$0042,False,False,True ,-1);
   AddGen('LDAA' ,$0086,True ,True ,True , 0);
   AddGen('LDAB' ,$00c6,True ,True ,True , 0);
   AddGen('LDD'  ,$00cc,True ,True ,True , 1);
   AddGen('LDS'  ,$00cf,True ,True ,True , 1);
   AddGen('LDX'  ,$00ce,True ,True ,True , 1);
   AddGen('LDY'  ,$00cd,True ,True ,True , 1);
   AddGen('LSL'  ,$0048,False,False,True ,-1);
   AddGen('LSR'  ,$0044,False,False,True ,-1);
   AddGen('MAXA' ,$18f8,False,False,False,-1);
   AddGen('MAXM' ,$18fc,False,False,False,-1);
   AddGen('MINA' ,$18f9,False,False,False,-1);
   AddGen('MINM' ,$18fd,False,False,False,-1);
   AddGen('NEG'  ,$0040,False,False,True ,-1);
   AddGen('ORAA' ,$008a,True ,True ,True , 0);
   AddGen('ORAB' ,$00ca,True ,True ,True , 0);
   AddGen('ROL'  ,$0045,False,False,True ,-1);
   AddGen('ROR'  ,$0046,False,False,True ,-1);
   AddGen('SBCA' ,$0082,True ,True ,True , 0);
   AddGen('SBCB' ,$00c2,True ,True ,True , 0);
   AddGen('STAA' ,$004a,False,True ,True , 0);
   AddGen('STAB' ,$004b,False,True ,True , 0);
   AddGen('STD'  ,$004c,False,True ,True ,-1);
   AddGen('STS'  ,$004f,False,True ,True ,-1);
   AddGen('STX'  ,$004e,False,True ,True ,-1);
   AddGen('STY'  ,$004d,False,True ,True ,-1);
   AddGen('SUBA' ,$0080,True ,True ,True , 0);
   AddGen('SUBB' ,$00c0,True ,True ,True , 0);
   AddGen('SUBD' ,$0083,True ,True ,True , 1);
   AddGen('TST'  ,$00c7,False,False,True ,-1);

   New(LoopOrders); z:=0;
   AddLoop('DBEQ',$00); AddLoop('DBNE',$20);
   AddLoop('IBEQ',$80); AddLoop('IBNE',$a0);
   AddLoop('TBEQ',$40); AddLoop('TBNE',$60);

   New(LEAOrders); z:=0;
   AddLEA('LEAS',$1b);
   AddLEA('LEAX',$1a);
   AddLEA('LEAY',$19);

   New(JmpOrders); z:=0;
   AddJmp('JMP',$06,False);
   AddJmp('JSR',$16,True);
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(BranchOrders);
   Dispose(GenOrders);
   Dispose(LoopOrders);
   Dispose(LEAOrders);
   Dispose(JmpOrders);
END;


{-----------------------------------------------------------------------------}

CONST
   PCReg=3;

	FUNCTION DecodeReg16(Asc:String; VAR Erg:Byte):Boolean;
BEGIN
   DecodeReg16:=True; NLS_UpString(Asc);
   IF Asc='X' THEN Erg:=0
   ELSE IF Asc='Y' THEN Erg:=1
   ELSE IF Asc='SP' THEN Erg:=2
   ELSE IF Asc='PC' THEN Erg:=PCReg
   ELSE DecodeReg16:=False;
END;

	FUNCTION ValidReg(Asc:String):Boolean;
VAR
   Dummy:Byte;
BEGIN
   IF (Asc[1]='-') OR (Asc[1]='+') THEN Delete(Asc,1,1)
   ELSE IF (Asc[Length(Asc)]='-') OR (Asc[Length(Asc)]='+') THEN Dec(Byte(Asc[0]));
   ValidReg:=DecodeReg16(Asc,Dummy);
END;

	FUNCTION DecodeReg8(Asc:String; VAR Erg:Byte):Boolean;
BEGIN
   DecodeReg8:=True; NLS_UpString(Asc);
   IF Asc='A' THEN Erg:=0
   ELSE IF Asc='B' THEN Erg:=1
   ELSE IF Asc='D' THEN Erg:=2
   ELSE DecodeReg8:=False;
END;

	FUNCTION DecodeReg(Asc:String; VAR Erg:Byte):Boolean;
BEGIN
   IF DecodeReg8(Asc,Erg) THEN
    BEGIN
     IF Erg=2 THEN Erg:=4; DecodeReg:=True;
    END
   ELSE IF DecodeReg16(Asc,Erg) THEN
    BEGIN
     Inc(Erg,5); DecodeReg:=Erg<>PCReg;
    END
   ELSE IF NLS_StrCaseCmp(Asc,'CCR')=0 THEN
    BEGIN
     Erg:=2; DecodeReg:=True;
    END
   ELSE DecodeReg:=False;
END;

	PROCEDURE CutShort(VAR Asc:String; VAR ShortMode:Integer);
BEGIN
   IF Asc[1]='>' THEN
    BEGIN
     ShortMode:=1;
     Delete(Asc,1,1);
    END
   ELSE IF Copy(Asc,1,2)='<<' THEN
    BEGIN
     ShortMode:=3;
     Delete(Asc,1,2);
    END
   ELSE IF Asc[1]='<' THEN
    BEGIN
     ShortMode:=2;
     Delete(Asc,1,1);
    END
   ELSE ShortMode:=0;
END;

	FUNCTION DistFits(Reg:Byte; Dist:Integer; Offs:Integer; Min,Max:LongInt):Boolean;
BEGIN
   IF Reg=PCReg THEN Dec(Dist,Offs);
   DistFits:=((Dist>=Min) AND (Dist<=Max)) OR ((Reg=PCReg) AND SymbolQuestionable);
END;

        PROCEDURE DecodeAdr(Start,Stop:Integer; Mask:Word);
LABEL
   Found;
VAR
   AdrWord,p,ShortMode:Integer;
   OK:Boolean;
   DecFlag,AutoFlag,PostFlag:Boolean;
BEGIN
   AdrMode:=ModNone; AdrCnt:=0;

   IF (Stop-Start=0) THEN
    BEGIN

     { immediate }

     IF ArgStr[Start][1]='#' THEN
      BEGIN
       Delete(ArgStr[Start],1,1);
       CASE OpSize OF
       -1:WrError(1132);
       0:BEGIN
          AdrVals[0]:=EvalIntExpression(ArgStr[Start],Int8,OK);
          IF OK THEN
           BEGIN
            AdrCnt:=1; AdrMode:=ModImm;
           END;
         END;
       1:BEGIN
          AdrWord:=EvalIntExpression(ArgStr[Start],Int16,OK);
          IF OK THEN
           BEGIN
            AdrVals[0]:=AdrWord SHR 8; AdrVals[1]:=AdrWord AND $ff;
            AdrCnt:=2; AdrMode:=ModImm;
           END;
         END;
       END;
       Goto Found;
      END;

     { indirekt }

     IF (ArgStr[Start][1]='[') AND (ArgStr[Start][Length(ArgStr[Start])]=']') THEN
      BEGIN
       ArgStr[Start]:=Copy(ArgStr[Start],2,Length(ArgStr[Start])-2);
       p:=QuotPos(ArgStr[Start],',');
       IF p>Length(ArgStr[Start]) THEN WrError(1350)
       ELSE IF NOT DecodeReg16(Copy(ArgStr[Start],p+1,Length(ArgStr[Start])-p),AdrVals[0]) THEN
        WrXError(1445,Copy(ArgStr[Start],p+1,Length(ArgStr[Start])-p))
       ELSE IF NLS_StrCaseCmp(Copy(ArgStr[Start],1,p-1),'D')=0 THEN
        BEGIN
         AdrVals[0]:=(AdrVals[0] SHL 3) OR $e7;
         AdrCnt:=1; AdrMode:=ModDIdx;
        END
       ELSE
        BEGIN
         AdrWord:=EvalIntExpression(Copy(ArgStr[Start],1,p-1),Int16,OK);
         IF OK THEN
          BEGIN
           IF AdrVals[0]=PCReg THEN Dec(AdrWord,EProgCounter+ExPos+3);
           AdrVals[0]:=(AdrVals[0] SHL 3) OR $e3;
           AdrVals[1]:=AdrWord SHR 8;
           AdrVals[2]:=AdrWord AND $ff;
           AdrCnt:=3; AdrMode:=ModIIdx2;
          END;
        END;
       Goto Found;
      END;

     { dann absolut }

     CutShort(ArgStr[Start],ShortMode);

     IF (ShortMode=2) OR ((ShortMode=0) AND (Mask AND MModExt=0)) THEN
      AdrWord:=EvalIntExpression(ArgStr[Start],UInt8,OK)
     ELSE
      AdrWord:=EvalIntExpression(ArgStr[Start],UInt16,OK);

     IF OK THEN
      IF (ShortMode<>1) AND (AdrWord AND $ff00=0) AND (Mask AND MModDir<>0) THEN
       BEGIN
        AdrMode:=ModDir; AdrVals[0]:=AdrWord AND $ff; AdrCnt:=1;
       END
      ELSE
       BEGIN
        AdrMode:=ModExt;
	AdrVals[0]:=(AdrWord SHR 8) AND $ff; AdrVals[1]:=AdrWord AND $ff;
	AdrCnt:=2;
       END;
     Goto Found;
    END

   ELSE IF (Stop-Start=1) THEN
    BEGIN

     { Autoin/-dekrement abspalten }

     IF (ArgStr[Stop][1]='-') OR (ArgStr[Stop][1]='+') THEN
      BEGIN
       DecFlag:=ArgStr[Stop][1]='-';
       AutoFlag:=True; PostFlag:=False; Delete(ArgStr[Stop],1,1);
      END
     ELSE IF (ArgStr[Stop][Length(ArgStr[Stop])]='-') OR (ArgStr[Stop][Length(ArgStr[Stop])]='+') THEN
      BEGIN
       DecFlag:=ArgStr[Stop][Length(ArgStr[Stop])]='-';
       AutoFlag:=True; PostFlag:=True; Dec(Byte(ArgStr[Stop][0]));
      END
     ELSE AutoFlag:=False;

     IF AutoFlag THEN
      BEGIN
       IF NOT DecodeReg16(ArgStr[Stop],AdrVals[0]) THEN WrXError(1445,ArgStr[Stop])
       ELSE IF AdrVals[0]=PCReg THEN WrXError(1445,ArgStr[Stop])
       ELSE
        BEGIN
         FirstPassUnknown:=False;
         AdrWord:=EvalIntExpression(ArgStr[Start],SInt8,OK);
         IF FirstPassUnknown THEN AdrWord:=1;
         IF AdrWord=0 THEN
          BEGIN
           AdrVals[0]:=0; AdrCnt:=1; AdrMode:=ModIdx;
          END
         ELSE IF AdrWord>8 THEN WrError(1320)
         ELSE IF AdrWord<-8 THEN WrError(1315)
         ELSE
          BEGIN
           IF AdrWord<0 THEN
            BEGIN
             DecFlag:=NOT DecFlag; AdrWord:=-AdrWord;
            END;
           IF DecFlag THEN AdrWord:=8-AdrWord ELSE Dec(AdrWord);
           IF Abs(AdrWord)=8 THEN AdrWord:=0;
           AdrVals[0]:=(AdrVals[0] SHL 6)+$20+(Ord(PostFlag) SHL 4)+(Ord(DecFlag) SHL 3)+(AdrWord AND 7);
           AdrCnt:=1; AdrMode:=ModIdx;
          END
        END;
       Goto Found;
      END

     ELSE
      BEGIN
       IF NOT DecodeReg16(ArgStr[Stop],AdrVals[0]) THEN WrXError(1445,ArgStr[Stop])
       ELSE IF DecodeReg8(ArgStr[Start],AdrVals[1]) THEN
        BEGIN
         AdrVals[0]:=(AdrVals[0] SHL 3)+AdrVals[1]+$e4;
         AdrCnt:=1; AdrMode:=ModIdx;
        END
       ELSE
        BEGIN
         CutShort(ArgStr[Start],ShortMode);
         AdrWord:=EvalIntExpression(ArgStr[Start],Int16,OK);
         IF AdrVals[0]=PCReg THEN Dec(AdrWord,EProgCounter+ExPos);
         IF OK THEN
          IF (ShortMode<>1) AND (ShortMode<>2) AND (Mask AND MModIdx<>0) AND DistFits(AdrVals[0],AdrWord,1,-16,15) THEN
           BEGIN
            IF AdrVals[0]=PCReg THEN Dec(AdrWord);
            AdrVals[0]:=(AdrVals[0] SHL 6)+(AdrWord AND $1f);
            AdrCnt:=1; AdrMode:=ModIdx;
           END
          ELSE IF (ShortMode<>1) AND (ShortMode<>3) AND (Mask AND MModIdx1<>0) AND DistFits(AdrVals[0],AdrWord,2,-256,255) THEN
           BEGIN
            IF AdrVals[0]=PCReg THEN Dec(AdrWord,2);
            AdrVals[0]:=$e0+(AdrVals[0] SHL 3)+((AdrWord SHR 8) AND 1);
            AdrVals[1]:=AdrWord AND $ff;
            AdrCnt:=2; AdrMode:=ModIdx1;
           END
          ELSE
           BEGIN
            IF AdrVals[0]=PCReg THEN Dec(AdrWord,3);
            AdrVals[0]:=$e3+(AdrVals[0] SHL 3);
            AdrVals[1]:=(AdrWord SHR 8) AND $ff;
            AdrVals[2]:=AdrWord AND $ff;
            AdrCnt:=3; AdrMode:=ModIdx2;
           END;
        END;
       Goto Found;
      END;
    END

   ELSE WrError(1350);

Found:
   IF (AdrMode<>ModNone) AND ((1 SHL AdrMode) AND Mask=0) THEN
    BEGIN
     AdrMode:=ModNone; AdrCnt:=0; WrError(1350);
    END;
END;

{-----------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

	PROCEDURE Try2Split(Src:Integer);
VAR
   p,z:Integer;
BEGIN
   KillPrefBlanks(ArgStr[Src]); KillPostBlanks(ArgStr[Src]);
   p:=Length(ArgStr[Src]);
   WHILE (p>=1) AND (NOT IsBlank(ArgStr[Src][p])) DO Dec(p);
   IF p>0 THEN
    BEGIN
     FOR z:=ArgCnt DOWNTO Src DO ArgStr[z+1]:=ArgStr[z]; Inc(ArgCnt);
     Byte(ArgStr[Src][0]):=p-1; Delete(ArgStr[Src+1],1,p);
     KillPostBlanks(ArgStr[Src]); KillPrefBlanks(ArgStr[Src+1]);
    END;
END;

        PROCEDURE MakeCode_6812;
	Far;
VAR
   z:Integer;
   Address:LongInt;
   HReg,HCnt:Byte;
   OK:Boolean;
   Mask:Word;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1;

   { Operandengrî·e festlegen }

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

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeMotoPseudo(True) THEN Exit;
   IF DecodeMoto16Pseudo(OpSize,True) THEN Exit;

   { Anweisungen ohne Argument }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
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

   { einfacher Adre·operand }

   FOR z:=1 TO GenOrderCount DO
    WITH GenOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         ExPos:=1+Ord(Hi(Code)<>0); OpSize:=ThisOpSize;
         Mask:=MModAllIdx;
         IF MayImm THEN Mask:=Mask+MModImm;
         IF MayDir THEN Mask:=Mask+MModDir;
         IF MayExt THEN Mask:=Mask+MModExt;
         DecodeAdr(1,ArgCnt,Mask);
         IF AdrMode<>ModNone THEN
          IF Hi(Code)=0 THEN
           BEGIN
            BAsmCode[0]:=Code; CodeLen:=1;
           END
          ELSE
           BEGIN
            BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code); CodeLen:=2;
           END;
         CASE AdrMode OF
         ModImm: ;
         ModDir: Inc(BAsmCode[CodeLen-1],$10);
         ModIdx,ModIdx1,ModIdx2,ModDIdx,ModIIdx2: Inc(BAsmCode[CodeLen-1],$20);
         ModExt: Inc(BAsmCode[CodeLen-1],$30);
         END;
         IF AdrMode<>ModNone THEN
          BEGIN
	   Move(AdrVals,BAsmCode[CodeLen],AdrCnt);
           Inc(CodeLen,AdrCnt);
          END;
        END;
       Exit;
      END;

   { Arithmetik }

   FOR z:=1 TO LEAOrderCount DO
    WITH LEAOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         ExPos:=1;
         DecodeAdr(1,ArgCnt,MModIdx+MModIdx1+MModIdx2);
         IF AdrMode<>ModNone THEN
          BEGIN
           BAsmCode[0]:=Code;
           Move(AdrVals,BAsmCode[1],AdrCnt);
           CodeLen:=1+AdrCnt;
          END;
        END;
       Exit;
      END;

   IF (Memo('TBL')) OR (Memo('ETBL')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       ExPos:=2;
       DecodeAdr(1,ArgCnt,MModIdx);
       IF AdrMode=ModIdx THEN
        BEGIN
         BAsmCode[0]:=$18;
         BAsmCode[1]:=$3d+(Ord(Memo('ETBL')) SHL 1);
         Move(AdrVals,BAsmCode[2],AdrCnt);
         CodeLen:=2+AdrCnt;
        END;
      END;
     Exit;
    END;

   IF Memo('EMACS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       Address:=EvalIntExpression(ArgStr[1],Int16,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$18;
         BAsmCode[1]:=$12;
         BAsmCode[2]:=(Address SHR 8) AND $ff;
         BAsmCode[3]:=Address AND $ff;
         CodeLen:=4;
        END;
      END;
     Exit;
    END;

   { Transfer }

   IF (Memo('TFR')) OR (Memo('EXG')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF NOT DecodeReg(ArgStr[2],BAsmCode[1]) THEN WrXError(1445,ArgStr[2])
     ELSE IF NOT DecodeReg(ArgStr[1],HReg) THEN WrXError(1445,ArgStr[1])
     ELSE
      BEGIN
       BAsmCode[0]:=$b7;
       Inc(BAsmCode[1],(Ord(Memo('EXG')) SHL 7)+(HReg SHL 4));
       CodeLen:=2;
      END;
     Exit;
    END;

   IF Memo('SEX') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF NOT DecodeReg(ArgStr[2],BAsmCode[1]) THEN WrXError(1445,ArgStr[2])
     ELSE IF BAsmCode[1]<4 THEN WrXError(1445,ArgStr[2])
     ELSE IF NOT DecodeReg(ArgStr[1],HReg) THEN WrXError(1445,ArgStr[1])
     ELSE IF HReg>3 THEN WrXError(1445,ArgStr[1])
     ELSE
      BEGIN
       BAsmCode[0]:=$b7;
       Inc(BAsmCode[1],(Ord(Memo('EXG')) SHL 7)+(HReg SHL 4));
       CodeLen:=2;
      END;
     Exit;
    END;

   IF (Memo('MOVB')) OR (Memo('MOVW')) THEN
    BEGIN
     CASE ArgCnt OF
     1:Try2Split(1);
     2:BEGIN
        Try2Split(1); IF ArgCnt=2 THEN Try2Split(2);
       END;
     3:Try2Split(2);
     END;
     IF (ArgCnt<2) OR (ArgCnt>4) THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       IF ArgCnt=2 THEN HReg:=2
       ELSE IF ArgCnt=4 THEN HReg:=3
       ELSE IF ValidReg(ArgStr[2]) THEN HReg:=3
       ELSE HReg:=2;
       OpSize:=Ord(Memo('MOVW')); ExPos:=2;
       BAsmCode[0]:=$18; BAsmCode[1]:=Ord(Memo('MOVB')) SHL 3;
       DecodeAdr(1,HReg-1,MModImm+MModExt+MModIdx);
       CASE AdrMode OF
       ModImm:
        BEGIN
         Move(AdrVals,AdrVals[2],AdrCnt); HCnt:=AdrCnt;
         ExPos:=4+2*OpSize; DecodeAdr(HReg,ArgCnt,MModExt+MModIdx);
         CASE AdrMode OF
         ModExt:
          BEGIN
           Inc(BAsmCode[1],3);
           Move(AdrVals[2],BAsmCode[2],HCnt);
           Move(AdrVals,BAsmCode[2+HCnt],AdrCnt);
           CodeLen:=2+HCnt+AdrCnt;
          END;
         ModIdx:
          BEGIN
           BAsmCode[2]:=AdrVals[0];
           Move(AdrVals[2],BAsmCode[3],HCnt);
           CodeLen:=3+HCnt;
          END;
         END;
        END;
       ModExt:
        BEGIN
         Move(AdrVals,AdrVals[2],AdrCnt); HCnt:=AdrCnt;
         ExPos:=6; DecodeAdr(HReg,ArgCnt,MModExt+MModIdx);
         CASE AdrMode OF
         ModExt:
          BEGIN
           Inc(BAsmCode[1],4);
           Move(AdrVals[2],BAsmCode[2],HCnt);
           Move(AdrVals,BAsmCode[2+HCnt],AdrCnt);
           CodeLen:=2+HCnt+AdrCnt;
          END;
         ModIdx:
          BEGIN
           Inc(BAsmCode[1],1);
           BAsmCode[2]:=AdrVals[0];
           Move(AdrVals[2],BAsmCode[3],HCnt);
           CodeLen:=3+HCnt;
          END;
         END;
        END;
       ModIdx:
        BEGIN
         HCnt:=AdrVals[0];
         ExPos:=4; DecodeAdr(HReg,ArgCnt,MModExt+MModIdx);
         CASE AdrMode OF
         ModExt:
          BEGIN
           Inc(BAsmCode[1],5);
           BAsmCode[2]:=HCnt;
           Move(AdrVals,BAsmCode[3],AdrCnt);
           CodeLen:=3+AdrCnt;
          END;
         ModIdx:
          BEGIN
           Inc(BAsmCode[1],2);
           BAsmCode[2]:=HCnt;
           BAsmCode[3]:=AdrVals[0];
           CodeLen:=4;
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   { Logik }

   IF (Memo('ANDCC')) OR (Memo('ORCC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       OpSize:=0; DecodeAdr(1,1,MModImm);
       IF AdrMode=ModImm THEN
        BEGIN
         BAsmCode[0]:=$10+(Ord(Memo('ORCC')) SHL 2);
         BAsmCode[1]:=AdrVals[0];
         CodeLen:=2;
        END;
      END;
     Exit;
    END;

    IF (Memo('BSET')) OR (Memo('BCLR')) THEN
     BEGIN
      IF (ArgCnt=1) OR (ArgCnt=2) THEN Try2Split(ArgCnt);
      IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
      ELSE IF AttrPart<>'' THEN WrError(1100)
      ELSE
       BEGIN
        IF ArgStr[ArgCnt][1]='#' THEN Delete(ArgStr[ArgCnt],1,1);
        HReg:=EvalIntExpression(ArgStr[ArgCnt],UInt8,OK);
        IF OK THEN
         BEGIN
          ExPos:=2; { wg. Masken-Postbyte }
          DecodeAdr(1,ArgCnt-1,MModDir+MModExt+MModIdx+MModIdx1+MModIdx2);
          IF AdrMode<>ModNone THEN
           BEGIN
            BAsmCode[0]:=$0c+Ord(Memo('BCLR'));
            CASE AdrMode OF
            ModDir:Inc(BAsmCode[0],$40);
            ModExt:Inc(BAsmCode[0],$10);
            END;
            Move(AdrVals,BAsmCode[1],AdrCnt);
            BAsmCode[1+AdrCnt]:=HReg;
            CodeLen:=2+AdrCnt;
           END;
         END;
       END;
      Exit;
     END;

   { SprÅnge }

   FOR z:=1 TO BranchOrderCount DO
    WITH BranchOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         Address:=EvalIntExpression(ArgStr[1],UInt16,OK)-EProgCounter-2;
         IF OK THEN
          IF ((Address<-128) OR (Address>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
          ELSE
           BEGIN
            BAsmCode[0]:=Code; BAsmCode[1]:=Address AND $ff; CodeLen:=2;
           END;
        END;
       Exit;
      END
     ELSE IF Memo('L'+Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         Address:=EvalIntExpression(ArgStr[1],UInt16,OK)-EProgCounter-4;
         IF OK THEN
          BEGIN
           BAsmCode[0]:=$18;
           BAsmCode[1]:=Code;
           BAsmCode[2]:=(Address SHR 8) AND $ff;
           BAsmCode[3]:=Address AND $ff;
           CodeLen:=4;
          END;
        END;
       Exit;
      END;

   FOR z:=1 TO LoopOrderCount DO
    WITH LoopOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         IF DecodeReg8(ArgStr[1],HReg) THEN
          BEGIN
           OK:=True; IF HReg=2 THEN HReg:=4;
          END
         ELSE IF DecodeReg16(ArgStr[1],HReg) THEN
          BEGIN
           OK:=HReg<>PCReg; Inc(HReg,5);
          END
         ELSE OK:=False;
         IF NOT OK THEN WrXError(1445,ArgStr[1])
         ELSE
          BEGIN
           Address:=EvalIntExpression(ArgStr[2],UInt16,OK)-(EProgCounter+3);
           IF OK THEN
            IF ((Address<-256) OR (Address>255)) AND (NOT SymbolQuestionable) THEN WrError(1370)
            ELSE
             BEGIN
              BAsmCode[0]:=$04;
              BAsmCode[1]:=Code+HReg+((Address SHR 4) AND $10);
              BAsmCode[2]:=Address AND $ff;
              CodeLen:=3;
             END;
          END;
        END;
       Exit;
      END;

   FOR z:=1 TO JmpOrderCount DO
    WITH JmpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
         Mask:=MModAllIdx+MModExt; IF MayDir THEN Inc(Mask,MModDir);
         ExPos:=1; DecodeAdr(1,ArgCnt,Mask);
         IF AdrMode<>ModNone THEN
          BEGIN
           CASE AdrMode OF
	   ModExt:BAsmCode[0]:=Code;
           ModDir:BAsmCode[0]:=Code+1;
           ModIdx,ModIdx1,ModIdx2,ModDIdx,ModIIdx2: BAsmCode[0]:=Code-1;
           END;
           Move(AdrVals,BAsmCode[1],AdrCnt);
           CodeLen:=1+AdrCnt;
          END;
        END;
       Exit;
      END;

   IF Memo('CALL') THEN
    BEGIN
     IF (ArgCnt<1) THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgStr[1][1]='[' THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        BEGIN
         ExPos:=1; DecodeAdr(1,1,MModDIdx+MModIIdx2);
         IF AdrMode<>ModNone THEN
          BEGIN
           BAsmCode[0]:=$4b;
           Move(AdrVals,BAsmCode[1],AdrCnt);
           CodeLen:=1+AdrCnt;
          END;
        END;
      END
     ELSE
      BEGIN
       IF (ArgCnt<2) OR (ArgCnt>3) THEN WrError(1110)
       ELSE
        BEGIN
         HReg:=EvalIntExpression(ArgStr[ArgCnt],UInt8,OK);
         IF OK THEN
          BEGIN
           ExPos:=2; { wg. Seiten-Byte eins mehr }
           DecodeAdr(1,ArgCnt-1,MModExt+MModIdx+MModIdx1+MModIdx2);
           IF AdrMode<>ModNone THEN
            BEGIN
             BAsmCode[0]:=$4a+Ord(AdrMode<>ModExt);
             Move(AdrVals,BAsmCode[1],AdrCnt);
             BAsmCode[1+AdrCnt]:=HReg;
             CodeLen:=2+AdrCnt;
            END;
          END;
        END;
      END;
     Exit;
    END;

   IF (Memo('BRSET')) OR (Memo('BRCLR')) THEN
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       Try2Split(1); Try2Split(1);
      END
     ELSE IF ArgCnt=2 THEN
      BEGIN
       Try2Split(ArgCnt); Try2Split(2);
      END;
     IF (ArgCnt<3) OR (ArgCnt>4) THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       IF ArgStr[ArgCnt-1][1]='#' THEN Delete(ArgStr[ArgCnt-1],1,1);
       HReg:=EvalIntExpression(ArgStr[ArgCnt-1],UInt8,OK);
       IF OK THEN
        BEGIN
         Address:=EvalIntExpression(ArgStr[ArgCnt],UInt16,OK)-EProgCounter;
         IF OK THEN
          BEGIN
           ExPos:=3; { Opcode, Maske+Distanz }
           DecodeAdr(1,ArgCnt-2,MModDir+MModExt+MModIdx+MModIdx1+MModIdx2);
           IF AdrMode<>ModNone THEN
            BEGIN
             BAsmCode[0]:=$0e+Ord(Memo('BRCLR'));
             Move(AdrVals,BAsmCode[1],AdrCnt);
             CASE AdrMode OF
             ModDir:Inc(BAsmCode[0],$40);
             ModExt:Inc(BAsmCode[0],$10);
             END;
             BAsmCode[1+AdrCnt]:=HReg;
             Dec(Address,3+AdrCnt);
             IF ((Address<-128) OR (Address>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
             ELSE
              BEGIN
               BAsmCode[2+AdrCnt]:=Address AND $ff;
               CodeLen:=3+AdrCnt;
              END;
            END;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('TRAP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       IF ArgStr[1][1]='#' THEN Delete(ArgStr[1],1,1);
       BAsmCode[1]:=EvalIntExpression(ArgStr[1],UInt8,OK);
       IF OK THEN
        IF (BAsmCode[1]<$30) OR ((BAsmCode[1]>$39) AND (BAsmCode[1]<$40)) THEN WrError(1320)
        ELSE
         BEGIN
          BAsmCode[0]:=$18; CodeLen:=2;
         END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_6812:Boolean;
	Far;
BEGIN
   IF ActPC=SegCode THEN ChkPC_6812:=(ProgCounter>=0) AND (ProgCounter<$10000)
   ELSE ChkPC_6812:=False;
END;

        FUNCTION IsDef_6812:Boolean;
	Far;
BEGIN
   IsDef_6812:=False;
END;

        PROCEDURE SwitchFrom_6812;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_6812;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$66; NOPCode:=$a7;
   DivideChars:=','; HasAttrs:=True;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_6812; ChkPC:=ChkPC_6812; IsDef:=IsDef_6812;
   SwitchFrom:=SwitchFrom_6812; InitFields;

   SetFlag(DoPadding,DoPaddingName,False);
END;

BEGIN
   CPU6812:=AddCPU('68HC12',SwitchTo_6812);
END.
