#ifndef _CPULIST_H
#define _CPULIST_H
/* cpulist.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* Manages CPU List                                                          */
/*                                                                           */
/*****************************************************************************/

typedef void (*tCPUSwitchProc)(
#ifdef __PROTOS__
void
#endif
);

typedef void (*tCPUSwitchUserProc)(
#ifdef __PROTOS__
void *pUserData
#endif
);

typedef void (*tPrintNextCPUProc)(
#ifdef __PROTOS__
void
#endif
);

typedef unsigned CPUVar;
#define CPUNone ((CPUVar)-1)

typedef struct sCPUDef
{
  struct sCPUDef *Next;
  char *Name;
  CPUVar Number, Orig;
  tCPUSwitchUserProc SwitchProc;
  void *pUserData;
} tCPUDef, *tpCPUDef;

typedef void (*tCPUListIterator)(const tCPUDef *pRun, void *pUser);

extern CPUVar AddCPU(const char *NewName, tCPUSwitchProc Switcher);
extern CPUVar AddCPUUser(const char *NewName, tCPUSwitchUserProc Switcher, void *pUserData);

extern Boolean AddCPUAlias(char *OrigName, char *AliasName);

extern const tCPUDef *LookupCPUDefByVar(CPUVar Var);

extern const tCPUDef *LookupCPUDefByName(const char *pName);

extern void IterateCPUList(tCPUListIterator Iterator, void *pUser);

extern void PrintCPUList(tPrintNextCPUProc NxtProc);

extern void ClearCPUList(void);

extern void cpulist_init(void);

#endif /* _CPULIST_H */
