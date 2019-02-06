{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

{$x+}

        UNIT Code3206;

INTERFACE
        Uses NLS,Chunks,
	     AsmDef,AsmSub,AsmPars,AsmCode,CodePseu;


IMPLEMENTATION

TYPE
   TUnit=(NoUnit,L1,L2,S1,S2,M1,M2,D1,D2,LastUnit);

   InstrRec=RECORD
             OpCode:LongInt;
             SrcMask,SrcMask2,DestMask:LongInt;
             CrossUsed:Byte; { Bit 0 -->X1 benutzt, Bit 1 -->X2 benutzt }
             AddrUsed:Byte; { Bit 0 -->Addr1 benutzt, Bit 1 -->Addr2 benutzt
                              Bit 2 -->LdSt1 benutzt, Bit 3 -->LdSt2 benutzt }
             LongUsed:Byte; { Bit 0 -->lange Quelle, Bit 1-->langes Ziel }
             StoreUsed,LongSrc,LongDest:Boolean;
             U:TUnit;
            END;

   FixedOrder=RECORD
               Name:String[6];
               Code:LongInt;
              END;

   MemOrder=RECORD
             Name:String[6];
             Code:LongInt;
             Scale:LongInt;
            END;

   MulOrder=RECORD
             Name:String[7];
             Code:LongInt;
             DSign,SSign1,SSign2:Boolean;
             MayImm:Boolean;
            END;

   CtrlReg=RECORD
            Name:String[7];
            Code:LongInt;
            Wr,Rd:Boolean;
           END;

CONST
   UnitNames:ARRAY[TUnit] OF String[2]=('  ','L1','L2','S1','S2','M1','M2','D1','D2','  ');
   MaxParCnt=8;
   FirstUnit=L1;

   LinAddCnt=6;
   CmpCnt=5;
   MemCnt=8;
   MulCnt=22;
   CtrlCnt=13;

   ModNone=-1;
   ModReg=0;    MModReg=1 SHL ModReg;
   ModLReg=1;   MModLReg=1 SHL ModLReg;
   ModImm=2;    MModImm=1 SHL ModImm;

TYPE
   LinAddField=ARRAY[0..LinAddCnt-1] OF FixedOrder;
   CmpField=ARRAY[0..CmpCnt-1] OF FixedOrder;
   MemField=ARRAY[0..MemCnt-1] OF MemOrder;
   MulField=ARRAY[0..MulCnt-1] OF MulOrder;
   CtrlField=ARRAY[0..CtrlCnt-1] OF CtrlReg;

VAR
   AdrMode:ShortInt;

   CPU32060:CPUVar;

   ThisPar,ThisCross,ThisStore:Boolean;
   ThisAddr,ThisLong:Byte;
   ThisSrc,ThisSrc2,ThisDest:LongInt;
   Condition:LongInt;
   ThisUnit:TUnit;
   UnitFlag,ThisInst:LongInt;
   ParCnt:Integer;
   PacketAddr:LongInt;
   ParRecs:ARRAY[0..MaxParCnt-1] OF InstrRec;

   LinAddOrders:^LinAddField;
   CmpOrders:^CmpField;
   MemOrders:^MemField;
   MulOrders:^MulField;
   CtrlRegs:^CtrlField;

{---------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

   	PROCEDURE AddLinAdd(NName:String; NCode:LongInt);
BEGIN
   IF z>=LinAddCnt THEN Halt(255);
   WITH LinAddOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

   	PROCEDURE AddCmp(NName:String; NCode:LongInt);
BEGIN
   IF z>=CmpCnt THEN Halt(255);
   WITH CmpOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
   Inc(z);
END;

   	PROCEDURE AddMem(NName:String; NCode,NScale:LongInt);
BEGIN
   IF z>=MemCnt THEN Halt(255);
   WITH MemOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Scale:=NScale;
    END;
   Inc(z);
END;

   	PROCEDURE AddMul(NName:String; NCode:LongInt;
	                 NDSign,NSSign1,NSSign2,NMay:Boolean);
BEGIN
   IF z>=MulCnt THEN Halt(255);
   WITH MulOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
     DSign:=NDSign; SSign1:=NSSign1; SSign2:=NSSign2;
     MayImm:=NMay;
    END;
   Inc(z);
END;

   	PROCEDURE AddCtrl(NName:String; NCode:LongInt;
	                  NWr,NRd:Boolean);
BEGIN
   IF z>=CtrlCnt THEN Halt(255);
   WITH CtrlRegs^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
     Wr:=NWr; Rd:=NRd;
    END;
   Inc(z);
END;

BEGIN
   New(LinAddOrders); z:=0;
   AddLinAdd('ADDAB',$30); AddLinAdd('ADDAH',$34); AddLinAdd('ADDAW',$38);
   AddLinAdd('SUBAB',$31); AddLinAdd('SUBAH',$35); AddLinAdd('SUBAW',$39);

   New(CmpOrders); z:=0;
   AddCmp('CMPEQ',$50); AddCmp('CMPGT',$44); AddCmp('CMPGTU',$4c);
   AddCmp('CMPLT',$54); AddCmp('CMPLTU',$5c);

   New(MemOrders); z:=0;
   AddMem('LDB',2,1);  AddMem('LDH',4,2);  AddMem('LDW',6,4);
   AddMem('LDBU',1,1); AddMem('LDHU',0,2); AddMem('STB',3,1);
   AddMem('STH',5,2);  AddMem('STW',7,4);

   New(MulOrders); z:=0;
   AddMul('MPY'    ,$19,True ,True ,True ,True );
   AddMul('MPYU'   ,$1f,False,False,False,False);
   AddMul('MPYUS'  ,$1d,True ,False,True ,False);
   AddMul('MPYSU'  ,$1b,True ,True ,False,True );
   AddMul('MPYH'   ,$01,True ,True ,True ,False);
   AddMul('MPYHU'  ,$07,False,False,False,False);
   AddMul('MPYHUS' ,$05,True ,False,True ,False);
   AddMul('MPYHSU' ,$03,True ,True ,False,False);
   AddMul('MPYHL'  ,$09,True ,True ,True ,False);
   AddMul('MPYHLU' ,$0f,False,False,False,False);
   AddMul('MPYHULS',$0d,True ,False,True ,False);
   AddMul('MPYHSLU',$0b,True ,True ,False,False);
   AddMul('MPYLH'  ,$11,True ,True ,True ,False);
   AddMul('MPYLHU' ,$17,False,False,False,False);
   AddMul('MPYLUHS',$15,True ,False,True ,False);
   AddMul('MPYLSHU',$13,True ,True ,False,False);
   AddMul('SMPY'   ,$1a,True ,True ,True ,False);
   AddMul('SMPYHL' ,$0a,True ,True ,True ,False);
   AddMul('SMPYLH' ,$12,True ,True ,True ,False);
   AddMul('SMPYH'  ,$02,True ,True ,True ,False);

   New(CtrlRegs); z:=0;
   AddCtrl('AMR'    , 0,True ,True );
   AddCtrl('CSR'    , 1,True ,True );
   AddCtrl('IFR'    , 2,False,True );
   AddCtrl('ISR'    , 2,True ,False);
   AddCtrl('ICR'    , 3,True ,False);
   AddCtrl('IER'    , 4,True ,True );
   AddCtrl('ISTP'   , 5,True ,True );
   AddCtrl('IRP'    , 6,True ,True );
   AddCtrl('NRP'    , 7,True ,True );
   AddCtrl('IN'     , 8,False,True );
   AddCtrl('OUT'    , 9,True ,True );
   AddCtrl('PCE1'   ,16,False,True );
   AddCtrl('PDATA_O',15,True ,True );
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(LinAddOrders);
   Dispose(CmpOrders);
   Dispose(MemOrders);
   Dispose(MulOrders);
   Dispose(CtrlRegs);
END;

{---------------------------------------------------------------------------}

	FUNCTION CheckOpt(VAR Asc:String):Boolean;
VAR
   Flag:Boolean;
BEGIN
   CheckOpt:=True;
   IF Asc='||' THEN ThisPar:=True
   ELSE IF (Asc[1]='[') AND (Asc[Length(Asc)]=']') THEN
    BEGIN
     Asc:=Copy(Asc,2,Length(Asc)-2);
     IF Asc[1]='!' THEN
      BEGIN
       Delete(Asc,1,1); Condition:=1;
      END
     ELSE Condition:=0;
     Flag:=True;
     IF Length(Asc)<>2 THEN Flag:=False
     ELSE IF UpCase(Asc[1])='A' THEN
      IF (Asc[2]>='1') AND (Asc[2]<='2') THEN Inc(Condition,(Ord(Asc[2])-AscOfs+3) SHL 1)
      ELSE Flag:=False
     ELSE IF UpCase(Asc[1])='B' THEN
      IF (Asc[2]>='0') AND (Asc[2]<='2') THEN Inc(Condition,(Ord(Asc[2])-AscOfs+1) SHL 1)
      ELSE Flag:=False;
     IF NOT Flag THEN WrXError(1445,Asc); CheckOpt:=Flag;
    END
   ELSE CheckOpt:=False;
END;

	FUNCTION ReiterateOpPart:Boolean;
VAR
   p:Integer;
BEGIN
   ReiterateOpPart:=False;

   IF NOT CheckOpt(OpPart) THEN Exit;

   IF ArgCnt<1 THEN
    BEGIN
     WrError(1210); Exit;
    END;
   p:=FirstBlank(ArgStr[1]);
   IF p=255 THEN
    BEGIN
     OpPart:=ArgStr[1];
     FOR p:=2 TO ArgCnt DO ArgStr[p-1]:=ArgStr[p];
     Dec(ArgCnt);
    END
   ELSE
    BEGIN
     OpPart:=Copy(ArgStr[1],1,p-1);
     Delete(ArgStr[1],1,p); KillPrefBlanks(ArgStr[1]);
    END;
   NLS_UpString(OpPart);
   p:=Pos('.',OpPart);
   IF p=0 THEN AttrPart:=''
   ELSE
    BEGIN
     AttrPart:=Copy(OpPart,p+1,Length(OpPart)-p);
     OpPart[0]:=Chr(p-1);
    END;
   ReiterateOpPart:=True;
END;

{---------------------------------------------------------------------------}

	PROCEDURE AddSrc(Reg:LongInt);
VAR
   Mask:LongInt;
BEGIN
   Mask:=1 SHL Reg;
   IF ThisSrc AND Mask=0 THEN ThisSrc:=ThisSrc OR Mask
   ELSE ThisSrc2:=ThisSrc2 OR Mask;
END;

	PROCEDURE AddLSrc(Reg:LongInt);
BEGIN
   AddSrc(Reg); AddSrc(Reg+1);
   ThisLong:=ThisLong OR 1;
END;

	PROCEDURE AddDest(Reg:LongInt);
BEGIN
   ThisDest:=ThisDest OR (1 SHL Reg);
END;

	PROCEDURE AddLDest(Reg:LongInt);
BEGIN
   ThisDest:=ThisDest OR (3 SHL Reg);
   ThisLong:=ThisLong OR 2;
END;

        FUNCTION FindReg(Mask:LongInt):LongInt;
VAR
   z:Integer;
BEGIN
   z:=0;
   WHILE (z<31) AND (Mask AND 1=0) DO
    BEGIN
     Inc(z); Mask:=Mask SHR 1;
    END;
   FindReg:=z;
END;

	FUNCTION RegName(Num:LongInt):String;
VAR
   s:String[4];
BEGIN
   Str(Num AND 15,s);
   IF Num>15 THEN s:='B'+s ELSE s:='A'+s;
   RegName:=s;
END;

	FUNCTION DecodeSReg(Asc:String; VAR Reg:LongInt; Quarrel:Boolean):Boolean;
VAR
   IO:Integer;
   RVal:Byte;
   TFlag:Boolean;
BEGIN
   TFlag:=True;
   IF UpCase(Asc[1])='A' THEN Reg:=0
   ELSE IF UpCase(Asc[1])='B' THEN Reg:=16
   ELSE TFlag:=False;
   IF TFlag THEN
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),RVal,IO);
     IF IO<>0 THEN TFlag:=False
     ELSE IF (RVal>15) THEN TFlag:=False
     ELSE Inc(Reg,RVal);
    END;
   DecodeSReg:=TFlag;
   IF (NOT TFlag) AND (Quarrel) THEN WrXError(1445,Asc);
END;

	FUNCTION DecodeReg(VAR Asc:String; VAR Reg:LongInt; VAR PFlag:Boolean; Quarrel:Boolean):Boolean;
VAR
   p:Integer;
   NextReg:LongInt;
BEGIN
   p:=Pos(':',Asc);
   IF p=0 THEN
    BEGIN
     PFlag:=False; DecodeReg:=DecodeSReg(Asc,Reg,Quarrel);
    END
   ELSE
    BEGIN
     PFlag:=True;
     IF NOT DecodeSReg(Copy(Asc,1,p-1),NextReg,Quarrel) THEN DecodeReg:=False
     ELSE IF NOT DecodeSReg(Copy(Asc,p+1,Length(Asc)-p),Reg,Quarrel) THEN DecodeReg:=False
     ELSE IF (Odd(Reg)) OR (NextReg<>Reg+1) OR (((Reg XOR NextReg) AND $10)<>0) THEN
      BEGIN
       IF Quarrel THEN WrXError(1760,Asc); DecodeReg:=False;
      END
     ELSE DecodeReg:=True;
    END;
END;

	FUNCTION DecodeCtrlReg(VAR Asc:String; VAR Erg:LongInt; Write:Boolean):Boolean;
VAR
   z:Integer;
   Fnd:Boolean;
BEGIN
   Fnd:=False; z:=0; DecodeCtrlReg:=False;
   FOR z:=0 TO CtrlCnt DO
    WITH CtrlRegs^[z] DO
     IF NLS_StrCaseCmp(Asc,Name)=0 THEN
      BEGIN
       Fnd:=True; Erg:=Code;
       DecodeCtrlReg:=(Write AND Wr) OR ((NOT Write) AND Rd);
       Exit;
      END;
END;

{ Was bedeutet das r-Feld im Adre·operanden mit kurzem Offset ???
  und wie ist das genau mit der Skalierung gemeint ??? }

	FUNCTION DecodeMem(Asc:String; VAR Erg:LongInt; Scale:LongInt):Boolean;
VAR
   RegPart,DispPart:String;
   DispAcc,BaseReg,Mode:LongInt;
   Counter:Char;
   p:Integer;
   OK:Boolean;
BEGIN
   DecodeMem:=False;

   { das mu· da sein }

   IF Asc[1]<>'*' THEN
    BEGIN
     WrError(1350); Exit;
    END;
   Delete(Asc,1,1);

   { teilen }

   p:=Pos('[',Asc); Counter:=']';
   IF p=0 THEN
    BEGIN
     p:=Pos('(',Asc); Counter:=')';
    END;
   IF p<>0 THEN
    BEGIN
     IF Asc[Length(Asc)]<>Counter THEN
      BEGIN
       WrError(1350); Exit;
      END;
     RegPart:=Copy(Asc,1,p-1); DispPart:=Copy(Asc,p+1,Length(Asc)-p-1);
    END
   ELSE
    BEGIN
     RegPart:=Asc; DispPart:='';
    END;

   { Registerfeld entschlÅsseln }

   Mode:=1; { Default ist *+R }
   IF RegPart[1]='+' THEN
    BEGIN
     Delete(RegPart,1,1); Mode:=1;
     IF RegPart[1]='+' THEN
      BEGIN
       Delete(RegPart,1,1); Mode:=9;
      END;
    END
   ELSE IF RegPart[1]='-' THEN
    BEGIN
     Delete(RegPart,1,1); Mode:=0;
     IF RegPart[1]='-' THEN
      BEGIN
       Delete(RegPart,1,1); Mode:=8;
      END;
    END
   ELSE IF RegPart[Length(RegPart)]='+' THEN
    BEGIN
     IF RegPart[Length(RegPart)-1]<>'+' THEN
      BEGIN
       WrError(1350); Exit;
      END;
     Dec(Byte(RegPart[0]),2); Mode:=11;
    END
   ELSE IF RegPart[Length(RegPart)]='-' THEN
    BEGIN
     IF RegPart[Length(RegPart)-1]<>'-' THEN
      BEGIN
       WrError(1350); Exit;
      END;
     Dec(Byte(RegPart[0]),2); Mode:=10;
    END;
   IF NOT DecodeSReg(RegPart,BaseReg,False) THEN
    BEGIN
     WrXError(1445,RegPart); Exit;
    END;
   AddSrc(BaseReg);

   { kein Offsetfeld ? --> Skalierungsgrî·e bei Autoinkrement/De-
     krement, sonst 0 }

   IF DispPart='' THEN
    IF (Mode<2) THEN DispAcc:=0 ELSE DispAcc:=Scale

   { Register als Offsetfeld? Dann Bit 2 in Modus setzen }

   ELSE IF DecodeSReg(DispPart,DispAcc,False) THEN
    BEGIN
     IF DispAcc XOR BaseReg>15 THEN
      BEGIN
       WrError(1350); Exit;
      END;
     Inc(Mode,4); AddSrc(DispAcc);
    END

   { ansonsten normaler Offset }

   ELSE
    BEGIN
     FirstPassUnknown:=False;
     DispAcc:=EvalIntExpression(DispPart,UInt15,OK);
     IF NOT OK THEN Exit;
     IF FirstPassUnknown THEN DispAcc:=DispAcc AND 7;
     IF Counter=']' THEN DispAcc:=DispAcc*Scale;
    END;

   { Benutzung des Adressierers markieren }

   IF BaseReg>15 THEN ThisAddr:=ThisAddr OR 2 ELSE ThisAddr:=ThisAddr OR 1;

   { Wenn Offset>31, muessen wir Variante 2 benutzen }

   IF (Mode AND 4=0) AND (DispAcc>31) THEN
    IF (BaseReg<$1e) OR (Mode<>1) THEN WrError(1350)
    ELSE
     BEGIN
      Erg:=((DispAcc AND $7fff) SHL 8)+((BaseReg AND 1) SHL 7)+12;
      DecodeMem:=True;
     END

   ELSE
    BEGIN
     Erg:=(BaseReg SHL 18)+((DispAcc AND $1f) SHL 13)+(Mode SHL 9)
         +((BaseReg AND $10) SHL 3)+4;
     DecodeMem:=True;
    END;
END;

	FUNCTION DecodeAdr(Asc:String; Mask:Byte; Signed:Boolean; VAR AdrVal:LongInt):Boolean;
LABEL
   Found;
VAR
   OK:Boolean;
BEGIN
   AdrMode:=ModNone;

   IF DecodeReg(Asc,AdrVal,OK,False) THEN
    BEGIN
     IF OK THEN AdrMode:=ModLReg ELSE AdrMode:=ModReg;
     Goto Found;
    END;

   IF Signed THEN AdrVal:=EvalIntExpression(Asc,SInt5,OK) AND $1f
   ELSE AdrVal:=EvalIntExpression(Asc,UInt5,OK);
   IF OK THEN AdrMode:=ModImm;

Found:
   IF (AdrMode<>ModNone) AND ((1 SHL AdrMode) AND Mask=0) THEN
    BEGIN
     WrError(1350); AdrMode:=ModNone; DecodeAdr:=False;
    END
   ELSE DecodeAdr:=True;
END;

	FUNCTION ChkUnit(Reg:LongInt; U1,U2:TUnit):Boolean;
BEGIN
   IF ThisUnit=NoUnit THEN
    BEGIN
     IF Reg>15 THEN ThisUnit:=U2 ELSE ThisUnit:=U1;
     ChkUnit:=True;
    END
   ELSE IF ((ThisUnit=U1) AND (Reg<16)) OR ((ThisUnit=U2) AND (Reg>15)) THEN ChkUnit:=True
   ELSE
    BEGIN
     ChkUnit:=False; WrError(1107);
    END;
   UnitFlag:=Ord(ThisUnit=U2);
END;

	FUNCTION UnitCode(c:Char):TUnit;
BEGIN
   CASE c OF
   'L': UnitCode:=L1;
   'S': UnitCode:=S1;
   'D': UnitCode:=D1;
   'M': UnitCode:=M1;
   END;
END;

	FUNCTION UnitUsed(TestUnit:TUnit):Boolean;
VAR
   z:Integer;
BEGIN
   UnitUsed:=False;
   FOR z:=0 TO Pred(ParCnt) DO
    IF ParRecs[z].U=TestUnit THEN UnitUsed:=True;
END;

	FUNCTION DecideUnit(Reg:LongInt; Units:String):Boolean;
VAR
   z:Integer;
   TestUnit:TUnit;
BEGIN
   IF ThisUnit=NoUnit THEN
    BEGIN
     z:=1;
     WHILE (z<=Length(Units)) AND (ThisUnit=NoUnit) DO
      BEGIN
       TestUnit:=UnitCode(Units[z]);
       IF Reg>=16 THEN TestUnit:=Succ(TestUnit);
       IF NOT UnitUsed(TestUnit) THEN ThisUnit:=TestUnit;
       Inc(z);
      END;
     IF ThisUnit=NoUnit THEN
      BEGIN
       ThisUnit:=UnitCode(Units[1]);
       IF Reg>16 THEN TestUnit:=Succ(TestUnit);
      END;
    END;
   UnitFlag:=(Ord(ThisUnit)-Ord(FirstUnit)) AND 1;
   IF (Reg SHR 4)<>UnitFlag THEN
    BEGIN
     DecideUnit:=False; WrError(1107);
    END
   ELSE DecideUnit:=True;
END;

	PROCEDURE SwapReg(VAR r1,r2:LongInt);
VAR
   tmp:LongInt;
BEGIN
   tmp:=r1; r1:=r2; r2:=tmp;
END;

	FUNCTION IsCross(Reg:LongInt):Boolean;
BEGIN
   IsCross:=(Reg SHR 4)<>UnitFlag;
END;

        PROCEDURE SetCross(Reg:LongInt);
BEGIN
   ThisCross:=(Reg SHR 4)<>UnitFlag;
END;

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

	FUNCTION DecodeInst:Boolean;
VAR
   OK:Boolean;
   Count,Value,Dist:LongInt;
   z:Integer;
   DReg,S1Reg,S2Reg,HReg:LongInt;
   Code1,Code2:LongInt;
   DPFlag,S1Flag,S2Flag,WithImm,IsStore,HasSign:Boolean;

	PROCEDURE CodeL(OpCode,Dest,Src1,Src2:LongInt);
BEGIN
   ThisInst:=$18+(Opcode SHL 5)+(UnitFlag SHL 1)+(Ord(ThisCross) SHL 12)
                +(Dest SHL 23)+(Src2 SHL 18)+(Src1 SHL 13);
   DecodeInst:=True;
END;

	PROCEDURE CodeM(OpCode,Dest,Src1,Src2:LongInt);
BEGIN
   ThisInst:=$00+(Opcode SHL 7)+(UnitFlag SHL 1)+(Ord(ThisCross) SHL 12)
                +(Dest SHL 23)+(Src2 SHL 18)+(Src1 SHL 13);
   DecodeInst:=True;
END;

	PROCEDURE CodeS(OpCode,Dest,Src1,Src2:LongInt);
BEGIN
   ThisInst:=$20+(OpCode SHL 6)+(UnitFlag SHL 1)+(Ord(ThisCross) SHL 12)
                +(Dest SHL 23)+(Src2 SHL 18)+(Src1 SHL 13);
   DecodeInst:=True;
END;

	PROCEDURE CodeD(OpCode,Dest,Src1,Src2:LongInt);
BEGIN
   ThisInst:=$40+(OpCode SHL 7)+(UnitFlag SHL 1)
                +(Dest SHL 23)+(Src2 SHL 18)+(Src1 SHL 13);
   DecodeInst:=True;
END;

BEGIN
   DecodeInst:=False;

   { erstmal der einfache Kram... }

   IF Memo('IDLE') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF (ThisCross) OR (ThisUnit<>NoUnit) THEN WrError(1107)
     ELSE
      BEGIN
       ThisInst:=$0001e000; DecodeInst:=True;
      END;
     Exit;
    END;

   IF Memo('NOP') THEN
    BEGIN
     IF (ArgCnt<>0) AND (ArgCnt<>1) THEN WrError(1110)
     ELSE IF (ThisCross) OR (ThisUnit<>NoUnit) THEN WrError(1107)
     ELSE
      BEGIN
       IF ArgCnt=0 THEN
        BEGIN
         OK:=True; Count:=0;
        END
       ELSE
        BEGIN
         FirstPassUnknown:=False;
         Count:=EvalIntExpression(ArgStr[1],UInt4,OK);
         IF FirstPassUnknown THEN Count:=0 ELSE Dec(Count);
         OK:=ChkRange(Count,0,8);
        END;
       IF OK THEN
        BEGIN
         ThisInst:=Count SHL 13; DecodeInst:=True;
        END;
      END;
     Exit;
    END;

   { Laden/Speichern }

   FOR z:=0 TO MemCnt-1 DO
    WITH MemOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE
        BEGIN
         IsStore:=OpPart[1]='S';
         IF IsStore THEN
          BEGIN
           ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
           ThisStore:=True;
          END;
         IF DecodeAdr(ArgStr[2],MModReg,False,DReg) THEN
          BEGIN
           IF IsStore THEN AddSrc(DReg);
           IF DReg>15 THEN ThisAddr:=ThisAddr OR 8 ELSE ThisAddr:=ThisAddr OR 4;
	   { Zielregister 4 Takte verzîgert, nicht als Dest eintragen }
           OK:=DecodeMem(ArgStr[1],S1Reg,Scale);
           IF OK THEN
            IF S1Reg AND 8=0 THEN OK:=ChkUnit((S1Reg SHR 18) AND 31,D1,D2)
            ELSE OK:=ChkUnit($1e,D1,D2);
           IF OK THEN
            BEGIN
             ThisInst:=S1Reg+(DReg SHL 23)+(Code SHL 4)
                      +((DReg AND 16) SHR 3);
             DecodeInst:=True;
            END;
          END;
        END;
       Exit;
      END;

   IF Memo('STP') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ChkUnit($10,S1,S2) THEN
      BEGIN
       IF DecodeAdr(ArgStr[1],MModReg,False,S2Reg) THEN
        IF (ThisCross) OR (S2Reg<16) THEN WrError(1110)
        ELSE
         BEGIN
          AddSrc(S2Reg);
          CodeS($0c,0,0,S2Reg);
         END;
      END;
     Exit;
    END;

   { jetzt geht's los... }

   IF Memo('ABS') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF DecodeReg(ArgStr[2],DReg,DPFlag,True) THEN
      IF ChkUnit(DReg,L1,L2) THEN
       IF DecodeReg(ArgStr[1],S1Reg,S1Flag,True) THEN
        IF DPFlag<>S1Flag THEN WrError(1350)
        ELSE IF (ThisCross) AND (S1Reg SHR 4=UnitFlag) THEN WrError(1350)
        ELSE
         BEGIN
          SetCross(S1Reg);
          IF DPFlag THEN CodeL($38,DReg,0,S1Reg)
          ELSE CodeL($1a,DReg,0,S1Reg);
          IF DPFlag THEN AddLSrc(S1Reg) ELSE AddSrc(S1Reg);
          IF DPFlag THEN AddLDest(DReg) ELSE AddDest(DReg);
         END;
     Exit;
    END;

   IF Memo('ADD') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[3],MModReg+MModLReg,True,DReg);
       UnitFlag:=DReg SHR 4;
       CASE AdrMode OF
       ModLReg:   { ADD ?,?,long }
        BEGIN
         AddLDest(DReg);
         DecodeAdr(ArgStr[1],MModReg+MModLReg+MModImm,True,S1Reg);
         CASE AdrMode OF
         ModReg:    { ADD int,?,long }
          BEGIN
           AddSrc(S1Reg);
           DecodeAdr(ArgStr[2],MModReg+MModLReg,True,S2Reg);
           CASE AdrMode OF
           ModReg:    { ADD int,int,long }
            IF ChkUnit(DReg,L1,L2) THEN
             IF (ThisCross) AND (S1Reg SHR 4=UnitFlag) AND (S2Reg SHR 4=UnitFlag) THEN WrError(1350)
             ELSE IF (S1Reg SHR 4<>UnitFlag) AND (S2Reg SHR 4<>UnitFlag) THEN WrError(1350)
             ELSE
              BEGIN
               AddSrc(S2Reg);
               IF S1Reg SHR 4<>UnitFlag THEN SwapReg(S1Reg,S2Reg);
               SetCross(S2Reg);
               CodeL($23,DReg,S1Reg,S2Reg);
              END;
           ModLReg:   { ADD int,long,long }
            IF ChkUnit(DReg,L1,L2) THEN
             IF (S2Reg SHR 4<>UnitFlag) THEN WrError(1350)
             ELSE IF (ThisCross) AND (S1Reg SHR 4=UnitFlag) THEN WrError(1350)
             ELSE
              BEGIN
               AddLSrc(S2Reg);
               SetCross(S1Reg);
               CodeL($21,DReg,S1Reg,S2Reg);
              END;
           END;
          END;
         ModLReg:   { ADD long,?,long }
          BEGIN
           AddLSrc(S1Reg);
           DecodeAdr(ArgStr[2],MModReg+MModImm,True,S2Reg);
           CASE AdrMode OF
           ModReg:    { ADD long,int,long }
            IF ChkUnit(DReg,L1,L2) THEN
             IF (S1Reg SHR 4<>UnitFlag) THEN WrError(1350)
             ELSE IF (ThisCross) AND (S2Reg SHR 4=UnitFlag) THEN WrError(1350)
             ELSE
              BEGIN
               AddSrc(S2Reg);
               SetCross(S2Reg);
               CodeL($21,DReg,S2Reg,S1Reg);
              END;
           ModImm:    { ADD long,imm,long }
            IF ChkUnit(DReg,L1,L2) THEN
             IF (S1Reg SHR 4<>UnitFlag) THEN WrError(1350)
             ELSE IF ThisCross THEN WrError(1350)
             ELSE CodeL($20,DReg,S2Reg,S1Reg);
           END;
          END;
         ModImm: { ADD imm,?,long }
          BEGIN
           IF DecodeAdr(ArgStr[2],MModLReg,True,S2Reg) THEN
            BEGIN { ADD imm,long,long }
             IF ChkUnit(DReg,L1,L2) THEN
              IF (S2Reg SHR 4<>UnitFlag) THEN WrError(1350)
              ELSE IF ThisCross THEN WrError(1350)
              ELSE
               BEGIN
                AddLSrc(S2Reg);
                CodeL($20,DReg,S1Reg,S2Reg);
               END;
            END;
          END;
         END;
        END;
       ModReg: { ADD ?,?,int }
        BEGIN
         AddDest(DReg);
         DecodeAdr(ArgStr[1],MModReg+MModImm,True,S1Reg);
         CASE AdrMode OF
         ModReg: { ADD int,?,int }
          BEGIN
           AddSrc(S1Reg);
           DecodeAdr(ArgStr[2],MModReg+MModImm,True,S2Reg);
           CASE AdrMode OF
           ModReg: { ADD int,int,int }
            BEGIN
             AddSrc(S2Reg);
             IF (DReg XOR S1Reg>15) AND (DReg XOR S2Reg>15) THEN WrError(1350)
             ELSE IF (ThisCross) AND (DReg XOR S1Reg<16) AND (DReg XOR S2Reg<15) THEN WrError(1350)
             ELSE
              BEGIN
               IF S1Reg XOR DReg>15 THEN SwapReg(S1Reg,S2Reg);
               IF S2Reg XOR DReg>15 THEN OK:=DecideUnit(DReg,'LS')
               ELSE OK:=DecideUnit(DReg,'LSD');
               IF OK THEN
                BEGIN
                 CASE ThisUnit OF
                 L1,L2: CodeL($03,DReg,S1Reg,S2Reg); { ADD.Lx int,int,int }
                 S1,S2: CodeS($07,DReg,S1Reg,S2Reg); { ADD.Sx int,int,int }
                 D1,D2: CodeD($10,DReg,S1Reg,S2Reg); { ADD.Dx int,int,int }
                 END;
                END;
              END;
            END;
           ModImm: { ADD int,imm,int }
            IF (ThisCross) AND (S1Reg SHR 4=UnitFlag) THEN WrError(1350)
            ELSE
             BEGIN
              SetCross(S1Reg);
              IF DecideUnit(DReg,'LS') THEN
               CASE ThisUnit OF
               L1,L2: CodeL($02,DReg,S2Reg,S1Reg);
               S1,S2: CodeS($06,DReg,S2Reg,S1Reg);
               END;
             END;
           END;
          END;
         ModImm: { ADD imm,?,int }
          BEGIN
           IF DecodeAdr(ArgStr[2],MModReg,True,S2Reg) THEN
            BEGIN
             AddSrc(S2Reg);
             IF (ThisCross) AND (S2Reg SHR 4=UnitFlag) THEN WrError(1350)
             ELSE
              BEGIN
               SetCross(S2Reg);
               IF DecideUnit(DReg,'LS') THEN
                CASE ThisUnit OF
                L1,L2: CodeL($02,DReg,S1Reg,S2Reg);
                S1,S2: CodeS($06,DReg,S1Reg,S2Reg);
                END;
              END;
            END;
          END;
         END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('ADDU') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[3],MModReg+MModLReg,False,DReg);
       CASE AdrMode OF
       ModReg: { ADDU ?,?,int }
        IF ChkUnit(DReg,D1,D2) THEN
         BEGIN
          AddDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModImm,False,S1Reg);
	  CASE AdrMode OF
          ModReg: { ADDU int,?,int }
           IF IsCross(S1Reg) THEN WrError(1350)
           ELSE
            BEGIN
             AddSrc(S1Reg);
             IF DecodeAdr(ArgStr[2],MModImm,False,S2Reg) THEN
              CodeD($12,DReg,S2Reg,S1Reg);
            END;
          ModImm: { ADDU imm,?,int }
           BEGIN
            IF DecodeAdr(ArgStr[2],MModReg,False,S2Reg) THEN
             IF IsCross(S2Reg) THEN WrError(1350)
             ELSE
              BEGIN
               AddSrc(S2Reg);
               CodeD($12,DReg,S1Reg,S2Reg);
              END;
           END;
          END;
         END;
       ModLReg: { ADDU ?,?,long }
        IF ChkUnit(DReg,L1,L2) THEN
         BEGIN
          AddLDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModLReg,False,S1Reg);
          CASE AdrMode OF
          ModReg: { ADDU int,?,long }
           BEGIN
            AddSrc(S1Reg);
            DecodeAdr(ArgStr[2],MModReg+MModLReg,False,S2Reg);
            CASE AdrMode OF
            ModReg: { ADDU int,int,long }
             IF (IsCross(S1Reg)) AND (IsCross(S2Reg)) THEN WrError(1350)
             ELSE IF (ThisCross) AND ((S1Reg XOR DReg<16) AND (S2Reg XOR DReg<16)) THEN WrError(1350)
             ELSE
              BEGIN
               IF S1Reg XOR DReg>15 THEN SwapReg(S1Reg,S2Reg);
               SetCross(S2Reg);
               CodeL($2b,DReg,S1Reg,S2Reg);
              END;
            ModLReg: { ADDU int,long,long }
             IF IsCross(S2Reg) THEN WrError(1350)
             ELSE IF (ThisCross) AND (S1Reg XOR DReg<16) THEN WrError(1350)
             ELSE
              BEGIN
               AddLSrc(S2Reg);
               SetCross(S1Reg);
               CodeL($29,DReg,S1Reg,S2Reg);
              END;
            END;
           END;
          ModLReg:
           IF IsCross(S1Reg) THEN WrError(1350)
           ELSE
            BEGIN
             AddLSrc(S1Reg);
             IF DecodeAdr(ArgStr[2],MModReg,False,S2Reg) THEN
              IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
              ELSE
               BEGIN
                AddSrc(S2Reg); SetCross(S2Reg);
                CodeL($29,DReg,S2Reg,S1Reg);
               END;
            END;
          END;
         END;
       END;
      END;
     Exit;
    END;

   IF Memo('SUB') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[3],MModReg+MModLReg,True,DReg);
       CASE AdrMode OF
       ModReg:
        BEGIN
         AddDest(DReg);
         DecodeAdr(ArgStr[1],MModReg+MModImm,True,S1Reg);
         CASE AdrMode OF
         ModReg:
          BEGIN
           AddSrc(S1Reg);
           DecodeAdr(ArgStr[2],MModReg+MModImm,True,S2Reg);
           CASE AdrMode OF
           ModReg:
            IF (ThisCross) AND (S1Reg XOR DReg<16) AND (S2Reg XOR DReg<16) THEN WrError(1350)
            ELSE IF (S1Reg XOR DReg>15) AND (S2Reg XOR DReg>15) THEN WrError(1350)
            ELSE
             BEGIN
              AddSrc(S2Reg);
              ThisCross:=(S1Reg XOR DReg>15) OR (S2Reg XOR DReg>15);
              IF S1Reg XOR DReg>15 THEN OK:=DecideUnit(DReg,'L')
              ELSE IF S2Reg XOR DReg>15 THEN OK:=DecideUnit(DReg,'LS')
              ELSE OK:=DecideUnit(DReg,'LSD');
              IF OK THEN
               CASE ThisUnit OF
               L1,L2:IF S1Reg XOR DReg>15 THEN CodeL($17,DReg,S1Reg,S2Reg)
                     ELSE CodeL($07,DReg,S1Reg,S2Reg);
               S1,S2:CodeS($17,DReg,S1Reg,S2Reg);
               D1,D2:CodeD($11,DReg,S2REg,S1Reg);
              END;
             END;
           ModImm:
            IF ChkUnit(DReg,D1,D2) THEN
             IF (ThisCross) OR (S1Reg XOR DReg>15) THEN WrError(1350)
             ELSE CodeD($13,DReg,S2Reg,S1Reg);
           END;
          END;
         ModImm:
          BEGIN
           IF DecodeAdr(ArgStr[2],MModReg,True,S2Reg) THEN
            IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
            ELSE
             BEGIN
              AddSrc(S2Reg);
              IF DecideUnit(DReg,'LS') THEN
               CASE ThisUnit OF
               L1,L2:CodeL($06,DReg,S1Reg,S2Reg);
               S1,S2:CodeS($16,DReg,S1Reg,S2Reg);
               END;
             END;
          END;
         END;
        END;
       ModLReg:
        BEGIN
         AddLDest(DReg);
         IF ChkUnit(DReg,L1,L2) THEN
          BEGIN
           DecodeAdr(ArgStr[1],MModImm+MModReg,True,S1Reg);
           CASE AdrMode OF
           ModImm:
            BEGIN
             IF DecodeAdr(ArgStr[2],MModLReg,True,S2Reg) THEN
              IF (ThisCross) OR (S2Reg XOR DReg>15) THEN WrError(1350)
              ELSE
               BEGIN
                AddLSrc(S2Reg);
                CodeL($24,DReg,S1Reg,S2Reg);
               END;
            END;
           ModReg:
            BEGIN
             AddSrc(S1Reg);
             IF DecodeAdr(ArgStr[2],MModReg,True,S2Reg) THEN
              IF (ThisCross) AND (S1Reg XOR DReg<16) AND (S2Reg XOR DReg<16) THEN WrError(1350)
              ELSE IF (S1Reg XOR DReg>15) AND (S2Reg XOR DReg>15) THEN WrError(1350)
              ELSE
               BEGIN
                AddSrc(S2Reg);
                ThisCross:=(S1Reg XOR DReg>15) OR (S2Reg XOR DReg>15);
                IF S1Reg XOR DReg>15 THEN CodeL($37,DReg,S1Reg,S2Reg)
                ELSE CodeL($47,DReg,S1Reg,S2Reg);
               END;
            END;
           END;
          END;
        END;
       END;
      END;
     Exit;
    END;

   IF Memo('SUBU') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       IF (DecodeAdr(ArgStr[3],MModLReg,False,DReg)) AND (ChkUnit(DReg,L1,L2)) THEN
        BEGIN
         AddLDest(DReg);
         IF DecodeAdr(ArgStr[1],MModReg,False,S1Reg) THEN
          BEGIN
           AddSrc(S1Reg);
           IF DecodeAdr(ArgStr[2],MModReg,False,S2Reg) THEN
            IF (ThisCross) AND (S1Reg XOR DReg<16) AND (S2Reg XOR DReg<16) THEN WrError(1350)
            ELSE IF (IsCross(S1Reg)) AND (IsCross(S2Reg)) THEN WrError(1350)
            ELSE
             BEGIN
              AddSrc(S2Reg);
              ThisCross:=IsCross(S1Reg) OR IsCross(S2Reg);
              IF IsCross(S1Reg) THEN CodeL($3f,DReg,S1Reg,S2Reg)
              ELSE CodeL($2f,DReg,S1Reg,S2Reg);
             END;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('SUBC') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       IF (DecodeAdr(ArgStr[3],MModReg,False,DReg)) AND (ChkUnit(DReg,L1,L2)) THEN
        BEGIN
         AddLDest(DReg);
         IF DecodeAdr(ArgStr[1],MModReg,False,S1Reg) THEN
          BEGIN
           IF DecodeAdr(ArgStr[2],MModReg,False,S2Reg) THEN
            IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
            ELSE IF IsCross(S1Reg) THEN WrError(1350)
            ELSE
             BEGIN
              AddSrc(S2Reg); SetCross(S2Reg);
              CodeL($4b,DReg,S1Reg,S2Reg);
             END;
          END;
        END;
      END;
     Exit;
    END;

   FOR z:=0 TO LinAddCnt-1 DO
    WITH LinAddOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE IF ThisCross THEN WrError(1350)
       ELSE
        BEGIN
         IF DecodeAdr(ArgStr[3],MModReg,True,DReg) THEN
          IF ChkUnit(DReg,D1,D2) THEN
           BEGIN
            AddDest(DReg);
            IF DecodeAdr(ArgStr[1],MModReg,True,S2Reg) THEN
             IF IsCross(S2Reg) THEN WrError(1350)
             ELSE
              BEGIN
               AddSrc(S2Reg);
               DecodeAdr(ArgStr[2],MModReg+MModImm,False,S1Reg);
               CASE AdrMode OF
               ModReg:
                IF IsCross(S1Reg) THEN WrError(1350)
                ELSE
                 BEGIN
                  AddSrc(S1Reg);
                  CodeD(Code,DReg,S1Reg,S2Reg);
                 END;
               ModImm:
                CodeD(Code+2,DReg,S1Reg,S2Reg);
               END;
              END;
           END
        END;
       Exit;
      END;

   IF Memo('ADDK') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeAdr(ArgStr[2],MModReg,False,DReg) THEN
        IF ChkUnit(DReg,S1,S2) THEN
         BEGIN
          AddDest(DReg);
          Value:=EvalIntExpression(ArgStr[1],SInt16,OK);
          IF OK THEN
           BEGIN
            ThisInst:=$50+(UnitFlag SHL 1)+((Value AND $ffff) SHL 7)+(DReg SHL 23);
            DecodeInst:=True;
           END;
         END;
      END;
     Exit;
    END;

   IF (Memo('ADD2')) OR (Memo('SUB2')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeAdr(ArgStr[3],MModReg,True,DReg) THEN
        IF ChkUnit(DReg,S1,S2) THEN
         BEGIN
          AddDest(DReg);
          IF DecodeAdr(ArgStr[1],MModReg,True,S1Reg) THEN
           BEGIN
            AddSrc(S1Reg);
            IF DecodeAdr(ArgStr[2],MModReg,True,S2Reg) THEN
             IF (ThisCross) AND (S1Reg XOR DReg<16) AND (S2Reg XOR DReg<16) THEN WrError(1350)
             ELSE IF (IsCross(S1Reg)) AND (IsCross(S2Reg)) THEN WrError(1350)
             ELSE
              BEGIN
               OK:=True; AddSrc(S2Reg);
               IF IsCross(S1Reg) THEN
                IF Memo('SUB2') THEN
                 BEGIN
                  WrError(1350); OK:=False;
                 END
                ELSE SwapReg(S1Reg,S2Reg);
               IF OK THEN
                BEGIN
                 SetCross(S2Reg);
                 IF Memo('ADD2') THEN CodeS($01,DReg,S1Reg,S2Reg)
                 ELSE CodeS($21,DReg,S1Reg,S2Reg);
                END;
              END;
           END;
         END;
      END;
     Exit;
    END;

   IF (Memo('AND')) OR (Memo('OR')) OR (Memo('XOR')) THEN (**)
    BEGIN
     IF Memo('AND') THEN
      BEGIN
       Code1:=$79; Code2:=$1f;
      END
     ELSE IF Memo('OR') THEN
      BEGIN
       Code1:=$7f; Code2:=$1b;
      END
     ELSE
      BEGIN
       Code1:=$6f; Code2:=$0b;
      END;
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeAdr(ArgStr[3],MModReg,True,DReg) THEN
        BEGIN
         AddDest(DReg);
         DecodeAdr(ArgStr[1],MModImm+MModReg,True,S1Reg);
         CASE AdrMode OF
         ModImm:
          BEGIN
           OK:=DecodeAdr(ArgStr[2],MModReg,True,S2Reg);
           IF OK THEN AddSrc(S2Reg);
           WithImm:=True;
          END;
         ModReg:
          BEGIN
           AddSrc(S1Reg);
	   OK:=DecodeAdr(ArgStr[2],MModImm+MModReg,True,S2Reg);
           CASE AdrMode OF
           ModImm:
            BEGIN
             SwapReg(S1Reg,S2Reg); WithImm:=True;
            END;
           ModReg:
            BEGIN
             AddSrc(S2Reg); WithImm:=False;
            END;
           END;
          END;
         ELSE OK:=False;
         END;
         IF OK THEN
          IF DecideUnit(DReg,'LS') THEN
           IF (NOT WithImm) AND (IsCross(S1Reg)) AND (IsCross(S2Reg)) THEN WrError(1350)
           ELSE IF (ThisCross) AND (S2Reg XOR DReg<16) AND ((WithImm) OR (S1Reg XOR DReg<15)) THEN WrError(1350)
           ELSE
            BEGIN
             IF (NOT WithImm) AND (IsCross(S1Reg)) THEN SwapReg(S1Reg,S2Reg);
             SetCross(S2Reg);
             CASE ThisUnit OF
             L1,L2:CodeL(Code1-Ord(WithImm),DReg,S1Reg,S2Reg);
             S1,S2:CodeS(Code1-Ord(WithImm),DReg,S1Reg,S2Reg);
             END;
            END;
        END;
      END;
     Exit;
    END;

   IF (Memo('CLR') OR (Memo('EXT')) OR (Memo('EXTU')) OR (Memo('SET'))) THEN
    BEGIN
     IF (ArgCnt<>3) AND (ArgCnt<>4) THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeAdr(ArgStr[ArgCnt],MModReg,Memo('EXT'),DReg) THEN
        IF ChkUnit(DReg,S1,S2) THEN
         BEGIN
          AddDest(DReg);
          IF DecodeAdr(ArgStr[1],MModReg,Memo('EXT'),S2Reg) THEN
           BEGIN
            AddSrc(S2Reg);
            IF ArgCnt=3 THEN
             BEGIN
              IF DecodeAdr(ArgStr[2],MModReg,False,S1Reg) THEN
               IF IsCross(S1Reg) THEN WrError(1350)
               ELSE IF (ThisCross) AND (DReg XOR S2Reg<16) THEN WrError(1350)
               ELSE
                BEGIN
                 SetCross(S2Reg);
                 IF Memo('CLR') THEN CodeS($3f,DReg,S1Reg,S2Reg)
                 ELSE IF Memo('EXTU') THEN CodeS($2b,DReg,S1Reg,S2Reg)
                 ELSE IF Memo('SET') THEN CodeS($3b,DReg,S1Reg,S2Reg)
                 ELSE CodeS($2f,DReg,S1Reg,S2Reg);
                END;
             END
            ELSE IF (ThisCross) OR (IsCross(S2Reg)) THEN WrError(1350)
            ELSE
             BEGIN
              S1Reg:=EvalIntExpression(ArgStr[2],UInt5,OK);
              IF OK THEN
               BEGIN
                HReg:=EvalIntExpression(ArgStr[3],UInt5,OK);
                IF OK THEN
                 BEGIN
                  ThisInst:=(DReg SHL 23)+(S2Reg SHL 18)+(S1Reg SHL 13)+
                            (HReg SHL 8)+(UnitFlag SHL 1);
                  IF Memo('CLR') THEN Inc(ThisInst,$c8)
                  ELSE IF Memo('SET') THEN Inc(ThisInst,$88)
		  ELSE IF Memo('EXT') THEN Inc(ThisInst,$48)
                  ELSE Inc(ThisInst,$08);
                  DecodeInst:=True;
                 END;
               END;
             END;
           END;
         END;
      END;
     Exit;
    END;

   FOR z:=0 TO CmpCnt-1 DO
    WITH CmpOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       WithImm:=OpPart[Length(OpPart)]<>'U';
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE
        BEGIN
         IF DecodeAdr(ArgStr[3],MModReg,False,DReg) THEN
          IF ChkUnit(DReg,L1,L2) THEN
           BEGIN
            AddDest(DReg);
            DecodeAdr(ArgStr[1],MModReg+MModImm,WithImm,S1Reg);
            CASE AdrMode OF
            ModReg:
             BEGIN
              AddSrc(S1Reg);
              DecodeAdr(ArgStr[2],MModReg+MModLReg,WithImm,S2Reg);
              CASE AdrMode OF
              ModReg:
               IF (IsCross(S1Reg)) AND (IsCross(S2Reg)) THEN WrError(1350)
               ELSE IF (ThisCross) AND (S1Reg XOR DReg<16) AND (S2Reg XOR DReg<16) THEN WrError(1350)
               ELSE
                BEGIN
                 AddSrc(S2Reg);
                 IF IsCross(S1Reg) THEN SwapReg(S1Reg,S2Reg);
                 SetCross(S2Reg);
                 CodeL(Code+3,DReg,S1Reg,S2Reg);
                END;
              ModLReg:
               IF IsCross(S2Reg) THEN WrError(1350)
               ELSE IF (ThisCross) AND (S1Reg XOR DReg<16) THEN WrError(1350)
               ELSE
                BEGIN
                 AddLSrc(S2Reg); SetCross(S1Reg);
                 CodeL(Code+1,DReg,S1Reg,S2Reg);
                END;
              END;
             END;
            ModImm:
             BEGIN
              DecodeAdr(ArgStr[2],MModReg+MModLReg,WithImm,S2Reg);
              CASE AdrMode OF
              ModReg:
               IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
               ELSE
                BEGIN
                 AddSrc(S2Reg); SetCross(S2Reg);
                 CodeL(Code+2,DReg,S1Reg,S2Reg);
                END;
              ModLReg:
               IF (ThisCross) OR (IsCross(S2Reg)) THEN WrError(1350)
               ELSE
                BEGIN
                 AddLSrc(S2Reg);
                 CodeL(Code,DReg,S1Reg,S2Reg);
                END;
              END;
             END;
            END;
           END;
        END;
       Exit;
      END;

   IF Memo('LMBD') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeAdr(ArgStr[3],MModReg,False,DReg) THEN
        IF ChkUnit(DReg,L1,L2) THEN
         BEGIN
          AddDest(DReg);
          IF DecodeAdr(ArgStr[2],MModReg,False,S2Reg) THEN
           IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
           ELSE
            BEGIN
             SetCross(S2Reg);
             IF DecodeAdr(ArgStr[1],MModImm+MModReg,False,S1Reg) THEN
              BEGIN
               IF AdrMode=ModReg THEN AddSrc(S1Reg);
               CodeL($6a+Ord(AdrMode=ModImm),DReg,S1Reg,S2Reg);
              END;
            END;
         END;
      END;
     Exit;
    END;

   IF Memo('NORM') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeAdr(ArgStr[2],MModReg,False,DReg) THEN
        IF ChkUnit(DReg,L1,L2) THEN
         BEGIN
          AddDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModLReg,True,S2Reg);
          CASE AdrMode OF
          ModReg:
           IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
           ELSE
            BEGIN
             SetCross(S2Reg); AddSrc(S2Reg);
             CodeL($63,DReg,0,S2Reg);
            END;
          ModLReg:
           IF (ThisCross) OR (IsCross(S2Reg)) THEN WrError(1350)
           ELSE
            BEGIN
             AddLSrc(S2Reg);
             CodeL($60,DReg,0,S2Reg);
            END;
          END;
         END;
      END;
     Exit;
    END;

   FOR z:=0 TO MulCnt-1 DO
    WITH MulOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>3 THEN WrError(1110)
       ELSE
        BEGIN
         IF DecodeAdr(ArgStr[3],MModReg,DSign,DReg) THEN
          IF ChkUnit(DReg,M1,M2) THEN
           BEGIN
            IF DecodeAdr(ArgStr[2],MModReg,SSign2,S2Reg) THEN
             BEGIN
              AddSrc(S2Reg);
              IF MayImm THEN DecodeAdr(ArgStr[1],MModImm+MModReg,SSign1,S1Reg)
    	      ELSE DecodeAdr(ArgStr[1],MModReg,SSign1,S1Reg);
              CASE AdrMode OF
              ModReg:
               IF (ThisCross) AND (S2Reg XOR DReg<16) AND (S1Reg XOR DReg<16) THEN WrError(1350)
               ELSE IF (S2Reg XOR DReg>15) AND (S1Reg XOR DReg>15) THEN WrError(1350)
               ELSE
 	        BEGIN
                 IF S1Reg XOR DReg>15 THEN SwapReg(S1Reg,S2Reg);
                 SetCross(S2Reg);
                 AddSrc(S1Reg);
	         CodeM(Code,DReg,S1Reg,S2Reg);
                END;
              ModImm:
	       IF Memo('MPY') THEN CodeM(Code-1,DReg,S1Reg,S2Reg)
               ELSE CodeM(Code+3,DReg,S1Reg,S2Reg);
              END;
             END;
           END;
	END;
       Exit;
      END;

   IF Memo('SADD') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       IF (DecodeAdr(ArgStr[3],MModReg+MModLReg,True,DReg)) AND (ChkUnit(DReg,L1,L2)) THEN
        CASE AdrMode OF
        ModReg:
         BEGIN
          AddDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModImm,True,S1Reg);
          CASE AdrMode OF
          ModReg:
           BEGIN
            AddSrc(S1Reg);
            DecodeAdr(ArgStr[2],MModReg+MModImm,True,S2Reg);
            CASE AdrMode OF
            ModReg:
             IF (ThisCross) AND (S1Reg XOR DReg<16) AND (S2Reg XOR DReg<16) THEN WrError(1350)
             ELSE IF (IsCross(S1Reg)) AND (IsCross(S2Reg)) THEN WrError(1350)
             ELSE
              BEGIN
               AddSrc(S2Reg);
               IF IsCross(S1Reg) THEN SwapReg(S1Reg,S2Reg);
               SetCross(S2Reg);
               CodeL($13,DReg,S1Reg,S2Reg);
              END;
            ModImm:
             IF (ThisCross) AND (S1Reg XOR DReg<16) THEN WrError(1350)
             ELSE
              BEGIN
               SetCross(S1Reg);
               CodeL($12,DReg,S2Reg,S1Reg);
              END;
            END;
           END;
          ModImm:
           BEGIN
            IF DecodeAdr(ArgStr[2],MModReg,True,S2Reg) THEN
             IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
             ELSE
              BEGIN
               SetCross(S2Reg);
               CodeL($12,DReg,S1Reg,S2Reg);
              END;
           END;
          END;
         END;
        ModLReg:
         BEGIN
          AddLDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModLReg+MModImm,True,S1Reg);
          CASE AdrMode OF
          ModReg:
           BEGIN
            AddSrc(S1Reg);
            IF DecodeAdr(ArgStr[2],MModLReg,True,S2Reg) THEN
             IF (ThisCross) AND (S1Reg XOR DReg<16) THEN WrError(1350)
             ELSE
              BEGIN
               AddLSrc(S2Reg); SetCross(S1Reg);
               CodeL($31,DReg,S1Reg,S2Reg);
              END;
           END;
          ModLReg:
           IF IsCross(S1Reg) THEN WrError(1350)
           ELSE
            BEGIN
             AddLSrc(S1Reg);
             DecodeAdr(ArgStr[2],MModReg+MModImm,True,S2Reg);
             CASE AdrMode OF
             ModReg:
              IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
              ELSE
               BEGIN
                AddSrc(S2Reg); SetCross(S2Reg);
                CodeL($31,DReg,S2Reg,S1Reg);
               END;
             ModImm:
              CodeL($30,DReg,S2Reg,S1Reg);
             END;
            END;
          ModImm:
           BEGIN
            IF DecodeAdr(ArgStr[2],MModLReg,True,S2Reg) THEN
             IF IsCross(S2Reg) THEN WrError(1350)
             ELSE
              BEGIN
               AddLSrc(S2Reg);
               CodeL($30,DReg,S1Reg,S2Reg);
              END;
           END;
          END;
         END;
        END;
      END;
     Exit;
    END;

   IF Memo('SAT') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeAdr(ArgStr[2],MModReg,True,DReg) THEN
        IF ChkUnit(DReg,L1,L2) THEN
         BEGIN
          AddDest(DReg);
          IF DecodeAdr(ArgStr[1],MModLReg,True,S2Reg) THEN
           IF (ThisCross) OR (IsCross(S2Reg)) THEN WrError(1350)
           ELSE
            BEGIN
             AddLSrc(S2Reg); CodeL($40,DReg,0,S2Reg);
            END;
         END;
      END;
     Exit;
    END;

   IF Memo('MVC') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (ThisUnit<>NoUnit) AND (ThisUnit<>S2) THEN WrError(1350)
     ELSE
      BEGIN
       z:=0; ThisUnit:=S2; UnitFlag:=1;
       IF DecodeCtrlReg(ArgStr[1],S2Reg,False) THEN z:=2
       ELSE IF DecodeCtrlReg(ArgStr[2],DReg,True) THEN z:=1
       ELSE WrXError(1440,ArgStr[1]);
       IF z>0 THEN
        BEGIN
         IF DecodeAdr(ArgStr[z],MModReg,False,S1Reg) THEN
          IF (ThisCross) AND ((z=2) OR (IsCross(S1Reg))) THEN WrError(1350)
          ELSE
           BEGIN
            IF z=1 THEN
             BEGIN
              S2Reg:=S1Reg;
	      AddSrc(S2Reg); SetCross(S2Reg);
             END
            ELSE
             BEGIN
              DReg:=S1Reg; AddDest(DReg);
             END;
            CodeS($0d+z,DReg,0,S2Reg);
           END;
        END;
      END;
     Exit;
    END;

   IF (Memo('MVK')) OR (Memo('MVKH')) OR (Memo('MVKLH')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeAdr(ArgStr[2],MModReg,True,DReg) THEN
        IF ChkUnit(DReg,S1,S2) THEN
         BEGIN
          IF Memo('MVKLH') THEN S1Reg:=EvalIntExpression(ArgStr[1],Int16,OK)
          ELSE S1Reg:=EvalIntExpression(ArgStr[1],Int32,OK);
          IF OK THEN
           BEGIN
            AddDest(DReg);
            IF Memo('MVKH') THEN S1Reg:=S1Reg SHR 16;
            ThisInst:=(DReg SHL 23)+((S1Reg AND $ffff) SHL 7)+(UnitFlag SHL 1);
            IF Memo('MVK') THEN Inc(ThisInst,$28)
            ELSE Inc(ThisInst,$68);
            DecodeInst:=True;
           END;
         END;
      END;
     Exit;
    END;

   IF Memo('SHL') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[3],MModReg+MModLReg,True,DReg);
       IF (AdrMode<>ModNone) AND (ChkUnit(DReg,S1,S2)) THEN
        CASE AdrMode OF
        ModReg:
         BEGIN
          AddDest(DReg);
          IF DecodeAdr(ArgStr[1],MModReg,True,S2Reg) THEN
           IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
           ELSE
            BEGIN
             AddSrc(S2Reg);
             SetCross(S2Reg);
             DecodeAdr(ArgStr[2],MModReg+MModImm,False,S1Reg);
             CASE AdrMode OF
             ModReg:
              IF IsCross(S1Reg) THEN WrError(1350)
              ELSE
               BEGIN
                AddSrc(S1Reg);
                CodeS($33,DReg,S1Reg,S2Reg);
               END;
             ModImm:
              CodeS($32,DReg,S1Reg,S2Reg);
             END;
            END;
         END;
        ModLReg:
         BEGIN
          AddLDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModLReg,True,S2Reg);
          CASE AdrMode OF
          ModReg:
           IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
           ELSE
            BEGIN
             AddSrc(S2Reg); SetCross(S2Reg);
             DecodeAdr(ArgStr[2],MModImm+MModReg,False,S1Reg);
             CASE AdrMode OF
             ModReg:
              IF IsCross(S1Reg) THEN WrError(1350)
              ELSE
               BEGIN
                AddSrc(S1Reg);
                CodeS($13,DReg,S1Reg,S2Reg);
               END;
             ModImm:
              CodeS($12,DReg,S1Reg,S2Reg);
             END;
            END;
          ModLReg:
           IF (ThisCross) OR (IsCross(S2Reg)) THEN WrError(1350)
           ELSE
            BEGIN
             AddLSrc(S2Reg);
             DecodeAdr(ArgStr[2],MModImm+MModReg,False,S1Reg);
             CASE AdrMode OF
             ModReg:
              IF IsCross(S1Reg) THEN WrError(1350)
              ELSE
               BEGIN
                AddSrc(S1Reg);
                CodeS($31,DReg,S1Reg,S2Reg);
               END;
             ModImm:
              CodeS($30,DReg,S1Reg,S2Reg);
             END;
            END;
          END;
         END;
        END;
      END;
     Exit;
    END;

   IF (Memo('SHR')) OR (Memo('SHRU')) THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       HasSign:=Memo('SHR'); z:=Ord(Memo('SHR')) SHL 4;
       DecodeAdr(ArgStr[3],MModReg+MModLReg,HasSign,DReg);
       IF (AdrMode<>ModNone) AND (ChkUnit(DReg,S1,S2)) THEN
        CASE AdrMode OF
        ModReg:
         BEGIN
          AddDest(DReg);
          IF DecodeAdr(ArgStr[1],MModReg,HasSign,S2Reg) THEN
           IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
           ELSE
            BEGIN
             AddSrc(S2Reg); SetCross(S2Reg);
             DecodeAdr(ArgStr[2],MModReg+MModImm,False,S1Reg);
             CASE AdrMode OF
             ModReg:
              IF IsCross(S1Reg) THEN WrError(1350)
              ELSE
               BEGIN
                AddSrc(S1Reg);
                CodeS($27+z,DReg,S1Reg,S2Reg);
               END;
             ModImm:
              CodeS($26+z,DReg,S1Reg,S2Reg);
             END;
            END;
         END;
        ModLReg:
         BEGIN
          AddLDest(DReg);
          IF DecodeAdr(ArgStr[1],MModLReg,HasSign,S2Reg) THEN
           IF (ThisCross) OR (IsCross(S2Reg)) THEN WrError(1350)
           ELSE
            BEGIN
             AddLSrc(S2Reg);
             DecodeAdr(ArgStr[2],MModReg+MModImm,False,S1Reg);
             CASE AdrMode OF
             ModReg:
              IF IsCross(S1Reg) THEN WrError(1350)
              ELSE
               BEGIN
                AddSrc(S1Reg);
                CodeS($25+z,DReg,S1Reg,S2Reg);
               END;
             ModImm:
              CodeS($24+z,DReg,S1Reg,S2Reg);
             END;
            END;
         END;
        END;
      END;
     Exit;
    END;

   IF Memo('SSHL') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeAdr(ArgStr[3],MModReg,True,DReg) THEN
        IF ChkUnit(DReg,S1,S2) THEN
         BEGIN
          AddDest(DReg);
          IF DecodeAdr(ArgStr[1],MModReg,True,S2Reg) THEN
           IF (ThisCross) AND (S2Reg XOR DReg<16) THEN WrError(1350)
           ELSE
            BEGIN
             AddSrc(S2Reg); SetCross(S2Reg);
             DecodeAdr(ArgStr[2],MModReg+MModImm,False,S1Reg);
             CASE AdrMode OF
             ModReg:
              IF IsCross(S1Reg) THEN WrError(1350)
              ELSE
               BEGIN
                AddSrc(S1Reg);
                CodeS($23,DReg,S1Reg,S2Reg);
               END;
             ModImm:
              CodeS($22,DReg,S1Reg,S2Reg);
             END;
            END;
         END;
      End;
     Exit;
    END;

   IF Memo('SSUB') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[3],MModReg+MModLReg,True,DReg);
       IF (AdrMode<>ModNone) AND (ChkUnit(DReg,L1,L2)) THEN
        CASE AdrMode OF
        ModReg:
         BEGIN
          AddDest(DReg);
          DecodeAdr(ArgStr[1],MModReg+MModImm,True,S1Reg);
          CASE AdrMode OF
          ModReg:
           BEGIN
            AddSrc(S1Reg);
            IF DecodeAdr(ArgStr[2],MModReg,True,S2Reg) THEN
             IF (ThisCross) AND (S1Reg XOR DReg<16) AND (S2Reg XOR DReg<16) THEN WrError(1350)
             ELSE IF (IsCross(S1Reg)) AND (IsCross(S2Reg)) THEN WrError(1350)
             ELSE IF IsCross(S1Reg) THEN
	      BEGIN
               ThisCross:=True;
	       CodeL($1f,DReg,S1Reg,S2Reg);
	      END
             ELSE
              BEGIN
               SetCross(S2Reg);
	       CodeL($0f,DReg,S1Reg,S2Reg);
              END;
           END;
          ModImm:
           BEGIN
            IF DecodeAdr(ArgStr[2],MModReg,True,S2Reg) THEN
             IF (ThisCross) AND (DReg XOR S2Reg<16) THEN WrError(1350)
             ELSE
              BEGIN
               AddSrc(S2Reg); SetCross(S2Reg);
	       CodeL($0e,DReg,S1Reg,S2Reg);
              END;
           END;
          END;
         END;
        ModLReg:
         BEGIN
          AddLDest(DReg);
          IF DecodeAdr(ArgStr[1],MModImm,True,S1Reg) THEN
           BEGIN
            IF DecodeAdr(ArgStr[2],MModLReg,True,S2Reg) THEN
             IF (ThisCross) OR (IsCross(S2Reg)) THEN WrError(1350)
             ELSE
              BEGIN
               AddLSrc(S2Reg);
               CodeL($2c,DReg,S1Reg,S2Reg);
              END;
           END;
         END;
        END;
      END;
     Exit;
    END;

   { SprÅnge }

   { Wie zum Henker unterscheiden sich B IRP und B NRP ???
     Kann TI keine ordentlichen HandbÅcher mehr schreiben ? }

   IF Memo('B') THEN  (**)
    BEGIN
     IF ArgCnt<>1 THEN WrError(1350)
     ELSE IF ThisCross THEN WrError(1350)
     ELSE IF (ThisUnit<>NoUnit) AND (ThisUnit<>S1) AND (ThisUnit<>S2) THEN WrError(1350)
     ELSE
      BEGIN
       OK:=True; S2Reg:=0; WithImm:=False;
       IF NLS_StrCaseCmp(ArgStr[1],'IRP')=0 THEN Code1:=$03
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'NRP')=0 THEN Code1:=$03 { !!! }
       ELSE IF DecodeReg(ArgStr[1],S2Reg,OK,False) THEN
        BEGIN
         IF OK THEN WrError(1350); OK:=NOT OK;
         Code1:=$0d;
        END
       ELSE WithImm:=True;
       IF OK THEN
        IF WithImm THEN
         BEGIN
          IF ThisUnit=NoUnit THEN
           IF UnitUsed(S1) THEN ThisUnit:=S2 ELSE ThisUnit:=S1;
          UnitFlag:=Ord(ThisUnit=S2);
          Dist:=EvalIntExpression(ArgStr[1],Int32,OK)-PacketAddr;
          IF OK THEN
           IF Dist AND 3<>0 THEN WrError(1325)
           ELSE IF (NOT SymbolQuestionable) AND ((Dist>$3fffff) OR (Dist<-$400000)) THEN WrError(1370)
           ELSE
            BEGIN
             ThisInst:=$50+((Dist AND $007ffffc) SHL 5)+(UnitFlag SHL 1);
             DecodeInst:=True;
            END;
         END
        ELSE
         BEGIN
          IF ChkUnit($10,S1,S2) THEN CodeS(Code1,0,0,S2Reg);
         END
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	PROCEDURE ChkPacket;
VAR
   EndAddr,Mask:LongInt;
   z,z1,z2:LongInt;
   RegReads:ARRAY[0..31] OF Integer;
   TestUnit:String[3];
BEGIN
   { nicht Åber 8er-Grenze }

   EndAddr:=PacketAddr+((ParCnt SHL 2)-1);
   IF (PacketAddr SHR 5)<>(EndAddr SHR 5) THEN WrError(2000);

   { doppelte Units,Crosspaths,Adressierer,Zielregister }

   FOR z1:=0 TO ParCnt-1 DO
    FOR z2:=z1+1 TO ParCnt-1 DO
     IF (ParRecs[z1].OpCode SHR 28)=(ParRecs[z2].OpCode SHR 28) THEN
      BEGIN
       { doppelte Units }
       IF (ParRecs[z1].U<>NoUnit) AND (ParRecs[z1].U=ParRecs[z2].U) THEN
        WrXError(2001,UnitNames[ParRecs[z1].U]);

       { Crosspaths }
       z:=ParRecs[z1].CrossUsed AND ParRecs[z2].CrossUsed;
       IF z<>0 THEN
        BEGIN
         WrXError(2001,Chr(z+AscOfs)+'X');
        END;

       z:=ParRecs[z1].AddrUsed AND ParRecs[z2].AddrUsed;
       { Adressgeneratoren }
       IF z AND 1=1 THEN WrXError(2001,'Addr. A');
       IF z AND 2=2 THEN WrXError(2001,'Addr. B');
       { Hauptspeicherpfade }
       IF z AND 4=4 THEN WrXError(2001,'LdSt. A');
       IF z AND 8=8 THEN WrXError(2001,'LdSt. B');

       { Åberlappende Zielregister }
       z:=ParRecs[z1].DestMask AND ParRecs[z2].DestMask;
       IF z<>0 THEN WrXError(2006,RegName(FindReg(z)));

       IF (Ord(ParRecs[z1].U) AND 1)=(Ord(ParRecs[z2].U) AND 1) THEN
        BEGIN
         TestUnit:=Chr(Ord(ParRecs[z1].U)-Ord(NoUnit)-1+Ord('A'));

         { mehrere Long-Reads }
         IF (ParRecs[z1].LongSrc) AND (ParRecs[z2].LongSrc) THEN
          WrXError(2002,TestUnit);

         { mehrere Long-Writes }
         IF (ParRecs[z1].LongDest) AND (ParRecs[z2].LongDest) THEN
          WrXError(2003,TestUnit);

         { Long-Read mit Store }
         IF (ParRecs[z1].StoreUsed) AND (ParRecs[z2].LongSrc) THEN
          WrXError(2004,TestUnit);
         IF (ParRecs[z2].StoreUsed) AND (ParRecs[z1].LongSrc) THEN
          WrXError(2004,TestUnit);
        END;
      END;

   FOR z2:=0 TO 31 DO
    RegReads[z2]:=0;
   FOR z1:=0 TO ParCnt-1 DO
    WITH ParRecs[z1] DO
     BEGIN
      Mask:=1;
      FOR z2:=0 TO 31 DO
       BEGIN
        IF SrcMask AND Mask<>0 THEN Inc(RegReads[z2]);
        IF SrcMask2 AND Mask<>0 THEN Inc(RegReads[z2]);
        Mask:=Mask SHL 1;
       END;
     END;

   { Register mehr als 4mal gelesen }

   FOR z1:=0 TO 31 DO
    IF RegReads[z1]>4 THEN WrXError(2005,RegName(z1));

END;

	PROCEDURE MakeCode_3206X;
	Far;
VAR
   z:Integer;
BEGIN
   CodeLen:=0; DontPrint:=False;

   { zu ignorierendes }

   IF (Memo('')) AND (LabPart='') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { Flags zurÅcksetzen }

   ThisPar:=False; Condition:=0;

   { Optionen aus Label holen }

   IF LabPart<>'' THEN
    IF (LabPart='||') OR (LabPart[1]='[') THEN
     IF NOT CheckOpt(LabPart) THEN Exit;

   { eventuell falsche Mnemonics verwerten }

   IF OpPart='||' THEN
    IF NOT ReiterateOpPart THEN Exit;
   IF OpPart[1]='[' THEN
    IF NOT ReiterateOpPart THEN Exit;

   IF Memo('') THEN Exit;

   { Attribut auswerten }

   ThisUnit:=NoUnit; ThisCross:=False;
   IF AttrPart<>'' THEN
    BEGIN
     IF UpCase(AttrPart[Length(AttrPart)])='X' THEN
      BEGIN
       ThisCross:=True;
       Dec(Byte(AttrPart[0]));
      END;
     IF AttrPart='' THEN ThisUnit:=NoUnit
     ELSE
      REPEAT
       ThisUnit:=Succ(ThisUnit);
      UNTIL (ThisUnit=LastUnit) OR (NLS_StrCaseCmp(AttrPart,UnitNames[ThisUnit])=0);
     IF ThisUnit=LastUnit THEN
      BEGIN
       WrError(1107); Exit;
      END;
     IF ((ThisUnit=D1) OR (ThisUnit=D2)) AND (ThisCross) THEN
      BEGIN
       WrError(1350); Exit;
      END;
    END;

   { falls nicht parallel, vorherigen Stack durchpruefen und verwerfen }

   IF (NOT ThisPar) AND (ParCnt>0) THEN
    BEGIN
     ChkPacket;
     ParCnt:=0; PacketAddr:=EProgCounter;
    END;

   { dekodieren }

   ThisSrc:=0; ThisSrc2:=0; ThisDest:=0;
   ThisAddr:=0; ThisStore:=False; ThisLong:=0;
   IF NOT DecodeInst THEN Exit;

   { einsortieren }

   WITH ParRecs[ParCnt] DO
    BEGIN
     OpCode:=(Condition SHL 28)+ThisInst;
     U:=ThisUnit;
     IF ThisCross THEN
      CASE ThisUnit OF
      L1,S1,M1,D1: CrossUsed:=1;
      ELSE CrossUsed:=2;
      END
     ELSE CrossUsed:=0;
     AddrUsed:=ThisAddr;
     SrcMask:=ThisSrc; SrcMask2:=ThisSrc2; DestMask:=ThisDest;
     LongSrc:=(ThisLong AND 1)=1; LongDest:=(ThisLong AND 2)=2;
     StoreUsed:=ThisStore;
    END;
   Inc(ParCnt);

   { wenn mehr als eine Instruktion, Ressourcenkonflikte abklopfen und
     vorherige Instruktion zurÅcknehmen }

   IF ParCnt>1 THEN
    BEGIN
     RetractWords(4);
     DAsmCode[CodeLen SHR 2]:=ParRecs[ParCnt-2].OpCode OR 1;
     Inc(CodeLen,4);
    END;

   { aktuelle Instruktion auswerfen: fÅr letzte kein Parallelflag setzen }

   DAsmCode[CodeLen SHR 2]:=ParRecs[ParCnt-1].OpCode;
   Inc(CodeLen,4);
END;

{--------------------------------------------------------------------------}

	FUNCTION ChkPC_3206X:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   ChkPC_3206X:=(ActPC=SegCode);
END;


	FUNCTION IsDef_3206X:Boolean;
	Far;
BEGIN
   IsDef_3206X:=(LabPart='||') OR (LabPart[1]='[');
END;

        PROCEDURE SwitchFrom_3206X;
        Far;
BEGIN
   IF ParCnt>1 THEN ChkPacket;
   DeinitFields;
END;

	PROCEDURE SwitchTo_3206X;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$47; NOPCode:=$00000000;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.';
   SetIsoccupied:=True;

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=4; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_3206X; ChkPC:=ChkPC_3206X; IsDef:=IsDef_3206X;
   SwitchFrom:=SwitchFrom_3206X; InitFields;

   ParCnt:=0; PacketAddr:=0;
END;

BEGIN
   CPU32060:=AddCPU('32060',SwitchTo_3206X);
END.
