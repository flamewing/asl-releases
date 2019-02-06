{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei PLIST.RSC - enthÑlt Stringdefinitionen fÅr PLIST             *}
{* 									    *}
{* Historie : 18.7.1993 Grundsteinlegung                                    *}
{*            5.10.1993 Fehlermeldungen nach TOOLS.RSC herausgezogen        *}
{*            1.11.1993 Erkennung Programmname                              *}
{*           10. 2.1996 Einsprungpunkt                                      *}
{*            3.12.1996 Erweiterung Segmentspalte                           *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Ansagen }

   MessFileRequest          = 'zu listende Programmdatei [.P] : ';

   MessHeaderLine1          = 'Codetyp     Segment  Startadresse   LÑnge (Byte)  Endadresse';
   MessHeaderLine2          = '------------------------------------------------------------';

   MessGenerator            = 'Erzeuger : ';

   MessSum1                 = 'insgesamt ';
   MessSumSing              = ' Byte  ';
   MessSumPlur              = ' Bytes ';

   MessEntryPoint           = '<Einsprung>           ';

   InfoMessHead2            = ' [Programmdateiname]';
   InfoMessHelpCnt          = 1;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
                  ('');
