{$r-,n+,v-,s-}

        UNIT StdHandl;

{***************************************************************************}
{*                                                                         *}
{*   STDHANDLES - Eine Unit, die die von C her bekannten Standardhandles   *}
{*                nachliefert                                              *}
{*                                                                         *}
{*   Autor: Alfred Arnold               E-Mail: a.arnold@kfa-juelich.de    *}
{*          Praelat-Lewen-Str.3                                            *}
{*          53819 Neunkirchen-1                                            *}
{*                                                                         *}
{*   Diese Unit ist sowohl zur Benutzung unter DOS, DPMI oder auch OS/2    *}
{*   konzipiert.  Sie ist frei kopier- und benutzbar!                      *}
{*                                                                         *}
{*   Literatur: - Arne Schaepers:                                          *}
{*                DOS 5 fuer Programmierer - Die endgueltige Referenz      *}
{*                Addison-Weseley 1991                                     *}
{*              - IBM Operating System/2 Programming Tools an Information: *}
{*                Version 1.2 Control Program Programming Reference        *}
{*                IBM Corporation 1989                                     *}
{*                                                                         *}
{***************************************************************************}

{  AH=5A
   CX=Flags: 0=R/O, 1=Hidden, 2=SYS, 4=Achiv
   DS:DX=@ASCIIZ-String mit Dir, wird um Namen erweitert }

INTERFACE

        USES {$IFDEF VIRTUALPASCAL}
             OS2Def,OS2Base,
             {$ENDIF}
             {$IFDEF WINDOWS}
             WinDOS;
             {$ELSE}
             DOS;
             {$ENDIF}

VAR
   Redirected:(NoRedir,RedirToDevice,RedirToFile);  { wohin zeigt StdOut ? }

   StdErr,StdPrn,StdAux:Text;                       { die fehlenden Dinge }

        PROCEDURE RewriteStandard(VAR T:Text; Path:String);

        PROCEDURE SetFileMode(NewMode:Word);

IMPLEMENTATION

CONST
   NumStdIn  = 0;    { Die Nummern der DOS-Standardhandles }
   NumStdOut = 1;
   NumStdErr = 2;
   NumStdPrn = 4;
   NumStdAux = 3;

{$IFDEF WINDOWS}
TYPE
   Registers=TRegisters;
   TextRec=TTextRec;
{$ENDIF}

VAR
   {$IFDEF OS2}
    {$IFDEF VIRTUALPASCAL}
   OS2Erg:ApiRet;
   HandType,DevAttr:ULong;
    {$ELSE}
   OS2Erg,HandType,DevAttr:Word;
    {$ENDIF}
   {$ELSE}
   Regs:Registers;
   {$ENDIF}


{ die unter BP-OS/2 benoetigten DLL-Calls }

{$IFDEF OS2}
 {$IFNDEF VIRTUALPASCAL}
        FUNCTION DosQHandType(Handle:Word;
                              VAR HandType:Word;
                              VAR FlagWord:Word):Word; Far;
        EXTERNAL 'DOSCALLS' INDEX 77;
 {$ENDIF}
{$ENDIF}


{ eine Textvariable auf einen der Standardkanaele umbiegen.  Die Reduzierung
  der Puffergroesse auf fast Null verhindert, daá durch Pufferung evtl. Ausgaben
  durcheinadergehen. }

        PROCEDURE AssignHandle(VAR T:Text; Num:Word);
BEGIN
   Assign(T,''); {$i-} Rewrite(T); {$i+}
   TextRec(T).BufSize:=1;
   IF Num<>NumStdOut THEN TextRec(T).Handle:=Num;
END;

{ Eine Datei unter Beruecksichtigung der Standardkanaele oeffnen }

        PROCEDURE RewriteStandard(VAR T:Text; Path:String);
BEGIN
   IF (Length(Path)=2) AND (Path[1]='!') AND (Path[2]>='0') AND (Path[2]<='4') THEN
    AssignHandle(T,Ord(Path[2])-Ord('0'))
   ELSE
    BEGIN
     Assign(T,Path); {$i-} Rewrite(T); {$i+}
    END;
END;

{****************************************************************************}
{ Dateizugriffsmodus setzen }

        PROCEDURE SetFileMode(NewMode:Word);
BEGIN
   {$IFDEF OS2}
   FileMode:=64+NewMode;
   {$ELSE}
   FileMode:=NewMode;
   {$ENDIF}
END;

VAR
   h1,h2:Word;
BEGIN
   { Standardkanaele oeffnen }

   AssignHandle(StdErr,NumStdErr);
   AssignHandle(StdPrn,NumStdPrn);
   AssignHandle(StdAux,NumStdAux);

   { wohin zeigt die Standardausgabe ? }

   {$IFDEF OS2}
    {$IFDEF VIRTUALPASCAL}

   OS2Erg:=DosQueryHType(TextRec(Output).Handle,HandType,DevAttr);
   IF HandType AND $ff=fht_DiskFile THEN Redirected:=RedirToFile
   ELSE IF DevAttr AND 2=0 THEN Redirected:=RedirToDevice
   ELSE Redirected:=NoRedir;


    {$ELSE}

   OS2Erg:=DosQHandType(TextRec(Output).Handle,HandType,DevAttr);
   IF HandType AND 7=0 THEN Redirected:=RedirToFile
   ELSE IF DevAttr AND 2=0 THEN Redirected:=RedirToDevice
   ELSE Redirected:=NoRedir;

    {$ENDIF}
   {$ELSE}

   WITH Regs DO
    BEGIN
     BX:=TextRec(Output).Handle; AX:=$4400;
     Msdos(Regs);
     IF DX AND 2=2 THEN Redirected:=NoRedir
     ELSE IF DX AND $8000=0 THEN Redirected:=RedirToFile
     ELSE Redirected:=RedirToDevice;
    END;

   {$ENDIF}
END.

