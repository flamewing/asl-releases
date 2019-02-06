{****************************************************************************}
{* Makroassembler AS 							    *}
{* 									    *}
{* Headerdatei P2BIN.RSC - enthÑlt Stringdefinitionen fÅr P2BIN             *}
{* 									    *}
{* Historie : 24.7.1993 Grundsteinlegung                                    *}
{*             7.8.1993 neue Parameter $                                    *}
{*            5.10.1993 Fehlermeldungen nach TOOLS.RSC herausgezogen        *}
{*            1.11.1993 Erkennung Programmname                              *}
{*           26.10.1997 kill-Option                                         *}
{*                      Header mit Startadresse                             *}
{*                                                                          *}
{****************************************************************************}

{****************************************************************************}
{ Ansagen }

   InfoMessChecksum         = 'PrÅfsumme: ';

   InfoMessHead2            = ' <Quelldatei(en)> <Zieldatei> [Optionen]';
   InfoMessHelpCnt          = 10;
   InfoMessHelp : ARRAY[1..InfoMessHelpCnt] OF String[80]=
                  ('',
		   'Optionen: -f <Headerliste>  :  auszufilternde Records',
		   '          -r <Start>-<Stop> :  auszufilternder Adre·bereich',
                   '          ($ = erste bzw. letzte auftauchende Adresse)',
		   '          -l <8-Bit-Zahl>   :  Inhalt unbenutzter Speicherzellen festlegen',
		   '          -s                :  PrÅfsumme in Datei ablegen',
                   '          -m <Modus>        :  EPROM-Modus (odd,even,byte0..byte3)',
                   '          -e <Adresse>      :  Startadresse festelegen',
                   '          -S [L|B]<LÑnge>   :  Startadresse voranstellen',
                   '          -k                :  Quelldateien automatisch lîschen');
