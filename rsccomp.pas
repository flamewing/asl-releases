        PROGRAM RSCComp;
        USES StdHandl;

TYPE
   PRSCEntry=^TRSCEntry;
   TRSCEntry=RECORD
              Next:PRSCEntry;
              Number,Offset:LongInt;
              Contents:^String;
             END;

VAR
   SrcFile:Text;
   DestFile:File;

   Entries:PRSCEntry;
   EntryCnt,StringSize:LongInt;

        PROCEDURE ReadSource;
VAR
   SrcFile:Text;
   LineCounter:LongInt;
   SrcLine,StrNo:String;
   IDNo:LongInt;
   ErrNo,p:Integer;
   Neu,Curr:PRSCEntry;

        PROCEDURE Terminate(CONST Msg:String);
BEGIN
   WriteLn(Msg,' in line ',LineCounter);
   Halt(1);
END;

        PROCEDURE CutPart(Start:Integer; VAR erg:String);
VAR
   p:Integer;
BEGIN
   p:=Start+1;
   WHILE (p<=Length(SrcLine)) AND (SrcLine[p]<>'}') DO Inc(p);
   IF p>Length(SrcLine) THEN Terminate('no closing }');
   erg:=Copy(SrcLine,Start+1,p-Start-1);
   Delete(SrcLine,Start,p-Start+1);
END;

        PROCEDURE InsSpecial(Position:Integer; VAR Ident:String);
TYPE
   IdentRec=RECORD
             Name:String[10];
             Erg:Char;
            END;
CONST
   IdentCnt=7;
   IdentRecs:ARRAY[0..IdentCnt-1] OF IdentRec=
             ((Name:'auml'     ;Erg:'Ñ'),
              (Name:'ouml'     ;Erg:'î'),
              (Name:'uuml'     ;Erg:'Å'),
              (Name:'Auml'     ;Erg:'é'),
              (Name:'Ouml'     ;Erg:'ô'),
              (Name:'Uuml'     ;Erg:'ö'),
              (Name:'ssharp'   ;Erg:'·'));
VAR
   Ch:Char;
   z:Integer;
BEGIN
   z:=0;
   WHILE (z<IdentCnt) AND (IdentRecs[z].Name<>Ident) DO Inc(z);
   IF z>=IdentCnt THEN Terminate('invalid special character sequence');
   Insert(IdentRecs[z].Erg,SrcLine,Position);
END;

BEGIN
   Assign(SrcFile,ParamStr(1)); SetFileMode(0); Reset(SrcFile);
   LineCounter:=0;  Entries:=Nil;
   WHILE NOT EOF(SrcFile) DO
    BEGIN
     ReadLn(SrcFile,SrcLine); Inc(LineCounter);
     IF (Length(SrcLine)>0) AND (SrcLine[1]<>';') THEN
      BEGIN
       IF SrcLine[1]<>'{' THEN Terminate('no message id');
       CutPart(1,StrNo); Val(StrNo,IDNo,ErrNo);
       IF ErrNo<>0 THEN Terminate('invalid id number');
       p:=Pos('\',SrcLine);
       WHILE p<>0 DO
        BEGIN
         Delete(SrcLine,p,1);
         IF SrcLine[p]<>'{' THEN
          BEGIN
           StrNo:=Copy(SrcLine,p,1);
           Delete(SrcLine,p,1);
          END
         ELSE CutPart(p,StrNo);
         InsSpecial(p,StrNo);
         p:=Pos('\',SrcLine);
        END;
       New(Neu); Neu^.Number:=IDNo; Neu^.Offset:=0;
       GetMem(Neu^.Contents,Length(SrcLine)+1); Neu^.Contents^:=SrcLine;
       IF (Entries=Nil) OR (Entries^.Number>=IDNo) THEN
        BEGIN
         Neu^.Next:=Entries; Entries:=Neu;
        END
       ELSE
        BEGIN
         Curr:=Entries;
         WHILE (Curr^.Next<>Nil) AND (Curr^.Next^.Number<IDno) DO
          Curr:=Curr^.Next;
         Neu^.Next:=Curr^.Next; Curr^.Next:=Neu;
        END;
      END;
    END;
   Close(SrcFile);
END;

        PROCEDURE PrintEntries;
VAR
   Curr:PRSCEntry;
BEGIN
   Curr:=Entries;
   WHILE Curr<>Nil DO
    BEGIN
     WriteLn(Curr^.Number,'-->',Curr^.Contents^);
     Curr:=Curr^.Next;
    END;
END;

        PROCEDURE LocateItems;
VAR
   Curr:PRSCEntry;
   FilePos,LastItem:LongInt;
BEGIN
   EntryCnt:=0; FilePos:=0;
   Curr:=Entries; LastItem:=-1;
   WHILE Curr<>Nil DO
    BEGIN
     Inc(EntryCnt);
     IF LastItem=Curr^.Number THEN
      WriteLn('warning: item ',LastItem,' double defined');
     Curr^.Offset:=FilePos;
     Inc(FilePos,Length(Curr^.Contents^)+2);
     LastItem:=Curr^.Number;
     Curr:=Curr^.Next;
    END;
   StringSize:=FilePos;
END;

        PROCEDURE WriteDest;
VAR
   DestFile:File;
   Curr:PRSCEntry;
   CTerm:Byte;
BEGIN
   Assign(DestFile,ParamStr(2)); Rewrite(DestFile,1);
   BlockWrite(DestFile,EntryCnt,Sizeof(EntryCnt));
   BlockWrite(DestFile,StringSize,Sizeof(StringSize));
   Curr:=Entries;
   WHILE Curr<>Nil DO
    BEGIN
     BlockWrite(DestFile,Curr^.Number,Sizeof(Curr^.Number));
     BlockWrite(DestFile,Curr^.Offset,Sizeof(Curr^.Offset));
     Curr:=Curr^.Next;
    END;
   Curr:=Entries; CTerm:=0;
   WHILE Curr<>Nil DO
    BEGIN
     BlockWrite(DestFile,Curr^.Contents^,Length(Curr^.Contents^)+1);
     BlockWrite(DestFile,CTerm,1);
     Curr:=Curr^.Next;
    END;
   Close(DestFile);
END;

BEGIN
   IF ParamCount<>2 THEN
    BEGIN
     WriteLn(StdErr,'calling convention: rsccomp <Quelldatei> <Zieldatei>');
     Halt(1);
    END;

   ReadSource;
   LocateItems;
   WriteDest;
END.
