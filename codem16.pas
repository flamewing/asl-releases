{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

        UNIT CodeM16;

{ AS - Codegeneratormodul Mitsubishi M16 }

INTERFACE

        Uses NLS,
             AsmDef,AsmSub,AsmPars,CodePseu;


IMPLEMENTATION

CONST
   ModNone=-1;
   ModReg      =0;          MModReg      =1 SHL ModReg;
   ModIReg     =1;          MModIReg     =1 SHL ModIReg;
   ModDisp16   =2;          MModDisp16   =1 SHL ModDisp16;
   ModDisp32   =3;          MModDisp32   =1 SHL ModDisp32;
   ModImm      =4;          MModImm      =1 SHL ModImm;
   ModAbs16    =5;          MModAbs16    =1 SHL ModAbs16;
   ModAbs32    =6;          MModAbs32    =1 SHL ModAbs32;
   ModPCRel16  =7;          MModPCRel16  =1 SHL ModPCRel16;
   ModPCRel32  =8;          MModPCRel32  =1 SHL ModPCRel32;
   ModPop      =9;          MModPop      =1 SHL ModPop;
   ModPush     =10;         MModPush     =1 SHL ModPush;
   ModRegChain =11;         MModRegChain =1 SHL ModRegChain;
   ModPCChain  =12;         MModPCChain  =1 SHL ModPCChain;
   ModAbsChain =13;         MModAbsChain =1 SHL ModAbsChain;

   Mask_RegOnly   =   MModReg;
   Mask_AllShort  =   MModReg+MModIReg+MModDisp16+MModImm+MModAbs16+MModAbs32
                     +MModPCRel16+MModPCRel32+MModPop+MModPush+MModPCChain
		     +MModAbsChain;
   Mask_AllGen    =   Mask_AllShort+MModDisp32+MModRegChain;
   Mask_NoImmShort=   Mask_AllShort-MModImm;
   Mask_NoImmGen  =   Mask_AllGen-MModImm;
   Mask_MemShort  =   Mask_NoImmShort-MModReg;
   Mask_MemGen    =   Mask_NoImmGen-MModReg;

   Mask_Source    =   Mask_AllGen-MModPush;
   Mask_Dest      =   Mask_NoImmGen-MModPop;
   Mask_PureDest  =   Mask_NoImmGen-MModPush-MModPop;
   Mask_PureMem   =   Mask_MemGen-MModPush-MModPop;

   FixedOrderCount=7;
   OneOrderCount=13;
   GE2OrderCount=11;
   BitOrderCount=6;
   GetPutOrderCount=8;
   BFieldOrderCount=4;
   MulOrderCount=4;
   ConditionCount=14;
   LogOrderCount=3;

TYPE
   FixedOrder=RECORD
	       Name:String[6];
	       Code:Word;
	      END;
   FixedOrderArray=ARRAY[1..FixedOrderCount] OF FixedOrder;

   OneOrder=RECORD
	     Name:String[6];
             Mask:Word;
             OpMask:Byte;
	     Code:Word;
	    END;
   OneOrderArray=ARRAY[1..OneOrderCount] OF OneOrder;

   GE2Order=RECORD
             Name:String[5];
             Mask1,Mask2:Word;
             SMask1,SMask2:Word;
             Code:Word;
             Signed:Boolean;
            END;
   GE2OrderArray=ARRAY[1..GE2OrderCount] OF GE2Order;

   BitOrder=RECORD
             Name:String[5];
             MustByte:Boolean;
             Code1,Code2:Word;
            END;
   BitOrderArray=ARRAY[1..BitOrderCount] OF BitOrder;

   GetPutOrder=RECORD
                Name:String[5];
                Size:Byte;
                Code:Word;
                Turn:Boolean;
               END;
   GetPutOrderArray=ARRAY[1..GetPutOrderCount] OF GetPutOrder;

   BFieldOrderArray=ARRAY[0..BFieldOrderCount-1] OF String[6];

   MulOrderArray=ARRAY[0..MulOrderCount-1] OF String[4];

   ConditionArray=ARRAY[0..ConditionCount-1] OF String[2];

   LogOrderArray=ARRAY[0..LogOrderCount-1] OF String[3];

VAR
   CPUM16:CPUVar;

   Format:String;
   FormatCode:Byte;
   DOpSize:ShortInt;
   OpSize:ARRAY[1..4] OF ShortInt;
   AdrMode:ARRAY[1..4] OF Word;
   AdrType:ARRAY[1..4] OF ShortInt;
   AdrCnt,AdrCnt2:ARRAY[1..4] OF Byte;
   AdrVals:ARRAY[1..4,0..7] OF Word;

   OptionCnt:Byte;
   Options:ARRAY[1..2] OF String[4];

   FixedOrders:^FixedOrderArray;
   OneOrders:^OneOrderArray;
   GE2Orders:^GE2OrderArray;
   BitOrders:^BitOrderArray;
   GetPutOrders:^GetPutOrderArray;
   BFieldOrders:^BFieldOrderArray;
   MulOrders:^MulOrderArray;
   Conditions:^ConditionArray;
   LogOrders:^LogOrderArray;

{--------------------------------------------------------------------------}

	PROCEDURE InitFields;
VAR
   z:Integer;

   	PROCEDURE AddFixed(NName:String; NCode:Word);
BEGIN
   IF z>FixedOrderCount THEN Halt;
   Inc(z);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

   	PROCEDURE AddOne(NName:String; NOpMask:Byte; NMask,NCode:Word);
BEGIN
   IF z>OneOrderCount THEN Halt;
   Inc(z);
   WITH OneOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Mask:=NMask; OpMask:=NOpMask;
    END;
END;

   	PROCEDURE AddGE2(NName:String; NMask1,NMask2:Word;
	                 NSMask1,NSMask2:Byte; NCode:Word; NSigned:Boolean);
BEGIN
   IF z>GE2OrderCount THEN Halt;
   Inc(z);
   WITH GE2Orders^[z] DO
    BEGIN
     Name:=NName;
     Mask1:=NMask1; Mask2:=NMask2;
     SMask1:=NSMask1; SMask2:=NSMask2;
     Code:=NCode;
     Signed:=NSigned;
    END;
END;

   	PROCEDURE AddBit(NName:String; NMust:Boolean; NCode1,NCode2:Word);
BEGIN
   IF z>BitOrderCount THEN Halt;
   Inc(z);
   WITH BitOrders^[z] DO
    BEGIN
     Name:=NName; Code1:=NCode1; Code2:=NCode2; MustByte:=NMust;
    END;
END;

	PROCEDURE AddGetPut(NName:String; NSize:Byte; NCode:Word; NTurn:Boolean);
BEGIN
   IF z>GetPutOrderCount THEN Halt;
   Inc(z);
   WITH GetPutOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; Turn:=NTurn; Size:=NSize;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('NOP'  ,$1bd6); AddFixed('PIB'  ,$0bd6);
   AddFixed('RIE'  ,$08f7); AddFixed('RRNG' ,$3bd6);
   AddFixed('RTS'  ,$2bd6); AddFixed('STCTX',$07d6);
   AddFixed('REIT' ,$2fd6);

   New(OneOrders); z:=0;
   AddOne('ACS'   ,$00,Mask_PureMem,                    $8300);
   AddOne('NEG'   ,$07,Mask_PureDest,                   $c800);
   AddOne('NOT'   ,$07,Mask_PureDest,                   $cc00);
   AddOne('JMP'   ,$00,Mask_PureMem,                    $8200);
   AddOne('JSR'   ,$00,Mask_PureMem,                    $aa00);
   AddOne('LDCTX' ,$00,MModIReg+MModDisp16+MModDisp32+
          MModAbs16+MModAbs32+MModPCRel16+MModPCRel32,  $8600);
   AddOne('LDPSB' ,$02,Mask_Source,                     $db00);
   AddOne('LDPSM' ,$02,Mask_Source,                     $dc00);
   AddOne('POP'   ,$04,Mask_PureDest,                   $9000);
   AddOne('PUSH'  ,$04,Mask_Source-MModPop,             $b000);
   AddOne('PUSHA' ,$00,Mask_PureMem,                    $a200);
   AddOne('STPSB' ,$02,Mask_Dest,                       $dd00);
   AddOne('STPSM' ,$02,Mask_Dest,                       $de00);

   New(GE2Orders); z:=0;
   AddGE2('ADDU' ,Mask_Source,Mask_PureDest,7,7,$0400,False);
   AddGE2('ADDX' ,Mask_Source,Mask_PureDest,7,7,$1000,True );
   AddGE2('SUBU' ,Mask_Source,Mask_PureDest,7,7,$0c00,False);
   AddGE2('SUBX' ,Mask_Source,Mask_PureDest,7,7,$1800,True );
   AddGE2('CMPU' ,Mask_Source,Mask_PureDest,7,7,$8400,False);
   AddGE2('LDC'  ,Mask_Source,Mask_PureDest,7,4,$9800,True );
   AddGE2('LDP'  ,Mask_Source,Mask_PureMem ,7,7,$9c00,True );
   AddGE2('MOVU' ,Mask_Source,Mask_Dest    ,7,7,$8c00,True );
   AddGE2('REM'  ,Mask_Source,Mask_PureDest,7,7,$5800,True );
   AddGE2('REMU' ,Mask_Source,Mask_PureDest,7,7,$5c00,True );
   AddGE2('ROT'  ,Mask_Source,Mask_PureDest,1,7,$3800,True );

   New(BitOrders); z:=0;
   AddBit('BCLR' ,False,$b400,$a180);
   AddBit('BCLRI',True ,$a400,$0000);
   AddBit('BNOT' ,False,$b800,$0000);
   AddBit('BSET' ,False,$b000,$8180);
   AddBit('BSETI',True ,$a000,$81c0);
   AddBit('BTST' ,False,$bc00,$a1c0);

   New(GetPutOrders); z:=0;
   AddGetPut('GETB0',0,$c000,False);
   AddGetPut('GETB1',0,$c400,False);
   AddGetPut('GETB2',0,$c800,False);
   AddGetPut('GETH0',1,$cc00,False);
   AddGetPut('PUTB0',0,$d000,True );
   AddGetPut('PUTB1',0,$d400,True );
   AddGetPut('PUTB2',0,$d800,True );
   AddGetPut('PUTH0',1,$dc00,True );

   New(BFieldOrders);
   BFieldOrders^[0]:='BFCMP'; BFieldOrders^[1]:='BFCMPU';
   BFieldOrders^[2]:='BFINS'; BFieldOrders^[3]:='BFINSU';

   New(MulOrders);
   MulOrders^[0]:='MUL'; MulOrders^[1]:='MULU';
   MulOrders^[2]:='DIV'; MulOrders^[3]:='DIVU';

   New(Conditions);
   Conditions^[ 0]:='XS'; Conditions^[ 1]:='XC';
   Conditions^[ 2]:='EQ'; Conditions^[ 3]:='NE';
   Conditions^[ 4]:='LT'; Conditions^[ 5]:='GE';
   Conditions^[ 6]:='LE'; Conditions^[ 7]:='GT';
   Conditions^[ 8]:='VS'; Conditions^[ 9]:='VC';
   Conditions^[10]:='MS'; Conditions^[11]:='MC';
   Conditions^[12]:='FS'; Conditions^[13]:='FC';

   New(LogOrders);
   LogOrders^[0]:='AND'; LogOrders^[1]:='OR'; LogOrders^[2]:='XOR';
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(OneOrders);
   Dispose(GE2Orders);
   Dispose(BitOrders);
   Dispose(GetPutOrders);
   Dispose(BFieldOrders);
   Dispose(MulOrders);
   Dispose(Conditions);
   Dispose(LogOrders);
END;

{--------------------------------------------------------------------------}

        FUNCTION IsD4(inp:LongInt):Boolean;
BEGIN
   IsD4:=(inp>=-32)AND(inp<=28);
END;

        FUNCTION IsD16(inp:LongInt):Boolean;
BEGIN
   IsD16:=(inp>=-$8000) AND (inp<=$7fff);
END;

	FUNCTION DecodeReg(Asc:String; VAR Erg:Word):Boolean;
VAR
   IO:ValErgType;
BEGIN
   DecodeReg:=True; NLS_UpString(Asc);
   IF Asc='SP' THEN Erg:=15
   ELSE IF Asc='FP' THEN Erg:=14
   ELSE IF (Length(Asc)>1) AND (Asc[1]='R') THEN
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,IO);
     DecodeReg:=(IO=0) AND (Erg<=15);
    END
   ELSE DecodeReg:=False;
END;

	FUNCTION DecodeAdr(VAR Asc:String; Index:Integer; Mask:Word):Boolean;
LABEL
   Found;
TYPE
   DispSize=(DispSizeNone,DispSize4,DispSize4Eps,DispSize16,DispSize32);
   PChainRec=^TChainRec;
   TChainRec=RECORD
              Next:PChainRec;
              RegCnt:Byte;
              Regs,Scales:ARRAY[0..4] OF Word;
              DispAcc:LongInt;
              HasDisp:Boolean;
              DSize:DispSize;
             END;
VAR
   AdrLong,MinReserve,MaxReserve:LongInt;
   z,z2,LastChain:Integer;
   OK,ErrFlag:Boolean;
   RootChain,RunChain,PrevChain:PChainRec;
   DSize:DispSize;

   	PROCEDURE SplitSize(VAR s:String; VAR Erg:DispSize);
VAR
   l:Integer;
BEGIN
   l:=Length(s);
   IF (l>2) AND (s[l]='4') AND (s[l-1]=':') THEN
    BEGIN
     Erg:=DispSize4;
     Dec(Byte(s[0]),2);
    END
   ELSE IF (l>3) AND (s[l]='6') AND (s[l-1]='1') AND (s[l-2]=':') THEN
    BEGIN
     Erg:=DispSize16;
     Dec(Byte(s[0]),3);
    END
   ELSE IF (l>3) AND (s[l]='2') AND (s[l-1]='3') AND (s[l-2]=':') THEN
    BEGIN
     Erg:=DispSize32;
     Dec(Byte(s[0]),3);
    END;
END;

   	PROCEDURE DecideAbs(Disp:LongInt; Size:DispSize);
BEGIN
   CASE Size OF
   DispSize4:
    Size:=DispSize16;
   DispSizeNone:
    IF (IsD16(Disp)) AND (Mask AND MModAbs16<>0) THEN Size:=DispSize16
    ELSE Size:=DispSize32;
   END;

   CASE Size OF
   DispSize16:
    IF ChkRange(Disp,-$8000,$7fff) THEN
     BEGIN
      AdrType[Index]:=ModAbs16; AdrMode[Index]:=$09;
      AdrVals[Index][0]:=Disp AND $ffff; AdrCnt[Index]:=2;
     END;
   DispSize32:
    BEGIN
     AdrType[Index]:=ModAbs32; AdrMode[Index]:=$0a;
     AdrVals[Index][0]:=Disp SHR 16;
     AdrVals[Index][1]:=Disp AND $ffff; AdrCnt[Index]:=4;
    END;
   END;
END;

	PROCEDURE SetError(Code:Word);
BEGIN
   WrError(Code); ErrFlag:=True;
END;

	FUNCTION DecodeChain(VAR Asc:String):PChainRec;
VAR
   Rec:PChainRec;
   Part:String;
   p,z:Integer;
   Scale:Byte;
BEGIN
   ChkStack;
   New(Rec);
   Rec^.Next:=Nil; Rec^.RegCnt:=0; Rec^.DispAcc:=0; Rec^.HasDisp:=False;
   Rec^.DSize:=DispSizeNone;
   WHILE (Asc<>'') AND (NOT ErrFlag) DO
    BEGIN

     { eine Komponente abspalten }

     SplitString(Asc,Part,Asc,QuotPos(Asc,','));
     p:=QuotPos(Part,'*');

     { weitere Indirektion ? }

     IF Part[1]='@' THEN
      IF Rec^.Next<>Nil THEN SetError(1350)
      ELSE
       BEGIN
        Delete(Part,1,1);
        IF IsIndirect(Part) THEN Part:=Copy(Part,2,Length(Part)-2);
	Rec^.Next:=DecodeChain(Part);
       END

     { Register, mit Skalierungsfaktor ? }

     ELSE IF DecodeReg(Copy(Part,1,p-1),Rec^.Regs[Rec^.RegCnt]) THEN
      BEGIN
       IF Rec^.RegCnt>=5 THEN SetError(1350)
       ELSE
        BEGIN
         FirstPassUnknown:=False;
         IF p>=Length(Part) THEN
          BEGIN
           OK:=True; Scale:=1;
          END
         ELSE Scale:=EvalIntExpression(Copy(Part,p+1,Length(Part)-p),UInt4,OK);
         IF FirstPassUnknown THEN Scale:=1;
         IF NOT OK THEN ErrFlag:=True
         ELSE IF (Scale<>1) AND (Scale<>2) AND (Scale<>4) AND (Scale<>8) THEN SetError(1350)
         ELSE
          BEGIN
           Rec^.Scales[Rec^.RegCnt]:=0;
	   WHILE Scale>1 DO
	    BEGIN
             Inc(Rec^.Scales[Rec^.RegCnt]); Scale:=Scale SHR 1;
	    END;
	   Inc(Rec^.RegCnt);
          END;
        END;
      END

     { PC, mit Skalierungsfaktor ? }

     ELSE IF NLS_StrCaseCmp(Copy(Part,1,p-1),'PC')=0 THEN
      BEGIN
       IF Rec^.RegCnt>=5 THEN SetError(1350)
       ELSE
        BEGIN
         FirstPassUnknown:=False;
         IF p>=Length(Part) THEN
          BEGIN
           OK:=True; Scale:=1;
          END
         ELSE Scale:=EvalIntExpression(Copy(Part,p+1,Length(Part)-p),UInt4,OK);
         IF FirstPassUnknown THEN Scale:=1;
         IF NOT OK THEN ErrFlag:=True
         ELSE IF (Scale<>1) AND (Scale<>2) AND (Scale<>4) AND (Scale<>8) THEN SetError(1350)
         ELSE
          BEGIN
           FOR z:=Rec^.RegCnt-1 DOWNTO 0 DO
            BEGIN
             Rec^.Regs[z+1]:=Rec^.Regs[z];
             Rec^.Scales[z+1]:=Rec^.Scales[z];
            END;
           Rec^.Scales[0]:=0;
	   WHILE Scale>1 DO
	    BEGIN
             Inc(Rec^.Scales[0]); Scale:=Scale SHR 1;
	    END;
           Rec^.Regs[0]:=16;
	   Inc(Rec^.RegCnt);
          END;
        END;
      END

     { ansonsten Displacement }

     ELSE
      BEGIN
       SplitSize(Part,Rec^.DSize);
       Inc(Rec^.DispAcc,EvalIntExpression(Part,Int32,OK));
       IF NOT OK THEN ErrFlag:=True;
       Rec^.HasDisp:=True;
      END;
    END;

   IF ErrFlag THEN
    BEGIN
     Dispose(Rec); DecodeChain:=Nil;
    END
   ELSE DecodeChain:=Rec;
END;

BEGIN
   AdrCnt[Index]:=0; AdrType[Index]:=ModNone;

   { Register ? }

   IF DecodeReg(Asc,AdrMode[Index]) THEN
    BEGIN
     AdrType[Index]:=ModReg; Inc(AdrMode[Index],$10); Goto Found;
    END;

   { immediate ? }

   IF Asc[1]='#' THEN
    BEGIN
     Delete(Asc,1,1);
     CASE OpSize[Index] OF
     -1:BEGIN
         WrError(1132); OK:=False;
        END;
     0:BEGIN
        AdrVals[Index][0]:=EvalIntExpression(Asc,Int8,OK) AND $ff;
        IF OK THEN AdrCnt[Index]:=2;
       END;
     1:BEGIN
        AdrVals[Index][0]:=EvalIntExpression(Asc,Int16,OK);
        IF OK THEN AdrCnt[Index]:=2;
       END;
     2:BEGIN
        AdrLong:=EvalIntExpression(Asc,Int32,OK);
        IF OK THEN
         BEGIN
          AdrVals[Index][0]:=AdrLong SHR 16;
          AdrVals[Index][1]:=AdrLong AND $ffff;
          AdrCnt[Index]:=4;
         END;
       END;
     END;
     IF OK THEN
      BEGIN
       AdrType[Index]:=ModImm; AdrMode[Index]:=$0c;
      END;
     Goto Found;
    END;

   { indirekt ? }

   IF Asc[1]='@' THEN
    BEGIN
     Delete(Asc,1,1);
     IF IsIndirect(Asc) THEN Asc:=Copy(Asc,2,Length(Asc)-2);

     { Stack Push ? }

     IF (NLS_StrCaseCmp(Asc,'-R15')=0) OR (NLS_StrCaseCmp(Asc,'-SP')=0) THEN
      BEGIN
       AdrType[Index]:=ModPush; AdrMode[Index]:=$05;
       Goto Found;
      END;

     { Stack Pop ? }

     IF (NLS_StrCaseCmp(Asc,'R15+')=0) OR (NLS_StrCaseCmp(Asc,'SP+')=0) THEN
      BEGIN
       AdrType[Index]:=ModPop; AdrMode[Index]:=$04;
       Goto Found;
      END;

     { Register einfach indirekt ? }

     IF DecodeReg(Asc,AdrMode[Index]) THEN
      BEGIN
       AdrType[Index]:=ModIReg; Inc(AdrMode[Index],$30);
       Goto Found;
      END;

     { zusammengesetzt indirekt ? }

     ErrFlag:=False;
     RootChain:=DecodeChain(Asc);

     IF ErrFlag THEN

     ELSE IF RootChain=Nil THEN

     { absolut ? }

     ELSE IF (RootChain^.Next=Nil) AND (RootChain^.RegCnt=0) THEN
      BEGIN
       IF NOT RootChain^.HasDisp THEN RootChain^.DispAcc:=0;
       DecideAbs(RootChain^.DispAcc,RootChain^.DSize);
       Dispose(RootChain);
      END

     { einfaches Register/PC mit Displacement ? }

     ELSE IF (RootChain^.Next=Nil) AND (RootChain^.RegCnt=1) AND (RootChain^.Scales[0]=0) THEN
      BEGIN
       IF RootChain^.Regs[0]=16 THEN Dec(RootChain^.DispAcc,EProgCounter);

       { Displacement-Grî·e entscheiden }

       IF RootChain^.DSize=DispSizeNone THEN
        IF (RootChain^.DispAcc=0) AND (RootChain^.Regs[0]<16) THEN
        ELSE IF IsD16(RootChain^.DispAcc) THEN
         RootChain^.DSize:=DispSize16
        ELSE RootChain^.DSize:=DispSize32;

       CASE RootChain^.DSize OF

       { kein Displacement mit Register }

       DispSizeNone:
        IF ChkRange(RootChain^.DispAcc,0,0) THEN
         IF RootChain^.Regs[0]>=16 THEN WrError(1350)
         ELSE
          BEGIN
           AdrType[Index]:=ModIReg;
           AdrMode[Index]:=$30+RootChain^.Regs[0];
          END;

       { 16-Bit-Displacement }

       DispSize4,DispSize16:
        IF ChkRange(RootChain^.DispAcc,-$8000,$7fff) THEN
         BEGIN
          AdrVals[Index][0]:=RootChain^.DispAcc AND $ffff; AdrCnt[Index]:=2;
          IF RootChain^.Regs[0]=16 THEN
           BEGIN
            AdrType[Index]:=ModPCRel16; AdrMode[Index]:=$0d;
           END
          ELSE
           BEGIN
            AdrType[Index]:=ModDisp16; AdrMode[Index]:=$20+RootChain^.Regs[0];
           END;
         END;

       { 32-Bit-Displacement }

       ELSE
        BEGIN
         AdrVals[Index][1]:=RootChain^.DispAcc AND $ffff;
	 AdrVals[Index][0]:=RootChain^.DispAcc SHR 16; AdrCnt[Index]:=4;
         IF RootChain^.Regs[0]=16 THEN
          BEGIN
           AdrType[Index]:=ModPCRel32; AdrMode[Index]:=$0e;
          END
         ELSE
          BEGIN
           AdrType[Index]:=ModDisp32; AdrMode[Index]:=$40+RootChain^.Regs[0];
          END;
        END;
       END;

       Dispose(RootChain);
      END

     { komplex: dann chained iterieren }

     ELSE
      BEGIN
       { bis zum innersten Element der Indirektion als Basis laufen }

       RunChain:=RootChain;
       WHILE RunChain^.Next<>Nil DO RunChain:=RunChain^.Next;

       { Entscheidung des Basismodus: die Basis darf nicht skaliert
         sein, und wenn ein Modus nicht erlaubt ist, mÅssen wir mit
         Base-none anfangen... }

       z:=0; WHILE (z<RunChain^.RegCnt) AND (RunChain^.Scales[z]<>0) DO Inc(z);
       IF z>=RunChain^.RegCnt THEN
        BEGIN
         AdrType[Index]:=ModAbsChain; AdrMode[Index]:=$0b;
        END
       ELSE
        BEGIN
         IF RunChain^.Regs[z]=16 THEN
          BEGIN
           AdrType[Index]:=ModPCChain; AdrMode[Index]:=$0f;
           Dec(RunChain^.DispAcc,EProgCounter);
          END
         ELSE
          BEGIN
           AdrType[Index]:=ModRegChain;
           AdrMode[Index]:=$60+RunChain^.Regs[z];
          END;
         FOR z2:=z TO RunChain^.RegCnt-2 DO
          BEGIN
           RunChain^.Regs[z2]:=RunChain^.Regs[z2+1];
           RunChain^.Scales[z2]:=RunChain^.Scales[z2+1];
          END;
         Dec(RunChain^.RegCnt);
        END;

       { Jetzt Åber die einzelnen Komponenten iterieren }

       WHILE RootChain<>Nil DO
	BEGIN
         RunChain:=RootChain; PrevChain:=Nil;
	 WHILE RunChain^.Next<>Nil DO
	  BEGIN
           PrevChain:=RunChain;
	   RunChain:=RunChain^.Next;
          END;

         { noch etwas abzulegen ? }

         IF (RunChain^.RegCnt<>0) OR (RunChain^.HasDisp) THEN
          BEGIN
           LastChain:=AdrCnt[Index] SHR 1;

           { Register ablegen }

           IF RunChain^.RegCnt<>0 THEN
            BEGIN
             IF RunChain^.Regs[0]=16 THEN AdrVals[Index][LastChain]:=$0600
             ELSE AdrVals[Index][LastChain]:=RunChain^.Regs[0] SHL 10;
             Inc(AdrVals[Index][LastChain],RunChain^.Scales[0] SHL 5);
             FOR z2:=0 TO RunChain^.RegCnt-2 DO
              BEGIN
               RunChain^.Regs[z2]:=RunChain^.Regs[z2+1];
               RunChain^.Scales[z2]:=RunChain^.Scales[z2+1];
              END;
             Dec(RunChain^.RegCnt);
            END
	   ELSE AdrVals[Index][LastChain]:=$0200;
           Inc(AdrCnt[Index],2);

           { Displacement ablegen }

           IF RunChain^.HasDisp THEN
            BEGIN
             IF AdrVals[Index][LastChain] AND $3e00=$0600 THEN
              Dec(RunChain^.DispAcc,EProgCounter);

             IF RunChain^.DSize=DispSizeNone THEN
              BEGIN
               MinReserve:=32*RunChain^.RegCnt; MaxReserve:=28*RunChain^.RegCnt;
               IF (IsD4(RunChain^.DispAcc)) THEN
                IF (RunChain^.DispAcc AND 3=0) THEN DSize:=DispSize4
                ELSE DSize:=DispSize16
               ELSE IF (RunChain^.DispAcc>=-32-MinReserve) AND
                       (RunChain^.DispAcc<=28+MaxReserve) THEN DSize:=DispSize4Eps
               ELSE IF IsD16(RunChain^.DispAcc) THEN DSize:=DispSize16
               ELSE IF (RunChain^.DispAcc>=-$8000-MinReserve) AND
                       (RunChain^.DispAcc<=$7fff+MaxReserve) THEN DSize:=DispSize4Eps
               ELSE DSize:=DispSize32;
              END
             ELSE DSize:=RunChain^.DSize;
             RunChain^.DSize:=DispSizeNone;

             CASE DSize OF

             { Fall 1: pa·t komplett in 4er-Displacement }

             DispSize4:
              IF ChkRange(RunChain^.DispAcc,-32,28) THEN
               IF RunChain^.DispAcc AND 3<>0 THEN WrError(1325)
               ELSE
                BEGIN
                 Inc(AdrVals[Index][LastChain],(RunChain^.DispAcc SHR 2) AND $000f);
                 RunChain^.HasDisp:=False;
                END;

             { Fall 2: pa·t nicht mehr in nÑchstkleineres Displacement, aber wir
               kînnen hier schon einen Teil ablegen, um im nÑchsten Iterations-
               schritt ein kÅrzeres Displacement zu bekommen }

             DispSize4Eps:
              IF RunChain^.DispAcc>0 THEN
               BEGIN
                Inc(AdrVals[Index][LastChain],$0007);
                Dec(RunChain^.DispAcc,28);
               END
              ELSE
               BEGIN
                Inc(AdrVals[Index][LastChain],$0008);
                Inc(RunChain^.DispAcc,32);
               END;

             { Fall 3: 16 Bit }

             DispSize16:
              IF ChkRange(RunChain^.DispAcc,-$8000,$7fff) THEN
               BEGIN
                Inc(AdrVals[Index][LastChain],$0011);
                AdrVals[Index][LastChain+1]:=RunChain^.DispAcc AND $ffff;
                Inc(AdrCnt[Index],2);
                RunChain^.HasDisp:=False;
               END;

             { Fall 4: 32 Bit }

             DispSize32:
              BEGIN
               Inc(AdrVals[Index][LastChain],$0012);
               AdrVals[Index][LastChain+1]:=RunChain^.DispAcc SHR 16;
	       AdrVals[Index][LastChain+2]:=RunChain^.DispAcc AND $ffff;
               Inc(AdrCnt[Index],4);
               RunChain^.HasDisp:=False;
              END;
             END;
            END;
          END

         { nichts mehr drin: dann ein leeres Steuerwort erzeugen.  Tritt
           auf, falls alles schon im Basisadressierungsbyte verschwunden }

         ELSE IF RunChain<>RootChain THEN
          BEGIN
           LastChain:=AdrCnt[Index] SHR 1;
           AdrVals[Index][LastChain]:=$0200; Inc(AdrCnt[Index],2);
          END;

         { nichts mehr drin: wegwerfen
	   wenn wir noch nicht ganz vorne angekommen sind, dann ein
	   Indirektionsflag setzen }

         IF (RunChain^.RegCnt=0) AND (NOT RunChain^.HasDisp) THEN
          BEGIN
           IF RunChain<>RootChain THEN Inc(AdrVals[Index][LastChain],$4000);
           IF PrevChain=Nil THEN RootChain:=Nil ELSE PrevChain^.Next:=Nil;
           Dispose(RunChain);
          END;
	END;

       { Ende-Kennung fÅr letztes Glied }

       Inc(AdrVals[Index][LastChain],$8000);
      END;

     Goto Found;
    END;

   { ansonsten absolut }

   DSize:=DispSizeNone;
   SplitSize(Asc,DSize);
   AdrLong:=EvalIntExpression(Asc,Int32,OK);
   IF OK THEN DecideAbs(AdrLong,DSize);

Found:
   AdrCnt2[Index]:=AdrCnt[Index] SHR 1;
   IF (AdrType[Index]<>-1) AND (Mask AND (1 SHL AdrType[Index])=0) THEN
    BEGIN
     AdrCnt[Index]:=0; AdrType[Index]:=ModNone;
     WrError(1350);
     DecodeAdr:=False;
    END
   ELSE DecodeAdr:=AdrType[Index]<>ModNone;
END;

	FUNCTION ImmVal(Index:Integer):LongInt;
BEGIN
   CASE OpSize[Index] OF
   0:ImmVal:=ShortInt(AdrVals[Index][0] AND $ff);
   1:ImmVal:=Integer(AdrVals[Index][0]);
   2:ImmVal:=(LongInt(AdrVals[Index][0]) SHL 16)+AdrVals[Index][1];
   END;
END;

	FUNCTION IsShort(Index:Integer):Boolean;
BEGIN
   IsShort:=AdrMode[Index] AND $c0=0;
END;

	PROCEDURE AdaptImm(Index:Integer; NSize:Byte; Signed:Boolean);
BEGIN
   CASE OpSize[Index] OF
   0:IF NSize<>0 THEN
      BEGIN
       IF (AdrVals[Index][0] AND $80=$80) AND (Signed) THEN
        AdrVals[Index][0]:=AdrVals[Index][0] OR $ff00
       ELSE AdrVals[Index][0]:=AdrVals[Index][0] AND $ff;
       IF NSize=2 THEN
        BEGIN
         IF (AdrVals[Index][0] AND $8000=$8000) AND (Signed) THEN
          AdrVals[Index][1]:=$ffff
         ELSE AdrVals[Index][1]:=0;
         Inc(AdrCnt[Index],2); Inc(AdrCnt2[Index]);
        END;
      END;
   1:IF NSize=0 THEN AdrVals[Index][0]:=AdrVals[Index][0] AND $ff
     ELSE IF NSize=2 THEN
      BEGIN
       IF (AdrVals[Index][0] AND $8000=$8000) AND (Signed) THEN
        AdrVals[Index][1]:=$ffff
       ELSE AdrVals[Index][1]:=0;
       Inc(AdrCnt[Index],2); Inc(AdrCnt2[Index]);
      END;
   2:IF NSize<>2 THEN
      BEGIN
       Dec(AdrCnt[Index],2); Dec(AdrCnt2[Index]);
       IF NSize=0 THEN AdrVals[Index][0]:=AdrVals[Index][0] AND $ff;
      END;
   END;
   OpSize[Index]:=NSize;
END;

	FUNCTION DefSize(Mask:Byte):ShortInt;
VAR
   z:ShortInt;
BEGIN
   z:=2;
   WHILE (z>=0) AND (Mask AND 4=0) DO
    BEGIN
     Mask:=(Mask SHL 1) AND 7; Dec(z);
    END;
   DefSize:=z;
END;

	FUNCTION DecodeRegList(Asc:String; VAR Erg:Word; Turn:Boolean):Boolean;
VAR
   Part:String[10];
   p,p1,p2,r1,r2:Word;

   	FUNCTION Mask(No:Word):Word;
BEGIN
   IF Turn THEN Mask:=$8000 SHR No ELSE Mask:=1 SHL No;
END;

BEGIN
   DecodeRegList:=False;
   IF IsIndirect(Asc) THEN Asc:=Copy(Asc,2,Length(Asc)-2);
   Erg:=0;
   WHILE Asc<>'' DO
    BEGIN
     p1:=Pos(',',Asc); p2:=Pos('/',Asc);
     IF (p1<>0) AND (p1<p2) THEN p:=p1 ELSE p:=p2;
     IF p=0 THEN
      BEGIN
       Part:=Asc; Asc:='';
      END
     ELSE
      BEGIN
       Part:=Copy(Asc,1,p-1); Delete(Asc,1,p);
      END;
     p:=Pos('-',Part);
     IF p=0 THEN
      BEGIN
       IF NOT DecodeReg(Part,r1) THEN
        BEGIN
         WrXError(1410,Part); Exit;
        END;
       Erg:=Erg OR Mask(r1);
      END
     ELSE
      BEGIN
       IF NOT DecodeReg(Copy(Part,1,p-1),r1) THEN
        BEGIN
         WrXError(1410,Copy(Part,1,p-1)); Exit;
        END;
       IF NOT DecodeReg(Copy(Part,p+1,Length(Part)-p),r2) THEN
        BEGIN
         WrXError(1410,Copy(Part,p+1,Length(Part)-p)); Exit;
        END;
       IF r1<=r2 THEN
        FOR p:=r1 TO r2 DO Erg:=Erg OR Mask(p)
       ELSE
        BEGIN
         FOR p:=r2 TO 15 DO Erg:=Erg OR Mask(p);
         FOR p:=0 TO r1 DO Erg:=Erg OR Mask(p);
	END;
      END;
    END;
   DecodeRegList:=True;
END;

	FUNCTION DecodeCondition(Asc:String; VAR Erg:Word):Boolean;
VAR
   z:Integer;
BEGIN
   z:=0; NLS_UpString(Asc);
   WHILE (z<ConditionCount) AND (Conditions^[z]<>Asc) DO Inc(z);
   Erg:=z; DecodeCondition:=z<ConditionCount;
END;

{--------------------------------------------------------------------------}

	FUNCTION CheckFormat(FSet:String):Boolean;
BEGIN
   CheckFormat:=True;
   IF Format=' ' THEN FormatCode:=0
   ELSE
    BEGIN
     FormatCode:=Pos(Format,FSet);
     CheckFormat:=FormatCode<>0;
     IF FormatCode=0 THEN WrError(1090);
    END;
END;

	FUNCTION CheckBFieldFormat:Boolean;
BEGIN
   CheckBFieldFormat:=True;
   IF (Format='G:R') OR (Format='R:G') THEN FormatCode:=1
   ELSE IF (Format='G:I') OR (Format='I:G') THEN FormatCode:=2
   ELSE IF (Format='E:R') OR (Format='R:E') THEN FormatCode:=3
   ELSE IF (Format='E:I') OR (Format='I:E') THEN FormatCode:=4
   ELSE
    BEGIN
     CheckBFieldFormat:=False; WrError(1090);
    END;
END;

	FUNCTION GetOpSize(VAR Asc:String; Index:Byte):Boolean;
VAR
   p:Integer;
BEGIN
   p:=RQuotPos(Asc,'.');
   IF p=0 THEN
    BEGIN
     GetOpSize:=True; OpSize[Index]:=DOpSize;
    END
   ELSE IF p=Length(Asc)-1 THEN
    BEGIN
     GetOpSize:=True;
     CASE Asc[Length(Asc)] OF
     'B':OpSize[Index]:=0;
     'H':OpSize[Index]:=1;
     'W':OpSize[Index]:=2;
     ELSE
      BEGIN
       WrError(1107); GetOpSize:=False;
      END;
     END;
     Delete(Asc,Length(Asc)-1,2);
    END
   ELSE
    BEGIN
     GetOpSize:=False; WrError(1107);
    END;
END;

	PROCEDURE SplitOptions;
VAR
   p,z:Integer;
BEGIN
   OptionCnt:=0; Options[1]:=''; Options[2]:='';
   REPEAT
    p:=RQuotPos(OpPart,'/');
    IF p>0 THEN
     BEGIN
      IF OptionCnt<2 THEN
       BEGIN
        FOR z:=OptionCnt DOWNTO 1 DO Options[z+1]:=Options[z];
        Inc(OptionCnt); Options[1]:=Copy(OpPart,p+1,Length(OpPart)-p);
       END;
      Byte(OpPart[0]):=p-1;
     END;
   UNTIL p=0;
END;

{--------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
BEGIN
   DecodePseudo:=True;

   DecodePseudo:=False;
END;

	PROCEDURE DecideBranch(Adr:LongInt; Index:Byte);
VAR
   Dist:LongInt;
BEGIN
   Dist:=Adr-EProgCounter;
   IF FormatCode=0 THEN
    BEGIN
     { Grî·enangabe erzwingt G-Format }
     IF OpSize[Index]<>-1 THEN FormatCode:=1
     { gerade 9-Bit-Zahl kurz darstellbar }
     ELSE IF (Dist AND 1=0) AND (Dist<=254) AND (Dist>=-256) THEN FormatCode:=2
     { ansonsten allgemein }
     ELSE FormatCode:=1;
    END;
   IF (FormatCode=1) AND (OpSize[Index]=-1) THEN
    IF (Dist<=127) AND (Dist>=-128) THEN OpSize[Index]:=0
    ELSE IF (Dist<=32767) AND (Dist>=-32768) THEN OpSize[Index]:=1
    ELSE OpSize[Index]:=2;
END;

	FUNCTION DecideBranchLength(VAR Addr:LongInt; Index:Integer):Boolean;
BEGIN
   Dec(Addr,EProgCounter);
   IF OpSize[Index]=-1 THEN
    BEGIN
     IF (Addr>=-128) AND (Addr<=127) THEN OpSize[Index]:=0
     ELSE IF (Addr>=-32768) AND (Addr<=32767) THEN OpSize[Index]:=1
     ELSE OpSize[Index]:=2;
    END;

   IF ((OpSize[Index]=0) AND ((Addr<-128) OR (Addr>127)))
   OR ((OpSize[Index]=1) AND ((Addr<-32768) OR (Addr>32767))) THEN
    BEGIN
     DecideBranchLength:=False; WrError(1370);
    END
   ELSE DecideBranchLength:=True;
END;

        PROCEDURE MakeCode_M16;
	Far;
VAR
   z,Num1:Integer;
   AdrWord,HReg,Mask,Mask2:Word;
   AdrLong,HVal:LongInt;
   OK:Boolean;
   Form:String[5];

	PROCEDURE Make_G(Code:Word);
BEGIN
   WAsmCode[0]:=$d000+(OpSize[1] SHL 8)+AdrMode[1];
   Move(AdrVals[1],WAsmCode[1],AdrCnt[1]);
   WAsmCode[1+AdrCnt2[1]]:=Code+(OpSize[2] SHL 8)+AdrMode[2];
   Move(AdrVals[2],WAsmCode[2+AdrCnt2[1]],AdrCnt[2]);
   CodeLen:=4+AdrCnt[1]+AdrCnt[2];
END;

	PROCEDURE Make_E(Code:Word; Signed:Boolean);
VAR
   HVal,Min,Max:LongInt;
BEGIN
   Min:=128*(-Ord(Signed)); Max:=Min+255;
   IF AdrType[1]<>ModImm THEN WrError(1350)
   ELSE
    BEGIN
     HVal:=ImmVal(1);
     IF ChkRange(HVal,Min,Max) THEN
      BEGIN
       WAsmCode[0]:=$bf00+(HVal AND $ff);
       WAsmCode[1]:=Code+(OpSize[2] SHL 8)+AdrMode[2];
       Move(AdrVals[2],WAsmCode[2],AdrCnt[2]);
       CodeLen:=4+AdrCnt[2];
      END;
    END;
END;

	PROCEDURE Make_I(Code:Word; Signed:Boolean);
BEGIN
   IF (AdrType[1]<>ModImm) OR (NOT IsShort(2)) THEN WrError(1350)
    ELSE
     BEGIN
      AdaptImm(1,OpSize[2],Signed);
      WAsmCode[0]:=Code+(OpSize[2] SHL 8)+AdrMode[2];
      Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
      Move(AdrVals[1],WAsmCode[1+AdrCnt2[2]],AdrCnt[1]);
      CodeLen:=2+AdrCnt[1]+AdrCnt[2];
     END;
END;

	FUNCTION Decode2:Boolean;
VAR
   z:Integer;
BEGIN
   Decode2:=True;

   IF (Memo('ADD')) OR (Memo('SUB')) THEN
    BEGIN
     z:=Ord(Memo('SUB'));
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GELQI') THEN
      IF GetOpSize(ArgStr[2],2) THEN
       IF GetOpSize(ArgStr[1],1) THEN
        BEGIN
         IF OpSize[2]=-1 THEN OpSize[2]:=2;
         IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[2];
         IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
          IF DecodeAdr(ArgStr[2],2,Mask_PureDest) THEN
           BEGIN
            IF FormatCode=0 THEN
             BEGIN
              IF AdrType[1]=ModImm THEN
               BEGIN
                HVal:=ImmVal(1);
                IF IsShort(2) THEN
                 IF (HVal>=1) AND (HVal<=8) THEN FormatCode:=4
                 ELSE FormatCode:=5
                ELSE IF (HVal>=-128) AND (HVal<127) THEN FormatCode:=2
                ELSE FormatCode:=1;
               END
              ELSE IF IsShort(1) AND (AdrType[2]=ModReg) AND (OpSize[1]=2) AND (OpSize[2]=2) THEN FormatCode:=3
              ELSE FormatCode:=1;
             END;
            CASE FormatCode OF
            1:Make_G(z SHL 11);
            2:Make_E(z SHL 11,True);
            3:IF (NOT IsShort(1)) OR (AdrType[2]<>ModReg) THEN WrError(1350)
              ELSE IF (OpSize[1]<>2) OR (OpSize[2]<>2) THEN WrError(1130)
	      ELSE
	       BEGIN
                WAsmCode[0]:=$8100+(z SHL 6)+((AdrMode[2] AND 15) SHL 10)+AdrMode[1];
                Move(AdrVals[1],WAsmCode[1],AdrCnt[1]);
                CodeLen:=2+AdrCnt[1];
                IF (AdrMode[1]=$04) AND (AdrMode[2]=15) THEN WrError(140);
               END;
            4:IF (AdrType[1]<>ModImm) OR (NOT IsShort(2)) THEN WrError(1350)
              ELSE
               BEGIN
                HVal:=ImmVal(1);
                IF ChkRange(HVal,1,8) THEN
                 BEGIN
                  WAsmCode[0]:=$4040+(z SHL 13)+((HVal AND 7) SHL 10)+(OpSize[2] SHL 8)+AdrMode[2];
                  Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                  CodeLen:=2+AdrCnt[2];
                 END;
               END;
            5:Make_I($44c0+(z SHL 11),True);
            END;
           END;
        END;
     Exit;
    END;

   IF Memo('CMP') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GELZQI') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        BEGIN
         IF OpSize[2]=-1 THEN OpSize[2]:=2;
         IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[2];
         IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
          IF DecodeAdr(ArgStr[2],2,Mask_NoImmGen-MModPush) THEN
           BEGIN
            IF FormatCode=0 THEN
             BEGIN
              IF AdrType[1]=ModImm THEN
               BEGIN
                HVal:=ImmVal(1);
                IF HVal=0 THEN FormatCode:=4
                ELSE IF (HVal>=1) AND (HVal<=8) AND (IsShort(2)) THEN FormatCode:=5
                ELSE IF (HVal>=-128) AND (HVal<=127) THEN FormatCode:=2
                ELSE IF AdrType[2]=ModReg THEN FormatCode:=3
                ELSE IF IsShort(2) THEN FormatCode:=5
                ELSE FormatCode:=1;
               END
              ELSE IF (IsShort(1)) AND (AdrType[2]=ModReg) THEN FormatCode:=3
              ELSE FormatCode:=1;
             END;
            CASE FormatCode OF
            1:Make_G($8000);
            2:Make_E($8000,True);
            3:IF (NOT IsShort(1)) OR (AdrType[2]<>ModReg) THEN WrError(1350)
              ELSE IF OpSize[1]<>2 THEN WrError(1130)
              ELSE
               BEGIN
                WAsmCode[0]:=((AdrMode[2] AND 15) SHL 10)+(OpSize[2] SHL 8)+AdrMode[1];
                Move(AdrVals[1],WAsmCode[1],AdrCnt[1]);
                CodeLen:=2+AdrCnt[1];
               END;
            4:IF AdrType[1]<>ModImm THEN WrError(1350)
              ELSE
               BEGIN
                HVal:=ImmVal(1);
                IF ChkRange(HVal,0,0) THEN
                 BEGIN
                  WAsmCode[0]:=$c000+(OpSize[2] SHL 8)+AdrMode[2];
                  Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                  CodeLen:=2+AdrCnt[2];
                 END;
               END;
            5:IF (AdrType[1]<>ModImm) OR (NOT IsShort(2)) THEN WrError(1350)
              ELSE
               BEGIN
                HVal:=ImmVal(1);
                IF ChkRange(HVal,1,8) THEN
                 BEGIN
                  WAsmCode[0]:=$4000+(OpSize[2] SHL 8)+AdrMode[2]+((HVal AND 7) SHL 10);
                  Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                  CodeLen:=2+AdrCnt[2];
                 END;
               END;
            6:Make_I($40c0,True);
            END;
           END;
        END;
     Exit;
    END;

   FOR z:=1 TO GE2OrderCount DO
    WITH GE2Orders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF CheckFormat('GE') THEN
        IF GetOpSize(ArgStr[2],2) THEN
         IF GetOpSize(ArgStr[1],1) THEN
          BEGIN
           IF OpSize[2]=-1 THEN OpSize[2]:=DefSize(SMask2);
           IF OpSize[1]=-1 THEN OpSize[1]:=DefSize(SMask1);
           IF (SMask1 AND (1 SHL OpSize[1])=0) OR (SMask2 AND (1 SHL OpSize[2])=0) THEN WrError(1130)
           ELSE IF DecodeAdr(ArgStr[1],1,Mask1) THEN
            IF DecodeAdr(ArgStr[2],2,Mask2) THEN
             BEGIN
              IF FormatCode=0 THEN
               BEGIN
                IF AdrType[1]=ModImm THEN
                 BEGIN
                  HVal:=ImmVal(1);
                  IF (Signed) AND (HVal>=-128) AND (HVal<=127) THEN FormatCode:=2
                  ELSE IF (NOT Signed) AND (HVal>=0) AND (HVal<=255) THEN FormatCode:=2
                  ELSE FormatCode:=1;
                 END
                ELSE FormatCode:=1;
               END;
              CASE FormatCode OF
              1:Make_G(Code);
              2:Make_E(Code,Signed);
              END;
             END;
          END;
       Exit;
      END;

   FOR z:=0 TO LogOrderCount-1 DO
    IF Memo(LogOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF CheckFormat('GERI') THEN
       IF GetOpSize(ArgStr[1],1) THEN
        IF GetOpSize(ArgStr[2],2) THEN
         BEGIN
          IF OpSize[2]=-1 THEN OpSize[2]:=2;
          IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[2];
          IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
           IF DecodeAdr(ArgStr[2],2,Mask_Dest-MModPush) THEN
            BEGIN
             IF FormatCode=0 THEN
              BEGIN
               IF AdrType[1]=ModImm THEN
                BEGIN
                 HVal:=ImmVal(1);
                 IF (HVal>=0) AND (HVal<=255) THEN FormatCode:=2
                 ELSE IF IsShort(2) THEN FormatCode:=4
                 ELSE FormatCode:=1;
                END
               ELSE IF (AdrType[1]=ModReg) AND (AdrType[2]=ModReg) AND (OpSize[1]=2) AND (OpSize[2]=2) THEN
                FormatCode:=3
               ELSE FormatCode:=1;
              END;
             CASE FormatCode OF
             1:Make_G($2000+(z SHL 10));
             2:Make_E($2000+(z SHL 10),False);
             3:IF (AdrType[1]<>ModReg) OR (AdrType[2]<>ModReg) THEN WrError(1350)
	       ELSE IF (OpSize[1]<>2) OR (OpSize[2]<>2) THEN WrError(1130)
	       ELSE
	        BEGIN
                 WAsmCode[0]:=$00c0+(z SHL 8)+(AdrMode[1] AND 15)+((AdrMode[2] AND 15) SHL 10);
                 CodeLen:=2;
		END;
             4:IF (AdrType[1]<>ModImm) OR (NOT IsShort(2)) THEN WrError(1350)
	       ELSE
	        BEGIN
                 WAsmCode[0]:=$50c0+(OpSize[2] SHL 8)+(z SHL 10)+AdrMode[2];
                 Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                 AdaptImm(1,OpSize[2],False);
                 Move(AdrVals[1],WAsmCode[1+AdrCnt2[2]],AdrCnt[1]);
                 CodeLen:=2+AdrCnt[1]+AdrCnt[2];
		END;
             END;
             IF OpSize[1]>OpSize[2] THEN WrError(140);
            END;
         END;
      Exit;
     END;

   FOR z:=0 TO MulOrderCount-1 DO
    IF Memo(MulOrders^[z]) THEN
     BEGIN
      IF Odd(z) THEN Form:='GE' ELSE Form:='GER';
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE IF CheckFormat(Form) THEN
       IF GetOpSize(ArgStr[1],1) THEN
        IF GetOpSize(ArgStr[2],2) THEN
         BEGIN
          IF OpSize[2]=-1 THEN OpSize[2]:=2;
          IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[2];
          IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
           IF DecodeAdr(ArgStr[2],2,Mask_PureDest) THEN
            BEGIN
             IF FormatCode=0 THEN
              BEGIN
               IF AdrType[1]=ModImm THEN
                BEGIN
                 HVal:=ImmVal(1);
                 IF (HVal>=-128+(Ord(Odd(z)) SHL 7)) AND
		    (HVal<=127+(Ord(Odd(z)) SHL 7)) THEN FormatCode:=2
		 ELSE FormatCode:=1;
                END
               ELSE IF (NOT Odd(z)) AND (AdrType[1]=ModReg) AND (OpSize[1]=2)
	           AND (AdrType[2]=ModReg) AND (OpSize[2]=2) THEN FormatCode:=3
               ELSE FormatCode:=1;
              END;
             CASE FormatCode OF
             1:Make_G($4000+(z SHL 10));
             2:Make_E($4000+(z SHL 10),NOT Odd(z));
             3:IF (AdrType[1]<>ModReg) OR (AdrType[2]<>ModReg) THEN WrError(1350)
               ELSE IF (OpSize[1]<>2) OR (OpSize[2]<>2) THEN WrError(1130)
               ELSE
                BEGIN
                 WAsmCode[0]:=$00d0+((AdrMode[2] AND 15) SHL 10)+(z SHL 7)+
		                     (AdrMode[1] AND 15);
		 CodeLen:=2;
                END;
             END;
            END;
         END;
      Exit;
     END;

   FOR z:=1 TO GetPutOrderCount DO
    WITH GetPutOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF Turn THEN
        BEGIN
         Mask:=Mask_Source; Mask2:=MModReg; AdrWord:=1;
        END
       ELSE
        BEGIN
         Mask:=MModReg; Mask2:=Mask_Dest; AdrWord:=2;
        END;
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF CheckFormat('G') THEN
        IF GetOpSize(ArgStr[1],1) THEN
         IF GetOpSize(ArgStr[2],2) THEN
          BEGIN
           IF OpSize[AdrWord]=-1 THEN OpSize[AdrWord]:=Size;
           IF OpSize[3-AdrWord]=-1 THEN OpSize[3-AdrWord]:=2;
           IF (OpSize[AdrWord]<>Size) OR (OpSize[3-AdrWord]<>2) THEN WrError(1130)
           ELSE IF DecodeAdr(ArgStr[1],1,Mask) THEN
            IF DecodeAdr(ArgStr[2],2,Mask2) THEN
             BEGIN
              Make_G(Code); Inc(WAsmCode[0],$0400);
             END;
          END;
       Exit;
      END;

   IF Memo('MOVA') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GR') THEN
      IF GetOpSize(ArgStr[2],2) THEN
       BEGIN
        IF OpSize[2]=-1 THEN OpSize[2]:=2;
        OpSize[1]:=0;
        IF OpSize[2]<>2 THEN WrError(1110)
        ELSE IF DecodeAdr(ArgStr[1],1,Mask_PureMem) THEN
         IF DecodeAdr(ArgStr[2],2,Mask_Dest) THEN
          BEGIN
           IF FormatCode=0 THEN
            IF (AdrType[1]=ModDisp16) AND (AdrType[2]=ModReg) THEN FormatCode:=2
            ELSE FormatCode:=1;
           CASE FormatCode OF
           1:BEGIN
	      Make_G($b400); Inc(WAsmCode[0],$800);
             END;
           2:IF (AdrType[1]<>ModDisp16) OR (AdrType[2]<>ModReg) THEN WrError(1350)
             ELSE
              BEGIN
               WAsmCode[0]:=$03c0+((AdrMode[2] AND 15) SHL 10)+(AdrMode[1] AND 15);
               WAsmCode[1]:=AdrVals[1][0];
               CodeLen:=4;
              END;
           END;
          END;
       END;
     Exit;
    END;

   IF (Memo('QINS')) OR (Memo('QDEL')) THEN
    BEGIN
     z:=Ord(Memo('QINS')) SHL 11;
     Mask:=Mask_PureMem;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF (Memo('QINS')) OR (GetOpSize(ArgStr[2],2)) THEN
       BEGIN
        IF OpSize[2]=-1 THEN OpSize[2]:=2;
        OpSize[1]:=0;
        IF OpSize[2]<>2 THEN WrError(1130)
        ELSE IF DecodeAdr(ArgStr[1],1,Mask) THEN
	 IF DecodeAdr(ArgStr[2],2,Mask+(Ord(Memo('QDEL'))*MModReg)) THEN
          BEGIN
           Make_G($b000+z); Inc(WAsmCode[0],$800);
          END;
       END;
     Exit;
    END;

   IF Memo('RVBY') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        BEGIN
         IF OpSize[2]=-1 THEN OpSize[2]:=2;
         IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[2];
         IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
 	  IF DecodeAdr(ArgStr[2],2,Mask_Dest) THEN
           BEGIN
            Make_G($4000); Inc(WAsmCode[0],$400);
           END;
        END;
     Exit;
    END;

   IF (Memo('SHL')) OR (Memo('SHA')) THEN
    BEGIN
     z:=Ord(Memo('SHA'));
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GEQ') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        BEGIN
         IF OpSize[1]=-1 THEN OpSize[1]:=0;
         IF OpSize[2]=-1 THEN OpSize[2]:=2;
         IF OpSize[1]<>0 THEN WrError(1130)
         ELSE IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
          IF DecodeAdr(ArgStr[2],2,Mask_PureDest) THEN
           BEGIN
            IF FormatCode=0 THEN
             BEGIN
              IF AdrType[1]=ModImm THEN
               BEGIN
                HVal:=ImmVal(1);
                IF (IsShort(2)) AND (Abs(HVal)>=1) AND (Abs(HVal)<=8) AND ((z=0) OR (HVal<0)) THEN FormatCode:=3
                ELSE IF (HVal>=-128) AND (HVal<=127) THEN FormatCode:=2
                ELSE FormatCode:=1;
               END
              ELSE FormatCode:=1;
             END;
            CASE FormatCode OF
            1:Make_G($3000+(z SHL 10));
            2:Make_E($3000+(z SHL 10),True);
            3:IF (AdrType[1]<>ModImm) OR (NOT IsShort(2)) THEN WrError(1350)
              ELSE
               BEGIN
                HVal:=ImmVal(1);
                IF ChkRange(HVal,-8,(1-z) SHL 3) THEN
                 IF HVal=0 THEN WrError(1135)
                 ELSE
                  BEGIN
                   IF HVal<0 THEN Inc(HVal,16)
                   ELSE HVal:=HVal AND 7;
                   WAsmCode[0]:=$4080+(HVal SHL 10)+(z SHL 6)+(OpSize[2] SHL 8)+AdrMode[2];
                   Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                   CodeLen:=2+AdrCnt[2];
                  END;
               END;
            END;
           END;
        END;
     Exit;
    END;

   IF (Memo('SHXL')) OR (Memo('SHXR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       BEGIN
        IF OpSize[1]=-1 THEN OpSize[1]:=2;
        IF OpSize[1]<>2 THEN WrError(1130)
        ELSE IF DecodeAdr(ArgStr[1],1,Mask_PureDest) THEN
	 BEGIN
          WAsmCode[0]:=$02f7;
          WAsmCode[1]:=$8a00+(Ord(Memo('SHXR')) SHL 12)+AdrMode[1];
          Move(AdrVals,WAsmCode[1],AdrCnt[1]);
          CodeLen:=4+AdrCnt[1];
	 END;
       END;
     Exit;
    END;

   Decode2:=False;
END;

BEGIN
   DOpSize:=-1; FOR z:=1 TO ArgCnt DO OpSize[z]:=-1;

   { zu ignorierendes }

   IF Memo('') THEN Exit;

   { Formatangabe abspalten }

   CASE AttrSplit OF
   '.':BEGIN
        Num1:=Pos(':',AttrPart);
        IF Num1<>0 THEN
         BEGIN
          IF Num1<Length(AttrPart) THEN Format:=AttrPart[Num1+1]
          ELSE Format:=' ';
          Delete(AttrPart,Num1,Length(AttrPart)-Num1+1);
         END
        ELSE Format:=' ';
       END;
   ':':BEGIN
        Num1:=Pos('.',AttrPart);
        IF Num1=0 THEN
         BEGIN
          Format:=AttrPart; AttrPart:='';
         END
        ELSE
         BEGIN
          IF Num1=1 THEN Format:=' ' ELSE Format:=Copy(AttrPart,1,Num1-1);
          Delete(AttrPart,1,Num1);
         END;
       END;
   ELSE Format:=' ';
   END;
   NLS_UpString(Format);

   { Attribut abarbeiten }

   IF AttrPart='' THEN DOpSize:=-1
   ELSE
    CASE UpCase(AttrPart[1]) OF
    'B':DOpSize:=0;
    'H':DOpSize:=1;
    'W':DOpSize:=2;
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
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE IF Format<>' ' THEN WrError(1090)
       ELSE
        BEGIN
         CodeLen:=2; WAsmCode[0]:=Code;
        END;
       Exit;
      END;

   IF (Memo('STOP')) OR (Memo('SLEEP')) THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE
      BEGIN
       CodeLen:=10;
       WAsmCode[0]:=$d20c;
       IF Memo('STOP') THEN
        BEGIN
         WAsmCode[1]:=$5374;
         WAsmCode[2]:=$6f70;
        END
       ELSE
        BEGIN
         WAsmCode[1]:=$5761;
         WAsmCode[2]:=$6974;
        END;
       WAsmCode[3]:=$9e09;
       WAsmCode[4]:=$0700;
      END;
     Exit;
    END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GELSZQI') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        BEGIN
         IF OpSize[2]=-1 THEN OpSize[2]:=2;
         IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[2];
         IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
          IF DecodeAdr(ArgStr[2],2,Mask_AllGen-MModPop) THEN
           BEGIN
            IF FormatCode=0 THEN
             BEGIN
              IF AdrType[1]=ModImm THEN
               BEGIN
                HVal:=ImmVal(1);
                IF HVal=0 THEN FormatCode:=5
                ELSE IF (HVal>=1) AND (HVal<=8) AND (IsShort(2)) THEN FormatCode:=6
                ELSE IF (HVal>=-128) AND (HVal<=127) THEN FormatCode:=2
                ELSE IF IsShort(2) THEN FormatCode:=7
		ELSE FormatCode:=1;
               END
              ELSE IF (AdrType[1]=ModReg) AND (OpSize[1]=2) AND (IsShort(2)) THEN FormatCode:=4
              ELSE IF (AdrType[2]=ModReg) AND (OpSize[2]=2) AND (IsShort(1)) THEN FormatCode:=3
              ELSE FormatCode:=1;
             END;
            CASE FormatCode OF
            1:Make_G($8800);
            2:Make_E($8800,True);
            3:IF (NOT IsShort(1)) OR (AdrType[2]<>ModReg) THEN WrError(1350)
              ELSE IF OpSize[2]<>2 THEN WrError(1130)
              ELSE
               BEGIN
                WAsmCode[0]:=$0040+((AdrMode[2] AND 15) SHL 10)+(OpSize[1] SHL 8)+AdrMode[1];
                Move(AdrVals[1],WAsmCode[1],AdrCnt[1]);
                CodeLen:=2+AdrCnt[1];
               END;
            4:IF (NOT IsShort(2)) OR (AdrType[1]<>ModReg) THEN WrError(1350)
              ELSE IF OpSize[1]<>2 THEN WrError(1130)
              ELSE
               BEGIN
                WAsmCode[0]:=$0080+((AdrMode[1] AND 15) SHL 10)+(OpSize[2] SHL 8)+AdrMode[2];
                Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                CodeLen:=2+AdrCnt[2];
               END;
            5:IF AdrType[1]<>ModImm THEN WrError(1350)
              ELSE
               BEGIN
                HVal:=ImmVal(1);
                IF ChkRange(HVal,0,0) THEN
                 BEGIN
                  WAsmCode[0]:=$c400+(OpSize[2] SHL 8)+AdrMode[2];
                  Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                  CodeLen:=2+AdrCnt[2];
                 END;
               END;
            6:IF (AdrType[1]<>ModImm) OR (NOT IsShort(2)) THEN WrError(1350)
              ELSE
               BEGIN
                HVal:=ImmVal(1);
                IF ChkRange(HVal,1,8) THEN
                 BEGIN
                  WAsmCode[0]:=$6000+((HVal AND 7) SHL 10)+(OpSize[2] SHL 8)+AdrMode[2];
                  Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                  CodeLen:=2+AdrCnt[2];
                 END;
               END;
            7:Make_I($48c0,True);
            END;
           END;
        END;
     Exit;
    END;

   { ein Operand }

   FOR z:=1 TO OneOrderCount DO
    WITH OneOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF CheckFormat('G') THEN
        IF GetOpSize(ArgStr[1],1) THEN
         BEGIN
          IF (OpSize[1]=-1) AND (OpMask<>0) THEN OpSize[1]:=DefSize(OpMask);
          IF (OpSize[1]<>-1) AND (1 SHL OpSize[1] AND OpMask=0) THEN WrError(1130)
          ELSE
           BEGIN
            IF DecodeAdr(ArgStr[1],1,Mask) THEN
             BEGIN
              { da nur G, Format ignorieren }
              WAsmCode[0]:=Code+AdrMode[1];
              IF OpMask<>0 THEN Inc(WAsmCode[0],OpSize[1] SHL 8);
              Move(AdrVals[1],WAsmCode[1],AdrCnt[1]);
              CodeLen:=2+AdrCnt[1];
             END;
           END;
         END;
     Exit;
    END;

   { zwei Operanden }

   IF Decode2 THEN Exit;

   { drei Operanden }

   IF (Memo('CHK/N')) OR (Memo('CHK/S')) OR (Memo('CHK')) THEN
    BEGIN
     z:=Ord(OpPart[Length(OpPart)]='S');
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF GetOpSize(ArgStr[1],2) THEN
       IF GetOpSize(ArgStr[2],1) THEN
        IF GetOpSize(ArgStr[3],3) THEN
         BEGIN
          IF OpSize[3]=-1 THEN OpSize[3]:=2;
          IF OpSize[2]=-1 THEN OpSize[2]:=OpSize[3];
          IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[3];
          IF (OpSize[1]<>OpSize[2]) OR (OpSize[2]<>OpSize[3]) THEN WrError(1131)
          ELSE
	   IF DecodeAdr(ArgStr[1],2,Mask_MemGen-MModPop-MModPush) THEN
            IF DecodeAdr(ArgStr[2],1,Mask_Source) THEN
             IF DecodeAdr(ArgStr[3],3,MModReg) THEN
              BEGIN
               OpSize[2]:=2+z;
               Make_G((AdrMode[3] AND 15) SHL 10);
	       Inc(WAsmcode[0],$400);
              END;
         END;
     Exit;
    END;

   IF Memo('CSI') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF GetOpSize(ArgStr[1],3) THEN
       IF GetOpSize(ArgStr[2],1) THEN
        IF GetOpSize(ArgStr[3],2) THEN
         BEGIN
          IF OpSize[3]=-1 THEN OpSize[3]:=2;
          IF OpSize[2]=-1 THEN OpSize[2]:=OpSize[3];
          IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[2];
          IF (OpSize[1]<>OpSize[2]) OR (OpSize[2]<>OpSize[3]) THEN WrError(1131)
          ELSE IF DecodeAdr(ArgStr[1],3,MModReg) THEN
           IF DecodeAdr(ArgStr[2],1,Mask_Source) THEN
            IF DecodeAdr(ArgStr[3],2,Mask_PureMem) THEN
             BEGIN
              OpSize[2]:=0;
              Make_G((AdrMode[3] AND 15) SHL 10);
              Inc(WAsmCode[0],$400);
             END;
         END;
     Exit;
    END;

   IF (Memo('DIVX')) OR (Memo('MULX')) THEN
    BEGIN
     z:=Ord(Memo('DIVX'));
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        IF GetOpSize(ArgStr[3],3) THEN
         BEGIN
          IF OpSize[3]=-1 THEN OpSize[3]:=2;
          IF OpSize[2]=-1 THEN OpSize[2]:=OpSize[3];
          IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[2];
          IF (OpSize[1]<>2) OR (OpSize[2]<>2) OR (OpSize[3]<>2) THEN WrError(1130)
          ELSE IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
           IF DecodeAdr(ArgStr[2],2,Mask_PureDest) THEN
            IF DecodeAdr(ArgStr[3],3,MModReg) THEN
             BEGIN
              OpSize[2]:=0;
              Make_G($8200+((AdrMode[3] AND 15) SHL 10)+(z SHL 8));
              Inc(WAsmCode[0],$400);
             END;
         END;
     Exit;
    END;

   { Bitoperationen }

   FOR z:=1 TO BitOrderCount DO
    WITH BitOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF Code2<>0 THEN Form:='GER' ELSE Form:='GE';
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF CheckFormat(Form) THEN
        IF GetOpSize(ArgStr[1],1) THEN
         IF GetOpSize(ArgStr[2],2) THEN
          BEGIN
           IF OpSize[1]=-1 THEN OpSize[1]:=2;
           IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
            IF DecodeAdr(ArgStr[2],2,Mask_PureDest) THEN
             BEGIN
              IF OpSize[2]=-1 THEN
               IF (AdrType[2]=ModReg) AND (NOT MustByte) THEN OpSize[2]:=2
	      ELSE OpSize[2]:=0;
              IF ((AdrType[2]<>ModReg) OR (MustByte)) AND (OpSize[2]<>0) THEN WrError(1130)
              ELSE
               BEGIN
                IF FormatCode=0 THEN
                 BEGIN
                  IF AdrType[1]=ModImm THEN
                   BEGIN
                    HVal:=ImmVal(1);
		    IF (HVal>=0) AND (HVal<=7) AND (IsShort(2)) AND (Code2<>0) AND (OpSize[2]=0) THEN FormatCode:=3
		    ELSE IF (HVal>=-128) AND (HVal<127) THEN FormatCode:=2
		    ELSE FormatCode:=1;
		   END
		  ELSE FormatCode:=1;
                 END;
                CASE FormatCode OF
                1:Make_G(Code1);
                2:Make_E(Code1,True);
                3:IF (AdrType[1]<>ModImm) OR (NOT IsShort(2)) THEN WrError(1350)
                  ELSE IF OpSize[2]<>0 THEN WrError(1130)
		  ELSE
		   BEGIN
                    HVal:=ImmVal(1);
                    IF ChkRange(HVal,0,7) THEN
                     BEGIN
                      WAsmCode[0]:=Code2+((HVal AND 7) SHL 10)+AdrMode[2];
                      Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                      CodeLen:=2+AdrCnt[2];
                     END;
		   END;
                END;
               END;
             END;
          END;
       Exit;
      END;

   FOR z:=0 TO BFieldOrderCount-1 DO
    IF Memo(BFieldOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>4 THEN WrError(1110)
      ELSE IF CheckBFieldFormat THEN
       IF GetOpSize(ArgStr[1],1) THEN
        IF GetOpSize(ArgStr[2],2) THEN
         IF GetOpSize(ArgStr[3],3) THEN
          IF GetOpSize(ArgStr[4],4) THEN
           BEGIN
            IF OpSize[1]=-1 THEN OpSize[1]:=2;
            IF OpSize[2]=-1 THEN OpSize[2]:=2;
            IF OpSize[3]=-1 THEN OpSize[3]:=2;
            IF OpSize[4]=-1 THEN OpSize[4]:=2;
            IF DecodeAdr(ArgStr[1],1,MModReg+MModImm) THEN
             IF DecodeAdr(ArgStr[3],3,MModReg+MModImm) THEN
              BEGIN
               IF AdrType[3]=ModReg THEN Mask:=Mask_Source ELSE Mask:=MModImm;
               IF DecodeAdr(ArgStr[2],2,Mask) THEN
                IF DecodeAdr(ArgStr[4],4,Mask_PureMem) THEN
                 BEGIN
                  IF FormatCode=0 THEN
                   BEGIN
                    IF AdrType[3]=ModReg THEN
                     IF AdrType[1]=ModReg THEN FormatCode:=1 ELSE FormatCode:=2
                    ELSE
                     IF AdrType[1]=ModReg THEN FormatCode:=3 ELSE FormatCode:=4
                   END;
                  CASE FormatCode OF
                  1:IF (AdrType[1]<>ModReg) OR (AdrType[3]<>ModReg) THEN WrError(1350)
                    ELSE IF (OpSize[1]<>2) OR (OpSize[3]<>2) OR (OpSize[4]<>2) THEN WrError(1130)
                    ELSE
                     BEGIN
                      WAsmCode[0]:=$d000+(OpSize[2] SHL 8)+AdrMode[2];
                      Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                      WAsmCode[1+AdrCnt2[2]]:=$c200+(z SHL 10)+AdrMode[4];
                      Move(AdrVals[4],WAsmCode[2+AdrCnt2[2]],AdrCnt[4]);
                      WAsmCode[2+AdrCnt2[2]+AdrCnt2[4]]:=((AdrMode[3] AND 15) SHL 10)+(AdrMode[1] AND 15);
                      CodeLen:=6+AdrCnt[2]+AdrCnt[4];
                     END;
                  2:IF (AdrType[1]<>ModImm) OR (AdrType[3]<>ModReg) THEN WrError(1350)
                    ELSE IF (OpSize[3]<>2) OR (OpSize[4]<>2) THEN WrError(1130)
                    ELSE
                     BEGIN
                      WAsmCode[0]:=$d000+(OpSize[2] SHL 8)+AdrMode[2];
                      Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
                      WAsmCode[1+AdrCnt2[2]]:=$d200+(z SHL 10)+AdrMode[4];
                      Move(AdrVals[4],WAsmCode[2+AdrCnt2[2]],AdrCnt[4]);
                      WAsmCode[2+AdrCnt2[2]+AdrCnt2[4]]:=((AdrMode[3] AND 15) SHL 10)+(OpSize[1] SHL 8);
                      CodeLen:=6+AdrCnt[2]+AdrCnt[4];
                      IF OpSize[1]=0 THEN Inc(WAsmCode[(CodeLen-2) SHR 1],AdrVals[1][0] AND $ff)
                      ELSE
                       BEGIN
                        Move(AdrVals[1],WAsmCode[CodeLen SHR 1],AdrCnt[1]);
                        Inc(CodeLen,AdrCnt[1]);
                       END;
                     END;
                  3:IF (AdrType[1]<>ModReg) OR (AdrType[2]<>ModImm) OR (AdrType[3]<>ModImm) THEN WrError(1350)
                    ELSE IF (OpSize[1]<>2) OR (OpSize[4]<>2) THEN WrError(1130)
                    ELSE
                     BEGIN
                      HVal:=ImmVal(2);
                      IF ChkRange(HVal,-128,-127) THEN
                       BEGIN
                        AdrLong:=ImmVal(3);
                        IF ChkRange(AdrLong,1,32) THEN
                         BEGIN
                          WAsmCode[0]:=$bf00+(HVal AND $ff);
                          WAsmCode[1]:=$c200+(z SHL 10)+AdrMode[4];
                          Move(AdrVals[4],WAsmCode[2],AdrCnt[4]);
                          WAsmCode[2+AdrCnt2[4]]:=((AdrLong AND 31) SHL 10)+(AdrMode[1] AND 15);
                          CodeLen:=6+AdrCnt[4];
                         END;
                       END;
                     END;
                  4:IF (AdrType[1]<>ModImm) OR (AdrType[2]<>ModImm) OR (AdrType[3]<>ModImm) THEN WrError(1350)
                    ELSE IF (OpSize[4]<>2) THEN WrError(1130)
                    ELSE
                     BEGIN
                      HVal:=ImmVal(2);
                      IF ChkRange(HVal,-128,-127) THEN
                       BEGIN
                        AdrLong:=ImmVal(3);
                        IF ChkRange(AdrLong,1,32) THEN
                         BEGIN
                          WAsmCode[0]:=$bf00+(HVal AND $ff);
                          WAsmCode[1]:=$d200+(z SHL 10)+AdrMode[4];
                          Move(AdrVals[4],WAsmCode[2],AdrCnt[4]);
                          WAsmCode[2+AdrCnt2[4]]:=((AdrLong AND 31) SHL 10)+(OpSize[1] SHL 8);
                          CodeLen:=6+AdrCnt[4];
                          IF OpSize[1]=0 THEN Inc(WAsmCode[(CodeLen-1) SHR 1],AdrVals[1][0] AND $ff)
                          ELSE
                           BEGIN
                            Move(AdrVals[1],WAsmCode[CodeLen SHR 1],AdrCnt[1]);
                            Inc(CodeLen,AdrCnt[1]);
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

   IF (Memo('BFEXT')) OR (Memo('BFEXTU')) THEN
    BEGIN
     z:=Ord(Memo('BFEXTU'));
     IF ArgCnt<>4 THEN WrError(1110)
     ELSE IF CheckFormat('GE') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        IF GetOpSize(ArgStr[3],3) THEN
         IF GetOpSize(ArgStr[4],4) THEN
          BEGIN
           IF OpSize[1]=-1 THEN OpSize[1]:=2;
           IF OpSize[2]=-1 THEN OpSize[2]:=2;
           IF OpSize[3]=-1 THEN OpSize[3]:=2;
           IF OpSize[4]=-1 THEN OpSize[4]:=2;
           IF DecodeAdr(ArgStr[4],4,MModReg) THEN
            IF DecodeAdr(ArgStr[3],3,Mask_MemGen-MModPop-MModPush) THEN
             IF DecodeAdr(ArgStr[2],2,MmodReg+MModImm) THEN
              BEGIN
               IF AdrType[2]=ModReg THEN Mask:=Mask_Source ELSE Mask:=MModImm;
               IF DecodeAdr(ArgStr[1],1,Mask) THEN
                BEGIN
                 IF FormatCode=0 THEN
                  BEGIN
                   IF AdrType[2]=ModReg THEN FormatCode:=1 ELSE FormatCode:=2;
                  END;
                 CASE FormatCode OF
                 1:IF (OpSize[2]<>2) OR (OpSize[3]<>2) OR (OpSize[4]<>2) THEN WrError(1130)
                   ELSE
                    BEGIN
                     WAsmCode[0]:=$d000+(OpSize[1] SHL 8)+AdrMode[1];
                     Move(AdrVals[1],WAsmCode[1],AdrCnt[1]);
                     WAsmCode[1+AdrCnt2[1]]:=$ea00+(z SHL 10)+AdrMode[3];
                     Move(AdrVals[3],WAsmCode[2+AdrCnt2[1]],AdrCnt[3]);
                     WAsmCode[2+AdrCnt2[1]+AdrCnt2[3]]:=((AdrMode[2] AND 15) SHL 10)+(AdrMode[4] AND 15);
                     CodeLen:=6+AdrCnt[1]+AdrCnt[3];
                    END;
                 2:IF (AdrType[1]<>ModImm) OR (AdrType[2]<>ModImm) THEN WrError(1350)
                   ELSE IF (OpSize[3]<>2) OR (OpSize[4]<>2) THEN WrError(1350)
                   ELSE
                    BEGIN
                     HVal:=ImmVal(1);
                     IF ChkRange(HVal,-128,127) THEN
                      BEGIN
                       AdrLong:=ImmVal(2);
                       IF ChkRange(AdrLong,1,32) THEN
		        BEGIN
                         WAsmCode[0]:=$bf00+(HVal AND $ff);
                         WAsmCode[1]:=$ea00+(z SHL 10)+AdrMode[3];
                         Move(AdrVals[3],WAsmCode[2],AdrCnt[3]);
                         WAsmCode[2+AdrCnt2[3]]:=((AdrLong AND 31) SHL 10)+(AdrMode[4] AND 15);
                         CodeLen:=6+AdrCnt[3];
                        END;
                      END;
                    END;
                 END;
                END;
              END;
          END;
     Exit;
    END;

   IF (Memo('BSCH/0')) OR (Memo('BSCH/1')) THEN
    BEGIN
     z:=Ord(OpPart[6])-AscOfs;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        BEGIN
         IF OpSize[1]=-1 THEN OpSize[1]:=2;
         IF OpSize[2]=-1 THEN OpSize[2]:=2;
         IF OpSize[1]<>2 THEN WrError(1130)
         ELSE
	  IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
	   IF DecodeAdr(ArgStr[2],2,Mask_PureDest) THEN
	    BEGIN
             { immer G-Format }
             WAsmCode[0]:=$d600+AdrMode[1];
             Move(AdrVals[1],WAsmCode[1],AdrCnt[1]);
             WAsmCode[1+AdrCnt2[1]]:=$5000+(z SHL 10)+(OpSize[2] SHL 8)+AdrMode[2];
             Move(AdrVals[2],WAsmCode[2+AdrCnt2[1]],AdrCnt[2]);
             CodeLen:=4+AdrCnt[1]+AdrCnt[2];
	    END;
        END;
     Exit;
    END;

   { SprÅnge }

   IF (Memo('BSR')) OR (Memo('BRA')) THEN
    BEGIN
     z:=Ord(Memo('BSR'));
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      IF CheckFormat('GD') THEN
       IF GetOpSize(ArgStr[1],1) THEN
        BEGIN
         AdrLong:=EvalIntExpression(ArgStr[1],Int32,OK);
         IF OK THEN
          BEGIN
           DecideBranch(AdrLong,1);
           CASE FormatCode OF
           2:IF OpSize[1]<>-1 THEN WrError(1100)
             ELSE
              BEGIN
               Dec(AdrLong,EProgCounter);
               IF (AdrLong<-256) OR (AdrLong>254) THEN WrError(1370)
               ELSE IF Odd(AdrLong) THEN WrError(1375)
               ELSE
                BEGIN
                 CodeLen:=2;
                 WAsmCode[0]:=$ae00+(z SHL 8)+Lo(AdrLong SHR 1);
                END
              END;
           1:BEGIN
              WAsmCode[0]:=$20f7+(z SHL 11)+(Word(OpSize[1]) SHL 8);
              Dec(AdrLong,EProgCounter);
	      CASE OpSize[1] OF
              0:IF (AdrLong<-128) OR (AdrLong>127) THEN WrError(1370)
                ELSE
                 BEGIN
                  CodeLen:=4; WAsmCode[1]:=Lo(AdrLong);
                 END;
              1:IF (AdrLong<-32768) OR (AdrLong>32767) THEN WrError(1370)
                ELSE
                 BEGIN
                  CodeLen:=4; WAsmCode[1]:=AdrLong AND $ffff;
                 END;
              2:BEGIN
                 CodeLen:=6; WAsmCode[1]:=AdrLong SHR 16;
                 WAsmCode[2]:=AdrLong AND $ffff;
                END;
              END;
             END;
           END;
          END;
        END;
     Exit;
    END;

   IF OpPart[1]='B' THEN
    FOR z:=0 TO ConditionCount-1 DO
     IF Memo('B'+Conditions^[z]) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE
        IF CheckFormat('GD') THEN
         IF GetOpSize(ArgStr[1],1) THEN
          BEGIN
           AdrLong:=EvalIntExpression(ArgStr[1],Int32,OK);
           IF OK THEN
            BEGIN
             DecideBranch(AdrLong,1);
             CASE FormatCode OF
             2:IF OpSize[1]<>-1 THEN WrError(1100)
               ELSE
                BEGIN
                 Dec(AdrLong,EProgCounter);
                 IF (AdrLong<-256) OR (AdrLong>254) THEN WrError(1370)
                 ELSE IF Odd(AdrLong) THEN WrError(1375)
                 ELSE
                  BEGIN
                   CodeLen:=2;
                   WAsmCode[0]:=$8000+(z SHL 10)+Lo(AdrLong SHR 1);
                  END
                END;
             1:BEGIN
                WAsmCode[0]:=$00f6+(z SHL 10)+(Word(OpSize[1]) SHL 8);
                Dec(AdrLong,EProgCounter);
	        CASE OpSize[1] OF
                0:IF (AdrLong<-128) OR (AdrLong>127) THEN WrError(1370)
                  ELSE
                   BEGIN
                    CodeLen:=4; WAsmCode[1]:=Lo(AdrLong);
                   END;
                1:IF (AdrLong<-32768) OR (AdrLong>32767) THEN WrError(1370)
                  ELSE
                   BEGIN
                    CodeLen:=4; WAsmCode[1]:=AdrLong AND $ffff;
                   END;
                2:BEGIN
                   CodeLen:=6; WAsmCode[1]:=AdrLong SHR 16;
                   WAsmCode[2]:=AdrLong AND $ffff;
                  END;
                END;
               END;
             END;
            END;
          END;
       Exit;
      END;

   IF (Memo('ACB')) OR (Memo('SCB')) THEN
    BEGIN
     AdrWord:=Ord(Memo('SCB'));
     IF ArgCnt<>4 THEN WrError(1110)
     ELSE IF CheckFormat('GEQR') THEN
      IF GetOpSize(ArgStr[2],3) THEN
      IF GetOpSize(ArgStr[4],4) THEN
      IF GetOpSize(ArgStr[1],1) THEN
      IF GetOpSize(ArgStr[3],2) THEN
       BEGIN
        IF (OpSize[3]=-1) AND (OpSize[2]=-1) THEN OpSize[3]:=2;
        IF (OpSize[3]=-1) AND (OpSize[2]<>-1) THEN OpSize[3]:=OpSize[2]
        ELSE IF (OpSize[3]<>-1) AND (OpSize[2]=-1) THEN OpSize[2]:=OpSize[3];
        IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[2];
        IF OpSize[3]<>OpSize[2] THEN WrError(1131)
        ELSE IF NOT DecodeReg(ArgStr[2],HReg) THEN WrError(1350)
	ELSE
	 BEGIN
          AdrLong:=EvalIntExpression(ArgStr[4],Int32,OK);
          IF OK THEN
           BEGIN
            IF DecodeAdr(ArgStr[1],1,Mask_Source) THEN
             IF DecodeAdr(ArgStr[3],2,Mask_Source) THEN
              BEGIN
               IF FormatCode=0 THEN
                BEGIN
                 IF AdrType[1]<>ModImm THEN FormatCode:=1
                 ELSE
                  BEGIN
                   HVal:=ImmVal(1);
                   IF (HVal=1) AND (AdrType[2]=ModReg) THEN FormatCode:=4
                   ELSE IF (HVal=1) AND (AdrType[2]=ModImm) THEN
                    BEGIN
                     HVal:=ImmVal(2);
                     IF (HVal>=1-AdrWord) AND (HVal<=64-AdrWord) THEN FormatCode:=3
                     ELSE FormatCode:=2;
                    END
                   ELSE IF (HVal>=-128) AND (HVal<=127) THEN FormatCode:=2
                   ELSE FormatCode:=1;
                  END;
                END;
               CASE FormatCode OF
               1:IF DecideBranchLength(AdrLong,4) THEN  { ??? }
                  BEGIN
                   WAsmCode[0]:=$d000+(OpSize[1] SHL 8)+AdrMode[1];
                   Move(AdrVals[1],WAsmCode[1],AdrCnt[1]);
                   WAsmCode[1+AdrCnt2[1]]:=$f000+(AdrWord SHL 11)+(OpSize[2] SHL 8)+AdrMode[2];
                   Move(AdrVals[2],WAsmCode[2+AdrCnt2[1]],AdrCnt[2]);
                   WAsmCode[2+AdrCnt2[1]+AdrCnt2[2]]:=(HReg SHL 10)+(OpSize[4] SHL 8);
                   CodeLen:=6+AdrCnt[1]+AdrCnt[2];
                  END;
               2:IF DecideBranchLength(AdrLong,4) THEN   { ??? }
                  IF AdrType[1]<>ModImm THEN WrError(1350)
                  ELSE
		   BEGIN
                    HVal:=ImmVal(1);
                    IF ChkRange(HVal,-128,127) THEN
                     BEGIN
                      WAsmCode[0]:=$bf00+(HVal AND $ff);
                      WAsmCode[1]:=$f000+(AdrWord SHL 11)+(OpSize[2] SHL 8)+AdrMode[2];
                      Move(AdrVals[2],WAsmCode[2],AdrCnt[2]);
                      WAsmCode[2+AdrCnt2[2]]:=(HReg SHL 10)+(OpSize[4] SHL 8);
                      CodeLen:=6+AdrCnt[2];
                     END;
		   END;
               3:IF DecideBranchLength(AdrLong,4) THEN   { ??? }
                  IF AdrType[1]<>ModImm THEN WrError(1350)
                  ELSE IF ImmVal(1)<>1 THEN WrError(1135)
                  ELSE IF AdrType[2]<>ModImm THEN WrError(1350)
                  ELSE
                   BEGIN
                    HVal:=ImmVal(2);
                    IF ChkRange(HVal,1-AdrWord,64-AdrWord) THEN
                     BEGIN
                      WAsmCode[0]:=$03d1+(HReg SHL 10)+(AdrWord SHL 1);
                      WAsmCode[1]:=((HVal AND $3f) SHL 10)+(OpSize[4] SHL 8);
                      CodeLen:=4;
                     END;
                   END;
               4:IF DecideBranchLength(AdrLong,4) THEN   { ??? }
                  IF AdrType[1]<>ModImm THEN WrError(1350)
                  ELSE IF ImmVal(1)<>1 THEN WrError(1135)
                  ELSE IF OpSize[2]<>2 THEN WrError(1130)
                  ELSE IF AdrType[2]<>ModReg THEN WrError(1350)
                  ELSE
                   BEGIN
                    WAsmCode[0]:=$03d0+(HReg SHL 10)+(AdrWord SHL 1);
                    WAsmCode[1]:=((AdrMode[2] AND 15) SHL 10)+(OpSize[4] SHL 8);
                    CodeLen:=4;
                   END;
               END;
               IF CodeLen>0 THEN
		CASE OpSize[4] OF
		0:Inc(WAsmCode[(CodeLen SHR 1)-1],AdrLong AND $ff);
                1:BEGIN
                   WAsmCode[CodeLen SHR 1]:=AdrLong AND $ffff;
                   Inc(CodeLen,2);
                  END;
                2:BEGIN
                   WAsmCode[ CodeLen SHR 1   ]:=AdrLong SHR 16;
                   WAsmCode[(CodeLen SHR 1)+1]:=AdrLong AND $ffff;
                   Inc(CodeLen,4);
                  END;
                END;
              END;
           END;
	 END;
       END;
     Exit;
    END;

   IF Memo('TRAPA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       AdrWord:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt4,OK);
       IF OK THEN
        BEGIN
         CodeLen:=2; WAsmCode[0]:=$03d5+(AdrWord SHL 10);
        END;
      END;
     Exit;
    END;

   IF Copy(OpPart,1,4)='TRAP' THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE
      BEGIN
       SplitOptions;
       IF OptionCnt<>1 THEN WrError(1115)
       ELSE IF NOT DecodeCondition(Options[1],AdrWord) THEN WrError(1360)
       ELSE
        BEGIN
         CodeLen:=2; WAsmCode[0]:=$03d4+(AdrWord SHL 10);
        END;
      END;
     Exit;
    END;

   { Specials }

   IF (Memo('ENTER')) OR (Memo('EXITD')) THEN
    BEGIN
     IF Memo('EXITD') THEN
      BEGIN
       z:=1; ArgStr[3]:=ArgStr[1];
       ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
      END
     ELSE z:=0;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('GE') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        BEGIN
         IF OpSize[1]=-1 THEN OpSize[1]:=2;
         IF OpSize[2]=-1 THEN OpSize[2]:=2;
         IF OpSize[2]<>2 THEN WrError(1130)
         ELSE IF DecodeAdr(ArgStr[1],1,MModReg+MModImm) THEN
          IF DecodeRegList(ArgStr[2],AdrWord,z=1) THEN
           IF z AND $c000<>0 THEN WrXError(1410,'SP/FP')
           ELSE
            BEGIN
             IF FormatCode=0 THEN
              BEGIN
               IF AdrType[1]=ModImm THEN
                BEGIN
                 HVal:=ImmVal(1);
                 IF (HVal>=-128) AND (HVal<=127) THEN FormatCode:=2
                 ELSE FormatCode:=1;
                END
               ELSE FormatCode:=1;
              END;
             CASE FormatCode OF
             1:BEGIN
                WAsmCode[0]:=$02f7;
                WAsmCode[1]:=$8c00+(z SHL 12)+(OpSize[1] SHL 8)+AdrMode[1];
                Move(AdrVals[1],WAsmCode[2],AdrCnt[1]);
                WAsmCode[2+AdrCnt2[1]]:=AdrWord;
                CodeLen:=6+AdrCnt[1];
               END;
             2:IF AdrType[1]<>ModImm THEN WrError(1350)
               ELSE
                BEGIN
                 HVal:=ImmVal(1);
                 IF ChkRange(HVal,-128,127) THEN
                  BEGIN
                   WAsmCode[0]:=$8e00+(z SHL 12)+(HVal AND $ff);
                   WAsmCode[1]:=AdrWord;
                   CodeLen:=4;
                  END;
                END;
             END;
            END;
        END;
     Exit;
    END;

   IF Copy(OpPart,1,4)='SCMP' THEN
    BEGIN
     IF DOpSize=-1 THEN DOpSize:=2;
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       SplitOptions;
       IF OptionCnt>1 THEN WrError(1115)
       ELSE
        BEGIN
         OK:=True;
         IF OptionCnt=0 THEN AdrWord:=6
         ELSE IF NLS_StrCaseCmp(Options[1],'LTU')=0 THEN AdrWord:=0
         ELSE IF NLS_StrCaseCmp(Options[1],'GEU')=0 THEN AdrWord:=1
         ELSE OK:=(DecodeCondition(Options[1],AdrWord)) AND (AdrWord>1) AND (AdrWord<6);
         IF NOT OK THEN WrXError(1360,Options[1])
         ELSE
          BEGIN
           WAsmCode[0]:=$00e0+(DOpSize SHL 8)+(AdrWord SHL 10);
           CodeLen:=2;
          END;
        END;
      END;
     Exit;
    END;

   IF (Copy(OpPart,1,4)='SMOV') OR (Copy(OpPart,1,4)='SSCH') THEN
    BEGIN
     IF DOpSize=-1 THEN DOpSize:=2;
     z:=Ord(OpPart[2]='S') SHL 4;
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       SplitOptions;
       IF NLS_StrCaseCmp(Options[1],'F')=0 THEN
        BEGIN
         Mask:=0; Options[1]:=Options[2]; Dec(OptionCnt);
        END
       ELSE IF NLS_StrCaseCmp(Options[1],'B')=0 THEN
        BEGIN
         Mask:=1; Options[1]:=Options[2]; Dec(OptionCnt);
        END
       ELSE IF NLS_StrCaseCmp(Options[2],'F')=0 THEN
        BEGIN
         Mask:=0; Dec(OptionCnt);
        END
       ELSE IF NLS_StrCaseCmp(Options[2],'B')=0 THEN
        BEGIN
         Mask:=1; Dec(OptionCnt);
        END
       ELSE Mask:=0;
       IF OptionCnt>1 THEN WrError(1115)
       ELSE
        BEGIN
         OK:=True;
         IF OptionCnt=0 THEN AdrWord:=6
         ELSE IF NLS_StrCaseCmp(Options[1],'LTU')=0 THEN AdrWord:=0
         ELSE IF NLS_StrCaseCmp(Options[1],'GEU')=0 THEN AdrWord:=1
         ELSE OK:=(DecodeCondition(Options[1],AdrWord)) AND (AdrWord>1) AND (AdrWord<6);
         IF NOT OK THEN WrXError(1360,Options[1])
         ELSE
          BEGIN
           WAsmCode[0]:=$00e4+(DOpSize SHL 8)+(AdrWord SHL 10)+Mask+z;
           CodeLen:=2;
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('SSTR') THEN
    BEGIN
     IF DOpSize=-1 THEN DOpSize:=2;
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       WAsmCode[0]:=$24f7+(DOpSize SHL 8); CodeLen:=2;
      END;
     Exit;
    END;

   IF (Memo('LDM')) OR (Memo('STM')) THEN
    BEGIN
     Mask:=MModIReg+MModDisp16+MModDisp32+MModAbs16+MModAbs32+MModPCRel16+MModPCRel32;
     IF Memo('LDM') THEN
      BEGIN
       z:=$1000; Inc(Mask,MModPop);
       ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
      END
     ELSE
      BEGIN
       z:=0; Inc(Mask,MModPush);
      END;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        BEGIN
         IF OpSize[1]=-1 THEN OpSize[1]:=2;
         IF OpSize[2]=-1 THEN OpSize[2]:=2;
         IF (OpSize[1]<>2) OR (OpSize[2]<>2) THEN WrError(1130)
         ELSE IF DecodeAdr(ArgStr[2],2,Mask) THEN
          IF DecodeRegList(ArgStr[1],AdrWord,AdrType[2]<>ModPush) THEN
           BEGIN
            WAsmCode[0]:=$8a00+z+AdrMode[2];
            Move(AdrVals[2],WAsmCode[1],AdrCnt[2]);
            WAsmCode[1+AdrCnt2[2]]:=AdrWord;
            CodeLen:=4+AdrCnt[2];
           END;
        END;
     Exit;
    END;

   IF (Memo('STC')) OR (Memo('STP')) THEN
    BEGIN
     z:=Ord(Memo('STP')) SHL 10;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckFormat('G') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       IF GetOpSize(ArgStr[2],2) THEN
        BEGIN
         IF OpSize[2]=-1 THEN OpSize[2]:=2;
         IF OpSize[1]=-1 THEN OpSize[1]:=OpSize[1];
         IF OpSize[1]<>OpSize[2] THEN WrError(1132)
         ELSE IF (z=0) AND (OpSize[2]<>2) THEN WrError(1130)
         ELSE IF DecodeAdr(ArgStr[1],1,Mask_PureMem) THEN
          IF DecodeAdr(ArgStr[2],2,Mask_Dest) THEN
           BEGIN
            OpSize[1]:=0;
            Make_G($a800+z);
            Inc(WasmCode[0],$800);
           END;
        END;
     Exit;
    END;

   IF Memo('WAIT') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE IF Format<>' ' THEN WrError(1090)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1350)
     ELSE
      BEGIN
       WAsmCode[1]:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt3,OK);
       IF OK THEN
        BEGIN
         WAsmCode[0]:=$0fd6; CodeLen:=4;
        END;
      END;
     Exit;
    END;

   IF Memo('JRNG') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckFormat('GE') THEN
      IF GetOpSize(ArgStr[1],1) THEN
       BEGIN
        IF OpSize[1]=-1 THEN OpSize[1]:=1;
        IF OpSize[1]<>1 THEN WrError(1130)
        ELSE IF DecodeAdr(ArgStr[1],1,MModReg+MModImm) THEN
         BEGIN
          IF FormatCode=0 THEN
           BEGIN
            IF AdrType[1]=ModImm THEN
             BEGIN
              HVal:=ImmVal(1);
              IF (HVal>=0) AND (HVal<=255) THEN FormatCode:=2
              ELSE FormatCode:=1
             END
            ELSE FormatCode:=1;
           END;
          CASE FormatCode OF
          1:BEGIN
             WAsmCode[0]:=$ba00+AdrMode[1];
             Move(AdrVals[1],WAsmCode[1],AdrCnt[1]);
             CodeLen:=2+AdrCnt[1];
            END;
          2:IF AdrType[1]<>ModImm THEN WrError(1350)
            ELSE
             BEGIN
              HVal:=ImmVal(1);
              IF ChkRange(HVal,0,255) THEN
               BEGIN
                WAsmCode[0]:=$be00+(HVal AND $ff); CodeLen:=2;
               END;
             END;
          END;
         END;
       END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

        FUNCTION ChkPC_M16:Boolean;
	Far;
VAR
   ok:Boolean;
BEGIN
   CASE ActPC OF
   SegCode  : ok:=True;
   ELSE ok:=False;
   END;
   ChkPC_M16:=ok;
END;


        FUNCTION IsDef_M16:Boolean;
	Far;
BEGIN
   IsDef_M16:=False;
END;

        PROCEDURE SwitchFrom_M16;
        Far;
BEGIN
   DeinitFields;
END;

        PROCEDURE SwitchTo_M16;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeIntel; SetIsOccupied:=False;

   PCSymbol:='$'; HeaderID:=$13; NOPCode:=$1bd6;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.:';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_M16; ChkPC:=ChkPC_M16; IsDef:=IsDef_M16;
   SwitchFrom:=SwitchFrom_M16; InitFields;
END;

BEGIN
   CPUM16:=AddCPU('M16',SwitchTo_M16);
END.

