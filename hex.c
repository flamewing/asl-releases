/* hex.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Dezimal-->Hexadezimal-Wandlung, Grossbuchstaben                           */
/*                                                                           */
/* Historie: 2. 6.1996                                                       */
/*          30. 5.2001 fixed 32-bit values on DOS platforms                  */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "hex.h"

#define BUFFERCNT 8

	char *HexNibble(Byte inp)
BEGIN
   static char Buffers[BUFFERCNT][3],*ret;
   static int z=0;

   sprintf(Buffers[z],"%01X",inp&0xf);
   ret=Buffers[z]; 
   z=(z+1)%BUFFERCNT;
   return ret;
END

	char *HexByte(Byte inp)
BEGIN
   static char Buffers[BUFFERCNT][4],*ret;
   static int z=0;

   sprintf(Buffers[z],"%02X",inp&0xff);
   ret=Buffers[z]; 
   z=(z+1)%BUFFERCNT;
   return ret;
END

	char *HexWord(Word inp)
BEGIN
   static char Buffers[BUFFERCNT][6],*ret;
   static int z=0;

   sprintf(Buffers[z],"%04X",inp&0xffff);
   ret=Buffers[z]; 
   z=(z+1)%BUFFERCNT;
   return ret;
END

	char *HexLong(LongWord inp)
BEGIN
   static char Buffers[BUFFERCNT][10],*ret;
   static int z=0;

#ifdef __STDC__
   sprintf(Buffers[z],"%08lX", ((long)inp) & 0xfffffffflu);
#else
   sprintf(Buffers[z],"%08lX", ((long)inp) & 0xffffffffl);
#endif
   ret=Buffers[z]; 
   z=(z+1)%BUFFERCNT;
   return ret;
END

	void hex_init(void)
BEGIN
END
