/* asmrelocs.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Relokationslisten                                          */
/*                                                                           */
/* Historie: 25. 7.1999 Grundsteinlegung                                     */
/*            8. 8.1999 Reloc-Liste gespeichert                              */
/*****************************************************************************/

extern PRelocEntry LastRelocs;

extern PRelocEntry MergeRelocs(PRelocEntry *list1, PRelocEntry *list2,
                                Boolean Add);
                        
extern void InvertRelocs(PRelocEntry *erg, PRelocEntry *src);

extern void FreeRelocs(PRelocEntry *list);

extern PRelocEntry DupRelocs(PRelocEntry src);

extern void SetRelocs(PRelocEntry List);
