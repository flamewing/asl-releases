/* lang_EN/p2bin.rsc */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Stringdefinitionen fuer P2BIN                                             */
/* englische Version                                                         */
/* Historie:  6. 6.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

/*****************************************************************************/
/* Ansagen */

#define InfoMessChecksum    "checksum: "

#define InfoMessHead2       " <source file(s)> <target file> [options]"
#define InfoMessHelpCnt     7
static char *InfoMessHelp[InfoMessHelpCnt]=
                  {"",
                   "options: -f <header list>  :  records to filter out",
                   "         -r <start>-<stop> :  address range to filter out",
                   "         ($ = first resp. last occuring address)",
                   "         -l <8-bit-number> :  set filler value for unused cells",
                   "         -s                :  put checksum into file",
                   "         -m <mode>         :  EPROM-mode (odd,even,byte0..byte3)"};
