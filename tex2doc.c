/* tex2doc.c */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Konverter TeX-->ASCII-DOC                                                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include "asmitree.h"
#include "chardefs.h"
#include <ctype.h>
#include <string.h>
#include "strutil.h"

#include "findhyphen.h"
#ifndef __MSDOS__
#include "ushyph.h"
#include "grhyph.h"
#endif

/*--------------------------------------------------------------------------*/

#define TOKLEN 250

static char *TableName,
            *BiblioName,
            *ContentsName,
#define ErrorEntryCnt 3
            *ErrorEntryNames[ErrorEntryCnt];

typedef enum
{
  EnvNone, EnvDocument, EnvItemize, EnvEnumerate, EnvDescription, EnvTable,
  EnvTabular, EnvRaggedLeft, EnvRaggedRight, EnvCenter, EnvVerbatim,
  EnvQuote, EnvTabbing, EnvBiblio, EnvMarginPar, EnvCaption, EnvHeading, EnvCount
} EnvType;

typedef enum
{
  FontStandard, FontEmphasized, FontBold, FontTeletype, FontItalic
} TFontType;

typedef enum
{
  FontTiny, FontSmall, FontNormalSize, FontLarge, FontHuge
} TFontSize;

typedef enum
{
  AlignNone, AlignCenter, AlignLeft, AlignRight
} TAlignment;

typedef struct sEnvSave
{
  struct sEnvSave *Next;
  EnvType SaveEnv;
  int ListDepth, ActLeftMargin, LeftMargin, RightMargin;
  int EnumCounter, FontNest;
  TAlignment Alignment;
} TEnvSave, *PEnvSave;

typedef struct sFontSave
{
  struct sFontSave *Next;
  TFontType FontType;
  TFontSize FontSize;
} TFontSave, *PFontSave;

typedef enum
{
  ColLeft, ColRight, ColCenter, ColBar
} TColumn;

#define MAXCOLS 30
#define MAXROWS 200
typedef char *TableLine[MAXCOLS];
typedef struct
{
  int ColumnCount, TColumnCount;
  TColumn ColTypes[MAXCOLS];
  int ColLens[MAXCOLS];
  int LineCnt;
  TableLine Lines[MAXROWS];
  Boolean LineFlags[MAXROWS];
  Boolean MultiFlags[MAXROWS];
} TTable;

typedef struct sRefSave
{
  struct sRefSave *Next;
  char *RefName, *Value;
} TRefSave, *PRefSave;

typedef struct sTocSave
{
  struct sTocSave *Next;
  char *TocName;
} TTocSave, *PTocSave;

static char *EnvNames[EnvCount] =
{
  "___NONE___", "document", "itemize", "enumerate", "description", "table", "tabular",
  "raggedleft", "raggedright", "center", "verbatim", "quote", "tabbing",
  "thebibliography", "___MARGINPAR___", "___CAPTION___", "___HEADING___"
};

static int IncludeNest;
static FILE *infiles[50], *outfile;
static char *infilename;
static char TocName[200];
static int CurrLine = 0, CurrColumn;

#define CHAPMAX 6
static int Chapters[CHAPMAX];
static int TableNum, FontNest, ErrState, FracState, BibIndent, BibCounter;
#define TABMAX 100
static int TabStops[TABMAX], TabStopCnt, CurrTabStop;
static Boolean InAppendix, InMathMode, DoRepass;
static TTable ThisTable;
static int CurrRow, CurrCol;
static Boolean GermanMode;

static EnvType CurrEnv;
static TFontType CurrFontType;
static TFontSize CurrFontSize;
static int CurrListDepth;
static int EnumCounter;
static int ActLeftMargin, LeftMargin, RightMargin;
static TAlignment Alignment;
static PEnvSave EnvStack;
static PFontSave FontStack;
static PRefSave FirstRefSave, FirstCiteSave;
static PTocSave FirstTocSave;

static PInstTable TeXTable;

/*--------------------------------------------------------------------------*/

void ChkStack(void)
{
}

static void error(char *Msg)
{
  int z;

  fprintf(stderr, "%s:%d.%d: %s\n", infilename, CurrLine, CurrColumn, Msg);
  for (z = 0; z < IncludeNest; fclose(infiles[z++]));
  fclose(outfile);
  exit(2);
}

static void warning(char *Msg)
{
  fprintf(stderr, "%s:%d.%d: %s\n", infilename, CurrLine, CurrColumn, Msg);
}

static void SetLang(Boolean IsGerman)
{
  char **pp;

  if (GermanMode == IsGerman)
    return;

  DestroyTree();
  GermanMode = IsGerman;
  if (GermanMode)
  {
    TableName = "Tabelle";
    BiblioName = "Literaturverzeichnis";
    ContentsName = "Inhalt";
    ErrorEntryNames[0] = "Typ";
    ErrorEntryNames[1] = "Ursache";
    ErrorEntryNames[2] = "Argument";
#ifndef __MSDOS__
    BuildTree(GRHyphens);
#endif
  }
  else
  {
    TableName = "Table";
    BiblioName = "Bibliography";
    ContentsName = "Contents";
    ErrorEntryNames[0] = "Type";
    ErrorEntryNames[1] = "Reason";
    ErrorEntryNames[2] = "Argument";
#ifndef __MSDOS__
    BuildTree(USHyphens);
    for (pp = USExceptions; *pp != NULL; pp++)
      AddException(*pp);
#endif
  }
}

/*--------------------------------------------------------------------------*/

static void AddLabel(char *Name, char *Value)
{
  PRefSave Run, Prev, Neu;
  int cmp = -1;
  char err[200];

  for (Run = FirstRefSave, Prev = NULL; Run; Prev = Run, Run = Run->Next)
    if ((cmp = strcmp(Run->RefName, Name)) >= 0)
      break;

  if ((Run) && (cmp == 0))
  {
    if (strcmp(Run->Value, Value))
    {
      as_snprintf(err, sizeof(err), "value of label '%s' has changed", Name);
      warning(err);
      DoRepass = True;
      free(Run->Value);
      Run->Value = as_strdup(Value);
    }
  }
  else
  {
    Neu = (PRefSave) malloc(sizeof(TRefSave));
    Neu->RefName = as_strdup(Name);
    Neu->Value = as_strdup(Value);
    Neu->Next = Run;
    if (!Prev)
      FirstRefSave = Neu;
    else
      Prev->Next = Neu;
  }
}

static void AddCite(char *Name, char *Value)
{
  PRefSave Run, Prev, Neu;
  int cmp = -1;
  char err[200];

  for (Run = FirstCiteSave, Prev = NULL; Run; Prev = Run, Run = Run->Next)
    if ((cmp = strcmp(Run->RefName, Name)) >= 0)
      break;

  if ((Run) && (cmp == 0))
  {
    if (strcmp(Run->Value, Value))
    {
      as_snprintf(err, sizeof(err), "value of citation '%s' has changed", Name);
      warning(err);
      DoRepass = True;
      free(Run->Value);
      Run->Value = as_strdup(Value);
    }
  }
  else
  {
    Neu = (PRefSave) malloc(sizeof(TRefSave));
    Neu->RefName = as_strdup(Name);
    Neu->Value = as_strdup(Value);
    Neu->Next = Run;
    if (!Prev)
      FirstCiteSave = Neu;
    else
      Prev->Next = Neu;
  }
}

static void GetLabel(char *Name, char *Dest)
{
  PRefSave Run;
  char err[200];

  for (Run = FirstRefSave; Run; Run = Run->Next)
    if (!strcmp(Name, Run->RefName))
      break;

  if (!Run)
  {
    as_snprintf(err, sizeof(err), "undefined label '%s'", Name);
    warning(err); DoRepass = True;
  }
  strcpy(Dest, (!Run) ? "???" : Run->Value);
}

static void GetCite(char *Name, char *Dest)
{
  PRefSave Run;
  char err[200];

  for (Run = FirstCiteSave; Run; Run = Run->Next)
    if (!strcmp(Name, Run->RefName))
      break;

  if (!Run)
  {
    as_snprintf(err, sizeof(err), "undefined citation '%s'", Name);
    warning(err);
    DoRepass = True;
  }
  strcpy(Dest, (!Run) ? "???" : Run->Value);
}

static void PrintLabels(char *Name)
{
  PRefSave Run;
  FILE *file = fopen(Name, "a");

  if (!file)
    perror(Name);

  for (Run = FirstRefSave; Run; Run = Run->Next)
    fprintf(file, "Label %s %s\n", Run->RefName, Run->Value);
  fclose(file);
}

static void PrintCites(char *Name)
{
  PRefSave Run;
  FILE *file = fopen(Name, "a");

  if (!file)
    perror(Name);

  for (Run = FirstCiteSave; Run; Run = Run->Next)
    fprintf(file, "Citation %s %s\n", Run->RefName, Run->Value);
  fclose(file);
}

static void PrintToc(char *Name)
{
  PTocSave Run;
  FILE *file = fopen(Name, "w");

  if (!file)
    perror(Name);

  for (Run = FirstTocSave; Run; Run = Run->Next)
    fprintf(file, "%s\n\n", Run->TocName);
  fclose(file);
}

/*------------------------------------------------------------------------------*/

static void GetNext(char *Src, char *Dest)
{
  char *c = strchr(Src, ' ');

  if (!c)
  {
    strcpy(Dest, Src);
    *Src = '\0';
  }
  else
  {
    *c = '\0';
    strcpy(Dest, Src);
    for (c++; *c == ' '; c++);
    strmov(Src, c);
  }
}

static void ReadAuxFile(char *Name)
{
  FILE *file = fopen(Name, "r");
  char Line[300], Cmd[300], Nam[300], Val[300];

  if (!file)
    return;

  while (!feof(file))
  {
    if (!fgets(Line, 299, file))
      break;
    if ((*Line) && (Line[strlen(Line) - 1] == '\n'))
      Line[strlen(Line) - 1] = '\0';
    GetNext(Line, Cmd);
    if (!strcmp(Cmd, "Label"))
    {
      GetNext(Line, Nam);
      GetNext(Line, Val);
      AddLabel(Nam, Val);
    }
    else if (!strcmp(Cmd, "Citation"))
    {
      GetNext(Line, Nam);
      GetNext(Line, Val);
      AddCite(Nam, Val);
    }
  }

  fclose(file);
}

/*--------------------------------------------------------------------------*/

static Boolean issep(char inp)
{
  return ((inp == ' ') || (inp == '\t') || (inp == '\n'));
}

static Boolean isalphanum(char inp)
{
  return ((inp >= 'A') && (inp <= 'Z'))
      || ((inp >= 'a') && (inp <= 'z'))
      || ((inp >= '0') && (inp <= '9'))
      || (inp == '.');
}

static char LastChar = '\0';
static char SaveSep = '\0', SepString[TOKLEN] = "";
static Boolean DidEOF = False;
static char BufferLine[TOKLEN] = "", *BufferPtr = BufferLine;
typedef struct
{
  char Token[TOKLEN], Sep[TOKLEN];
} PushedToken;
static int PushedTokenCnt = 0;
static PushedToken PushedTokens[16];

static int GetChar(void)
{
  Boolean Comment;
  static Boolean DidPar = False;
  char *Result;

  if (*BufferPtr == '\0')
  {
    do
    {
      if (IncludeNest <= 0)
        return EOF;
      do
      {
        Result = fgets(BufferLine, TOKLEN, infiles[IncludeNest - 1]);
        if (Result)
          break;
        fclose(infiles[--IncludeNest]);
        if (IncludeNest <= 0)
          return EOF;
      }
      while (True);
      CurrLine++;
      BufferPtr = BufferLine;
      Comment = (strlen(BufferLine) >= 2) && (!strncmp(BufferLine, "%%", 2));
      if ((*BufferLine == '\0') || (*BufferLine == '\n'))
      {
        if ((CurrEnv == EnvDocument) && (!DidPar))
        {
          strcpy(BufferLine, "\\par\n");
          DidPar = True;
          Comment = False;
        }
      }
      else if (!Comment)
        DidPar = False;
    }
    while (Comment);
  }
  return *(BufferPtr++);
}

static Boolean ReadToken(char *Dest)
{
  int ch, z;
  Boolean Good;
  char *run;

  if (PushedTokenCnt > 0)
  {
    strcpy(Dest, PushedTokens[0].Token);
    strcpy(SepString, PushedTokens[0].Sep);
    for (z = 0; z < PushedTokenCnt - 1; z++)
      PushedTokens[z] = PushedTokens[z + 1];
    PushedTokenCnt--;
    return True;
  }

  if (DidEOF)
    return FALSE;

  CurrColumn = BufferPtr - BufferLine + 1;

  /* falls kein Zeichen gespeichert, fuehrende Blanks ueberspringen */

  *Dest = '\0';
  *SepString = SaveSep;
  run = SepString + ((SaveSep == '\0') ? 0 : 1);
  if (LastChar == '\0')
  {
    do
    {
      ch = GetChar();
      if (ch == '\r')
        ch = GetChar();
      if (issep(ch))
        *(run++) = ' ';
    }
    while ((issep(ch)) && (ch != EOF));
    *run = '\0';
    if (ch == EOF)
    {
      DidEOF = TRUE;
      return FALSE;
    }
  }
  else
  {
    ch = LastChar;
    LastChar = '\0';
  }

  /* jetzt Zeichen kopieren, bis Leerzeichen */

  run = Dest;
  SaveSep = '\0';
  if (isalphanum(*(run++) = ch))
  {
    do
    {
      ch = GetChar();
      Good = (!issep(ch)) && (isalphanum(ch)) && (ch != EOF);
      if (Good)
        *(run++) = ch;
    }
    while (Good);

    /* Dateiende ? */

    if (ch == EOF)
      DidEOF = TRUE;

    /* Zeichen speichern ? */

    else if ((!issep(ch)) && (!isalphanum(ch)))
      LastChar = ch;

    /* Separator speichern ? */

    else if (issep(ch))
      SaveSep = ' ';
  }

  /* Ende */

  *run = '\0';
  return True;
}

static void BackToken(char *Token)
{
  if (PushedTokenCnt >= 16)
    return;
  strcpy(PushedTokens[PushedTokenCnt].Token, Token);
  strcpy(PushedTokens[PushedTokenCnt].Sep, SepString);
  PushedTokenCnt++;
}

/*--------------------------------------------------------------------------*/

static void assert_token(char *ref)
{
  char token[TOKLEN];

  ReadToken(token);
  if (strcmp(ref, token))
  {
    as_snprintf(token, sizeof(token), "\"%s\" expected", ref);
    error(token);
  }
}

static void collect_token(char *dest, char *term)
{
  char Comp[TOKLEN];
  Boolean first = TRUE, done;

  *dest = '\0';
  do
  {
    ReadToken(Comp);
    done = !strcmp(Comp, term);
    if (!done)
    {
      if (!first)
        strcat(dest, SepString);
      strcat(dest, Comp);
    }
    first = False;
  }
  while (!done);
}

/*--------------------------------------------------------------------------*/

static char OutLineBuffer[TOKLEN] = "", SideMargin[TOKLEN];

static void PutLine(Boolean DoBlock)
{
  int ll = RightMargin - LeftMargin + 1;
  int l, n, ptrcnt, diff, div, mod, divmod;
  char *chz, *ptrs[50];
  Boolean SkipFirst, IsFirst;

  fputs(Blanks(LeftMargin - 1), outfile);
  if ((Alignment != AlignNone) || (!DoBlock))
  {
    l = strlen(OutLineBuffer);
    diff = ll - l;
    switch (Alignment)
    {
      case AlignRight:
        fputs(Blanks(diff), outfile);
        l = ll;
        break;
      case AlignCenter:
        fputs(Blanks(diff >> 1), outfile);
        l += diff >> 1;
        break;
      default:
        break;
    }
    fputs(OutLineBuffer, outfile);
  }
  else
  {
    SkipFirst = ((CurrEnv == EnvItemize) || (CurrEnv == EnvEnumerate) || (CurrEnv == EnvDescription) || (CurrEnv == EnvBiblio));
    if (LeftMargin == ActLeftMargin)
      SkipFirst = False;
    l = ptrcnt = 0;
    IsFirst = SkipFirst;
    for (chz = OutLineBuffer; *chz != '\0'; chz++)
    {
      if ((chz > OutLineBuffer) && (*(chz - 1) != ' ') && (*chz == ' '))
      {
        if (!IsFirst)
          ptrs[ptrcnt++] = chz;
        IsFirst = False;
      }
      l++;
    }
    (void)ptrs;
    diff = ll + 1 - l;
    div = (ptrcnt > 0) ? diff / ptrcnt : 0;
    mod = diff - (ptrcnt*div);
    divmod = (mod > 0) ? ptrcnt / mod : ptrcnt + 1;
    IsFirst = SkipFirst;
    ptrcnt = 0;
    for (chz = OutLineBuffer; *chz != '\0'; chz++)
    {
      fputc(*chz, outfile);
      if ((chz > OutLineBuffer) && (*(chz - 1) != ' ') && (*chz == ' '))
      {
        if (!IsFirst)
        {
          n = div;
          if ((mod > 0) && (!(ptrcnt % divmod)))
          {
            mod--;
            n++;
          }
          if (n > 0)
            fputs(Blanks(n), outfile);
          ptrcnt++;
        }
        IsFirst = False;
      }
    }
    l = RightMargin - LeftMargin + 1;
  }
  if (*SideMargin != '\0')
  {
    fputs(Blanks(ll + 3 - l), outfile);
    fprintf(outfile, "%s", SideMargin);
    *SideMargin = '\0';
  }
  fputc('\n', outfile);
  LeftMargin = ActLeftMargin;
}

static void AddLine(const char *Part, char *Sep)
{
  int mlen = RightMargin - LeftMargin + 1, *hyppos, hypcnt, z, hlen;
  char *search, save, *lastalpha;

  if (strlen(Sep) > 1)
    Sep[1] = '\0';
  if (*OutLineBuffer != '\0')
    strcat(OutLineBuffer, Sep);
  strcat(OutLineBuffer, Part);
  if ((int)strlen(OutLineBuffer) >= mlen)
  {
    search = OutLineBuffer + mlen;
    while (search >= OutLineBuffer)
    {
      if (*search == ' ')
        break;
      if (search > OutLineBuffer)
      {
        if (*(search - 1) == '-')
          break;
        else if (*(search - 1) == '/')
          break;
        else if (*(search - 1) == ';')
          break;
        else if (*(search - 1) == ';')
          break;
      }
      search--;
    }
    if (search <= OutLineBuffer)
    {
      PutLine(True);
      *OutLineBuffer = '\0';
    }
    else
    {
      if (*search == ' ')
      {
        for (lastalpha = search + 1; *lastalpha != '\0'; lastalpha++)
          if ((mytolower(*lastalpha) < 'a') || (mytolower(*lastalpha) > 'z'))
            break;
        if (lastalpha - search > 3)
        {
          save = (*lastalpha);
          *lastalpha = '\0';
          DoHyphens(search + 1, &hyppos, &hypcnt);
          *lastalpha = save;
          hlen = -1;
          for (z = 0; z < hypcnt; z++)
            if ((search - OutLineBuffer) + hyppos[z] + 1 < mlen)
              hlen = hyppos[z];
          if (hlen > 0)
          {
            memmove(search + hlen + 2, search + hlen + 1, strlen(search + hlen + 1) + 1);
            search[hlen + 1] = '-';
            search += hlen + 2;
          }
          if (hypcnt > 0)
            free(hyppos);
        }
      }
      save = (*search);
      *search = '\0';
      PutLine(True);
      *search = save;
      for (; *search == ' '; search++);
      strcpy(OutLineBuffer, search);
    }
  }
}

static void AddSideMargin(const char *Part, char *Sep)
{
  if (strlen(Sep) > 1)
    Sep[1] = '\0';
  if (*Sep != '\0')
    if ((*SideMargin != '\0') || (!issep(*Sep)))
      strcat(SideMargin, Sep);
  strcat(SideMargin, Part);
}

static void FlushLine(void)
{
  if (*OutLineBuffer != '\0')
  {
    PutLine(False);
    *OutLineBuffer = '\0';
  }
}

static void ResetLine(void)
{
  *OutLineBuffer = '\0';
}

/*--------------------------------------------------------------------------*/

static void SaveFont(void)
{
  PFontSave NewSave;

  NewSave = (PFontSave) malloc(sizeof(TFontSave));
  NewSave->Next = FontStack;
  NewSave->FontSize = CurrFontSize;
  NewSave->FontType = CurrFontType;
  FontStack = NewSave; FontNest++;
}

static void RestoreFont(void)
{
  PFontSave OldSave;

  if (!FontStack) return;

  OldSave = FontStack;
  FontStack = FontStack->Next;
  CurrFontSize = OldSave->FontSize;
  CurrFontType = OldSave->FontType;
  free(OldSave);
  FontNest--;
}

static void SaveEnv(EnvType NewEnv)
{
  PEnvSave NewSave;

  NewSave = (PEnvSave) malloc(sizeof(TEnvSave));
  NewSave->Next = EnvStack;
  NewSave->ListDepth = CurrListDepth;
  NewSave->LeftMargin = LeftMargin;
  NewSave->Alignment = Alignment;
  NewSave->ActLeftMargin = ActLeftMargin;
  NewSave->RightMargin = RightMargin;
  NewSave->EnumCounter = EnumCounter;
  NewSave->SaveEnv = CurrEnv;
  NewSave->FontNest = FontNest;
  EnvStack = NewSave;
  CurrEnv = NewEnv;
  FontNest = 0;
}

static void RestoreEnv(void)
{
  PEnvSave OldSave;

  OldSave = EnvStack;
  EnvStack = OldSave->Next;
  CurrListDepth = OldSave->ListDepth;
  LeftMargin = OldSave->LeftMargin;
  ActLeftMargin = OldSave->ActLeftMargin;
  RightMargin = OldSave->RightMargin;
  Alignment = OldSave->Alignment;
  EnumCounter = OldSave->EnumCounter;
  FontNest = OldSave->FontNest;
  CurrEnv = OldSave->SaveEnv;
  free(OldSave);
}

static void InitTableRow(int Index)
{
  int z;

  for (z = 0; z < ThisTable.TColumnCount; ThisTable.Lines[Index][z++] = NULL);
  ThisTable.MultiFlags[Index] = False;
  ThisTable.LineFlags[Index] = False;
}

static void NextTableColumn(void)
{
  if (CurrEnv != EnvTabular)
    error("table separation char not within tabular environment");

  if ((ThisTable.MultiFlags[CurrRow])
   || (CurrCol >= ThisTable.TColumnCount))
    error("too many columns within row");

  CurrCol++;
}

static void AddTableEntry(const char *Part, char *Sep)
{
  char *Ptr = ThisTable.Lines[CurrRow][CurrCol];
  int nlen = Ptr ? strlen(Ptr) : 0;
  Boolean UseSep = (nlen > 0);

  if (strlen(Sep) > 1)
    Sep[1] = '\0';
  if (UseSep)
    nlen += strlen(Sep);
  nlen += strlen(Part);
  if (!Ptr)
  {
    Ptr = (char *) malloc(nlen + 1);
    *Ptr = '\0';
  }
  else
  {
    char *NewPtr = (char *) realloc(Ptr, nlen + 1);
    if (NewPtr)
      Ptr = NewPtr;
  }
  if (UseSep)
    strcat(Ptr, Sep);
  strcat(Ptr, Part);
  ThisTable.Lines[CurrRow][CurrCol] = Ptr;
}

static void DoPrnt(char *Ptr, TColumn Align, int len)
{
  int l = (!Ptr) ? 0 : strlen(Ptr), diff;

  len -= 2;
  diff = len - l;
  fputc(' ', outfile);
  switch (Align)
  {
    case ColRight:
      fputs(Blanks(diff), outfile);
      break;
    case ColCenter:
      fputs(Blanks((diff + 1) / 2), outfile);
      break;
    default:
      break;
  }
  if (Ptr)
  {
    fputs(Ptr, outfile);
    free(Ptr);
  }
  switch (Align)
  {
    case ColLeft:
      fputs(Blanks(diff), outfile);
      break;
    case ColCenter:
      fputs(Blanks(diff / 2), outfile);
      break;
    default:
      break;
  }
  fputc(' ', outfile);
}

static void DumpTable(void)
{
  int RowCnt, rowz, colz, colptr, ml, l, diff, sumlen, firsttext, indent;

  /* compute widths of individual rows */
  /* get index of first text column */

  RowCnt = (ThisTable.Lines[CurrRow][0]) ? CurrRow + 1 : CurrRow;
  firsttext = -1;
  for (colz = colptr = 0; colz < ThisTable.ColumnCount; colz++)
    if (ThisTable.ColTypes[colz] == ColBar)
      ThisTable.ColLens[colz] = 1;
    else
    {
      ml = 0;
      for (rowz = 0; rowz < RowCnt; rowz++)
        if ((!ThisTable.LineFlags[rowz]) && (!ThisTable.MultiFlags[rowz]))
        {
          l = (!ThisTable.Lines[rowz][colptr]) ? 0 : strlen(ThisTable.Lines[rowz][colptr]);
          if (ml < l)
            ml = l;
        }
      ThisTable.ColLens[colz] = ml + 2;
      colptr++;
      if (firsttext < 0)
        firsttext = colz;
    }

  /* get total width */

  for (colz = sumlen = 0; colz < ThisTable.ColumnCount; sumlen += ThisTable.ColLens[colz++]);
  indent = (RightMargin - LeftMargin + 1 - sumlen) / 2;
  if (indent < 0)
    indent = 0;

  /* search for multicolumns and extend first field if table is too lean */

  ml = 0;
  for (rowz = 0; rowz < RowCnt; rowz++)
    if ((!ThisTable.LineFlags[rowz]) && (ThisTable.MultiFlags[rowz]))
    {
      l = ThisTable.Lines[rowz][0] ? strlen(ThisTable.Lines[rowz][0]) : 0;
      if (ml < l)
        ml = l;
    }
  if (ml + 4 > sumlen)
  {
    diff = ml + 4 - sumlen;
    ThisTable.ColLens[firsttext] += diff;
  }

  /* print rows */

  for (rowz = 0; rowz < RowCnt; rowz++)
  {
    fputs(Blanks(LeftMargin - 1 + indent), outfile);
    if (ThisTable.MultiFlags[rowz])
    {
      l = sumlen;
      if (ThisTable.ColTypes[0] == ColBar)
      {
        l--;
        fputc('|', outfile);
      }
      if (ThisTable.ColTypes[ThisTable.ColumnCount - 1] == ColBar)
        l--;
      DoPrnt(ThisTable.Lines[rowz][0], ThisTable.ColTypes[firsttext], l);
      if (ThisTable.ColTypes[ThisTable.ColumnCount - 1] == ColBar)
        fputc('|', outfile);
    }
    else
    {
      for (colz = colptr = 0; colz < ThisTable.ColumnCount; colz++)
        if (ThisTable.LineFlags[rowz])
        {
          if (ThisTable.ColTypes[colz] == ColBar)
            fputc('+', outfile);
          else
            for (l = 0; l < ThisTable.ColLens[colz]; l++)
              fputc('-', outfile);
        }
        else
          if (ThisTable.ColTypes[colz] == ColBar)
            fputc('|', outfile);
          else
          {
            DoPrnt(ThisTable.Lines[rowz][colptr], ThisTable.ColTypes[colz], ThisTable.ColLens[colz]);
            colptr++;
          }
    }
    fputc('\n', outfile);
  }
}

static void DoAddNormal(const char *Part, char *Sep)
{
  switch (CurrEnv)
  {
    case EnvMarginPar:
      AddSideMargin(Part, Sep);
      break;
    case EnvTabular:
      AddTableEntry(Part, Sep);
      break;
    default:
      AddLine(Part, Sep);
  }
}

static void GetTableName(char *Dest, int DestSize)
{
  if (InAppendix)
    as_snprintf(Dest, DestSize, "%c.%d", Chapters[0] + 'A', TableNum);
  else
    as_snprintf(Dest, DestSize, "%d.%d", Chapters[0], TableNum);
}

static void GetSectionName(char *Dest, int DestSize)
{
  int z;

  *Dest = '\0';
  for (z = 0; z <= 2; z++)
  {
    if ((z > 0) && (Chapters[z] == 0))
      break;
    if ((InAppendix) && (z == 0))
      as_snprcatf(Dest, DestSize, "%c.", Chapters[z] + 'A');
    else
      as_snprcatf(Dest, DestSize, "%d.", Chapters[z]);
  }
}

/*--------------------------------------------------------------------------*/

static char BackSepString[TOKLEN];

static void TeXFlushLine(Word Index)
{
  UNUSED(Index);

  if (CurrEnv == EnvTabular)
  {
    for (CurrCol++; CurrCol < ThisTable.TColumnCount; ThisTable.Lines[CurrRow][CurrCol++] = as_strdup(""));
    CurrRow++;
    if (CurrRow == MAXROWS)
      error("too many rows in table");
    InitTableRow(CurrRow);
    CurrCol = 0;
  }
  else
  {
    if (*OutLineBuffer == '\0')
      strcpy(OutLineBuffer, " ");
    FlushLine();
  }
  if (CurrEnv == EnvTabbing)
    CurrTabStop = 0;
}

static void TeXKillLine(Word Index)
{
  UNUSED(Index);

  ResetLine();
}

static void TeXDummy(Word Index)
{
  UNUSED(Index);
}

static void TeXDummyNoBrack(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  ReadToken(Token);
}

static void TeXDummyInCurl(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  ReadToken(Token);
  assert_token("}");
}

static void TeXNewCommand(Word Index)
{
  char Token[TOKLEN];
  int level;
  UNUSED(Index);

  assert_token("{");
  assert_token("\\");
  ReadToken(Token);
  assert_token("}");
  ReadToken(Token);
  if (!strcmp(Token, "["))
  {
    ReadToken(Token);
    assert_token("]");
  }
  assert_token("{");
  level = 1;
  do
  {
    ReadToken(Token);
    if (!strcmp(Token, "{"))
      level++;
    else if (!strcmp(Token, "}"))
      level--;
  }
  while (level != 0);
}

static void TeXDef(Word Index)
{
  char Token[TOKLEN];
  int level;
  UNUSED(Index);

  assert_token("\\");
  ReadToken(Token);
  assert_token("{");
  level = 1;
  do
  {
    ReadToken(Token);
    if (!strcmp(Token, "{"))
      level++;
    else if (!strcmp(Token, "}"))
      level--;
  }
  while (level != 0);
}

static void TeXFont(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  assert_token("\\");
  ReadToken(Token);
  assert_token("=");
  ReadToken(Token);
  ReadToken(Token);
  assert_token("\\");
  ReadToken(Token);
}

static void TeXAppendix(Word Index)
{
  int z;
  UNUSED(Index);

  InAppendix = True;
  *Chapters = -1;
  for (z = 1; z < CHAPMAX; Chapters[z++] = 0);
}

static int LastLevel;

static void TeXNewSection(Word Level)
{
  int z;

  if (Level >= CHAPMAX)
    return;

  FlushLine();
  fputc('\n', outfile);

  assert_token("{");
  LastLevel = Level;
  SaveEnv(EnvHeading);
  RightMargin = 200;

  Chapters[Level]++;
  for (z = Level + 1; z < CHAPMAX; Chapters[z++] = 0);
  if (Level == 0)
    TableNum = 0;
}

static void EndSectionHeading(void)
{
  int Level = LastLevel, z;
  char Line[TOKLEN], Title[TOKLEN];
  PTocSave NewTocSave, RunToc;

  strcpy(Title, OutLineBuffer);
  *OutLineBuffer = '\0';

  *Line = '\0';
  if (Level < 3)
  {
    GetSectionName(Line, sizeof(Line));
    as_snprcatf(Line, sizeof(Line), " ");
    if ((Level == 2) && (((strlen(Line) + strlen(Title))&1) == 0))
      as_snprcatf(Line, sizeof(Line), " ");
  }
  as_snprcatf(Line, sizeof(Line), "%s", Title);

  fprintf(outfile, "        %s\n        ", Line);
  for (z = 0; z < (int)strlen(Line); z++)
    switch(Level)
    {
      case 0:
        fputc('=', outfile);
        break;
      case 1:
        fputc('-', outfile);
        break;
      case 2:
        fputc(((z&1) == 0) ? '-' : ' ', outfile);
        break;
      case 3:
        fputc('.', outfile);
        break;
    }
  fprintf(outfile, "\n");

  if (Level < 3)
  {
    NewTocSave = (PTocSave) malloc(sizeof(TTocSave));
    NewTocSave->Next = NULL;
    GetSectionName(Line, sizeof(Line));
    as_snprcatf(Line, sizeof(Line), " %s", Title);
    NewTocSave->TocName = (char *) malloc(6 + Level + strlen(Line));
    strcpy(NewTocSave->TocName, Blanks(5 + Level));
    strcat(NewTocSave->TocName, Line);
    if (!FirstTocSave)
      FirstTocSave = NewTocSave;
    else
    {
      for (RunToc = FirstTocSave; RunToc->Next; RunToc = RunToc->Next);
      RunToc->Next = NewTocSave;
    }
  }
}

static EnvType GetEnvType(char *Name)
{
  EnvType z;

  for (z = EnvNone + 1; z < EnvCount; z++)
    if (!strcmp(Name, EnvNames[z]))
      return z;

  error("unknown environment");
  return EnvNone;
}

static void TeXBeginEnv(Word Index)
{
  char EnvName[TOKLEN], Add[TOKLEN];
  EnvType NEnv;
  Boolean done;
  TColumn NCol;
  int z;
  UNUSED(Index);

  assert_token("{");
  ReadToken(EnvName);
  if ((NEnv = GetEnvType(EnvName)) == EnvTable)
  {
    ReadToken(Add);
    if (!strcmp(Add, "*"))
      assert_token("}");
    else if (strcmp(Add, "}"))
      error("unknown table environment");
  }
  else
    assert_token("}");

  if (NEnv != EnvVerbatim)
    SaveEnv(NEnv);

  switch (NEnv)
  {
    case EnvItemize:
    case EnvEnumerate:
    case EnvDescription:
      FlushLine();
      if (CurrListDepth == 0)
        fputc('\n', outfile);
      ++CurrListDepth;
      ActLeftMargin = LeftMargin = (CurrListDepth * 4) + 1;
      RightMargin = 70;
      EnumCounter = 0;
      break;
    case EnvBiblio:
      FlushLine(); fputc('\n', outfile);
      fprintf(outfile, "        %s\n        ", BiblioName);
      for (z = 0; z < (int)strlen(BiblioName); z++)
        fputc('=', outfile);
      fputc('\n', outfile);
      assert_token("{");
      ReadToken(Add);
      assert_token("}");
      ActLeftMargin = LeftMargin = 4 + (BibIndent = strlen(Add));
      break;
    case EnvVerbatim:
      FlushLine();
      if ((*BufferLine != '\0') && (*BufferPtr != '\0'))
      {
        fprintf(outfile, "%s", BufferPtr);
        *BufferLine = '\0';
        BufferPtr = BufferLine;
      }
      do
      {
        if (!fgets(Add, TOKLEN-1, infiles[IncludeNest - 1]))
          break;
        CurrLine++;
        done = strstr(Add, "\\end{verbatim}") != NULL;
        if (!done)
          fprintf(outfile, "%s", Add);
      }
      while (!done);
      fputc('\n', outfile);
      break;
    case EnvQuote:
      FlushLine();
      fputc('\n', outfile);
      ActLeftMargin = LeftMargin = 5;
      RightMargin = 70;
      break;
    case EnvTabbing:
      FlushLine();
      fputc('\n', outfile);
      TabStopCnt = 0;
      CurrTabStop = 0;
      break;
    case EnvTable:
      ReadToken(Add);
      if (strcmp(Add, "["))
        BackToken(Add);
      else
        do
        {
          ReadToken(Add);
        }
        while (strcmp(Add, "]"));
      FlushLine();
      fputc('\n', outfile);
      ++TableNum;
      break;
    case EnvCenter:
      FlushLine();
      Alignment = AlignCenter;
      break;
    case EnvRaggedRight:
      FlushLine();
      Alignment = AlignLeft;
      break;
    case EnvRaggedLeft:
      FlushLine();
      Alignment = AlignRight;
      break;
    case EnvTabular:
      FlushLine();
      assert_token("{");
      ThisTable.ColumnCount = ThisTable.TColumnCount = 0;
      do
      {
        ReadToken(Add);
        done = !strcmp(Add, "}");
        if (!done)
        {
          if (ThisTable.ColumnCount >= MAXCOLS)
            error("too many columns in table");
          if (!strcmp(Add, "|"))
            NCol = ColBar;
          else if (!strcmp(Add, "l"))
            NCol = ColLeft;
          else if (!strcmp(Add, "r"))
            NCol = ColRight;
          else if (!strcmp(Add, "c"))
            NCol = ColCenter;
          else
          {
            NCol = ColBar;
            error("unknown table column descriptor");
          }
          if ((ThisTable.ColTypes[ThisTable.ColumnCount++] = NCol) != ColBar)
           ThisTable.TColumnCount++;
        }
      }
      while (!done);
      InitTableRow(CurrRow = 0);
      CurrCol = 0;
      break;
    default:
      break;
  }
}

static void TeXEndEnv(Word Index)
{
  char EnvName[TOKLEN], Add[TOKLEN];
  EnvType NEnv;
  UNUSED(Index);

  assert_token("{");
  ReadToken(EnvName);
  if ((NEnv = GetEnvType(EnvName)) == EnvTable)
  {
    ReadToken(Add);
    if (!strcmp(Add, "*"))
      assert_token("}");
    else if (strcmp(Add, "}"))
      error("unknown table environment");
  }
  else
    assert_token("}");

  if (!EnvStack)
    error("end without begin");
  if (CurrEnv != NEnv)
    error("begin and end of environment do not match");

  switch (CurrEnv)
  {
    case EnvItemize:
    case EnvEnumerate:
    case EnvDescription:
      FlushLine();
      if (CurrListDepth == 1)
        fputc('\n', outfile);
      break;
    case EnvBiblio:
    case EnvQuote:
    case EnvTabbing:
      FlushLine();
      fputc('\n', outfile);
      break;
    case EnvCenter:
    case EnvRaggedRight:
    case EnvRaggedLeft:
      FlushLine();
      break;
    case EnvTabular:
      DumpTable();
      break;
    case EnvTable:
      FlushLine();
      fputc('\n', outfile);
      break;
    default:
      break;
  }

  RestoreEnv();
}

static void TeXItem(Word Index)
{
  char NumString[20], Token[TOKLEN], Acc[TOKLEN];
  UNUSED(Index);

  FlushLine();
  switch(CurrEnv)
  {
    case EnvItemize:
      LeftMargin = ActLeftMargin - 3;
      AddLine(" - ", "");
      break;
    case EnvEnumerate:
      LeftMargin = ActLeftMargin - 4;
      as_snprintf(NumString, sizeof(NumString), "%3d ", ++EnumCounter);
      AddLine(NumString, "");
      break;
    case EnvDescription:
      ReadToken(Token);
      if (strcmp(Token, "[")) BackToken(Token);
      else
      {
        collect_token(Acc, "]");
        LeftMargin = ActLeftMargin - 4;
        as_snprintf(NumString, sizeof(NumString), "%3s ", Acc);
        AddLine(NumString, "");
      }
      break;
    default:
      error("\\item not in a list environment");
  }
}

static void TeXBibItem(Word Index)
{
  char NumString[20], Token[TOKLEN], Name[TOKLEN], Format[10];
  UNUSED(Index);

  if (CurrEnv != EnvBiblio)
    error("\\bibitem not in bibliography environment");

  assert_token("{");
  collect_token(Name, "}");

  FlushLine();
  fputc('\n', outfile);
  ++BibCounter;

  LeftMargin = ActLeftMargin - BibIndent - 3;
  as_snprintf(Format, sizeof(Format), "[%%%dd] ", BibIndent);
  as_snprintf(NumString, sizeof(NumString), Format, BibCounter);
  AddLine(NumString, "");
  as_snprintf(NumString, sizeof(NumString), "%d", BibCounter);
  AddCite(Name, NumString);
  ReadToken(Token);
  *SepString = '\0';
  BackToken(Token);
}

static void TeXAddDollar(Word Index)
{
  UNUSED(Index);

  DoAddNormal("$", BackSepString);
}

static void TeXAddUnderbar(Word Index)
{
  UNUSED(Index);

  DoAddNormal("_", BackSepString);
}

#if 0
static void TeXAddPot(Word Index)
{
  UNUSED(Index);

  DoAddNormal("^", BackSepString);
}
#endif

static void TeXAddAmpersand(Word Index)
{
  UNUSED(Index);

  DoAddNormal("&", BackSepString);
}

static void TeXAddAt(Word Index)
{
  UNUSED(Index);

  DoAddNormal("@", BackSepString);
}

static void TeXAddImm(Word Index)
{
  UNUSED(Index);

  DoAddNormal("#", BackSepString);
}

static void TeXAddPercent(Word Index)
{
  UNUSED(Index);

  DoAddNormal("%", BackSepString);
}

static void TeXAddSSharp(Word Index)
{
  UNUSED(Index);

  DoAddNormal(CH_sz, BackSepString);
}

static void TeXAddIn(Word Index)
{
  UNUSED(Index);

  DoAddNormal("in", BackSepString);
}

static void TeXAddReal(Word Index)
{
  UNUSED(Index);

  DoAddNormal("R", BackSepString);
}

static void TeXAddGreekMu(Word Index)
{
  UNUSED(Index);

  DoAddNormal(CH_mu, BackSepString);
}

static void TeXAddGreekPi(Word Index)
{
  UNUSED(Index);

  DoAddNormal("Pi", BackSepString);
}

static void TeXAddLessEq(Word Index)
{
  UNUSED(Index);

  DoAddNormal("<=", BackSepString);
}

static void TeXAddGreaterEq(Word Index)
{
  UNUSED(Index);

  DoAddNormal(">=", BackSepString);
}

static void TeXAddNotEq(Word Index)
{
  UNUSED(Index);

  DoAddNormal("<>", BackSepString);
}

static void TeXAddMid(Word Index)
{
  UNUSED(Index);

  DoAddNormal("|", BackSepString);
}

static void TeXAddRightArrow(Word Index)
{
  UNUSED(Index);

  DoAddNormal("->", BackSepString);
}

static void TeXAddLongRightArrow(Word Index)
{
  UNUSED(Index);

  DoAddNormal("-->", BackSepString);
}

static void TeXAddLeftArrow(Word Index)
{
  UNUSED(Index);

  DoAddNormal("<-", BackSepString);
}

static void TeXAddLeftRightArrow(Word Index)
{
  UNUSED(Index);

  DoAddNormal("<->", BackSepString);
}

static void TeXDoFrac(Word Index)
{
  UNUSED(Index);

  assert_token("{");
  *SepString = '\0';
  BackToken("(");
  FracState = 0;
}

static void NextFracState(void)
{
  if (FracState == 0)
  {
    assert_token("{");
    *SepString = '\0';
    BackToken(")");
    BackToken("/");
    BackToken("(");
  }
  else if (FracState == 1)
  {
    *SepString = '\0';
    BackToken(")");
  }
  if ((++FracState) == 2)
    FracState = -1;
}

static void TeXNewFontType(Word Index)
{
  CurrFontType = (TFontType) Index;
}

static void TeXEnvNewFontType(Word Index)
{
  char NToken[TOKLEN];

  SaveFont();
  CurrFontType = (TFontType) Index;
  assert_token("{");
  ReadToken(NToken);
  strcpy(SepString, BackSepString);
  BackToken(NToken);
}

static void TeXNewFontSize(Word Index)
{
  CurrFontSize = (TFontSize) Index;
}

static void TeXEnvNewFontSize(Word Index)
{
  char NToken[TOKLEN];

  SaveFont();
  CurrFontSize = (TFontSize) Index;
  assert_token("{");
  ReadToken(NToken);
  strcpy(SepString, BackSepString);
  BackToken(NToken);
}

static void TeXAddMarginPar(Word Index)
{
  UNUSED(Index);

  assert_token("{");
  SaveEnv(EnvMarginPar);
}

static void TeXAddCaption(Word Index)
{
  char tmp[100];
  int cnt;
  UNUSED(Index);

  assert_token("{");
  if (CurrEnv != EnvTable)
    error("caption outside of a table");
  FlushLine();
  fputc('\n', outfile);
  SaveEnv(EnvCaption);
  AddLine(TableName, "");
  cnt = strlen(TableName);
  GetTableName(tmp, sizeof(tmp));
  strcat(tmp, ": ");
  AddLine(tmp, " ");
  cnt += 1 + strlen(tmp);
  LeftMargin = 1;
  ActLeftMargin = cnt + 1;
  RightMargin = 70;
}

static void TeXHorLine(Word Index)
{
  UNUSED(Index);

  if (CurrEnv != EnvTabular)
    error("\\hline outside of a table");

  if (ThisTable.Lines[CurrRow][0])
    InitTableRow(++CurrRow);
  ThisTable.LineFlags[CurrRow] = True;
  InitTableRow(++CurrRow);
}

static void TeXMultiColumn(Word Index)
{
  char Token[TOKLEN], *endptr;
  int cnt;
  UNUSED(Index);

  if (CurrEnv != EnvTabular) error("\\hline outside of a table");
  if (CurrCol != 0) error("\\multicolumn must be in first column");

  assert_token("{");
  ReadToken(Token);
  assert_token("}");
  cnt = strtol(Token, &endptr, 10);
  if (*endptr != '\0')
    error("invalid numeric format to \\multicolumn");
  if (cnt != ThisTable.TColumnCount)
    error("\\multicolumn must span entire table");
  assert_token("{");
  do
  {
    ReadToken(Token);
  }
  while (strcmp(Token, "}"));
  ThisTable.MultiFlags[CurrRow] = True;
}

static void TeXIndex(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  do
  {
    ReadToken(Token);
  }
  while (strcmp(Token, "}"));
}

static int GetDim(Double *Factors)
{
  char Acc[TOKLEN];
  static char *UnitNames[] = {"cm", "mm", ""}, **run, *endptr;
  Double Value;

  assert_token("{");
  collect_token(Acc, "}");
  for (run = UnitNames; **run != '\0'; run++)
    if (!strcmp(*run, Acc + strlen(Acc) - strlen(*run)))
      break;
  if (**run == '\0')
    error("unknown unit for dimension");
  Acc[strlen(Acc) - strlen(*run)] = '\0';
  Value = strtod(Acc, &endptr);
  if (*endptr != '\0')
    error("invalid numeric format for dimension");
  return (int)(Value*Factors[run - UnitNames]);
}

static Double HFactors[] = { 4.666666, 0.4666666, 0 };
static Double VFactors[] = { 3.111111, 0.3111111, 0 };

static void TeXHSpace(Word Index)
{
  UNUSED(Index);

  DoAddNormal(Blanks(GetDim(HFactors)), "");
}

static void TeXVSpace(Word Index)
{
  int z, erg;
  UNUSED(Index);

  erg = GetDim(VFactors);
  FlushLine();
  for (z = 0; z < erg; z++)
    fputc('\n', outfile);
}

static void TeXRule(Word Index)
{
  int h = GetDim(HFactors), v = GetDim(VFactors);
  char Rule[200];
  UNUSED(Index);

  for (v = 0; v < h; Rule[v++] = '-');
  Rule[v] = '\0';
  DoAddNormal(Rule, BackSepString);
}

static void TeXAddTabStop(Word Index)
{
  int z, n, p;
  UNUSED(Index);

  if (CurrEnv != EnvTabbing)
    error("tab marker outside of tabbing environment");
  if (TabStopCnt >= TABMAX)
    error("too many tab stops");

  n = strlen(OutLineBuffer);
  for (p = 0; p < TabStopCnt; p++)
    if (TabStops[p] > n)
      break;
  for (z = TabStopCnt - 1; z >= p; z--)
    TabStops[z + 1] = TabStops[z];
  TabStops[p] = n;
  TabStopCnt++;
}

static void TeXJmpTabStop(Word Index)
{
  int diff;
  UNUSED(Index);

  if (CurrEnv != EnvTabbing)
    error("tab trigger outside of tabbing environment");
  if (CurrTabStop >= TabStopCnt)
    error("not enough tab stops");

  diff = TabStops[CurrTabStop] - strlen(OutLineBuffer);
  if (diff > 0)
    DoAddNormal(Blanks(diff), "");
  CurrTabStop++;
}

static void TeXDoVerb(Word Index)
{
  char Token[TOKLEN], *pos, Marker;
  UNUSED(Index);

  ReadToken(Token);
  if (*SepString != '\0')
    error("invalid control character for \\verb");
  Marker = (*Token);
  strmov(Token, Token + 1);
  strcpy(SepString, BackSepString);
  do
  {
    DoAddNormal(SepString, "");
    pos = strchr(Token, Marker);
    if (pos)
    {
      *pos = '\0';
      DoAddNormal(Token, "");
      *SepString = '\0';
      BackToken(pos + 1);
      break;
    }
    else
    {
      DoAddNormal(Token, "");
      ReadToken(Token);
    }
  }
  while (True);
}

static void TeXErrEntry(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  ReadToken(Token);
  assert_token("}");
  assert_token("{");
  *SepString = '\0';
  BackToken("\\");
  BackToken("item");
  BackToken("[");
  BackToken(Token);
  BackToken("]");
  ErrState = 0;
}

static void NextErrState(void)
{
  if (ErrState < 3)
    assert_token("{");
  if (ErrState == 0)
  {
    *SepString = '\0';
    BackToken("\\");
    BackToken("begin");
    BackToken("{");
    BackToken("description");
    BackToken("}");
  }
  if ((ErrState >= 0) && (ErrState <= 2))
  {
    *SepString = '\0';
    BackToken("\\");
    BackToken("item");
    BackToken("[");
    BackToken(ErrorEntryNames[ErrState]);
    BackToken(":");
    BackToken("]");
    BackToken("\\");
    BackToken("\\");
  }
  if (ErrState == 3)
  {
    *SepString = '\0';
    BackToken("\\");
    BackToken("\\");
    BackToken(" ");
    BackToken("\\");
    BackToken("end");
    BackToken("{");
    BackToken("description");
    BackToken("}");
    ErrState = -1;
  }
  else
    ErrState++;
}

static void TeXWriteLabel(Word Index)
{
  char Name[TOKLEN], Value[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  collect_token(Name, "}");

  if (CurrEnv == EnvCaption)
    GetTableName(Value, sizeof(Value));
  else
  {
    GetSectionName(Value, sizeof(Value));
    if ((*Value) && (Value[strlen(Value) - 1] == '.'))
      Value[strlen(Value) - 1] = '\0';
  }

  AddLabel(Name, Value);
}

static void TeXWriteRef(Word Index)
{
  char Name[TOKLEN], Value[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  collect_token(Name, "}");
  GetLabel(Name, Value);
  DoAddNormal(Value, BackSepString);
}

static void TeXWriteCitation(Word Index)
{
  char Name[TOKLEN], Value[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  collect_token(Name, "}");
  GetCite(Name, Value);
  as_snprintf(Name, sizeof(Name), "[%s]", Value);
  DoAddNormal(Name, BackSepString);
}

static void TeXNewParagraph(Word Index)
{
  UNUSED(Index);

  FlushLine();
  fputc('\n', outfile);
}

static void TeXContents(Word Index)
{
  FILE *file = fopen(TocName, "r");
  char Line[200];
  UNUSED(Index);

  if (!file)
  {
    warning("contents file not found.");
    DoRepass = True;
    return;
  }

  FlushLine();
  fprintf(outfile, "        %s\n\n", ContentsName);
  while (!feof(file))
  {
    if (!fgets(Line, 199, file))
      break;
    fputs(Line, outfile);
  }

  fclose(file);
}

static void TeXParSkip(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  ReadToken(Token);
  do
  {
    ReadToken(Token);
    if ((!strncmp(Token, "plus", 4)) || (!strncmp(Token, "minus", 5)))
    {
    }
    else
    {
      BackToken(Token);
      return;
    }
  }
  while (1);
}

static void TeXNLS(Word Index)
{
  char Token[TOKLEN], *Repl = "";
  Boolean Found = True;
  UNUSED(Index);

  *Token = '\0';
  ReadToken(Token);
  if (*SepString == '\0')
    switch (*Token)
    {
      case 'a':
        Repl = CH_ae;
        break;
      case 'e':
        Repl = CH_ee;
        break;
      case 'i':
        Repl = CH_ie;
        break;
      case 'o':
        Repl = CH_oe;
        break;
      case 'u':
        Repl = CH_ue;
        break;
      case 'A':
        Repl = CH_Ae;
        break;
      case 'E':
        Repl = CH_Ee;
        break;
      case 'I':
        Repl = CH_Ie;
        break;
      case 'O':
        Repl = CH_Oe;
        break;
      case 'U':
        Repl = CH_Ue;
        break;
      case 's':
        Repl = CH_sz;
        break;
      default :
        Found = False;
    }
  else
    Found = False;

  if (Found)
  {
    if (strlen(Repl) > 1)
      memmove(Token + strlen(Repl), Token + 1, strlen(Token));
    memcpy(Token, Repl, strlen(Repl));
    strcpy(SepString, BackSepString);
  }
  else
    DoAddNormal("\"", BackSepString);

  BackToken(Token);
}

static void TeXNLSGrave(Word Index)
{
  char Token[TOKLEN], *Repl = "";
  Boolean Found = True;
  UNUSED(Index);

  *Token = '\0';
  ReadToken(Token);
  if (*SepString == '\0')
    switch (*Token)
    {
      case 'a':
        Repl = CH_agrave;
        break;
      case 'A':
        Repl = CH_Agrave;
        break;
      case 'e':
        Repl = CH_egrave;
        break;
      case 'E':
        Repl = CH_Egrave;
        break;
      case 'i':
        Repl = CH_igrave;
        break;
      case 'I':
        Repl = CH_Igrave;
        break;
      case 'o':
        Repl = CH_ograve;
        break;
      case 'O':
        Repl = CH_Ograve;
        break;
      case 'u':
        Repl = CH_ugrave;
        break;
      case 'U':
        Repl = CH_Ugrave;
        break;
      default:
        Found = False;
    }
  else
    Found = False;

  if (Found)
  {
    if (strlen(Repl) > 1)
      memmove(Token + strlen(Repl), Token + 1, strlen(Token));
    memcpy(Token, Repl, strlen(Repl));
    strcpy(SepString, BackSepString);
  }
  else
    DoAddNormal("\"", BackSepString);

  BackToken(Token);
}

static void TeXNLSAcute(Word Index)
{
  char Token[TOKLEN], *Repl = "";
  Boolean Found = True;
  UNUSED(Index);

  *Token = '\0';
  ReadToken(Token);
  if (*SepString == '\0')
    switch (*Token)
    {
      case 'a':
        Repl = CH_aacute;
        break;
      case 'A':
        Repl = CH_Aacute;
        break;
      case 'e':
        Repl = CH_eacute;
        break;
      case 'E':
        Repl = CH_Eacute;
        break;
      case 'i':
        Repl = CH_iacute;
        break;
      case 'I':
        Repl = CH_Iacute;
        break;
      case 'o':
        Repl = CH_oacute;
        break;
      case 'O':
        Repl = CH_Oacute;
        break;
      case 'u':
        Repl = CH_uacute;
        break;
      case 'U':
        Repl = CH_Uacute;
        break;
      default:
        Found = False;
    }
  else
    Found = False;

  if (Found)
  {
    if (strlen(Repl) > 1)
      memmove(Token + strlen(Repl), Token + 1, strlen(Token));
    memcpy(Token, Repl, strlen(Repl));
    strcpy(SepString, BackSepString);
  }
  else
    DoAddNormal("\"", BackSepString);

  BackToken(Token);
}

static void TeXNLSCirc(Word Index)
{
  char Token[TOKLEN], *Repl = "";
  Boolean Found = True;
  UNUSED(Index);

  *Token = '\0';
  ReadToken(Token);
  if (*SepString == '\0')
    switch (*Token)
    {
      case 'a':
        Repl = CH_acirc;
        break;
      case 'A':
        Repl = CH_Acirc;
        break;
      case 'e':
        Repl = CH_ecirc;
        break;
      case 'E':
        Repl = CH_Ecirc;
        break;
      case 'i':
        Repl = CH_icirc;
        break;
      case 'I':
        Repl = CH_Icirc;
        break;
      case 'o':
        Repl = CH_ocirc;
        break;
      case 'O':
        Repl = CH_Ocirc;
        break;
      case 'u':
        Repl = CH_ucirc;
        break;
      case 'U':
        Repl = CH_Ucirc;
        break;
      default:
        Found = False;
    }
  else
    Found = False;

  if (Found)
  {
    if (strlen(Repl) > 1)
      memmove(Token + strlen(Repl), Token + 1, strlen(Token));
    memcpy(Token, Repl, strlen(Repl));
    strcpy(SepString, BackSepString);
  }
  else
    DoAddNormal("\"", BackSepString);

  BackToken(Token);
}

static void TeXNLSTilde(Word Index)
{
  char Token[TOKLEN], *Repl = "";
  Boolean Found = True;
  UNUSED(Index);

  *Token = '\0';
  ReadToken(Token);
  if (*SepString == '\0')
    switch (*Token)
    {
      case 'n':
        Repl = CH_ntilde;
        break;
      case 'N':
        Repl = CH_Ntilde;
        break;
      default:
        Found = False;
    }
  else
    Found = False;

  if (Found)
  {
    if (strlen(Repl) > 1)
      memmove(Token + strlen(Repl), Token + 1, strlen(Token));
    memcpy(Token, Repl, strlen(Repl));
    strcpy(SepString, BackSepString);
  }
  else
    DoAddNormal("\"", BackSepString);

  BackToken(Token);
}

static void TeXCedilla(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  collect_token(Token, "}");
  if (!strcmp(Token, "c"))
    strcpy(Token, CH_ccedil);
  if (!strcmp(Token, "C"))
    strcpy(Token, CH_Ccedil);

  DoAddNormal(Token, BackSepString);
}

static void TeXAsterisk(Word Index)
{
  (void)Index;
  DoAddNormal("*", BackSepString);
}

static Boolean TeXNLSSpec(char *Line)
{
  Boolean Found = True;
  char *Repl = NULL;
  int cnt = 0;

  if (*SepString == '\0')
    switch (*Line)
    {
      case 'o':
        cnt = 1;
        Repl = CH_oslash;
        break;
      case 'O':
        cnt = 1;
        Repl = CH_Oslash;
        break;
      case 'a':
        switch (Line[1])
        {
          case 'a':
            cnt = 2;
            Repl = CH_aring;
            break;
          case 'e':
            cnt = 2;
            Repl = CH_aelig;
            break;
          default:
            Found = False;
        }
        break;
      case 'A':
        switch (Line[1])
        {
          case 'A':
            cnt = 2;
            Repl = CH_Aring;
            break;
          case 'E':
            cnt = 2;
            Repl = CH_Aelig;
            break;
          default:
            Found = False;
        }
        break;
      default:
        Found = False;
    }

  if (Found)
  {
    if ((int)strlen(Repl) != cnt)
      memmove(Line + strlen(Repl), Line + cnt, strlen(Line) - cnt + 1);
    memcpy(Line, Repl, strlen(Repl));
    strcpy(SepString, BackSepString);
  }
  else
    DoAddNormal("\"", BackSepString);

  BackToken(Line);
  return Found;
}

static void TeXHyphenation(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  collect_token(Token, "}");
  AddException(Token);
}

static void TeXDoPot(void)
{
  char Token[TOKLEN];

  ReadToken(Token);
  if (*Token == '2')
  {
    if (strlen(CH_e2) > 1)
      memmove(Token + strlen(CH_e2), Token + 1, strlen(Token));
    memcpy(Token, CH_e2, strlen(CH_e2));
  }
  else
    DoAddNormal("^", BackSepString);

  BackToken(Token);
}

static void TeXDoSpec(void)
{
  strcpy(BackSepString, SepString);
  TeXNLS(0);
}

static void TeXInclude(Word Index)
{
  char Token[TOKLEN], Msg[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  collect_token(Token, "}");
  infiles[IncludeNest] = fopen(Token, "r");
  if (!infiles[IncludeNest])
  {
    as_snprintf(Msg, sizeof(Msg), "file %s not found", Token);
    error(Msg);
  }
  else
    IncludeNest++;
}

static void TeXDocumentStyle(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  ReadToken(Token);
  if (!strcmp(Token, "["))
  {
    do
    {
      ReadToken(Token);
      if (!strcmp(Token, "german"))
        SetLang(True);
    }
    while (strcmp(Token, "]"));
    assert_token("{");
    ReadToken(Token);
    if (!strcasecmp(Token,  "article"))
    {
      AddInstTable(TeXTable, "section", 0, TeXNewSection);
      AddInstTable(TeXTable, "subsection", 1, TeXNewSection);
      AddInstTable(TeXTable, "subsubsection", 3, TeXNewSection);
    }
    else
    {
      AddInstTable(TeXTable, "chapter", 0, TeXNewSection);
      AddInstTable(TeXTable, "section", 1, TeXNewSection);
      AddInstTable(TeXTable, "subsection", 2, TeXNewSection);
      AddInstTable(TeXTable, "subsubsection", 3, TeXNewSection);
    }
    assert_token("}");
  }
}

static void TeXUsePackage(Word Index)
{
  char Token[TOKLEN];
  UNUSED(Index);

  assert_token("{");
  ReadToken(Token);
  if (!strcmp(Token, "german"))
    SetLang(True);
  assert_token("}");
}

/*--------------------------------------------------------------------------*/

int main(int argc, char **argv)
{
  char Line[TOKLEN], Comp[TOKLEN], *p, AuxFile[200];
  int z;

  if (argc < 3)
  {
    fprintf(stderr, "calling convention: %s <input file> <output file>\n", *argv);
    exit(1);
  }

  TeXTable = CreateInstTable(301);

  IncludeNest = 0;
  *infiles = fopen(argv[1], "r");
  if (!*infiles)
  {
    perror(argv[1]);
    exit(3);
  }
  else
    IncludeNest++;
  infilename = argv[1];
  if (!strcmp(argv[2], "-"))
    outfile = stdout;
  else
  {
    outfile = fopen(argv[2], "w");
    if (!outfile)
    {
      perror(argv[2]);
      exit(3);
    }
  }

  AddInstTable(TeXTable, "\\", 0, TeXFlushLine);
  AddInstTable(TeXTable, "par", 0, TeXNewParagraph);
  AddInstTable(TeXTable, "-", 0, TeXDummy);
  AddInstTable(TeXTable, "hyphenation", 0, TeXHyphenation);
  AddInstTable(TeXTable, "kill", 0, TeXKillLine);
  AddInstTable(TeXTable, "/", 0, TeXDummy);
  AddInstTable(TeXTable, "pagestyle", 0, TeXDummyInCurl);
  AddInstTable(TeXTable, "thispagestyle", 0, TeXDummyInCurl);
  AddInstTable(TeXTable, "sloppy", 0, TeXDummy);
  AddInstTable(TeXTable, "clearpage", 0, TeXDummy);
  AddInstTable(TeXTable, "cleardoublepage", 0, TeXDummy);
  AddInstTable(TeXTable, "topsep", 0, TeXDummyNoBrack);
  AddInstTable(TeXTable, "parskip", 0, TeXParSkip);
  AddInstTable(TeXTable, "parindent", 0, TeXDummyNoBrack);
  AddInstTable(TeXTable, "textwidth", 0, TeXDummyNoBrack);
  AddInstTable(TeXTable, "evensidemargin", 0, TeXDummyNoBrack);
  AddInstTable(TeXTable, "oddsidemargin", 0, TeXDummyNoBrack);
  AddInstTable(TeXTable, "newcommand", 0, TeXNewCommand);
  AddInstTable(TeXTable, "def", 0, TeXDef);
  AddInstTable(TeXTable, "font", 0, TeXFont);
  AddInstTable(TeXTable, "documentstyle", 0, TeXDocumentStyle);
  AddInstTable(TeXTable, "documentclass", 0, TeXDocumentStyle);
  AddInstTable(TeXTable, "usepackage", 0, TeXUsePackage);
  AddInstTable(TeXTable, "appendix", 0, TeXAppendix);
  AddInstTable(TeXTable, "makeindex", 0, TeXDummy);
  AddInstTable(TeXTable, "begin", 0, TeXBeginEnv);
  AddInstTable(TeXTable, "end", 0, TeXEndEnv);
  AddInstTable(TeXTable, "item", 0, TeXItem);
  AddInstTable(TeXTable, "bibitem", 0, TeXBibItem);
  AddInstTable(TeXTable, "errentry", 0, TeXErrEntry);
  AddInstTable(TeXTable, "$", 0, TeXAddDollar);
  AddInstTable(TeXTable, "_", 0, TeXAddUnderbar);
  AddInstTable(TeXTable, "&", 0, TeXAddAmpersand);
  AddInstTable(TeXTable, "@", 0, TeXAddAt);
  AddInstTable(TeXTable, "#", 0, TeXAddImm);
  AddInstTable(TeXTable, "%", 0, TeXAddPercent);
  AddInstTable(TeXTable, "ss", 0, TeXAddSSharp);
  AddInstTable(TeXTable, "in", 0, TeXAddIn);
  AddInstTable(TeXTable, "rz", 0, TeXAddReal);
  AddInstTable(TeXTable, "mu", 0, TeXAddGreekMu);
  AddInstTable(TeXTable, "pi", 0, TeXAddGreekPi);
  AddInstTable(TeXTable, "leq", 0, TeXAddLessEq);
  AddInstTable(TeXTable, "geq", 0, TeXAddGreaterEq);
  AddInstTable(TeXTable, "neq", 0, TeXAddNotEq);
  AddInstTable(TeXTable, "mid", 0, TeXAddMid);
  AddInstTable(TeXTable, "frac", 0, TeXDoFrac);
  AddInstTable(TeXTable, "rm", FontStandard, TeXNewFontType);
  AddInstTable(TeXTable, "em", FontEmphasized, TeXNewFontType);
  AddInstTable(TeXTable, "bf", FontBold, TeXNewFontType);
  AddInstTable(TeXTable, "tt", FontTeletype, TeXNewFontType);
  AddInstTable(TeXTable, "it", FontItalic, TeXNewFontType);
  AddInstTable(TeXTable, "bb", FontBold, TeXEnvNewFontType);
  AddInstTable(TeXTable, "tty", FontTeletype, TeXEnvNewFontType);
  AddInstTable(TeXTable, "ii", FontItalic, TeXEnvNewFontType);
  AddInstTable(TeXTable, "tiny", FontTiny, TeXNewFontSize);
  AddInstTable(TeXTable, "small", FontSmall, TeXNewFontSize);
  AddInstTable(TeXTable, "normalsize", FontNormalSize, TeXNewFontSize);
  AddInstTable(TeXTable, "large", FontLarge, TeXNewFontSize);
  AddInstTable(TeXTable, "huge", FontHuge, TeXNewFontSize);
  AddInstTable(TeXTable, "tin", FontTiny, TeXEnvNewFontSize);
  AddInstTable(TeXTable, "rightarrow", 0, TeXAddRightArrow);
  AddInstTable(TeXTable, "longrightarrow", 0, TeXAddLongRightArrow);
  AddInstTable(TeXTable, "leftarrow", 0, TeXAddLeftArrow);
  AddInstTable(TeXTable, "leftrightarrow", 0, TeXAddLeftRightArrow);
  AddInstTable(TeXTable, "marginpar", 0, TeXAddMarginPar);
  AddInstTable(TeXTable, "caption", 0, TeXAddCaption);
  AddInstTable(TeXTable, "label", 0, TeXWriteLabel);
  AddInstTable(TeXTable, "ref", 0, TeXWriteRef);
  AddInstTable(TeXTable, "cite", 0, TeXWriteCitation);
  AddInstTable(TeXTable, "hline", 0, TeXHorLine);
  AddInstTable(TeXTable, "multicolumn", 0, TeXMultiColumn);
  AddInstTable(TeXTable, "ttindex", 0, TeXIndex);
  AddInstTable(TeXTable, "hspace", 0, TeXHSpace);
  AddInstTable(TeXTable, "vspace", 0, TeXVSpace);
  AddInstTable(TeXTable, "=", 0, TeXAddTabStop);
  AddInstTable(TeXTable, ">", 0, TeXJmpTabStop);
  AddInstTable(TeXTable, "verb", 0, TeXDoVerb);
  AddInstTable(TeXTable, "printindex", 0, TeXDummy);
  AddInstTable(TeXTable, "tableofcontents", 0, TeXContents);
  AddInstTable(TeXTable, "rule", 0, TeXRule);
  AddInstTable(TeXTable, "\"", 0, TeXNLS);
  AddInstTable(TeXTable, "`", 0, TeXNLSGrave);
  AddInstTable(TeXTable, "'", 0, TeXNLSAcute);
  AddInstTable(TeXTable, "^", 0, TeXNLSCirc);
  AddInstTable(TeXTable, "~", 0, TeXNLSTilde);
  AddInstTable(TeXTable, "c", 0, TeXCedilla);
  AddInstTable(TeXTable, "*", 0, TeXAsterisk);
  AddInstTable(TeXTable, "newif", 0, TeXDummy);
  AddInstTable(TeXTable, "fi", 0, TeXDummy);
  AddInstTable(TeXTable, "ifelektor", 0, TeXDummy);
  AddInstTable(TeXTable, "elektortrue", 0, TeXDummy);
  AddInstTable(TeXTable, "elektorfalse", 0, TeXDummy);
  AddInstTable(TeXTable, "input", 0, TeXInclude);

  for (z = 0; z < CHAPMAX; Chapters[z++] = 0);
  TableNum = 0;
  FontNest = 0;
  TabStopCnt = 0;
  CurrTabStop = 0;
  ErrState = FracState = -1;
  InAppendix = False;
  EnvStack = NULL;
  CurrEnv = EnvNone;
  CurrListDepth = 0;
  ActLeftMargin = LeftMargin = 1;
  RightMargin = 70;
  Alignment = AlignNone;
  EnumCounter = 0;
  CurrFontType = FontStandard;
  CurrFontSize = FontNormalSize;
  FontStack = NULL;
  FirstRefSave = NULL;
  FirstCiteSave = NULL;
  FirstTocSave = NULL;
  *SideMargin = '\0';
  DoRepass = False;
  BibIndent = BibCounter = 0;
  GermanMode = True;
  SetLang(False);

  strcpy(TocName, argv[1]);
  p = strrchr(TocName, '.');
  if (p)
    *p = '\0';
  strcat(TocName, ".dtoc");

  strcpy(AuxFile, argv[1]);
  p = strrchr(AuxFile, '.');
  if (p)
    *p = '\0';
  strcat(AuxFile, ".daux");
  ReadAuxFile(AuxFile);

  while (1)
  {
    if (!ReadToken(Line))
      break;
    if (!strcmp(Line, "\\"))
    {
      strcpy(BackSepString, SepString);
      if (!ReadToken(Line))
        error("unexpected end of file");
      if (*SepString != '\0')
        BackToken(Line);
      else if (!LookupInstTable(TeXTable, Line))
        if (!TeXNLSSpec(Line))
        {
          as_snprintf(Comp, sizeof(Comp), "unknown TeX command %s", Line);
          warning(Comp);
        }
    }
    else if (!strcmp(Line, "$"))
    {
      InMathMode = !InMathMode;
      if (InMathMode)
      {
        strcpy(BackSepString, SepString);
        ReadToken(Line);
        strcpy(SepString, BackSepString);
        BackToken(Line);
      }
    }
    else if (!strcmp(Line, "&"))
      NextTableColumn();
    else if ((!strcmp(Line, "^")) && (InMathMode))
      TeXDoPot();
    else if ((!strcmp(Line, "\"")) && (GermanMode))
      TeXDoSpec();
    else if (!strcmp(Line, "{"))
      SaveFont();
    else if (!strcmp(Line, "}"))
    {
      if (FontNest > 0)
        RestoreFont();
      else if (ErrState >= 0)
        NextErrState();
      else if (FracState >= 0)
        NextFracState();
      else switch (CurrEnv)
      {
        case EnvMarginPar:
          RestoreEnv();
          break;
        case EnvCaption:
          FlushLine();
          RestoreEnv();
          break;
        case EnvHeading:
          EndSectionHeading();
          RestoreEnv();
          break;
        default:
          RestoreFont();
      }
    }
    else
      DoAddNormal(Line, SepString);
  }
  FlushLine();
  DestroyInstTable(TeXTable);

  for (z = 0; z < IncludeNest; fclose(infiles[z++]));
  fclose(outfile);

  unlink(AuxFile);
  PrintLabels(AuxFile);
  PrintCites(AuxFile);
  PrintToc(TocName);

  if (DoRepass)
    fprintf(stderr, "additional pass recommended\n");

  return 0;
}

#ifdef CKMALLOC
#undef malloc
#undef realloc

void *ckmalloc(size_t s)
{
  void *tmp = malloc(s);
  if (!tmp)
  {
    fprintf(stderr, "allocation error(malloc): out of memory");
    exit(255);
  }
  return tmp;
}

void *ckrealloc(void *p, size_t s)
{
  void *tmp = realloc(p, s);
  if (!tmp)
  {
    fprintf(stderr, "allocation error(realloc): out of memory");
    exit(255);
  }
  return tmp;
}
#endif

