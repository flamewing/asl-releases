/* stdhandl.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Bereitstellung von fuer AS benoetigten Handle-Funktionen                  */
/*                                                                           */
/* Historie:  5. 4.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef enum {NoRedir,RedirToDevice,RedirToFile} TRedirected;  /* Umleitung von Handles */

#define NumStdIn 0
#define NumStdOut 1
#define NumStdErr 2

extern TRedirected Redirected;

extern void RewriteStandard(FILE **T, char *Path);

extern void stdhandl_init(void);
