#ifndef _ASMMAC_H
#define _ASMMAC_H
/* asmmac.h  */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen des Makroprozessors                                         */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*           2001-12-31 added DoIntLabel flag                                */
/*           2002-03-03 added FromFile flag, LineRun pointer to input tag    */
/*                                                                           */
/*****************************************************************************/

typedef struct _MacroRec
{
  char *Name;            /* Name des Makros */
  Byte ParamCount;       /* Anzahl Parameter */
  StringList FirstLine;  /* Zeiger auf erste Zeile */
  StringList ParamNames; /* parameter names, needed for named parameters */
  StringList ParamDefVals; /* default values */
  LongInt UseCounter;    /* to limit recursive calls */
  Boolean LocMacExp;     /* Makroexpansion wird aufgelistet */
  Boolean LocIntLabel;   /* Label used internally instead at beginning */
  Boolean GlobalSymbols; /* labels not local to macro */
} MacroRec, *PMacroRec;

#define BufferArraySize 1024

typedef struct _TInputTag
{
  struct _TInputTag *Next;
  Boolean IsMacro;
  Integer IfLevel;
  Boolean First;
  Boolean GlobalSymbols;
  Boolean OrigDoLst;
  LongInt StartLine;
  Boolean (*Processor)(
#ifdef __PROTOS__
                       struct _TInputTag *P, char *erg
#endif
                                                      );
  LongInt ParCnt,ParZ;
  StringList Params;
  LongInt LineCnt, LineZ;
  StringRecPtr Lines, LineRun;
  String SpecName,SaveAttr,SaveLabel,AllArgs,NumArgs;
  Boolean IsEmpty, FromFile;
  FILE *Datei;
  void *Buffer;
  void (*Cleanup)(
#ifdef __PROTOS__
                  struct _TInputTag *P
#endif
                                      );
  void (*Restorer)(
#ifdef __PROTOS__
                   struct _TInputTag *P
#endif
                                       );
  Boolean (*GetPos)(
#ifdef __PROTOS__
                    struct _TInputTag *P, char *Dest
#endif
                                                    );
  PMacroRec Macro;
} TInputTag, *PInputTag;

typedef struct _TOutputTag
{
  struct _TOutputTag *Next;
  void (*Processor)(
#ifdef __PROTOS__
                    void
#endif
                        );
  Integer NestLevel;
  PInputTag Tag;
  PMacroRec Mac;
  StringList ParamNames, ParamDefVals;
  LongInt PubSect,GlobSect;
  Boolean DoExport, DoGlobCopy;
  String GName;
} TOutputTag, *POutputTag;


extern PInputTag FirstInputTag;
extern POutputTag FirstOutputTag;


extern void Preprocess(void);


extern void AddMacro(PMacroRec Neu, LongInt DefSect, Boolean Protest);

extern Boolean FoundMacro(PMacroRec *Erg);

extern void ClearMacroList(void);

extern void ResetMacroDefines(void);

extern void ClearMacroRec(PMacroRec *Alt, Boolean Complete);

extern void PrintMacroList(void);


extern void PrintDefineList(void);

extern void ClearDefineList(void);

extern void ExpandDefines(char *Line);


extern void asmmac_init(void);
#endif /* _ASMMAC_H */
