{$i STDINC.PAS}
	UNIT AsmDeb;


INTERFACE

        USES StringUt,
	     AsmDef,AsmSub,AsmPars,FileNums;

	PROCEDURE AddLineInfo(InAsm:Boolean; LineNum:LongInt;
	                      VAR FileName:String; Space:ShortInt;
			      Address:LongInt);

	PROCEDURE InitLineInfo;

        PROCEDURE ClearLineInfo;

        PROCEDURE DumpDebugInfo;


IMPLEMENTATION

TYPE
   TLineInfo=RECORD
              InASM:Boolean;
              LineNum:LongInt;
              FileName:Integer;
              Space:ShortInt;
              Address:LongInt;
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

	PROCEDURE AddLineInfo(InAsm:Boolean; LineNum:LongInt;
	                      VAR FileName:String; Space:ShortInt;
			      Address:LongInt);
VAR
   P,Run,Prev:PLineInfoList;
BEGIN
   New(P);
   P^.Contents.InASM:=InASM;
   P^.Contents.LineNum:=LineNum;
   P^.Contents.FileName:=GetFileNum(FileName);
   P^.Contents.Space:=Space;
   P^.Contents.Address:=Address;

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

        PROCEDURE DumpDebugInfo;
BEGIN
   CASE DebugMode OF
   DebugMAP:DumpDebugInfo_MAP;
   END;
END;

END.
