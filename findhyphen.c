/* findhyphen.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Zerlegung von Worten in Silben gemaess dem TeX-Algorithmus                */
/*                                                                           */
/* Historie: 17.2.1998 Grundsteinlegung                                      */
/*                                                                           */
/*****************************************************************************/

#undef DEBUG

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "strutil.h"

#ifdef DEBUG
#include "ushyph.h"
#endif

#include "findhyphen.h"

/*****************************************************************************/

#define LCNT (26 + 1 + 4)
#define SEPCNT 10

typedef struct sHyphenNode
{
  Byte sepcnts[SEPCNT];
  struct sHyphenNode *Daughters[LCNT];
} THyphenNode, *PHyphenNode;

typedef struct sHyphenException
{
  struct sHyphenException *next;
  char *word;
  int poscnt, *posis;
} THyphenException, *PHyphenException;

/*****************************************************************************/

static PHyphenNode HyphenRoot = NULL;
static PHyphenException FirstException = NULL;

/*****************************************************************************/

#if 0
char b[10];

static void PrintNode(PHyphenNode Node, int Level)
{
  int z;

  for (z = 1; z < Level; z++)
    putchar(' ');
  for (z = 1; z <= Level; z++)
    putchar(b[z]);
  for (z = 0; z < SEPCNT; z++)
    if (Node->sepcnts[z] > 0)
      break;
  if (z < SEPCNT)
    putchar('!');
  printf(" %p", Node);
  puts("");
  for (z = 0; z < LCNT; z++)
    if (Node->Daughters[z])
    {
      b[Level + 1] = z + 'a' - 1;
      PrintNode(Node->Daughters[z], Level + 1);
    }
}
#endif

static int GetIndex(char ch)
{
  if ((as_tolower(ch) >= 'a' - 1) && (as_tolower(ch) <= 'z'))
    return (as_tolower(ch) - ('a' - 1));
  else if (ch == '.')
    return 0;
  else if ((ch == *HYPHEN_CHR_ae) || (ch == *HYPHEN_CHR_AE))
    return 27;
  else if ((ch == *HYPHEN_CHR_oe) || (ch == *HYPHEN_CHR_OE))
    return 28;
  else if ((ch == *HYPHEN_CHR_ue) || (ch == *HYPHEN_CHR_UE))
    return 29;
  else if (ch == *HYPHEN_CHR_sz)
    return 30;
  else
  {
    printf("unallowed character %d\n", ch); return -1;
  }
}

static void InitHyphenNode(PHyphenNode Node)
{
  int z;

  for (z = 0; z < LCNT; Node->Daughters[z++] = NULL);
  for (z = 0; z < SEPCNT; Node->sepcnts[z++] = 0);
}

void BuildTree(char **Patterns)
{
  char **run, ch, *pos, sing[500], *rrun;
  Byte RunCnts[SEPCNT];
  int z, l, rc, index;
  PHyphenNode Lauf;

  HyphenRoot = (PHyphenNode) malloc(sizeof(THyphenNode));
  InitHyphenNode(HyphenRoot);

  for (run = Patterns; *run != NULL; run++)
  {
    strcpy(sing, *run);
    rrun = sing;
    do
    {
      pos = strchr(rrun, ' ');
      if (pos) *pos = '\0';
      l = strlen(rrun);
      rc = 0;
      Lauf = HyphenRoot;
      for (z = 0; z < SEPCNT; RunCnts[z++] = 0);
      for (z = 0; z < l; z++)
      {
        ch = rrun[z];
        if ((ch >= '0') && (ch <= '9'))
          RunCnts[rc] = ch - '0';
        else
        {
          index = GetIndex(ch);
          if (!Lauf->Daughters[index])
          {
            Lauf->Daughters[index] = (PHyphenNode) malloc(sizeof(THyphenNode));
            InitHyphenNode(Lauf->Daughters[index]);
          }
          Lauf = Lauf->Daughters[index];
          rc++;
        }
      }
      memcpy(Lauf->sepcnts, RunCnts, sizeof(Byte)*SEPCNT);
      if (pos)
        rrun = pos + 1;
    }
    while (pos);
  }
}

void AddException(char *Name)
{
  char tmp[300], *dest, *src;
  int pos[100];
  PHyphenException New;

  New = (PHyphenException) malloc(sizeof(THyphenException));
  New->next = FirstException;
  New->poscnt = 0;
  dest = tmp;
  for (src = Name; *src != '\0'; src++)
    if (*src == '-') pos[New->poscnt++] = dest - tmp;
  else
    *(dest++) = *src;
  *dest = '\0';
  New->word = as_strdup(tmp);
  if (New->poscnt)
  {
    New->posis = (int *) malloc(sizeof(int) * New->poscnt);
    memcpy(New->posis, pos, sizeof(int) * New->poscnt);
  }
  else
    New->posis = NULL;
  FirstException = New;
}

void DestroyNode(PHyphenNode Node)
{
  int z;

  for (z = 0; z < LCNT; z++)
    if (Node->Daughters[z]) DestroyNode(Node->Daughters[z]);
  free(Node);
}

void DestroyTree(void)
{
  PHyphenException Old;

  if (HyphenRoot)
    DestroyNode(HyphenRoot);
  HyphenRoot = NULL;

  while (FirstException)
  {
    Old = FirstException;
    FirstException = Old->next;
    free(Old->word);
    if (Old->poscnt > 0)
      free(Old->posis);
    free(Old);
  }
}

void DoHyphens(char *word, int **posis, int *posicnt)
{
  char Field[300];
  Byte Res[300];
  int z, z2, z3, l;
  PHyphenNode Lauf;
  PHyphenException Ex;

  for (Ex = FirstException; Ex; Ex = Ex->next)
    if (!as_strcasecmp(Ex->word, word))
    {
      if (Ex->poscnt)
      {
        *posis = (int *) malloc(sizeof(int) * Ex->poscnt);
        memcpy(*posis, Ex->posis, sizeof(int) * Ex->poscnt);
      }
      else
        *posis = NULL;
      *posicnt = Ex->poscnt;
      return;
    }

  l = strlen(word);
  *posicnt = 0;
  *Field = 'a' - 1;
  for (z = 0; z < l; z++)
  {
    Field[z + 1] = tolower((unsigned int) word[z]);
    if (GetIndex(Field[z + 1]) <= 0)
      return;
  }
  Field[l + 1] = 'a' - 1;
  l += 2;
  for (z = 0; z <= l + 1; Res[z++] = 0);

  if (!HyphenRoot)
    return;

  for (z = 0; z < l; z++)
  {
    Lauf = HyphenRoot;
    for (z2 = z; z2 < l; z2++)
    {
      Lauf = Lauf->Daughters[GetIndex(Field[z2])];
      if (!Lauf)
        break;
#ifdef DEBUG
      for (z3 = 0; z3 < SEPCNT; z3++)
        if (Lauf->sepcnts[z3] > 0)
          break;
      if (z3 < SEPCNT)
      {
        printf("Apply pattern ");
        for (z3 = z; z3 <= z2; putchar(Field[z3++]));
        printf(" at position %d with values", z);
        for (z3 = 0; z3 < SEPCNT; printf(" %d", Lauf->sepcnts[z3++]));
        puts("");
      }
#endif
      for (z3 = 0; z3 <= z2 - z + 2; z3++)
        if (Lauf->sepcnts[z3] > Res[z + z3])
          Res[z + z3] = Lauf->sepcnts[z3];
    }
  }

#ifdef DEBUG
  for (z = 0; z < l; z++)
    printf(" %c", Field[z]);
  puts("");
  for (z = 0; z <= l; z++)
    printf("%d ", Res[z]);
  puts("");
  for (z = 0; z < l - 2; z++)
  {
    if ((z > 0) && ((Res[z + 1]) & 1))
      putchar('-');
    putchar(Field[z + 1]);
  }
  puts("");
#endif

  *posis = (int *) malloc(sizeof(int)*l);
  *posicnt = 0;
  for (z = 3; z < l - 2; z++)
    if ((Res[z]&1) == 1)
      (*posis)[(*posicnt)++] = z - 1;
  if (*posicnt == 0)
  {
    free(*posis);
    *posis = NULL;
  }
}

/*****************************************************************************/

#ifdef DEBUG
int main(int argc, char **argv)
{
  int z, z2, cnt, *posis, posicnt;

  BuildTree(USHyphens);
  for (z = 1; z < argc; z++)
  {
    DoHyphens(argv[z], &posis, &cnt);
    for (z2 = 0; z2 < cnt; printf("%d ", posis[z2++]));
     puts("");
    if (posicnt > 0)
      free(posis);
  }
#if 0
  DoHyphens("hyphenation");
  DoHyphens("concatenation");
  DoHyphens("supercalifragilisticexpialidocous");
#endif
}	
#endif
