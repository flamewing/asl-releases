{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code3203;

{ AS - Codegenerator TMS320C3x }

INTERFACE
        Uses Chunks,NLS,
	     AsmDef,AsmSub,AsmPars,AsmCode,CodePseu;


IMPLEMENTATION

CONST
   ConditionCount=28;
   FixedOrderCount=3;
   RotOrderCount=4;
   StkOrderCount=4;
   GenOrderCount=41;
   ParOrderCount=8;
   SingOrderCount=3;

TYPE
   Condition=RECORD
	      Name:String[4];
	      Code:Byte;
	     END;
   FixedOrder=RECORD
	       Name:String[4];
	       Code:LongInt;
	      END;
   GenOrder=RECORD
	     Name:String[5];
	     May1,May3:Boolean;
	     Code,Code3:Byte;
	     OnlyMem:Boolean;
	     SwapOps:Boolean;
	     ImmFloat:Boolean;
	     ParMask,Par3Mask:Byte;
	     PCodes,P3Codes:ARRAY[0..7] OF Byte;
	    END;
   SingOrder=RECORD
	      Name:String[4];
	      Code:LongInt;
	      Mask:Byte;
	     END;

   ConditionArray=ARRAY[1..ConditionCount] OF Condition;
   FixedOrderArray=ARRAY[1..FixedOrderCount] OF FixedOrder;
   RotOrderArray=ARRAY[1..RotOrderCount] OF String[4];
   StkOrderArray=ARRAY[1..RotOrderCount] OF String[5];
   GenOrderArray=ARRAY[1..GenOrderCount] OF GenOrder;
   ParOrderArray=ARRAY[0..ParOrderCount-1] OF String[5];
   SingOrderArray=ARRAY[1..SingOrderCount] OF SingOrder;

VAR
   CPU32030,CPU32031:CPUVar;
   SaveInitProc:PROCEDURE;

   NextPar,ThisPar:Boolean;
   PrevARs,ARs:Byte;
   PrevOp:String[6];
   z2:Integer;
   PrevSrc1Mode,PrevSrc2Mode,PrevDestMode:ShortInt;
   CurrSrc1Mode,CurrSrc2Mode,CurrDestMode:ShortInt;
   PrevSrc1Part,PrevSrc2Part,PrevDestPart:Word;
   CurrSrc1Part,CurrSrc2Part,CurrDestPart:Word;

   Conditions:^ConditionArray;
   FixedOrders:^FixedOrderArray;
   RotOrders:^RotOrderArray;
   StkOrders:^StkOrderArray;
   GenOrders:^GenOrderArray;
   ParOrders:^ParOrderArray;
   SingOrders:^SingOrderArray;

   DPValue:LongInt;

{---------------------------------------------------------------------------}
{ Befehlstabellenverwaltung }

	PROCEDURE InitFields;
VAR
   z:Integer;

	PROCEDURE AddCondition(NName:String; NCode:Byte);
BEGIN
   Inc(z); IF z>ConditionCount THEN Halt;
   WITH Conditions^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddFixed(NName:String; NCode:LongInt);
BEGIN
   Inc(z); IF z>FixedOrderCount THEN Halt;
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

	PROCEDURE AddSing(NName:String; NCode:LongInt; NMask:Byte);
BEGIN
   Inc(z); IF z>SingOrderCount THEN Halt;
   WITH SingOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Mask:=NMask;
    END;
END;

	PROCEDURE AddGen(NName:String; NMay1,NMay3:Boolean; NCode,NCode3:Byte;
			 NOnly,NSwap,NImm:Boolean; NMask1,NMask3:Byte;
			 C20,C21,C22,C23,C24,C25,C26,C27,C30,C31,C32,C33,C34,C35,C36,C37:Byte);
BEGIN
   Inc(z); IF z>GenOrderCount THEN Halt;
   WITH GenOrders^[z] DO
    BEGIN
     Name:=NName;
     May1:=NMay1; May3:=NMay3;
     Code:=NCode; Code3:=NCode3;
     OnlyMem:=NOnly; SwapOps:=NSwap; ImmFloat:=NImm;
     ParMask:=NMask1; Par3Mask:=NMask3;
     PCodes[0]:=C20;  PCodes[1]:=C21;  PCodes[2]:=C22;  PCodes[3]:=C23;
     PCodes[4]:=C24;  PCodes[5]:=C25;  PCodes[6]:=C26;  PCodes[7]:=C27;
     P3Codes[0]:=C30; P3Codes[1]:=C31; P3Codes[2]:=C32; P3Codes[3]:=C33;
     P3Codes[4]:=C34; P3Codes[5]:=C35; P3Codes[6]:=C36; P3Codes[7]:=C37;
    END;
END;

BEGIN
   New(Conditions); z:=0;
   AddCondition('U'  ,$00); AddCondition('LO' ,$01);
   AddCondition('LS' ,$02); AddCondition('HI' ,$03);
   AddCondition('HS' ,$04); AddCondition('EQ' ,$05);
   AddCondition('NE' ,$06); AddCondition('LT' ,$07);
   AddCondition('LE' ,$08); AddCondition('GT' ,$09);
   AddCondition('GE' ,$0a); AddCondition('Z'  ,$05);
   AddCondition('NZ' ,$06); AddCondition('P'  ,$09);
   AddCondition('N'  ,$07); AddCondition('NN' ,$0a);
   AddCondition('NV' ,$0c); AddCondition('V'  ,$0d);
   AddCondition('NUF',$0e); AddCondition('UF' ,$0f);
   AddCondition('NC' ,$04); AddCondition('C'  ,$01);
   AddCondition('NLV',$10); AddCondition('LV' ,$11);
   AddCondition('NLUF',$12);AddCondition('LUF',$13);
   AddCondition('ZUF',$14); AddCondition(''   ,$00);

   New(FixedOrders); z:=0;
   AddFixed('IDLE',$06000000); AddFixed('SIGI',$16000000);
   AddFixed('SWI' ,$66000000);

   New(RotOrders);
   RotOrders^[1]:='ROL'; RotOrders^[2]:='ROLC';
   RotOrders^[3]:='ROR'; RotOrders^[4]:='RORC';

   New(StkOrders);
   StkOrders^[1]:='POP';  StkOrders^[2]:='POPF';
   StkOrders^[3]:='PUSH'; StkOrders^[4]:='PUSHF';

   New(GenOrders); z:=0;
{          Name         May3      Cd3       Swap       PM1                                              PCodes3      }
{                 May1        Cd1     OMem        ImmF     PM3           PCodes1                                     }
   AddGen('ABSF' ,True ,False,$00,$ff,False,False,True , 4, 0,
	  $ff,$ff,$04,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('ABSI' ,True ,False,$01,$ff,False,False,False, 8, 0,
	  $ff,$ff,$ff,$05,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('ADDC' ,False,True ,$02,$00,False,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('ADDF' ,False,True ,$03,$01,False,False,True , 0, 4,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$06,$ff,$ff,$ff,$ff,$ff);
   AddGen('ADDI' ,False,True ,$04,$02,False,False,False, 0, 8,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$07,$ff,$ff,$ff,$ff);
   AddGen('AND'  ,False,True ,$05,$03,False,False,False, 0, 8,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$08,$ff,$ff,$ff,$ff);
   AddGen('ANDN' ,False,True ,$06,$04,False,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('ASH'  ,False,True ,$07,$05,False,False,False, 0, 8,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$09,$ff,$ff,$ff,$ff);
   AddGen('CMPF' ,False,True ,$08,$06,False,False,True , 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('CMPI' ,False,True ,$09,$07,False,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('FIX'  ,True ,False,$0a,$ff,False,False,True , 8, 0,
	  $ff,$ff,$ff,$0a,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('FLOAT',True ,False,$0b,$ff,False,False,False, 4, 0,
	  $ff,$ff,$0b,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('LDE'  ,False,False,$0d,$ff,False,False,True , 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('LDF'  ,False,False,$0e,$ff,False,False,True , 5, 0,
	  $02,$ff,$0c,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('LDFI' ,False,False,$0f,$ff,True ,False,True , 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('LDI'  ,False,False,$10,$ff,False,False,False,10, 0,
	  $ff,$03,$ff,$0d,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('LDII' ,False,False,$11,$ff,True ,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('LDM'  ,False,False,$12,$ff,False,False,True , 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('LSH'  ,False,True ,$13,$08,False,False,False, 0, 8,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$0e,$ff,$ff,$ff,$ff);
   AddGen('MPYF' ,False,True ,$14,$09,False,False,True , 0,52,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$0f,$ff,$00,$01,$ff,$ff);
   AddGen('MPYI' ,False,True ,$15,$0a,False,False,False, 0,200,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$10,$ff,$ff,$02,$03);
   AddGen('NEGB' ,True ,False,$16,$ff,False,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('NEGF' ,True ,False,$17,$ff,False,False,True , 4, 0,
	  $ff,$ff,$11,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('NEGI' ,True ,False,$18,$ff,False,False,False, 8, 0,
	  $ff,$ff,$ff,$12,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('NORM' ,True ,False,$1a,$ff,False,False,True , 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('NOT'  ,True ,False,$1b,$ff,False,False,False, 8, 0,
	  $ff,$ff,$ff,$13,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('OR'   ,False,True ,$20,$0b,False,False,False, 0, 8,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$14,$ff,$ff,$ff,$ff);
   AddGen('RND'  ,True ,False,$22,$ff,False,False,True , 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('STF'  ,False,False,$28,$ff,True ,True ,True , 4, 0,
	  $ff,$ff,$00,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('STFI' ,False,False,$29,$ff,True ,True ,True , 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('STI'  ,False,False,$2a,$ff,True ,True ,False, 8, 0,
	  $ff,$ff,$ff,$01,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('STII' ,False,False,$2b,$ff,True ,True ,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('SUBB' ,False,True ,$2d,$0c,False,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('SUBC' ,False,False,$2e,$ff,False,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('SUBF' ,False,True ,$2f,$0d,False,False,True , 0, 4,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$15,$ff,$ff,$ff,$ff,$ff);
   AddGen('SUBI' ,False,True ,$30,$0e,False,False,False, 0, 8,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$16,$ff,$ff,$ff,$ff);
   AddGen('SUBRB',False,False,$31,$ff,False,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('SUBRF',False,False,$32,$ff,False,False,True , 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('SUBRI',False,False,$33,$ff,False,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('TSTB' ,False,True ,$34,$0f,False,False,False, 0, 0,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff);
   AddGen('XOR'  ,False,True ,$35,$10,False,False,False, 0, 8,
	  $ff,$ff,$ff,$ff,$ff,$ff,$ff,$ff, $ff,$ff,$ff,$17,$ff,$ff,$ff,$ff);

   New(ParOrders);
   ParOrders^[0]:='LDF';   ParOrders^[1]:='LDI';
   ParOrders^[2]:='STF';   ParOrders^[3]:='STI';
   ParOrders^[4]:='ADDF3'; ParOrders^[5]:='SUBF3';
   ParOrders^[6]:='ADDI3'; ParOrders^[7]:='SUBI3';

   New(SingOrders); z:=0;
   AddSing('IACK',$1b000000,6);
   AddSing('NOP' ,$0c800000,5);
   AddSing('RPTS',$139b0000,15);
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(Conditions);
   Dispose(FixedOrders);
   Dispose(RotOrders);
   Dispose(StkOrders);
   Dispose(GenOrders);
   Dispose(ParOrders);
   Dispose(SingOrders);
END;

{---------------------------------------------------------------------------}
{ Gleitkommawandler }

	PROCEDURE SplitExt(Inp:Extended; VAR Expo,Mant:LongInt);
VAR
   TwoFace:RECORD CASE Boolean OF
	    True:(Ext:Extended);
	    False:(Field:RECORD
			  LowMant:LongInt;
			  HighMant:LongInt;
			  S_Exp:Word;
			  END;);
	   END;
   Sign:Boolean;
BEGIN
   WITH TwoFace DO
    BEGIN
     Ext:=Inp;
     WITH Field DO
      BEGIN
       Sign:=S_Exp>$7fff;
       Expo:=(S_Exp AND $7fff);
       Expo:=Expo-$3fff;
       Mant:=HighMant;
{
       IF Sign THEN
	BEGIN
	 Mant:=-Mant; Inc(Mant,$80000000);
	END
       ELSE Mant:=Mant AND $7fffffff;
}
       IF Sign THEN Mant:=NOT Mant;
       Mant := Mant XOR $80000000;
      END;
    END;
END;

	FUNCTION ExtToShort(Inp:Extended; VAR Erg:Word):Boolean;
VAR
   Expo,Mant:LongInt;
BEGIN
   IF Inp=0 THEN Erg:=$8000
   ELSE
    BEGIN
     SplitExt(Inp,Expo,Mant);
     IF Abs(Expo)>7 THEN
      BEGIN
       ExtToShort:=False;
       IF Expo>0 THEN WrError(1320) ELSE WrError(1315);
       Exit;
      END;
     Erg:=((Expo SHL 12) AND $f000) OR ((Mant SHR 20) AND $fff);
    END;
   ExtToShort:=True;
END;

	FUNCTION ExtToSingle(Inp:Extended; VAR Erg:LongInt):Boolean;
VAR
   Expo,Mant:LongInt;
BEGIN
   IF Inp=0 THEN Erg:=$80000000
   ELSE
    BEGIN
     SplitExt(Inp,Expo,Mant);
     IF Abs(Expo)>127 THEN
      BEGIN
       ExtToSingle:=False;
       IF Expo>0 THEN WrError(1320) ELSE WrError(1315);
       Exit;
      END;
     Erg:=((Expo SHL 24) AND $ff000000)+(Mant SHR 8);
    END;
   ExtToSingle:=True;
END;

	FUNCTION ExtToExt(Inp:Extended; VAR ErgL,ErgH:LongInt):Boolean;
BEGIN
   IF Inp=0 THEN
    BEGIN
     ErgH:=$80; ErgL:=$00000000;
    END
   ELSE
    BEGIN
     SplitExt(Inp,ErgH,ErgL);
     IF Abs(ErgH)>127 THEN
      BEGIN
       ExtToExt:=False;
       IF ErgH>0 THEN WrError(1320) ELSE WrError(1315);
       Exit;
      END;
     ErgH:=ErgH AND $ff;
    END;
   ExtToExt:=True;
END;

{---------------------------------------------------------------------------}
{ Adre·parser }

CONST
   ModNone=-1;
   ModReg=0;     MModReg=1 SHL ModReg;
   ModDir=1;     MModDir=1 SHL ModDir;
   ModInd=2;     MModInd=1 SHL ModInd;
   ModImm=3;     MModImm=1 SHL ModImm;

VAR
   AdrMode:ShortInt;
   AdrPart:LongInt;

	FUNCTION DecodeReg(Asc:String; VAR Erg:Byte):Boolean;
CONST
   RegCnt=12;
   RegStart=$10;
   Regs:ARRAY[0..RegCnt-1] OF String[3]=
	('DP','IR0','IR1','BK','SP','ST','IE','IF','IOF','RS','RE','RC');
VAR
   Err:ValErgType;
BEGIN
   DecodeReg:=False;

   IF (UpCase(Asc[1])='R') AND (Length(Asc)<=3) AND (Length(Asc)>=2) THEN
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,Err);
     IF (Err=0) AND (Erg>=0) AND (Erg<=$1b) THEN
      BEGIN
       DecodeReg:=True; Exit;
      END;
    END;

   IF (Length(Asc)=3) AND (UpCase(Asc[1])='A') AND (UpCase(Asc[2])='R') AND (Asc[3]>='0') AND (Asc[3]<='7') THEN
    BEGIN
     DecodeReg:=True; Erg:=Ord(Asc[3])-AscOfs+8; Exit;
    END;

   Erg:=0; NLS_UpString(Asc);
   WHILE (Erg<RegCnt) AND (Regs[Erg]<>Asc) DO Inc(Erg);
   IF Erg<RegCnt THEN
    BEGIN
     DecodeReg:=True; Inc(Erg,RegStart);
    END;
END;

	PROCEDURE DecodeAdr(Asc:String; Erl:Byte; ImmFloat:Boolean);
LABEL
   Found;
VAR
   HReg:Byte;
   Disp,p:Integer;
   f:Extended;
   fi:Word;
   AdrLong:LongInt;
   BitRev,Circ:Boolean;
   NDisp:String;
   OK:Boolean;
   Mode:(ModBase,ModAdd,ModSub,ModPreInc,ModPreDec,ModPostInc,ModPostDec);
BEGIN
   KillBlanks(Asc);

   AdrMode:=ModNone;

   { I. Register? }

   IF DecodeReg(Asc,HReg) THEN
    BEGIN
     AdrMode:=ModReg; AdrPart:=HReg; Goto Found;
    END;

   { II. indirekt ? }

   IF Asc[1]='*' THEN
    BEGIN
     { II.1. Erkennungszeichen entfernen }

     Delete(Asc,1,1);

     { II.2. ExtrawÅrste erledigen }

     BitRev:=False; Circ:=False;
     IF UpCase(Asc[Length(Asc)])='B' THEN
      BEGIN
       BitRev:=True; Dec(Byte(Asc[0]));
      END
     ELSE IF Asc[Length(Asc)]='%' THEN
      BEGIN
       Circ:=True; Dec(Byte(Asc[0]));
      END;

     { II.3. Displacement entfernen und auswerten:
	     0..255-->Displacement
	     -1,-2 -->IR0,IR1
	     -3    -->Default }

     p:=QuotPos(Asc,'(');
     IF p<=Length(Asc) THEN
      BEGIN
       IF Asc[Length(Asc)]<>')' THEN
	BEGIN
	 WrError(1350); Exit;
	END;
       NDisp:=Copy(Asc,p+1,Length(Asc)-p-1); Byte(Asc[0]):=p-1;
       IF NLS_StrCaseCmp(NDisp,'IR0')=0 THEN Disp:=-1
       ELSE IF NLS_StrCaseCmp(NDisp,'IR1')=0 THEN Disp:=-2
       ELSE
	BEGIN
	 Disp:=EvalIntExpression(NDisp,UInt8,OK);
	 IF NOT OK THEN Exit;
	END;
      END
     ELSE Disp:=-3;

     { II.4. Addieren/Subtrahieren mit/ohne Update? }

     IF Asc[1]='-' THEN
      BEGIN
       IF Asc[2]='-' THEN
	BEGIN
	 Mode:=ModPreDec; Delete(Asc,1,2);
	END
       ELSE
	BEGIN
	 Mode:=ModSub; Delete(Asc,1,1);
	END;
      END
     ELSE IF Asc[1]='+' THEN
      BEGIN
       IF Asc[2]='+' THEN
	BEGIN
	 Mode:=ModPreInc; Delete(Asc,1,2);
	END
       ELSE
	BEGIN
	 Mode:=ModAdd; Delete(Asc,1,1);
	END;
      END
     ELSE IF Asc[Length(Asc)]='-' THEN
      BEGIN
       IF Asc[Length(Asc)-1]='-' THEN
	BEGIN
	 Mode:=ModPostDec; Dec(Byte(Asc[0]),2);
	END
       ELSE
	BEGIN
	 WrError(1350); Exit;
	END;
      END
     ELSE IF Asc[Length(Asc)]='+' THEN
      BEGIN
       IF Asc[Length(Asc)-1]='+' THEN
	BEGIN
	 Mode:=ModPostInc; Dec(Byte(Asc[0]),2);
	END
       ELSE
	BEGIN
	 WrError(1350); Exit;
	END;
      END
     ELSE Mode:=ModBase;

     { II.5. Rest mu· Basisregister sein }

     IF (NOT DecodeReg(Asc,HReg)) OR (HReg<8) OR (HReg>15) THEN
      BEGIN
       WrError(1350); Exit;
      END;
     Dec(HReg,8);
     IF ARs AND (1 SHL HReg)=0 THEN Inc(ARs,1 SHL HReg)
     ELSE WrXError(210,Asc);

     { II.6. Default-Displacement explizit machen }

     IF Disp=-3 THEN
      IF (Mode=ModBase) THEN Disp:=0 ELSE Disp:=1;

     { II.7. Entscheidungsbaum }

     CASE Mode OF
     ModBase,
     ModAdd:
      IF (Circ) OR (BitRev) THEN WrError(1350)
      ELSE
       BEGIN
	CASE Disp OF
	-2:AdrPart:=$8000;
	-1:AdrPart:=$4000;
	 0:AdrPart:=$c000;
	ELSE AdrPart:=Disp;
	END;
	Inc(AdrPart,Word(HReg) SHL 8); AdrMode:=ModInd;
       END;
     ModSub:
      IF (Circ) OR (BitRev) THEN WrError(1350)
      ELSE
       BEGIN
	CASE Disp OF
	-2:AdrPart:=$8800;
	-1:AdrPart:=$4800;
	 0:AdrPart:=$c000;
	ELSE AdrPart:=$0800+Disp;
	END;
	Inc(AdrPart,Word(HReg) SHL 8); AdrMode:=ModInd;
       END;
     ModPreInc:
      IF (Circ) OR (BitRev) THEN WrError(1350)
      ELSE
       BEGIN
	CASE Disp OF
	-2:AdrPart:=$9000;
	-1:AdrPart:=$5000;
	ELSE AdrPart:=$1000+Disp;
	END;
	Inc(AdrPart,Word(HReg) SHL 8); AdrMode:=ModInd;
       END;
     ModPreDec:
      IF (Circ) OR (BitRev) THEN WrError(1350)
      ELSE
       BEGIN
	CASE Disp OF
	-2:AdrPart:=$9800;
	-1:AdrPart:=$5800;
	ELSE AdrPart:=$1800+Disp;
	END;
	Inc(AdrPart,Word(HReg) SHL 8); AdrMode:=ModInd;
       END;
     ModPostInc:
      IF BitRev THEN
       BEGIN
	IF Disp<>-1 THEN WrError(1350)
	ELSE
	 BEGIN
	  AdrPart:=$c800+(Word(HReg) SHL 8); AdrMode:=ModInd;
	 END;
       END
      ELSE
       BEGIN
	CASE Disp OF
	-2:AdrPart:=$a000;
	-1:AdrPart:=$6000;
	ELSE AdrPart:=$2000+Disp;
	END;
	IF Circ THEN Inc(AdrPart,$1000);
	Inc(AdrPart,Word(HReg) SHL 8); AdrMode:=ModInd;
       END;
     ModPostDec:
      IF BitRev THEN WrError(1350)
      ELSE
       BEGIN
	CASE Disp OF
	-2:AdrPart:=$a800;
	-1:AdrPart:=$6800;
	ELSE AdrPart:=$2800+Disp;
	END;
	IF Circ THEN Inc(AdrPart,$1000);
	Inc(AdrPart,Word(HReg) SHL 8); AdrMode:=ModInd;
       END;
     END;

     Goto Found;
    END;

   { III. absolut }

   IF Asc[1]='@' THEN
    BEGIN
     AdrLong:=EvalIntExpression(Copy(Asc,2,Length(Asc)-1),UInt24,OK);
     IF OK THEN
      BEGIN
       IF (DPValue<>-1) AND (AdrLong SHR 16<>DPValue) THEN WrError(110);
       AdrMode:=ModDir; AdrPart:=AdrLong AND $ffff;
      END;
     Goto Found;
    END;

   { IV. immediate }

   IF ImmFloat THEN
    BEGIN
     f:=EvalFloatExpression(Asc,Float80,OK);
     IF OK THEN
      IF ExtToShort(f,fi) THEN
       BEGIN
	AdrPart:=fi; AdrMode:=ModImm;
       END;
    END
   ELSE
    BEGIN
     AdrPart:=EvalIntExpression(Asc,Int16,OK);
     IF OK THEN
      BEGIN
       AdrPart:=AdrPart AND $ffff; AdrMode:=ModImm;
      END;
    END;

Found:
   IF (AdrMode<>ModNone) AND (Erl AND (1 SHL AdrMode)=0) THEN
    BEGIN
     AdrMode:=ModNone; WrError(1350);
    END;
END;

	FUNCTION EffPart(Mode:Byte; Part:Word):Word;
BEGIN
   CASE Mode OF
   ModReg:EffPart:=Lo(Part);
   ModInd:EffPart:=Hi(Part);
   END;
END;

{---------------------------------------------------------------------------}
{ Code-Erzeugung }

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUME3203Count=1;
   ASSUME3203s:ARRAY[1..ASSUME3203Count] OF ASSUMERec=
               ((Name:'DP'; Dest:@DPValue; Min:-1; Max:$ff; NothingVal:$100));
VAR
   OK:Boolean;
   z,z2:Integer;
   Size:LongInt;
   f:Extended;
   t:TempResult;
BEGIN
   DecodePseudo:=True;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME3203s,ASSUME3203Count);
     Exit;
    END;

   IF Memo('SINGLE') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  f:=EvalFloatExpression(ArgStr[z],Float80,OK);
	  IF OK THEN
	   OK:=OK AND ExtToSingle(f,DAsmCode[CodeLen]);
	  Inc(CodeLen);
	 END;
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF Memo('EXTENDED') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  f:=EvalFloatExpression(ArgStr[z],Float80,OK);
	  IF OK THEN
	   OK:=OK AND ExtToExt(f,DAsmCode[CodeLen+1],DAsmCode[CodeLen]);
	  Inc(CodeLen,2);
	 END;
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF Memo('WORD') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  DAsmCode[CodeLen]:=EvalIntExpression(ArgStr[z],Int32,OK);
	  Inc(CodeLen);
	 END;
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF Memo('DATA') THEN
    BEGIN
     IF ArgCnt=0 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True;
       FOR z:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
	  EvalExpression(ArgStr[z],t);
	  CASE t.Typ OF
	  TempInt:
	   BEGIN
	    DAsmCode[CodeLen]:=t.Int; Inc(CodeLen);
	   END;
	  TempFloat:
	   IF ExtToSingle(t.Float,DAsmCode[CodeLen]) THEN Inc(CodeLen)
	   ELSE OK:=False;
	  TempString:
	   BEGIN
	    FOR z2:=0 TO Length(t.Ascii)-1 DO
	     BEGIN
	      IF z2 AND 3=0 THEN
	       BEGIN
		DAsmCode[CodeLen]:=0; Inc(CodeLen);
	       END;
	      Inc(DAsmCode[CodeLen-1],
		  (LongInt(CharTransTable[t.Ascii[z2+1]])) SHL (8*(3-(z2 AND 3))) );
	     END;
	   END;
	  TempNone:OK:=False;
	  END;
	 END;
       IF NOT OK THEN CodeLen:=0;
      END;
     Exit;
    END;

   IF Memo('BSS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       Size:=EvalIntExpression(ArgStr[1],UInt24,OK);
       IF FirstPassUnknown THEN WrError(1820);
       IF (OK) AND (NOT FirstPassUnknown) THEN
	BEGIN
	 DontPrint:=True;
	 CodeLen:=Size;
	 IF MakeUseList THEN
	  IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
	END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_3203X;
	Far;

	PROCEDURE JudgePar(VAR Prim:GenOrder; Sec:Integer; VAR ErgMode,ErgCode:Byte);
BEGIN
   IF Sec>3 THEN ErgMode:=3
   ELSE IF Prim.May3 THEN ErgMode:=1
   ELSE ErgMode:=2;
   IF ErgMode=2 THEN ErgCode:=Prim.PCodes[Sec]
		ELSE ErgCode:=Prim.P3Codes[Sec];
END;

	FUNCTION EvalAdrExpression(VAR Asc:String; VAR OK:Boolean):LongInt;
BEGIN
   IF Asc[1]='@' THEN Delete(Asc,1,1);
   EvalAdrExpression:=EvalIntExpression(Asc,UInt24,OK);
END;

	PROCEDURE SwapMode(VAR M1,M2:ShortInt);
BEGIN
   AdrMode:=M1; M1:=M2; M2:=AdrMode;
END;

	PROCEDURE SwapPart(VAR P1,P2:Word);
BEGIN
   AdrPart:=P1; P1:=P2; P2:=AdrPart;
END;

VAR
   OK,Is3:Boolean;
   HReg,HReg2,Sum:Byte;
   z,z3:Integer;
   AdrLong,DFlag,Disp:LongInt;
   HOp:String;
BEGIN
   CodeLen:=0; DontPrint:=False;

   ThisPar:=LabPart='||';
   IF (Length(OpPart)>2) AND (Copy(OpPart,1,2)='||') THEN
    BEGIN
     ThisPar:=True; Delete(OpPart,1,2);
    END;
   IF (NOT NextPar) AND (ThisPar) THEN
    BEGIN
     WrError(1950); Exit;
    END;
   ARs:=0;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF ThisPar THEN WrError(1950)
       ELSE
	BEGIN
	 DAsmCode[0]:=Code; CodeLen:=1;
	END;
       NextPar:=False; Exit;
      END;

   { Arithmetik/Logik }

   FOR z:=1 TO GenOrderCount DO
    WITH GenOrders^[z] DO
     IF (Memo(Name)) OR (Memo(Name+'3')) THEN
      BEGIN
       NextPar:=False;
       { Argumentzahl abgleichen }
       IF ArgCnt=1 THEN
	IF May1 THEN
	 BEGIN
	  ArgCnt:=2; ArgStr[2]:=ArgStr[1];
	 END
	ELSE
	 BEGIN
	  WrError(1110); Exit;
	 END;
       IF (ArgCnt=3) AND (OpPart[Length(OpPart)]<>'3') THEN OpPart:=OpPart+'3';
       Is3:=OpPart[Length(OpPart)]='3';
       IF (SwapOps) AND (NOT Is3) THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       IF (Is3) AND (ArgCnt=2) THEN
	BEGIN
	 ArgCnt:=3; ArgStr[3]:=ArgStr[2];
	END;
       IF (ArgCnt<2) OR (ArgCnt>3) OR ((Is3) AND (NOT May3)) THEN
	BEGIN
	 WrError(1110); Exit;
	END;
       { Argumente parsen }
       IF Is3 THEN
	BEGIN
	 IF Memo('TSTB3') THEN
	  BEGIN
	   CurrDestMode:=ModReg; CurrDestPart:=0;
	  END
	 ELSE
	  BEGIN
	   DecodeAdr(ArgStr[3],MModReg,ImmFloat);
	   IF AdrMode=ModNone THEN Exit;
	   CurrDestMode:=AdrMode; CurrDestPart:=AdrPart;
	  END;
	 DecodeAdr(ArgStr[2],MModReg+MModInd,ImmFloat);
	 IF AdrMode=ModNone THEN Exit;
	 IF (AdrMode=ModInd) AND (AdrPart AND $e000=0) AND (Lo(AdrPart)<>1) THEN
	  BEGIN
	   WrError(1350); Exit;
	  END;
	 CurrSrc2Mode:=AdrMode; CurrSrc2Part:=AdrPart;
	 DecodeAdr(ArgStr[1],MModReg+MModInd,ImmFloat);
	 IF AdrMode=ModNone THEN Exit;
	 IF (AdrMode=ModInd) AND (AdrPart AND $e000=0) AND (Lo(AdrPart)<>1) THEN
	  BEGIN
	   WrError(1350); Exit;
	  END;
	 CurrSrc1Mode:=AdrMode; CurrSrc1Part:=AdrPart;
	END
       ELSE { NOT Is3 }
	BEGIN
         IF OnlyMem THEN DecodeAdr(ArgStr[1],MModDir+MModInd,ImmFloat)
                    ELSE DecodeAdr(ArgStr[1],MModDir+MModInd+MModReg+MModImm,ImmFloat);
         IF AdrMode=ModNone THEN Exit;
         CurrSrc1Mode:=AdrMode; CurrSrc1Part:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg+MModInd,ImmFloat);
         CASE AdrMode OF
         ModReg:
          BEGIN
           CurrDestMode:=AdrMode; CurrDestPart:=AdrPart;
           CurrSrc2Mode:=CurrSrc1Mode; CurrSrc2Part:=CurrSrc1Part;
          END;
         ModInd:
          IF ((OpPart<>'TSTB') AND (OpPart<>'CMPI') AND (OpPart<>'CMPF'))
          OR ((CurrSrc1Mode=ModDir) OR (CurrSrc1Mode=ModImm))
          OR ((CurrSrc1Mode=ModInd) AND (CurrSrc1Part AND $e000=0) AND (Lo(CurrSrc1Part)<>1))
          OR ((AdrPart AND $e000=0) AND (Lo(AdrPart)<>1)) THEN
           BEGIN
            WrError(1350); Exit;
           END
          ELSE
           BEGIN
            Is3:=True; CurrDestMode:=ModReg; CurrDestPart:=0;
            CurrSrc2Mode:=AdrMode; CurrSrc2Part:=AdrPart;
           END;
         ModNone:Exit;
         END;
	END;
       { auswerten: parallel... }
       IF ThisPar THEN
	BEGIN
	 { in Standardreihenfolge suchen }
	 IF PrevOp[Length(PrevOp)]='3' THEN HReg:=GenOrders^[z2].Par3Mask
	 ELSE HReg:=GenOrders^[z2].ParMask;
         z3:=0;
         WHILE (z3<ParOrderCount) AND ((NOT Odd(HReg)) OR (ParOrders^[z3]<>OpPart)) DO
          BEGIN
           Inc(z3); HReg:=HReg SHR 1;
	  END;
         IF z3<ParOrderCount THEN JudgePar(GenOrders^[z2],z3,HReg,HReg2)
         { in gedrehter Reihenfolge suchen }
         ELSE
          BEGIN
           IF OpPart[Length(OpPart)]='3' THEN HReg:=GenOrders^[z].Par3Mask
           ELSE HReg:=GenOrders^[z].ParMask;
           z3:=0;
           WHILE (z3<ParOrderCount) AND ((NOT Odd(HReg)) OR (ParOrders^[z3]<>PrevOp)) DO
            BEGIN
             Inc(z3); HReg:=HReg SHR 1;
            END;
           IF z3<ParOrderCount THEN
	    BEGIN
	     JudgePar(GenOrders^[z],z3,HReg,HReg2);
             SwapMode(CurrDestMode,PrevDestMode);
             SwapMode(CurrSrc1Mode,PrevSrc1Mode);
             SwapMode(CurrSrc2Mode,PrevSrc2Mode);
             SwapPart(CurrDestPart,PrevDestPart);
             SwapPart(CurrSrc1Part,PrevSrc1Part);
	     SwapPart(CurrSrc2Part,PrevSrc2Part);
            END
	   ELSE
	    BEGIN
	     WrError(1950); Exit;
	    END;
	  END;
	 { mehrfache Registernutzung ? }
	 FOR z3:=0 TO 7 DO
	  IF ARs AND PrevARs AND (1 SHL z3)<>0 THEN
	   WrXError(210,'AR'+Chr(z3+AscOfs));
	 { 3 BasisfÑlle }
	 CASE HReg OF
	 1:BEGIN
	    IF (PrevOp='LSH3') OR (PrevOp='ASH3') OR (PrevOp='SUBF3') OR (PrevOp='SUBI3') THEN
	     BEGIN
	      SwapMode(PrevSrc1Mode,PrevSrc2Mode);
	      SwapPart(PrevSrc1Part,PrevSrc2Part);
	     END;
	    IF (PrevDestPart>7) OR (CurrDestPart>7) THEN
	     BEGIN
	      WrError(1445); Exit;
	     END;
            { Bei Addition und Multiplikation KommutativitÑt nutzen }
            IF  (PrevSrc2Mode=ModInd) AND (PrevSrc1Mode=ModReg)
            AND ((Copy(PrevOp,1,3)='ADD') OR (Copy(PrevOp,1,3)='MPY')
              OR (Copy(PrevOp,1,3)='AND') OR (Copy(PrevOp,1,3)='XOR')
              OR (Copy(PrevOp,1,2)='OR')) THEN
             BEGIN
              SwapMode(PrevSrc1Mode,PrevSrc2Mode);
              SwapPart(PrevSrc1Part,PrevSrc2Part);
             END;
	    IF (PrevSrc2Mode<>ModReg) OR (PrevSrc2Part>7)
	    OR (PrevSrc1Mode<>ModInd) OR (CurrSrc1Mode<>ModInd) THEN
	     BEGIN
	      WrError(1355); Exit;
	     END;
	    RetractWords(1);
	    DAsmCode[0]:=$c0000000+(LongInt(HReg2) SHL 25)
			+(LongInt(PrevDestPart) SHL 22)
			+(LongInt(PrevSrc2Part) SHL 19)
			+(LongInt(CurrDestPart) SHL 16)
			+(CurrSrc1Part AND $ff00)+Hi(PrevSrc1Part);
	    CodeLen:=1; NextPar:=False;
	   END;
	 2:BEGIN
	    IF (PrevDestPart>7) OR (CurrDestPart>7) THEN
	     BEGIN
	      WrError(1445); Exit;
	     END;
	    IF (PrevSrc1Mode<>ModInd) OR (CurrSrc1Mode<>ModInd) THEN
	     BEGIN
	      WrError(1355); Exit;
	     END;
	    RetractWords(1);
	    DAsmCode[0]:=$c0000000+(LongInt(HReg2) SHL 25)
			+(LongInt(PrevDestPart) SHL 22)
			+(CurrSrc1Part AND $ff00)+Hi(PrevSrc1Part);
	    IF (PrevOp=OpPart) AND (OpPart[1]='L') THEN
	     BEGIN
	      Inc(DAsmCode[0],LongInt(CurrDestPart) SHL 19);
	      IF PrevDestPart=CurrDestPart THEN WrError(140);
	     END
	    ELSE
	     Inc(DAsmCode[0],LongInt(CurrDestPart) SHL 16);
	    CodeLen:=1; NextPar:=False;
	   END;
	 3:BEGIN
	    IF (PrevDestPart>1) OR (CurrDestPart<2) OR (CurrDestPart>3) THEN
	     BEGIN
	      WrError(1445); Exit;
	     END;
	    Sum:=0;
	    IF PrevSrc1Mode=ModInd THEN Inc(Sum);
	    IF PrevSrc2Mode=ModInd THEN Inc(Sum);
	    IF CurrSrc1Mode=ModInd THEN Inc(Sum);
	    IF CurrSrc2Mode=ModInd THEN Inc(Sum);
	    IF Sum<>2 THEN
	     BEGIN
	      WrError(1355); Exit;
	     END;
	    RetractWords(1);
	    DAsmCode[0]:=$80000000+(LongInt(HReg2) SHL 26)
			+(LongInt(PrevDestPart AND 1) SHL 23)
			+(LongInt(CurrDestPart AND 1) SHL 22);
	    CodeLen:=1;
	    IF CurrSrc2Mode=ModReg THEN
	     IF CurrSrc1Mode=ModReg THEN
	      BEGIN
	       Inc(DAsmCode[0],LongInt($00000000)
                              +(LongInt(CurrSrc2Part) SHL 19)
                              +(LongInt(CurrSrc1Part) SHL 16)
                              +(PrevSrc2Part AND $ff00)+Hi(PrevSrc1Part));
              END
             ELSE
              BEGIN
               Inc(DAsmCode[0],LongInt($03000000)
                              +(LongInt(CurrSrc2Part) SHL 16)
                              +Hi(CurrSrc1Part));
               IF PrevSrc1Mode=ModReg THEN
                Inc(DAsmCode[0],(LongInt(PrevSrc1Part) SHL 19)+(PrevSrc2Part AND $ff00))
	       ELSE
                Inc(DAsmCode[0],(LongInt(PrevSrc2Part) SHL 19)+(PrevSrc1Part AND $ff00))
              END
            ELSE
             IF CurrSrc1Mode=ModReg THEN
              BEGIN
               Inc(DAsmCode[0],LongInt($01000000)
                              +(LongInt(CurrSrc1Part) SHL 16)
                              +Hi(CurrSrc2Part));
               IF PrevSrc1Mode=ModReg THEN
                Inc(DAsmCode[0],(LongInt(PrevSrc1Part) SHL 19)+(PrevSrc2Part AND $ff00))
               ELSE
                Inc(DAsmCode[0],(LongInt(PrevSrc2Part) SHL 19)+(PrevSrc1Part AND $ff00))
              END
	     ELSE
              BEGIN
               Inc(DAsmCode[0],LongInt($02000000)
                              +(LongInt(PrevSrc2Part) SHL 19)
                              +(LongInt(PrevSrc1Part) SHL 16)
                              +(CurrSrc2Part AND $ff00)+Hi(CurrSrc1Part));
              END
           END;
         END;
	END
       { ...sequentiell }
       ELSE
	BEGIN
	 PrevSrc1Mode:=CurrSrc1Mode; PrevSrc1Part:=CurrSrc1Part;
	 PrevSrc2Mode:=CurrSrc2Mode; PrevSrc2Part:=CurrSrc2Part;
	 PrevDestMode:=CurrDestMode; PrevDestPart:=CurrDestPart;
	 PrevOp:=OpPart; PrevARs:=ARs; z2:=z;
	 IF Is3 THEN
	  DAsmCode[0]:=$20000000+(LongInt(Code3) SHL 23)
                      +(LongInt(CurrDestPart) SHL 16)
		      +(LongInt(CurrSrc2Mode) SHL 20)+EffPart(CurrSrc2Mode,CurrSrc2Part) SHL 8
		      +(LongInt(CurrSrc1Mode) SHL 21)+EffPart(CurrSrc1Mode,CurrSrc1Part)
	 ELSE
	  DAsmCode[0]:=$00000000+(LongInt(Code) SHL 23)
		      +(LongInt(CurrSrc1Mode) SHL 21)+CurrSrc1Part
		      +(LongInt(CurrDestPart) SHL 16);
	 CodeLen:=1; NextPar:=True;
	END;
       Exit;
      END;

   FOR z:=1 TO RotOrderCount DO
    IF Memo(RotOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE IF ThisPar THEN WrError(1950)
      ELSE IF NOT DecodeReg(ArgStr[1],HReg) THEN WrError(1350)
      ELSE
       BEGIN
	DAsmCode[0]:=$11e00000+(LongInt(z-1) SHL 23)+(LongInt(HReg) SHL 16);
	CodeLen:=1;
       END;
      NextPar:=False; Exit;
     END;

   FOR z:=1 TO StkOrderCount DO
    IF Memo(StkOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>1 THEN WrError(1110)
      ELSE IF ThisPar THEN WrError(1950)
      ELSE IF NOT DecodeReg(ArgStr[1],HReg) THEN WrError(1350)
      ELSE
       BEGIN
	DAsmCode[0]:=$0e200000+(LongInt(z-1) SHL 23)+(LongInt(HReg) SHL 16);
	CodeLen:=1;
       END;
      NextPar:=False; Exit;
     END;

   { Datentransfer }

   IF (Copy(OpPart,1,3)='LDI') OR (Copy(OpPart,1,3)='LDF') THEN
    BEGIN
     HOp:=OpPart; Delete(OpPart,1,3);
     FOR z:=1 TO ConditionCount DO
      WITH Conditions^[z] DO
       IF Memo(Name) THEN
	BEGIN
	 IF ArgCnt<>2 THEN WrError(1110)
	 ELSE IF ThisPar THEN WrError(1950)
	 ELSE
	  BEGIN
	   DecodeAdr(ArgStr[2],MModReg,False);
	   IF AdrMode<>ModNone THEN
	    BEGIN
	     HReg:=AdrPart;
	     DecodeAdr(ArgStr[1],MModReg+MModDir+MmodInd+MModImm,HOp[3]='F');
	     IF AdrMode<>ModNone THEN
	      BEGIN
	       DAsmCode[0]:=$40000000+(LongInt(HReg) SHL 16)
			   +(LongInt(Code) SHL 23)
			   +(LongInt(AdrMode) SHL 21)+AdrPart;
	       IF HOp[3]='I' THEN Inc(DAsmCode[0],$10000000);
	       CodeLen:=1;
	      END;
	    END;
	  END;
	 NextPar:=False; Exit;
	END;
     WrXError(1200,HOp); NextPar:=False; Exit;
    END;


   { Sonderfall NOP auch ohne Argumente }

   IF (Memo('NOP')) AND (ArgCnt=0) THEN
    BEGIN
     CodeLen:=1; DAsmCode[0]:=NOPCode; Exit;
    END;

   { SonderfÑlle }

   FOR z:=1 TO SingOrderCount DO
    WITH SingOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF ThisPar THEN WrError(1950)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],Mask,False);
	 IF AdrMode<>ModNone THEN
	  BEGIN
	   DAsmCode[0]:=Code+(LongInt(AdrMode) SHL 21)+AdrPart;
	   CodeLen:=1;
	  END;
	END;
       NextPar:=False; Exit;
      END;

   IF Memo('LDP') THEN
    BEGIN
     IF (ArgCnt<>1) AND (ArgCnt<>2) THEN WrError(1110)
     ELSE IF ThisPar THEN WrError(1950)
     ELSE IF (ArgCnt=2) AND (NLS_StrCaseCmp(ArgStr[2],'DP')<>0) THEN WrError(1350)
     ELSE
      BEGIN
       AdrLong:=EvalAdrExpression(ArgStr[1],OK);
       IF OK THEN
	BEGIN
	 DAsmCode[0]:=$08700000+(AdrLong SHR 16);
	 CodeLen:=1;
	END;
      END;
     NextPar:=False; Exit;
    END;

   { Schleifen }

   IF Memo('RPTB') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ThisPar THEN WrError(1950)
     ELSE
      BEGIN
       AdrLong:=EvalAdrExpression(ArgStr[1],OK);
       IF OK THEN
	BEGIN
	 DAsmCode[0]:=$64000000+AdrLong;
	 CodeLen:=1;
	END;
      END;
     NextPar:=False; Exit;
    END;

   { SprÅnge }

   IF (Memo('BR')) OR (Memo('BRD')) OR (Memo('CALL')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ThisPar THEN WrError(1950)
     ELSE
      BEGIN
       AdrLong:=EvalAdrExpression(ArgStr[1],OK);
       IF OK THEN
	BEGIN
	 DAsmCode[0]:=$60000000+AdrLong;
	 IF Memo('BRD') THEN Inc(DAsmCode[0],$01000000)
	 ELSE IF Memo('CALL') THEN Inc(DAsmCode[0],$02000000);
	 CodeLen:=1;
	END;
      END;
     NextPar:=False; Exit;
    END;

   IF OpPart[1]='B' THEN
    BEGIN
     HOp:=OpPart;
     Delete(OpPart,1,1);
     IF OpPart[Length(OpPart)]='D' THEN
      BEGIN
       Dec(Byte(OpPart[0])); DFlag:=1 SHL 21;
       Disp:=3;
      END
     ELSE
      BEGIN
       DFlag:=0; Disp:=1;
      END;
     FOR z:=1 TO ConditionCount DO
      WITH Conditions^[z] DO
       IF Memo(Name) THEN
	BEGIN
	 IF ArgCnt<>1 THEN WrError(1110)
	 ELSE IF ThisPar THEN WrError(1950)
	 ELSE IF DecodeReg(ArgStr[1],HReg) THEN
	  BEGIN
	   DAsmCode[0]:=$68000000+(LongInt(Code) SHL 16)+DFlag+HReg;
	   CodeLen:=1;
	  END
	 ELSE
	  BEGIN
           AdrLong:=EvalAdrExpression(ArgStr[1],OK)-(EProgCounter+Disp);
	   IF OK THEN
	    IF (AdrLong>$7fff) OR (AdrLong<-$8000) THEN WrError(1370)
	    ELSE
	     BEGIN
	      DAsmCode[0]:=$6a000000+(LongInt(Code) SHL 16)+DFlag+(AdrLong AND $ffff);
	      CodeLen:=1;
	     END;
	  END;
	 NextPar:=False; Exit;
	END;
     WrXError(1200,HOp); NextPar:=False; Exit;
    END;

   IF Copy(OpPart,1,4)='CALL' THEN
    BEGIN
     HOp:=OpPart; Delete(OpPart,1,4);
     FOR z:=1 TO ConditionCount DO
      WITH Conditions^[z] DO
       IF Memo(Name) THEN
	BEGIN
	 IF ArgCnt<>1 THEN WrError(1110)
	 ELSE IF ThisPar THEN WrError(1950)
	 ELSE IF DecodeReg(ArgStr[1],HReg) THEN
	  BEGIN
	   DAsmCode[0]:=$70000000+(LongInt(Code) SHL 16)+HReg;
	   CodeLen:=1;
	  END
	 ELSE
	  BEGIN
	   AdrLong:=EvalAdrExpression(ArgStr[1],OK)-(EProgCounter+1);
	   IF OK THEN
	    IF (AdrLong>$7fff) OR (AdrLong<-$8000) THEN WrError(1370)
	    ELSE
	     BEGIN
	      DAsmCode[0]:=$72000000+(LongInt(Code) SHL 16)+(AdrLong AND $ffff);
	      CodeLen:=1;
	     END;
	  END;
	 NextPar:=False; Exit;
	END;
     WrXError(1200,HOp); NextPar:=False; Exit;
    END;

   IF Copy(OpPart,1,2)='DB' THEN
    BEGIN
     HOp:=OpPart;
     Delete(OpPart,1,2);
     IF OpPart[Length(OpPart)]='D' THEN
      BEGIN
       Dec(Byte(OpPart[0])); DFlag:=1 SHL 21;
       Disp:=3;
      END
     ELSE
      BEGIN
       DFlag:=0; Disp:=1;
      END;
     FOR z:=1 TO ConditionCount DO
      WITH Conditions^[z] DO
       IF Memo(Name) THEN
	BEGIN
	 IF ArgCnt<>2 THEN WrError(1110)
	 ELSE IF ThisPar THEN WrError(1950)
	 ELSE IF NOT DecodeReg(ArgStr[1],HReg2) THEN WrError(1350)
	 ELSE IF (HReg2<8) OR (HReg2>15) THEN WrError(1350)
	 ELSE
	  BEGIN
	   Dec(HReg2,8);
	   IF DecodeReg(ArgStr[2],HReg) THEN
	    BEGIN
	     DAsmCode[0]:=$6c000000
			 +(LongInt(Code) SHL 16)
			 +DFlag
			 +(LongInt(HReg2) SHL 22)
			 +HReg;
	     CodeLen:=1;
	    END
	   ELSE
	    BEGIN
             AdrLong:=EvalAdrExpression(ArgStr[2],OK)-(EProgCounter+Disp);
	     IF OK THEN
	      IF (AdrLong>$7fff) OR (AdrLong<-$8000) THEN WrError(1370)
	      ELSE
	       BEGIN
		DAsmCode[0]:=$6e000000
			    +(LongInt(Code) SHL 16)
			    +DFlag
			    +(LongInt(HReg2) SHL 22)
			    +(AdrLong AND $ffff);
		CodeLen:=1;
	       END;
	    END;
	  END;
	 NextPar:=False; Exit;
	END;
     WrXError(1200,HOp); NextPar:=False; Exit;
    END;

   IF (Copy(OpPart,1,4)='RETI') OR (Copy(OpPart,1,4)='RETS') THEN
    BEGIN
     IF OpPart[4]='S' THEN DFlag:=1 SHL 23 ELSE DFlag:=0;
     HOp:=OpPart; Delete(OpPart,1,4);
     FOR z:=1 TO ConditionCount DO
      WITH Conditions^[z] DO
       IF Memo(Name) THEN
	BEGIN
	 IF ArgCnt<>0 THEN WrError(1110)
	 ELSE IF ThisPar THEN WrError(1950)
	 ELSE
	  BEGIN
	   DAsmCode[0]:=$78000000+DFlag+(LongInt(Code) SHL 16);
	   CodeLen:=1;
	  END;
	 NextPar:=False; Exit;
	END;
     WrXError(1200,HOp); NextPar:=False; Exit;
    END;

   IF Copy(OpPart,1,4)='TRAP' THEN
    BEGIN
     HOp:=OpPart; Delete(OpPart,1,4);
     FOR z:=1 TO ConditionCount DO
      WITH Conditions^[z] DO
       IF Memo(Name) THEN
	BEGIN
	 IF ArgCnt<>1 THEN WrError(1110)
	 ELSE IF ThisPar THEN WrError(1950)
	 ELSE
	  BEGIN
	   HReg:=EvalIntExpression(ArgStr[1],UInt4,OK);
	   IF OK THEN
	    BEGIN
	     DAsmCode[0]:=$74000000+HReg+(LongInt(Code) SHL 16);
	     CodeLen:=1;
	    END;
	  END;
	 NextPar:=False; Exit;
	END;
     WrXError(1200,HOp); NextPar:=False; Exit;
    END;

   WrXError(1200,OpPart); NextPar:=False;
END;

	PROCEDURE InitCode_3203x;
	Far;
BEGIN
   SaveInitProc;
   DPValue:=0;
END;

	FUNCTION ChkPC_3203X:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter <=$ffffff;
   ELSE ok:=False;
   END;
   ChkPC_3203X:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_3203X:Boolean;
	Far;
BEGIN
   IsDef_3203X:=LabPart='||';
END;

	PROCEDURE SwitchFrom_3203X;
	Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_3203X;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$76; NOPCode:=$0c800000;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=4; ListGrans[SegCode]:=4; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_3203X; ChkPC:=ChkPC_3203X; IsDef:=IsDef_3203X;
   SwitchFrom:=SwitchFrom_3203X; InitFields; NextPar:=False;
END;

BEGIN
   CPU32030:=AddCPU('320C30',SwitchTo_3203X);
   CPU32031:=AddCPU('320C31',SwitchTo_3203X);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_3203x;
END.
