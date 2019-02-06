{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei P2BIN.RSC - enthÑlt Stringdefinitionen fÅr P2BIN (englisch)  *}
{* 									    *}
{* Historie : 24.7.1993 Grundsteinlegung                                    *}
{*             7.8.1993 neue Parameter $                                    *}
{*            5.10.1993 Fehlermeldungen nach TOOLS.RSC herausgezogen        *}
{*            6.10.1993 englische öbersetzung begonnen                      *}
{*            1.11.1993 Erkennung Programmname                              *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Ansagen }

   InfoMessChecksum         = 'checksum: ';

   InfoMessHead2            = ' <source file(s)> <target file> [options]';
   InfoMessHelpCnt          = 7;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
                  ('',
                   'options: -f <header list>  :  auszufilternde Records',
                   '         -r <start>-<stop> :  auszufilternder Adre·bereich',
                   '         ($ = first resp. last occuring address)',
                   '         -l <8-bit-number> :  set filler value for unused cells',
                   '         -s                :  put checksum into file',
                   '         -m <mode>         :  EPROM-mode (odd,even,byte0..byte3)');
