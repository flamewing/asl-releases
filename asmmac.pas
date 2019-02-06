{$i STDINC.PAS}
{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}
	UNIT AsmMac;

INTERFACE

        Uses NLS,StringLi,StringUt,
	     AsmDef,AsmSub,AsmPars;

TYPE
   PMacroRec=^MacroRec;
   MacroRec=RECORD
             Name:StringPtr;            { Name des Makros }
	     ParamCount:Byte;           { Anzahl Parameter }
	     FirstLine:StringList;      { Zeiger auf erste Zeile }
	     Used:Boolean;              { wird gerade benutzt-verhindert Rekusion }
	     LocMacExp:Boolean;         { Makroexpansion wird aufgelistet }
	    END;

   PInputTag=^TInputTag;
   BufferArray=ARRAY[0..1023] OF Byte;
   TInputTag=RECORD
              Next:PInputTag;
              IsMacro:Boolean;
              IfLevel:Integer;
              First:Boolean;
              OrigPos:String;
              OrigDoLst:Boolean;
              StartLine:LongInt;
              Processor:FUNCTION(P:PInputTag; VAR erg:String):Boolean;
              ParCnt,ParZ:LongInt;
              Params:StringList;
              LineCnt,LineZ:LongInt;
              Lines:StringRecPtr;
              SpecName:String;
              SaveAttr:String;
              IsEmpty:Boolean;
              Datei:^Text;
              Buffer:^BufferArray;
              Cleanup:PROCEDURE(P:PInputTag);
              Restorer:PROCEDURE(P:PInputTag);
             END;

   POutputTag=^TOutputTag;
   TOutputTag=RECORD
               Next:POutputTag;
               Processor:PROCEDURE;
               NestLevel:Integer;
               Tag:PInputTag;
               Mac:PMacroRec;
               Params:StringList;
               PubSect,GlobSect:LongInt;
               DoExport,DoGlobCopy:Boolean;
               GName:String;
              END;

VAR
   FirstInputTag:PInputTag;
   FirstOutputTag:POutputTag;

	PROCEDURE Preprocess;


	PROCEDURE AddMacro(Neu:PMacroRec; DefSect:LongInt; Protest:Boolean);

	FUNCTION  FoundMacro(VAR Erg:PMacroRec):Boolean;

	PROCEDURE ClearMacroList;

	PROCEDURE ResetMacroDefines;

	PROCEDURE ClearMacroRec(VAR Alt:PMacroRec);

	PROCEDURE PrintMacroList;


	PROCEDURE EnterDefine(Name,Definition:String);

	PROCEDURE RemoveDefine(Name:String);

	PROCEDURE PrintDefineList;

	PROCEDURE ClearDefineList;

	PROCEDURE ExpandDefines(VAR Line:String);


IMPLEMENTATION

{$i as.rsc}

{==== PrÑprozessor =========================================================}

	PROCEDURE Preprocess;
VAR
   h,Cmd,Arg:String;
   p:Integer;
BEGIN
   h:=Copy(OneLine,2,Length(OneLine)-1);
   p:=FirstBlank(h);
   IF p>Length(h) THEN
    BEGIN
     Cmd:=h; h:='';
    END
   ELSE SplitString(h,Cmd,h,p);

   KillPrefBlanks(h); KillPrefBlanks(h); NLS_UpString(Cmd);

   IF Cmd='DEFINE' THEN
    BEGIN
     p:=FirstBlank(h);
     IF p<Length(h) THEN
      BEGIN
       SplitString(h,Arg,h,p); KillPrefBlanks(h);
       EnterDefine(Arg,h);
      END;
    END
   ELSE IF Cmd='UNDEF' THEN RemoveDefine(h);

   CodeLen:=0;
END;

{---------------------------------------------------------------------------}
{ Verwaltung define-Symbole }

	PROCEDURE FreeDefine(P:PDefinement);
BEGIN
   WITH P^ DO
    BEGIN
     FreeMem(TransFrom,Length(TransFrom^)+1);
     FreeMem(TransTo,Length(TransTo^)+1);
    END;
   Dispose(P);
END;

	PROCEDURE EnterDefine(Name,Definition:String);
VAR
   Neu:PDefinement;
   z:Integer;
BEGIN
   IF NOT ChkSymbName(Name) THEN
    BEGIN
     WrXError(1020,Name); Exit;
    END;

   Neu:=FirstDefine;
   WHILE Neu<>Nil DO
    BEGIN
     IF Neu^.TransFrom^=Name THEN
      BEGIN
       IF PassNo=1 THEN WrXError(1000,Name); Exit;
      END;
     Neu:=Neu^.Next;
    END;

   New(Neu);
   WITH Neu^ DO
    BEGIN
     Next:=FirstDefine;
     GetMem(TransFrom,Length(Name)+1); TransFrom^:=Name;
     IF (NOT CaseSensitive) THEN NLS_UpString(TransFrom^);
     GetMem(TransTo,Length(Definition)+1); TransTo^:=Definition;
     FOR z:=0 TO 255 DO Compiled[Chr(z)]:=Length(Name);
     FOR z:=1 TO Length(Name)-1 DO Compiled[TransFrom^[z]]:=Length(Name)-z;
    END;
   FirstDefine:=Neu;
END;

	PROCEDURE RemoveDefine(Name:String);
VAR
   Lauf,Del:PDefinement;
BEGIN
   Del:=Nil;

   IF NOT CaseSensitive THEN NLS_UpString(Name);

   IF (FirstDefine<>Nil) THEN
    IF (FirstDefine^.TransFrom^=Name) THEN
     BEGIN
      Del:=FirstDefine; FirstDefine:=FirstDefine^.Next;
     END
    ELSE
     BEGIN
      Lauf:=FirstDefine;
      WHILE (Lauf^.Next<>Nil) AND (Lauf^.Next^.TransFrom^<>Name) DO
       Lauf:=Lauf^.Next;
      IF Lauf^.Next<>Nil THEN
       BEGIN
	Del:=Lauf^.Next; Lauf^.Next:=Del^.Next;
       END;
     END;

    IF Del=Nil THEN WrXError(1010,Name)
    ELSE FreeDefine(Del);
END;

	PROCEDURE PrintDefineList;
VAR
   Lauf:PDefinement;
   OneS:String;
BEGIN
   IF FirstDefine=Nil THEN Exit;

   NewPage(ChapDepth,True);
   WrLstLine(ListDefListHead1);
   WrLstLine(ListDefListHead2);
   WrLstLine('');

   OneS:=''; Lauf:=FirstDefine;
   WHILE Lauf<>Nil DO
   WITH Lauf^ DO
    BEGIN
     OneS:=TransFrom^+Blanks(10-(Length(TransFrom^) MOD 10))+' = '+TransTo^;
     WrLstLine(OneS);
     Lauf:=Lauf^.Next;
    END;
   WrLstLine('');
END;

	PROCEDURE ClearDefineList;
VAR
   Temp:PDefinement;
BEGIN
   WHILE FirstDefine<>Nil DO
    BEGIN
     Temp:=FirstDefine; FirstDefine:=FirstDefine^.Next;
     FreeDefine(Temp);
    END;
END;

	PROCEDURE ExpandDefines(VAR Line:String);
CONST
   NErl:Set OF Char=['0'..'9','A'..'Z','a'..'z'];
VAR
   Lauf:PDefinement;
   p,p2,p3,z,z2,LPos,Diff:Integer;
   Fnd:Boolean;

        FUNCTION T_ToUpper(Ch:Char):Char;
BEGIN
   IF CaseSensitive THEN T_ToUpper:=Ch
   ELSE T_ToUpper:=UpCase(Ch);
END;

BEGIN
   Lauf:=FirstDefine;
   WHILE Lauf<>Nil DO
    WITH Lauf^ DO
     BEGIN
      LPos:=1; Diff:=Length(TransTo^)-Length(TransFrom^);
      REPEAT
       { Stelle, ab der verbatim, suchen -->p }
       p:=LPos;
       WHILE (p<=Length(Line)) AND (Line[p]<>'''') AND (Line[p]<>'"') DO Inc(p);
       { nach Quellstring suchen, ersetzen, bis keine Treffer mehr }
       p2:=LPos;
       REPEAT
	z2:=1;
	WHILE (z2>0) AND (p2<=p-Length(TransFrom^)) DO
	 BEGIN
	  z2:=Length(TransFrom^); z:=p2+z2-1;
          WHILE (z2>0) AND (T_ToUpper(Line[z])=TransFrom^[z2]) DO
	   BEGIN
	    Dec(z2); Dec(z);
	   END;
          IF z2>0 THEN Inc(p2,Compiled[T_ToUpper(Line[z])]);
	 END;
	IF z2=0 THEN
	 BEGIN
	  IF ((p2=1) OR (NOT (Line[p2-1] IN NErl)))
	  AND ((p2+Length(TransFrom^)=p) OR (NOT (Line[p2+Length(TransFrom^)] IN NErl))) THEN
	   BEGIN
	    IF Diff<>0 THEN
	     BEGIN
	      Move(Line[p2+Length(TransFrom^)],Line[p2+Diff+Length(TransFrom^)],Length(Line)-p2-Length(TransFrom^)+1);
	      Inc(Byte(Line[0]),Diff);
	     END;
	    Move(TransTo^[1],Line[p2],Length(TransTo^));
	    Inc(p,Diff); { !!! }
	    Inc(p2,Length(TransTo^));
	   END
	  ELSE Inc(p2,Length(TransFrom^));
	 END;
       UNTIL z2>0;
       { Endposition verbatim suchen }
       p3:=Succ(p);
       WHILE (p3<=Length(Line)) AND (Line[p3]<>Line[p]) DO Inc(p3);
       { ZÑhler entsprechend herauf }
       LPos:=Succ(p3);
      UNTIL LPos>Length(Line)-Length(TransFrom^)+1;
      Lauf:=Lauf^.Next;
     END;
END;

{==== Makrolistenverwaltung ================================================}

TYPE
   PMacroNode=^TMacroNode;
   TMacroNode=RECORD
	       Left,Right:PMacroNode;   { Sîhne im Baum }
               Balance:ShortInt;
	       DefSection:LongInt;      { GÅltigkeitssektion }
	       Defined:Boolean;
	       Contents:PMacroRec;
	      END;

VAR
   MacroRoot:PMacroNode;

	PROCEDURE AddMacro(Neu:PMacroRec; DefSect:LongInt; Protest:Boolean);

        FUNCTION AddNode(VAR Node:PMacroNode):Boolean;
VAR
   CompErg:ShortInt;
   Grown:Boolean;
   p1,p2:PMacroNode;
BEGIN
   ChkStack;

   IF Node=Nil THEN
    BEGIN
     New(Node);
     WITH Node^ DO
      BEGIN
       Left:=Nil; Right:=Nil; Balance:=0; Defined:=True;
       DefSection:=DefSect; Contents:=Neu;
       AddNode:=True;
      END;
     Exit;
    END
   ELSE AddNode:=False;

   CompErg:=StrCmp(Neu^.Name^,Node^.Contents^.Name^,DefSect,Node^.DefSection);
   CASE CompErg OF
    1:BEGIN
       Grown:=AddNode(Node^.Right);
       IF (BalanceTree) AND (Grown) THEN
       CASE Node^.Balance OF
       -1:Node^.Balance:=0;
       0:BEGIN Node^.Balance:=1; AddNode:=True; END;
       1:BEGIN
          p1:=Node^.Right;
          IF p1^.Balance=1 THEN
           BEGIN
            Node^.Right:=p1^.Left; p1^.Left:=Node;
            Node^.Balance:=0; Node:=p1;
           END
          ELSE
           BEGIN
            p2:=p1^.Left;
            p1^.Left:=p2^.Right; p2^.Right:=p1;
            Node^.Right:=p2^.Left; p2^.Left:=Node;
            IF p2^.Balance= 1 THEN Node^.Balance:=-1 ELSE Node^.Balance:=0;
            IF p2^.Balance=-1 THEN p1^  .Balance:= 1 ELSE p1^  .Balance:=0;
            Node:=p2;
           END;
          Node^.Balance:=0;
         END;
       END;
      END;
   -1:BEGIN
       Grown:=AddNode(Node^.Left);
       IF (BalanceTree) AND (Grown) THEN
        CASE Node^.Balance OF
        1:Node^.Balance:=0;
        0:BEGIN Node^.Balance:=-1; AddNode:=True; END;
        -1:BEGIN
            p1:=Node^.Left;
            IF p1^.Balance=-1 THEN
             BEGIN
              Node^.Left:=p1^.Right; p1^.Right:=Node;
              Node^.Balance:=0; Node:=p1;
             END
            ELSE
             BEGIN
              p2:=p1^.Right;
              p1^.Right:=p2^.Left; p2^.Left:=p1;
              Node^.Left:=p2^.Right; p2^.Right:=Node;
              IF p2^.Balance=-1 THEN Node^.Balance:= 1 ELSE Node^.Balance:=0;
              IF p2^.Balance= 1 THEN p1^  .Balance:=-1 ELSE p1  ^.Balance:=0;
              Node:=p2;
             END;
            Node^.Balance:=0;
           END;
        END;
      END;
   0:IF Node^.Defined THEN
      IF Protest THEN WrXError(1815,Neu^.Name^)
      ELSE
       BEGIN
	ClearMacroRec(Node^.Contents); Node^.Contents:=Neu;
	Node^.DefSection:=DefSect;
       END
     ELSE WITH Node^ DO
      BEGIN
       ClearMacroRec(Contents); Contents:=Neu;
       DefSection:=DefSect; Defined:=True;
      END;
   END;
END;

BEGIN
   IF NOT CaseSensitive THEN NLS_UpString(Neu^.Name^);
   IF AddNode(MacroRoot) THEN;
END;

	FUNCTION FoundMacro(VAR Erg:PMacroRec):Boolean;
VAR
   Lauf:PSaveSection;
   Part:String;

	FUNCTION FNode(Handle:LongInt):Boolean;
VAR
   Lauf:PMacroNode;
   CErg:ShortInt;
BEGIN
   Lauf:=MacroRoot; CErg:=2;
   WHILE (Lauf<>Nil) AND (CErg<>0) DO
    BEGIN
     CErg:=StrCmp(Part,Lauf^.Contents^.Name^,Handle,Lauf^.DefSection);
     CASE CErg OF
     -1:Lauf:=Lauf^.Left;
     1:Lauf:=Lauf^.Right;
     END;
    END;
   IF Lauf<>Nil THEN Erg:=Lauf^.Contents;
   FNode:=Lauf<>Nil;
END;

BEGIN
   FoundMacro:=True;
   Part:=LOpPart; IF NOT CaseSensitive THEN NLS_UpString(Part);
   IF FNode(MomSectionHandle) THEN Exit;
   Lauf:=SectionStack;
   WHILE Lauf<>Nil DO
    BEGIN
     IF FNode(Lauf^.Handle) THEN Exit;
     Lauf:=Lauf^.Next;
    END;
   FoundMacro:=False;
END;

	PROCEDURE ClearMacroList;

	PROCEDURE ClearNode(VAR Node:PMacroNode);
BEGIN
   ChkStack;

   IF Node=Nil THEN Exit;

   ClearNode(Node^.Left); ClearNode(Node^.Right);

   ClearMacroRec(Node^.Contents); Dispose(Node); Node:=Nil;
END;

BEGIN
   ClearNode(MacroRoot);
END;

	PROCEDURE ResetMacroDefines;

	PROCEDURE ResetNode(Node:PMacroNode);
BEGIN
   ChkStack;

   IF Node=Nil THEN Exit;

   ResetNode(Node^.Left); ResetNode(Node^.Right);
   Node^.Defined:=False;
END;

BEGIN
   ResetNode(MacroRoot);
END;

	PROCEDURE ClearMacroRec(VAR Alt:PMacroRec);
BEGIN
   WITH Alt^ DO
    BEGIN
     FreeMem(Name,Succ(Length(Name^)));
     ClearStringList(FirstLine);
    END;
   Dispose(Alt); Alt:=Nil;
END;

	PROCEDURE PrintMacroList;
VAR
   OneS:String;
   cnt:Boolean;
   Sum:LongInt;

        PROCEDURE PNode(Node:PMacroNode);
VAR
   h:String;
BEGIN
   WITH Node^ DO
    BEGIN
     h:=Contents^.Name^;
     IF DefSection<>-1 THEN h:=h+' ['+GetSectionName(DefSection)+']';
     OneS:=OneS+h;
     IF Length(h)<37 THEN OneS:=OneS+Blanks(37-Length(h));
     IF NOT cnt THEN OneS:=OneS+' | '
     ELSE
      BEGIN
       WrLstLine(OneS); OneS:='';
      END;
     cnt:=NOT cnt; Inc(Sum);
    END;
END;

	PROCEDURE PrintNode(Node:PMacroNode);
BEGIN
   IF Node=Nil THEN Exit;
   ChkStack;

   PrintNode(Node^.Left);

   PNode(Node);

   PrintNode(Node^.Right);
END;

BEGIN
   IF MacroRoot=NIL THEN Exit;

   NewPage(ChapDepth,True);
   WrLstLine(ListMacListHead1);
   WrLstLine(ListMacListHead2);
   WrLstLine('');

   OneS:=''; cnt:=False; sum:=0; PrintNode(MacroRoot);
   IF cnt THEN
    BEGIN
     Dec(Byte(OneS[0]),2);
     WrLstLine(OneS);
    END;
   WrLstLine('');
   Str(sum:7,OneS);
   IF sum=1 THEN OneS:=OneS+ListMacSumMsg ELSE OneS:=OneS+ListMacSumsMsg;
   WrLstLine(OneS);
   WrLstLine('');
END;

{==== Eingabefilter Makroprozessor =========================================}

BEGIN
   MacroRoot:=Nil;
END.
