{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

{ AS - Codegeneratormodul TMS370x0 }

	UNIT Code370;

INTERFACE

        Uses StringUt,NLS,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[6];
	       Code:Word;
	      END;

CONST
   ModNone=-1;
   ModAccA=0;       MModAccA=1 SHL ModAccA;           { A }
   ModAccB=1;       MModAccB=1 SHL ModAccB;           { B }
   ModReg=2;        MModReg=1 SHL ModReg;             { Rn }
   ModPort=3;       MModPort=1 SHL ModPort;           { Pn }
   ModAbs=4;        MModAbs=1 SHL ModAbs;             { nnnn }
   ModBRel=5;       MModBRel=1 SHL ModBRel;           { nnnn(B) }
   ModSPRel=6;      MModSPRel=1 SHL ModSPRel;         { nn(SP) }
   ModIReg=7;       MModIReg=1 SHL ModIReg;           { @Rn }
   ModRegRel=8;     MModRegRel=1 SHL ModRegRel;       { nn(Rn) }
   ModImm=9;        MModImm=1 SHL ModImm;             { #nn }
   ModImmBRel=10;   MModImmBRel=1 SHL ModImmBRel;     { #nnnn(B) }
   ModImmRegRel=11; MModImmRegRel=1 SHL ModImmRegRel;

   FixedOrderCount=12;
   Rel8OrderCount=18;
   ALU1OrderCount=7;
   ALU2OrderCount=5;
   JmpOrderCount=4;
   ABRegOrderCount=14;
   BitOrderCount=5;

TYPE
   FixedOrderArray =ARRAY[1..FixedOrderCount] OF FixedOrder;
   Rel8OrderArray  =ARRAY[1..Rel8OrderCount ] OF FixedOrder;
   ALU1OrderArray  =ARRAY[1..ALU1OrderCount ] OF FixedOrder;
   ALU2OrderArray  =ARRAY[1..ALU2OrderCount ] OF FixedOrder;
   JmpOrderArray   =ARRAY[1..JmpOrderCount  ] OF FixedOrder;
   ABRegOrderArray =ARRAY[1..ABRegOrderCount] OF FixedOrder;
   BitOrderArray   =ARRAY[1..BitOrderCount  ] OF FixedOrder;

VAR
   CPU37010,CPU37020,CPU37030,CPU37040,CPU37050:CPUVar;

   OpSize:Byte;
   AdrType:ShortInt;
   AdrCnt:Byte;
   AdrVals:ARRAY[0..1] OF Byte;
   AddrRel:Boolean;

   FixedOrders:^FixedOrderArray;
   Rel8Orders:^Rel8OrderArray;
   ALU1Orders:^ALU1OrderArray;
   ALU2Orders:^ALU2OrderArray;
   JmpOrders:^JmpOrderArray;
   ABRegOrders:^ABRegOrderArray;
   BitOrders:^BitOrderArray;

	PROCEDURE InitFields;
VAR
   z:Integer;

	PROCEDURE InitFixed(NName:String; NCode:Word);
BEGIN
   IF z>FixedOrderCount THEN Exit;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitRel8(NName:String; NCode:Word);
BEGIN
   IF z>Rel8OrderCount THEN Exit;
   WITH Rel8Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitALU1(NName:String; NCode:Word);
BEGIN
   IF z>ALU1OrderCount THEN Exit;
   WITH ALU1Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitALU2(NName:String; NCode:Word);
BEGIN
   IF z>ALU2OrderCount THEN Exit;
   WITH ALU2Orders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitJmp(NName:String; NCode:Word);
BEGIN
   IF z>JmpOrderCount THEN Exit;
   WITH JmpOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitABReg(NName:String; NCode:Word);
BEGIN
   IF z>ABRegOrderCount THEN Exit;
   WITH ABRegOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

	PROCEDURE InitBit(NName:String; NCode:Word);
BEGIN
   IF z>BitOrderCount THEN Exit;
   WITH BitOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

BEGIN
   New(FixedOrders); z:=1;
   InitFixed('CLRC' ,$00b0); InitFixed('DINT' ,$f000);
   InitFixed('EINT' ,$f00c); InitFixed('EINTH',$f004);
   InitFixed('EINTL',$f008); InitFixed('IDLE' ,$00f6);
   InitFixed('LDSP' ,$00fd); InitFixed('NOP'  ,$00ff);
   InitFixed('RTI'  ,$00fa); InitFixed('RTS'  ,$00f9);
   InitFixed('SETC' ,$00f8); InitFixed('STSP' ,$00fe);

   New(Rel8Orders); z:=1;
   InitRel8('JMP',$00); InitRel8('JC' ,$03); InitRel8('JEQ',$02);
   InitRel8('JG' ,$0e); InitRel8('JGE',$0d); InitRel8('JHS',$0b);
   InitRel8('JL' ,$09); InitRel8('JLE',$0a); InitRel8('JLO',$0f);
   InitRel8('JN' ,$01); InitRel8('JNC',$07); InitRel8('JNE',$06);
   InitRel8('JNV',$0c); InitRel8('JNZ',$06); InitRel8('JP' ,$04);
   InitRel8('JPZ',$05); InitRel8('JV' ,$08); InitRel8('JZ' ,$02);

   New(ALU1Orders); z:=1;
   InitALU1('ADC', 9); InitALU1('ADD', 8);
   InitALU1('DAC',14); InitALU1('DSB',15);
   InitALU1('SBB',11); InitALU1('SUB',10); InitALU1('MPY',12);

   New(ALU2Orders); z:=1;
   InitALU2('AND' , 3); InitALU2('BTJO', 6);
   InitALU2('BTJZ', 7); InitALU2('OR'  , 4); InitALU2('XOR', 5);

   New(JmpOrders); z:=1;
   InitJmp('BR'  ,12); InitJmp('CALL' ,14);
   InitJmp('JMPL', 9); InitJmp('CALLR',15);

   New(ABRegOrders); z:=1;
   InitABReg('CLR'  , 5); InitABReg('COMPL',11); InitABReg('DEC'  , 2);
   InitABReg('INC'  , 3); InitABReg('INV'  , 4); InitABReg('POP'  , 9);
   InitABReg('PUSH' , 8); InitABReg('RL'   ,14); InitABReg('RLC'  ,15);
   InitABReg('RR'   ,12); InitABReg('RRC'  ,13); InitABReg('SWAP' , 7);
   InitABReg('XCHB' , 6); InitABReg('DJNZ' ,10);

   New(BitOrders); z:=1;
   InitBit('CMPBIT', 5); InitBit('JBIT0' , 7); InitBit('JBIT1' , 6);
   InitBit('SBIT0' , 3); InitBit('SBIT1' , 4);
END;

        PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(Rel8Orders);
   Dispose(ALU1Orders);
   Dispose(ALU2Orders);
   Dispose(JmpOrders);
   Dispose(ABRegOrders);
   Dispose(BitOrders);
END;

        FUNCTION HasDisp(VAR Asc:String):Integer;
VAR
   z,Lev:Integer;
BEGIN
   IF Asc[Length(Asc)]=')' THEN
    BEGIN
     z:=Length(Asc)-1; Lev:=0;
     WHILE (z>=1) AND (Lev<>-1) DO
      BEGIN
       CASE Asc[z] OF
       '(':Dec(Lev);
       ')':Inc(Lev);
       END;
       IF Lev<>-1 THEN Dec(z);
      END;
     IF Lev<>-1 THEN
      BEGIN
       WrError(1300); Exit;
      END;
    END
   ELSE z:=0;

   HasDisp:=z;
END;

        PROCEDURE DecodeAdr(Asc:String; Mask:Word);
LABEL
   AdrFound;
VAR
   HVal,z:Integer;
   OK:Boolean;
BEGIN
   AdrType:=ModNone; AdrCnt:=0;

   IF NLS_StrCaseCmp(Asc,'A')=0 THEN
    BEGIN
     IF (Mask AND MModAccA<>0) THEN AdrType:=ModAccA
     ELSE IF (Mask AND MModReg<>0) THEN
      BEGIN
       AdrCnt:=1; AdrVals[0]:=0; AdrType:=ModReg;
      END
     ELSE
      BEGIN
       AdrCnt:=2; AdrVals[0]:=0; AdrVals[1]:=0; AdrType:=ModAbs;
      END;
     Goto AdrFound;
    END;

   IF NLS_StrCaseCmp(Asc,'B')=0 THEN
    BEGIN
     IF (Mask AND MModAccB<>0) THEN AdrType:=ModAccB
     ELSE IF (Mask AND MModReg<>0) THEN
      BEGIN
       AdrCnt:=1; AdrVals[0]:=1; AdrType:=ModReg;
      END
     ELSE
      BEGIN
       AdrCnt:=2; AdrVals[0]:=0; AdrVals[1]:=1; AdrType:=ModAbs;
      END;
     Goto AdrFound;
    END;

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     z:=HasDisp(Asc);
     IF z=0 THEN
      BEGIN
       CASE OpSize OF
       0:AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
       1:BEGIN
          HVal:=EvalIntExpression(Asc,Int16,OK);
          AdrVals[0]:=Hi(HVal); AdrVals[1]:=Lo(HVal);
         END;
       END;
       IF OK THEN
        BEGIN
         AdrCnt:=1+OpSize; AdrType:=ModImm;
        END;
      END
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       HVal:=EvalIntExpression(Copy(Asc,1,z-1),Int16,OK);
       IF OK THEN
        BEGIN
         Delete(Asc,1,z-1);
         IF NLS_StrCaseCmp(Asc,'(B)')=0 THEN
          BEGIN
           AdrVals[0]:=Hi(HVal); AdrVals[1]:=Lo(HVal);
           AdrCnt:=2; AdrType:=ModImmBRel;
          END
         ELSE
          BEGIN
           IF FirstPassUnknown THEN HVal:=HVal AND 127;
           IF ChkRange(HVal,-128,127) THEN
            BEGIN
             AdrVals[0]:=HVal AND $ff; AdrCnt:=1;
             AdrVals[1]:=EvalIntExpression(Asc,UInt8,OK);
             IF OK THEN
              BEGIN
               AdrCnt:=2; AdrType:=ModImmRegRel;
              END;
            END;
          END;
        END;
      END;
     Goto AdrFound;
    END;

   IF Asc[1]='@' THEN
    BEGIN
     Delete(Asc,1,1);
     AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
     IF OK THEN
      BEGIN
       AdrCnt:=1; AdrType:=ModIReg;
      END;
     Goto AdrFound;
    END;

   z:=HasDisp(Asc);

   IF z=0 THEN
    BEGIN
     HVal:=EvalIntExpression(Asc,Int16,OK);
     IF OK THEN
      IF (Mask AND MModReg<>0) AND (Hi(HVal)=0) THEN
       BEGIN
	AdrVals[0]:=Lo(HVal); AdrCnt:=1; AdrType:=ModReg;
       END
      ELSE IF (Mask AND MModPort<>0) AND (Hi(HVal)=$10) THEN
       BEGIN
	AdrVals[0]:=Lo(HVal); AdrCnt:=1; AdrType:=ModPort;
       END
      ELSE
       BEGIN
	IF AddrRel THEN Dec(HVal,EProgCounter+3);
        AdrVals[0]:=Hi(HVal); AdrVals[1]:=Lo(HVal); AdrCnt:=2;
	AdrType:=ModAbs;
       END;
     Goto AdrFound;
    END
   ELSE
    BEGIN
     FirstPassUnknown:=False;
     HVal:=EvalIntExpression(Copy(Asc,1,z-1),Int16,OK);
     IF FirstPassUnknown THEN HVal:=HVal AND $7f;
     IF OK THEN
      BEGIN
       Asc:=Copy(Asc,z+1,Length(Asc)-z-1);
       IF NLS_StrCaseCmp(Asc,'B')=0 THEN
	BEGIN
	 IF AddrRel THEN Dec(HVal,EProgCounter+3);
         AdrVals[0]:=Hi(HVal); AdrVals[1]:=Lo(HVal); AdrCnt:=2;
	 AdrType:=ModBRel;
	END
       ELSE IF NLS_StrCaseCmp(Asc,'SP')=0 THEN
	BEGIN
	 IF AddrRel THEN Dec(HVal,EProgCounter+3);
	 IF (HVal>127) THEN WrError(1320)
	 ELSE IF (HVal<-128) THEN WrError(1315)
	 ELSE
	  BEGIN
	   AdrVals[0]:=HVal AND $ff; AdrCnt:=1; AdrType:=ModSPRel;
	  END;
	END
       ELSE IF ChkRange(HVal,-128,127) THEN
        BEGIN
         AdrVals[0]:=HVal AND $ff;
         AdrVals[1]:=EvalIntExpression(Asc,Int8,OK);
         IF OK THEN
          BEGIN
           AdrCnt:=2; AdrType:=ModRegRel;
          END;
        END;
      END;
     Goto AdrFound;
    END;

AdrFound:
   IF (AdrType<>-1) AND (Mask AND (Word(1) SHL AdrType)=0) THEN
    BEGIN
     WrError(1350); AdrType:=ModNone; AdrCnt:=0;
    END;
END;

	FUNCTION DecodePseudo:Boolean;
VAR
   OK:Boolean;
   Bit:Byte;
   Adr:Word;
BEGIN
   DecodePseudo:=True;

   IF Memo('DBIT') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Bit:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF (OK) AND (NOT FirstPassUnknown) THEN
	BEGIN
         IF (NLS_StrCaseCmp(ArgStr[2],'A')=0) OR (NLS_StrCaseCmp(ArgStr[2],'B')=0) THEN
	  BEGIN
	   Adr:=Ord(ArgStr[2][1])-Ord('A'); OK:=True;
	  END
	 ELSE Adr:=EvalIntExpression(ArgStr[2],Int16,OK);
	 IF (OK) AND (NOT FirstPassUnknown) THEN
	  BEGIN
           PushLocHandle(-1);
	   EnterIntSymbol(LabPart,(LongInt(Bit) SHL 16)+Adr,SegNone,False);
	   ListLine:='='+HexString(Adr,0)+':'+Chr(Bit+AscOfs);
           PopLocHandle;
	  END;
	END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_370;
	Far;
VAR
   z,AdrInt:Integer;
   Bit:LongInt;
   OK,Rela:Boolean;

	PROCEDURE PutCode(Code:Word);
BEGIN
   IF Hi(Code)=0 THEN
    BEGIN
     CodeLen:=1; BAsmCode[0]:=Code;
    END
   ELSE
    BEGIN
     CodeLen:=2; BAsmCode[0]:=Hi(Code); BAsmCode[1]:=Lo(Code);
    END;
END;

BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=0; AddrRel:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(True) THEN Exit;

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE PutCode(Code);
       Exit;
      END;

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModAccB+MModReg+MModPort+MModAbs+MmodIReg+MModBRel
			  +MModSPRel+MModRegRel+MModAccA);
       CASE AdrType OF
       ModAccA:
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg+MModAbs+MModIReg+MModBRel+MModRegRel
			    +MModSPRel+MModAccB+MModPort+MModImm);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$12; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModAbs:
	  BEGIN
	   BAsmCode[0]:=$8a; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
	  END;
	 ModIReg:
	  BEGIN
	   BAsmCode[0]:=$9a; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModBRel:
	  BEGIN
	   BAsmCode[0]:=$aa; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
	  END;
	 ModRegRel:
	  BEGIN
	   BAsmCode[0]:=$f4; BAsmCode[1]:=$ea;
	   Move(AdrVals,BAsmCode[2],2); CodeLen:=4;
	  END;
	 ModSPRel:
	  BEGIN
	   BAsmCode[0]:=$f1; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModAccB:
	  BEGIN
	   BAsmCode[0]:=$62; CodeLen:=1;
	  END;
	 ModPort:
	  BEGIN
	   BAsmCode[0]:=$80; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$22; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	END;
       ModAccB:
	BEGIN
	 DecodeAdr(ArgStr[1],MModAccA+MModReg+MModPort+MModImm);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   BAsmCode[0]:=$c0; CodeLen:=1;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$32; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModPort:
	  BEGIN
	   BAsmCode[0]:=$91; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$52; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	END;
       ModReg:
	BEGIN
	 BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModAccA+MModAccB+MModReg+MModPort+MModImm);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   BAsmCode[0]:=$d0; CodeLen:=2;
	  END;
	 ModAccB:
	  BEGIN
	   BAsmCode[0]:=$d1; CodeLen:=2;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$42; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 ModPort:
	  BEGIN
	   BAsmCode[0]:=$a2; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$72; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 END;
	END;
       ModPort:
	BEGIN
	 BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModAccA+MModAccB+MModReg+MModImm);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   BAsmCode[0]:=$21; CodeLen:=2;
	  END;
	 ModAccB:
	  BEGIN
	   BAsmCode[0]:=$51; CodeLen:=2;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$71; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$f7; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 END;
	END;
       ModAbs:
	BEGIN
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 DecodeAdr(ArgStr[1],MModAccA);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$8b; CodeLen:=3;
	  END;
	END;
       ModIReg:
	BEGIN
	 BAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModAccA);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$9b; CodeLen:=2;
	  END;
	END;
       ModBRel:
	BEGIN
	 Move(AdrVals,BAsmCode[1],AdrCnt);
	 DecodeAdr(ArgStr[1],MModAccA);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$ab; CodeLen:=3;
	  END;
	END;
       ModSPRel:
	BEGIN
	 BAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModAccA);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$f2; CodeLen:=2;
	  END;
	END;
       ModRegRel:
	BEGIN
	 Move(AdrVals,BAsmCode[2],AdrCnt);
	 DecodeAdr(ArgStr[1],MModAccA);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$f4; BAsmCode[1]:=$eb; CodeLen:=4;
	  END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('MOVW') THEN
    BEGIN
     OpSize:=1;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       IF AdrType<>ModNone THEN
	BEGIN
	 z:=AdrVals[0];
         DecodeAdr(ArgStr[1],MModReg+MModImm+MModImmBRel+MModImmRegRel);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$98; BAsmCode[1]:=AdrVals[0]; BAsmCode[2]:=z;
	   CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$88; Move(AdrVals,BAsmCode[1],2);
	   BAsmCode[3]:=z;
	   CodeLen:=4;
	  END;
         ModImmBRel:
	  BEGIN
	   BAsmCode[0]:=$a8; Move(AdrVals,BAsmCode[1],2);
	   BAsmCode[3]:=z;
	   CodeLen:=4;
	  END;
         ModImmRegRel:
	  BEGIN
	   BAsmCode[0]:=$f4; BAsmCode[1]:=$e8;
	   Move(AdrVals,BAsmCode[2],2); BAsmCode[4]:=z;
	   CodeLen:=5;
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   FOR z:=1 TO Rel8OrderCount DO
    WITH Rel8Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
         AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK)-(EProgCounter+2);
	 IF OK THEN
	  IF (AdrInt>127) OR (AdrInt<-128) THEN WrError(1370)
	  ELSE
	   BEGIN
	    CodeLen:=2;
	    BAsmCode[0]:=Code; BAsmCode[1]:=AdrInt AND $ff;
	   END;
	END;
       Exit;
      END;

   IF Memo('CMP') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModAccA+MModAccB+MModReg);
       CASE AdrType OF
       ModAccA:
	BEGIN
	 DecodeAdr(ArgStr[1],MModAbs+MModIReg+MModBRel+MModRegRel+MModSPRel+MModAccB+MModReg+MModImm);
	 CASE AdrType OF
	 ModAbs:
	  BEGIN
	   BAsmCode[0]:=$8d; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
	  END;
	 ModIReg:
	  BEGIN
	   BAsmCode[0]:=$9d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModBRel:
	  BEGIN
	   BAsmCode[0]:=$ad; Move(AdrVals,BAsmCode[1],2); CodeLen:=3;
	  END;
	 ModRegRel:
	  BEGIN
	   BAsmCode[0]:=$f4; BAsmCode[1]:=$ed;
	   Move(AdrVals,BAsmCode[2],2); CodeLen:=4;
	  END;
	 ModSPRel:
	  BEGIN
	   BAsmCode[0]:=$f3; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModAccB:
	  BEGIN
	   BAsmCode[0]:=$6d; CodeLen:=1;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$1d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$2d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	END;
       ModAccB:
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg+MModImm);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$3d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$5d; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	END;
       ModReg:
	BEGIN
	 BAsmCode[2]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModReg+MModImm);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$4d; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 ModImm:
	  BEGIN
	   BAsmCode[0]:=$7d; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	 END;
	END;
       END;
      END;
     Exit;
    END;

   FOR z:=1 TO ALU1OrderCount DO
    WITH ALU1Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2],MModAccA+MModAccB+MModReg);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   DecodeAdr(ArgStr[1],MModAccB+MModReg+MModImm);
	   CASE AdrType OF
	   ModAccB:
	    BEGIN
	     CodeLen:=1; BAsmCode[0]:=$60+Code;
	    END;
	   ModReg:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$10+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   ModImm:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$20+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   END;
	  END;
	 ModAccB:
	  BEGIN
	   DecodeAdr(ArgStr[1],MModReg+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$30+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   ModImm:
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$50+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   END;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[2]:=AdrVals[0];
	   DecodeAdr(ArgStr[1],MModReg+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     CodeLen:=3; BAsmCode[0]:=$40+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   ModImm:
	    BEGIN
	     CodeLen:=3; BAsmCode[0]:=$70+Code; BAsmCode[1]:=AdrVals[0];
	    END;
	   END;
	  END;
	 END;
	END;
       Exit;
      END;

   FOR z:=1 TO ALU2OrderCount DO
    WITH ALU2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       Rela:=(Memo('BTJO')) OR (Memo('BTJZ'));
       IF ((Rela) AND (ArgCnt<>3))
       OR ((NOT Rela) AND (ArgCnt<>2)) THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2],MModAccA+MModAccB+MModReg+MModPort);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   DecodeAdr(ArgStr[1],MModAccB+MModReg+MModImm);
	   CASE AdrType OF
	   ModAccB:
	    BEGIN
	     BAsmCode[0]:=$60+Code; CodeLen:=1;
	    END;
	   ModReg:
	    BEGIN
	     BAsmCode[0]:=$10+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=$20+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	    END;
	   END;
	  END;
	 ModAccB:
	  BEGIN
	   DecodeAdr(ArgStr[1],MModReg+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     BAsmCode[0]:=$30+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=$50+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	    END;
	   END;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[2]:=AdrVals[0];
	   DecodeAdr(ArgStr[1],MModReg+MModImm);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     BAsmCode[0]:=$40+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=$70+Code; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	    END;
	   END;
	  END;
	 ModPort:
	  BEGIN
	   BAsmCode[1]:=AdrVals[0];
	   DecodeAdr(ArgStr[1],MModAccA+MModAccB+MModImm);
	   CASE AdrType OF
	   ModAccA:
	    BEGIN
	     BAsmCode[0]:=$80+Code; CodeLen:=2;
	    END;
	   ModAccB:
	    BEGIN
	     BAsmCode[0]:=$90+Code; CodeLen:=2;
	    END;
	   ModImm:
	    BEGIN
	     BAsmCode[0]:=$a0+Code; BAsmCode[2]:=BAsmCode[1];
	     BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	    END;
	   END;
	  END;
	 END;
	 IF (CodeLen<>0) AND (Rela) THEN
	  BEGIN
	   AdrInt:=EvalIntExpression(ArgStr[3],Int16,OK)-(EProgCounter+CodeLen+1);
	   IF NOT OK THEN CodeLen:=0
	   ELSE IF (AdrInt>127) OR (AdrInt<-128) THEN
	    BEGIN
	     WrError(1370); CodeLen:=0;
	    END
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen]:=AdrInt AND $ff; Inc(CodeLen);
	    END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO JmpOrderCount DO
    WITH JmpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       AddrRel:=Memo('CALLR') OR Memo('JMPL');
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModAbs+MModIReg+MModBRel+MModRegRel);
	 CASE AdrType OF
	 ModAbs:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$80+Code; Move(AdrVals,BAsmCode[1],2);
	  END;
	 ModIReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$90+Code; BAsmCode[1]:=AdrVals[0];
	  END;
	 ModBRel:
	  BEGIN
	   CodeLen:=3; BAsmCode[0]:=$a0+Code; Move(AdrVals,BAsmCode[1],2);
	  END;
	 ModRegRel:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$f4; BAsmCode[1]:=$e0+Code;
	   Move(AdrVals,BAsmCode[2],2);
	  END;
	 END;
	END;
       Exit;
      END;

   FOR z:=1 TO ABRegOrderCount DO
    WITH ABRegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ((NOT Memo('DJNZ')) AND (ArgCnt<>1))
       OR ((    Memo('DJNZ')) AND (ArgCnt<>2)) THEN WrError(1110)
       ELSE IF (NLS_StrCaseCmp(ArgStr[1],'ST')=0) THEN
	BEGIN
	 IF (Memo('PUSH')) OR (Memo('POP')) THEN
	  BEGIN
	   BAsmCode[0]:=$f3+Code; CodeLen:=1;
	  END
	 ELSE WrError(1350);
	END
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModAccA+MModAccB+MModReg);
	 CASE AdrType OF
	 ModAccA:
	  BEGIN
	   BAsmCode[0]:=$b0+Code; CodeLen:=1;
	  END;
	 ModAccB:
	  BEGIN
	   BAsmCode[0]:=$c0+Code; CodeLen:=1;
	  END;
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$d0+Code; BAsmCode[CodeLen+1]:=AdrVals[0]; CodeLen:=2;
	  END;
	 END;
	 IF (Memo('DJNZ')) AND (CodeLen<>0) THEN
	  BEGIN
	   AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK)-(EProgCounter+CodeLen+1);
	   IF NOT OK THEN CodeLen:=0
	   ELSE IF (AdrInt>127) OR (AdrInt<-128) THEN
	    BEGIN
	     WrError(1370); CodeLen:=0;
	    END
	   ELSE
	    BEGIN
	     BAsmCode[CodeLen]:=AdrInt AND $ff; Inc(CodeLen);
	    END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO BitOrderCount DO
    WITH BitOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       Rela:=(Memo('JBIT0')) OR (Memo('JBIT1'));
       IF ((Rela) AND (ArgCnt<>2))
       OR ((NOT Rela) AND (ArgCnt<>1)) THEN WrError(1110)
       ELSE
	BEGIN
	 FirstPassUnknown:=False;
	 Bit:=EvalIntExpression(ArgStr[1],Int32,OK);
	 IF OK THEN
	  BEGIN
	   IF FirstPassUnknown THEN Bit:=Bit AND $000710ff;
	   BAsmCode[1]:=1 SHL ((Bit SHR 16) AND 7);
	   BAsmCode[2]:=Lo(Bit);
	   CASE Hi(Bit) OF
	   0:BEGIN
	      BAsmCode[0]:=$70+Code; CodeLen:=3;
	     END;
	   16:BEGIN
	       BAsmCode[0]:=$a0+Code; CodeLen:=3;
	      END;
	   ELSE WrError(1350);
	   END;
	   IF (CodeLen<>0) AND (Rela) THEN
	    BEGIN
	     AdrInt:=EvalIntExpression(ArgStr[2],Int16,OK)-(EProgCounter+CodeLen+1);
	     IF NOT OK THEN CodeLen:=0
	     ELSE IF (AdrInt>127) OR (AdrInt<-128) THEN
	      BEGIN
	       WrError(1370); CodeLen:=0;
	      END
	     ELSE
	      BEGIN
	       BAsmCode[CodeLen]:=AdrInt AND $ff; Inc(CodeLen);
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
       DecodeAdr(ArgStr[2],MModAccA);
       IF AdrType<>ModNone THEN
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$f4; BAsmCode[1]:=$f8;
	   BAsmCode[2]:=AdrVals[0]; CodeLen:=3;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('INCW') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg);
       IF AdrType<>ModNone THEN
	BEGIN
	 BAsmCode[2]:=AdrVals[0];
	 DecodeAdr(ArgStr[1],MModImm);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   BAsmCode[0]:=$70; BAsmCode[1]:=AdrVals[0]; CodeLen:=3;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('LDST') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModImm);
       IF AdrType<>ModNone THEN
	BEGIN
	 BAsmCode[0]:=$f0;
	 BAsmCode[1]:=AdrVals[0];
	 CodeLen:=2;
	END;
      END;
     Exit;
    END;

   IF Memo('TRAP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[0]:=EvalIntExpression(ArgStr[1],Int4,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[0]:=$ef-BAsmCode[0]; CodeLen:=1;
	END;
      END;
     Exit;
    END;

   IF Memo('TST') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModAccA+MModAccB);
       CASE AdrType OF
       ModAccA:
	BEGIN
	 BAsmCode[0]:=$b0; CodeLen:=1;
	END;
       ModAccB:
	BEGIN
	 BAsmCode[0]:=$c6; CodeLen:=1;
	END;
       END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	FUNCTION ChkPC_370:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter < $10000;
   ELSE ok:=False;
   END;
   ChkPC_370:=(ok) AND (ProgCounter>=0);
END;

	FUNCTION IsDef_370:Boolean;
	Far;
BEGIN
   IsDef_370:=Memo('DBIT');
END;

	PROCEDURE InternSymbol_370(VAR Asc:String; VAR Erg:TempResult);
        Far;
VAR
   h:String;
   err:ValErgType;
BEGIN
   Erg.Typ:=TempNone;
   IF (Length(Asc)<2) OR ((UpCase(Asc[1])<>'R') AND (UpCase(Asc[1])<>'P')) THEN Exit;

   h:=Copy(Asc,2,Length(Asc)-1);
   IF (h[1]='0') AND (Length(h)>1) THEN h[1]:='$';
   Val(h,Erg.Int,err);
   IF (Err<>0) OR (Erg.Int<0) OR (Erg.Int>255) THEN Exit;

   Erg.Typ:=TempInt; IF UpCase(Asc[1])='P' THEN Inc(Erg.Int,$1000);
END;

        PROCEDURE SwitchFrom_370;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_370;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$49; NOPCode:=$ff;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode ]:=1; ListGrans[SegCode ]:=1; SegInits[SegCode ]:=0;

   MakeCode:=MakeCode_370; ChkPC:=ChkPC_370; IsDef:=IsDef_370;
   SwitchFrom:=SwitchFrom_370; InternSymbol:=InternSymbol_370;

   InitFields;
END;

BEGIN
   CPU37010:=AddCPU('370C010' ,SwitchTo_370);
   CPU37020:=AddCPU('370C020' ,SwitchTo_370);
   CPU37030:=AddCPU('370C030' ,SwitchTo_370);
   CPU37040:=AddCPU('370C040' ,SwitchTo_370);
   CPU37050:=AddCPU('370C050' ,SwitchTo_370);
END.
