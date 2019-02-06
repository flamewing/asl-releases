{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        Unit Code6809;

{ AS - Codegeneratormodul 6809/6309 }

Interface

        Uses NLS,
             AsmDef,AsmPars,AsmSub,CodePseu;

Implementation

TYPE
   BaseOrder=RECORD
	      Name:String[5];
	      Code:Word;
              MinCPU:CPUVar;
	     END;

   FlagOrder=RECORD
	      Name:String[5];
	      Code:Word;
              Inv:Boolean;
              MinCPU:CPUVar;
	     END;

   RelOrder=RECORD
	     Name:String[4];
	     Code8:Word;
	     Code16:Word;
             MinCPU:CPUVar;
	    END;

   ALUOrder=RECORD
	     Name:String[4];
	     Code:Word;
	     Op16:Byte;
	     MayImm:Boolean;
             MinCPU:CPUVar;
	    END;

CONST
   ModNone=-1;
   ModImm=1;
   ModDir=2;
   ModInd=3;
   ModExt=4;

   FixedOrderCnt=73;
   RelOrderCnt=19;
   ALUOrderCnt=65;
   ALU2OrderCount=8;
   RMWOrderCnt=13;
   FlagOrderCnt=3;
   LEAOrderCnt=4;
   ImmOrderCnt=4;
   StackOrderCnt=4;
   BitOrderCnt=8;

   StackRegCnt=11;
   StackRegNames:ARRAY[0..StackRegCnt-1] OF String[3]=
                 ('CCR','A','B','DPR','X','Y','S/U','PC','CC','DP','S');
   StackRegCodes:ARRAY[0..StackRegCnt-1] OF Byte=
                 (    0,  1,  2,    3,  4,  5,    6,   7,   0,   3,  6);

   FlagChars='CVZNIHFE';

TYPE
   FixedOrderArray = ARRAY[1..FixedOrderCnt] OF BaseOrder;
   RelOrderArray   = ARRAY[1..RelOrderCnt  ] OF RelOrder;
   ALUOrderArray   = ARRAY[1..ALUOrderCnt  ] OF ALUOrder;
   ALU2OrderArray  = ARRAY[0..ALU2OrderCount-1] OF String[3];
   RMWOrderARRAY   = ARRAY[1..RMWOrderCnt  ] OF BaseOrder;
   FlagOrderArray  = ARRAY[1..FlagOrderCnt ] OF FlagOrder;
   LEAOrderArray   = ARRAY[1..LEAOrderCnt  ] OF BaseOrder;
   ImmOrderArray   = ARRAY[1..ImmOrderCnt  ] OF BaseOrder;
   StackOrderArray = ARRAY[1..StackOrderCnt] OF BaseOrder;
   BitOrderArray   = ARRAY[0..BitOrderCnt-1] OF String[5];

VAR
   AdrMode:ShortInt;
   AdrCnt:Byte;
   AdrVals:ARRAY[0..4] OF Byte;
   OpSize:Byte;
   ExtFlag:Boolean;
   DPRValue:LongInt;

   FixedOrders : ^FixedOrderArray;
   RelOrders   : ^RelOrderArray;
   ALUOrders   : ^ALUOrderArray;
   ALU2Orders  : ^ALU2OrderArray;
   RMWOrders   : ^RMWOrderArray;
   FlagOrders  : ^FlagOrderArray;
   LEAOrders   : ^LEAOrderArray;
   ImmOrders   : ^ImmOrderArray;
   StackOrders : ^StackOrderArray;
   BitOrders   : ^BitOrderArray;

   SaveInitProc:PROCEDURE;

   CPU6809,CPU6309:CPUVar;

{---------------------------------------------------------------------------}
{ Erzeugung/Auflîsung Codetabellen }

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

   	PROCEDURE AddRel(NName:String; NCode8,NCode16:Word);
BEGIN
   Inc(z); IF z>RelOrderCnt THEN Halt;
   WITH RelOrders^[z] DO
    BEGIN
     Name:=NName; Code8:=NCode8; Code16:=NCode16;
    END;
END;

   	PROCEDURE AddALU(NName:String; NCode:Word; NSize:Byte; NImm:Boolean; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>ALUOrderCnt THEN Halt;
   WITH ALUOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Op16:=NSize; MayImm:=NImm; MinCPU:=NCPU;
    END;
END;

   	PROCEDURE AddRMW(NName:String; NCode:Word; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>RMWOrderCnt THEN Halt;
   WITH RMWOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NCPU;
    END;
END;

   	PROCEDURE AddFlag(NName:String; NCode:Word; NInv:Boolean; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>FlagOrderCnt THEN Halt;
   WITH FlagOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Inv:=NInv; MinCPU:=NCPU;
    END;
END;

   	PROCEDURE AddLEA(NName:String; NCode:Word; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>LEAOrderCnt THEN Halt;
   WITH LEAOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NCPU;
    END;
END;

   	PROCEDURE AddImm(NName:String; NCode:Word; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>ImmOrderCnt THEN Halt;
   WITH ImmOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NCPU;
    END;
END;

   	PROCEDURE AddStack(NName:String; NCode:Word; NCPU:CPUVar);
BEGIN
   Inc(z); IF z>StackOrderCnt THEN Halt;
   WITH StackOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NCPU;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('NOP'  ,$0012,CPU6809); AddFixed('SYNC' ,$0013,CPU6809);
   AddFixed('DAA'  ,$0019,CPU6809); AddFixed('SEX'  ,$001d,CPU6809);
   AddFixed('RTS'  ,$0039,CPU6809); AddFixed('ABX'  ,$003a,CPU6809);
   AddFixed('RTI'  ,$003b,CPU6809); AddFixed('MUL'  ,$003d,CPU6809);
   AddFixed('SWI2' ,$103f,CPU6809); AddFixed('SWI3' ,$113f,CPU6809);
   AddFixed('NEGA' ,$0040,CPU6809); AddFixed('COMA' ,$0043,CPU6809);
   AddFixed('LSRA' ,$0044,CPU6809); AddFixed('RORA' ,$0046,CPU6809);
   AddFixed('ASRA' ,$0047,CPU6809); AddFixed('ASLA' ,$0048,CPU6809);
   AddFixed('LSLA' ,$0048,CPU6809); AddFixed('ROLA' ,$0049,CPU6809);
   AddFixed('DECA' ,$004a,CPU6809); AddFixed('INCA' ,$004c,CPU6809);
   AddFixed('TSTA' ,$004d,CPU6809); AddFixed('CLRA' ,$004f,CPU6809);
   AddFixed('NEGB' ,$0050,CPU6809); AddFixed('COMB' ,$0053,CPU6809);
   AddFixed('LSRB' ,$0054,CPU6809); AddFixed('RORB' ,$0056,CPU6809);
   AddFixed('ASRB' ,$0057,CPU6809); AddFixed('ASLB' ,$0058,CPU6809);
   AddFixed('LSLB' ,$0058,CPU6809); AddFixed('ROLB' ,$0059,CPU6809);
   AddFixed('DECB' ,$005a,CPU6809); AddFixed('INCB' ,$005c,CPU6809);
   AddFixed('TSTB' ,$005d,CPU6809); AddFixed('CLRB' ,$005f,CPU6809);
   AddFixed('PSHSW',$1038,CPU6309); AddFixed('PULSW',$1039,CPU6309);
   AddFixed('PSHUW',$103a,CPU6309); AddFixed('PULUW',$103b,CPU6309);
   AddFixed('SEXW' ,$0014,CPU6309); AddFixed('NEGD' ,$1040,CPU6309);
   AddFixed('COMD' ,$1043,CPU6309); AddFixed('LSRD' ,$1044,CPU6309);
   AddFixed('RORD' ,$1046,CPU6309); AddFixed('ASRD' ,$1047,CPU6309);
   AddFixed('ASLD' ,$1048,CPU6309); AddFixed('LSLD' ,$1048,CPU6309);
   AddFixed('ROLD' ,$1049,CPU6309); AddFixed('DECD' ,$104a,CPU6309);
   AddFixed('INCD' ,$104c,CPU6309); AddFixed('TSTD' ,$104d,CPU6309);
   AddFixed('CLRD' ,$104f,CPU6309); AddFixed('COMW' ,$1053,CPU6309);
   AddFixed('LSRW' ,$1054,CPU6309); AddFixed('RORW' ,$1056,CPU6309);
   AddFixed('ROLW' ,$1059,CPU6309); AddFixed('DECW' ,$105a,CPU6309);
   AddFixed('INCW' ,$105c,CPU6309); AddFixed('TSTW' ,$105d,CPU6309);
   AddFixed('CLRW' ,$105f,CPU6309); AddFixed('COME' ,$1143,CPU6309);
   AddFixed('DECE' ,$114a,CPU6309); AddFixed('INCE' ,$114c,CPU6309);
   AddFixed('TSTE' ,$114d,CPU6309); AddFixed('CLRE' ,$114f,CPU6309);
   AddFixed('COMF' ,$1153,CPU6309); AddFixed('DECF' ,$115a,CPU6309);
   AddFixed('INCF' ,$115c,CPU6309); AddFixed('TSTF' ,$115d,CPU6309);
   AddFixed('CLRF' ,$115f,CPU6309); AddFixed('CLRS' ,$1fd4,CPU6309);
   AddFixed('CLRV' ,$1fd7,CPU6309); AddFixed('CLRX' ,$1fd1,CPU6309);
   AddFixed('CLRY' ,$1fd2,CPU6309);

   New(RelOrders); z:=0;
   AddRel('BRA',$0020,$0016); AddRel('BRN',$0021,$1021);
   AddRel('BHI',$0022,$1022); AddRel('BLS',$0023,$1023);
   AddRel('BHS',$0024,$1024); AddRel('BCC',$0024,$1024);
   AddRel('BLO',$0025,$1025); AddRel('BCS',$0025,$1025);
   AddRel('BNE',$0026,$1026); AddRel('BEQ',$0027,$1027);
   AddRel('BVC',$0028,$1028); AddRel('BVS',$0029,$1029);
   AddRel('BPL',$002a,$102a); AddRel('BMI',$002b,$102b);
   AddRel('BGE',$002c,$102c); AddRel('BLT',$002d,$102d);
   AddRel('BGT',$002e,$102e); AddRel('BLE',$002f,$102f);
   AddRel('BSR',$008d,$0017);

   New(ALUOrders); z:=0;
   AddALU('LDA' ,$0086,0,True ,CPU6809);
   AddALU('STA' ,$0087,0,False,CPU6809);
   AddALU('CMPA',$0081,0,True ,CPU6809);
   AddALU('ADDA',$008b,0,True ,CPU6809);
   AddALU('ADCA',$0089,0,True ,CPU6809);
   AddALU('SUBA',$0080,0,True ,CPU6809);
   AddALU('SBCA',$0082,0,True ,CPU6809);
   AddALU('ANDA',$0084,0,True ,CPU6809);
   AddALU('ORA' ,$008a,0,True ,CPU6809);
   AddALU('EORA',$0088,0,True ,CPU6809);
   AddALU('BITA',$0085,0,True ,CPU6809);

   AddALU('LDB' ,$00c6,0,True ,CPU6809);
   AddALU('STB' ,$00c7,0,False,CPU6809);
   AddALU('CMPB',$00c1,0,True ,CPU6809);
   AddALU('ADDB',$00cb,0,True ,CPU6809);
   AddALU('ADCB',$00c9,0,True ,CPU6809);
   AddALU('SUBB',$00c0,0,True ,CPU6809);
   AddALU('SBCB',$00c2,0,True ,CPU6809);
   AddALU('ANDB',$00c4,0,True ,CPU6809);
   AddALU('ORB' ,$00ca,0,True ,CPU6809);
   AddALU('EORB',$00c8,0,True ,CPU6809);
   AddALU('BITB',$00c5,0,True ,CPU6809);

   AddALU('LDD' ,$00cc,1,True ,CPU6809);
   AddALU('STD' ,$00cd,1,False,CPU6809);
   AddALU('CMPD',$1083,1,True ,CPU6809);
   AddALU('ADDD',$00c3,1,True ,CPU6809);
   AddALU('ADCD',$1089,1,True ,CPU6309);
   AddALU('SUBD',$0083,1,True ,CPU6809);
   AddALU('SBCD',$1082,1,True ,CPU6309);
   AddALU('MULD',$118f,1,True ,CPU6309);
   AddALU('DIVD',$118d,1,True ,CPU6309);
   AddALU('ANDD',$1084,1,True ,CPU6309);
   AddALU('ORD' ,$108a,1,True ,CPU6309);
   AddALU('EORD',$1088,1,True ,CPU6309);
   AddALU('BITD',$1085,1,True ,CPU6309);

   AddALU('LDW' ,$1086,1,True ,CPU6309);
   AddALU('STW' ,$1087,1,False,CPU6309);
   AddALU('CMPW',$1081,1,True ,CPU6309);
   AddALU('ADDW',$108b,1,True ,CPU6309);
   AddALU('SUBW',$1080,1,True ,CPU6309);

   AddALU('STQ' ,$10cd,1,True ,CPU6309);
   AddALU('DIVQ',$118e,1,True ,CPU6309);

   AddALU('LDE' ,$1186,0,True ,CPU6309);
   AddALU('STE' ,$1187,0,False,CPU6309);
   AddALU('CMPE',$1181,0,True ,CPU6309);
   AddALU('ADDE',$118b,0,True ,CPU6309);
   AddALU('SUBE',$1180,0,True ,CPU6309);

   AddALU('LDF' ,$11c6,0,True ,CPU6309);
   AddALU('STF' ,$11c7,0,False,CPU6309);
   AddALU('CMPF',$11c1,0,True ,CPU6309);
   AddALU('ADDF',$11cb,0,True ,CPU6309);
   AddALU('SUBF',$11c0,0,True ,CPU6309);

   AddALU('LDX' ,$008e,1,True ,CPU6809);
   AddALU('STX' ,$008f,1,False,CPU6809);
   AddALU('CMPX',$008c,1,True ,CPU6809);

   AddALU('LDY' ,$108e,1,True ,CPU6809);
   AddALU('STY' ,$108f,1,False,CPU6809);
   AddALU('CMPY',$108c,1,True ,CPU6809);

   AddALU('LDU' ,$00ce,1,True ,CPU6809);
   AddALU('STU' ,$00cf,1,False,CPU6809);
   AddALU('CMPU',$1183,1,True ,CPU6809);

   AddALU('LDS' ,$10ce,1,True ,CPU6809);
   AddALU('STS' ,$10cf,1,False,CPU6809);
   AddALU('CMPS',$118c,1,True ,CPU6809);

   AddALU('JSR' ,$008d,1,False,CPU6809);

   New(ALU2Orders);
   ALU2Orders^[0]:='ADD'; ALU2Orders^[1]:='ADC';
   ALU2Orders^[2]:='SUB'; ALU2Orders^[3]:='SBC';
   ALU2Orders^[4]:='AND'; ALU2Orders^[5]:='OR' ;
   ALU2Orders^[6]:='EOR'; ALU2Orders^[7]:='CMP';

   New(RMWOrders); z:=0;
   AddRMW('NEG',$00,CPU6809);
   AddRMW('COM',$03,CPU6809);
   AddRMW('LSR',$04,CPU6809);
   AddRMW('ROR',$06,CPU6809);
   AddRMW('ASR',$07,CPU6809);
   AddRMW('ASL',$08,CPU6809);
   AddRMW('LSL',$08,CPU6809);
   AddRMW('ROL',$09,CPU6809);
   AddRMW('DEC',$0a,CPU6809);
   AddRMW('INC',$0c,CPU6809);
   AddRMW('TST',$0d,CPU6809);
   AddRMW('JMP',$0e,CPU6809);
   AddRMW('CLR',$0f,CPU6809);

   New(FlagOrders); z:=0;
   AddFlag('CWAI' ,$3c,True ,CPU6809);
   AddFlag('ANDCC',$1c,True ,CPU6809);
   AddFlag('ORCC' ,$1a,False,CPU6809);

   New(LEAOrders); z:=0;
   AddLEA('LEAX',$30,CPU6809);
   AddLEA('LEAY',$31,CPU6809);
   AddLEA('LEAS',$32,CPU6809);
   AddLEA('LEAU',$33,CPU6809);

   New(ImmOrders); z:=0;
   AddImm('AIM',$02,CPU6309);
   AddImm('OIM',$01,CPU6309);
   AddImm('EIM',$05,CPU6309);
   AddImm('TIM',$0b,CPU6309);

   New(StackOrders); z:=0;
   AddStack('PSHS',$34,CPU6809);
   AddStack('PULS',$35,CPU6809);
   AddStack('PSHU',$36,CPU6809);
   AddStack('PULU',$37,CPU6809);

   New(BitOrders);
   BitOrders^[0]:='BAND'; BitOrders^[1]:='BIAND';
   BitOrders^[2]:='BOR';  BitOrders^[3]:='BIOR' ;
   BitOrders^[4]:='BEOR'; BitOrders^[5]:='BIEOR';
   BitOrders^[6]:='LDBT'; BitOrders^[7]:='STBT' ;
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(RelOrders);
   Dispose(ALUOrders);
   Dispose(ALU2Orders);
   Dispose(RMWOrders);
   Dispose(FlagOrders);
   Dispose(LEAOrders);
   Dispose(ImmOrders);
   Dispose(StackOrders);
   Dispose(BitOrders);
END;

{---------------------------------------------------------------------------}

	PROCEDURE DecodeAdr;
VAR
   Asc,LAsc,temp:String;
   AdrLong:LongInt;
   AdrWord:Word;
   IndFlag,OK:Boolean;
   EReg,p,ZeroMode:Byte;
   AdrInt:Integer;

	FUNCTION CodeReg(ChIn:String; VAR erg:Byte):Boolean;
CONST
   Regs:String[4]='XYUS';
BEGIN
   IF Length(ChIn)<>1 THEN CodeReg:=False
   ELSE
    BEGIN
     erg:=Pos(UpCase(ChIn[1]),Regs)-1;
     CodeReg:=erg<>$ff;
    END;
END;

	FUNCTION ChkZero(s:String; VAR Erg:Byte):String;
BEGIN
   IF s[1]='>' THEN
    BEGIN
     ChkZero:=Copy(s,2,Length(s)-1); Erg:=1;
    END
   ELSE IF Copy(s,1,2)='<<' THEN
    BEGIN
     ChkZero:=Copy(s,3,Length(s)-2); Erg:=3;
    END
   ELSE IF s[1]='<' THEN
    BEGIN
     ChkZero:=Copy(s,2,Length(s)-1); Erg:=2;
    END
   ELSE
    BEGIN
     ChkZero:=s; Erg:=0;
    END;
END;

	FUNCTION MayShort(Arg:Integer):Boolean;
BEGIN
   MayShort:=(Arg>=-128) AND (Arg<127);
END;

BEGIN
   AdrMode:=ModNone; AdrCnt:=0;
   Asc:=ArgStr[1]; LAsc:=ArgStr[ArgCnt];

   { immediate }

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OpSize OF
     2:BEGIN
        AdrLong:=EvalIntExpression(Asc,Int32,OK);
        IF OK THEN
	 BEGIN
	  AdrVals[0]:=Lo(AdrLong SHR 24);
	  AdrVals[1]:=Lo(AdrLong SHR 16);
	  AdrVals[2]:=Lo(AdrLong SHR  8);
	  AdrVals[3]:=Lo(AdrLong);
	  AdrCnt:=4;
	 END;
       END;
     1:BEGIN
        AdrWord:=EvalIntExpression(Asc,Int16,OK);
        IF OK THEN
	 BEGIN
	  AdrVals[0]:=Hi(AdrWord); AdrVals[1]:=Lo(AdrWord);
	  AdrCnt:=2;
	 END;
       END;
     0:BEGIN
        AdrVals[0]:=EvalIntExpression(Asc,Int8,OK);
        IF OK THEN AdrCnt:=1;
       END;
     END;
     IF OK THEN AdrMode:=ModImm;
     Exit;
    END;

   { indirekter Ausdruck ? }

   IF (Asc[1]='[') AND (Asc[Length(Asc)]=']') THEN
    BEGIN
     IndFlag:=True; Asc:=Copy(Asc,2,Length(Asc)-2);
     ArgCnt:=0;
     WHILE Asc<>'' DO
      BEGIN
       Inc(ArgCnt);
       p:=QuotPos(Asc,',');
       IF p<=Length(Asc) THEN
	BEGIN
	 SplitString(Asc,ArgStr[ArgCnt],temp,p); Asc:=temp;
	END
       ELSE
	BEGIN
	 ArgStr[ArgCnt]:=Asc; Asc:='';
	END;
      END;
     Asc:=ArgStr[1]; LAsc:=ArgStr[ArgCnt];
    END
   ELSE IndFlag:=False;

   { Predekrement ? }

   IF (ArgCnt>=1) AND (ArgCnt<=2) AND (Length(LAsc)=2) AND (LAsc[1]='-') AND (CodeReg(LAsc[2],EReg)) THEN
    BEGIN
     IF (ArgCnt=2) AND (Asc<>'') THEN WrError(1350)
     ELSE
      BEGIN
       AdrCnt:=1; AdrVals[0]:=$82+(EReg SHL 5)+(Ord(IndFlag) SHL 4);
       AdrMode:=ModInd;
      END;
     Exit;
    END;

   IF (ArgCnt>=1) AND (ArgCnt<=2) AND (Length(LAsc)=3) AND (Copy(LAsc,1,2)='--') AND (CodeReg(LAsc[3],EReg)) THEN
    BEGIN
     IF (ArgCnt=2) AND (Asc<>'') THEN WrError(1350)
     ELSE
      BEGIN
       AdrCnt:=1; AdrVals[0]:=$83+(EReg SHL 5)+(Ord(IndFlag) SHL 4);
       AdrMode:=ModInd;
      END;
     Exit;
    END;

   IF (ArgCnt>=1) AND (ArgCnt<=2) AND (NLS_StrCaseCmp(LAsc,'--W')=0) THEN
    BEGIN
     IF (ArgCnt=2) AND (Asc<>'') THEN WrError(1350)
     ELSE IF MomCPU<CPU6309 THEN WrError(1505)
     ELSE
      BEGIN
       AdrCnt:=1; AdrVals[0]:=$ef+Ord(IndFlag);
       AdrMode:=ModInd;
      END;
     Exit;
    END;

   { Postinkrement ? }

   IF (ArgCnt>=1) AND (ArgCnt<=2) AND (Length(LAsc)=2) AND (LAsc[2]='+') AND (CodeReg(LAsc[1],EReg)) THEN
    BEGIN
     IF (ArgCnt=2) AND (Asc<>'') THEN WrError(1350)
     ELSE
      BEGIN
       AdrCnt:=1; AdrVals[0]:=$80+(EReg SHL 5)+(Ord(IndFlag) SHL 4);
       AdrMode:=ModInd;
      END;
     Exit;
    END;

   IF (ArgCnt>=1) AND (ArgCnt<=2) AND (Length(LAsc)=3) AND (Copy(LAsc,2,2)='++') AND (CodeReg(LAsc[1],EReg)) THEN
    BEGIN
     IF (ArgCnt=2) AND (Asc<>'') THEN WrError(1350)
     ELSE
      BEGIN
       AdrCnt:=1; AdrVals[0]:=$81+(EReg SHL 5)+(Ord(IndFlag) SHL 4);
       AdrMode:=ModInd;
      END;
     Exit;
    END;

   IF (ArgCnt>=1) AND (ArgCnt<=2) AND (NLS_StrCaseCmp(LAsc,'W++')=0) THEN
    BEGIN
     IF (ArgCnt=2) AND (Asc<>'') THEN WrError(1350)
     ELSE IF MomCPU<CPU6309 THEN WrError(1505)
     ELSE
      BEGIN
       AdrCnt:=1; AdrVals[0]:=$cf+Ord(IndFlag);
       AdrMode:=ModInd;
      END;
     Exit;
    END;

   { 16-Bit-Register (mit Index) ? }

   IF (ArgCnt<=2) AND (ArgCnt>=1) AND (CodeReg(ArgStr[ArgCnt],EReg)) THEN
    BEGIN
     AdrVals[0]:=(EReg SHL 5)+(Ord(IndFlag) SHL 4);

     { nur 16-Bit-Register }

     IF ArgCnt=1 THEN
      BEGIN
       AdrCnt:=1; Inc(AdrVals[0],$84);
       AdrMode:=ModInd; Exit;
      END;

     { mit Index }

     IF NLS_StrCaseCmp(Asc,'A')=0 THEN
      BEGIN
       AdrCnt:=1; Inc(AdrVals[0],$86);
       AdrMode:=ModInd; Exit;
      END;
     IF NLS_StrCaseCmp(Asc,'B')=0 THEN
      BEGIN
       AdrCnt:=1; Inc(AdrVals[0],$85);
       AdrMode:=ModInd; Exit;
      END;
     IF NLS_StrCaseCmp(Asc,'D')=0 THEN
      BEGIN
       AdrCnt:=1; Inc(AdrVals[0],$8b);
       AdrMode:=ModInd; Exit;
      END;
     IF (NLS_StrCaseCmp(Asc,'E')=0) AND (MomCPU>=CPU6309) THEN
      BEGIN
       IF EReg<>0 THEN WrError(1350)
       ELSE
        BEGIN
         AdrCnt:=1; Inc(AdrVals[0],$87); AdrMode:=ModInd;
        END;
       Exit;
      END;
     IF (NLS_StrCaseCmp(Asc,'F')=0) AND (MomCPU>=CPU6309) THEN
      BEGIN
       IF EReg<>0 THEN WrError(1350)
       ELSE
        BEGIN
         AdrCnt:=1; Inc(AdrVals[0],$8a); AdrMode:=ModInd;
        END;
       Exit;
      END;
     IF (NLS_StrCaseCmp(Asc,'W')=0) AND (MomCPU>=CPU6309) THEN
      BEGIN
       IF EReg<>0 THEN WrError(1350)
       ELSE
        BEGIN
         AdrCnt:=1; Inc(AdrVals[0],$8e); AdrMode:=ModInd;
        END;
       Exit;
      END;

     { Displacement auswerten }

     Asc:=ChkZero(Asc,ZeroMode);
     IF ZeroMode>1 THEN
      BEGIN
       AdrInt:=EvalIntExpression(Asc,Int8,OK);
       IF (FirstPassUnknown) AND (ZeroMode=3) THEN AdrInt:=AdrInt AND $0f;
      END
     ELSE
      AdrInt:=EvalIntExpression(Asc,Int16,OK);

     { Displacement 0 ? }

     IF (ZeroMode=0) AND (AdrInt=0) THEN
      BEGIN
       AdrCnt:=1; Inc(AdrVals[0],$84);
       AdrMode:=ModInd; Exit;
      END

     { 5-Bit-Displacement }

     ELSE IF (ZeroMode=3) OR ((ZeroMode=0) AND (NOT IndFlag) AND (AdrInt>=-16) AND (AdrInt<=15)) THEN
      BEGIN
       IF (AdrInt<-16) OR (AdrInt>15) THEN WrError(1340)
       ELSE IF (IndFlag) THEN WrError(1350)
       ELSE
	BEGIN
	 AdrMode:=ModInd;
	 AdrCnt:=1; Inc(AdrVals[0],AdrInt AND $1f);
	END;
       Exit;
      END

     { 8-Bit-Displacement }

     ELSE IF (ZeroMode=2) OR ((ZeroMode=0) AND (MayShort(AdrInt))) THEN
      BEGIN
       IF NOT MayShort(AdrInt) THEN WrError(1340)
       ELSE
	BEGIN
	 AdrMode:=ModInd;
	 AdrCnt:=2; Inc(AdrVals[0],$88); AdrVals[1]:=Lo(AdrInt);
	END;
       Exit;
      END

     { 16-Bit-Displacement }

     ELSE
      BEGIN
       AdrMode:=ModInd;
       AdrCnt:=3; Inc(AdrVals[0],$89);
       AdrVals[1]:=Hi(AdrInt); AdrVals[2]:=Lo(AdrInt);
       Exit;
      END;
    END;

   IF (ArgCnt<=2) AND (ArgCnt>=1) AND (MomCPU>=CPU6309) AND (NLS_StrCaseCmp(ArgStr[ArgCnt],'W')=0) THEN
    BEGIN
     AdrVals[0]:=$8f+Ord(IndFlag);

     { nur W-Register }

     IF ArgCnt=1 THEN
      BEGIN
       AdrCnt:=1; AdrMode:=ModInd; Exit;
      END;

     { Displacement auswerten }

     Asc:=ChkZero(Asc,ZeroMode);
     AdrInt:=EvalIntExpression(Asc,Int16,OK);

     { Displacement 0 ? }

     IF (ZeroMode=0) AND (AdrInt=0) THEN
      BEGIN
       AdrCnt:=1; AdrMode:=ModInd; Exit;
      END

     { 16-Bit-Displacement }

     ELSE
      BEGIN
       AdrMode:=ModInd;
       AdrCnt:=3; Inc(AdrVals[0],$20);
       AdrVals[1]:=Hi(AdrInt); AdrVals[2]:=Lo(AdrInt);
       Exit;
      END;
    END;

   { PC-relativ ? }

   IF (ArgCnt=2) AND ((NLS_StrCaseCmp(ArgStr[2],'PCR')=0) OR (NLS_StrCaseCmp(ArgStr[2],'PC')=0)) THEN
    BEGIN
     AdrVals[0]:=Ord(IndFlag) SHL 4;
     Asc:=ChkZero(Asc,ZeroMode);
     AdrInt:=EvalIntExpression(Asc,Int16,OK);
     IF OK THEN
      BEGIN
       Dec(AdrInt,EProgCounter+3+Ord(ExtFlag));

       IF ZeroMode=3 THEN WrError(1350)

       ELSE IF (ZeroMode=2) OR ((ZeroMode=0) AND MayShort(AdrInt)) THEN
	BEGIN
	 IF NOT MayShort(AdrInt) THEN WrError(1320)
	 ELSE
	  BEGIN
	   AdrCnt:=2; Inc(AdrVals[0],$8c);
	   AdrVals[1]:=Lo(AdrInt);
	   AdrMode:=ModInd;
	  END;
	END

       ELSE
	BEGIN
	 Dec(AdrInt);
	 AdrCnt:=3; Inc(AdrVals[0],$8d);
	 AdrVals[1]:=Hi(AdrInt); AdrVals[2]:=Lo(AdrInt);
	 AdrMode:=ModInd;
	END;
      END;
     Exit;
    END;

   IF (ArgCnt=1) THEN
    BEGIN
     Asc:=ChkZero(Asc,ZeroMode);
     FirstPassUnknown:=False;
     AdrInt:=EvalIntExpression(Asc,Int16,OK);
     IF (FirstPassUnknown) AND (ZeroMode=2) THEN
      AdrInt:=(AdrInt AND $ff) OR (DPRValue SHL 8);

     IF OK THEN
      BEGIN
       IF (ZeroMode=3) THEN WrError(1350)

       ELSE IF (ZeroMode=2) OR ((ZeroMode=0) AND (Hi(AdrInt)=DPRValue) AND (NOT IndFlag)) THEN
	BEGIN
	 IF IndFlag THEN WrError(1990)
         ELSE IF Hi(AdrInt)<>DPRValue THEN WrError(1340)
	 ELSE
	  BEGIN
	   AdrCnt:=1; AdrMode:=ModDir; AdrVals[0]:=Lo(AdrInt);
	  END;
	END

       ELSE
	BEGIN
	 IF IndFlag THEN
	  BEGIN
	   AdrMode:=ModInd; AdrCnt:=3; AdrVals[0]:=$9f;
	   AdrVals[1]:=Hi(AdrInt); AdrVals[2]:=Lo(AdrInt);
	  END
	 ELSE
	  BEGIN
	   AdrMode:=ModExt; AdrCnt:=2;
	   AdrVals[0]:=Hi(AdrInt); AdrVals[1]:=Lo(AdrInt);
	  END;
	END;
      END;
     Exit;
    END;

   IF AdrMode=ModNone THEN WrError(1350);
END;

        FUNCTION CodeCPUReg(Asc:String; VAR Erg:Byte):Boolean;
CONST
   RegCnt=18;
   RegNames:ARRAY[1..RegCnt] OF String[3]=('D','X','Y','U','S','SP','PC','W','V','A','B','CCR','DPR','CC','DP','Z','E','F');
   RegVals :ARRAY[1..RegCnt] OF Byte     =(0  ,1  ,2  ,3  ,4  ,4   ,5   ,6  ,7  ,8  ,9  ,10   ,11   ,10  ,11  ,13 ,14 ,15 );
VAR
   z:Integer;
BEGIN
   CodeCPUReg:=False; NLS_UpString(Asc);
   FOR z:= 1 TO RegCnt DO
    IF Asc=RegNames[z] THEN
     IF (RegVals[z] AND 6=6) AND (MomCPU<CPU6309) THEN WrError(1505)
     ELSE
      BEGIN
       Erg:=RegVals[z]; CodeCPUReg:=True; Exit;
      END;
END;

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUME09Count=1;
   ASSUME09s:ARRAY[1..ASSUME09Count] OF ASSUMERec=
             ((Name:'DPR'; Dest:@DPRValue; Min:0; Max:$ff; NothingVal:$100));
BEGIN
   DecodePseudo:=True;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME09s,ASSUME09Count);
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE MakeCode_6809;
	Far;
VAR
   z,z2,z3,AdrInt:Integer;
   LongFlag,OK,RegFnd,Extent:Boolean;

	PROCEDURE SplitPM(VAR s:String; VAR Erg:Integer);
BEGIN
   IF Length(s)=0 THEN Erg:=0
   ELSE IF s[Length(s)]='+' THEN
    BEGIN
     Delete(s,Length(s),1); Erg:=1;
    END
   ELSE IF s[Length(s)]='-' THEN
    BEGIN
     Delete(s,Length(s),1); Erg:=-1;
    END
   ELSE Erg:=0;
END;

	FUNCTION SplitBit(VAR Asc:String; VAR Erg:Integer):Boolean;
VAR
   p:Integer;
   OK:Boolean;
BEGIN
   SplitBit:=False;
   p:=QuotPos(Asc,'.');
   IF p=0 THEN
    BEGIN
     WrError(1510); Exit;
    END;
   Erg:=EvalIntExpression(Copy(Asc,p+1,Length(Asc)-p),UInt3,OK);
   IF NOT OK THEN Exit;
   Delete(Asc,p,Length(Asc)-p+1);
   SplitBit:=True;
END;

BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=0; ExtFlag:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeMotoPseudo(True) THEN Exit;

   { Anweisungen ohne Argument }

   FOR z:=1 TO FixedOrderCnt DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE IF Hi(Code)=0 THEN
	BEGIN
	 BAsmCode[0]:=Lo(Code); CodeLen:=1;
	END
       ELSE
	BEGIN
	 WAsmCode[0]:=Swap(Code); CodeLen:=2;
	END;
       Exit;
      END;

   { Specials... }

   IF Memo('SWI') THEN
    BEGIN
     IF ArgCnt=0 THEN
      BEGIN
       BAsmCode[0]:=$3f; CodeLen:=1;
      END
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'2')=0 THEN
      BEGIN
       BAsmCode[0]:=$10; BAsmCode[1]:=$3f; CodeLen:=2;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'3')=0 THEN
      BEGIN
       BAsmCode[0]:=$11; BAsmCode[1]:=$3f; CodeLen:=2;
      END
     ELSE WrError(1135);
     Exit;
    END;

   { relative SprÅnge }

   FOR z:=1 TO RelOrderCnt DO
    WITH RelOrders^[z] DO
     IF (Memo(Name)) OR ((OpPart[1]='L') AND (Name=Copy(OpPart,2,Length(OpPart)-1))) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
	BEGIN
	 LongFlag:=OpPart[1]='L'; ExtFlag:=(LongFlag) AND (Hi(Code16)<>0);
	 AdrInt:=EvalIntExpression(ArgStr[1],Int16,OK);
	 IF OK THEN
	  BEGIN
	   IF LongFlag THEN Dec(AdrInt,EProgCounter+3+Ord(ExtFlag))
	   ELSE Dec(AdrInt,EProgCounter+2+Ord(ExtFlag));
	   IF (NOT LongFlag) AND ((AdrInt<-128) OR (AdrInt>127)) THEN WrError(1370)
	   ELSE
	    BEGIN
	     CodeLen:=1+Ord(ExtFlag);
	     IF LongFlag THEN
	      IF ExtFlag THEN
	       BEGIN
                BAsmCode[0]:=Hi(Code16); BAsmCode[1]:=Lo(Code16);
	       END
	      ELSE BAsmCode[0]:=Lo(Code16)
	     ELSE BAsmCode[0]:=Lo(Code8);
	     IF LongFlag THEN
	      BEGIN
	       BAsmCode[CodeLen]:=Hi(AdrInt); BAsmCode[CodeLen+1]:=Lo(AdrInt);
	       Inc(CodeLen,2);
	      END
	     ELSE
	      BEGIN
	       BAsmCode[CodeLen]:=Lo(AdrInt);
	       Inc(CodeLen);
	      END;
	    END;
	  END;
	END;
       Exit;
      END;

   { ALU-Operationen }

   FOR z:=1 TO ALUOrderCnt DO
    WITH ALUOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 OpSize:=Op16; ExtFlag:=Hi(Code)<>0;
	 DecodeAdr;
	 IF AdrMode<>ModNone THEN
          IF (NOT MayImm) AND (AdrMode=ModImm) THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=Ord(ExtFlag)+1+AdrCnt;
	    IF ExtFlag THEN WAsmCode[0]:=Swap(Code) ELSE BAsmCode[0]:=Lo(Code);
	    Inc(BAsmCode[Ord(ExtFlag)],(AdrMode-1) SHL 4);
	    Move(AdrVals,BAsmCode[Ord(ExtFlag)+1],AdrCnt);
	   END;
	END;
       Exit;
      END;

    IF Memo('LDQ') THEN
     BEGIN
      IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
      ELSE IF MomCPU<CPU6309 THEN WrError(1500)
      ELSE
       BEGIN
	OpSize:=2;
	DecodeAdr;
	IF AdrMode=ModImm THEN
         BEGIN
          BAsmCode[0]:=$cd; Move(AdrVals,BAsmCode[1],AdrCnt);
          CodeLen:=1+AdrCnt;
         END
	ELSE
	 BEGIN
          BAsmCode[0]:=$10; BAsmCode[1]:=$cc+((AdrMode-1) SHL 4);
	  CodeLen:=2+AdrCnt;
	  Move(AdrVals,BAsmCode[2],AdrCnt);
	 END;
       END;
      Exit;
     END;

   { Read-Modify-Write-Operationen }

   FOR z:=1 TO RMWOrderCnt DO
    WITH RMWOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 DecodeAdr;
	 IF AdrMode<>ModNone THEN
          IF AdrMode=ModImm THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=1+AdrCnt;
	    CASE AdrMode OF
	    ModDir:BAsmCode[0]:=Code;
	    ModInd:BAsmCode[0]:=Code+$60;
	    ModExt:BAsmCode[0]:=Code+$70;
	    END;
	    Move(AdrVals,BAsmCode[1],AdrCnt);
	   END;
	END;
       Exit;
      END;

   { Anweisungen mit Flag-Operand }

   FOR z:=1 TO FlagOrderCnt DO
    WITH FlagOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<1 THEN WrError(1110)
       ELSE
        BEGIN
         OK:=True;
         IF Inv THEN BAsmCode[1]:=$ff ELSE BAsmCode[1]:=$00;
         FOR z2:=1 TO ArgCnt DO
          IF Ok THEN
           BEGIN
            z3:=Pos(UpCase(ArgStr[z2][1]),FlagChars)-1;
            IF z3<>-1 THEN
             IF Inv THEN BAsmCode[1]:=BAsmCode[1] AND (NOT (1 SHL z3))
             ELSE BAsmCode[1]:=BAsmCode[1] OR (1 SHL z3)
            ELSE IF ArgStr[z2][1]<>'#' THEN
             BEGIN
              WrError(1120); OK:=False;
             END
            ELSE
             BEGIN
   	      BAsmCode[2]:=EvalIntExpression(Copy(ArgStr[z2],2,Length(ArgStr[z2])-1),Int8,OK);
              IF OK THEN
               IF Inv THEN BAsmCode[1]:=BAsmCode[1] AND BAsmCode[2]
               ELSE BAsmCode[1]:=BAsmCode[1] OR BAsmCode[2];
             END;
           END;
	 IF OK THEN
	  BEGIN
	   CodeLen:=2; BAsmCode[0]:=Code;
	  END;
        END;
       Exit;
      END;

   { Bit-Befehle }

   FOR z:=1 TO ImmOrderCnt DO
    WITH ImmOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<>2) AND (ArgCnt<>3) THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE IF ArgStr[1][1]<>'#' THEN WrError(1120)
       ELSE
        BEGIN
         BAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),Int8,OK);
         IF OK THEN
         BEGIN
          FOR z2:=1 TO Pred(ArgCnt) DO ArgStr[z2]:=ArgStr[z2+1];
          Dec(ArgCnt); DecodeAdr;
          IF AdrMode=ModImm THEN WrError(1350)
          ELSE
           BEGIN
            CASE AdrMode OF
	    ModDir:BAsmCode[0]:=Code;
	    ModExt:BAsmCode[0]:=Code+$70;
	    ModInd:BAsmCode[0]:=Code+$60;
            END;
            Move(AdrVals,BAsmCode[2],AdrCnt);
            CodeLen:=2+AdrCnt;
           END;
         END;
        END;
       Exit;
      END;

   FOR z:=0 TO BitOrderCnt-1 DO
    IF Memo(BitOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF MomCPU<CPU6309 THEN WrError(1500)
      ELSE
       IF SplitBit(ArgStr[1],z2) THEN
        IF SplitBit(ArgStr[2],z3) THEN
         IF NOT CodeCPUReg(ArgStr[1],BAsmCode[2]) THEN WrError(1980)
         ELSE IF (BAsmCode[2]<8) OR (BAsmCode[2]>11) THEN WrError(1980)
         ELSE
          BEGIN
           ArgStr[1]:=ArgStr[2]; ArgCnt:=1; DecodeAdr;
           IF AdrMode<>ModDir THEN WrError(1350)
           ELSE
            BEGIN
             Dec(BAsmCode[2],7);
	     IF BAsmCode[2]=3 THEN BAsmCode[2]:=0;
             BAsmCode[0]:=$11; BAsmCode[1]:=$30+z;
             BAsmCode[2]:=(BAsmCode[2] SHL 6)+(z3 SHL 3)+z2;
             BAsmCode[3]:=AdrVals[0];
             CodeLen:=4;
            END;
          END;
      Exit;
     END;

   { Register-Register-Operationen }

   IF (Memo('TFR')) OR (Memo('TFM')) OR (Memo('EXG')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       SplitPM(ArgStr[1],z2); SplitPM(ArgStr[2],z3);
       IF (z2<>0) OR (z3<>0) THEN
        BEGIN
         IF Memo('EXG') THEN WrError(1350)
         ELSE IF NOT CodeCPUReg(ArgStr[1],BAsmCode[3]) THEN WrError(1980)
         ELSE IF NOT CodeCPUReg(ArgStr[2],BAsmCode[2]) THEN WrError(1980)
         ELSE IF (BAsmCode[2]<1) OR (BAsmCode[2]>4) THEN WrError(1980)
         ELSE IF (BAsmCode[3]<1) OR (BAsmCode[3]>4) THEN WrError(1980)
         ELSE
          BEGIN
           BAsmCode[0]:=$11; BAsmCode[1]:=0;
	   Inc(BAsmCode[2],BAsmCode[3] SHL 4);
           IF (z2=1) AND (z3=1) THEN BAsmCode[1]:=$38
           ELSE IF (z2=-1) AND (z3=-1) THEN BAsmCode[1]:=$39
           ELSE IF (z2= 1) AND (z3= 0) THEN BAsmCode[1]:=$3a
           ELSE IF (z2= 0) AND (z3= 1) THEN BAsmCode[1]:=$3b;
           IF BAsmCode[1]=0 THEN WrError(1350) ELSE CodeLen:=3;
          END;
        END
       ELSE IF Memo('TFM') THEN WrError(1350)
       ELSE IF NOT CodeCPUReg(ArgStr[1],BAsmCode[2]) THEN WrError(1980)
       ELSE IF NOT CodeCPUReg(ArgStr[2],BAsmCode[1]) THEN WrError(1980)
       ELSE IF (BAsmCode[1]<>13) AND (BAsmCode[2]<>13) AND  { Z-Register mit allen kompatibel }
               ((BAsmCode[1] XOR BAsmCode[2]) AND $08<>0) THEN WrError(1131)
       ELSE
        BEGIN
         CodeLen:=2;
         BAsmCode[0]:=$1e+Ord(Memo('TFR'));
         Inc(BAsmCode[1],BAsmCode[2] SHL 4);
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO ALU2OrderCount-1 DO
    IF (Memo(ALU2Orders^[z])) OR (Memo(ALU2Orders^[z]+'R')) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF NOT CodeCPUReg(ArgStr[1],BAsmCode[3]) THEN WrError(1980)
      ELSE IF NOT CodeCPUReg(ArgStr[2],BAsmCode[2]) THEN WrError(1980)
      ELSE IF (BAsmCode[1]<>13) AND (BAsmCode[2]<>13) AND  { Z-Register mit allen kompatibel }
              ((BAsmCode[2] XOR BAsmCode[3]) AND $08<>0) THEN WrError(1131)
      ELSE
       BEGIN
        CodeLen:=3;
        BAsmCode[0]:=$10;
	BAsmCode[1]:=$30+z;
        Inc(BAsmCode[2],BAsmCode[3] SHL 4);
       END;
      Exit;
     END;

   { Berechnung effektiver Adressen }

   FOR z:=1 TO LEAOrderCnt DO
    WITH LEAOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF (ArgCnt<1) OR (ArgCnt>2) THEN WrError(1110)
       ELSE
	BEGIN
	 DecodeAdr;
	 IF AdrMode<>ModNone THEN
          IF AdrMode<>ModInd THEN WrError(1350)
	  ELSE
	   BEGIN
	    CodeLen:=1+AdrCnt;
	    BAsmCode[0]:=Code;
	    Move(AdrVals,BAsmCode[1],AdrCnt)
	   END;
	END;
       Exit;
      END;

   { Push/Pull }

   FOR z:=1 TO StackOrderCnt DO
    WITH StackOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       BAsmCode[1]:=0; OK:=True; Extent:=False;
       { S oder U einsetzen, entsprechend Opcode }
       StackRegNames[StackRegCnt-1]:=
	Chr(Ord(OpPart[Length(OpPart)]) XOR Ord('S') XOR Ord('U'));
       FOR z2:=1 TO ArgCnt DO
	IF OK THEN
	 BEGIN
          IF NLS_StrCaseCmp(ArgStr[z2],'W')=0 THEN
           BEGIN
            IF MomCPU<CPU6309 THEN
             BEGIN
              WrError(1500); OK:=False;
             END
            ELSE IF ArgCnt<>1 THEN
             BEGIN
              WrError(1335); OK:=False;
             END
            ELSE Extent:=True;
           END
          ELSE
           BEGIN
	    RegFnd:=False;
	    FOR z3:=0 TO StackRegCnt-1 DO
             IF NLS_StrCaseCmp(ArgStr[z2],StackRegNames[z3])=0 THEN
	      BEGIN
	       RegFnd:=True; BAsmCode[1]:=BAsmCode[1] OR (1 SHL StackRegCodes[z3]);
	      END;
	    IF NOT RegFnd THEN
             IF NLS_StrCaseCmp(ArgStr[z2],'ALL')=0 THEN BAsmCode[1]:=$ff
	     ELSE IF ArgStr[z2][1]<>'#' THEN OK:=False
             ELSE
	      BEGIN
               BAsmCode[2]:=EvalIntExpression(Copy(ArgStr[z2],2,Length(ArgStr[z2])-1),Int8,OK);
               IF OK THEN BAsmCode[1]:=BAsmCode[1] OR BAsmCode[2];
	      END;
           END;
	 END;
       IF OK THEN
        IF Extent THEN
         BEGIN
          CodeLen:=2; BAsmCOde[0]:=$10; BAsmCode[1]:=Code+4;
         END
        ELSE
	 BEGIN
	  CodeLen:=2; BAsmCode[0]:=Code;
	 END
       ELSE WrError(1980);
       Exit;
      END;

   IF (Memo('BITMD')) OR (Memo('LDMD')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<CPU6309 THEN WrError(1500)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1120)
     ELSE
      BEGIN
       BAsmCode[2]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),Int8,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$11;
         BAsmCode[1]:=$3c+Ord(Memo('LDMD'));
         CodeLen:=3;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        PROCEDURE InitCode_6809;
        Far;
BEGIN
   SaveInitProc;
   DPRValue:=0;
END;

	FUNCTION ChkPC_6809:Boolean;
	Far;
BEGIN
   ChkPC_6809:=(ActPC=SegCode) AND (ProgCounter<$10000);
END;

	FUNCTION IsDef_6809:Boolean;
	Far;
BEGIN
   IsDef_6809:=False;
END;

        PROCEDURE SwitchFrom_6809;
        Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_6809;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$63; NOPCode:=$9d;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_6809; ChkPC:=ChkPC_6809; IsDef:=IsDef_6809;

   SwitchFrom:=SwitchFrom_6809; InitFields;
END;

BEGIN
   CPU6809:=AddCPU('6809',SwitchTo_6809);
   CPU6309:=AddCPU('6309',SwitchTo_6809);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_6809;
END.
