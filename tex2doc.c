/* tex2doc.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Konverter TeX-->ASCII-DOC                                                 */
/*                                                                           */
/* Historie: 9. 2.1998 Grundsteinlegung                                      */
/*          20. 6.1998 Zentrierung                                           */
/*          11. 7.1998 weitere Landessonderzeichen                           */
/*          13. 7.1998 Cedilla                                               */
/*          12. 9.1998 input-Statement                                       */
/*          12. 1.1999 andere Kapitelherarchie fuer article                  */
/*           14. 1.2001 silenced warnings about unused parameters            */
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

typedef enum{EnvNone,EnvDocument,EnvItemize,EnvEnumerate,EnvDescription,EnvTable,
             EnvTabular,EnvRaggedLeft,EnvRaggedRight,EnvCenter,EnvVerbatim,
             EnvQuote,EnvTabbing,EnvBiblio,EnvMarginPar,EnvCaption,EnvHeading,EnvCount} EnvType;

typedef enum{FontStandard,FontEmphasized,FontBold,FontTeletype,FontItalic} TFontType;
typedef enum{FontTiny,FontSmall,FontNormalSize,FontLarge,FontHuge} TFontSize;

typedef enum{AlignNone,AlignCenter,AlignLeft,AlignRight} TAlignment;

typedef struct _TEnvSave
         {
          struct _TEnvSave *Next;
          EnvType SaveEnv;
          int ListDepth,ActLeftMargin,LeftMargin,RightMargin;
          int EnumCounter,FontNest;
          TAlignment Alignment;
         } TEnvSave,*PEnvSave;

typedef struct _TFontSave
         {
          struct _TFontSave *Next;
          TFontType FontType;
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

static char *EnvNames[EnvCount]=
            {"___NONE___","document","itemize","enumerate","description","table","tabular",
             "raggedleft","raggedright","center","verbatim","quote","tabbing",
             "thebibliography","___MARGINPAR___","___CAPTION___","___HEADING___"};

static int IncludeNest;
static FILE *infiles[50],*outfile;
static char *infilename;
static char TocName[200];
static int CurrLine=0,CurrColumn;

#define CHAPMAX 6
static int Chapters[CHAPMAX];
static int TableNum,FontNest,ErrState,FracState,BibIndent,BibCounter;
#define TABMAX 100
static int TabStops[TABMAX],TabStopCnt,CurrTabStop;
static Boolean InAppendix,InMathMode,DoRepass;
static TTable ThisTable;
static int CurrRow,CurrCol;
static Boolean GermanMode;

static EnvType CurrEnv;
static TFontType CurrFontType;
static TFontSize CurrFontSize;
static int CurrListDepth;
static int EnumCounter;
static int ActLeftMargin,LeftMargin,RightMargin;
static TAlignment Alignment;
static PEnvSave EnvStack;
static PFontSave FontStack;
static PRefSave FirstRefSave,FirstCiteSave;
static PTocSave FirstTocSave;

static PInstTable TeXTable;

/*--------------------------------------------------------------------------*/

	void ChkStack(void)
BEGIN
END

	static void error(char *Msg)
BEGIN
   int z;

   fprintf(stderr,"%s:%d.%d: %s\n",infilename,CurrLine,CurrColumn,Msg);
   for (z=0; z<IncludeNest; fclose(infiles[z++]));
   fclose(outfile);
   exit(2);
END

	static void warning(char *Msg)
BEGIN
   fprintf(stderr,"%s:%d.%d: %s\n",infilename,CurrLine,CurrColumn,Msg);
END

	static void SetLang(Boolean IsGerman)
BEGIN
   char **pp;

   if (GermanMode==IsGerman) return;

   DestroyTree();
   if ((GermanMode=IsGerman))
    BEGIN
     TableName="Tabelle";
     BiblioName="Literaturverzeichnis";
     ContentsName="Inhalt";
     ErrorEntryNames[0]="Typ";
     ErrorEntryNames[1]="Ursache";
     ErrorEntryNames[2]="Argument";
#ifndef __MSDOS__
     BuildTree(GRHyphens);
#endif
    END
   else
    BEGIN
     TableName="Table";
     BiblioName="Bibliography";
     ContentsName="Contents";
     ErrorEntryNames[0]="Type";
     ErrorEntryNames[1]="Reason";
     ErrorEntryNames[2]="Argument";
#ifndef __MSDOS__
     BuildTree(USHyphens);
     for (pp=USExceptions; *pp!=NULL; pp++) AddException(*pp);
#endif
    END
END

/*--------------------------------------------------------------------------*/

	static void AddLabel(char *Name, char *Value)
BEGIN
   PRefSave Run,Prev,Neu;
   int cmp=(-1);
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
   int cmp=(-1);
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
     if ((*Line) && (Line[strlen(Line)-1]=='\n'))
       Line[strlen(Line)-1]='\0';
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
   char *Result;

   if (*BufferPtr=='\0')
    BEGIN
     do
      BEGIN
       if (IncludeNest<=0) return EOF;
       do
        BEGIN
         Result=fgets(BufferLine,TOKLEN,infiles[IncludeNest-1]);
         if (Result!=Nil) break;
         fclose(infiles[--IncludeNest]);
         if (IncludeNest<=0) return EOF;
        END
       while (True);
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
       else if (NOT Comment) DidPar=False;
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
       ch=GetChar(); if (ch=='\r') ch=GetChar();
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
   int ll=RightMargin-LeftMargin+1;
   int l,n,ptrcnt,diff,div,mod,divmod;
   char *chz,*ptrs[50];
   Boolean SkipFirst,IsFirst;

   fputs(Blanks(LeftMargin-1),outfile);
   if ((Alignment!=AlignNone) OR (NOT DoBlock))
    BEGIN
     l=strlen(OutLineBuffer); 
     diff=ll-l;
     switch (Alignment)
      BEGIN
       case AlignRight: fputs(Blanks(diff),outfile); l=ll; break;
       case AlignCenter: fputs(Blanks(diff>>1),outfile); l+=diff>>1; break;
       default: break;
      END
     fputs(OutLineBuffer,outfile);
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
     diff=ll+1-l;
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
     fputs(Blanks(ll+3-l),outfile);
     fprintf(outfile,"%s",SideMargin);
     *SideMargin='\0';
    END
   fputc('\n',outfile);
   LeftMargin=ActLeftMargin;
END

	static void AddLine(char *Part, char *Sep)
BEGIN
   int mlen=RightMargin-LeftMargin+1,*hyppos,hypcnt,z,hlen;
   char *search,save,*lastalpha;

   if (strlen(Sep)>1) Sep[1]='\0';
   if (*OutLineBuffer!='\0') strcat(OutLineBuffer,Sep);
   strcat(OutLineBuffer,Part);
   if (strlen(OutLineBuffer)>=mlen)
    BEGIN
     search=OutLineBuffer+mlen;
     while (search>=OutLineBuffer)
      BEGIN
       if (*search==' ') break;
       if (search>OutLineBuffer)
        BEGIN 
         if (*(search-1)=='-') break;
         else if (*(search-1)=='/') break;
         else if (*(search-1)==';') break;
         else if (*(search-1)==';') break;
        END
       search--;
      END
     if (search<=OutLineBuffer)
      BEGIN
       PutLine(True); *OutLineBuffer='\0';
      END
     else
      BEGIN
       if (*search==' ')
        BEGIN
         for (lastalpha=search+1; *lastalpha!='\0'; lastalpha++)
          if ((mytolower(*lastalpha)<'a') OR (mytolower(*lastalpha)>'z')) break;
         if (lastalpha-search>3)
          BEGIN
           save=(*lastalpha); *lastalpha='\0';
           DoHyphens(search+1,&hyppos,&hypcnt);
           *lastalpha=save;
           hlen=(-1);
           for (z=0; z<hypcnt; z++)
            if ((search-OutLineBuffer)+hyppos[z]+1<mlen) hlen=hyppos[z];
           if (hlen>0)
            BEGIN
             memmove(search+hlen+2,search+hlen+1,strlen(search+hlen+1)+1);
             search[hlen+1]='-';
             search+=hlen+2;
            END
           if (hypcnt>0) free(hyppos);
          END
        END
       save=(*search); *search='\0';
       PutLine(True); *search=save;
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

/*--------------------------------------------------------------------------*/

	static void SaveFont(void)
BEGIN
   PFontSave NewSave;

   NewSave=(PFontSave) malloc(sizeof(TFontSave));
   NewSave->Next=FontStack;
   NewSave->FontSize=CurrFontSize;
   NewSave->FontType=CurrFontType;
   FontStack=NewSave; FontNest++;
END

	static void RestoreFont(void)
BEGIN
   PFontSave OldSave;
  
   if (FontStack==Nil) return;

   OldSave=FontStack; FontStack=FontStack->Next;
   CurrFontSize=OldSave->FontSize;
   CurrFontType=OldSave->FontType;
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
   NewSave->Alignment=Alignment;
   NewSave->ActLeftMargin=ActLeftMargin;
   NewSave->RightMargin=RightMargin;
   NewSave->EnumCounter=EnumCounter;
   NewSave->SaveEnv=CurrEnv;
   NewSave->FontNest=FontNest;
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
   Alignment=OldSave->Alignment;
   EnumCounter=OldSave->EnumCounter;
   FontNest=OldSave->FontNest;
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

	static void DoPrnt(char *Ptr, TColumn Align, int len)
BEGIN
   int l=(Ptr==Nil) ? 0 : strlen(Ptr),diff;

   len-=2;
   diff=len-l;
   fputc(' ',outfile);
   switch (Align)
    BEGIN
     case ColRight: fputs(Blanks(diff),outfile); break;
     case ColCenter: fputs(Blanks((diff+1)/2),outfile); break;
     default: break;
    END
   if (Ptr!=Nil)
    BEGIN
     fputs(Ptr,outfile); free(Ptr);
    END
   switch (Align)
    BEGIN
     case ColLeft: fputs(Blanks(diff),outfile); break;
     case ColCenter: fputs(Blanks(diff/2),outfile); break;
     default: break;
    END
   fputc(' ',outfile);
END

	static void DumpTable(void)
BEGIN
   int RowCnt,rowz,colz,colptr,ml,l,diff,sumlen,firsttext,indent;

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
   
   /* print rows */

   for (rowz=0; rowz<RowCnt; rowz++)
    BEGIN
     fputs(Blanks(LeftMargin-1+indent),outfile);
     if (ThisTable.MultiFlags[rowz])
      BEGIN
       l=sumlen;
       if (ThisTable.ColTypes[0]==ColBar)
        BEGIN
         l--; fputc('|',outfile);
        END
       if (ThisTable.ColTypes[ThisTable.ColumnCount-1]==ColBar) l--;
       DoPrnt(ThisTable.Lines[rowz][0],ThisTable.ColTypes[firsttext],l);
       if (ThisTable.ColTypes[ThisTable.ColumnCount-1]==ColBar) fputc('|',outfile);
      END
     else
      BEGIN
       for (colz=colptr=0; colz<ThisTable.ColumnCount; colz++)
        if (ThisTable.LineFlags[rowz])
         if (ThisTable.ColTypes[colz]==ColBar) fputc('+',outfile);
         else for (l=0; l<ThisTable.ColLens[colz]; l++) fputc('-',outfile);
        else
         if (ThisTable.ColTypes[colz]==ColBar) fputc('|',outfile);
         else
          BEGIN
           DoPrnt(ThisTable.Lines[rowz][colptr],ThisTable.ColTypes[colz],ThisTable.ColLens[colz]);
           colptr++;
          END
      END
     fputc('\n',outfile);
    END
END

	static void DoAddNormal(char *Part, char *Sep)
BEGIN
   switch (CurrEnv)
    BEGIN
     case EnvMarginPar: AddSideMargin(Part,Sep); break;
     case EnvTabular: AddTableEntry(Part,Sep); break;
     default: AddLine(Part,Sep);
    END
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

/*--------------------------------------------------------------------------*/

static char BackSepString[TOKLEN];

	static void TeXFlushLine(Word Index)
BEGIN
   UNUSED(Index);

   if (CurrEnv==EnvTabular)
    BEGIN
     for (CurrCol++; CurrCol<ThisTable.TColumnCount; ThisTable.Lines[CurrRow][CurrCol++]=strdup(""));
     CurrRow++;
     if (CurrRow==MAXROWS) error("too many rows in table");
     InitTableRow(CurrRow); CurrCol=0;
    END
   else
    BEGIN
     if (*OutLineBuffer=='\0') strcpy(OutLineBuffer," ");
     FlushLine();
    END
   if (CurrEnv==EnvTabbing) CurrTabStop=0;
END

	static void TeXKillLine(Word Index)
BEGIN
   UNUSED(Index);

   ResetLine();
END

	static void TeXDummy(Word Index)
BEGIN
   UNUSED(Index);
END

	static void TeXDummyNoBrack(Word Index)
BEGIN
   char Token[TOKLEN];
   UNUSED(Index);

   ReadToken(Token);
END

	static void TeXDummyInCurl(Word Index)
BEGIN
   char Token[TOKLEN];
   UNUSED(Index);

   assert_token("{");
   ReadToken(Token);
   assert_token("}");
END

	static void TeXNewCommand(Word Index)
BEGIN
   char Token[TOKLEN];
   int level;
   UNUSED(Index);

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
   UNUSED(Index);

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
   UNUSED(Index);

   assert_token("\\"); 
   ReadToken(Token); assert_token("="); ReadToken(Token); ReadToken(Token);
   assert_token("\\"); ReadToken(Token);
END

	static void TeXAppendix(Word Index)
BEGIN
   int z;
   UNUSED(Index);

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
   int Level=LastLevel,z;
   char Line[TOKLEN],Title[TOKLEN],*run;
   PTocSave NewTocSave,RunToc;

   strcpy(Title,OutLineBuffer); *OutLineBuffer='\0';

   run=Line;
   if (Level<3)
    BEGIN
     run=GetSectionName(run);
     run+=sprintf(run," ");
     if ((Level==2) AND (((strlen(Line)+strlen(Title))&1)==0)) run+=sprintf(run," ");
    END
   sprintf(run,"%s",Title);

   fprintf(outfile,"        %s\n        ",Line);
   for (z=0; z<strlen(Line); z++)
    switch(Level)
     BEGIN
      case 0: fputc('=',outfile); break;
      case 1: fputc('-',outfile); break;
      case 2: fputc(((z&1)==0) ? '-' : ' ',outfile); break;
      case 3: fputc('.',outfile); break;
     END
   fprintf(outfile,"\n");

   if (Level<3)
    BEGIN
     NewTocSave=(PTocSave) malloc(sizeof(TTocSave));
     NewTocSave->Next=Nil;
     run=Line; run=GetSectionName(run); run+=sprintf(run," "); sprintf(run,"%s",Title);
     NewTocSave->TocName=(char *) malloc(6+Level+strlen(Line));
     strcpy(NewTocSave->TocName,Blanks(5+Level));
     strcat(NewTocSave->TocName,Line);
     if (FirstTocSave==Nil) FirstTocSave=NewTocSave;
     else
      BEGIN
       for (RunToc=FirstTocSave; RunToc->Next!=Nil; RunToc=RunToc->Next);
       RunToc->Next=NewTocSave;
      END
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
   char EnvName[TOKLEN],Add[TOKLEN];
   EnvType NEnv;
   Boolean done;
   TColumn NCol;
   int z;
   UNUSED(Index);

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
     case EnvItemize:
     case EnvEnumerate:
     case EnvDescription:
      FlushLine(); if (CurrListDepth==0) fputc('\n',outfile);
      ++CurrListDepth;
      ActLeftMargin=LeftMargin=(CurrListDepth*4)+1;
      RightMargin=70;
      EnumCounter=0;
      break;
     case EnvBiblio:
      FlushLine(); fputc('\n',outfile);
      fprintf(outfile,"        %s\n        ",BiblioName);
      for (z=0; z<strlen(BiblioName); z++) fputc('=',outfile);
      fputc('\n',outfile);
      assert_token("{"); ReadToken(Add); assert_token("}");
      ActLeftMargin=LeftMargin=4+(BibIndent=strlen(Add));
      break;
     case EnvVerbatim:
      FlushLine();
      if ((*BufferLine!='\0') AND (*BufferPtr!='\0'))
       BEGIN
        fprintf(outfile,"%s",BufferPtr);
        *BufferLine='\0'; BufferPtr=BufferLine;
       END
      do
       BEGIN
        fgets(Add,TOKLEN-1,infiles[IncludeNest-1]); CurrLine++;
        done=strstr(Add,"\\end{verbatim}")!=Nil;
        if (NOT done) fprintf(outfile,"%s",Add);
       END
      while (NOT done);
      fputc('\n',outfile);
      break;
     case EnvQuote:
      FlushLine(); fputc('\n',outfile);
      ActLeftMargin=LeftMargin=5;
      RightMargin=70;
      break;
     case EnvTabbing:
      FlushLine(); fputc('\n',outfile); 
      TabStopCnt=0; CurrTabStop=0;
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
      FlushLine(); Alignment=AlignCenter; break;
     case EnvRaggedRight:
      FlushLine(); Alignment=AlignLeft; break;
     case EnvRaggedLeft:
      FlushLine(); Alignment=AlignRight; break;
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
          if (strcmp(Add,"|")==0) NCol=ColBar;
          else if (strcmp(Add,"l")==0) NCol=ColLeft;
          else if (strcmp(Add,"r")==0) NCol=ColRight;
          else if (strcmp(Add,"c")==0) NCol=ColCenter;
          else
           BEGIN
            NCol=ColBar;
            error("unknown table column descriptor");
           END
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
   UNUSED(Index);

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
     case EnvItemize:
     case EnvEnumerate:
     case EnvDescription:
      FlushLine(); if (CurrListDepth==1) fputc('\n',outfile);
      break;
     case EnvBiblio:
     case EnvQuote:
     case EnvTabbing:
      FlushLine(); fputc('\n',outfile);
      break;
     case EnvCenter:
     case EnvRaggedRight:
     case EnvRaggedLeft:
      FlushLine(); break;
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
   char NumString[20],Token[TOKLEN],Acc[TOKLEN];
   UNUSED(Index);

   FlushLine();
   switch(CurrEnv)
    BEGIN
     case EnvItemize:
      LeftMargin=ActLeftMargin-3;
      AddLine(" - ","");
      break;
     case EnvEnumerate:
      LeftMargin=ActLeftMargin-4;
      sprintf(NumString,"%3d ",++EnumCounter);
      AddLine(NumString,"");
      break;
     case EnvDescription:
      ReadToken(Token);
      if (strcmp(Token,"[")!=0) BackToken(Token);
      else
       BEGIN
        collect_token(Acc,"]");
        LeftMargin=ActLeftMargin-4;
        sprintf(NumString,"%3s ",Acc);
        AddLine(NumString,"");
       END
      break;
     default:
      error("\\item not in a list environment");
    END
END

	static void TeXBibItem(Word Index)
BEGIN
   char NumString[20],Token[TOKLEN],Name[TOKLEN],Format[10];
   UNUSED(Index);

   if (CurrEnv!=EnvBiblio) error("\\bibitem not in bibliography environment");

   assert_token("{"); collect_token(Name,"}");

   FlushLine(); fputc('\n',outfile); ++BibCounter;

   LeftMargin=ActLeftMargin-BibIndent-3;
   sprintf(Format,"[%%%dd] ",BibIndent);
   sprintf(NumString,Format,BibCounter);
   AddLine(NumString,"");
   sprintf(NumString,"%d",BibCounter);
   AddCite(Name,NumString);
   ReadToken(Token); *SepString='\0'; BackToken(Token);
END

	static void TeXAddDollar(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("$",BackSepString);
END

	static void TeXAddUnderbar(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("_",BackSepString);
END

        static void TeXAddPot(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("^",BackSepString);
END

	static void TeXAddAmpersand(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("&",BackSepString);
END

        static void TeXAddAt(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("@",BackSepString);
END

        static void TeXAddImm(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("#",BackSepString);
END

        static void TeXAddPercent(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("%",BackSepString);
END

        static void TeXAddSSharp(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal(CH_sz,BackSepString);
END

        static void TeXAddIn(Word Index)  
BEGIN
   UNUSED(Index);

   DoAddNormal("in",BackSepString);
END

        static void TeXAddReal(Word Index)  
BEGIN
   UNUSED(Index);

   DoAddNormal("R",BackSepString);
END

	static void TeXAddGreekMu(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal(CH_mu,BackSepString);
END

	static void TeXAddGreekPi(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("Pi",BackSepString);
END

	static void TeXAddLessEq(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("<=",BackSepString);
END

        static void TeXAddGreaterEq(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal(">=",BackSepString);
END
     
        static void TeXAddNotEq(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("<>",BackSepString);
END

        static void TeXAddMid(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("|",BackSepString);
END  

	static void TeXAddRightArrow(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("->",BackSepString);
END

	static void TeXAddLongRightArrow(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("-->",BackSepString);
END

	static void TeXAddLeftArrow(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("<-",BackSepString);
END

	static void TeXAddLeftRightArrow(Word Index)
BEGIN
   UNUSED(Index);

   DoAddNormal("<->",BackSepString);
END

	static void TeXDoFrac(Word Index)
BEGIN
   UNUSED(Index);

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
   CurrFontType=(TFontType) Index;
END

	static void TeXEnvNewFontType(Word Index)
BEGIN
   char NToken[TOKLEN];

   SaveFont();
   CurrFontType=(TFontType) Index;
   assert_token("{");
   ReadToken(NToken);
   strcpy(SepString,BackSepString);
   BackToken(NToken);
END

	static void TeXNewFontSize(Word Index)
BEGIN
   CurrFontSize=(TFontSize) Index;
END

	static void TeXEnvNewFontSize(Word Index)
BEGIN
   char NToken[TOKLEN];

   SaveFont();
   CurrFontSize=(TFontSize) Index;
   assert_token("{");
   ReadToken(NToken);
   strcpy(SepString,BackSepString);
   BackToken(NToken);
END

	static void TeXAddMarginPar(Word Index)
BEGIN
   UNUSED(Index);

   assert_token("{");
   SaveEnv(EnvMarginPar);
END

	static void TeXAddCaption(Word Index)
BEGIN
   char tmp[100];
   int cnt;
   UNUSED(Index);

   assert_token("{");
   if (CurrEnv!=EnvTable) error("caption outside of a table");
   FlushLine(); fputc('\n',outfile);
   SaveEnv(EnvCaption);
   AddLine(TableName,""); cnt=strlen(TableName);
   GetTableName(tmp); strcat(tmp,": ");
   AddLine(tmp," "); cnt+=1+strlen(tmp);
   LeftMargin=1; ActLeftMargin=cnt+1; RightMargin=70;
END

	static void TeXHorLine(Word Index)
BEGIN
   UNUSED(Index);

   if (CurrEnv!=EnvTabular) error("\\hline outside of a table");

   if (ThisTable.Lines[CurrRow][0]!=Nil) InitTableRow(++CurrRow);
   ThisTable.LineFlags[CurrRow]=True;
   InitTableRow(++CurrRow);
END

	static void TeXMultiColumn(Word Index)
BEGIN
   char Token[TOKLEN],*endptr;
   int cnt;
   UNUSED(Index);

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
   char Token[TOKLEN];
   UNUSED(Index);
   
   assert_token("{"); 
   do
    BEGIN
     ReadToken(Token);
    END
   while (strcmp(Token,"}")!=0);
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
   UNUSED(Index);

   DoAddNormal(Blanks(GetDim(HFactors)),"");
END

	static void TeXVSpace(Word Index)
BEGIN
   int z,erg;
   UNUSED(Index);

   erg=GetDim(VFactors);
   FlushLine();
   for (z=0; z<erg; z++) fputc('\n',outfile);
END

	static void TeXRule(Word Index)
BEGIN
   int h=GetDim(HFactors),v=GetDim(VFactors);
   char Rule[200];
   UNUSED(Index);
   
   for (v=0; v<h; Rule[v++]='-'); Rule[v]='\0';
   DoAddNormal(Rule,BackSepString);
END

	static void TeXAddTabStop(Word Index)
BEGIN
   int z,n,p;
   UNUSED(Index);

   if (CurrEnv!=EnvTabbing) error("tab marker outside of tabbing environment");
   if (TabStopCnt>=TABMAX) error("too many tab stops");
   
   n=strlen(OutLineBuffer);
   for (p=0; p<TabStopCnt; p++)
    if (TabStops[p]>n) break;
   for (z=TabStopCnt-1; z>=p; z--) TabStops[z+1]=TabStops[z];
   TabStops[p]=n; TabStopCnt++;
END

	static void TeXJmpTabStop(Word Index)
BEGIN
   int diff;
   UNUSED(Index);

   if (CurrEnv!=EnvTabbing) error("tab trigger outside of tabbing environment");
   if (CurrTabStop>=TabStopCnt) error("not enough tab stops");

   diff=TabStops[CurrTabStop]-strlen(OutLineBuffer);
   if (diff>0) DoAddNormal(Blanks(diff),"");
   CurrTabStop++;
END

	static void TeXDoVerb(Word Index)
BEGIN
   char Token[TOKLEN],*pos,Marker;
   UNUSED(Index);

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
   UNUSED(Index);

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
   UNUSED(Index);

   assert_token("{"); collect_token(Name,"}");

   if (CurrEnv==EnvCaption) GetTableName(Value);
   else
    BEGIN
     GetSectionName(Value);
     if ((*Value) && (Value[strlen(Value)-1]=='.'))
       Value[strlen(Value)-1]='\0';
    END
   
   AddLabel(Name,Value);
END

	static void TeXWriteRef(Word Index)
BEGIN
   char Name[TOKLEN],Value[TOKLEN];
   UNUSED(Index);

   assert_token("{"); collect_token(Name,"}");
   GetLabel(Name,Value);
   DoAddNormal(Value,BackSepString);
END

	static void TeXWriteCitation(Word Index)
BEGIN
   char Name[TOKLEN],Value[TOKLEN];
   UNUSED(Index);

   assert_token("{"); collect_token(Name,"}");
   GetCite(Name,Value);
   sprintf(Name,"[%s]",Value);
   DoAddNormal(Name,BackSepString);
END

	static void TeXNewParagraph(Word Index)
BEGIN
   UNUSED(Index);

   FlushLine();
   fputc('\n',outfile);
END

	static void TeXContents(Word Index)
BEGIN
   FILE *file=fopen(TocName,"r");
   char Line[200];
   UNUSED(Index);

   if (file==Nil)
    BEGIN
     warning("contents file not found.");
     DoRepass=True; return;
    END

   FlushLine();
   fprintf(outfile,"        %s\n\n",ContentsName);
   while (NOT feof(file))
    BEGIN
     fgets(Line,199,file); fputs(Line,outfile);
    END

   fclose(file);
END

	static void TeXParSkip(Word Index)
BEGIN
   char Token[TOKLEN];
   UNUSED(Index);

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
   UNUSED(Index);

   *Token='\0'; 
   ReadToken(Token);
   if (*SepString=='\0')
    switch (*Token)
     BEGIN
      case 'a': Repl=CH_ae; break;
      case 'e': Repl=CH_ee; break;
      case 'i': Repl=CH_ie; break;
      case 'o': Repl=CH_oe; break;
      case 'u': Repl=CH_ue; break;
      case 'A': Repl=CH_Ae; break;
      case 'E': Repl=CH_Ee; break;
      case 'I': Repl=CH_Ie; break;
      case 'O': Repl=CH_Oe; break;
      case 'U': Repl=CH_Ue; break;
      case 's': Repl=CH_sz; break;
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

	static void TeXNLSGrave(Word Index)
BEGIN
   char Token[TOKLEN],*Repl="";
   Boolean Found=True;
   UNUSED(Index);

   *Token='\0'; 
   ReadToken(Token);
   if (*SepString=='\0')
    switch (*Token)
     BEGIN
      case 'a': Repl=CH_agrave; break;
      case 'A': Repl=CH_Agrave; break;
      case 'e': Repl=CH_egrave; break;
      case 'E': Repl=CH_Egrave; break;
      case 'i': Repl=CH_igrave; break;
      case 'I': Repl=CH_Igrave; break;
      case 'o': Repl=CH_ograve; break;
      case 'O': Repl=CH_Ograve; break;
      case 'u': Repl=CH_ugrave; break;
      case 'U': Repl=CH_Ugrave; break;
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

	static void TeXNLSAcute(Word Index)
BEGIN
   char Token[TOKLEN],*Repl="";
   Boolean Found=True;
   UNUSED(Index);

   *Token='\0'; 
   ReadToken(Token);
   if (*SepString=='\0')
    switch (*Token)
     BEGIN
      case 'a': Repl=CH_aacute; break;
      case 'A': Repl=CH_Aacute; break;
      case 'e': Repl=CH_eacute; break;
      case 'E': Repl=CH_Eacute; break;
      case 'i': Repl=CH_iacute; break;
      case 'I': Repl=CH_Iacute; break;
      case 'o': Repl=CH_oacute; break;
      case 'O': Repl=CH_Oacute; break;
      case 'u': Repl=CH_uacute; break;
      case 'U': Repl=CH_Uacute; break;
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

	static void TeXNLSCirc(Word Index)
BEGIN
   char Token[TOKLEN],*Repl="";
   Boolean Found=True;
   UNUSED(Index);

   *Token='\0'; 
   ReadToken(Token);
   if (*SepString=='\0')
    switch (*Token)
     BEGIN
      case 'a': Repl=CH_acirc; break;
      case 'A': Repl=CH_Acirc; break;
      case 'e': Repl=CH_ecirc; break;
      case 'E': Repl=CH_Ecirc; break;
      case 'i': Repl=CH_icirc; break;
      case 'I': Repl=CH_Icirc; break;
      case 'o': Repl=CH_ocirc; break;
      case 'O': Repl=CH_Ocirc; break;
      case 'u': Repl=CH_ucirc; break;
      case 'U': Repl=CH_Ucirc; break;
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

	static void TeXNLSTilde(Word Index)
BEGIN
   char Token[TOKLEN],*Repl="";
   Boolean Found=True;
   UNUSED(Index);

   *Token='\0'; 
   ReadToken(Token);
   if (*SepString=='\0')
    switch (*Token)
     BEGIN
      case 'n': Repl=CH_ntilde; break;
      case 'N': Repl=CH_Ntilde; break;
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

	static void TeXCedilla(Word Index)
BEGIN
   char Token[TOKLEN];
   UNUSED(Index);

   assert_token("{"); collect_token(Token,"}");
   if (strcmp(Token,"c")==0) strcpy(Token,CH_ccedil);
   if (strcmp(Token,"C")==0) strcpy(Token,CH_Ccedil);

   DoAddNormal(Token,BackSepString);
END

	static Boolean TeXNLSSpec(char *Line)
BEGIN
   Boolean Found=True;
   char *Repl=Nil;
   int cnt=0;

   if (*SepString=='\0')
    switch (*Line)
     BEGIN
      case 'o': cnt=1; Repl=CH_oslash; break;
      case 'O': cnt=1; Repl=CH_Oslash; break;
      case 'a': 
       switch (Line[1])
        BEGIN
         case 'a': cnt=2; Repl=CH_aring; break;
         case 'e': cnt=2; Repl=CH_aelig; break;
         default: Found=False;
        END
       break;
      case 'A': 
       switch (Line[1])
        BEGIN
         case 'A': cnt=2; Repl=CH_Aring; break;
         case 'E': cnt=2; Repl=CH_Aelig; break;
         default: Found=False;
        END
       break;
      default: Found=False;
     END

   if (Found)
    BEGIN
     if (strlen(Repl)!=cnt) memmove(Line+strlen(Repl),Line+cnt,strlen(Line)-cnt+1);
     memcpy(Line,Repl,strlen(Repl)); strcpy(SepString,BackSepString);
    END
   else DoAddNormal("\"",BackSepString);

   BackToken(Line);
   return Found;
END

	static void TeXHyphenation(Word Index)
BEGIN
   char Token[TOKLEN];
   UNUSED(Index);

   assert_token("{"); collect_token(Token,"}");
   AddException(Token);
END

	static void TeXDoPot(void)
BEGIN
   char Token[TOKLEN];

   ReadToken(Token);
   if (*Token=='2')
    BEGIN
     if (strlen(CH_e2)>1) memmove(Token+strlen(CH_e2),Token+1,strlen(Token));
     memcpy(Token,CH_e2,strlen(CH_e2));
    END
   else DoAddNormal("^",BackSepString);

   BackToken(Token);
END

	static void TeXDoSpec(void)
BEGIN
   strcpy(BackSepString,SepString);
   TeXNLS(0);
END

	static void TeXInclude(Word Index)
BEGIN
   char Token[TOKLEN],Msg[TOKLEN];
   UNUSED(Index);

   assert_token("{"); collect_token(Token,"}");
   if ((infiles[IncludeNest]=fopen(Token,"r"))==Nil)
    BEGIN
     sprintf(Msg,"file %s not found",Token);
     error(Msg);
    END
   else IncludeNest++;
END

	static void TeXDocumentStyle(Word Index)
BEGIN
   char Token[TOKLEN];
   UNUSED(Index);

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
     if (strcasecmp(Token, "article") == 0)
      BEGIN
       AddInstTable(TeXTable,"section",0,TeXNewSection);
       AddInstTable(TeXTable,"subsection",1,TeXNewSection);
       AddInstTable(TeXTable,"subsubsection",3,TeXNewSection);
      END
     else
      BEGIN
       AddInstTable(TeXTable,"chapter",0,TeXNewSection);
       AddInstTable(TeXTable,"section",1,TeXNewSection);
       AddInstTable(TeXTable,"subsection",2,TeXNewSection);
       AddInstTable(TeXTable,"subsubsection",3,TeXNewSection);
      END
     assert_token("}");
    END
END

/*--------------------------------------------------------------------------*/

	int main(int argc, char **argv)
BEGIN
   char Line[TOKLEN],Comp[TOKLEN],*p,AuxFile[200];
   int z;

   if (argc<3)
    BEGIN
     fprintf(stderr,"calling convention: %s <input file> <output file>\n",*argv);
     exit(1);
    END

   TeXTable=CreateInstTable(301);

   IncludeNest=0;
   if ((*infiles=fopen(argv[1],"r"))==Nil)
    BEGIN
     perror(argv[1]); exit(3);
    END
   else IncludeNest++;
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
   AddInstTable(TeXTable,"appendix",0,TeXAppendix);
   AddInstTable(TeXTable,"makeindex",0,TeXDummy);
   AddInstTable(TeXTable,"begin",0,TeXBeginEnv);
   AddInstTable(TeXTable,"end",0,TeXEndEnv);
   AddInstTable(TeXTable,"item",0,TeXItem);
   AddInstTable(TeXTable,"bibitem",0,TeXBibItem);
   AddInstTable(TeXTable,"errentry",0,TeXErrEntry);
   AddInstTable(TeXTable,"$",0,TeXAddDollar);
   AddInstTable(TeXTable,"_",0,TeXAddUnderbar);
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
   AddInstTable(TeXTable,"printindex",0,TeXDummy);
   AddInstTable(TeXTable,"tableofcontents",0,TeXContents);
   AddInstTable(TeXTable,"rule",0,TeXRule);
   AddInstTable(TeXTable,"\"",0,TeXNLS);
   AddInstTable(TeXTable,"`",0,TeXNLSGrave);
   AddInstTable(TeXTable,"'",0,TeXNLSAcute);
   AddInstTable(TeXTable,"^",0,TeXNLSCirc);
   AddInstTable(TeXTable,"~",0,TeXNLSTilde);
   AddInstTable(TeXTable,"c",0,TeXCedilla);
   AddInstTable(TeXTable,"newif",0,TeXDummy);
   AddInstTable(TeXTable,"fi",0,TeXDummy);
   AddInstTable(TeXTable,"ifelektor",0,TeXDummy);
   AddInstTable(TeXTable,"elektortrue",0,TeXDummy);
   AddInstTable(TeXTable,"elektorfalse",0,TeXDummy);
   AddInstTable(TeXTable,"input",0,TeXInclude);

   for (z=0; z<CHAPMAX; Chapters[z++]=0);
   TableNum=0; FontNest=0;
   TabStopCnt=0; CurrTabStop=0;
   ErrState=FracState=(-1);
   InAppendix=False;
   EnvStack=Nil;
   CurrEnv=EnvNone; CurrListDepth=0;
   ActLeftMargin=LeftMargin=1; RightMargin=70;
   Alignment=AlignNone;
   EnumCounter=0;
   CurrFontType=FontStandard; CurrFontSize=FontNormalSize;
   FontStack=Nil; FirstRefSave=Nil; FirstCiteSave=Nil; FirstTocSave=Nil;
   *SideMargin='\0';
   DoRepass=False;
   BibIndent=BibCounter=0;
   GermanMode=True; SetLang(False);

   strcpy(TocName,argv[1]); 
   if ((p=strrchr(TocName,'.'))!=Nil) *p='\0';
   strcat(TocName,".dtoc");

   strcpy(AuxFile,argv[1]); 
   if ((p=strrchr(AuxFile,'.'))!=Nil) *p='\0';
   strcat(AuxFile,".daux");
   ReadAuxFile(AuxFile);

   while (1)
    BEGIN
     if (NOT ReadToken(Line)) break;
     if (strcmp(Line,"\\")==0)
      BEGIN
       strcpy(BackSepString,SepString);
       if (NOT ReadToken(Line)) error("unexpected end of file");
       if (*SepString!='\0') BackToken(Line);
       else if (NOT LookupInstTable(TeXTable,Line))
        if (NOT TeXNLSSpec(Line))
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
        case EnvMarginPar: RestoreEnv(); break;
        case EnvCaption: FlushLine(); RestoreEnv(); break;
        case EnvHeading: EndSectionHeading(); RestoreEnv(); break;
        default: RestoreFont();
       END
     else DoAddNormal(Line,SepString);
    END
   FlushLine();
   DestroyInstTable(TeXTable);

   for (z=0; z<IncludeNest; fclose(infiles[z++]));
   fclose(outfile);

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

