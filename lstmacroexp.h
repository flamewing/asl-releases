#ifndef LSTMACROEXP_H
#define LSTMACROEXP_H
/* lstmacroexp.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Functions & variables regarding macro expansion in listing                */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

typedef enum
{
  eLstMacroExpNone = 0,
  eLstMacroExpRest = 1,
  eLstMacroExpIf = 2,
  eLstMacroExpMacro = 4,
  eLstMacroExpAll = eLstMacroExpRest | eLstMacroExpIf | eLstMacroExpMacro
} tLstMacroExp;

#ifdef __cplusplus
#include "cppops.h"
DefCPPOps_Mask(tLstMacroExp)
#endif

#define LSTMACROEXPMOD_MAX 8

typedef struct
{
  unsigned Count;
  Byte Modifiers[LSTMACROEXPMOD_MAX];
} tLstMacroExpMod;

extern void SetLstMacroExp(tLstMacroExp NewMacroExp);
extern tLstMacroExp GetLstMacroExp(void);

extern void InitLstMacroExpMod(tLstMacroExpMod *pLstMacroExpMod);

extern Boolean AddLstMacroExpMod(tLstMacroExpMod *pLstMacroExpMod, Boolean Set, tLstMacroExp Mod);

extern Boolean ChkLstMacroExpMod(const tLstMacroExpMod *pLstMacroExpMod);

extern void DumpLstMacroExpMod(const tLstMacroExpMod *pLstMacroExpMod, char *pDest, int DestLen);

extern tLstMacroExp ApplyLstMacroExpMod(tLstMacroExp Src, const tLstMacroExpMod *pLstMacroExpMod);

#endif /* LSTMACROEXP_H */
