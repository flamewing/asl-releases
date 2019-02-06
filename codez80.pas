{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

	UNIT CodeZ80;

{ AS Codegeneratormodul Zx80 }

INTERFACE
        Uses NLS,Chunks,
	     AsmDef,AsmSub,AsmPars,AsmCode,CodePseu;


IMPLEMENTATION

{---------------------------------------------------------------------------}
{ Instruktionsgruppendefinitionen }

TYPE
   BaseOrder=RECORD
	      Name:String[5];
	      MinCPU:CPUVar;
	      Len:Byte;
	      Code:Word;
	     END;

   Condition=RECORD
              Name:String[2];
              Code:Byte;
             END;

   ALUOrder=RECORD
	     Name:String[4];
	     Code:Byte;
	    END;

{---------------------------------------------------------------------------}
{ PrÑfixtyp }

TYPE
   PrefType=(Pref_IN_N,Pref_IN_W ,Pref_IB_W ,Pref_IW_W ,Pref_IB_N ,
		       Pref_IN_LW,Pref_IB_LW,Pref_IW_LW,Pref_IW_N);

CONST
   ExtFlagName=   'INEXTMODE';        { Flag-Symbolnamen }
   LWordFlagName= 'INLWORDMODE';

   ModNone=-1;
   ModReg8=1;
   ModReg16=2;
   ModIndReg16=3;
   ModImm=4;
   ModAbs=5;
   ModRef=6;
   ModInt=7;
   ModSPRel=8;

   FixedOrderCnt=53;
   AccOrderCnt=3;
   HLOrderCnt=3;

   ConditionCnt=12;

   ALUOrderCnt=5;
   ALUOrders:ARRAY[1..ALUOrderCnt] OF ALUOrder=
	     ((Name:'SUB'; Code:2),(Name:'AND'; Code:4),
	      (Name:'OR' ; Code:6),(Name:'XOR'; Code:5),
	      (Name:'CP' ; Code:7));

   ShiftOrderCnt=8;
   ShiftOrders:ARRAY[0..ShiftOrderCnt-1] OF String[4]=
               ('RLC','RRC','RL','RR','SLA','SRA','SLIA','SRL');

   BitOrderCnt=3;
   BitOrders:ARRAY[1..BitOrderCnt] OF String[3]=('BIT','RES','SET');

   IXPrefix=$dd;
   IYPrefix=$fd;

TYPE
   ConditionArray  = ARRAY [1..ConditionCnt]   OF Condition;
   FixedOrderArray = ARRAY [1..FixedOrderCnt]  OF BaseOrder;
   AccOrderArray   = ARRAY [1..AccOrderCnt]    OF BaseOrder;
   HLOrderArray    = ARRAY [1..HLOrderCnt]     OF BaseOrder;

VAR
   PrefixCnt:Byte;
   AdrPart,OpSize:Byte;
   AdrVals:ARRAY[0..3] OF Byte;
   AdrCnt:Byte;
   AdrMode:ShortInt;

   Conditions:^ConditionArray;
   FixedOrders:^FixedOrderArray;
   AccOrders:^AccOrderArray;
   HLOrders:^HLOrderArray;

   SaveInitProc:PROCEDURE;

   CPUZ80,CPUZ80U,CPUZ180,CPUZ380:CPUVar;

   MayLW,			  { Instruktion erlaubt 32 Bit }
   ExtFlag,                       { Prozessor im 4GByte-Modus ? }
   LWordFlag:Boolean;             { 32-Bit-Verarbeitung ? }

   CurrPrefix,                    { mom. explizit erzeugter PrÑfix }
   LastPrefix:PrefType;           { von der letzten Anweisung generierter PrÑfix }

{============================================================================}
{ Codetabellenerzeugung }

	PROCEDURE InitFields;
VAR
   z:Integer;

	PROCEDURE AddCondition(NewName:String; NewCode:Byte);
BEGIN
   Inc(z); IF z>ConditionCnt THEN Halt;
   WITH Conditions^[z] DO
    BEGIN
     Name:=NewName; Code:=NewCode;
    END;
END;

	PROCEDURE AddFixed(NewName:String; NewMin:CPUVar; NewLen:Byte; NewCode:Word);
BEGIN
   Inc(z); IF z>FixedOrderCnt THEN Halt;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NewName; MinCPU:=NewMin; Len:=NewLen; Code:=NewCode;
    END;
END;

	PROCEDURE AddAcc(NewName:String; NewMin:CPUVar; NewLen:Byte; NewCode:Word);
BEGIN
   Inc(z); IF z>AccOrderCnt THEN Halt;
   WITH AccOrders^[z] DO
    BEGIN
     Name:=NewName; MinCPU:=NewMin; Len:=NewLen; Code:=NewCode;
    END;
END;

	PROCEDURE AddHL(NewName:String; NewMin:CPUVar; NewLen:Byte; NewCode:Word);
BEGIN
   Inc(z); IF z>HLOrderCnt THEN Halt;
   WITH HLOrders^[z] DO
    BEGIN
     Name:=NewName; MinCPU:=NewMin; Len:=NewLen; Code:=NewCode;
    END;
END;

BEGIN
   z:=0; New(Conditions);
   AddCondition('NZ',0); AddCondition('Z' ,1);
   AddCondition('NC',2); AddCondition('C' ,3);
   AddCondition('PO',4); AddCondition('NV',4);
   AddCondition('PE',5); AddCondition('V' ,5);
   AddCondition('P' ,6); AddCondition('NS',6);
   AddCondition('M' ,7); AddCondition('S' ,7);

   z:=0; New(FixedOrders);
   AddFixed('EXX'  ,CPUZ80 ,1,$00d9); AddFixed('LDI'  ,CPUZ80 ,2,$eda0);
   AddFixed('LDIR' ,CPUZ80 ,2,$edb0); AddFixed('LDD'  ,CPUZ80 ,2,$eda8);
   AddFixed('LDDR' ,CPUZ80 ,2,$edb8); AddFixed('CPI'  ,CPUZ80 ,2,$eda1);
   AddFixed('CPIR' ,CPUZ80 ,2,$edb1); AddFixed('CPD'  ,CPUZ80 ,2,$eda9);
   AddFixed('CPDR' ,CPUZ80 ,2,$edb9); AddFixed('RLCA' ,CPUZ80 ,1,$0007);
   AddFixed('RRCA' ,CPUZ80 ,1,$000f); AddFixed('RLA'  ,CPUZ80 ,1,$0017);
   AddFixed('RRA'  ,CPUZ80 ,1,$001f); AddFixed('RLD'  ,CPUZ80 ,2,$ed6f);
   AddFixed('RRD'  ,CPUZ80 ,2,$ed67); AddFixed('DAA'  ,CPUZ80 ,1,$0027);
   AddFixed('CCF'  ,CPUZ80 ,1,$003f); AddFixed('SCF'  ,CPUZ80 ,1,$0037);
   AddFixed('NOP'  ,CPUZ80 ,1,$0000); AddFixed('HALT' ,CPUZ80 ,1,$0076);
   AddFixed('RETI' ,CPUZ80 ,2,$ed4d); AddFixed('RETN' ,CPUZ80 ,2,$ed45);
   AddFixed('INI'  ,CPUZ80 ,2,$eda2); AddFixed('INIR' ,CPUZ80 ,2,$edb2);
   AddFixed('IND'  ,CPUZ80 ,2,$edaa); AddFixed('INDR' ,CPUZ80 ,2,$edba);
   AddFixed('OUTI' ,CPUZ80 ,2,$eda3); AddFixed('OTIR' ,CPUZ80 ,2,$edb3);
   AddFixed('OUTD' ,CPUZ80 ,2,$edab); AddFixed('OTDR' ,CPUZ80 ,2,$edbb);
   AddFixed('SLP'  ,CPUZ180,2,$ed76); AddFixed('OTIM' ,CPUZ180,2,$ed83);
   AddFixed('OTIMR',CPUZ180,2,$ed93); AddFixed('OTDM' ,CPUZ180,2,$ed8b);
   AddFixed('OTDMR',CPUZ180,2,$ed9b); AddFixed('BTEST',CPUZ380,2,$edcf);
   AddFixed('EXALL',CPUZ380,2,$edd9); AddFixed('EXXX' ,CPUZ380,2,$ddd9);
   AddFixed('EXXY' ,CPUZ380,2,$fdd9); AddFixed('INDW' ,CPUZ380,2,$edea);
   AddFixed('INDRW',CPUZ380,2,$edfa); AddFixed('INIW' ,CPUZ380,2,$ede2);
   AddFixed('INIRW',CPUZ380,2,$edf2); AddFixed('LDDW' ,CPUZ380,2,$ede8);
   AddFixed('LDDRW',CPUZ380,2,$edf8); AddFixed('LDIW' ,CPUZ380,2,$ede0);
   AddFixed('LDIRW',CPUZ380,2,$edf0); AddFixed('MTEST',CPUZ380,2,$ddcf);
   AddFixed('OTDRW',CPUZ380,2,$edfb); AddFixed('OTIRW',CPUZ380,2,$edf3);
   AddFixed('OUTDW',CPUZ380,2,$edeb); AddFixed('OUTIW',CPUZ380,2,$ede3);
   AddFixed('RETB' ,CPUZ380,2,$ed55);

   z:=0; New(AccOrders);
   AddAcc('CPL'  ,CPUZ80 ,1,$002f); AddAcc('NEG'  ,CPUZ80 ,2,$ed44);
   AddAcc('EXTS' ,CPUZ380,2,$ed65);

   z:=0; New(HLOrders);
   AddHL('CPLW' ,CPUZ380,2,$dd2f);  AddHL('NEGW' ,CPUZ380,2,$ed54);
   AddHL('EXTSW',CPUZ380,2,$ed75);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(Conditions);
   Dispose(FixedOrders);
   Dispose(AccOrders);
   Dispose(HLOrders);
END;

{============================================================================}
{ Adre·bereiche }

	FUNCTION CodeEnd:LongInt;
BEGIN
   IF ExtFlag THEN CodeEnd:=$ffffffff
   ELSE IF MomCPU=CPUZ180 THEN CodeEnd:=$7ffff
   ELSE CodeEnd:=$ffff;
END;

	FUNCTION PortEnd:LongInt;
BEGIN
   IF ExtFlag THEN PortEnd:=$ffffffff ELSE PortEnd:=$ff;
END;

{============================================================================}
{ PrÑfix dazuaddieren }

	FUNCTION ExtendPrefix(VAR Dest:PrefType; AddArg:String):Boolean;
VAR
   SPart,IPart:Byte;
BEGIN
   NLS_UpString(AddArg);

   CASE Dest OF
   Pref_IB_N,Pref_IB_W,Pref_IB_LW:IPart:=1;
   Pref_IW_N,Pref_IW_W,Pref_IW_LW:IPart:=2;
   ELSE IPart:=0;
   END;

   CASE Dest OF
   Pref_IN_W,Pref_IB_W,Pref_IW_W:SPart:=1;
   Pref_IN_LW,Pref_IB_LW,Pref_IW_LW:SPart:=2;
   ELSE SPart:=0;
   END;

   ExtendPrefix:=True;
   IF AddArg='W' THEN            { Wortverarbeitung }
    SPart:=1
   ELSE IF AddArg='LW' THEN      { Langwortverarbeitung }
    SPart:=2
   ELSE IF AddArg='IB' THEN      { ein Byte im Argument mehr }
    IPart:=1
   ELSE IF AddArg='IW' THEN      { ein Wort im Argument mehr }
    IPart:=2
   ELSE ExtendPrefix:=False;

   CASE (IPart SHL 4)+SPart OF
   $00:Dest:=Pref_IN_N;
   $01:Dest:=Pref_IN_W;
   $02:Dest:=Pref_IN_LW;
   $10:Dest:=Pref_IB_N;
   $11:Dest:=Pref_IB_W;
   $12:Dest:=Pref_IB_LW;
   $20:Dest:=Pref_IW_N;
   $21:Dest:=Pref_IW_W;
   $22:Dest:=Pref_IW_LW;
   END;
END;

{----------------------------------------------------------------------------}
{ Code fÅr PrÑfix bilden }

	PROCEDURE GetPrefixCode(inp:PrefType; VAR b1,b2:Byte);
VAR
   z:Integer;
BEGIN
   z:=Pred(Ord(inp));
   b1:=$dd+((z AND 4) SHL 3);
   b2:=$c0+(z AND 3);
END;

{----------------------------------------------------------------------------}
{ DD-PrÑfix addieren, nur EINMAL pro Instruktion benutzen! }

	PROCEDURE ChangeDDPrefix(Add:String);
VAR
   ActPrefix:PrefType;
   z:Integer;
BEGIN
   ActPrefix:=LastPrefix;
   IF ExtendPrefix(ActPrefix,Add) THEN;
   IF LastPrefix<>ActPrefix THEN
    BEGIN
     IF LastPrefix<>Pref_IN_N THEN RetractWords(2);
     FOR z:=PrefixCnt-1 DOWNTO 0 DO BAsmCode[2+z]:=BAsmCode[z];
     Inc(PrefixCnt,2);
     GetPrefixCode(ActPrefix,BAsmCode[0],BAsmCode[1]);
    END;
END;

{----------------------------------------------------------------------------}
{ Wortgrî·e ? }


	FUNCTION InLongMode:Boolean;
BEGIN
   CASE LastPrefix OF
   Pref_IN_W,Pref_IB_W,Pref_IW_W:InLongMode:=False;
   Pref_IN_LW,Pref_IB_LW,Pref_IW_LW:InLongMode:=MayLW;
   ELSE InLongMode:=LWordFlag AND MayLW;
   END;
END;

{----------------------------------------------------------------------------}
{ absolute Adresse }

	FUNCTION EvalAbsAdrExpression(VAR inp:String; VAR OK:Boolean):LongInt;
BEGIN
   IF ExtFlag THEN EvalAbsAdrExpression:=EvalIntExpression(inp,Int32,OK)
   ELSE EvalAbsAdrExpression:=EvalIntExpression(inp,UInt16,OK);
END;

{============================================================================}
{ Adre·parser }

        FUNCTION DecodeReg8(Asc:String; VAR Erg:Byte):Boolean;
CONST
   Reg8Cnt=7;
   Reg8Names:ARRAY[1..Reg8Cnt] OF String[1]=('B','C','D','E','H','L','A');
VAR
   z:Integer;
BEGIN
   NLS_UpString(Asc);
   DecodeReg8:=False;
   FOR z:=1 TO Reg8Cnt DO
    IF Asc=Reg8Names[z] THEN
     BEGIN
      AdrMode:=ModReg8; DecodeReg8:=True;
      Erg:=z-1; IF z=7 THEN Inc(Erg);
      Exit;
     END;
END;

	PROCEDURE DecodeAdr(Asc:String);
CONST
   Reg8XCnt=4;
   Reg8XNames:ARRAY[1..Reg8XCnt] OF String[3]=('IXU','IXL','IYU','IYL');
   Reg16Cnt=6;
   Reg16Names:ARRAY[1..Reg16Cnt] OF String[2]=('BC','DE','HL','SP','IX','IY');
VAR
   z,AdrInt:Integer;
   AdrLong:LongInt;
   OK:Boolean;
BEGIN
   AdrMode:=ModNone; AdrCnt:=0; AdrPart:=0;

   { 0. Sonderregister }

   IF NLS_StrCaseCmp(Asc,'R')=0 THEN
    BEGIN
     AdrMode:=ModRef; Exit;
    END;

   IF NLS_StrCaseCmp(Asc,'I')=0 THEN
    BEGIN
     AdrMode:=ModInt; Exit;
    END;

   { 1. 8-Bit-Register ? }

   IF DecodeReg8(Asc,AdrPart) THEN
    BEGIN
     AdrMode:=ModReg8;
     Exit;
    END;

   { 1a. 8-Bit-HÑlften von IX/IY ? (nur Z380, sonst als Symbole zulassen) }

   IF (MomCPU>=CPUZ380) OR (MomCPU=CPUZ80U) THEN
    FOR z:=1 TO Reg8XCnt DO
     IF NLS_StrCaseCmp(Asc,Reg8XNames[z])=0 THEN
      BEGIN
       AdrMode:=ModReg8;
       IF z<=2 THEN BAsmCode[PrefixCnt]:=IXPrefix ELSE BAsmCode[PrefixCnt]:=IYPrefix;
       Inc(PrefixCnt);
       AdrPart:=4+((z-1) AND 1); { = H /L }
       Exit;
      END;

   { 2. 16-Bit-Register ? }

   FOR z:=1 TO Reg16Cnt DO
    IF NLS_StrCaseCmp(Asc,Reg16Names[z])=0 THEN
     BEGIN
      AdrMode:=ModReg16;
      IF z<=4 THEN AdrPart:=z-1
      ELSE
       BEGIN
	IF z=5 THEN BAsmCode[PrefixCnt]:=IXPrefix ELSE BAsmCode[PrefixCnt]:=IYPrefix;
	Inc(PrefixCnt);
	AdrPart:=2; { = HL }
       END;
      Exit;
     END;

   { 3. 16-Bit-Register indirekt ? }

   IF Length(Asc)>=4 THEN
   FOR z:=1 TO Reg16Cnt DO
    IF  (Asc[1]='(')
    AND (Asc[Length(Asc)]=')')
    AND (NLS_StrCaseCmp(Copy(Asc,2,2),Reg16Names[z])=0)
    AND (NOT(Asc[4] IN ['_','0'..'9','A'..'Z','a'..'z'])) THEN
     BEGIN
      IF z<4 THEN
       BEGIN
	IF Length(Asc)<>4 THEN
	 BEGIN
	  WrError(1350); Exit;
	 END;
	CASE z OF
	1,2:BEGIN    { BC,DE }
	     AdrMode:=ModIndReg16; AdrPart:=z-1;
	    END;
	3:BEGIN      { HL=M-Register }
	   AdrMode:=ModReg8; AdrPart:=6;
	  END;
	END;
       END
      ELSE
       BEGIN         { SP,IX,IY }
	Asc:=Copy(Asc,4,Length(Asc)-4); IF Asc[1]='+' THEN Delete(Asc,1,1);
        IF MomCPU>=CPUZ380 THEN AdrLong:=EvalIntExpression(Asc,SInt24,OK)
        ELSE AdrLong:=EvalIntExpression(Asc,SInt8,OK);
	IF OK THEN
	 BEGIN
          IF z=4 THEN AdrMode:=ModSPRel
          ELSE
           BEGIN
	    AdrMode:=ModReg8; AdrPart:=6;
	    IF z=5 THEN BAsmCode[PrefixCnt]:=IXPrefix ELSE BAsmCode[PrefixCnt]:=IYPrefix;
	    Inc(PrefixCnt);
	   END;
	  AdrVals[0]:=AdrLong AND $ff; AdrCnt:=1;
          IF (AdrLong>=-$80) AND (AdrLong<=$7f) THEN
	  ELSE
	   BEGIN
	    AdrVals[1]:=(AdrLong SHR 8) AND $ff; Inc(AdrCnt);
            IF (AdrLong>=-$8000) AND (AdrLong<=$7fff) THEN ChangeDDPrefix('IB')
	    ELSE
	     BEGIN
	      AdrVals[2]:=(AdrLong SHR 16) AND $ff; Inc(AdrCnt);
	      ChangeDDPrefix('IW');
	     END;
	   END;
	 END;
       END;
      Exit;
     END;

   { absolut ? }

   IF IsIndirect(Asc) THEN
    BEGIN
     AdrLong:=EvalAbsAdrExpression(Asc,OK);
     IF OK THEN
      BEGIN
       ChkSpace(SegCode);
       AdrMode:=ModAbs;
       AdrVals[0]:=AdrLong AND $ff;
       AdrVals[1]:=(AdrLong SHR 8) AND $ff;
       AdrCnt:=2;
       IF AdrLong AND $ffff0000=0 THEN
       ELSE
        BEGIN
         AdrVals[2]:=(AdrLong SHR 16) AND $ff; Inc(AdrCnt);
         IF AdrLong AND $ff000000=0 THEN ChangeDDPrefix('IB')
         ELSE
          BEGIN
           AdrVals[3]:=(AdrLong SHR 24) AND $ff; Inc(AdrCnt);
           ChangeDDPrefix('IW');
          END;
        END;
      END;
     Exit;
    END;

   { ...immediate }

   CASE OpSize OF
   $ff:WrError(1132);
   0:BEGIN
      AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
      IF OK THEN
       BEGIN
        AdrMode:=ModImm; AdrCnt:=1; AdrVals[1]:=0;
       END;
     END;
   1:IF InLongMode THEN
      BEGIN
       AdrLong:=EvalIntExpression(Asc,Int32,OK);
       IF OK THEN
        BEGIN
         AdrVals[0]:=Lo(AdrLong); AdrVals[1]:=Hi(AdrLong);
         AdrMode:=ModImm; AdrCnt:=2;
         IF AdrLong AND $ffff0000=0 THEN
         ELSE
          BEGIN
           AdrVals[2]:=(AdrLong SHR 16) AND $ff; Inc(AdrCnt);
           IF AdrLong AND $ff000000=0 THEN ChangeDDPrefix('IB')
           ELSE
            BEGIN
             AdrVals[3]:=(AdrLong SHR 24) AND $ff; Inc(AdrCnt);
             ChangeDDPrefix('IW');
            END;
          END;
        END;
      END
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(Asc,Int16,OK);
       IF OK THEN
        BEGIN
         AdrVals[0]:=Lo(AdrInt); AdrVals[1]:=Hi(AdrInt);
         AdrMode:=ModImm; AdrCnt:=2;
        END;
      END;
   END;
END;

{---------------------------------------------------------------------------}
{ Bedingung entschlÅsseln }

        FUNCTION DecodeCondition(Name:String; VAR Erg:Integer):Boolean;
VAR
   z:Integer;
BEGIN
   z:=0; NLS_UpString(Name);
   WHILE (z<=ConditionCnt) AND (Conditions^[z].Name<>Name) DO Inc(z);
   IF z>ConditionCnt THEN DecodeCondition:=False
   ELSE
    BEGIN
     DecodeCondition:=True; Erg:=Conditions^[z].Code;
    END;
END;

{---------------------------------------------------------------------------}
{ Sonderregister dekodieren }

        FUNCTION DecodeSFR(Inp:String; VAR Erg:Byte):Boolean;
BEGIN
   DecodeSFR:=True; NLS_UpString(Inp);
   IF Inp='SR' THEN Erg:=1
   ELSE IF Inp='XSR' THEN Erg:=5
   ELSE IF Inp='DSR' THEN Erg:=6
   ELSE IF Inp='YSR' THEN Erg:=7
   ELSE DecodeSFR:=False;
END;

{===========================================================================}

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFFZ80Count=2;
   ONOFFZ80s:ARRAY[1..ONOFFZ80Count] OF ONOFFRec=
	    ((Name:'EXTMODE';   Dest:@ExtFlag   ; FlagName:ExtFlagName   ),
	     (Name:'LWORDMODE'; Dest:@LWordFlag ; FlagName:LWordFlagName ));
BEGIN
   DecodePseudo:=True;

   IF Memo('PORT') THEN
    BEGIN
     CodeEquate(SegIO,0,PortEnd);
     Exit;
    END;

   { erweiterte Modi nur bei Z380 }

   IF CodeONOFF(@ONOFFZ80s,ONOFFZ80Count) THEN
    BEGIN
     IF MomCPU<CPUZ380 THEN
      BEGIN
       WrError(1500);
       SetFlag(ExtFlag,ExtFlagName,False);
       SetFlag(LWordFlag,LWordFlagName,False);
      END;
     Exit;
    END;

   { KompatibilitÑt zum M80 }

   IF Memo('DEFB') THEN OpPart:='DB';
   IF Memo('DEFW') THEN OpPart:='DW';

   DecodePseudo:=False;
END;

	PROCEDURE DecodeLD;
VAR
   AdrByte,HLen:Byte;
   z:Integer;
   HVals:ARRAY[0..4] OF Byte;
BEGIN
   IF ArgCnt<>2 THEN WrError(1110)
   ELSE
    BEGIN
     DecodeAdr(ArgStr[1]);
     CASE AdrMode OF
     ModReg8:
      IF AdrPart=7 THEN { LD A,... }
       BEGIN
	OpSize:=0; DecodeAdr(ArgStr[2]);
	CASE AdrMode OF
	ModReg8: { LD A,R8/RX8/(HL)/(XY+D) }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$78+AdrPart;
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	  CodeLen:=PrefixCnt+1+AdrCnt;
	 END;
	ModIndReg16: { LD A,(BC)/(DE) }
	 BEGIN
	  BAsmCode[0]:=$0a+(AdrPart SHL 4); CodeLen:=1;
	 END;
	ModImm: { LD A,imm8 }
	 BEGIN
	  BAsmCode[0]:=$3e; BAsmCode[1]:=AdrVals[0]; CodeLen:=2;
	 END;
	ModAbs: { LD a,(adr) }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$3a;
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	  CodeLen:=PrefixCnt+1+AdrCnt;
	 END;
	ModRef: { LD A,R }
	 BEGIN
	  BAsmCode[0]:=$ed; BAsmCode[1]:=$5f;
	  CodeLen:=2;
	 END;
	ModInt: { LD A,I }
	 BEGIN
	  BAsmCode[0]:=$ed; BAsmCode[1]:=$57;
	  CodeLen:=2;
	 END;
	ELSE IF AdrMode<>ModNone THEN WrError(1350);
	END;
       END
      ELSE IF (AdrPart<>6) AND (PrefixCnt=0) THEN { LD R8,... }
       BEGIN
	AdrByte:=AdrPart; OpSize:=0; DecodeAdr(ArgStr[2]);
	CASE AdrMode OF
	ModReg8: { LD R8,R8/RX8/(HL)/(XY+D) }
	 IF ((AdrByte=4) OR (AdrByte=5)) AND (PrefixCnt=1) AND (AdrCnt=0) THEN WrError(1350)
	 ELSE
	  BEGIN
	   BAsmCode[PrefixCnt]:=$40+(AdrByte SHL 3)+AdrPart;
	   Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	   CodeLen:=PrefixCnt+1+AdrCnt;
	  END;
	ModImm: { LD R8,imm8 }
	 BEGIN
	  BAsmCode[0]:=$06+(AdrByte SHL 3); BAsmCode[1]:=AdrVals[0];
	  CodeLen:=2;
	 END;
	ELSE IF AdrMode<>ModNone THEN WrError(1350);
	END;
       END
      ELSE IF (AdrPart=4) OR (AdrPart=5) THEN { LD RX8,... }
       BEGIN
	AdrByte:=AdrPart; OpSize:=0; DecodeAdr(ArgStr[2]);
	CASE AdrMode OF
	ModReg8: { LD RX8,R8/RX8 }
	 IF (AdrPart=6) THEN WrError(1350)
	 ELSE IF (AdrPart>=4) AND (AdrPart<=5) AND (PrefixCnt<>2) THEN WrError(1350)
	 ELSE IF (AdrPart>=4) AND (AdrPart<=5) AND (BAsmCode[0]<>BAsmCode[1]) THEN WrError(1350)
	 ELSE
	  BEGIN
	   IF PrefixCnt=2 THEN Dec(PrefixCnt);
	   BAsmCode[PrefixCnt]:=$40+(AdrByte SHL 3)+AdrPart;
	   CodeLen:=PrefixCnt+1;
	  END;
	ModImm: { LD RX8,imm8 }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$06+(AdrByte SHL 3);
	  BAsmCode[PrefixCnt+1]:=AdrVals[0];
	  CodeLen:=PrefixCnt+2;
	 END;
	ELSE IF AdrMode<>ModNone THEN WrError(1350);
	END;
       END
      ELSE { LD (HL)/(XY+d),... }
       BEGIN
	HLen:=AdrCnt; Move(AdrVals,HVals,AdrCnt); z:=PrefixCnt;
	IF (z=0) AND (Memo('LDW')) THEN
	 BEGIN
	  OpSize:=1; MayLW:=True;
	 END
	ELSE OpSize:=0;
	DecodeAdr(ArgStr[2]);
	CASE AdrMode OF
	ModReg8: { LD (HL)/(XY+D),R8 }
	 IF (PrefixCnt<>z) OR (AdrPart=6) THEN WrError(1350)
	 ELSE
	  BEGIN
	   BAsmCode[PrefixCnt]:=$70+AdrPart;
	   Move(HVals,BAsmCode[PrefixCnt+1],HLen);
	   CodeLen:=PrefixCnt+1+HLen;
	  END;
	ModImm: { LD (HL)/(XY+D),imm8:16:32 }
	 IF (z=0) AND (Memo('LDW')) THEN
	  IF MomCPU<CPUZ380 THEN WrError(1500)
	  ELSE
	   BEGIN
	    BAsmCode[PrefixCnt]:=$ed; BAsmcode[PrefixCnt+1]:=$36;
	    Move(AdrVals,BAsmCode[PrefixCnt+2],AdrCnt);
	    CodeLen:=PrefixCnt+2+AdrCnt;
	   END
	 ELSE
	  BEGIN
	   BAsmCode[PrefixCnt]:=$36;
	   Move(HVals,BAsmCode[1+PrefixCnt],HLen);
	   BAsmCode[PrefixCnt+1+HLen]:=AdrVals[0];
	   CodeLen:=PrefixCnt+1+HLen+AdrCnt;
	  END;
	ModReg16: { LD (HL)/(XY+D),R16/XY }
	 IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE IF AdrPart=3 THEN WrError(1350)
	 ELSE IF HLen=0 THEN
	  IF PrefixCnt=z THEN { LD (HL),R16 }
	   BEGIN
	    IF AdrPart=2 THEN AdrPart:=3;
	    BAsmCode[0]:=$fd; BAsmCode[1]:=$0f+(AdrPart SHL 4);
	    CodeLen:=2;
	   END
	  ELSE { LD (HL),XY }
	   BEGIN
	    CodeLen:=PrefixCnt+1; BAsmCode[PrefixCnt]:=$31;
	    CodeLen:=1+PrefixCnt;
	   END
	 ELSE
	  IF PrefixCnt=z THEN { LD (XY+D),R16 }
	   BEGIN
	    IF AdrPart=2 THEN AdrPart:=3;
	    BAsmCode[PrefixCnt]:=$cb;
	    Move(HVals,BAsmCode[PrefixCnt+1],HLen);
	    BAsmCode[PrefixCnt+1+HLen]:=$0b+(AdrPart SHL 4);
	    CodeLen:=PrefixCnt+1+HLen+1;
	   END
	  ELSE IF BAsmCode[0]=BAsmCode[1] THEN WrError(1350)
	  ELSE
	   BEGIN
	    Dec(PrefixCnt);
	    BAsmCode[PrefixCnt]:=$cb;
	    Move(HVals,BAsmCode[PrefixCnt+1],HLen);
	    BAsmCode[PrefixCnt+1+HLen]:=$2b;
	    CodeLen:=PrefixCnt+1+HLen+1;
	   END;
	ELSE IF AdrMode<>ModNone THEN WrError(1350);
	END;
       END;
     ModReg16:
      IF AdrPart=3 THEN { LD SP,... }
       BEGIN
	OpSize:=1; MayLW:=True; DecodeAdr(ArgStr[2]);
	CASE AdrMode OF
	ModReg16: { LD SP,HL/XY }
	 IF AdrPart<>2 THEN WrError(1350)
	 ELSE
	  BEGIN
	   BAsmCode[PrefixCnt]:=$f9; CodeLen:=PrefixCnt+1;
	  END;
	ModImm: { LD SP,imm16:32 }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$31;
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	  CodeLen:=PrefixCnt+1+AdrCnt;
	 END;
	ModAbs: { LD SP,(adr) }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$ed; BAsmCode[PrefixCnt+1]:=$7b;
	  Move(AdrVals,BAsmCode[PrefixCnt+2],AdrCnt);
	  CodeLen:=PrefixCnt+2+AdrCnt;
	 END;
	ELSE IF AdrMode<>ModNone THEN WrError(1350);
	END;
       END
      ELSE IF PrefixCnt=0 THEN { LD R16,... }
       BEGIN
	IF AdrPart=2 THEN AdrByte:=3 ELSE AdrByte:=AdrPart;
	OpSize:=1; MayLW:=True; DecodeAdr(ArgStr[2]);
	CASE AdrMode OF
	ModInt: { LD HL,I }
	 IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE IF AdrByte<>3 THEN WrError(1350)
	 ELSE
	  BEGIN
	   BAsmCOde[0]:=$dd; BAsmCode[1]:=$57; CodeLen:=2;
	  END;
	ModReg8:
	 IF AdrPart<>6 THEN WrError(1350)
	 ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE IF PrefixCnt=0 THEN { LD R16,(HL) }
	  BEGIN
	   BAsmCode[0]:=$dd; BAsmCode[1]:=$0f+(AdrByte SHL 4);
	   CodeLen:=2;
	  END
	 ELSE { LD R16,(XY+d) }
	  BEGIN
	   BAsmCode[PrefixCnt]:=$cb;
	   Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	   BAsmCode[PrefixCnt+1+AdrCnt]:=$03+(AdrByte SHL 4);
	   CodeLen:=PrefixCnt+1+AdrCnt+1;
	  END;
	ModReg16:
	 IF AdrPart=3 THEN WrError(1350)
	 ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE IF PrefixCnt=0 THEN { LD R16,R16 }
	  BEGIN
	   IF AdrPart=2 THEN AdrPart:=3
	   ELSE IF AdrPart=0 THEN AdrPart:=2;
	   BAsmCode[0]:=$cd+(AdrPart SHL 4);
	   BAsmCode[1]:=$02+(AdrByte SHL 4);
	   CodeLen:=2;
	  END
	 ELSE { LD R16,XY }
	  BEGIN
	   BAsmCode[PrefixCnt]:=$0b+(AdrByte SHL 4);
	   CodeLen:=PrefixCnt+1;
	  END;
	ModIndReg16: { LD R16,(R16) }
	 IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$dd;
	   BAsmCode[1]:=$0c+(AdrByte SHL 4)+AdrPart;
	  END;
	ModImm: { LD R16,imm }
	 BEGIN
	  IF AdrByte=3 THEN AdrByte:=2;
	  CodeLen:=PrefixCnt+1+AdrCnt;
	  BAsmCode[PrefixCnt]:=$01+(AdrByte SHL 4);
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	 END;
	ModAbs: { LD R16,(adr) }
	 IF AdrByte=3 THEN
	  BEGIN
	   BAsmCode[PrefixCnt]:=$2a;
	   Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	   CodeLen:=1+PrefixCnt+AdrCnt;
	  END
	 ELSE
	  BEGIN
	   BAsmCode[PrefixCnt]:=$ed;
	   BAsmCode[PrefixCnt+1]:=$4b+(AdrByte SHL 4);
	   Move(AdrVals,BAsmCode[PrefixCnt+2],AdrCnt);
	   CodeLen:=PrefixCnt+2+AdrCnt;
	  END;
	ModSPRel: { LD R16,(SP+D) }
	 IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE
	  BEGIN
	   BAsmCode[PrefixCnt]:=$dd;
	   BAsmCode[PrefixCnt+1]:=$cb;
	   Move(AdrVals,BAsmCode[PrefixCnt+2],AdrCnt);
	   BAsmCode[PrefixCnt+2+AdrCnt]:=$01+(AdrByte SHL 4);
	   CodeLen:=PrefixCnt+3+AdrCnt;
	  END;
	ELSE IF AdrMode<>ModNone THEN WrError(1350);
	END;
       END
      ELSE { LD XY,... }
       BEGIN
	OpSize:=1; MayLW:=True; DecodeAdr(ArgStr[2]);
	CASE AdrMode OF
	ModReg8:
	 IF AdrPart<>6 THEN WrError(1350)
	 ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE IF AdrCnt=0 THEN { LD XY,(HL) }
	  BEGIN
	   BAsmCode[PrefixCnt]:=$33; CodeLen:=PrefixCnt+1;
	  END
	 ELSE IF BAsmCode[0]=BAsmCode[1] THEN WrError(1350)
	 ELSE { LD XY,(XY+D) }
	  BEGIN
	   BAsmCode[0]:=BAsmCode[1]; Dec(PrefixCnt);
	   BAsmCode[PrefixCnt]:=$cb;
	   Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	   BAsmCode[PrefixCnt+1+AdrCnt]:=$23;
	   CodeLen:=PrefixCnt+1+AdrCnt+1;
	  END;
	ModReg16:
	 IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE IF AdrPart=3 THEN WrError(1350)
	 ELSE IF PrefixCnt=1 THEN { LD XY,R16 }
	  BEGIN
	   IF AdrPart=2 THEN AdrPart:=3;
	   CodeLen:=1+PrefixCnt;
	   BAsmCode[PrefixCnt]:=$07+(AdrPart SHL 4);
	  END
	 ELSE IF BAsmCode[0]=BAsmCode[1] THEN WrError(1350)
	 ELSE { LD XY,XY }
	  BEGIN
	   Dec(PrefixCnt); BAsmCode[PrefixCnt]:=$27;
	   CodeLen:=1+PrefixCnt;
	  END;
	ModIndReg16:
	 IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE { LD XY,(R16) }
	  BEGIN
	   BAsmCode[PrefixCnt]:=$03+(AdrPart SHL 4);
	   CodeLen:=PrefixCnt+1;
	  END;
	ModImm:
	 BEGIN { LD XY,imm16:32 }
	  BAsmCode[PrefixCnt]:=$21;
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	  CodeLen:=PrefixCnt+1+AdrCnt;
	 END;
	ModAbs:
	 BEGIN { LD XY,(adr) }
	  BAsmCode[PrefixCnt]:=$2a;
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	  CodeLen:=PrefixCnt+1+AdrCnt;
	 END;
	ModSPRel: { LD XY,(SP+D) }
	 IF MomCPU<CPUZ380 THEN WrError(1500)
	 ELSE
	  BEGIN
	   BAsmCode[PrefixCnt]:=$cb;
	   Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	   BAsmCode[PrefixCnt+1+AdrCnt]:=$21;
	   CodeLen:=PrefixCnt+1+AdrCnt+1;
	  END;
	ELSE IF AdrMode<>ModNone THEN WrError(1350);
	END;
       END;
     ModIndReg16:
      BEGIN
       AdrByte:=AdrPart;
       IF Memo('LDW') THEN
	BEGIN
	 OpSize:=1; MayLW:=True;
	END
       ELSE OpSize:=0;
       DecodeAdr(ArgStr[2]);
       CASE AdrMode OF
       ModReg8: { LD (R16),A }
	IF AdrPart<>7 THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=1; BAsmCode[0]:=$02+(AdrByte SHL 4);
	 END;
       ModReg16:
	IF AdrPart=3 THEN WrError(1350)
	ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	ELSE IF PrefixCnt=0 THEN { LD (R16),R16 }
	 BEGIN
	  IF AdrPart=2 THEN AdrPart:=3;
	  BAsmCode[0]:=$fd; BAsmCode[1]:=$0c+AdrByte+(AdrPart SHL 4);
	  CodeLen:=2;
	 END
	ELSE { LD (R16),XY }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$01+(AdrByte SHL 4);
	  CodeLen:=PrefixCnt+1;
	 END;
       ModImm:
	IF NOT Memo('LDW') THEN WrError(1350)
	ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	ELSE
	 BEGIN
	  BAsmCode[PrefixCnt]:=$ed;
	  BAsmCode[PrefixCnt+1]:=$06+(AdrByte SHL 4);
	  Move(AdrVals,BAsmCode[PrefixCnt+2],AdrCnt);
	  CodeLen:=PrefixCnt+2+AdrCnt;
	 END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     ModAbs:
      BEGIN
       HLen:=AdrCnt; Move(AdrVals,HVals,AdrCnt);
       OpSize:=0; DecodeAdr(ArgStr[2]);
       CASE AdrMode OF
       ModReg8: { LD (adr),A }
	IF AdrPart<>7 THEN WrError(1350)
	ELSE
	 BEGIN
	  BAsmCode[PrefixCnt]:=$32;
	  Move(HVals,BAsmCode[PrefixCnt+1],HLen);
	  CodeLen:=PrefixCnt+1+HLen;
	 END;
       ModReg16:
	IF AdrPart=2 THEN { LD (adr),HL/XY }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$22;
	  Move(HVals,BAsmCode[PrefixCnt+1],HLen);
	  CodeLen:=PrefixCnt+1+HLen;
	 END
	ELSE { LD (adr),R16 }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$ed;
	  BAsmCode[PrefixCnt+1]:=$43+(AdrPart SHL 4);
	  Move(HVals,BAsmCode[PrefixCnt+2],HLen);
	  CodeLen:=PrefixCnt+2+HLen;
	 END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350)
       END;
      END;
     ModInt:
      IF NLS_StrCaseCmp(ArgStr[2],'A')=0 THEN { LD I,A }
       BEGIN
	CodeLen:=2; BAsmCode[0]:=$ed; BAsmCode[1]:=$47;
       END
      ELSE IF NLS_StrCaseCmp(ArgStr[2],'HL')=0 THEN { LD I,HL }
       IF MomCPU<CPUZ380 THEN WrError(1500)
       ELSE
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$dd; BAsmCode[1]:=$47;
	END
      ELSE WrError(1350);
     ModRef:
      IF NLS_StrCaseCmp(ArgStr[2],'A')=0 THEN { LD R,A }
       BEGIN
	CodeLen:=2; BAsmCode[0]:=$ed; BAsmCode[1]:=$4f;
       END
      ELSE WrError(1350);
     ModSPRel:
      IF MomCPU<CPUZ380 THEN WrError(1500)
      ELSE
       BEGIN
	HLen:=AdrCnt; Move(AdrVals,HVals,AdrCnt);
	OpSize:=0; DecodeAdr(ArgStr[2]);
	CASE AdrMode OF
	ModReg16:
	 IF AdrPart=3 THEN WrError(1350)
	 ELSE IF PrefixCnt=0 THEN { LD (SP+D),R16 }
	  BEGIN
	   IF AdrPArt=2 THEN AdrPart:=3;
	   BAsmCode[PrefixCnt]:=$dd;
	   BAsmCode[PrefixCnt+1]:=$cb;
	   Move(HVals,BAsmCode[PrefixCnt+2],HLen);
	   BAsmCode[PrefixCnt+2+HLen]:=$09+(AdrPart SHL 4);
	   CodeLen:=PrefixCnt+2+HLen+1;
	  END
	 ELSE { LD (SP+D),XY }
	  BEGIN
	   BAsmCode[PrefixCnt]:=$cb;
	   Move(HVals,BAsmCode[PrefixCnt+1],HLen);
	   BAsmCode[PrefixCnt+1+HLen]:=$29;
	   CodeLen:=PrefixCnt+1+HLen+1;
	  END;
	ELSE IF AdrMode<>ModNone THEN WrError(1350);
	END;
       END;
     ELSE IF AdrMode<>ModNone THEN WrError(1350);
     END; { CASE }
    END;
END;

	FUNCTION ParPair(Name1,Name2:String):Boolean;
BEGIN
   ParPair:=((NLS_StrCaseCmp(ArgStr[1],Name1)=0) AND (NLS_StrCaseCmp(ArgStr[2],Name2)=0)) OR
            ((NLS_StrCaseCmp(ArgStr[1],Name2)=0) AND (NLS_StrCaseCmp(ArgStr[2],Name1)=0));
END;

	FUNCTION ImmIs8:Boolean;
BEGIN
   ImmIs8:=(Word(AdrVals[AdrCnt-2])<=$ff) OR (Word(AdrVals[AdrCnt-2])>=$ff80);
END;

	PROCEDURE MakeCode_Z80;
	Far;
VAR
   OK:Boolean;
   AdrWord:Word;
   AdrLong:LongInt;
   AdrInt:Integer;
   AdrByte:Byte;
   z:Integer;
BEGIN
   CodeLen:=0; DontPrint:=False; PrefixCnt:=0; OpSize:=$ff; MayLW:=False;

   { zu ignorierendes }

   IF Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { letzten PrÑfix umkopieren }

   LastPrefix:=CurrPrefix; CurrPrefix:=Pref_IN_N;

   { evtl. Datenablage }

   IF DecodeIntelPseudo(False) THEN Exit;

{----------------------------------------------------------------------------}
{ InstruktionsprÑfix }

   IF Memo('DDIR') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE
      BEGIN
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  OK:=ExtendPrefix(CurrPrefix,ArgStr[z]);
	  IF NOT OK THEN WrError(1135);
	 END;
       IF OK THEN
	BEGIN
         GetPrefixCode(CurrPrefix,BAsmCode[0],BAsmCode[1]);
	 CodeLen:=2;
	END;
      END;
     Exit;
    END;

{----------------------------------------------------------------------------}
{ mit Sicherheit am hÑufigsten... }

   IF (Memo('LD')) OR (Memo('LDW')) THEN
    BEGIN
     DecodeLD;
     Exit;
    END;

{----------------------------------------------------------------------------}
{ ohne Operanden }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 CodeLen:=Len;
	 IF Len=2 THEN
	  BEGIN
	   BAsmCode[0]:=Hi(Code);
	   BAsmCode[1]:=Lo(Code);
	  END
	 ELSE BAsmCode[0]:=Lo(Code);
	END;
       Exit;
      END;

   { nur Akku zugelassen }

   FOR z:=1 TO AccOrderCnt DO
    WITH AccOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=0 THEN
	BEGIN
	 ArgCnt:=1; ArgStr[1]:='A';
	END;
       IF (ArgCnt<>1) THEN WrError(1110)
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 CodeLen:=Len;
	 IF Len=2 THEN
	  BEGIN
	   BAsmCode[0]:=Hi(Code);
	   BAsmCode[1]:=Lo(Code);
	  END
	 ELSE BAsmCode[0]:=Lo(Code);
	END;
       Exit;
      END;

   FOR z:=1 TO HLOrderCnt DO
    WITH HLOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=0 THEN
	BEGIN
	 ArgCnt:=1; ArgStr[1]:='HL';
	END;
       IF (ArgCnt<>1) THEN WrError(1110)
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'HL')<>0 THEN WrError(1350)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 CodeLen:=Len;
	 IF Len=2 THEN
	  BEGIN
	   BAsmCode[0]:=Hi(Code);
	   BAsmCode[1]:=Lo(Code);
	  END
	 ELSE BAsmCode[0]:=Lo(Code);
	END;
       Exit;
      END;

{---------------------------------------------------------------------------}
{ Datentransfer }

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     z:=Ord(Memo('PUSH')) SHL 2;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'SR')=0 THEN
      IF MomCPU<CPUZ380 THEN WrError(1500)
      ELSE
       BEGIN
	CodeLen:=2; BAsmCode[0]:=$ed; BAsmCode[1]:=$c1+z;
       END
     ELSE
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[1],'SP')=0 THEN ArgStr[1]:='A';
       IF NLS_StrCaseCmp(ArgStr[1],'AF')=0 THEN ArgStr[1]:='SP';
       OpSize:=1; MayLW:=True;
       DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModReg16:
	BEGIN
	 CodeLen:=1+PrefixCnt;
	 BAsmCode[PrefixCnt]:=$c1+(AdrPart SHL 4)+z;
	END;
       ModImm:
	IF MomCPU<CPUZ380 THEN WrError(1500)
	ELSE
	 BEGIN
	  BAsmCode[PrefixCnt]:=$fd; BAsmCode[PrefixCnt+1]:=$f5;
	  Move(AdrVals,BAsmCode[PrefixCnt+2],AdrCnt);
	  CodeLen:=PrefixCnt+2+AdrCnt;
	 END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF Memo('EX') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF ParPair('DE','HL') THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$eb;
      END
     ELSE IF ParPair('AF','AF''') THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$08;
      END
     ELSE IF ParPair('AF','AF`') THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$08;
      END
     ELSE IF ParPair('(SP)','HL') THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$e3;
      END
     ELSE IF ParPair('(SP)','IX') THEN
      BEGIN
       CodeLen:=2; BAsmCode[0]:=IXPrefix; BAsmCode[1]:=$e3;
      END
     ELSE IF ParPair('(SP)','IY') THEN
      BEGIN
       CodeLen:=2; BAsmCode[0]:=IYPrefix; BAsmCode[1]:=$e3;
      END
     ELSE IF ParPair('(HL)','A') THEN
      BEGIN
       IF MomCPU<CPUZ380 THEN WrError(1500)
       ELSE
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$ed; BAsmCode[1]:=$37;
	END;
      END
     ELSE
      BEGIN
       IF ArgStr[2][Length(ArgStr[2])]='''' THEN
	BEGIN
	 OK:=True; Delete(ArgStr[2],Length(ArgStr[2]),1);
	END
       ELSE OK:=False;
       DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModReg8:
	IF (AdrPart=6) OR (PrefixCnt<>0) THEN WrError(1350)
	ELSE
	 BEGIN
	  AdrByte:=AdrPart;
	  DecodeAdr(ArgStr[2]);
	  CASE AdrMode OF
	  ModReg8:
	   IF (AdrPart=6) OR (PrefixCnt<>0) THEN WrError(1350)
	   ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	   ELSE IF (AdrByte=7) AND (NOT OK) THEN
	    BEGIN
	     BAsmCode[0]:=$ed; BAsmCode[1]:=$07+(AdrPart SHL 3);
	     CodeLen:=2;
	    END
	   ELSE IF (AdrPart=7) AND (NOT OK) THEN
	    BEGIN
	     BAsmCode[0]:=$ed; BAsmCode[1]:=$07+(AdrByte SHL 3);
	     CodeLen:=2;
	    END
	   ELSE IF (OK) AND (AdrPart=AdrByte) THEN
	    BEGIN
	     BAsmCode[0]:=$cb; BAsmCode[1]:=$30+AdrPart;
	     CodeLen:=2;
	    END
	   ELSE WrError(1350);
	  ELSE IF AdrMode<>ModNone THEN WrError(1350);
	  END;
	 END;
       ModReg16:
	IF AdrPart=3 THEN WrError(1350)
	ELSE IF PrefixCnt=0 THEN { EX R16,... }
	 BEGIN
	  IF AdrPart=2 THEN AdrByte:=3 ELSE AdrByte:=AdrPart;
	  DecodeAdr(ArgStr[2]);
	  CASE AdrMode OF
	  ModReg16:
	   IF AdrPart=3 THEN WrError(1350)
	   ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	   ELSE IF OK THEN
	    BEGIN
	     IF AdrPart=2 THEN AdrPart:=3;
	     IF (PrefixCnt<>0) OR (AdrPart<>AdrByte) THEN  WrError(1350)
	     ELSE
	      BEGIN
	       CodeLen:=3; BAsmCode[0]:=$ed; BAsmCode[1]:=$cb;
	       BAsmCode[2]:=$30+AdrByte;
	      END;
	    END
	   ELSE IF PrefixCnt=0 THEN
	    BEGIN
	     IF AdrByte=0 THEN
	      BEGIN
	       IF AdrPart=2 THEN AdrPart:=3;
	       BAsmCode[0]:=$ed; BAsmCode[1]:=$01+(AdrPart SHL 2);
	       CodeLen:=2;
	      END
	     ELSE IF AdrPart=0 THEN
	      BEGIN
	       BAsmCode[0]:=$ed; BAsmCode[1]:=$01+(AdrByte SHL 2);
	       CodeLen:=2;
	      END
	    END
	   ELSE
	    BEGIN
	     IF AdrPart=2 THEN AdrPart:=3;
	     BAsmCode[1]:=$03+((BAsmCode[0] SHR 2) AND 8)+(AdrByte SHL 4);
	     BAsmCode[0]:=$ed;
	     CodeLen:=2;
	    END;
	  ELSE IF AdrMode<>ModNone THEN WrError(1350);
	  END;
	 END
	ELSE { EX XY,... }
	 BEGIN
	  DecodeAdr(ArgStr[2]);
	  CASE AdrMode OF
	  ModReg16:
	   IF AdrPart=3 THEN WrError(1350)
	   ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	   ELSE IF OK THEN
	    IF (PrefixCnt<>2) OR (BAsmCode[0]<>BAsmCode[1]) THEN WrError(1350)
	    ELSE
	     BEGIN
	      CodeLen:=3; BAsmCode[2]:=((BAsmCode[0] SHR 5) AND 1)+$34;
	      BAsmCode[0]:=$ed; BAsmCode[1]:=$cb;
	     END
	   ELSE IF PrefixCnt=1 THEN
	    BEGIN
	     IF AdrPart=2 THEN AdrPart:=3; 
	     BAsmCode[1]:=((BAsmCode[0] SHR 2) AND 8)+3+(AdrPart SHL 4);
	     BAsmCode[0]:=$ed;
	     CodeLen:=2;
	    END
	   ELSE IF BAsmCode[0]=BAsmCode[1] THEN WrError(1350)
	   ELSE
	    BEGIN
	     BAsmCode[0]:=$ed; BAsmCode[1]:=$2b; CodeLen:=2;
	    END;
	  ELSE IF AdrMode<>ModNone THEN WrError(1350)
	  END;
	 END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

{---------------------------------------------------------------------------}
{ Arithmetik }

   FOR z:=1 TO ALUOrderCnt DO
    WITH ALUOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 ArgStr[2]:=ArgStr[1]; ArgStr[1]:='A'; ArgCnt:=2;
	END;
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'HL')=0 THEN
	IF NOT Memo('SUB') THEN WrError(1350)
	ELSE
	 BEGIN
	  OpSize:=1; DecodeAdr(ArgStr[2]);
	  CASE AdrMode OF
	  ModAbs:
	   BEGIN
	    BAsmCode[PrefixCnt]:=$ed; BAsmCode[PrefixCnt+1]:=$d6;
	    Move(AdrVals,BAsmCode[PrefixCnt+2],AdrCnt);
	    CodeLen:=PrefixCnt+2+AdrCnt;
	   END;
	  ELSE IF AdrMode<>ModNone THEN WrError(1350);
	  END;
	 END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'SP')=0 THEN
	IF NOT Memo('SUB') THEN WrError(1350)
	ELSE
         BEGIN
          OpSize:=1; DecodeAdr(ArgStr[2]);
          CASE AdrMode OF
	  ModImm:
	   BEGIN
            BAsmCode[0]:=$ed; BAsmCode[1]:=$92;
	    Move(AdrVals,BAsmCode[2],AdrCnt);
            CodeLen:=2+AdrCnt;
	   END;
          ELSE IF AdrMode<>ModNone THEN WrError(1350);
          END;
         END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
       ELSE
        BEGIN
         OpSize:=0; DecodeAdr(ArgStr[2]);
         CASE AdrMode OF
         ModReg8:BEGIN
                  CodeLen:=PrefixCnt+1+AdrCnt;
                  BAsmCode[PrefixCnt]:=$80+(Code SHL 3)+AdrPart;
		  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
                 END;
         ModImm:IF NOT ImmIs8 THEN WrError(1320)
                ELSE
                 BEGIN
                  CodeLen:=2;
                  BAsmCode[0]:=$c6+(Code SHL 3);
                  BAsmCode[1]:=AdrVals[0];
                 END;
         ELSE IF AdrMode<>ModNone THEN WrError(1350);
         END;
        END;
       Exit;
      END
     ELSE IF Memo(Name+'W') THEN
      BEGIN
       IF (ArgCnt<>2) AND (ArgCnt<>1) THEN WrError(1110)
       ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
       ELSE IF (ArgCnt=2) AND (NLS_StrCaseCmp(ArgStr[1],'HL')<>0) THEN WrError(1350)
       ELSE
        BEGIN
         OpSize:=1; DecodeAdr(ArgStr[ArgCnt]);
         CASE AdrMode OF
         ModReg16:
	  IF PrefixCnt>0 THEN      { wenn Register, dann nie DDIR! }
           BEGIN
            BAsmCode[PrefixCnt]:=$87+(Code SHL 3);
	    CodeLen:=1+PrefixCnt;
           END
          ELSE IF AdrPart=3 THEN WrError(1350)
          ELSE
           BEGIN
            IF AdrPart=2 THEN AdrPart:=3;
            BAsmCode[0]:=$ed; BAsmCode[1]:=$84+(Code SHL 3)+AdrPart;
	    CodeLen:=2;
           END;
         ModReg8:
          IF (AdrPart<>6) OR (AdrCnt=0) THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[PrefixCnt]:=$c6+(Code SHL 3);
            Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
            CodeLen:=PrefixCnt+1+AdrCnt;
           END;
         ModImm:
          BEGIN
           BAsmCode[0]:=$ed; BAsmCode[1]:=$86+(Code SHL 3);
           Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
          END;
	 ELSE IF AdrMode<>ModNone THEN WrError(1350);
         END;
        END;
       Exit;
      END;

   IF Memo('ADD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModReg8:
        IF AdrPart<>7 THEN WrError(1350)
        ELSE
         BEGIN
          OpSize:=0; DecodeAdr(ArgStr[2]);
          CASE AdrMode OF
          ModReg8:
	   BEGIN
            CodeLen:=PrefixCnt+1+AdrCnt;
            BAsmCode[PrefixCnt]:=$80+AdrPart;
            Move(AdrVals,BAsmCode[1+PrefixCnt],AdrCnt);
           END;
          ModImm:
	   BEGIN
            CodeLen:=PrefixCnt+1+AdrCnt;
            BAsmCode[PrefixCnt]:=$c6;
            Move(AdrVals,BAsmCode[1+PrefixCnt],AdrCnt);
           END;
	  ELSE IF AdrMode<>ModNone THEN WrError(1350);
          END;
         END;
       ModReg16:
	IF AdrPart=3 THEN { SP }
	 BEGIN
	  OpSize:=1; DecodeAdr(ArgStr[2]);
	  CASE AdrMode OF
	  ModImm:
	   IF MomCPU<CPUZ380 THEN WrError(1500)
	   ELSE
	    BEGIN
	     BAsmCode[0]:=$ed; BAsmCode[1]:=$82;
	     Move(AdrVals,BAsmCode[2],AdrCnt);
	     CodeLen:=2+AdrCnt;
	    END;
          ELSE IF AdrMode<>ModNone THEN WrError(1350);
	  END;
	 END
	ELSE IF AdrPart<>2 THEN WrError(1350)
	ELSE
	 BEGIN
	  z:=PrefixCnt; { merkt, ob Indexregister }
	  OpSize:=1; DecodeAdr(ArgStr[2]);
	  CASE AdrMode OF
	  ModReg16:
	   IF (AdrPart=2) AND (PrefixCnt<>0) AND ((PrefixCnt<>2) OR (BAsmCode[0]<>BAsmCode[1])) THEN WrError(1350)
	   ELSE
	    BEGIN
	     IF PrefixCnt=2 THEN Dec(PrefixCnt); CodeLen:=1+PrefixCnt;
	     BAsmCode[PrefixCnt]:=$09+(AdrPart SHL 4);
	    END;
	  ModAbs:
	   IF z<>0 THEN WrError(1350)
	   ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	   ELSE
	    BEGIN
	     BAsmCode[PrefixCnt]:=$ed; BAsmCode[PrefixCnt+1]:=$c2;
	     Move(AdrVals,BAsmCode[PrefixCnt+2],AdrCnt);
	     CodeLen:=PrefixCnt+2+AdrCnt;
	    END;
          ELSE IF AdrMode<>ModNone THEN WrError(1350);
	  END;
	 END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF Memo('ADDW') THEN
    BEGIN
     IF (ArgCnt<>2) AND (ArgCnt<>1) THEN WrError(1110)
     ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE IF (ArgCnt=2) AND (NLS_StrCaseCmp(ArgStr[1],'HL')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       OpSize:=1; DecodeAdr(ArgStr[ArgCnt]);
       CASE AdrMode OF
       ModReg16:
	IF PrefixCnt>0 THEN      { wenn Register, dann nie DDIR! }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$87;
	  CodeLen:=1+PrefixCnt;
	 END
	ELSE IF AdrPart=3 THEN WrError(1350)
	ELSE
	 BEGIN
	  IF AdrPart=2 THEN AdrPart:=3;
	  BAsmCode[0]:=$ed; BAsmCode[1]:=$84+AdrPart;
	  CodeLen:=2;
	 END;
       ModReg8:
	IF (AdrPart<>6) OR (AdrCnt=0) THEN WrError(1350)
	ELSE
	 BEGIN
	  BAsmCode[PrefixCnt]:=$c6;
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	  CodeLen:=PrefixCnt+1+AdrCnt;
	 END;
       ModImm:
	BEGIN
	 BAsmCode[0]:=$ed; BAsmCode[1]:=$86;
	 Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
	END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (Memo('ADC')) OR (Memo('SBC')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModReg8:
	IF AdrPart<>7 THEN WrError(1350)
	ELSE
	 BEGIN
	  OpSize:=0; DecodeAdr(ArgStr[2]);
	  CASE AdrMode OF
	  ModReg8:
	   BEGIN
	    CodeLen:=PrefixCnt+1+AdrCnt;
	    BAsmCode[PrefixCnt]:=$88+AdrPart;
	    Move(AdrVals,BAsmCode[1+PrefixCnt],AdrCnt);
	   END;
	  ModImm:
	   BEGIN
	    CodeLen:=PrefixCnt+1+AdrCnt;
	    BAsmCode[PrefixCnt]:=$ce;
	    Move(AdrVals,BAsmCode[1+PrefixCnt],AdrCnt);
	   END;
	  ELSE IF AdrMode<>ModNone THEN WrError(1350);
	  END;
	  IF (Memo('SBC')) AND (CodeLen<>0) THEN Inc(BAsmCode[PrefixCnt],$10);
	 END;
       ModReg16:
	IF (AdrPart<>2) OR (PrefixCnt<>0) THEN WrError(1350)
	ELSE
	 BEGIN
	  OpSize:=1; DecodeAdr(ArgStr[2]);
	  CASE AdrMode OF
	  ModReg16:
	   IF PrefixCnt<>0 THEN WrError(1350)
	   ELSE
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$ed;
	     BAsmCode[1]:=$42+(AdrPart SHL 4);
	     IF Memo('ADC') THEN Inc(BAsmCode[1],8);
	    END;
	  ELSE IF AdrMode<>ModNone THEN WrError(1350);
	  END;
	 END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (Memo('ADCW')) OR (Memo('SBCW')) THEN
    BEGIN
     IF (ArgCnt<>2) AND (ArgCnt<>1) THEN WrError(1110)
     ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE IF (ArgCnt=2) AND (NLS_StrCaseCmp(ArgStr[1],'HL')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       z:=Ord(Memo('SBCW')) SHL 4;
       OpSize:=1; DecodeAdr(ArgStr[ArgCnt]);
       CASE AdrMode OF
       ModReg16:
	IF PrefixCnt>0 THEN      { wenn Register, dann nie DDIR! }
	 BEGIN
	  BAsmCode[PrefixCnt]:=$8f+z;
	  CodeLen:=1+PrefixCnt;
	 END
	ELSE IF AdrPart=3 THEN WrError(1350)
	ELSE
	 BEGIN
	  IF AdrPart=2 THEN AdrPart:=3;
	  BAsmCode[0]:=$ed; BAsmCode[1]:=$8c+z+AdrPart;
	  CodeLen:=2;
	 END;
       ModReg8:
	IF (AdrPart<>6) OR (AdrCnt=0) THEN WrError(1350)
	ELSE
	 BEGIN
	  BAsmCode[PrefixCnt]:=$ce+z;
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	  CodeLen:=PrefixCnt+1+AdrCnt;
	 END;
       ModImm:
	BEGIN
	 BAsmCode[0]:=$ed; BAsmCode[1]:=$8e+z;
	 Move(AdrVals,BAsmCode[2],AdrCnt); CodeLen:=2+AdrCnt;
	END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF (Memo('INC')) OR (Memo('DEC')) OR (Memo('INCW')) OR (Memo('DECW')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       z:=Ord((Memo('DEC')) OR (Memo('DECW')));
       DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModReg8:
	IF OpPart[Length(OpPart)]='W' THEN WrError(1350)
	ELSE
	 BEGIN
	  CodeLen:=PrefixCnt+1+AdrCnt;
	  BAsmCode[PrefixCnt]:=$04+(AdrPart SHL 3)+z;
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	 END;
       ModReg16:
	BEGIN
	 CodeLen:=1+PrefixCnt;
	 BAsmCode[PrefixCnt]:=$03+(AdrPart SHL 4)+(z SHL 3);
	END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   FOR z:=0 TO Pred(ShiftOrderCnt) DO
    IF Memo(ShiftOrders[z]) THEN
     BEGIN
      IF (ArgCnt=0) OR (ArgCnt>2) THEN WrError(1110)
      ELSE IF (z=6) AND (MomCPU<>CPUZ80U) THEN WrError(1500) { SLIA undok. Z80 }
      ELSE
       BEGIN
        OpSize:=0;
        DecodeAdr(ArgStr[ArgCnt]);
        CASE AdrMode OF
        ModReg8:
         IF (PrefixCnt>0) AND (AdrPart<>6) THEN WrError(1350) { IXL..IYU verbieten }
         ELSE
          BEGIN
           IF ArgCnt=1 THEN OK:=True
           ELSE IF MomCPU<>CPUZ80U THEN
            BEGIN
             WrError(1500); OK:=False;
            END
           ELSE IF (AdrPart<>6) OR (PrefixCnt<>1) OR (NOT DecodeReg8(ArgStr[1],AdrPart)) THEN
            BEGIN
             WrError(1350); OK:=False;
            END
           ELSE OK:=True;
           IF OK THEN
            BEGIN
             CodeLen:=PrefixCnt+1+AdrCnt+1;
             BAsmCode[PrefixCnt]:=$cb;
             Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
             BAsmCode[PrefixCnt+1+AdrCnt]:=AdrPart+(z SHL 3);
             IF (AdrPart=7) AND (z<4) THEN WrError(10);
            END;
          END;
        ELSE IF AdrMode<>ModNone THEN WrError(1350);
        END;
       END;
      Exit;
     END
    ELSE IF Memo(ShiftOrders[z]+'W') THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE IF (MomCPU<CPUZ380) OR (z=6) THEN WrError(1500)
      ELSE
       BEGIN
        OpSize:=1; DecodeAdr(ArgStr[1]);
        CASE AdrMode OF
        ModReg16:
         IF PrefixCnt>0 THEN
          BEGIN
           BAsmCode[2]:=$04+(z SHL 3)+((BAsmCode[0] SHR 5) AND 1);
           BAsmCode[0]:=$ed; BAsmCode[1]:=$cb;
           CodeLen:=3;
          END
         ELSE IF AdrPart=3 THEN WrError(1350)
         ELSE
          BEGIN
           IF AdrPart=2 THEN AdrPart:=3;
           BAsmCode[0]:=$ed; BAsmCode[1]:=$cb;
           BAsmCode[2]:=(z SHL 3)+AdrPart;
           CodeLen:=3;
          END;
        ModReg8:
         IF AdrPart<>6 THEN WrError(1350)
         ELSE
          BEGIN
           IF AdrCnt=0 THEN
            BEGIN
             BAsmCode[0]:=$ed; PrefixCnt:=1;
            END;
           BAsmCode[PrefixCnt]:=$cb;
           Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
           BAsmCode[PrefixCnt+1+AdrCnt]:=$02+(z SHL 3);
           CodeLen:=PrefixCnt+1+AdrCnt+1;
          END;
        ELSE IF AdrMode<>ModNone THEN WrError(1350);
        END;
       END;
      Exit;
     END;

   FOR z:=1 TO BitOrderCnt DO
    IF Memo(BitOrders[z]) THEN
     BEGIN
      IF (ArgCnt<>2) AND (ArgCnt<>3) THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[ArgCnt]);
	CASE AdrMode OF
	ModReg8:
	 IF (AdrPart<>6) AND (PrefixCnt<>0) THEN WrError(1350)
	 ELSE
	  BEGIN
           AdrByte:=EvalIntExpression(ArgStr[ArgCnt-1],UInt3,OK);
	   IF OK THEN
	    BEGIN
             IF ArgCnt=2 THEN OK:=True
             ELSE IF MomCPU<>CPUZ80U THEN
              BEGIN
               WrError(1500); OK:=False;
              END
             ELSE IF (AdrPart<>6) OR (PrefixCnt<>1) OR (z=1) OR (NOT DecodeReg8(ArgStr[1],AdrPart)) THEN
              BEGIN
               WrError(1350); OK:=False;
              END
             ELSE OK:=True;
             IF OK THEN
              BEGIN
               CodeLen:=PrefixCnt+2+AdrCnt;
               BAsmCode[PrefixCnt]:=$cb;
               Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
               BAsmCode[PrefixCnt+1+AdrCnt]:=AdrPart+(AdrByte SHL 3)+(z SHL 6);
              END;
            END;
	  END;
	END;
       END;
      Exit;
     END;

   IF Memo('MLT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPUZ180 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       IF (AdrMode<>ModReg16) OR (PrefixCnt<>0) THEN WrError(1350)
       ELSE
	BEGIN
	 BAsmCode[CodeLen]:=$ed; BAsmCode[CodeLen+1]:=$4c+(AdrPart SHL 4);
	 CodeLen:=2;
	END;
      END;
     Exit;
    END;

   IF (Memo('DIVUW')) OR (Memo('MULTW')) OR (Memo('MULTUW')) THEN
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       ArgStr[2]:=ArgStr[1]; ArgStr[1]:='HL'; ArgCnt:=2;
      END;
     IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'HL')<>0 THEN WrError(1350)
     ELSE
      BEGIN
       AdrByte:=Ord(OpPart[1]='D');
       z:=Ord(OpPart[Length(OpPart)-1]='U');
       OpSize:=1; DecodeAdr(ArgStr[ArgCnt]);
       CASE AdrMode OF
       ModReg8:
	IF (AdrPart<>6) OR (PrefixCnt=0) THEN WrError(1350)
	ELSE
	 BEGIN
	  BAsmCode[PrefixCnt]:=$cb;
	  Move(AdrVals,BAsmCode[PrefixCnt+1],AdrCnt);
	  BAsmCode[PrefixCnt+1+AdrCnt]:=$92+(z SHL 3)+(AdrByte SHL 5);
	  CodeLen:=PrefixCnt+1+AdrCnt+1;
	 END;
       ModReg16:
	IF AdrPart=3 THEN WrError(1350)
	ELSE IF PrefixCnt=0 THEN
	 BEGIN
	  IF AdrPart=2 THEN AdrPart:=3;
	  BAsmCode[0]:=$ed; BAsmCode[1]:=$cb;
	  BAsmCode[2]:=$90+AdrPart+(z SHL 3)+(AdrByte SHL 5);
	  CodeLen:=3;
	 END
	ELSE
	 BEGIN
	  BAsmCode[2]:=$94+((BAsmCode[0] SHR 5) AND 1)+(z SHL 3)+(AdrByte SHL 5);
	  BAsmCode[0]:=$ed; BAsmCode[1]:=$cb;
	  CodeLen:=3;
	 END;
       ModImm:
	BEGIN
	 BAsmCode[0]:=$ed; BAsmCode[1]:=$cb;
	 BAsmCode[2]:=$97+(z SHL 3)+(AdrByte SHL 5);
	 Move(AdrVals,BAsmCode[3],AdrCnt);
	 CodeLen:=3+AdrCnt;
	END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF Memo('TST') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPUZ180 THEN WrError(1500)
     ELSE
      BEGIN
       OpSize:=0; DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModReg8:
	IF PrefixCnt<>0 THEN WrError(1350)
	ELSE
	 BEGIN
	  BAsmCode[0]:=$ed; BAsmCode[1]:=4+(AdrPart SHL 3);
	  CodeLen:=2;
	 END;
       ModImm:
	BEGIN
	 BAsmCode[0]:=$ed; BAsmCode[1]:=$64; BAsmCode[2]:=AdrVals[0];
	 CodeLen:=3;
	END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

   IF Memo('SWAP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModReg16:
        IF AdrPart=3 THEN WrError(1350)
	ELSE IF PrefixCnt=0 THEN
	 BEGIN
	  IF AdrPart=2 THEN AdrPart:=3;
	  BAsmCode[0]:=$ed; BAsmCode[1]:=$0e+(AdrPart SHL 4);
	  CodeLen:=2;
	 END
	ELSE
	 BEGIN
	  BAsmCode[PrefixCnt]:=$3e; CodeLen:=PrefixCnt+1;
	 END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END;
     Exit;
    END;

{---------------------------------------------------------------------------}
{ Ein/Ausgabe }

   IF Memo('TSTI') THEN
    BEGIN
     IF MomCPU<>CPUZ80U THEN WrError(1500)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[0]:=$ed; BAsmCode[1]:=$70; CodeLen:=2;
      END;
     Exit;
    END;

   IF (Memo('IN')) OR (Memo('OUT')) THEN
    BEGIN
     IF (ArgCnt=1) AND (Memo('IN')) THEN
      BEGIN
       IF MomCPU<>CPUZ80U THEN WrError(1500)
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'(C)')<>0 THEN WrError(1350)
       ELSE
        BEGIN
         BAsmCode[0]:=$ed; BAsmCode[1]:=$70; CodeLen:=2;
        END;
      END
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF Memo('OUT') THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       IF NLS_StrCaseCmp(ArgStr[2],'(C)')=0 THEN
	BEGIN
	 OpSize:=0; DecodeAdr(ArgStr[1]);
	 CASE AdrMode OF
	 ModReg8:
	  IF (AdrPart=6) OR (PrefixCnt<>0) THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=2;
	    BAsmCode[0]:=$ed; BAsmCode[1]:=$40+(AdrPart SHL 3);
	    IF Memo('OUT') THEN Inc(BAsmCode[1]);
	   END;
	 ModImm:
	  IF Memo('IN') THEN WrError(1350)
          ELSE IF (MomCPU=CPUZ80U) AND (AdrVals[0]=0) THEN
           BEGIN
            BAsmCode[0]:=$ed; BAsmCode[1]:=$71; CodeLen:=2;
           END
	  ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
	  ELSE
	   BEGIN
	    BAsmCode[0]:=$ed; BAsmCode[1]:=$71; BAsmCode[2]:=AdrVals[0];
	    CodeLen:=3;
	   END;
	 ELSE IF AdrMode<>ModNone THEN WrError(1350);
	 END;
	END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
       ELSE
	BEGIN
	 BAsmCode[1]:=EvalIntExpression(ArgStr[2],UInt8,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegIO);
	   CodeLen:=2;
	   IF Memo('OUT') THEN BAsmCode[0]:=$d3 ELSE BAsmCode[0]:=$db;
	  END;
	END
      END;
     Exit;
    END;

   IF (Memo('INW')) OR (Memo('OUTW')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE
      BEGIN
       IF Memo('OUTW') THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       IF NLS_StrCaseCmp(ArgStr[2],'(C)')<>0 THEN WrError(1350)
       ELSE
	BEGIN
	 OpSize:=1; DecodeAdr(ArgStr[1]);
	 CASE AdrMode OF
	 ModReg16:
	  IF (AdrPart=3) OR (PrefixCnt>0) THEN WrError(1350)
	  ELSE
	   BEGIN
	    CASE AdrPart OF
	    1:AdrPart:=2;
	    2:AdrPart:=7;
	    END;
	    BAsmCode[0]:=$dd; BAsmCode[1]:=$40+(AdrPart SHL 3);
	    IF Memo('OUTW') THEN Inc(BAsmCode[1]);
	    CodeLen:=2;
	   END;
	 ModImm:
	  IF Memo('INW') THEN WrError(1350)
	  ELSE
	   BEGIN
	    BAsmCode[0]:=$fd; BAsmCode[1]:=$79;
	    Move(AdrVals,BAsmCode[2],AdrCnt);
	    CodeLen:=2+AdrCnt;
	   END;
	 ELSE IF AdrMode<>ModNone THEN WrError(1350);
	 END;
	END;
      END;
     Exit;
    END;

   IF (Memo('IN0')) OR (Memo('OUT0')) THEN
    BEGIN
     IF (ArgCnt<>2) AND (ArgCnt<>1) THEN WrError(1110)
     ELSE IF (ArgCnt=1) AND (Memo('OUT0')) THEN WrError(1110)
     ELSE IF MomCPU<CPUZ180 THEN WrError(1500)
     ELSE
      BEGIN
       IF Memo('OUT0') THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       OpSize:=0;
       IF ArgCnt=1 THEN
	BEGIN
	 AdrPart:=6; OK:=True;
	END
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1]);
	 IF (AdrMode=ModReg8) AND (AdrPart<>6) AND (PrefixCnt=0) THEN OK:=True
	 ELSE
	  BEGIN
	   OK:=False; IF AdrMode<>ModNone THEN WrError(1350);
	  END;
	END;
       IF OK THEN
	BEGIN
	 BAsmCode[2]:=EvalIntExpression(ArgStr[ArgCnt],UInt8,OK);
	 IF OK THEN
	  BEGIN
	   BAsmCode[0]:=$ed; BAsmCode[1]:=AdrPart SHL 3;
	   IF Memo('OUT0') THEN Inc(BAsmCode[1]);
	   CodeLen:=3;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Memo('INA')) OR (Memo('INAW')) OR (Memo('OUTA')) OR (Memo('OUTAW')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE
      BEGIN
       IF (Memo('OUTA')) OR (Memo('OUTAW')) THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       OpSize:=Ord(OpPart[Length(OpPart)]='W');
       IF ((OpSize=0) AND (NLS_StrCaseCmp(ArgStr[1],'A')<>0))
       OR ((OpSize=1) AND (NLS_StrCaseCmp(ArgStr[1],'HL')<>0)) THEN WrError(1350)
       ELSE
	BEGIN
	 IF ExtFlag THEN AdrLong:=EvalIntExpression(ArgStr[2],Int32,OK)
	 ELSE AdrLong:=EvalIntExpression(ArgStr[2],UInt8,OK);
	 IF OK THEN
	  BEGIN
	   ChkSpace(SegIO);
           IF (AdrLong AND $ff000000<>0) THEN ChangeDDPrefix('IW')
           ELSE IF (AdrLong AND $ff0000<>0) THEN ChangeDDPrefix('IB');
	   BAsmCode[PrefixCnt]:=$ed+(OpSize SHL 4);
           BAsmCode[PrefixCnt+1]:=$d3+(Ord(OpPart[1]='I') SHL 3);
	   BAsmCode[PrefixCnt+2]:=AdrLong AND $ff;
	   BAsmCode[PrefixCnt+3]:=(AdrLong SHR 8) AND $ff;
	   CodeLen:=PrefixCnt+4;
           IF (AdrLong AND $ffff0000<>0) THEN
	    BEGIN
	     BAsmCode[CodeLen]:=(AdrLong SHR 16) AND $ff; Inc(CodeLen);
	    END;
           IF (AdrLong AND $ff000000<>0) THEN
	    BEGIN
	     BAsmCode[CodeLen]:=(AdrLong SHR 24) AND $ff; Inc(CodeLen);
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('TSTIO') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPUZ180 THEN WrError(1500)
     ELSE
      BEGIN
       BAsmCode[2]:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[0]:=$ed; BAsmCode[1]:=$74;
	 CodeLen:=3;
	END;
      END;
     Exit;
    END;

{---------------------------------------------------------------------------}
{ SprÅnge }

   IF Memo('RET') THEN
    BEGIN
     IF ArgCnt=0 THEN
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$c9;
      END
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT DecodeCondition(ArgStr[1],z) THEN WrError(1360)
     ELSE
      BEGIN
       CodeLen:=1; BAsmCode[0]:=$c0+(z SHL 3);
      END;
     Exit;
    END;

   IF Memo('JP') THEN
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       IF NLS_StrCaseCmp(ArgStr[1],'(HL)')=0 THEN
        BEGIN
         CodeLen:=1; BAsmCode[0]:=$e9; OK:=False;
        END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'(IX)')=0 THEN
        BEGIN
         CodeLen:=2; BAsmCode[0]:=IXPrefix; BAsmCode[1]:=$e9; OK:=False;
	END
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'(IY)')=0 THEN
        BEGIN
         CodeLen:=2; BAsmCode[0]:=IYPrefix; BAsmCode[1]:=$e9; OK:=False;
        END
       ELSE
        BEGIN
	 z:=1; OK:=True;
        END;
      END
     ELSE IF ArgCnt=2 THEN
      BEGIN
       OK:=DecodeCondition(ArgStr[1],z);
       IF OK THEN z:=z SHL 3 ELSE WrError(1360);
      END
     ELSE
      BEGIN
       WrError(1110); OK:=False;
      END;
     IF OK THEN
      BEGIN
       AdrLong:=EvalAbsAdrExpression(ArgStr[ArgCnt],OK);
       IF OK THEN
        IF AdrLong AND $ffff0000=0 THEN
         BEGIN
          CodeLen:=3; BAsmCode[0]:=$c2+z;
	  BAsmCode[1]:=Lo(AdrLong); BAsmCode[2]:=Hi(AdrLong);
         END
        ELSE IF AdrLong AND $ff000000=0 THEN
         BEGIN
          ChangeDDPrefix('IB');
          CodeLen:=4+PrefixCnt; BAsmCode[PrefixCnt]:=$c2+z;
          BAsmCode[PrefixCnt+1]:=Lo(AdrLong);
          BAsmCode[PrefixCnt+2]:=Hi(AdrLong);
          BAsmCode[PrefixCnt+3]:=Hi(AdrLong SHR 8);
         END
        ELSE
         BEGIN
          ChangeDDPrefix('IW');
          CodeLen:=5+PrefixCnt; BAsmCode[PrefixCnt]:=$c2+z;
          BAsmCode[PrefixCnt+1]:=Lo(AdrLong);
          BAsmCode[PrefixCnt+2]:=Hi(AdrLong);
	  BAsmCode[PrefixCnt+3]:=Hi(AdrLong SHR 8);
          BAsmCode[PrefixCnt+4]:=Hi(AdrLong SHR 16);
         END;
      END;
     Exit;
    END;

   IF Memo('CALL') THEN
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       z:=9; OK:=True;
      END
     ELSE IF ArgCnt=2 THEN
      BEGIN
       OK:=DecodeCondition(ArgStr[1],z);
       IF OK THEN z:=z SHL 3 ELSE WrError(1360);
      END
     ELSE
      BEGIN
       WrError(1110); OK:=False;
      END;
     IF OK THEN
      BEGIN
       AdrLong:=EvalAbsAdrExpression(ArgStr[ArgCnt],OK);
       IF OK THEN
        IF AdrLong AND $ffff0000=0 THEN
         BEGIN
          CodeLen:=3; BAsmCode[0]:=$c4+z;
	  BAsmCode[1]:=Lo(AdrLong); BAsmCode[2]:=Hi(AdrLong);
         END
        ELSE IF AdrLong AND $ff000000=0 THEN
         BEGIN
          ChangeDDPrefix('IB');
          CodeLen:=4+PrefixCnt; BAsmCode[PrefixCnt]:=$c4+z;
	  BAsmCode[PrefixCnt+1]:=Lo(AdrLong);
          BAsmCode[PrefixCnt+2]:=Hi(AdrLong);
          BAsmCode[PrefixCnt+3]:=Hi(AdrLong SHR 8);
         END
        ELSE
         BEGIN
          ChangeDDPrefix('IW');
          CodeLen:=5+PrefixCnt; BAsmCode[PrefixCnt]:=$c4+z;
          BAsmCode[PrefixCnt+1]:=Lo(AdrLong);
          BAsmCode[PrefixCnt+2]:=Hi(AdrLong);
          BAsmCode[PrefixCnt+3]:=Hi(AdrLong SHR 8);
          BAsmCode[PrefixCnt+4]:=Hi(AdrLong SHR 16);
         END;
      END;
     Exit;
    END;

   IF Memo('JR') THEN
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       z:=3; OK:=True;
      END
     ELSE IF ArgCnt=2 THEN
      BEGIN
       OK:=DecodeCondition(ArgStr[1],z);
       IF (OK) AND (z>3) THEN OK:=False;
       IF OK THEN Inc(z,4) ELSE WrError(1360);
      END
     ELSE
      BEGIN
       WrError(1110); Exit;
      END;
     IF OK THEN
      BEGIN
       AdrLong:=EvalAbsAdrExpression(ArgStr[ArgCnt],OK);
       IF OK THEN
        BEGIN
         Dec(AdrLong,EProgCounter+2);
         IF (AdrLong<=$7f) AND (AdrLong>=-$80) THEN
          BEGIN
           CodeLen:=2; BAsmCode[0]:=z SHL 3;
           BAsmCode[1]:=AdrLong AND $ff;
          END
         ELSE
          IF MomCPU<CPUZ380 THEN WrError(1370)
          ELSE
           BEGIN
	    Dec(AdrLong,2);
            IF (AdrLong<=$7fff) AND (AdrLong>=-$8000) THEN
             BEGIN
              CodeLen:=4; BAsmCode[0]:=$dd; BAsmCode[1]:=z SHL 3;
              BAsmCode[2]:=AdrLong AND $ff;
              BAsmCode[3]:=(AdrLong SHR 8) AND $ff;
             END
            ELSE
             BEGIN
              Dec(AdrLong);
              IF (AdrLong<=$7fffff) AND (AdrLong>=-$800000) THEN
               BEGIN
                CodeLen:=5; BAsmCode[0]:=$fd; BAsmCode[1]:=z SHL 3;
                BAsmCode[2]:=AdrLong AND $ff;
                BAsmCode[3]:=(AdrLong SHR 8) AND $ff;
                BAsmCode[4]:=(AdrLong SHR 16) AND $ff;
               END
              ELSE WrError(1370);
             END;
           END;
        END;
      END;
     Exit;
    END;

   IF Memo('CALR') THEN
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       z:=9; OK:=True;
      END
     ELSE IF ArgCnt=2 THEN
      BEGIN
       OK:=DecodeCondition(ArgStr[1],z);
       IF OK THEN z:=z SHL 3 ELSE WrError(1360);
      END
     ELSE
      BEGIN
       WrError(1110); Exit;
      END;
     IF OK THEN
      IF MomCPU<CPUZ380 THEN WrError(1500)
      ELSE
       BEGIN
        AdrLong:=EvalAbsAdrExpression(ArgStr[ArgCnt],OK);
        IF OK THEN
         BEGIN
          Dec(AdrLong,EProgCounter+3);
          IF (AdrLong<=$7f) AND (AdrLong>=-$80) THEN
           BEGIN
            CodeLen:=3; BAsmCode[0]:=$ed; BAsmCode[1]:=$c4+z;
           BAsmCode[2]:=AdrLong AND $ff;
          END
          ELSE
           BEGIN
            Dec(AdrLong);
            IF (AdrLong<=$7fff) AND (AdrLong>=-$8000) THEN
	     BEGIN
              CodeLen:=4; BAsmCode[0]:=$dd; BAsmCode[1]:=$c4+z;
              BAsmCode[2]:=AdrLong AND $ff;
              BAsmCode[3]:=(AdrLong SHR 8) AND $ff;
             END
            ELSE
             BEGIN
              Dec(AdrLong);
              IF (AdrLong<=$7fffff) AND (AdrLong>=-$800000) THEN
               BEGIN
                CodeLen:=5; BAsmCode[0]:=$fd; BAsmCode[1]:=$c4+z;
                BAsmCode[2]:=AdrLong AND $ff;
                BAsmCode[3]:=(AdrLong SHR 8) AND $ff;
		BAsmCode[4]:=(AdrLong SHR 16) AND $ff;
               END
              ELSE WrError(1370);
             END;
           END;
         END;
       END;
     Exit;
    END;

   IF Memo('DJNZ') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalAbsAdrExpression(ArgStr[1],OK);
       IF OK THEN
        BEGIN
         Dec(AdrLong,EProgCounter+2);
         IF (AdrLong<=$7f) AND (AdrLong>=-$80) THEN
          BEGIN
           CodeLen:=2; BAsmCode[0]:=$10; BAsmCode[1]:=Lo(AdrLong);
	  END
         ELSE IF MomCPU<CPUZ380 THEN WrError(1370)
         ELSE
          BEGIN
           Dec(AdrLong,2);
           IF (AdrLong<=$7fff) AND (AdrLong>=-$8000) THEN
            BEGIN
             CodeLen:=4; BAsmCode[0]:=$dd; BAsmCode[1]:=$10;
             BAsmCode[2]:=AdrLong AND $ff;
             BAsmCode[3]:=(AdrLong SHR 8) AND $ff;
            END
           ELSE
            BEGIN
	     Dec(AdrLong);
             IF (AdrLong<=$7fffff) AND (AdrLong>=-$800000) THEN
              BEGIN
               CodeLen:=5; BAsmCode[0]:=$fd; BAsmCode[1]:=$10;
               BAsmCode[2]:=AdrLong AND $ff;
               BAsmCode[3]:=(AdrLong SHR 8) AND $ff;
               BAsmCode[4]:=(AdrLong SHR 16) AND $ff;
              END
             ELSE WrError(1370);
            END
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('RST') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrByte:=EvalIntExpression(ArgStr[1],Int8,OK);
       IF OK THEN
        IF (AdrByte>$38) OR (AdrByte AND 7<>0) THEN WrError(1320)
        ELSE
         BEGIN
	  CodeLen:=1; BAsmCode[0]:=$c7+AdrByte;
         END;
      END;
     Exit;
    END;

{---------------------------------------------------------------------------}
{ Sonderbefehle }

   IF (Memo('EI')) OR (Memo('DI')) THEN
    BEGIN
     IF ArgCnt=0 THEN
      BEGIN
       BAsmCode[0]:=$f3+(Ord(Memo('EI')) SHL 3);
       CodeLen:=1;
      END
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE
      BEGIN
       BAsmCode[2]:=EvalIntExpression(ArgStr[1],UInt8,OK);
       IF OK THEN
	BEGIN
	 BAsmCode[0]:=$dd; BAsmCode[1]:=$f3+(Ord(Memo('EI')) SHL 3);
	 CodeLen:=3;
	END;
      END;
     Exit;
    END;

   IF Memo('IM') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrByte:=EvalIntExpression(ArgStr[1],UInt2,OK);
       IF OK THEN
	IF AdrByte>3 THEN WrError(1320)
	ELSE IF (AdrByte=3) AND (MomCPU<CPUZ380) THEN WrError(1500)
	ELSE
	 BEGIN
	  IF AdrByte=3 THEN AdrByte:=1
	  ELSE IF AdrByte>=1 THEN Inc(AdrByte);
	  CodeLen:=2; BAsmCode[0]:=$ed; BAsmCode[1]:=$46+(AdrByte SHL 3);
	 END;
      END;
     Exit;
    END;

   IF Memo('LDCTL') THEN
    BEGIN
     OpSize:=0;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE IF DecodeSFR(ArgStr[1],AdrByte) THEN
      BEGIN
       DecodeAdr(ArgStr[2]);
       CASE AdrMode OF
       ModReg8:
        IF AdrPart<>7 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$cd+((AdrByte AND 3) SHL 4);
          BAsmCode[1]:=$c8+((AdrByte AND 4) SHL 2);
          CodeLen:=2;
         END;
       ModReg16:
        IF (AdrByte<>1) OR (AdrPart<>2) OR (PrefixCnt<>0) THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$ed; BAsmCode[1]:=$c8; CodeLen:=2;
         END;
       ModImm:
        BEGIN
         BAsmCode[0]:=$cd+((AdrByte AND 3) SHL 4);
         BAsmCode[1]:=$ca+((AdrByte AND 4) SHL 2);
         BAsmCode[2]:=AdrVals[0];
         CodeLen:=3;
        END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END
     ELSE IF DecodeSFR(ArgStr[2],AdrByte) THEN
      BEGIN
       DecodeAdr(ArgStr[1]);
       CASE AdrMode OF
       ModReg8:
        IF (AdrPart<>7) OR (AdrByte=1) THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$cd+((AdrByte AND 3) SHL 4);
          BAsmCode[1]:=$d0;
          CodeLen:=2;
         END;
       ModReg16:
        IF (AdrByte<>1) OR (AdrPart<>2) OR (PrefixCnt<>0) THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=$ed; BAsmCode[1]:=$c0; CodeLen:=2;
         END;
       ELSE IF AdrMode<>ModNone THEN WrError(1350);
       END;
      END
     ELSE WrError(1350);
     Exit;
    END;

   IF (Memo('RESC')) OR (Memo('SETC')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPUZ380 THEN WrError(1500)
     ELSE
      BEGIN
       AdrByte:=$ff; NLS_UpString(ArgStr[1]);
       IF ArgStr[1]='LW' THEN AdrByte:=1
       ELSE IF ArgStr[1]='LCK' THEN AdrByte:=2
       ELSE IF ArgStr[1]='XM' THEN AdrByte:=3
       ELSE WrError(1440);
       IF AdrByte<>$ff THEN
        BEGIN
         CodeLen:=2; BAsmCode[0]:=$cd+(AdrByte SHL 4);
	 BAsmCode[1]:=$f7+(Ord(Memo('RESC')) SHL 3);
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_Z80;
	Far;
BEGIN
   SaveInitProc;
   SetFlag(ExtFlag,ExtFlagName,FALSE);
   SetFlag(LWordFlag,LWordFlagName,FALSE);
END;

	FUNCTION ChkPC_Z80:Boolean;
	Far;
VAR
   OK:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : OK:=IsUGreaterEq(CodeEnd,ProgCounter);
   SegIO    : OK:=IsUGreaterEq(PortEnd,ProgCounter);
   ELSE ok:=False;
   END;
   ChkPC_Z80:=OK;
END;

	FUNCTION IsDef_Z80:Boolean;
	Far;
BEGIN
   IsDef_Z80:=Memo('PORT');
END;

	PROCEDURE SwitchFrom_Z80;
	Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_Z80;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=True;

   PCSymbol:='$'; HeaderID:=$51; NOPCode:=$00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegIO];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;
   Grans[SegIO  ]:=1; ListGrans[SegIO  ]:=1; SegInits[SegIO  ]:=0;

   MakeCode:=MakeCode_Z80; ChkPC:=ChkPC_Z80; IsDef:=IsDef_Z80;
   SwitchFrom:=SwitchFrom_Z80; InitFields;
END;

BEGIN
   CPUZ80 :=AddCPU('Z80' ,SwitchTo_Z80);
   CPUZ80U:=AddCPU('Z80UNDOC' ,SwitchTo_Z80);
   CPUZ180:=AddCPU('Z180',SwitchTo_Z80);
   CPUZ380:=AddCPU('Z380',SwitchTo_Z80);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_Z80;
END.
