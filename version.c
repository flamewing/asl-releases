/* version.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Lagert die Versionsnummer                                                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>

char *Version = "1.42 Beta [Bld 152]";
char *DebVersion = "1.42bld152-1";
LongInt VerNo = 0x142f;

char *InfoMessCopyright = "(C) 1992,2020 Alfred Arnold";

LongInt Magic = 0x12372c46;

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
