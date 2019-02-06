{$i STDINC.PAS}
	UNIT AsmDeb;


INTERFACE

        USES StringUt,
	     AsmDef,AsmSub,AsmPars,FileNums;

        PROCEDURE AddLineInfo(InMacro:Boolean; LineNum:LongInt;
	                      VAR FileName:String; Space:ShortInt;
			      Address:LongInt);

	PROCEDURE InitLineInfo;

        PROCEDURE ClearLineInfo;

        PROCEDURE DumpDebugInfo;


IMPLEMENTATION

TYPE
   TLineInfo=RECORD
              InMacro:Boolean;
              LineNum:LongInt;
              FileName:Integer;
              Space:ShortInt;
              Address:LongInt;
              Code:Word;
             END;

   PLineInfoList=^TLineInfoList;
   TLineInfoList=RECORD
                  Next:PLineInfoList;
                  Contents:TLineInfo;
                 END;

VAR
   TempFileName:String;
   TempFile:FILE OF TLineInfo;
   LineInfoRoot:PLineInfoList;

        PROCEDURE AddLineInfo(InMacro:Boolean; LineNum:LongInt;
	                      VAR FileName:String; Space:ShortInt;
			      Address:LongInt);
VAR
   P,Run,Prev:PLineInfoList;
BEGIN
   New(P);
   P^.Contents.InMacro:=InMacro;
   P^.Contents.LineNum:=LineNum;
   P^.Contents.FileName:=GetFileNum(FileName);
   P^.Contents.Space:=Space;
   P^.Contents.Address:=Address;
   IF CodeLen<1 THEN p^.Contents.Code:=0 ELSE p^.Contents.Code:=WAsmCode[0];

   Run:=LineInfoRoot;
   IF Run=Nil THEN
    BEGIN
     LineInfoRoot:=P; P^.Next:=Nil;
    END
   ELSE
    BEGIN
     WHILE (Run^.Next<>Nil) AND (Run^.Next^.Contents.Space<Space) DO Run:=Run^.Next;
     WHILE (Run^.Next<>Nil) AND (Run^.Next^.Contents.FileName<P^.Contents.FileName) DO Run:=Run^.Next;
     WHILE (Run^.Next<>Nil) AND (Run^.Next^.Contents.Address<Address) DO Run:=Run^.Next;
     P^.Next:=Run^.Next; Run^.Next:=P;
    END
END;

	PROCEDURE InitLineInfo;
BEGIN
   TempFileName:=''; LineInfoRoot:=Nil;
END;

        PROCEDURE ClearLineInfo;
VAR
   Run:PLineInfoList;
BEGIN
   IF TempFileName<>'' THEN
    BEGIN
     Close(TempFile); Erase(TempFile);
    END;

   WHILE LineInfoRoot<>Nil DO
    BEGIN
     Run:=LineInfoRoot; LineInfoRoot:=LineInfoRoot^.Next;
     Dispose(Run);
    END;

   InitLineInfo;
END;

	PROCEDURE DumpDebugInfo_MAP;
VAR
   Run:PLineInfoList;
   ActFile,ModZ:Integer;
   ActSeg:ShortInt;
   MAPFile:Text;
   MAPName:String;
BEGIN
   MAPName:=SourceFile; KillSuffix(MAPName); AddSuffix(MAPName,MAPSuffix);
   Assign(MAPFile,MAPName); Rewrite(MAPFile); ChkIO(10001);

   Run:=LineInfoRoot; ActSeg:=-1; ActFile:=-1; ModZ:=0;
   WHILE Run<>Nil DO
    BEGIN
     WITH Run^.Contents DO
      BEGIN
       IF Space<>ActSeg THEN
        BEGIN
         ActSeg:=Space;
         IF ModZ<>0 THEN
          BEGIN
           WriteLn(MAPFile); ChkIO(10004);
          END;
         ModZ:=0;
         WriteLn(MAPFile,'Segment ',SegNames[ActSeg]); ChkIO(10004);
         ActFile:=-1;
        END;
       IF FileName<>ActFile THEN
        BEGIN
         ActFile:=FileName;
         IF ModZ<>0 THEN
          BEGIN
           WriteLn(MAPFile); ChkIO(10004);
          END;
         ModZ:=0;
         WriteLn(MAPFile,'File ',GetFileName(FileName)); ChkIO(10004);
        END;
       Write(MAPFile,Run^.Contents.LineNum:5,':',HexString(Run^.Contents.Address,8),' '); ChkIO(10004);
       Inc(ModZ);
       IF ModZ=5 THEN
        BEGIN
         WriteLn(MAPFile); ChkIO(10004); ModZ:=0;
        END;
      END;
     Run:=Run^.Next;
    END;
   IF ModZ<>0 THEN
    BEGIN
     WriteLn(MAPFile); ChkIO(10004);
    END;

   PrintDebSymbols(MAPFile);

   PrintDebSections(MAPFile);

   Close(MAPFile);
END;

        PROCEDURE DumpDebugInfo_Atmel;
CONST
   OBJString:String[20]='AVR Object File';
VAR
   Run:PLineInfoList;
   FNamePos,RecPos:LongInt;
   OBJFile:File;
   OBJName,FName:String;
   TByte,TNum,NameCnt:Byte;
   z:Integer;
   TurnField:ARRAY[0..3] OF Byte;
BEGIN
   {$i-}
   OBJName:=SourceFile; KillSuffix(OBJName); AddSuffix(OBJName,OBJSuffix);
   Assign(OBJFile,OBJName); Rewrite(OBJFile,1); ChkIO(10001);

   { initialer Kopf, Positionen noch unbekannt }

   FNamePos:=0; RecPos:=0;
   BlockWrite(OBJFile,FNamePos,4); ChkIO(10004);
   BlockWrite(OBJFile,RecPos,4); ChkIO(10004);
   TByte:=9; BlockWrite(OBJFile,TByte,1); ChkIO(10004);
   NameCnt:=GetFileCount-1; BlockWrite(OBJFile,NameCnt,1); ChkIO(10004);
   BlockWrite(OBJFile,OBJString[1],Length(OBJString));
   TByte:=0; BlockWrite(OBJFile,TByte,1); ChkIO(10004);

   { Objekt-Records }

   RecPos:=FilePos(OBJFile);
   Run:=LineInfoRoot;
   WHILE Run<>Nil DO
    BEGIN
     IF Run^.Contents.Space=SegCode THEN
      BEGIN
       Move(Run^.Contents.Address,TurnField,4);
       TNum:=TurnField[0]; TurnField[0]:=TurnField[3]; TurnField[3]:=TNum;
       TNum:=TurnField[1]; TurnField[1]:=TurnField[2]; TurnField[2]:=TNum;
       BlockWrite(OBJFile,TurnField[1],3); ChkIO(10004);
       Move(Run^.Contents.Code,TurnField,2);
       TNum:=TurnField[0]; TurnField[0]:=TurnField[1]; TurnField[1]:=TNum;
       BlockWrite(OBJFile,TurnField,2); ChkIO(10004);
       TNum:=Run^.Contents.FileName-1; BlockWrite(OBJFile,TNum,1); ChkIO(10004);
       Move(Run^.Contents.LineNum,TurnField,2);
       TNum:=TurnField[0]; TurnField[0]:=TurnField[1]; TurnField[1]:=TNum;
       BlockWrite(ObjFile,TurnField,2);
       TNum:=Ord(Run^.Contents.InMacro); BlockWrite(OBJFile,TNum,1); ChkIO(10004);
      END;
     Run:=Run^.Next;
    END;

   { Dateinamen }

   FNamePos:=FilePos(OBJFile);
   TByte:=0;
   FOR z:=1 TO NameCnt DO
    BEGIN
     FName:=GetFileName(z); FName:=NamePart(FName);
     BlockWrite(OBJFile,FName[1],Length(FName)); ChkIO(10004);
     BlockWrite(OBJFile,TByte,1); ChkIO(10004);
    END;
   BlockWrite(OBJFile,TByte,1); ChkIO(10004);

   { korrekte Positionen in Kopf schreiben }

   Seek(OBJFile,0);
   Move(FNamePos,TurnField,4);
   TNum:=TurnField[0]; TurnField[0]:=TurnField[3]; TurnField[3]:=TNum;
   TNum:=TurnField[1]; TurnField[1]:=TurnField[2]; TurnField[2]:=TNum;
   BlockWrite(OBJFile,TurnField,4); ChkIO(10004);
   Move(RecPos,TurnField,4);
   TNum:=TurnField[0]; TurnField[0]:=TurnField[3]; TurnField[3]:=TNum;
   TNum:=TurnField[1]; TurnField[1]:=TurnField[2]; TurnField[2]:=TNum;
   BlockWrite(OBJFile,TurnField,4); ChkIO(10004);

   Close(ObjFile);
   {$i+}
END;

        PROCEDURE DumpDebugInfo;
BEGIN
   CASE DebugMode OF
   DebugMAP:DumpDebugInfo_MAP;
   DebugAtmel:DumpDebugInfo_Atmel;
   END;
END;

END.
