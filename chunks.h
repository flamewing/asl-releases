#ifndef _CHUNKS_H
#define _CHUNKS_H

/* chunks.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Adressbereichslisten                                       */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef struct
         {
	  LargeWord Start,Length;
	 } OneChunk;

typedef struct 
         {
	  Word RealLen,AllocLen;
	  OneChunk *Chunks;
	 } ChunkList;


extern Boolean AddChunk(ChunkList *NChunk, LargeWord NewStart, LargeWord NewLen, Boolean Warn);

extern void DeleteChunk(ChunkList *NChunk, LargeWord DelStart, LargeWord DelLen);

extern void InitChunk(ChunkList *NChunk);


extern void chunks_init(void);

#endif /* _CHUNKS_H */
