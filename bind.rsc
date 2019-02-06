{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei BIND.RSC - enthÑlt Stringdefinitionen fÅr BIND               *}
{* 									    *}
{* Historie : 24.7.1993 Grundsteinlegung                                    *}
{*            5.10.1993 Fehlermeldungen nach TOOLS.RSC herausgezogen        *}
{*            1.11.1993 Erkennung Programmname                              *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Fehlermeldungen }

CONST
   ErrMsgTargetMissing='Zieldateiangabe fehlt!';

{****************************************************************************}
{ Ansagen }

CONST
   InfoMessHead2            = ' <Quelldatei(en)> <Zieldatei> [Optionen]';
   InfoMessHelpCnt          = 2;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
                  ('',
		   'Optionen: -f <Headerliste>  :  auszufilternde Records');
