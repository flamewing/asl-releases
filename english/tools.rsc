{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei TOOLS.RSC - enthÑlt Stringdefinitionen fÅr BIND (englisch)   *}
{* 									    *}
{* Historie : 5.10.1993 Grundsteinlegung                                    *}
{*                      detailliertere Formatfehlermeldungen                *}
{*            6.10.1993 Umstellung auf Englisch begonnen                    *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Ansagen }

CONST
   InfoMessHead1            = 'calling convention: ';

{****************************************************************************}
{ Fehlermeldungen }

   FormatErr1aMsg           = 'the format of file "';
   FormatErr1bMsg           = '" is invalid!';
   FormatErr2Msg            = 'please reassemble the source file!';

   FormatInvHeaderMsg       = 'invalid file header';
   FormatInvRecordHeaderMsg = 'invalid record header';
   FormatInvRecordLenMsg    = 'invalid record length';

   IOErrAHeaderMsg          = 'the following error occured while processing "';
   IOErrBHeaderMsg          = '" :';

   ErrMsgTerminating        = 'program terminated!';

   ErrMsgNullMaskA          = 'warning : file mask ';
   ErrMsgNullMaskB          = ' does not fit to any file!';

   ErrMsgInvEnvParam        ='invalid environment parameter: ';
   ErrMsgInvParam           ='invalid parameter : ';

   ErrMsgTargMissing        = 'destination file specification missing!';
   ErrMsgAutoFailed         = 'automatic range setting failed!';

   ErrMsgOverlap            = 'warning: overlapping memory allocation!';

   ErrMsgProgTerm           = 'program terminated';

{****************************************************************************}

CONST
   Suffix='.P';                  { Suffix Eingabedateien }
   FileID:Word=$1489;            { Dateiheader Eingabedateien }
   OutName:String[10]='STDOUT';  { Pseudoname Output }

