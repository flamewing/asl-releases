/* asmmac.h  */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterroutinen des Makroprozessors                                         */
/*                                                                           */
/* Historie: 16. 5.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

typedef struct _MacroRec
         {
          char *Name;            /* Name des Makros */
          Byte ParamCount;       /* Anzahl Parameter */
          StringList FirstLine;  /* Zeiger auf erste Zeile */
          LongInt UseCounter;    /* to limit recursive calls */
          Boolean LocMacExp;     /* Makroexpansion wird aufgelistet */
         } MacroRec, *PMacroRec;

#define BufferArraySize 1024

typedef struct _TInputTag
         {
          struct _TInputTag *Next;
          Boolean IsMacro;
          Integer IfLevel;
          Boolean First;
          Boolean OrigDoLst;
          LongInt StartLine;
          Boolean (*Processor)(
#ifdef __PROTOS__
                               struct _TInputTag *P, char *erg
#endif
                                                              );
          LongInt ParCnt,ParZ;
          StringList Params;
          LongInt LineCnt,LineZ;
          StringRecPtr Lines;
          String SpecName,SaveAttr,AllArgs,NumArgs;
          Boolean IsEmpty;
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
          StringList Params;
          LongInt PubSect,GlobSect;
          Boolean DoExport,DoGlobCopy;
          String GName;
         } TOutputTag, *POutputTag;


extern PInputTag FirstInputTag;
extern POutputTag FirstOutputTag;


extern void Preprocess(void);


extern void AddMacro(PMacroRec Neu, LongInt DefSect, Boolean Protest);

extern Boolean FoundMacro(PMacroRec *Erg);

extern void ClearMacroList(void);

extern void ResetMacroDefines(void);

extern void ClearMacroRec(PMacroRec *Alt);

extern void PrintMacroList(void);


extern void PrintDefineList(void);

extern void ClearDefineList(void);

extern void ExpandDefines(char *Line);


extern void asmmac_init(void);
