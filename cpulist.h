#ifndef CPULIST_H
#define CPULIST_H
/* cpulist.h */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* Manages CPU List                                                          */
/*                                                                           */
/*****************************************************************************/

#include "datatypes.h"

typedef void (*tPrintLineCPUProc)(char const*);

typedef void (*tCPUSwitchProc)(void);
typedef void (*tCPUSwitchUserProc)(void* pUserData);
typedef void (*tCPUSwitchConstUserProc)(void const* pUserData);
typedef void (*tPrintNextCPUProc)(void);
typedef void (*tCPUFreeUserDataProc)(void* pUserData);

typedef unsigned CPUVar;
#define CPUNone ((CPUVar) - 1)

typedef struct sCPUArg {
    char const*   pName;
    LongInt const Min, Max, DefValue;
    LongInt*      pValue;
} tCPUArg;

typedef struct sCPUDef {
    struct sCPUDef*         Next;
    char*                   Name;
    CPUVar                  Number, Orig;
    tCPUSwitchUserProc      SwitchProc;
    tCPUSwitchConstUserProc ConstSwitchProc;
    tCPUFreeUserDataProc    FreeProc;
    void*                   pUserData;
    void const*             pConstUserData;
    tCPUArg const*          pArgs;
} tCPUDef, *tpCPUDef;

typedef void (*tCPUListIterator)(tCPUDef const* pRun, void* pUser);

extern CPUVar AddCPUWithArgs(
        char const* NewName, tCPUSwitchProc Switcher, tCPUArg const* pArgs);
#define AddCPU(NewName, Switcher) AddCPUWithArgs(NewName, Switcher, NULL)
extern CPUVar AddCPUUserWithArgs(
        char const* NewName, tCPUSwitchUserProc Switcher, void* pUserData,
        tCPUFreeUserDataProc Freeer, tCPUArg const* pArgs);
#define AddCPUUser(NewName, Switcher, pUserData, Freeer) \
    AddCPUUserWithArgs(NewName, Switcher, pUserData, Freeer, NULL)

/* Read-only borrowed properties; the registry never owns or frees this data. */
extern CPUVar AddCPUConstUserWithArgs(
        char const* NewName, tCPUSwitchConstUserProc Switcher, void const* pUserData,
        tCPUArg const* pArgs);
#define AddCPUConstUser(NewName, Switcher, pUserData) \
    AddCPUConstUserWithArgs(NewName, Switcher, pUserData, NULL)
extern void SwitchCPU(tCPUDef const* pCPUDef);

extern Boolean AddCPUAlias(char const* OrigName, char const* AliasName);

extern tCPUDef const* LookupCPUDefByVar(CPUVar Var);

extern tCPUDef const* LookupCPUDefByName(char const* pName);

extern void IterateCPUList(tCPUListIterator Iterator, void* pUser);

extern void PrintCPUList(tPrintLineCPUProc PrintProc);

extern void ClearCPUList(void);
extern void ClearCPUAliases(void);

extern void cpulist_init(void);

#endif /* CPULIST_H */
