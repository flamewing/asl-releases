/* lang_EN/p2hex.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* String-Definitionen fuer P2HEX                                            */
/* englische Version                                                         */
/* Historie: 8. 9.1996 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Steuerstrings HEX-Files */

#define DSKHeaderLine       "K_DSKA_1.00_DSK_"

/*****************************************************************************/
/* Fehlermeldungen */

#define ErrMsgAdrOverflow   "warning: address overflow "

/*****************************************************************************/
/* Ansagen */

#define InfoMessHead2       " <source file(s)> <target file> [options]"
#define InfoMessHelpCnt     16
static char *InfoMessHelp[InfoMessHelpCnt]=
                  {"",
                   "options: -f <header list>  : records to filter out",
                   "         -r <start>-<stop> : address range to filter out",
                   "         ($ = first resp. last occuring address)",
                   "         -a                : addresses in hex file relativ to start",
                   "         -l <length>       : set length of data line in bytes",
                   "         -i <0|1|2>        : terminating line for intel hex",
                   "         -m <0..3>         : multibyte mode",
                   "         -F <Default|Moto|",
                   "             Intel|MOS|Tek|",
                   "             Intel16|DSK|",
                   "             Intel32>      : target format",
                   "         +5                : supress S5-records",
                   "         -s                : separate terminators for S-record groups",
                   "         -d <start>-<stop> : set data range",
                   "         -e <address>      : set start address"};

