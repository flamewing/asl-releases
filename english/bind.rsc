{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei BIND.RSC - enthÑlt Stringdefinitionen fÅr BIND (englisch)    *}
{* 									    *}
{* Historie : 24.7.1993 Grundsteinlegung                                    *}
{*            5.10.1993 Fehlermeldungen nach TOOLS.RSC herausgezogen        *}
{*            6.10.1993 öbersetzung begonnen                                *}
{*            1.11.1993 Erkennung Programmname                              *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Fehlermeldungen }

CONST
   ErrMsgTargetMissing='target file specification is missing!';

{****************************************************************************}
{ Ansagen }

CONST
   InfoMessHead2            = ' <source file(s)> <target file> [options]';
   InfoMessHelpCnt          = 2;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
                  ('',
                   'options: -f <header list>  :  records to filter out');
