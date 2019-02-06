{$I STDINC.PAS}

        UNIT StringLi;

INTERFACE

TYPE
   StringRecPtr=^StringRec;
   StringRec=RECORD
              Next:StringRecPtr;
              Content:^String;
             END;
   StringList=StringRecPtr;

	PROCEDURE InitStringList(VAR List:StringList);

        PROCEDURE ClearStringEntry(VAR Elem:StringRecPtr);

        PROCEDURE ClearStringList(VAR List:StringList);

        PROCEDURE AddStringListFirst(VAR List:StringList; NewStr:String);

        PROCEDURE AddStringListLast(VAR List:StringList; NewStr:String);

        PROCEDURE RemoveStringList(VAR List:StringList; OldStr:String);

        FUNCTION  GetStringListFirst(List:StringList; VAR Lauf:StringRecPtr):String;

        FUNCTION  GetStringListNext(VAR Lauf:StringRecPtr):String;

	FUNCTION  GetAndCutStringList(VAR List:StringList):String;

        FUNCTION  StringListEmpty(List:StringList):Boolean;

        FUNCTION  DuplicateStringList(Src:StringList):StringList;

        FUNCTION  StringListPresent(List:StringList; VAR Search:String):Boolean;

IMPLEMENTATION

	PROCEDURE InitStringList(VAR List:StringList);
BEGIN
   List:=Nil;
END;

        PROCEDURE ClearStringEntry(VAR Elem:StringRecPtr);
BEGIN
   FreeMem(Elem^.Content,Length(Elem^.Content^)+1);
   Dispose(Elem); Elem:=Nil;
END;

        PROCEDURE ClearStringList(VAR List:StringList);
VAR
   Hilf:StringRecPtr;
BEGIN
   WHILE List<>Nil DO
    BEGIN
     Hilf:=List; List:=List^.Next;
     ClearStringEntry(Hilf);
    END;
END;

        PROCEDURE AddStringListFirst(VAR List:StringList; NewStr:String);
VAR
   Neu:StringRecPtr;
BEGIN
   New(Neu); GetMem(Neu^.Content,Length(NewStr)+1);
   WITH Neu^ DO
    BEGIN
     Content^:=NewStr; Next:=List;
    END;
   List:=Neu;
END;

        PROCEDURE AddStringListLast(VAR List:StringList; NewStr:String);
VAR
   Neu,Lauf:StringRecPtr;
BEGIN
   New(Neu); GetMem(Neu^.Content,Length(NewStr)+1);
   WITH Neu^ DO
    BEGIN
     Content^:=NewStr; Next:=Nil;
    END;
   IF List=Nil THEN List:=Neu
   ELSE
    BEGIN
     Lauf:=List; WHILE Lauf^.Next<>Nil DO Lauf:=Lauf^.Next;
     Lauf^.Next:=Neu;
    END;
END;

        PROCEDURE RemoveStringList(VAR List:StringList; OldStr:String);
VAR
   Lauf,Hilf:StringRecPtr;
BEGIN
   IF List=Nil THEN Exit;
   IF List^.Content^=OldStr THEN
    BEGIN
     Hilf:=List; List:=List^.Next; ClearStringEntry(Hilf);
    END
   ELSE
    BEGIN
     Lauf:=List;
     WHILE (Lauf^.Next<>Nil) AND (Lauf^.Next^.Content^<>OldStr) DO Lauf:=Lauf^.Next;
     IF Lauf^.Next<>Nil THEN
      BEGIN
       Hilf:=Lauf^.Next; Lauf^.Next:=Hilf^.Next; ClearStringEntry(Hilf);
      END;
    END;
END;

        FUNCTION GetStringListFirst(List:StringList; VAR Lauf:StringRecPtr):String;
BEGIN
   Lauf:=List;
   IF Lauf=Nil THEN GetStringListFirst:=''
   ELSE
    BEGIN
     GetStringListFirst:=Lauf^.Content^; Lauf:=Lauf^.Next;
    END;
END;

        FUNCTION GetStringListNext(VAR Lauf:StringRecPtr):String;
BEGIN
   IF Lauf=Nil THEN GetStringListNext:=''
   ELSE
    BEGIN
     GetStringListNext:=Lauf^.Content^; Lauf:=Lauf^.Next;
    END;
END;

	FUNCTION GetAndCutStringList(VAR List:StringList):String;
VAR
   Hilf:StringRecPtr;
BEGIN
   IF List=Nil THEN GetAndCutStringList:=''
   ELSE
    BEGIN
     Hilf:=List; List:=List^.Next;
     GetAndCutStringList:=Hilf^.Content^;
     ClearStringEntry(Hilf);
    END;
END;

        FUNCTION StringListEmpty(List:StringList):Boolean;
BEGIN
   StringListEmpty:=List=Nil;
END;

        FUNCTION DuplicateStringList(Src:StringList):StringList;
VAR
   Lauf:StringRecPtr;
   Dest:StringList;
BEGIN
   InitStringList(Dest);
   IF Src<>Nil THEN
    BEGIN
     AddStringListLast(Dest,GetStringListFirst(Src,Lauf));
     WHILE Lauf<>Nil DO
      AddStringListLast(Dest,GetStringListNext(Lauf));
    END;
   DuplicateStringList:=Dest;
END;

        FUNCTION StringListPresent(List:StringList; VAR Search:String):Boolean;
BEGIN
   WHILE (List<>Nil) AND (List^.Content^<>Search) DO List:=List^.Next;
   StringListPresent:=List<>Nil;
END;

END.
