{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei PLIST.RSC - enthÑlt Stringdefinitionen fÅr PLIST (englisch)  *}
{* 									    *}
{* Historie : 18.7.1993 Grundsteinlegung                                    *}
{*            5.10.1993 Fehlermeldungen nach TOOLS.RSC herausgezogen        *}
{*            6.10.1993 Umstellung auf Englisch begonnen                    *}
{*            1.11.1993 Erkennung Programmname                              *}
{*           10. 3.1996 Einsprungpunkt                                      *}
{*            3.12.1996 Erweiterung Segmentspalte                           *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Ansagen }

   MessFileRequest          = 'program file to list [.P] : ';

   MessHeaderLine1          = 'code type   segment  start address length (byte)  end address';
   MessHeaderLine2          = '-------------------------------------------------------------';

   MessGenerator            = 'creator : ';

   MessSum1                 = 'altogether ';
   MessSumSing              = ' byte  ';
   MessSumPlur              = ' bytes ';

   MessEntryPoint           = '<entry point>         ';

   InfoMessHead2            = ' [name of program file]';
   InfoMessHelpCnt          = 1;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
                  ('');
