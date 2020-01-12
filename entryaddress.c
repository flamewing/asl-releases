/* entryaddress.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Maintain entry addresses                                                  */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include "strutil.h"

#include "entryaddress.h"

typedef struct sEntryAddress
{
  struct sEntryAddress *pNext;
  LargeWord Address;
} tEntryAddress;

static tEntryAddress *pFirstEntryAddress;

Boolean EntryAddressAvail(void)
{
  return pFirstEntryAddress != NULL;
}

void AddEntryAddress(LargeWord Address)
{
  tEntryAddress *pRun, *pPrev, *pNew;

  for (pPrev = NULL, pRun = pFirstEntryAddress; pRun; pPrev = pRun, pRun = pRun->pNext)
  {
    if (pRun->Address == Address)
      return;
    if (pRun->Address > Address)
      break;
  }

  pNew = (tEntryAddress*)malloc(sizeof(*pNew));
  pNew->Address = Address;

  pNew->pNext = pRun;
  if (pPrev)
    pPrev->pNext = pNew;
  else
    pFirstEntryAddress = pNew;
}

LargeWord GetEntryAddress(Boolean UsePreferredAddress, LargeWord PreferredAddress)
{
  tEntryAddress *pOld = NULL, *pPrev = NULL;
  LargeWord Result;

  if (!pFirstEntryAddress)
  {
    fprintf(stderr, "internal error: entry address list is empty\n");
    exit(255);
  }

  if (UsePreferredAddress)
  {
    for (pPrev = NULL, pOld = pFirstEntryAddress; pOld; pPrev = pOld, pOld = pOld->pNext)
      if (pOld->Address >= PreferredAddress)
      {
        if (pOld->Address > PreferredAddress)
          pOld = NULL;
        break;
      }
  }
  if (!pOld)
  {
    pOld = pFirstEntryAddress;
    pPrev = NULL;
  }

  if (pPrev)
    pPrev->pNext = pOld->pNext;
  else
    pFirstEntryAddress = pOld->pNext;

  Result = pOld->Address;
  free(pOld);
  return Result;
}

void PrintEntryAddress(FILE *pFile)
{
  tEntryAddress *pRun;
  String Str;

  fprintf(pFile, "(");
  for (pRun = pFirstEntryAddress; pRun; pRun = pRun->pNext)
  {
    as_snprintf(Str, sizeof Str, " %lllx", pRun->Address);
    fprintf(pFile, "%s\n", Str);
  }
  fprintf(pFile, ")\n");
}

void entryaddress_init()
{
  pFirstEntryAddress = NULL;
}
