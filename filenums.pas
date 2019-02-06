{$i STDINC.PAS}
{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}

        UNIT FileNums;

INTERFACE

        USES AsmDef;

        PROCEDURE InitFileList;

        PROCEDURE ClearFileList;

        PROCEDURE AddFile(FName:String);

        FUNCTION GetFileNum(Name:String):Integer;

        FUNCTION GetFileName(Num:Byte):String;

        FUNCTION GetFileCount:Integer;

IMPLEMENTATION

TYPE
   PToken=^TToken;
   TToken=RECORD
           Next:PToken;
           Name:StringPtr;
          END;

VAR
   FirstFile:PToken;

        PROCEDURE InitFileList;
BEGIN
   FirstFile:=Nil;
END;

        PROCEDURE ClearFileList;
VAR
   F:PToken;
BEGIN
   WHILE FirstFile<>Nil DO
    BEGIN
     F:=FirstFile^.Next;
     FreeMem(FirstFile^.Name,Length(FirstFile^.Name^)+1);
     Dispose(FirstFile);
     FirstFile:=F;
    END;
END;

        PROCEDURE AddFile(FName:String);
VAR
   Lauf,Neu:PToken;
BEGIN
   IF GetFileNum(FName)<>-1 THEN Exit;

   New(Neu);
   Neu^.Next:=Nil;
   GetMem(Neu^.Name,Length(FName)+1); Neu^.Name^:=FName;
   IF FirstFile=Nil THEN FirstFile:=Neu
   ELSE
    BEGIN
     Lauf:=FirstFile;
     WHILE Lauf^.Next<>Nil DO Lauf:=Lauf^.Next;
     Lauf^.Next:=Neu;
    END;
END;

        FUNCTION GetFileNum(Name:String):Integer;
VAR
   FLauf:PToken;
   Cnt:Integer;
BEGIN
   Cnt:=0; FLauf:=FirstFile;
   WHILE (FLauf<>Nil) AND (FLauf^.Name^<>Name) DO
    BEGIN
     Inc(Cnt);
     FLauf:=FLauf^.Next;
    END;
   IF FLauf=Nil THEN GetFileNum:=-1 ELSE GetFileNum:=Cnt;
END;

        FUNCTION GetFileName(Num:Byte):String;
VAR
   Lauf:PToken;
   z:Integer;
BEGIN
   Lauf:=FirstFile;
   FOR z:=1 TO Num DO
    IF Lauf<>Nil THEN Lauf:=Lauf^.Next;
   IF Lauf=Nil THEN GetFileName:=''
   ELSE GetFileName:=Lauf^.Name^;
END;

        FUNCTION GetFileCount:Integer;
VAR
   Lauf:PToken;
   z:Integer;
BEGIN
   Lauf:=FirstFile; z:=0;
   WHILE Lauf<>Nil DO
    BEGIN
     Inc(z); Lauf:=Lauf^.Next;
    END;
   GetFileCount:=z;
END;

BEGIN
   FirstFile:=Nil;
END.

