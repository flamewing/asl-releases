/* dasmdef.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* DAS common variables                                                      */
/*                                                                           */
/*****************************************************************************/

#include "dasmdef.h"

tCodeChunkList CodeChunks;
ChunkList UsedChunks;

void dasmdef_init(void)
{
  InitCodeChunkList(&CodeChunks);
  InitChunk(&UsedChunks);
}
