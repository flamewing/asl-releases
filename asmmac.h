#ifndef ASMMAC_H
#define ASMMAC_H
/* asmmac.h  */
/*****************************************************************************/
/* SPDX-License-Identifier: GPL-2.0-only OR GPL-3.0-only                     */
/*                                                                           */
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen des Makroprozessors                                         */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*           2001-12-31 added DoIntLabel flag                                */
/*           2002-03-03 added FromFile flag, LineRun pointer to input tag    */
/*                                                                           */
/*****************************************************************************/

#include "errmsg.h"
#include "lstmacroexp.h"
#include "strcomp.h"
#include "stringlists.h"
#include <stddef.h>
#include <stdio.h>

typedef struct tag_MacroRec
{
  char *Name;            /* Name des Makros */
  Byte ParamCount;       /* Anzahl Parameter */
  StringList FirstLine;  /* Zeiger auf erste Zeile */
  StringList ParamNames; /* parameter names, needed for named parameters */
  StringList ParamDefVals; /* default values */
  LongInt UseCounter;    /* to limit recursive calls */
  tLstMacroExpMod LstMacroExpMod; /* modify macro expansion list subset */
  Boolean LocIntLabel;   /* Label used internally instead at beginning */
  Boolean GlobalSymbols; /* labels not local to macro */
  Boolean UsesNumArgs,   /* NUMARGS referenced in macro body */
          UsesAllArgs;   /* ALLARGS referenced in macro body */
} MacroRec, *PMacroRec;

#define BufferArraySize 1024

struct as_dynstr;

typedef struct tag_TInputTag
{
  struct tag_TInputTag *Next;
  Boolean IsMacro;
  Integer IfLevel, IncludeLevel;
  Boolean First;
  Boolean GlobalSymbols;
  tLstMacroExp OrigDoLst;
  LongInt StartLine;
  Boolean (*Processor)(struct tag_TInputTag *P, struct as_dynstr *p_dest);
  LongInt ParCnt,ParZ,ParIter;
  StringList Params;
  LongInt LineCnt, LineZ;
  StringRecPtr Lines, LineRun;
  String SpecNameStr, SaveAttr, SaveLabel, AllArgs;
  tStrComp SpecName;
  ShortString NumArgs;
  Boolean IsEmpty, FromFile, UsesAllArgs, UsesNumArgs;
  FILE *Datei;
  void *Buffer;
  void (*Cleanup)(struct tag_TInputTag *P);
  void (*Restorer)(struct tag_TInputTag *P);
  Boolean (*GetPos)(struct tag_TInputTag *P, char *Dest, size_t DestSize);
  PMacroRec Macro;
} TInputTag, *PInputTag;

typedef struct tag_TOutputTag
{
  struct tag_TOutputTag *Next;
  void (*Processor)(void);
  Integer NestLevel;
  PInputTag Tag;
  PMacroRec Mac;
  StringList ParamNames, ParamDefVals;
  LongInt PubSect,GlobSect;
  Boolean DoExport, DoGlobCopy, UsesNumArgs, UsesAllArgs;
  String GName;
  tErrorNum OpenErrMsg;
} TOutputTag, *POutputTag;


extern PInputTag FirstInputTag;
extern POutputTag FirstOutputTag;


extern void Preprocess(void);


extern void AddMacro(PMacroRec Neu, LongInt DefSect, Boolean Protest);

extern Boolean FoundMacroByName(PMacroRec *Erg, StringPtr Name);

extern Boolean FoundMacro(PMacroRec *Erg);

extern void ClearMacroList(void);

extern void ResetMacroDefines(void);

extern void ClearMacroRec(PMacroRec *Alt, Boolean Complete);

extern void PrintMacroList(void);


extern void PrintDefineList(void);

extern void ClearDefineList(void);

extern void ExpandDefines(char *Line);


extern void asmmac_init(void);
#endif /* ASMMAC_H */
