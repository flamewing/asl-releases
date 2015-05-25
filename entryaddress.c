/* entryaddress.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Maintain entry addresses                                                  */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

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

LargeWord GetEntryAddress(void)
{
  tEntryAddress *pOld;
  LargeWord Result;

  if (!pFirstEntryAddress)
  {
    fprintf(stderr, "internal error: entry address list is empty\n");
    exit(255);
  }

  pOld = pFirstEntryAddress;
  pFirstEntryAddress = pOld->pNext;

  Result = pOld->Address;
  free(pOld);
  return Result;
}

void entryaddress_init()
{
  pFirstEntryAddress = NULL;
}
