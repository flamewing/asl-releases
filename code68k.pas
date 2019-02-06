{$I STDINC.PAS}
{$IFDEF SEGATTRS}
{$C Moveable DemandLoad Discardable }
{$ENDIF}

       UNIT Code68K;

INTERFACE
       USES NLS,
            AsmPars, AsmSub, AsmDef,
            CodePseu;


IMPLEMENTATION

VAR
   OpSize:Byte;
   RelPos:ShortInt;

   PMMUAvail:Boolean;               { PMMU-Befehle erlaubt? }
   FullPMMU:Boolean;                { voller PMMU-Befehlssatz? }
   AdrNum:Byte;                     { Adressierungsnummer }
   AdrMode:Word;                    { Adressierungsmodus }
   AdrCnt:Integer;                  { Anzahl Zusatzworte Adressierung }
   AdrVals:ARRAY[0..9] OF Word;     { die Worte selber }

   SaveInitProc:PROCEDURE;
   CPU68008,CPU68000,CPU68010,CPU68012,
   CPUCOLD,
   CPU68332,CPU68340,CPU68360,
   CPU68020,CPU68030,CPU68040:CPUVar;


CONST
   D_CPU68008=0;
   D_CPU68000=1;
   D_CPU68010=2;
   D_CPU68012=3;
   D_CPUCOLD=4;
   D_CPU68332=5;
   D_CPU68340=6;
   D_CPU68360=7;
   D_CPU68020=8;
   D_CPU68030=9;
   D_CPU68040=10;

   PMMUAvailName  ='HASPMMU';     { PMMU-Befehle erlaubt }
   FullPMMUName   ='FULLPMMU';    { voller PMMU-Befehlssatz }

   Mdata=1;                { Adressierungsmasken }
   Madr=2;
   Madri=4;
   Mpost=8;
   Mpre=16;
   Mdadri=32;
   Maix=64;
   Mpc=128;
   Mpcidx=256;
   Mabs=512;
   Mimm=1024;
   Mfpn=2048;
   Mfpcr=4096;
   Masks:ARRAY[1..13] OF Word=(1,2,4,8,16,32,64,128,256,512,1024,2048,4096);

{$f-}

{ Adreáargument kopieren : -------------------------------------------------}

	PROCEDURE CopyAdrVals(VAR Dest);

{$IFDEF SPEEDUP}

	Assembler;
ASM
	les     di,Dest         { Zieladresse laden }
	lea     si,[AdrVals]    { Quelladresse }
	mov     cx,[AdrCnt]     { Anzahl Bytes (immer gerade) }
	shr     cx,1            { wg. Wortoperation }
	cld
	rep     movsw           { bagger, schaufel }
END;

{$ELSE}

BEGIN
   Move(AdrVals,Dest,AdrCnt);
END;

{$ENDIF}

	FUNCTION ValReg(Ch:Char):Boolean;

{$IFDEF SPEEDUP}

Inline($5b/             { pop   bx }
       $b8/$00/$f8/     { mov   ax,0f800h }
       $20/$e3/         { and   bl,ah }
       $80/$fb/$30/     { cmp   bl,'0' }
       $75/$01/         { jne   falsch }
       $40);            { inc   al }
		  {falsch: }

{$ELSE}

BEGIN
   ValReg:=(Ch>='0') AND (Ch<='7');
END;

{$ENDIF}


	FUNCTION CodeReg(s:String; VAR Erg:Word):Boolean;

{$IFDEF SPEEDUP}

	Assembler;
ASM
	sub     dx,dx           { Annahme FALSE }
	les     si,s
	cld

	seges   lodsb           { L„nge korrekt ? }
	cmp     al,2
	jne     @falsch         { nein-->Ende }

	seges   lodsw           { Inhalt holen }

	mov     cx,15
	cmp     ax,5053h        { SP ? }
	je      @korr           { ja-->Code 15 }
        cmp     ax,7073h        { sp ? }
        je      @korr
        cmp     ax,5073h
        je      @korr
        cmp     ax,7053h
        je      @korr

	sub     ah,'0'          { untere Grenze testen }
	jc      @falsch
	cmp     ah,7            { obere Grenze testen }
	ja      @falsch
	mov     cl,8            { Offset Adreáregister }
	cmp     al,'A'          { Adreáregister ? }
        je      @rechne         { ja--> }
        cmp     al,'a'
        je      @rechne
	mov     cl,0            { Offset Datenregister }
	cmp     al,'D'          { Datenregister ? }
        je      @rechne
        cmp     al,'d'
        jne     @falsch
@rechne:add     cl,ah           { zusammenrechnen }
@korr:  inc     dx              { Ergebnis TRUE }
	les     di,Erg          { Ergebnis ablegen }
	mov     es:[di],cx
@falsch:mov     al,dl           { Funktionsergebnis }
END;

{$ELSE}

BEGIN
   CodeReg:=True;
   IF NLS_StrCaseCmp(s,'SP')=0 THEN Erg:=15
   ELSE IF ValReg(s[2]) THEN
    IF UpCase(s[1])='D' THEN Erg:=Ord(s[2])-Ord('0')
    ELSE IF UpCase(s[1])='A' THEN Erg:=Ord(s[2])-Ord('0')+8
    ELSE CodeReg:=False
   ELSE CodeReg:=False;
END;

{$ENDIF}

       FUNCTION CodeRegPair(VAR Asc:String; VAR Erg1,Erg2:Word):Boolean;
BEGIN
   CodeRegPair:=False;

   IF (Length(Asc)<>5) THEN Exit;
   IF UpCase(Asc[1])<>'D' THEN Exit;
   IF Asc[3]<>':' THEN Exit;
   IF UpCase(Asc[4])<>'D' THEN Exit;
   IF NOT (ValReg(Asc[2]) AND ValReg(Asc[5])) THEN Exit;

   Erg1:=Ord(Asc[2])-48; Erg2:=Ord(Asc[5])-48;

   CodeRegPair:=True;
END;

       FUNCTION CodeIndRegPair(VAR Asc:String; VAR Erg1,Erg2:Word):Boolean;
BEGIN
   CodeIndRegPair:=False;

   IF (Length(Asc)<>9) THEN Exit;
   IF Asc[1]<>'(' THEN Exit;
   IF (UpCase(Asc[2])<>'D') AND (UpCase(Asc[2])<>'A') THEN Exit;
   IF Asc[4]<>')' THEN Exit;
   IF Asc[5]<>':' THEN Exit;
   IF Asc[6]<>'(' THEN Exit;
   IF (UpCase(Asc[7])<>'D') AND (UpCase(Asc[7])<>'A') THEN Exit;
   IF Asc[9]<>')' THEN Exit;
   IF NOT (ValReg(Asc[3]) AND ValReg(Asc[8])) THEN Exit;

   Erg1:=Ord(Asc[3])-48; IF UpCase(Asc[2])='A' THEN Inc(Erg1,8);
   Erg2:=Ord(Asc[8])-48; IF UpCase(Asc[7])='A' THEN Inc(Erg2,8);

   CodeIndRegPair:=True;
END;

        FUNCTION CodeCache(VAR Asc:String; VAR Erg:Word):Boolean;
BEGIN
   CodeCache:=True;
   IF NLS_StrCaseCmp(Asc,'IC')=0 THEN Erg:=2
   ELSE IF NLS_StrCaseCmp(Asc,'DC')=0 THEN Erg:=1
   ELSE IF NLS_StrCaseCmp(Asc,'IC/DC')=0 THEN Erg:=3
   ELSE IF NLS_StrCaseCmp(Asc,'DC/IC')=0 THEN Erg:=3
   ELSE CodeCache:=False;
END;

        PROCEDURE ChkEven(Adr:LongInt);
BEGIN
   IF (MomCPU<=CPU68340) AND (Odd(Adr)) THEN WrError(180);
END;

        PROCEDURE DecodeAdr(Asc:String; Erl:Word);
LABEL
   AdrFnd;
TYPE
   CompType=(PC,AReg,Index,indir,Disp,none);
   AdrComp=RECORD
	    Name:String;
	    CASE Art:CompType OF
	    AReg:(ANummer:Word);
	    Index:(INummer:Word; Long:Boolean; Scale:Byte);
	    Disp:(Size:Byte; Wert:LongInt);
	   END;
VAR
   l,i,idx,p:Byte;
   rerg:Word;
   lklamm,rklamm,lastrklamm:Byte;
   doklamm:Boolean;
   sh:String[10];

   AdrComps:ARRAY[1..3] OF AdrComp;
   OneComp:AdrComp;
   CompCnt:Byte;
   OutDisp:String;
   OutDispLen:Byte;
   PreInd:Boolean;

   HVal:LongInt;
   HVal16:Integer;
   HVal8:ShortInt;
   FVal:Extended;
   ValOK:Boolean;

	FUNCTION ClassComp(VAR C:AdrComp):Boolean;
BEGIN
   ClassComp:=False;
   WITH C DO
    BEGIN
     IF (Name[1]='[') AND (Name[Length(Name)]=']') THEN
      BEGIN
       Art:=indir; ClassComp:=True; Exit;
      END;
     IF NLS_StrCaseCmp(Name,'PC')=0 THEN
      BEGIN
       Art:=PC; ClassComp:=True; Exit;
      END;
     IF (CodeReg(Copy(Name,1,2),ANummer)) THEN
      IF (ANummer>7) AND (Length(Name)=2) THEN
       BEGIN
	Art:=AReg; Dec(ANummer,8); ClassComp:=True; Exit;
       END
      ELSE
       BEGIN
	IF (Length(Name)>3) AND (Name[3]='.') THEN
	 BEGIN
          CASE UpCase(Name[4]) OF
	  'L':Long:=True;
	  'W':Long:=False;
	  ELSE Exit;
	  END;
	  Delete(Name,3,2);
	 END
	ELSE Long:=MomCPU=CPUCold;
	IF (Length(Name)>3) AND (Name[3]='*') THEN
	 BEGIN
	  CASE Name[4] OF
	  '1':Scale:=0;
	  '2':Scale:=1;
	  '4':Scale:=2;
	  '8':IF MomCPU=CPUCold THEN Exit ELSE Scale:=3;
	  ELSE Exit;
	  END;
	  Delete(Name,3,2);
	 END
	ELSE Scale:=0;
	Inummer:=ANummer; Art:=Index; ClassComp:=True; Exit;
       END;
     Art:=Disp;
     IF Name[Length(Name)-1]='.' THEN
      BEGIN
       CASE UpCase(Name[Length(Name)]) OF
       'L':Size:=2;
       'W':Size:=1;
       ELSE Exit;
       END;
       Delete(Name,Length(Name)-1,2);
      END
     ELSE Size:=1;
     ClassComp:=True; Art:=Disp;
    END;
END;

	PROCEDURE ACheckCPU(MinCPU:CPUVar);
BEGIN
   IF MomCPU<MinCPU THEN
    BEGIN
     WrError(1505); AdrNum:=0; AdrCnt:=0;
    END;
END;

BEGIN
   KillBlanks(Asc);
   l:=Length(Asc);
   AdrNum:=0; AdrCnt:=0;

   { immediate : }

   IF (Asc[1]='#') THEN
    BEGIN
     Delete(Asc,1,1);
     AdrNum:=11;
     AdrMode:=$3c;
     CASE OpSize OF
     0:BEGIN
	AdrCnt:=2;
	HVal8:=EvalIntExpression(Asc,Int8,ValOK);
	IF ValOK THEN AdrVals[0]:=Word(Byte(HVal8));
       END;
     1:BEGIN
	AdrCnt:=2;
	HVal16:=EvalIntExpression(Asc,Int16,ValOK);
	IF ValOK THEN AdrVals[0]:=Word(HVal16);
       END;
     2:BEGIN
	AdrCnt:=4;
	HVal:=EvalIntExpression(Asc,Int32,ValOK);
	IF ValOK THEN
	 BEGIN
	  AdrVals[0]:=HVal SHR 16;
	  AdrVals[1]:=HVal AND $ffff;
	 END;
       END;
     3:BEGIN
	AdrCnt:=8;
	MultiFace.ValCo:=EvalFloatExpression(Asc,FloatCo,ValOK);
	IF ValOK THEN
	 BEGIN
	  AdrVals[0]:=MultiFace.feld[3];
	  AdrVals[1]:=MultiFace.feld[2];
	  AdrVals[2]:=MultiFace.feld[1];
	  AdrVals[3]:=MultiFace.feld[0];
	 END;
       END;
     4:BEGIN
	AdrCnt:=4;
	MultiFace.Val32:=EvalFloatExpression(Asc,Float32,ValOK);
	IF ValOK THEN
	 BEGIN
	  AdrVals[0]:=MultiFace.feld[1];
	  AdrVals[1]:=MultiFace.feld[0];
	 END;
       END;
     5:BEGIN
	AdrCnt:=8;
	MultiFace.Val64:=EvalFloatExpression(Asc,Float64,ValOK);
	IF ValOK THEN
	 BEGIN
	  AdrVals[0]:=MultiFace.feld[3];
	  AdrVals[1]:=MultiFace.feld[2];
	  AdrVals[2]:=MultiFace.feld[1];
	  AdrVals[3]:=MultiFace.feld[0];
	 END;
       END;
     6:BEGIN
	AdrCnt:=12;
	MultiFace.Val80:=EvalFloatExpression(Asc,Float80,ValOK);
	IF ValOK THEN
	 BEGIN
	  AdrVals[0]:=MultiFace.feld[4];
	  AdrVals[1]:=0;
	  AdrVals[2]:=MultiFace.feld[3];
	  AdrVals[3]:=MultiFace.feld[2];
	  AdrVals[4]:=MultiFace.feld[1];
	  AdrVals[5]:=MultiFace.feld[0];
	 END;
       END;
     7:BEGIN
	AdrCnt:=12;
	FVal:=EvalFloatExpression(Asc,Float80,ValOK);
	IF ValOK THEN
	 BEGIN
	  ConvertDec(FVal,MultiFace.feld);
	  AdrVals[0]:=MultiFace.feld[5];
	  AdrVals[1]:=MultiFace.feld[4];
	  AdrVals[2]:=MultiFace.feld[3];
	  AdrVals[3]:=MultiFace.feld[2];
	  AdrVals[4]:=MultiFace.feld[1];
	  AdrVals[5]:=MultiFace.feld[0];
	 END;
       END;
     END;
     Goto AdrFnd;
    END;

   { CPU-Register direkt: }

   IF CodeReg(Asc,AdrMode) THEN
    BEGIN
     AdrCnt:=0; AdrNum:=Succ(AdrMode SHR 3); Goto AdrFnd;
    END;

   { Gleitkommaregister direkt: }

   IF NLS_StrCaseCmp(Copy(Asc,1,2),'FP')=0 THEN
    BEGIN
     IF (Length(Asc)=3) AND (ValReg(Asc[3])) THEN
      BEGIN
       AdrMode:=(Ord(Asc[3])-48); AdrCnt:=0; AdrNum:=12; Goto AdrFnd;
      END;
     IF NLS_StrCaseCmp(Asc,'FPCR')=0 THEN
      BEGIN
       AdrMode:=4; AdrNum:=13; Goto AdrFnd;
      END;
     IF NLS_StrCaseCmp(Asc,'FPSR')=0 THEN
      BEGIN
       AdrMode:=2; AdrNum:=13; Goto AdrFnd;
      END;
     IF NLS_StrCaseCmp(Asc,'FPIAR')=0 THEN
      BEGIN
       AdrMode:=1; AdrNum:=13; Goto AdrFnd;
      END;
    END;


   { Adreáregister indirekt mit Predekrement: }

   IF (l=5) AND (Asc[1]='-') AND (Asc[2]='(') AND (Asc[5]=')') THEN
    IF CodeReg(Copy(Asc,3,2),rerg) THEN
     IF rerg>7 THEN
      BEGIN
       AdrMode:=rerg+24; AdrCnt:=0; AdrNum:=5; Goto AdrFnd;
      END;

   { Adreáregister indirekt mit Postinkrement }

   IF (l=5) AND (Asc[1]='(') AND (Asc[4]=')') AND (Asc[5]='+') THEN
    IF CodeReg(Copy(Asc,2,2),rerg) THEN
     IF rerg>7 THEN
      BEGIN
       AdrMode:=rerg+16; AdrCnt:=0; AdrNum:=4; Goto AdrFnd;
      END;

   { Unterscheidung direkt<->indirekt: }

   lklamm:=0; rklamm:=0; lastrklamm:=0; doklamm:=True;
   FOR i:=1 TO Length(Asc) DO
    BEGIN
     IF Asc[i]='[' THEN doklamm:=False;
     IF Asc[i]=']' THEN doklamm:=True;
     IF doklamm THEN
      IF Asc[i]='(' THEN Inc(lklamm)
      ELSE IF Asc[i]=')' THEN
       BEGIN
	Inc(rklamm); lastrklamm:=i;
       END;
    END;

   IF (lklamm=1) AND (rklamm=1) AND (lastrklamm=Length(Asc)) THEN
    BEGIN

     { „uáeres Displacement abspalten, Klammern l”schen: }

     p:=Pos('(',Asc); OutDisp:=Copy(Asc,1,p-1); Delete(Asc,1,p);
     IF (Length(OutDisp)>2) AND (OutDisp[Length(OutDisp)-1]='.') THEN
      BEGIN
       CASE UpCase(OutDisp[Length(OutDisp)]) OF
       'B':OutDispLen:=0;
       'W':OutDispLen:=1;
       'L':OutDispLen:=2;
       ELSE
	BEGIN
	 WrError(1130); Exit;
	END;
       END;
       Delete(OutDisp,Length(OutDisp)-1,2);
      END
     ELSE OutDispLen:=0;
     Delete(Asc,Length(Asc),1);

     { in Komponenten zerteilen: }

     CompCnt:=0;
     REPEAT
      doklamm:=True;
      p:=1;
      REPEAT
       IF Asc[p]='[' THEN doklamm:=False
       ELSE IF Asc[p]=']' THEN doklamm:=True;
       Inc(p);
      UNTIL ((doklamm) AND (Asc[p]=',')) OR (p>Length(Asc));
      Inc(CompCnt);
      WITH AdrComps[CompCnt] DO
       BEGIN
	Name:=Copy(Asc,1,p-1); Delete(Asc,1,p);
	IF NOT ClassComp(AdrComps[CompCnt]) THEN
	 BEGIN
	  WrError(1350); Exit;
	 END;
	IF (CompCnt=2) AND (Art=AReg) THEN
	 BEGIN
	  Art:=Index; INummer:=ANummer+8; Long:=False; Scale:=0;
	 END;
	IF (Art=Disp) OR ((Art<>Index) AND (CompCnt<>1)) THEN
	 BEGIN
	  WrError(1350); Exit;
	 END;
       END;
     UNTIL Asc='';
     IF (CompCnt>2) OR ((AdrComps[1].Art=Index) AND (CompCnt<>1)) THEN
      BEGIN
       WrError(1350); Exit;
      END;

     { 1. Variante (An....), d(An....) }

     IF AdrComps[1].Art=AReg THEN
      BEGIN

       { 1.1. Variante (An), d(An) }

       IF CompCnt=1 THEN
	BEGIN

         { 1.1.1. Variante (An) }

	 IF (OutDisp='') AND (MAdri AND erl<>0) THEN
	  BEGIN
	   AdrMode:=$10+AdrComps[1].ANummer; AdrNum:=3; AdrCnt:=0;
	   Goto AdrFnd;
	  END

         { 1.1.2. Variante d(An) }

	 ELSE
	  BEGIN
           IF OutDispLen>=2 THEN HVal:=EvalIntExpression(OutDisp,Int32,ValOK)
           ELSE HVal:=EvalIntExpression(OutDisp,SInt16,ValOK);
	   IF NOT ValOK THEN
	    BEGIN
	     WrError(1350); Exit;
	    END;
	   IF (ValOK) AND (HVal=0) AND (MAdri AND erl<>0) AND (OutDispLen=0) THEN
	    BEGIN
             AdrMode:=$10+AdrComps[1].ANummer; AdrNum:=3; AdrCnt:=0;
	     Goto AdrFnd;
	    END;
           IF OutDispLen=0 THEN OutDispLen:=1;
           CASE OutDispLen OF
	   1:BEGIN                   { d16(An) }
	      AdrMode:=$28+AdrComps[1].ANummer; AdrNum:=6;
	      AdrCnt:=2; AdrVals[0]:=HVal AND $ffff;
	      Goto AdrFnd;
	     END;
	   2:BEGIN                   { d32(An) }
	      AdrMode:=$30+AdrComps[1].ANummer; AdrNum:=7;
	      AdrCnt:=6; AdrVals[0]:=$0170;
	      AdrVals[1]:=HVal SHR 16; AdrVals[2]:=HVal AND $ffff;
	      ACheckCPU(CPU68332); Goto AdrFnd;
	     END;
	   END;
	  END;
	END

       { 1.2. Variante d(An,Xi) }

       ELSE
	BEGIN
	 WITH AdrComps[2] DO
	  AdrVals[0]:=(INummer SHL 12)+(Ord(Long) SHL 11)+(Scale SHL 9);
	 AdrMode:=$30+AdrComps[1].ANummer;
	 CASE OutDispLen OF
	 0:BEGIN
	    HVal8:=EvalIntExpression(OutDisp,Int8,ValOK);
	    IF ValOK THEN
	     BEGIN
	      AdrNum:=7; AdrCnt:=2; AdrVals[0]:=AdrVals[0]+Byte(HVal8);
	      IF AdrComps[2].Scale<>0 THEN ACheckCPU(CPUCOLD);
	     END;
	    Goto AdrFnd;
	   END;
	 1:BEGIN
	    HVal16:=EvalIntExpression(OutDisp,Int16,ValOK);
	    IF ValOK THEN
	     BEGIN
	      AdrNum:=7; AdrCnt:=4;
	      AdrVals[0]:=AdrVals[0]+$120; AdrVals[1]:=HVal16;
	      ACheckCPU(CPU68332);
	     END;
	    Goto AdrFnd;
	   END;
	 2:BEGIN
	    HVal:=EvalIntExpression(OutDisp,Int32,ValOK);
	    IF ValOK THEN
	     BEGIN
	      AdrNum:=7; AdrCnt:=6; AdrVals[0]:=AdrVals[0]+$130;
	      AdrVals[1]:=HVal SHR 16; AdrVals[2]:=HVal AND $ffff;
	      ACheckCPU(CPU68332);
	     END;
	    Goto AdrFnd;
	   END;
	 END;
	END;
      END

     { 2. Variante d(PC....) }

     ELSE IF AdrComps[1].Art=PC THEN
      BEGIN

       { 2.1. Variante d(PC) }

       IF CompCnt=1 THEN
	BEGIN
	 IF OutDispLen=0 THEN OutDispLen:=1;
	 HVal:=EvalIntExpression(OutDisp,Int32,ValOK)-(EProgCounter+RelPos);
	 IF NOT ValOK THEN
	  BEGIN
	   WrError(1350); Exit;
	  END;
	 CASE OutDispLen OF
	 1:BEGIN
	    AdrMode:=$3a;
	    HVal16:=HVal;
	    IF HVal16<>HVal THEN
	     BEGIN
	      WrError(1330); Exit;
	     END;
	    AdrNum:=8; AdrCnt:=2; AdrVals[0]:=HVal16;
	    Goto AdrFnd;
	   END;
	 2:BEGIN
	    AdrMode:=$3b;
	    AdrNum:=9; AdrCnt:=6; AdrVals[0]:=$170;
	    AdrVals[1]:=HVal SHR 16; AdrVals[2]:=HVal AND $ffff;
	    ACheckCPU(CPU68332); Goto AdrFnd;
	   END;
	 END;
	END

       { 2.2. Variante d(PC,Xi) }

       ELSE
	BEGIN
	 WITH AdrComps[2] DO
	  AdrVals[0]:=(INummer SHL 12)+(Ord(Long) SHL 11)+(Scale SHL 9);
	 HVal:=EvalIntExpression(OutDisp,Int32,ValOK)-(EProgCounter+RelPos);
	 IF NOT ValOK THEN
	  BEGIN
	   WrError(1350); Exit;
	  END;
	 AdrMode:=$3b;
	 CASE OutDispLen OF
	 0:BEGIN
	    HVal8:=HVal;
	    IF HVal8<>HVal THEN
	     BEGIN
	      WrError(1330); Exit;
	     END;
	    AdrVals[0]:=AdrVals[0]+Byte(HVal8); AdrCnt:=2; AdrNum:=9;
	    IF AdrComps[2].Scale<>0 THEN ACheckCPU(CPUCOLD);
	    Goto AdrFnd;
	   END;
	 1:BEGIN
	    HVal16:=HVal;
	    IF HVal16<>HVal THEN
	     BEGIN
	      WrError(1330); Exit;
	     END;
	    AdrVals[0]:=AdrVals[0]+$120; AdrCnt:=4; AdrNum:=9;
	    AdrVals[1]:=HVal16;
	    ACheckCPU(CPU68332);
	    Goto AdrFnd;
	   END;
	 2:BEGIN
	    AdrVals[0]:=AdrVals[0]+$120; AdrCnt:=6; AdrNum:=9;
	    AdrVals[1]:=HVal SHR 16; AdrVals[2]:=HVal AND $ffff;
	    ACheckCPU(CPU68332);
	    Goto AdrFnd;
	   END;
	 END;
	END;
      END

     { 3. Variante (Xi), d(Xi) }

     ELSE IF AdrComps[1].Art=Index THEN
      BEGIN
       WITH AdrComps[1] DO
	AdrVals[0]:=(INummer SHL 12)+(Ord(Long) SHL 11)+(Scale SHL 9)+$180;
       AdrMode:=$30;
       IF OutDisp='' THEN
	BEGIN
	 AdrVals[0]:=AdrVals[0]+$0010; AdrCnt:=2;
	 AdrNum:=7; ACheckCPU(CPU68332); Goto AdrFnd;
	END
       ELSE CASE OutDispLen OF
       0,
       1:BEGIN
	  HVal16:=EvalIntExpression(OutDisp,Int16,ValOK);
	  IF ValOK THEN
	   BEGIN
	    AdrVals[0]:=AdrVals[0]+$0020; AdrVals[1]:=HVal16;
	    AdrNum:=7; AdrCnt:=4; ACheckCPU(CPU68332);
	   END;
	  Goto AdrFnd;
	 END;
       2:BEGIN
	  HVal:=EvalIntExpression(OutDisp,Int32,ValOK);
	  IF ValOK THEN
	   BEGIN
	    AdrVals[0]:=AdrVals[0]+$0030; AdrNum:=7; AdrCnt:=6;
	    AdrVals[1]:=HVal SHR 16; AdrVals[2]:=HVal AND $ffff;
	    ACheckCPU(CPU68332);
	   END;
	  Goto AdrFnd;
	 END;
       END;
      END

     { 4. Variante indirekt: }

     ELSE IF AdrComps[1].Art=indir THEN
      BEGIN

       { erst ab 68020 erlaubt }

       IF MomCPU<CPU68020 THEN
        BEGIN
         WrError(1505); Goto AdrFnd;
        END;

       { Unterscheidung Vor- <---> Nachindizierung: }

       IF CompCnt=2 THEN
	BEGIN
	 PreInd:=False;
	 AdrComps[3]:=AdrComps[2];
	END
       ELSE
	BEGIN
	 PreInd:=True;
	 AdrComps[3].Art:=None;
	END;

       { indirektes Argument herauskopieren: }

       WITH AdrComps[1] DO
	Asc:=Copy(Name,2,Length(Name)-2);

       { Felder l”schen: }

       FOR i:=1 TO 2 DO AdrComps[i].Art:=None;

       { indirekten Ausdruck auseinanderfieseln: }

       REPEAT

	{ abschneiden & klassifizieren: }

	p:=Pos(',',Asc);
	IF p=0 THEN p:=Succ(Length(Asc));
	OneComp.Name:=Copy(Asc,1,p-1); Delete(Asc,1,p);
	IF NOT ClassComp(OneComp) THEN
	 BEGIN
	  WrError(1350); Exit;
	 END;

	{ passend einsortieren: }

	WITH OneComp DO
	 BEGIN
	  IF (AdrComps[2].Art<>None) AND (Art=AReg) THEN
	   BEGIN
	    Art:=Index; INummer:=ANummer+8; Long:=False; Scale:=0;
	   END;
	  CASE Art OF
	  Disp:i:=1;
	  AReg,PC:i:=2;
	  Index:i:=3;
	  END;
	 END;
	IF AdrComps[i].Art<>None THEN
	 BEGIN
	  WrError(1350); Exit;
	 END
	ELSE AdrComps[i]:=OneComp;
       UNTIL Asc='';

       { Vor-oder Nachindizierung? }

       AdrVals[0]:=$100+Ord(PreInd) SHL 2;

       { Indexregister eintragen }

       WITH AdrComps[3] DO
	IF Art=None THEN Inc(AdrVals[0],$40)
	ELSE Inc(AdrVals[0],(INummer SHL 12)+(Ord(Long) SHL 11)+(Scale SHL 9));

       { 4.1 Variante d([...PC...]...) }

       IF AdrComps[2].Art=PC THEN
	BEGIN
         ValOK:=True;
         IF AdrComps[1].Art=None THEN HVal:=0
         ELSE HVal:=EvalIntExpression(AdrComps[1].Name,Int32,ValOK);
         Dec(HVal,EProgCounter+RelPos);
	 IF NOT ValOK THEN Exit;
	 AdrMode:=$3b;
	 CASE AdrComps[1].Size OF
	 1:BEGIN
	    HVal16:=HVal;
	    IF HVal16<>HVal THEN
	     BEGIN
	      WrError(1330); Exit;
	     END;
	    AdrVals[1]:=HVal16; Inc(AdrVals[0],$20); AdrNum:=7; AdrCnt:=4;
	   END;
	 2:BEGIN
	    AdrVals[1]:=HVal SHR 16; AdrVals[2]:=HVal AND $ffff;
	    Inc(AdrVals[0],$30); AdrNum:=7; AdrCnt:=6;
	   END;
	 END;
	END

       { 4.2 Variante d([...An...]...) }

       ELSE
	BEGIN
	 WITH AdrComps[2] DO
	  IF Art=None THEN
	   BEGIN
	    AdrMode:=$30; Inc(AdrVals[0],$80);
	   END
	 ELSE AdrMode:=$30+AdrComps[2].ANummer;

	 WITH AdrComps[1] DO
	  IF Art=None THEN
	   BEGIN
	    AdrNum:=7; AdrCnt:=2; Inc(AdrVals[0],$10);
	   END
	  ELSE CASE Size OF
	  1:BEGIN
	     HVal16:=EvalIntExpression(Name,Int16,ValOK);
	     IF NOT ValOK THEN Exit;
	     AdrNum:=7; AdrVals[1]:=HVal16; AdrCnt:=4; Inc(AdrVals[0],$20);
	    END;
	  2:BEGIN
	     HVal:=EvalIntExpression(Name,Int32,ValOK);
	     IF NOT ValOK THEN Exit;
	     AdrNum:=7; AdrCnt:=6; Inc(AdrVals[0],$30);
	     AdrVals[1]:=HVal SHR 16; AdrVals[2]:=HVal AND $ffff;
	    END;
	  END;
	END;

       { „uáeres Displacement: }

       IF OutDispLen=0 THEN OutDispLen:=1;
       IF OutDisp='' THEN
	BEGIN
	 Inc(AdrVals[0]); Goto AdrFnd;
	END
       ELSE CASE OutDispLen OF
       1:BEGIN
	  HVal16:=EvalIntExpression(OutDisp,Int16,ValOK);
	  IF NOT ValOK THEN
	   BEGIN
	    AdrNum:=0; AdrCnt:=0; Exit;
	   END;
	  AdrVals[AdrCnt SHR 1]:=HVal16; Inc(AdrCnt,2); Inc(AdrVals[0],2);
	 END;
       2:BEGIN
	  HVal:=EvalIntExpression(OutDisp,Int32,ValOK);
	  IF NOT ValOK THEN
	   BEGIN
	    AdrNum:=0; AdrCnt:=0; Exit;
	   END;
	  AdrVals[AdrCnt SHR 1  ]:=HVal SHR 16;
	  AdrVals[AdrCnt SHR 1+1]:=HVal AND $ffff;
	  Inc(AdrCnt,4); Inc(AdrVals[0],3);
	 END;
       END;

       Goto AdrFnd;

      END;

    END

   { absolut: }

   ELSE
    BEGIN
     AdrCnt:=0;
     IF NLS_StrCaseCmp(Copy(Asc,l-1,2),'.W')=0 THEN
      BEGIN
       AdrCnt:=2; Asc:=Copy(Asc,1,l-2);
      END
     ELSE IF NLS_StrCaseCmp(Copy(Asc,l-1,2),'.L')=0 THEN
      BEGIN
       AdrCnt:=4; Asc:=Copy(Asc,1,l-2);
      END;

     FirstPassUnknown:=False;
     HVal:=EvalIntExpression(Asc,Int32,ValOK);
     IF (NOT FirstPassUnknown) AND (OpSize>0) THEN ChkEven(HVal);
     HVal16:=HVal;

     IF ValOK THEN
      BEGIN
       IF AdrCnt=0 THEN
	IF HVal16=HVal THEN AdrCnt:=2 ELSE AdrCnt:=4;
       AdrNum:=10;

       IF AdrCnt=2 THEN
	BEGIN
	 IF (HVal16<>HVal) THEN
	  BEGIN
	   WrError(1340); AdrNum:=0;
	  END
	 ELSE
	  BEGIN
	   AdrMode:=$38; AdrVals[0]:=HVal16;
	  END;
	END
       ELSE
	BEGIN
	 AdrMode:=$39; AdrVals[0]:=HVal SHR 16; AdrVals[1]:=HVal AND $ffff;
	END;
      END;
    END;

AdrFnd:
    IF Erl AND Masks[AdrNum]=0 THEN
     BEGIN
      WrError(1350); AdrNum:=0;
     END;
END;

       PROCEDURE MakeCode_68K;
       Far;
TYPE
   SetOfByte=Set OF Byte;
CONST
   CondNams:ARRAY[0..19] OF String[2]=('T' ,'F' ,'HI','LS','CC','CS','NE','EQ',
				       'VC','VS','PL','MI','GE','LT','GT','LE',
				       'HS','LO','RA','SR');
   CondVals:ARRAY[0..19] OF Byte     =(   0,   1,   2,   3,   4,   5,   6,   7,
					  8,   9,  10,  11,  12,  13,  14,  15,
					  4,   5,   0,   1);

VAR
   ValOK:Boolean;
   HVal:LongInt;
   i,HVal16:Integer;
   HVal8:ShortInt;
   FVal:Extended;
   z,z2:Byte;
   w1,w2:Word;
   s:String;
   cpuz:CpuVar;


       FUNCTION ShiftCodes(s:String):Word;
BEGIN
   s:=Copy(s,1,Length(s)-1);
   IF s='AS' THEN ShiftCodes:=0
   ELSE IF s='LS' THEN ShiftCodes:=1
   ELSE IF s='RO' THEN ShiftCodes:=3
   ELSE IF s='ROX' THEN ShiftCodes:=2;
END;

       FUNCTION DecodeRegList(Asc:String; VAR Erg:Word):Boolean;
CONST
    Masks:ARRAY[0..15] OF Word=(1,2,4,8,16,32,64,128,256,512,1024,2048,4096,8192,16384,32768);
VAR
   h,h2,p,z:Byte;
   s:String;

       FUNCTION OneReg(Asc:String):Byte;
BEGIN
   OneReg:=16;
   IF Length(Asc)<>2 THEN Exit;
   IF (UpCase(Asc[1])<>'A') AND (UpCase(Asc[1])<>'D') THEN Exit;
   IF NOT ValReg(Asc[2]) THEN Exit;
   IF UpCase(Asc[1])='D' THEN OneReg:=Ord(Asc[2])-48
                         ELSE OneReg:=Ord(Asc[2])-40;
END;

BEGIN
   Erg:=0; DecodeRegList:=False;
   REPEAT
    p:=Pos('/',Asc); IF p=0 THEN p:=Length(Asc)+1;
    s:=Copy(Asc,1,p-1); Delete(Asc,1,p-1);
    IF Asc[1]='/' THEN Delete(Asc,1,1);
    p:=Pos('-',s);
    IF p=0 THEN
     BEGIN
      h:=OneReg(s); IF h=16 THEN Exit;
      Erg:=Erg OR Masks[h];
     END
    ELSE
     BEGIN
      h:=OneReg(Copy(s,1,p-1)); IF h=16 THEN Exit;
      h2:=OneReg(Copy(s,p+1,Length(s)-p)); IF h2=16 THEN Exit;
      FOR z:=h TO h2 DO Erg:=Erg OR Masks[z];
     END;
   UNTIL Asc='';
   DecodeRegList:=True;
END;

       FUNCTION DecodeCtrlReg(Asc:String; VAR Erg:Word):Boolean;
TYPE
    CtReg=RECORD
	   Name:String[7];
	   Code:Word;
           FirstCPU,LastCPU:Byte;
	  END;
CONST
    CtRegCnt=29;
    CtRegs:ARRAY[0..CtRegCnt-1] OF CtReg=((Name:'SFC'  ;Code:$000;FirstCPU:D_CPU68010;LastCPU:D_CPU68040),
                                          (Name:'DFC'  ;Code:$001;FirstCPU:D_CPU68010;LastCPU:D_CPU68040),
                                          (Name:'CACR' ;Code:$002;FirstCPU:D_CPUCOLD ;LastCPU:D_CPU68040),
                                          (Name:'USP'  ;Code:$800;FirstCPU:D_CPU68010;LastCPU:D_CPU68040),
                                          (Name:'VBR'  ;Code:$801;FirstCPU:D_CPU68010;LastCPU:D_CPU68040),
                                          (Name:'CAAR' ;Code:$802;FirstCPU:D_CPU68020;LastCPU:D_CPU68030),
                                          (Name:'MSP'  ;Code:$803;FirstCPU:D_CPU68020;LastCPU:D_CPU68040),
                                          (Name:'ISP'  ;Code:$804;FirstCPU:D_CPU68020;LastCPU:D_CPU68040),
                                          (Name:'TC'   ;Code:$003;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'ITT0' ;Code:$004;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'ITT1' ;Code:$005;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'DTT0' ;Code:$006;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'DTT1' ;Code:$007;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'MMUSR';Code:$805;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'URP'  ;Code:$806;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'SRP'  ;Code:$807;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'IACR0';Code:$004;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'IACR1';Code:$005;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'DACR0';Code:$006;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'DACR1';Code:$007;FirstCPU:D_CPU68040;LastCPU:D_CPU68040),
                                          (Name:'TCR'  ;Code:$003;FirstCPU:D_CPUCOLD ;LastCPU:D_CPUCOLD ),
                                          (Name:'ACR2' ;Code:$004;FirstCPU:D_CPUCOLD ;LastCPU:D_CPUCOLD ),
                                          (Name:'ACR3' ;Code:$005;FirstCPU:D_CPUCOLD ;LastCPU:D_CPUCOLD ),
                                          (Name:'ACR0' ;Code:$006;FirstCPU:D_CPUCOLD ;LastCPU:D_CPUCOLD ),
                                          (Name:'ACR1' ;Code:$007;FirstCPU:D_CPUCOLD ;LastCPU:D_CPUCOLD ),
                                          (Name:'ROMBAR';Code:$c00;FirstCPU:D_CPUCOLD ;LastCPU:D_CPUCOLD),
                                          (Name:'RAMBAR0';Code:$c04;FirstCPU:D_CPUCOLD;LastCPU:D_CPUCOLD),
                                          (Name:'RAMBAR1';Code:$c05;FirstCPU:D_CPUCOLD;LastCPU:D_CPUCOLD),
                                          (Name:'MBAR' ;Code:$c0f;FirstCPU:D_CPUCOLD ;LastCPU:D_CPUCOLD ));
VAR
   z:Byte;
BEGIN
   z:=0; NLS_UpString(Asc);
   WHILE (z<CtRegCnt) AND ((CtRegs[z].Name<>Asc) OR (CtRegs[z].FirstCPU+CPU68008>MomCPU)
                                                 OR (CtRegs[z].LastCPU+CPU68008<MomCPU)) DO
    Inc(z);
   DecodeCtrlReg:=z<>CtRegCnt;
   IF z<>CtRegCnt THEN Erg:=CtRegs[z].Code;
END;

	FUNCTION SplitBitField(VAR Arg:String; VAR Erg:Word):Boolean;
VAR
   p:Byte;
   OfsVal:Word;
   Desc:String;

	FUNCTION OneField(Asc:String; VAR Erg:Word; Ab1:Boolean):Boolean;
VAR
   ValOK:Boolean;
BEGIN
   OneField:=False;

   IF (Length(Asc)=2) AND (UpCase(Asc[1])='D') AND (ValReg(Asc[2])) THEN
    BEGIN
     Erg:=$20+(Ord(Asc[2])-48); OneField:=True;
    END
   ELSE
    BEGIN
     Erg:=EvalIntExpression(Asc,Int8,ValOK);
     IF (Ab1) AND (Erg=32) THEN Erg:=0;
     OneField:=(ValOK) AND (Erg<32);
    END;
END;

BEGIN
   SplitBitField:=False;

   p:=Pos('{',Arg);
   IF p=0 THEN Exit;
   Desc:=Copy(Arg,p+1,Length(Arg)-p); Arg:=Copy(Arg,1,p-1);
   IF Pos('}',Desc)<>Length(Desc) THEN Exit;
   Delete(Desc,Length(Desc),1);

   p:=Pos(':',Desc);
   IF p=0 THEN Exit;
   IF NOT OneField(Copy(Desc,1,p-1),OfsVal,False) THEN Exit;
   IF NOT OneField(Copy(Desc,p+1,Length(Desc)-p),Erg,True) THEN Exit;
   Erg:=Erg+(OfsVal SHL 6);
   SplitBitField:=True;
END;

       PROCEDURE CheckCPU(Level:CpuVar);
BEGIN
   IF MomCPU<Level THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    END;
END;

	PROCEDURE Check020;
BEGIN
   IF MomCPU<>CPU68020 THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    End;
END;

        PROCEDURE Check32;
BEGIN
   IF (MomCPU<CPU68332) OR (MomCPU>CPU68360) THEN
    BEGIN
     WrError(1500); CodeLen:=0;
    End;
END;

       PROCEDURE CheckSup;
BEGIN
   IF NOT SupAllowed THEN WrError(50);
END;

	FUNCTION CheckColdSize:Boolean;
BEGIN
   IF (OpSize>2) OR ((MomCPU=CPUCold) AND (OpSize<2)) THEN
    BEGIN
     WrError(1130); CheckColdSize:=False;
    END
   ELSE CheckColdSize:=True;
END;

{$i code68k.i81}
{$i code68k.i51}

       FUNCTION DecodePseudo:Boolean;
CONST
   ONOFF68KCount=4;
   ONOFF68Ks:ARRAY[1..ONOFF68KCount] OF ONOFFRec=
             ((Name:'PMMU'    ; Dest:@PMMUAvail ; FlagName:PMMUAvailName ),
              (Name:'FULLPMMU'; Dest:@FullPMMU  ; FlagName:FullPMMUName  ),
              (Name:'FPU'     ; Dest:@FPUAvail  ; FlagName:FPUAvailName  ),
              (Name:'SUPMODE' ; Dest:@SupAllowed; FlagName:SupAllowedName));
VAR
   z:Integer;

       PROCEDURE PutByte(b:Byte);
BEGIN
   IF (Odd(CodeLen)) THEN
    BEGIN
     BAsmCode[CodeLen]:=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]:=b;
    END
   ELSE
    BEGIN
     BAsmCode[CodeLen]:=b;
    END;
   Inc(CodeLen);
END;

BEGIN
   DecodePseudo:=True;

   IF Memo('STR') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF Length(ArgStr[1])<2 THEN WrError(1135)
     ELSE IF ArgStr[1][1]<>'''' THEN WrError(1135)
     ELSE IF ArgStr[1][Length(ArgStr[1])]<>'''' THEN WrError(1135)
     ELSE
      BEGIN
       PutByte(Length(ArgStr[2])-2);
       FOR z:=2 TO Length(ArgStr[1]) DO
        PutByte(Ord(CharTransTable[ArgStr[1][z]]));
      END;
     Exit;
    END;

   IF CodeONOFF(@ONOFF68Ks,ONOFF68KCount) THEN Exit;

   DecodePseudo:=False;
END;

	FUNCTION CodeDual:Boolean;
BEGIN
   CodeDual:=True;

   IF Memo('MOVE') OR Memo('MOVEA') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'USP')=0 THEN
      BEGIN
       IF (AttrPart<>'') AND (OpSize<>2) THEN WrError(1130)
       ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[2],Madr);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=2; WAsmCode[0]:=$4e68 OR (AdrMode AND 7); CheckSup;
	  END;
	END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'USP')=0 THEN
      BEGIN
       IF (AttrPart<>'') AND (OpSize<>2) THEN WrError(1130)
       ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],Madr);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=2; WAsmCode[0]:=$4e60 OR (AdrMode AND 7); CheckSup;
	  END;
	END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'SR')=0 THEN
      BEGIN
       IF (OpSize<>1) THEN WrError(1130)
       ELSE
	BEGIN
         IF MomCPU=CPUCOLD THEN DecodeAdr(ArgStr[2],Mdata)
	 ELSE DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=2+AdrCnt; WAsmCode[0]:=$40c0 OR AdrMode;
	   CopyAdrVals(WAsmCode[1]);
	   IF MomCPU>=CPU68010 THEN CheckSup;
	  END;
	END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[1],'CCR')=0 THEN
      BEGIN
       IF (AttrPart<>'') AND (OpSize>1) THEN WrError(1130)
       ELSE
	BEGIN
         OpSize:=0;
         IF MomCPU=CPUCOLD THEN DecodeAdr(ArgStr[2],Mdata)
	 ELSE DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=2+AdrCnt; WAsmCode[0]:=$42c0 OR AdrMode;
	   CopyAdrVals(WAsmCode[1]); CheckCPU(CPU68010);
	  END;
	END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'SR')=0 THEN
      BEGIN
       IF (OpSize<>1) THEN WrError(1130)
       ELSE
	BEGIN
         IF MomCPU=CPUCOLD THEN DecodeAdr(ArgStr[1],Mdata+Mimm)
	 ELSE DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=2+AdrCnt; WAsmCode[0]:=$46c0 OR AdrMode;
	   CopyAdrVals(WAsmCode[1]); CheckSup;
	  END;
	END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'CCR')=0 THEN
      BEGIN
       IF (AttrPart<>'') AND (OpSize>1) THEN WrError(1130)
       ELSE
	BEGIN
         OpSize:=0;
         IF MomCPU=CPUCOLD THEN DecodeAdr(ArgStr[1],Mdata+Mimm)
	 ELSE DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=2+AdrCnt; WAsmCode[0]:=$44c0 OR AdrMode;
	   CopyAdrVals(WAsmCode[1]);
	  END;
	END;
      END
     ELSE
      BEGIN
       IF OpSize>2 THEN WrError(1130)
       ELSE
	BEGIN
	 DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 IF AdrNum<>0 THEN
	  BEGIN
	   z:=AdrCnt; CodeLen:=2+z; CopyAdrVals(WAsmCode[1]);
	   IF OpSize=0 THEN WAsmCode[0]:=$1000
	   ELSE IF OpSize=1 THEN WAsmCode[0]:=$3000
	   ELSE WAsmCode[0]:=$2000;
	   WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	   DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	   IF AdrMode<>0 THEN
            IF (MomCPU=CPUCOLD) AND (z>0) AND (AdrCnt>0) THEN WrError(1350)
            ELSE
	     BEGIN
	      AdrMode:=((AdrMode AND 7) SHL 3) OR (AdrMode SHR 3);
	      WAsmCode[0]:=WAsmCode[0] OR (AdrMode SHL 6);
	      CopyAdrVals(WAsmCode[CodeLen SHR 1]);
	      Inc(CodeLen,AdrCnt);
	     END;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('LEA') THEN
    BEGIN
     IF (AttrPart<>'') AND (OpSize<>2) THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],Madr);
       IF AdrNum<>0 THEN
	BEGIN
         OpSize:=0;
	 WAsmCode[0]:=$41c0 OR ((AdrMode AND 7) SHL 9);
	 DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR AdrMode; CodeLen:=2+AdrCnt;
	   CopyAdrVals(WAsmCode[1]);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('ASL') OR Memo('ASR') OR Memo('LSL' ) OR Memo('LSR' )
   OR Memo('ROL') OR Memo('ROR') OR Memo('ROXL') OR Memo('ROXR') THEN
    BEGIN
     IF ArgCnt=1 THEN
      BEGIN
       ArgStr[2]:=ArgStr[1]; ArgStr[1]:='#1';
       ArgCnt:=2;
      END;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (OpPart[1]='R') AND (MomCPU=CPUCOLD) THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       IF AdrNum=1 THEN
	BEGIN
	 IF CheckColdSize THEN
	  BEGIN
	   WAsmCode[0]:=$e000 OR AdrMode OR (ShiftCodes(OpPart) SHL 3) OR (OpSize SHL 6);
	   IF OpPart[Length(OpPart)]='L' THEN WAsmCode[0]:=WAsmCode[0] OR $100;
	   OpSize:=1;
	   DecodeAdr(ArgStr[1],Mdata+Mimm);
	   IF (AdrNum=1) OR ((AdrNum=11) AND (Lo(AdrVals[0]) IN [1..8])) THEN
	    BEGIN
	     CodeLen:=2;
	     IF AdrNum=1 THEN WAsmCode[0]:=WAsmCode[0] OR $20 OR (AdrMode SHL 9)
	     ELSE WAsmCode[0]:=WAsmCode[0] OR ((AdrVals[0] AND 7) SHL 9);
	    END
           ELSE WrError(1380);
	  END;
	END
       ELSE IF AdrNum<>0 THEN
        IF MomCPU=CPUCold THEN WrError(1350)
        ELSE
	 BEGIN
	  IF OpSize<>1 THEN WrError(1130)
	  ELSE
	   BEGIN
	    WAsmCode[0]:=$e0c0 OR AdrMode OR (ShiftCodes(OpPart) SHL 9);
	    IF OpPart[Length(OpPart)]='L' THEN WAsmCode[0]:=WAsmCode[0] OR $100;
	    CopyAdrVals(WAsmCode[1]);
	    IF ArgStr[1][1]='#' THEN Delete(ArgStr[1],1,1);
	    HVal8:=EvalIntExpression(ArgStr[1],Int8,ValOK);
	    IF (ValOK) AND (HVal8=1) THEN CodeLen:=2+AdrCnt
            ELSE WrError(1390);
	   END;
	 END;
      END;
     Exit;
    END;

   IF Memo('ADDQ') OR Memo('SUBQ') THEN
    BEGIN
     IF CheckColdSize THEN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
        IF AdrNum<>0 THEN
 	 BEGIN
 	  WAsmCode[0]:=$5000 OR AdrMode OR (OpSize SHL 6);
	  IF OpPart='SUBQ' THEN WAsmCode[0]:=WAsmCode[0] OR $100;
	  CopyAdrVals(WAsmCode[1]);
	  IF ArgStr[1][1]='#' THEN Delete(ArgStr[1],1,1);
          FirstPassUnknown:=False;
	  HVal8:=EValIntExpression(ArgStr[1],UInt4,ValOK);
          IF FirstPassUnknown THEN HVal8:=1;
	  IF (ValOK) AND (HVal8 IN [1..8]) THEN
	   BEGIN
	    CodeLen:=2+AdrCnt;
	    WAsmCode[0]:=WAsmCode[0] OR (Word(HVal8 AND 7) SHL 9);
	   END
          ELSE WrError(1390);
	 END;
       END;
     Exit;
    END;

   IF Memo('ADDX') OR Memo('SUBX') THEN
    BEGIN
     IF CheckColdSize THEN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[1],Mdata+Mpre);
        IF AdrNum<>0 THEN
 	 BEGIN
	  WAsmCode[0]:=$9100 OR (OpSize SHL 6) OR (AdrMode AND 7);
	  IF AdrNum=5 THEN WAsmCode[0]:=WAsmCode[0] OR 8;
	  IF Memo('ADDX') THEN WAsmCode[0]:=WAsmCode[0] OR $4000;
	  DecodeAdr(ArgStr[2],Masks[AdrNum]);
	  IF AdrNum<>0 THEN
	   BEGIN
	    CodeLen:=2;
	    WAsmCode[0]:=WAsmCode[0] OR ((AdrMode AND 7) SHL 9);
	   END;
	 END;
       END;
     Exit;
    END;

   IF Memo('CMPM') THEN
    BEGIN
     IF OpSize>2 THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mpost);
       IF AdrNum=4 THEN
	BEGIN
	 WAsmCode[0]:=$b108 OR (OpSize SHL 6) OR (AdrMode AND 7);
	 DecodeAdr(ArgStr[2],Mpost);
	 IF AdrNum=4 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR ((AdrMode AND 7) SHL 9);
	   CodeLen:=2;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Copy(OpPart,1,3)='ADD') OR (Copy(OpPart,1,3)='SUB') OR ((Copy(OpPart,1,3)='CMP') AND (OpPart[4]<>'2')) THEN
    BEGIN
     OpPart:=Copy(OpPart,1,3);
     IF CheckColdSize THEN
      IF ArgCnt<>2 THEN WrError(1110)
      ELSE
       BEGIN
        DecodeAdr(ArgStr[2],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
        IF AdrNum=2 THEN { ADDA ? }
	 IF OpSize=0 THEN WrError(1130)
	 ELSE
	  BEGIN
	   WAsmCode[0]:=$90c0 OR ((AdrMode AND 7) SHL 9);
	   IF OpPart='ADD' THEN WAsmCode[0]:=WAsmCode[0] OR $4000
	   ELSE IF OpPart='CMP' THEN WAsmCode[0]:=WAsmCode[0] OR $2000;
	   IF OpSize=2 THEN WAsmCode[0]:=WAsmCode[0] OR $100;
	   DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	   IF AdrNum<>0 THEN
	    BEGIN
	     WAsmCode[0]:=WAsmCode[0] OR AdrMode; CodeLen:=2+AdrCnt;
	     CopyAdrVals(WAsmCode[1]);
	    END;
	  END
        ELSE IF AdrNum=1 THEN            { ADD <EA>,Dn ? }
	 BEGIN
	  WAsmCode[0]:=$9000 OR (OpSize SHL 6) OR (AdrMode SHL 9);
	  IF OpPart='ADD' THEN WAsmCode[0]:=WAsmCode[0] OR $4000
	  ELSE IF OpPart='CMP' THEN WAsmCode[0]:=WAsmCode[0] OR $2000;
	  DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	  IF AdrNum<>0 THEN
	   BEGIN
	    CodeLen:=2+AdrCnt; CopyAdrVals(WAsmCode[1]);
	    WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	   END;
	 END
        ELSE
	 BEGIN
	  DecodeAdr(ArgStr[1],Mdata+Mimm);
	  IF AdrNum=11 THEN             { ADDI ? }
	   BEGIN
	    WAsmCode[0]:=$400 OR (OpSize SHL 6);
	    IF OpPart='ADD' THEN WAsmCode[0]:=WAsmCode[0] OR $200
	    ELSE IF OpPart='CMP' THEN WAsmCode[0]:=WAsmCode[0] OR $800;
	    CodeLen:=2+AdrCnt;
	    CopyAdrVals(WAsmCode[1]);
            IF MomCPU=CPUCOLD THEN DecodeAdr(ArgStr[2],Mdata)
	    ELSE DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	    IF AdrNum<>0 THEN
	     BEGIN
	      WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	      CopyAdrVals(WAsmCode[CodeLen SHR 1]);
	      Inc(CodeLen,AdrCnt);
	     END
	    ELSE CodeLen:=0;
	   END
	  ELSE IF AdrNum<>0 THEN         { ADD Dn,<EA> ? }
	   BEGIN
	    IF OpPart='CMP' THEN WrError(1420)
	    ELSE
	     BEGIN
	      WAsmCode[0]:=$9100 OR (OpSize SHL 6) OR (AdrMode SHL 9);
	      IF OpPart='ADD' THEN WAsmCode[0]:=WAsmCode[0] OR $4000;
	      DecodeAdr(ArgStr[2],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	      IF AdrNum<>0 THEN
	       BEGIN
	        CodeLen:=2+AdrCnt; CopyAdrVals(WAsmCode[1]);
	        WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	       END;
	     END;
	   END;
	 END;
       END;
     Exit;
    END;

   IF (Copy(OpPart,1,3)='AND') OR (Copy(OpPart,1,2)='OR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF CheckColdSize THEN
      BEGIN
       IF (NLS_StrCaseCmp(ArgStr[2],'CCR')<>0) AND (NLS_STrCaseCmp(ArgStr[2],'SR')<>0) THEN
       DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       IF NLS_StrCaseCmp(ArgStr[2],'CCR')=0 THEN               { AND #...,CCR }
 	BEGIN
         IF (AttrPart<>'') AND (OpSize<>0) THEN WrError(1130)
         ELSE IF (MomCPU=CPU68008) OR (MomCPU=CPUCOLD) THEN WrError(1500)
	 ELSE
	  BEGIN
	   IF Copy(OpPart,1,3)='AND' THEN WAsmCode[0]:=$023c
	   ELSE WAsmCode[0]:=$003c;
	   OpSize:=0; DecodeAdr(ArgStr[1],Mimm);
	   IF AdrNum<>0 THEN
	    BEGIN
	     CodeLen:=4; WAsmCode[1]:=AdrVals[0];
	    END;
	  END;
	END
       ELSE IF NLS_StrCaseCmp(ArgStr[2],'SR')=0 THEN           { AND #...,SR }
	BEGIN
         IF (AttrPart<>'') AND (OpSize<>1) THEN WrError(1130)
         ELSE IF (MomCPU=CPU68008) OR (MomCPU=CPUCOLD) THEN WrError(1500)
	 ELSE
	  BEGIN
	   IF Copy(OpPart,1,3)='AND' THEN WAsmCode[0]:=$027c
	   ELSE WAsmCode[0]:=$007c;
	   OpSize:=1; DecodeAdr(ArgStr[1],Mimm);
	   IF AdrNum<>0 THEN
	    BEGIN
	     CodeLen:=4; WAsmCode[1]:=AdrVals[0]; CheckSup;
	    END;
	  END;
	END
       ELSE IF AdrNum=1 THEN                 { AND <EA>,Dn }
	BEGIN
	 WAsmCode[0]:=$8000 OR (OpSize SHL 6) OR (AdrMode SHL 9);
	 IF Copy(OpPart,1,3)='AND' THEN WAsmCode[0]:=WAsmCode[0] OR $4000;
	 DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	   CodeLen:=2+AdrCnt; CopyAdrVals(WAsmCode[1]);
	  END;
	END
       ELSE IF AdrNum<>0 THEN                { AND ...,<EA> }
	BEGIN
	 DecodeAdr(ArgStr[1],Mdata+Mimm);
	 IF AdrNum=11 THEN                   { AND #..,<EA> }
	  BEGIN
	   WAsmCode[0]:=OpSize SHL 6;
	   IF Copy(OpPart,1,3)='AND' THEN WAsmCode[0]:=WAsmCode[0] OR $200;
	   CodeLen:=2+AdrCnt;
	   CopyAdrVals(WAsmCode[1]);
	   DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	   IF AdrNum<>0 THEN
	    BEGIN
	     WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	     CopyAdrVals(WAsmCode[CodeLen SHR 1]);
	     Inc(CodeLen,AdrCnt);
	    END
	   ELSE CodeLen:=0;
	  END
	 ELSE IF AdrNum<>0 THEN         { AND Dn,<EA> ? }
	  BEGIN
	   WAsmCode[0]:=$8100 OR (OpSize SHL 6) OR (AdrMode SHL 9);
	   IF Copy(OpPart,1,3)='AND' THEN WAsmCode[0]:=WAsmCode[0] OR $4000;
	   DecodeAdr(ArgStr[2],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	   IF AdrNum<>0 THEN
	    BEGIN
	     CodeLen:=2+AdrCnt; CopyAdrVals(WAsmCode[1]);
	     WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF (Copy(OpPart,1,3)='EOR') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'CCR')=0 THEN
      BEGIN
       IF (AttrPart<>'') AND (OpSize<>0) THEN WrError(1130)
       ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
       ELSE
	BEGIN
	 WAsmCode[0]:=$a3c; OpSize:=0;
	 DecodeAdr(ArgStr[1],Mimm);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=4; WAsmCode[1]:=AdrVals[0];
	  END;
	END;
      END
     ELSE IF NLS_StrCaseCmp(ArgStr[2],'SR')=0 THEN
      BEGIN
       IF OpSize<>1 THEN WrError(1130)
       ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
       ELSE
	BEGIN
	 WAsmCode[0]:=$a7c;
	 DecodeAdr(ArgStr[1],Mimm);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=4; WAsmCode[1]:=AdrVals[0]; CheckSup;
	   CheckCPU(CPU68000);
	  END;
	END;
      END
     ELSE IF CheckColdSize THEN
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Mimm);
       IF AdrNum=1 THEN
	BEGIN
	 WAsmCode[0]:=$b100 OR (AdrMode SHL 9) OR (OpSize SHL 6);
	 DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=2+AdrCnt; CopyAdrVals(WAsmCode[1]);
	   WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	  END;
	END
       ELSE IF AdrNum=11 THEN
	BEGIN
	 WAsmCode[0]:=$0a00 OR (OpSize SHL 6);
	 CopyAdrVals(WAsmCode[1]); CodeLen:=2+AdrCnt;
         IF MomCPU=CPUCold THEN DecodeAdr(ArgStr[2],Mdata)
	 ELSE DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CopyAdrVals(WAsmCode[CodeLen SHR 1]);
	   Inc(CodeLen,AdrCnt);
	   WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	  END
	 ELSE CodeLen:=0;
	END;
      END;
     Exit;
    END;

   CodeDual:=False;
END;

BEGIN
   CodeLen:=0;
   IF MomCPU=CPUCold THEN OpSize:=2 ELSE OpSize:=1;
   DontPrint:=False; RelPos:=2;

   IF AttrPart<>'' THEN
   CASE UpCase(AttrPart[1]) OF
   'B':OpSize:=0;
   'W':OpSize:=1;
   'L':OpSize:=2;
   'Q':OpSize:=3;
   'S':OpSize:=4;
   'D':OpSize:=5;
   'X':OpSize:=6;
   'P':OpSize:=7;
   ELSE
    BEGIN
     WrError(1107); Exit;
    END;
   END;

   { Nullanweisung }

   IF Memo('') AND (AttrPart='') AND (ArgCnt=0) THEN Exit;

   { Pseudoanweisungen }

   IF DecodeMoto16Pseudo(OpSize,True) THEN Exit;

   IF DecodePseudo THEN Exit;

   { ungerader Befehlsz„hler ? }

   IF Odd(EProgCounter) THEN WrError(180);

   { Befehlserweiterungen }

   IF (OpPart[1]='F') AND (FPUAvail) THEN
    BEGIN
     Delete(OpPart,1,1);
     DecodeFPUOrders;
     Exit;
    END;

   IF (OpPart[1]='P') AND (OpPart<>'PEA') AND (PMMUAvail) THEN
    BEGIN
     Delete(OpPart,1,1);
     DecodePMMUOrders;
     Exit;
    END;

   { Anweisungen ohne Argument }

   IF Memo('NOP') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4e71;
      END;
     Exit;
    END;

   IF Memo('RESET') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF MomCPU=CPUCold THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4e70; CheckSup;
      END;
     Exit;
    END;

   IF Memo('ILLEGAL') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4afc;
      END;
     Exit;
    END;

   IF Memo('TRAPV') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4e76;
      END;
     Exit;
    END;

   IF Memo('RTE') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4e73; CheckSup;
      END;
     Exit;
    END;

   IF Memo('RTR') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4e77;
      END;
     Exit;
    END;

   IF Memo('RTS') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4e75;
      END;
     Exit;
    END;

   IF Memo('BGND') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4afa;
       Check32;
      END;
     Exit;
    END;

   IF Memo('HALT') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4ac8;
      END;
     Exit;
    END;

   IF Memo('PULSE') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>0 THEN WrError(1110)
     ELSE
      BEGIN
       CodeLen:=2; WAsmCode[0]:=$4acc;
      END;
     Exit;
    END;

   { Anweisungen mit konstantem Argument }

   IF Memo('STOP') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1120)
     ELSE
      BEGIN
       HVal:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),Int16,ValOK);
       IF ValOK THEN
	BEGIN
	 CodeLen:=4; WAsmCode[0]:=$4e72; WAsmCode[1]:=HVal; CheckSup;
	END;
      END;
     Exit;
    END;

   IF Memo('LPSTOP') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1120)
     ELSE
      BEGIN
       HVal:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),Int16,ValOK);
       IF ValOK THEN
	BEGIN
	 CodeLen:=6;
	 WAsmCode[0]:=$f800;
	 WAsmCode[1]:=$01c0;
	 WAsmCOde[2]:=HVal;
         CheckSup; Check32;
	END;
      END;
     Exit;
    END;

   IF Memo('TRAP') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1120)
     ELSE
      BEGIN
       HVal:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),Int4,ValOK);
       IF ValOK THEN
	BEGIN
	 HVal8:=HVal;
	 CodeLen:=2; WAsmCode[0]:=$4e40+(HVal8 AND 15);
	END;
      END;
     Exit;
    END;

   IF Memo('BKPT') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1120)
     ELSE
      BEGIN
       HVal:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),UInt3,ValOK);
       IF ValOK THEN
        BEGIN
         HVal8:=HVal;
         CodeLen:=2; WAsmCode[0]:=$4848+(HVal8 AND 7);
         CheckCPU(CPU68010);
        END;
      END;
     Exit;
    END;

   IF Memo('RTD') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE IF ArgStr[1][1]<>'#' THEN WrError(1120)
     ELSE
      BEGIN
       HVal:=EvalIntExpression(Copy(ArgStr[1],2,Length(ArgStr[1])-1),Int16,ValOK);
       IF ValOK THEN
	BEGIN
	 CodeLen:=4; WAsmCode[0]:=$4e74; WAsmCode[1]:=HVal;
	 CheckCPU(CPU68010);
	END;
      END;
     Exit;
    END;

   { Anweisungen mit einem Speicheroperanden }

   IF Memo('PEA') THEN
    BEGIN
     IF (AttrPart<>'') AND (OpSize<>2) THEN WrError(1100)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=0;
       DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       IF AdrNum<>0 THEN
	BEGIN
	 CodeLen:=2+AdrCnt;
	 WAsmCode[0]:=$4840 OR AdrMode;
	 CopyAdrVals(WAsmCode[1]);
	END
      END;
     Exit;
    END;

   IF Memo('CLR') OR Memo('TST') THEN
    BEGIN
     IF OpSize>2 THEN WrError(1130)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       w1:=Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs;
       IF (OpSize>0) AND (Memo('TST')) AND (MomCPU>=CPU68332) THEN
        Inc(w1,Madr+Mpc+Mpcidx+Mimm);
       DecodeAdr(ArgStr[1],w1);
       IF AdrNum<>0 THEN
        BEGIN
         CodeLen:=2+AdrCnt;
         IF OpPart='TST' THEN WAsmCode[0]:=$4a00 ELSE WAsmCode[0]:=$4200;
         WAsmCode[0]:=WAsmCode[0] OR (OpSize SHL 6) OR AdrMode;
         CopyAdrVals(WAsmCode[1]);
        END
      END;
     Exit;
    END;

   IF Memo('JMP') OR Memo('JSR') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1130)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       IF AdrNum<>0 THEN
	BEGIN
	 CodeLen:=2+AdrCnt;
	 IF OpPart='JMP' THEN WAsmCode[0]:=$4ec0 ELSE WAsmCode[0]:=$4e80;
	 WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	 CopyAdrVals(WAsmCode[1]);
	END
      END;
     Exit;
    END;

   IF Memo('NBCD') OR Memo('TAS') THEN
    BEGIN
     IF (AttrPart<>'') AND (OpSize<>0) THEN WrError(1130)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       OpSize:=0;
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       IF AdrNum<>0 THEN
	BEGIN
	 CodeLen:=2+AdrCnt;
	 IF OpPart='NBCD' THEN WAsmCode[0]:=$4800 ELSE WAsmCode[0]:=$4ac0;
	 WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	 CopyAdrVals(WAsmCode[1]);
	END
      END;
     Exit;
    END;

   IF Memo('NEG') OR Memo('NOT') OR Memo('NEGX') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF CheckColdSize THEN
      BEGIN
       IF MomCPU=CPUCOLD THEN DecodeAdr(ArgStr[1],Mdata)
       ELSE DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       IF AdrNum<>0 THEN
	BEGIN
	 CodeLen:=2+AdrCnt;
	 IF OpPart='NOT' THEN WAsmCode[0]:=$4600
	 ELSE IF OpPart='NEG' THEN WAsmCode[0]:=$4400
	 ELSE WAsmCode[0]:=$4000;
	 WAsmCode[0]:=WAsmCode[0] OR (OpSize SHL 6) OR AdrMode;
	 CopyAdrVals(WAsmCode[1]);
	END
      END;
     Exit;
    END;

   IF Memo('SWAP') THEN
    BEGIN
     IF (AttrPart<>'') AND (OpSize<>2) THEN WrError(1130)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata);
       IF AdrNum<>0 THEN
	BEGIN
	 CodeLen:=2; WAsmCode[0]:=$4840 OR AdrMode;
	END;
      END;
     Exit;
    END;

   IF Memo('UNLK') THEN
    BEGIN
     IF (AttrPart<>'') THEN WrError(1130)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Madr);
       IF AdrNum<>0 THEN
	BEGIN
	 CodeLen:=2; WAsmCode[0]:=$4e58 OR AdrMode;
	END;
      END;
     Exit;
    END;

   IF Memo('EXT') THEN
    BEGIN
     IF (ArgCnt<>1) THEN WrError(1110)
     ELSE IF (OpSize=0) OR (OpSize>2) THEN WrError(1130)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata);
       IF AdrNum=1 THEN
	BEGIN
	 WAsmCode[0]:=$4880 OR AdrMode OR (Word(OpSize-1) SHL 6);
	 CodeLen:=2;
	END;
      END;
     Exit;
    END;

   IF Memo('WDDATA') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<>CPUCOLD THEN WrError(1500)
     ELSE IF OpSize>2 THEN WrError(1130)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       IF AdrNum<>0 THEN
        BEGIN
         WAsmCode[0]:=$f400+(OpSize SHL 6)+AdrMode;
         CopyAdrVals(WAsmCode[1]); CodeLen:=2+AdrCnt;
         CheckSup;
        END;
      END;
     Exit;
    END;

   IF Memo('WDEBUG') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF MomCPU<>CPUCOLD THEN WrError(1500)
     ELSE IF CheckColdSize THEN
      BEGIN
       DecodeAdr(ArgStr[1],Madri+Mdadri);
       IF AdrNum<>0 THEN
        BEGIN
         WAsmCode[0]:=$fbc0+AdrMode; WAsmCode[1]:=$0003;
         CopyAdrVals(WAsmCode[2]); CodeLen:=4+AdrCnt;
         CheckSup;
        END;
      END;
     Exit;
    END;

   { zwei Operanden }

   IF CodeDual THEN Exit;

   IF Memo('BSET') OR Memo('BCLR') OR Memo('BCHG') OR Memo('BTST') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF AttrPart='' THEN OpSize:=0;
       IF OpPart<>'BTST' THEN DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs)
       ELSE DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       IF AttrPart='' THEN
        IF AdrNum=1 THEN OpSize:=2 ELSE OpSize:=0;
       IF AdrNum<>0 THEN
	BEGIN
         IF ((AdrNum=1) AND (OpSize<>2)) OR ((AdrNum<>1) AND (OpSize<>0)) THEN WrError(1130)
         ELSE
          BEGIN
           WAsmCode[0]:=AdrMode;
           IF OpPart='BSET' THEN WAsmCode[0]:=WAsmCode[0] OR $c0
           ELSE IF OpPart='BCLR' THEN WAsmCode[0]:=WAsmCode[0] OR $80
           ELSE IF OpPart='BCHG' THEN WAsmCode[0]:=WAsmCode[0] OR $40;
           CodeLen:=2+AdrCnt; CopyAdrVals(WAsmCode[1]);
           OpSize:=0;
           DecodeAdr(ArgStr[1],Mdata+Mimm);
           IF AdrNum=1 THEN
            BEGIN
             WAsmCode[0]:=WAsmCode[0] OR $100 OR (AdrMode SHL 9);
            END
           ELSE IF AdrNum=11 THEN
            BEGIN
             Move(WAsmCode[1],WAsmCode[2],CodeLen-2); WAsmCode[1]:=AdrVals[0];
             WAsmCode[0]:=WAsmCode[0] OR $800;
             Inc(CodeLen,2);
             IF (AdrVals[0]>31)
             OR ((WAsmCode[0] AND $38<>0) AND (AdrVals[0]>7)) THEN
              BEGIN
               CodeLen:=0; WrError(1510);
              END;
            END
           ELSE CodeLen:=0;
          END;
	END;
      END;
     Exit;
    END;

   IF Memo('BFSET') OR Memo('BFCLR')
   OR Memo('BFCHG') OR Memo('BFTST') THEN
    BEGIN
     IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1130)
     ELSE IF NOT SplitBitField(ArgStr[1],WAsmCode[1]) THEN WrError(1750)
     ELSE
      BEGIN
       RelPos:=4;
       OpSize:=0;
       IF Memo('BFTST') THEN DecodeAdr(ArgStr[1],Mdata+Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs)
       ELSE DecodeAdr(ArgStr[1],Mdata+Madri+Mdadri+Maix+Mabs);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$e8c0 OR AdrMode;
	 CopyAdrVals(WAsmCode[2]); CodeLen:=4+AdrCnt;
	 IF OpPart='BFSET' THEN WAsmCode[0]:=WAsmCode[0] OR $600
	 ELSE IF OpPart='BFCLR' THEN WAsmCode[0]:=WAsmCode[0] OR $400
	 ELSE IF OpPart='BFCHG' THEN WAsmCode[0]:=WAsmCode[0] OR $200;
	 CheckCPU(CPU68020);
	END;
      END;
     Exit;
    END;

   IF Memo('BFEXTS') OR Memo('BFEXTU') OR Memo('BFFFO') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1130)
     ELSE IF NOT SplitBitField(ArgStr[1],WAsmCode[1]) THEN WrError(1750)
     ELSE
      BEGIN
       RelPos:=4;
       OpSize:=0;
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$e9c0+AdrMode; CopyAdrVals(WAsmCode[2]);
	 IF OpPart='BFEXTS' THEN WAsmCode[0]:=WAsmCode[0]+$200
	 ELSE IF OpPart='BFFFO' THEN WAsmCode[0]:=WAsmCode[0]+$400;
	 CodeLen:=4+AdrCnt;
	 DecodeAdr(ArgStr[2],Mdata);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[1]:=WAsmCode[1] OR (AdrMode SHL 12);
	   CheckCPU(CPU68020);
	  END
	 ELSE CodeLen:=0;
	END;
      END;
     Exit;
    END;

   IF Memo('BFINS') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1130)
     ELSE IF NOT SplitBitField(ArgStr[2],WAsmCode[1]) THEN WrError(1750)
     ELSE
      BEGIN
       OpSize:=0;
       DecodeAdr(ArgStr[2],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$efc0+AdrMode; CopyAdrVals(WAsmCode[2]);
	 CodeLen:=4+AdrCnt;
	 DecodeAdr(ArgStr[1],Mdata);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[1]:=WAsmCode[1] OR (AdrMode SHL 12);
	   CheckCPU(CPU68020);
	  END
	 ELSE CodeLen:=0;
	END;
      END;
     Exit;
    END;

   IF Memo('MOVEM') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (OpSize<1) OR (OpSize>2) THEN WrError(1130)
     ELSE IF (MomCPU=CPUCOLD) AND (OpSize=1) THEN WrError(1130)
     ELSE
      BEGIN
       IF DecodeRegList(ArgStr[2],WAsmCode[1]) THEN
	BEGIN
         IF (MomCPU=CPUCOLD) THEN DecodeAdr(ArgStr[1],Madri+Mdadri)
	 ELSE DecodeAdr(ArgStr[1],Madri+Mpost+Mdadri+Maix+Mpc+Mpcidx+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=$4c80 OR AdrMode OR ((OpSize-1) SHL 6);
	   CodeLen:=4+AdrCnt; CopyAdrVals(WAsmCode[2]);
	  END;
	END
       ELSE IF DecodeRegList(ArgStr[1],WAsmCode[1]) THEN
	BEGIN
         IF (MomCPU=CPUCOLD) THEN DecodeAdr(ArgStr[2],Madri+Mdadri)
	 ELSE DecodeAdr(ArgStr[2],Madri+Mpre+Mdadri+Maix+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=$4880 OR AdrMode OR ((OpSize-1) SHL 6);
	   CodeLen:=4+AdrCnt; CopyAdrVals(WAsmCode[2]);
	   IF AdrNum=5 THEN
	    BEGIN
	     WAsmCode[9]:=WAsmCode[1]; WAsmCode[1]:=0;
	     FOR z:=0 TO 15 DO
	      BEGIN
	       WAsmCode[1]:=WAsmCode[1] SHL 1;
	       IF Odd(WAsmCode[9]) THEN Inc(WAsmCode[1]);
	       WAsmCode[9]:=WAsmCode[9] SHR 1;
	      END;
	    END;
	  END;
	END
       ELSE WrError(1410);
      END;
     Exit;
    END;

   IF Memo('MOVEQ') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (AttrPart<>'') AND (OpSize<>2) THEN WrError(1130)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],Mdata);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$7000 OR (AdrMode SHL 9);
	 OpSize:=0;
	 DecodeAdr(ArgStr[1],Mimm);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=2; WAsmCode[0]:=WAsmCode[0] OR AdrVals[0];
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('MOVE16') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mpost+Madri+Mabs);
       IF AdrNum<>0 THEN
        BEGIN
         w1:=AdrNum; z:=AdrMode AND 7;
         IF (w1=10) AND (AdrCnt=2) THEN
          BEGIN
           AdrVals[1]:=AdrVals[0];
           AdrVals[0]:=0-(AdrVals[1] SHR 15);
          END;
         DecodeAdr(ArgStr[2],Mpost+Madri+Mabs);
         IF AdrNum<>0 THEN
          BEGIN
           w2:=AdrNum; z2:=AdrMode AND 7;
           IF (w2=10) AND (AdrCnt=2) THEN
            BEGIN
             AdrVals[1]:=AdrVals[0];
             AdrVals[0]:=0-(AdrVals[1] SHR 15);
            END;
           IF (w1=4) AND (w2=4) THEN
            BEGIN
             WAsmCode[0]:=$f620+z;
             WAsmCode[1]:=$8000+(z2 SHL 12);
             CodeLen:=4;
            END
           ELSE
            BEGIN
             WAsmCode[1]:=AdrVals[0]; WAsmCode[2]:=AdrVals[1];
             CodeLen:=6;
             IF (w1=4) AND (w2=10) THEN WAsmCode[0]:=$f600+z
             ELSE IF (w1=10) AND (w2=4) THEN WAsmCode[0]:=$f608+z2
             ELSE IF (w1=3) AND (w2=10) THEN WAsmCode[0]:=$f610+z
             ELSE IF (w1=10) AND (w2=3) THEN WAsmCode[0]:=$f618+z2
             ELSE
              BEGIN
               WrError(1350); CodeLen:=0;
              END;
            END;
           IF CodeLen>0 THEN CheckCPU(CPU68040);
          END;
        END;
      END;
     Exit;
    END;

   IF Memo('EXG') THEN
    BEGIN
     IF (AttrPart<>'') AND (OpSize<>2) THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madr);
       IF AdrNum=1 THEN
	BEGIN
	 WAsmCode[0]:=$c100 OR (AdrMode SHL 9);
	 DecodeAdr(ArgStr[2],Mdata+Madr);
	 IF AdrNum=1 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR $40 OR AdrMode; CodeLen:=2;
	  END
	 ELSE IF AdrNum=2 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR $88 OR (AdrMode AND 7); CodeLen:=2;
	  END;
	END
       ELSE IF AdrNum=2 THEN
	BEGIN
	 WAsmCode[0]:=$c100 OR (AdrMode AND 7);
	 DecodeAdr(ArgStr[2],Mdata+Madr);
	 IF AdrNum=1 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR $88 OR (AdrMode SHL 9); CodeLen:=2;
	  END
	 ELSE
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR $48 OR ((AdrMode AND 7) SHL 9); CodeLen:=2;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('DIVSL') OR Memo('DIVUL') THEN
    BEGIN
     IF AttrPart='' THEN OpSize:=2;
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF OpSize<>2 THEN WrError(1130)
     ELSE IF NOT CodeRegPair(ArgStr[2],w1,w2) THEN WrError(1760)
     ELSE
      BEGIN
       RelPos:=4;
       WAsmCode[1]:=w1+(w2 SHL 12);
       IF OpPart[4]='S' THEN WAsmCode[1]:=WAsmCode[1] OR $800;
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$4c40+AdrMode;
	 CopyAdrVals(WAsmCode[2]); CodeLen:=4+AdrCnt;
	 CheckCPU(CPU68332);
	END;
      END;
     Exit;
    END;

   IF (Copy(OpPart,1,3)='MUL') OR (Copy(OpPart,1,3)='DIV') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF (MomCPU=CPUCOLD) AND (OpPart[1]='D') THEN WrError(1500)
     ELSE IF OpSize=1 THEN
      BEGIN
       DecodeAdr(ArgStr[2],Mdata);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$80c0 OR (AdrMode SHL 9);
	 IF Copy(OpPart,1,3)='MUL' THEN WAsmCode[0]:=WAsmCode[0] OR $4000;
	 IF OpPart[4]='S' THEN WAsmCode[0]:=WAsmCode[0] OR $100;
	 DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR AdrMode;
	   CodeLen:=2+AdrCnt; CopyAdrVals(WAsmCode[1]);
	  END;
	END;
      END
     ELSE IF OpSize=2 THEN
      BEGIN
       IF Pos(':',ArgStr[2])=0 THEN ArgStr[2]:=ArgStr[2]+':'+ArgStr[2];
       IF NOT CodeRegPair(ArgStr[2],w1,w2) THEN WrError(1760)
       ELSE
	BEGIN
	 WAsmCode[1]:=w1+(w2 SHL 12); RelPos:=4;
	 IF w1<>w2 THEN WAsmCode[1]:=WAsmCode[1] OR $400;
	 IF OpPart[4]='S' THEN WAsmCode[1]:=WAsmCode[1] OR $800;
	 DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=$4c00+AdrMode;
	   IF Copy(OpPart,1,3)='DIV' THEN WAsmCode[0]:=WAsmCode[0] OR $40;
	   CopyAdrVals(WAsmCode[2]); CodeLen:=4+AdrCnt;
	   IF w1<>w2 THEN CheckCPU(CPU68332) ELSE CheckCPU(CPUCOLD);
	  END;
	END;
      END
     ELSE WrError(1130);
     Exit;
    END;

   IF Memo('ABCD') OR Memo('SBCD') THEN
    BEGIN
     IF (OpSize<>0) AND (AttrPart<>'') THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       OpSize:=0;
       DecodeAdr(ArgStr[1],Mdata+Mpre);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$8100 OR (AdrMode AND 7);
	 IF AdrNum=5 THEN WAsmCode[0]:=WAsmCode[0] OR 8;
	 IF OpPart='ABCD' THEN WAsmCode[0]:=WAsmCode[0] OR $4000;
	 DecodeAdr(ArgStr[2],Masks[AdrNum]);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=2;
	   WAsmCode[0]:=WAsmCode[0] OR ((AdrMode AND 7) SHL 9);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('CHK') THEN
    BEGIN
     IF OpSize<>1 THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mpc+Mpcidx+Mabs+Mimm);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$4180 OR AdrMode; CodeLen:=2+AdrCnt;
	 CopyAdrVals(WAsmCode[1]);
	 DecodeAdr(ArgStr[2],Mdata);
	 IF AdrNum=1 THEN WAsmCode[0]:=WAsmCode[0] OR (AdrMode SHL 9)
	 ELSE CodeLen:=0;
	END;
      END;
     Exit;
    END;

   IF Memo('LINK') THEN
    BEGIN
     IF (AttrPart='') AND (MomCPU=CPUCOLD) THEN OpSize:=1;
     IF (OpSize<1) OR (OpSize>2) THEN WrError(1130)
     ELSE IF (OpSize=2) AND (MomCPU<CPU68332) THEN WrError(1500)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Madr);
       IF AdrNum<>0 THEN
	BEGIN
	 IF OpSize=1 THEN WAsmCode[0]:=$4e50
	 ELSE WAsmCode[0]:=$4808;
	 Inc(WAsmCode[0],AdrMode AND 7);
	 DecodeAdr(ArgStr[2],Mimm);
	 IF AdrNum=11 THEN
	  BEGIN
	   CodeLen:=2+AdrCnt; Move(AdrVals,WAsmCode[1],AdrCnt);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('MOVEP') THEN
    BEGIN
     IF (OpSize=0) OR (OpSize>2) THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Mdadri);
       IF AdrNum=1 THEN
	BEGIN
	 WAsmCode[0]:=$188 OR ((OpSize-1) SHL 6) OR (AdrMode SHL 9);
	 DecodeAdr(ArgStr[2],Mdadri);
	 IF AdrNum=6 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR (AdrMode AND 7);
	   CodeLen:=4; WAsmCode[1]:=AdrVals[0];
	  END;
	END
       ELSE IF AdrNum=6 THEN
	BEGIN
	 WAsmCode[0]:=$108 OR ((OpSize-1) SHL 6) OR (AdrMode AND 7);
	 WAsmCode[1]:=AdrVals[0];
	 DecodeAdr(ArgStr[2],Mdata);
	 IF AdrNum=1 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR ((AdrMode AND 7) SHL 9);
	   CodeLen:=4;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('MOVEC') THEN
    BEGIN
     IF (AttrPart<>'') AND (OpSize<>2) THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       IF DecodeCtrlReg(ArgStr[1],WAsmCode[1]) THEN
	BEGIN
	 DecodeAdr(ArgStr[2],Mdata+Madr);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=4; WAsmCode[0]:=$4e7a;
	   WAsmCode[1]:=WAsmCode[1] OR (AdrMode SHL 12); CheckSup;
	  END;
	END
       ELSE IF DecodeCtrlReg(ArgStr[2],WAsmCode[1]) THEN
	BEGIN
	 DecodeAdr(ArgStr[1],Mdata+Madr);
	 IF AdrNum<>0 THEN
	  BEGIN
	   CodeLen:=4; WAsmCode[0]:=$4e7b;
	   WAsmCode[1]:=WAsmCode[1] OR (AdrMode SHL 12); CheckSup;
	  END;
	END
       ELSE WrError(1440);
      END;
     Exit;
    END;

   IF Memo('MOVES') THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF OpSize>2 THEN WrError(1130)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
       IF (AdrNum=1) OR (AdrNum=2) THEN
	BEGIN
	 WAsmCode[1]:=$800 OR (AdrMode SHL 12);
	 DecodeAdr(ArgStr[2],Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=$e00 OR AdrMode OR (OpSize SHL 6); CodeLen:=4+AdrCnt;
	   CopyAdrVals(WAsmCode[2]); CheckSup;
	   CheckCPU(CPU68010);
	  END;
	END
       ELSE IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$e00 OR AdrMode OR (OpSize SHL 6);
	 CodeLen:=4+AdrCnt; CopyAdrVals(WAsmCode[2]);
	 DecodeAdr(ArgStr[2],Mdata+Madr);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[1]:=AdrMode SHL 12;
	   CheckSup;
	   CheckCPU(CPU68010);
	  END
	 ELSE CodeLen:=0;
	END;
      END;
     Exit;
    END;

   IF Memo('CALLM') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       OpSize:=0;
       DecodeAdr(ArgStr[1],Mimm);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[1]:=AdrVals[0]; RelPos:=4;
	 DecodeAdr(ArgStr[2],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=$06c0+AdrMode;
	   CopyAdrVals(WAsmCode[2]); CodeLen:=4+AdrCnt;
	   CheckCPU(CPU68020); Check020;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('CAS') THEN
    BEGIN
     IF OpSize>2 THEN WrError(1130)
     ELSE IF ArgCnt<>3 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[1]:=AdrMode;
	 DecodeAdr(ArgStr[2],Mdata);
	 IF AdrNum<>0 THEN
	  BEGIN
	   RelPos:=4;
	   WAsmCode[1]:=WAsmCode[1]+(Word(AdrMode) SHL 6);
	   DecodeAdr(ArgStr[3],Mdata+Madr+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	   IF AdrNum<>0 THEN
	    BEGIN
	     WAsmCode[0]:=$08c0+AdrMode+(Word(OpSize+1) SHL 9);
	     CopyAdrVals(WAsmCode[2]); CodeLen:=4+AdrCnt;
	     CheckCPU(CPU68020);
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('CAS2') THEN
    BEGIN
     IF (OpSize<>1) AND (OpSize<>2) THEN WrError(1130)
     ELSE IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF NOT CodeRegPair(ArgStr[1],WAsmCode[1],WAsmCode[2]) THEN WrError(1760)
     ELSE IF NOT CodeRegPair(ArgStr[2],w1,w2) THEN WrError(1760)
     ELSE
      BEGIN
       WAsmCode[1]:=WAsmCode[1]+(w1 SHL 6);
       WAsmCode[2]:=WAsmCode[2]+(w2 SHL 6);
       IF NOT CodeIndRegPair(ArgStr[3],w1,w2) THEN WrError(1760)
       ELSE
	BEGIN
	 WAsmCode[1]:=WAsmCode[1]+(w1 SHL 12);
	 WAsmCode[2]:=WAsmCode[2]+(w2 SHL 12);
	 WAsmCode[0]:=$0cfc+(Word(OpSize-1) SHL 9);
	 CodeLen:=6;
	 CheckCPU(CPU68020);
	END;
      END;
     Exit;
    END;

   IF Memo('CHK2') OR Memo('CMP2') THEN
    BEGIN
     IF OpSize>2 THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],Mdata+Madr);
       IF AdrNum<>0 THEN
	BEGIN
	 RelPos:=4;
	 WAsmCode[1]:=Word(AdrMode) SHL 12;
	 DecodeAdr(ArgStr[1],Madri+Mdadri+Maix+Mpc+Mpcidx+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=$00c0+(Word(OpSize) SHL 9)+AdrMode;
	   IF OpPart='CHK2' THEN WAsmCode[1]:=WAsmCode[1] OR $0800;
	   CopyAdrVals(WAsmCode[2]); CodeLen:=4+AdrCnt;
	   CheckCPU(CPU68332);
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('EXTB') THEN
    BEGIN
     IF (OpSize<>2) AND (AttrPart<>'') THEN WrError(1130)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$49c0+AdrMode; CodeLen:=2;
	 CheckCPU(CPU68332);
	END;
      END;
     Exit;
    END;

   IF Memo('PACK') OR Memo('UNPK') THEN
    BEGIN
     IF ArgCnt<>3 THEN WrError(1110)
     ELSE IF AttrPart<>'' THEN WrError(1130)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Mpre);
       IF AdrNum<>0 THEN
	BEGIN
	 IF OpPart='PACK'THEN WAsmCode[0]:=$8140 OR (AdrMode AND 7)
	 ELSE WAsmCode[0]:=$8180 OR (AdrMode AND 7);
	 IF AdrNum=5 THEN WAsmCode[0]:=WAsmCode[0]+8;
	 DecodeAdr(ArgStr[2],Masks[AdrNum]);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0] OR ((AdrMode AND 7) SHL 9);
	   DecodeAdr(ArgStr[3],Mimm);
	   IF AdrNum<>0 THEN
	    BEGIN
	     WAsmCode[1]:=AdrVals[0]; CodeLen:=4;
	     CheckCPU(CPU68020);
	    END;
	  END;
	END;
      END;
     Exit;
    END;

   IF Memo('RTM') THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1130)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[1],Mdata+Madr);
       IF AdrNum<>0 THEN
	BEGIN
	 WAsmCode[0]:=$06c0+AdrMode; CodeLen:=2;
	 CheckCPU(CPU68020); Check020;
	END;
      END;
     Exit;
    END;

   IF (Memo('TBLS')) OR (Memo('TBLSN')) OR (Memo('TBLU')) OR (Memo('TBLUN')) THEN
    BEGIN
     IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF OpSize>2 THEN WrError(1130)
     ELSE IF MomCPU<CPU68332 THEN WrError(1500)
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],MData);
       IF AdrNum<>0 THEN
        BEGIN
         HVal:=AdrMode;
         w1:=Pos(':',ArgStr[1]);
         IF w1=0 THEN
          BEGIN
           DecodeAdr(ArgStr[1],MAdrI+MDAdrI+MAIx+MAbs+MPC+MPCIdx);
           IF AdrNum<>0 THEN
            BEGIN
             WAsmCode[0]:=$f800+AdrMode;
             WAsmCode[1]:=$0100+(OpSize SHL 6)+(HVal SHL 12);
             IF OpPart[4]='S' THEN Inc(WAsmCode[1],$0800);
             IF OpPart[Length(OpPart)]='N' THEN Inc(WAsmCode[1],$0400);
             Move(AdrVals,WAsmCode[2],AdrCnt);
             CodeLen:=4+AdrCnt; Check32;
            END;
          END
         ELSE
          BEGIN
           SplitString(ArgStr[1],ArgStr[1],ArgStr[3],w1);
           DecodeAdr(ArgStr[1],MData);
           IF AdrNum<>0 THEN
            BEGIN
             w2:=AdrMode;
             DecodeAdr(ArgStr[3],MData);
             IF AdrNum<>0 THEN
              BEGIN
               WAsmCode[0]:=$f800+w2;
               WAsmCode[1]:=$0000+(OpSize SHL 6)+(HVal SHL 12)+AdrMode;
               IF OpPart[4]='S' THEN Inc(WAsmCode[1],$0800);
               IF OpPart[Length(OpPart)]='N' THEN Inc(WAsmCode[1],$0400);
               CodeLen:=4; Check32;
              END;
            END;
          END;
        END;
      END;
     Exit;
    END;

   { bedingte Befehle }

   IF (Length(OpPart)<=3) AND (OpPart[1]='B') THEN
    BEGIN
     { .W, .S, .L, .X erlaubt }

     IF (OpSize<>1) AND (OpSize<>2) AND (OpSize<>4) AND (OpSize<>6) THEN
      WrError(1130)

     { nur ein Operand erlaubt }

     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE
      BEGIN

       { Bedingung finden, evtl. meckern }

       i:=0; Delete(OpPart,1,1);
       WHILE (i<20) AND (OpPart<>CondNams[i]) DO Inc(i);
       IF i=20 THEN WrError(1360)
       ELSE
	BEGIN

	 { Zieladresse ermitteln, zum Programmz„hler relativieren }

	 HVal:=EvalIntExpression(ArgStr[1],Int32,ValOK);
	 HVal:=HVal-(EProgCounter+2);

	 { Bei Automatik Gr”áe festlegen }

	 IF OpSize=1 THEN
	  BEGIN
	   IF (HVal>-128) AND (HVal<127) THEN OpSize:=4
	   ELSE IF (HVal>-32768) AND (HVal<32767) THEN OpSize:=2
	   ELSE OpSize:=6;
	  END;

	 IF ValOK THEN
	  BEGIN

	   { 16 Bit ? }

	   IF OpSize=2 THEN
	    BEGIN

	     { zu weit ? }

	     HVal16:=HVal;
             IF (HVal16<>HVal) AND (NOT SymbolQuestionable) THEN WrError(1370)
	     ELSE
	      BEGIN

	       { Code erzeugen }

	       CodeLen:=4; WAsmCode[0]:=$6000 OR (CondVals[i] SHL 8);
	       WAsmCode[1]:=HVal16;
	      END
	    END

	   { 8 Bit ? }

	   ELSE IF OpSize=4 THEN
	    BEGIN

	     { zu weit ? }

	     HVal8:=HVal;
             IF (HVal8<>HVal) AND (NOT SymbolQuestionable) THEN WrError(1370)

	     { Code erzeugen }

	     ELSE
	      BEGIN
	       CodeLen:=2;
	       IF HVal8<>0 THEN
		BEGIN
		 WAsmCode[0]:=$6000 OR (CondVals[i] SHL 8) OR Byte(HVal8);
		END
	       ELSE
		BEGIN
		 WAsmCode[0]:=NOPCode;
                 IF (NOT Repass) AND (AttrPart<>'') THEN WrError(60);
		END
	      END
	    END

	   { 32 Bit ? }

	   ELSE
	    BEGIN
	     CodeLen:=6; WAsmCode[0]:=$60ff OR (CondVals[i] SHL 8);
	     WAsmCode[1]:=HVal SHR 16; WAsmCode[2]:=HVal AND $ffff;
	     CheckCPU(CPU68332);
	    END;
	  END;
	END;
       Exit;
      END;
    END;

   IF (Length(OpPart)<=3) AND (OpPart[1]='S') THEN
    BEGIN
     IF (AttrPart<>'') AND (OpSize<>0) THEN WrError(1130)
     ELSE IF ArgCnt<>1 THEN WrError(1130)
     ELSE
      BEGIN
       i:=0; Delete(OpPart,1,1);
       WHILE (i<18) AND (OpPart<>CondNams[i]) DO Inc(i);
       IF i=18 THEN WrError(1360)
       ELSE
	BEGIN
         OpSize:=0;
         IF MomCPU=CPUCOLD THEN DecodeAdr(ArgStr[1],Mdata)
	 ELSE DecodeAdr(ArgStr[1],Mdata+Madri+Mpost+Mpre+Mdadri+Maix+Mabs);
	 IF AdrNum<>0 THEN
	  BEGIN
	   WAsmCode[0]:=$50c0 OR (CondVals[i] SHL 8) OR AdrMode;
	   CodeLen:=2+AdrCnt; CopyAdrVals(WAsmCode[1]);
	  END;
	END;
      END;
     Exit;
    END;

   IF (Length(OpPart)<=4) AND (Copy(OpPart,1,2)='DB') THEN
    BEGIN
     IF (OpSize<>1) THEN WrError(1130)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF MomCPU=CPUCOLD THEN WrError(1500)
     ELSE
      BEGIN
       i:=0; Delete(OpPart,1,2);
       WHILE (i<19) AND (OpPart<>CondNams[i]) DO Inc(i);
       IF i=18 THEN i:=1;
       IF i=19 THEN WrError(1360)
       ELSE
	BEGIN
	 HVal:=EvalIntExpression(ArgStr[2],Int32,ValOK);
	 IF ValOK THEN
	  BEGIN
	   HVal:=HVal-(EProgCounter+2); HVal16:=HVal;
           IF (HVal16<>HVal) AND (NOT SymbolQuestionable) THEN WrError(1370)
	   ELSE
	    BEGIN
	     CodeLen:=4; WAsmCode[0]:=$50c8 OR (CondVals[i] SHL 8);
	     WAsmCode[1]:=HVal16;
	     DecodeAdr(ArgStr[1],Mdata);
	     IF AdrNum=1 THEN WAsmCode[0]:=WAsmCode[0] OR AdrMode
	     ELSE CodeLen:=0;
	    END
	  END;
	END;
       Exit;
      END;
    END;

   IF (Length(OpPart)<=6) AND (Copy(OpPart,1,4)='TRAP') THEN
    BEGIN
     IF AttrPart='' THEN OpSize:=0;
     IF OpSize=0 THEN i:=0 ELSE i:=1;
     IF OpSize>2 THEN WrError(1130)
     ELSE IF ArgCnt<>i THEN WrError(1110)
     ELSE
      BEGIN
       i:=0; Delete(OpPart,1,4);
       WHILE (i<18) AND (OpPart<>CondNams[i]) DO Inc(i);
       IF i=18 THEN WrError(1360)
       ELSE IF (MomCPU=CPUCOLD) AND (i<>1) THEN WrError(1500)
       ELSE
	BEGIN
	 WAsmCode[0]:=$50f8+(i SHL 8);
	 IF OpSize=0 THEN
	  BEGIN
	   WAsmCode[0]:=WAsmCode[0]+4; CodeLen:=2;
	  END
	 ELSE
	  BEGIN
	   DecodeAdr(ArgStr[1],Mimm);
	   IF AdrNum<>0 THEN
	    BEGIN
	     WAsmCode[0]:=WAsmCode[0]+(OpSize+1);
	     CopyAdrVals(WAsmCode[1]); CodeLen:=2+AdrCnt;
	    END;
	  END;
	 CheckCPU(CPUCOLD);
	END;
      END;
     Exit;
    END;

   IF (Memo('CPUSHA')) OR (Memo('CINVA')) THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>1 THEN WrError(1110)
     ELSE IF NOT CodeCache(ArgStr[1],w1) THEN WrXError(1445,ArgStr[1])
     ELSE
      BEGIN
       WAsmCode[0]:=$f418+(w1 SHL 6)+(Ord(Memo('CPUSHA')) SHL 5);
       CodeLen:=2;
       CheckCPU(CPU68040); CheckSup;
      END;
     Exit;
    END;

   IF (Memo('CPUSHL')) OR (Memo('CPUSHP')) OR (Memo('CINVL')) OR (Memo('CINVP')) THEN
    BEGIN
     IF AttrPart<>'' THEN WrError(1100)
     ELSE IF ArgCnt<>2 THEN WrError(1110)
     ELSE IF NOT CodeCache(ArgStr[1],w1) THEN WrXError(1445,ArgStr[1])
     ELSE
      BEGIN
       DecodeAdr(ArgStr[2],Madri);
       IF AdrNum<>0 THEN
        BEGIN
         WAsmCode[0]:=$f400+(w1 SHL 6)+(AdrMode AND 7);
         IF OpPart[2]='P' THEN Inc(WAsmCode[0],$20);
         IF OpPart[Length(OpPart)]='L' THEN Inc(WAsmCode[0],8)
         ELSE Inc(WAsmCode[0],16);
         CodeLen:=2;
         CheckCPU(CPU68040); CheckSup;
        END;
      END;
     Exit;
    END;

   { unbekannter Befehl }

   WrXError(1200,OpPart);
END;

	PROCEDURE InitCode_68K;
	Far;
BEGIN
   SaveInitProc;
   SetFlag(PMMUAvail,PMMUAvailName,False);
   SetFlag(FullPMMU,FullPMMUName,True);
END;

	FUNCTION ChkPC_68K:Boolean;
	Far;
BEGIN
   ChkPC_68K:=ActPC=SegCode;
END;

	FUNCTION IsDef_68K:Boolean;
	Far;
BEGIN
   IsDef_68K:=False;
END;

        PROCEDURE SwitchFrom_68K;
        Far;
BEGIN
END;

	PROCEDURE SwitchTo_68K;
	Far;
BEGIN
   TurnWords:=True; ConstMode:=ConstModeMoto; SetIsOccupied:=False;

   PCSymbol:='*'; HeaderID:=$01; NOPCode:=$4e71;
   DivideChars:=','; HasAttrs:=True; AttrChars:='.';

   ValidSegs:=[SegCode];
   Grans[SegCode]:=1; ListGrans[SegCode]:=2; SegInits[SegCode]:=0;

   MakeCode:=MakeCode_68K; ChkPC:=ChkPC_68K; IsDef:=IsDef_68K;
   SwitchFrom:=SwitchFrom_68K;

   SetFlag(FullPMMU,FullPMMUName,MomCPU<=CPU68020);
   SetFlag(DoPadding,DoPaddingName,True);
END;

BEGIN
   CPU68008:=AddCPU('68008',SwitchTo_68K);
   CPU68000:=AddCPU('68000',SwitchTo_68K);
   CPU68010:=AddCPU('68010',SwitchTo_68K);
   CPU68012:=AddCPU('68012',SwitchTo_68K);
   CPUCOLD :=AddCPU('MCF5200',SwitchTo_68K);
   CPU68332:=AddCPU('68332',SwitchTo_68K);
   CPU68340:=AddCPU('68340',SwitchTo_68K);
   CPU68360:=AddCPU('68360',SwitchTo_68K);
   CPU68020:=AddCPU('68020',SwitchTo_68K);
   CPU68030:=AddCPU('68030',SwitchTo_68K);
   CPU68040:=AddCPU('68040',SwitchTo_68K);

   SaveInitProc:=InitPassProc; InitPassProc:=InitCode_68K;
END.
