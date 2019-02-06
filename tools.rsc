{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei TOOLS.RSC - enthÑlt Stringdefinitionen fÅr BIND              *}
{* 									    *}
{* Historie : 5.10.1993 Grundsteinlegung                                    *}
{*                      detailliertere Formatfehlermeldungen                *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Durchsagen }

CONST
   InfoMessHead1            = 'Aufruf: ';


{****************************************************************************}
{ Fehlermeldungen }

CONST
   FormatErr1aMsg           = 'Das Format der Datei "';
   FormatErr1bMsg           = '" ist fehlerhaft!';
   FormatErr2Msg            = 'Bitte Åbersetzen Sie die Quelldatei neu!';

   FormatInvHeaderMsg       = 'ungÅltiger Dateikopf';
   FormatInvRecordHeaderMsg = 'ungÅltiger Datensatzkopf';
   FormatInvRecordLenMsg    = 'fehlerhafte DatensatzlÑnge';

   IOErrAHeaderMsg          = 'Bei Bearbeitung der Datei"';
   IOErrBHeaderMsg          = '" ist folgender Fehler aufgetreten:';

   ErrMsgTerminating        = 'Das Programm wird beendet!';

   ErrMsgNullMaskA          = 'Warnung : Dateimaske ';
   ErrMsgNullMaskB          = ' pa·t auf keine Datei!';

   ErrMsgInvEnvParam        = 'Fehlerhafter Environment-Parameter : ';
   ErrMsgInvParam           = 'Fehlerhafter Parameter : ';

   ErrMsgTargMissing        = 'Zieldateiangabe fehlt!';
   ErrMsgAutoFailed         = 'automatische Bereichserkennung fehlgeschlagen!';

   ErrMsgOverlap            = 'Warnung: öberlappende Belegung!';

   ErrMsgProgTerm           = 'Programmabbruch';

{****************************************************************************}

CONST
   Suffix='.P';                  { Suffix Eingabedateien }
   FileID:Word=$1489;            { Dateiheader Eingabedateien }
   OutName:String[10]='STDOUT';  { Pseudoname Output }

