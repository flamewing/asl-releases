/* hex.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Dezimal-->Hexadezimal-Wandlung, Grossbuchstaben                           */
/*                                                                           */
/* Historie: 2. 6.1996                                                       */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>

#include "hex.h"

#define BUFFERCNT 8

	char *HexNibble(Byte inp)
BEGIN
   static char Buffers[BUFFERCNT][3],*ret;
   static int z=0;

   sprintf(Buffers[z],"%01x",inp&0xf);
   for (ret=Buffers[z]; *ret!='\0'; ret++) *ret=toupper(*ret);
   ret=Buffers[z]; 
   z=(z+1)%BUFFERCNT;
   return ret;
END

	char *HexByte(Byte inp)
BEGIN
   static char Buffers[BUFFERCNT][4],*ret;
   static int z=0;

   sprintf(Buffers[z],"%02x",inp&0xff);
   for (ret=Buffers[z]; *ret!='\0'; ret++) *ret=toupper(*ret);
   ret=Buffers[z]; 
   z=(z+1)%BUFFERCNT;
   return ret;
END

	char *HexWord(Word inp)
BEGIN
   static char Buffers[BUFFERCNT][6],*ret;
   static int z=0;

   sprintf(Buffers[z],"%04x",inp&0xffff);
   for (ret=Buffers[z]; *ret!='\0'; ret++) *ret=toupper(*ret);
   ret=Buffers[z]; 
   z=(z+1)%BUFFERCNT;
   return ret;
END

	char *HexLong(LongWord inp)
BEGIN
   static char Buffers[BUFFERCNT][10],*ret;
   static int z=0;

#ifdef __STDC__
   sprintf(Buffers[z],"%08x",inp&0xffffffffu);
#else
   sprintf(Buffers[z],"%08x",inp&0xffffffff);
#endif
   for (ret=Buffers[z]; *ret!='\0'; ret++) *ret=toupper(*ret);
   ret=Buffers[z]; 
   z=(z+1)%BUFFERCNT;
   return ret;
END

	void hex_init(void)
BEGIN
END
