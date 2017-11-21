#ifndef _CPULIST_H
#define _CPULIST_H
/* cpulist.h */
/*****************************************************************************/
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

typedef unsigned CPUVar;
#define CPUNone ((CPUVar)-1)

typedef struct sCPUDef
{
  struct sCPUDef *Next;
  char *Name;
  CPUVar Number, Orig;
  tCPUSwitchProc SwitchProc;
} tCPUDef, *tpCPUDef;

typedef void (*tCPUListIterator)(const tCPUDef *pRun, void *pUser);

extern CPUVar AddCPU(char *NewName, tCPUSwitchProc Switcher);

extern Boolean AddCPUAlias(char *OrigName, char *AliasName);

extern const tCPUDef *LookupCPUDefByVar(CPUVar Var);

extern const tCPUDef *LookupCPUDefByName(const char *pName);

extern void IterateCPUList(tCPUListIterator Iterator, void *pUser);

extern void PrintCPUList(tCPUSwitchProc NxtProc);

extern void ClearCPUList(void);

extern void cpulist_init(void);

#endif /* _CPULIST_H */
