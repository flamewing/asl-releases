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
  const char *pRemark;
} tDisassInfo;

extern tCodeChunkList CodeChunks;
extern ChunkList UsedDataChunks, UsedCodeChunks;

extern void(*Disassemble)(LargeWord Address, tDisassInfo *pInfo, Boolean IsData, int DataSize);

extern void dasmdef_init(void);

#endif /* _DASMDEF_H */
