/* version.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Lagert die Versionsnummer                                                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

char *Version = "1.42 Beta [Bld 147]";
char *DebVersion = "1.42bld147-1";
LongInt VerNo = 0x142f;

char *InfoMessCopyright = "(C) 1992,2019 Alfred Arnold";

LongInt Magic = 0x12372545;

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
