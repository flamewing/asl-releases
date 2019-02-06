	PROGRAM AS2Msg;

{$i as2msg.rsc}

VAR
   Line:String;
   MsgType,FName,ErrName,LName:String;
   LastFile,MainFile:String;
   LNo:Word;
   p,Err:Integer;
   StatAct:Boolean;

	PROCEDURE KillBlanks(VAR s:String);
VAR
   p:Integer;
BEGIN
   REPEAT
    p:=Pos(' ',s); IF p=0 THEN p:=Pos(#9,s);
    IF p<>0 THEN Delete(s,p,1);
   UNTIL p=0;
END;

{ nÑchste Meldungen gehîren zu anderer Datei }

	PROCEDURE SetFile(NewFile:String);
BEGIN
{   IF LastFile<>NewFile THEN }   { muss wohl doch immer geschickt werden }
    BEGIN
     Write(#0,NewFile,#0);
     LastFile:=NewFile;
    END;
END;

{ Statistikzeile ausgeben -- Dummy-Zeilennummer 1 }

	PROCEDURE WriteStat;
BEGIN
   SetFile(MainFile);
   Write(#1, #1,#0, #1,#0, Line,#0);
END;

BEGIN
   { Dateinamencache lîschen }

   LastFile:=''; StatAct:=False;

   { Header schreiben }

   Write('BI#PIP#OK'#0);

   { Zeilen lesen, bis fertig }

   WHILE NOT EOF(Input) DO
    BEGIN
     ReadLn(Line);

     { Leerzeilen bestimmt nicht }

     IF Line<>'' THEN
      BEGIN
       { 1. Zeile, die Hauptdateinamen enthÑlt }

       IF Pos(MainMark,Line)=1 THEN
	BEGIN
	 Delete(Line,1,Length(MainMark)+1); KillBlanks(Line);
	 MainFile:=Line;
	END

       { Beginn Statistik }

       ELSE IF Pos(StartStat,Line)<>0 THEN
	BEGIN
	 WriteStat;
	 StatAct:=True;
	END

       { Rest/Ende Statistik }

       ELSE IF StatAct THEN
	BEGIN
	 WriteStat;
	 IF Pos(EndStat,Line)<>0 THEN StatAct:=False;
	END

       { Rest Fehlermeldungen }

       ELSE IF Line[1]<>' ' THEN
	BEGIN
	 { Dateinamen abtennen=GÅltigkeitsprÅfung }

	 p:=Pos(':',Line);

	 IF p<>0 THEN
	  BEGIN
	   { Dateinamen abtrennen & ausgeben }

	   p:=Pos('(',Line);
	   FName:=Copy(Line,1,p-1); Delete(Line,1,p); KillBlanks(FName);
	   SetFile(FName);

	   { Zeilennummer }

	   p:=Pos(')',Line); LName:=Copy(Line,1,p-1); Delete(Line,1,p);
	   Val(LName,LNo,Err);

	   { Doppelpunkt beseitigen }

	   WHILE (Line<>'') AND (Line[1] IN [#9,' ']) DO Delete(Line,1,1);
	   IF (Line<>'') AND (Line[1]=':') THEN Delete(Line,1,1);

	   IF Err=0 THEN
	    BEGIN
	     Write(#1,Chr(Lo(LNo)),Chr(Hi(LNo)),#0,#1,Line,#0);
	    END;
	  END;
	END;
      END;
    END;

   { Terminierzeichen schreiben }

   Write(#127);
END.
