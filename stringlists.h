#ifndef _STRINGLISTS_H
#define _STRINGLISTS_H
/* stringlists.h */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von String-Listen                                              */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef struct _StringRec
         {
          struct _StringRec *Next;
          char *Content;
         } StringRec,*StringRecPtr;
typedef StringRecPtr StringList;

extern void InitStringList(StringList *List);

extern void ClearStringEntry(StringRecPtr *Elem);

extern void ClearStringList(StringList *List);

extern void AddStringListFirst(StringList *List, char *NewStr);

extern void AddStringListLast(StringList *List, char *NewStr);

extern void RemoveStringList(StringList *List, char *OldStr);

extern char *GetStringListFirst(StringList List, StringRecPtr *Lauf);

extern char *GetStringListNext(StringRecPtr *Lauf);

extern char *GetAndCutStringList(StringList *List);

extern Boolean StringListEmpty(StringList List);

extern StringList DuplicateStringList(StringList Src);

extern Boolean StringListPresent(StringList List, char *Search);


extern void stringlists_init(void);
#endif /* _STRINGLISTS_H */
