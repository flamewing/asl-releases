/* version.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Lagert die Versionsnummer                                                 */
/*                                                                           */
/* Historie: 14.10.1997 Grundsteinlegung                                     */
/*           25. 7.1998 Version 1.41r8beta                                   */
/*           18.10.1998 Build 4                                              */
/*           25.10.1998 Build 5                                              */
/*           10. 1.1999 Build 6                                              */
/*           17. 1.1999 Build 7                                              */
/*           27. 1.1999 Build 8                                              */
/*           27. 1.1999 Build 9                                              */
/*            7. 2.1999 Build 10                                             */
/*           19. 4.1999 Build 11                                             */
/*           20. 4.1999 Build 12                                             */
/*            2. 5.1999 Build 13                                             */
/*            6. 7.1999 Build 14                                             */
/*           15. 9.1999 Build 15                                             */
/*            7.11.1999 Final Build                                          */
/*            9. 1.2000 Version 1.42beta                                     */
/*           13. 2.2000 Build 2                                              */
/*           13. 3.2000 Build 3                                              */
/*            4. 6.2000 Build 5                                              */
/*           18. 6.2000 Build 6                                              */
/*            6. 8.2000 Build 7                                              */
/*           20. 9.2000 Build 8                                              */
/*           15.10.2000 Build 9                                              */
/*            7. 1.2001 Build 10                                             */
/*           14. 1.2001 Build 11                                             */
/*            4. 2.2001 Build 12                                             */
/*           30. 5.2001 Build 13                                             */
/*            1. 7.2001 Build 14                                             */
/*           20. 8.2001 Build 15                                             */
/*            7.10.2001 Build 16                                             */
/*           2001-11-04 Build 17                                             */
/*           2002-01-13 Build 18                                             */
/*           2002-01-27 Build 19                                             */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

char *Version = "1.42 Beta [Bld 128]";
char *DebVersion = "1.42bld128-1";
LongInt VerNo = 0x142f;

char *InfoMessCopyright = "(C) 1992,2018 Alfred Arnold";

LongInt Magic = 0x12372445;

void version_init(void)
{
  unsigned int z;
  char *CMess = InfoMessCopyright;
  LongWord XORVal;

  for (z = 0; z < strlen(CMess); z++)
  {
    XORVal = CMess[z];
    XORVal = XORVal << (((z + 1) % 4) * 8);
    Magic = Magic ^ XORVal;
  }
}
