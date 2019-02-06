{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

	UNIT Code51;

INTERFACE

        Uses StringUt,Chunks,NLS,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

{---------------------------------------------------------------------------}

TYPE
   FixedOrder=RECORD
               Name:String[4];
               MinCPU:CPUVar;
               Code:Word;
              END;


CONST
   ModNone=-1;
   ModReg=1;       MModReg=1 SHL ModReg;
   ModIReg8=2;     MModIReg8=1 SHL ModIReg8;
   ModIReg=3;      MModIReg=1 SHL ModIReg;
   ModInd=5;       MModInd=1 SHL ModInd;
   ModImm=7;       MModImm=1 SHL ModImm;
   ModImmEx=8;     MModImmEx=1 SHL ModImmEx;
   ModDir8=9;      MModDir8=1 SHL ModDir8;
   ModDir16=10;    MModDir16=1 SHL ModDir16;
   ModAcc=11;      MModAcc=1 SHL ModAcc;
   ModBit51=12;    MModBit51=1 SHL ModBit51;
   ModBit251=13;   MModBit251=1 SHL ModBit251;

   MMod51=MModReg+MModIReg8+MModImm+MModAcc+MModDir8;
   MMod251=MModIReg+MModInd+MModImmEx+MModDir16;

   AccOrderCnt=6;
   FixedOrderCnt=5;
   CondOrderCnt=13;
   BCondOrderCnt=3;

   AccReg=11;

TYPE
   FixedOrderField=ARRAY[1..FixedOrderCnt] OF FixedOrder;
   AccOrderField=ARRAY[1..AccOrderCnt] OF FixedOrder;
   CondOrderField=ARRAY[1..CondOrderCnt] OF FixedOrder;
   BCondOrderField=ARRAY[1..BCondOrderCnt] OF FixedOrder;

VAR
   FixedOrders:^FixedOrderField;
   AccOrders:^AccOrderField;
   CondOrders:^CondOrderField;
   BCondOrders:^BCondOrderField;

   AdrVals:ARRAY[0..4] OF Byte;
   AdrCnt,AdrPart,AdrSize:Byte;
   AdrMode,OpSize:ShortInt;
   MinOneIs0:Boolean;

   SrcMode,BigEndian:Boolean;

   SaveInitProc:PROCEDURE;
   CPU87C750,CPU8051,CPU8052,CPU80C320,
   CPU80501,CPU80502,CPU80504,CPU80515,CPU80517,
   CPU80251:CPUVar;

{---------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

   	PROCEDURE AddFixed(NName:String; NCode:Word; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>FixedOrderCnt THEN Halt;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NCPU;
    END;
END;

   	PROCEDURE AddAcc(NName:String; NCode:Word; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>AccOrderCnt THEN Halt;
   WITH AccOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NCPU;
    END;
END;

   	PROCEDURE AddCond(NName:String; NCode:Word; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>CondOrderCnt THEN Halt;
   WITH CondOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NCPU;
    END;
END;

   	PROCEDURE AddBCond(NName:String; NCode:Word; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>BCondOrderCnt THEN Halt;
   WITH BCondOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NCPU;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('NOP' ,$0000,CPU87C750);
   AddFixed('RET' ,$0022,CPU87C750);
   AddFixed('RETI',$0032,CPU87C750);
   AddFixed('ERET',$01aa,CPU80251);
   AddFixed('TRAP',$01b9,CPU80251);

   New(AccOrders); z:=0;
   AddAcc('DA'  ,$00d4,CPU87C750);
   AddAcc('RL'  ,$0023,CPU87C750);
   AddAcc('RLC' ,$0033,CPU87C750);
   AddAcc('RR'  ,$0003,CPU87C750);
   AddAcc('RRC' ,$0013,CPU87C750);
   AddAcc('SWAP',$00c4,CPU87C750);


   New(CondOrders); z:=0;
   AddCond('SJMP',$0080,CPU87C750);
   AddCond('JC'  ,$0040,CPU87C750);
   AddCond('JNC' ,$0050,CPU87C750);
   AddCond('JNZ' ,$0070,CPU87C750);
   AddCond('JZ'  ,$0060,CPU87C750);
   AddCond('JE'  ,$0168,CPU80251);
   AddCond('JG'  ,$0138,CPU80251);
   AddCond('JLE' ,$0128,CPU80251);
   AddCond('JNE' ,$0178,CPU80251);
   AddCond('JSG' ,$0118,CPU80251);
   AddCond('JSGE',$0158,CPU80251);
   AddCond('JSL' ,$0148,CPU80251);
   AddCond('JSLE',$0108,CPU80251);

   New(BCondOrders); z:=0;
   AddBCond('JB' ,$0020,CPU87C750);
   AddBCond('JBC',$0010,CPU87C750);
   AddBCond('JNB',$0030,CPU87C750);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(AccOrders);
   Dispose(CondOrders);
   Dispose(BCondOrders);
END;

{---------------------------------------------------------------------------}

   	PROCEDURE SetOpSize(NewSize:ShortInt);
BEGIN
   IF OpSize=-1 THEN OpSize:=NewSize
   ELSE IF OpSize<>NewSize THEN
    BEGIN
     WrError(1131); AdrMode:=ModNone; AdrCnt:=0;
    END;
END;

	FUNCTION DecodeReg(Asc:String; VAR Erg,Size:Byte):Boolean;
CONST
   Masks:ARRAY[0..2] OF Byte=(0,1,3);
VAR
   Start:Integer;
   IO:ValErgType;
BEGIN
   DecodeReg:=True;

   IF NLS_StrCaseCmp(Asc,'DPX')=0 THEN
    BEGIN
     Erg:=14; Size:=2; Exit;
    END;

   IF NLS_StrCaseCmp(Asc,'SPX')=0 THEN
    BEGIN
     Erg:=15; Size:=2; Exit;
    END;

   DecodeReg:=False;

   IF (Length(Asc)>=2) AND (UpCase(Asc[1])='R') THEN
    BEGIN
     Start:=2; Size:=0;
    END
   ELSE IF (MomCPU>=CPU80251) AND (Length(Asc)>=3) AND (UpCase(Asc[1])='W') AND (UpCase(Asc[2])='R') THEN
    BEGIN
     Start:=3; Size:=1;
    END
   ELSE IF (MomCPU>=CPU80251) AND (Length(Asc)>=3) AND (UpCase(Asc[1])='D') AND (UpCase(Asc[2])='R') THEN
    BEGIN
     Start:=3; Size:=2;
    END
   ELSE Exit;

   Val(Copy(Asc,Start,Length(Asc)-Start+1),Erg,IO);
   IF IO<>0 THEN Exit
   ELSE IF Erg AND Masks[Size]<>0 THEN Exit
   ELSE
    BEGIN
     Erg:=Erg SHR Size;
     CASE Size OF
     0:DecodeReg:=(Erg<8) OR ((MomCPU>=CPU80251) AND (Erg<16));
     1:DecodeReg:=Erg<16;
     2:DecodeReg:=(Erg<8) OR (Erg=14) OR (Erg=15);
     END;
    END;
END;

   	PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   AdrFound;
VAR
   OK,FirstFlag:Boolean;
   HSize:Byte;
   H16,PPos,MPos,DispPos:Integer;
   H32:LongInt;
   Part:String;
   ExtMask:Word;
BEGIN
   AdrMode:=ModNone; AdrCnt:=0;

   ExtMask:=MMod251 AND Mask;
   IF MomCPU<CPU80251 THEN Mask:=Mask AND MMod51;

   IF Asc='' THEN Exit;

   IF NLS_StrCaseCmp(Asc,'A')=0 THEN
    BEGIN
     IF (Mask AND MModAcc=0) THEN
      BEGIN
       AdrMode:=ModReg; AdrPart:=AccReg;
      END
     ELSE AdrMode:=ModAcc;
     SetOpSize(0);
     Goto AdrFound;
    END;

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     IF (OpSize=-1) AND (MinOneIs0) THEN SetOpSize(0);
     CASE OpSize OF
     -1:WrError(1132);
     0:BEGIN
        AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
        IF OK THEN
         BEGIN
          AdrMode:=ModImm; AdrCnt:=1;
         END;
       END;
     1:BEGIN
        H16:=EvalIntExpression(Asc,Int16,OK);
        IF OK THEN
         BEGIN
          AdrVals[0]:=Hi(H16); AdrVals[1]:=Lo(H16);
          AdrMode:=ModImm; AdrCnt:=2;
         END;
       END;
     2:BEGIN
        FirstPassUnknown:=False;
        H32:=EvalIntExpression(Asc,Int32,OK);
        IF FirstPassUnknown THEN H32:=H32 AND $ffff;
        IF OK THEN
         BEGIN
          AdrVals[1]:=H32 AND $ff; AdrVals[0]:=(H32 SHR 8) AND $ff;
          H32:=H32 SHR 16;
          IF H32=0 THEN AdrMode:=ModImm
          ELSE IF (H32=1) OR (H32=$ffff) THEN AdrMode:=ModImmEx
          ELSE WrError(1132);
          IF AdrMode<>ModNone THEN AdrCnt:=2;
         END;
       END;
     END;
     Goto AdrFound;
    END;

   IF DecodeReg(Asc,AdrPart,HSize) THEN
    BEGIN
     IF (MomCPU>=CPU80251) AND (Mask AND MModReg=0) THEN
      IF (HSize=0) AND (AdrPart=AccReg) THEN AdrMode:=ModAcc
      ELSE AdrMode:=ModReg
     ELSE AdrMode:=ModReg;
     SetOpSize(HSize);
     Goto AdrFound;
    END;

   IF Asc[1]='@' THEN
    BEGIN
     PPos:=Pos('+',Asc); MPos:=Pos('-',Asc);
     IF (MPos<>0) AND ((MPos<PPos) OR (PPos=0)) THEN PPos:=MPos;
     IF PPos=0 THEN PPos:=Length(Asc)+1;
     IF DecodeReg(Copy(Asc,2,PPos-2),AdrPart,HSize) THEN
      BEGIN
       DispPos:=PPos; IF Asc[DispPos]='+' THEN Inc(DispPos);
       H32:=EvalIntExpression(Copy(Asc,DispPos,Length(Asc)-DispPos+1),SInt16,OK);
       IF OK THEN
        CASE HSize OF
        0:IF (AdrPart>1) OR (H32<>0) THEN WrError(1350)
          ELSE AdrMode:=ModIReg8;
        1:IF H32=0 THEN
	   BEGIN
	    AdrMode:=ModIReg; AdrSize:=0;
           END
          ELSE
           BEGIN
            AdrMode:=ModInd; AdrSize:=0;
            AdrVals[1]:=H32 AND $ff; AdrVals[0]:=(H32 SHR 8) AND $ff;
            AdrCnt:=2;
           END;
        2:IF H32=0 THEN
	   BEGIN
	    AdrMode:=ModIReg; AdrSize:=2;
           END
          ELSE
           BEGIN
            AdrMode:=ModInd; AdrSize:=2;
            AdrVals[1]:=H32 AND $ff; AdrVals[0]:=(H32 SHR 8) AND $ff;
            AdrCnt:=2;
           END;
        END;
      END
     ELSE WrError(1350);
     Goto AdrFound;
    END;

   FirstFlag:=False;
   MPos:=-1; PPos:=QuotPos(Asc,':');
   IF PPos<=Length(Asc) THEN
    IF MomCPU<CPU80251 THEN
     BEGIN
      WrError(1350); Exit;
     END
    ELSE
     BEGIN
      SplitString(Asc,Part,Asc,PPos);
      IF NLS_StrCaseCmp(Part,'S')=0 THEN MPos:=-2
      ELSE
       BEGIN
        FirstPassUnknown:=False;
        MPos:=EvalIntExpression(Asc,UInt8,OK);
        IF NOT OK THEN Exit;
        IF FirstPassUnknown THEN FirstFlag:=True;
       END;
     END;

   FirstPassUnknown:=False;
   CASE MPos OF
   -2:BEGIN
       H32:=EvalIntExpression(Asc,UInt9,OK);
       ChkSpace(SegIO);
       IF FirstPassUnknown THEN H32:=(H32 AND $ff) OR $80;
      END;
   -1:H32:=EvalIntExpression(Asc,UInt24,OK);
   ELSE H32:=EvalIntExpression(Asc,UInt16,OK);
   END;
   IF FirstPassUnknown THEN FirstFlag:=True;


   IF (MPos=-2) OR ((MPos=-1) AND (TypeFlag AND (1 SHL SegIO)<>0)) THEN
    BEGIN
     IF ChkRange(H32,$80,$ff) THEN
      BEGIN
       AdrMode:=ModDir8; AdrVals[0]:=H32 AND $ff; AdrCnt:=1;
      END;
    END

   ELSE
    BEGIN
     IF MPos>=0 THEN Inc(H32,LongInt(MPos) SHL 16);
     IF FirstFlag THEN
      IF (MomCPU<CPU80251) OR (Mask AND ModDir16=0) THEN H32:=H32 AND $ff
      ELSE H32:=H32 AND $ffff;
     IF ((H32<128) OR ((H32<256) AND (MomCPU<CPU80251))) AND (Mask AND MModDir8<>0) THEN
      BEGIN
       IF MomCPU<CPU80251 THEN ChkSpace(SegData);
       AdrMode:=ModDir8; AdrVals[0]:=H32 AND $ff; AdrCnt:=1;
      END
     ELSE IF (MomCPU<CPU80251) OR (H32>$ffff) THEN WrError(1925)
     ELSE
      BEGIN
       AdrMode:=ModDir16; AdrCnt:=2;
       AdrVals[1]:=H32 AND $ff; AdrVals[0]:=(H32 SHR 8) AND $ff;
      END;
    END;

AdrFound:
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     IF ExtMask AND (1 SHL AdrMode)=0 THEN WrError(1350) ELSE WrError(1505);
     AdrCnt:=0; AdrMode:=ModNone;
    END;
END;

	FUNCTION DecodeBitAdr(Asc:String; VAR Erg:LongInt; MayShorten:Boolean):ShortInt;
VAR
   OK:Boolean;
   PPos:Integer;
BEGIN
   IF MomCPU<CPU80251 THEN
    BEGIN
     Erg:=EvalIntExpression(Asc,UInt8,OK);
     IF OK THEN
      BEGIN
       ChkSpace(SegBData);
       DecodeBitAdr:=ModBit51;
      END
     ELSE DecodeBitAdr:=ModNone;
    END
   ELSE
    BEGIN
     PPos:=RQuotPos(Asc,'.');
     IF PPos=0 THEN
      BEGIN
       FirstPassUnknown:=False;
       Erg:=EvalIntExpression(Asc,Int32,OK);
       IF FirstPassUnknown THEN Erg:=Erg AND $070000ff;
       IF Erg AND $f8ffff00<>0 THEN
        BEGIN
	 WrError(1510); OK:=False;
        END;
      END
     ELSE
      BEGIN
       DecodeAdr(Copy(Asc,1,PPos-1),MModDir8);
       IF AdrMode=ModNone THEN OK:=False
       ELSE
        BEGIN
         Erg:=EvalIntExpression(Copy(Asc,PPos+1,Length(Asc)-PPos),UInt3,OK) SHL 24;
         IF OK THEN Inc(Erg,AdrVals[0]);
        END;
      END;
     IF NOT OK THEN DecodeBitAdr:=ModNone
     ELSE IF (MayShorten) AND (NOT SrcMode) THEN
      IF Erg AND $87=$80 THEN
       BEGIN
        Erg:=(Erg AND $f8)+(Erg SHR 24); DecodeBitAdr:=ModBit51;
       END
      ELSE IF Erg AND $f0=$20 THEN
       BEGIN
        Erg:=((Erg AND $0f) SHL 3)+(Erg SHR 24); DecodeBitAdr:=ModBit51;
       END
      ELSE DecodeBitAdr:=ModBit251
     ELSE DecodeBitAdr:=ModBit251;
    END;
END;

        FUNCTION Chk504(Adr:LongInt):Boolean;
BEGIN
   Chk504:=(MomCPU=CPU80504) AND ((Adr AND $7ff)=$7fe);
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFF51Count=2;
   ONOFF51s:ARRAY[1..ONOFF51Count] OF ONOFFRec=
           ((Name:'SRCMODE';   Dest:@SrcMode;   FlagName:SrcModeName),
	    (Name:'BIGENDIAN'; Dest:@BigEndian; FlagName:BigEndianName));
VAR
   z,DSeg:Integer;
   OK:Boolean;
   s:String;
   AdrByte:Word;
   AdrLong:LongInt;
BEGIN
   DecodePseudo:=True;

   IF (Memo('SFR')) OR (Memo('SFRB')) THEN
    BEGIN
     FirstPassUnknown:=False;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF (Memo('SFRB')) AND (MomCPU>=CPU80251) THEN WrError(1500)
     ELSE
      BEGIN
       IF MomCPU>=CPU80251 THEN AdrByte:=EvalIntExpression(ArgStr[1],UInt9,OK)
       ELSE AdrByte:=EvalIntExpression(ArgStr[1],UInt8,OK);
       IF (OK) AND (NOT FirstPassUnknown) THEN
        BEGIN
         PushLocHandle(-1);
         IF MomCPU>=CPU80251 THEN DSeg:=SegIO ELSE DSeg:=SegData;
	 EnterIntSymbol(LabPart,AdrByte,DSeg,False);
	 IF MakeUseList THEN
	  IF AddChunk(SegChunks[DSeg],AdrByte,1,False) THEN WrError(90);
	 IF Memo('SFRB') THEN
	  BEGIN
           IF AdrByte>$7f THEN
            BEGIN
             IF AdrByte AND 7<>0 THEN WrError(220);
            END
           ELSE
            BEGIN
             IF AdrByte AND $e0<>$20 THEN WrError(220);
             AdrByte:=(AdrByte-$20) SHL 3;
            END;
	   FOR z:=0 TO 7 DO
	    BEGIN
	     s:=LabPart+'.'+Chr(z+AscOfs);
             EnterIntSymbol(s,AdrByte+z,SegBData,False)
	    END;
	   IF MakeUseList THEN
            IF AddChunk(SegChunks[SegBData],AdrByte,8,False) THEN WrError(90);
           ListLine:='='+HexString(AdrByte,2)+'H'+'-'+HexString(AdrByte+7,2)+'H';
          END
	 ELSE
	  BEGIN
           ListLine:='='+HexString(AdrByte,2)+'H';
	  END;
         PopLocHandle;
        END;
      END;
     Exit;
    END;

   IF Memo('BIT') THEN
    BEGIN
     IF MomCPU>=CPU80251 THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF DecodeBitAdr(ArgStr[1],AdrLong,False)=ModBit251 THEN
        BEGIN
         EnterIntSymbol(LabPart,AdrLong,SegNone,False);
         ListLine:='='+HexString(AdrLong AND $ff,2)+'H.'+HexString(AdrLong SHR 24,1);
        END;
      END
     ELSE CodeEquate(SegBData,0,$ff);
     Exit;
    END;

   IF Memo('PORT') THEN
    BEGIN
     IF MomCPU<CPU80251 THEN WrError(1500)
     ELSE CodeEquate(SegIO,0,$1ff);
     Exit;
    END;

   IF CodeONOFF(@ONOFF51s,ONOFF51Count) THEN
    BEGIN
     IF (MomCPU<CPU80251) AND (SrcMode) THEN
      BEGIN
       WrError(1500);
       SrcMode:=False;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

	FUNCTION NeedsPrefix(Opcode:Word):Boolean;
BEGIN
   NeedsPrefix:=(OpCode AND 15>=6) AND ((SrcMode) XOR (Hi(OpCode)<>0))
END;

	PROCEDURE PutCode(Opcode:Word);
BEGIN
   IF (Opcode AND 15<6) OR ((SrcMode) XOR (Hi(Opcode)=0)) THEN
    BEGIN
     BAsmCode[0]:=OpCode AND $ff; CodeLen:=1;
    END
   ELSE
    BEGIN
     BAsmCode[0]:=$a5; BAsmCode[1]:=OpCode AND $ff; CodeLen:=2;
    END;
END;

	PROCEDURE MakeCode_51;
	Far;
VAR
   OK,InvFlag,Questionable:Boolean;
   AdrWord:Word;
   AdrLong,BitLong:LongInt;
   AdrInt,z:Integer;
   Dist:LongInt;
   HReg,HSize:Byte;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1; MinOneIs0:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(BigEndian) THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE PutCode(Code);
       Exit;
      END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'C')=0) OR (NLS_StrCaseCmp(ArgStr[1],'CY')=0) THEN
      BEGIN
       CASE DecodeBitAdr(ArgStr[2],AdrLong,True) OF
       ModBit51:
        BEGIN
         PutCode($a2);
         BAsmCode[CodeLen]:=AdrLong AND $ff;
         Inc(CodeLen);
        END;
       ModBit251:
        BEGIN
         PutCode($1a9);
         BAsmCode[CodeLen]:=$a0+(AdrLong SHR 24);
         BAsmCode[CodeLen+1]:=AdrLong AND $ff;
         Inc(CodeLen,2);
        END;
       END;
      END
     ELSE IF (NLS_StrCaseCmp(ArgStr[2],'C')=0) OR (NLS_StrCaseCmp(ArgStr[2],'CY')=0) THEN
      BEGIN
       CASE DecodeBitAdr(ArgStr[1],AdrLong,True) OF
       ModBit51:
        BEGIN
         PutCode($92);
         BAsmCode[CodeLen]:=AdrLong AND $ff;
         Inc(CodeLen);
        END;
       ModBit251:
        BEGIN
         PutCode($1a9);
         BAsmCode[CodeLen]:=$90+(AdrLong SHR 24);
         BAsmCode[CodeLen+1]:=AdrLong AND $ff;
         Inc(CodeLen,2);
        END;
       END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'DPTR')=0 THEN
      BEGIN
       SetOpSize(1); DecodeAdr(ArgStr[2],MModImm);
       CASE AdrMode OF
       ModImm:
        BEGIN
         PutCode($90);
         Move(AdrVals,BAsmCode[CodeLen],AdrCnt);
         Inc(CodeLen,AdrCnt);
        END;
       END;
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModReg+MModIReg8+MModIReg+MModInd+MModDir8+MModDir16);
       CASE AdrMode OF
       ModAcc:
        BEGIN
         DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModIReg+MModInd+MModDir8+MModDir16+MModImm);
         CASE AdrMode OF
         ModReg:
          IF (AdrPart<8) AND (NOT SrcMode) THEN PutCode($e8+AdrPart)
          ELSE IF MomCPU<CPU80251 THEN WrError(1505)
          ELSE
           BEGIN
            PutCode($17c);
            BAsmCode[CodeLen]:=(AccReg SHL 4)+AdrPart;
            Inc(CodeLen);
           END;
         ModIReg8:PutCode($e6+AdrPart);
         ModIReg:
          BEGIN
           PutCode($17e);
           BAsmCode[CodeLen]:=(AdrPart SHL 4)+$09+AdrSize;
           BAsmCode[CodeLen+1]:=(AccReg SHL 4);
           Inc(CodeLen,2);
          END;
         ModInd:
          BEGIN
           PutCode($109+(AdrSize SHL 4));
           BAsmCode[CodeLen]:=(AccReg SHL 4)+AdrPart;
           Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
           Inc(CodeLen,1+AdrCnt);
          END;
         ModDir8:
          BEGIN
           PutCode($e5);
           BAsmCode[CodeLen]:=AdrVals[0];
           Inc(CodeLen);
          END;
         ModDir16:
          BEGIN
           PutCode($17e);
           BAsmCode[CodeLen]:=(AccReg SHL 4)+$03;
           Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
           Inc(CodeLen,1+AdrCnt);
          END;
         ModImm:
          BEGIN
           PutCode($74);
           BAsmCode[CodeLen]:=AdrVals[0];
           Inc(CodeLen);
          END;
         END;
        END;
       ModReg:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModIReg+MModInd+MModDir8+MModDir16+MModImm+MModImmEx);
         CASE AdrMode OF
         ModReg:
          IF (OpSize=0) AND (AdrPart=AccReg) AND (HReg<8) THEN PutCode($f8+HReg)
          ELSE IF (OpSize=0) AND (HReg=AccReg) AND (AdrPart<8) THEN PutCode($e8+AdrPart)
          ELSE IF MomCPU<CPU80251 THEN WrError(1505)
          ELSE
           BEGIN
            PutCode($17c+OpSize); IF OpSize=2 THEN Inc(BAsmCode[CodeLen-1]);
            BAsmCode[CodeLen]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen);
           END;
         ModIReg8:
          IF (OpSize<>0) OR (HReg<>AccReg) THEN WrError(1350)
          ELSE PutCode($e6+AdrPart);
         ModIReg:
          IF OpSize=0 THEN
           BEGIN
            PutCode($17e);
            BAsmCode[CodeLen]:=(AdrPart SHL 4)+$09+AdrSize;
            BAsmCode[CodeLen+1]:=HReg SHL 4;
            Inc(CodeLen,2);
           END
          ELSE IF OpSize=1 THEN
           BEGIN
            PutCode($10b);
            BAsmCode[CodeLen]:=(AdrPart SHL 4)+$08+AdrSize;
            BAsmCode[CodeLen+1]:=HReg SHL 4;
            Inc(CodeLen,2);
           END
          ELSE WrError(1350);
         ModInd:
          IF OpSize=2 THEN WrError(1350)
          ELSE
           BEGIN
            PutCode($109+(AdrSize SHL 4)+(OpSize SHL 6));
            BAsmCode[CodeLen]:=(HReg SHL 4)+AdrPart;
            Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
            Inc(CodeLen,1+AdrCnt);
           END;
         ModDir8:
          IF (OpSize=0) AND (HReg=AccReg) THEN
           BEGIN
            PutCode($e5);
            BAsmCode[CodeLen]:=AdrVals[0];
            Inc(CodeLen);
           END
          ELSE IF (OpSize=0) AND (HReg<8) AND (NOT SrcMode) THEN
           BEGIN
            PutCode($a8+HReg);
            BAsmCode[CodeLen]:=AdrVals[0];
            Inc(CodeLen);
           END
          ELSE IF MomCPU<CPU80251 THEN WrError(1505)
          ELSE
           BEGIN
            PutCode($17e);
            BAsmCode[CodeLen]:=$01+(HReg SHL 4)+(OpSize SHL 2);
            IF OpSize=2 THEN Inc(BAsmCode[CodeLen],4);
            BAsmCode[CodeLen+1]:=AdrVals[0];
            Inc(CodeLen,2);
           END;
         ModDir16:
          BEGIN
           PutCode($17e);
           BAsmCode[CodeLen]:=$03+(HReg SHL 4)+(OpSize SHL 2);
           IF OpSize=2 THEN Inc(BAsmCode[CodeLen],4);
           Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
           Inc(CodeLen,1+AdrCnt);
          END;
         ModImm:
          IF (OpSize=0) AND (HReg=AccReg) THEN
           BEGIN
            PutCode($74);
            BAsmCode[CodeLen]:=AdrVals[0];
            Inc(CodeLen);
           END
          ELSE IF (OpSize=0) AND (HReg<8) AND (NOT SrcMode) THEN
           BEGIN
            PutCode($78+HReg);
            BAsmCode[CodeLen]:=AdrVals[0];
            Inc(CodeLen);
           END
          ELSE IF MomCPU<CPU80251 THEN WrError(1505)
          ELSE
           BEGIN
            PutCode($17e);
            BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2);
            Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
            Inc(CodeLen,1+AdrCnt);
           END;
         ModImmEx:
          BEGIN
           PutCode($17e);
           BAsmCode[CodeLen]:=$0c+(HReg SHL 4);
           Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
           Inc(CodeLen,1+AdrCnt);
          END;
         END;
        END;
       ModIReg8:
        BEGIN
         SetOpSize(0); HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModAcc+MModDir8+MModImm);
         CASE AdrMode OF
          ModAcc:PutCode($f6+HReg);
          ModDir8:
           BEGIN
            PutCode($a6+HReg);
            BAsmCode[CodeLen]:=AdrVals[0];
            Inc(CodeLen);
           END;
          ModImm:
           BEGIN
            PutCode($76+HReg);
            BAsmCode[CodeLen]:=AdrVals[0];
            Inc(CodeLen);
           END;
         END;
        END;
       ModIReg:
        BEGIN
         HReg:=AdrPart; HSize:=AdrSize;
         DecodeAdr(ArgStr[2],MModReg);
         CASE AdrMode OF
         ModReg:
          IF OpSize=0 THEN
           BEGIN
            PutCode($17a);
            BAsmCode[CodeLen]:=(HReg SHL 4)+$09+HSize;
            BAsmCode[CodeLen+1]:=AdrPart SHL 4;
            Inc(CodeLen,2);
           END
          ELSE IF OpSize=1 THEN
           BEGIN
            PutCode($11b);
            BAsmCode[CodeLen]:=(HReg SHL 4)+$08+HSize;
            BAsmCode[CodeLen+1]:=AdrPart SHL 4;
            Inc(CodeLen,2);
           END
          ELSE WrError(1350)
         END;
        END;
       ModInd:
        BEGIN
         HReg:=AdrPart; HSize:=AdrSize;
	 AdrInt:=(Word(AdrVals[0]) SHL 8)+AdrVals[1];
         DecodeAdr(ArgStr[2],MModReg);
         CASE Adrmode OF
         ModReg:
          IF OpSize=2 THEN WrError(1350)
          ELSE
           BEGIN
            PutCode($119+(HSize SHL 4)+(OpSize SHL 6));
            BAsmCode[CodeLen]:=(AdrPart SHL 4)+HReg;
            BAsmCode[CodeLen+1]:=Hi(AdrInt);
            BAsmCode[CodeLen+2]:=Lo(AdrInt);
            Inc(CodeLen,3);
           END;
         END;
        END;
       ModDir8:
        BEGIN
         MinOneIs0:=True; HReg:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModDir8+MModImm);
         CASE AdrMode OF
         ModReg:
          IF (OpSize=0) AND (AdrPart=AccReg) THEN
           BEGIN
            PutCode($f5);
            BAsmCode[CodeLen]:=HReg;
            Inc(CodeLen);
           END
          ELSE IF (OpSize=0) AND (AdrPart<8) AND (NOT SrcMode) THEN
           BEGIN
            PutCode($88+AdrPart);
            BAsmCode[CodeLen]:=HReg;
            Inc(CodeLen);
           END
          ELSE IF MomCPU<CPU80251 THEN WrError(1505)
          ELSE
           BEGIN
            PutCode($17a);
            BAsmCode[CodeLen]:=$03+(AdrPart SHL 4)+(OpSize SHL 1);
            IF OpSize=2 THEN Inc(BAsmCode[CodeLen],6);
            BAsmCode[CodeLen+1]:=HReg;
            Inc(CodeLen,2);
           END;
         ModIReg8:
          BEGIN
           PutCode($86+AdrPart);
           BAsmCode[CodeLen]:=HReg;
           Inc(CodeLen);
          END;
         ModDir8:
          BEGIN
           PutCode($85);
           BAsmCode[CodeLen]:=AdrVals[0];
           BAsmCode[CodeLen+1]:=HReg;
           Inc(CodeLen,2);
          END;
         ModImm:
          BEGIN
           PutCode($75);
           BAsmCode[CodeLen]:=HReg;
           BAsmCode[CodeLen+1]:=AdrVals[0];
           Inc(CodeLen,2);
          END;
         END;
        END;
       ModDir16:
        BEGIN
         AdrInt:=(Word(AdrVals[0]) SHL 8)+AdrVals[1];
         DecodeAdr(ArgStr[2],MModReg);
         CASE AdrMode OF
         ModReg:
          BEGIN
           PutCode($17a);
           BAsmCode[CodeLen]:=$03+(AdrPArt SHL 4)+(OpSize SHL 2);
           IF OpSize=2 THEN Inc(BAsmCOde[CodeLen],4);
           BAsmCode[CodeLen+1]:=Hi(AdrInt);
           BAsmCode[CodeLen+2]:=Lo(AdrInt);
           Inc(CodeLen,3);
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
       DecodeAdr(ArgStr[1],MModAcc);
       CASE AdrMode OF
       ModAcc:
        IF NLS_StrCaseCmp(ArgStr[2],'@A+DPTR')=0 THEN PutCode($93)
        ELSE IF NLS_StrCaseCmp(ArgStr[2],'@A+PC')=0 THEN PutCode($83)
        ELSE WrError(1350);
       END;
      END;
     Exit;
    END;

   IF Memo('MOVH') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80251 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       CASE AdrMode OF
       ModReg:
        IF OpSize<>2 THEN WrError(1350)
        ELSE
         BEGIN
          HReg:=AdrPart; Dec(OpSize);
          DecodeAdr(ArgStr[2],MModImm);
          CASE AdrMode OF
          ModImm:
           BEGIN
            PutCode($17a);
            BAsmCode[CodeLen]:=$0c+(HReg SHL 4);
            Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
            Inc(CodeLen,1+AdrCnt);
           END;
          END;
         END;
       END;
      END;
     Exit;
    END;

   IF (Memo('MOVS')) OR (Memo('MOVZ')) THEN
    BEGIN
     z:=Ord(Memo('MOVS')) SHL 4;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80251 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       CASE AdrMode OF
       ModReg:
        IF OpSize<>1 THEN WrError(1350)
        ELSE
         BEGIN
          HReg:=AdrPart; Dec(OpSize);
          DecodeAdr(ArgStr[2],MModReg);
          CASE AdrMode OF
          ModReg:
           BEGIN
            PutCode($10a+z);
            BAsmCode[CodeLen]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen);
           END;
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
       z:=0;
       IF (NLS_StrCaseCmp(ArgStr[2],'A')=0) OR ((MomCPU>=CPU80251) AND (NLS_StrCaseCmp(ArgStr[2],'R11')=0)) THEN
        BEGIN
         z:=$10; ArgStr[2]:=ArgStr[1]; ArgStr[1]:='A';
        END;
       IF (NLS_StrCaseCmp(ArgStr[1],'A')<>0) AND ((MomCPU<CPU80251) OR (NLS_StrCaseCmp(ArgStr[2],'R11')<>0)) THEN WrError(1350)
       ELSE IF NLS_StrCaseCmp(ArgStr[2],'@DPTR')=0 THEN PutCode($e0+z)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[2],MModIReg8);
         CASE AdrMode OF
         ModIReg8:
	  PutCode($e2+AdrPart+z);
         END;
        END;
      END;
     Exit;
    END;

   IF (Memo('POP')) OR (Memo('PUSH')) OR (Memo('PUSHW')) THEN
    BEGIN
     z:=Ord(Memo('POP')) SHL 4;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgStr[1][1]='#' THEN
        SetOpSize(Ord(Memo('PUSHW')));
       IF Memo('POP') THEN DecodeAdr(ArgStr[1],MModDir8+MModReg)
       ELSE DecodeAdr(ArgStr[1],MModDir8+MModReg+MModImm);
       CASE AdrMode OF
       ModDir8:
        BEGIN
         PutCode($c0+z);
         BAsmCode[CodeLen]:=AdrVals[0]; Inc(CodeLen);
        END;
       ModReg:
        IF MomCPU<CPU80251 THEN WrError(1505)
        ELSE
         BEGIN
          PutCode($1ca+z);
          BAsmCode[CodeLen]:=$08+(AdrPart SHL 4)+OpSize+(Ord(OpSize=2));
          Inc(CodeLen);
         END;
       ModImm:
        IF MomCPU<CPU80251 THEN WrError(1505)
        ELSE
         BEGIN
          PutCode($1ca);
          BAsmCode[CodeLen]:=$02+(OpSize SHL 2);
          Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
          Inc(CodeLen,1+AdrCnt);
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
       DecodeAdr(ArgStr[1],MModAcc+MModReg+MModIReg8+MModDir8);
       CASE AdrMode OF
       ModAcc:
        BEGIN
         DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModDir8);
         CASE AdrMode OF
         ModReg:
          IF AdrPart>7 THEN WrError(1350)
          ELSE PutCode($c8+AdrPart);
         ModIReg8:
	  PutCode($c6+AdrPart);
         ModDir8:
          BEGIN
           PutCode($c5);
           BAsmCode[CodeLen]:=AdrVals[0];
           Inc(CodeLen);
          END;
         END;
        END;
       ModReg:
        IF (OpSize<>0) OR (AdrPart>7) THEN WrError(1350)
        ELSE
         BEGIN
          HReg:=AdrPart;
          DecodeAdr(ArgStr[2],MModAcc);
          CASE AdrMode OF
          ModAcc:
           PutCode($c8+HReg);
          END;
         END;
       ModIReg8:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModAcc);
         CASE AdrMode OF
         ModAcc:
          PutCode($c6+HReg);
         END;
        END;
       ModDir8:
        BEGIN
         HReg:=AdrVals[0];
         DecodeAdr(ArgStr[2],MModAcc);
         CASE AdrMode OF
         ModAcc:
          BEGIN
           PutCode($c5);
           BAsmCode[CodeLen]:=HReg;
           Inc(CodeLen);
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('XCHD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModIReg8);
       CASE AdrMode OF
       ModAcc:
        BEGIN
         DecodeAdr(ArgStr[2],MModIReg8);
         CASE AdrMode OF
         ModIReg8:
	  PutCode($d6+AdrPart);
         END;
        END;
       ModIReg8:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModAcc);
         CASE AdrMode OF
         ModAcc:
          PutCode($d6+HReg);
         END;
        END;
       END;
      END;
     Exit;
    END;

   { ein Operand }

   FOR z:=1 TO AccOrderCnt DO
    WITH AccOrders^[z] DO
     IF (Memo(Name)) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
        BEGIN
         DecodeAdr(ArgStr[1],MModAcc);
         CASE AdrMode OF
         ModAcc:PutCode(Code);
         END;
        END;
       Exit;
      END;

   { Arithmetik }

   IF Memo('ADD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc+MModReg);
       CASE AdrMode OF
       ModAcc:
        BEGIN
         DecodeAdr(ArgStr[2],MModImm+MModDir8+MModDir16+MModIReg8+MModIReg+MModReg);
         CASE AdrMode OF
         ModImm:
          BEGIN
           PutCode($24); BAsmCode[CodeLen]:=AdrVals[0]; Inc(CodeLen);
          END;
         ModDir8:
          BEGIN
           PutCode($25); BAsmCode[CodeLen]:=AdrVals[0]; Inc(CodeLen);
          END;
         ModDir16:
          BEGIN
           PutCode($12e);
	   BAsmCode[CodeLen]:=(AccReg SHL 4)+3;
           Move(AdrVals,BAsmCode[CodeLen+1],2);
           Inc(CodeLen,3);
          END;
         ModIReg8:
          PutCode($26+AdrPart);
         ModIReg:
          BEGIN
           PutCode($12e);
           BAsmCode[CodeLen]:=$09+AdrSize+(AdrPart SHL 4);
           BAsmCode[CodeLen+1]:=AccReg SHL 4;
           Inc(CodeLen,2);
          END;
         ModReg:
          IF (AdrPart<8) AND (NOT SrcMode) THEN PutCode($28+AdrPart)
          ELSE IF MomCPU<CPU80251 THEN WrError(1505)
          ELSE
           BEGIN
            PutCode($12c);
            BAsmCode[CodeLen]:=AdrPart+(AccReg SHL 4);
            Inc(CodeLen);
           END;
         END;
        END; { ModAcc }
       ModReg:
        IF MomCPU<CPU80251 THEN WrError(1505)
        ELSE
         BEGIN
          HReg:=AdrPart;
          DecodeAdr(ArgStr[2],MModImm+MModReg+MModDir8+MModDir16+MModIReg8+MModIReg);
          CASE AdrMode OF
          ModImm:
           IF (OpSize=0) AND (HReg=AccReg) THEN
            BEGIN
             PutCode($24);
	     BAsmCode[CodeLen]:=AdrVals[0]; Inc(CodeLen);
            END
           ELSE
            BEGIN
             PutCode($12e);
             BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2);
             Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
             Inc(CodeLen,1+AdrCnt);
            END;
          ModReg:
           BEGIN
            PutCode($12c+OpSize); IF OpSize=2 THEN Inc(BAsmCode[CodeLen-1]);
	    BAsmCode[CodeLen]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen);
           END;
          ModDir8:
           IF OpSize=2 THEN WrError(1350)
           ELSE IF (OpSize=0) AND (HReg=AccReg) THEN
            BEGIN
             PutCode($25);
             BAsmCode[CodeLen]:=AdrVals[0]; Inc(CodeLen);
            END
           ELSE
            BEGIN
             PutCode($12e);
             BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2)+1;
             BAsmCode[CodeLen+1]:=AdrVals[0];
             Inc(CodeLen,2);
            END;
          ModDir16:
           IF OpSize=2 THEN WrError(1350)
           ELSE
            BEGIN
             PutCode($12e);
             BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2)+3;
             Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
             Inc(CodeLen,1+AdrCnt);
            END;
          ModIReg8:
           IF (OpSize<>0) OR (HReg<>AccReg) THEN WrError(1350)
	   ELSE PutCode($26+AdrPart);
          ModIReg:
           IF OpSize<>0 THEN WrError(1350)
           ELSE
            BEGIN
             PutCode($12e);
             BAsmCode[CodeLen]:=$09+AdrSize+(AdrPart SHL 4);
             BAsmCode[CodeLen+1]:=HReg SHL 4;
             Inc(CodeLen,2);
            END;
          END;
         END;
        { ModReg }
       END;
      END;
     Exit;
    END;

   IF (Memo('SUB')) OR (Memo('CMP')) THEN
    BEGIN
     IF Memo('SUB') THEN z:=$90 ELSE z:=$b0;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80251 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       CASE AdrMode OF
       ModReg:
        BEGIN
         HReg:=AdrPart;
         IF Memo('CMP') THEN
	  DecodeAdr(ArgStr[2],MModImm+MModImmEx+MModReg+MModDir8+MModDir16+MModIReg)
         ELSE
          DecodeAdr(ArgStr[2],MModImm+MModReg+MModDir8+MModDir16+MModIReg);
         CASE AdrMode OF
         ModImm:
          BEGIN
           PutCode($10e+z);
           BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2);
           Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
           Inc(CodeLen,1+AdrCnt);
          END;
         ModImmEx:
          BEGIN
           PutCode($10e+z);
           BAsmCode[CodeLen]:=(HReg SHL 4)+$0c;
           Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
           Inc(CodeLen,1+AdrCnt);
          END;
         ModReg:
          BEGIN
           PutCode($10c+z+OpSize);
	   IF OpSize=2 THEN Inc(BAsmCode[CodeLen-1]);
	   BAsmCode[CodeLen]:=(HReg SHL 4)+AdrPart;
           Inc(CodeLen);
          END;
         ModDir8:
          IF OpSize=2 THEN WrError(1350)
          ELSE
           BEGIN
            PutCode($10e+z);
            BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2)+1;
            BAsmCode[CodeLen+1]:=AdrVals[0];
            Inc(CodeLen,2);
           END;
         ModDir16:
          IF OpSize=2 THEN WrError(1350)
          ELSE
           BEGIN
            PutCode($10e+z);
            BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2)+3;
            Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
            Inc(CodeLen,1+AdrCnt);
           END;
         ModIReg:
          IF OpSize<>0 THEN WrError(1350)
          ELSE
           BEGIN
            PutCode($10e+z);
            BAsmCode[CodeLen]:=$09+AdrSize+(AdrPart SHL 4);
            BAsmCode[CodeLen+1]:=HReg SHL 4;
            Inc(CodeLen,2);
           END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('ADDC')) OR (Memo('SUBB')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAcc);
       CASE AdrMode OF
       ModAcc:
        BEGIN
         IF Memo('ADDC') THEN HReg:=$30 ELSE HReg:=$90;
         DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModDir8+MModImm);
         CASE AdrMode OF
         ModReg:
	  IF AdrPart>7 THEN WrError(1350)
	  ELSE PutCode(HReg+$08+AdrPart);
         ModIReg8:PutCode(HReg+$06+AdrPart);
         ModDir8:
	  BEGIN
           PutCode(HReg+$05);
	   BAsmCode[CodeLen]:=AdrVals[0]; Inc(CodeLen);
          END;
         ModImm:
	  BEGIN
           PutCode(HReg+$04);
	   BAsmCode[CodeLen]:=AdrVals[0]; Inc(CodeLen);
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('DEC')) OR (Memo('INC')) THEN
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       ArgStr[2]:='#1'; Inc(ArgCnt);
      END;
     z:=Ord(Memo('DEC')) SHL 4;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF ArgStr[2][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       HReg:=EvalIntExpression(Copy(ArgStr[2],2,Length(ArgStr[2])-1),UInt3,OK);
       IF FirstPassUnknown THEN HReg:=1;
       IF OK THEN
        BEGIN
         OK:=True;
         IF HReg=1 THEN HReg:=0
         ELSE IF HReg=2 THEN HReg:=1
         ELSE IF HReg=4 THEN HReg:=2
         ELSE OK:=False;
         IF NOT OK THEN WrError(1320)
         ELSE IF NLS_StrCaseCmp(ArgStr[1],'DPTR')=0 THEN
          BEGIN
           IF Memo('DEC') THEN WrError(1350)
           ELSE IF HReg<>0 THEN WrError(1320)
           ELSE PutCode($a3);
          END
         ELSE
          BEGIN
           DecodeAdr(ArgStr[1],MModAcc+MModReg+MModDir8+MModIReg8);
           CASE AdrMode OF
           ModAcc:
            IF HReg=0 THEN PutCode($04+z)
            ELSE IF MomCPU<CPU80251 THEN WrError(1320)
            ELSE
             BEGIN
              PutCode($10b+z);
              BAsmCode[CodeLen]:=(AccReg SHL 4)+HReg;
              Inc(CodeLen);
             END;
           ModReg:
            IF (OpSize=0) AND (AdrPart=AccReg) AND (HReg=0) THEN PutCode($04+z)
            ELSE IF (AdrPart<8) AND (OpSize=0) AND (HReg=0) AND (NOT SrcMode) THEN PutCode($08+z+AdrPart)
            ELSE IF MomCPU<CPU80251 THEN WrError(1505)
            ELSE
             BEGIN
              PutCode($10b+z);
              BAsmCode[CodeLen]:=(AdrPart SHL 4)+(OpSize SHL 2)+HReg;
              IF OpSize=2 THEN Inc(BAsmCode[CodeLen],4);
              Inc(CodeLen);
             END;
           ModDir8:
            IF HReg<>0 THEN WrError(1320)
            ELSE
             BEGIN
              PutCode($05+z);
              BAsmCode[CodeLen]:=AdrVals[0];
              Inc(CodeLen);
             END;
           ModIReg8:
            IF HReg<>0 THEN WrError(1320)
            ELSE PutCode($06+z+AdrPart);
           END;
          END;
        END;
      END;
     Exit;
    END;

   IF (Memo('DIV')) OR (Memo('MUL')) THEN
    BEGIN
     z:=Ord(Memo('MUL')) SHL 5;
     IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
     ELSE IF ArgCnt=1 THEN
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[1],'AB')<>0 THEN WrError(1350)
       ELSE PutCode($84+z);
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg);
       CASE AdrMode OF
       ModReg:
        BEGIN
         HReg:=AdrPArt;
         DecodeAdr(ArgStr[2],MModReg);
         CASE AdrMode OF
         ModReg:
          IF MomCPU<CPU80251 THEN WrError(1505)
          ELSE IF OpSize=2 THEN WrError(1350)
          ELSE
           BEGIN
            PutCode($18c+z+OpSize);
            BAsmCode[CodeLen]:=(HReg SHL 4)+AdrPart;
            Inc(CodeLen);
           END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   { Logik }

   IF (Memo('ANL')) OR (Memo('ORL')) OR (Memo('XRL')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'C')=0) OR (NLS_StrCaseCmp(ArgStr[1],'CY')=0) THEN
      BEGIN
       IF Memo('XRL') THEN WrError(1350)
       ELSE
        BEGIN
         HReg:=Ord(Memo('ANL')) SHL 4;
         IF ArgStr[2][1]='/' THEN
          BEGIN
           InvFlag:=True; Delete(ArgStr[2],1,1);
          END
         ELSE InvFlag:=False;
         CASE DecodeBitAdr(ArgStr[2],AdrLong,True) OF
         ModBit51:
          BEGIN
           IF InvFlag THEN PutCode($a0+HReg) ELSE PutCode($72+HReg);
           BAsmCode[CodeLen]:=AdrLong AND $ff; Inc(CodeLen);
          END;
         ModBit251:
          BEGIN
           PutCode($1a9);
           IF InvFlag THEN BAsmCode[CodeLen]:=$e0+HReg+(AdrLong SHR 24)
           ELSE BAsmCode[CodeLen]:=$70+HReg+(AdrLong SHR 24);
           BAsmCode[CodeLen+1]:=AdrLong AND $ff;
           Inc(CodeLen,2);
          END;
         END;
        END;
      END
     ELSE
      BEGIN
       IF OpPart='ANL' THEN z:=$50
       ELSE IF OpPart='ORL' THEN z:=$40
       ELSE z:=$60;
       DecodeAdr(ArgStr[1],MModAcc+MModReg+MModDir8);
       CASE AdrMode OF
       ModAcc:
        BEGIN
         DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModIReg+MModDir8+MModDir16+MModImm);
         CASE AdrMode OF
         ModReg:
          IF (AdrPart<8) AND (NOT SrcMode) THEN PutCode(z+8+AdrPart)
          ELSE
           BEGIN
            PutCode(z+$10c);
            BAsmCode[CodeLen]:=AdrPart+(AccReg SHL 4); Inc(CodeLen);
           END;
         ModIReg8:
	  PutCode(z+6+AdrPart);
         ModIReg:
          BEGIN
           PutCode(z+$10e);
           BAsmCode[CodeLen]:=$09+AdrSize+(AdrPart SHL 4);
           BAsmCode[CodeLen+1]:=AccReg SHL 4;
           Inc(CodeLen,2);
          END;
         ModDir8:
          BEGIN
           PutCode(z+$05);
           BAsmCode[CodeLen]:=AdrVals[0];
           Inc(CodeLen);
          END;
         ModDir16:
          BEGIN
           PutCode($10e+z);
           BAsmCode[CodeLen]:=$03+(AccReg SHL 4);
           Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
           Inc(CodeLen,1+AdrCnt);
          END;
         ModImm:
          BEGIN
           PutCode(z+$04);
           BAsmCode[CodeLen]:=AdrVals[0];
           Inc(CodeLen);
          END;
         END;
        END;
       ModReg:
        IF MomCPU<CPU80251 THEN WrError(1350)
        ELSE
         BEGIN
          HReg:=AdrPart;
          DecodeAdr(ArgStr[2],MModReg+MModIReg8+MModIReg+MModDir8+MModDir16+MModImm);
          CASE AdrMode OF
          ModReg:
           IF OpSize=2 THEN WrError(1350)
           ELSE
            BEGIN
             PutCode(z+$10c+OpSize);
             BAsmCode[CodeLen]:=(HReg SHL 4)+AdrPart;
             Inc(CodeLen);
            END;
          ModIReg8:
           IF (OpSize<>0) OR (HReg<>AccReg) THEN WrError(1350)
           ELSE PutCode(z+$06+AdrPart);
          ModIReg:
           IF OpSize<>0 THEN WrError(1350)
           ELSE
            BEGIN
             PutCode($10e+z);
             BAsmCode[CodeLen]:=$09+AdrSize+(AdrPart SHL 4);
             BAsmCode[CodeLen+1]:=HReg SHL 4;
             Inc(CodeLen,2);
            END;
          ModDir8:
           IF (OpSize=0) AND (HReg=AccReg) THEN
            BEGIN
             PutCode($05+z);
             BAsmCode[CodeLen]:=AdrVals[0];
             Inc(CodeLen);
            END
           ELSE IF OpSize=2 THEN WrError(1350)
           ELSE
            BEGIN
             PutCode($10e+z);
             BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2)+1;
             BAsmCode[CodeLen+1]:=AdrVals[0];
             Inc(CodeLen,2);
            END;
          ModDir16:
           IF OpSize=2 THEN WrError(1350)
           ELSE
            BEGIN
             PutCode($10e+z);
             BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2)+3;
             Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
             Inc(CodeLen,1+AdrCnt);
            END;
          ModImm:
           IF (OpSize=0) AND (HReg=AccReg) THEN
            BEGIN
             PutCode($04+z);
             BAsmCode[CodeLen]:=AdrVals[0];
             Inc(CodeLen);
            END
           ELSE IF OpSize=2 THEN WrError(1350)
           ELSE
            BEGIN
             PutCode($10e+z);
             BAsmCode[CodeLen]:=(HReg SHL 4)+(OpSize SHL 2);
             Move(AdrVals,BAsmCode[CodeLen+1],AdrCnt);
             Inc(CodeLen,1+AdrCnt);
            END;
          END;
         END;
       ModDir8:
        BEGIN
         HReg:=AdrVals[0]; SetOpSize(0);
         DecodeAdr(ArgStr[2],MModAcc+MModImm);
         CASE AdrMode OF
         ModAcc:
          BEGIN
           PutCode(z+$02);
           BAsmCode[CodeLen]:=HReg; Inc(CodeLen);
          END;
         ModImm:
          BEGIN
           PutCode(z+$03);
           BAsmCode[CodeLen]:=HReg;
	   BAsmCode[CodeLen+1]:=AdrVals[0];
           Inc(CodeLen,2);
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF (Memo('CLR')) OR (Memo('CPL')) OR (Memo('SETB')) THEN
    BEGIN
     IF Memo('CLR') THEN z:=$10
     ELSE IF Memo('CPL') THEN z:=0
     ELSE z:=$20;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')=0 THEN
      BEGIN
       IF Memo('SETB') THEN WrError(1350)
       ELSE PutCode($f4-z);
      END
     ELSE IF (NLS_StrCaseCmp(ArgStr[1],'C')=0) OR (NLS_StrCaseCmp(ArgStr[1],'CY')=0) THEN
      PutCode($b3+z)
     ELSE CASE DecodeBitAdr(ArgStr[1],AdrLong,True) OF
     ModBit51:
      BEGIN
       PutCode($b2+z);
       BAsmCode[CodeLen]:=AdrLong AND $ff;
       Inc(CodeLen);
      END;
     ModBit251:
      BEGIN
       PutCode($1a9);
       BAsmCode[CodeLen]:=$b0+z+(AdrLong SHR 24);
       BAsmCode[CodeLen+1]:=AdrLong AND $ff;
       Inc(CodeLen,2);
      END;
     END;
     Exit;
    END;

   IF (Memo('SRA')) OR (Memo('SRL')) OR (Memo('SLL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU80251 THEN WrError(1500)
     ELSE
      BEGIN
       IF Memo('SLL') THEN z:=$30
       ELSE IF Memo('SRA') THEN z:=0
       ELSE z:=$10;
       DecodeAdr(ArgStr[1],MModReg);
       CASE AdrMode OF
       ModReg:
        IF OpSize=2 THEN WrError(1350)
        ELSE
         BEGIN
          PutCode($10e+z);
          BAsmCode[CodeLen]:=(AdrPart SHL 4)+(OpSize SHL 2);
          Inc(CodeLen);
         END;
       END;
      END;
     Exit;
    END;

   { Sprnge }

   IF (Memo('ACALL')) OR (Memo('AJMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
	IF (EProgCounter+2) SHR 11<>AdrLong SHR 11 THEN WrError(1910)
        ELSE IF Chk504(EProgCounter) THEN WrError(1900)
	ELSE
         BEGIN
          ChkSpace(SegCode);
          PutCode($01+(Ord(Memo('ACALL')) SHL 4)+(Hi(AdrLong) AND 7 SHL 5));
	  BAsmCode[CodeLen]:=Lo(AdrLong);
          Inc(CodeLen);
         END;
      END;
     Exit;
    END;

   IF (Memo('LCALL')) OR (Memo('LJMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU8051 THEN WrError(1500)
     ELSE IF ArgStr[1][1]='@' THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       CASE AdrMode OF
       ModIReg:
        IF AdrSize<>0 THEN WrError(1350)
        ELSE
         BEGIN
          PutCode($189+(Ord(Memo('LCALL')) SHL 4));
          BAsmCode[CodeLen]:=$04+(AdrPart SHL 4); Inc(CodeLen);
         END;
       END;
      END
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
        IF (MomCPU>=CPU80251) AND ((EProgCounter+3) SHR 16<>AdrLong SHR 16) THEN WrError(1910)
        ELSE
	BEGIN
	 ChkSpace(SegCode);
         PutCode($02+(Ord(Memo('LCALL')) SHL 4));
	 BAsmCode[CodeLen]:=(AdrLong SHR 8) AND $ff;
	 BAsmCode[CodeLen+1]:=AdrLong AND $ff;
         Inc(CodeLen,2);
        END;
      END;
     Exit;
    END;

   IF (Memo('ECALL')) OR (Memo('EJMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU80251 THEN WrError(1500)
     ELSE IF ArgStr[1][1]='@' THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       CASE AdrMode OF
       ModIReg:
        IF AdrSize<>2 THEN WrError(1350)
        ELSE
         BEGIN
          PutCode($189+(Ord(Memo('ECALL')) SHL 4));
          BAsmCode[CodeLen]:=$08+(AdrPart SHL 4); Inc(CodeLen);
         END;
       END;
      END
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
	BEGIN
	 ChkSpace(SegCode);
         PutCode($18a+(Ord(Memo('ECALL')) SHL 4));
	 BAsmCode[CodeLen]:=(AdrLong SHR 16) AND $ff;
	 BAsmCode[CodeLen+1]:=(AdrLong SHR 8) AND $ff;
	 BAsmCode[CodeLen+2]:=AdrLong AND $ff;
         Inc(CodeLen,3);
        END;
      END;
     Exit;
    END;

   FOR z:=1 TO CondOrderCnt DO
    WITH CondOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
        BEGIN
         AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
         IF OK THEN
          BEGIN
           Dec(AdrLong,EProgCounter+2+Ord(NeedsPrefix(Code)));
           IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT SymbolQuestionable) THEN WrError(1370)
           ELSE
            BEGIN
             ChkSpace(SegCode);
             PutCode(Code);
	     BAsmCode[CodeLen]:=AdrLong AND $ff; Inc(CodeLen);
            END;
          END;
        END;
       Exit;
      END;

   IF Memo('JMP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'@A+DPTR')=0 THEN PutCode($73)
     ELSE IF ArgStr[1][1]='@' THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       CASE AdrMode OF
       ModIReg:
        BEGIN
         PutCode($189);
         BAsmCode[CodeLen]:=$04+(AdrSize SHL 1)+(AdrPart SHL 4); Inc(CodeLen);
        END;
       END;
      END
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
	BEGIN
	 Dist:=AdrLong-(EProgCounter+2);
	 IF (Dist<=127) AND (Dist>=-128) THEN
	  BEGIN
           PutCode($80);
	   BAsmCode[CodeLen]:=Dist AND $ff; Inc(CodeLen);
	  END
         ELSE IF (NOT Chk504(EProgCounter)) AND (AdrLong SHR 11=(EProgCounter+2) SHR 11) THEN
	  BEGIN
	   PutCode($01+((Hi(AdrLong) AND 7) SHL 5));
	   BAsmCode[CodeLen]:=Lo(AdrLong); Inc(CodeLen);
	  END
         ELSE IF MomCPU<CPU8051 THEN WrError(1910)
	 ELSE IF (EProgCounter+3) SHR 16=AdrLong SHR 16 THEN
	  BEGIN
	   PutCode($02);
	   BAsmCode[CodeLen]:=Hi(AdrLong); BAsmCode[CodeLen+1]:=Lo(AdrLong);
           Inc(CodeLen,2);
	  END
         ELSE IF MomCPU<CPU80251 THEN WrError(1910)
         ELSE
          BEGIN
           PutCode($18a);
           BAsmCode[CodeLen]:=(AdrLong SHR 16) AND $ff;
           BAsmCode[CodeLen+1]:=(AdrLong SHR 8) AND $ff;
           BAsmCode[CodeLen+2]:=AdrLong AND $ff;
           Inc(CodeLen,3);
          END;
	END;
      END;
     Exit;
    END;

   IF Memo('CALL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE If ArgStr[1][1]='@' THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg);
       CASE AdrMode OF
       ModIReg:
        BEGIN
         PutCode($199);
         BAsmCode[CodeLen]:=$04+(AdrSize SHL 1)+(AdrPart SHL 4); Inc(CodeLen);
        END;
       END;
      END
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF OK THEN
	BEGIN
         IF (NOT Chk504(EProgCounter)) AND (AdrLong SHR 11=(EProgCounter+2) SHR 11) THEN
	  BEGIN
	   PutCode($11+((Hi(AdrLong) AND 7) SHL 5));
	   BAsmCode[CodeLen]:=Lo(AdrLong); Inc(CodeLen);
	  END
         ELSE IF MomCPU<CPU8051 THEN WrError(1910)
         ELSE IF AdrLong SHR 16<>(EProgCounter+3) SHR 16 THEN WrError(1910)
         ELSE
	  BEGIN
	   PutCode($12);
	   BAsmCode[CodeLen]:=Hi(AdrLong);
	   BAsmCode[CodeLen+1]:=Lo(AdrLong);
           Inc(CodeLen,2);
	  END;
	END;
      END;
     Exit;
    END;

   FOR z:=1 TO BCondOrderCnt DO
    WITH BCondOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         AdrLong:=EvalIntExpression(ArgStr[2],UInt24,OK);
         Questionable:=SymbolQuestionable;
         IF OK THEN
          BEGIN
           ChkSpace(SegCode);
           CASE DecodeBitAdr(ArgStr[1],BitLong,True) OF
           ModBit51:
            BEGIN
             Dec(AdrLong,EProgCounter+3+Ord(NeedsPrefix(Code)));
             IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable) THEN WrError(1370)
             ELSE
              BEGIN
               PutCode(Code);
               BAsmCode[CodeLen]:=BitLong AND $ff;
               BAsmCode[CodeLen+1]:=AdrLong AND $ff;
               Inc(CodeLen,2);
              END;
            END;
           ModBit251:
            BEGIN
             Dec(AdrLong,EProgCounter+4+Ord(NeedsPrefix($1a9)));
             IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable) THEN WrError(1370)
             ELSE
              BEGIN
               PutCode($1a9);
               BAsmCode[CodeLen]:=Code+(BitLong SHR 24);
               BAsmCode[CodeLen+1]:=BitLong AND $ff;
               BAsmCode[CodeLen+2]:=AdrLong AND $ff;
               Inc(CodeLen,3);
              END
            END;
           END;
          END;
        END;
       Exit;
      END;

   IF Memo('DJNZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[2],UInt24,OK);
       Questionable:=SymbolQuestionable;
       IF OK THEN
        BEGIN
         DecodeAdr(ArgStr[1],MModReg+MModDir8);
         CASE AdrMode OF
         ModReg:
	  IF (OpSize<>0) OR (AdrPart>7) THEN WrError(1350)
	  ELSE
	   BEGIN
	    Dec(AdrLong,EProgCounter+2+Ord(NeedsPrefix($d8+AdrPart)));
            IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable) THEN WrError(1370)
            ELSE
             BEGIN
	      PutCode($d8+AdrPart);
              BAsmCode[CodeLen]:=AdrLong AND $ff; Inc(CodeLen);
             END;
           END;
         ModDir8:
	  BEGIN
           Dec(AdrLong,EProgCounter+3+Ord(NeedsPrefix($d5)));
           IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable) THEN WrError(1370)
           ELSE
            BEGIN
             PutCode($d5);
             BAsmCode[CodeLen]:=AdrVals[0];
             BAsmCode[CodeLen+1]:=Lo(AdrLong);
             Inc(CodeLen,2);
            END;
          END;
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
       AdrLong:=EvalIntExpression(ArgStr[3],UInt24,OK);
       Questionable:=SymbolQuestionable;
       IF OK THEN
        BEGIN
	 DecodeAdr(ArgStr[1],MModAcc+MModIReg8+MModReg);
         CASE AdrMode OF
         ModAcc:
	  BEGIN
           DecodeAdr(ArgStr[2],MModDir8+MModImm);
           CASE AdrMode OF
           ModDir8:
	    BEGIN
             Dec(AdrLong,EProgCounter+3+Ord(NeedsPrefix($b5)));
             IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable) THEN WrError(1370)
             ELSE
              BEGIN
               PutCode($b5);
	       BAsmCode[CodeLen]:=AdrVals[0];
               BAsmCode[CodeLen+1]:=AdrLong AND $ff;
               Inc(CodeLen,2);
              END;
            END;
 	   ModImm:
	    BEGIN
             Dec(AdrLong,EProgCounter+3+Ord(NeedsPrefix($b5)));
             IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable) THEN WrError(1370)
             ELSE
              BEGIN
	       PutCode($b4);
	       BAsmCode[CodeLen]:=AdrVals[0];
               BAsmCode[CodeLen+1]:=AdrLong AND $ff;
               Inc(CodeLen,2);
	      END;
            END;
           END;
          END;
         ModReg:
	  IF (OpSize<>0) OR (AdrPart>7) THEN WrError(1350)
	  ELSE
	   BEGIN
            HReg:=AdrPart;
            DecodeAdr(ArgStr[2],MModImm);
            CASE AdrMode OF
            ModImm:
             BEGIN
              Dec(AdrLong,EProgCounter+3+Ord(NeedsPrefix($b8+HReg)));
              IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable) THEN WrError(1370)
              ELSE
               BEGIN
                PutCode($b8+HReg);
                BAsmCode[CodeLen]:=AdrVals[0];
                BAsmCode[CodeLen+1]:=AdrLong AND $ff;
                Inc(CodeLen,2);
               END;
             END;
            END;
           END;
         ModIReg8:
	  BEGIN
           HReg:=AdrPart; SetOpSize(0);
           DecodeAdr(ArgStr[2],MModImm);
           CASE AdrMode OF
	   ModImm:
            BEGIN
             Dec(AdrLong,EProgCounter+3+Ord(NeedsPrefix($b6+HReg)));
             IF ((AdrLong<-128) OR (AdrLong>127)) AND (NOT Questionable) THEN WrError(1370)
             ELSE
              BEGIN
               PutCode($b6+HReg);
               BAsmCode[CodeLen]:=AdrVals[0];
               BAsmCode[CodeLen+1]:=AdrLong AND $ff;
               Inc(CodeLen,2);
              END;
            END;
           END;
          END;
         END;
        END;
      END;
     Exit;
    END;

{===========================================================================}

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_51:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   IF MomCPU<CPU80251 THEN
    CASE ActPC OF
    SegCode  : IF MomCPU=CPU87C750 THEN ok:=ProgCounter<$800
               ELSE ok:=ProgCounter<$10000;
    SegXData : ok:=ProgCounter < $10000;
    SegData,
    SegBData : ok:=ProgCounter <   $100;
    SegIData : ok:=ProgCounter <   $100;
    ELSE ok:=False;
    END
   ELSE
    CASE ActPC OF
    SegCode  : ok:=ProgCounter<$1000000;
    SegIO    : ok:=ProgCounter<$200;
    ELSE ok:=False;
    END;
   ChkPC_51:=(ok) AND (ProgCounter>=0);
END;

	FUNCTION IsDef_51:Boolean;
	Far;
BEGIN
   IF MomCPU<CPU80251 THEN
    IsDef_51:=(Memo('BIT')) OR (Memo('SFR')) OR (Memo('SFRB'))
   ELSE
    IsDef_51:=(Memo('BIT')) OR (Memo('SFR')) OR (Memo('PORT'));
END;

        PROCEDURE InitPass_51;
        Far;
BEGIN
   SaveInitProc;
   SetFlag(SrcMode,SrcModeName,False);
   SetFlag(BigEndian,BigEndianName,False);
END;

        PROCEDURE SwitchFrom_51;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_51;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$31; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   IF MomCPU>=CPU80251 THEN
    BEGIN
     ValidSegs:=[SegCode,SegIO];
     Grans[SegCode ]:=1; ListGrans[SegCode ]:=1; SegInits[SegCode ]:=0;
     Grans[SegIO   ]:=1; ListGrans[SegIO   ]:=1; SegInits[SegIO   ]:=0;
    END
   ELSE
    BEGIN
     ValidSegs:=[SegCode,SegData,SegIData,SegXData,SegBData];
     Grans[SegCode ]:=1; ListGrans[SegCode ]:=1; SegInits[SegCode ]:=0;
     Grans[SegData ]:=1; ListGrans[SegData ]:=1; SegInits[SegData ]:=$30;
     Grans[SegIData]:=1; ListGrans[SegIData]:=1; SegInits[SegIData]:=$80;
     Grans[SegXData]:=1; ListGrans[SegXData]:=1; SegInits[SegXData]:=0;
     Grans[SegBData]:=1; ListGrans[SegBData]:=1; SegInits[SegBData]:=0;
    END;

   MakeCode:=MakeCode_51; ChkPC:=ChkPC_51; IsDef:=IsDef_51;

   InitFields; SwitchFrom:=SwitchFrom_51;
END;

BEGIN
   CPU87C750 :=AddCPU('87C750',SwitchTo_51);
   CPU8051   :=AddCPU('8051'  ,SwitchTo_51);
   CPU8052   :=AddCPU('8052'  ,SwitchTo_51);
   CPU80C320 :=AddCPU('80C320',SwitchTo_51);
   CPU80501  :=AddCPU('80C501',SwitchTo_51);
   CPU80502  :=AddCPU('80C502',SwitchTo_51);
   CPU80504  :=AddCPU('80C504',SwitchTo_51);
   CPU80515  :=AddCPU('80515' ,SwitchTo_51);
   CPU80517  :=AddCPU('80517' ,SwitchTo_51);
   CPU80251  :=AddCPU('80C251',SwitchTo_51);

   SaveInitProc:=InitPassProc;
   InitPassProc:=InitPass_51;
END.
