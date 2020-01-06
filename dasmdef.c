/* dasmdef.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* DAS common variables                                                      */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include "dasmdef.h"

tCodeChunkList CodeChunks;
ChunkList UsedCodeChunks, UsedDataChunks;

void(*Disassemble)(LargeWord Address, tDisassInfo *pInfo, Boolean IsData, int DataSize);

FILE *Debug;

void dasmdef_init(void)
{
  InitCodeChunkList(&CodeChunks);
  InitChunk(&UsedDataChunks);
  InitChunk(&UsedCodeChunks);
  Disassemble = NULL;
}
