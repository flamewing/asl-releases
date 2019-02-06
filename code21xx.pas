{$i STDINC.PAS}
{$IFDEF SEGATTRS}
{$g+,o-,f-,C Moveable DemandLoad Discardable }
{$ENDIF}

	UNIT Code21xx;

{ AS - Codegenerator Analog Devices ADSP21xx (yes, they make digital chips :-)) }
{ (C) 1995 by Thomas Sailer }

INTERFACE
        Uses AsmDef,AsmSub,AsmPars,AsmCode,CodePseu,AsmMac;


IMPLEMENTATION

VAR
   CPU2100,CPU2101,CPU2103,CPU2105,CPU2111,CPU2115:CPUVar;
   SaveInitProc:PROCEDURE;

   NextComment:Boolean;

   InstrWordOk:Boolean;
   InstrWord:LongInt;

{---------------------------------------------------------------------------}

	PROCEDURE InitFields;
BEGIN
END;

	PROCEDURE DeinitFields;
BEGIN
END;

{---------------------------------------------------------------------------}
{---------------------------------------------------------------------------}

        FUNCTION GetDAGGroup(VAR s:String;VAR G,I,M:Byte):Boolean;
BEGIN
   GetDAGGroup:=False;
   IF(Length(s)<7) OR (s[1]<>'(') OR (s[2]<>'I') OR (s[4]<>',') OR (s[5]<>'M') OR (s[7]<>')') THEN Exit;
   I:=ORD(s[3])-ORD('0');
   IF I>7 THEN Exit;
   G:=0;
   IF I>3 THEN G:=1;
   M:=ORD(s[6])-ORD('0');
   IF M>7 THEN Exit;
   IF (M AND 4) <> (I AND 4) THEN Exit; (* Index and Modify not from the same DAG *)
   I:=I AND 3;
   M:=M AND 3;
   Delete(s,1,7);
   GetDAGGroup:=True;
END;

        FUNCTION GetCond(VAR s:String; term:Boolean):Byte;
(* term is true for DO UNTIL cond and false for IF cond *)
VAR
   cs:String[3];
BEGIN
   GetCond:=15; (* default is FOREVER (Unconditional) *)
   KillPrefBlanks(s);
   IF term AND (Copy(s,1,7)='FOREVER') THEN
    BEGIN
     Delete(s,1,7);
     KillPrefBlanks(s);
     Exit;
    END;
   IF (NOT term) AND (Copy(s,1,4)='TRUE') THEN
    BEGIN
     Delete(s,1,4);
     KillPrefBlanks(s);
     Exit;
    END;
   IF(Copy(s,1,3)='NOT') THEN
    BEGIN
     Delete(s,1,3);
     KillPrefBlanks(s);
     cs:=Copy(s,1,2);
     IF(cs='EQ') THEN GetCond:=1 ELSE
     IF(cs='AV') THEN GetCond:=7 ELSE
     IF(cs='AC') THEN GetCond:=9 ELSE
     IF(cs='MV') THEN GetCond:=13 ELSE
     IF (NOT term) AND (cs='CE') THEN GetCond:=14 ELSE
      BEGIN
       WrXError(1200,'NOT '+cs);
       Exit;
      END;
     Delete(s,1,2);
     KillPrefBlanks(s);
     Exit;
    END;
   cs:=Copy(s,1,3);
   IF(cs='NEG') THEN GetCond:=10 ELSE
   IF(cs='POS') THEN GetCond:=11 ELSE
    BEGIN
     cs:=Copy(s,1,2);
     IF(cs='EQ') THEN GetCond:=0 ELSE
     IF(cs='NE') THEN GetCond:=1 ELSE
     IF(cs='GT') THEN GetCond:=2 ELSE
     IF(cs='LE') THEN GetCond:=3 ELSE
     IF(cs='LT') THEN GetCond:=4 ELSE
     IF(cs='GE') THEN GetCond:=5 ELSE
     IF(cs='AV') THEN GetCond:=6 ELSE
     IF(cs='AC') THEN GetCond:=8 ELSE
     IF(cs='MV') THEN GetCond:=12 ELSE
     IF term AND (cs='CE') THEN GetCond:=14 ELSE
      BEGIN
       WrXError(1200,cs);
       Exit;
      END;
     Delete(s,1,2);
     KillPrefBlanks(s);
     Exit;
    END;
   Delete(s,1,3);
   KillPrefBlanks(s);
END;

        FUNCTION GetReg(VAR s:String;write:Boolean;VAR RGP,REG:Byte):Boolean;
TYPE
   TReg=String[7];
   TRgp=ARRAY[0..15] OF TReg;
   TAll=ARRAY[0..3] OF TRgp;
CONST (* SSTAT is read only, and IFC write only *)
   kRegs:Tall=(('AX0','AX1','MX0','MX1','AY0','AY1','MY0','MY1',
                'SI','SE','AR','MR0','MR1','MR2','SR0','SR1'),
               ('I0','I1','I2','I3','M0','M1','M2','M3',
                'L0','L1','L2','L3','','','',''),
               ('I4','I5','I6','I7','M4','M5','M6','M7',
                'L4','L5','L6','L7','','','',''),
               ('ASTAT','MSTAT','SSTAT','IMASK','ICNTL','CNTR','SB','PX',
                'RX0','TX0','RX1','TX1','IFC','OWRCNTR','',''));
VAR
   fatal,fatal2:String;
BEGIN
   RGP:=0;
   WHILE(RGP<4) DO
    BEGIN
     REG:=0;
     WHILE(REG<16) DO
      BEGIN
       IF(Length(kRegs[RGP,REG])>0) AND (Copy(s,1,Length(kRegs[RGP,REG]))=kRegs[RGP,REG]) THEN
        BEGIN
         Delete(s,1,Length(kRegs[RGP,REG]));
         fatal2:='';
         IF(RGP=3)AND(REG=2)AND write THEN
          BEGIN
           fatal:='Register SSTAT is read only';
           WrErrorString(fatal,fatal2,True,False)
          END ELSE
         IF(RGP=3)AND(REG=12)AND (NOT write) THEN
          BEGIN
           fatal:='Register IFC is write only';
           WrErrorString(fatal,fatal2,True,False);
          END;
         GetReg:=True;
         Exit;
        END;
       Inc(REG);
      END;
     Inc(RGP);
    END;
   GetReg:=False;
END;

        FUNCTION GetXOP(VAR s:String;xunit:Byte;VAR xop:Byte):Boolean;
TYPE
   TXop=String[3];
   TXopc=ARRAY[0..7] OF TXop;
   TAllX=ARRAY[0..2] OF TXopc;
CONST
   kXops:TAllX=(('AX0','AX1','AR','MR0','MR1','MR2','SR0','SR1'),
                ('MX0','MX1','AR','MR0','MR1','MR2','SR0','SR1'),
                ('SI','','AR','MR0','MR1','MR2','SR0','SR1'));
BEGIN
   xop:=0;
   WHILE(xop<8) DO
    BEGIN
     IF(Length(kXops[xunit,xop])>0) AND (Copy(s,1,Length(kXops[xunit,xop]))=kXops[xunit,xop]) THEN
      BEGIN
       GetXOP:=TRUE;
       Delete(s,1,Length(kXops[xunit,xop]));
       Exit;
      END;
     Inc(xop);
    END;
   GetXOP:=False;
END;

        FUNCTION GetYOP(VAR s:String;xunit:Byte;VAR yop:Byte):Boolean;
TYPE
   TYop=String[3];
   TYopc=ARRAY[0..3] OF TYop;
   TAllY=ARRAY[0..1] OF TYopc;
CONST
   kYops:TAllY=(('AY0','AY1','AF','0'),
                ('MY0','MY1','MF','0'));
BEGIN
   yop:=0;
   WHILE(yop<4) DO
    BEGIN
     IF(Length(kYops[xunit,yop])>0) AND (Copy(s,1,Length(kYops[xunit,yop]))=kYops[xunit,yop]) THEN
      BEGIN
       GetYOP:=TRUE;
       Delete(s,1,Length(kYops[xunit,yop]));
       Exit;
      END;
     Inc(yop);
    END;
   GetYOP:=False;
END;


{---------------------------------------------------------------------------}

        FUNCTION DoNop(VAR s:String):Boolean;
BEGIN
   DoNop:=False;
   IF(Copy(s,1,3)<>'NOP') THEN Exit;
   InstrWordOk:=True;
   InstrWord:=0;
   Delete(s,1,3);
   KillBlanks(s);
   IF s<>'' THEN WrXError(1140,s);
   DoNop:=True;
END;

        FUNCTION DoPopPush(VAR s:String):Boolean;
BEGIN
   DoPopPush:=False;
   IF (Copy(s,1,3)<>'POP') AND (Copy(s,1,4)<>'PUSH') THEN Exit;
   DoPopPush:=True;
   KillBlanks(s);
   InstrWordOk:=True;
   InstrWord:=$040000;
   WHILE Length(s)>0 DO
    BEGIN
     IF(s[1]=',') THEN Delete(s,1,1) ELSE
     IF(Copy(s,1,7)='POPCNTR') THEN BEGIN Delete(s,1,7); InstrWord:=InstrWord OR $4; END ELSE
     IF(Copy(s,1,7)='POPLOOP') THEN BEGIN Delete(s,1,7); InstrWord:=InstrWord OR $8; END ELSE
     IF(Copy(s,1,5)='POPPC') THEN BEGIN Delete(s,1,5); InstrWord:=InstrWord OR $10; END ELSE
     IF(Copy(s,1,6)='POPSTS') THEN BEGIN Delete(s,1,6); InstrWord:=(InstrWord AND $FFFFFC) OR $3; END ELSE
     IF(Copy(s,1,7)='PUSHSTS') THEN BEGIN Delete(s,1,7); InstrWord:=(InstrWord AND $FFFFFC) OR $2; END ELSE
      BEGIN
       WrXError(1140,s);
       s:='';
      END;
    END;
END;

        FUNCTION DoEnaDis(VAR s:String):Boolean;
VAR
   mc:Byte;
BEGIN
   DoEnaDis:=False;
   IF (Copy(s,1,3)<>'ENA') AND (Copy(s,1,3)<>'DIS') THEN Exit;
   DoEnaDis:=True;
   KillBlanks(s);
   InstrWordOk:=True;
   InstrWord:=$0C0000;
   mc:=0;
   WHILE Length(s)>0 DO
    BEGIN
     IF(s[1]=',') THEN Delete(s,1,1) ELSE
     IF(Copy(s,1,3)='ENA') THEN BEGIN Delete(s,1,3); mc:=3; END ELSE
     IF(Copy(s,1,3)='DIS') THEN BEGIN Delete(s,1,3); mc:=2; END ELSE
     IF(Copy(s,1,7)='BIT_REV') THEN BEGIN Delete(s,1,7); InstrWord:=(InstrWord AND $FFFF3F) OR (mc SHL 6); END ELSE
     IF(Copy(s,1,8)='AV_LATCH') THEN BEGIN Delete(s,1,8); InstrWord:=(InstrWord AND $FFFCFF) OR (mc SHL 8); END ELSE
     IF(Copy(s,1,6)='AR_SAT') THEN BEGIN Delete(s,1,6); InstrWord:=(InstrWord AND $FFF3FF) OR (mc SHL 10); END ELSE
     IF(Copy(s,1,7)='SEC_REG') THEN BEGIN Delete(s,1,7); InstrWord:=(InstrWord AND $FFFFCF) OR (mc SHL 4); END ELSE
     IF(Copy(s,1,6)='G_MODE') THEN BEGIN Delete(s,1,6); InstrWord:=(InstrWord AND $FFFFF3) OR (mc SHL 2); END ELSE
     IF(Copy(s,1,6)='M_MODE') THEN BEGIN Delete(s,1,6); InstrWord:=(InstrWord AND $FFCFFF) OR (mc SHL 12); END ELSE
     IF(Copy(s,1,5)='TIMER') THEN BEGIN Delete(s,1,5); InstrWord:=(InstrWord AND $FF3FFF) OR (LongInt(mc) SHL 14); END ELSE
      BEGIN
       WrXError(1200,s);
       s:='';
      END;
    END;
END;

        FUNCTION DoSatMR(s:String):Boolean;
BEGIN
   DoSatMR:=False;
   KillBlanks(s);
   IF(s<>'SATMR') THEN Exit;
   InstrWordOk:=True;
   InstrWord:=$050000;
   DoSatMR:=True;
END;

        FUNCTION DoFlagInJump(s:String):Boolean;
VAR
   x:LongInt;
BEGIN
   DoFlagInJump:=False;
   KillBlanks(s);
   IF(Copy(s,1,13)='IFFLAG_INJUMP') THEN
    BEGIN
     Delete(s,1,13);
     InstrWordOk:=True;
     InstrWord:=$030002;
    END;
   IF(Copy(s,1,16)='IFNOTFLAG_INJUMP') THEN
    BEGIN
     Delete(s,1,16);
     InstrWordOk:=True;
     InstrWord:=$030000;
    END;
   IF(Copy(s,1,13)='IFFLAG_INCALL') THEN
    BEGIN
     Delete(s,1,13);
     InstrWordOk:=True;
     InstrWord:=$030003;
    END;
   IF(Copy(s,1,16)='IFNOTFLAG_INCALL') THEN
    BEGIN
     Delete(s,1,16);
     InstrWordOk:=True;
     InstrWord:=$030001;
    END;
   IF NOT InstrWordOk THEN Exit;
   DoFlagInJump:=True;
   x:=EvalIntExpression(s,UInt16,InstrWordOk);
   IF NOT InstrWordOk THEN BEGIN WrXError(1020,s); Exit; END;
   IF FirstPassUnknown THEN Exit;
   IF x>$3FFF THEN BEGIN WrXError(1320,s); Exit; END;
   InstrWord := InstrWord OR ((x SHR 10) AND $c) OR ((x AND $FFF) SHL 4);
END;

        FUNCTION DoIdle(VAR s:String):Boolean;
VAR
   x:LongInt;
   ok:Boolean;
BEGIN
   DoIdle:=False;
   IF(Copy(s,1,4)<>'IDLE') THEN Exit;
   InstrWordOk:=True;
   InstrWord:=$028000;
   KillBlanks(s);
   Delete(s,1,4);
   IF s<>'' THEN
    BEGIN
     x:=EvalIntExpression(s,UInt8,ok);
     IF NOT Ok THEN WrXError(1200,s);
     IF MomCPU=CPU2100 THEN WrError(1500);
     IF NOT FirstPassUnknown THEN
      BEGIN
       IF x=16 THEN InstrWord:=InstrWord OR 1 ELSE
       IF x=32 THEN InstrWord:=InstrWord OR 2 ELSE
       IF x=64 THEN InstrWord:=InstrWord OR 4 ELSE
       IF x=128 THEN InstrWord:=InstrWord OR 8 ELSE WrXError(1985,s);
      END;
    END;
   DoIdle:=True;
END;

        FUNCTION DoTrap(VAR s:String;cond:Byte):Boolean;
BEGIN
   DoTrap:=False;
   IF(Copy(s,1,4)<>'TRAP') THEN Exit;
   InstrWordOk:=True;
   InstrWord:=$080000 OR cond;
   Delete(s,1,4);
   KillBlanks(s);
   IF s<>'' THEN WrXError(1140,s);
   DoTrap:=True;
END;

        FUNCTION DoModify(VAR s:String):Boolean;
VAR
   G,I,M : Byte;
BEGIN
   DoModify:=False;
   IF(Copy(s,1,6)<>'MODIFY') THEN Exit;
   InstrWordOk:=True;
   KillBlanks(s);
   Delete(s,1,6);
   IF NOT GetDAGGroup(s,G,I,M) THEN Exit;
   IF s<>'' THEN Exit;
   InstrWord:=$090000 OR M OR (I SHL 2) OR (G SHL 4);
   DoModify:=True;
END;

        FUNCTION DoFlagOut(VAR s:String;cond:Byte):Boolean;
VAR
   FO:Byte;
BEGIN
   DoFlagOut:=False;
   IF(Copy(s,1,3)<>'SET') AND (Copy(s,1,5)<>'RESET') AND (Copy(s,1,6)<>'TOGGLE') THEN Exit;
   InstrWordOk:=True;
   KillBlanks(s);
   InstrWord:=$020000 OR cond;
   FO:=0;
   WHILE s<>'' DO
    BEGIN
     IF (Length(s)>=1) AND (s[1]=',') THEN Delete(s,1,1) ELSE
     IF Copy(s,1,3)='SET' THEN BEGIN Delete(s,1,3); FO:=3; END ELSE
     IF Copy(s,1,5)='RESET' THEN BEGIN Delete(s,1,5); FO:=2; END ELSE
     IF Copy(s,1,6)='TOGGLE' THEN BEGIN Delete(s,1,6); FO:=1; END ELSE
     IF Copy(s,1,8)='FLAG_OUT' THEN BEGIN Delete(s,1,8); InstrWord:=InstrWord AND $FFFFCF OR (FO SHL 4); END ELSE
     IF Copy(s,1,3)='FL0' THEN BEGIN Delete(s,1,3); InstrWord:=InstrWord AND $FFFF3F OR (FO SHL 6); END ELSE
     IF Copy(s,1,3)='FL1' THEN BEGIN Delete(s,1,3); InstrWord:=InstrWord AND $FFFCFF OR (FO SHL 8); END ELSE
     IF Copy(s,1,3)='FL2' THEN BEGIN Delete(s,1,3); InstrWord:=InstrWord AND $FFF3FF OR (FO SHL 10); END ELSE
      BEGIN
       WrXError(1140,s);
       s:='';
      END;
    END;
   IF (MomCPU<>CPU2111) AND ((InstrWord AND $FC0)<>0) THEN WrError(1500);
   InstrWordOk:=True;
   DoFlagOut:=True;
END;

        FUNCTION DoReturn(VAR s:String;cond:Byte):Boolean;
BEGIN
   DoReturn:=False;
   IF(Copy(s,1,3)='RTS') THEN InstrWord:=$0A0000 OR cond ELSE
   IF(Copy(s,1,3)='RTI') THEN InstrWord:=$0A0010 OR cond ELSE Exit;
   InstrWordOk:=True;
   Delete(s,1,3);
   KillBlanks(s);
   IF s<>'' THEN WrXError(1140,s);
   DoReturn:=True;
END;

        FUNCTION DoJump(VAR s:String;cond:Byte):Boolean;
VAR
   sub:Byte;
   x:LongInt;
BEGIN
   DoJump:=False;
   IF(Copy(s,1,4)='JUMP') THEN sub:=0 ELSE
   IF(Copy(s,1,4)='CALL') THEN sub:=1 ELSE Exit;
   InstrWordOk:=True;
   InstrWord:=$180000;
   DoJump:=True;
   Delete(s,1,4);
   KillBlanks(s);
   IF(Length(s)=4)AND(s[1]='(')AND(s[2]='I')AND(s[4]=')')AND(s[3]>='4')AND(s[3]<='7') THEN
    BEGIN
     InstrWord:=$0B0000 OR cond OR (((ORD(s[3])-ORD('4')) AND 3) SHL 6) OR (sub SHL 4);
     Exit;
    END;
   x:=EvalIntExpression(s,UInt16,InstrWordOk);
   IF NOT InstrWordOk THEN BEGIN WrXError(1140,s); Exit; END;
   IF FirstPassUnknown THEN Exit;
   IF x>$3FFF THEN BEGIN WrXError(1320,s); Exit; END;
   InstrWord:=$180000 OR (LongInt(sub) SHL 18) OR ((LongInt(x) AND $3FFF) SHL 4) OR cond;
END;

        FUNCTION DoDoUntil(VAR s:String):Boolean;
VAR
   i:Integer;
   x:LongInt;
   cond:Byte;
   cs:String[15];
BEGIN
   DoDoUntil:=False;
   IF(Copy(s,1,2)<>'DO') THEN Exit;
   DoDoUntil:=True;
   InstrWordOk:=True;
   Delete(s,1,2);
   cond:=15;
   i:=Length(s)-6;
   WHILE(i>0) DO
    BEGIN
     IF(Copy(s,i,5)='UNTIL') THEN
      BEGIN
       cs:=Copy(s,i+5,15);
       Byte(s[0]):=i-1;
       cond:=GetCond(cs,TRUE);
       i:=1;
      END;
     Dec(i);
    END;
   KillBlanks(s);
   x:=EvalIntExpression(s,UInt16,InstrWordOk);
   IF NOT InstrWordOk THEN BEGIN WrXError(1020,s); Exit; END;
   InstrWord:=$140000 OR cond;
   IF FirstPassUnknown THEN Exit;
   IF x>$3FFF THEN BEGIN WrXError(1320,s); Exit; END;
   InstrWord:=InstrWord OR ((LongInt(x) AND $3FFF) SHL 4);
END;

        FUNCTION DoIntDataMove(s:String):Boolean;
VAR
   DRGP,DREG,SRGP,SREG:Byte;
   x:LongInt;
BEGIN
   DoIntDataMove:=False;
   IF NOT GetReg(s,TRUE,DRGP,DREG) THEN Exit;
   KillBlanks(s);
   IF s[1]<>'=' THEN Exit;
   Delete(s,1,1);
   IF GetReg(s,FALSE,SRGP,SREG) THEN
    BEGIN
     IF s<>'' THEN Exit;
     DoIntDataMove:=True;
     InstrWordOk:=True;
     InstrWord:=$0D0000 OR SREG OR (DREG SHL 4) OR (LongInt(SRGP) SHL 8) OR (LongInt(DRGP) SHL 10);
    END;
END;

        FUNCTION DoRegImmed(s:String):Boolean;
VAR
   RGP,REG:Byte;
   x:LongInt;
BEGIN
   DoRegImmed:=False;
   IF NOT GetReg(s,TRUE,RGP,REG) THEN Exit;
   KillBlanks(s);
   IF s[1]<>'=' THEN Exit;
   Delete(s,1,1);
   x:=EvalIntExpression(s,Int16,InstrWordOk);
   IF NOT InstrWordOk THEN Exit;
   IF (NOT FirstPassUnknown) AND (RGP>0) AND (x>$3FFF) THEN WrXError(1320,s);
   IF RGP = 0 THEN InstrWord:=$400000 OR REG OR ((x AND $FFFF) SHL 4)
              ELSE InstrWord:=$300000 OR (LongInt(RGP) SHL 18) OR ((x AND $3FFF) SHL 4) OR REG;
   DoRegImmed:=True;
END;

        FUNCTION GetParDataMove(s:String; VAR code:Byte):Boolean;
VAR
   DRGP,DREG,SRGP,SREG:Byte;
BEGIN
   GetParDataMove:=False;
   IF NOT GetReg(s,TRUE,DRGP,DREG) THEN Exit;
   KillBlanks(s);
   IF s[1]<>'=' THEN Exit;
   Delete(s,1,1);
   IF NOT GetReg(s,FALSE,SRGP,SREG) THEN Exit;
   IF (s<>'') OR (DRGP<>0) OR (SRGP<>0) THEN Exit;
   code:=(DREG SHL 4) OR SREG;
   GetParDataMove:=True;
END;

        FUNCTION GetParDMRW(s:String; VAR D,G,code:Byte):Boolean;
VAR
   RGP,REG,I,M:Byte;
BEGIN
   GetParDMRW:=False;
   D:=0;
   G:=0;
   IF NOT GetReg(s,TRUE,RGP,REG) THEN (* might be write *)
    BEGIN
     D:=1;
     IF (Copy(s,1,2)<>'DM') THEN Exit;
     Delete(s,1,2);
     KillBlanks(s);
     IF NOT GetDAGGroup(s,G,I,M) THEN Exit;
     IF s[1]<>'=' THEN Exit;
     Delete(s,1,1);
     IF NOT GetReg(s,FALSE,RGP,REG) THEN Exit;
     IF s<>'' THEN Exit;
     GetParDMRW:=True;
     Code:=(REG SHL 4) OR (I SHL 2) OR M;
     Exit;
    END;
   KillBlanks(s);
   IF Copy(s,1,3)<>'=DM' THEN Exit;
   Delete(s,1,3);
   IF NOT GetDAGGroup(s,G,I,M) THEN Exit;
   IF s<>'' THEN Exit;
   Code:=(REG SHL 4) OR (I SHL 2) OR M;
   GetParDMRW:=True;
END;

        FUNCTION GetParPMRW(s:String; VAR D,code:Byte):Boolean;
VAR
   G,RGP,REG,I,M:Byte;
   x:LongInt;
BEGIN
   GetParPMRW:=False;
   D:=0;
   G:=0;
   IF NOT GetReg(s,TRUE,RGP,REG) THEN (* might be write *)
    BEGIN
     D:=1;
     IF (Copy(s,1,2)<>'PM') THEN Exit;
     Delete(s,1,2);
     KillBlanks(s);
     IF NOT GetDAGGroup(s,G,I,M) THEN Exit;
     IF s[1]<>'=' THEN Exit;
     Delete(s,1,1);
     IF NOT GetReg(s,FALSE,RGP,REG) THEN Exit;
     IF s<>'' THEN Exit;
     GetParPMRW:=True;
     Code:=(REG SHL 4) OR (I SHL 2) OR M;
     Exit;
    END;
   KillBlanks(s);
   IF Copy(s,1,3)<>'=PM' THEN Exit;
   Delete(s,1,3);
   IF NOT GetDAGGroup(s,G,I,M) THEN Exit;
   IF (s<>'') OR (G<>1) THEN Exit;
   Code:=(REG SHL 4) OR (I SHL 2) OR M;
   GetParPMRW:=True;
END;

        FUNCTION GetParRdDP(s:String;var PD,DD,PMI,PMM,DMI,DMM : Byte):Boolean;
VAR
   sp,sd,sx:String;
   i:Integer;
   G:Byte;
BEGIN
   GetParRdDP:=False;
   IF ((s[1]<>'M') AND (s[1]<>'A')) OR ((s[2]<>'X') AND (s[2]<>'Y')) THEN Exit;
   KillBlanks(s);
   i:=QuotPos(s,',');
   sp:=copy(s,1,i-1);
   sd:=copy(s,i+1,255);
   IF (Length(sp)<12) OR (Length(sd)<12) THEN Exit;
   IF sp[2] = 'X' THEN BEGIN sx:=sp; sp:=sd; sd:=sx; END;
   IF sp[2] <> 'Y' THEN Exit;
   IF sp[1] = 'A' THEN PD:=0 ELSE IF sp[1] = 'M' THEN PD:=2 ELSE Exit;
   IF (sp[3]<'0') OR (sp[3]>'1') OR (sp[4]<>'=') OR (sp[5]<>'P') OR (sp[6]<>'M') THEN Exit;
   PD:=PD+ORD(sp[3])-ORD('0');
   Delete(sp,1,6);
   IF sd[2] <> 'X' THEN Exit;
   IF sd[1] = 'A' THEN DD:=0 ELSE IF sd[1] = 'M' THEN DD:=2 ELSE Exit;
   IF (sd[3]<'0') OR (sd[3]>'1') OR (sd[4]<>'=') OR (sd[5]<>'D') OR (sd[6]<>'M') THEN Exit;
   DD:=DD+ORD(sd[3])-ORD('0');
   Delete(sd,1,6);
   IF NOT GetDAGGroup(sd,G,DMI,DMM) THEN Exit;
   IF (G<>0) OR (sd<>'') THEN Exit; (* Wrong DAG group *)
   IF NOT GetDAGGroup(sp,G,PMI,PMM) THEN Exit;
   IF (G<>1) OR (sp<>'') THEN Exit; (* Wrong DAG group *)
   GetParRdDP:=True;
END;

        FUNCTION DoShifter(s:String;cond:Byte):Boolean;
VAR
   D,G,code,xop:Byte;
   sp:String;
   i:Integer;
   x:LongInt;
BEGIN
   DoShifter:=False;
   IF s[1]<>'S' THEN
    BEGIN
     i:=QuotPos(s,',');
     IF (i>=Length(s)) OR (i<=0) THEN Exit;
     sp:=s;
     s:=Copy(sp,i+1,255);
     sp[0]:=Chr(i-1);
     KillPrefBlanks(s);
     IF s[1]<>'S' THEN Exit;
     s:=s+','+sp;
    END;
   KillBlanks(s);
   InstrWord:=-1;
   IF Copy(s,1,9)='SR=ASHIFT' THEN BEGIN InstrWord:=$002000; Delete(s,1,9); END;
   IF Copy(s,1,13)='SR=SRORASHIFT' THEN BEGIN InstrWord:=$002800; Delete(s,1,13); END;
   IF Copy(s,1,9)='SR=LSHIFT' THEN BEGIN InstrWord:=$000000; Delete(s,1,9); END;
   IF Copy(s,1,13)='SR=SRORLSHIFT' THEN BEGIN InstrWord:=$000800; Delete(s,1,13); END;
   IF Copy(s,1,7)='SR=NORM' THEN BEGIN InstrWord:=$004000; Delete(s,1,7); END;
   IF Copy(s,1,11)='SR=SRORNORM' THEN BEGIN InstrWord:=$004800; Delete(s,1,11); END;
   IF Copy(s,1,6)='SE=EXP' THEN BEGIN InstrWord:=$006000; Delete(s,1,6); END;
   IF Copy(s,1,9)='SB=EXPADJ' THEN BEGIN InstrWord:=$007800; Delete(s,1,9); END;
   IF InstrWord=-1 THEN Exit;
   i:=QuotPos(s,',');
   sp:=Copy(s,i+1,255);
   Delete(s,i,255);
   IF NOT GetXOP(s,2,xop) THEN Exit;
   IF Copy(s,Length(s)-3,4)='(HI)' THEN
    BEGIN
     Dec(s[0],4);
     IF InstrWord=$7800 THEN Exit;
    END ELSE IF Copy(s,Length(s)-3,4)='(LO)' THEN BEGIN
     Dec(s[0],4);
     IF InstrWord=$7800 THEN Exit;
     InstrWord:=InstrWord OR $001000;
    END ELSE IF Copy(s,Length(s)-4,5)='(HIX)' THEN BEGIN
     Dec(s[0],5);
     IF InstrWord<>$6000 THEN Exit;
     InstrWord:=$006800;
    END ELSE IF InstrWord<$7800 THEN Exit; (* specification of (LO), (HI) or (HIX) is mandatory!! *)
   IF Copy(s,1,2)='BY' THEN
    BEGIN
     IF cond<>15 THEN Exit;
     Delete(s,1,2);
     x:=EvalIntExpression(s,Int8,InstrWordOk);
     IF NOT InstrWordOk THEN Exit;
     IF sp<>'' THEN Exit;
     InstrWord:=InstrWord OR (x AND $ff) OR $0F0000 OR (LongInt(xop) SHL 8);
     DoShifter:=True;
     Exit;
    END;
   IF (s<>'') THEN Exit;
   IF (cond<>15) OR (sp='') THEN
    BEGIN
     IF (sp<>'') THEN Exit;
     InstrWord:=InstrWord OR cond OR $0E0000 OR (LongInt(xop) SHL 8);
     InstrWordOk:=True;
     DoShifter:=True;
     Exit;
    END;
   IF GetParDataMove(sp,code) THEN
    BEGIN
     InstrWord:=InstrWord OR code OR $100000 OR (LongInt(xop) SHL 8);
     InstrWordOk:=True;
     DoShifter:=True;
     Exit;
    END;
   IF GetParDMRW(sp,D,G,code) THEN
    BEGIN
     InstrWord:=InstrWord OR code OR $120000 OR (LongInt(xop) SHL 8) OR (LongInt((G SHL 1) OR D) SHL 15);
     InstrWordOk:=True;
     DoShifter:=True;
     Exit;
    END;
   IF GetParPMRW(sp,D,code) THEN
    BEGIN
     InstrWord:=InstrWord OR code OR $110000 OR (LongInt(xop) SHL 8) OR (LongInt(D) SHL 15);
     InstrWordOk:=True;
     DoShifter:=True;
     Exit;
    END;
END;

        FUNCTION GetAluMacFcn(VAR s:String;VAR amf,xop,yop:Byte):BOOLEAN;
CONST
   kNumAluMac=40;
TYPE
   TOneAluMac=RECORD
                Code:Byte;
                St:String[16];
              END;
   TAluMacCodes=ARRAY[1..kNumAluMac] OF TOneAluMac;
CONST
   kAluMac:TAluMacCodes=((Code: 1;St:'MR=x*y(RND)'),  (Code: 2;St:'MR=MR+x*y(RND)'),(Code: 3;St:'MR=MR-x*y(RND)'),
                         (Code: 4;St:'MR=x*y(SS)'),   (Code: 5;St:'MR=x*y(SU)'),    (Code: 6;St:'MR=x*y(US)'),
                         (Code: 7;St:'MR=x*y(UU)'),   (Code: 8;St:'MR=MR+x*y(SS)'),
                         (Code: 9;St:'MR=MR+x*y(SU)'),(Code:10;St:'MR=MR+x*y(US)'), (Code:11;St:'MR=MR+x*y(UU)'),
                         (Code:12;St:'MR=MR-x*y(SS)'),(Code:13;St:'MR=MR-x*y(SU)'),
                         (Code:14;St:'MR=MR-x*y(US)'),(Code:15;St:'MR=MR-x*y(UU)'),
                         (Code: 2;St:'MR=MR(RND)'),   (Code: 4;St:'MR=0'),          (Code: 8;St:'MR=MR'),
                         (Code:16;St:'AR=PASSy'),     (Code:16;St:'AR=PASS0'),      (Code:17;St:'AR=y+1'),
                         (Code:18;St:'AR=x+y+C'),     (Code:19;St:'AR=x+y'),        (Code:19;St:'AR=PASSx'),
                         (Code:20;St:'AR=NOTy'),      (Code:21;St:'AR=-y'),         (Code:22;St:'AR=x-y+C-1'),
                         (Code:23;St:'AR=x-y'),       (Code:26;St:'AR=y-x+C-1'),
                         (Code:18;St:'AR=x+C'),       (Code:22;St:'AR=x+C-1'),      (Code:26;St:'AR=-x+C-1'),
                         (Code:24;St:'AR=y-1'),       (Code:25;St:'AR=y-x'),        (Code:25;St:'AR=-x'),
                         (Code:27;St:'AR=NOTx'),      (Code:28;St:'AR=xANDy'),      (Code:29;St:'AR=xORy'),
                         (Code:30;St:'AR=xXORy'),     (Code:31;St:'AR=ABSx'));
VAR
   i,j:Integer;
   s1:String;
BEGIN
   GetAluMacFcn:=True;
   FOR i:=1 TO kNumAluMac DO
    BEGIN
     s1:=s;
     xop:=0;
     yop:=3; (* this is yop constant 0, important!! *)
     j:=1;
     WHILE(j<=Length(kAluMac[i].St)) DO
      BEGIN
       IF(kAluMac[i].St[j]='x') THEN
        BEGIN
         IF kAluMac[i].Code>=16 THEN
          BEGIN
           IF NOT GetXOP(s1,0,xop) THEN j:=512;
          END ELSE BEGIN
           IF NOT GetXOP(s1,1,xop) THEN j:=512;
          END;
        END ELSE IF(kAluMac[i].St[j]='y') THEN BEGIN
         IF kAluMac[i].Code>=16 THEN
          BEGIN
           IF NOT GetYOP(s1,0,yop) THEN j:=512;
          END ELSE BEGIN
           IF NOT GetYOP(s1,1,yop) THEN j:=512;
          END;
        END ELSE IF (kAluMac[i].St[j]<>s1[1]) OR (Length(s1)<1) THEN j:=512 ELSE Delete(s1,1,1);
       Inc(j);
      END;
     IF(j<512) THEN
      BEGIN
       s:=s1;
       amf:=kAluMac[i].Code;
       Exit;
      END;
    END;
   amf:=0;
   xop:=0;
   yop:=0;
   GetAluMacFcn:=False;
END;

        FUNCTION DoAluMac(s:String;cond:Byte):Boolean;
VAR
   Z,AMF,D,G,code,xop,yop,PD,DD,PMI,PMM,DMI,DMM:Byte;
   sp:String;
   i:Integer;
BEGIN
   DoAluMac:=False;
   IF ((s[1]<>'A') AND (s[1]<>'M')) OR (Length(s)<4) THEN
    BEGIN
     i:=QuotPos(s,',');
     IF (i>=Length(s)) OR (i<=0) THEN Exit;
     sp:=s;
     s:=Copy(sp,i+1,255);
     sp[0]:=Chr(i-1);
     KillPrefBlanks(s);
     IF ((s[1]<>'A') AND (s[1]<>'M')) OR (Length(s)<4) THEN Exit;
    END ELSE BEGIN
     i:=QuotPos(s,',');
     sp:=Copy(s,i+1,255);
     s[0]:=Chr(i-1);
    END;
   KillBlanks(s);
   KillBlanks(sp);
   InstrWord:=-1;
   Z:=0;
   IF s[2]='F' THEN
    BEGIN
     Z:=1;
     s[2]:='R';
    END;
   IF NOT GetAluMacFcn(s,amf,xop,yop) THEN Exit;
   IF (s<>'') THEN Exit;
   IF (cond<>15) OR (sp='') THEN
    BEGIN
     IF sp<>'' THEN Exit;
     InstrWord:=$200000 OR (LongInt(Z) SHL 18) OR (LongInt(amf) SHL 13) OR
                (LongInt(yop) SHL 11) OR (LongInt(xop) SHL 8) OR cond;
     InstrWordOk:=True;
     DoAluMac:=True;
     Exit;
    END;
   IF GetParDataMove(sp,code) THEN
    BEGIN
     InstrWord:=$280000 OR (LongInt(Z) SHL 18) OR (LongInt(amf) SHL 13) OR
                (LongInt(yop) SHL 11) OR (LongInt(xop) SHL 8) OR code;
     InstrWordOk:=True;
     DoAluMac:=True;
     Exit;
    END;
   IF GetParDMRW(sp,D,G,code) THEN
    BEGIN
     InstrWord:=$600000 OR (LongInt(Z OR (D SHL 1) OR (G SHL 2)) SHL 18) OR (LongInt(amf) SHL 13) OR
                (LongInt(yop) SHL 11) OR (LongInt(xop) SHL 8) OR code;
     InstrWordOk:=True;
     DoAluMac:=True;
     Exit;
    END;
   IF GetParPMRW(sp,D,code) THEN
    BEGIN
     InstrWord:=$500000 OR (LongInt(Z OR (D SHL 1)) SHL 18) OR (LongInt(amf) SHL 13) OR
                (LongInt(yop) SHL 11) OR (LongInt(xop) SHL 8) OR code;
     InstrWordOk:=True;
     DoAluMac:=True;
     Exit;
    END;
   IF GetParRdDP(sp,PD,DD,PMI,PMM,DMI,DMM) THEN
    BEGIN
     IF(Z<>0) THEN Exit;
     InstrWord:=$c00000 OR (LongInt((PD SHL 2) OR DD) SHL 18) OR (LongInt(amf) SHL 13) OR
                (LongInt(yop) SHL 11) OR (LongInt(xop) SHL 8) OR ((PMI SHL 6) OR (PMM SHL 4) OR (DMI SHL 2) OR DMM);
     InstrWordOk:=True;
     DoAluMac:=True;
     Exit;
    END;
END;

        FUNCTION DoDIVQ(VAR s:String):Boolean;
VAR
   xop:Byte;
BEGIN
   DoDIVQ:=False;
   IF(Copy(s,1,4)<>'DIVQ') THEN Exit;
   DoDIVQ:=True;
   InstrWordOk:=True;
   Delete(s,1,4);
   KillBlanks(s);
   IF NOT GetXOP(s,0,xop) THEN WrXError(1320,s);
   IF s<>'' THEN WrXError(1140,s);
   InstrWord:=$071000 OR (LongInt(xop AND 7) SHL 8);
END;

        FUNCTION DoDIVS(VAR s:String):Boolean;
VAR
   xop,yop:Byte;
BEGIN
   DoDIVS:=False;
   IF(Copy(s,1,4)<>'DIVS') THEN Exit;
   DoDIVS:=True;
   InstrWordOk:=True;
   Delete(s,1,4);
   KillBlanks(s);
   IF NOT GetYOP(s,0,yop) THEN WrXError(1320,s);
   IF (s[1]<>',') OR (Length(s)<1) THEN WrXError(1140,s);
   Delete(s,1,1);
   IF NOT GetXOP(s,0,xop) THEN WrXError(1320,s);
   IF (s<>'') THEN WrXError(1140,s);
   InstrWord:=$060000 OR (LongInt(xop AND 7) SHL 8) OR (LongInt(yop AND 3) SHL 11);
END;


        FUNCTION DoDMWriteImmed(s:String):Boolean;
VAR
   G,I,M:Byte;
   x:LongInt;
BEGIN
   DoDMWriteImmed:=False;
   IF Copy(s,1,2)<>'DM' THEN Exit;
   Delete(s,1,2);
   KillBlanks(s);
   IF NOT GetDAGGroup(s,G,I,M) THEN Exit;
   IF (Length(s)<2) OR (s[1]<>'=') THEN Exit;
   Delete(s,1,1);
   x:=EvalIntExpression(s,Int16,InstrWordOk);
   IF NOT InstrWordOk THEN Exit;
   InstrWord:=$A00000 OR (LongInt(G) SHL 20) OR ((x AND $FFFF) SHL 4) OR (I SHL 2) OR M;
   DoDMWriteImmed:=True;
END;

        FUNCTION DoDMRdImmedAddr(s:String):Boolean;
VAR
   REG,RGP:Byte;
   x:LongInt;
BEGIN
   DoDMRdImmedAddr:=False;
   IF NOT GetReg(s,TRUE,RGP,REG) THEN Exit;
   KillBlanks(s);
   IF (Length(s)<3) OR (Copy(s,1,3)<>'=DM') THEN Exit;
   Delete(s,1,3);
   x:=EvalIntExpression(s,UInt16,InstrWordOk);
   IF NOT InstrWordOk THEN Exit;
   InstrWord:=$800000 OR (LongInt(RGP) SHL 18) OR ((x AND $3FFF) SHL 4) OR REG;
   DoDMRdImmedAddr:=True;
END;

        FUNCTION DoDMWrImmedAddr(s:String):Boolean;
VAR
   REG,RGP:Byte;
   x:LongInt;
   s2:String;
   i,j:Integer;
BEGIN
   DoDMWrImmedAddr:=False;
   IF Copy(s,1,3)<>'DM(' THEN Exit;
   KillBlanks(s);
   Delete(s,1,2);
   j:=1;
   i:=2;
   WHILE (i<=Length(s)) AND (j>0) DO
    BEGIN
     IF(s[i]='(') THEN Inc(j) ELSE
     IF(s[i]=')') THEN Dec(j);
     Inc(i);
    END;
   IF j>0 THEN Exit;
   s2:=Copy(s,i,255);
   Delete(s,i,255);
   IF (Length(s2)<3) OR (s2[1]<>'=') THEN Exit;
   Delete(s2,1,1);
   IF NOT GetReg(s2,FALSE,RGP,REG) THEN Exit;
   x:=EvalIntExpression(s,UInt16,InstrWordOk);
   IF NOT InstrWordOk THEN Exit;
   InstrWord:=$900000 OR (LongInt(RGP) SHL 18) OR ((x AND $3FFF) SHL 4) OR REG;
   DoDMWrImmedAddr:=True;
END;

        FUNCTION DoAluMacNop(s:String):Boolean;
VAR
   D,G,code,PD,DD,PMI,PMM,DMI,DMM:Byte; (* AMF, Z, xop and yop is 0 *)
BEGIN
   DoAluMacNop:=False;
   KillBlanks(s);
   IF GetParDataMove(s,code) THEN
    BEGIN
     InstrWord:=$280000 OR code;
     InstrWordOk:=True;
     DoAluMacNop:=True;
     Exit;
    END;
   IF GetParDMRW(s,D,G,code) THEN
    BEGIN
     InstrWord:=$600000 OR (LongInt(D OR (G SHL 1)) SHL 19) OR code;
     InstrWordOk:=True;
     DoAluMacNop:=True;
     Exit;
    END;
   IF GetParPMRW(s,D,code) THEN
    BEGIN
     InstrWord:=$500000 OR (LongInt(D) SHL 19) OR code;
     InstrWordOk:=True;
     DoAluMacNop:=True;
     Exit;
    END;
   IF GetParRdDP(s,PD,DD,PMI,PMM,DMI,DMM) THEN
    BEGIN
     InstrWord:=$c00000 OR (LongInt((PD SHL 2) OR DD) SHL 18) OR
                ((PMI SHL 6) OR (PMM SHL 4) OR (DMI SHL 2) OR DMM);
     InstrWordOk:=True;
     DoAluMacNop:=True;
     Exit;
    END;
END;

        FUNCTION DoConst(VAR s:String):Boolean;
VAR
   t : TempResult;
   Ok : Boolean;
   maychange:Boolean;
   sp:String;
   sl:String;
   i:Integer;
BEGIN
   DoConst:=False;
   maychange:=False;
   IF Copy(s,1,4)='.SET' THEN
    BEGIN
     Delete(s,1,4);
     maychange:=True;
    END ELSE IF Copy(s,1,6)='.CONST' THEN Delete(s,1,6) ELSE Exit;
   DoConst:=True;
   KillBlanks(s);
   WHILE s<>'' DO
    BEGIN
     i:=QuotPos(s,',');
     sp:=s;
     Delete(s,1,i);
     sp[0]:=Chr(i-1);
     i:=QuotPos(sp,'=');
     sl:=sp;
     Delete(sp,1,i);
     sl[0]:=Chr(i-1);
     FirstPassUnknown:=False;
     EvalExpression(sp,t);
     IF NOT FirstPassUnknown THEN
      BEGIN
       SetListLineVal(t);
       PushLocHandle(-1);
       CASE t.Typ OF
       TempInt   :EnterIntSymbol   (sl,t.Int,SegNone,maychange);
       TempFloat :EnterFloatSymbol (sl,t.Float,maychange);
       TempString:EnterStringSymbol(sl,t.Ascii,maychange);
       END;
       PopLocHandle;
      END;
    END;
END;



        PROCEDURE DoOneInstruction(Instr:String);
VAR
   cond:Byte;
   x:LongInt;
BEGIN
   InstrWord:=-1; { debugging purposes }
   IF(Length(Instr)>=1) AND (Instr[1]>='0') AND (Instr[1]<='9') THEN (* define data *)
    BEGIN
     InstrWord:=EvalIntExpression(Instr,Int32,InstrWordOk);
     IF InstrWordOk AND (NOT FirstPassUnknown) AND ((InstrWord<-$800000) OR (InstrWord>$FFFFFF)) THEN WrError(1315);
     IF NOT InstrWordOk THEN WrXError(1200,Instr);
     Exit;
    END;
   IF DoConst(Instr) THEN Exit;
   cond:=15; (* FOREVER (unconditional) *)
   IF(Copy(Instr,1,2)='IF') THEN
    BEGIN
     IF DoFlagInJump(Instr) THEN Exit;
     Delete(Instr,1,2);
     cond:=GetCond(Instr,FALSE);
    END;
   IF cond = 12 THEN IF DoSatMR(Instr) THEN Exit; (* IF MV SAT MR *)
   IF cond = 15 THEN (* unconditional commands *)
    BEGIN
     IF DoNop(Instr) THEN Exit;
     IF DoPopPush(Instr) THEN Exit;
     IF DoEnaDis(Instr) THEN Exit;
     IF DoIdle(Instr) THEN Exit;
     IF DoModify(Instr) THEN Exit;
     IF DoIntDataMove(Instr) THEN Exit;
     IF DoDoUntil(Instr) THEN Exit;
     IF DoDIVQ(Instr) THEN Exit;
     IF DoDIVS(Instr) THEN Exit;


    END;
   IF DoTrap(Instr,Cond) THEN Exit;
   IF DoFlagOut(Instr,Cond) THEN Exit;
   IF DoReturn(Instr,Cond) THEN Exit;
   IF DoJump(Instr,Cond) THEN Exit;
   IF DoShifter(Instr,Cond) THEN Exit;
   IF DoAluMac(Instr,Cond) THEN Exit;

   IF cond<>15 THEN
    BEGIN
     WrXError(1200,Instr);
     Exit;
    END;

   IF DoAluMacNop(Instr) THEN Exit;
   IF DoDMRdImmedAddr(Instr) THEN Exit;
   IF DoDMWrImmedAddr(Instr) THEN Exit;
   IF DoRegImmed(Instr) THEN Exit;
   IF DoDMWriteImmed(Instr) THEN Exit;

   x:=EvalIntExpression(Instr,SInt24,InstrWordOk);
   IF NOT InstrWordOk THEN
    BEGIN
     WrXError(1200,Instr);
     Exit;
    END;
   InstrWord:=x AND $ffffff;
END;

(*
     MySubInstr1:='';
     MySubInstr2:='';
     i:=RQuotPos(MyOneInstr,',');
     IF(i>0) THEN
      BEGIN
       MySubInstr1:=Copy(MyOneInstr,i+1,255);
       KillBlanks(MySubInstr1);
       Delete(MyOneInstr,1,i-1);
       i:=RQuotPos(MyOneInstr,',');
       IF(i>0) THEN
        BEGIN
         MySubInstr2:=MySubInstr1;
         MySubInstr1:=Copy(MyOneInstr,i+1,255);
         KillBlanks(MySubInstr1);
         Delete(MyOneInstr,1,i-1);
        END;
      END;

*)

        FUNCTION LabelPseudo(VAR MyOneInstr:String):BOOLEAN;
VAR
   maynotchange : Boolean;
   t : TempResult;
   Ok : Boolean;
BEGIN
   LabelPseudo:=True;
   maynotchange:=False;
   IF (Copy(MyOneInstr,1,3) = 'EQU') THEN
    BEGIN
     maynotchange:=True;
     Delete(MyOneInstr,1,3);
    END;
   IF (Length(MyOneInstr)>0) AND (MyOneInstr[1] = '=') THEN
    BEGIN
     maynotchange:=True;
     Delete(MyOneInstr,1,1);
    END;
   IF maynotchange OR (Copy(MyOneInstr,1,3) = 'SET') THEN
    BEGIN
     IF NOT maynotchange THEN Delete(MyOneInstr,1,3);
     FirstPassUnknown:=False;
     EvalExpression(MyOneInstr,t);
     IF NOT FirstPassUnknown THEN
      BEGIN
       SetListLineVal(t);
       PushLocHandle(-1);
       CASE t.Typ OF
       TempInt   :EnterIntSymbol   (LabPart,t.Int,SegNone,NOT maynotchange);
       TempFloat :EnterFloatSymbol (LabPart,t.Float,NOT maynotchange);
       TempString:EnterStringSymbol(LabPart,t.Ascii,NOT maynotchange);
       END;
       PopLocHandle;
      END;
     LabPart:='';
     Exit;
    END;

   LabelPseudo:=False;
END;


	PROCEDURE MakeCode_21xx;
	Far;
VAR
   MyOneLine:String;
   MyOneInstr:String;
   i,j:Integer;
   Ok:Boolean;
   Erg:LongInt;
   t:TempResult;
BEGIN
   CodeLen:=0;
   MyOneLine:=OneLine;
   IF NextComment THEN
    BEGIN
     i:=Pos('}',MyOneLine); (* check if comment ends *)
     IF(i=0) THEN Exit;
     NextComment:=False;
     Delete(MyOneLine,1,i);
    END;
   i:=QuotPos(MyOneLine,'{');
   WHILE (i>0) AND (i<=Length(MyOneLine)) DO
    BEGIN
     j:=i+1;
     WHILE(j<=Length(MyOneLine)) AND (MyOneLine[j]<>'}') DO Inc(j);
     IF(j > Length(MyOneLine)) THEN NextComment := True;
     Delete(MyOneLine,i,j-i+1);
     i:=QuotPos(MyOneLine,'{');
    END;
   LabPart:='';
   IF NOT IsBlank(MyOneLine[1]) AND (Length(MyOneLine)>1) THEN
    BEGIN
     i:=FirstBlank(MyOneLine);
     LabPart:=Copy(MyOneLine,1,i);
     Delete(MyOneLine,1,i);
     KillPrefBlanks(MyOneLine);
    END;
   WHILE MyOneLine<>'' DO
    BEGIN
     i:=QuotPos(MyOneLine,';');
     IF(i<=0) OR (i>Length(MyOneLine)) THEN
      BEGIN
       MyOneInstr:=MyOneLine;
       MyOneLine:='';
      END ELSE BEGIN
       MyOneInstr:=Copy(MyOneLine,1,i-1);
       Delete(MyOneLine,1,i);
      END;
     KillPrefBlanks(MyOneInstr);
     KillPostBlanks(MyOneInstr);
     UpString(MyOneInstr);
     ExpandDefines(MyOneInstr);
     IF LabPart = '' THEN
      BEGIN
       i:=QuotPos(MyOneInstr,':');
       IF (i<=0) OR (i>Length(MyOneInstr)) THEN LabPart:='' ELSE
        BEGIN
         LabPart:=Copy(MyOneInstr,1,i-1);
         Delete(MyOneInstr,1,i);
         KillPrefBlanks(MyOneInstr);
        END;
      END;
     UpString(LabPart);
     KillBlanks(LabPart);
     IF LabPart[Length(LabPart)] = ':' THEN Dec(LabPart[0]);
     IF(Length(LabPart)>=1) AND (LabPart[1]>='0') AND (LabPart[1]<='9') THEN
      BEGIN
       Erg:=EvalIntExpression(LabPart,UInt16,Ok);
       IF (Ok) THEN
        IF FirstPassUnknown THEN WrError(1820) ELSE
         BEGIN
          PCs[ActPC]:=Erg; DontPrint:=True;
         END ELSE WrXError(1200,LabPart);
       LabPart:='';
      END;
     IF (MyOneInstr<>'') AND (LabPart<>'') THEN IF LabelPseudo(MyOneInstr) THEN
      BEGIN
       LabPart:='';
       MyOneInstr:='';
      END;
     IF LabPart<>'' THEN
      BEGIN
       PushLocHandle(-1);
       EnterIntSymbol(LabPart,PCs[ActPC],SegNone,False);
       PopLocHandle;
       LabPart:='';
      END;
     IF MyOneInstr<>'' THEN
      BEGIN
       InstrWordOk:=False;
       DoOneInstruction(MyOneInstr);
       IF InstrWordOk THEN
        BEGIN
         DAsmCode[CodeLen]:=InstrWord;
         Inc(CodeLen);
        END;
      END;
     LabPart:='';
    END;
    IF LabPart<>'' THEN
     BEGIN
      UpString(LabPart);
      KillBlanks(LabPart);
      IF LabPart[Length(LabPart)] = ':' THEN Dec(LabPart[0]);
      PushLocHandle(-1);
      EnterIntSymbol(LabPart,PCs[ActPC],SegNone,False);
      PopLocHandle;
     END;
END;

	PROCEDURE InitCode_21xx;
	Far;
BEGIN
   SaveInitProc;
   (* here comes my init code stuff *)
END;

	FUNCTION ChkPC_21xx:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode,SegData  : ok:=ProgCounter <=$3fff;
   ELSE ok:=False;
   END;
   ChkPC_21xx:=(ok) AND (ProgCounter>=0);
END;


	FUNCTION IsDef_21xx:Boolean;
	Far;
BEGIN
   IsDef_21xx:=True; (* can't use as' default behaviour *)
END;

	PROCEDURE SwitchFrom_21xx;
	Far;
BEGIN
   DeinitFields;
END;

	PROCEDURE SwitchTo_21xx;
	Far;
BEGIN
   TurnWords:=False; ConstMode:=ConstModeC; SetIsOccupied:=True;

   PCSymbol:='$'; HeaderID:=$0; NOPCode:=$00000000;
   DivideChars:=''; HasAttrs:=False; (* need to do my own instr. separation *)

   ValidSegs:=[SegCode,SegData];
   Grans[SegCode]:=4; ListGrans[SegCode]:=4; SegInits[SegCode]:=0;
   Grans[SegData]:=2; ListGrans[SegData]:=2; SegInits[SegData]:=0;

   MakeCode:=MakeCode_21xx; ChkPC:=ChkPC_21xx; IsDef:=IsDef_21xx;
   SwitchFrom:=SwitchFrom_21xx;

   InitFields;
   NextComment := FALSE;
END;

BEGIN
   CPU2100:=AddCPU('2100',SwitchTo_21xx);
   CPU2101:=AddCPU('2101',SwitchTo_21xx);
   CPU2103:=AddCPU('2103',SwitchTo_21xx);
   CPU2105:=AddCPU('2105',SwitchTo_21xx);
   CPU2111:=AddCPU('2111',SwitchTo_21xx);
   CPU2115:=AddCPU('2115',SwitchTo_21xx);

   AddCopyright('ADSP21xx-Generator (C) 1995 Thomas Sailer');

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_21xx;
END.
