/* lang_DE/p2bin.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Stringdefinitionen fuer P2BIN                                             */
/*                                                                           */
/* Historie:  3. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Ansagen */

#define InfoMessChecksum    "Pr"CH_ue"fsumme: "

#define InfoMessHead2       " <Quelldatei(en)> <Zieldatei> [Optionen]"
#define InfoMessHelpCnt     7
static char *InfoMessHelp[InfoMessHelpCnt]=
             {"",
              "Optionen: -f <Headerliste>  :  auszufilternde Records",
              "          -r <Start>-<Stop> :  auszufilternder Adre"CH_sz"bereich",
              "          ($ = erste bzw. letzte auftauchende Adresse)",
              "          -l <8-Bit-Zahl>   :  Inhalt unbenutzter Speicherzellen festlegen",
              "          -s                :  Pr"CH_ue"fsumme in Datei ablegen",
              "          -m <Modus>        :  EPROM-Modus (odd,even,byte0..byte3)"};
