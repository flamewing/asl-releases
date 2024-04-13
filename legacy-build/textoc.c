/* textoc.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS                                                                        */
/*                                                                           */
/* TeX-->ASCII/HTML Converter: Table Of Contents                             */
/*                                                                           */
/*****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "strutil.h"
#include "textoc.h"

/*--------------------------------------------------------------------------*/

typedef struct sTocSave
{
  struct sTocSave *pNext;
  char *pTocName;
} tTocSave, *tpTocSave;

/*--------------------------------------------------------------------------*/

static tpTocSave pFirstTocSave;

/*--------------------------------------------------------------------------*/

/*!------------------------------------------------------------------------
 * \fn     InitToc(void)
 * \brief  initialize TOC
 * ------------------------------------------------------------------------ */

void InitToc(void)
{
  pFirstTocSave = NULL;
}

/*!------------------------------------------------------------------------
 * \fn     AddToc(const char *pLine, unsigned Indent)
 * \brief  add TOC entry
 * \param  pLine title of TOC entry
 * \param  Indent indentation of title
 * ------------------------------------------------------------------------ */

void AddToc(const char *pLine, unsigned Indent)
{
  tpTocSave pNewTocSave, pRunToc;

  pNewTocSave = (tpTocSave) malloc(sizeof(*pNewTocSave));
  pNewTocSave->pNext = NULL;
  pNewTocSave->pTocName = (char *) malloc(1 + Indent + strlen(pLine));
  strcpy(pNewTocSave->pTocName, Blanks(Indent));
  strcat(pNewTocSave->pTocName, pLine);
  if (!pFirstTocSave)
    pFirstTocSave = pNewTocSave;
  else
  {
    for (pRunToc = pFirstTocSave; pRunToc->pNext; pRunToc = pRunToc->pNext);
    pRunToc->pNext = pNewTocSave;
  }
}

/*!------------------------------------------------------------------------
 * \fn     PrintToc(char *pFileName)
 * \brief  dump TOC to file
 * \param  pFileName where to write
 * ------------------------------------------------------------------------ */

void PrintToc(char *pFileName)
{
  tpTocSave pRun;
  FILE *pFile = fopen(pFileName, "w");

  if (!pFile)
    perror(pFileName);

  for (pRun = pFirstTocSave; pRun; pRun = pRun->pNext)
    fprintf(pFile, "%s\n\n", pRun->pTocName);
  fclose(pFile);
}

/*!------------------------------------------------------------------------
 * \fn     FreeToc(void)
 * \brief  free TOC list
 * ------------------------------------------------------------------------ */

void FreeToc(void)
{
  while (pFirstTocSave)
  {
    tpTocSave pOld = pFirstTocSave;
    pFirstTocSave = pOld->pNext;
    free(pOld->pTocName);
    free(pOld);
  }
}
