{$i STDINC.PAS}

{$IFNDEF MSDOS}
{$C Moveable PreLoad Permanent }
{$ENDIF}
       	UNIT AsmDef;

INTERFACE
        USES StringLi,Chunks;


TYPE
   {$IFDEF VIRTUALPASCAL}
   ValErgType=LongInt;
   {$ELSE}
   ValErgType=Integer;
   {$ENDIF}
   CPUVar=Byte;

   PCPUDef=^TCPUDef;
   TCPUDef=RECORD
	    Next:PCPUDef;
	    Name:String[9];
	    Number,Orig:CPUVar;
	    SwitchProc:PROCEDURE;
	   END;

   TempType=(TempInt,TempFloat,TempString,TempNone);
   TempResult=RECORD CASE Typ:TempType OF
		      TempInt   :(Int:LongInt);
		      TempFloat :(Float:Extended);
		      TempString:(Ascii:String);
		     END;

   DebugType=(DebugNone,DebugMAP,DebugAtmel,DebugAOUT,DebugCOFF,DebugELF);

{$i FILEFORM.INC}

CONST
   Char_NUL=#0;
   Char_BEL=#7;
   Char_BS=#8;
   Char_HT=#9;
   Char_LF=#10;
   Char_FF=#12;
   Char_CR=#13;
   Char_ESC=#27;

   {$IFDEF WINDOWS}
   Ch_sz='ﬂ'; Ch_ae='‰';  Ch_oe='ˆ';  Ch_ue='¸';
              Ch_gae='ƒ'; Ch_goe='÷'; Ch_gue='‹';
   {$ELSE}
   Ch_sz=#225; Ch_ae=#132;  Ch_oe=#148;  Ch_ue=#129;
	       Ch_gae=#142; Ch_goe=#153; Ch_gue=#154;
   {$ENDIF}

   SrcSuffix:String[4]='.ASM';              { Standardendungen: Hauptdatei }
   IncSuffix:String[4]='.INC';              { Includedatei }
   PrgSuffix:String[4]='.P';                { Programmdatei }
   LstSuffix:String[4]='.LST';              { Listingdatei }
   MacSuffix:String[4]='.MAC';              { Makroausgabe }
   PreSuffix:String[4]='.I';                { Ausgabe Makroprozessor }
   LogSuffix:String[4]='.LOG';              { Fehlerdatei }
   MAPSuffix:String[4]='.MAP';              { Debug-Info/Map-Format }
   OBJSuffix:String[4]='.OBJ';              { dito Atmel }

   MomCPUName     :String[6]='MOMCPU';	    { mom. Prozessortyp }
   MomCPUIdentName:String[10]='MOMCPUNAME'; { mom. Prozessortyp }
   SupAllowedName           ='INSUPMODE';   { privilegierte Befehle erlaubt }
   DoPaddingName            ='PADDING';     { Padding an }
   MaximumName              ='INMAXMODE';   { CPU im Maximum-Modus }
   FPUAvailName             ='HASFPU';      { FPU-Befehle erlaubt }
   LstMacroExName           ='MACEXP';      { expandierte Makros anzeigen }
   ListOnName               ='LISTON';      { Listing an/aus }
   RelaxedName		    ='RELAXED';     { alle Zahlenschreibweisen zugelassen }
   SrcModeName              ='INSRCMODE';   { CPU im Quellcode-kompatiblen Modus }
   BigEndianName            ='BIGENDIAN';   { Datenablage MSB first }
   Has64Name                ='HAS64';       { Parser mit 64 Bit ? }
   FlagTrueName   :String[4]='TRUE';	    { Flagkonstanten }
   FlagFalseName  :String[5]='FALSE';
   PiName         :String[7]='CONSTPI';     { Zahl Pi }
   DateName       :String[4]='DATE'; 	    { Datum & Zeit }
   TimeName       :String[4]='TIME';
   VerName        :String[7]='VERSION';     { speichert Versionsnummer }
   ArchName                 ='ARCHITECTURE';{ Zielarchitektur von AS }
   CaseSensName   :String[13]='CASESENSITIVE'; { zeigt Gro·/Kleinunterscheidung an }

   AttrName                 ='ATTRIBUTE';   { Attributansprache in Makros }

   DefStackName             ='DEFSTACK';    { Default-Stack }

   Version:String[7]='1.41r7';
   StartMagic=$1b34244d;
   VerNo:LongInt=$1417;

   ArchVal:String[30]=
   {$IFDEF MSDOS}
   'i86-unknown-msdos';
   {$ENDIF}
   {$IFDEF DPMI}
   'i286-unknown-dpmi';
   {$ENDIF}
   {$IFDEF OS2}
    {$IFDEF VIRTUALPASCAL}
   'i386-unknown-os2';
    {$ELSE}
   'i286-unknown-os2';
    {$ENDIF}
   {$ENDIF}

   EnvName='ASCMD';                         { Environment-Variable fÅr Default-
					      Parameter }

   ParMax=20;

   ChapMax=4;

   StructSeg=PCMax+1;

   SegNames:ARRAY[0..PCMax] OF String[10]=('NOTHING','CODE','DATA','IDATA',
                                           'XDATA','YDATA','BITDATA','IO','REG');
   SegShorts:ARRAY[0..PCMax] OF Char=('-','C','D','I','X','Y','B','P','R');

   AscOfs=Ord('0');

   MaxCodeLen=1024;

   InfoMessCopyright:String[30]='(C) 1992,1998 Alfred Arnold';

   ValidSymChars:SET OF Char=['A'..'Z','a'..'z',#128..#165,'0'..'9','_','.'];

TYPE
   SimpProc=PROCEDURE;

   WordField=ARRAY[0..5] OF Word;              { fÅr Zahlenumwandlung }
   ArgStrField=ARRAY[1..ParMax] OF String;     { Feld mit Befehlsparametern }
   StringPtr=^String;

   TConstMode=(ConstModeIntel,	    { Hex xxxxh, Okt xxxxo, Bin xxxxb }
	       ConstModeMoto,	    { Hex $xxxx, Okt @xxxx, Bin %xxxx }
	       ConstModeC);         { Hex 0x..., Okt 0...., Bin ----- }

   PFunction=^TFunction;
   TFunction=RECORD
	      Next:PFunction;
	      ArguCnt:Byte;
              Name,Definition:StringPtr;
	     END;

   PSaveState=^TSaveState;
   TSaveState=RECORD
	       Next:PSaveState;
	       SaveCPU:CPUVar;
	       SavePC:Integer;
               SaveListOn:Byte;
	       SaveLstMacroEx:Boolean;
	      END;

   PForwardSymbol=^TForwardSymbol;
   TForwardSymbol=RECORD
		   Next:PForwardSymbol;
                   Name:StringPtr;
		   DestSection:LongInt;
		  END;

   PSaveSection=^TSaveSection;
   TSaveSection=RECORD
		 Next:PSaveSection;
		 LocSyms,GlobSyms,ExportSyms:PForwardSymbol;
		 Handle:LongInt;
		END;

   PDefinement=^TDefinement;
   TDefinement=RECORD
		Next:PDefinement;
                TransFrom,TransTo:StringPtr;
                Compiled:ARRAY[Char] OF Byte;
	       END;

   PStructure=^TStructure;
   TStructure=RECORD
               Next:PStructure;
               DoExt:Boolean;
               Name:^String;
               CurrPC:LongInt;
              END;

VAR
   SourceFile:String;                       { Hauptquelldatei }

   ClrEol:String[20];       	    { String fÅr lîschen bis Zeilenende }
   CursUp:String[10];		    {   "     "  Cursor hoch }

   PCs:ARRAY[1..StructSeg] OF LongInt;      { ProgrammzÑhler }
   StartAdr:LongInt;                        { Programmstartadresse }
   StartAdrPresent:Boolean;                 {          "           definiert? }
   Phases:ARRAY[1..StructSeg] OF LongInt;   { Verschiebungen }
   Grans:ARRAY[1..StructSeg] OF Word;       { Grî·e der Adressierungselemente }
   ListGrans:ARRAY[1..StructSeg] OF Word;   { Wortgrî·e im Listing }
   SegChunks:ARRAY[1..StructSeg] OF ChunkList;  { Belegungen }
   ActPC:Integer;                           { gewÑhlter ProgrammzÑhler }
   PCsUsed:ARRAY[1..StructSeg] OF Boolean;  { PCs bereits initialisiert ? }
   SegInits:ARRAY[1..PCMax] OF LongInt;     { Segmentstartwerte }
   ValidSegs:SET OF 0..7;                   { erlaubte Segmente }
   ENDOccured:Boolean;	                    { END-Statement aufgetreten ? }
   Retracted:Boolean;			    { Codes zurÅckgenommen ? }

   TypeFlag:Word;		    { Welche Adre·rÑume genutzt ? }
   SizeFlag:ShortInt;		    { Welche Operandengroessen definiert ? }

   PassNo:Byte;                     { Durchlaufsnummer }
   JmpErrors:Integer;               { Anzahl fraglicher Sprungfehler }
   ThrowErrors:Boolean;             { Fehler verwerfen bei Repass ? }
   Repass:Boolean;		    { noch ein Durchlauf erforderlich }
   MaxSymPass:Byte;	            { Pass, nach dem Symbole definiert sein mÅssen }
   ShareMode:Byte;                  { 0=kein SHARED,1=Pascal-,2=C-Datei, 3=ASM-Datei }
   DebugMode:DebugType;             { Ausgabeformat Debug-Datei }
   ListMode:Byte;                   { 0=kein Listing,1=Konsole,2=auf Datei }
   ListOn:Byte;                     { Listing erzeugen ? }
   MakeUseList:Boolean; 	    { Belegungsliste ? }
   MakeCrossList:Boolean;	    { Querverweisliste ? }
   MakeSectionList:Boolean;	    { Sektionsliste ? }
   MakeIncludeList:Boolean;         { Includeliste ? }
   RelaxedMode:Boolean;		    { alle Integer-Syntaxen zulassen ? }
   ListMask:Byte;		    { Listingmaske }
   ExtendErrors:Boolean;	    { erweiterte Fehlermeldungen }
   NumericErrors:Boolean;	    { Fehlermeldungen mit Nummer }
   CodeOutput:Boolean;		    { Code erzeugen }
   MacProOutput:Boolean;	    { Makroprozessorausgabe schreiben }
   MacroOutput:Boolean;             { gelesene Makros schreiben }
   QuietMode:Boolean;               { keine Meldungen }
   HardRanges:Boolean;              { Bereichsfehler echte Fehler ? }
   DivideChars:String[4];           { Trennzeichen fÅr Parameter }
   HasAttrs:Boolean;                { Opcode hat Attribut }
   AttrChars:String[4];             { Zeichen, mit denen Attribut abgetrennt wird }
   MsgIfRepass:Boolean;		    { Meldungen, falls neuer Pass erforderlich }
   PassNoForMessage:Integer;        { falls ja: ab welchem Pass ? }
   CaseSensitive:Boolean;           { Gro·/Kleinschreibung unterscheiden ? }

   PrgFile:File;                    { Codedatei }

   ErrorPath,ErrorName:String;	    { Ausgabedatei Fehlermeldungen }
   OutName:String;                  { Name Code-Datei }
   IsErrorOpen:Boolean;
   CurrFileName:String;             { mom. bearbeitete Datei }
   MomLineCounter:LongInt;          { Position in mom. Datei }
   CurrLine:LongInt;       	    { virtuelle Position }
   LineSum:LongInt;		    { Gesamtzahl Quellzeilen }
   MacLineSum:LongInt;              { inkl. Makroexpansion }

   NOPCode:LongInt;                 { Maschinenbefehl NOP zum Stopfen }
   TurnWords:Boolean;		    { TRUE  = Motorola-Wortformat }
				    { FALSE = Intel-Wortformat }
   HeaderID:Byte;		    { Kennbyte des Codeheaders }
   PCSymbol:String[4];		    { Symbol, womit ProgrammzÑhler erreicht wird }
   ConstMode:TConstMode;
   SetIsOccupied:Boolean;           { TRUE: SET ist Prozessorbefehl }
   MakeCode:PROCEDURE;		    { Codeerzeugungsprozedur }
   ChkPC:FUNCTION:Boolean;	    { ÅberprÅft CodelÑngenÅberschreitungen }
   IsDef:FUNCTION:Boolean;	    { ist Label nicht als solches zu werten ? }
   SwitchFrom:PROCEDURE;            { bevor von einer CPU weggeschaltet wird }
   InternSymbol:                    { vordefinierte Symbole ? }
   PROCEDURE(VAR Asc:String; VAR Erg:TempResult);
   Magic:LongInt;
   InitPassProc:PROCEDURE;          { Funktion zur Vorinitialisierung vor einem Pass }
   ClearUpProc:PROCEDURE;           { AufrÑumen nach Assemblierung }

   IncludeList:String;		    { Suchpfade fÅr Includedateien }
   IncDepth,NextIncDepth:Integer;   { Verschachtelungstiefe INCLUDEs }
   ErrorFile:Text;		    { Fehlerausgabe }
   LstFile:Text;                    { Listdatei }
   ShareFile:Text;                  { Sharefile }
   MacProFile:Text;		    { Makroprozessorausgabe }
   MacroFile:Text;		    { Ausgabedatei Makroliste }
   LstName:String;		    { Name der Listdatei }
   DoLst,NextDoLst:Boolean;         { Listing an }
   ShareName:String;                { Name des Sharefiles }
   PrgName:String;                  { Name der Codedatei }

   MomCPU,MomVirtCPU:CPUVar;        { definierter/vorgegaukelter Prozessortyp }
   MomCPUIdent:String[9];           { dessen Name in ASCII }
   FirstCPUDef:PCPUDef;		    { Liste mit Prozessordefinitionen }
   CPUCnt:CPUVar;	    	    { Gesamtzahl Prozessoren }

   FPUAvail:Boolean;		    { Koprozessor erlaubt ? }
   DoPadding:Boolean;		    { auf gerade Byte-Zahl ausrichten ? }
   SupAllowed:Boolean;              { Supervisormode freigegeben }
   Maximum:Boolean;		    { CPU nicht kastriert }

   LabPart,OpPart,AttrPart,         { Komponenten der Zeile }
   ArgPart,CommPart,LOpPart:String;
   AttrSplit:Char;
   ArgStr:ArgStrField;              { Komponenten des Arguments }
   ArgCnt:Byte;                     { Argumentzahl }
   OneLine:String;                  { eingelesene Zeile }

   LstCounter:Byte;                 { ZeilenzÑhler fÅr automatischen Umbruch }
   PageCounter:ARRAY[0..ChapMax] OF Word;  { hierarchische SeitenzÑhler }
   ChapDepth:Byte;                  { momentane Kapitelverschachtelung }
   ListLine:String;		    { alternative Ausgabe vor Listing fÅr EQU }
   ErrorPos:String;		    { zus. Positionsinformation in Makros }
   PageLength,PageWidth:Byte;       { SeitenlÑnge/breite in Zeilen/Spalten }
   LstMacroEx:Boolean;              { Makroexpansionen auflisten }
   PrtInitString:String;	    { Druckerinitialisierungsstring }
   PrtExitString:String;	    { Druckerdeinitialisierungsstring }
   PrtTitleString:String;	    { Titelzeile }
   ExtendError:String;              { erweiterte Fehlermeldung }

   MomSectionHandle:LongInt;        { mom. Namensraum }
   SectionStack:PSaveSection;	    { gespeicherte Sektionshandles }

   CodeLen:LongInt;                 { LÑnge des erzeugten Befehls }
   BAsmCode:ARRAY[0..MaxCodeLen-1] OF Byte; { Zwischenspeicher erzeugter Code }
   WAsmCode:ARRAY[0..(MaxCodeLen DIV 2)-1] OF Word ABSOLUTE BAsmCode;
   DAsmCode:ARRAY[0..(MaxCodeLen DIV 4)-1] OF LongInt ABSOLUTE BAsmCode;

   DontPrint:Boolean;               { Flag:PC verÑndert, aber keinen Code erzeugt }
   MultiFace:RECORD Case Byte OF
		    0:(Feld:WordField);
		    1:(Val32:Single);
		    2:(Val64:Double);
		    3:(Val80:Extended);
		    4:(ValCo:Comp);
		    END;

   StopfZahl:Byte;                  { Anzahl der im 2.Pass festgestellten
				      ÅberflÅssigen Worte, die mit NOP ge-
				      fÅllt werden mÅssen }

   SuppWarns:Boolean;

   CharTransTable:ARRAY[Char] OF Char; { ZeichenÅbersetzungstabelle }

   FirstFunction:PFunction;	    { Liste definierter Funktionen }

   FirstDefine:PDefinement;         { Liste von PrÑprozessor-Defines }

   StructureStack:PStructure;       { momentan offene Strukturen }
   StructSaveSeg:Integer;           { gesichertes Segment wÑhrend Strukturdef. }

   FirstSaveState:PSaveState;	    { gesicherte ZustÑnde }

   MakeDebug:Boolean;		    { Debugginghilfe }
   Debug:Text;

   StartAvail:LongInt;


        PROCEDURE _Init_AsmDef;

	PROCEDURE AsmDefInit;

        PROCEDURE NullProc;

        PROCEDURE Default_InternSymbol(VAR Asc:String; VAR Erg:TempResult);

Implementation

	PROCEDURE AsmDefInit;
VAR
   z:Integer;
BEGIN
   DoLst:=True; PassNo:=1; MaxSymPass:=1;

   LineSum:=0;

   FOR z:=0 TO ChapMax DO PageCounter[z]:=0;
   LstCounter:=0; ChapDepth:=0;

   PrtInitString:=''; PrtExitString:=''; PrtTitleString:='';

   ExtendError:='';

   CurrFileName:=''; MomLineCounter:=0;

   FirstFunction:=Nil; FirstDefine:=Nil; FirstSaveState:=Nil;
END;

	PROCEDURE NullProc;
BEGIN
END;

        PROCEDURE Default_InternSymbol(VAR Asc:String; VAR Erg:TempResult);
BEGIN
   Erg.Typ:=TempNone;
END;

        PROCEDURE _Init_AsmDef;
VAR
   z:Char;
BEGIN
   ClearUpProc:=NullProc;
   FirstFunction:=Nil;
   SwitchFrom:=NullProc;
   InternSymbol:=Default_InternSymbol;

   FOR z:=#0 TO #255 DO CharTransTable[z]:=z;
   RelaxedMode:=True; ConstMode:=ConstModeC;

   StartAvail:=MemAvail;
END;

BEGIN
   FirstCPUDef:=Nil;
   CPUCnt:=0;
   InitPassProc:=NullProc;
END.

