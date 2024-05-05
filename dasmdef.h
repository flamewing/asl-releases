#ifndef DASMDEF_H
#define DASMDEF_H
/* dasmdef.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* DAS common variables                                                      */
/*                                                                           */
/*****************************************************************************/

#include "chunks.h"
#include "codechunks.h"

typedef struct sDisassInfo {
    unsigned    CodeLen;
    unsigned    NextAddressCount;
    LargeWord   NextAddresses[10];
    String      SrcLine;
    char const* pRemark;
} tDisassInfo;

extern tCodeChunkList CodeChunks;
extern ChunkList      UsedDataChunks, UsedCodeChunks;

extern void (*Disassemble)(
        LargeWord Address, tDisassInfo* pInfo, Boolean IsData, int DataSize);

extern void dasmdef_init(void);

#endif /* DASMDEF_H */
