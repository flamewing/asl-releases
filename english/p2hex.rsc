{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei P2HEX.RSC - enthÑlt Stringdefinitionen fÅr P2HEX (englisch)  *}
{* 									    *}
{* Historie : 24.7.1993 Grundsteinlegung                                    *}
{*             7.8.1993 Zielformattypen                                     *}
{*            5.10.1993 Fehlermeldungen nsach TOOLS.RSC herausgezogen       *}
{*            6.10.1993 Umstellung auf Englisch begonnen                    *}
{*           11.10.1993 Parameter 5                                         *}
{*           17.10.1993 Parameter s                                         *}
{*           24.10.1993 Tek, Int16-Format                                   *}
{*                      Fehlermeldung Adre·Åberlauf                         *}
{*            1.11.1993 Erkennung Programmname                              *}
{*            9. 4.1994 Erweiterungen TI-DSK-Format                         *}
{*           13. 3.1995 Erweiterungen Intel32-Format                        *}
{*           23. 8.1997 Atmel-Hex-Format                                    *}
{*           26.10.1997 kill-Option                                         *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Steuerstrings HEX-Files }

   DSKHeaderLine='K_DSKA_1.00_DSK_';

{****************************************************************************}
{ Fehlermeldungen }

   ErrMsgAdrOverflow        = 'warning: address overflow ';

{****************************************************************************}
{ Ansagen }

   InfoMessHead2            = ' <source file(s)> <target file> [options]';
   InfoMessHelpCnt          = 17;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
                  ('',
                   'options: -f <header list>  : records to filter out',
                   '         -r <start>-<stop> : address range to filter out',
                   '         ($ = first resp. last occuring address)',
                   '         -a                : addresses in hex file relativ to start',
                   '         -l <length>       : set length of data line in bytes',
                   '         -i <0|1|2>        : terminating line for intel hex',
                   '         -m <0..3>         : multibyte mode',
                   '         -F <Default|Moto|',
                   '             Intel|MOS|Tek|',
                   '             Intel16|DSK|',
                   '             Intel32|Atmel>: target format',
                   '         +5                : supress S5-records',
                   '         -s                : separate terminators for S-record groups',
                   '         -d <start>-<stop> : set data range',
                   '         -e <address>      : set start address',
                   '         -k                : automatically erase source files');

