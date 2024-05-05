#ifndef CODECHUNKS_H
#define CODECHUNKS_H
/* codechunks.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Maintain address ranges & included code                                   */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

typedef struct {
    LargeWord Start, Length;
    unsigned  Granularity;
    LongWord* pLongCode;
    Word*     pWordCode;
    Byte*     pCode;
} tCodeChunk;

typedef struct {
    Word        RealLen, AllocLen;
    tCodeChunk* Chunks;
} tCodeChunkList;

extern void InitCodeChunk(tCodeChunk* pChunk);
extern int  ReadCodeChunk(
         tCodeChunk* pChunk, char const* pFileName, LargeWord Start, LargeWord Length,
         unsigned Granularity);
extern void FreeCodeChunk(tCodeChunk* pChunk);

extern void InitCodeChunkList(tCodeChunkList* pCodeChunkList);
extern int  MoveCodeChunkToList(
         tCodeChunkList* pCodeChunkList, tCodeChunk* pNewCodeChunk, Boolean WarnOverlap);
extern void    FreeCodeChunkList(tCodeChunkList* pCodeChunkList);
extern Boolean RetrieveCodeFromChunkList(
        tCodeChunkList const* pCodeChunkList, LargeWord Start, Byte* pData,
        unsigned Count);

extern LargeWord GetCodeChunksStored(tCodeChunkList const* pCodeChunkList);

#endif /* CODECHUNKS_H */
