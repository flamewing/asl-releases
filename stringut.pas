{$i STDINC.PAS}
{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}
	UNIT StringUt;

INTERFACE

VAR
   HexLowerCase:Boolean;	    { Hex-Konstanten mit Kleinbuchstaben }

        FUNCTION  Blanks(cnt:Integer):String;

        FUNCTION  HexString(i:LongInt; Stellen:Byte):String;

        FUNCTION  HexBlankString(i:LongInt; Stellen:Byte):String;

IMPLEMENTATION

{----------------------------------------------------------------------------}
{ eine bestimmte Anzahl Leerzeichen liefern }

       FUNCTION Blanks(cnt:Integer):String;
CONST
   BlkStr='                                                      ';
BEGIN
   Blanks:=Copy(BlkStr,1,cnt);
END;


{****************************************************************************}
{ eine Integerzahl in eine Hexstring umsetzen. Weitere vordere Stellen als }
{ Nullen }

        FUNCTION HexString(i:LongInt; Stellen:Byte):String;
CONST
   UDigitVals='0123456789ABCDEF';
   LDigitVals='0123456789abcdef';
VAR
   h:String[8];
   d:Byte;
BEGIN
   IF Stellen>8 THEN Stellen:=8;
   h:='';
   REPEAT
    IF HexLowerCase THEN
     h:=Copy(LDigitVals,(i AND 15)+1,1)+h
    ELSE
     h:=Copy(UDigitVals,(i AND 15)+1,1)+h;
    i:=i SHR 4;
   UNTIL (i=0) AND (Length(h)>=Stellen);
   HexString:=h;
END;


{----------------------------------------------------------------------------}
{ dito, nur vorne Leerzeichen }

        FUNCTION HexBlankString(i:LongInt; Stellen:Byte):String;
VAR
   temp:String;
BEGIN
   temp:=HexString(i,0);
   IF Length(temp)>=Stellen THEN HexBlankString:=temp
   ELSE HexBlankString:=Blanks(Stellen-Length(temp))+temp;
END;


BEGIN
   HexLowerCase:=False;
END.
