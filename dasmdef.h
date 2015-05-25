#ifndef _DASMDEF_H
#define _DASMDEF_H

/* dasmdef.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* DAS common variables                                                      */
/*                                                                           */
/*****************************************************************************/

#include "codechunks.h"
#include "chunks.h"

typedef struct sDisassInfo
{
  unsigned CodeLen;
  unsigned NextAddressCount;
  LargeWord NextAddresses[10];
  String SrcLine;
} tDisassInfo;

extern tCodeChunkList CodeChunks;
extern ChunkList UsedChunks;

extern void dasmdef_init(void);

#endif /* _DASMDEF_H */
