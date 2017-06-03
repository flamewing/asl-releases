#include <stdio.h>

#include "lstmacroexp.h"

void InitLstMacroExpMod(tLstMacroExpMod *pLstMacroExpMod)
{
  pLstMacroExpMod->ClrAll =
  pLstMacroExpMod->SetAll = False;
  pLstMacroExpMod->ANDMask =
  pLstMacroExpMod->ORMask = eLstMacroExpNone;
}

Boolean ChkLstMacroExpMod(const tLstMacroExpMod *pLstMacroExpMod)
{
  if (pLstMacroExpMod->ClrAll && pLstMacroExpMod->SetAll)
    return False;
  if (pLstMacroExpMod->ANDMask & pLstMacroExpMod->ORMask)
    return False;
  return True;
}

tLstMacroExp ApplyLstMacroExpMod(tLstMacroExp Src, const tLstMacroExpMod *pLstMacroExpMod)
{
  if (pLstMacroExpMod->ClrAll)
    Src &= ~eLstMacroExpNone;
  Src &= ~pLstMacroExpMod->ANDMask;
  if (pLstMacroExpMod->SetAll)
    Src |= eLstMacroExpAll;
  Src |= pLstMacroExpMod->ORMask;
  return Src;
}
