/* toolutils.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen fuer die AS-Tools                                           */
/*                                                                           */
/* Historie: 31. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "fileformat.h"

extern LongWord Magic;

extern Word FileID;

extern char *OutName;


extern void WrCopyRight(char *Msg);

extern void DelSuffix(char *Name);

extern void AddSuffix(char *Name, char *Suff);

extern void FormatError(char *Name, char *Detail);

extern void ChkIO(char *Name);

extern Word Granularity(Byte Header);

extern void ReadRecordHeader(Byte *Header, Byte* Segment, Byte *Gran,
                             char *Name, FILE *f);

extern void WriteRecordHeader(Byte *Header, Byte* Segment, Byte *Gran,
                              char *Name, FILE *f);

extern CMDResult CMD_FilterList(Boolean Negate, char *Arg);

extern Boolean FilterOK(Byte Header);

extern Boolean RemoveOffset(char *Name, LongWord *Offset);

extern void toolutils_init(void);
