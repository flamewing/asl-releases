{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code97C2;

{ AS - Codegeneratormodul TLCS-9000 }

INTERFACE

        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

TYPE
   FixedOrder=RECORD
	       Name:String[5];
	       Code:Word;
	      END;

   RMWOrder=RECORD
	     Name:String[5];
	     Code:Byte;
	     Mask:Byte;          { B0..2=OpSizes, B4=-MayImm, B5=-MayReg }
	    END;

   GAOrder=RECORD
	    Name:String[5];
	    Code:Byte;
	    Mask:Byte;   { B7: DD in A-Format gedreht }
	    SizeType:(Equal,FirstCounts,SecondCounts,Op2Half);
	    ImmKorr,ImmErl,RegErl:Boolean;
	   END;

CONST
   ConditionCount=20;
   Conditions:ARRAY[0..ConditionCount-1] OF String[3]=
	      ('C','NC','Z','NZ','OV','NOV','MI','PL','LE','GT','LT','GE',
	       'ULE','UGT','N','A','ULT','UGE','EQ','NE');

   FixedOrderCount=20;
   FixedOrders:ARRAY[1..FixedOrderCount] OF FixedOrder=
	       ((Name:'CCF' ; Code:$7f82),
		(Name:'CSF' ; Code:$7f8a),
		(Name:'CVF' ; Code:$7f86),
		(Name:'CZF' ; Code:$7f8e),
		(Name:'DI'  ; Code:$7fa1),
		(Name:'EI'  ; Code:$7fa3),
		(Name:'HALT'; Code:$7fa5),
		(Name:'NOP' ; Code:$7fa0),
		(Name:'RCF' ; Code:$7f80),
		(Name:'RET' ; Code:$7fa4),
		(Name:'RETI'; Code:$7fa9),
		(Name:'RETS'; Code:$7fab),
		(Name:'RSF' ; Code:$7f88),
		(Name:'RVF' ; Code:$7f84),
		(Name:'RZF' ; Code:$7f8c),
		(Name:'SCF' ; Code:$7f81),
		(Name:'SSF' ; Code:$7f89),
		(Name:'SVF' ; Code:$7f85),
		(Name:'SZF' ; Code:$7f8b),
		(Name:'UNLK'; Code:$7fa2));

   RMWOrderCount=14;
   RMWOrders:ARRAY[1..RMWOrderCount] OF RMWOrder=
	     ((Name:'CALL' ; Code:$35; Mask:$36),
	      (Name:'CLR'  ; Code:$2b; Mask:$17),
	      (Name:'CPL'  ; Code:$28; Mask:$17),
	      (Name:'EXTS' ; Code:$33; Mask:$16),
	      (Name:'EXTZ' ; Code:$32; Mask:$16),
	      (Name:'JP'   ; Code:$34; Mask:$36),
	      (Name:'MIRR' ; Code:$23; Mask:$17),
	      (Name:'NEG'  ; Code:$29; Mask:$17),
	      (Name:'POP'  ; Code:$20; Mask:$17),
	      (Name:'PUSH' ; Code:$21; Mask:$07),
	      (Name:'PUSHA'; Code:$31; Mask:$36),
	      (Name:'RVBY' ; Code:$22; Mask:$17),
	      (Name:'TJP'  ; Code:$36; Mask:$16),
	      (Name:'TST'  ; Code:$2a; Mask:$17));

   TrinomOrderCount=4;
   TrinomOrders:ARRAY[0..TrinomOrderCount-1] OF String[4]=('ADD3','SUB3','MAC','MACS');

   StringOrderCount=4;
   StringOrders:ARRAY[0..StringOrderCount-1] OF String[4]=('CPSZ','CPSN','','LDS');

   BFieldOrderCount=3;
   BFieldOrders:ARRAY[0..BFieldOrderCount-1] OF String[5]=('BFEX','BFEXS','BFIN');

   GAEqOrderCount=10;
   GAEqOrders:ARRAY[1..GAEqOrderCount] OF FixedOrder=
	      ((Name:'ABCD' ; Code:$10),
	       (Name:'ADC'  ; Code:$04),
	       (Name:'CBCD' ; Code:$12),
	       (Name:'CPC'  ; Code:$06),
	       (Name:'MAX'  ; Code:$16),
	       (Name:'MAXS' ; Code:$17),
	       (Name:'MIN'  ; Code:$14),
	       (Name:'MINS' ; Code:$15),
	       (Name:'SBC'  ; Code:$05),
	       (Name:'SBCD' ; Code:$11));

   GAFirstOrderCount=6;
   GAFirstOrders:ARRAY[1..GAFirstOrderCount] OF FixedOrder=
		 ((Name:'ANDCF'; Code:$44),
		  (Name:'LDCF' ; Code:$47),
		  (Name:'ORCF' ; Code:$45),
		  (Name:'STCF' ; Code:$43),
		  (Name:'TSET' ; Code:$70),
		  (Name:'XORCF'; Code:$46));

   GASecondOrderCount=4;
   GASecondOrders:ARRAY[1..GASecondOrderCount] OF FixedOrder=
		  ((Name:'BS0B' ; Code:$54),
		   (Name:'BS0F' ; Code:$55),
		   (Name:'BS1B' ; Code:$56),
		   (Name:'BS1F' ; Code:$57));

   GAHalfOrderCount=4;
   GAHalfOrders:ARRAY[1..GAHalfOrderCount] OF FixedOrder=
		  ((Name:'DIV'  ; Code:$26),
		   (Name:'DIVS' ; Code:$27),
		   (Name:'MUL'  ; Code:$24),
		   (Name:'MULS' ; Code:$25));

   GASI1OrderCount=4;
   GASI1Orders:ARRAY[0..GASI1OrderCount-1] OF String[3]=
	       ('ADD','SUB','CP','LD');

   GASI2OrderCount=3;
   GASI2Orders:ARRAY[0..GASI2OrderCount-1] OF String[3]=
	       ('AND','OR','XOR');

   BitOrderCount=4;
   BitOrders:ARRAY[0..BitOrderCount-1] OF String[4]=
	     ('BRES','BSET','BCHG','BTST');

   ShiftOrderCount=8;
   ShiftOrders:ARRAY[0..ShiftOrderCount-1] OF String[3]=
	       ('SLL','SRL','SLA','SRA','RL','RR','RLC','RRC');

VAR
   CPU97C241:CPUVar;

   OpSize,OpSize2:Integer;
   LowLim4,LowLim8:Integer;

   AdrOK:Boolean;
   AdrMode,AdrMode2:Byte;
   AdrCnt,AdrCnt2:Byte;
   AdrVals,AdrVals2:ARRAY[0..1] OF Word;
   AdrInc:Integer;
   Prefs:ARRAY[0..1] OF Word;
   PrefUsed:ARRAY [0..1] OF Boolean;
   Format:Char;
   MinOneIs0:Boolean;


	PROCEDURE AddSignedPrefix(Index:Byte; MaxBits:Byte; Value:LongInt);
VAR
   Max:LongInt;
BEGIN
   Max:=LongInt(1) SHL (MaxBits-1);
   IF (Value<-Max) OR (Value>=Max) THEN
    BEGIN
     PrefUsed[Index]:=True;
     Prefs[Index]:=(Value SHR MaxBits) AND $7ff;
    END;
END;

	FUNCTION AddRelPrefix(Index:Byte; MaxBits:Byte; VAR Value:LongInt):Boolean;
VAR
   Max1,Max2:LongInt;
BEGIN
   Max1:=LongInt(1) SHL (MaxBits-1);
   Max2:=LongInt(1) SHL (MaxBits+10);
   AddRelPrefix:=False;
   IF (Value<-Max2) OR (Value>=Max2) THEN WrError(1370)
   ELSE
    BEGIN
     IF (Value<-Max1) OR (Value>=Max1) THEN
      BEGIN
       PrefUsed[Index]:=True;
       Prefs[Index]:=(Value SHR MaxBits) AND $7ff;
      END;
     AddRelPrefix:=True;
    END;
END;

	PROCEDURE AddAbsPrefix(Index:Byte; MaxBits:Byte; Value:LongInt);
VAR
   Dist:LongInt;
BEGIN
   Dist:=LongInt(1) SHL (MaxBits-1);
   IF (Value>=Dist) AND (Value<$1000000-Dist) THEN
    BEGIN
     PrefUsed[Index]:=True;
     Prefs[Index]:=(Value SHR MaxBits) AND $7ff;
    END;
END;

	PROCEDURE InsertSinglePrefix(Index:Byte);
BEGIN
   IF PrefUsed[Index] THEN
    BEGIN
     Move(WAsmCode[0],WAsmCode[1],CodeLen);
     WAsmCode[0]:=Prefs[Index]+$d000+(Word(Index) SHL 11);
     Inc(CodeLen,2);
    END;
END;


	FUNCTION DecodeReg(Asc:String; VAR Result:Byte):Boolean;
VAR
   tmp:Byte;
   Err:ValErgType;
BEGIN
   DecodeReg:=False;
   IF (Length(Asc)>4) OR (Length(Asc)<3) OR (UpCase(Asc[1])<>'R') THEN Exit;
   CASE UpCase(Asc[2]) OF
   'B':Result:=$00;
   'W':Result:=$40;
   'D':Result:=$80;
   ELSE Exit;
   END;
   Val(Copy(Asc,3,Length(Asc)-2),tmp,Err);
   IF (Err<>0) OR (tmp>15) THEN Exit;
   IF (Result=$80) AND (Odd(tmp)) THEN Exit;
   Inc(Result,tmp); DecodeReg:=True; Exit;
END;

	FUNCTION DecodeSpecReg(VAR Asc:String; VAR Result:Byte):Boolean;
BEGIN
   DecodeSpecReg:=True;
   IF NLS_StrCaseCmp(Asc,'SP')=0 THEN Result:=$8c
   ELSE IF NLS_StrCaseCmp(Asc,'ISP')=0 THEN Result:=$81
   ELSE IF NLS_StrCaseCmp(Asc,'ESP')=0 THEN Result:=$83
   ELSE IF NLS_StrCaseCmp(Asc,'PBP')=0 THEN Result:=$05
   ELSE IF NLS_StrCaseCmp(Asc,'CBP')=0 THEN Result:=$07
   ELSE IF NLS_StrCaseCmp(Asc,'PSW')=0 THEN Result:=$89
   ELSE IF NLS_StrCaseCmp(Asc,'IMC')=0 THEN Result:=$0b
   ELSE IF NLS_StrCaseCmp(Asc,'CC' )=0 THEN Result:=$0e
   ELSE DecodeSpecReg:=False;
END;

	FUNCTION DecodeRegAdr(Asc:String; VAR Erg:Byte):Boolean;
BEGIN
   DecodeRegAdr:=False;
   IF NOT DecodeReg(Asc,Erg) THEN Exit;
   IF OpSize=-1 THEN OpSize:=Erg SHR 6;
   IF Erg SHR 6<>OpSize THEN
    BEGIN
     WrError(1132); Exit;
    END;
   Erg:=Erg AND $3f;
   DecodeRegAdr:=True;
END;

	PROCEDURE DecodeAdr(Asc:String; PrefInd:Byte; MayImm,MayReg:Boolean);
CONST
   FreeReg=$ff;
   SPReg=$fe;
   PCReg=$fd;
VAR
   Reg:Byte;
   AdrPart:String;
   BaseReg,IndReg,ScaleFact:Byte;
   DispPart,DispAcc:LongInt;
   OK,MinFlag,NMinFlag:Boolean;
   MPos,PPos,EPos:Integer;
BEGIN
   AdrCnt:=0; AdrOK:=False;

   { I. Speicheradresse }

   IF IsIndirect(Asc) THEN
    BEGIN
     { I.1. vorkonditionieren }

     Asc:=Copy(Asc,2,Length(Asc)-2); KillBlanks(Asc);

     { I.2. Predekrement }

     IF (Asc[1]='-') AND (Asc[2]='-')
     AND (DecodeReg(Copy(Asc,3,Length(Asc)-2),Reg)) THEN
      BEGIN
       CASE Reg SHR 6 OF
       0:WrError(1350);
       1:BEGIN
	  AdrMode:=$50+(Reg AND 15); AdrOK:=True;
	 END;
       2:BEGIN
	  AdrMode:=$71+(Reg AND 14); AdrOK:=True;
	 END;
       END;
       Exit;
      END;

     { I.3. Postinkrement }

     IF (Asc[Length(Asc)]='+') AND (Asc[Length(Asc)-1]='+')
     AND (DecodeReg(Copy(Asc,1,Length(Asc)-2),Reg)) THEN
      BEGIN
       CASE Reg SHR 6 OF
       0:WrError(1350);
       1:BEGIN
	  AdrMode:=$40+(Reg AND 15); AdrOK:=True;
	 END;
       2:BEGIN
	  AdrMode:=$70+(Reg AND 14); AdrOK:=True;
	 END;
       END;
       Exit;
      END;

     { I.4. Adre·komponenten zerlegen }

     BaseReg:=FreeReg; IndReg:=FreeReg; ScaleFact:=0;
     DispAcc:=AdrInc; MinFlag:=False;
     WHILE Asc<>'' DO
      BEGIN

       { I.4.a. Trennzeichen suchen }

       MPos:=QuotPos(Asc,'-'); PPos:=QuotPos(Asc,'+');
       IF MPos>=PPos THEN
	BEGIN
	 EPos:=PPos; NMinFlag:=False;
	END
       ELSE
	BEGIN
	 EPos:=MPos; NMinFlag:=True;
	END;
       SplitString(Asc,AdrPart,Asc,EPos);

       { I.4.b. Indexregister mit Skalierung }

       EPos:=QuotPos(AdrPart,'*');
       IF (EPos=Length(AdrPart)-1) AND (AdrPart[Length(AdrPart)] IN ['1','2','4','8'])
       AND (DecodeReg(Copy(AdrPart,1,Length(AdrPart)-2),Reg)) THEN
	BEGIN
	 IF (Reg SHR 6=0) OR (MinFlag) OR (IndReg<>FreeReg) THEN
	  BEGIN
	   WrError(1350); Exit;
	  END;
	 IndReg:=Reg;
	 CASE AdrPart[Length(AdrPart)] OF
	 '1':ScaleFact:=0;
	 '2':ScaleFact:=1;
	 '4':ScaleFact:=2;
	 '8':ScaleFact:=3;
	 END;
	END

       { I.4.c. Basisregister }

       ELSE IF DecodeReg(AdrPart,Reg) THEN
	BEGIN
	 IF (Reg SHR 6=0) OR (MinFlag) THEN
	  BEGIN
	   WrError(1350); Exit;
	  END;
	 IF BaseReg=FreeReg THEN BaseReg:=Reg
	 ELSE IF IndReg=FreeReg THEN
	  BEGIN
	   IndReg:=Reg; ScaleFact:=0;
	  END
	 ELSE
	  BEGIN
	   WrError(1350); Exit;
	  END;
	END

       { I.4.d. Sonderregister }

       ELSE IF (NLS_StrCaseCmp(AdrPart,'PC')=0) OR (NLS_StrCaseCmp(AdrPart,'SP')=0) THEN
	BEGIN
	 IF (BaseReg<>FreeReg) AND (IndReg=FreeReg) THEN
	  BEGIN
	   IndReg:=BaseReg; BaseReg:=FreeReg; ScaleFact:=0;
	  END;
	 IF (BaseReg<>FreeReg) OR (MinFlag) THEN
	  BEGIN
	   WrError(1350); Exit;
	  END;
         IF NLS_StrCaseCmp(AdrPart,'SP')=0 THEN BaseReg:=SPReg ELSE BaseReg:=PCReg;
	END

       { I.4.e. Displacement }

       ELSE
	BEGIN
	 FirstPassUnknown:=False;
	 DispPart:=EvalIntExpression(AdrPart,Int32,OK);
	 IF NOT OK THEN Exit;
	 IF FirstPassUnknown THEN DispPart:=1;
	 IF MinFlag THEN Dec(DispAcc,DispPart) ELSE Inc(DispAcc,DispPart);
	END;
       MinFlag:=NMinFlag;
      END;

     { I.5. Indexregister mit Skalierung 1 als Basis behandeln }

     IF (BaseReg=FreeReg) AND (IndReg<>FreeReg) AND (ScaleFact=0) THEN
      BEGIN
       BaseReg:=IndReg; IndReg:=FreeReg;
      END;

     { I.6. absolut }

     IF (BaseReg=FreeReg) AND (IndReg=FreeReg) THEN
      BEGIN
       AdrMode:=$20;
       AdrVals[0]:=$e000+(DispAcc AND $1fff); AdrCnt:=2;
       AddAbsPrefix(PrefInd,13,DispAcc);
       AdrOK:=True; Exit;
      END;

     { I.7. Basis [mit Displacement] }

     IF (BaseReg<>FreeReg) AND (IndReg=FreeReg) THEN
      BEGIN

       { I.7.a. Basis ohne Displacement }

       IF DispAcc=0 THEN
	BEGIN
	 IF BaseReg SHR 6=1 THEN AdrMode:=$10+(BaseReg AND 15)
	 ELSE AdrMode:=$61+(BaseReg AND 14);
	 AdrOK:=True; Exit;
	END

       { I.7.b. Nullregister mit Displacement mu· in Erweiterungswort }

       ELSE IF BaseReg AND 15=0 THEN
	BEGIN
	 IF DispAcc>$7ffff THEN WrError(1320)
	 ELSE IF DispAcc<-$80000 THEN WrError(1315)
	 ELSE
	  BEGIN
	   AdrMode:=$20;
	   IF BaseReg SHR 6=1 THEN AdrVals[0]:=Word(BaseReg AND 15) SHL 11
	   ELSE AdrVals[0]:=(Word(BaseReg AND 14) SHL 11)+$8000;
	   Inc(AdrVals[0],DispAcc AND $1ff); AdrCnt:=2;
	   AddSignedPrefix(PrefInd,9,DispAcc);
	   AdrOK:=True;
	  END;
	 Exit;
	END

       { I.7.c. Stack mit Displacement: Optimierung mîglich }

       ELSE IF BaseReg=SPReg THEN
	BEGIN
	 IF DispAcc>$7ffff THEN WrError(1320)
	 ELSE IF DispAcc<-$80000 THEN WrError(1315)
	 ELSE IF (DispAcc>=0) AND (DispAcc<=127) THEN
	  BEGIN
	   AdrMode:=$80+(DispAcc AND $7f); AdrOK:=True;
	  END
	 ELSE
	  BEGIN
	   AdrMode:=$20;
	   AdrVals[0]:=$d000+(DispAcc AND $1ff); AdrCnt:=2;
	   AddSignedPrefix(PrefInd,9,DispAcc);
	   AdrOK:=True;
	  END;
         Exit;
	END

       { I.7.d. ProgrammzÑhler mit Displacement: keine Optimierung }

       ELSE IF BaseReg=PCReg THEN
	BEGIN
	 IF DispAcc>$7ffff THEN WrError(1320)
	 ELSE IF DispAcc<-$80000 THEN WrError(1315)
	 ELSE
	  BEGIN
	   AdrMode:=$20;
	   AdrVals[0]:=$d800+(DispAcc AND $1ff); AdrCnt:=2;
	   AddSignedPrefix(PrefInd,9,DispAcc);
	   AdrOK:=True;
	  END;
         Exit;
	END

       { I.7.e. einfaches Basisregister mit Displacement }

       ELSE
	BEGIN
	 IF DispAcc>$7fffff THEN WrError(1320)
	 ELSE IF DispAcc<-$800000 THEN WrError(1315)
	 ELSE
	  BEGIN
	   IF BaseReg SHR 6=1 THEN AdrMode:=$20+(BaseReg AND 15)
	   ELSE AdrMode:=$60+(BaseReg AND 14);
	   AdrVals[0]:=$e000+(DispAcc AND $1fff); AdrCnt:=2;
	   AddSignedPrefix(PrefInd,13,DispAcc);
	   AdrOK:=True;
	  END;
	 Exit;
	END;
      END

     { I.8. Index- [und Basisregister] }

     ELSE
      BEGIN
       IF DispAcc>$7ffff THEN WrError(1320)
       ELSE IF DispAcc<-$80000 THEN WrError(1315)
       ELSE IF IndReg AND 15=0 THEN WrError(1350)
       ELSE
	BEGIN
	 IF IndReg SHR 6=1 THEN AdrMode:=$20+(IndReg AND 15)
	 ELSE AdrMode:=$60+(IndReg AND 14);
	 CASE BaseReg OF
	 FreeReg :AdrVals[0]:=$c000;
	 SPReg   :AdrVals[0]:=$d000;
	 PCReg   :AdrVals[0]:=$d800;
	 $40..$4f:AdrVals[0]:=Word(BaseReg AND 15) SHL 11;
	 $80..$8e:AdrVals[0]:=$8000+Word(BaseReg AND 14) SHL 10;
	 END;
	 Inc(AdrVals[0],(Word(ScaleFact) SHl 9)+(DispAcc AND $1ff));
	 AdrCnt:=2;
	 AddSignedPrefix(PrefInd,9,DispAcc);
	 AdrOK:=True;
	END;
       Exit;
      END;
    END

   { II. Arbeitsregister }

   ELSE IF DecodeReg(Asc,Reg) THEN
    BEGIN
     IF NOT MayReg THEN WrError(1350)
     ELSE
      BEGIN
       IF OpSize=-1 THEN OpSize:=Reg SHR 6;
       IF Reg SHR 6<>OpSize THEN WrError(1131)
       ELSE
	BEGIN
	 AdrMode:=Reg AND 15; AdrOK:=True;
	END;
      END;
     Exit;
    END

   { III. Spezialregister }

   ELSE IF DecodeSpecReg(Asc,Reg) THEN
    BEGIN
     IF NOT MayReg THEN WrError(1350)
     ELSE
      BEGIN
       IF OpSize=-1 THEN OpSize:=Reg SHR 6;
       IF Reg SHR 6<>OpSize THEN WrError(1131)
       ELSE
	BEGIN
	 AdrMode:=$30+(Reg AND 15); AdrOK:=True;
	END;
      END;
     Exit;
    END

   ELSE IF NOT MayImm THEN WrError(1350)
   ELSE
    BEGIN
     IF (OpSize=-1) AND (MinOneIs0) THEN OpSize:=0;
     IF OpSize=-1 THEN WrError(1132)
     ELSE
      BEGIN
       AdrMode:=$30;
       CASE OpSize OF
       0:BEGIN
	  AdrVals[0]:=EvalIntExpression(Asc,Int8,OK) AND $ff;
	  IF OK THEN
	   BEGIN
	    AdrCnt:=2; AdrOK:=True;
	   END;
	 END;
       1:BEGIN
	  AdrVals[0]:=EvalIntExpression(Asc,Int16,OK);
	  IF OK THEN
	   BEGIN
	    AdrCnt:=2; AdrOK:=True;
	   END;
	 END;
       2:BEGIN
	  DispAcc:=EvalIntExpression(Asc,Int32,OK);
	  IF OK THEN
	   BEGIN
	    AdrVals[0]:=DispAcc AND $ffff;
	    AdrVals[1]:=DispAcc SHR 16;
	    AdrCnt:=4; AdrOK:=True;
	   END;
	 END;
       END;
      END;
    END;
END;

	PROCEDURE CopyAdr;
VAR
   z:Byte;
BEGIN
   OpSize2:=OpSize;
   AdrMode2:=AdrMode;
   AdrCnt2:=AdrCnt;
   Move(AdrVals,AdrVals2,AdrCnt);
END;

	FUNCTION IsReg:Boolean;
BEGIN
   IsReg:=AdrMode<=15;
END;

	FUNCTION Is2Reg:Boolean;
BEGIN
   Is2Reg:=AdrMode2<=15;
END;

	FUNCTION IsImmediate:Boolean;
BEGIN
   IsImmediate:=AdrMode=$30;
END;

	FUNCTION Is2Immediate:Boolean;
BEGIN
   Is2Immediate:=AdrMode2=$30;
END;

	FUNCTION ImmVal:LongInt;
VAR
   Tmp1:LongInt;
   Tmp2:Integer;
   Tmp3:ShortInt;
BEGIN
   CASE OpSize OF
   0:BEGIN
      Tmp3:=AdrVals[0] AND $ff; ImmVal:=Tmp3;
     END;
   1:BEGIN
      Tmp2:=AdrVals[0]; ImmVal:=Tmp2;
     END;
   2:BEGIN
      Tmp1:=(LongInt(AdrVals[1]) SHL 16)+AdrVals[0]; ImmVal:=Tmp1;
     END;
   END;
END;

	FUNCTION ImmVal2:LongInt;
VAR
   Tmp1:LongInt;
   Tmp2:Integer;
   Tmp3:ShortInt;
BEGIN
   CASE OpSize OF
   0:BEGIN
      Tmp3:=AdrVals2[0] AND $ff; ImmVal2:=Tmp3;
     END;
   1:BEGIN
      Tmp2:=AdrVals2[0]; ImmVal2:=Tmp2;
     END;
   2:BEGIN
      Tmp1:=(LongInt(AdrVals2[1]) SHL 16)+AdrVals2[0]; ImmVal2:=Tmp1;
     END;
   END;
END;

	FUNCTION IsAbsolute:Boolean;
BEGIN
   IsAbsolute:=((AdrMode=$20) OR (AdrMode=$60)) AND (AdrCnt=2)
	   AND (AdrVals[0] AND $e000=$e000);
END;

	FUNCTION Is2Absolute:Boolean;
BEGIN
   Is2Absolute:=((AdrMode2=$20) OR (AdrMode2=$60)) AND (AdrCnt2=2)
	   AND (AdrVals2[0] AND $e000=$e000);
END;

	FUNCTION IsShort:Boolean;
BEGIN
   IF AdrMode<$30 THEN IsShort:=True
   ELSE IF AdrMode=$30 THEN IsShort:=(ImmVal>=LowLim4) AND (ImmVal<=7)
   ELSE IsShort:=False;
END;

	FUNCTION Is2Short:Boolean;
BEGIN
   IF AdrMode2<$30 THEN Is2Short:=True
   ELSE IF AdrMode2=$30 THEN Is2Short:=(ImmVal2>=LowLim4) AND (ImmVal2<=7)
   ELSE Is2Short:=False;
END;

	PROCEDURE ConvertShort;
BEGIN
   IF AdrMode=$30 THEN
    BEGIN
     Inc(AdrMode,ImmVal AND 15); AdrCnt:=0;
    END;
END;

	PROCEDURE Convert2Short;
BEGIN
   IF AdrMode2=$30 THEN
    BEGIN
     Inc(AdrMode2,ImmVal2 AND 15); AdrCnt2:=0;
    END;
END;

	PROCEDURE SetULowLims;
BEGIN
   LowLim4:=0; LowLim8:=0;
END;

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_97C241;
	Far;
LABEL
   AddPrefixes;
VAR
   z,Cnt:Integer;
   Reg,Num1,Num2:Byte;
   AdrInt,AdrLong:LongInt;
   OK:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; PrefUsed[0]:=False; PrefUsed[1]:=False;
   AdrInc:=0; MinOneIs0:=False; LowLim4:=-8; LowLim8:=-128;

   { zu ignorierendes }

   IF Memo('') THEN Exit;

   { Formatangabe abspalten }

   CASE AttrSplit OF
   '.':BEGIN
        Num1:=Pos(':',AttrPart);
        IF Num1<>0 THEN
         BEGIN
          IF Num1<Length(AttrPart) THEN Format:=AttrPart[Num1+1];
          Delete(AttrPart,Num1,Length(AttrPart)-Num1+1);
         END
        ELSE Format:=' ';
       END;
   ':':BEGIN
        Num1:=Pos('.',AttrPart);
        IF Num1=0 THEN
         BEGIN
          Format:=AttrPart[1]; AttrPart:='';
         END
        ELSE
         BEGIN
          IF Num1=1 THEN Format:=' ' ELSE Format:=AttrPart[1];
          Delete(AttrPart,1,Num1);
         END;
       END;
   ELSE Format:=' ';
   END;
   Format:=UpCase(Format);

   { Attribut abarbeiten }

   IF AttrPart='' THEN OpSize:=-1
   ELSE
    CASE UpCase(AttrPart[1]) OF
    'B':OpSize:=0;
    'W':OpSize:=1;
    'D':OpSize:=2;
    ELSE
     BEGIN
      WrError(1107); Exit;
     END;
    END;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(False) THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
	BEGIN
	 WAsmCode[0]:=Code;
	 CodeLen:=2;
	END;
       Exit;
      END;

   { ein Operand }

   FOR z:=1 TO RMWOrderCount DO
    WITH RMWOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (OpSize=-1) AND (Mask AND $20<>0) THEN OpSize:=2;
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 IF (NOT IsIndirect(ArgStr[1])) AND (Mask AND $20<>0) THEN
	  ArgStr[1]:='('+ArgStr[1]+')';
	 DecodeAdr(ArgStr[1],0,Mask AND $10=0,Mask AND $20=0);
	 IF AdrOK THEN
	  IF OpSize=-1 THEN WrError(1132)
	  ELSE IF Mask AND (1 SHL OpSize)=0 THEN WrError(1130)
	  ELSE
	   BEGIN
	    WAsmCode[0]:=(Word(OpSize+1) SHL 14)+(Word(Code) SHL 8)+AdrMode;
	    Move(AdrVals,WAsmCode[1],AdrCnt);
	    CodeLen:=2+AdrCnt;
	   END;
	END;
       Goto AddPrefixes;
      END;

   { Arithmetik }

   FOR z:=0 TO GASI1OrderCount-1 DO
    IF Memo(GASI1Orders[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],1,False,True);
	IF AdrOK THEN
	 BEGIN
	  CopyAdr; DecodeAdr(ArgStr[2],0,True,True);
	  IF AdrOK THEN
	   IF OpSize=-1 THEN WrError(1132)
	   ELSE
	    BEGIN
	     IF Format=' ' THEN
	      BEGIN
	       IF ((IsReg) AND (Is2Short))
	       OR ((Is2Reg) AND (IsShort)) THEN Format:='S'
	       ELSE IF ((IsAbsolute) AND (Is2Short))
		    OR ((Is2Absolute) AND (IsShort)) THEN Format:='A'
	       ELSE IF (IsImmediate) AND (OpSize>0) AND ((ImmVal>127) OR (ImmVal<-128)) THEN Format:='I'
	       ELSE Format:='G';
	      END;
	     CASE Format OF
	     'G':
	      BEGIN
	       WAsmCode[0]:=$700+(Word(OpSize+1) SHL 14);
	       IF (IsImmediate) AND (ImmVal<=127) AND (ImmVal>=-128) THEN
		BEGIN
		 AdrMode:=ImmVal AND $ff; AdrCnt:=0;
		END
	       ELSE Inc(WAsmCode[0],$800);
	       Inc(WAsmCode[0],AdrMode);
	       Move(AdrVals,WAsmCode[1],AdrCnt);
	       WAsmCode[1+(AdrCnt SHR 1)]:=$8400+(z SHL 8)+AdrMode2;
	       Move(AdrVals2,WAsmCode[2+(AdrCnt SHR 1)],AdrCnt2);
	       CodeLen:=4+AdrCnt+AdrCnt2;
	      END;
	     'A':
	      IF (IsShort) AND (Is2Absolute) THEN
	       BEGIN
		ConvertShort;
		WAsmCode[0]:=$3900+(Word(OpSize+1) SHL 14)
			    +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15);
		Move(AdrVals,WAsmCode[1],AdrCnt);
		WAsmCode[1+(AdrCnt SHR 1)]:=(AdrVals2[0] AND $1fff)+(z SHL 13);
		CodeLen:=4+AdrCnt;
	       END
	      ELSE IF (Is2Short) AND (IsAbsolute) THEN
	       BEGIN
		Convert2Short;
		WAsmCode[0]:=$3980+(Word(OpSize+1) SHL 14)
			    +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15);
		Move(AdrVals2,WAsmCode[1],AdrCnt2);
		WAsmCode[1+(AdrCnt2 SHR 1)]:=(AdrVals[0] AND $1fff)+(z SHL 13);
		CodeLen:=4+AdrCnt2;
	       END
	      ELSE WrError(1350);
	     'S':
	      IF (IsShort) AND (Is2Reg) THEN
	       BEGIN
		ConvertShort;
		WAsmCode[0]:=$0000+(Word(OpSize+1) SHL 14)
			    +(AdrMode AND 15)+(Word(AdrMode AND $f0) SHL 5)
			    +(Word(AdrMode2 AND 1) SHL 12)+((AdrMode2 AND 14) SHL 4)
			    +((z AND 1) SHL 4)+((z AND 2) SHL 10);
		Move(AdrVals,WAsmCode[1],AdrCnt); CodeLen:=2+AdrCnt;
	       END
	      ELSE IF (Is2Short) AND (IsReg) THEN
	       BEGIN
		Convert2Short;
		WAsmCode[0]:=$0100+(Word(OpSize+1) SHL 14)
			    +(AdrMode2 AND 15)+(Word(AdrMode2 AND $f0) SHL 5)
			    +(Word(AdrMode AND 1) SHL 12)+((AdrMode AND 14) SHL 4)
			    +((z AND 1) SHL 4)+((z AND 2) SHL 11);
		Move(AdrVals2,WAsmCode[1],AdrCnt2); CodeLen:=2+AdrCnt2;
	       END
	      ELSE WrError(1350);
	     'I':
	      IF (NOT IsImmediate) OR (OpSize=0) THEN WrError(1350)
	      ELSE
	       BEGIN
		WAsmCode[0]:=AdrMode2+(Word(OpSize-1) SHL 11)+(z SHL 8);
		Move(AdrVals2,WAsmCode[1],AdrCnt2);
		Move(AdrVals,WAsmCode[1+(AdrCnt2 SHR 1)],AdrCnt);
		CodeLen:=2+AdrCnt+AdrCnt2;
	       END;
	     ELSE WrError(1090);
	     END;
	    END;
	 END;
       END;
      Goto AddPrefixes;
     END;

   FOR z:=0 TO GASI2OrderCount-1 DO
    IF Memo(GASI2Orders[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],1,False,True);
	IF AdrOK THEN
	 BEGIN
	  CopyAdr; DecodeAdr(ArgStr[2],0,True,True);
	  IF AdrOK THEN
	   IF OpSize=-1 THEN WrError(1132)
	   ELSE
	    BEGIN
	     IF Format=' ' THEN
	      BEGIN
	       IF (IsReg) AND (Is2Reg) THEN Format:='S'
	       ELSE IF ((IsAbsolute) AND (Is2Short))
		    OR ((Is2Absolute) AND (IsShort)) THEN Format:='A'
	       ELSE IF (IsImmediate) AND (OpSize>0) AND ((ImmVal>127) OR (ImmVal<-128)) THEN Format:='I'
	       ELSE Format:='G';
	      END;
	     CASE Format OF
	     'G':
	      BEGIN
	       WAsmCode[0]:=$700+(Word(OpSize+1) SHL 14);
	       IF (IsImmediate) AND (ImmVal<=127) AND (ImmVal>=-128) THEN
		BEGIN
		 AdrMode:=ImmVal AND $ff; AdrCnt:=0;
		END
	       ELSE Inc(WAsmCode[0],$800);
	       Inc(WAsmCode[0],AdrMode);
	       Move(AdrVals,WAsmCode[1],AdrCnt);
	       WAsmCode[1+(AdrCnt SHR 1)]:=$c400+(z SHL 8)+AdrMode2;
	       Move(AdrVals2,WAsmCode[2+(AdrCnt SHR 1)],AdrCnt2);
	       CodeLen:=4+AdrCnt+AdrCnt2;
	      END;
	     'A':
	      IF (IsShort) AND (Is2Absolute) THEN
	       BEGIN
		ConvertShort;
		WAsmCode[0]:=$3940+(Word(OpSize+1) SHL 14)
			    +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15);
		Move(AdrVals,WAsmCode[1],AdrCnt);
		WAsmCode[1+(AdrCnt SHR 1)]:=(AdrVals2[0] AND $1fff)+(z SHL 13);
		CodeLen:=4+AdrCnt;
	       END
	      ELSE IF (Is2Short) AND (IsAbsolute) THEN
	       BEGIN
		Convert2Short;
		WAsmCode[0]:=$39c0+(Word(OpSize+1) SHL 14)
			    +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15);
		Move(AdrVals2,WAsmCode[1],AdrCnt2);
		WAsmCode[1+(AdrCnt2 SHR 1)]:=(AdrVals[0] AND $1fff)+(z SHL 13);
		CodeLen:=4+AdrCnt2;
	       END
	      ELSE WrError(1350);
	     'S':
	      IF (IsReg) AND (Is2Reg) THEN
	       BEGIN
		WAsmCode[0]:=$3800+(Word(OpSize+1) SHL 14)
			    +(AdrMode AND 15)+(AdrMode2 SHL 4)
			    +(z SHL 9);
		CodeLen:=2;
	       END
	      ELSE WrError(1350);
	     'I':
	      IF (NOT IsImmediate) OR (OpSize=0) THEN WrError(1350)
	      ELSE
	       BEGIN
		WAsmCode[0]:=$400+AdrMode2+(Word(OpSize-1) SHL 11)+(z SHL 8);
		Move(AdrVals2,WAsmCode[1],AdrCnt2);
		Move(AdrVals,WAsmCode[1+(AdrCnt2 SHR 1)],AdrCnt);
		CodeLen:=2+AdrCnt+AdrCnt2;
	       END;
	     ELSE WrError(1090);
	     END;
	    END;
	 END;
       END;
      Goto AddPrefixes;
     END;

   FOR z:=0 TO TrinomOrderCount-1 DO
    IF Memo(TrinomOrders[z]) THEN
     BEGIN
      IF Memo('MAC') THEN LowLim8:=0;
      IF ArgCnt<>3 THEN WrError(1110)
      ELSE IF NOT DecodeRegAdr(ArgStr[1],Reg) THEN WrError(1350)
      ELSE
       BEGIN
	IF z>=2 THEN Dec(OpSize);
	IF OpSize<0 THEN WrError(1130)
	ELSE
	 BEGIN
	  DecodeAdr(ArgStr[3],0,True,True);
	  IF AdrOK THEN
	   BEGIN
	    WAsmCode[0]:=$700;
	    IF (IsImmediate) AND (ImmVal<127) AND (ImmVal>LowLim8) THEN
	     BEGIN
	      AdrMode:=ImmVal AND $ff; AdrCnt:=0;
	     END
	    ELSE Inc(WAsmCode[0],$800);
	    Inc(WAsmCode[0],(Word(OpSize+1) SHL 14)+AdrMode);
	    Move(AdrVals,WAsmCode[1],AdrCnt); Cnt:=AdrCnt;
	    DecodeAdr(ArgStr[2],1,False,True);
	    IF AdrOK THEN
	     BEGIN
	      WAsmCode[1+(Cnt SHR 1)]:=AdrMode+(z SHL 8)+(Word(Reg) SHL 11);
	      Move(AdrVals,WAsmCode[2+(Cnt SHR 1)],AdrCnt);
	      CodeLen:=4+Cnt+AdrCnt;
	     END;
	   END;
	 END;
       END;
      Goto AddPrefixes;
     END;

   IF (Memo('RLM')) OR (Memo('RRM')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[2],Reg) THEN WrError(1350)
     ELSE IF Reg SHR 6<>1 THEN WrError(1130)
     ELSE
      BEGIN
       Reg:=Reg AND $3f;
       DecodeAdr(ArgStr[3],0,True,True);
       IF AdrOK THEN
	BEGIN
	 WAsmCode[0]:=$700;
	 IF (IsImmediate) AND (ImmVal<127) AND (ImmVal>-128) THEN
	  BEGIN
	   AdrMode:=ImmVal AND $ff; AdrCnt:=0;
	  END
	 ELSE Inc(WAsmCode[0],$800);
	 Inc(WAsmCode[0],AdrMode);
	 Move(AdrVals,WAsmCode[1],AdrCnt); Cnt:=AdrCnt;
	 DecodeAdr(ArgStr[1],1,False,True);
	 IF AdrOK THEN
	  IF OpSize=-1 THEN WrError(1132)
	  ELSE
	   BEGIN
	    Inc(WAsmCode[0],Word(OpSize+1) SHL 14);
	    WAsmCode[1+(Cnt SHR 1)]:=$400+(Word(Reg) SHL 11)+AdrMode;
	    IF Memo('RRM') THEN Inc(WAsmCode[1+(Cnt SHR 1)],$100);
	    Move(AdrVals,WAsmCode[2+(Cnt SHR 1)],AdrCnt);
	    CodeLen:=4+AdrCnt+Cnt;
	   END;
	END;
      END;
     Goto AddPrefixes;
    END;

   FOR z:=0 TO BitOrderCount-1 DO
    IF Memo(BitOrders[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],1,False,True);
	IF AdrOK THEN
	 IF OpSize=-1 THEN WrError(1132)
	 ELSE
	  BEGIN
	   CopyAdr; OpSize:=-1; MinOneIs0:=True;
	   DecodeAdr(ArgStr[2],0,True,True);
	   IF AdrOK THEN
	    BEGIN
	     OpSize:=OpSize2;
	     IF Format=' ' THEN
	      BEGIN
	       IF (Is2Reg) AND (IsImmediate)
	       AND (ImmVal>0) AND (ImmVal<1 SHL (OpSize+3)) THEN Format:='S'
	       ELSE IF ((IsShort) AND (Is2Absolute))
		    OR ((Is2Short) AND (IsAbsolute)) THEN Format:='A'
	       ELSE Format:='G';
	      END;
	     CASE Format OF
	     'G':
	      BEGIN
	       WAsmCode[0]:=$700+(Word(OpSize+1) SHL 14);
	       IF (IsImmediate) AND (ImmVal>=LowLim8) AND (ImmVal<127) THEN
		BEGIN
		 AdrMode:=ImmVal AND $ff; AdrCnt:=0;
		END
	       ELSE Inc(WAsmCode[0],$800);
	       Inc(WAsmCode[0],AdrMode);
	       Move(AdrVals,WAsmCode[1],AdrCnt);
	       WAsmCode[1+(AdrCnt SHR 1)]:=$d400+(z SHL 8)+AdrMode2;
	       Move(AdrVals2,WAsmCode[2+(AdrCnt SHR 1)],AdrCnt2);
	       CodeLen:=4+AdrCnt+AdrCnt2;
	      END;
	     'A':
	      IF (IsAbsolute) AND (Is2Short) THEN
	       BEGIN
		Convert2Short;
		WAsmCode[0]:=$39d0+(Word(OpSize+1) SHL 14)
			    +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15);
		Move(AdrVals2,WAsmCode[1],AdrCnt2);
		WAsmCode[1+(AdrCnt2 SHR 1)]:=(AdrVals[0] AND $1fff)+(z SHL 13);
		CodeLen:=4+AdrCnt2;
	       END
	      ELSE IF (Is2Absolute) AND (IsShort) THEN
	       BEGIN
		ConvertShort;
		WAsmCode[0]:=$3950+(Word(OpSize+1) SHL 14)
			    +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15);
		Move(AdrVals,WAsmCode[1],AdrCnt);
		WAsmCode[1+(AdrCnt SHR 1)]:=(AdrVals2[0] AND $1fff)+(z SHL 13);
		CodeLen:=4+AdrCnt;
	       END
	      ELSE WrError(1350);
	     'S':
	      IF (Is2Reg) AND (IsImmediate) AND (ImmVal>=0) AND (ImmVal<1 SHL (3+OpSize)) THEN
	       BEGIN
		IF OpSize=2 THEN
		 BEGIN
		  IF ImmVal>=16 THEN
		   BEGIN
		    Dec(AdrVals[0],16); Inc(AdrMode2);
		   END;
		  OpSize:=1;
		 END;
		IF OpSize=1 THEN
		 IF ImmVal<8 THEN OpSize:=0
		 ELSE Dec(AdrVals[0],8);
		WAsmCode[0]:=$1700+(Word(OpSize+1) SHL 14)
			    +((z AND 1) SHL 7)+((z AND 2) SHL 10)
			    +(ImmVal SHL 4)+AdrMode2;
		CodeLen:=2;
	       END
	      ELSE WrError(1350);
	     ELSE WrError(1090);
	     END;
	    END;
	  END;
       END;
      Goto AddPrefixes;
     END;

   FOR z:=0 TO ShiftOrderCount-1 DO
    IF Memo(ShiftOrders[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[1],1,False,True);
	IF AdrOK THEN
	 IF OpSize=-1 THEN WrError(1132)
	 ELSE
	  BEGIN
	   CopyAdr; OpSize:=-1; MinOneIs0:=True;
	   DecodeAdr(ArgStr[2],0,True,True);
	   IF AdrOK THEN
	    BEGIN
	     OpSize:=OpSize2;
	     IF Format=' ' THEN
	      BEGIN
	       IF (IsImmediate) AND (ImmVal=1) THEN Format:='S'
	       ELSE IF ((IsShort) AND (Is2Absolute))
		    OR ((Is2Short) AND (IsAbsolute)) THEN Format:='A'
	       ELSE Format:='G';
	      END;
	     CASE Format OF
	     'G':
	      BEGIN
	       WAsmCode[0]:=$700+(Word(OpSize+1) SHL 14);
	       IF (IsImmediate) AND (ImmVal>=LowLim8) AND (ImmVal<127) THEN
		BEGIN
		 AdrMode:=ImmVal AND $ff; AdrCnt:=0;
		END
	       ELSE Inc(WAsmCode[0],$800);
	       Inc(WAsmCode[0],AdrMode);
	       Move(AdrVals,WAsmCode[1],AdrCnt);
	       WAsmCode[1+(AdrCnt SHR 1)]:=$b400+((z AND 3) SHL 8)+((z AND 4) SHL 9)+AdrMode2;
	       Move(AdrVals2,WAsmCode[2+(AdrCnt SHR 1)],AdrCnt2);
	       CodeLen:=4+AdrCnt+AdrCnt2;
	      END;
	     'A':
	      IF (IsAbsolute) AND (Is2Short) THEN
	       BEGIN
		Convert2Short;
		WAsmCode[0]:=$39b0+(Word(OpSize+1) SHL 14)
			    +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15);
		Move(AdrVals2,WAsmCode[1],AdrCnt2);
		WAsmCode[1+(AdrCnt2 SHR 1)]:=(AdrVals[0] AND $1fff)+(z SHL 13);
		CodeLen:=4+AdrCnt2;
	       END
	      ELSE IF (Is2Absolute) AND (IsShort) THEN
	       BEGIN
		ConvertShort;
		WAsmCode[0]:=$3930+(Word(OpSize+1) SHL 14)
			    +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15);
		Move(AdrVals,WAsmCode[1],AdrCnt);
		WAsmCode[1+(AdrCnt SHR 1)]:=(AdrVals2[0] AND $1fff)+(z SHL 13);
		CodeLen:=4+AdrCnt;
	       END
	      ELSE WrError(1350);
	     'S':
	      IF (IsImmediate) AND (ImmVal=1) THEN
	       BEGIN
		WAsmCode[0]:=$2400+(Word(OpSize+1) SHL 14)+AdrMode2
			    +((z AND 3) SHL 8)+((z AND 4) SHL 9);
		Move(AdrVals2,WAsmCode[1],AdrCnt2);
		CodeLen:=2+AdrCnt2;
	       END
	      ELSE WrError(1350);
	     ELSE WrError(1090);
	     END;
	    END;
	  END;
       END;
      Goto AddPrefixes;
     END;

   FOR z:=0 TO BFieldOrderCount-1 DO
    IF Memo(BFieldOrders[z]) THEN
     BEGIN
      IF ArgCnt<>4 THEN WrError(1110)
      ELSE
       BEGIN
	IF z=2 THEN
	 BEGIN
	  ArgStr[5]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[5];
	 END;
	IF NOT DecodeReg(ArgStr[1],Reg) THEN WrError(1350)
	ELSE IF Reg SHR 6<>1 THEN WrError(1130)
	ELSE
	 BEGIN
	  Reg:=Reg AND $3f;
	  Num2:=EvalIntExpression(ArgStr[4],Int5,OK);
	  IF OK THEN
	   BEGIN
	    IF FirstPassUnknown THEN Num2:=Num2 AND 15; Dec(Num2);
	    IF Num2>15 THEN WrError(1320)
	    ELSE IF (OpSize=-1) AND (NOT DecodeRegAdr(ArgStr[2],Num1)) THEN WrError(1132)
	    ELSE
	     BEGIN
	      CASE OpSize OF
	      0:Num1:=EvalIntExpression(ArgStr[3],UInt3,OK) AND 7;
	      1:Num1:=EvalIntExpression(ArgStr[3],Int4,OK) AND 15;
	      2:Num1:=EvalIntExpression(ArgStr[3],Int5,OK) AND 31;
	      END;
	      IF OK THEN
	       BEGIN
		IF (OpSize=2) AND (Num1>15) THEN AdrInc:=2;
		DecodeAdr(ArgStr[2],1,False,True);
		IF AdrOK THEN
		 BEGIN
		  IF (OpSize=2) AND (Num1>15) THEN
		   BEGIN
		    Dec(Num1,16); Dec(OpSize);
		    IF AdrMode AND $f0=0 THEN Inc(AdrMode);
		   END;
		  WAsmCode[0]:=$7000+(Word(OpSize+1) SHL 8)+AdrMode;
		  Move(AdrVals,WAsmCode[1],AdrCnt);
		  WAsmCode[1+(AdrCnt SHR 1)]:=(Word(Reg) SHL 11)
					     +Num2+(Word(Num1) SHL 5)
					     +((z AND 1) SHL 10)
					     +((z AND 2) SHL 14);
		  CodeLen:=4+AdrCnt;
		 END;
	       END;
	     END;
	   END;
	 END;
       END;
      Goto AddPrefixes;
     END;

   FOR z:=1 TO GAEqOrderCount DO
    WITH GAEqOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (Memo('ABCD')) OR (Memo('SBCD')) OR (Memo('CBCD'))
       OR (Memo('MAX')) OR (Memo('MIN')) OR (Memo('SBC')) THEN SetULowLims;
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],1,False,True);
	 IF AdrOK THEN
	  BEGIN
	   CopyAdr;
	   DecodeAdr(ArgStr[2],0,True,True);
	   IF AdrOK THEN
	    IF OpSize=-1 THEN WrError(1132)
	    ELSE
	     BEGIN
	      IF OpSize=0 THEN LowLim8:=-128;
	      IF Format=' ' THEN
	       BEGIN
		IF ((Is2Absolute) AND (IsShort))
		OR ((Is2Short) AND (IsAbsolute)) THEN Format:='A'
						 ELSE Format:='G';
	       END;
	      CASE Format OF
	      'G':
	       BEGIN
		WAsmCode[0]:=$700+(Word(OpSize+1) SHL 14);
		IF (IsImmediate) AND (ImmVal<127) AND (ImmVal>LowLim8) THEN
		 BEGIN
		  AdrMode:=ImmVal AND $ff; AdrCnt:=0;
		 END
		ELSE Inc(WAsmCode[0],$800);
		Inc(WAsmCode[0],AdrMode);
		Move(AdrVals,WAsmCode[1],AdrCnt);
		WAsmCode[1+(AdrCnt SHR 1)]:=$8400+AdrMode2
					   +(Word(Code AND $f0) SHL 8)
					   +(Word(Code AND 4) SHL 9)
					   +(Word(Code AND 3) SHL 8);
		Move(AdrVals2,WAsmCode[2+(AdrCnt SHR 1)],AdrCnt2);
		CodeLen:=4+AdrCnt+AdrCnt2;
	       END;
	      'A':
	       IF (IsAbsolute) AND (Is2Short) THEN
		BEGIN
		 Convert2Short;
		 WAsmCode[0]:=$3980+(Word(OpSize+1) SHL 14)
			     +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15)
			     +(Code AND $f0);
		 Move(AdrVals2,WAsmCode[1],AdrCnt2);
		 WAsmCode[1+(AdrCnt2 SHR 1)]:=(AdrVals[0] AND $1fff)
					    +(Word(Code AND 15) SHL 13);
		 CodeLen:=4+AdrCnt2;
		END
	       ELSE IF (Is2Absolute) AND (IsShort) THEN
		BEGIN
		 ConvertShort;
		 WAsmCode[0]:=$3900+(Word(OpSize+1) SHL 14)
			     +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15)
			     +(Code AND $f0);
		 Move(AdrVals,WAsmCode[1],AdrCnt);
		 WAsmCode[1+(AdrCnt SHR 1)]:=(AdrVals2[0] AND $1fff)
					    +(Word(Code AND 15) SHL 13);
		 CodeLen:=4+AdrCnt;
		END
	       ELSE WrError(1350);
	      ELSE WrError(1090);
	      END;
	     END;
	  END;
	END;
       Goto AddPrefixes;
      END;

   FOR z:=1 TO GAHalfOrderCount DO
    WITH GAHalfOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],1,False,True);
	 IF AdrOK THEN
	  IF OpSize=0 THEN WrError(1130)
	  ELSE
	   BEGIN
	    IF OpSize<>-1 THEN Dec(OpSize); CopyAdr;
	    DecodeAdr(ArgStr[2],0,True,True);
	    IF AdrOK THEN
	     IF OpSize=2 THEN WrError(1130)
	     ELSE IF OpSize=-1 THEN WrError(1132)
	     ELSE
	      BEGIN
	       IF Format=' ' THEN
		BEGIN
		 IF ((Is2Absolute) AND (IsShort))
		 OR ((Is2Short) AND (IsAbsolute)) THEN Format:='A'
						  ELSE Format:='G';
		END;
	       CASE Format OF
	       'G':
		BEGIN
		 WAsmCode[0]:=$700+(Word(OpSize+1) SHL 14);
		 IF (IsImmediate) AND (ImmVal<127) AND (ImmVal>LowLim8) THEN
		  BEGIN
		   AdrMode:=ImmVal AND $ff; AdrCnt:=0;
		  END
		 ELSE Inc(WAsmCode[0],$800);
		 Inc(WAsmCode[0],AdrMode);
		 Move(AdrVals,WAsmCode[1],AdrCnt);
		 WAsmCode[1+(AdrCnt SHR 1)]:=$8400+AdrMode2
					    +(Word(Code AND $f0) SHL 8)
					    +(Word(Code AND 4) SHL 9)
					    +(Word(Code AND 3) SHL 8);
		 Move(AdrVals2,WAsmCode[2+(AdrCnt SHR 1)],AdrCnt2);
		 CodeLen:=4+AdrCnt+AdrCnt2;
		END;
	       'A':
		IF (IsAbsolute) AND (Is2Short) THEN
		 BEGIN
		  Convert2Short;
		  WAsmCode[0]:=$3980+(Word(OpSize+1) SHL 14)
			      +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15)
			      +(Code AND $f0);
		  Move(AdrVals2,WAsmCode[1],AdrCnt2);
		  WAsmCode[1+(AdrCnt2 SHR 1)]:=(AdrVals[0] AND $1fff)
					     +(Word(Code AND 15) SHL 13);
		  CodeLen:=4+AdrCnt2;
		 END
		ELSE IF (Is2Absolute) AND (IsShort) THEN
		 BEGIN
		  ConvertShort;
		  WAsmCode[0]:=$3900+(Word(OpSize+1) SHL 14)
			      +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15)
			      +(Code AND $f0);
		  Move(AdrVals,WAsmCode[1],AdrCnt);
		  WAsmCode[1+(AdrCnt SHR 1)]:=(AdrVals2[0] AND $1fff)
					     +(Word(Code AND 15) SHL 13);
		  CodeLen:=4+AdrCnt;
		 END
		ELSE WrError(1350);
	       ELSE WrError(1090);
	       END;
	      END;
	   END;
	END;
       Goto AddPrefixes;
      END;

   FOR z:=1 TO GAFirstOrderCount DO
    WITH GAFirstOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],1,NOT(Memo('STCF') OR Memo('TSET')),True);
	 IF AdrOK THEN
	  IF OpSize=-1 THEN WrError(1132)
	  ELSE
	   BEGIN
	    CopyAdr; OpSize:=-1; MinOneIs0:=True;
	    DecodeAdr(ArgStr[2],0,True,True);
	    OpSize:=OpSize2;
	    IF AdrOK THEN
	     BEGIN
	      IF Format=' ' THEN
	       BEGIN
		IF ((Is2Absolute) AND (IsShort))
		OR ((Is2Short) AND (IsAbsolute)) THEN Format:='A'
						 ELSE Format:='G';
	       END;
	      CASE Format OF
	      'G':
	       BEGIN
		WAsmCode[0]:=$700+(Word(OpSize+1) SHL 14);
		IF (IsImmediate) AND (ImmVal<127) AND (ImmVal>LowLim8) THEN
		 BEGIN
		  AdrMode:=ImmVal AND $ff; AdrCnt:=0;
		 END
		ELSE Inc(WAsmCode[0],$800);
		Inc(WAsmCode[0],AdrMode);
		Move(AdrVals,WAsmCode[1],AdrCnt);
		WAsmCode[1+(AdrCnt SHR 1)]:=$8400+AdrMode2
					   +(Word(Code AND $f0) SHL 8)
					   +(Word(Code AND 4) SHL 9)
					   +(Word(Code AND 3) SHL 8);
		Move(AdrVals2,WAsmCode[2+(AdrCnt SHR 1)],AdrCnt2);
		CodeLen:=4+AdrCnt+AdrCnt2;
	       END;
	      'A':
	       IF (IsAbsolute) AND (Is2Short) THEN
		BEGIN
		 Convert2Short;
		 WAsmCode[0]:=$3980+(Word(OpSize+1) SHL 14)
			     +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15)
			     +(Code AND $f0);
		 Move(AdrVals2,WAsmCode[1],AdrCnt2);
		 WAsmCode[1+(AdrCnt2 SHR 1)]:=(AdrVals[0] AND $1fff)
					    +(Word(Code AND 15) SHL 13);
		 CodeLen:=4+AdrCnt2;
		END
	       ELSE IF (Is2Absolute) AND (IsShort) THEN
		BEGIN
		 ConvertShort;
		 WAsmCode[0]:=$3900+(Word(OpSize+1) SHL 14)
			     +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15)
			     +(Code AND $f0);
		 Move(AdrVals,WAsmCode[1],AdrCnt);
		 WAsmCode[1+(AdrCnt SHR 1)]:=(AdrVals2[0] AND $1fff)
					    +(Word(Code AND 15) SHL 13);
		 CodeLen:=4+AdrCnt;
		END
	       ELSE WrError(1350);
	      ELSE WrError(1090);
	      END;
	     END;
	   END;
	END;
       Goto AddPrefixes;
      END;

   FOR z:=1 TO GASecondOrderCount DO
    WITH GASecondOrders[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2],0,True,True);
	 IF AdrOK THEN
	  IF OpSize=-1 THEN WrError(1132)
	  ELSE
	   BEGIN
	    CopyAdr; OpSize:=-1;
	    DecodeAdr(ArgStr[1],1,False,True);
	    OpSize:=OpSize2;
	    IF AdrOK THEN
	     BEGIN
	      IF Format=' ' THEN
	       BEGIN
		IF ((Is2Absolute) AND (IsShort))
		OR ((Is2Short) AND (IsAbsolute)) THEN Format:='A'
						 ELSE Format:='G';
	       END;
	      CASE Format OF
	      'G':
	       BEGIN
		WAsmCode[0]:=$700+(Word(OpSize+1) SHL 14);
		IF (Is2Immediate) AND (ImmVal2<127) AND (ImmVal2>LowLim8) THEN
		 BEGIN
		  AdrMode2:=ImmVal2 AND $ff; AdrCnt:=0;
		 END
		ELSE Inc(WAsmCode[0],$800);
		Inc(WAsmCode[0],AdrMode2);
		Move(AdrVals2,WAsmCode[1],AdrCnt2);
		WAsmCode[1+(AdrCnt2 SHR 1)]:=$8400+AdrMode
					   +(Word(Code AND $f0) SHL 8)
					   +(Word(Code AND 4) SHL 9)
					   +(Word(Code AND 3) SHL 8);
		Move(AdrVals,WAsmCode[2+(AdrCnt2 SHR 1)],AdrCnt);
		CodeLen:=4+AdrCnt+AdrCnt2;
	       END;
	      'A':
	       IF (IsAbsolute) AND (Is2Short) THEN
		BEGIN
		 Convert2Short;
		 WAsmCode[0]:=$3900+(Word(OpSize+1) SHL 14)
			     +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15)
			     +(Code AND $f0);
		 Move(AdrVals2,WAsmCode[1],AdrCnt2);
		 WAsmCode[1+(AdrCnt2 SHR 1)]:=(AdrVals[0] AND $1fff)
					    +(Word(Code AND 15) SHL 13);
		 CodeLen:=4+AdrCnt2;
		END
	       ELSE IF (Is2Absolute) AND (IsShort) THEN
		BEGIN
		 ConvertShort;
		 WAsmCode[0]:=$3980+(Word(OpSize+1) SHL 14)
			     +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15)
			     +(Code AND $f0);
		 Move(AdrVals,WAsmCode[1],AdrCnt);
		 WAsmCode[1+(AdrCnt SHR 1)]:=(AdrVals2[0] AND $1fff)
					    +(Word(Code AND 15) SHL 13);
		 CodeLen:=4+AdrCnt;
		END
	       ELSE WrError(1350);
	      ELSE WrError(1090);
	      END;
	     END;
	   END;
	END;
       Goto AddPrefixes;
      END;

   IF (Memo('CHK')) OR (Memo('CHKS')) THEN
    BEGIN
     IF Memo('CHK') THEN SetULowLims;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],1,False,True);
       IF AdrOK THEN
	IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
	ELSE IF OpSize=-1 THEN WrError(1132)
	ELSE
	 BEGIN
	  CopyAdr;
	  DecodeAdr(ArgStr[1],0,False,False);
	  IF AdrOK THEN
	   BEGIN
	    IF OpSize=0 THEN LowLim8:=-128;
	    IF Format=' ' THEN
	     BEGIN
	      IF ((Is2Absolute) AND (IsShort))
	      OR ((Is2Short) AND (IsAbsolute)) THEN Format:='A'
					       ELSE Format:='G';
	     END;
	    CASE Format OF
	    'G':
	     BEGIN
	      WAsmCode[0]:=$f00+(Word(OpSize+1) SHL 14)+AdrMode2;
	      Move(AdrVals2,WAsmCode[1],AdrCnt2);
	      WAsmCode[1+(AdrCnt2 SHR 1)]:=$a600+AdrMode
					 +(Word(Memo('CHKS')) SHL 8);
	      Move(AdrVals,WAsmCode[2+(AdrCnt2 SHR 1)],AdrCnt);
	      CodeLen:=4+AdrCnt+AdrCnt2;
	     END;
	    'A':
	     IF (IsAbsolute) AND (Is2Short) THEN
	      BEGIN
	       Convert2Short;
	       WAsmCode[0]:=$3920+(Word(OpSize+1) SHL 14)
			   +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15);
	       Move(AdrVals2,WAsmCode[1],AdrCnt2);
	       WAsmCode[1+(AdrCnt2 SHR 1)]:=$4000+(AdrVals[0] AND $1fff)
					  +(Word(Memo('CHKS')) SHL 13);
	       CodeLen:=4+AdrCnt2;
	      END
	     ELSE IF (Is2Absolute) AND (IsShort) THEN
	      BEGIN
	       ConvertShort;
	       WAsmCode[0]:=$39a0+(Word(OpSize+1) SHL 14)
			   +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15);
	       Move(AdrVals,WAsmCode[1],AdrCnt);
	       WAsmCode[1+(AdrCnt SHR 1)]:=$4000+(AdrVals2[0] AND $1fff)
					  +(Word(Memo('CHKS')) SHL 13);
	       CodeLen:=4+AdrCnt;
	      END
	     ELSE WrError(1350);
	    ELSE WrError(1090);
	    END;
	   END;
	 END;
      END;
     Goto AddPrefixes;
    END;

   { Datentransfer }

   FOR z:=0 TO StringOrderCount-1 DO
    IF Memo(StringOrders[z]) THEN
     BEGIN
      IF ArgCnt<>3 THEN WrError(1110)
      ELSE IF NOT DecodeReg(ArgStr[3],Reg) THEN WrError(1350)
      ELSE IF Reg SHR 6<>1 THEN WrError(1130)
      ELSE
       BEGIN
	Reg:=Reg AND $3f;
	DecodeAdr(ArgStr[2],0,True,True);
	IF AdrOK THEN
	 BEGIN
	  WAsmCode[0]:=$700;
	  IF (IsImmediate) AND (ImmVal<127) AND (ImmVal>LowLim8) THEN
	   BEGIN
	    AdrMode:=ImmVal AND $ff; AdrCnt:=0;
	   END
	  ELSE Inc(WAsmCode[0],$800);
	  Inc(WAsmCode[0],AdrMode);
	  Move(AdrVals,WAsmCode[1],AdrCnt); Cnt:=AdrCnt;
	  DecodeAdr(ArgStr[1],1,True,True);
	  IF AdrOK THEN
	   IF OpSize=-1 THEN WrError(1132)
	   ELSE
	    BEGIN
	     Inc(WAsmCode[0],Word(OpSize+1) SHL 14);
	     WAsmCode[1+(Cnt SHR 1)]:=$8000+AdrMode+(z SHL 8)+(Word(Reg) SHL 11);
	     Move(AdrVals,WAsmCode[2+(Cnt SHR 1)],AdrCnt);
	     CodeLen:=4+AdrCnt+Cnt;
	    END;
	 END;
       END;
      Goto AddPrefixes;
     END;

   IF Memo('EX') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],1,False,True);
       IF AdrOK THEN
	BEGIN
	 CopyAdr; DecodeAdr(ArgStr[2],0,False,True);
	 IF AdrOK THEN
	  IF OpSize=-1 THEN WrError(1132)
	  ELSE
	   BEGIN
	    IF Format=' ' THEN
	     BEGIN
	      IF (IsReg) AND (Is2Reg) THEN Format:='S'
	      ELSE IF ((IsShort) AND (Is2Absolute))
		   OR ((Is2Short) AND (IsAbsolute)) THEN Format:='A'
	      ELSE Format:='G';
	     END;
	    CASE Format OF
	    'G':
	     BEGIN
	      WAsmCode[0]:=$0f00+(Word(OpSize+1) SHL 14)+AdrMode;
	      Move(AdrVals,WAsmCode[1],AdrCnt);
	      WAsmCode[1+(AdrCnt SHR 1)]:=$8f00+AdrMode2;
	      Move(AdrVals2,WAsmCode[2+(AdrCnt SHR 1)],AdrCnt2);
	      CodeLen:=4+AdrCnt+AdrCnt2;
	     END;
	    'A':
	     IF (IsAbsolute) AND (Is2Short) THEN
	      BEGIN
	       Convert2Short;
	       WAsmCode[0]:=$3980+(Word(OpSize+1) SHL 14)
			   +(Word(AdrMode2 AND $f0) SHL 5)+(AdrMode2 AND 15);
	       Move(AdrVals2,WAsmCode[1],AdrCnt2);
	       WAsmCode[1+(AdrCnt2 SHR 1)]:=AdrVals[0];
	       CodeLen:=4+AdrCnt2;
	      END
	     ELSE IF (Is2Absolute) AND (IsShort) THEN
	      BEGIN
	       ConvertShort;
	       WAsmCode[0]:=$3900+(Word(OpSize+1) SHL 14)
			   +(Word(AdrMode AND $f0) SHL 5)+(AdrMode AND 15);
	       Move(AdrVals,WAsmCode[1],AdrCnt);
	       WAsmCode[1+(AdrCnt SHR 1)]:=AdrVals2[0];
	       CodeLen:=4+AdrCnt;
	      END
	     ELSE WrError(1350);
	    'S':
	     IF (IsReg) AND (Is2Reg) THEN
	      BEGIN
	       WAsmCode[0]:=$3e00+(Word(OpSize+1) SHL 14)+(AdrMode2 SHL 4)+AdrMode;
	       CodeLen:=2;
	      END
	     ELSE WrError(1350);
	    ELSE WrError(1090);
	    END;
	   END;
	END;
      END;
     Goto AddPrefixes;
    END;

   { SprÅnge }

   IF (Memo('CALR')) OR (Memo('JR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],Int32,OK)-EProgCounter;
       IF OK THEN
	IF AddRelPrefix(0,13,AdrInt) THEN
	 IF Odd(AdrInt) THEN WrError(1375)
	 ELSE
	  BEGIN
	   WAsmCode[0]:=$2000+(AdrInt AND $1ffe)+Ord(Memo('CALR'));
	   CodeLen:=2;
	  END;
      END;
     Goto AddPrefixes;
    END;

   IF Memo('JRC') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       z:=0; NLS_UpString(ArgStr[1]);
       WHILE (z<ConditionCount) AND (ArgStr[1]<>Conditions[z]) DO Inc(z);
       IF z>=ConditionCount THEN WrError(1360)
       ELSE
	BEGIN
	 z:=z MOD 16;
	 AdrInt:=EvalIntExpression(ArgStr[2],Int32,OK)-EProgCounter;
	 IF OK THEN
	  IF AddRelPrefix(0,9,AdrInt) THEN
	   IF Odd(AdrInt) THEN WrError(1375)
	   ELSE
	    BEGIN
	     WAsmCode[0]:=$1000+((z AND 14) SHL 8)+(AdrInt AND $1fe)+(z AND 1);
	     CodeLen:=2;
	    END;
	END;
      END;
     Goto AddPrefixes;
    END;

   IF (Memo('JRBC')) OR (Memo('JRBS')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       z:=EvalIntExpression(ArgStr[1],UInt3,OK);
       IF OK THEN
	BEGIN
	 FirstPassUnknown:=False;
	 AdrLong:=EvalIntExpression(ArgStr[2],Int24,OK);
	 IF OK THEN
	  BEGIN
	   AddAbsPrefix(1,13,AdrLong);
	   AdrInt:=EvalIntExpression(ArgStr[3],Int32,OK)-EProgCounter;
	   IF OK THEN
	    IF AddRelPrefix(0,9,AdrInt) THEN
	     IF Odd(AdrInt) THEN WrError(1375)
	     ELSE
	      BEGIN
	       CodeLen:=4;
	       WAsmCode[1]:=(z SHL 13)+(AdrLong AND $1fff);
	       WAsmCode[0]:=$1e00+(AdrInt AND $1fe)+Ord(Memo('JRBS'));
	      END;
	  END;
	END;
      END;
     Goto AddPrefixes;
    END;

   IF Memo('DJNZ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],0,False,True);
       IF AdrOK THEN
	IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
	ELSE
	 BEGIN
	  AdrInt:=EvalIntExpression(ArgStr[2],Int32,OK)-(EProgCounter+4+AdrCnt+2*Ord(PrefUsed[0]));
	  IF OK THEN
	   IF AddRelPrefix(1,13,AdrInt) THEN
	    IF Odd(AdrInt) THEN WrError(1375)
	    ELSE
	     BEGIN
	      WAsmCode[0]:=$3700+(Word(OpSize+1) SHL 14)+AdrMode;
	      Move(AdrVals,WAsmCode[1],AdrCnt);
	      WAsmCode[1+(AdrCnt SHR 1)]:=$e000+(AdrInt AND $1ffe);
	      CodeLen:=4+AdrCnt;
	     END;
	 END;
      END;
     Goto AddPrefixes;
    END;

   IF Memo('DJNZC') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       z:=0; NLS_UpString(ArgStr[2]);
       WHILE (z<ConditionCount) AND (ArgStr[2]<>Conditions[z]) DO Inc(z);
       IF z>=ConditionCount THEN WrError(1360)
       ELSE
	BEGIN
	 z:=z MOD 16;
	 DecodeAdr(ArgStr[1],0,False,True);
	 IF AdrOK THEN
	  IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
	  ELSE
	   BEGIN
	    AdrInt:=EvalIntExpression(ArgStr[3],Int32,OK)-EProgCounter;
	    IF OK THEN
	     IF AddRelPrefix(1,13,AdrInt) THEN
	      IF Odd(AdrInt) THEN WrError(1375)
	      ELSE
	       BEGIN
		WAsmCode[0]:=$3700+(Word(OpSize+1) SHL 14)+AdrMode;
		Move(AdrVals,WAsmCode[1],AdrCnt);
		WAsmCode[1+(AdrCnt SHR 1)]:=((z AND 14) SHL 12)+(AdrInt AND $1ffe)+(z AND 1);
		CodeLen:=4+AdrCnt;
	       END;
	   END;
	END;
      END;
     Goto AddPrefixes;
    END;

   { vermischtes... }

   IF (Memo('LINK')) OR (Memo('RETD')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       AdrInt:=EvalIntExpression(ArgStr[1],Int32,OK);
       IF FirstPassUnknown THEN AdrInt :=AdrInt AND $1fe;
       IF AdrInt>$7ffff THEN WrError(1320)
       ELSE IF AdrInt<-$80000 THEN WrError(1315)
       ELSE IF Odd(AdrInt) THEN WrError(1325)
       ELSE
	BEGIN
	 WAsmCode[0]:=$c001+(AdrInt AND $1fe);
	 IF Memo('RETD') THEN Inc(WAsmCode[0],$800);
	 AddSignedPrefix(0,9,AdrInt);
	 CodeLen:=2;
	END;
      END;
     Goto AddPrefixes;
    END;

   IF Memo('SWI') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       WAsmCode[0]:=EvalIntExpression(ArgStr[1],Int4,OK)+$7f90;
       IF OK THEN CodeLen:=2;
      END;
     Exit;
    END;

   IF Memo('LDA') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],0,False,False);
       IF AdrOK THEN
	BEGIN
	 WAsmCode[0]:=$3000+AdrMode;
	 z:=AdrCnt; Move(AdrVals,WAsmCode[1],z);
	 DecodeAdr(ArgStr[1],1,False,True);
	 IF AdrOK THEN
	  IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
	  ELSE
	   BEGIN
	    Inc(WAsmCode[0],Word(OpSize) SHL 14);
	    WAsmCode[1+(z SHR 1)]:=$9700+AdrMode;
	    Move(AdrVals,WAsmCode[2+(z SHR 1)],AdrCnt);
	    CodeLen:=4+z+AdrCnt;
	   END;
	END;
      END;
     Goto AddPrefixes;
    END;

   WrXError(1200,OpPart);

AddPrefixes:
   IF CodeLen<>0 THEN
    BEGIN
     InsertSinglePrefix(1);
     InsertSinglePrefix(0);
    END;
END;

	FUNCTION ChkPC_97C241:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<=$ffffff;
   ELSE ok:=False;
   END;
   ChkPC_97C241:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_97C241:Boolean;
	Far;
BEGIN
   IsDef_97C241:=False;
END;

        PROCEDURE SwitchFrom_97C241;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_97C241;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$56; NOPCode:=$7fa0;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.:';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_97C241; ChkPC:=ChkPC_97C241; IsDef:=IsDef_97C241;
   SwitchFrom:=SwitchFrom_97C241;
END;

BEGIN
   CPU97C241:=AddCPU('97C241',SwitchTo_97C241);
END.
