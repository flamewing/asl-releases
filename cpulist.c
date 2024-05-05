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

static tpCPUDef FirstCPUDef; /* Liste mit Prozessordefinitionen */
static CPUVar   CPUCnt;      /* Gesamtzahl Prozessoren */
static int      MaxNameLen = 0;

/****************************************************************************/
/* neuen Prozessor definieren */

CPUVar AddCPUUserWithArgs(
        char const* NewName, tCPUSwitchUserProc Switcher, void* pUserData,
        tCPUFreeUserDataProc Freeer, tCPUArg const* pArgs) {
    tpCPUDef Lauf, Neu;
    char*    p;
    int      l;

    Neu        = (tpCPUDef)malloc(sizeof(tCPUDef));
    Neu->Name  = as_strdup(NewName);
    Neu->pArgs = pArgs;
    /* kein UpString, weil noch nicht initialisiert ! */
    for (p = Neu->Name; *p != '\0'; p++) {
        *p = as_toupper(*p);
    }
    Neu->SwitchProc = Switcher;
    Neu->FreeProc   = Freeer;
    Neu->pUserData  = pUserData;
    Neu->Next       = NULL;
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

    return CPUCnt++;
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

Boolean AddCPUAlias(char* OrigName, char* AliasName) {
    tpCPUDef Lauf = FirstCPUDef, Neu;

    while ((Lauf) && (strcmp(Lauf->Name, OrigName))) {
        Lauf = Lauf->Next;
    }

    if (!Lauf) {
        return False;
    } else {
        Neu             = (tpCPUDef)malloc(sizeof(tCPUDef));
        Neu->Next       = NULL;
        Neu->Name       = as_strdup(AliasName);
        Neu->Number     = CPUCnt++;
        Neu->Orig       = Lauf->Orig;
        Neu->SwitchProc = Lauf->SwitchProc;
        Neu->FreeProc   = Lauf->FreeProc;
        Neu->pUserData  = Lauf->pUserData;
        Neu->pArgs      = Lauf->pArgs;
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
    int                cnt, perline;
    tCPUSwitchUserProc Proc;
    tCPUSwitchProc     NoUserProc;
    tPrintNextCPUProc  NxtProc;
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

        if (pCPUDef->SwitchProc != pContext->Proc) {
            Unequal = True;
        } else if (pCPUDef->SwitchProc == SwitchNoUserProc) {
            Unequal = (pContext->NoUserProc != ThisNoUserProc);
        } else {
            Unequal = False;
        }
        if (Unequal || (pContext->cnt == pContext->perline)) {
            pContext->Proc       = pCPUDef->SwitchProc;
            pContext->NoUserProc = ThisNoUserProc;
            printf("\n");
            pContext->NxtProc();
            pContext->cnt = 0;
        }
        printf("%-*s", MaxNameLen + 1, pCPUDef->Name);
        pContext->cnt++;
    }
}

void PrintCPUList(tPrintNextCPUProc NxtProc) {
    tPrintContext Context;

    Context.Proc       = NULL;
    Context.NoUserProc = NULL;
    Context.cnt        = 0;
    Context.NxtProc    = NxtProc;
    Context.perline    = 79 / (MaxNameLen + 1);
    IterateCPUList(PrintIterator, &Context);
    printf("\n");
    NxtProc();
}

void ClearCPUList(void) {
    tpCPUDef Save;

    while (FirstCPUDef) {
        Save        = FirstCPUDef;
        FirstCPUDef = Save->Next;
        free(Save->Name);
        if (Save->FreeProc) {
            Save->FreeProc(Save->pUserData);
        }
        free(Save);
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
