#ifndef _ASMRELOCS_H
#define _ASMRELOCS_H
/* asmrelocs.h */
/****************************************************************************/
/* AS-Portierung                                                            */
/*                                                                          */
/* Verwaltung von Relokationslisten                                         */
/*                                                                          */
/* Historie: 25. 7.1999 Grundsteinlegung                                    */
/*            8. 8.1999 Reloc-Liste gespeichert                             */
/*           19. 1.2000 TransferRelocs begonnen                             */
/*           26. 6.2000 added exports                                       */
/*                                                                          */
/****************************************************************************/

extern PRelocEntry LastRelocs;

extern PRelocEntry MergeRelocs(PRelocEntry *list1, PRelocEntry *list2,
                                Boolean Add);
                        
extern void InvertRelocs(PRelocEntry *erg, PRelocEntry *src);

extern void FreeRelocs(PRelocEntry *list);

extern PRelocEntry DupRelocs(PRelocEntry src);

extern void SetRelocs(PRelocEntry List);

extern void TransferRelocs(LargeWord Addr, LongWord Type);

extern void AddExport(char *Name, LargeInt Value);

#endif /* _ASMRELOCS_H */
