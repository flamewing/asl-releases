/* lang_DE/p2hex.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* String-Definitionen fuer P2HEX                                            */
/*                                                                           */
/* Historie: 1. 6.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Steuerstrings HEX-Files */

#define DSKHeaderLine       "K_DSKA_1.00_DSK_"

/*****************************************************************************/
/* Fehlermeldungen */

#define ErrMsgAdrOverflow   "Warnung: Adre"CH_sz""CH_ue"berlauf "

/*****************************************************************************/
/* Ansagen */

#define InfoMessHead2       " <Quelldatei(en)> <Zieldatei> [Optionen]"
#define InfoMessHelpCnt     16
static char *InfoMessHelp[InfoMessHelpCnt]=
                  {"",
                   "Optionen: -f <Headerliste>  : auszufilternde Records",
                   "          -r <Start>-<Stop> : auszufilternder Adre"CH_sz"bereich",
                   "          ($ = erste bzw. letzte auftauchende Adresse)",
                   "          -a                : Adressen relativ zum Start ausgeben",
                   "          -l <L"CH_ae"nge>        : L"CH_ae"nge Datenzeile/Bytes",
                   "          -i <0|1|2>        : Terminierer f"CH_ue"r Intel-Hexfiles",
                   "          -m <0..3>         : Multibyte-Modus",
		   "          -F <Default|Moto|",
                   "              Intel|MOS|Tek|",
                   "              Intel16|DSK|",
                   "              Intel32>      : Zielformat",
                   "          +5                : S5-Records unterdr"CH_ue"cken",
                   "          -s                : S-Record-Gruppen einzeln terminieren",
                   "          -d <Start>-<Stop> : Datenbereich festlegen",
                   "          -e <Adresse>      : Startadresse festlegen"};

