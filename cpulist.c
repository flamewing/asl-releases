/* cpulist.c */
/*****************************************************************************/
/* AS-Port                                                                   */
/*                                                                           */
/* Manages CPU List                                                          */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>
#include "strutil.h"

#include "cpulist.h"

static tpCPUDef FirstCPUDef;                   /* Liste mit Prozessordefinitionen */
static CPUVar CPUCnt;	                       /* Gesamtzahl Prozessoren */
static int MaxNameLen = 0;

/****************************************************************************/
/* neuen Prozessor definieren */

CPUVar AddCPUUser(const char *NewName, tCPUSwitchUserProc Switcher, void *pUserData)
{
  tpCPUDef Lauf, Neu;
  char *p;
  int l;

  Neu = (tpCPUDef) malloc(sizeof(tCPUDef));
  Neu->Name = strdup(NewName);
  /* kein UpString, weil noch nicht initialisiert ! */
  for (p = Neu->Name; *p != '\0'; p++)
    *p = mytoupper(*p);
  Neu->SwitchProc = Switcher;
  Neu->pUserData = pUserData;
  Neu->Next = NULL;
  Neu->Number = Neu->Orig = CPUCnt;

  l = strlen(NewName);
  if (l > MaxNameLen)
    MaxNameLen = l;

  Lauf = FirstCPUDef;
  if (!Lauf)
    FirstCPUDef = Neu;
  else
  {
    while (Lauf->Next)
      Lauf = Lauf->Next;
    Lauf->Next = Neu;
  }

  return CPUCnt++;
}

static void SwitchNoUserProc(void *pUserData)
{
  ((tCPUSwitchProc)pUserData)();
}

CPUVar AddCPU(const char *NewName, tCPUSwitchProc Switcher)
{
  return AddCPUUser(NewName, SwitchNoUserProc, Switcher);
}

Boolean AddCPUAlias(char *OrigName, char *AliasName)
{
  tpCPUDef Lauf = FirstCPUDef, Neu;

  while ((Lauf) && (strcmp(Lauf->Name, OrigName)))
    Lauf = Lauf->Next;

  if (!Lauf)
    return False;
  else
  {
    Neu = (tpCPUDef) malloc(sizeof(tCPUDef));
    Neu->Next = NULL;
    Neu->Name = strdup(AliasName);
    Neu->Number = CPUCnt++;
    Neu->Orig = Lauf->Orig;
    Neu->SwitchProc = Lauf->SwitchProc;
    while (Lauf->Next)
      Lauf = Lauf->Next;
    Lauf->Next = Neu;
    return True;
  }
}

void IterateCPUList(tCPUListIterator Iterator, void *pUser)
{
  tpCPUDef pRun;

  for (pRun= FirstCPUDef; pRun; pRun = pRun->Next)
    Iterator(pRun, pUser);
}

typedef struct
{
  int cnt, perline;
  tCPUSwitchUserProc Proc;
  void *pUserData;
  tPrintNextCPUProc NxtProc;
} tPrintContext;

static void PrintIterator(const tCPUDef *pCPUDef, void *pUser)
{
  tPrintContext *pContext = (tPrintContext*)pUser;

  /* ignore aliases */

  if (pCPUDef->Number == pCPUDef->Orig)
  {
    Boolean Unequal = (pCPUDef->SwitchProc != pContext->Proc)
                   || (pCPUDef->SwitchProc == SwitchNoUserProc && (pContext->pUserData != pCPUDef->pUserData));
    if (Unequal || (pContext->cnt == pContext->perline))
    {
      pContext->Proc = pCPUDef->SwitchProc;
      pContext->pUserData = pCPUDef->pUserData;
      printf("\n");
      pContext->NxtProc();
      pContext->cnt = 0;
    }
    printf("%-*s", MaxNameLen + 1, pCPUDef->Name);
    pContext->cnt++;
  }
}

void PrintCPUList(tPrintNextCPUProc NxtProc)
{
  tPrintContext Context;

  Context.Proc = NULL;
  Context.pUserData = NULL;
  Context.cnt = 0;
  Context.NxtProc = NxtProc;
  Context.perline = 80 / MaxNameLen;
  IterateCPUList(PrintIterator, &Context);
  printf("\n");
  NxtProc();
}

void ClearCPUList(void)
{
  tpCPUDef Save;

  while (FirstCPUDef)
  {
    Save = FirstCPUDef;
    FirstCPUDef = Save->Next;
    free(Save->Name);
    free(Save);
  }
}

const tCPUDef *LookupCPUDefByVar(CPUVar Var)
{
  tpCPUDef pRun;

  for (pRun = FirstCPUDef; pRun; pRun = pRun->Next)
    if (pRun->Number == Var)
      break;
  return pRun;
}

const tCPUDef *LookupCPUDefByName(const char *pName)
{
  tpCPUDef pRun;

  for (pRun = FirstCPUDef; pRun; pRun = pRun->Next)
    if (!strcmp(pRun->Name, pName))
      break;
  return pRun;
}

void cpulist_init(void)
{
  FirstCPUDef = NULL;
  CPUCnt = 0;
}
