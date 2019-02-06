{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei P2HEX.RSC - enthÑlt Stringdefinitionen fÅr P2HEX             *}
{* 									    *}
{* Historie : 24.7.1993 Grundsteinlegung                                    *}
{*             7.8.1993 Zielformattypen                                     *}
{*            5.10.1993 Fehlermeldungen nsach TOOLS.RSC herausgezogen       *}
{*           11.10.1993 Parameter 5                                         *}
{*           17.10.1993 Parameter s                                         *}
{*           24.10.1993 Tek, Int16-Format                                   *}
{*                      Fehlermeldung Adre·Åberlauf                         *}
{*            1.11.1993 Erkennung Programmname                              *}
{*            9. 4.1994 Erweiterungen TI-DSK-Format                         *}
{*           13. 3.1995 Erweiterungen Intel32-Format                        *}
{*            4.11.1995 Erweiterung ZeilenlÑnge                             *}
{*           23. 8.1997 Atmel-Hex-Format                                    *}
{*           26.10.1997 kill-Option                                         *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Steuerstrings HEX-Files }

   DSKHeaderLine='K_DSKA_1.00_DSK_';

{****************************************************************************}
{ Fehlermeldungen }

   ErrMsgAdrOverflow        = 'Warnung: Adre·Åberlauf ';

{****************************************************************************}
{ Ansagen }

   InfoMessHead2            = ' <Quelldatei(en)> <Zieldatei> [Optionen]';
   InfoMessHelpCnt          = 17;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
                  ('',
                   'Optionen: -f <Headerliste>  : auszufilternde Records',
                   '          -r <Start>-<Stop> : auszufilternder Adre·bereich',
                   '          ($ = erste bzw. letzte auftauchende Adresse)',
                   '          -a                : Adressen relativ zum Start ausgeben',
                   '          -l <LÑnge>        : LÑnge Datenzeile/Bytes',
                   '          -i <0|1|2>        : Terminierer fÅr Intel-Hexfiles',
                   '          -m <0..3>         : Multibyte-Modus',
		   '          -F <Default|Moto|',
                   '              Intel|MOS|Tek|',
                   '              Intel16|DSK|',
                   '              Intel32|Atmel>: Zielformat',
                   '          +5                : S5-Records unterdrÅcken',
                   '          -s                : S-Record-Gruppen einzeln terminieren',
                   '          -d <Start>-<Stop> : Datenbereich festlegen',
                   '          -e <Adresse>      : Startadresse festlegen',
                   '          -k                : Quelldateien autom. lîschen');

