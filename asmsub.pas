{$i STDINC.PAS}
{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}
        UNIT AsmSub;

INTERFACE

        USES {$IFDEF WINDOWS}
             WinDOS,Strings,
             {$ELSE}
             DOS,
             {$ENDIF}
             {$IFDEF MSDOS}
             Overlay,
             {$ENDIF}
             StdHandl,NLS,StringUt,StringLi,Chunks,
             AsmDef;


TYPE
   TSwitchProc=PROCEDURE;


        PROCEDURE _Init_AsmSub;


        PROCEDURE AsmSubInit;


        FUNCTION  AddCPU(NewName:String; Switcher:TSwitchProc):CPUVar;

        FUNCTION  AddCPUAlias(OrigName,AliasName:String):Boolean;

	PROCEDURE PrintCPUList(NxtLineProc:SimpProc);

        PROCEDURE ClearCPUList;


        PROCEDURE AddCopyright(NewLine:String);

        PROCEDURE WriteCopyrights(NxtProc:TSwitchProc);

        PROCEDURE ClearCopyrightList;


        FUNCTION  IsBlank(Zeichen:Char):Boolean;

        FUNCTION  QuotPos(VAR s:String; Zeichen:Char):Word;

        FUNCTION  RQuotPos(VAR s:String; Zeichen:Char):Word;

        FUNCTION  FirstBlank(VAR s:String):Byte;

        PROCEDURE SplitString(VAR Source,Left,Right:String; Trenner:Byte);

        PROCEDURE UpString(VAR s:String);

        PROCEDURE KillBlanks(VAR s:String);

        PROCEDURE KillPrefBlanks(VAR s:String);

        PROCEDURE KillPostBlanks(VAR s:String);

        PROCEDURE TranslateString(VAR s:String);

        FUNCTION  StrCmp(VAR s1,s2:String; Hand1,Hand2:LongInt):ShortInt;

        FUNCTION  Memo(s:String):Boolean;


        PROCEDURE AddSuffix(VAR s:String; Suff:String);

        PROCEDURE KillSuffix(VAR s:String);

        FUNCTION  PathPart(Name:String):String;

        FUNCTION  NamePart(Name:String):String;


        FUNCTION  FloatString(f:Extended):String;

        FUNCTION  StrSym(VAR t:TempResult; WithSystem:Boolean):String;


        PROCEDURE ResetPageCounter;

        PROCEDURE NewPage(Level:ShortInt; WithFF:Boolean);

        PROCEDURE WrLstLine(Line:String);

        PROCEDURE SetListLineVal(VAR t:TempResult);


        FUNCTION  ChkSymbName(sym:String):Boolean;

        FUNCTION  ChkMacSymbName(sym:String):Boolean;


        PROCEDURE WrErrorString(VAR Message,Add:String; Warning,Fatal:Boolean);

        PROCEDURE WrError(Num:Word);

        PROCEDURE WrXError(Num:Word; Message:String);

        PROCEDURE ChkIO(ErrNo:Word);

        FUNCTION  ChkRange(Value,Min,Max:LongInt):Boolean;


        FUNCTION  ProgCounter:LongInt;

        FUNCTION  EProgCounter:LongInt;

        FUNCTION  Granularity:Word;

        FUNCTION  ListGran:Word;

        PROCEDURE ChkSpace(Space:Byte);


        PROCEDURE PrintChunk(NChunk:ChunkList);

        PROCEDURE PrintUseList;

        PROCEDURE ClearUseList;


        PROCEDURE AddIncludeList(NewPath:String);

        PROCEDURE RemoveIncludeList(RemPath:String);


        PROCEDURE ClearOutList;

        PROCEDURE AddToOutList(NewName:String);

        PROCEDURE RemoveFromOutList(OldName:String);

        FUNCTION  GetFromOutList:String;


        PROCEDURE CompressLine(TokNam:String; Num:Byte; VAR Line:String);

        PROCEDURE ExpandLine(TokNam:String; Num:Byte; VAR Line:String);

        PROCEDURE KillCtrl(VAR Line:String);


        PROCEDURE ChkStack;

        PROCEDURE ResetStack;

        FUNCTION  StackRes:
{$IFDEF Use32}
        LongInt;
{$ELSE}
        Word;
{$ENDIF}


        FUNCTION  DTime(t1,t2:LongInt):LongInt;


VAR
   ErrorCount,WarnCount:Word;
   GTime:FUNCTION:LongInt;

IMPLEMENTATION

VAR
   CopyrightList,OutList:StringList;
{$IFDEF USE32}
   StartStack,MinStack,LowStack:LongInt;
{$ELSE}
   StartStack,MinStack,LowStack:Word;
{$ENDIF}

{$i as.rsc }
{$i ioerrors.rsc }

{****************************************************************************}
{ Modulinitialisierung }


        PROCEDURE AsmSubInit;
BEGIN
   PageLength:=60; PageWidth:=0;
   ErrorCount:=0; WarnCount:=0;
END;

{****************************************************************************}
{* neuen Prozessor definieren }

        FUNCTION AddCPU(NewName:String; Switcher:TSwitchProc):CPUVar;
VAR
   Lauf,Neu:PCPUDef;
   z:Integer;
BEGIN
   New(Neu);
   WITH Neu^ DO
    BEGIN
     Name:=NewName;
     { nicht NLS_UpString benutzen, weil noch nicht initialisiert }
     FOR z:=1 TO Length(Neu^.Name) DO Neu^.Name[z]:=System.UpCase(Neu^.Name[z]);
     SwitchProc:=Switcher;
     Next:=Nil; Number:=CPUCnt; Orig:=CPUCnt;
    END;
   AddCPU:=CPUCnt; Inc(CPUCnt);

   Lauf:=FirstCPUDef;
   IF Lauf=Nil THEN FirstCPUDef:=Neu
   ELSE
    BEGIN
     WHILE (Lauf^.Next<>Nil) DO Lauf:=Lauf^.Next;
     Lauf^.Next:=Neu;
    END;
END;

        FUNCTION AddCPUAlias(OrigName,AliasName:String):Boolean;
VAR
   Lauf,Neu:PCPUDef;
BEGIN
   Lauf:=FirstCPUDef;

   WHILE (Lauf<>Nil) AND (Lauf^.Name<>OrigName) DO Lauf:=Lauf^.Next;

   IF Lauf=Nil THEN AddCPUAlias:=False
   ELSE
    BEGIN
     New(Neu);
     WITH Neu^ DO
      BEGIN
       Next:=Nil; Name:=AliasName;
       Number:=CPUCnt; Inc(CPUCnt);
       Orig:=Lauf^.Orig;
       SwitchProc:=Lauf^.SwitchProc;
      END;
     WHILE Lauf^.Next<>Nil DO Lauf:=Lauf^.Next;
     Lauf^.Next:=Neu;
     AddCPUAlias:=True;
    END;
END;

	PROCEDURE PrintCPUList(NxtLineProc:SimpProc);
VAR
   Lauf:PCPUDef;
   Proc:TSwitchProc;
BEGIN
   Lauf:=FirstCPUDef; Proc:=NullProc;
   WHILE Lauf<>Nil DO
    BEGIN
     IF Lauf^.Number=Lauf^.Orig THEN
      BEGIN
       IF @Lauf^.SwitchProc<>@Proc THEN
        BEGIN
         Proc:=Lauf^.SwitchProc; WriteLn; NxtLineProc;
        END;
       Write(Lauf^.Name,'':10-Length(Lauf^.Name));
      END;
     Lauf:=Lauf^.Next;
    END;
   WriteLn; NxtLineProc;
END;

        PROCEDURE ClearCPUList;
VAR
   Save:PCPUDef;
BEGIN
   WHILE FirstCPUDef<>Nil DO
    BEGIN
     Save:=FirstCPUDef; FirstCPUDef:=Save^.Next;
     Dispose(Save);
    END;
END;

{****************************************************************************}
{ Copyrightlistenverwaltung }

        PROCEDURE AddCopyright(NewLine:String);
BEGIN
   AddStringListLast(CopyrightList,NewLine);
END;

        PROCEDURE WriteCopyrights(NxtProc:TSwitchProc);
VAR
   Lauf:StringRecPtr;
BEGIN
   IF NOT StringListEmpty(CopyrightList) THEN
    BEGIN
     WriteLn(GetStringListFirst(CopyrightList,Lauf)); NxtProc;
     WHILE Lauf<>Nil DO
      BEGIN
       WriteLn(GetStringListNext(Lauf)); NxtProc;
      END;
    END;
END;

        PROCEDURE ClearCopyrightList;
BEGIN
   ClearStringList(CopyrightList);
END;

{****************************************************************************}
{ ist ein Zeichen ein Leerzeichen ? }

        FUNCTION IsBlank(Zeichen:Char):Boolean;

{$IFDEF SPEEDUP}

        Assembler;
ASM
        mov     al,Zeichen      { Zeichen holen }
        mov     ah,1            { Annahme TRUE }
        cmp     al,' '          { der Reihe nach abfragen }
        je      @isbl
        cmp     al,9
        je      @isbl
        cmp     al,0
        je      @isbl
        cmp     al,255
        je      @isbl
        dec     ah              { doch nicht.... }
@isbl:  mov     al,ah           { Ergebnis in AL }
END;

{$ELSE}

BEGIN
   IsBlank:=(Zeichen=' ') OR (Zeichen=#9) OR (Zeichen=#0) OR (Zeichen=#255);
END;

{$ENDIF}

{----------------------------------------------------------------------------}
{ ermittelt das erste/letzte Auftauchen eines Zeichens au·erhalb }
{ "geschÅtzten" Bereichen }

        FUNCTION  QuotPos(VAR s:String; Zeichen:Char):Word;

{$IFDEF SPEEDUP}

        Assembler;
ASM
        les     si,s            { Zeiger auf s laden }
        cld                     { aufwÑrts suchen }
        seges   lodsb           { LÑngenbyte holen }
        mov     ah,0            { LÑnge nach DI }
        mov     di,ax
        mov     bx,0            { BL=mom. Position }
        mov     dh,Zeichen      { zu suchendes Zeichen nach DH }
        sub     cx,cx           { KlammerzÑhler nullen }
        mov     dl,cl           { AnfÅhrungsflags=FALSE }

@srch:  cmp     dh,']'          { schliessende Klammer beeinflusst sich }
        jne     @nores1         { selber nicht }
        sub     ch,ch
@nores1:inc     bx              { Position eins weiter }
        cmp     bx,di           { Åber Ende weg ? }
        ja      @ende           { ja-->Abflug }
        seges   lodsb           { ansonsten Zeichen holen }
        cmp     al,dh           { passend ? }
        jne     @nosrch
        mov     ah,cl           { wenn ja: alle ZÑhler & Flags mÅssen 0 }
        or      ah,ch           { sein }
        or      ah,dl
        jz      @ende           { heureka! }
        jmp     @srch           { Pech gehabt... }
        cmp     al,'a'          { jenseits von dort kommt nichts }
        jae     @srch
@nosrch:cmp     al,'('          { ansonsten etwas BuchfÅhrung... }
        jne     @no1
        mov     ah,ch           { runde Klammern: keine eckigen Klammern }
        or      ah,dl           { oder Quotes }
        jnz     @srch
        inc     cl
        jmp     @srch
@no1:   cmp     al,')'
        jne     @no2
        mov     ah,ch
        or      ah,dl
        jnz     @srch
        dec     cl
        jmp     @srch
@no2:   cmp     al,'['
        jne     @no3
        mov     ah,cl           { eckige Klammern: keine runden Klammern }
        or      ah,dl           { oder Quotes }
        jnz     @srch
        inc     ch
        jmp     @srch
@no3:   cmp     al,']'
        jne     @no4
        mov     ah,cl
        or      ah,dl
        jnz     @srch
        dec     ch
        jmp     @srch
@no4:   cmp     al,''''
        jne     @no5
        mov     ah,dl           { einfacher Quote: keine Klammern oder }
        and     ah,0feh         { doppelten Quotes }
        or      ah,ch
        or      ah,cl
        jnz     @srch
        xor     dl,1
        jmp     @srch
@no5:   cmp     al,'"'
        jne     @srch
        mov     ah,dl           { doppelter Quote: keine Klammern oder }
        and     ah,0fdh         { einfachen Quotes }
        or      ah,ch
        or      ah,cl
        jnz     @srch
        xor     dl,2
        jmp     @srch

@ende:  mov     ax,bx
END;

{$ELSE}

VAR
   Brack,AngBrack:ShortInt;
   i:Word;
   Quot,Paren:Boolean;
BEGIN
   i:=1;

   Brack:=0; AngBrack:=0; Quot:=False; Paren:=False;
   WHILE (i<=Length(s)) AND
   (NOT ((s[i]=Zeichen) AND (Brack=0) AND (AngBrack=0) AND (NOT Quot) AND (NOT ParEn))) DO
    BEGIN
     CASE s[i] OF
     '(':IF (AngBrack=0) AND (NOT ParEn) AND (NOT Quot) THEN Inc(Brack);
     ')':IF (AngBrack=0) AND (NOT ParEn) AND (NOT Quot) THEN Dec(Brack);
     '[':IF (Brack=0) AND (NOT ParEn) AND (NOT Quot) THEN Inc(AngBrack);
     ']':IF (Brack=0) AND (NOT ParEn) AND (NOT Quot) THEN Dec(AngBrack);
     '"':IF (Brack=0) AND (AngBrack=0) AND (NOT Quot) THEN Paren:=NOT Paren;
     '''':IF (Brack=0) AND (AngBrack=0) AND (NOT Paren) THEN Quot:=NOT Quot;
     END;
     Inc(i);
    END;
   QuotPos:=i;
END;

{$ENDIF}

        FUNCTION  RQuotPos(VAR s:String; Zeichen:Char):Word;
VAR
   Brack,AngBrack:ShortInt;
   i:Word;
   Quot,Paren:Boolean;
BEGIN
   i:=Length(s);

   Brack:=0; AngBrack:=0; Quot:=True; Paren:=True;
   WHILE (i>0) AND
   (NOT ((s[i]=Zeichen) AND (brack=0) AND (angbrack=0) AND (quot) AND (paren))) DO
    BEGIN
     CASE s[i] OF
     ')':IF (AngBrack=0) AND (NOT ParEn) AND (NOT Quot) THEN Inc(Brack);
     '(':IF (AngBrack=0) AND (NOT ParEn) AND (NOT Quot) THEN Dec(Brack);
     ']':IF (Brack=0) AND (NOT ParEn) AND (NOT Quot) THEN Inc(AngBrack);
     '[':IF (Brack=0) AND (NOT ParEn) AND (NOT Quot) THEN Dec(AngBrack);
     '"':IF (Brack=0) AND (AngBrack=0) AND (NOT Quot) THEN Paren:=NOT Paren;
     '''':IF (Brack=0) AND (AngBrack=0) AND (NOT Paren) THEN Quot:=NOT Quot;
     END;
     Dec(i);
    END;
   RQuotPos:=i;
END;

{----------------------------------------------------------------------------}
{ ermittelt das erste Leerzeichen in einem String }

        FUNCTION FirstBlank(VAR s:String):Byte;

{$IFDEF SPEEDUP}

        Assembler;
ASM
        les     si,s            { Zeiger auf String }
        cld
        seges lodsb             { LÑnge holen }
        sub     cx,cx           { nullerweitert in CX }
        mov     cl,al
        jcxz    @nfnd           { nix im Nullstring }

        mov     bl,0            { PositionszÑhler in BL }

@suche: inc     bl              { Position weiter }
        seges   lodsb           { Zeichen holen }
        cmp     al,' '          { Mîglichkeiten abklappern }
        je      @fnd
        cmp     al,9
        je      @fnd
        cmp     al,0
        je      @fnd
        cmp     al,255
        je      @fnd
        loop    @suche          { ansonsten nÑchster }

@nfnd:  mov     bl,255          { nichts gefunden-->$ff }

@fnd:   mov     al,bl
END;

{$ELSE}

VAR
   h,min:Byte;
BEGIN
   min:=255;
   h:=Pos(' ',s);  IF (h<min) AND (h<>0) THEN min:=h;
   h:=Pos(#9,s);   IF (h<min) AND (h<>0) THEN min:=h;
   h:=Pos(#0,s);   IF (h<min) AND (h<>0) THEN min:=h;
   h:=Pos(#255,s); IF (h<min) AND (h<>0) THEN min:=h;
   IF min=255 THEN min:=255;
   FirstBlank:=min;
END;

{$ENDIF}

{----------------------------------------------------------------------------}
{ einen String in zwei Teile zerlegen }

        PROCEDURE SplitString(VAR Source,Left,Right:String; Trenner:Byte);

{$IFDEF SPEEDUP}

        Assembler;
ASM
        mov     dx,ds           { Datensegment retten }
        cld                     { aufwÑrts steppen }
        lds     si,Source       { Zeiger auf Quelle laden }
        lodsb                   { QuellÑnge holen }
        mov     bl,al           { nach BL }
        mov     bh,Trenner      { Trennposition nach BH }
        or      bh,bh           { Trenner=0 ? }
        jnz     @nozero
        mov     bh,bl           { ja, dann Trenner=LÑnge+1 }
        inc     bh
        jmp     @docopy
@nozero:cmp     bh,bl           { Trenner hinter String ? }
        jbe     @docopy
        mov     bh,bl           { dann dito }
        inc     bh

@docopy:mov     ch,0            { nur 8-Bit-ZÑhler }

        mov     cl,bh           { bis Trenner-1 kopieren }
        dec     cl
        les     di,Left         { Zielstring 1 }
        mov     al,cl           { LÑnge ablegen }
        stosb
        shr     cl,1            { Bit 0 nach AX }
        rcl     ax,1
        rep     movsw           { erst wortweise kopieren }
        mov     cx,ax           { dann noch das letzte Zwickel }
        and     cx,1
        rep     movsb

        inc     si              { Åber Trenner hinweg }

        mov     cl,bl           { 2.LÑnge=QuellÑnge-Trenner }
        sub     cl,bh
        jnc     @noneg          { falls<0-->Nullstring }
        sub     cl,cl
@noneg: les     di,Right        { Zielring 2 }
        mov     al,cl           { LÑnge ablegen }
        stosb
        shr     cl,1            { Bit 0 nach AX }
        rcl     ax,1
        rep     movsw           { erst wortweise }
        mov     cx,ax
        and     cx,1
        rep     movsb

        mov     ds,dx           { DS zurÅck }
END;

{$ELSE}

BEGIN
   IF (Trenner=0) OR (Trenner>Length(Source)+1) THEN
    Trenner:=Length(Source)+1;
   Left:=Copy(Source,1,Trenner-1);
   Right:=Copy(Source,Trenner+1,Length(Source)-Trenner);
END;

{$ENDIF}


{----------------------------------------------------------------------------}
{ verbesserte Gro·buchstabenfunktion }

{ einen String in Gro·buchstaben umwandeln.  Dabei Stringkonstanten in Ruhe }
{ lassen }

        PROCEDURE UpString(VAR s:String);

{$IFDEF SPEEDUP}

        Assembler;
ASM
        sub     dx,dx           { onoff:=False }
        les     si,s            { Zeiger auf s }
        lea     bx,[UpcaseTable]{ Umsetzungstabelle adressieren }
        cld
        seges lodsb             { LÑnge in CX }
        mov     cl,al
        mov     ch,0
        jcxz    @null           { Nullstring abfangen }
@schl:  seges   lodsb           { Zeichen holen }
        cmp     al,''''         { Hochkomma ? }
        jne     @nohyp
        inc     dl              { ja: Flag drehen ( nur Bit 0 wichtig ) }
        jmp     @noconv         { keine Konvertierung }
@nohyp: cmp     al,'"'          { dito GÑnsefÅ·chen }
        jne     @noquot
        inc     dh
        jmp     @noconv
@noquot:test    dx,0101h        { dÅrfen wir konvertieren ? }
        jnz     @noconv         { nein... }
        xlat                    { ansonsten zu Gro·buchstaben... }
        mov     es:[si-1],al    { ...und ablegen }
@noconv:loop    @schl           { Schleifenende }
@null:
END;

{$ELSE}

VAR
   z,hyp,quot:Byte;
BEGIN
   hyp:=0; quot:=0;
   FOR z:=1 TO Length(s) DO
    BEGIN
     IF (s[z]='''') AND (NOT Odd(quot)) THEN Inc(hyp);
     IF (s[z]='"') AND (NOT Odd(hyp)) THEN Inc(quot);
     IF NOT (Odd(hyp) OR Odd(quot)) THEN s[z]:=UpCaseTable[s[z]];
    END;
END;

{$ENDIF}


{----------------------------------------------------------------------------}
{ alle Leerzeichen aus einem String lîschen }

       PROCEDURE KillBlanks(VAR s:String);
VAR
   z,dest:Byte;
   InHyp,InQuot:Boolean;
BEGIN
   dest:=0; InHyp:=False; InQuot:=False;
   FOR z:=1 TO Length(s) DO
    BEGIN
     CASE s[z] OF
     '''':IF NOT InQuot THEN InHyp:=NOT InHyp;
     '"':IF NOT InHyp THEN InQuot:=NOT InQuot;
     END;
     IF (NOT IsBlank(s[z])) OR (InHyp) OR (InQuot) THEN
      BEGIN
       Inc(dest); s[dest]:=s[z];
      END;
    END;
   Byte(s[0]):=dest;
END;


{----------------------------------------------------------------------------}
{ fÅhrende Leerzeichen lîschen }

        PROCEDURE KillPrefBlanks(VAR s:String);

{$IFDEF SPEEDUP}

        Assembler;
ASM
        mov     dx,ds           { DS retten }
        lds     si,s            { DS:SI auf s }
        cld                     { aufwÑrts steppen }
        lodsb                   { LÑngenbyte holen }
        mov     di,si           { fÅrs spÑtere Umkopieren }
        mov     bl,al           { LÑngenbyte nach BL }
        mov     bh,0            { auf 16 Bit erweitern }
        sub     cx,cx           { BlankzÑhler initialisieren }

@lblank:inc     cx              { BlankzÑhler hoch }
        cmp     cx,bx           { Stringende erreicht ? }
        ja      @lend
        lodsb                   { zu prÅfendes Zeichen holen }
        cmp     al,' '          { alle mîgl. Blanks abklappern }
        je      @lblank         { wen gefunden, nÑchstes Zeichen prÅfen }
        cmp     al,9
        je      @lblank
        cmp     al,0
        je      @lblank
        cmp     al,255
        je      @lblank

        dec     si              { SI zurÅck auf erstes Nonblank }
@lend:  cmp     cx,1            { Abbruch, falls nichts zu verschieben }
        je      @kend
        sub     bx,cx           { Zahl zu verschiebender Bytes berechnen }
        inc     bx              { =Alte LÑnge-Pos. erstes Nonblank+1 }
        mov     cx,bx           { in CX kopieren fÅr MOVS }
        mov     [di-1],cl       { neue LÑnge einschreiben }

        mov     ax,ds           { DS=ES setzen }
        mov     es,ax
        shr     cx,1            { zuerst wortweise kopieren }
        rep     movsw
        mov     cx,bx           { dann noch ein evtl. Einzelbyte }
        and     cx,1
        rep     movsb

@kend:  mov     ds,dx           { DS wiederherstellen }
END;

{$ELSE}

VAR
   z:Byte;
BEGIN
   z:=1;
   WHILE (z<=Length(s)) AND (IsBlank(s[z])) DO Inc(z);
   IF z<>1 THEN
   {$IFDEF SPEEDUP}
    BEGIN
     Move(s[z],s[1],Length(s)-(z-1));
     Dec(Byte(s[0]),z-1);
    END
   {$ELSE}
   Delete(s,1,z-1);
   {$ENDIF}
END;

{$ENDIF}

{----------------------------------------------------------------------------}
{ anhÑngende Leerzeichen lîschen }

        PROCEDURE KillPostBlanks(VAR s:String);
BEGIN
   WHILE (s<>'') AND (IsBlank(s[Length(s)])) DO Dec(Byte(s[0]));
END;

{****************************************************************************}

        PROCEDURE TranslateString(VAR s:String);
VAR
   z:Integer;
BEGIN
   FOR z:=1 TO Length(s) DO s[z]:=CharTransTable[s[z]];
END;

        FUNCTION StrCmp(VAR s1,s2:String; Hand1,Hand2:LongInt):ShortInt;
{$IFDEF SPEEDUP}
        Assembler;
ASM
        mov     dx,ds           { Datensegment retten }
        lds     si,s1           { Startadressen laden }
        les     di,s2
        cld
        lodsb                   { LÑngenbytes holen }
        mov     ah,es:[di]
        inc     di
        mov     cl,ah           { Minimum beider nach CL }
        cmp     ah,al
        jb      @ismin
        mov     cl,al
@ismin: mov     ch,0            { LÑnge auf 16 Bit aufblasen }
        repe    cmpsb           { Zeichen vergleichen }
        jne     @build          { falls bereits ungleich...}
        cmp     al,ah           { ..ansonsten ist kÅrzerer String kleiner }
        jne     @build
        mov     ax,word ptr Hand1+2 { immer noch gleich:Hi-Anteil Handles }
        cmp     ax,word ptr Hand2+2
        jne     @build
        mov     ax,word ptr Hand1 { zuletzt Lo-Anteile }
        cmp     ax,word ptr Hand2
@build: mov     al,ch           { aus Flags 0,-1,1 bilden }
        je      @end            { Gleichheit entspr. 0 }
        sbb     al,al           { ergibt -1 fÅr kleiner, 0 fÅr grî·er }
        add     al,al           { ergibt -2 oder 0 }
        inc     al              { ergibt -1 oder 1 }
@end:   mov     ds,dx           { Datensegment restaurieren }
END;
{$ELSE}
BEGIN
   IF s1<s2 THEN StrCmp:=-1
   ELSE IF s1>s2 THEN StrCmp:=1
   ELSE IF Hand1<Hand2 THEN StrCmp:=-1
   ELSE IF Hand1>Hand2 THEN StrCmp:=1
   ELSE StrCmp:=0;
END;
{$ENDIF}

{****************************************************************************}
{ nach best. Mnemonic suchen }

        FUNCTION Memo(s:String):Boolean;

{$IFDEF SPEEDUP}

        Assembler;
ASM
        mov     al,0            { Annahme ungleich }
        lea     si,[OpPart]     { Startadressen laden }
        mov     cl,[si]         { LÑnge holen }
        mov     ch,al           { auf 16 Bit erweitern }
        les     di,[s]
        inc     cx              { Vergleich inkl. LÑngenbyte }
        cld
        repe    cmpsb           { vergleichen }
        jne     @end
        inc     al
@end:
END;

{$ELSE}

BEGIN
   Memo:=OpPart=s;
END;

{$ENDIF}


{****************************************************************************}
{ an einen Dateinamen eine Endung anhÑngen }

       PROCEDURE AddSuffix(VAR s:String; Suff:String);
VAR
   p,z:Integer;
   Part:String;
BEGIN
   p:=0;
   FOR z:=1 TO Length(s) DO
    IF s[z]='\' THEN p:=z;
   IF p<>0 THEN Part:=Copy(s,p+1,Length(s)-p) ELSE Part:=s;
   IF Pos('.',Part)=0 THEN s:=s+Suff;
END;


{----------------------------------------------------------------------------}
{ von einem Dateinamen die Endung lîschen }

        PROCEDURE KillSuffix(VAR s:String);
VAR
   z,p:Integer;
   Part:String;
BEGIN
   p:=0;
   FOR z:=1 TO Length(s) DO
    IF s[z]='\' THEN p:=z;
   IF p<>0 THEN Part:=Copy(s,p+1,Length(s)-p) ELSE Part:=s;
   IF Pos('.',Part)<>0 THEN s:=Copy(s,1,p+Pos('.',Part)-1);
END;


{----------------------------------------------------------------------------}
{ Pfadanteil (Laufwerk+Verzeichnis) von einem Dateinamen abspalten }

        FUNCTION  PathPart(Name:String):String;
VAR
   {$IFDEF WINDOWS}
   Dummy,Inp,Erg:ARRAY[0..255] OF Char;
   {$ELSE}
   Dummy,Erg:String;
   {$ENDIF}
BEGIN
   {$IFDEF WINDOWS}
   StrPCopy(@Inp,Name);
   FileSplit(@Inp,@Erg,@Dummy,@Dummy);
   PathPart:=StrPas(Erg);
   {$ELSE}
   FSplit(Name,Erg,Dummy,Dummy);
   PathPart:=Erg;
   {$ENDIF}
END;


{----------------------------------------------------------------------------}
{ Namensanteil von einem Dateinamen abspalten }

        FUNCTION  NamePart(Name:String):String;
VAR
   {$IFDEF WINDOWS}
   Inp,Dummy,Erg1,Erg2:Array[0..255] OF Char;
   {$ELSE}
   Dummy,Erg1,Erg2:String;
   {$ENDIF}
BEGIN
   {$IFDEF WINDOWS}
   StrPCopy(@Inp,Name);
   FileSplit(@Inp,@Dummy,@Erg1,@Erg2);
   NamePart:=StrPas(Erg1)+StrPas(Erg2);
   {$ELSE}
   FSplit(Name,Dummy,Erg1,Erg2);
   NamePart:=Erg1+Erg2;
   {$ENDIF}
END;

{***************************************************************************}
{ eine Gleitkommazahl in einen String umwandeln }

        FUNCTION FloatString(f:Extended):String;
CONST
   MaxLen=18;
VAR
   p,n,d:Integer;
   ExpVal:Integer;
   Erg:ValErgType;
   WithE:Boolean;
   s:String;
BEGIN

   { 1. mit MaximallÑnge wandeln, fÅhrendes Vorzeichen weg }

   Str(f,s); IF (s[1]=' ') OR (s[1]='+') THEN Delete(s,1,1);

   { 2. Exponenten soweit als mîglich kÅrzen, evtl. ganz streichen }

   p:=Pos('E',s);
   CASE s[p+1] OF
   '+':Delete(s,p+1,1);
   '-':Inc(p);
   END;

   WHILE (p<Length(s)) AND (s[p+1]='0') DO Delete(s,p+1,1);
   WithE:=p<Length(s);
   IF (NOT WithE) THEN Delete(s,Length(s),1);

   { 3. Nullen am Ende der Mantisse entfernen,Komma bleibt noch }

   IF (WithE) THEN p:=Pos('E',s) ELSE p:=Length(s)+1; Dec(p);
   WHILE (s[p]='0') DO
    BEGIN
     Delete(s,p,1); Dec(p);
    END;

   { 4. auf die gewÅnschte Maximalstellenzahl begrenzen }

   IF (WithE) THEN p:=Pos('E',s) ELSE p:=Length(s)+1;
   d:=Pos('.',s);
   n:=p-d-1;

   { 5. MaximallÑnge Åberschritten ? }

   IF (WithE) THEN p:=Pos('E',s) ELSE p:=Length(s)+1;
   IF Length(s)>MaxLen THEN
    BEGIN
     n:=p-(Length(s)-MaxLen);
     Delete(s,n,Length(s)-MaxLen);
    END;

   { 6. Exponentenwert berechnen }

   IF WithE THEN
    BEGIN
     p:=Pos('E',s);
     Val(Copy(s,p+1,Length(s)-p),ExpVal,Erg);
    END
   ELSE
    BEGIN
     p:=Length(s)+1;
     ExpVal:=0;
    END;

   { 7. soviel Platz, da· wir den Exponenten weglassen und evtl. Nullen
        anhÑngen kînnen ? }

   IF (ExpVal>0) THEN
    BEGIN
     d:=ExpVal-(p-Pos('.',s)-1); { = Nullenzahl }

     { 7a. nur Kommaverschiebung erforderlich. Exponenten lîschen und
           evtl. auch Komma }

     IF (d<=0) THEN
      BEGIN
       s:=Copy(s,1,p-1);
       n:=Pos('.',s); Delete(s,n,1);
       IF d<>0 THEN Insert('.',s,Length(s)+1+d);
      END

     { 7b. Es mÅssen Nullen angehÑngt werden. Schauen, ob nach Lîschen von
           Punkt und E-Teil genÅgend Platz ist }

     ELSE
      BEGIN
       n:=Length(s)-p+2+(MaxLen-Length(s));
       IF n>=d THEN
        BEGIN
         s:=Copy(s,1,p-1);
         Delete(s,Pos('.',s),1);
         FOR n:=1 TO d DO s:=s+'0';
        END
      END
    END

   { 8. soviel Platz, da· Exponent wegkann und die Zahl mit vielen Nullen
        vorne geschrieben werden kann ? }

   ELSE IF (ExpVal<0) THEN
    BEGIN
     n:=-ExpVal-(Length(s)-p+1); { = VerlÑngerung nach Operation }
     IF Length(s)+n<=MaxLen THEN
      BEGIN
       s:=Copy(s,1,p-1);
       Delete(s,Pos('.',s),1);
       IF (s[1]='-') THEN d:=2 ELSE d:=1;
       FOR n:=1 TO -ExpVal DO Insert('0',s,d);
       Insert('.',s,d+1);
      END;
    END;


   { 9. ÅberflÅssiges Komma entfernen }

   IF (WithE) THEN p:=Pos('E',s) ELSE p:=Length(s)+1;
   IF s[p-1]='.' THEN Delete(s,p-1,1);

   FloatString:=s;
END;

{****************************************************************************}
{ Symbol in String wandeln }

        FUNCTION StrSym(VAR t:TempResult; WithSystem:Boolean):String;
VAR
   s:String;
BEGIN
   CASE t.Typ OF
   TempInt:BEGIN
            s:=HexString(t.Int,1);
            IF NOT WithSystem THEN StrSym:=s
            ELSE CASE ConstMode OF
            ConstModeIntel:StrSym:=s+'H';
            ConstModeMoto:StrSym:='$'+s;
            ConstModeC:StrSym:='0x'+s;
            END;
           END;
   TempFloat:StrSym:=FloatString(t.Float);
   TempString:StrSym:=t.Ascii;
   ELSE StrSym:='???';
   END;
END;

{****************************************************************************}
{ ListingzÑhler zurÅcksetzen }

        PROCEDURE ResetPageCounter;
VAR
   z:Integer;
BEGIN
   FOR z:=0 TO ChapMax DO PageCounter[z]:=0;
   LstCounter:=0; ChapDepth:=0;
END;

{----------------------------------------------------------------------------}
{ eine neue Seite im Listing beginnen }

        PROCEDURE NewPage(Level:ShortInt; WithFF:Boolean);
VAR
   z:ShortInt;
   Header,s:String;
BEGIN
   IF ListOn=0 THEN Exit;

   LstCounter:=0;

   IF ChapDepth<Level THEN
    BEGIN
     Move(PageCounter[0],PageCounter[Level-ChapDepth],Succ(ChapDepth)*2);
     FOR z:=0 TO Level-ChapDepth DO PageCounter[z]:=1;
     ChapDepth:=Level;
    END;
   FOR z:=0 TO Pred(Level) DO PageCounter[z]:=1;
   Inc(PageCounter[Level]);

   {$i-}
   IF WithFF THEN
    BEGIN
     Write(LstFile,#12);
     ChkIO(10002);
    END;

   Header:=' AS V'+Version+HeadingFileNameLab+NamePart(SourceFile);
   IF (CurrFileName<>'INTERNAL') AND (NamePart(CurrFileName)<>NamePart(SourceFile)) THEN
    Header:=Header+'('+NamePart(CurrFileName)+')';
   Header:=Header+HeadingPageLab;

   FOR z:=ChapDepth DOWNTO 0 DO
    BEGIN
     Str(PageCounter[z],s); Header:=Header+s;
     IF z<>0 THEN Header:=Header+'.';
    END;

   Header:=Header+' - '+NLS_CurrDateString+' '+NLS_CurrTimeString(False);

   IF PageWidth<>0 THEN
    WHILE Length(Header)>PageWidth DO
     BEGIN
      WriteLn(LstFile,Copy(Header,1,PageWidth));
      ChkIO(10002); Delete(Header,1,PageWidth);
     END;
   WriteLn(LstFile,Header); ChkIO(10002);

   IF PrtTitleString<>'' THEN
    BEGIN
     WriteLn(LstFile,PrtTitleString);
     ChkIO(10002);
    END;

   WriteLn(LstFile,Char_CR,Char_LF);
   ChkIO(10002);
END;


{----------------------------------------------------------------------------}
{ eine Zeile ins Listing schieben }

        PROCEDURE WrLstLine(Line:String);
VAR
   LLength:Integer;
   bbuf:ARRAY[1..2500] OF Char;
   blen,hlen,z,Start:Integer;
BEGIN
   IF ListOn=0 THEN Exit;

   {$i-}

   IF PageLength=0 THEN
    BEGIN
     WriteLn(LstFile,Line); ChkIO(10002);
    END
   ELSE
    BEGIN
     IF (PageWidth=0) OR (Length(Line) SHL 3<PageWidth) THEN LLength:=1
     ELSE
      BEGIN
       blen:=0;
       FOR z:=1 TO Length(Line) DO
        IF Line[z]=#9 THEN
         BEGIN
          FillChar(bbuf[blen+1],8-(blen AND 7),Ord(' '));
          Inc(blen,8-(blen AND 7));
         END
        ELSE
         BEGIN
          Inc(blen); bbuf[blen]:=Line[z];
         END;
       LLength:=blen DIV PageWidth; IF blen MOD PageWidth<>0 THEN Inc(LLength);
      END;
     IF LLength=1 THEN
      BEGIN
       WriteLn(LstFile,Line); ChkIO(10002);
       Inc(LstCounter);
       IF LstCounter=PageLength THEN NewPage(0,True);
      END
     ELSE
      BEGIN
       Start:=1;
       FOR z:=1 TO LLength DO
        BEGIN
         hlen:=PageWidth; IF blen-Start+1<hlen THEN hlen:=blen-Start+1;
         Move(bbuf[Start],Line[1],hlen); Byte(Line[0]):=hlen;
         WriteLn(LstFile,Line); Inc(LstCounter);
         IF LstCounter=PageLength THEN NewPage(0,True);
         Inc(Start,hlen);
        END;
      END;
    END;

   {$i+}
END;

{****************************************************************************}
{ Ausdruck in Spalte vor Listing }


        PROCEDURE SetListLineVal(VAR t:TempResult);
BEGIN
   ListLine:='='+StrSym(t,True);
   IF Length(ListLine)>14 THEN ListLine:=Copy(ListLine,1,12)+'..';
END;

{****************************************************************************}
{ einen Symbolnamen auf GÅltigkeit ÅberprÅfen }

        FUNCTION ChkSymbName(sym:String):Boolean;
VAR
   z:Byte;
BEGIN
   ChkSymbName:=False;
   IF sym='' THEN Exit;
   IF NOT (sym[1] IN ['A'..'Z','a'..'z',#128..#165,'_','.']) THEN Exit;
   FOR z:=2 TO Length(sym) DO
    IF NOT (sym[z] IN ValidSymChars) THEN Exit;
   ChkSymbName:=True;
END;

        FUNCTION ChkMacSymbName(sym:String):Boolean;
VAR
   z:Integer;
BEGIN
   ChkMacSymbName:=False;
   IF sym='' THEN Exit;
   IF NOT (sym[1] IN ['A'..'Z','a'..'z']) THEN Exit;
   FOR z:=2 TO Length(sym) DO
    IF NOT (sym[z] IN ['A'..'Z','a'..'z','0'..'9']) THEN Exit;
   ChkMacSymbName:=True;
END;

{****************************************************************************}
{ Fehlerkanal offen ? }

        PROCEDURE ForceErrorOpen;
BEGIN
   IF NOT IsErrorOpen THEN
    BEGIN
     RewriteStandard(ErrorFile,ErrorName); IsErrorOpen:=True;
     ChkIO(10001);
    END;
END;

{----------------------------------------------------------------------------}
{ eine Fehlermeldung mit Klartext ausgeben }

        PROCEDURE EmergencyStop;
BEGIN
   {$i-}
   IF IsErrorOpen THEN Close(ErrorFile);
   Close(LstFile);
   IF ShareMode<>0 THEN
    BEGIN
     Close(ShareFile); Erase(ShareFile);
    END;
   IF MacProOutput THEN
    BEGIN
     Close(MacProFile); Erase(MacProFile);
    END;
   IF MacroOutput THEN
    BEGIN
     Close(MacroFile); Erase(MacroFile);
    END;
   IF MakeDebug THEN Close(Debug);
   IF CodeOutput THEN
    BEGIN
     Close(PrgFile); Erase(PrgFile);
    END;
   {$i+}
END;

        PROCEDURE WrErrorString(VAR Message,Add:String; Warning,Fatal:Boolean);
VAR
   h:String;
BEGIN
   h:=ErrorPos;
   IF NOT Warning THEN
    BEGIN
     h:=h+ErrName+Add+' : ';
     Inc(ErrorCount);
    END
   ELSE
    BEGIN
     h:=h+WarnName+Add+' : ';
     Inc(WarnCount);
    END;

   IF (LstName<>'NUL') AND (NOT Fatal) THEN
    BEGIN
     WrLstLine(h+Message);
     IF (ExtendErrors) AND (ExtendError<>'') THEN
      WrLstLine('> > > '+ExtendError);
    END;
   ForceErrorOpen;
   IF (LstName<>'') OR (Fatal) THEN
    BEGIN
     WriteLn(ErrorFile,h,Message,ClrEol);
     IF (ExtendErrors) AND (ExtendError<>'') THEN
      WriteLn(ErrorFile,'> > > ',ExtendError,ClrEol);
    END;
   ExtendError:='';

   IF Fatal THEN
    BEGIN
     WriteLn(ErrorFile,ErrMsgIsFatal);
     EmergencyStop;
     Halt(3);
    END;
END;


{----------------------------------------------------------------------------}
{ eine Fehlermeldung Åber Code ausgeben }

        PROCEDURE WrErrorNum(Num:Word);
VAR
   h:String;
   Add:String[10];
BEGIN
   IF (NOT CodeOutput) AND (Num=1200) THEN Exit;

   IF (SuppWarns) AND (Num<1000) THEN Exit;

   CASE Num OF
      0 : h:=ErrMsgUselessDisp;
     10 : h:=ErrMsgShortAddrPossible;
     20 : h:=ErrMsgShortJumpPossible;
     30 : h:=ErrMsgNoShareFile;
     40 : h:=ErrMsgBigDecFloat;
     50 : h:=ErrMsgPrivOrder;
     60 : h:=ErrMsgDistNull;
     70 : h:=ErrMsgWrongSegment;
     75 : h:=ErrMsgInAccSegment;
     80 : h:=ErrMsgPhaseErr;
     90 : h:=ErrMsgOverlap;
    100 : h:=ErrMsgNoCaseHit;
    110 : h:=ErrMsgInAccPage;
    120 : h:=ErrMsgRMustBeEven;
    130 : h:=ErrMsgObsolete;
    140 : h:=ErrMsgUnpredictable;
    150 : h:=ErrMsgAlphaNoSense;
    160 : h:=ErrMsgSenseless;
    170 : h:=ErrMsgRepassUnknown;
    180 : h:=ErrMsgAddrNotAligned;
    190 : h:=ErrMsgIOAddrNotAllowed;
    200 : h:=ErrMsgPipeline;
    210 : h:=ErrMsgDoubleAdrRegUse;
    220 : h:=ErrMsgNotBitAddressable;
    230 : h:=ErrMsgStackNotEmpty;
    240 : h:=ErrMsgNULCharacter;
    250 : h:=ErrMsgPageCrossing;
   1000 : h:=ErrMsgDoubleDef;
   1010 : h:=ErrMsgSymbolUndef;
   1020 : h:=ErrMsgInvSymName;
   1090 : h:=ErrMsgInvFormat;
   1100 : h:=ErrMsgUseLessAttr;
   1105 : h:=ErrMsgTooLongAttr;
   1107 : h:=ErrMsgUndefAttr;
   1110 : h:=ErrMsgWrongArgCnt;
   1115 : h:=ErrMsgWrongOptCnt;
   1120 : h:=ErrMsgOnlyImmAddr;
   1130 : h:=ErrMsgInvOpsize;
   1131 : h:=ErrMsgConfOpSizes;
   1132 : h:=ErrMsgUndefOpSizes;
   1135 : h:=ErrMsgInvOpType;
   1140 : h:=ErrMsgTooMuchArgs;
   1200 : h:=ErrMsgUnknownOpcode;
   1300 : h:=ErrMsgBrackErr;
   1310 : h:=ErrMsgDivByZero;
   1315 : h:=ErrMsgUnderRange;
   1320 : h:=ErrMsgOverRange;
   1325 : h:=ErrMsgNotAligned;
   1330 : h:=ErrMsgDistTooBig;
   1335 : h:=ErrMsgInAccReg;
   1340 : h:=ErrMsgNoShortAddr;
   1350 : h:=ErrMsgInvAddrMode;
   1351 : h:=ErrMsgMustBeEven;
   1355 : h:=ErrMsgInvParAddrMode;
   1360 : h:=ErrMsgUndefCond;
   1370 : h:=ErrMsgJmpDistTooBig;
   1375 : h:=ErrMsgDistIsOdd;
   1380 : h:=ErrMsgInvShiftArg;
   1390 : h:=ErrMsgRange18;
   1400 : h:=ErrMsgShiftCntTooBig;
   1410 : h:=ErrMsgInvRegList;
   1420 : h:=ErrMsgInvCmpMode;
   1430 : h:=ErrMsgInvCPUType;
   1440 : h:=ErrMsgInvCtrlReg;
   1445 : h:=ErrMsgInvReg;
   1450 : h:=ErrMsgNoSaveFrame;
   1460 : h:=ErrMsgNoRestoreFrame;
   1465 : h:=ErrMsgUnknownMacArg;
   1470 : h:=ErrMsgMissEndif;
   1480 : h:=ErrMsgInvIfConst;
   1483 : h:=ErrMsgDoubleSection;
   1484 : h:=ErrMsgInvSection;
   1485 : h:=ErrMsgMissingEndSect;
   1486 : h:=ErrMsgWrongEndSect;
   1487 : h:=ErrMsgNotInSection;
   1488 : h:=ErrMsgUndefdForward;
   1489 : h:=ErrMsgContForward;
   1490 : h:=ErrMsgInvFuncArgCnt;
   1495 : h:=ErrMsgMissingLTORG;
   1500 : h:=ErrMsgNotOnThisCPU1+MomCPUIdent+ErrMsgNotOnThisCPU2;
   1505 : h:=ErrMsgNotOnThisCPU3+MomCPUIdent+ErrMsgNotOnThisCPU2;
   1510 : h:=ErrMsgInvBitPos;
   1520 : h:=ErrMsgOnlyOnOff;
   1530 : h:=ErrMsgStackEmpty;
   1540 : h:=ErrMsgNotOneBit;
   1600 : h:=ErrMsgShortRead;
   1700 : h:=ErrMsgRomOffs063;
   1710 : h:=ErrMsgInvFCode;
   1720 : h:=ErrMsgInvFMask;
   1730 : h:=ErrMsgInvMMUReg;
   1740 : h:=ErrMsgLevel07;
   1750 : h:=ErrMsgInvBitMask;
   1760 : h:=ErrMsgInvRegPair;
   1800 : h:=ErrMsgOpenMacro;
   1805 : h:=ErrMsgEXITMOutsideMacro;
   1810 : h:=ErrMsgTooManyMacParams;
   1815 : h:=ErrMsgDoubleMacro;
   1820 : h:=ErrMsgFirstPassCalc;
   1830 : h:=ErrMsgTooManyNestedIfs;
   1840 : h:=ErrMsgMissingIf;
   1850 : h:=ErrMsgRekMacro;
   1860 : h:=ErrMsgUnknownFunc;
   1870 : h:=ErrMsgInvFuncArg;
   1880 : h:=ErrMsgFloatOverflow;
   1890 : h:=ErrMsgInvArgPair;
   1900 : h:=ErrMsgNotOnThisAddress;
   1905 : h:=ErrMsgNotFromThisAddress;
   1910 : h:=ErrMsgTargOnDiffPage;
   1920 : h:=ErrMsgCodeOverflow;
   1925 : h:=ErrMsgAdrOverflow;
   1930 : h:=ErrMsgMixDBDS;
   1940 : h:=ErrMsgOnlyInCode;
   1950 : h:=ErrMsgParNotPossible;
   1960 : h:=ErrMsgInvSegment;
   1961 : h:=ErrMsgUnknownSegment;
   1962 : h:=ErrMsgUnknownSegReg;
   1970 : h:=ErrMsgInvString;
   1980 : h:=ErrMsgInvRegName;
   1985 : h:=ErrMsgInvArg;
   1990 : h:=ErrMsgNoIndir;
   1995 : h:=ErrMsgNotInThisSegment;
   1996 : h:=ErrMsgNotInMaxmode;
   1997 : h:=ErrMsgOnlyInMaxmode;
   10001: h:=ErrMsgOpeningFile;
   10002: h:=ErrMsgListWrError;
   10003: h:=ErrMsgFileReadError;
   10004: h:=ErrMsgFileWriteError;
   10006: h:=ErrMsgHeapOvfl;
   10007: h:=ErrMsgStackOvfl;
   ELSE   h:=ErrMsgIntError;
   END;

   IF ((Num=1910) OR (Num=1370)) AND (NOT Repass) THEN Inc(JmpErrors);

   IF NumericErrors THEN
    BEGIN
     Str(Num,Add); Add:='#'+Add;
    END
   ELSE Add:='';
   WrErrorString(h,Add,Num<1000,Num>=10000);
END;

        PROCEDURE WrError(Num:Word);
BEGIN
   ExtendError:=''; WrErrorNum(Num);
END;

        PROCEDURE WrXError(Num:Word; Message:String);
BEGIN
   ExtendError:=Message; WrErrorNum(Num);
END;

{---------------------------------------------------------------------------}
{ I/O-Fehler }

        PROCEDURE ChkIO(ErrNo:Word);
VAR
   io:Integer;
   hs:String;
BEGIN
   io:=IoResult; IF io=0 THEN Exit;

   CASE io OF
     2 : hs:=IoErrFileNotFound;
     3 : hs:=IoErrPathNotFound;
     4 : hs:=IoErrTooManyOpenFiles;
     5 : hs:=IoErrAccessDenied;
     6 : hs:=IoErrInvHandle;
    12 : hs:=IoErrInvAccMode;
    15 : hs:=IoErrInvDriveLetter;
    16 : hs:=IoErrCannotRemoveActDir;
    17 : hs:=IoErrNoRenameAcrossDrives;
   100 : hs:=IoErrFileEnd;
   101 : hs:=IoErrDiskFull;
   102 : hs:=IoErrMissingAssign;
   103 : hs:=IoErrFileNotOpen;
   104 : hs:=IoErrNotOpenForRead;
   105 : hs:=IoErrNotOpenForWrite;
   106 : hs:=IoErrInvNumFormat;
   150 : hs:=IoErrWriteProtect;
   151 : hs:=IoErrUnknownDevice;
   152 : hs:=IoErrDrvNotReady;
   153 : hs:=IoErrUnknownDOSFunc;
   154 : hs:=IoErrCRCError;
   155 : hs:=IoErrInvDPB;
   156 : hs:=IoErrPositionErr;
   157 : hs:=IoErrInvSecFormat;
   158 : hs:=IoErrSectorNotFound;
   159 : hs:=IoErrPaperEnd;
   160 : hs:=IoErrDevReadError;
   161 : hs:=IoErrDevWriteError;
   162 : hs:=IoErrGenFailure;
   ELSE
    BEGIN
     Str(io,hs); hs:=IoErrUnknown+hs;
     ExtendError:=IoErrUnknown+ExtendError;
    END;
   END;

   WrXError(ErrNo,hs);
END;

{----------------------------------------------------------------------------}
{ Bereichsfehler }

        FUNCTION ChkRange(Value,Min,Max:LongInt):Boolean;
VAR
   s1,s2:String[15];
BEGIN
   ChkRange:=False;
   IF Value<Min THEN
    BEGIN
     Str(Value,s1); Str(Min,s2);
     WrXError(1315,s1+'<'+s2);
    END
   ELSE IF Value>Max THEN
    BEGIN
     Str(Value,s1); Str(Max,s2);
     WrXError(1320,s1+'>'+s2);
    END
   ELSE ChkRange:=True;
END;

{****************************************************************************}

        FUNCTION ProgCounter:LongInt;
BEGIN
   ProgCounter:=PCs[ActPC];
END;


{----------------------------------------------------------------------------}
{ aktuellen ProgrammzÑhler mit Phasenverschiebung holen }

        FUNCTION EProgCounter:LongInt;
BEGIN
   EProgCounter:=PCs[ActPC]+Phases[ActPC];
END;


{----------------------------------------------------------------------------}
{ GranularitÑt des aktuellen Segments holen }

        FUNCTION Granularity:Word;
BEGIN
   Granularity:=Grans[ActPC];
END;

{----------------------------------------------------------------------------}
{ Linstingbreite des aktuellen Segments holen }

        FUNCTION ListGran:Word;
BEGIN
   ListGran:=ListGrans[ActPC];
END;

{----------------------------------------------------------------------------}
{ prÅfen, ob alle Symbole einer Formel im korrekten Adre·raum lagen }

        PROCEDURE ChkSpace(Space:Byte);
BEGIN
   IF TypeFlag AND (NOT (1 SHL Space))<>0 THEN WrError(70);
END;

{****************************************************************************}
{ eine Chunkliste im Listing ausgeben & Speicher lîschen }

        PROCEDURE PrintChunk(NChunk:ChunkList);
VAR
   NewMin,FMin:LongInt;
   Found:Boolean;
   p,z:Word;
   BufferZ:Integer;
   BufferS:String;

        FUNCTION Len(VAR s:String):Byte;
BEGIN
   Len:=Length(s);
END;

BEGIN
   NewMin:=0; BufferZ:=0; BufferS:='';

   WITH NChunk DO
   REPEAT
    Found:=False; FMin:=$ffffffff;

    FOR z:=1 TO RealLen DO
     WITH Chunks^[z] DO
      IF IsUGreaterEq(Start,NewMin) THEN
       IF IsUGreater(FMin,Start) THEN
        BEGIN
         Found:=True; FMin:=Start; p:=z;
        END;

    IF Found THEN
    WITH Chunks^[p] DO
     BEGIN
      BufferS:=BufferS+HexString(Start,0);
      IF Length<>1 THEN BufferS:=BufferS+'-'+HexString(Start+Length-1,0);
      BufferS:=BufferS+Blanks(19-Len(BufferS) MOD 19);
      Inc(BufferZ);
      IF BufferZ=4 THEN
       BEGIN
        WrLstLine(BufferS); BufferS:=''; BufferZ:=0;
       END;
      NewMin:=Start+Length;
     END;
   UNTIL NOT Found;

   IF BufferZ<>0 THEN WrLstLine(BufferS);
END;

{----------------------------------------------------------------------------}
{ Listen ausgeben }

        PROCEDURE PrintUseList;
VAR
   z,z2:Integer;
   s:String;
BEGIN
   FOR z:=1 TO PCMax DO
    IF SegChunks[z].Chunks<>Nil THEN
     BEGIN
      WrLstLine('  '+ListSegListHead1+SegNames[z]+ListSegListHead2);
      s:='  ';
      FOR z2:=1 TO 3+Length(SegNames[z])+Length(ListSegListHead1)+Length(ListSegListHead2) DO
       s:=s+'-';
      WrLstLine(s);
      WrLstLine('');
      PrintChunk(SegChunks[z]);
      WrLstLine('');
     END;
END;

        PROCEDURE ClearUseList;
VAR
   z:Integer;
BEGIN
   FOR z:=1 TO PCMax DO
   WITH SegChunks[z] DO
    BEGIN
     FreeMem(Chunks,AllocLen*SizeOf(OneChunk));
     InitChunk(SegChunks[z]);
    END;
END;

{****************************************************************************}
{ Include-Pfadlistenverarbeitung }

        FUNCTION GetPath(VAR Acc:String):String;
VAR
   p:Integer;
BEGIN
   p:=QuotPos(Acc,';');
   GetPath:=Copy(Acc,1,p-1); Delete(Acc,1,p);
END;

        PROCEDURE AddIncludeList(NewPath:String);
VAR
   Test:String;
BEGIN
   UpString(NewPath);
   Test:=IncludeList;
   WHILE Test<>'' DO
    IF GetPath(Test)=NewPath THEN Exit;
   IF IncludeList<>'' THEN IncludeList:=';'+IncludeList;
   IncludeList:=NewPath+IncludeList;
END;


        PROCEDURE RemoveIncludeList(RemPath:String);
VAR
   Save,Part:String;
BEGIN
   UpString(RemPath);
   Save:=IncludeList; IncludeList:='';
   WHILE Save<>'' DO
    BEGIN
     Part:=GetPath(Save);
     IF Part<>RemPath THEN
      BEGIN
       IF IncludeList<>'' THEN IncludeList:=IncludeList+';';
       IncludeList:=IncludeList+Part;
      END;
    END;
END;

{****************************************************************************}
{ Liste mit Ausgabedateien }

        PROCEDURE ClearOutList;
BEGIN
   ClearStringList(OutList);
END;

        PROCEDURE AddToOutList(NewName:String);
BEGIN
   AddStringListLast(OutList,NewName);
END;

        PROCEDURE RemoveFromOutList(OldName:String);
BEGIN
   RemoveStringList(OutList,OldName);
END;

        FUNCTION GetFromOutList:String;
BEGIN
   GetFromOutList:=GetAndCutStringList(OutList);
END;

{****************************************************************************}
{ Tokenverarbeitung }

        PROCEDURE CompressLine(TokNam:String; Num:Byte; VAR Line:String);
VAR
   OneS:String[1];
   Part:String;
   z,e:Byte;
   SFound:Boolean;
BEGIN
   z:=1; OneS:=Chr(Num);
   WHILE z<=Length(Line)-Length(TokNam)+1 DO
    BEGIN
     e:=z+Length(TokNam);
     Part:=Copy(Line,z,Length(TokNam));
     IF CaseSensitive THEN SFound:=Part=TokNam
     ELSE SFound:=NLS_StrCaseCmp(Part,TokNam)=0;
     IF  (SFound)
     AND ((z=1) OR (NOT (Line[z-1] IN ['A'..'Z','a'..'z','0'..'9'])))
     AND ((e>Length(Line)) OR (NOT (Line[e] IN ['A'..'Z','a'..'z','0'..'9'])))
     THEN
      BEGIN
       Delete(Line,z,Length(TokNam));
       Insert(OneS,Line,z);
      END;
     Inc(z);
    END;
END;

       PROCEDURE ExpandLine(TokNam:String; Num:Byte; VAR Line:String);
VAR
   z:Byte;
BEGIN
    REPEAT
     z:=Pos(Chr(Num),Line);
     IF z<>0 THEN
      BEGIN
       Delete(Line,z,1); Insert(TokNam,Line,z);
      END;
    UNTIL z=0;
END;

        PROCEDURE KillCtrl(VAR Line:String);
VAR
   z:Byte;
BEGIN
   z:=1;
   REPEAT
    IF Line[z]=#9 THEN
     BEGIN
      Delete(Line,z,1);
      Insert(Blanks(8-((z-1) MOD 8)),Line,z);
     END
    ELSE IF Line[z]<' ' THEN Line[z]:=' ';
    Inc(z);
   UNTIL z>Length(Line);
END;

{****************************************************************************}
{ Differenz zwischen zwei Zeiten mit JahresÅberlauf berechnen }

        FUNCTION DTime(t1,t2:LongInt):LongInt;
VAR
   d:LongInt;
BEGIN
   d:=t2-t1; IF d<0 THEN d:=d+(24*360000);
   DTime:=Abs(d);
END;


{----------------------------------------------------------------------------}
{ Zeit DOS-kompatibel holen }

{$f+}
        FUNCTION GtimeDOS:LongInt;
VAR
   h,m,s,s100:DateTimeType;
BEGIN
   {$IFDEF OS2}
   GetTime(h,m,s,s100);
   {$ELSE}
   ASM
        mov     ah,2ch
        int     21h
        mov     ah,0            { zur Erweiterung auf 16 Bit }
        mov     al,dl           { Hundertstel }
        mov     word ptr[s100],ax
        mov     al,dh           { Sekunden }
        mov     word ptr[s],ax
        mov     al,cl           { Minuten }
        mov     word ptr[m],ax
        mov     al,ch           { Stunden }
        mov     word ptr[h],ax
   END;
   {$ENDIF}

   {$IFDEF SPEEDUP}
   ASM
        mov     si,word ptr[s100]       { Hunderstelsekunden addieren }
        sub     di,di
        mov     ax,100          { Sekunden*100 dazu }
        mul     word ptr[s]
        add     si,ax
        adc     di,dx
        mov     ax,6000         { Minuten*6000 dazu }
        mul     word ptr[m]
        add     si,ax
        adc     di,dx
        mov     ax,45000        { Stunden*360000 dazu }
        mul     word ptr[h]
        shl     ax,1            { zweiter Faktor=8 }
        rcl     dx,1
        shl     ax,1
        rcl     dx,1
        shl     ax,1
        rcl     dx,1
        add     ax,si           { Summe in DX:AX }
        adc     dx,di
        mov     word ptr @result,ax
        mov     word ptr @result+2,dx
   END;
   {$ELSE}
   GTimeDOS:=(h*360000)+(m*6000)+(s*100)+(s100);
   {$ENDIF}
END;

{----------------------------------------------------------------------------}
{ Zeit schnell aus Ticker holen }

{$IFNDEF OS2}
        FUNCTION GTimeFast:LongInt;
BEGIN
   GTimeFast:=Round(Meml[$40:$6c]*5.4854);
END;
{$ENDIF}

{****************************************************************************}
{ Heapfehler abfedern }

        FUNCTION MyHeapError(Size:Word):Integer;
        Far;
BEGIN
   IF Size<>0 THEN WrError(10006);
   MyHeapError:=1;
END;

{----------------------------------------------------------------------------}
{ Stackfehler abfangen }

        PROCEDURE ChkStack;
BEGIN
   IF SPtr<MinStack THEN WrError(10007);
   IF SPtr<LowStack THEN LowStack:=SPtr;
END;

        PROCEDURE ResetStack;
BEGIN
   LowStack:=SPtr;
END;

        FUNCTION StackRes:
{$IFDEF USE32}
        LongInt;
{$ELSE}
        Word;
{$ENDIF}
BEGIN
   StackRes:=LowStack-MinStack;
END;

        PROCEDURE _Init_AsmSub;
VAR
   z:Word;
   XORVal:LongInt;
BEGIN
   Magic:=StartMagic;
   FOR z:=1 TO Length(InfoMessCopyRight) DO
    BEGIN
     XORVal:=Ord(InfoMessCopyRight[z]);
     XORVal:=XORVal SHL ((z MOD 4)*8);
     Magic:=Magic XOR XORVal;
    END;
END;

{****************************************************************************}
{ Overlays installieren, PC-KompatibilitÑt }
{ XMS-Overlays von Wilbert van Leijen }

{$IFDEF MSDOS}
CONST
  OvrNoXMSDriver = -7;   { kein XMS Treiber installiert }
  OvrNoXMSMemory = -8;   { zuwenig XMS Speicher vorhanden }

        PROCEDURE OvrInitXMS; External;
{$L OVERXMS.OBJ }
{$ENDIF}

{$IFDEF DPMI}
        FUNCTION MemInitSwapFile(FileName: pChar; FileSize: LongInt): INTEGER;
        EXTERNAL 'RTM' INDEX 35;

        FUNCTION MemCloseSwapFile(Delete: INTEGER): INTEGER;
        EXTERNAL 'RTM' INDEX 36;
{$ENDIF}

VAR
   Cnt:Char;
   FileLen,NBuf,OldAvail:LongInt;
   z:Word;
   p,err:Integer;
   MemFlag,TempName:String;

BEGIN
   { FÅr DOS-Version Overlays initialisieren }

   {$IFDEF MSDOS}
   OvrInit('AS.OVR');
   OldAvail:=MemAvail;
   IF OvrResult<>0 THEN OvrInit(ParamStr(0));
   IF OvrResult<>0 THEN
    BEGIN
     WriteLn(StdErr,ErrMsgOvlyError); Halt(4);
    END;
   OvrSetBuf(OvrGetBuf+OvrGetBuf SHR 1);
   Dec(StartAvail,OldAvail-MemAvail);

   MemFlag:=GetEnv('USEXMS'); IF MemFlag='' THEN MemFlag:='Y';
   IF System.Upcase(MemFlag[1])='N'
    THEN OvrResult:=OvrNoXMSMemory
    ELSE OvrInitXMS;

   IF OvrResult<>0 THEN
    BEGIN
     MemFlag:=GetEnv('USEEMS'); IF MemFlag='' THEN MemFlag:='Y';
     IF System.Upcase(MemFlag[1])<>'N' THEN OvrInitEMS;
    END;
   {$ENDIF}

   { FÅr DPMI evtl. Swapfile anlegen }

{$IFDEF DPMI}
   MemFlag:=GetEnv('ASXSWAP');
   IF MemFlag<>'' THEN
    BEGIN
     p:=Pos(',',MemFlag);
     IF p=0 THEN TempName:='ASX.TMP'
     ELSE
      BEGIN
       TempName:=Copy(MemFlag,p+1,Length(MemFlag)-p);
       MemFlag:=Copy(MemFlag,1,p-1);
      END;
     KillBlanks(TempName); KillBlanks(MemFlag);
     TempName:=TempName+#0;
     Val(MemFlag,FileLen,Err);
     IF Err<>0 THEN
      BEGIN
       WriteLn(StdErr,ErrMsgInvSwapSize); Halt(4);
      END;
     IF MemInitSwapFile(@TempName[1],FileLen SHL 20)<>0 THEN
      BEGIN
       WriteLn(StdErr,ErrMsgSwapTooBig); Halt(4);
      END;
    END;
{$ENDIF}

   { Auf Ticker nur bei IBM XT/AT und Kompatiblen zugreifen:
     Das geht auch nur unter DOS... }

{$IFDEF MSDOS}
   IF Mem[$f000:$fffe] IN [$fe,$fc,$f8]   { XT/AT/PS2 }
    THEN
     GTime:=GTimeFast
    ELSE
{$ENDIF}
     GTime:=GTimeDOS;

   HeapError:=@MyHeapError;

   InitStringList(CopyrightList);
   InitStringList(OutList);

   StartStack:=SPtr; LowStack:=SPtr;
   {$IFDEF OS2}
    {$IFDEF VIRTUALPASCAL}
     MinStack:=StartStack-196608+$c00;
    {$ELSE}
     MinStack:=StartStack-15700+$c00;
    {$ENDIF}
   {$ELSE}
    {$IFDEF WINDOWS}
     MinStack:=StartStack-16384+$1000;
    {$ELSE}
     MinStack:=StartStack-49152+$800;
    {$ENDIF}
   {$ENDIF}
END.

