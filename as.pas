{$i STDINC.PAS}

{$IFDEF SEGATTRS}
 {$C Moveable PreLoad Permanent }
{$ENDIF}

{$IFDEF MSDOS}
 {$m 49152,0,655360}
{$ENDIF}

{$IFDEF DPMI}
 {$m 49152,0,655360}
{$ENDIF}

{$IFDEF WINDOWS}
 {$m 16384,1024}
{$ENDIF}

{$IFDEF OS2}
 {$IFDEF VIRTUALPASCAL}
  {$m 196608}
 {$ELSE}
  {$m 15700,0,655360}
 {$ENDIF}
{$ENDIF}

        PROGRAM AS;
        Uses {$IFDEF WINDOWS}
             WinDOS,WinCrt,Strings,
             {$ELSE}
             DOS,
             {$ENDIF}
             {$IFDEF MSDOS}
             Overlay,
             {$ENDIF}
             StdHandl,DecodeCm,NLS,StringUt,StringLi,Chunks,IncList,FileNums,
             AsmDef,AsmSub,AsmPars,AsmMac,AsmIF,ASMCode,AsmDeb,
             CodeAllg,CodePseu,
             Code68K,
             Code56K,
             Code601,
             Code68,Code6805,Code6809,Code6812,Code6816,
             CodeH8_3,CodeH8_5,Code7000,
             Code65,Code7700,
             Code4500,
             CodeM16,CodeM16C,
             Code48,Code51,Code96,Code85,Code86,
             Code8x30,
             CodeXA,
             CodeAVR,
             Code29k,
             Code166,
             CodeZ80,
             CodeZ8,
             Code96C1,Code90C1,Code87C8,Code47C0,Code97C2,
             Code16C5,Code16C8,Code17C4,
             CodeST62,CodeST7,CodeST9,Code6804,
             Code3201,Code3202,Code3203,Code3205,
             Code9900,CodeTMS7,Code370,
             CodeMSP,
             CodeSCMP,CodeCOP8,
             Code78C1,
             Code75K0,Code78K0{,
             Code21xx};

{$IFDEF MSDOS}
{$o CodePseu}
{$o Code68K}
{$o Code56K}
{$o Code601}
{$o Code68}
{$o Code6805}
{$o Code6809}
{$o Code6812}
{$o Code6816}
{$o CodeH8_3}
{$o CodeH8_5}
{$o Code7000}
{$o Code65}
{$o Code7700}
{$o Code4500}
{$o CodeM16}
{$o CodeM16C}
{$o Code48}
{$o Code51}
{$o Code96}
{$o Code85}
{$o Code86}
{$o Code8x30}
{$o CodeXA}
{$o CodeAVR}
{$o Code29K}
{$o Code166}
{$o CodeZ80}
{$o CodeZ8}
{$o Code96C1}
{$o Code90C1}
{$o Code87C8}
{$o Code47C0}
{$o Code97C2}
{$o Code16C5}
{$o Code16C8}
{$o Code17C4}
{$o CodeST62}
{$o CodeST7}
{$o CodeST9}
{$o Code6804}
{$o Code3201}
{$o Code3202}
{$o Code3203}
{$o Code3205}
{$o Code9900}
{$o CodeTMS7}
{$o Code370}
{$o CodeMSP}
{$o CodeSCMP}
{$o CodeCOP8}
{$o Code78C1}
{$o Code75K0}
{$o Code78K0}
{$ENDIF}

VAR
   ParCnt,i,k,LineZ:Integer;
   CPU:CPUVar;
   FileMask,Dummy:String;
   Search:SearchRec;
   MasterFile:Boolean;

   StartTime,StopTime:LongInt;

   ParUnProcessed:CMDProcessed;     { bearbeitete Kommandozeilenparameter }

   ErrFlag:Boolean;     { Fehler aufgetreten }


{$DEFINE MAIN}
{$i AS.RSC}

{==== Zeilen einlesen =======================================================}

        PROCEDURE NULL_Restorer(P:PInputTag);
        Far;
BEGIN
END;

        PROCEDURE GenerateProcessor(VAR P:PInputTag);
BEGIN
   New(P);
   WITH P^ DO
    BEGIN
     IsMacro:=False;
     Next:=Nil;
     First:=True;
     OrigPos:=ErrorPos;
     OrigDoLst:=DoLst;
     StartLine:=CurrLine;
     ParCnt:=0; ParZ:=0;
     InitStringList(Params);
     LineCnt:=0; LineZ:=1;
     Lines:=Nil;
     SpecName:='';
     IsEmpty:=False;
     Buffer:=Nil;
     Datei:=Nil;
     IfLevel:=SaveIFs;
     Restorer:=NULL_Restorer;
    END;
END;

{===========================================================================}
{ Listing erzeugen }

        PROCEDURE MakeList;
VAR
   h,h2:String;
   i,k:Byte;
   n:Word;
   EffLen:Word;

        PROCEDURE Gen2Line;
VAR
   z,Rest:Integer;
BEGIN
   Rest:=EffLen-n; IF Rest>8 THEN Rest:=8; IF DontPrint THEN Rest:=0;
   FOR z:=0 TO (Rest SHR 1)-1 DO
    BEGIN
     h:=h+HexString(WAsmCode[n SHR 1],4)+' '; Inc(n,2);
    END;
   IF Odd(Rest) THEN
    BEGIN
     h:=h+HexString(BAsmCode[n],2)+'   '; Inc(n);
    END;
   FOR z:=1 TO (8-Rest) SHR 1 DO
    h:=h+'     ';
END;

        PROCEDURE Gen4Line;
VAR
   z,Rest:Integer;
BEGIN
   Rest:=EffLen-n; IF Rest>8 THEN Rest:=8; IF DontPrint THEN Rest:=0;
   FOR z:=0 TO (Rest SHR 2)-1 DO
    BEGIN
     h:=h+HexString(DAsmCode[n SHR 2],8)+' '; Inc(n,4);
    END;
   IF (Rest AND 3<>0) THEN
    BEGIN
     FOR z:=0 TO Rest-1 DO
      BEGIN
       h:=h+HexString(BAsmCode[n],2); Inc(n);
      END;
     FOR z:=Rest TO 3 DO h:=h+'  ';
     h:=h+' ';
    END;
   FOR z:=1 TO (8-Rest) SHR 2 DO
    h:=h+'     ';
END;

BEGIN
   EffLen:=CodeLen*Granularity;

   IF (LstName<>'NUL') AND (DoLst) AND (ListMask AND 1<>0) AND (NOT IFListMask) THEN
    BEGIN
     { Zeilennummer / ProgrammzÑhleradresse: }

     IF IncDepth=0 THEN h2:='   '
     ELSE
      BEGIN
       Str(IncDepth,h2); h2:='('+h2+')';
      END;
     IF ListMask AND $10<>0 THEN
      BEGIN
       Str(CurrLine:5,h); h2:=h2+h+'/';
      END;
     h:=h2+HexBlankString(EProgCounter-CodeLen,8);
     IF Retracted THEN h:=h+' R ' ELSE h:=h+' : ';

     { Extrawurst in Listing ? }

     IF ListLine<>'' THEN
      BEGIN
       WrLstLine(h+ListLine+Blanks(20-Length(ListLine))+OneLine);
       ListLine:='';
      END

     { Code ausgeben }

     ELSE
      CASE ListGran OF
      4:BEGIN
         n:=0; Gen4Line;
         WrLstLine(h+OneLine);
         IF NOT DontPrint THEN
          WHILE n<EffLen DO
           BEGIN
            h:='                    ';
            Gen4Line;
            WrLstLine(h);
           END;
        END;
      2:BEGIN
         n:=0; Gen2Line;
         WrLstLine(h+OneLine);
         IF NOT DontPrint THEN
          WHILE n<EffLen DO
           BEGIN
            h:='                    ';
            Gen2Line;
            WrLstLine(h);
           END;
        END;
      ELSE
       BEGIN
        IF (TurnWords) AND (Granularity<>ListGran) THEN DreheCodes;
        FOR i:=0 TO 5 DO
         IF (NOT DontPrint) AND (EffLen > i )
          THEN h:=h+HexString(BAsmCode[i],2)+' '
          ELSE h:=h+'   ';
        WrLstLine(h+'  '+OneLine);
        IF (EffLen>6) AND (NOT DontPrint) THEN
         BEGIN
          Dec(EffLen,6);
          n:=EffLen DIV 6; IF EffLen MOD 6=0 THEN Dec(n);
          FOR i:=0 TO n DO
           BEGIN
            h:='                    ';
            FOR k:=0 TO 5 DO
            IF EffLen > i*6+k THEN h:=h+HexString(BAsmCode[i*6+k+6],2)+' ';
            WrLstLine(h);
           END;
         END;
        IF (TurnWords) AND (Granularity<>ListGran) THEN DreheCodes;
       END;
      END;

    END;
END;

{===========================================================================}
{ Makroprozessor }

{---------------------------------------------------------------------------}
{ allgemein gebrauchte Subfunktionen }

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ werden gebraucht, um festzustellen, ob innerhalb eines Makrorumpfes weitere
  Makroschachtelungen auftreten }

        FUNCTION MacroStart:Boolean;
BEGIN
   MacroStart:=Memo('MACRO') OR Memo('IRP') OR Memo('REPT') OR Memo('WHILE');
END;

        FUNCTION MacroEnd:Boolean;
BEGIN
   MacroEnd:=Memo('ENDM');
END;


{---------------------------------------------------------------------------}
{ Dieser Einleseprozessor dient nur dazu, eine fehlerhafte Makrodefinition
  bis zum Ende zu Åberlesen }

        PROCEDURE WaitENDM_Processor;
        Far;
VAR
   Tmp:POutputTag;
BEGIN
   WITH FirstOutputTag^ DO
    BEGIN
     IF MacroStart THEN Inc(NestLevel)
     ELSE IF MacroEnd THEN Dec(NestLevel);
     IF NestLevel<=-1 THEN
      BEGIN
       Tmp:=FirstOutputTag;
       FirstOutputTag:=Tmp^.Next;
       Dispose(Tmp);
      END;
    END;
END;

        PROCEDURE AddWaitENDM_Processor;
VAR
   Neu:POutputTag;
BEGIN
   New(Neu);
   WITH Neu^ DO
    BEGIN
     Processor:=WaitENDM_Processor;
     NestLevel:=0;
     Next:=FirstOutputTag;
    END;
   FirstOutputTag:=Neu;
END;

{---------------------------------------------------------------------------}
{ normale Makros }

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ Diese Routine leitet die Quellcodezeilen bei der Makrodefinition in den
  Makro-Record um }

	PROCEDURE Macro_OutProcessor;
        Far;
VAR
   Tmp:POutputTag;
   z:Integer;
   l:StringRecPtr;
   GMacro:PMacroRec;
   s:String;
BEGIN
   IF MacroOutput AND FirstOutputTag^.DoExport THEN
    BEGIN
     {$i-} WriteLn(MacroFile,OneLine); {$i+}
     ChkIO(10004);
    END;
   IF MacroStart THEN Inc(FirstOutputTag^.NestLevel)
   ELSE IF MacroEnd THEN Dec(FirstOutputTag^.NestLevel);
   IF FirstOutputTag^.NestLevel<>-1 THEN
    BEGIN
     s:=OneLine;
     KillCtrl(s);
     l:=FirstOutputTag^.Params;
     FOR z:=1 TO FirstOutputTag^.Mac^.ParamCount DO
      CompressLine(GetStringListNext(l),z,s);
     IF HasAttrs THEN CompressLine(AttrName,ParMax+1,s);
     AddStringListLast(FirstOutputTag^.Mac^.FirstLine,s);
    END;

   IF FirstOutputTag^.NestLevel=-1 THEN
    BEGIN
     IF IfAsm THEN
      BEGIN
       AddMacro(FirstOutputTag^.Mac,FirstOutputTag^.PubSect,True);
       IF FirstOutputTag^.DoGlobCopy AND (SectionStack<>Nil) THEN
        BEGIN
         New(GMacro);
         WITH GMacro^ DO
          BEGIN
           GetMem(Name,Length(FirstOutputTag^.GName)+1);
	   Name^:=FirstOutputTag^.GName;
           ParamCount:=FirstOutputTag^.Mac^.ParamCount;
           FirstLine:=DuplicateStringList(FirstOutputTag^.Mac^.FirstLine);
          END;
         AddMacro(GMacro,FirstOutputTag^.GlobSect,False);
        END;
      END
     ELSE ClearMacroRec(FirstOutputTag^.Mac);

     Tmp:=FirstOutputTag; FirstOutputTag:=Tmp^.Next;
     ClearStringList(Tmp^.Params); Dispose(Tmp);
    END;
END;

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ Hierher kommen bei einem Makroaufruf die expandierten Zeilen }

        FUNCTION MACRO_Processor(P:PInputTag; VAR erg:String):Boolean;
        Far;
VAR
   Lauf:StringRecPtr;
   z:Integer;
   hs:String[10];
BEGIN
   MACRO_Processor:=True;

   WITH P^ DO
    BEGIN
     Lauf:=Lines; FOR z:=1 TO LineZ-1 DO Lauf:=Lauf^.Next;
     erg:=Lauf^.Content^;
     Lauf:=Params;
     FOR z:=1 TO ParCnt DO
      BEGIN
       ExpandLine(Lauf^.Content^,z,erg);
       Lauf:=Lauf^.Next;
      END;
     IF HasAttrs THEN ExpandLine(SaveAttr,ParMax+1,erg);
     CurrLine:=StartLine;

     Str(LineZ,hs); ErrorPos:=OrigPos+' '+SpecName+'('+hs+')';

     IF LineZ=1 THEN PushLocHandle(GetLocHandle);

     Inc(LineZ);
     IF LineZ>LineCnt THEN MACRO_Processor:=False;
    END;
END;

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ Initialisierung des Makro-Einleseprozesses }

        PROCEDURE ReadMacro;
VAR
   PList:String;
   RunSection:PSaveSection;
   OneMacro,GMacro:PMacroRec;
   z1,z2:Integer;
   Neu:POutputTag;

   DoMacExp,DoGlobCopy,DoPublic:Boolean;
   HSect:LongInt;
   ErrFlag:Boolean;

        FUNCTION SearchSect(Test:String; Comp:String; VAR Erg:Boolean; VAR Section:LongInt):Boolean;
VAR
   p:Integer;
   Sect:String;
   Lauf:PSaveSection;
BEGIN
   KillBlanks(Test);
   p:=Pos(':',Test); IF p=0 THEN p:=Length(Test)+1;
   Sect:=Copy(Test,p+1,Length(Test)-p); Byte(Test[0]):=p-1;
   NLS_UpString(Test);
   IF (Length(Test)>2) AND (Copy(Test,1,2)='NO') AND (Copy(Test,3,Length(Test)-2)=Comp) THEN
    BEGIN
     Erg:=False; SearchSect:=True;
    END
   ELSE IF Test=Comp THEN
    BEGIN
     SearchSect:=IdentifySection(Sect,Section);
     Erg:=True;
    END
   ELSE SearchSect:=False;
END;

        FUNCTION SearchArg(Test:String; Comp:String; VAR Erg:Boolean):Boolean;
BEGIN
   NLS_UpString(Test);
   IF Test=Comp THEN
    BEGIN
     Erg:=True; SearchArg:=True;
    END
   ELSE IF (Length(Test)>2) AND (Copy(Test,1,2)='NO') AND (Copy(Test,3,Length(Test)-2)=Comp) THEN
    BEGIN
     Erg:=False; SearchArg:=True;
    END
   ELSE SearchArg:=False;
END;

BEGIN
   CodeLen:=0; ErrFlag:=False;

   { Makronamen prÅfen }
   { Definition nur im ersten Pass }

   IF PassNo<>1 THEN ErrFlag:=True
   ELSE IF NOT ExpandSymbol(LabPart) THEN ErrFlag:=True
   ELSE IF NOT ChkSymbName(LabPart) THEN
    BEGIN
     WrXError(1020,LabPart); ErrFlag:=True;
    END;

   New(Neu);
   Neu^.Processor:=Macro_OutProcessor;
   Neu^.NestLevel:=0;
   Neu^.Params:=Nil;
   Neu^.DoExport:=False;
   Neu^.DoGlobCopy:=False;
   Neu^.Next:=FirstOutputTag;

   { Argumente ÅberprÅfen }

   DoMacExp:=LstMacroEx; DoPublic:=False;
   PList:=''; z2:=0;
   FOR z1:=1 TO ArgCnt DO
    IF (ArgStr[z1][1]='{') AND (ArgStr[z1][Length(ArgStr[z1])]='}') THEN
     BEGIN
      ArgStr[z1]:=Copy(ArgStr[z1],2,Length(ArgStr[z1])-2);
      IF SearchArg(ArgStr[z1],'EXPORT',Neu^.DoExport) THEN
      ELSE IF SearchArg(ArgStr[z1],'EXPAND',DoMacExp) THEN PList:=PList+','+ArgStr[z1]
      ELSE IF SearchSect(ArgStr[z1],'GLOBAL',Neu^.DoGlobCopy,Neu^.GlobSect) THEN
      ELSE IF SearchSect(ArgStr[z1],'PUBLIC',DoPublic,Neu^.PubSect) THEN
      ELSE
       BEGIN
        WrXError(1465,ArgStr[z1]); ErrFlag:=True;
       END;
     END
    ELSE
     BEGIN
      PList:=PList+','+ArgStr[z1]; Inc(z2);
      IF NOT ChkMacSymbName(ArgStr[z1]) THEN
       BEGIN
        WrXError(1020,ArgStr[z1]); ErrFlag:=True;
       END;
      AddStringListLast(Neu^.Params,ArgStr[z1]);
     END;

   { Abbruch bei Fehler }

   IF ErrFlag THEN
    BEGIN
     ClearStringList(Neu^.Params);
     Dispose(Neu);
     AddWaitENDM_Processor;
     Exit;
    END;

   { Bei Globalisierung Namen des Extramakros ermitteln }

   IF Neu^.DoGlobCopy THEN
    BEGIN
     Neu^.GName:=LabPart;
     RunSection:=SectionStack; HSect:=MomSectionHandle;
     WHILE (HSect<>Neu^.GlobSect) AND (RunSection<>Nil) DO
      BEGIN
       Neu^.GName:=GetSectionName(HSect)+'_'+Neu^.GName;
       HSect:=RunSection^.Handle; RunSection:=RunSection^.Next;
      END;
    END;
   IF NOT DoPublic THEN Neu^.PubSect:=MomSectionHandle;

   New(OneMacro); Neu^.Mac:=OneMacro;
   WITH OneMacro^ DO
    BEGIN
     IF MacroOutput AND Neu^.DoExport THEN
      BEGIN
       IF Length(PList)<>0 THEN Delete(PList,1,1);
       {$i-}
       IF Neu^.DoGlobCopy THEN WriteLn(MacroFile,Neu^.GName,' MACRO ',PList)
       ELSE WriteLn(MacroFile,LabPart,' MACRO ',PList);
       {$i+}
       ChkIO(10004);
      END;

     Used:=False;
     GetMem(Name,Length(LabPart)+1); Name^:=LabPart;
     ParamCount:=z2;
     FirstLine:=Nil; LocMacExp:=DoMacExp;
    END;

   FirstOutputTag:=Neu;
END;

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ Beendigung der Expansion eines Makros }

        PROCEDURE MACRO_Cleanup(P:PInputTag);
        Far;
BEGIN
   WITH P^ DO
    ClearStringList(Params);
END;

        PROCEDURE MACRO_Restorer(P:PInputTag);
        Far;
BEGIN
   WITH P^ DO
    BEGIN
     PopLocHandle;
     ErrorPos:=OrigPos;
     DoLst:=OrigDoLst;
    END;
END;

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ Dies initialisiert eine Makroexpansion }

        PROCEDURE ExpandMacro(OneMacro:PMacroRec);
VAR
   z1:Integer;
   OneRec,Lauf:StringRecPtr;
   Tag:PInputTag;
BEGIN
   CodeLen:=0;
   WITH OneMacro^ DO

   {IF Used THEN WrError(1850)
   ELSE }

    BEGIN
     Used:=True;

     { 1. Tag erzeugen }

     GenerateProcessor(Tag);
     Tag^.Processor:= MACRO_Processor;
     Tag^.Restorer := MACRO_Restorer;
     Tag^.Cleanup  := MACRO_Cleanup;
     Tag^.SpecName := OneMacro^.Name^;
     Tag^.SaveAttr := AttrPart;
     Tag^.IsMacro  := True;

     { 2. Parameterzahl anpassen }

     IF ArgCnt<ParamCount THEN
      BEGIN
       FOR z1:=ArgCnt+1 TO ParamCount DO ArgStr[z1]:='';
      END;
     ArgCnt:=ParamCount;

     { 3. Parameterliste aufbauen - umgekehrt einfacher }

     FOR z1:=ArgCnt DOWNTO 1 DO
      BEGIN
       IF NOT CaseSensitive THEN UpString(ArgStr[z1]);
       AddStringListFirst(Tag^.Params,ArgStr[z1]);
      END;
     Tag^.ParCnt:=ArgCnt;

     { 4. Zeilenliste anhÑngen }

     Tag^.Lines:=FirstLine; Tag^.IsEmpty:=FirstLine=Nil;
     Lauf:=FirstLine;
     WHILE Lauf<>Nil DO
      BEGIN
       Inc(Tag^.LineCnt); Lauf:=Lauf^.Next;
      END;
    END;

   { 5. anhÑngen }

   IF IfAsm THEN
    BEGIN
     NextDoLst:=DoLst AND OneMacro^.LocMacExp;
     Tag^.Next:=FirstInputTag; FirstInputTag:=Tag;
    END
   ELSE
    BEGIN
     ClearStringList(Tag^.Params); Dispose(Tag);
    END;
END;

{---------------------------------------------------------------------------}
{ vorzeitiger Abbruch eines Makros }

        PROCEDURE ExpandEXITM;
BEGIN
   IF ArgCnt<>0 THEN WrError(1110)
   ELSE IF (FirstInputTag=Nil) THEN WrError(1805)
   ELSE IF (NOT FirstInputTag^.IsMacro) THEN WrError(1805)
   ELSE IF IfAsm THEN
    BEGIN
     FirstInputTag^.Cleanup(FirstInputTag);
     RestoreIfs(FirstInputTag^.IfLevel);
     FirstInputTag^.IsEmpty:=True;
    END;
END;

{---------------------------------------------------------------------------}
{---- IRP (was das bei MASM auch immer hei·en mag...)
      Ach ja: Individual Repeat! Danke Bernhard, jetzt hab'
      ich's gerafft! ------------------------}

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ AufrÑumroutine }

        PROCEDURE IRP_Cleanup(P:PInputTag);
        Far;
BEGIN
   WITH P^ DO
    BEGIN
     ClearStringList(Lines); ClearStringList(Params);
    END;
END;

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ Diese Routine liefert bei der Expansion eines IRP-Statements die expan-
  dierten Zeilen }

        FUNCTION IRP_Processor(P:PInputTag; VAR erg:String):Boolean;
        Far;
VAR
   Lauf:StringRecPtr;
   z:Integer;
   hs:String[10];
BEGIN
   IRP_Processor:=True;

   WITH P^ DO
    BEGIN
     Lauf:=Lines; FOR z:=1 TO LineZ-1 DO Lauf:=Lauf^.Next;
     erg:=Lauf^.Content^;
     Lauf:=Params; FOR z:=1 TO ParZ-1 DO Lauf:=Lauf^.Next;
     ExpandLine(Lauf^.Content^,1,erg); CurrLine:=StartLine+LineZ;

     Str(LineZ,hs); ErrorPos:=OrigPos+' IRP:'+Lauf^.Content^+'/'+hs;

     IF LineZ=1 THEN
      BEGIN
       IF NOT First THEN PopLocHandle; First:=False;
       PushLocHandle(GetLocHandle);
      END;


     Inc(LineZ);
     IF LineZ>LineCnt THEN
      BEGIN
       LineZ:=1; Inc(ParZ);
       IF ParZ>ParCnt THEN IRP_Processor:=False;
      END;
    END;
END;

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ Diese Routine sammelt wÑhrend der Definition eines IRP-Statements die
  Quellzeilen ein }

	PROCEDURE IRP_OutProcessor;
        Far;
VAR
   Tmp:POutputTag;
   Dummy:StringRecPtr;
   s:String;
BEGIN
   WITH FirstOutputTag^ DO
    BEGIN

     { Schachtelungen mitzÑhlen }

     IF MacroStart THEN Inc(NestLevel)
     ELSE IF MacroEnd THEN Dec(NestLevel);

     { falls noch nicht zuende, weiterzÑhlen }

     IF NestLevel>-1 THEN
      BEGIN
       s:=OneLine; KillCtrl(s);
       CompressLine(GetStringListFirst(Params,Dummy),1,s);
       AddStringListLast(Tag^.Lines,s);
       Inc(Tag^.LineCnt);
      END;

     { alles zusammen? Dann umhÑngen }

     IF NestLevel=-1 THEN
      BEGIN
       Tmp:=FirstOutputTag; FirstOutputTag:=FirstOutputTag^.Next;
       Tag^.IsEmpty:=Tag^.Lines=Nil;
       IF IfAsm THEN
        BEGIN
         NextDoLst:=DoLst AND LstMacroEx;
         Tag^.Next:=FirstInputTag; FirstInputTag:=Tag;
        END
       ELSE
        BEGIN
         ClearStringList(Tag^.Lines); ClearStringList(Tag^.Params);
         Dispose(Tag);
        END;
       ClearStringList(Tmp^.Params);
       Dispose(Tmp);
      END;
    END;
END;

{- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -}
{ Initialisierung der IRP-Bearbeitung }

       PROCEDURE ExpandIRP;
VAR
   ValOK:Boolean;
   Parameter:String;
   z1:Integer;
   Tag:PInputTag;
   Neu:POutputTag;
   NestLevel:Integer;
   ErrFlag:Boolean;
BEGIN
   { 1.Parameter prÅfen }

   IF ArgCnt<2 THEN
    BEGIN
     WrError(1110); ErrFlag:=True;
    END
   ELSE
    BEGIN
     Parameter:=ArgStr[1];
     IF NOT ChkMacSymbName(ArgStr[1]) THEN
      BEGIN
       WrXError(1020,Parameter); ErrFlag:=True;
      END
     ELSE ErrFlag:=False;
    END;
   IF ErrFlag THEN
    BEGIN
     AddWaitENDM_Processor;
     Exit;
    END;

   { 2. Tag erzeugen }

   GenerateProcessor(Tag);
   Tag^.ParCnt:=ArgCnt-1;
   Tag^.Processor:=IRP_Processor;
   Tag^.Restorer :=MACRO_Restorer;
   Tag^.Cleanup  :=IRP_Cleanup;
   Tag^.ParZ     :=1;
   Tag^.IsMacro  :=True;

   { 3. Parameterliste aufbauen; rÅckwÑrts einen Tucken schneller }

   FOR z1:=ArgCnt DOWNTO 2 DO
    BEGIN
     IF NOT CaseSensitive THEN UpString(ArgStr[z1]);
     AddStringListFirst(Tag^.Params,ArgStr[z1]);
    END;

   { 4. einbetten }

   New(Neu);
   Neu^.Next:=FirstOutputTag;
   Neu^.Processor:=IRP_OutProcessor;
   Neu^.NestLevel:=0;
   Neu^.Tag:=Tag;
   Neu^.Params:=Nil; AddStringListFirst(Neu^.Params,ArgStr[1]);
   FirstOutputTag:=Neu;
END;

{---- Repetition ------------------------------------------------------------}

        PROCEDURE REPT_Cleanup(P:PInputTag);
        Far;
BEGIN
   WITH P^ DO
    ClearStringList(Lines);
END;

        FUNCTION REPT_Processor(P:PInputTag; VAR erg:String):Boolean;
        Far;
VAR
   Lauf:StringRecPtr;
   z:Integer;
   hs:String[10];
BEGIN
   REPT_Processor:=True;

   WITH P^ DO
    BEGIN
     Lauf:=Lines; FOR z:=1 TO LineZ-1 DO Lauf:=Lauf^.Next;
     erg:=Lauf^.Content^; CurrLine:=StartLine+LineZ;

     Str(ParZ,hs);  ErrorPos:= OrigPos+ ' REPT '+hs+'/';
     Str(LineZ,hs); ErrorPos:=ErrorPos+hs;

     IF LineZ=1 THEN
      BEGIN
       IF NOT First THEN PopLocHandle; First:=False;
       PushLocHandle(GetLocHandle);
      END;

     Inc(LineZ);
     IF LineZ>LineCnt THEN
      BEGIN
       LineZ:=1; Inc(ParZ);
       IF ParZ>ParCnt THEN REPT_Processor:=False;
      END;
    END;
END;

	PROCEDURE REPT_OutProcessor;
        Far;
VAR
   Tmp:POutputTag;
BEGIN
   WITH FirstOutputTag^ DO
    BEGIN

     { Schachtelungen mitzÑhlen }

     IF MacroStart THEN Inc(NestLevel)
     ELSE IF MacroEnd THEN Dec(NestLevel);

     { falls noch nicht zuende, weiterzÑhlen }

     IF NestLevel>-1 THEN
      BEGIN
       AddStringListLast(Tag^.Lines,OneLine);
       Inc(Tag^.LineCnt);
      END;

     { alles zusammen? Dann umhÑngen }

     IF NestLevel=-1 THEN
      BEGIN
       Tmp:=FirstOutputTag; FirstOutputTag:=FirstOutputTag^.Next;
       Tag^.IsEmpty:=Tag^.Lines=Nil;
       IF (IfAsm) AND (Tag^.ParCnt>0) THEN
        BEGIN
         NextDoLst:=DoLst AND LstMacroEx;
         Tag^.Next:=FirstInputTag; FirstInputTag:=Tag;
        END
       ELSE
        BEGIN
         ClearStringList(Tag^.Lines);
         Dispose(Tag);
        END;
       Dispose(Tmp);
      END;
    END;
END;

        PROCEDURE ExpandREPT;
VAR
   ValOK:Boolean;
   ReptCount:LongInt;
   Tag:PInputTag;
   Neu:POutputTag;
   NestLevel:Integer;
   ErrFlag:Boolean;
BEGIN
   { 1.Repetitionszahl ermitteln }

   IF ArgCnt<>1 THEN
    BEGIN
     WrError(1110); ErrFlag:=True;
    END
   ELSE
    BEGIN
     FirstPassUnknown:=False;
     ReptCount:=EvalIntExpression(ArgStr[1],Int32,ValOK);
     IF FirstPassUnknown THEN WrError(1820);
     ErrFlag:=(NOT ValOK) OR (FirstPassUnknown);
    END;
   IF ErrFlag THEN
    BEGIN
     AddWaitENDM_Processor;
     Exit;
    END;

   { 2. Tag erzeugen }

   GenerateProcessor(Tag);
   Tag^.ParCnt:=ReptCount;
   Tag^.Processor:=REPT_Processor;
   Tag^.Restorer :=MACRO_Restorer;
   Tag^.Cleanup  :=REPT_Cleanup;
   Tag^.IsMacro  :=True;
   Tag^.ParZ:=1;

   { 3. einbetten }

   New(Neu);
   Neu^.Processor:=REPT_OutProcessor;
   Neu^.NestLevel:=0;
   Neu^.Next:=FirstOutputTag;
   Neu^.Tag:=Tag;
   FirstOutputTag:=Neu;
END;

{- bedingte Wiederholung ----------------------------------------------------}

        PROCEDURE WHILE_Cleanup(P:PInputTag);
        Far;
BEGIN
   WITH P^ DO
    ClearStringList(Lines);
END;

        FUNCTION WHILE_Processor(P:PInputTag; VAR erg:String):Boolean;
        Far;
VAR
   Lauf:StringRecPtr;
   z:Integer;
   OK:Boolean;
   hs:String[10];
BEGIN

   WITH P^ DO
    BEGIN
     Str(ParZ,hs);  ErrorPos:= OrigPos+ ' WHILE '+hs+'/';
     Str(LineZ,hs); ErrorPos:=ErrorPos+hs;
     CurrLine:=StartLine+LineZ;

     IF LineZ=1 THEN
      BEGIN
       IF NOT First THEN PopLocHandle; First:=False;
       PushLocHandle(GetLocHandle);
      END
     ELSE OK:=True;

     Lauf:=Lines; FOR z:=1 TO LineZ-1 DO Lauf:=Lauf^.Next;
     erg:=Lauf^.Content^;

     Inc(LineZ);
     IF LineZ>LineCnt THEN
      BEGIN
       LineZ:=1; Inc(ParZ);
       z:=EvalIntExpression(SpecName,Int32,OK);
       OK:=OK AND (z<>0);
       WHILE_Processor:=OK;
      END
     ELSE WHILE_Processor:=True;
    END;
END;

	PROCEDURE WHILE_OutProcessor;
        Far;
VAR
   Tmp:POutputTag;
   OK:Boolean;
   Erg:LongInt;
BEGIN
   WITH FirstOutputTag^ DO
    BEGIN

     { Schachtelungen mitzÑhlen }

     IF MacroStart THEN Inc(NestLevel)
     ELSE IF MacroEnd THEN Dec(NestLevel);

     { falls noch nicht zuende, weiterzÑhlen }

     IF NestLevel>-1 THEN
      BEGIN
       AddStringListLast(Tag^.Lines,OneLine);
       Inc(Tag^.LineCnt);
      END;

     { alles zusammen? Dann umhÑngen }

     IF NestLevel=-1 THEN
      BEGIN
       Tmp:=FirstOutputTag; FirstOutputTag:=FirstOutputTag^.Next;
       Tag^.IsEmpty:=Tag^.Lines=Nil;
       FirstPassUnknown:=False;
       Erg:=EvalIntExpression(Tag^.SpecName,Int32,OK);
       IF FirstPassUnknown THEN WrError(1820);
       OK:=OK AND (NOT FirstPassUnknown) AND (Erg<>0);
       IF (IfAsm) AND (OK) THEN
        BEGIN
         NextDoLst:=DoLst AND LstMacroEx;
         Tag^.Next:=FirstInputTag; FirstInputTag:=Tag;
        END
       ELSE
        BEGIN
         ClearStringList(Tag^.Lines);
         Dispose(Tag);
        END;
       Dispose(Tmp);
      END;
    END;
END;

        PROCEDURE ExpandWHILE;
VAR
   ValOK:Boolean;
   Tag:PInputTag;
   Neu:POutputTag;
   NestLevel:Integer;
   ErrFlag:Boolean;
BEGIN
   { 1.Bedingung ermitteln }

   IF ArgCnt<>1 THEN
    BEGIN
     WrError(1110); ErrFlag:=True;
    END
   ELSE ErrFlag:=False;
   IF ErrFlag THEN
    BEGIN
     AddWaitENDM_Processor;
     Exit;
    END;

   { 2. Tag erzeugen }

   GenerateProcessor(Tag);
   Tag^.Processor:=WHILE_Processor;
   Tag^.Restorer :=MACRO_Restorer;
   Tag^.Cleanup  :=WHILE_Cleanup;
   Tag^.IsMacro  :=True;
   Tag^.ParZ     :=1;
   Tag^.SpecName :=ArgStr[1];

   { 3. einbetten }

   New(Neu);
   Neu^.Processor:=WHILE_OutProcessor;
   Neu^.NestLevel:=0;
   Neu^.Next:=FirstOutputTag;
   Neu^.Tag:=Tag;
   FirstOutputTag:=Neu;
END;

{----------------------------------------------------------------------------}
{ Einziehen von Include-Files }

        PROCEDURE INCLUDE_Cleanup(P:PInputTag);
        Far;
BEGIN
   WITH P^ DO
    BEGIN
     Close(Datei^);
     Dispose(Datei);
     Dispose(Buffer);
     Inc(LineSum,MomLineCounter);
     IF (LstName<>'') AND (NOT QuietMode) THEN
      WriteLn(NamePart(CurrFileName),'(',MomLineCounter,')',ClrEol);
     IF MakeIncludeList THEN PopInclude;
    END;
END;

        FUNCTION INCLUDE_Processor(P:PInputTag; VAR erg:String):Boolean;
        Far;
VAR
   hz:String[10];
BEGIN
   INCLUDE_Processor:=True;

   WITH P^ DO
    BEGIN
     IF EOF(Datei^) THEN Erg:=''
     ELSE
      BEGIN
       {$i-} ReadLn(Datei^,erg); {$i+}
       ChkIO(10003);
      END;
     Inc(MomLineCounter); CurrLine:=MomLineCounter;
     Str(CurrLine,hz);
{     ErrorPos:=OrigPos; IF ErrorPos<>'' THEN ErrorPos:=ErrorPos+' ';}
     ErrorPos:={ErrorPos+}NamePart(CurrFileName)+'('+hz+')';
     IF EOF(Datei^) THEN INCLUDE_Processor:=False;
    END;
END;

        PROCEDURE INCLUDE_Restorer(P:PInputTag);
        Far;
BEGIN
   WITH P^ DO
    BEGIN
     MomLineCounter:=StartLine;
     CurrFileName:=SpecName;
     ErrorPos:=OrigPos;
    END;
   Dec(IncDepth);
END;

        PROCEDURE ExpandINCLUDE(SearchPath:Boolean);
VAR
   Tag:PInputTag;
   Hilf:Text;
BEGIN
   IF NOT IfAsm THEN Exit;

   IF ArgCnt<>1 THEN
    BEGIN
     WrError(1110); Exit;
    END;

   ArgPart:=ArgStr[1];
   IF ArgPart[1]='"' THEN Delete(ArgPart,1,1);
   IF ArgPart[Length(ArgPart)]='"' THEN Dec(Byte(ArgPart[0]));
   UpString(ArgPart); AddSuffix(ArgPart,IncSuffix); ArgStr[1]:=ArgPart;
   IF SearchPath THEN
    BEGIN
     ArgPart:=FExpand(FSearch(ArgPart,IncludeList));
     IF ArgPart[Length(ArgPart)]='\' THEN ArgPart:=ArgPart+ArgStr[1];
    END;

   { Tag erzeugen }

   GenerateProcessor(Tag);
   Tag^.Processor:=INCLUDE_Processor;
   Tag^.Restorer :=INCLUDE_Restorer;
   Tag^.Cleanup  :=INCLUDE_Cleanup;
   New(Tag^.Buffer);
   New(Tag^.Datei);

   { Sicherung alter Daten }

   WITH Tag^ DO
    BEGIN
     StartLine:=MomLineCounter; SpecName:=CurrFileName;
    END;

   { Datei îffnen. Frage mich keiner, wieso BPOS2 keine Textvariablen
     auf dem Heap erîffnen kann...O.K., Bug in RTL gefunden :-) }

   {$i-}
   Assign(Tag^.Datei^,ArgPart); SetTextBuf(Tag^.Datei^,Tag^.Buffer^);
   SetFileMode(0); Reset(Tag^.Datei^); ChkIO(10001);
   {$i+}

   { neu besetzen }

   CurrFileName:=ArgPart; MomLineCounter:=0;
   Inc(NextIncDepth); AddFile(ArgPart);
   PushInclude(ArgPart);

   { einhÑngen }

   Tag^.Next:=FirstInputTag; FirstInputTag:=Tag;
END;

{===========================================================================}
{ Einlieferung von Zeilen }

        PROCEDURE GetNextLine(VAR Line:String);
VAR
   HTag:PInputTag;
BEGIN
   WHILE (FirstInputTag<>Nil) AND (FirstInputTag^.IsEmpty) DO
    WITH FirstInputTag^ DO
     BEGIN
      Restorer(FirstInputTag);
      HTag:=FirstInputTag; FirstInputTag:=HTag^.Next;
      Dispose(HTag);
     END;

   IF FirstInputTag=Nil THEN
    BEGIN
     Line:=''; Exit;
    END;

   IF NOT FirstInputTag^.Processor(FirstInputTag,Line) THEN
    BEGIN
     FirstInputTag^.IsEmpty:=True;
     FirstInputTag^.Cleanup(FirstInputTag);
    END;

   Inc(MacLineSum);
END;

        FUNCTION InputEnd:Boolean;
VAR
   Lauf:PInputTag;
BEGIN
   InputEnd:=True;
   Lauf:=FirstInputTag;
   WHILE Lauf<>Nil DO
    BEGIN
     IF NOT Lauf^.IsEmpty THEN
      BEGIN
       InputEnd:=False; Exit;
      END;
     Lauf:=Lauf^.Next;
    END;
END;


{==== Eine Quelldatei ( Haupt-oder Includedatei ) bearbeiten ================}

{---- aus der zerlegten Zeile Code erzeugen ---------------------------------}

        FUNCTION HasLabel:Boolean;
BEGIN
   HasLabel:=(LabPart<>'')
         AND ((NOT Memo('SET')) OR (SETIsOccupied))
         AND ((NOT Memo('EVAL')) OR (NOT SETIsOccupied))
         AND (NOT Memo('EQU'))
         AND (NOT Memo('='))
         AND (NOT Memo(':='))
         AND (NOT Memo('MACRO'))
         AND (NOT Memo('FUNCTION'))
         AND (NOT Memo('LABEL'))
         AND (NOT IsDef);
END;

       PROCEDURE Produce_Code;
VAR
   z:Byte;
   OneMacro:PMacroRec;
   SearchMacros:Boolean;

BEGIN
   { Makrosuche unterdrÅcken ? }

   IF OpPart[1]='!' THEN
    BEGIN
     SearchMacros:=False; Delete(OpPart,1,1);
    END
   ELSE
    BEGIN
     SearchMacros:=True; IF ExpandSymbol(OpPart) THEN;
    END;
   LOpPart:=OpPart; NLS_UpString(OpPart);

   { Prozessor eingehÑngt ? }

   IF FirstOutputTag<>Nil THEN
    BEGIN
     FirstOutputTag^.Processor; Exit;
    END;

   { ansonsten Code erzeugen }

   { evtl. voranstehendes Label ablegen }

   IF IfAsm THEN
    IF HasLabel THEN EnterIntSymbol(LabPart,EProgCounter,ActPC,False);

   { Makroliste ? }

   IF Memo('IRP') THEN
    BEGIN
     ExpandIRP;
    END

   { Repetition ? }

   ELSE IF Memo('REPT') THEN
    BEGIN
     ExpandREPT;
    END

   { bedingte Repetition ? }

   ELSE IF Memo('WHILE') THEN
    BEGIN
     ExpandWHILE;
    END

   { bedingte Assemblierung ? }

   ELSE IF CodeIFs THEN

   { Makrodefinition ? }

   ELSE IF Memo('MACRO') THEN
    BEGIN
     ReadMacro;
    END

   { Abbruch Makroexpansion ? }

   ELSE IF Memo('EXITM') THEN
    BEGIN
     ExpandEXITM;
    END

   { Includefile? }

   ELSE IF Memo('INCLUDE') THEN
    BEGIN
     ExpandINCLUDE(NOT MasterFile); MasterFile:=False;
    END

   { Makroaufruf ? }

   ELSE IF (SearchMacros) AND (FoundMacro(OneMacro)) THEN
    BEGIN
     IF IfAsm THEN
      BEGIN
       ExpandMacro(OneMacro);
       ListLine:='(MACRO)';
      END
    END

   ELSE
    BEGIN
     StopfZahl:=0; CodeLen:=0; DontPrint:=False;

     IF IfAsm THEN
      BEGIN
       IF NOT CodeGlobalPseudo THEN MakeCode;
       IF (MacProOutput) AND (Length(OpPart)<>0) THEN
        BEGIN
         {$i-} WriteLn(MacProFile,OneLine); ChkIO(10002); {$i+}
        END;
      END;

     FOR z:=1 TO StopfZahl DO
      BEGIN
       CASE ListGran OF
       4:DAsmCode[CodeLen SHR 2]:=NOPCode;
       2:WAsmCode[CodeLen SHR 1]:=NOPCode;
       1:BAsmCode[CodeLen      ]:=NOPCode;
       END;
       Inc(CodeLen,ListGran DIV Granularity);
      END;

     IF (NOT ChkPC) AND (CodeLen<>0) THEN WrError(1925)
     ELSE
      BEGIN
       IF NOT DontPrint THEN
        BEGIN
         IF MakeUseList THEN
          IF AddChunk(SegChunks[ActPC],ProgCounter,CodeLen,ActPC=SegCode) THEN WrError(90);
         IF DebugMode<>DebugNone THEN AddSectionUsage(ProgCounter,CodeLen);
        END;
       Inc(PCs[ActPC],CodeLen);
       {IF ActPC<>SegCode THEN
        BEGIN
         IF (CodeLen<>0) AND (NOT DontPrint) THEN WrError(1940);
        END
       ELSE} IF CodeOutput THEN
        BEGIN
         IF DontPrint THEN NewRecord ELSE WriteBytes;
        END;
       IF (DebugMode<>DebugNone) AND (CodeLen>0) AND (NOT DontPrint) THEN
        AddLineInfo(True,CurrLine,CurrFileName,ActPC,PCs[ActPC]-CodeLen);
      END;
    END;
END;

{---- Zeile in Listing zerteilen --------------------------------------------}

       PROCEDURE SplitLine;
LABEL
   Retry;
VAR
   h:String;
   i,k,z,p:Integer;
   brack,angbrack:ShortInt;
   lpos,paren,quot:Boolean;
BEGIN
   Retracted:=False;

   { Kommentar lîschen }

   h:=OneLine; i:=QuotPos(h,';');
   CommPart:=Copy(h,i+1,Length(h)-i);

   {$IFDEF SPEEDUP}
   Byte(h[0]):=i-1;
   {$ELSE}
   IF i<=Length(h) THEN Delete(h,i,Length(h)-i+1);
   {$ENDIF}

   { alles in Grossbuchstaben wandeln, PrÑprozessor laufen lassen }

   ExpandDefines(h);

   { Label abspalten }

   IF (h<>'') AND (NOT IsBlank(h[1])) THEN
    BEGIN
     i:=FirstBlank(h); k:=Pos(':',h);
     IF (k<>0) AND (k<i) THEN i:=k;
     SplitString(h,LabPart,h,i);
     IF LabPart[Length(LabPart)]=':' THEN Delete(LabPart,Length(LabPart),1);
    END
   ELSE LabPart:='';

   { Opcode & Argument trennen }
Retry:
   KillPrefBlanks(h);
   SplitString(h,OpPart,ArgPart,FirstBlank(h));

   { Falls noch kein Label da war, kann es auch ein Label sein }

   i:=Pos(':',OpPart);
   IF (LabPart='') AND (i<>0) AND (i=Length(OpPart)) THEN
    BEGIN
     LabPart:=Copy(OpPart,1,i-1); Delete(OpPart,1,i);
     IF OpPart='' THEN
      BEGIN
       h:=ArgPart;
       Goto Retry;
      END;
    END;

   { Attribut abspalten }

   IF HasAttrs THEN
    BEGIN
     k:=MaxInt; AttrSplit:=' ';
     FOR z:=1 TO Length(AttrChars) DO
      BEGIN
       p:=Pos(AttrChars[z],OpPart); IF (p<>0) AND (p<k) THEN k:=p;
      END;
     IF k<MaxInt THEN
      BEGIN
       AttrSplit:=OpPart[k];
       SplitString(OpPart,OpPart,AttrPart,k);
       IF (OpPart='') AND (AttrPart<>'') THEN
        BEGIN
         OpPart:=AttrPart; AttrPart:='';
        END;
      END
     ELSE AttrPart:='';
    END
   ELSE AttrPart:='';

   KillPostBlanks(ArgPart);

   { Argumente zerteilen }
   ArgCnt:=0; h:=ArgPart;
   IF h<>'' THEN
    REPEAT
     KillPrefBlanks(h);
     i:=Length(h)+1;
     FOR z:=1 TO Length(DivideChars) DO
      BEGIN
       p:=QuotPos(h,DivideChars[z]); IF i>p THEN i:=p;
      END;
     lpos:=i=Length(h);
     IF i>0 THEN
      BEGIN
       Inc(ArgCnt);
       SplitString(h,ArgStr[ArgCnt],h,i);
      END;
     IF (lpos) AND (ArgCnt<>ParMax) THEN
      BEGIN
       Inc(ArgCnt);
       ArgStr[ArgCnt]:='';
      END;
     KillPostBlanks(ArgStr[ArgCnt]);
    UNTIL (h='') OR (ArgCnt=ParMax);

   IF h<>'' THEN WrError(1140);

   Produce_Code;
END;

CONST
   LineBuffer:String='';
   InComment:Boolean=FALSE;

	PROCEDURE C_SplitLine;
VAR
   p,p2:Integer;
   SaveLine,h:String;
BEGIN
   { alten Inhalt sichern }

   SaveLine:=OneLine; h:=OneLine;

   { Falls in Kommentar, nach schlie·ender Klammer suchen und den Teil bis
     dahin verwerfen; falls keine Klammer drin ist, die ganze Zeile weg-
     schmei·en; da wir dann OneLine bisher noch nicht verÑndert hatten,
     stîrt der Abbruch ohne Wiederherstellung von Oneline nicht. }

   IF InComment THEN
    BEGIN
     p:=Pos('}',h);
     IF p>Length(h) THEN Exit
     ELSE
      BEGIN
       Delete(h,1,p); InComment:=False;
      END;
    END;

   { in der Zeile befindliche Teile lîschen; falls am Ende keine
     schlie·ende Klammer kommt, mÅssen wir das Kommentarflag setzen. }

   REPEAT
    p:=QuotPos(h,'{');
    IF p>Length(h) THEN p:=0
    ELSE
     BEGIN
      p2:=QuotPos(h,'}');
      IF (p2>p) AND (Length(h)>=p2) THEN Delete(h,p,p2-p+1)
      ELSE
       BEGIN
        Byte(h[0]):=Pred(p);
        InComment:=True;
        p:=0;
       END;
     END;
   UNTIL p=0;

   { alten Inhalt zurÅckkopieren }

   OneLine:=SaveLine;
END;

{--------------------------------------------------------------------------}

        PROCEDURE ProcessFile(FileName:String);
VAR
   NxtTime,ListTime:LongInt;
BEGIN
   OneLine:=' INCLUDE '+FileName;
   NextIncDepth:=IncDepth; MasterFile:=True;
   SplitLine;

   IncDepth:=NextIncDepth;

   ListTime:=Gtime;

   WHILE (NOT InputEnd) AND (NOT ENDOccured) DO
    BEGIN
     { Zeile lesen }

     GetNextLine(OneLine);

     { Ergebnisfelder vorinitialisieren }

     DontPrint:=False; CodeLen:=0; ListLine:='';

     NextDoLst:=DoLst;
     NextIncDepth:=IncDepth;

     IF OneLine[1]='#' THEN Preprocess
     ELSE
      BEGIN
       SplitLine;
      END;
     MakeList;
     DoLst:=NextDoLst;
     IncDepth:=NextIncDepth;

     { ZeilenzÑhler }

     IF NOT QuietMode THEN
      BEGIN
       NxtTime:=GTime;
       IF ((Length(LstName)<>0) OR (ListMask AND 1=0)) AND (DTime(ListTime,NxtTime)>50) THEN
        BEGIN
         Write(NamePart(CurrFileName),'(',MomLineCounter,')',ClrEol,#13);
         ListTime:=NxtTime;
        END;
      END;

     { bei Ende Makroprozessor ausrÑumen
       OK - das ist eine Hauruckmethode... }

     IF ENDOccured THEN
      WHILE FirstInputTag<>Nil DO GetNextLine(OneLine);
    END;

   { irgendeine Makrodefinition nicht abgeschlossen ? }

   IF FirstOutputTag<>Nil THEN WrError(1800);

   WHILE FirstInputTag<>Nil DO GetNextLine(OneLine);
END;

       PROCEDURE TWrite(DTime:Single; VAR dest:String);
VAR
   h:Integer;
   s:String;

       FUNCTION Plur(n:Integer):String;
BEGIN
   IF n<>1 THEN Plur:=ListPlurName ELSE Plur:='';
END;

       PROCEDURE RWrite(r:Real; Stellen:Byte);
VAR
   s:String;
BEGIN
   Str(r:20:Stellen,s); WHILE s[1]=' ' DO Delete(s,1,1); dest:=dest+s;
END;

BEGIN
   dest:='';
   h:=Trunc(DTime/3600);
   IF h>0 THEN
    BEGIN
     Str(h,s); dest:=dest+s+ListHourName+Plur(h)+', ';
     DTime:=DTime-3600*h;
    END;
   h:=Trunc(DTime/60);
   IF h>0 THEN
    BEGIN
     Str(h,s); dest:=dest+s+ListMinuName+Plur(h)+', ';
     DTime:=DTime-60*h;
    END;
   RWrite(DTime,1); dest:=dest+ListSecoName;
   IF DTime<>1 THEN dest:=dest+ListPlurName;
END;


        PROCEDURE AssembleFile;
VAR
   s,Param:String;
   MacroName,MacProName,OutFileName:String;
   z:Integer;

        PROCEDURE InitPass;
CONST
   DateS:String[30]='';
   TimeS:String[30]='';
VAR
   z:Integer;
   Ch:Char;
BEGIN
   FirstInputTag:=Nil; FirstOutputTag:=Nil;

   ErrorPos:=''; MomLineCounter:=0;
   MomLocHandle:=-1; LocHandleCnt:=0;

   SectionStack:=Nil;
   FirstIfSave:=Nil;
   FirstSaveState:=Nil;

   InitPassProc;

   ActPC:=SegCode; PCs[ActPC]:=0; ENDOccured:=False;
   ErrorCount:=0; WarnCount:=0; LineSum:=0; MacLineSum:=0;
   FOR z:=1 TO PCMax DO
    BEGIN
     PCsUsed[z]:=(z=1);
     Phases[z]:=0;
     InitChunk(SegChunks[z]);
    END;
   FOR z:=0 TO 255 DO CharTransTable[Chr(z)]:=Chr(z);

   CurrFileName:='INTERNAL'; AddFile(CurrFileName); CurrLine:=0;

   IncDepth:=-1;
   DoLst:=True;

   { Pseudovariablen initialisieren }

   ResetSymbolDefines; ResetMacroDefines;

   EnterIntSymbol(FlagTrueName,1,0,True);
   EnterIntSymbol(FlagFalseName,0,0,True);
   EnterFloatSymbol(PiName,4*ArcTan(1),True);
   EnterIntSymbol(VerName,VerNo,0,True);
   EnterIntSymbol(Has64Name,0,0,True);
   EnterIntSymbol(CaseSensName,Ord(CaseSensitive),0,True);
   IF PassNo=0 THEN
    BEGIN
     DateS:=NLS_CurrDateString;
     TimeS:=NLS_CurrTimeString(False);
    END;
   EnterStringSymbol(DateName,DateS,True);
   EnterStringSymbol(TimeName,TimeS,True);
   SetCPU(0,True);
   SetFlag(SupAllowed,SupAllowedName,False);
   SetFlag(FPUAvail,FPUAvailName,False);
   SetFlag(DoPadding,DoPaddingName,True);
   SetFlag(Maximum,MaximumName,False);
   ListOn:=1; EnterIntSymbol(ListOnName,ListOn,0,True);
   SetFlag(LstMacroEx,LstMacroExName,True);
   SetFlag(RelaxedMode,RelaxedName,False);
   CopyDefSymbols;

   ResetPageCounter;

   StartAdrPresent:=False;

   JmpErrors:=0;

   Repass:=False; Inc(PassNo);
END;

        PROCEDURE ExitPass;
BEGIN
   SwitchFrom;

   ClearLocStack;

   ClearStacks;

   IF FirstIfSave<>Nil THEN
    WrError(1470);
   IF FirstSaveState<>Nil THEN
    WrError(1460);
   IF SectionStack<>Nil THEN
    WrError(1485);
END;

BEGIN
   IF MakeDebug THEN WriteLn(Debug,'File ',SourceFile);

   { Untermodule initialisieren }

   AsmDefInit; AsmParsInit; AsmIFInit; InitFileList; ResetStack;

   { Kommandozeilenoptionen verarbeiten }

   OutName:=GetFromOutList;
   IF OutName='' THEN
    BEGIN
     OutName:=SourceFile; KillSuffix(OutName);
     AddSuffix(OutName,PrgSuffix);
    END;

   IF ErrorPath='' THEN
    BEGIN
     ErrorName:=SourceFile;
     KillSuffix(ErrorName);
     AddSuffix(ErrorName,LogSuffix);
     Assign(ErrorFile,ErrorName);
    END;

   CASE ListMode OF
   0:LstName:='NUL';
   1:LstName:='';
   2:BEGIN
      LstName:=SourceFile;
      KillSuffix(LstName);
      AddSuffix(LstName,LstSuffix);
     END;
   END;

   IF ShareMode<>0 THEN
    BEGIN
     ShareName:=SourceFile;
     KillSuffix(ShareName);
     CASE ShareMode OF
     1:AddSuffix(ShareName,'.INC');
     2:AddSuffix(ShareName,'.H');
     3:AddSuffix(ShareName,IncSuffix);
     END;
     Assign(ShareFile,ShareName);
    END;

   IF MacProOutput THEN
    BEGIN
     MacProName:=SourceFile; KillSuffix(MacProName);
     AddSuffix(MacProName,PreSuffix);
     Assign(MacProFile,MacProName);
    END;

   IF MacroOutput THEN
    BEGIN
     MacroName:=SourceFile; KillSuffix(MacroName);
     AddSuffix(MacroName,MacSuffix);
     Assign(MacroFile,MacroName);
    END;

   IF MakeIncludeList THEN ClearIncludeList;

   IF DebugMode<>DebugNone THEN InitLineInfo;

   { Variablen initialisieren }

   StartTime:=GTime;

   PassNo:=0; MomLineCounter:=0;

   { Listdatei erîffnen }

   IF NOT QuietMode THEN WriteLn(InfoMessAssembling,SourceFile);

   REPEAT
    { Durchlauf initialisieren }

    InitPass; AsmSubInit;

    IF NOT QuietMode THEN WriteLn(InfoMessPass,PassNo,ClrEol);

    { Dateien îffnen }

    IF CodeOutput THEN OpenFile;

    IF ShareMode<>0 THEN
     BEGIN
      {$i-} SetFileMode(2); Rewrite(ShareFile); {$i+}
      ChkIO(10001);
      CASE ShareMode OF
      1:WriteLn(ShareFile,'(* ' ,SourceFile,'-Includefile f',Ch_ue,'r CONST-Sektion *)');
      2:WriteLn(ShareFile,'/* ',SourceFile,'-Includefile f',Ch_ue,'r C-Programm */');
      3:WriteLn(ShareFile,'; ' ,SourceFile,'-Includefile f',Ch_ue,'r Assembler-Programm');
      END;
      ChkIO(10002);
     END;

    IF MacProOutput THEN
     BEGIN
      {$i-} SetFileMode(2); Rewrite(MacProFile); {$i+}
      ChkIO(10001);
     END;

    IF (MacroOutput) AND (PassNo=1) THEN
     BEGIN
      {$i-} SetFileMode(2); Rewrite(MacroFile); {$i+}
      ChkIO(10001);
     END;

    { Listdatei îffnen }

    {$i-}
    Assign(LstFile,LstName);
    SetFileMode(2); Rewrite(LstFile);
    ChkIO(10001);
    Write(LstFile,PrtInitString);
    ChkIO(10002);
    IF ListMask AND 1<>0 THEN NewPage(0,False);
    {$i+}

    { assemblieren }

    ProcessFile(SourceFile);

    ExitPass;

    { Dateien schlie·en }

    IF CodeOutput THEN CloseFile;

    IF ShareMode<>0 THEN
     BEGIN
      CASE ShareMode OF
      1:WriteLn(ShareFile,'(* Ende Includefile f',Ch_ue,'r CONST-Sektion *)');
      2:WriteLn(ShareFile,'/* Ende Includefile f',Ch_ue,'r C-Programm */');
      3:WriteLn(ShareFile,'; Ende Includefile f',Ch_ue,'r Assembler-Programm');
      END;
      ChkIO(10002);
      Close(ShareFile);
     END;

    IF MacProOutput THEN Close(MacProFile);
    IF (MacroOutput) AND (PassNo=1)  THEN Close(MacroFile);

    { evtl. fÅr nÑchsten Durchlauf aufrÑumen }

    IF (ErrorCount=0) AND (Repass) THEN
     BEGIN
      Close(LstFile);
      IF CodeOutput THEN Erase(PrgFile);
      IF MakeUseList THEN ClearUseList;
      IF MakeCrossList THEN ClearCrossList;
      ClearDefineList;
      ClearIncludeList;
      IF DebugMode<>DebugNone THEN ClearLineInfo;
     END;

   UNTIL (ErrorCount<>0) OR (NOT Repass);

   { bei Fehlern lîschen }

   IF ErrorCount<>0 THEN
    BEGIN
     {$i-}
     IF CodeOutput THEN Erase(PrgFile); z:=IOResult;
     IF MacProOutput THEN Erase(MacProFile); z:=IOResult;
     IF (MacroOutput) AND (PassNo=1)  THEN Erase(MacroFile); z:=IOResult;
     IF ShareMode<>0 THEN Erase(ShareFile); z:=IOResult;
     {$i+}
     ErrFlag:=True;
    END;

   { Debug-Ausgabe mu· VOR die Symbollistenausgabe, weil letztere die
     Symbolliste lîscht }

   IF DebugMode<>DebugNone THEN
    BEGIN
     IF ErrorCount=0 THEN DumpDebugInfo;
     ClearLineInfo;
    END;

   { Listdatei abschlie·en }

   IF LstName<>'NUL' THEN
    BEGIN
     IF ListMask AND 2<>0 THEN PrintSymbolList;

     IF ListMask AND 4<>0 THEN PrintMacroList;

     IF ListMask AND 8<>0 THEN PrintFunctionList;

     IF ListMask AND 32<>0 THEN PrintDefineList;

     IF MakeUseList THEN
      BEGIN
       NewPage(ChapDepth,True);
       PrintUseList;
      END;

     IF MakeCrossList THEN
      BEGIN
       NewPage(ChapDepth,True);
       PrintCrossList;
      END;

     IF MakeSectionList THEN PrintSectionList;

     IF MakeIncludeList THEN PrintIncludeList;

     {$i-} Write(LstFile,PrtExitString); {$i+}
     ChkIO(10002);
    END;

   IF MakeUseList THEN ClearUseList;

   IF MakeCrossList THEN ClearCrossList;

   ClearSectionList;

   ClearIncludeList;

   IF (ErrorPath='') AND (IsErrorOpen) THEN
    BEGIN
     Close(ErrorFile); IsErrorOpen:=False;
    END;

   ClearUpProc;

   { Statistik ausgeben }

   StopTime:=GTime;
   TWrite(DTime(StartTime,StopTime)/100,s);
   IF NOT QuietMode THEN
    BEGIN
     WriteLn; WriteLn(s,InfoMessAssTime,ClrEol); WriteLn;
    END;
   IF ListMode=2 THEN
    BEGIN
     WrLstLine(''); WrLstLine(s+InfoMessAssTime); WrLstLine('');
    END;

   Str(LineSum:7,s);
   IF LineSum=1 THEN s:=s+InfoMessAssLine ELSE s:=s+InfoMessAssLines;
   IF NOT QuietMode THEN WriteLn(s,ClrEol);
   IF ListMode=2 THEN WrLstLine(s);

   IF LineSum<>MacLineSum THEN
    BEGIN
     Str(MacLineSum:7,s);
     IF MacLineSum=1 THEN s:=s+InfoMessMacAssLine ELSE s:=s+InfoMessMacAssLines;
     IF NOT QuietMode THEN WriteLn(s,ClrEol);
     IF ListMode=2 THEN WrLstLine(s);
    END;

   Str(PassNo:7,s);
   IF PassNo=1 THEN s:=s+InfoMessPassCnt ELSE s:=s+InfoMessPPassCnt;
   IF NOT QuietMode THEN WriteLn(s,ClrEol);
   IF ListMode=2 THEN WrLstLine(s);

   IF (ErrorCount>0) AND (Repass) AND (ListMode<>0) THEN
    WrLstLine(InfoMessNoPass);

   Str(ErrorCount:7,s); s:=s+InfoMessErrCnt;
   IF ErrorCount<>1 THEN s:=s+InfoMessErrPCnt;
   IF NOT QuietMode THEN WriteLn(s,ClrEol);
   IF ListMode=2 THEN WrLstLine(s);

   Str(WarnCount:7,s); s:=s+InfoMessWarnCnt;
   IF WarnCount<>1 THEN s:=s+InfoMessWarnPCnt;
   IF NOT QuietMode THEN WriteLn(s,ClrEol);
   IF ListMode=2 THEN WrLstLine(s);

   Str(Round(MemAvail/1024):7,s); s:=s+InfoMessRemainMem;
   IF NOT QuietMode THEN WriteLn(s,ClrEol);
   IF ListMode=2 THEN WrLstLine(s);

   Str(StackRes:7,s); s:=s+InfoMessRemainStack;
   IF NOT QuietMode THEN WriteLn(s,ClrEol);
   IF ListMode=2 THEN WrLstLine(s);

   Close(LstFile);

   { verstecktes }

   IF MakeDebug THEN PrintSymbolDepth;

   { Speicher freigeben }

   ClearSymbolList;
   ClearMacroList;
   ClearFunctionList;
   ClearDefineList;
   ClearFileList;
END;

        PROCEDURE AssembleGroup;
VAR
   PathPrefix:String;
BEGIN
   FileMask:=FExpand(FileMask);
   AddSuffix(FileMask,SrcSuffix);
   FindFirst(FileMask,AnyFile,Search);
   PathPrefix:=PathPart(FileMask);

   IF DosError<>0 THEN WriteLn(StdErr,FileMask,InfoMessNFilesFound,Char_LF)
   ELSE
    REPEAT
     IF (Search.Attr AND (Hidden OR SysFile OR VolumeID OR Directory)=0) THEN
      BEGIN
       SourceFile:=PathPrefix+Search.Name;
       AssembleFile;
      END;
     FindNext(Search);
    UNTIL DosError<>0
END;

{---------------------------------------------------------------------------}

        FUNCTION CMD_SharePascal(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   IF NOT Negate THEN ShareMode:=1
   ELSE IF ShareMode=1 THEN ShareMode:=0;
   CMD_SharePascal:=CMDOK;
END;

        FUNCTION CMD_ShareC(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   IF NOT Negate THEN ShareMode:=2
   ELSE IF ShareMode=2 THEN ShareMode:=0;
   CMD_ShareC:=CMDOK;
END;

        FUNCTION CMD_ShareAssembler(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   IF NOT Negate THEN ShareMode:=3
   ELSE IF ShareMode=3 THEN ShareMode:=0;
   CMD_ShareAssembler:=CMDOK;
END;

        FUNCTION CMD_DebugMode(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
{   UpString(Arg);

   IF Negate THEN
    IF Arg<>'' THEN CMD_DebugMode:=CMDErr
    ELSE
     BEGIN
      DebugMode:=DebugNone; CMD_DebugMode:=CMDOK;
     END
   ELSE IF Arg='' THEN
    BEGIN
     DebugMode:=DebugMap; CMD_DebugMode:=CMDOK;
    END
   ELSE IF Arg='MAP' THEN
    BEGIN
     DebugMode:=DebugMap; CMD_DebugMode:=CMDArg;
    END
   ELSE IF Arg='A.OUT' THEN
    BEGIN
     DebugMode:=DebugAOUT; CMD_DebugMode:=CMDArg;
    END
   ELSE IF Arg='COFF' THEN
    BEGIN
     DebugMode:=DebugCOFF; CMD_DebugMode:=CMDArg;
    END
   ELSE IF Arg='ELF' THEN
    BEGIN
     DebugMode:=DebugELF; CMD_DebugMode:=CMDArg;
    END
   ELSE CMD_DebugMode:=CMDErr;}

   IF Negate THEN DebugMode:=DebugNone
   ELSE DebugMode:=DebugMap;
   CMD_DebugMode:=CMDOK;
END;

        FUNCTION CMD_ListConsole(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   IF NOT Negate THEN ListMode:=1
   ELSE IF ListMode=1 THEN ListMode:=0;
   CMD_ListConsole:=CMDOK;
END;

        FUNCTION CMD_ListFile(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   IF NOT Negate THEN ListMode:=2
   ELSE IF ListMode=2 THEN ListMode:=0;
   CMD_ListFile:=CMDOK;
END;

        FUNCTION CMD_SuppWarns(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   SuppWarns:=NOT Negate;
   CMD_SuppWarns:=CMDOK;
END;

        FUNCTION CMD_UseList(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   MakeUseList:=NOT Negate;
   CMD_UseList:=CMDOK;
END;

        FUNCTION CMD_CrossList(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   MakeCrossList:=NOT Negate;
   CMD_CrossList:=CMDOK;
END;

        FUNCTION CMD_SectionList(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   MakeSectionList:=NOT Negate;
   CMD_SectionList:=CMDOK;
END;

        FUNCTION CMD_BalanceTree(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   BalanceTree:=NOT Negate;
   CMD_BalanceTree:=CMDOK;
END;

        FUNCTION CMD_MakeDebug(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   IF NOT Negate THEN
    BEGIN
     MakeDebug:=True;
     Assign(Debug,'AS.DEB'); SetFileMode(1); Rewrite(Debug); ChkIO(10002);
    END
   ELSE IF MakeDebug THEN
    BEGIN
     MakeDebug:=False;
     Close(Debug);
    END;
   CMD_MakeDebug:=CMDOK;
END;

        FUNCTION CMD_MacProOutput(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   MacProOutput:=NOT Negate;
   CMD_MacProOutput:=CMDOK;
END;

        FUNCTION CMD_MacroOutput(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   MacroOutput:=NOT Negate;
   CMD_MacroOutput:=CMDOK;
END;

        FUNCTION CMD_MakeIncludeList(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   MakeIncludeList:=NOT Negate;
   CMD_MakeIncludeList:=CMDOK;
END;

        FUNCTION CMD_CodeOutput(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   CodeOutput:=NOT Negate;
   CMD_CodeOutput:=CMDOK;
END;

        FUNCTION CMD_MsgIfRepass(Negate:Boolean; Arg:String):CMDResult;
        Far;
VAR
   Err:Integer;
BEGIN
   MsgIfRepass:=NOT Negate;
   IF MsgIfRepass THEN
    IF Arg='' THEN
     BEGIN
      PassNoForMessage:=1; CMD_MsgIfRepass:=CMDOK;
     END
    ELSE
     BEGIN
      Val(Arg,PassNoForMessage,Err);
      IF Err<>0 THEN
       BEGIN
        PassNoForMessage:=1; CMD_MsgIfRepass:=CMDOK;
       END
      ELSE IF PassNoForMessage<1 THEN CMD_MsgIfRepass:=CMDErr
      ELSE CMD_MsgIfRepass:=CMDArg;
     END
   ELSE CMD_MsgIfRepass:=CMDOK;
END;

        FUNCTION CMD_ExtendErrors(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   ExtendErrors:=NOT Negate;
   CMD_ExtendErrors:=CMDOK;
END;

        FUNCTION CMD_NumericErrors(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   NumericErrors:=NOT Negate;
   CMD_NumericErrors:=CMDOK;
END;

        FUNCTION CMD_HexLowerCase(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   HexLowerCase:=NOT Negate;
   CMD_HexLowerCase:=CMDOK;
END;

        FUNCTION CMD_QuietMode(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   QuietMode:=NOT Negate;
   CMD_QuietMode:=CMDOK;
END;

        FUNCTION CMD_ThrowErrors(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   ThrowErrors:=NOT Negate;
   CMD_ThrowErrors:=CMDOK;
END;

        FUNCTION CMD_CaseSensitive(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   CaseSensitive:=NOT Negate;
   CMD_CaseSensitive:=CMDOK;
END;

        FUNCTION CMD_IncludeList(Negate:Boolean; Arg:String):CMDResult;
        Far;
VAR
   p:Integer;
BEGIN
   IF Arg='' THEN CMD_IncludeList:=CMDErr
   ELSE
    BEGIN
     REPEAT
      p:=Length(Arg); WHILE (p>0) AND (Arg[p]<>';') DO Dec(p);
      IF Negate THEN RemoveIncludeList(Copy(Arg,p+1,Length(Arg)-p))
      ELSE AddIncludeList(Copy(Arg,p+1,Length(Arg)-p));
      IF p=0 THEN p:=1;
      Arg:=Copy(Arg,1,p-1);
     UNTIL Arg='';
     CMD_IncludeList:=CMDArg;
    END;
END;

        FUNCTION CMD_ListMask(Negate:Boolean; Arg:String):CMDResult;
        Far;
VAR
   erg:Byte;
   err:ValErgType;
BEGIN
   IF Arg='' THEN CMD_ListMask:=CMDErr
   ELSE
    BEGIN
     Val(Arg,erg,err);
     IF (err<>0) OR (erg>31) THEN CMD_ListMask:=CMDErr
     ELSE
      BEGIN
       IF NOT Negate THEN ListMask:=ListMask AND (NOT erg)
       ELSE ListMask:=ListMask OR erg;
       CMD_ListMask:=CMDArg;
      END;
    END;
END;

        FUNCTION CMD_DefSymbol(Negate:Boolean; Arg:String):CMDResult;
        Far;
VAR
   p:Integer;
   t:TempResult;
   Part,Name:String;
BEGIN
   CMD_DefSymbol:=CMDErr;

   IF Arg='' THEN Exit;

   REPEAT
    p:=QuotPos(Arg,',');
    Part:=Copy(Arg,1,p-1); Delete(Arg,1,p);
    p:=QuotPos(Part,'=');
    Name:=Copy(Part,1,p-1); Delete(Part,1,p);
    IF NOT ChkSymbName(Name) THEN Exit;
    IF Negate THEN RemoveDefSymbol(Name)
    ELSE
     BEGIN
      AsmParsInit;
      IF Part<>'' THEN
       BEGIN
        FirstPassUnknown:=False;
        EvalExpression(Part,t);
        IF (t.Typ=TempNone) OR (FirstPassUnknown) THEN Exit;
       END
      ELSE
       BEGIN
        t.Typ:=TempInt; t.Int:=1;
       END;
      AddDefSymbol(Name,t);
     END;
   UNTIL Arg='';
   CMD_DefSymbol:=CMDArg;
END;

        FUNCTION CMD_ErrorPath(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   IF Negate THEN CMD_ErrorPath:=CMDErr
   ELSE IF Arg='' THEN
    BEGIN
     CMD_ErrorPath:=CMDOK; ErrorPath:='';
    END
   ELSE
    BEGIN
     CMD_ErrorPath:=CMDArg; ErrorPath:=Arg;
    END;
END;

        FUNCTION CMD_OutFile(Negate:Boolean; Arg:String):CMDResult;
        Far;
BEGIN
   IF Arg='' THEN
    IF Negate THEN
     BEGIN
      ClearOutList; CMD_OutFile:=CMDOK;
     END
    ELSE CMD_OutFile:=CMDErr
   ELSE
    BEGIN
     IF Negate THEN RemoveFromOutList(Arg)
     ELSE AddToOutList(Arg);
     CMD_OutFile:=CMDArg;
    END;
END;

	FUNCTION CMD_CPUAlias(Negate:Boolean; Arg:String):CMDResult;
        Far;
VAR
   p:Integer;
   s1,s2:String;

   	FUNCTION ChkCPUName(VAR s:String):Boolean;
VAR
   z:Integer;
BEGIN
   ChkCPUName:=True;
   FOR z:=1 TO Length(s) DO
    IF NOT (s[z] IN ['0'..'9','A'..'Z','é','ô','ö']) THEN ChkCPUName:=False;
END;

BEGIN
   IF Negate THEN CMD_CPUAlias:=CMDErr
   ELSE IF Arg='' THEN CMD_CPUAlias:=CMDErr
   ELSE
    BEGIN
     p:=Pos('=',Arg);
     IF p=0 THEN CMD_CPUAlias:=CMDErr
     ELSE
      BEGIN
       s1:=Copy(Arg,1,p-1); UpString(s1);
       s2:=Copy(Arg,p+1,Length(Arg)-p); UpString(s2);
       IF NOT (ChkCPUName(s1) AND ChkCPUName(s2)) THEN CMD_CPUAlias:=CMDErr
       ELSE IF NOT AddCPUAlias(s2,s1) THEN CMD_CPUAlias:=CMDErr
       ELSE CMD_CPUAlias:=CMDArg;
      END;
    END;
END;

        PROCEDURE ParamError(InEnv:Boolean; Arg:String);
        Far;
BEGIN
   IF InEnv THEN Write(ErrMsgInvEnvParam)
            ELSE Write(ErrMsgInvParam);
   WriteLn(Arg);
   Halt(4);
END;

CONST
   ASParamCnt=30;
   ASParams:ARRAY[1..ASParamCnt] OF CMDRec=
            ((Ident:'A'; Callback:CMD_BalanceTree),
             (Ident:'ALIAS'; Callback:CMD_CPUAlias),
             (Ident:'a'; Callback:CMD_ShareAssembler),
             (Ident:'C'; Callback:CMD_CrossList),
             (Ident:'c'; Callback:CMD_ShareC),
             (Ident:'D'; Callback:CMD_DefSymbol),
             (Ident:'E'; Callback:CMD_ErrorPath),
             (Ident:'g'; Callback:CMD_DebugMode),
             (Ident:'G'; Callback:CMD_CodeOutput),
             (Ident:'h'; Callback:CMD_HexLowerCase),
             (Ident:'i'; Callback:CMD_IncludeList),
             (Ident:'I'; Callback:CMD_MakeIncludeList),
             (Ident:'L'; Callback:CMD_ListFile),
             (Ident:'l'; Callback:CMD_ListConsole),
             (Ident:'M'; Callback:CMD_MacroOutput),
             (Ident:'n'; Callback:CMD_NumericErrors),
             (Ident:'o'; Callback:CMD_OutFile),
             (Ident:'P'; Callback:CMD_MacProOutput),
             (Ident:'p'; Callback:CMD_SharePascal),
             (Ident:'q'; Callback:CMD_QuietMode),
             (Ident:'QUIET'; Callback:CMD_QuietMode),
             (Ident:'r'; Callback:CMD_MsgIfRepass),
             (Ident:'s'; Callback:CMD_SectionList),
             (Ident:'t'; Callback:CMD_ListMask),
             (Ident:'u'; Callback:CMD_UseList),
             (Ident:'U'; Callback:CMD_CaseSensitive),
             (Ident:'w'; Callback:CMD_SuppWarns),
             (Ident:'x'; Callback:CMD_ExtendErrors),
             (Ident:'X'; Callback:CMD_MakeDebug),
             (Ident:'Y'; Callback:CMD_ThrowErrors));

{---------------------------------------------------------------------------}

VAR
   SaveExitProc:Pointer;

        PROCEDURE GlobExitProc; Far;
BEGIN
   IF MakeDebug THEN Close(Debug);
   ExitProc:=SaveExitProc;
END;

        PROCEDURE NxtLine;
        Far;
BEGIN
   Inc(LineZ);
   IF LineZ=23 THEN
    BEGIN
     LineZ:=0;
     IF Redirected<>NoRedir THEN Exit;
     Write(KeyWaitMsg); ReadLn; Write(CursUp,ClrEol);
    END;
END;

	PROCEDURE WrHead;
BEGIN
   IF NOT QuietMode THEN
    BEGIN
     TextRec(Output).BufSize:=1;
     WriteLn(InfoMessMacroAss,Version); NxtLine;
     {$IFDEF OS2}
      {$IFDEF USE32}
       WriteLn('(OS/2-32-',InfoMessVar,')'); NxtLine;
      {$ELSE}
       WriteLn('(OS/2-16-',InfoMessVar,')'); NxtLine;
      {$ENDIF}
     {$ELSE}
      {$IFDEF DPMI}
       WriteLn('(DPMI-',InfoMessVar,')'); NxtLine;
      {$ELSE}
       WriteLn('(DOS-',InfoMessVar,')'); NxtLine;
      {$ENDIF}
     {$ENDIF}
     WriteLn(InfoMessCopyright); NxtLine;
     WriteCopyrights(NxtLine);
     WriteLn; NxtLine;
    END;

END;

BEGIN
   _Init_AsmDef; _Init_AsmSub; _Init_AsmPars;

   SaveExitProc:=ExitProc; ExitProc:=@GlobExitProc;

   NLS_Initialize;

   CursUp:=''; ClrEol:='';
   CASE Redirected OF
   NoRedir:
    BEGIN
     Dummy:=GetEnv('USEANSI'); IF Dummy='' THEN Dummy:='Y';
     IF UpCase(Dummy[1])='N' THEN
      BEGIN
       FOR i:=1 TO 20 DO ClrEol:=ClrEol+' ';
       FOR i:=1 TO 20 DO ClrEol:=ClrEol+Char_BS;
      END
     ELSE
      BEGIN
       ClrEol:=Char_ESC+'[K';   { ANSI-Sequenzen }
       CursUp:=Char_ESC+'[A';
      END;
    END;
   RedirToDevice:
    BEGIN
     { Basissteuerzeichen fÅr GerÑte }
     FOR i:=1 TO 20 DO ClrEol:=ClrEol+' ';
     FOR i:=1 TO 20 DO ClrEol:=ClrEol+Char_BS;
    END;
   RedirToFile:
    BEGIN
     ClrEol:=Char_CR+Char_LF;  { CRLF auf Datei }
    END;
   END;

   ShareMode:=0; ListMode:=0; IncludeList:=''; SuppWarns:=False;
   MakeUseList:=False; MakeCrossList:=False; MakeSectionList:=False;
   MakeIncludeList:=False; ListMask:=$3f;
   MakeDebug:=False; ExtendErrors:=False;
   MacroOutput:=False; MacProOutput:=False; CodeOutput:=True;
   ErrorPath:='!2'; MsgIfRepass:=False; QuietMode:=False;
   NumericErrors:=False; DebugMode:=DebugNone; CaseSensitive:=False;
   ThrowErrors:=False;

   LineZ:=0;

   IF ParamCount=0 THEN
    BEGIN
     WrHead;
     WriteLn(InfoMessHead1,GetEXEName,InfoMessHead2); NxtLine;
     FOR i:=1 TO InfoMessHelpCnt DO
      BEGIN
       WriteLn(InfoMessHelp[i]); NxtLine;
      END;
     PrintCPUList(NxtLine);
     ClearCPUList;
     ClearCopyrightList;
     ClearOutList;
     Halt(1);
    END;

   ProcessCMD(@ASParams,ASParamCnt,ParUnprocessed,EnvName,ParamError);

   { wegen QuietMode dahinter }

   WrHead;

   ErrFlag:=False;
   IF ErrorPath<>'' THEN ErrorName:=ErrorPath;
   IsErrorOpen:=False;

   IF ParUnProcessed=[] THEN
    BEGIN
     Write(InvMsgSource,' [',SrcSuffix,'] '); ReadLn(FileMask);
     AssembleGroup;
    END
   ELSE
    FOR i:=1 TO ParamCount DO
    IF i IN ParUnProcessed THEN
     BEGIN
      FileMask:=ParamStr(i);
      AssembleGroup;
     END;

   IF (ErrorPath<>'') AND (IsErrorOpen) THEN
    BEGIN
     Close(ErrorFile); IsErrorOpen:=False;
    END;

   ClearCPUList; ClearCopyrightList; ClearOutList;

   IF ErrFlag THEN Halt(2) ELSE Halt(0);
END.
