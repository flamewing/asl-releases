/* lang_DE/plist.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Stringdefinitionen fuer PLIST                                             */
/*                                                                           */
/* Historie: 31. 5.1996 Grundsteinlegung                                     */
/*            3.12.1996 Erweiterung um Segment-Spalte                        */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Ansagen */

#define MessFileRequest          "zu listende Programmdatei [.P] : "

#define MessHeaderLine1          "Codetyp     Segment  Startadresse   L"CH_ae"nge (Byte)  Endadresse"
#define MessHeaderLine2          "------------------------------------------------------------"

#define MessGenerator            "Erzeuger : "

#define MessSum1                 "insgesamt "
#define MessSumSing              " Byte  "
#define MessSumPlur              " Bytes "

#define MessEntryPoint           "<Einsprung>           "

#define InfoMessHead2            " [Programmdateiname]"
#define InfoMessHelpCnt          1
static char *InfoMessHelp[InfoMessHelpCnt]=
            {""};
