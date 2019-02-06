{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT Code75K0;

{ AS Codegeneratormodul uPD75K0-Familie }

INTERFACE
        Uses StringUt,NLS,
	     AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

CONST
   ModNone=-1;
   ModReg4=0;     MModReg4=1 SHL ModReg4;
   ModReg8=1;     MModReg8=1 SHL ModReg8;
   ModImm=2;      MModImm=1 SHL ModImm;
   ModInd=3;	  MModInd=1 SHL ModInd;
   ModAbs=4;	  MModAbs=1 SHL ModAbs;

   FixedOrderCount=6;
   AriOrderCount=3;
   LogOrderCount=3;

TYPE
   FixedOrder=RECORD
               Name:String[4];
               Code:Word;
              END;

   FixedOrderArray = ARRAY[1..FixedOrderCount] OF FixedOrder;
   AriOrderArray   = ARRAY[1..AriOrderCount]   OF String[4];
   LogOrderArray   = ARRAY[1..LogOrderCount]   OF String[3];

VAR
   SaveInitProc:PROCEDURE;

   FixedOrders:^FixedOrderArray;
   AriOrders:^AriOrderArray;
   LogOrders:^LogOrderArray;

   MBSValue,MBEValue:LongInt;
   MinOneIs0:Boolean;

   CPU75402,CPU75004,CPU75006,CPU75008,
   CPU75268,CPU75304,CPU75306,CPU75308,
   CPU75312,CPU75316,CPU75328,CPU75104,
   CPU75106,CPU75108,CPU75112,CPU75116,
   CPU75206,CPU75208,CPU75212,CPU75216,
   CPU75512,CPU75516:CPUVar;
   ROMEnd:Word;

   OpSize:ShortInt;
   AdrPart:Byte;
   AdrMode:ShortInt;

{---------------------------------------------------------------------------}
{ dynamische Codetabellenverwaltung }

	PROCEDURE InitFields;
VAR
   z:Integer;
   Err:ValErgType;

   	PROCEDURE AddFixed(NewName:String; NewCode:Word);
BEGIN
   IF z=FixedOrderCount THEN Halt;
   Inc(z);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NewName; Code:=NewCode;
    END;
END;

BEGIN
   Val(Copy(MomCPUName,4,2),ROMEnd,Err);
   IF ROMEnd>2 THEN ROMEnd:=ROMEnd MOD 10;
   ROMEnd:=(ROMEnd SHL 10)-1;

   New(FixedOrders); z:=0;
   AddFixed('RET' ,$00ee);
   AddFixed('RETS',$00e0);
   AddFixed('RETI',$00ef);
   AddFixed('HALT',$a39d);
   AddFixed('STOP',$b39d);
   AddFixed('NOP' ,$0060);

   New(AriOrders);
   AriOrders^[1]:='ADDC';
   AriOrders^[2]:='SUBS';
   AriOrders^[3]:='SUBC';

   New(LogOrders);
   LogOrders^[1]:='AND';
   LogOrders^[2]:='OR';
   LogOrders^[3]:='XOR';
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(AriOrders);
   Dispose(LogOrders);
END;

{---------------------------------------------------------------------------}
{ Untermengen von Befehlssatz abprÅfen }

	PROCEDURE CheckCPU(MinCPU:CPUVar);
BEGIN
   IF MomCPU<MinCPU THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    END;
END;

{---------------------------------------------------------------------------}
{ Adre·ausdruck parsen }

	FUNCTION SetOpSize(NewSize:ShortInt):Boolean;
BEGIN
   SetOpSize:=True;
   IF OpSize=-1 THEN OpSize:=NewSize
   ELSE IF NewSize<>OpSize THEN
    BEGIN
     WrError(1131); SetOpSize:=False;
    END;
END;

	PROCEDURE ChkDataPage(Adr:Word);
BEGIN
   CASE MBEValue OF
   0:IF (Adr>$7f) AND (Adr<$f80) THEN WrError(110);
   1:IF Hi(Adr)<>MBSValue THEN WrError(110);
   END;
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:Byte);
LABEL
   Found;
CONST
   RegNames='XAHLDEBC';
VAR
   p:Integer;
   OK:Boolean;
   s:String;
BEGIN
   AdrMode:=ModNone;

   { Register ? }

   s:=Copy(Asc,1,2); NLS_UpString(s);
   p:=Pos(s,RegNames);

   IF p<>0 THEN
    BEGIN
     Dec(p);

     { 8-Bit-Register ? }

     IF Length(Asc)=1 THEN
      BEGIN
       AdrPart:=p XOR 1;
       IF SetOpSize(0) THEN
        IF (AdrPart>4) AND (MomCPU<CPU75004) THEN WrError(1505)
	ELSE AdrMode:=ModReg4;
       Goto Found;
      END;

     { 16-Bit-Register ? }

     IF (Length(Asc)=2) AND (NOT Odd(p)) THEN
      BEGIN
       AdrPart:=p;
       IF SetOpSize(1) THEN
        IF (AdrPart>2) AND (MomCPU<CPU75004) THEN WrError(1505)
	ELSE AdrMode:=ModReg8;
       Goto Found;
      END;

     { 16-Bit-Schattenregister ? }

     IF (Length(Asc)=3) AND ((Asc[3]='''') OR (Asc[3]='`')) AND (NOT Odd(p)) THEN
      BEGIN
       AdrPart:=p+1;
       IF SetOpSize(1) THEN
        IF MomCPU<CPU75104 THEN WrError(1505) ELSE AdrMode:=ModReg8;
       Goto Found;
      END;
    END;

   { immediate? }

   IF Asc[1]='#' THEN
    BEGIN
     IF (OpSize=-1) AND (MinOneIs0) THEN IF SetOpSize(0) THEN;
     Delete(Asc,1,1); FirstPassUnknown:=False;
     CASE OpSize OF
     -1:WrError(1132);
     0:AdrPart:=EvalIntExpression(Asc,Int4,OK) AND 15;
     1:AdrPart:=EvalIntExpression(Asc,Int8,OK);
     END;
     IF Ok THEN AdrMode:=ModImm;
     Goto Found;
    END;

   { indirekt ? }

   IF Asc[1]='@' THEN
    BEGIN
     s:=Copy(Asc,2,Length(Asc)-1); NLS_UpString(s);
     IF s='HL' THEN AdrPart:=1
     ELSE IF s='HL+' THEN AdrPart:=2
     ELSE IF s='HL-' THEN AdrPart:=3
     ELSE IF s='DE' THEN AdrPart:=4
     ELSE IF s='DL' THEN AdrPart:=5
     ELSE AdrPart:=0;
     IF AdrPart<>0 THEN
      BEGIN
       IF (MomCPU<CPU75004) AND (AdrPart<>1) THEN WrError(1505)
       ELSE IF (MomCPU<CPU75104) AND ((AdrPart=2) OR (AdrPart=3)) THEN WrError(1505)
       ELSE AdrMode:=ModInd;
       Goto Found;
      END;
    END;

   { absolut }

   FirstPassUnknown:=False;
   p:=EvalIntExpression(Asc,UInt12,OK);
   IF OK THEN
    BEGIN
     AdrPart:=Lo(p); AdrMode:=ModAbs;
     ChkSpace(SegData);
     IF NOT FirstPassUnknown THEN ChkDataPage(p);
    END;

Found:
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     WrError(1350); AdrMode:=ModNone;
    END;
END;

VAR
   BName:String;

	FUNCTION DecodeBitAddr(Asc:String; VAR Erg:Word):Boolean;
VAR
   p:Byte;
   OK:Boolean;
   Adr:Word;
   bpart:String;
BEGIN
   DecodeBitAddr:=False;

   p:=QuotPos(Asc,'.');
   IF p>Length(Asc) THEN
    BEGIN
     Erg:=EvalIntExpression(Asc,Int16,OK); DecodeBitAddr:=OK;
     IF Hi(Erg)<>0 THEN ChkDataPage(((Erg SHR 4) AND $f00)+Lo(Erg));
     Exit;
    END;

   bpart:=Copy(Asc,p+1,Length(Asc)-p); Delete(Asc,p,Length(Asc)-p+1);

   IF NLS_StrCaseCmp(bpart,'@L')=0 THEN
    BEGIN
     FirstPassUnknown:=False;
     Adr:=EvalIntExpression(Asc,UInt12,OK);
     IF FirstPassUnknown THEN Adr:=(Adr AND $ffc) OR $fc0;
     IF OK THEN
      BEGIN
       ChkSpace(SegData);
       IF Adr AND 3<>0 THEN WrError(1325)
       ELSE IF Adr<$fc0 THEN WrError(1315)
       ELSE IF MomCPU<CPU75004 THEN WrError(1505)
       ELSE
        BEGIN
         Erg:=$40+((Adr AND $3c) SHR 2);
	 DecodeBitAddr:=True;
         BName:=HexString(Adr,3)+'H.@L';
        END;
      END;
    END
   ELSE
    BEGIN
     p:=EvalIntExpression(bpart,UInt2,OK);
     IF OK THEN
      IF NLS_StrCaseCmp(Copy(Asc,1,2),'@H')=0 THEN
       BEGIN
        Adr:=EvalIntExpression(Copy(Asc,3,Length(Asc)-2),UInt4,OK);
        IF OK THEN
         IF MomCPU<CPU75004 THEN WrError(1505)
         ELSE
          BEGIN
           Erg:=(p SHL 4)+Adr; DecodeBitAddr:=True;
           BName:='@H+'+HexString(Adr,1)+'.'+Chr(p+AscOfs);
          END;
       END
      ELSE
       BEGIN
        FirstPassUnknown:=False;
        Adr:=EvalIntExpression(Asc,UInt12,OK);
        IF FirstPassUnknown THEN Adr:=(Adr OR $ff0);
        IF OK THEN
         BEGIN
          ChkSpace(SegData);
	  IF (Adr>=$fb0) AND (Adr<$fc0) THEN
           Erg:=$80+(p SHL 4)+(Adr AND 15)
          ELSE IF Adr>=$ff0 THEN
           Erg:=$c0+(p SHL 4)+(Adr AND 15)
          ELSE
           Erg:=$400+(Word(p) SHL 8)+Lo(Adr)+(Hi(Adr) SHL 12);
          DecodeBitAddr:=True;
          BName:=HexString(Adr,3)+'H.'+Chr(AscOfs+p);
         END;
       END;
    END;
END;

        FUNCTION DecodeIntName(Asc:String; VAR Erg:Byte):Boolean;
VAR
   HErg:Word;
   LPart:Byte;
BEGIN
   NLS_UpString(Asc);
   IF MomCPU<=CPU75402 THEN LPart:=0
   ELSE IF MomCPU<CPU75004 THEN LPart:=1
   ELSE IF MomCPU<CPU75104 THEN LPart:=2
   ELSE LPart:=3;
        IF Asc='IEBT'   THEN HErg:=$000
   ELSE IF Asc='IEW'    THEN HErg:=$102
   ELSE IF Asc='IETPG'  THEN HErg:=$203
   ELSE IF Asc='IET0'   THEN HErg:=$104
   ELSE IF Asc='IECSI'  THEN HErg:=$005
   ELSE IF Asc='IECSIO' THEN HErg:=$205
   ELSE IF Asc='IE0'    THEN HErg:=$006
   ELSE IF Asc='IE2'    THEN HErg:=$007
   ELSE IF Asc='IE4'    THEN HErg:=$120
   ELSE IF Asc='IEKS'   THEN HErg:=$123
   ELSE IF Asc='IET1'   THEN HErg:=$224
   ELSE IF Asc='IE1'    THEN HErg:=$126
   ELSE IF Asc='IE3'    THEN HErg:=$227
   ELSE HErg:=$fff;
   IF HErg=$fff THEN DecodeIntName:=False
   ELSE IF Hi(HErg)>LPart THEN DecodeIntName:=False
   ELSE
    BEGIN
     DecodeIntName:=True; Erg:=Lo(HErg);
    END;
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ASSUME75Count=2;
   ASSUME75s:ARRAY[1..ASSUME75Count] OF ASSUMERec=
	     ((Name:'MBS'; Dest:@MBSValue; Min:0; Max:$0f; NothingVal:$10),
	      (Name:'MBE'; Dest:@MBEValue; Min:0; Max:$01; NothingVal:$01));
VAR
   BErg:Word;
BEGIN
   DecodePseudo:=True;

   IF Memo('ASSUME') THEN
    BEGIN
     CodeASSUME(@ASSUME75s,ASSUME75Count);
     IF (MomCPU=CPU75402) AND (MBEValue<>0) THEN
      BEGIN
       MBEValue:=0; WrError(1440);
      END;
     Exit;
    END;

   IF Memo('SFR') THEN
    BEGIN
     CodeEquate(SegData,0,$fff);
     Exit;
    END;

   IF Memo('BIT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       FirstPassUnknown:=False;
       IF DecodeBitAddr(ArgStr[1],BErg) THEN
        IF NOT FirstPassUnknown THEN
	 BEGIN
          PushLocHandle(-1);
	  EnterIntSymbol(LabPart,BErg,SegNone,False);
          ListLine:='='+BName;
          PopLocHandle;
         END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE PutCode(Code:Word);
BEGIN
   BAsmCode[0]:=Lo(Code);
   IF Hi(Code)=0 THEN CodeLen:=1
   ELSE
    BEGIN
     BAsmCode[1]:=Hi(Code); CodeLen:=2;
    END;
END;

        PROCEDURE MakeCode_75K0;
	Far;
VAR
   z,AdrInt,Dist:Integer;
   HReg:Byte;
   BVal:Word;
   OK,BrRel,BrLong:Boolean;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1; MinOneIs0:=False;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   IF DecodeIntelPseudo(True) THEN Exit;

   { ohne Argument }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE PutCode(Code);
       Exit;
      END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModInd+MModAbs);
       CASE AdrMode OF
       ModReg4:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg4+MModInd+MModAbs+MModImm);
         CASE AdrMode OF
         ModReg4:
          IF HReg=0 THEN
	   BEGIN
            PutCode($7899+(Word(AdrPart) SHL 8)); CheckCPU(CPU75004);
           END
          ELSE IF AdrPart=0 THEN
	   BEGIN
            PutCode($7099+(Word(HReg) SHL 8)); CheckCPU(CPU75004);
           END
          ELSE WrError(1350);
         ModInd:
          IF HReg<>0 THEN WrError(1350)
          ELSE PutCode($e0+AdrPart);
         ModAbs:
          IF HReg<>0 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$a3; BAsmCode[1]:=AdrPart; CodeLen:=2;
           END;
         ModImm:
          IF HReg=0 THEN PutCode($70+AdrPart)
          ELSE
	   BEGIN
	    PutCode($089a+(Word(AdrPart) SHL 12)+(Word(HReg) SHL 8));
            CheckCPU(CPU75004);
           END;
         END;
        END;
       ModReg8:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModInd+MModImm);
         CASE AdrMode OF
         ModReg8:
          IF HReg=0 THEN
	   BEGIN
            PutCode($58aa+(Word(AdrPart) SHL 8)); CheckCPU(CPU75004);
           END
	  ELSE IF AdrPart=0 THEN
	   BEGIN
            PutCode($50aa+(Word(HReg) SHL 8)); CheckCPU(CPU75004);
           END
	  ELSE WrError(1350);
         ModAbs:
          IF HReg<>0 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$a2; BAsmCode[1]:=AdrPart; CodeLen:=2;
            IF (NOT FirstPassUnknown) AND (Odd(AdrPart)) THEN WrError(180)
           END;
         ModInd:
          IF (HReg<>0) OR (AdrPart<>1) THEN WrError(1350)
          ELSE
	   BEGIN
            PutCode($18aa); CheckCPU(CPU75004);
           END;
         ModImm:
          IF Odd(HReg) THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$89+HReg; BAsmCode[1]:=AdrPart; CodeLen:=2;
           END;
         END;
        END;
       ModInd:
        IF AdrPart<>1 THEN WrError(1350)
        ELSE
         BEGIN
          DecodeAdr(ArgStr[2],MModReg4+MModReg8);
          CASE AdrMode OF
          ModReg4:
           IF AdrPart<>0 THEN WrError(1350)
	   ELSE
	    BEGIN
             PutCode($e8); CheckCPU(CPU75004);
            END;
          ModReg8:
           IF AdrPart<>0 THEN WrError(1350)
	   ELSE
	    BEGIN
             PutCode($10aa); CheckCPU(CPU75004);
            END;
          END;
         END;
       ModAbs:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg4+MModReg8);
         CASE AdrMode OF
         ModReg4:
          IF AdrPart<>0 THEN WrError(1350)
	  ELSE
	   BEGIN
	    BAsmCode[0]:=$93; BAsmCode[1]:=HReg; CodeLen:=2;
           END;
         ModReg8:
          IF AdrPart<>0 THEN WrError(1350)
	  ELSE
	   BEGIN
	    BAsmCode[0]:=$92; BAsmCode[1]:=HReg; CodeLen:=2;
            IF (NOT FirstPassUnknown) AND (Odd(HReg)) THEN WrError(180)
	   END;
         END;
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
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModAbs+MModInd);
       CASE AdrMode OF
       ModReg4:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg4+MModAbs+MModInd);
         CASE AdrMode OF
         ModReg4:
          IF HReg=0 THEN PutCode($d8+AdrPart)
          ELSE IF AdrPart=0 THEN PutCode($d8+HReg)
          ELSE WrError(1350);
         ModAbs:
          IF HReg<>0 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$b3; BAsmCode[1]:=AdrPart; CodeLen:=2;
           END;
         ModInd:
          IF (HReg<>0) THEN WrError(1350)
          ELSE PutCode($e8+AdrPart);
         END;
        END;
       ModReg8:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg8+MModAbs+MModInd);
         CASE AdrMode OF
         ModReg8:
          IF HReg=0 THEN
	   BEGIN
            PutCode($40aa+(Word(AdrPart) SHL 8)); CheckCPU(CPU75004);
           END
          ELSE IF AdrPart=0 THEN
	   BEGIN
            PutCode($40aa+(Word(HReg) SHL 8)); CheckCPU(CPU75004);
           END
          ELSE WrError(1350);
         ModAbs:
          IF HReg<>0 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$b2; BAsmCode[1]:=AdrPart; CodeLen:=2;
            IF (FirstPassUnknown) AND (Odd(AdrPart)) THEN WrError(180);
           END;
         ModInd:
          IF (AdrPart<>1) OR (HReg<>0) THEN WrError(1350)
          ELSE
	   BEGIN
            PutCode($11aa); CheckCPU(CPU75004);
           END;
         END;
        END;
       ModAbs:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg4+MModReg8);
         CASE AdrMode OF
         ModReg4:
          IF AdrPart<>0 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$b3; BAsmCode[1]:=HReg; CodeLen:=2;
           END;
         ModReg8:
          IF AdrPart<>0 THEN WrError(1350)
          ELSE
           BEGIN
            BAsmCode[0]:=$b2; BAsmCode[1]:=HReg; CodeLen:=2;
            IF (FirstPassUnknown) AND (Odd(HReg)) THEN WrError(180);
           END;
         END;
        END;
       ModInd:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModReg4+MModReg8);
         CASE AdrMode OF
         ModReg4:
          IF AdrPart<>0 THEN WrError(1350)
          ELSE PutCode($e8+HReg);
         ModReg8:
          IF (AdrPart<>0) OR (HReg<>1) THEN WrError(1350)
          ELSE
	   BEGIN
            PutCode($11aa); CheckCPU(CPU75004);
           END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('MOVT') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'XA')<>0 THEN WrError(1350)
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'@PCDE')=0 THEN
      BEGIN
       PutCode($d4); CheckCPU(CPU75004);
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'@PCXA')=0 THEN PutCode($d0)
     ELSE WrError(1350);
     Exit;
    END;

   IF (Memo('PUSH')) OR (Memo('POP')) THEN
    BEGIN
     OK:=Memo('PUSH');
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'BS')=0 THEN
      BEGIN
       PutCode($0699+(Ord(OK) SHL 8)); CheckCPU(CPU75004);
      END
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg8);
       CASE AdrMode OF
       ModReg8:
        IF Odd(AdrPart) THEN WrError(1350)
        ELSE PutCode($48+Ord(OK)+AdrPart);
       END;
      END;
     Exit;
    END;

   IF (Memo('IN')) OR (Memo('OUT')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       z:=Ord(Memo('IN'));
       IF z>0 THEN
        BEGIN
         ArgStr[3]:=ArgStr[2]; ArgStr[2]:=ArgStr[1]; ArgStr[1]:=ArgStr[3];
        END;
       IF NLS_StrCaseCmp(Copy(ArgStr[1],1,4),'PORT')<>0 THEN WrError(1350)
       ELSE
        BEGIN
         BAsmCode[1]:=$f0+EvalIntExpression(Copy(ArgStr[1],5,Length(ArgStr[1])-4),UInt4,OK);
         IF OK THEN
          BEGIN
           DecodeAdr(ArgStr[2],MModReg8+MModReg4);
           CASE AdrMode OF
           ModReg4:
            IF AdrPart<>0 THEN WrError(1350)
            ELSE
             BEGIN
              BAsmCode[0]:=$93+(z SHL 4); CodeLen:=2;
             END;
           ModReg8:
            IF AdrPart<>0 THEN WrError(1350)
            ELSE
             BEGIN
              BAsmCode[0]:=$92+(z SHL 4); CodeLen:=2;
              CheckCPU(CPU75004);
             END;
           END;
          END;
        END;
      END;
     Exit;
    END;

   { Arithmetik }

   IF Memo('ADDS') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8);
       CASE AdrMode OF
       ModReg4:
        IF AdrPart<>0 THEN WrError(1350)
        ELSE
         BEGIN
          DecodeAdr(ArgStr[2],MModImm+MModInd);
          CASE AdrMode OF
          ModImm:PutCode($60+AdrPart);
          ModInd:IF AdrPart=1 THEN PutCode($d2) ELSE WrError(1350);
          END;
         END;
       ModReg8:
        IF AdrPart=0 THEN
         BEGIN
          DecodeAdr(ArgStr[2],MModReg8+MModImm);
          CASE AdrMode OF
          ModReg8:
	   BEGIN
	    PutCode($c8aa+(Word(AdrPart) SHL 8));
            CheckCPU(CPU75104);
           END;
          ModImm:
           BEGIN
            BAsmCode[0]:=$b9; BAsmCode[1]:=AdrPart;
            CodeLen:=2;
            CheckCPU(CPU75104);
           END;
          END;
         END
        ELSE IF NLS_StrCaseCmp(ArgStr[2],'XA')<>0 THEN WrError(1350)
        ELSE
	 BEGIN
	  PutCode($c0aa+(Word(AdrPart) SHL 8));
          CheckCPU(CPU75104);
         END;
       END;
      END;
     Exit;
    END;

   FOR z:=1 TO AriOrderCount DO
    IF Memo(AriOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[1],MModReg4+MModReg8);
        CASE AdrMode OF
        ModReg4:
         IF AdrPart<>0 THEN WrError(1350)
         ELSE
          BEGIN
           DecodeAdr(ArgStr[2],MModInd);
           CASE AdrMode OF
           ModInd:
	    IF AdrPart=1 THEN
             BEGIN
              BAsmCode[0]:=$a8;
              IF z=1 THEN Inc(BAsmCode[0]);
              IF z=3 THEN Inc(BAsmCode[0],$10);
              CodeLen:=1;
              IF NOT Memo('ADDC') THEN CheckCPU(CPU75004);
             END
	    ELSE WrError(1350);
           END;
          END;
        ModReg8:
         IF AdrPart=0 THEN
          BEGIN
           DecodeAdr(ArgStr[2],MModReg8);
           CASE AdrMode OF
           ModReg8:
	    BEGIN
	     PutCode($c8aa+(z SHL 12)+(Word(AdrPart) SHL 8));
             CheckCPU(CPU75104);
            END;
           END;
          END
         ELSE IF NLS_StrCaseCmp(ArgStr[2],'XA')<>0 THEN WrError(1350)
         ELSE
	  BEGIN
	   PutCode($c0aa+(z SHL 12)+(Word(AdrPart) SHL 8));
           CheckCPU(CPU75104);
          END;
        END;
       END;
      Exit;
     END;

   FOR z:=1 TO LogOrderCount DO
    IF Memo(LogOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[1],MModReg4+MModReg8);
        CASE AdrMode OF
        ModReg4:
         IF AdrPart<>0 THEN WrError(1350)
         ELSE
          BEGIN
           DecodeAdr(ArgStr[2],MModImm+MModInd);
           CASE AdrMode OF
           ModImm:
	    BEGIN
	     PutCode($2099+(Word(AdrPart AND 15) SHL 8)+(z SHL 12));
             CheckCPU(CPU75004);
            END;
           ModInd:
	    IF AdrPart=1 THEN PutCode($80+(z SHL 4)) ELSE WrError(1350);
           END;
          END;
        ModReg8:
         IF AdrPart=0 THEN
          BEGIN
           DecodeAdr(ArgStr[2],MModReg8);
           CASE AdrMode OF
           ModReg8:
	    BEGIN
	     PutCode($88aa+(Word(AdrPart) SHL 8)+(z SHL 12));
             CheckCPU(CPU75104);
            END;
           END;
          END
         ELSE IF NLS_StrCaseCmp(ArgStr[2],'XA')<>0 THEN WrError(1350)
         ELSE
	  BEGIN
	   PutCode($80aa+(Word(AdrPart) SHL 8)+(z SHL 12));
           CheckCPU(CPU75104);
          END;
        END;
       END;
      Exit;
     END;

   IF Memo('INCS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModInd+MModAbs);
       CASE AdrMode OF
       ModReg4:
        PutCode($c0+AdrPart);
       ModReg8:
        IF (AdrPart<1) OR (Odd(AdrPart)) THEN WrError(1350)
        ELSE
         BEGIN
          PutCode($88+AdrPart); CheckCPU(CPU75104);
         END;
       ModInd:
        IF AdrPart=1 THEN
	 BEGIN
          PutCode($0299); CheckCPU(CPU75004);
	 END
	ELSE WrError(1350);
       ModAbs:
        BEGIN
         BAsmCode[0]:=$82; BAsmCode[1]:=AdrPart; CodeLen:=2;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('DECS') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8);
       CASE AdrMode OF
       ModReg4:
        PutCode($c8+AdrPart);
       ModReg8:
        BEGIN
         PutCode($68aa+(Word(AdrPart) SHL 8));
         CheckCPU(CPU75104);
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('SKE') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModReg4+MModReg8+MModInd);
       CASE AdrMode OF
       ModReg4:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModImm+MModInd+MModReg4);
         CASE AdrMode OF
         ModReg4:
          IF HReg=0 THEN
	   BEGIN
            PutCode($0899+(Word(AdrPart) SHL 8)); CheckCPU(CPU75004);
           END
          ELSE IF AdrPart=0 THEN
	   BEGIN
            PutCode($0899+(Word(HReg) SHL 8)); CheckCPU(CPU75004);
           END
          ELSE WrError(1350);
         ModImm:
	  BEGIN
	   BAsmCode[0]:=$9a; BAsmCode[1]:=(AdrPart SHL 4)+HReg;
           CodeLen:=2;
          END;
         ModInd:
	  IF (AdrPart=1) AND (HReg=0) THEN PutCode($80)
	  ELSE WrError(1350);
         END;
        END;
       ModReg8:
        BEGIN
         HReg:=AdrPart;
         DecodeAdr(ArgStr[2],MModInd+MModReg8);
         CASE AdrMode OF
         ModReg8:
	  IF HReg=0 THEN
	   BEGIN
            PutCode($48aa+(Word(AdrPart) SHL 8)); CheckCPU(CPU75104);
           END
	  ELSE IF AdrPart=0 THEN
	   BEGIN
            PutCode($48aa+(Word(HReg) SHL 8)); CheckCPU(CPU75104);
           END
          ELSE WrError(1350);
         ModInd:
          IF AdrPart=1 THEN
	   BEGIN
            PutCode($19aa); CheckCPU(CPU75104);
	   END
	  ELSE WrError(1350);
         END;
        END;
       ModInd:
        IF AdrPart<>1 THEN WrError(1350)
        ELSE
         BEGIN
          MinOneIs0:=True;
          DecodeAdr(ArgStr[2],MModImm+MModReg4+MModReg8);
          CASE AdrMode OF
          ModImm:
           BEGIN
            PutCode($6099+(Word(AdrPart) SHL 8)); CheckCPU(CPU75004);
           END;
          ModReg4:
           IF AdrPart=0 THEN PutCode($80) ELSE WrError(1350);
          ModReg8:
           IF AdrPart=0 THEN
	    BEGIN
             PutCode($19aa); CheckCPU(CPU75004);
	    END
	   ELSE WrError(1350);
          END;
         END;
       END;
      END;
     Exit;
    END;

   IF (Memo('RORC')) OR (Memo('NOT')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'A')<>0 THEN WrError(1350)
     ELSE IF Memo('RORC') THEN PutCode($98)
     ELSE PutCode($5f99);
     Exit;
    END;

   { Bitoperationen }

   IF Memo('MOV1') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       OK:=True;
       IF NLS_StrCaseCmp(ArgStr[1],'CY')=0 THEN z:=$bd
       ELSE IF NLS_StrCaseCmp(ArgStr[2],'CY')=0 THEN z:=$9b
       ELSE OK:=False;
       IF NOT OK THEN WrError(1350)
       ELSE IF DecodeBitAddr(ArgStr[((z SHR 2) AND 3)-1],BVal) THEN
        IF Hi(BVal)<>0 THEN WrError(1350)
        ELSE
         BEGIN
          BAsmCode[0]:=z; BAsmCode[1]:=BVal; CodeLen:=2;
          CheckCPU(CPU75104);
         END;
      END;
     Exit;
    END;

   IF (Memo('SET1')) OR (Memo('CLR1')) THEN
    BEGIN
     OK:=Memo('SET1');
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CY')=0 THEN PutCode($e6+Ord(OK))
     ELSE IF DecodeBitAddr(ArgStr[1],BVal) THEN
      IF Hi(BVal)<>0 THEN
       BEGIN
        BAsmCode[0]:=$84+Ord(OK)+(Hi(BVal AND $300) SHL 4);
	BAsmCode[1]:=Lo(BVal); CodeLen:=2;
       END
      ELSE
       BEGIN
        BAsmCode[0]:=$9c+Ord(OK); BAsmCode[1]:=BVal; CodeLen:=2;
       END;
     Exit;
    END;

   IF (Memo('SKT')) OR (Memo('SKF')) THEN
    BEGIN
     OK:=Memo('SKT');
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CY')=0 THEN
      IF Memo('SKT') THEN PutCode($d7)
      ELSE WrError(1350)
     ELSE IF DecodeBitAddr(ArgStr[1],BVal) THEN
      IF Hi(BVal)<>0 THEN
       BEGIN
        BAsmCode[0]:=$86+Ord(OK)+(Hi(BVal AND $300) SHL 4);
	BAsmCode[1]:=Lo(BVal); CodeLen:=2;
       END
      ELSE
       BEGIN
        BAsmCode[0]:=$be+Ord(OK); BAsmCode[1]:=BVal; CodeLen:=2;
       END;
     Exit;
    END;

   IF Memo('NOT1') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CY')<>0 THEN WrError(1350)
     ELSE PutCode($d6);
     Exit;
    END;

   IF Memo('SKTCLR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF DecodeBitAddr(ArgStr[1],BVal) THEN
      IF Hi(BVal)<>0 THEN WrError(1350)
      ELSE
       BEGIN
        BAsmCode[0]:=$9f; BAsmCode[1]:=BVal; CodeLen:=2;
       END;
     Exit;
    END;

   FOR z:=0 TO LogOrderCount-1 DO
    IF Memo(LogOrders^[z+1]+'1') THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF NLS_StrCaseCmp(ArgStr[1],'CY')<>0 THEN WrError(1350)
      ELSE IF DecodeBitAddr(ArgStr[2],BVal) THEN
       IF Hi(BVal)<>0 THEN WrError(1350)
       ELSE
        BEGIN
         BAsmCode[0]:=$ac+((z AND 1) SHL 1)+((z AND 2) SHL 3);
         BAsmCode[1]:=BVal; CodeLen:=2;
        END;
      Exit;
     END;

   { SprÅnge }

   IF Memo('BR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'PCDE')=0 THEN
      BEGIN
       PutCode($0499); CheckCPU(CPU75004);
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'PCXA')=0 THEN
      BEGIN
       BAsmCode[0]:=$99; BAsmCode[1]:=$00; CodeLen:=2;
       CheckCPU(CPU75104);
      END
     ELSE
      BEGIN
       BrRel:=False; BrLong:=False;
       IF ArgStr[1][1]='$' THEN
        BEGIN
         BrRel:=True; Delete(ArgStr[1],1,1);
        END
       ELSE IF ArgStr[1][1]='!' THEN
        BEGIN
         BrLong:=True; Delete(ArgStr[1],1,1);
        END;
       AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK);
       IF OK THEN
        BEGIN
         Dist:=AdrInt-EProgCounter;
         IF (BrRel) OR ((Dist<=16) AND (Dist>=-15) AND (Dist<>0)) THEN
          BEGIN
           IF Dist>0 THEN
            BEGIN
             Dec(Dist);
             IF Dist>15 THEN WrError(1370) ELSE PutCode($00+Dist);
            END
           ELSE
            BEGIN
             IF Dist<-15 THEN WrError(1370)
             ELSE PutCode($f0+15+Dist);
            END;
          END
         ELSE IF (NOT BrLong) AND (AdrInt SHR 12=EProgCounter SHR 12) AND (EProgCounter AND $fff<$ffe) THEN
          BEGIN
	   BAsmCode[0]:=$50+((AdrInt SHR 8) AND 15);
           BAsmCode[1]:=Lo(AdrInt);
           CodeLen:=2;
          END
         ELSE
          BEGIN
           BAsmCode[0]:=$ab;
           BAsmCode[1]:=Hi(AdrInt AND $3fff);
	   BAsmCode[2]:=Lo(AdrInt);
           CodeLen:=3;
           CheckCPU(CPU75004);
          END;
         ChkSpace(SegCode);
        END;
      END;
     Exit;
    END;

   IF Memo('BRCB') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK);
       IF OK THEN
        IF AdrInt SHR 12<>EProgCounter SHR 12 THEN WrError(1910)
	ELSE IF EProgCounter AND $fff>=$ffe THEN WrError(1905)
        ELSE
         BEGIN
	  BAsmCode[0]:=$50+((AdrInt SHR 8) AND 15);
          BAsmCode[1]:=Lo(AdrInt);
          CodeLen:=2;
          ChkSpace(SegCode);
         END
      END;
     Exit;
    END;

   IF Memo('CALL') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgStr[1][1]='!' THEN
        BEGIN
         Delete(ArgStr[1],1,1); BrLong:=True;
        END
       ELSE BrLong:=False;
       FirstPassUnknown:=False;
       AdrInt:=EvalIntExpression(ArgStr[1],UInt16,OK);
       IF FirstPassUnknown THEN AdrInt:=AdrInt AND $7ff;
       IF OK THEN
        BEGIN
         IF (BrLong) OR (AdrInt>$7ff) THEN
          BEGIN
           BAsmCode[0]:=$ab;
           BAsmCode[1]:=$40+Hi(AdrInt AND $3fff);
           BAsmCode[2]:=Lo(AdrInt);
           CodeLen:=3;
           CheckCPU(CPU75004);
          END
	 ELSE
	  BEGIN
           BAsmCode[0]:=$40+Hi(AdrInt AND $7ff);
           BAsmCode[1]:=Lo(AdrInt);
           CodeLen:=2;
	  END;
         ChkSpace(SegCode);
        END;
      END;
     Exit;
    END;

   IF Memo('CALLF') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       IF ArgStr[1][1]='!' THEN Delete(ArgStr[1],1,1);
       AdrInt:=EvalIntExpression(ArgStr[1],UInt11,OK);
       IF OK THEN
        BEGIN
         BAsmCode[0]:=$40+Hi(AdrInt);
         BAsmCode[1]:=Lo(AdrInt);
         CodeLen:=2;
         ChkSpace(SegCode);
        END;
      END;
     Exit;
    END;

   IF Memo('GETI') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       BAsmCode[0]:=EvalIntExpression(ArgStr[1],UInt6,OK);
       CodeLen:=Ord(OK);
       CheckCPU(CPU75004);
      END;
     Exit;
    END;

   { Steueranweisungen }

   IF (Memo('EI')) OR (Memo('DI')) THEN
    BEGIN
     OK:=Memo('EI');
     IF ArgCnt=0 THEN PutCode($b29c+Ord(OK))
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF DecodeIntName(ArgStr[1],HReg) THEN PutCode($989c+Ord(OK)+(Word(HReg) SHL 8))
     ELSE WrError(1440);
     Exit;
    END;

   IF Memo('SEL') THEN
    BEGIN
     BAsmCode[0]:=$99;
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(Copy(ArgStr[1],1,2),'RB')=0 THEN
      BEGIN
       BAsmCode[1]:=$20+EvalIntExpression(Copy(ArgStr[1],3,Length(ArgStr[1])-2),UInt2,OK);
       IF OK THEN
        BEGIN
         CodeLen:=2; CheckCPU(CPU75104);
        END;
      END
     ELSE IF NLS_StrCaseCmp(Copy(ArgStr[1],1,2),'MB')=0 THEN
      BEGIN
       BAsmCode[1]:=$10+EvalIntExpression(Copy(ArgStr[1],3,Length(ArgStr[1])-2),UInt4,OK);
       IF OK THEN
        BEGIN
         CodeLen:=2; CheckCPU(CPU75004);
        END;
      END
     ELSE WrError(1350);
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        PROCEDURE InitCode_75K0;
	Far;
BEGIN
   SaveInitProc;
   MBSValue:=0; MBEValue:=0;
END;

        FUNCTION ChkPC_75K0:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=ProgCounter<=ROMEnd;
   SegData  : ok:=ProgCounter<$1000;
   ELSE ok:=False;
   END;
   ChkPC_75K0:=(ok) AND (ProgCounter>=0);
END;


        FUNCTION IsDef_75K0:Boolean;
	Far;
BEGIN
   IsDef_75K0:=Memo('SFR') OR Memo('BIT');
END;

        PROCEDURE SwitchFrom_75K0;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_75K0;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='PC'; HeaderID:=$7b; NOPCode:=$60;
   DivideChars:=','; HasAttrs:=False;

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=1; ListGrans[SegCode]:=1; SegInits[SegCode]:=0;
   Grans[SegData]:=1; ListGrans[SegData]:=1; SegInits[SegData]:=0;

   MakeCode:=MakeCode_75K0; ChkPC:=ChkPC_75K0; IsDef:=IsDef_75K0;
   SwitchFrom:=SwitchFrom_75K0; InitFields;
END;

BEGIN
   CPU75402:=AddCPU('75402',SwitchTo_75K0);
   CPU75004:=AddCPU('75004',SwitchTo_75K0);
   CPU75006:=AddCPU('75006',SwitchTo_75K0);
   CPU75008:=AddCPU('75008',SwitchTo_75K0);
   CPU75268:=AddCPU('75268',SwitchTo_75K0);
   CPU75304:=AddCPU('75304',SwitchTo_75K0);
   CPU75306:=AddCPU('75306',SwitchTo_75K0);
   CPU75308:=AddCPU('75308',SwitchTo_75K0);
   CPU75312:=AddCPU('75312',SwitchTo_75K0);
   CPU75316:=AddCPU('75316',SwitchTo_75K0);
   CPU75328:=AddCPU('75328',SwitchTo_75K0);
   CPU75104:=AddCPU('75104',SwitchTo_75K0);
   CPU75106:=AddCPU('75106',SwitchTo_75K0);
   CPU75108:=AddCPU('75108',SwitchTo_75K0);
   CPU75112:=AddCPU('75112',SwitchTo_75K0);
   CPU75116:=AddCPU('75116',SwitchTo_75K0);
   CPU75206:=AddCPU('75206',SwitchTo_75K0);
   CPU75208:=AddCPU('75208',SwitchTo_75K0);
   CPU75212:=AddCPU('75212',SwitchTo_75K0);
   CPU75216:=AddCPU('75216',SwitchTo_75K0);
   CPU75512:=AddCPU('75512',SwitchTo_75K0);
   CPU75516:=AddCPU('75516',SwitchTo_75K0);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_75K0;
END.



