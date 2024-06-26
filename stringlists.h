#ifndef STRINGLISTS_H
#define STRINGLISTS_H
/* stringlists.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von String-Listen                                              */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

typedef struct sStringRec {
    struct sStringRec* Next;
    char*              Content;
} StringRec, *StringRecPtr;

typedef StringRecPtr StringList;

extern void InitStringList(StringList* List);

extern void ClearStringEntry(StringRecPtr* Elem);

extern void ClearStringList(StringList* List);

extern void AddStringListFirst(StringList* List, char const* NewStr);

extern void AddStringListLast(StringList* List, char const* NewStr);

extern void RemoveStringList(StringList* List, char const* OldStr);

extern char const* GetStringListFirst(StringList List, StringRecPtr* Lauf);

extern char const* GetStringListNext(StringRecPtr* Lauf);

extern char* GetAndCutStringList(StringList* List);

extern Boolean StringListEmpty(StringList List);

extern StringList DuplicateStringList(StringList Src);

extern Boolean StringListPresent(StringList List, char* Search);

extern void DumpStringList(StringList List);
#endif /* STRINGLISTS_H */
