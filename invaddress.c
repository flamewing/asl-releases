/* invaddress.c */
/*****************************************************************************/
/* Disassembler                                                              */
/*                                                                           */
/* inverse symbol storage                                                    */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include "strutil.h"
#include "trees.h"
#include "invaddress.h"

typedef struct
{
  TTree Tree;
  char *pSymbolName;
} tInvSymbol;

static tInvSymbol *pInvSymbolRoot;
static int MaxSymbolNameLen;

static void MakeInvSymbolName(char *pDest, unsigned DestSize, LargeWord Num)
{
  *pDest = 'I';
  HexString2(pDest + 1, DestSize - 1, Num, 8, False);
}

static Boolean InvSymbolAdder(PTree *ppDest, PTree pNew, void *pData)
{
  /*tInvSymbol *pNewInvSymbol = (tInvSymbol*)pNew, *pNode;*/
  UNUSED(pData);

  /* added to an empty leaf ? */

  if (!ppDest)
  {
    return True;
  }
  return False;
}

void AddInvSymbol(const char *pSymbolName, LargeWord Value)
{
  String Name;
  tInvSymbol *pNew;
  PTree pTreeRoot;
  int ThisSymbolNameLen;

  if ((ThisSymbolNameLen = strlen(pSymbolName)) > MaxSymbolNameLen)
    MaxSymbolNameLen = ThisSymbolNameLen;

  MakeInvSymbolName(Name, sizeof(Name), Value);
  pNew = (tInvSymbol*)calloc(1, sizeof(*pNew));
  pNew->Tree.Name = strdup(Name);
  pNew->pSymbolName = strdup(pSymbolName);

  pTreeRoot = &(pInvSymbolRoot->Tree);
  EnterTree(&pTreeRoot, &(pNew->Tree), InvSymbolAdder, NULL);
  pInvSymbolRoot = (tInvSymbol*)pTreeRoot;
}

const char *LookupInvSymbol(LargeWord Value)
{
  String Name;
  TTree *pTree;

  MakeInvSymbolName(Name, sizeof(Name), Value);
  pTree = SearchTree(&pInvSymbolRoot->Tree, Name, 0);
  if (pTree)
    return ((tInvSymbol*)pTree)->pSymbolName;
  else
    return NULL;
}

int GetMaxInvSymbolNameLen(void)
{
  return MaxSymbolNameLen;
}
