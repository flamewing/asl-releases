{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}
	UNIT Code7000;

{ AS - Codegenerator SH7x00 }

INTERFACE

        USES StringUt,NLS,
	     AsmDef,AsmPars,AsmSub,CodePseu;


IMPLEMENTATION

CONST
   FixedOrderCount=13;
   OneRegOrderCount=22;
   TwoRegOrderCount=20;
   MulRegOrderCount=3;
   BWOrderCount=3;
   LogOrderCount=4;

   ModNone=-1;
   ModReg=0;           MModReg=1 SHL ModReg;
   ModIReg=1;          MModIReg=1 SHL ModIReg;
   ModPreDec=2;        MModPreDec=1 SHL ModPreDec;
   ModPostInc=3;       MModPostInc=1 SHL ModPostInc;
   ModIndReg=4;        MModIndReg=1 SHL ModIndReg;
   ModR0Base=5;        MModR0Base=1 SHL ModR0Base;
   ModGBRBase=6;       MModGBRBase=1 SHL ModGBRBase;
   ModGBRR0=7;         MModGBRR0=1 SHL ModGBRR0;
   ModPCRel=8;         MModPCRel=1 SHL ModPCRel;
   ModImm=9;           MModImm=1 SHL ModImm;

   CompLiteralsName='COMPRESSEDLITERALS';

TYPE
   FixedOrder=RECORD
	       Name:String[7];
               MinCPU:CPUVar;
               Priv:Boolean;
	       Code:Word;
	      END;

   TwoRegOrder=RECORD
	        Name:String[7];
                MinCPU:CPUVar;
	        Code:Word;
                Priv:Boolean;
                DefSize:ShortInt;
	       END;

   FixedMinOrder=RECORD
                  Name:String[7];
                  MinCPU:CPUVar;
                  Code:Word;
                 END;

   FixedOrderArray=ARRAY[1..FixedOrderCount] OF FixedOrder;
   OneRegOrderArray=ARRAY[1..OneRegOrderCount] OF FixedMinOrder;
   TwoRegOrderArray=ARRAY[1..TwoRegOrderCount] OF TwoRegOrder;
   MulRegOrderArray=ARRAY[1..MulRegOrderCount] OF FixedMinOrder;
   BWOrderArray=ARRAY[1..BWOrderCount] OF FixedOrder;
   LogOrderArray=ARRAY[0..LogOrderCount-1] OF String[3];

   PLiteral=^TLiteral;
   TLiteral=RECORD
	     Next:PLiteral;
	     Value,FCount:LongInt;
	     Is32,IsForward:Boolean;
	     PassNo:Integer;
	     DefSection:LongInt;
	    END;

VAR
   OpSize:ShortInt;         { Grî·e=8*(2^OpSize) }
   AdrMode:ShortInt; 	    { Ergebnisadre·modus }
   AdrPart:Word;            { Adressierungsmodusbits im Opcode }

   FirstLiteral:PLiteral;
   ForwardCount:LongInt;
   SaveInitProc:PROCEDURE;

   CPU7000,CPU7600,CPU7700:CPUVar;

   FixedOrders:^FixedOrderArray;
   OneRegOrders:^OneRegOrderArray;
   TwoRegOrders:^TwoRegOrderArray;
   MulRegOrders:^MulRegOrderArray;
   BWOrders:^BWOrderArray;
   LogOrders:^LogOrderArray;

   CurrDelayed,PrevDelayed,CompLiterals:Boolean;
   DelayedAdr:LongInt;

{---------------------------------------------------------------------------}
{ dynamische Belegung/Freigabe Codetabellen }

	PROCEDURE InitFields;
VAR
   z:Word;

	PROCEDURE AddFixed(NName:String; NCode:Word; NPriv:Boolean; NMin:CPUVar);
BEGIN
   IF z=FixedOrderCount THEN Halt;
   Inc(z);
   WITH FixedOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
     Priv:=NPriv; MinCPU:=NMin;
    END;
END;

	PROCEDURE AddOneReg(NName:String; NCode:Word; NMin:CPUVar);
BEGIN
   IF z=OneRegOrderCount THEN Halt;
   Inc(z);
   WITH OneRegOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NMin;
    END;
END;

	PROCEDURE AddTwoReg(NName:String; NCode:Word; NPriv:Boolean; NMin:CPUVar; NDef:ShortInt);
BEGIN
   IF z=TwoRegOrderCount THEN Halt;
   Inc(z);
   WITH TwoRegOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
     Priv:=NPriv; MinCPU:=NMin;
     DefSize:=NDef;
    END;
END;

	PROCEDURE AddMulReg(NName:String; NCode:Word; NMin:CPUVar);
BEGIN
   IF z=MulRegOrderCount THEN Halt;
   Inc(z);
   WITH MulRegOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode; MinCPU:=NMin;
    END;
END;

	PROCEDURE AddBW(NName:String; NCode:Word);
BEGIN
   IF z=BWOrderCount THEN Halt;
   Inc(z);
   WITH BWOrders^[z] DO
    BEGIN
     Name:=NName; Code:=NCode;
    END;
END;

BEGIN
   New(FixedOrders); z:=0;
   AddFixed('CLRT'  ,$0008, False, CPU7000);
   AddFixed('CLRMAC',$0028, False, CPU7000);
   AddFixed('NOP'   ,$0009, False, CPU7000);
   AddFixed('RTE'   ,$002b, False, CPU7000);
   AddFixed('SETT'  ,$0018, False, CPU7000);
   AddFixed('SLEEP' ,$001b, False, CPU7000);
   AddFixed('RTS'   ,$000b, False, CPU7000);
   AddFixed('DIV0U' ,$0019, False, CPU7000);
   AddFixed('BRK'   ,$0000, True , CPU7000);
   AddFixed('RTB'   ,$0001, True , CPU7000);
   AddFixed('CLRS'  ,$0048, False, CPU7700);
   AddFixed('SETS'  ,$0058, False, CPU7700);
   AddFixed('LDTLB' ,$0038, True , CPU7700);

   New(OneRegOrders); z:=0;
   AddOneReg('MOVT'  ,$0029,CPU7000); AddOneReg('CMP/PZ',$4011,CPU7000);
   AddOneReg('CMP/PL',$4015,CPU7000); AddOneReg('ROTL'  ,$4004,CPU7000);
   AddOneReg('ROTR'  ,$4005,CPU7000); AddOneReg('ROTCL' ,$4024,CPU7000);
   AddOneReg('ROTCR' ,$4025,CPU7000); AddOneReg('SHAL'  ,$4020,CPU7000);
   AddOneReg('SHAR'  ,$4021,CPU7000); AddOneReg('SHLL'  ,$4000,CPU7000);
   AddOneReg('SHLR'  ,$4001,CPU7000); AddOneReg('SHLL2' ,$4008,CPU7000);
   AddOneReg('SHLR2' ,$4009,CPU7000); AddOneReg('SHLL8' ,$4018,CPU7000);
   AddOneReg('SHLR8' ,$4019,CPU7000); AddOneReg('SHLL16',$4028,CPU7000);
   AddOneReg('SHLR16',$4029,CPU7000); AddOneReg('LDBR'  ,$0021,CPU7000);
   AddOneReg('STBR'  ,$0020,CPU7000); AddOneReg('DT'    ,$4010,CPU7600);
   AddOneReg('BRAF'  ,$0032,CPU7600); AddOneReg('BSRF'  ,$0003,CPU7600);

   New(TwoRegOrders); z:=0;
   AddTwoReg('XTRCT' ,$200d, False, CPU7000,2);
   AddTwoReg('ADDC'  ,$300e, False, CPU7000,2);
   AddTwoReg('ADDV'  ,$300f, False, CPU7000,2);
   AddTwoReg('CMP/HS',$3002, False, CPU7000,2);
   AddTwoReg('CMP/GE',$3003, False, CPU7000,2);
   AddTwoReg('CMP/HI',$3006, False, CPU7000,2);
   AddTwoReg('CMP/GT',$3007, False, CPU7000,2);
   AddTwoReg('CMP/STR',$200c,False, CPU7000,2);
   AddTwoReg('DIV1'  ,$3004, False, CPU7000,2);
   AddTwoReg('DIV0S' ,$2007, False, CPU7000,-1);
   AddTwoReg('MULS'  ,$200f, False, CPU7000,1);
   AddTwoReg('MULU'  ,$200e, False, CPU7000,1);
   AddTwoReg('NEG'   ,$600b, False, CPU7000,2);
   AddTwoReg('NEGC'  ,$600a, False, CPU7000,2);
   AddTwoReg('SUB'   ,$3008, False, CPU7000,2);
   AddTwoReg('SUBC'  ,$300a, False, CPU7000,2);
   AddTwoReg('SUBV'  ,$300b, False, CPU7000,2);
   AddTwoReg('NOT'   ,$6007, False, CPU7000,2);
   AddTwoReg('SHAD'  ,$400c, False, CPU7700,2);
   AddTwoReg('SHLD'  ,$400d, False, CPU7700,2);

   New(MulRegOrders); z:=0;
   AddMulReg('MUL'   ,$0007,CPU7600);
   AddMulReg('DMULU' ,$3005,CPU7600);
   AddMulReg('DMULS' ,$300d,CPU7600);

   New(BWOrders); z:=0;
   AddBW('SWAP',$6008); AddBW('EXTS',$600e); AddBW('EXTU',$600c);

   New(LogOrders);
   LogOrders^[0]:='TST'; LogOrders^[1]:='AND';
   LogOrders^[2]:='XOR'; LogOrders^[3]:='OR' ;
END;

	PROCEDURE DeinitFields;
BEGIN
   Dispose(FixedOrders);
   Dispose(OneRegORders);
   Dispose(TwoRegOrders);
   Dispose(MulRegOrders);
   Dispose(BWOrders);
   Dispose(LogOrders);
END;

{---------------------------------------------------------------------------}
{ die PC-relative Adresse: direkt nach verzîgerten SprÅngen = Sprungziel+2 }

	FUNCTION PCRelAdr:LongInt;
BEGIN
   IF PrevDelayed THEN PCRelAdr:=DelayedAdr+2
   ELSE PCRelAdr:=EProgCounter+4;
END;

	PROCEDURE ChkDelayed;
BEGIN
   IF PrevDelayed THEN WrError(200);
END;

{---------------------------------------------------------------------------}
{ Adre·parsing }

	FUNCTION LiteralName(Lit:PLiteral):String;
VAR
   Tmp:String;
BEGIN
   WITH Lit^ DO
    BEGIN
     IF IsForward THEN Tmp:='F_'+HexString(FCount,8)
     ELSE IF Is32 THEN Tmp:='L_'+HexString(Value,8)
     ELSE Tmp:='W_'+HexString(Value,4);
     LiteralName:='LITERAL_'+Tmp+'_'+HexString(PassNo,0);
    END;
END;

	PROCEDURE PrintLiterals;
VAR
   Lauf:PLiteral;
BEGIN
   WrLstLine('LiteralList');
   Lauf:=FirstLiteral;
   WHILE Lauf<>Nil DO
    BEGIN
     WrLstLine(LiteralName(Lauf)); Lauf:=Lauf^.Next;
    END;
END;

	PROCEDURE SetOpSize(Size:ShortInt);
BEGIN
   IF OpSize=-1 THEN OpSize:=Size
   ELSE IF Size<>OpSize THEN
    BEGIN
     WrError(1131); AdrMode:=ModNone;
    END;
END;

	FUNCTION DecodeReg(Asc:String; VAR Erg:Byte):Boolean;
VAR
   Err:ValErgType;
BEGIN
   IF NLS_StrCaseCmp(Asc,'SP')=0 THEN
    BEGIN
     Erg:=15; DecodeReg:=True;
    END
   ELSE IF (Length(Asc)<2) OR (Length(Asc)>3) OR (UpCase(Asc[1])<>'R') THEN DecodeReg:=False
   ELSE
    BEGIN
     Val(Copy(Asc,2,Length(Asc)-1),Erg,Err);
     DecodeReg:=(Err=0) AND (Erg<=15);
    END;
END;

	FUNCTION DecodeCtrlReg(Asc:String; VAR Erg:Byte):Boolean;
VAR
   MinCPU:CPUVar;
BEGIN
   MinCPU:=CPU7000; Erg:=$ff;
   IF NLS_StrCaseCmp(Asc,'SR')=0 THEN Erg:=0
   ELSE IF NLS_StrCaseCmp(Asc,'GBR')=0 THEN Erg:=1
   ELSE IF NLS_StrCaseCmp(Asc,'VBR')=0 THEN Erg:=2
   ELSE IF NLS_StrCaseCmp(Asc,'SSR')=0 THEN
    BEGIN
     Erg:=3; MinCPU:=CPU7700;
    END
   ELSE IF NLS_StrCaseCmp(Asc,'SPC')=0 THEN
    BEGIN
     Erg:=4; MinCPU:=CPU7700;
    END
   ELSE IF (Length(Asc)=7) AND (UpCase(Asc[1])='R')
       AND (NLS_StrCaseCmp(Copy(Asc,3,5),'_BANK')=0)
       AND (Asc[2]>='0') AND (Asc[2]<='7') THEN
    BEGIN
     Erg:=Ord(Asc[2])-AscOfs+8; MinCPU:=CPU7700;
    END;
   IF (Erg=$ff) OR (MomCPU<MinCPU) THEN
    BEGIN
     DecodeCtrlReg:=False; WrXError(1440,Asc);
    END
   ELSE DecodeCtrlReg:=True;
END;

	PROCEDURE DecodeAdr(Asc:String; Mask:Word; Signed:Boolean);
LABEL
   AdrFound;
CONST
   RegNone=-1;
   RegPC=-2;
   RegGBR=-3;
VAR
   HReg,p:Byte;
   BaseReg,IndReg,DOpSize:ShortInt;
   DispAcc:LongInt;
   AdrStr:String;
   OK,FirstFlag,NIs32,Critical,Found:Boolean;
   Lauf,Last:PLiteral;

   	FUNCTION ExtOp(Inp:LongInt; Src:Byte):LongInt;
BEGIN
   CASE Src OF
   0:Inp:=Inp AND $ff;
   1:Inp:=Inp AND $ffff;
   END;
   IF Signed THEN
    BEGIN
     IF Src<1 THEN
      IF Inp AND $80=$80 THEN Inc(Inp,$ff00);
     IF Src<2 THEN
      IF Inp AND $8000=$8000 THEN Inc(Inp,$ffff0000);
    END;
   ExtOp:=Inp;
END;

	FUNCTION OpMask(OpSize:ShortInt):LongInt;
BEGIN
   CASE OpSize OF
   0:OpMask:=$ff;
   1:OpMask:=$ffff;
   2:OpMask:=$ffffffff;
   END;
END;

BEGIN
   AdrMode:=ModNone;

   IF DecodeReg(Asc,HReg) THEN
    BEGIN
     AdrPart:=HReg; AdrMode:=ModReg; Goto AdrFound;
    END;

   IF Asc[1]='@' THEN
    BEGIN
     Delete(Asc,1,1);
     IF IsIndirect(Asc) THEN
      BEGIN
       Asc:=Copy(Asc,2,Length(Asc)-2);
       BaseReg:=RegNone; IndReg:=RegNone;
       DispAcc:=0; FirstFlag:=False; OK:=True;
       WHILE (Asc<>'') AND (OK) DO
	BEGIN
	 p:=QuotPos(Asc,','); SplitString(Asc,AdrStr,Asc,p);
         IF NLS_StrCaseCmp(AdrStr,'PC')=0 THEN
	  IF BaseReg=RegNone THEN BaseReg:=RegPC
	  ELSE
	   BEGIN
	    WrError(1350); OK:=False;
	   END
         ELSE IF NLS_StrCaseCmp(AdrStr,'GBR')=0 THEN
	  IF BaseReg=RegNone THEN  BaseReg:=RegGBR
	  ELSE
	   BEGIN
	    WrError(1350); OK:=False;
	   END
	 ELSE IF DecodeReg(AdrStr,HReg) THEN
	  IF IndReg=RegNone THEN IndReg:=HReg
	  ELSE IF (BaseReg=RegNone) AND (HReg=0) THEN BaseReg:=0
	  ELSE IF (IndReg=0) AND (BaseReg=RegNone) THEN
	   BEGIN
	    BaseReg:=0; IndReg:=HReg;
	   END
	  ELSE
	   BEGIN
	    WrError(1350); OK:=False;
	   END
	 ELSE
	  BEGIN
	   FirstPassUnknown:=False;
	   Inc(DispAcc,EvalIntExpression(AdrStr,Int32,OK));
	   IF FirstPassUnknown THEN FirstFlag:=True;
	  END;
	END;
       IF FirstFlag THEN DispAcc:=0;
       IF (OK) AND (DispAcc AND ((1 SHL OpSize)-1)<>0) THEN
	BEGIN
	 WrError(1325); OK:=False;
	END
       ELSE IF (OK) AND (DispAcc<0) THEN
	BEGIN
	 WrXError(1315,'Disp<0'); OK:=False;
	END
       ELSE DispAcc:=DispAcc SHR OpSize;
       IF OK THEN
	BEGIN
	 CASE BaseReg OF
	 0:
	  IF (IndReg<0) OR (DispAcc<>0) THEN WrError(1350)
	  ELSE
	   BEGIN
	    AdrMode:=ModR0Base; AdrPart:=IndReg;
	   END;
	 RegGBR:
	  IF (IndReg=0) AND (DispAcc=0) THEN AdrMode:=ModGBRR0
	  ELSE IF IndReg<>RegNone THEN WrError(1350)
	  ELSE IF DispAcc>255 THEN WrError(1320)
	  ELSE
	   BEGIN
	    AdrMode:=ModGBRBase; AdrPart:=DispAcc;
	   END;
	 RegNone:
	  IF IndReg=RegNone THEN WrError(1350)
	  ELSE IF DispAcc>15 THEN WrError(1320)
	  ELSE
	   BEGIN
	    AdrMode:=ModIndReg; AdrPart:=(IndReg SHL 4)+DispAcc;
	   END;
	 RegPC:
	  IF IndReg<>RegNone THEN WrError(1350)
	  ELSE IF DispAcc>255 THEN WrError(1320)
	  ELSE
	   BEGIN
	    AdrMode:=ModPCRel; AdrPart:=DispAcc;
	   END;
	 END;
	END;
       Goto AdrFound;
      END
     ELSE
      BEGIN
       IF DecodeReg(Asc,HReg) THEN
	BEGIN
	 AdrPart:=HReg; AdrMode:=ModIReg;
	END
       ELSE IF (Length(Asc)>1) AND (Asc[1]='-') AND (DecodeReg(Copy(Asc,2,Length(Asc)-1),HReg)) THEN
	BEGIN
	 AdrPart:=HReg; AdrMode:=ModPreDec;
	END
       ELSE IF (Length(Asc)>1) AND (Asc[Length(Asc)]='+') AND (DecodeReg(Copy(Asc,1,Length(Asc)-1),HReg)) THEN
	BEGIN
	 AdrPart:=HReg; AdrMode:=ModPostInc;
	END
       ELSE WrError(1350);
       Goto AdrFound;
      END;
    END;

   IF Asc[1]='#' THEN
    BEGIN
     FirstPassUnknown:=False;
     Delete(Asc,1,1);
     CASE OpSize OF
     0:DispAcc:=EvalIntExpression(Asc,Int8,OK);
     1:DispAcc:=EvalIntExpression(Asc,Int16,OK);
     2:DispAcc:=EvalIntExpression(Asc,Int32,OK);
     ELSE
      BEGIN
       DispAcc:=0; OK:=True;
      END;
     END;
     Critical:=FirstPassUnknown OR UsesForwards;
     IF OK THEN
      BEGIN
       { minimale Groesse optimieren }
       IF OpSize=0 THEN DOpsize:=0 ELSE DOpSize:=Ord(Critical);
       WHILE (ExtOp(DispAcc,DOpSize) XOR DispAcc) AND OpMask(OpSize)<>0 DO Inc(DOpSize);
       IF DOpSize=0 THEN
        BEGIN
	 AdrPart:=DispAcc AND $ff;
	 AdrMode:=ModImm;
        END
       ELSE IF Mask AND MModPCRel<>0 THEN
	BEGIN
	 { Literalgrî·e ermitteln }
         NIs32:=DOpSize=2;
	 IF NOT Nis32 THEN DispAcc:=DispAcc AND $ffff;
	 { Literale sektionsspezifisch }
         AdrStr:='[PARENT0]';
	 { schon vorhanden ? }
	 Lauf:=FirstLiteral; p:=0; OK:=False; Last:=Nil; Found:=False;
	 WHILE (Lauf<>Nil) AND (NOT Found) DO
	  BEGIN
	   Last:=Lauf;
           IF (NOT Critical) AND (NOT Lauf^.IsForward)
	   AND (Lauf^.DefSection=MomSectionHandle) THEN
            IF ((Lauf^.Is32=NIs32) AND (DispAcc=Lauf^.Value))
            OR ((Lauf^.Is32) AND (NOT NIs32) AND (DispAcc=Lauf^.Value SHR 16)) THEN Found:=True
            ELSE IF (Lauf^.Is32) AND (NOT NIs32) AND (DispAcc=Lauf^.Value AND $ffff) THEN
             BEGIN
              Found:=True; p:=2;
             END;
	   IF NOT Found THEN Lauf:=Lauf^.Next;
	  END;
	 { nein - erzeugen }
	 IF NOT Found THEN
	  BEGIN
	   New(Lauf);
	   Lauf^.Is32:=NIs32; Lauf^.Value:=DispAcc;
           Lauf^.IsForward:=Critical;
           IF Critical THEN
            BEGIN
             Lauf^.FCount:=ForwardCount; Inc(ForwardCount);
            END;
	   Lauf^.Next:=Nil; Lauf^.PassNo:=1; Lauf^.DefSection:=MomSectionHandle;
	   WHILE IsSymbolDefined(LiteralName(Lauf)+AdrStr) DO
	    Inc(Lauf^.PassNo);
           IF Last=Nil THEN FirstLiteral:=Lauf ELSE Last^.Next:=Lauf;
	  END;
	 { Distanz abfragen - im nÑchsten Pass... }
	 FirstPassUnknown:=False;
         DispAcc:=EvalIntExpression(LiteralName(Lauf)+AdrStr,Int32,OK)+p;
	 IF OK THEN
	  BEGIN
	   IF FirstPassUnknown
	    THEN DispAcc:=0
           ELSE IF NIs32 THEN
	    DispAcc:=(DispAcc-(PCRelAdr AND $fffffffc)) SHR 2
           ELSE
	    DispAcc:=(DispAcc-PCRelAdr) SHR 1;
	   IF DispAcc<0 THEN
	    BEGIN
	     WrXError(1315,'Disp<0'); OK:=False;
	    END
           ELSE IF (DispAcc>255) AND (NOT SymbolQuestionable) THEN WrError(1330)
	   ELSE
	    BEGIN
	     AdrMode:=ModPCRel; AdrPart:=DispAcc; OpSize:=Ord(NIs32)+1;
	    END;
	  END;
	END
       ELSE WrError(1350);
      END;
     Goto AdrFound;
    END;

   { absolut Åber PC-relativ abwickeln }

   IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
   ELSE
    BEGIN
     FirstPassUnknown:=False;
     DispAcc:=EvalIntExpression(Asc,Int32,OK);
     IF FirstPassUnknown THEN DispAcc:=0
     ELSE IF OpSize=2 THEN Dec(DispAcc,PCRelAdr AND $fffffffc)
     ELSE Dec(DispAcc,PCRelAdr);
     IF DispAcc<0 THEN WrXError(1315,'Disp<0')
     ELSE IF DispAcc AND ((1 SHL OpSize)-1)<>0 THEN WrError(1325)
     ELSE
      BEGIN
       DispAcc:=DispAcc SHR OpSize;
       IF DispAcc>255 THEN WrError(1320)
       ELSE
        BEGIN
	 AdrMode:=ModPCRel; AdrPart:=DispAcc;
        END;
      END;
    END;

AdrFound:
   IF (AdrMode<>ModNone) AND (Mask AND (1 SHL AdrMode)=0) THEN
    BEGIN
     WrError(1350); AdrMode:=ModNone;
    END;
END;

{---------------------------------------------------------------------------}

	FUNCTION DecodePseudo:Boolean;
CONST
   ONOFF7000Count=2;
   ONOFF7000s:ARRAY[1..ONOFF7000Count] OF ONOFFRec=
              ((Name:'SUPMODE'; Dest:@SupAllowed; FlagName:SupAllowedName),
	       (Name:'COMPLITERALS'; Dest:@CompLiterals; FlagName:CompLiteralsName));
 VAR
   Lauf,Tmp,Last:PLiteral;

	PROCEDURE LTORG_16;
VAR
   Lauf:PLiteral;
BEGIN
   Lauf:=FirstLiteral;
   WHILE Lauf<>Nil DO
    WITH Lauf^ DO
     BEGIN
      IF (NOT Is32) AND (DefSection=MomSectionHandle) THEN
       BEGIN
	WAsmCode[CodeLen SHR 1]:=Value;
	EnterIntSymbol(LiteralName(Lauf),EProgCounter+CodeLen,SegCode,False);
        PassNo:=-1;
	Inc(CodeLen,2);
       END;
      Lauf:=Next;
     END;
END;

	PROCEDURE LTORG_32;
VAR
   Lauf,EqLauf:PLiteral;
BEGIN
   Lauf:=FirstLiteral;
   WHILE Lauf<>Nil DO
    WITH Lauf^ DO
     BEGIN
      IF (Is32) AND (DefSection=MomSectionHandle) AND (PassNo>=0) THEN
       BEGIN
	IF (EProgCounter+CodeLen) AND 2<>0 THEN
	 BEGIN
	  WAsmCode[CodeLen SHR 1]:=0; Inc(CodeLen,2);
	 END;
	WAsmCode[CodeLen SHR 1]:=(Value SHR 16);
	WAsmCode[(CodeLen SHR 1)+1]:=(Value AND $ffff);
        EnterIntSymbol(LiteralName(Lauf),EProgCounter+CodeLen,SegCode,False);
        PassNo:=-1;
        IF CompLiterals THEN
         BEGIN
          EqLauf:=Lauf^.Next;
          WHILE EqLauf<>Nil DO
           BEGIN
            IF (EqLauf^.Is32) AND (EqLauf^.PassNo>=0) AND
	       (EqLauf^.DefSection=MomSectionHandle) AND
               (EqLauf^.Value=Value) THEN
             BEGIN
              EnterIntSymbol(LiteralName(EqLauf),EProgCounter+CodeLen,SegCode,False);
              EqLauf^.PassNo:=-1;
	     END;
            EqLauf:=EqLauf^.Next;
           END;
         END;
	Inc(CodeLen,4);
       END;
      Lauf:=Next;
     END;
END;

BEGIN
   DecodePseudo:=True;

   IF CodeONOFF(@ONOFF7000s,ONOFF7000Count) THEN Exit;

   { ab hier (und weiter in der Hauptroutine) stehen die Befehle,
     die Code erzeugen, deshalb wird der Merker fÅr verzîgerte
     SprÅnge hier weiter geschaltet. }

   PrevDelayed:=CurrDelayed; CurrDelayed:=False;

   IF Memo('LTORG') THEN
    BEGIN
     IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       IF EProgCounter AND 3=0 THEN
	BEGIN
	 LTORG_32; LTORG_16;
	END
       ELSE
	BEGIN
	 LTORG_16; LTORG_32;
	END;
       Lauf:=FirstLiteral; Last:=Nil;
       WHILE Lauf<>Nil DO
	BEGIN
	 IF (Lauf^.DefSection=MomSectionHandle) AND (Lauf^.PassNo<0) THEN
	  BEGIN
	   Tmp:=Lauf^.Next;
	   IF Last=Nil THEN FirstLiteral:=Tmp ELSE Last^.Next:=Tmp;
	   Dispose(Lauf); Lauf:=Tmp;
	  END
	 ELSE
	  BEGIN
	   Last:=Lauf; Lauf:=Lauf^.Next;
	  END;
	END;
      END;
     Exit;
    END;

   DecodePseudo:=False;
END;

	PROCEDURE SetCode(Code:Word);
BEGIN
   CodeLen:=2; WAsmCode[0]:=Code;
END;

	PROCEDURE MakeCode_7000;
	Far;
VAR
   z:Integer;
   AdrLong:LongInt;
   OK:Boolean;
   HReg:Byte;
BEGIN
   CodeLen:=0; DontPrint:=False; OpSize:=-1;

   { zu ignorierendes }

   if Memo('') THEN Exit;

   { Pseudoanweisungen }

   IF DecodePseudo THEN Exit;

   { Attribut verwursten }

   IF AttrPart<>'' THEN
    BEGIN
     IF Length(AttrPart)<>1 THEN
      BEGIN
       WrError(1105); Exit;
      END;
     CASE UpCase(AttrPart[1]) OF
     'B':SetOpSize(0);
     'W':SetOpSize(1);
     'L':SetOpSize(2);
     'Q':SetOpSize(3);
     'S':SetOpSize(4);
     'D':SetOpSize(5);
     'X':SetOpSize(6);
     'P':SetOpSize(7);
     ELSE
      BEGIN
       WrError(1107); Exit;
      END;
     END;
    END;

   IF DecodeMoto16Pseudo(OpSize,True) THEN Exit;

   { Anweisungen ohne Argument }

   FOR z:=1 TO FixedOrderCount DO
    WITH FixedOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>0 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE
        BEGIN
	 SetCode(Code);
         IF (NOT SupAllowed) AND (Priv) THEN WrError(50);
        END;
       Exit;
      END;

   { Datentransfer }

   IF Memo('MOV') THEN
    BEGIN
     IF OpSize=-1 THEN SetOpSize(2);
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF OpSize>2 THEN WrError(1130)
     ELSE IF DecodeReg(ArgStr[1],HReg) THEN
      BEGIN
       DecodeAdr(ArgStr[2],MModReg+MModIReg+MModPreDec+MModIndReg+MModR0Base+MModGBRBase,True);
       CASE AdrMode OF
       ModReg:
	IF OpSize<>2 THEN WrError(1130)
	ELSE SetCode($6003+(HReg SHL 4)+(AdrPart SHL 8));
       ModIReg:
	SetCode($2000+(HReg SHL 4)+(AdrPart SHL 8)+OpSize);
       ModPreDec:
	SetCode($2004+(HReg SHL 4)+(AdrPart SHL 8)+OpSize);
       ModIndReg:
	IF OpSize=2 THEN
	 SetCode($1000+(HReg SHL 4)+(AdrPart AND 15)+((AdrPart AND $f0) SHL 4))
	ELSE IF HReg<>0 THEN WrError(1350)
	ELSE SetCode($8000+AdrPart+(Word(OpSize) SHL 8));
       ModR0Base:
	SetCode($0004+(AdrPart SHL 8)+(HReg SHL 4)+OpSize);
       ModGBRBase:
	IF HReg<>0 THEN WrError(1350)
	ELSE SetCode($c000+AdrPart+(Word(OpSize) SHL 8));
       END;
      END
     ELSE IF DecodeReg(ArgStr[2],HReg) THEN
      BEGIN
       DecodeAdr(ArgStr[1],MModImm+MModPCRel+MModIReg+MModPostInc+MModIndReg+MModR0Base+MModGBRBase,True);
       CASE AdrMode OF
       ModIReg:
	SetCode($6000+(AdrPart SHL 4)+(Word(HReg) SHL 8)+OpSize);
       ModPostInc:
	SetCode($6004+(AdrPart SHL 4)+(Word(HReg) SHL 8)+OpSize);
       ModIndReg:
	IF OpSize=2 THEN
	 SetCode($5000+(Word(HReg) SHL 8)+AdrPart)
	ELSE IF HReg<>0 THEN WrError(1350)
	ELSE SetCode($8400+AdrPArt+(Word(OpSize) SHL 8));
       ModR0Base:
	SetCode($000c+(AdrPart SHL 4)+(Word(HReg) SHL 8)+OpSize);
       ModGBRBase:
	IF HReg<>0 THEN WrError(1350)
	ELSE SetCode($c400+AdrPart+(Word(OpSize) SHL 8));
       ModPCRel:
	IF OpSize=0 THEN WrError(1350)
	ELSE SetCode($9000+(Word(OpSize-1) SHL 14)+(Word(HReg) SHL 8)+AdrPart);
       ModImm:
	SetCode($e000+(Word(HReg) SHL 8)+AdrPart);
       END;
      END
     ELSE WrError(1350);
     Exit;
    END;

   IF Memo('MOVA') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT DecodeReg(ArgStr[2],HReg) THEN WrError(1350)
     ELSE IF HReg<>0 THEN WrError(1350)
     ELSE
      BEGIN
       SetOpSize(2);
       DecodeAdr(ArgStr[1],MModPCRel,False);
       IF AdrMode<>ModNone THEN
        BEGIN
         CodeLen:=2; WAsmCode[0]:=$c700+AdrPart;
        END;
      END;
     Exit;
    END;

   IF Memo('PREF') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg,False);
       IF AdrMode<>ModNone THEN
        BEGIN
         CodeLen:=2; WAsmCode[0]:=$0083+(AdrPart SHL 8);
        END;
      END;
     Exit;
    END;

   IF (Memo('LDC')) OR (Memo('STC')) THEN
    BEGIN
     IF OpSize=-1 THEN SetOpSize(2);
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF Memo('LDC') THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       IF DecodeCtrlReg(ArgStr[1],HReg) THEN
	BEGIN
	 IF Memo('LDC') THEN DecodeAdr(ArgStr[2],MModReg+MModPostInc,False)
	 ELSE DecodeAdr(ArgStr[2],MModReg+MModPreDec,False);
	 CASE AdrMode OF
	 ModReg:
	  IF Memo('LDC') THEN SetCode($400e+(AdrPart SHL 8)+(HReg SHL 4))
	  ELSE SetCode($0002+(AdrPart SHL 8)+(HReg SHL 4));
	 ModPostInc:
	  SetCode($4007+(AdrPart SHL 8)+(HReg SHL 4));
	 ModPreDec:
	  SetCode($4003+(AdrPart SHL 8)+(HReg SHL 4));
	 END;
         IF (AdrMode<>ModNone) AND (NOT SupAllowed) THEN WrError(50);
	END;
      END;
     Exit;
    END;

   IF (Memo('LDS')) OR (Memo('STS')) THEN
    BEGIN
     IF OpSize=-1 THEN SetOpSize(2);
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF Memo('LDS') THEN
	BEGIN
	 ArgStr[3]:=ArgStr[1]; ArgStr[1]:=ArgStr[2]; ArgStr[2]:=ArgStr[3];
	END;
       IF NLS_StrCaseCmp(ArgStr[1],'MACH')=0 THEN HReg:=0
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'MACL')=0 THEN HReg:=1
       ELSE IF NLS_StrCaseCmp(ArgStr[1],'PR')=0 THEN HReg:=2
       ELSE
	BEGIN
	 WrError(1440); HReg:=$ff;
	END;
       IF HReg<$ff THEN
	BEGIN
	 IF Memo('LDS') THEN DecodeAdr(ArgStr[2],MModReg+MModPostInc,False)
	 ELSE DecodeAdr(ArgStr[2],MModReg+MModPreDec,False);
	 CASE AdrMode OF
	 ModReg:
	  IF Memo('LDS') THEN SetCode($400a+(AdrPart SHL 8)+(HReg SHL 4))
	  ELSE SetCode($000a+(AdrPart SHL 8)+(HReg SHL 4));
	 ModPostInc:
	  SetCode($4006+(AdrPart SHL 8)+(HReg SHL 4));
	 ModPreDec:
	  SetCode($4002+(AdrPart SHL 8)+(HReg SHL 4));
	 END;
	END;
      END;
     Exit;
    END;

   { nur ein Register als Argument }

   FOR z:=1 TO OneRegOrderCount DO
    WITH OneRegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>1 THEN WrError(1110)
       ELSE IF AttrPart<>'' THEN WrError(1100)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg,False);
	 IF AdrMode<>ModNone THEN
	  SetCode(Code+(AdrPart SHL 8));
	 IF (NOT SupAllowed) AND ((Memo('STBR')) OR (Memo('LDBR'))) THEN WrError(50);
         IF OpPart[1]='B' THEN
          BEGIN
           CurrDelayed:=True; DelayedAdr:=$7fffffff;
           ChkDelayed;
          END;
	END;
       Exit;
      END;

   IF Memo('TAS') THEN
    BEGIN
     IF OpSize=-1 THEN SetOpSize(0);
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF OpSize<>0 THEN WrError(1130)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg,False);
       IF AdrMode<>ModNone THEN SetCode($401b+(AdrPart SHL 8));
      END;
     Exit;
    END;

   { zwei Register }

   FOR z:=1 TO TwoRegOrderCount DO
    WITH TwoRegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF (AttrPart<>'') AND (OpSize<>DefSize) THEN WrError(1100)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg,False);
	 IF AdrMode<>ModNone THEN
	  BEGIN
	   WAsmCode[0]:=Code+(AdrPart SHL 4);
	   DecodeAdr(ArgStr[2],MModReg,False);
	   IF AdrMode<>ModNone THEN SetCode(WAsmCode[0]+(Word(AdrPart) SHL 8));
           IF (NOT SupAllowed) AND (Priv) THEN WrError(1500);
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO MulRegOrderCount DO
    WITH MulRegOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF MomCPU<MinCPU THEN WrError(1500)
       ELSE
        BEGIN
	 IF AttrPart='' THEN OpSize:=2;
         IF OpSize<>2 THEN WrError(1130)
         ELSE
	  BEGIN
	   DecodeAdr(ArgStr[1],MModReg,False);
	   IF AdrMode<>ModNone THEN
	    BEGIN
	     WAsmCode[0]:=Code+(AdrPart SHL 4);
	     DecodeAdr(ArgStr[2],MModReg,False);
	     IF AdrMode<>ModNone THEN SetCode(WAsmCode[0]+(Word(AdrPart) SHL 8));
            END;
	  END;
	END;
       Exit;
      END;

   FOR z:=1 TO BWOrderCount DO
    WITH BWOrders^[z] DO
     IF Memo(Name) THEN
      BEGIN
       IF OpSize=-1 THEN SetOpSize(1);
       IF ArgCnt<>2 THEN WrError(1110)
       ELSE IF (OpSize<>0) AND (OpSize<>1) THEN WrError(1130)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],MModReg,False);
	 IF AdrMode<>ModNone THEN
	  BEGIN
	   WAsmCode[0]:=Code+OpSize+(AdrPart SHL 4);
	   DecodeAdr(ArgStr[2],MModReg,False);
	   IF AdrMode<>ModNone THEN SetCode(WAsmCode[0]+(Word(AdrPart) SHL 8));
	  END;
	END;
       Exit;
      END;

   IF Memo('MAC') THEN
    BEGIN
     IF OpSize=-1 THEN SetOpSize(1);
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
     ELSE IF (OpSize=2) AND (MomCPU<CPU7600) THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModPostInc,False);
       IF AdrMode<>ModNone THEN
	BEGIN
	 WAsmCode[0]:=$000f+(AdrPart SHL 4)+(Word(2-OpSize) SHL 14);
	 DecodeAdr(ArgStr[2],MModPostInc,False);
	 IF AdrMode<>ModNone THEN SetCode(WAsmCode[0]+(Word(AdrPart) SHL 8));
	END;
      END;
     Exit;
    END;

   IF Memo('ADD') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg,False);
       IF AdrMode<>ModNone THEN
	BEGIN
         HReg:=AdrPart; OpSize:=2;
	 DecodeAdr(ArgStr[1],MModReg+MModImm,True);
	 CASE AdrMode OF
	 ModReg:
	  SetCode($300c+(Word(HReg) SHL 8)+(AdrPart SHL 4));
	 ModImm:
	  SetCode($7000+AdrPart+(Word(HReg) SHL 8));
	 END;
	END;
      END;
     Exit;
    END;

   IF Memo('CMP/EQ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MModReg,False);
       IF AdrMode<>ModNone THEN
	BEGIN
         HReg:=AdrPart; OpSize:=2; DecodeAdr(ArgStr[1],MModReg+MModImm,True);
	 CASE AdrMode OF
	 ModReg:
	  SetCode($3000+(Word(HReg) SHL 8)+(AdrPart SHL 4));
	 ModImm:
	  IF HReg<>0 THEN WrError(1350)
	  ELSE SetCode($8800+AdrPart);
	 END;
	END;
      END;
     Exit;
    END;

   FOR z:=0 TO LogOrderCount-1 DO
    IF Memo(LogOrders^[z]) THEN
     BEGIN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
	DecodeAdr(ArgStr[2],MModReg+MModGBRR0,False);
	CASE AdrMode OF
	ModReg:
	 IF (AttrPart<>'') AND (OpSize<>2) THEN WrError(1130)
	 ELSE
	  BEGIN
           OpSize:=2;
	   HReg:=AdrPart; DecodeAdr(ArgStr[1],MModReg+MModImm,False);
	   CASE AdrMode OF
	   ModReg:
	    SetCode($2008+z+(Word(HReg) SHL 8)+(AdrPart SHL 4));
	   ModImm:
	    IF HReg<>0 THEN WrError(1350)
	    ELSE SetCode($c800+(z SHL 8)+AdrPart);
	   END;
	  END;
	ModGBRR0:
	 BEGIN
	  DecodeAdr(ArgStr[1],MModImm,False);
	  IF AdrMode<>ModNone THEN
	   SetCode($cc00+(z SHL 8)+AdrPart);
	 END;
	END;
       END;
      Exit;
     END;

   { Miszellaneen.. }

   IF Memo('TRAPA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1100)
     ELSE
      BEGIN
       OpSize:=0;
       DecodeAdr(ArgStr[1],MModImm,False);
       IF AdrMode=ModImm THEN SetCode($c300+AdrPart);
       ChkDelayed;
      END;
     Exit;
    END;

   { SprÅnge }

   IF (Memo('BF')) OR (Memo('BT'))
   OR (Memo('BF/S')) OR (Memo('BT/S')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1110)
     ELSE IF (Length(OpPart)=4) AND (MomCPU<CPU7600) THEN WrError(1500)
     ELSE
      BEGIN
       DelayedAdr:=EvalIntExpression(ArgStr[1],Int32,OK);
       AdrLong:=DelayedAdr-(EProgCounter+4);
       IF OK THEN
	IF Odd(AdrLong) THEN WrError(1375)
        ELSE IF ((AdrLong<-256) OR (AdrLong>254)) AND (NOT SymbolQuestionable) THEN WrError(1370)
	ELSE
	 BEGIN
	  WAsmCode[0]:=$8900+((AdrLong SHR 1) AND $ff);
	  IF OpPart[2]='F' THEN Inc(WAsmCode[0],$200);
          IF Length(OpPart)=4 THEN
	   BEGIN
	    Inc(WAsmCode[0],$400); CurrDelayed:=True;
           END;
	  CodeLen:=2;
          ChkDelayed;
	 END;
      END;
     Exit;
    END;

   IF (Memo('BRA')) OR (Memo('BSR')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1110)
     ELSE
      BEGIN
       DelayedAdr:=EvalIntExpression(ArgStr[1],Int32,OK);
       AdrLong:=DelayedAdr-(EProgCounter+4);
       IF OK THEN
	IF Odd(AdrLong) THEN WrError(1375)
        ELSE IF ((AdrLong<-4096) OR (AdrLong>4094)) AND (NOT SymbolQuestionable) THEN WrError(1370)
	ELSE
	 BEGIN
	  WAsmCode[0]:=$a000+((AdrLong SHR 1) AND $fff);
	  IF Memo('BSR') THEN Inc(WAsmCode[0],$1000);
	  CodeLen:=2;
          CurrDelayed:=True; ChkDelayed;
	 END;
      END;
     Exit;
    END;

   IF (Memo('JSR')) OR (Memo('JMP')) THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1130)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],MModIReg,False);
       IF AdrMode<>ModNone THEN
        BEGIN
         SetCode($400b+(AdrPart SHL 8)+(Ord(Memo('JMP')) SHL 5));
         CurrDelayed:=True; DelayedAdr:=$7fffffff;
         ChkDelayed;
        END;
      END;
     Exit;
    END;

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_7000;
	Far;
BEGIN
   SaveInitProc;
   FirstLiteral:=Nil; ForwardCount:=0;
END;

	FUNCTION ChkPC_7000:Boolean;
	Far;
BEGIN
   ChkPC_7000:=ActPC=SegCode;
END;

	FUNCTION IsDef_7000:Boolean;
	Far;
BEGIN
   IsDef_7000:=False;
END;

	PROCEDURE SwitchFrom_7000;
	Far;
BEGIN
   DeinitFields;
   IF FirstLiteral<>Nil THEN WrError(1495);
END;

	PROCEDURE SwitchTo_7000;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$6c; NOPCode:=$0009;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_7000; ChkPC:=ChkPC_7000; IsDef:=IsDef_7000;
   SwitchFrom:=SwitchFrom_7000; InitFields;

   CurrDelayed:=False; PrevDelayed:=False;

   SetFlag(DoPadding,DoPaddingName,False);
END;

BEGIN
   CPU7000:=AddCPU('SH7000',SwitchTo_7000);
   CPU7600:=AddCPU('SH7600',SwitchTo_7000);
   CPU7700:=AddCPU('SH7700',SwitchTo_7000);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_7000;
   FirstLiteral:=Nil;
END.
