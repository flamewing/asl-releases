/* tex2html.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Konverter TeX-->HTML                                                      */
/*                                                                           */
/* Historie: 2.4.1998 Grundsteinlegung (Transfer von tex2doc.c)              */
/*           5.4.1998 Sonderzeichen, Fonts, <>                               */
/*           6.4.1998 geordnete Listen                                       */
/*          20.6.1998 Ausrichtung links/rechts/zentriert                     */
/*                    überlagerte Textattribute                              */
/*                    mehrspaltiger Index                                    */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include "asmitree.h"
#include "chardefs.h"
#include <ctype.h>
#include <sys/stat.h>
#include <time.h>
#include <string.h>
#include "strutil.h"

/*--------------------------------------------------------------------------*/

#define TOKLEN 350

static char *TableName,
            *BiblioName,
            *ContentsName,
            *IndexName,
#define ErrorEntryCnt 3
            *ErrorEntryNames[ErrorEntryCnt];

typedef enum{EnvNone,EnvDocument,EnvItemize,EnvEnumerate,EnvDescription,EnvTable,
             EnvTabular,EnvRaggedLeft,EnvRaggedRight,EnvCenter,EnvVerbatim,
             EnvQuote,EnvTabbing,EnvBiblio,EnvMarginPar,EnvCaption,EnvHeading,EnvCount} EnvType;

typedef enum{FontStandard,FontEmphasized,FontBold,FontTeletype,FontItalic,FontCnt} TFontType;
static char *FontNames[FontCnt]={"","EM","B","TT","IT"};
#define MFontEmphasized (1<<FontEmphasized)
#define MFontBold (1<<FontBold)
#define MFontTeletype (1<<FontTeletype)
#define MFontItalic (1<<FontItalic)
typedef enum{FontTiny,FontSmall,FontNormalSize,FontLarge,FontHuge} TFontSize;

typedef struct _TEnvSave
         {
          struct _TEnvSave *Next;
          EnvType SaveEnv;
          int ListDepth,ActLeftMargin,LeftMargin,RightMargin;
          int EnumCounter,FontNest;
          Boolean InListItem;
         } TEnvSave,*PEnvSave;

typedef struct _TFontSave
         {
          struct _TFontSave *Next;
          int FontFlags;
          TFontSize FontSize;
         } TFontSave,*PFontSave;

typedef enum{ColLeft,ColRight,ColCenter,ColBar} TColumn;
#define MAXCOLS 30
#define MAXROWS 200
typedef char *TableLine[MAXCOLS];
typedef struct
         {
          int ColumnCount,TColumnCount;
          TColumn ColTypes[MAXCOLS];
          int ColLens[MAXCOLS];
          int LineCnt;
          TableLine Lines[MAXROWS];
          Boolean LineFlags[MAXROWS];
          Boolean MultiFlags[MAXROWS];
         } TTable;

typedef struct _TRefSave
         {
          struct _TRefSave *Next;
          char *RefName,*Value;
         } TRefSave,*PRefSave;

typedef struct _TTocSave
         {
          struct _TTocSave *Next;
          char *TocName;
         } TTocSave,*PTocSave;

typedef struct _TIndexSave
         {
          struct _TIndexSave *Next;
          char *Name;
          int RefCnt;
         } TIndexSave,*PIndexSave;

static char *EnvNames[EnvCount]=
            {"___NONE___","document","itemize","enumerate","description","table","tabular",
             "raggedleft","raggedright","center","verbatim","quote","tabbing",
             "thebibliography","___MARGINPAR___","___CAPTION___","___HEADING___"};

static FILE *infile,*outfile;
static char *infilename;
static char TocName[200];
static int CurrLine=0,CurrColumn;

#define CHAPMAX 6
static int Chapters[CHAPMAX];
static int TableNum,FontNest,ErrState,FracState,BibIndent,BibCounter;
#define TABMAX 100
static int TabStops[TABMAX],TabStopCnt,CurrTabStop;
static Boolean InAppendix,InMathMode,DoRepass,InListItem;
static TTable ThisTable;
static int CurrRow,CurrCol;
static Boolean GermanMode;

static EnvType CurrEnv;
static int CurrFontFlags;
static TFontSize CurrFontSize;
static int CurrListDepth;
static int EnumCounter;
static int ActLeftMargin,LeftMargin,RightMargin;
static PEnvSave EnvStack;
static PFontSave FontStack;
static PRefSave FirstRefSave,FirstCiteSave;
static PTocSave FirstTocSave;
static PIndexSave FirstIndex;

/*--------------------------------------------------------------------------*/

	void ChkStack(void)
BEGIN
END

	static void error(char *Msg)
BEGIN
   fprintf(stderr,"%s:%d.%d: %s\n",infilename,CurrLine,CurrColumn,Msg);
   fclose(infile); fclose(outfile);
   exit(2);
END

	static void warning(char *Msg)
BEGIN
   fprintf(stderr,"%s:%d.%d: %s\n",infilename,CurrLine,CurrColumn,Msg);
END

	static void SetLang(Boolean IsGerman)
BEGIN
   if (GermanMode==IsGerman) return;

   if ((GermanMode=IsGerman))
    BEGIN
     TableName="Tabelle";
     BiblioName="Literaturverzeichnis";
     ContentsName="Inhalt";
     IndexName="Index";
     ErrorEntryNames[0]="Typ";
     ErrorEntryNames[1]="Ursache";
     ErrorEntryNames[2]="Argument";
    END
   else
    BEGIN
     TableName="Table";
     BiblioName="Bibliography";
     ContentsName="Contents";
     IndexName="Index";
     ErrorEntryNames[0]="Type";
     ErrorEntryNames[1]="Reason";
     ErrorEntryNames[2]="Argument";
    END
END

/*--------------------------------------------------------------------------*/

	static void AddLabel(char *Name, char *Value)
BEGIN
   PRefSave Run,Prev,Neu;
   int cmp;
   char err[200];

   for (Run=FirstRefSave,Prev=Nil; Run!=Nil; Prev=Run,Run=Run->Next)
    if ((cmp=strcmp(Run->RefName,Name))>=0) break;

   if ((Run!=Nil) AND (cmp==0))
    BEGIN
     if (strcmp(Run->Value,Value)!=0)
      BEGIN
       sprintf(err,"value of label '%s' has changed",Name);
       warning(err); DoRepass=True;
       free(Run->Value); Run->Value=strdup(Value);
      END
    END
   else
    BEGIN
     Neu=(PRefSave) malloc(sizeof(TRefSave));
     Neu->RefName=strdup(Name);
     Neu->Value=strdup(Value);
     Neu->Next=Run;
     if (Prev==Nil) FirstRefSave=Neu; else Prev->Next=Neu;
    END
END

	static void AddCite(char *Name, char *Value)
BEGIN
   PRefSave Run,Prev,Neu;
   int cmp;
   char err[200];

   for (Run=FirstCiteSave,Prev=Nil; Run!=Nil; Prev=Run,Run=Run->Next)
    if ((cmp=strcmp(Run->RefName,Name))>=0) break;

   if ((Run!=Nil) AND (cmp==0))
    BEGIN
     if (strcmp(Run->Value,Value)!=0)
      BEGIN
       sprintf(err,"value of citation '%s' has changed",Name);
       warning(err); DoRepass=True;
       free(Run->Value); Run->Value=strdup(Value);
      END
    END
   else
    BEGIN
     Neu=(PRefSave) malloc(sizeof(TRefSave));
     Neu->RefName=strdup(Name);
     Neu->Value=strdup(Value);
     Neu->Next=Run;
     if (Prev==Nil) FirstCiteSave=Neu; else Prev->Next=Neu;
    END
END

	static void GetLabel(char *Name, char *Dest)
BEGIN
   PRefSave Run;
   char err[200];
 
   for (Run=FirstRefSave; Run!=Nil; Run=Run->Next)
    if (strcmp(Name,Run->RefName)==0) break;

   if (Run==Nil)
    BEGIN
     sprintf(err,"undefined label '%s'",Name);
     warning(err); DoRepass=True;
    END
   strcpy(Dest,(Run==Nil) ? "???" : Run->Value);
END

	static void GetCite(char *Name, char *Dest)
BEGIN
   PRefSave Run;
   char err[200];
 
   for (Run=FirstCiteSave; Run!=Nil; Run=Run->Next)
    if (strcmp(Name,Run->RefName)==0) break;

   if (Run==Nil)
    BEGIN
     sprintf(err,"undefined citation '%s'",Name);
     warning(err); DoRepass=True;
    END
   strcpy(Dest,(Run==Nil) ? "???" : Run->Value);
END

	static void PrintLabels(char *Name)
BEGIN
   PRefSave Run;
   FILE *file=fopen(Name,"a");
   
   if (file==Nil) perror(Name);

   for (Run=FirstRefSave; Run!=Nil; Run=Run->Next)
    fprintf(file,"Label %s %s\n",Run->RefName,Run->Value);
   fclose(file);
END

	static void PrintCites(char *Name)
BEGIN
   PRefSave Run;
   FILE *file=fopen(Name,"a");
   
   if (file==Nil) perror(Name);

   for (Run=FirstCiteSave; Run!=Nil; Run=Run->Next)
    fprintf(file,"Citation %s %s\n",Run->RefName,Run->Value);
   fclose(file);
END

	static void PrintToc(char *Name)
BEGIN
   PTocSave Run;
   FILE *file=fopen(Name,"w"); 

   if (file==Nil) perror(Name);

   for (Run=FirstTocSave; Run!=Nil; Run=Run->Next)
    fprintf(file,"%s\n\n",Run->TocName);
   fclose(file);
END

/*------------------------------------------------------------------------------*/


	static void GetNext(char *Src, char *Dest)
BEGIN
   char *c=strchr(Src,' ');

   if (c==Nil)
    BEGIN
     strcpy(Dest,Src); *Src='\0';
    END
   else
    BEGIN
     *c='\0'; strcpy(Dest,Src);
     for (c++; *c==' '; c++); 
     strcpy(Src,c); 
    END
END

	static void ReadAuxFile(char *Name)
BEGIN
   FILE *file=fopen(Name,"r");
   char Line[300],Cmd[300],Nam[300],Val[300];

   if (file==Nil) return;

   while (NOT feof(file))
    BEGIN
     if (fgets(Line,299,file)==Nil) break;
     if (Line[strlen(Line)-1]=='\n') Line[strlen(Line)-1]='\0';
     GetNext(Line,Cmd);
     if (strcmp(Cmd,"Label")==0)
      BEGIN
       GetNext(Line,Nam); GetNext(Line,Val);
       AddLabel(Nam,Val);
      END
     else if (strcmp(Cmd,"Citation")==0)
      BEGIN
       GetNext(Line,Nam); GetNext(Line,Val);
       AddCite(Nam,Val);
      END
    END

   fclose(file);
END

/*--------------------------------------------------------------------------*/

	static Boolean issep(char inp)
BEGIN
   return ((inp==' ') OR (inp=='\t') OR (inp=='\n'));
END

	static Boolean isalphanum(char inp)
BEGIN
   return ((inp>='A') AND (inp<='Z'))
       OR ((inp>='a') AND (inp<='z'))
       OR ((inp>='0') AND (inp<='9'))
       OR (inp=='.');
END

static char LastChar='\0';
static char SaveSep='\0',SepString[TOKLEN]="";
static Boolean DidEOF=False;
static char BufferLine[TOKLEN]="",*BufferPtr=BufferLine;
typedef struct
         {
          char Token[TOKLEN],Sep[TOKLEN];
         } PushedToken;
static int PushedTokenCnt=0;
static PushedToken PushedTokens[16];

	static int GetChar(void)
BEGIN
   Boolean Comment;
   static Boolean DidPar=False;

   if (*BufferPtr=='\0')
    BEGIN
     do
      BEGIN
       if (feof(infile)) return EOF;
       if (fgets(BufferLine,TOKLEN,infile)==Nil) return EOF;
       CurrLine++;
       BufferPtr=BufferLine;
       Comment=(strlen(BufferLine)>=2) AND (strncmp(BufferLine,"%%",2)==0);
       if ((*BufferLine=='\0') OR (*BufferLine=='\n'))
        BEGIN
         if ((CurrEnv==EnvDocument) AND (NOT DidPar))
          BEGIN
           strcpy(BufferLine,"\\par\n"); DidPar=True; Comment=False;
          END
        END
       else if (Comment)
        BEGIN
         if (BufferLine[strlen(BufferLine)-1]=='\n')
          BufferLine[strlen(BufferLine)-1]='\0';
         if (strncmp(BufferLine+2,"TITLE ",6)==0)
          fprintf(outfile,"<TITLE>%s</TITLE>\n",BufferLine+8);
        END
       else DidPar=False;
      END
     while (Comment);
    END
   return *(BufferPtr++);
END

	static Boolean ReadToken(char *Dest)
BEGIN
   int ch,z;
   Boolean Good;
   char *run;

   if (PushedTokenCnt>0)
    BEGIN
     strcpy(Dest,PushedTokens[0].Token); strcpy(SepString,PushedTokens[0].Sep);
     for (z=0; z<PushedTokenCnt-1; z++) PushedTokens[z]=PushedTokens[z+1];
     PushedTokenCnt--;
     return True;
    END

   if (DidEOF) return FALSE;

   CurrColumn=BufferPtr-BufferLine+1;

   /* falls kein Zeichen gespeichert, fuehrende Blanks ueberspringen */

   *Dest='\0'; *SepString=SaveSep; run=SepString+((SaveSep=='\0')?0:1);
   if (LastChar=='\0')
    BEGIN
     do
      BEGIN
       ch=GetChar(); if (ch=='\r') ch=fgetc(infile);
       if (issep(ch)) *(run++)=' ';
      END
     while ((issep(ch)) AND (ch!=EOF));
     *run='\0';
     if (ch==EOF)
      BEGIN
       DidEOF=TRUE; return FALSE;
      END
    END
   else
    BEGIN
     ch=LastChar; LastChar='\0';
    END

   /* jetzt Zeichen kopieren, bis Leerzeichen */

   run=Dest;
   SaveSep='\0';
   if (isalphanum(*(run++)=ch))
    BEGIN
     do
      BEGIN
       ch=GetChar();
       Good=(NOT issep(ch)) AND (isalphanum(ch)) AND (ch!=EOF);
       if (Good) *(run++)=ch;
      END
     while (Good);

     /* Dateiende ? */

     if (ch==EOF) DidEOF=TRUE;

     /* Zeichen speichern ? */

     else if ((NOT issep(ch)) AND (NOT isalphanum(ch))) LastChar=ch;

     /* Separator speichern ? */

     else if (issep(ch)) SaveSep=' ';
    END

   /* Ende */

   *run='\0'; return True;
END

	static void BackToken(char *Token)
BEGIN
   if (PushedTokenCnt>=16) return;
   strcpy(PushedTokens[PushedTokenCnt].Token,Token);
   strcpy(PushedTokens[PushedTokenCnt].Sep,SepString);
   PushedTokenCnt++;
END

/*--------------------------------------------------------------------------*/

	static void assert_token(char *ref)
BEGIN
   char token[TOKLEN];

   ReadToken(token);
   if (strcmp(ref,token)!=0)
    BEGIN
     sprintf(token,"\"%s\" expected",ref); error(token);
    END
END

	static void collect_token(char *dest, char *term)
BEGIN
   char Comp[TOKLEN];
   Boolean first=TRUE,done;

   *dest='\0';
   do
    BEGIN
     ReadToken(Comp);
     done=(strcmp(Comp,term)==0);
     if (NOT done)
      BEGIN
       if (NOT first) strcat(dest,SepString);
       strcat(dest,Comp);
      END
     first=False;
    END
   while (NOT done);
END

/*--------------------------------------------------------------------------*/

static char OutLineBuffer[TOKLEN]="",SideMargin[TOKLEN];

	static void PutLine(Boolean DoBlock)
BEGIN
   int l,n,ptrcnt,diff,div,mod,divmod;
   char *chz,*ptrs[50];
   Boolean SkipFirst,IsFirst;

   fputs(Blanks(LeftMargin-1),outfile);
   if ((CurrEnv==EnvRaggedRight) OR (NOT DoBlock))
    BEGIN
     fprintf(outfile,"%s",OutLineBuffer);
     l=strlen(OutLineBuffer);
    END
   else
    BEGIN
     SkipFirst=((CurrEnv==EnvItemize) OR (CurrEnv==EnvEnumerate) OR (CurrEnv==EnvDescription) OR (CurrEnv==EnvBiblio));
     if (LeftMargin==ActLeftMargin) SkipFirst=False;
     l=ptrcnt=0; IsFirst=SkipFirst;
     for (chz=OutLineBuffer; *chz!='\0'; chz++)
      BEGIN
       if ((chz>OutLineBuffer) AND (*(chz-1)!=' ') AND (*chz==' '))
        BEGIN
         if (NOT IsFirst) ptrs[ptrcnt++]=chz;
         IsFirst=False;
        END
       l++;
      END
     diff=RightMargin-LeftMargin+1-l;
     div=(ptrcnt>0) ? diff/ptrcnt : 0; mod=diff-(ptrcnt*div);
     divmod=(mod>0) ? ptrcnt/mod : ptrcnt+1;
     IsFirst=SkipFirst;
     ptrcnt=0;
     for (chz=OutLineBuffer; *chz!='\0'; chz++)
      BEGIN
       fputc(*chz,outfile);
       if ((chz>OutLineBuffer) AND (*(chz-1)!=' ') AND (*chz==' '))
        BEGIN
         if (NOT IsFirst)
          BEGIN
           n=div;
           if ((mod>0) AND ((ptrcnt%divmod)==0))
            BEGIN
             mod--; n++;
            END
           if (n>0) fputs(Blanks(n),outfile);
           ptrcnt++;
          END
         IsFirst=False;
        END
      END
     l=RightMargin-LeftMargin+1;
    END
   if (*SideMargin!='\0')
    BEGIN
     fputs(Blanks(RightMargin-LeftMargin+4-l),outfile);
#if 0
     fprintf(outfile,"%s",SideMargin);
#endif
     *SideMargin='\0';
    END
   fputc('\n',outfile);
   LeftMargin=ActLeftMargin;
END

	static void AddLine(char *Part, char *Sep)
BEGIN
   int mlen=RightMargin-LeftMargin+1;
   char *search,save;

   if (strlen(Sep)>1) Sep[1]='\0';
   if (*OutLineBuffer!='\0') strcat(OutLineBuffer,Sep);
   strcat(OutLineBuffer,Part);
   if (strlen(OutLineBuffer)>=mlen)
    BEGIN
     search=OutLineBuffer+mlen;
     while (search>=OutLineBuffer)
      BEGIN
       if (*search==' ') break;
       search--;
      END
     if (search<=OutLineBuffer)
      BEGIN
       PutLine(False); *OutLineBuffer='\0';
      END
     else
      BEGIN
       save=(*search); *search='\0';
       PutLine(False); *search=save;
       for (; *search==' '; search++);
       strcpy(OutLineBuffer,search);
      END
    END
END

	static void AddSideMargin(char *Part, char *Sep)
BEGIN
   if (strlen(Sep)>1) Sep[1]='\0';
   if (*Sep!='\0')
    if ((*SideMargin!='\0') OR (NOT issep(*Sep))) strcat(SideMargin,Sep);
   strcat(SideMargin,Part);
END

	static void FlushLine(void)
BEGIN
   if (*OutLineBuffer!='\0')
    BEGIN
     PutLine(False);
     *OutLineBuffer='\0';
    END
END

	static void ResetLine(void)
BEGIN
   *OutLineBuffer='\0';
END

	static void AddTableEntry(char *Part, char *Sep)
BEGIN
   char *Ptr=ThisTable.Lines[CurrRow][CurrCol];
   int nlen=(Ptr==Nil) ? 0 : strlen(Ptr);
   Boolean UseSep=(nlen>0);

   if (strlen(Sep)>1) Sep[1]='\0';
   if (UseSep) nlen+=strlen(Sep); nlen+=strlen(Part);
   if (Ptr==Nil) 
    BEGIN
     Ptr=(char *) malloc(nlen+1); *Ptr='\0';
    END
   else Ptr=(char *) realloc(Ptr,nlen+1);
   if (UseSep) strcat(Ptr,Sep); strcat(Ptr,Part);
   ThisTable.Lines[CurrRow][CurrCol]=Ptr;
END

	static void DoAddNormal(char *Part, char *Sep)
BEGIN
   if (strcmp(Part,"<")==0) Part="&lt;";
   else if (strcmp(Part,">")==0) Part="&gt;";
   else if (strcmp(Part,"&")==0) Part="&amp;";

   switch (CurrEnv)
    BEGIN
     case EnvMarginPar: AddSideMargin(Part,Sep); break;
     case EnvTabular: AddTableEntry(Part,Sep); break;
     default: AddLine(Part,Sep);
    END
END

/*--------------------------------------------------------------------------*/

	static void SaveFont(void)
BEGIN
   PFontSave NewSave;

   NewSave=(PFontSave) malloc(sizeof(TFontSave));
   NewSave->Next=FontStack;
   NewSave->FontSize=CurrFontSize;
   NewSave->FontFlags=CurrFontFlags;
   FontStack=NewSave; FontNest++;
END

	static void PrFontDiff(int OldFlags, int NewFlags)
BEGIN
   TFontType z;
   int Mask;
   char erg[10];
   
   for (z=FontStandard+1,Mask=2; z<FontCnt; z++,Mask=Mask<<1)
    if ((OldFlags^NewFlags)&Mask)
     BEGIN
      sprintf(erg,"<%s%s>",(NewFlags&Mask)?"":"/",FontNames[z]);
      DoAddNormal(erg,"");
     END
END

	static void PrFontSize(TFontSize Type, Boolean On)
BEGIN
   char erg[10];

   strcpy(erg,"<");
   if (FontNormalSize==Type) return;

   if (NOT On) strcat(erg,"/");
   switch (Type)
    BEGIN
     case FontTiny:
     case FontSmall: strcat(erg,"SMALL"); break;
     case FontLarge:
     case FontHuge:  strcat(erg,"BIG"); break;
     default: break;
    END
   strcat (erg,">");
   DoAddNormal(erg,"");
   if ((FontTiny==Type) OR (FontHuge==Type)) DoAddNormal(erg,"");
END

	static void RestoreFont(void)
BEGIN
   PFontSave OldSave;
  
   if (FontStack==Nil) return;

   PrFontDiff(CurrFontFlags,FontStack->FontFlags);
   PrFontSize(CurrFontSize,False);

   OldSave=FontStack; FontStack=FontStack->Next;
   CurrFontSize=OldSave->FontSize;
   CurrFontFlags=OldSave->FontFlags;
   free(OldSave);
   FontNest--;
END

	static void SaveEnv(EnvType NewEnv)
BEGIN
   PEnvSave NewSave;

   NewSave=(PEnvSave) malloc(sizeof(TEnvSave));
   NewSave->Next=EnvStack;
   NewSave->ListDepth=CurrListDepth;
   NewSave->LeftMargin=LeftMargin;
   NewSave->ActLeftMargin=ActLeftMargin;
   NewSave->RightMargin=RightMargin;
   NewSave->EnumCounter=EnumCounter;
   NewSave->SaveEnv=CurrEnv;
   NewSave->FontNest=FontNest;
   NewSave->InListItem=InListItem;
   EnvStack=NewSave;
   CurrEnv=NewEnv;
   FontNest=0;
END

	static void RestoreEnv(void)
BEGIN
   PEnvSave OldSave;

   OldSave=EnvStack; EnvStack=OldSave->Next;
   CurrListDepth=OldSave->ListDepth;
   LeftMargin=OldSave->LeftMargin;
   ActLeftMargin=OldSave->ActLeftMargin;
   RightMargin=OldSave->RightMargin;
   EnumCounter=OldSave->EnumCounter;
   FontNest=OldSave->FontNest;
   InListItem=OldSave->InListItem;
   CurrEnv=OldSave->SaveEnv;
   free(OldSave);
END

	static void InitTableRow(int Index)
BEGIN
   int z;

   for (z=0; z<ThisTable.TColumnCount; ThisTable.Lines[Index][z++]=Nil);
   ThisTable.MultiFlags[Index]=False;
   ThisTable.LineFlags[Index]=False;
END

	static void NextTableColumn(void)
BEGIN
   if (CurrEnv!=EnvTabular) error("table separation char not within tabular environment");

   if ((ThisTable.MultiFlags[CurrRow])
   OR  (CurrCol>=ThisTable.TColumnCount))
    error("too many columns within row");

   CurrCol++;
END

	static void DumpTable(void)
BEGIN
   int RowCnt,rowz,rowz2,rowz3,colz,colptr,ml,l,diff,sumlen,firsttext,indent;
   char *ColTag;

   /* compute widths of individual rows */
   /* get index of first text column */

   RowCnt=(ThisTable.Lines[CurrRow][0]!=Nil) ? CurrRow+1 : CurrRow;
   firsttext=(-1);
   for (colz=colptr=0; colz<ThisTable.ColumnCount; colz++)
    if (ThisTable.ColTypes[colz]==ColBar) ThisTable.ColLens[colz]=1;
    else
     BEGIN
      ml=0;
      for (rowz=0; rowz<RowCnt; rowz++)
       if ((NOT ThisTable.LineFlags[rowz]) AND (NOT ThisTable.MultiFlags[rowz]))
        BEGIN
         l=(ThisTable.Lines[rowz][colptr]==Nil) ? 0 : strlen(ThisTable.Lines[rowz][colptr]);
         if (ml<l) ml=l;
        END
      ThisTable.ColLens[colz]=ml+2;
      colptr++;
      if (firsttext<0) firsttext=colz;
     END

   /* get total width */

   for (colz=sumlen=0; colz<ThisTable.ColumnCount; sumlen+=ThisTable.ColLens[colz++]);
   indent=(RightMargin-LeftMargin+1-sumlen)/2; if (indent<0) indent=0;

   /* search for multicolumns and extend first field if table is too lean */

   ml=0;
   for (rowz=0; rowz<RowCnt; rowz++)
    if ((NOT ThisTable.LineFlags[rowz]) AND (ThisTable.MultiFlags[rowz]))
     BEGIN 
      l=(ThisTable.Lines[rowz][0]==Nil) ? 0 : strlen(ThisTable.Lines[rowz][0]);
      if (ml<l) ml=l;
     END
   if (ml+4>sumlen)
    BEGIN
     diff=ml+4-sumlen;
     ThisTable.ColLens[firsttext]+=diff;
    END
   
   /* tell browser to switch to table mode */

   fprintf(outfile,"<P><CENTER><TABLE BORDER=1 CELLPADDING=5>\n");

   /* print rows */

   rowz=0;
   while (rowz<RowCnt)
    BEGIN
     /* find first text line */

     for (; rowz<RowCnt; rowz++)
      if (NOT ThisTable.LineFlags[rowz]) break;

     /* find last text line */

     for (rowz2=rowz; rowz2<RowCnt; rowz2++)
      if (ThisTable.LineFlags[rowz2]) break;
     rowz2--;

     if (rowz<RowCnt)
      BEGIN
       /* if more than one line follows, take this as header line(s) */

       if ((rowz2<=RowCnt-3) AND (ThisTable.LineFlags[rowz2+1]) AND (ThisTable.LineFlags[rowz2+2]))
        ColTag="TH";
       else
        ColTag="TD";

       /* start a row */

       fprintf(outfile,"<TR ALIGN=LEFT>\n");

       /* over all columns... */

       colptr=0;
       for (colz=0; colz<((ThisTable.MultiFlags[rowz])?firsttext+1:ThisTable.ColumnCount); colz++)
        if (ThisTable.ColTypes[colz]!=ColBar)
         BEGIN
          /* start a column */

          fprintf(outfile,"<%s VALIGN=TOP NOWRAP",ColTag);
          if (ThisTable.MultiFlags[rowz]) fprintf(outfile," COLSPAN=%d",ThisTable.ColumnCount);
          switch(ThisTable.ColTypes[colz])
           BEGIN
            case ColLeft: fputs(" ALIGN=LEFT>",outfile); break;
            case ColCenter: fputs(" ALIGN=CENTER>",outfile); break;
            case ColRight: fputs(" ALIGN=RIGHT>",outfile); break;
            default: break;
           END

          /* write items */

          for (rowz3=rowz; rowz3<=rowz2; rowz3++)
           BEGIN
            if (ThisTable.Lines[rowz3][colptr]!=Nil)
             BEGIN
              fputs(ThisTable.Lines[rowz3][colptr],outfile);
              free(ThisTable.Lines[rowz3][colptr]);
             END
            if (rowz3!=rowz2) fputs("<BR>\n",outfile);
           END

          /* end column */

          fprintf(outfile,"</%s>\n",ColTag);

          colptr++;
         END

       /* end row */

       fprintf(outfile,"</TR>\n");

       rowz=rowz2+1;
      END
    END

   /* end table mode */

   fprintf(outfile,"</TABLE></CENTER>\n");
END

	static void GetTableName(char *Dest)
BEGIN
   if (InAppendix) sprintf(Dest,"%c.%d",Chapters[0]+'A',TableNum);
   else sprintf(Dest,"%d.%d",Chapters[0],TableNum);
END

	static char *GetSectionName(char *Dest)
BEGIN
   char *run=Dest;
   int z;

   for (z=0; z<=2; z++)
    BEGIN
     if ((z>0) AND (Chapters[z]==0)) break;
     if ((InAppendix) AND (z==0))
      run+=sprintf(run,"%c.",Chapters[z]+'A');
      else run+=sprintf(run,"%d.",Chapters[z]);
    END
   return run;
END

	static void AddToc(char *Line)
BEGIN
   PTocSave NewTocSave,RunToc;

   NewTocSave=(PTocSave) malloc(sizeof(TTocSave));
   NewTocSave->Next=Nil;
   NewTocSave->TocName=strdup(Line);
   if (FirstTocSave==Nil) FirstTocSave=NewTocSave;
   else
    BEGIN
     for (RunToc=FirstTocSave; RunToc->Next!=Nil; RunToc=RunToc->Next);
     RunToc->Next=NewTocSave;
    END
END

/*--------------------------------------------------------------------------*/

static char BackSepString[TOKLEN];

	static void TeXFlushLine(Word Index)
BEGIN
   if (CurrEnv==EnvTabular)
    BEGIN
     for (CurrCol++; CurrCol<ThisTable.TColumnCount; ThisTable.Lines[CurrRow][CurrCol++]=strdup(""));
     CurrRow++;
     if (CurrRow==MAXROWS) error("too many rows in table");
     InitTableRow(CurrRow); CurrCol=0;
    END
   else if (CurrEnv==EnvTabbing)
    BEGIN
     CurrTabStop=0;
     PrFontDiff(CurrFontFlags,0);
     AddLine("</TD></TR>",""); FlushLine();
     AddLine("<TR><TD NOWRAP>","");
     PrFontDiff(0,CurrFontFlags);
    END
   else
    BEGIN
     if (*OutLineBuffer=='\0') strcpy(OutLineBuffer," ");
     AddLine("<BR>",""); FlushLine();
    END
END

	static void TeXKillLine(Word Index)
BEGIN
   ResetLine();
   if (CurrEnv==EnvTabbing)
    BEGIN
     AddLine("<TR><TD NOWRAP>","");
     PrFontDiff(0,CurrFontFlags);
    END
END

	static void TeXDummy(Word Index)
BEGIN
END

	static void TeXDummyNoBrack(Word Index)
BEGIN
   char Token[TOKLEN];

   ReadToken(Token);
END

	static void TeXDummyInCurl(Word Index)
BEGIN
   char Token[TOKLEN];

   assert_token("{");
   ReadToken(Token);
   assert_token("}");
END

	static void TeXNewCommand(Word Index)
BEGIN
   char Token[TOKLEN];
   int level;

   assert_token("{");
   assert_token("\\");
   ReadToken(Token);
   assert_token("}");
   ReadToken(Token);
   if (strcmp(Token,"[")==0)
    BEGIN
     ReadToken(Token);
     assert_token("]");
    END
   assert_token("{"); level=1;
   do
    BEGIN
     ReadToken(Token);
     if (strcmp(Token,"{")==0) level++;
     else if (strcmp(Token,"}")==0) level--;
    END
   while (level!=0);
END

	static void TeXDef(Word Index)
BEGIN
   char Token[TOKLEN];
   int level;

   assert_token("\\");
   ReadToken(Token);
   assert_token("{"); level=1;
   do
    BEGIN
     ReadToken(Token);
     if (strcmp(Token,"{")==0) level++;
     else if (strcmp(Token,"}")==0) level--;
    END
   while (level!=0);
END

	static void TeXFont(Word Index)
BEGIN
   char Token[TOKLEN];

   assert_token("\\"); 
   ReadToken(Token); assert_token("="); ReadToken(Token); ReadToken(Token);
   assert_token("\\"); ReadToken(Token);
END

	static void TeXDocumentStyle(Word Index)
BEGIN
   char Token[TOKLEN];

   ReadToken(Token);
   if (strcmp(Token,"[")==0)
    BEGIN
     do
      BEGIN
       ReadToken(Token);
       if (strcmp(Token,"german")==0) SetLang(True);
      END
     while (strcmp(Token,"]")!=0);
     assert_token("{");
     ReadToken(Token);
     assert_token("}");
    END
END

	static void TeXAppendix(Word Index)
BEGIN
   int z;

   InAppendix=True;
   *Chapters=(-1);
   for (z=1; z<CHAPMAX; Chapters[z++]=0);
END

static int LastLevel;

	static void TeXNewSection(Word Level)
BEGIN
   int z;

   if (Level>=CHAPMAX) return;

   FlushLine(); fputc('\n',outfile);

   assert_token("{"); LastLevel=Level;
   SaveEnv(EnvHeading); RightMargin=200;

   Chapters[Level]++;
   for (z=Level+1; z<CHAPMAX; Chapters[z++]=0);
   if (Level==0) TableNum=0;
END

	static void EndSectionHeading(void)
BEGIN
   int Level=LastLevel;
   char Line[TOKLEN],Title[TOKLEN],Ref[TOKLEN],*run,*rep;

   strcpy(Title,OutLineBuffer); *OutLineBuffer='\0';

   run=Line;
   if (Level<3)
    BEGIN
     GetSectionName(Ref);
     for (rep=Ref; *rep!='\0'; rep++)
      if (*rep=='.') *rep='_';
     fprintf(outfile,"<A NAME=\"sect_%s\">",Ref);
     run=GetSectionName(run);
     run+=sprintf(run," ");
    END
   sprintf(run,"%s",Title);

   fprintf(outfile,"<P><H%d>%s</H%d><P>\n",Level+1,Line,Level+1);

   if (Level<3)
    BEGIN
     fputs("</A>\n",outfile);
     run=Line; run=GetSectionName(run); run+=sprintf(run," "); sprintf(run,"%s",Title);
     AddToc(Line);
    END
END

	static EnvType GetEnvType(char *Name)
BEGIN
   EnvType z;

   for (z=EnvNone+1; z<EnvCount; z++)
    if (strcmp(Name,EnvNames[z])==0) return z;
  
   error("unknown environment");
   return EnvNone;
END

	static void TeXBeginEnv(Word Index)
BEGIN
   char EnvName[TOKLEN],Add[TOKLEN],*p;
   EnvType NEnv;
   Boolean done;
   TColumn NCol;

   assert_token("{");
   ReadToken(EnvName);
   if ((NEnv=GetEnvType(EnvName))==EnvTable)
    BEGIN
     ReadToken(Add);
     if (strcmp(Add,"*")==0) assert_token("}");
     else if (strcmp(Add,"}")!=0) error("unknown table environment");
    END
   else assert_token("}");

   if (NEnv!=EnvVerbatim) SaveEnv(NEnv);

   switch (NEnv)
    BEGIN
     case EnvDocument:
      fputs("</HEAD>\n",outfile);
      fputs("<BODY>\n",outfile);
      break;
     case EnvItemize:
      FlushLine(); fprintf(outfile,"<UL>\n");
      ++CurrListDepth;
      ActLeftMargin=LeftMargin=(CurrListDepth*4)+1;
      RightMargin=70;
      EnumCounter=0;
      InListItem=False;
      break;
     case EnvDescription:
      FlushLine(); fprintf(outfile,"<DL COMPACT>\n");
      ++CurrListDepth;
      ActLeftMargin=LeftMargin=(CurrListDepth*4)+1;
      RightMargin=70;
      EnumCounter=0;
      InListItem=False;
      break;
     case EnvEnumerate:
      FlushLine(); fprintf(outfile,"<OL>\n");
      ++CurrListDepth;
      ActLeftMargin=LeftMargin=(CurrListDepth*4)+1;
      RightMargin=70;
      EnumCounter=0;
      InListItem=False;
      break;
     case EnvBiblio:
      FlushLine(); fprintf(outfile,"<P>\n");
      fprintf(outfile,"<A NAME=\"sect_bib\"><H1>%s<P></H1></A>\n<DL COMPACT>\n",BiblioName);
      assert_token("{"); ReadToken(Add); assert_token("}");
      ActLeftMargin=LeftMargin=4+(BibIndent=strlen(Add));
      AddToc(BiblioName);
      break;
     case EnvVerbatim:
      FlushLine(); fprintf(outfile,"<PRE>\n");
      if ((*BufferLine!='\0') AND (*BufferPtr!='\0'))
       BEGIN
        fprintf(outfile,"%s",BufferPtr);
        *BufferLine='\0'; BufferPtr=BufferLine;
       END
      do
       BEGIN
        fgets(Add,TOKLEN-1,infile); CurrLine++;
        done=strstr(Add,"\\end{verbatim}")!=Nil;
        if (NOT done)
         BEGIN
          for (p=Add; *p!='\0';)
           if (*p=='<')
            BEGIN
             memmove(p+3,p,strlen(p)+1);
             memcpy(p,"&lt;",4);
             p+=4;
            END
           else if (*p=='>')
            BEGIN
             memmove(p+3,p,strlen(p)+1);
             memcpy(p,"&gt;",4);
             p+=4;
            END
           else p++;
          fprintf(outfile,"%s",Add);
         END
       END
      while (NOT done);
      fprintf(outfile,"\n</PRE>\n");
      break;
     case EnvQuote:
      FlushLine(); fprintf(outfile,"<BLOCKQUOTE>\n");
      ActLeftMargin=LeftMargin=5;
      RightMargin=70;
      break;
     case EnvTabbing:
      FlushLine(); fputs("<TABLE CELLPADDING=2>\n",outfile); 
      TabStopCnt=0; CurrTabStop=0; RightMargin=TOKLEN-1;
      AddLine("<TR><TD NOWRAP>","");
      PrFontDiff(0,CurrFontFlags);
      break;
     case EnvTable:
      ReadToken(Add);
      if (strcmp(Add,"[")!=0) BackToken(Add);
      else
       do
        BEGIN
         ReadToken(Add);
        END
       while (strcmp(Add,"]")!=0);
      FlushLine(); fputc('\n',outfile);
      ++TableNum;
      break;
     case EnvCenter:
      FlushLine(); 
      fputs("<CENTER>\n",outfile);
      break;
     case EnvRaggedRight:
      FlushLine(); 
      fputs("<DIV ALIGN=LEFT>\n",outfile);
      break;
     case EnvRaggedLeft:
      FlushLine(); 
      fputs("<DIV ALIGN=RIGHT>\n",outfile);
      break;
     case EnvTabular:
      FlushLine(); assert_token("{");
      ThisTable.ColumnCount=ThisTable.TColumnCount=0;
      do
       BEGIN
        ReadToken(Add);
        done=strcmp(Add,"}")==0;
        if (NOT done)
         BEGIN
          if (ThisTable.ColumnCount>=MAXCOLS) error("too many columns in table");
          NCol=ColLeft;
          if (strcmp(Add,"|")==0) NCol=ColBar;
          else if (strcmp(Add,"l")==0) NCol=ColLeft;
          else if (strcmp(Add,"r")==0) NCol=ColRight;
          else if (strcmp(Add,"c")==0) NCol=ColCenter;
          else error("unknown table column descriptor");
          if ((ThisTable.ColTypes[ThisTable.ColumnCount++]=NCol)!=ColBar)
           ThisTable.TColumnCount++;
         END
       END
      while (NOT done);
      InitTableRow(CurrRow=0); CurrCol=0;
      break;
     default:
      break;
    END
END

	static void TeXEndEnv(Word Index)
BEGIN
   char EnvName[TOKLEN],Add[TOKLEN];
   EnvType NEnv;

   assert_token("{");
   ReadToken(EnvName);
   if ((NEnv=GetEnvType(EnvName))==EnvTable)
    BEGIN
     ReadToken(Add);
     if (strcmp(Add,"*")==0) assert_token("}");
     else if (strcmp(Add,"}")!=0) error("unknown table environment");
    END
   else assert_token("}");

   if (EnvStack==Nil) error("end without begin");
   if (CurrEnv!=NEnv) error("begin and end of environment do not match");

   switch (CurrEnv)
    BEGIN
     case EnvDocument:
      fputs("</BODY>\n",outfile);
      break;
     case EnvItemize:
      if (InListItem) AddLine("</LI>","");
      FlushLine(); fprintf(outfile,"</UL>\n");
      break;
     case EnvDescription:
      if (InListItem) AddLine("</DD>","");
      FlushLine(); fprintf(outfile,"</DL>\n");
      break;
     case EnvEnumerate:
      if (InListItem) AddLine("</LI>","");
      FlushLine(); fprintf(outfile,"</OL>\n");
      break;
     case EnvQuote:
      FlushLine(); fprintf(outfile,"</BLOCKQUOTE>\n");
     case EnvBiblio:
      FlushLine(); fprintf(outfile,"</DL>\n");
      break;
     case EnvTabbing:
      PrFontDiff(CurrFontFlags,0);
      AddLine("</TD></TR>",""); FlushLine();
      fputs("</TABLE>",outfile);
      break;
     case EnvCenter:
      FlushLine(); 
      fputs("</CENTER>\n",outfile);
      break;
     case EnvRaggedRight:
     case EnvRaggedLeft: 
      FlushLine(); 
      fputs("</DIV>\n",outfile);
      break;
     case EnvTabular:
      DumpTable();
      break;
     case EnvTable:
      FlushLine(); fputc('\n',outfile);
      break;
     default:
      break;
    END

   RestoreEnv();
END

	static void TeXItem(Word Index)
BEGIN
   char Token[TOKLEN],Acc[TOKLEN];

   if (InListItem)
    AddLine((CurrEnv==EnvDescription) ? "</DD>" : "</LI>","");
   FlushLine();
   InListItem=True;
   switch(CurrEnv)
    BEGIN
     case EnvItemize:
      fprintf(outfile,"<LI>");
      LeftMargin=ActLeftMargin-3;
      break;
     case EnvEnumerate:
      fprintf(outfile,"<LI>");
      LeftMargin=ActLeftMargin-4;
      break;
     case EnvDescription:
      ReadToken(Token);
      if (strcmp(Token,"[")!=0) BackToken(Token);
      else
       BEGIN
        collect_token(Acc,"]");
        LeftMargin=ActLeftMargin-4;
        fprintf(outfile,"<DT>%s",Acc);
       END
      fprintf(outfile,"<DD>");
      break;
     default:
      error("\\item not in a list environment");
    END
END

	static void TeXBibItem(Word Index)
BEGIN
   char NumString[20],Token[TOKLEN],Name[TOKLEN],Value[TOKLEN];

   if (CurrEnv!=EnvBiblio) error("\\bibitem not in bibliography environment");

   assert_token("{"); collect_token(Name,"}");

   FlushLine(); AddLine("<DT>",""); ++BibCounter;

   LeftMargin=ActLeftMargin-BibIndent-3;
   sprintf(Value,"<A NAME=\"cite_%s\">",Name);
   DoAddNormal(Value,"");
   sprintf(NumString,"[%*d] </A><DD>",BibIndent,BibCounter);
   AddLine(NumString,"");
   sprintf(NumString,"%d",BibCounter);
   AddCite(Name,NumString);
   ReadToken(Token); *SepString='\0'; BackToken(Token);
END

	static void TeXAddDollar(Word Index)
BEGIN
   DoAddNormal("$",BackSepString);
END

	static void TeXAddUnderbar(Word Index)
BEGIN
   DoAddNormal("_",BackSepString);
END

        static void TeXAddPot(Word Index)
BEGIN
   DoAddNormal("^",BackSepString);
END

	static void TeXAddAmpersand(Word Index)
BEGIN
   DoAddNormal("&",BackSepString);
END

        static void TeXAddAt(Word Index)
BEGIN
   DoAddNormal("@",BackSepString);
END

        static void TeXAddImm(Word Index)
BEGIN
   DoAddNormal("#",BackSepString);
END

        static void TeXAddPercent(Word Index)
BEGIN
   DoAddNormal("%",BackSepString);
END

        static void TeXAddSSharp(Word Index)
BEGIN
   DoAddNormal("&szlig;",BackSepString);
END

        static void TeXAddIn(Word Index)  
BEGIN
   DoAddNormal("in",BackSepString);
END

        static void TeXAddReal(Word Index)  
BEGIN
   DoAddNormal("R",BackSepString);
END

	static void TeXAddGreekMu(Word Index)
BEGIN
   DoAddNormal("&micro;",BackSepString);
END

	static void TeXAddGreekPi(Word Index)
BEGIN
   DoAddNormal("Pi",BackSepString);
END

	static void TeXAddLessEq(Word Index)
BEGIN
   DoAddNormal("<=",BackSepString);
END

        static void TeXAddGreaterEq(Word Index)
BEGIN
   DoAddNormal(">=",BackSepString);
END
     
        static void TeXAddNotEq(Word Index)
BEGIN
   DoAddNormal("<>",BackSepString);
END

        static void TeXAddMid(Word Index)
BEGIN
   DoAddNormal("|",BackSepString);
END  

	static void TeXAddRightArrow(Word Index)
BEGIN
   DoAddNormal("->",BackSepString);
END

	static void TeXAddLongRightArrow(Word Index)
BEGIN
   DoAddNormal("-->",BackSepString);
END

	static void TeXAddLeftArrow(Word Index)
BEGIN
   DoAddNormal("<-",BackSepString);
END

	static void TeXAddLeftRightArrow(Word Index)
BEGIN
   DoAddNormal("<->",BackSepString);
END

	static void TeXDoFrac(Word Index)
BEGIN
   assert_token("{"); *SepString='\0'; BackToken("("); FracState=0;
END

	static void NextFracState(void)
BEGIN
   if (FracState==0)
    BEGIN
     assert_token("{");
     *SepString='\0';
     BackToken(")"); BackToken("/"); BackToken("(");
    END
   else if (FracState==1)
    BEGIN
     *SepString='\0'; BackToken(")"); 
    END
   if ((++FracState)==2) FracState=(-1);
END

	static void TeXNewFontType(Word Index)
BEGIN
   int NewFontFlags;
    
   if (Index==FontStandard) NewFontFlags=0; else NewFontFlags=CurrFontFlags|(1<<Index);
   PrFontDiff(CurrFontFlags,NewFontFlags);
   CurrFontFlags=NewFontFlags;
END

	static void TeXEnvNewFontType(Word Index)
BEGIN
   char NToken[TOKLEN];

   SaveFont();
   TeXNewFontType(Index);
   assert_token("{");
   ReadToken(NToken);
   strcpy(SepString,BackSepString);
   BackToken(NToken);
END

	static void TeXNewFontSize(Word Index)
BEGIN
   PrFontSize(CurrFontSize=(TFontSize) Index,True);
END

	static void TeXEnvNewFontSize(Word Index)
BEGIN
   char NToken[TOKLEN];

   SaveFont();
   TeXNewFontSize(Index);
   assert_token("{");
   ReadToken(NToken);
   strcpy(SepString,BackSepString);
   BackToken(NToken);
END

	static void TeXAddMarginPar(Word Index)
BEGIN
   assert_token("{");
   SaveEnv(EnvMarginPar);
END

	static void TeXAddCaption(Word Index)
BEGIN
   char tmp[100];
   int cnt;

   assert_token("{");
   if (CurrEnv!=EnvTable) error("caption outside of a table");
   FlushLine(); fputs("<P><CENTER>",outfile);
   SaveEnv(EnvCaption);
   AddLine(TableName,""); cnt=strlen(TableName);
   GetTableName(tmp); strcat(tmp,": ");
   AddLine(tmp," "); cnt+=1+strlen(tmp);
   LeftMargin=1; ActLeftMargin=cnt+1; RightMargin=70;
END

	static void TeXHorLine(Word Index)
BEGIN
   if (CurrEnv!=EnvTabular) error("\\hline outside of a table");

   if (ThisTable.Lines[CurrRow][0]!=Nil) InitTableRow(++CurrRow);
   ThisTable.LineFlags[CurrRow]=True;
   InitTableRow(++CurrRow);
END

	static void TeXMultiColumn(Word Index)
BEGIN
   char Token[TOKLEN],*endptr;
   int cnt;

   if (CurrEnv!=EnvTabular) error("\\hline outside of a table");
   if (CurrCol!=0) error("\\multicolumn must be in first column");

   assert_token("{"); ReadToken(Token); assert_token("}");
   cnt=strtol(Token,&endptr,10);
   if (*endptr!='\0') error("invalid numeric format to \\multicolumn");
   if (cnt!=ThisTable.TColumnCount) error("\\multicolumn must span entire table");
   assert_token("{"); 
   do
    BEGIN
     ReadToken(Token);
    END
   while (strcmp(Token,"}")!=0);
   ThisTable.MultiFlags[CurrRow]=True;
END

	static void TeXIndex(Word Index)
BEGIN
   char Token[TOKLEN],Erg[TOKLEN];
   PIndexSave run,prev,neu;
   
   assert_token("{"); 
   collect_token(Token,"}");
   run=FirstIndex; prev=Nil;
   while ((run!=Nil) AND (strcmp(Token,run->Name)>0))
    BEGIN
     prev=run; run=run->Next;
    END
   if ((run==Nil) OR (strcmp(Token,run->Name)<0))
    BEGIN
     neu=(PIndexSave) malloc(sizeof(TIndexSave));
     neu->Next=run; 
     neu->RefCnt=1;
     neu->Name=strdup(Token);
     if (prev==Nil) FirstIndex=neu; else prev->Next=neu;
     run=neu;
    END
   else run->RefCnt++;
   sprintf(Erg,"<A NAME=\"index_%s_%d\"></A>",Token,run->RefCnt);
   DoAddNormal(Erg,"");
END

	static int GetDim(Double *Factors)
BEGIN
   char Acc[TOKLEN];
   static char *UnitNames[]={"cm","mm",""},**run,*endptr;
   Double Value;

   assert_token("{"); collect_token(Acc,"}");
   for (run=UnitNames; **run!='\0'; run++)
    if (strcmp(*run,Acc+strlen(Acc)-strlen(*run))==0) break;
   if (**run=='\0') error("unknown unit for dimension");
   Acc[strlen(Acc)-strlen(*run)]='\0';
   Value=strtod(Acc,&endptr);
   if (*endptr!='\0') error("invalid numeric format for dimension");
   return (int)(Value*Factors[run-UnitNames]);
END

static Double HFactors[]={4.666666,0.4666666,0};
static Double VFactors[]={3.111111,0.3111111,0};

	static void TeXHSpace(Word Index)
BEGIN

   DoAddNormal(Blanks(GetDim(HFactors)),"");
END

	static void TeXVSpace(Word Index)
BEGIN
   int z,erg;

   erg=GetDim(VFactors);
   FlushLine();
   for (z=0; z<erg; z++) fputc('\n',outfile);
END

	static void TeXRule(Word Index)
BEGIN
   int h=GetDim(HFactors);
   char Rule[200];
   
   GetDim(VFactors);
   sprintf(Rule,"<HR WIDTH=%d%% ALIGN=LEFT>",(h*100)/70);
   DoAddNormal(Rule,BackSepString);
END

	static void TeXAddTabStop(Word Index)
BEGIN
   int z,n,p;

   if (CurrEnv!=EnvTabbing) error("tab marker outside of tabbing environment");
   if (TabStopCnt>=TABMAX) error("too many tab stops");
   
   n=strlen(OutLineBuffer);
   for (p=0; p<TabStopCnt; p++)
    if (TabStops[p]>n) break;
   for (z=TabStopCnt-1; z>=p; z--) TabStops[z+1]=TabStops[z];
   TabStops[p]=n; TabStopCnt++;

   PrFontDiff(CurrFontFlags,0);
   DoAddNormal("</TD><TD NOWRAP>","");
   PrFontDiff(0,CurrFontFlags);
END

	static void TeXJmpTabStop(Word Index)
BEGIN
   if (CurrEnv!=EnvTabbing) error("tab trigger outside of tabbing environment");
   if (CurrTabStop>=TabStopCnt) error("not enough tab stops");

   PrFontDiff(CurrFontFlags,0);
   DoAddNormal("</TD><TD NOWRAP>","");
   PrFontDiff(0,CurrFontFlags);
   CurrTabStop++;
END

	static void TeXDoVerb(Word Index)
BEGIN
   char Token[TOKLEN],*pos,Marker;

   ReadToken(Token);
   if (*SepString!='\0') error("invalid control character for \\verb");
   Marker=(*Token); strcpy(Token,Token+1); strcpy(SepString,BackSepString);
   do
    BEGIN
     DoAddNormal(SepString,"");
     pos=strchr(Token,Marker);
     if (pos!=Nil)
      BEGIN
       *pos='\0'; DoAddNormal(Token,"");
       *SepString='\0'; BackToken(pos+1);
       break;
      END
     else
      BEGIN
       DoAddNormal(Token,""); ReadToken(Token);
      END
    END
   while (True);
END

	static void TeXErrEntry(Word Index)
BEGIN
   char Token[TOKLEN];

   assert_token("{"); ReadToken(Token); assert_token("}"); assert_token("{");
   *SepString='\0';
   BackToken("\\"); BackToken("item"); BackToken("["); BackToken(Token); BackToken("]");
   ErrState=0;
END

	static void NextErrState(void)
BEGIN
   if (ErrState<3) assert_token("{");
   if (ErrState==0)
    BEGIN
     *SepString='\0';
     BackToken("\\"); BackToken("begin"); BackToken("{"); BackToken("description"); BackToken("}");
    END
   if ((ErrState>=0) AND (ErrState<=2))
    BEGIN
     *SepString='\0';
     BackToken("\\"); BackToken("item"); BackToken("["); BackToken(ErrorEntryNames[ErrState]);
     BackToken(":"); BackToken("]"); BackToken("\\"); BackToken("\\");
    END
   if (ErrState==3)
    BEGIN
     *SepString='\0';
     BackToken("\\"); BackToken("\\"); BackToken(" ");
     BackToken("\\"); BackToken("end"); BackToken("{"); BackToken("description"); BackToken("}");
     ErrState=(-1);
    END
   else ErrState++;
END

	static void TeXWriteLabel(Word Index)
BEGIN
   char Name[TOKLEN],Value[TOKLEN];

   assert_token("{"); collect_token(Name,"}");

   if (CurrEnv==EnvCaption) GetTableName(Value);
   else
    BEGIN
     GetSectionName(Value);
     if (Value[strlen(Value)-1]=='.') Value[strlen(Value)-1]='\0';
    END
   
   AddLabel(Name,Value);
   sprintf(Value,"<A NAME=\"ref_%s\">",Name);
   DoAddNormal(Value,"");
END

	static void TeXWriteRef(Word Index)
BEGIN
   char Name[TOKLEN],Value[TOKLEN],HRef[TOKLEN];

   assert_token("{"); collect_token(Name,"}");
   GetLabel(Name,Value);
   sprintf(HRef,"<A HREF=\"#ref_%s\">",Name);
   DoAddNormal(HRef,BackSepString);
   DoAddNormal(Value,"");
   DoAddNormal("</A>","");
END

	static void TeXWriteCitation(Word Index)
BEGIN
   char Name[TOKLEN],Value[TOKLEN],HRef[TOKLEN];

   assert_token("{"); collect_token(Name,"}");
   GetCite(Name,Value);
   sprintf(HRef,"<A HREF=\"#cite_%s\">",Name);
   DoAddNormal(HRef,BackSepString);
   sprintf(Name,"[%s]",Value);
   DoAddNormal(Name,"");
   DoAddNormal("</A>","");
END

	static void TeXNewParagraph(Word Index)
BEGIN
   FlushLine();
   fprintf(outfile,"<P>\n");
END

	static void TeXContents(Word Index)
BEGIN
   FILE *file=fopen(TocName,"r");
   char Line[200],Ref[50],*ptr,*run;
   int Level;

   if (file==Nil)
    BEGIN
     warning("contents file not found.");
     DoRepass=True; return;
    END

   FlushLine();
   fprintf(outfile,"<P>\n<H1>%s</H1><P>\n",ContentsName);
   while (NOT feof(file))
    BEGIN
     fgets(Line,199,file);
     if ((*Line!='\0') AND (*Line!='\n'))
      BEGIN
       if (strncmp(Line,BiblioName,strlen(BiblioName))==0)
        BEGIN
         strcpy(Ref,"bib"); Level=1;
        END
       else if (strncmp(Line,IndexName,strlen(IndexName))==0)
        BEGIN
         strcpy(Ref,"index"); Level=1;
        END
       else
        BEGIN
         ptr=Ref; Level=1;
         if (Line[strlen(Line)-1]=='\n') Line[strlen(Line)-1]='\0';
         for (run=Line; *run!='\0'; run++) if (*run!=' ') break;
         for (; *run!='\0'; run++)
          if (*run==' ') break;
          else if (*run=='.')
           BEGIN
            *(ptr++)='_';
            Level++;
           END
          else if ((*run>='0') AND (*run<='9')) *(ptr++)=(*run);
          else if ((*run>='A') AND (*run<='Z')) *(ptr++)=(*run);
         *ptr='\0';
        END
       fprintf(outfile,"<P><H%d>",Level);
       if (*Ref!='\0') fprintf(outfile,"<A HREF=\"#sect_%s\">",Ref);
       fputs(Line,outfile);
       if (*Ref!='\0') fprintf(outfile,"</A></H%d>",Level);
       fputc('\n',outfile);
      END
    END

   fclose(file);
END

	static void TeXPrintIndex(Word Index)
BEGIN
   PIndexSave run;
   int i,rz;

   FlushLine();
   fprintf(outfile,"<A NAME=\"sect_index\"><H1>%s<P></H1></A>\n<DL COMPACT>\n",IndexName);
   AddToc(IndexName);

   fputs("<TABLE BORDER=0 CELLPADDING=5>\n",outfile); rz=0;
   for (run=FirstIndex; run!=Nil; run=run->Next)
    BEGIN
     if (!(rz%5)) fputs("<TR ALIGN=LEFT>\n",outfile);
     fputs("<TD VALIGN=TOP NOWRAP>",outfile);
     fputs(run->Name,outfile);
     for (i=0; i<run->RefCnt; i++)
      fprintf(outfile," <A HREF=\"#index_%s_%d\">%d</A>",run->Name,i+1,i+1);
     fputs("</TD>\n",outfile);
     if ((rz%5)==4) fputs("</TR>\n",outfile);
     rz++;
    END
   if (!(rz%5)) fputs("</TR>\n",outfile);
   fputs("</TABLE>\n",outfile);
END

	static void TeXParSkip(Word Index)
BEGIN
   char Token[TOKLEN];

   ReadToken(Token);
   do
    BEGIN
     ReadToken(Token);
     if ((strncmp(Token,"plus",4)==0) OR (strncmp(Token,"minus",5)==0))
      BEGIN
      END
     else
      BEGIN
       BackToken(Token); return;
      END
    END
   while (1);
END

	static void TeXNLS(Word Index)
BEGIN
   char Token[TOKLEN],*Repl="";
   Boolean Found=True;

   *Token='\0';
   ReadToken(Token);
   if (*SepString=='\0')
    switch (*Token)
     BEGIN
      case 'a': Repl="&auml;"; break;
      case 'o': Repl="&ouml;"; break;
      case 'u': Repl="&uuml;"; break;
      case 'A': Repl="&Auml;"; break;
      case 'O': Repl="&Ouml;"; break;
      case 'U': Repl="&Uuml;"; break;
      case 's': Repl="&szlig;"; break;
      default : Found=False;
     END
   else Found=False;

   if (Found)
    BEGIN
     if (strlen(Repl)>1) memmove(Token+strlen(Repl),Token+1,strlen(Token));
     memcpy(Token,Repl,strlen(Repl)); strcpy(SepString,BackSepString);
    END
   else DoAddNormal("\"",BackSepString);

   BackToken(Token);
END

	static void TeXHyphenation(Word Index)
BEGIN
   char Token[TOKLEN];

   assert_token("{"); collect_token(Token,"}");
END

	static void TeXDoPot(void)
BEGIN
   char Token[TOKLEN];

   ReadToken(Token);
   if (strcmp(Token,"2")==0)
    DoAddNormal("&sup2;",BackSepString);
   else
    BEGIN
     DoAddNormal("^",BackSepString);
     AddLine(Token,"");
    END
END

	static void TeXDoSpec(void)
BEGIN
   strcpy(BackSepString,SepString);
   TeXNLS(0);
END

/*--------------------------------------------------------------------------*/

	int main(int argc, char **argv)
BEGIN
   char Line[TOKLEN],Comp[TOKLEN],*p,AuxFile[200];
   PInstTable TeXTable=CreateInstTable(301);
   struct stat st;
   int z;

   if (argc<3)
    BEGIN
     fprintf(stderr,"calling convention: %s <input file> <output file>\n",*argv);
     exit(1);
    END

   if ((infile=fopen(argv[1],"r"))==Nil)
    BEGIN
     perror(argv[1]); exit(3);
    END
   infilename=argv[1];
   if (strcmp(argv[2],"-")==0) outfile=stdout;
   else if ((outfile=fopen(argv[2],"w"))==Nil)
    BEGIN
     perror(argv[2]); exit(3);
    END

   AddInstTable(TeXTable,"\\",0,TeXFlushLine);
   AddInstTable(TeXTable,"par",0,TeXNewParagraph);
   AddInstTable(TeXTable,"-",0,TeXDummy);
   AddInstTable(TeXTable,"hyphenation",0,TeXHyphenation);
   AddInstTable(TeXTable,"kill",0,TeXKillLine);
   AddInstTable(TeXTable,"/",0,TeXDummy);
   AddInstTable(TeXTable,"pagestyle",0,TeXDummyInCurl);
   AddInstTable(TeXTable,"thispagestyle",0,TeXDummyInCurl);
   AddInstTable(TeXTable,"sloppy",0,TeXDummy);
   AddInstTable(TeXTable,"clearpage",0,TeXDummy);
   AddInstTable(TeXTable,"cleardoublepage",0,TeXDummy);
   AddInstTable(TeXTable,"topsep",0,TeXDummyNoBrack);
   AddInstTable(TeXTable,"parskip",0,TeXParSkip);
   AddInstTable(TeXTable,"parindent",0,TeXDummyNoBrack);
   AddInstTable(TeXTable,"textwidth",0,TeXDummyNoBrack);   
   AddInstTable(TeXTable,"evensidemargin",0,TeXDummyNoBrack);   
   AddInstTable(TeXTable,"oddsidemargin",0,TeXDummyNoBrack);
   AddInstTable(TeXTable,"newcommand",0,TeXNewCommand);
   AddInstTable(TeXTable,"def",0,TeXDef);
   AddInstTable(TeXTable,"font",0,TeXFont);
   AddInstTable(TeXTable,"documentstyle",0,TeXDocumentStyle);
   AddInstTable(TeXTable,"chapter",0,TeXNewSection);
   AddInstTable(TeXTable,"section",1,TeXNewSection);
   AddInstTable(TeXTable,"subsection",2,TeXNewSection);
   AddInstTable(TeXTable,"subsubsection",3,TeXNewSection);
   AddInstTable(TeXTable,"appendix",0,TeXAppendix);
   AddInstTable(TeXTable,"makeindex",0,TeXDummy);
   AddInstTable(TeXTable,"begin",0,TeXBeginEnv);
   AddInstTable(TeXTable,"end",0,TeXEndEnv);
   AddInstTable(TeXTable,"item",0,TeXItem);
   AddInstTable(TeXTable,"bibitem",0,TeXBibItem);
   AddInstTable(TeXTable,"errentry",0,TeXErrEntry);
   AddInstTable(TeXTable,"$",0,TeXAddDollar);
   AddInstTable(TeXTable,"_",0,TeXAddUnderbar);
   AddInstTable(TeXTable,"^",0,TeXAddPot);
   AddInstTable(TeXTable,"&",0,TeXAddAmpersand);
   AddInstTable(TeXTable,"@",0,TeXAddAt);
   AddInstTable(TeXTable,"#",0,TeXAddImm);
   AddInstTable(TeXTable,"%",0,TeXAddPercent);
   AddInstTable(TeXTable,"ss",0,TeXAddSSharp);
   AddInstTable(TeXTable,"in",0,TeXAddIn);
   AddInstTable(TeXTable,"rz",0,TeXAddReal);
   AddInstTable(TeXTable,"mu",0,TeXAddGreekMu);
   AddInstTable(TeXTable,"pi",0,TeXAddGreekPi);
   AddInstTable(TeXTable,"leq",0,TeXAddLessEq);
   AddInstTable(TeXTable,"geq",0,TeXAddGreaterEq);
   AddInstTable(TeXTable,"neq",0,TeXAddNotEq);
   AddInstTable(TeXTable,"mid",0,TeXAddMid);
   AddInstTable(TeXTable,"frac",0,TeXDoFrac);
   AddInstTable(TeXTable,"rm",FontStandard,TeXNewFontType);
   AddInstTable(TeXTable,"em",FontEmphasized,TeXNewFontType);
   AddInstTable(TeXTable,"bf",FontBold,TeXNewFontType);
   AddInstTable(TeXTable,"tt",FontTeletype,TeXNewFontType);
   AddInstTable(TeXTable,"it",FontItalic,TeXNewFontType);
   AddInstTable(TeXTable,"bb",FontBold,TeXEnvNewFontType);
   AddInstTable(TeXTable,"tty",FontTeletype,TeXEnvNewFontType);
   AddInstTable(TeXTable,"ii",FontItalic,TeXEnvNewFontType);
   AddInstTable(TeXTable,"tiny",FontTiny,TeXNewFontSize);
   AddInstTable(TeXTable,"small",FontSmall,TeXNewFontSize);
   AddInstTable(TeXTable,"normalsize",FontNormalSize,TeXNewFontSize);
   AddInstTable(TeXTable,"large",FontLarge,TeXNewFontSize);
   AddInstTable(TeXTable,"huge",FontHuge,TeXNewFontSize);
   AddInstTable(TeXTable,"tin",FontTiny,TeXEnvNewFontSize);
   AddInstTable(TeXTable,"rightarrow",0,TeXAddRightArrow);
   AddInstTable(TeXTable,"longrightarrow",0,TeXAddLongRightArrow);
   AddInstTable(TeXTable,"leftarrow",0,TeXAddLeftArrow);
   AddInstTable(TeXTable,"leftrightarrow",0,TeXAddLeftRightArrow);
   AddInstTable(TeXTable,"marginpar",0,TeXAddMarginPar);
   AddInstTable(TeXTable,"caption",0,TeXAddCaption);
   AddInstTable(TeXTable,"label",0,TeXWriteLabel);
   AddInstTable(TeXTable,"ref",0,TeXWriteRef);
   AddInstTable(TeXTable,"cite",0,TeXWriteCitation);
   AddInstTable(TeXTable,"hline",0,TeXHorLine);
   AddInstTable(TeXTable,"multicolumn",0,TeXMultiColumn);
   AddInstTable(TeXTable,"ttindex",0,TeXIndex);
   AddInstTable(TeXTable,"hspace",0,TeXHSpace);
   AddInstTable(TeXTable,"vspace",0,TeXVSpace);
   AddInstTable(TeXTable,"=",0,TeXAddTabStop);
   AddInstTable(TeXTable,">",0,TeXJmpTabStop);
   AddInstTable(TeXTable,"verb",0,TeXDoVerb);
   AddInstTable(TeXTable,"printindex",0,TeXPrintIndex);
   AddInstTable(TeXTable,"tableofcontents",0,TeXContents);
   AddInstTable(TeXTable,"rule",0,TeXRule);
   AddInstTable(TeXTable,"\"",0,TeXNLS);
   AddInstTable(TeXTable,"newif",0,TeXDummy);
   AddInstTable(TeXTable,"fi",0,TeXDummy);
   AddInstTable(TeXTable,"ifelektor",0,TeXDummy);
   AddInstTable(TeXTable,"elektortrue",0,TeXDummy);
   AddInstTable(TeXTable,"elektorfalse",0,TeXDummy);

   for (z=0; z<CHAPMAX; Chapters[z++]=0);
   TableNum=0; FontNest=0;
   TabStopCnt=0; CurrTabStop=0;
   ErrState=FracState=(-1);
   InAppendix=False;
   EnvStack=Nil;
   CurrEnv=EnvNone; CurrListDepth=0;
   ActLeftMargin=LeftMargin=1; RightMargin=70;
   EnumCounter=0; InListItem=False;
   CurrFontFlags=0; CurrFontSize=FontNormalSize;
   FontStack=Nil;
   FirstRefSave=Nil; FirstCiteSave=Nil; FirstTocSave=Nil; FirstIndex=Nil;
   *SideMargin='\0';
   DoRepass=False;
   BibIndent=BibCounter=0;
   GermanMode=True; SetLang(False);

   strcpy(TocName,argv[1]); 
   if ((p=strrchr(TocName,'.'))!=Nil) *p='\0';
   strcat(TocName,".htoc");

   strcpy(AuxFile,argv[1]); 
   if ((p=strrchr(AuxFile,'.'))!=Nil) *p='\0';
   strcat(AuxFile,".haux");
   ReadAuxFile(AuxFile);

   fputs("<!DOCTYPE HTML PUBLIC \"-//W3C//DTD HTML 3.2 Final//EN\">\n",outfile);
   fputs("<HTML>\n",outfile);
   fputs("<HEAD>\n",outfile);
   fprintf(outfile,"<META NAME=\"Author\" CONTENT=\"automatically generated by tex2html from %s\">\n",argv[1]);
   stat(argv[1],&st);
   strncpy(Line,ctime(&st.st_mtime),TOKLEN-1);
   if (Line[strlen(Line)-1]=='\n') Line[strlen(Line)-1]='\0';
   fprintf(outfile,"<META NAME=\"Last-modified\" CONTENT=\"%s\">\n",Line);

   while (1)
    BEGIN
     if (NOT ReadToken(Line)) break;
     if (strcmp(Line,"\\")==0)
      BEGIN
       strcpy(BackSepString,SepString);
       if (NOT ReadToken(Line)) error("unexpected end of file");
       if (*SepString!='\0') BackToken(Line);
       else if (NOT LookupInstTable(TeXTable,Line))
        BEGIN
         sprintf(Comp,"unknown TeX command %s",Line);
         warning(Comp);
        END
      END
     else if (strcmp(Line,"$")==0)
      BEGIN
       if ((InMathMode=(NOT InMathMode)))
        BEGIN
         strcpy(BackSepString,SepString);
         ReadToken(Line); strcpy(SepString,BackSepString);
         BackToken(Line);
        END
      END
     else if (strcmp(Line,"&")==0) NextTableColumn();
     else if ((strcmp(Line,"^")==0) AND (InMathMode)) TeXDoPot();
     else if ((strcmp(Line,"\"")==0) AND (GermanMode)) TeXDoSpec();
     else if (strcmp(Line,"{")==0)
      SaveFont();
     else if (strcmp(Line,"}")==0)
      if (FontNest>0) RestoreFont();
      else if (ErrState>=0) NextErrState();
      else if (FracState>=0) NextFracState();
      else switch (CurrEnv)
       BEGIN
        case EnvMarginPar:
         RestoreEnv(); break;
        case EnvCaption:
         FlushLine(); fputs("</CENTER><P>\n",outfile); RestoreEnv(); break;
        case EnvHeading:
         EndSectionHeading(); RestoreEnv(); break;
        default: RestoreFont();
       END
     else DoAddNormal(Line,SepString);
    END
   FlushLine();
   DestroyInstTable(TeXTable);

   fputs("</HTML>\n",outfile);

   fclose(infile); fclose(outfile);

   unlink(AuxFile);
   PrintLabels(AuxFile); PrintCites(AuxFile);
   PrintToc(TocName);

   if (DoRepass) fprintf(stderr,"additional pass recommended\n");

   return 0;
END

#ifdef CKMALLOC
#undef malloc
#undef realloc

        void *ckmalloc(size_t s)
BEGIN
   void *tmp=malloc(s);
   if (tmp==NULL) 
    BEGIN
     fprintf(stderr,"allocation error(malloc): out of memory");
     exit(255);
    END
   return tmp;
END

        void *ckrealloc(void *p, size_t s)
BEGIN
   void *tmp=realloc(p,s);
   if (tmp==NULL)
    BEGIN
     fprintf(stderr,"allocation error(realloc): out of memory");
     exit(255);
    END
   return tmp;
END
#endif

