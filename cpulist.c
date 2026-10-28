/* cpulist.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Port                                                                   */
/*                                                                           */
/* Manages CPU List                                                          */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "cpulist.h"

#include "strutil.h"

#include <string.h>

#include "c_implementation.h"

static tpCPUDef FirstCPUDef; /* Liste mit Prozessordefinitionen */
static CPUVar   CPUCnt;      /* Gesamtzahl Prozessoren */
static int      MaxNameLen = 0;

/****************************************************************************/
/* neuen Prozessor definieren */

static tpCPUDef AddCPUDef(char const* NewName, tCPUArg const* pArgs) {
    tpCPUDef Lauf, Neu;
    char*    p;
    int      l;

    Neu        = (tpCPUDef)calloc(1, sizeof(tCPUDef));
    Neu->Name  = as_strdup(NewName);
    Neu->pArgs = pArgs;
    /* kein UpString, weil noch nicht initialisiert ! */
    for (p = Neu->Name; *p != '\0'; p++) {
        *p = as_toupper(*p);
    }
    Neu->Next   = NULL;
    Neu->Number = Neu->Orig = CPUCnt;

    l = strlen(NewName);
    if (l > MaxNameLen) {
        MaxNameLen = l;
    }

    Lauf = FirstCPUDef;
    if (!Lauf) {
        FirstCPUDef = Neu;
    } else {
        while (Lauf->Next) {
            Lauf = Lauf->Next;
        }
        Lauf->Next = Neu;
    }

    CPUCnt++;
    return Neu;
}

CPUVar AddCPUUserWithArgs(
        char const* NewName, tCPUSwitchUserProc Switcher, void* pUserData,
        tCPUFreeUserDataProc Freeer, tCPUArg const* pArgs) {
    tpCPUDef cpu    = AddCPUDef(NewName, pArgs);
    cpu->SwitchProc = Switcher;
    cpu->FreeProc   = Freeer;
    cpu->pUserData  = pUserData;
    return cpu->Number;
}

CPUVar AddCPUConstUserWithArgs(
        char const* NewName, tCPUSwitchConstUserProc Switcher, void const* pUserData,
        tCPUArg const* pArgs) {
    tpCPUDef cpu         = AddCPUDef(NewName, pArgs);
    cpu->ConstSwitchProc = Switcher;
    cpu->pConstUserData  = pUserData;
    return cpu->Number;
}

void SwitchCPU(tCPUDef const* pCPUDef) {
    if (pCPUDef->ConstSwitchProc) {
        pCPUDef->ConstSwitchProc(pCPUDef->pConstUserData);
    } else {
        pCPUDef->SwitchProc(pCPUDef->pUserData);
    }
}

typedef struct {
    tCPUSwitchProc Switcher;
} tNoUserData;

static void SwitchNoUserProc(void* pUserData) {
    ((tNoUserData*)pUserData)->Switcher();
}

static void FreeNoUserProc(void* pUserData) {
    free(pUserData);
}

CPUVar AddCPUWithArgs(
        char const* NewName, tCPUSwitchProc Switcher, tCPUArg const* pArgs) {
    tNoUserData* pData = (tNoUserData*)malloc(sizeof(*pData));

    pData->Switcher = Switcher;
    return AddCPUUserWithArgs(NewName, SwitchNoUserProc, pData, FreeNoUserProc, pArgs);
}

Boolean AddCPUAlias(char const* OrigName, char const* AliasName) {
    tpCPUDef Lauf = FirstCPUDef, Neu;

    while ((Lauf) && (strcmp(Lauf->Name, OrigName))) {
        Lauf = Lauf->Next;
    }

    if (!Lauf) {
        return False;
    } else {
        Neu                  = (tpCPUDef)malloc(sizeof(tCPUDef));
        Neu->Next            = NULL;
        Neu->Name            = as_strdup(AliasName);
        Neu->Number          = CPUCnt++;
        Neu->Orig            = Lauf->Orig;
        Neu->SwitchProc      = Lauf->SwitchProc;
        Neu->ConstSwitchProc = Lauf->ConstSwitchProc;
        Neu->FreeProc        = Lauf->FreeProc;
        Neu->pUserData       = Lauf->pUserData;
        Neu->pConstUserData  = Lauf->pConstUserData;
        Neu->pArgs           = Lauf->pArgs;
        while (Lauf->Next) {
            Lauf = Lauf->Next;
        }
        Lauf->Next = Neu;
        return True;
    }
}

void IterateCPUList(tCPUListIterator Iterator, void* pUser) {
    tpCPUDef pRun;

    for (pRun = FirstCPUDef; pRun; pRun = pRun->Next) {
        Iterator(pRun, pUser);
    }
}

typedef struct {
    int                     cnt, perline;
    tCPUSwitchUserProc      Proc;
    tCPUSwitchConstUserProc ConstProc;
    tCPUSwitchProc          NoUserProc;
    tPrintLineCPUProc       PrintProc;
    String                  line_buffer;
} tPrintContext;

static void PrintIterator(tCPUDef const* pCPUDef, void* pUser) {
    tPrintContext* pContext = (tPrintContext*)pUser;

    /* ignore aliases */

    if (pCPUDef->Number == pCPUDef->Orig) {
        tCPUSwitchProc ThisNoUserProc
                = (pCPUDef->SwitchProc == SwitchNoUserProc)
                          ? ((tNoUserData*)pCPUDef->pUserData)->Switcher
                          : NULL;
        Boolean Unequal;

        if ((pCPUDef->SwitchProc != pContext->Proc)
            || (pCPUDef->ConstSwitchProc != pContext->ConstProc)) {
            Unequal = True;
        } else if (pCPUDef->SwitchProc == SwitchNoUserProc) {
            Unequal = (pContext->NoUserProc != ThisNoUserProc);
        } else {
            Unequal = False;
        }
        if (Unequal || (pContext->cnt == pContext->perline)) {
            pContext->Proc       = pCPUDef->SwitchProc;
            pContext->ConstProc  = pCPUDef->ConstSwitchProc;
            pContext->NoUserProc = ThisNoUserProc;
            pContext->PrintProc(pContext->line_buffer);
            pContext->cnt            = 0;
            pContext->line_buffer[0] = '\0';
        }
        as_snprcatf(
                pContext->line_buffer, sizeof(pContext->line_buffer), "%-*s",
                MaxNameLen + 1, pCPUDef->Name);
        pContext->cnt++;
    }
}

void PrintCPUList(tPrintLineCPUProc PrintProc) {
    tPrintContext Context;

    Context.Proc           = NULL;
    Context.ConstProc      = NULL;
    Context.NoUserProc     = NULL;
    Context.cnt            = 0;
    Context.line_buffer[0] = '\0';
    Context.PrintProc      = PrintProc;
    Context.perline        = 79 / (MaxNameLen + 1);
    IterateCPUList(PrintIterator, &Context);
    if (Context.cnt) {
        PrintProc(Context.line_buffer);
    }
    PrintProc("");
}

void ClearCPUList(void) {
    tpCPUDef Save;

    while (FirstCPUDef) {
        Save        = FirstCPUDef;
        FirstCPUDef = Save->Next;
        free(Save->Name);
        if (Save->FreeProc && (Save->Number == Save->Orig)) {
            Save->FreeProc(Save->pUserData);
        }
        free(Save);
    }
}

void ClearCPUAliases(void) {
    tpCPUDef* link = &FirstCPUDef;
    CPUCnt         = 0;
    while (*link) {
        tpCPUDef cpu = *link;
        if (cpu->Number != cpu->Orig) {
            *link = cpu->Next;
            free(cpu->Name);
            free(cpu);
        } else {
            CPUCnt = cpu->Number + 1;
            link   = &cpu->Next;
        }
    }
}

tCPUDef const* LookupCPUDefByVar(CPUVar Var) {
    tpCPUDef pRun;

    for (pRun = FirstCPUDef; pRun; pRun = pRun->Next) {
        if (pRun->Number == Var) {
            break;
        }
    }
    return pRun;
}

tCPUDef const* LookupCPUDefByName(char const* pName) {
    tpCPUDef pRun;

    for (pRun = FirstCPUDef; pRun; pRun = pRun->Next) {
        int l = strlen(pRun->Name);

        if (strncmp(pRun->Name, pName, l)) {
            continue;
        }
        if ((pName[l] == '\0') || (pName[l] == ':')) {
            break;
        }
    }
    return pRun;
}

void cpulist_init(void) {
    FirstCPUDef = NULL;
    CPUCnt      = 0;
}
