/****************************************************************************/
/* Makroassembler AS 							    */
/* 									    */
/* Headerdatei BIND.RSC - enthaelt Stringdefinitionen fuer BIND (englisch)  */
/* 									    */
/* Historie : 28.1.1997 Grundsteinlegung                                    */
/*                                                                          */
/****************************************************************************/

/****************************************************************************/
/* Fehlermeldungen */

#define ErrMsgTargetMissing  "target file specification is missing!"

/****************************************************************************/
/* Ansagen */

#define InfoMessHead2        " <source file(s)> <target file> [options]"
#define InfoMessHelpCnt      2
static char *InfoMessHelp[InfoMessHelpCnt]=
                  {"",
                   "options: -f <header list>  :  records to filter out"};
