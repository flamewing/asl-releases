#ifndef LSTMACROEXP_H
#define LSTMACROEXP_H

#include "datatypes.h"

typedef enum
{
  eLstMacroExpNone = 0,
  eLstMacroExpRest = 1,
  eLstMacroExpIf = 2,
  eLstMacroExpMacro = 4,
  eLstMacroExpAll = eLstMacroExpRest | eLstMacroExpIf | eLstMacroExpMacro,
} tLstMacroExp;

typedef struct
{
  Boolean SetAll, ClrAll;
  tLstMacroExp ANDMask, ORMask;
} tLstMacroExpMod;

extern void InitLstMacroExpMod(tLstMacroExpMod *pLstMacroExpMod);

extern Boolean ChkLstMacroExpMod(const tLstMacroExpMod *pLstMacroExpMod);

extern tLstMacroExp ApplyLstMacroExpMod(tLstMacroExp Src, const tLstMacroExpMod *pLstMacroExpMod);

#endif /* LSTMACROEXP_H */
