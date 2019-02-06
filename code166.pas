{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
	Unit Code166;

{ AS - Codegenerator 80C166/167 }

Interface

        Uses StringUt,NLS,
	     AsmDef,AsmPars,AsmSub,CodePseu;


Implementation

TYPE
   BaseOrder=RECORD
	      Name:String[6];
	      MinCPU:CPUVar;
	      Code1,Code2:Word;
	     END;
   SimpOrder=RECORD
	      Name:String[5];
	      Code:Byte;
	     END;
   Condition=RECORD
	      Name:String[3];
	      Code:Byte;
	     END;

CONST
   FixedOrderCount=10;
   ConditionCount=20;
   ALU2OrderCount=8;
   ShiftOrderCount=5;
   Bit2OrderCount=6;
   LoopOrderCount=4;
   DivOrderCount=4;
   BJmpOrderCount=4;
   MulOrderCount=3;

   DPPCount=4;
   RegNames:ARRAY[0..5] OF String[4]=('DPP0','DPP1','DPP2','DPP3','CP','SP');

TYPE
   FixedOrderArray=ARRAY[1..FixedOrderCount] OF BaseOrder;
   ConditionArray=ARRAY[1..ConditionCount] OF Condition;
   ALU2OrderArray=ARRAY[0..ALU2OrderCount-1] OF String[4];
   ShiftOrderArray=ARRAY[1..ShiftOrderCount] OF SimpOrder;
   Bit2OrderArray=ARRAY[1..Bit2OrderCount] OF SimpOrder;
   LoopOrderArray=ARRAY[1..LoopOrderCount] OF SimpOrder;
   DivOrderArray=ARRAY[0..DivOrderCount-1] OF String[5];
   BJmpOrderArray=ARRAY[0..BJmpOrderCount-1] OF String[4];
   MulOrderArray=ARRAY[0..MulOrderCount-1] OF String[5];

VAR
   CPU80C166,CPU80C163,CPU80C165,CPU80C167:CPUVar;

   FixedOrders:^FixedOrderArray;
   Conditions:^ConditionArray;
   TrueCond:Integer;
   ALU2Orders:^ALU2OrderArray;
   ShiftOrders:^ShiftOrderArray;
   Bit2Orders:^Bit2OrderArray;
   LoopOrders:^LoopOrderArray;
   DivOrders:^DivOrderArray;
   BJmpOrders:^BJmpOrderArray;
   MulOrders:^MulOrderArray;

   SaveInitProc:PROCEDURE;
   DPPAssumes:ARRAY[0..DPPCount-1] OF LongInt;
   MemInt,MemInt2:IntType;
   MemEnd:LongInt;
   OpSize:Byte;

   DPPChanged,N_DPPChanged:ARRAY[0..DPPCount-1] OF Boolean;
   SPChanged,CPChanged,N_SPChanged,N_CPChanged:Boolean;

   ExtCounter:ShortInt;
   MemMode:(MemModeStd,MemModeNoCheck,MemModeZeroPage,MemModeFixedBank,MemModeFixedPage);
	   { normal    EXTS Rn        EXTP Rn         EXTS nn          EXTP nn         }
   MemPage:Word;
   ExtSFRs:Boolean;

{---------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

	PROCEDURE AddFixed(NName:String; NMin:CPUVar; NCode1,NCode2:Word);
BEGIN
   IF z=FixedOrderCount THEN Halt
   ELSE
    BEGIN
     Inc(z);
     WITH FixedOrders^[z] DO
      BEGIN
       Name:=NName; MinCPU:=NMin; Code1:=NCode1; Code2:=NCode2;
      END;
    END;
END;

	PROCEDURE AddShift(NName:String; NCode:Byte);
BEGIN
   IF z=ShiftOrderCount THEN Halt
   ELSE
    BEGIN
     Inc(z);
     WITH ShiftOrders^[z] DO
      BEGIN
       Name:=NName; Code:=NCode;
      END;
    END;
END;

	PROCEDURE AddBit2(NName:String; NCode:Byte);
BEGIN
   IF z=Bit2OrderCount THEN Halt
   ELSE
    BEGIN
     Inc(z);
     WITH Bit2Orders^[z] DO
      BEGIN
       Name:=NName; Code:=NCode;
      END;
    END;
END;

	PROCEDURE AddLoop(NName:String; NCode:Byte);
BEGIN
   IF z=LoopOrderCount THEN Halt
   ELSE
    BEGIN
     Inc(z);
     WITH LoopOrders^[z] DO
      BEGIN
       Name:=NName; Code:=NCode;
      END;
    END;
END;

	PROCEDURE AddCondition(Nname:String; NCode:Byte);
BEGIN
   IF z=ConditionCount THEN Halt
   ELSE
    BEGIN
     Inc(z);
     WITH Conditions^[z] DO
      BEGIN
       Name:=NName; Code:=NCode;
      END;
    END;
END;

BEGIN
   z:=0; New(FixedOrders);
   AddFixed('DISWDT',CPU80C166,$5aa5,$a5a5);
   AddFixed('EINIT' ,CPU80C166,$4ab5,$b5b5);
   AddFixed('IDLE'  ,CPU80C166,$7887,$8787);
   AddFixed('NOP'   ,CPU80C166,$00cc,$0000);
   AddFixed('PWRDN' ,CPU80C166,$6897,$9797);
   AddFixed('RET'   ,CPU80C166,$00cb,$0000);
   AddFixed('RETI'  ,CPU80C166,$88fb,$0000);
   AddFixed('RETS'  ,CPU80C166,$00db,$0000);
   AddFixed('SRST'  ,CPU80C166,$48b7,$b7b7);
   AddFixed('SRVWDT',CPU80C166,$58a7,$a7a7);

   z:=0; New(Conditions);
   AddCondition('UC' ,$0); TrueCond:=z; AddCondition('Z'  ,$2);
   AddCondition('NZ' ,$3); AddCondition('V'  ,$4);
   AddCondition('NV' ,$5); AddCondition('N'  ,$6);
   AddCondition('NN' ,$7); AddCondition('C'  ,$8);
   AddCondition('NC' ,$9); AddCondition('EQ' ,$2);
   AddCondition('NE' ,$3); AddCondition('ULT',$8);
   AddCondition('ULE',$f); AddCondition('UGE',$9);
   AddCondition('UGT',$e); AddCondition('SLT',$c);
   AddCondition('SLE',$b); AddCondition('SGE',$d);
   AddCondition('SGT',$a); AddCondition('NET',$1);

   New(ALU2Orders);
   ALU2Orders^[0]:='ADD' ; ALU2Orders^[1]:='ADDC';
   ALU2Orders^[2]:='SUB' ; ALU2Orders^[3]:='SUBC';
   ALU2Orders^[4]:='CMP' ; ALU2Orders^[5]:='XOR' ;
   ALU2Orders^[6]:='AND' ; ALU2Orders^[7]:='OR'  ;

   New(ShiftOrders); z:=0;
   AddShift('ASHR',$ac); AddShift('ROL' ,$0c);
   AddShift('ROR' ,$2c); AddShift('SHL' ,$4c);
   AddShift('SHR' ,$6c);

   New(Bit2Orders); z:=0;
   AddBit2('BAND',$6a); AddBit2('BCMP' ,$2a);
   AddBit2('BMOV',$4a); AddBit2('BMOVN',$3a);
   AddBit2('BOR' ,$5a); AddBit2('BXOR' ,$7a);

   New(LoopOrders); z:=0;
   AddLoop('CMPD1',$a0); AddLoop('CMPD2',$b0);
   AddLoop('CMPI1',$80); AddLoop('CMPI2',$90);

   New(DivOrders);
   DivOrders^[0]:='DIV'; DivOrders^[1]:='DIVU';
   DivOrders^[2]:='DIVL'; DivOrders^[3]:='DIVLU';

   New(BJmpOrders);
   BJmpOrders^[0]:='JB'; BJmpOrders^[1]:='JNB';
   BJmpOrders^[2]:='JBC'; BJmpOrders^[3]:='JNBS';

   New(MulOrders);
   MulOrders^[0]:='MUL'; MulOrders^[1]:='MULU';
   MulOrders^[2]:='PRIOR';
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(Conditions);
   Dispose(ALU2Orders);
   Dispose(ShiftOrders);
   Dispose(Bit2Orders);
   Dispose(LoopOrders);
   Dispose(DivOrders);
   Dispose(BJmpOrders);
   Dispose(MulOrders);
END;

{---------------------------------------------------------------------------}

CONST
   ModNone=-1;
   ModReg=0;     MModReg=1 SHL ModReg;
   ModImm=1;     MModImm=1 SHL ModImm;
   ModIReg=2;    MModIReg=1 SHL ModIReg;
   ModPreDec=3;  MModPreDec=1 SHL ModPreDec;
   ModPostInc=4; MModPostInc=1 SHL ModPostInc;
   ModIndex=5;   MModIndex=1 SHL ModIndex;
   ModAbs=6;     MModAbs=1 SHL ModAbs;
   ModMReg=7;    MModMReg=1 SHL ModMReg;
   ModLAbs=8;    MModLAbs=1 SHL ModLAbs;
VAR
   AdrCnt,AdrMode:Byte;
   AdrVals:ARRAY[0..1] OF Byte;
   AdrType:ShortInt;

	FUNCTION IsReg(Asc:String; VAR Erg:Byte; WordWise:Boolean):Boolean;
VAR
   err:ValErgType;
   s:StringPtr;
BEGIN
   IF FindRegDef(Asc,s) THEN Asc:=s^;

   IF (Length(Asc)<2) OR (UpCase(Asc[1])<>'R') THEN IsReg:=False
   ELSE IF (Length(Asc)>2) AND (UpCase(Asc[2])='L') AND (NOT WordWise) THEN
    BEGIN
     Val(Copy(Asc,3,Length(Asc)-2),Erg,err); Erg:=Erg SHL 1;
     IsReg:=(err=0) AND (Erg<=15);
    END
   ELSE IF (Length(Asc)>2) AND (UpCase(Asc[2])='H') AND (NOT WordWise) THEN
    BEGIN
     Val(Copy(Asc,3,Length(Asc)-2),Erg,err); Erg:=Succ(Erg SHL 1);
     IsReg:=(err=0) AND (Erg<=15);
    END
   ELSE
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,err);
     IsReg:=(err=0) AND (Erg<=15);
    END;
END;

	FUNCTION SFRStart:LongInt;
BEGIN
   IF ExtSFRs THEN SFRStart:=$f000 ELSE SFRStart:=$fe00;
END;

	FUNCTION SFREnd:LongInt;
BEGIN
   IF ExtSFRs THEN SFREnd:=$f1de ELSE SFREnd:=$ffde;
END;

	FUNCTION CalcPage(VAR Adr:LongInt; DoAnyway:Boolean):Boolean;
VAR
   z:Integer;
BEGIN
   CalcPage:=True;
   CASE MemMode OF
   MemModeStd:
    BEGIN
     z:=0;
     WHILE (z<=3) AND (Adr SHR 14<>DPPAssumes[z]) DO Inc(z);
     IF z>3 THEN
      BEGIN
       WrError(110); Adr:=Adr AND $ffff; CalcPage:=DoAnyway;
      END
     ELSE
      BEGIN
       Adr:=(Adr AND $3fff)+(z SHL 14);
       IF DPPChanged[z] THEN WrXError(200,RegNames[z]);
      END;
    END;
   MemModeZeroPage:Adr:=Adr AND $3fff;
   MemModeFixedPage:
    BEGIN
     Adr:=Adr AND $3fff;
     IF Adr SHR 14<>MemPage THEN
      BEGIN
       WrError(110); CalcPage:=DoAnyway;
      END;
    END;
   MemModeNoCheck:Adr:=Adr AND $ffff;
   MemModeFixedBank:
    BEGIN
     Adr:=Adr AND $ffff;
     IF Adr SHR 16<>MemPage THEN
      BEGIN
       WrError(110); CalcPage:=DoAnyway;
      END;
    END;
   END;
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:Word; InCode,Dest:Boolean);
VAR
   HDisp,DispAcc:LongInt;
   PPos,MPos:Integer;
   Part:String;
   OK,NegFlag,NNegFlag:Boolean;
   HReg:Byte;

	PROCEDURE DecideAbsolute;
CONST
   DPPAdr=$fe00;
   SPAdr=$fe12;
   CPAdr=$fe10;
VAR
   z:Integer;
BEGIN
   IF InCode THEN
    IF (EProgCounter SHR 16=DispAcc SHR 16) AND (Mask AND MModAbs<>0) THEN
     BEGIN
      AdrType:=ModAbs; AdrCnt:=2;
      AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc);
     END
    ELSE
     BEGIN
      AdrType:=ModLAbs; AdrCnt:=2; AdrMode:=DispAcc SHR 16;
      AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc);
     END
   ELSE IF (Mask AND MModMReg<>0) AND (DispAcc>=SFRStart) AND (DispAcc<=SFREnd) AND (NOT Odd(DispAcc)) THEN
    BEGIN
     AdrType:=ModMReg; AdrCnt:=1; AdrVals[0]:=(DispAcc-SFRStart) SHR 1;
    END
   ELSE CASE MemMode OF
   MemModeStd:
    BEGIN
     z:=0;
     WHILE (z<=3) AND (DispAcc SHR 14<>DPPAssumes[z]) DO Inc(z);
     IF z>3 THEN
      BEGIN
       WrError(110); z:=(DispAcc SHR 14) AND 3;
      END;
     AdrType:=ModAbs; AdrCnt:=2;
     AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=(Hi(DispAcc) AND $3f)+(z SHL 6);
     IF DPPChanged[z] THEN WrXError(200,RegNames[z]);
    END;
   MemModeZeroPage:
    BEGIN
     AdrType:=ModAbs; AdrCnt:=2;
     AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc) AND $3f;
    END;
   MemModeFixedPage:
    BEGIN
     IF DispAcc SHR 14<>MemPage THEN WrError(110);
     AdrType:=ModAbs; AdrCnt:=2;
     AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc) AND $3f;
    END;
   MemModeNoCheck:
    BEGIN
     AdrType:=ModAbs; AdrCnt:=2;
     AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc);
    END;
   MemModeFixedBank:
    BEGIN
     IF DispAcc SHR 16<>MemPage THEN WrError(110);
     AdrType:=ModAbs; AdrCnt:=2;
     AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc);
    END;
   END;

   IF (AdrType<>ModNone) AND (Dest) THEN
    CASE Word(DispAcc) OF
    SPAdr:N_SPChanged:=True;
    CPAdr:N_CPChanged:=True;
    DPPAdr..DPPAdr+(2*DPPCount)-1:N_DPPChanged[(DispAcc-DPPAdr) SHR 1]:=True;
    END;
END;

BEGIN
   AdrType:=ModNone; AdrCnt:=0;

   { immediate ? }

   IF Asc[1]='#' THEN
    BEGIN
     CASE OpSize OF
     0:BEGIN
	AdrVals[0]:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),Int8,OK);
	AdrVals[1]:=0;
       END;
     1:BEGIN
	HDisp:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),Int16,OK);
	AdrVals[0]:=Lo(HDisp); AdrVals[1]:=Hi(HDisp);
       END;
     END;
     IF OK THEN
      BEGIN
       AdrType:=ModImm; AdrCnt:=OpSize+1;
      END;
    END

   { Register ? }

   ELSE IF IsReg(Asc,AdrMode,OpSize=1) THEN
    BEGIN
     IF Mask AND MModReg<>0 THEN AdrType:=ModReg
     ELSE
      BEGIN
       AdrType:=ModMReg; AdrVals[0]:=$f0+AdrMode; AdrCnt:=1;
      END;
     IF CPChanged THEN WrXError(200,RegNames[4]);
    END

   { indirekt ? }

   ELSE IF (Asc[1]='[') AND (Asc[Length(Asc)]=']') THEN
    BEGIN
     Asc:=Copy(Asc,2,Length(Asc)-2);

     { Predekrement ? }

     IF (Length(Asc)>2) AND (Asc[1]='-') AND (IsReg(Copy(Asc,2,Length(Asc)-1),AdrMode,True)) THEN
      AdrType:=ModPreDec

     { Postinkrement ? }

     ELSE IF (Length(Asc)>2) AND (Asc[Length(Asc)]='+') AND (IsReg(Copy(Asc,1,Length(Asc)-1),AdrMode,True)) THEN
      AdrType:=ModPostInc

     { indiziert ? }

     ELSE
      BEGIN
       NegFlag:=False; DispAcc:=0; AdrMode:=$ff;
       WHILE Asc<>'' DO
	BEGIN
	 MPos:=QuotPos(Asc,'-'); PPos:=QuotPos(Asc,'+');
	 IF MPos<PPos THEN
	  BEGIN
	   PPos:=MPos; NNegFlag:=True;
	  END
	 ELSE NNegFlag:=False;
	 Part:=Asc; SplitString(Part,Part,Asc,PPos);
	 IF IsReg(Part,HReg,True) THEN
	  IF (NegFlag) OR (AdrMode<>$ff) THEN WrError(1350) ELSE AdrMode:=HReg
	 ELSE
	  BEGIN
	   IF Part[1]='#' THEN Delete(Part,1,1);
	   HDisp:=EvalIntExpression(Part,Int32,OK);
	   IF OK THEN
	    IF NegFlag THEN Dec(DispAcc,HDisp) ELSE Inc(DispAcc,HDisp);
	  END;
	 NegFlag:=NNegFlag;
	END;
       IF AdrMode=$ff THEN DecideAbsolute
       ELSE IF DispAcc=0 THEN AdrType:=ModIReg
       ELSE IF (DispAcc>$ffff) THEN WrError(1320)
       ELSE IF (DispAcc<-$8000) THEN WrError(1315)
       ELSE
	BEGIN
	 AdrVals[0]:=Lo(DispAcc); AdrVals[1]:=Hi(DispAcc);
	 AdrType:=ModIndex; AdrCnt:=2;
	END;
      END;
    END
   ELSE
    BEGIN
     DispAcc:=EvalIntExpression(Asc,MemInt,OK);
     IF OK THEN DecideAbsolute;
    END;

   IF (AdrType<>-1) AND ((1 SHL AdrType) AND Mask=0) THEN
    BEGIN
     WrError(1350); AdrType:=ModNone; AdrCnt:=0;
    END;
END;

        FUNCTION DecodeCondition(Name:String):Integer;
VAR
   z:Integer;
BEGIN
   z:=1; NLS_UpString(Name);
   WHILE (z<=ConditionCount) AND (Conditions^[z].Name<>Name) DO Inc(z);
   DecodeCondition:=z;
END;

	FUNCTION DecodeBitAddr(Asc:String; VAR Adr:Word; VAR Bit:Byte; MayBeOut:Boolean):Boolean;
VAR
   p:Integer;
   LAdr:Word;
   Reg:Byte;
   OK:Boolean;
BEGIN
   DecodeBitAddr:=False; p:=QuotPos(Asc,'.');
   IF (p<=1) THEN Exit
   ELSE IF p>Length(Asc) THEN
    BEGIN
     LAdr:=EvalIntExpression(Asc,UInt16,OK) AND $1fff;
     IF OK THEN
      BEGIN
       IF (NOT MayBeOut) AND (LAdr SHR 12<>Ord(ExtSFRs)) THEN
	BEGIN
	 WrError(1335); Exit;
	END;
       Adr:=LAdr SHR 4; Bit:=LAdr AND 15; DecodeBitAddr:=True;
       IF NOT MayBeOut THEN Adr:=Lo(Adr);
      END;
    END
   ELSE
    BEGIN
     IF IsReg(Copy(Asc,1,p-1),Reg,True) THEN Adr:=$f0+Reg
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       LAdr:=EvalIntExpression(Copy(Asc,1,p-1),UInt16,OK); IF NOT OK THEN Exit;
       IF FirstPassUnknown THEN LAdr:=$fd00;
       IF Odd(LAdr) THEN
	BEGIN
	 WrError(1325); Exit;
	END;
       IF (LAdr>=$fd00) AND (LAdr<=$fdfe) THEN Adr:=(LAdr-$fd00) DIV 2
       ELSE IF (LAdr>=$ff00) AND (LAdr<=$ffde) THEN
	BEGIN
	 IF (ExtSFRs) AND (NOT MayBeOut) THEN
	  BEGIN
	   WrError(1335); Exit;
	  END;
	 Adr:=$80+((LAdr-$ff00) DIV 2);
	END
       ELSE IF (LAdr>=$f100) AND (LAdr<=$f1de) THEN
	BEGIN
	 IF (NOT ExtSFRs) AND (NOT MayBeOut) THEN
	  BEGIN
	   WrError(1335); Exit;
	  END;
	 Adr:=$80+((LAdr-$f100) DIV 2);
	 IF MayBeOut THEN Inc(Adr,$100);
	END
       ELSE
	BEGIN
	 WrError(1320); Exit;
	END;
      END;

     Bit:=EvalIntExpression(Copy(Asc,p+1,Length(Asc)-p),UInt4,OK);
     DecodeBitAddr:=OK;
    END;
END;

	FUNCTION WordVal:Word;
BEGIN
   WordVal:=AdrVals[0]+Word(AdrVals[1]) SHL 8;
END;

	FUNCTION DecodePref(Asc:String; VAR Erg:Byte):Boolean;
VAR
   OK:Boolean;
BEGIN
   DecodePref:=False;
   IF Asc[1]<>'#' THEN
    BEGIN
     WrError(1350); Exit;
    END;
   Delete(Asc,1,1);
   FirstPassUnknown:=False;
   Erg:=EvalIntExpression(Asc,UInt3,OK);
   IF FirstPassUnknown THEN Erg:=1;
   IF NOT OK THEN Exit;
   IF Erg<1 THEN WrError(1315)
   ELSE IF Erg>4 THEN WrError(1320)
   ELSE
    BEGIN
     DecodePref:=True;
     Dec(Erg);
    END;
END;

{---------------------------------------------------------------------------}

CONST
   ASSUME166Count=4;
   ASSUME166s:ARRAY[1..ASSUME166Count] OF ASSUMERec=
	     ((Name:'DPP0'; Dest:@DPPAssumes[0]; Min:0; Max:15; NothingVal:-1),
	      (Name:'DPP1'; Dest:@DPPAssumes[1]; Min:0; Max:15; NothingVal:-1),
	      (Name:'DPP2'; Dest:@DPPAssumes[2]; Min:0; Max:15; NothingVal:-1),
	      (Name:'DPP3'; Dest:@DPPAssumes[3]; Min:0; Max:15; NothingVal:-1));

	FUNCTION DecodePseudo:Boolean;
VAR
   Adr:Word;
   Bit:Byte;
BEGIN
   DecodePseudo:=True;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME166s,ASSUME166Count);
     Exit;
    END;

   IF Memo('BIT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF DecodeBitAddr(ArgStr[1],Adr,Bit,True) THEN
      BEGIN
       PushLocHandle(-1);
       EnterIntSymbol(LabPart,(Adr SHL 4)+Bit,SegNone,False);
       PopLocHandle;
       ListLine:='='+HexString(Adr,2)+'H.'+HexString(Bit,1);
      END;
     Exit;
    END;

   IF Memo('REG') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE AddRegDef(LabPart,ArgStr[1]);
     Exit;
    END;

   DecodePseudo:=False;
END;

	FUNCTION BMemo(Name:String):Boolean;
BEGIN
   IF Memo(Name) THEN
    BEGIN
     BMemo:=True; OpSize:=1;
    END
   ELSE IF Memo(Name+'B') THEN
    BEGIN
     BMemo:=True; OpSize:=0;
    END
   ELSE BMemo:=False;
END;

	PROCEDURE MakeCode_166;
	Far;
VAR
   z,Cond:Integer;
   AdrWord:Word;
   AdrBank,HReg:Byte;
   BOfs1,BOfs2:Byte;
   BAdr1,BAdr2:Word;
   AdrLong:LongInt;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=1;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   { Pipeline-Flags weiterschalten }

   SPChanged:=N_SPChanged; N_SPChanged:=False;
   CPChanged:=N_CPChanged; N_CPChanged:=False;
   FOR z:=0 TO DPPCount-1 DO
    BEGIN
     DPPChanged[z]:=N_DPPChanged[z];
     N_DPPChanged[z]:=False;
    END;

   { PrÑfixe herunterzÑhlen }

   IF ExtCounter>=0 THEN
    BEGIN
     Dec(ExtCounter);
     IF ExtCounter<0 THEN
      BEGIN
       MemMode:=MemModeStd;
       ExtSFRs:=False;
      END;
    END;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE
	BEGIN
	 CodeLen:=2;
	 BAsmCode[0]:=Lo(Code1); BAsmCode[1]:=Hi(Code1);
	 IF Code2<>0 THEN
	  BEGIN
	   CodeLen:=4;
	   BAsmCode[2]:=Lo(Code2); BAsmCode[3]:=Hi(Code2);
	   IF (Copy(Name,1,3)='RET') AND (SPChanged) THEN WrXError(200,RegNames[5]);
	  END;
	END;
       Exit;
      END;

   { Datentransfer }

   IF BMemo('MOV') THEN
    BEGIN
     Cond:=1-OpSize;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModMReg+MModIReg+MModPreDec+MModPostInc+MModIndex+MModAbs,False,True);
       CASE AdrType OF
       ModReg:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg+MModImm+MModIReg+MModPostInc+MModIndex+MModAbs,False,False);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$f0+Cond;
	   BAsmCode[1]:=(HReg SHL 4)+AdrMode;
	  END;
	 ModImm:
	  IF WordVal<=15 THEN
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$e0+Cond;
	    BAsmCode[1]:=(WordVal SHL 4)+HReg;
	   END
	  ELSE
	   BEGIN
	    CodeLen:=4; BAsmCode[0]:=$e6+Cond; BAsmCode[1]:=HReg+$f0;
	    Move(AdrVals,BAsmCode[2],2);
	   END;
	 ModIReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$a8+Cond;
	   BAsmCode[1]:=(HReg SHL 4)+AdrMode;
	  END;
	 ModPostInc:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$98+Cond;
	   BAsmCode[1]:=(HReg SHL 4)+AdrMode;
	  END;
	 ModIndex:
	  BEGIN
	   CodeLen:=2+AdrCnt; BAsmCode[0]:=$d4+(Cond SHL 5);
	   BAsmCode[1]:=(HReg SHL 4)+AdrMode; Move(AdrVals,BAsmCode[2],AdrCnt);
	  END;
	 ModAbs:
	  BEGIN
	   CodeLen:=2+AdrCnt; BAsmCode[0]:=$f2+Cond;
	   BAsmCode[1]:=$f0+HReg; Move(AdrVals,BAsmCode[2],AdrCnt);
	  END;
	 END;
	END;
       ModMReg:
	BEGIN
	 BAsmCode[1]:=AdrVals[0];
	 IF DPPAssumes[3]=3 THEN
	  DecodeAdr(ArgStr[2],MModImm+MModMReg+MModIReg+MModAbs,False,False)
	 ELSE
	  DecodeAdr(ArgStr[2],MModImm+MModMReg+MModAbs,False,False);
	 CASE AdrType OF
	 ModImm:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$e6+Cond;
	   Move(AdrVals,BAsmCode[2],2);
	  END;
         ModMReg: { BAsmCode[1] sicher absolut darstellbar, da Rn vorher }
          BEGIN   { abgefangen wird! }
           BAsmCode[0]:=$f6+Cond;
           AdrLong:=$fe00+Word(BAsmCode[1]) SHL 1;
           IF CalcPage(AdrLong,True) THEN;
	   BAsmCode[2]:=Lo(AdrLong);
           BAsmCode[3]:=Hi(AdrLong);
           BAsmCode[1]:=AdrVals[0];
           CodeLen:=4;
          END;
	 ModIReg:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$94+(Cond SHL 5);
	   BAsmCode[2]:=BAsmCode[1] SHL 1;
	   BAsmCode[3]:=$fe+(BAsmCode[1] SHR 7);
	   BAsmCode[1]:=AdrMode;
	  END;
	 ModAbs:
	  BEGIN
	   CodeLen:=2+AdrCnt; BAsmCode[0]:=$f2+Cond;
	   Move(AdrVals,BAsmCode[2],AdrCnt);
	  END;
	 END;
	END;
       ModIReg:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPostInc+MModAbs,False,False);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$b8+Cond;
	   BAsmCode[1]:=HReg+(AdrMode SHL 4);
	  END;
	 ModIReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$c8+Cond;
	   BAsmCode[1]:=(HReg SHL 4)+AdrMode;
	  END;
	 ModPostInc:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$e8+Cond;
	   BAsmCode[1]:=(HReg SHL 4)+AdrMode;
	  END;
	 ModAbs:
	  BEGIN
	   CodeLen:=2+AdrCnt; BAsmCode[0]:=$84+(Cond SHL 5);
	   BAsmCode[1]:=HReg; Move(AdrVals,BAsmCode[2],AdrCnt);
	  END;
	 END;
	END;
       ModPreDec:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModReg,False,False);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$88+Cond;
	   BAsmCode[1]:=HReg+(AdrMode SHL 4);
	  END;
	 END;
	END;
       ModPostInc:
	BEGIN
	 HReg:=AdrMode;
	 DecodeAdr(ArgStr[2],MModIReg,False,False);
	 CASE AdrType OF
	 ModIReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$d8+Cond;
	   BAsmCode[1]:=(HReg SHL 4)+AdrMode;
	  END;
	 END;
	END;
       ModIndex:
	BEGIN
	 BAsmCode[1]:=AdrMode; Move(AdrVals,BAsmCode[2],AdrCnt);
	 DecodeAdr(ArgStr[2],MModReg,False,False);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   BAsmCode[0]:=$c4+(Cond SHL 5);
	   CodeLen:=4; Inc(BAsmCode[1],AdrMode SHL 4);
	  END;
	 END;
	END;
       ModAbs:
	BEGIN
	 Move(AdrVals,BAsmCode[2],AdrCnt);
	 DecodeAdr(ArgStr[2],MModIReg+MModMReg,False,False);
	 CASE AdrType OF
	 ModIReg:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$94+(Cond SHL 5);
	   BAsmCode[1]:=AdrMode;
	  END;
	 ModMReg:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$f6+Cond;
	   BAsmCode[1]:=AdrVals[0];
	  END;
	 END;
	END;
       END;
      END;
     Exit;
    END;

   IF (Memo('MOVBS')) OR (Memo('MOVBZ')) THEN
    BEGIN
     Cond:=Ord(Memo('MOVBS')) SHL 4;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=1;
       DecodeAdr(ArgStr[1],MModReg+MModMReg+MModAbs,False,True);
       OpSize:=0;
       CASE AdrType OF
       ModReg:
	BEGIN
         HReg:=AdrMode; DecodeAdr(ArgStr[2],MModReg+MModAbs,False,False);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$c0+Cond;
	   BAsmCode[1]:=HReg+(AdrMode SHL 4);
	  END;
         ModAbs:
	  BEGIN
           CodeLen:=4; BAsmCode[0]:=$c2+Cond;
           BAsmCode[1]:=$f0+HReg;
	   Move(AdrVals,BAsmCode[2],AdrCnt);
          END;
         END;
	END;
       ModMReg:
	BEGIN
	 BAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[2],MModAbs+MModMReg,False,False);
	 CASE AdrType OF
         ModMReg: { BAsmCode[1] sicher absolut darstellbar, da Rn vorher }
          BEGIN   { abgefangen wird! }
           BAsmCode[0]:=$c5+Cond;
           AdrLong:=$fe00+Word(BAsmCode[1]) SHL 1;
           IF CalcPage(AdrLong,True) THEN;
	   BAsmCode[2]:=Lo(AdrLong);
           BAsmCode[3]:=Hi(AdrLong);
           BAsmCode[1]:=AdrVals[0];
           CodeLen:=4;
          END;
	 ModAbs:
	  BEGIN
	   CodeLen:=2+AdrCnt; BAsmCode[0]:=$c2+Cond;
	   Move(AdrVals,BAsmCode[2],AdrCnt);
	  END;
	 END;
	END;
       ModAbs:
	BEGIN
	 Move(AdrVals,BAsmCode[2],AdrCnt);
	 DecodeAdr(ArgStr[2],MModMReg,False,False);
	 CASE AdrType OF
	 ModMReg:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$c5+Cond;
	   BAsmCode[1]:=AdrVals[0];
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
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModMReg,False,Memo('POP'));
       CASE AdrType OF
       ModMReg:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$ec+(Ord(Memo('POP')) SHL 4);
	 BAsmCode[1]:=AdrVals[0];
	 IF SPChanged THEN WrXError(200,RegNames[5]);
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('SCXT') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModMReg,False,True);
       CASE AdrType OF
       ModMReg:
	BEGIN
	 BAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[2],MModAbs+MModImm,False,False);
	 IF AdrType<>ModNone THEN
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$c6+(Ord(AdrType=ModAbs) SHL 4);
	   Move(AdrVals,BAsmCode[2],2);
	  END;
	END;
       END;
      END;
     Exit;
    END;

   { Arithmetik }

   FOR z:=0 TO ALU2OrderCount-1 DO
    IF BMemo(ALU2Orders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	Cond:=(1-OpSize)+(z SHL 4);
	DecodeAdr(ArgStr[1],MModReg+MModMReg+MModAbs,False,True);
	CASE AdrType OF
	ModReg:
	 BEGIN
	  HReg:=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPostInc+MModAbs+MModImm,False,False);
	  CASE AdrType OF
	  ModReg:
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=Cond;
	    BAsmCode[1]:=(HReg SHL 4)+AdrMode;
	   END;
	  ModIReg:
	   IF AdrMode>3 THEN WrError(1350)
	   ELSE
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$08+Cond;
	     BAsmCode[1]:=(HReg SHL 4)+8+AdrMode;
	    END;
	  ModPostInc:
	   IF AdrMode>3 THEN WrError(1350)
	   ELSE
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$08+Cond;
	     BAsmCode[1]:=(HReg SHL 4)+12+AdrMode;
	    END;
	  ModAbs:
	   BEGIN
	    CodeLen:=4; BAsmCode[0]:=$02+Cond; BAsmCode[1]:=$f0+HReg;
	    Move(AdrVals,BAsmCode[2],AdrCnt);
	   END;
	  ModImm:
	   IF WordVal<=7 THEN
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$08+Cond;
	     BAsmCode[1]:=(HReg SHL 4)+AdrVals[0];
	    END
	   ELSE
	    BEGIN
	    CodeLen:=4; BAsmCode[0]:=$06+Cond; BAsmCode[1]:=$f0+HReg;
	    Move(AdrVals,BAsmCode[2],2);
	    END;
	  END;
	 END;
	ModMReg:
	 BEGIN
	  BAsmCode[1]:=AdrVals[0];
	  DecodeAdr(ArgStr[2],MModAbs+MModMReg+MModImm,False,False);
	  CASE AdrType OF
	  ModAbs:
	   BEGIN
	    CodeLen:=4; BAsmCode[0]:=$02+Cond;
	    Move(AdrVals,BAsmCode[2],AdrCnt);
	   END;
          ModMReg: { BAsmCode[1] sicher absolut darstellbar, da Rn vorher }
           BEGIN   { abgefangen wird! }
            BAsmCode[0]:=$04+Cond;
            AdrLong:=$fe00+Word(BAsmCode[1]) SHL 1;
            IF CalcPage(AdrLong,True) THEN;
	    BAsmCode[2]:=Lo(AdrLong);
            BAsmCode[3]:=Hi(AdrLong);
            BAsmCode[1]:=AdrVals[0];
            CodeLen:=4;
           END;
	  ModImm:
	   BEGIN
	    CodeLen:=4; BAsmCode[0]:=$06+Cond;
	    Move(AdrVals,BAsmCode[2],2);
	   END;
	  END;
	 END;
	ModAbs:
	 BEGIN
	  Move(AdrVals,BAsmCode[2],AdrCnt);
	  DecodeAdr(ArgStr[2],MModMReg,False,False);
	  CASE AdrType OF
	  ModMReg:
	   BEGIN
	    CodeLen:=4; BAsmCode[0]:=$04+Cond; BAsmCode[1]:=AdrVals[0];
	   END;
	  END;
	 END;
	END;
       END;
      Exit;
     END;

   IF (BMemo('CPL')) OR (BMemo('NEG')) THEN
    BEGIN
     Cond:=$81+((1-OpSize) SHL 5);
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg,False,True);
       IF AdrType=ModReg THEN
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=Cond+(Ord(BMemo('CPL')) SHL 4);
	 BAsmCode[1]:=AdrMode SHL 4;
	END;
      END;
     Exit;
    END;

   FOR z:=0 TO DivOrderCount-1 DO
    IF Memo(DivOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],MModReg,False,False);
	IF AdrType=ModReg THEN
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$4b+(z SHL 4);
	  BAsmCode[1]:=AdrMode*$11;
	 END;
       END;
      Exit;
     END;

   FOR z:=1 TO LoopOrderCount DO
    WITH LoopOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg,False,True);
	 IF AdrType=ModReg THEN
	  BEGIN
	   BAsmCode[1]:=AdrMode;
	   DecodeAdr(ArgStr[2],MModAbs+MModImm,False,False);
	   CASE AdrType OF
	   ModAbs:
	    BEGIN
	     CodeLen:=4; BAsmCode[0]:=Code+2; Inc(BAsmCode[1],$f0);
	     Move(AdrVals,BAsmCode[2],2);
	    END;
	   ModImm:
	    IF WordVal<16 THEN
	     BEGIN
	      CodeLen:=2; BAsmCode[0]:=Code; Inc(BAsmCode[1],WordVal SHL 4);
	     END
	    ELSE
	     BEGIN
	      CodeLen:=4; BAsmCode[0]:=Code+6; Inc(BAsmCode[1],$f0);
	      Move(AdrVals,BAsmCode[2],2);
	     END;
	   END;
	  END;
	END;
       Exit;
      END;

   FOR z:=0 TO MulOrderCount-1 DO
    IF Memo(MulOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],MModReg,False,False);
	CASE AdrType OF
	ModReg:
	 BEGIN
	  HReg:=AdrMode;
	  DecodeAdr(ArgStr[2],MModReg,False,False);
	  CASE AdrType OF
	  ModReg:
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$0b+(z SHL 4);
	    BAsmCode[1]:=(HReg SHL 4)+AdrMode;
	   END;
	  END;
	 END;
	END;
       END;
      Exit;
     END;

   { Logik }

   FOR z:=1 TO ShiftOrderCount DO
    WITH ShiftOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 OpSize:=1;
	 DecodeAdr(ArgStr[1],MModReg,False,True);
	 CASE AdrType OF
	 ModReg:
	  BEGIN
	   HReg:=AdrMode;
	   DecodeAdr(ArgStr[2],MModReg+MModImm,False,True);
	   CASE AdrType OF
	   ModReg:
	    BEGIN
	     BAsmCode[0]:=Code; BAsmCode[1]:=AdrMode+(HReg SHL 4);
	     CodeLen:=2;
	    END;
	   ModImm:
	    IF WordVal>15 THEN WrError(1320)
	    ELSE
	     BEGIN
	     BAsmCode[0]:=Code+$10; BAsmCode[1]:=(WordVal SHL 4)+HReg;
	     CodeLen:=2;
	     END;
	   END;
	  END;
	 END;
	END;
       Exit;
      END;

   FOR z:=1 TO Bit2OrderCount DO
    WITH Bit2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	IF DecodeBitAddr(ArgStr[1],BAdr1,BOfs1,False) THEN
	IF DecodeBitAddr(ArgStr[2],BAdr2,BOfs2,False) THEN
	 BEGIN
	  CodeLen:=4; BAsmCode[0]:=Code;
	  BAsmCode[1]:=BAdr2; BAsmCode[2]:=BAdr1;
	  BAsmCode[3]:=(BOfs2 SHL 4)+BOfs1;
	 END;
       Exit;
      END;

   IF (Memo('BCLR')) OR (Memo('BSET')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF DecodeBitAddr(ArgStr[1],BAdr1,BOfs1,False) THEN
      BEGIN
       CodeLen:=2; BAsmCode[0]:=(BOfs1 SHL 4)+$0e+Ord(Memo('BSET'));
       BAsmCode[1]:=BAdr1;
      END;
     Exit;
    END;

   IF (Memo('BFLDH')) OR (Memo('BFLDL')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF DecodeBitAddr(ArgStr[1]+'.0',BAdr1,BOfs1,False) THEN
      BEGIN
       OpSize:=0; BAsmCode[1]:=BAdr1;
       DecodeAdr(ArgStr[2],MModImm,False,False);
       IF AdrType=ModImm THEN
	BEGIN
	 BAsmCode[2]:=AdrVals[0];
	 DecodeAdr(ArgStr[3],MModImm,False,False);
         IF AdrType=ModImm THEN
	  BEGIN
	   BAsmCode[3]:=AdrVals[0];
	   CodeLen:=4; BAsmCode[0]:=$0a;
	   IF Memo('BFLDH') THEN
	    BEGIN
	     BAdr1:=BAsmCode[2]; BAsmCode[2]:=BAsmCode[3]; BAsmCode[3]:=BAdr1;
	     Inc(BAsmCode[0],$10);
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   {SprÅnge }

   IF Memo('JMP') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN Cond:=TrueCond
       ELSE Cond:=DecodeCondition(ArgStr[1]);
       IF Cond>ConditionCount THEN WrXError(1360,ArgStr[1])
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[ArgCnt],MModAbs+MModLAbs+MModIReg,True,False);
	 CASE AdrType OF
	 ModLAbs:
	  IF Cond<>TrueCond THEN WrXError(1360,ArgStr[1])
	  ELSE
	   BEGIN
	    CodeLen:=2+AdrCnt; BAsmCode[0]:=$fa; BAsmCode[1]:=AdrMode;
	    Move(AdrVals,BAsmCode[2],AdrCnt);
	   END;
	 ModAbs:
	  BEGIN
	   AdrLong:=WordVal-(EProgCounter+2);
	   IF (AdrLong<=254) AND (AdrLong>=-256) AND (NOT Odd(AdrLong)) THEN
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$0d+(Conditions^[Cond].Code SHL 4);
	     BAsmCode[1]:=(AdrLong DIV 2) AND $ff;
	    END
	   ELSE
	    BEGIN
	     CodeLen:=2+AdrCnt; BAsmCode[0]:=$ea;
	     BAsmCode[1]:=Conditions^[Cond].Code SHL 4;
	     Move(AdrVals,BAsmCode[2],AdrCnt);
	    END
	  END;
	 ModIReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$9c;
	   BAsmCode[1]:=Conditions^[Cond].Code SHL 4+AdrMode;
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   IF Memo('CALL') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN Cond:=TrueCond
       ELSE Cond:=DecodeCondition(ArgStr[1]);
       IF Cond>ConditionCount THEN WrXError(1360,ArgStr[1])
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[ArgCnt],MModAbs+MModLAbs+MModIReg,True,False);
	 CASE AdrType OF
	 ModLAbs:
	  IF Cond<>TrueCond THEN WrXError(1360,ArgStr[1])
	  ELSE
	   BEGIN
	    CodeLen:=2+AdrCnt; BAsmCode[0]:=$da; BAsmCode[1]:=AdrMode;
	    Move(AdrVals,BAsmCode[2],AdrCnt);
	   END;
	 ModAbs:
	  BEGIN
	   AdrLong:=WordVal-(EProgCounter+2);
	   IF (AdrLong<=254) AND (AdrLong>=-256) AND (NOT Odd(AdrLong)) AND (Cond=TrueCond) THEN
	    BEGIN
	     CodeLen:=2; BAsmCode[0]:=$bb;
	     BAsmCode[1]:=(AdrLong DIV 2) AND $ff;
	    END
	   ELSE
	    BEGIN
	     CodeLen:=2+AdrCnt; BAsmCode[0]:=$ca;
             BAsmCode[1]:=$00+Conditions^[Cond].Code SHL 4;
	     Move(AdrVals,BAsmCode[2],AdrCnt);
	    END
	  END;
	 ModIReg:
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=$ab;
	   BAsmCode[1]:=Conditions^[Cond].Code SHL 4+AdrMode;
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   IF Memo('JMPR') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN Cond:=TrueCond
       ELSE Cond:=DecodeCondition(ArgStr[1]);
       IF Cond>ConditionCount THEN WrXError(1360,ArgStr[1])
       ELSE
	BEGIN
	 AdrLong:=EvalIntExpression(ArgStr[ArgCnt],MemInt,OK)-(EProgCounter+2);
	 IF OK THEN
	  IF Odd(AdrLong) THEN WrError(1375)
          ELSE IF (NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256)) THEN WrError(1370)
	  ELSE
	   BEGIN
	    CodeLen:=2; BAsmCode[0]:=$0d+(Conditions^[Cond].Code SHL 4);
	    BAsmCode[1]:=(AdrLong DIV 2) AND $ff;
	   END;
	END;
      END;
     Exit;
    END;

   IF Memo('CALLR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrLong:=EvalIntExpression(ArgStr[ArgCnt],MemInt,OK)-(EProgCounter+2);
       IF OK THEN
	IF Odd(AdrLong) THEN WrError(1375)
        ELSE IF (NOT SymbolQuestionable) AND ((AdrLong>254) OR (AdrLong<-256)) THEN WrError(1370)
	ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=$bb;
	  BAsmCode[1]:=(AdrLong DIV 2) AND $ff;
	 END;
      END;
     Exit;
    END;

   IF (Memo('JMPA')) OR (Memo('CALLA')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN Cond:=TrueCond
       ELSE Cond:=DecodeCondition(ArgStr[1]);
       IF Cond>ConditionCount THEN WrXError(1360,ArgStr[1])
       ELSE
	BEGIN
	 AdrLong:=EvalIntExpression(ArgStr[ArgCnt],MemInt,OK);
	 IF OK THEN
	  IF AdrLong SHR 16<>EProgCounter SHR 16 THEN WrError(1910)
	  ELSE
	   BEGIN
	    CodeLen:=4;
	    IF Memo('JMPA') THEN BAsmCode[0]:=$ea ELSE BAsmCode[0]:=$ca;
	    BAsmCode[1]:=$00+(Conditions^[Cond].Code SHL 4);
	    BAsmCode[2]:=Lo(AdrLong); BAsmCode[3]:=Hi(AdrLong);
	   END;
	END;
      END;
     Exit;
    END;

   IF (Memo('JMPS')) OR (Memo('CALLS')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN
	BEGIN
	 AdrLong:=EvalIntExpression(ArgStr[1],MemInt,OK);
	 AdrWord:=AdrLong AND $ffff; AdrBank:=AdrLong SHR 16;
	END
       ELSE
	BEGIN
	 AdrWord:=EvalIntExpression(ArgStr[2],UInt16,OK);
	 IF OK THEN AdrBank:=EvalIntExpression(ArgStr[1],MemInt2,OK);
	END;
       IF OK THEN
	BEGIN
	 CodeLen:=4;
	 IF Memo('JMPS') THEN BAsmCode[0]:=$fa ELSE BAsmCode[0]:=$da;
	 BAsmCode[1]:=AdrBank;
	 BAsmCode[2]:=Lo(AdrWord); BAsmCode[3]:=Hi(AdrWord);
	END;
      END;
     Exit;
    END;

   IF (Memo('JMPI')) OR (Memo('CALLI')) THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgCnt=1 THEN Cond:=TrueCond
       ELSE Cond:=DecodeCondition(ArgStr[1]);
       IF Cond>ConditionCount THEN WrXError(1360,ArgStr[1])
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[ArgCnt],MModIReg,True,False);
	 CASE AdrType OF
	 ModIReg:
	  BEGIN
	   CodeLen:=2;
	   IF Memo('JMPI') THEN BAsmCode[0]:=$9c ELSE BAsmCode[0]:=$ab;
	   BAsmCode[1]:=AdrMode+(Conditions^[Cond].Code SHL 4);
	  END;
	 END;
	END;
      END;
     Exit;
    END;

   FOR z:=0 TO BJmpOrderCount-1 DO
    IF Memo(BJmpOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF DecodeBitAddr(ArgStr[1],BAdr1,BOfs1,False) THEN
       BEGIN
	AdrLong:=EvalIntExpression(ArgStr[2],MemInt,OK)-(EProgCounter+4);
	IF OK THEN
	 IF Odd(AdrLong) THEN WrError(1375)
         ELSE IF (NOT SymbolQuestionable) AND ((AdrLong<-256) OR (AdrLong>254)) THEN WrError(1370)
	 ELSE
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$8a+(z SHL 4);
	   BAsmCode[1]:=BAdr1;
	   BAsmCode[2]:=(AdrLong DIV 2) AND $ff;
	   BAsmCode[3]:=BOfs1 SHL 4;
	  END;
       END;
      Exit;
     END;

   IF Memo('PCALL') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModMReg,False,False);
       CASE AdrType OF
       ModMReg:
	BEGIN
	 BAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[2],MModAbs,True,False);
	 CASE AdrType OF
	 ModAbs:
	  BEGIN
	   CodeLen:=4; BAsmCode[0]:=$e2; Move(AdrVals,BAsmCode[2],2);
	  END;
	 END;
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('RETP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModMReg,False,False);
       CASE AdrType OF
       ModMReg:
	BEGIN
	 BAsmCode[1]:=AdrVals[0]; BAsmCode[0]:=$eb; CodeLen:=2;
	 IF SPChanged THEN WrXError(200,RegNames[5]);
	END;
       END;
      END;
     Exit;
    END;

   IF Memo('TRAP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       BAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt7,OK) SHL 1;
       IF OK THEN
	BEGIN
	 BAsmCode[0]:=$9b; CodeLen:=2;
	END;
      END;
     Exit;
    END;

   { spezielle Steuerbefehle }

   IF Memo('ATOMIC') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU80C163 THEN WrError(1500)
     ELSE IF DecodePref(ArgStr[1],HReg) THEN
      BEGIN
       CodeLen:=2; BAsmCode[0]:=$d1; BAsmCode[1]:=HReg SHL 4;
      END;
     Exit;
    END;

   IF Memo('EXTR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU80C163 THEN WrError(1500)
     ELSE IF DecodePref(ArgStr[1],HReg) THEN
      BEGIN
       CodeLen:=2; BAsmCode[0]:=$d1; BAsmCode[1]:=$80+(HReg SHL 4);
       ExtCounter:=HReg+1; ExtSFRs:=True;
      END;
     Exit;
    END;

   IF (Memo('EXTP')) OR (Memo('EXTPR')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80C163 THEN WrError(1500)
     ELSE IF DecodePref(ArgStr[2],HReg) THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModImm,False,False);
       CASE AdrType OF
       ModReg:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$dc; BAsmCode[1]:=$40+(HReg SHL 4)+AdrMode;
	 IF Memo('EXTPR') THEN Inc(BAsmCode[1],$80);
	 ExtCounter:=HReg+1; MemMode:=MemModeZeroPage;
	END;
       ModImm:
	BEGIN
	 CodeLen:=4; BAsmCode[0]:=$d7; BAsmCode[1]:=$40+(HReg SHL 4);
	 IF Memo('EXTPR') THEN Inc(BAsmCode[1],$80);
         BAsmCode[2]:=WordVal AND $ff; BAsmCode[3]:=(WordVal SHR 8) AND 3;
	 ExtCounter:=HReg+1; MemMode:=MemModeFixedPage; MemPage:=WordVal AND $3ff;
	END;
       END;
      END;
     Exit;
    END;

   IF (Memo('EXTS')) OR (Memo('EXTSR')) THEN
    BEGIN
     OpSize:=0;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU<CPU80C163 THEN WrError(1500)
     ELSE IF DecodePref(ArgStr[2],HReg) THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModReg+MModImm,False,False);
       CASE AdrType OF
       ModReg:
	BEGIN
	 CodeLen:=2; BAsmCode[0]:=$dc; BAsmCode[1]:=$00+(HReg SHL 4)+AdrMode;
	 IF Memo('EXTSR') THEN Inc(BAsmCode[1],$80);
	 ExtCounter:=HReg+1; MemMode:=MemModeNoCheck;
	END;
       ModImm:
	BEGIN
	 CodeLen:=4; BAsmCode[0]:=$d7; BAsmCode[1]:=$00+(HReg SHL 4);
	 IF Memo('EXTSR') THEN Inc(BAsmCode[1],$80);
	 BAsmCode[2]:=AdrVals[0]; BAsmCode[3]:=0;
	 ExtCounter:=HReg+1; MemMode:=MemModeFixedBank; MemPage:=AdrVals[0];
	END;
       END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_166;
	Far;
VAR
   z:Integer;
BEGIN
   SaveInitProc;
   FOR z:=0 TO DPPCount-1 DO
    BEGIN
     DPPAssumes[z]:=z; N_DPPChanged[z]:=False;
    END;
   N_CPChanged:=False; N_SPChanged:=False;

   MemMode:=MemModeStd; ExtSFRs:=False; ExtCounter:=-1;
END;

	FUNCTION ChkPC_166:Boolean;
	Far;
BEGIN
   IF ActPC=SegCode THEN ChkPC_166:=(ProgCounter>=0) AND (ProgCounter<MemEnd)
   ELSE ChkPC_166:=False;
END;

	FUNCTION IsDef_166:Boolean;
	Far;
BEGIN
   IsDef_166:=Memo('BIT') OR Memo('REG');
END;

	PROCEDURE SwitchFrom_166;
	Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_166;
	Far;
VAR
   z:Byte;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;
   OpSize:=1;

   PCSymbol:='$'; HeaderID:=$4c; NOPCode:=$cc00;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_166; ChkPC:=ChkPC_166; IsDef:=IsDef_166;
   SwitchFrom:=SwitchFrom_166;

   IF (MomCPU=CPU80C166) THEN
    BEGIN
     MemInt:=UInt18; MemInt2:=UInt2; MemEnd:=$40000; ASSUME166s[1].Max:=15;
    END
   ELSE
    BEGIN
     MemInt:=UInt24; MemInt2:=UInt8; MemEnd:=$1000000; ASSUME166s[1].Max:=1023;
    END;
   FOR z:=2 TO 4 DO ASSUME166s[z].Max:=ASSUME166s[1].Max;

   InitFields;
END;

BEGIN
   CPU80C166:=AddCPU('80C166',SwitchTo_166);
   CPU80C163:=AddCPU('80C163',SwitchTo_166);
   CPU80C165:=AddCPU('80C165',SwitchTo_166);
   CPU80C167:=AddCPU('80C167',SwitchTo_166);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_166;
END.
