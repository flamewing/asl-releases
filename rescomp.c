/* rescomp.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Compiler fuer Message-Dateien                                             */
/*                                                                           */
/*    17.5.1998  Symbol gegen Mehrfachinklusion eingebaut                    */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "strutil.h"
#include "bpemu.h"

/*****************************************************************************/

typedef struct _TMsgList
         {
          struct _TMsgList *Next;
          LongInt Position;
          char *Contents;
         } TMsgList,*PMsgList;

typedef struct
         {
          PMsgList Messages,LastMessage;
          char *CtryName;
          LongInt *CtryCodes,CtryCodeCnt;
          LongInt FilePos,HeadLength,TotLength;
         } TMsgCat,*PMsgCat;

typedef struct
         {
          char *AbbString,*Character;
         } TransRec;

/*****************************************************************************/

#ifdef __TURBOC__ 
unsigned _stklen=16384;
#endif

static char *IdentString="AS Message Catalog - not readable\n\032\004";

static LongInt MsgCounter;
static PMsgCat MsgCats;
static LongInt CatCount,DefCat;
static FILE *SrcFile,*MsgFile,*HFile;
static char *IncSym;

static TransRec TransRecs[]=
                {{"&auml;",CH_ae},
                 {"&Auml;",CH_Ae},
                 {"&ouml;",CH_oe},
                 {"&Ouml;",CH_Oe},
                 {"&uuml;",CH_ue},
                 {"&Uuml;",CH_Ue},
                 {"&szlig;",CH_sz},
                 {"\\n","\n"},
                 {Nil,Nil}};

/*****************************************************************************/

typedef struct _TIncList
         {
          struct _TIncList *Next;
          FILE *Contents;
         } TIncList,*PIncList;

PIncList IncList=Nil;

	static void GetLine(char *Dest)
BEGIN
   PIncList OneFile;

   ReadLn(SrcFile,Dest);
   if (strncasecmp(Dest,"INCLUDE",7)==0)
    BEGIN
     OneFile=(PIncList) malloc(sizeof(TIncList));
     OneFile->Next=IncList; OneFile->Contents=SrcFile;
     IncList=OneFile;
     strcpy(Dest,Dest+7); KillPrefBlanks(Dest); KillPrefBlanks(Dest);
     SrcFile=fopen(Dest,"r");
     if (SrcFile==Nil)
      BEGIN
       perror(Dest); exit(2);
      END
     GetLine(Dest);
    END
   if ((feof(SrcFile)) AND (IncList!=Nil))
    BEGIN
     fclose(SrcFile);
     OneFile=IncList; IncList=OneFile->Next;
     SrcFile=OneFile->Contents; free(OneFile);
    END
END

/*****************************************************************************/

	static void SynError(char *LinePart)
BEGIN
   fprintf(stderr,"syntax error : %s\n",LinePart); exit(10);
END

/*---------------------------------------------------------------------------*/

	static void Process_LANGS(char *Line)
BEGIN
   char NLine[1024],*p,*p2,*p3,*end,z,Part[1024];
   PMsgCat PCat;
   int num;
  
   strmaxcpy(NLine,Line,1024);
   KillPrefBlanks(NLine); KillPostBlanks(NLine);

   p=NLine; z=0;
   while (*p!='\0')
    BEGIN
     for (; ((*p!='\0') AND (NOT isspace((unsigned int) *p))); p++);
     for (; ((*p!='\0') AND (isspace((unsigned int) *p))); p++);
     z++;
    END

   MsgCats=(PMsgCat) malloc(sizeof(TMsgCat)*(CatCount=z)); p=NLine;
   for (z=0,PCat=MsgCats; z<CatCount; z++,PCat++)
    BEGIN
     PCat->Messages=Nil;
     PCat->TotLength=0;
     PCat->CtryCodeCnt=0;
     for (p2=p; ((*p2!='\0') AND (NOT isspace((unsigned int) *p2))); p2++);
     if (*p2=='\0') strcpy(Part,p);
     else
      BEGIN
       *p2='\0'; strcpy(Part,p);
       for (p=p2+1; ((*p!='\0') AND (isspace((unsigned int) *p2))); p++);
      END
     if (Part[strlen(Part)-1]!=')') SynError(Part); Part[strlen(Part)-1]='\0';
     p2=strchr(Part,'(');
     if (p2==Nil) SynError(Part); *p2='\0';
     PCat->CtryName=strdup(Part); p2++;
     do
      BEGIN
       for (p3=p2; ((*p3!='\0') AND (*p3!=',')); p3++);
       switch (*p3)
        BEGIN
         case '\0':
          num=strtol(p2,&end,10); p2=p3; break;
         case ',':
          *p3='\0'; num=strtol(p2,&end,10); p2=p3+1; break;
         default:
          num=0; 
        END
       if (*end!='\0') SynError("numeric format");
       PCat->CtryCodes=(PCat->CtryCodeCnt==0) ?
                       (LongInt *) malloc(sizeof(LongInt)) :
                       (LongInt *) realloc(PCat->CtryCodes,sizeof(LongInt)*(PCat->CtryCodeCnt+1));
       PCat->CtryCodes[PCat->CtryCodeCnt++]=num;
      END
     while (*p2!='\0');
    END
END

	static void Process_DEFAULT(char *Line)
BEGIN
   PMsgCat z;

   if (CatCount==0) SynError("DEFAULT before LANGS");
   KillPrefBlanks(Line); KillPostBlanks(Line);
   for (z=MsgCats; z<MsgCats+CatCount; z++) if (strcasecmp(z->CtryName,Line)==0) break;
   if (z>=MsgCats+CatCount) SynError("unknown country name in DEFAULT");
   DefCat=z-MsgCats;
END

	static void Process_MESSAGE(char *Line)
BEGIN
   char Msg[4096];
   String OneLine;
   int l;
   PMsgCat z;
   TransRec *PRec;
   char *pos;
   PMsgList List;
   Boolean Cont;

   KillPrefBlanks(Line); KillPostBlanks(Line);
   if (HFile!=Nil) fprintf(HFile,"#define Num_%s %d\n",Line,MsgCounter);
   MsgCounter++;
   
   for (z=MsgCats; z<MsgCats+CatCount; z++)
    BEGIN
     *Msg='\0';
     do
      BEGIN
       GetLine(OneLine); KillPrefBlanks(OneLine); KillPostBlanks(OneLine);
       l=strlen(OneLine);
       if ((Cont=(OneLine[l-1]=='\\')))
        BEGIN
         OneLine[l-1]='\0'; KillPostBlanks(OneLine); l=strlen(OneLine);
        END
       if (*OneLine!='"') SynError(OneLine);
       if (OneLine[l-1]!='"') SynError(OneLine); OneLine[l-1]='\0';
       strmaxcat(Msg,OneLine+1,4095);
      END
     while (Cont);
     for (PRec=TransRecs; PRec->AbbString!=Nil; PRec++)
      while ((pos=strstr(Msg,PRec->AbbString))!=Nil)
       BEGIN
        strcpy(pos,pos+strlen(PRec->AbbString)-1);
        *pos= *PRec->Character;
       END
     List=(PMsgList) malloc(sizeof(TMsgList));
     List->Next=Nil;
     List->Position=z->TotLength;
     List->Contents=strdup(Msg);
     if (z->Messages==Nil) z->Messages=List;
     else z->LastMessage->Next=List;
     z->LastMessage=List;
     z->TotLength+=strlen(Msg)+1;
    END
END

/*---------------------------------------------------------------------------*/

	int main(int argc, char **argv)
BEGIN
   char Line[1024],Cmd[1024],*p,Save;
   PMsgCat z;
   TMsgCat TmpCat;
   PMsgList List;
   int c;
   time_t stamp;
   LongInt Id1,Id2;
   LongInt RunPos,StrPos;

   endian_init(); strutil_init();

   if ((argc<3) OR (argc>4))
    BEGIN
     fprintf(stderr,"usage: %s <input resource file> <output msg file> [header file]\n",*argv);
     exit(1);
    END

   SrcFile=fopen(argv[1],"r");
   if (SrcFile==Nil)
    BEGIN
     perror(argv[1]); exit(2);
    END

   if (argc==4)
    BEGIN
     HFile=fopen(argv[3],"w");
     if (HFile==Nil)
      BEGIN
       perror(argv[3]); exit(3);
      END
     IncSym=strdup(argv[3]);
     for (p=IncSym; *p!='\0'; p++)
      if (isalpha(((unsigned int) *p)&0xff)) *p=toupper(*p);
      else *p='_';
    END
   else HFile=Nil;

   stamp=GetFileTime(argv[1]); Id1=stamp&0x7fffffff;
   Id2=0;
   for (c=0; c<min(strlen(argv[1]),4); c++)
    Id2=(Id2<<8)+((Byte) argv[1][c]);
   if (HFile!=Nil)
    BEGIN
     fprintf(HFile,"#ifndef %s\n",IncSym);
     fprintf(HFile,"#define %s\n",IncSym);
     fprintf(HFile,"#define MsgId1 %ld\n",(long) Id1);
     fprintf(HFile,"#define MsgId2 %ld\n",(long) Id2);
    END

   MsgCounter=CatCount=0; MsgCats=Nil; DefCat= -1;
   while (NOT feof(SrcFile))
    BEGIN
     GetLine(Line); KillPrefBlanks(Line); KillPostBlanks(Line);
     if ((*Line!=';') AND (*Line!='#') AND (*Line!='\0'))
      BEGIN
       for (p=Line; ((NOT isspace((unsigned int) *p)) AND (*p!='\0')); p++);
       Save= *p; *p='\0'; strmaxcpy(Cmd,Line,1024); *p=Save; strcpy(Line,p);
       if (strcasecmp(Cmd,"LANGS")==0) Process_LANGS(Line);
       else if (strcasecmp(Cmd,"DEFAULT")==0) Process_DEFAULT(Line);
       else if (strcasecmp(Cmd,"MESSAGE")==0) Process_MESSAGE(Line);
      END
    END

   fclose(SrcFile);
   if (HFile!=Nil)
    BEGIN
     fprintf(HFile,"#endif /* #ifndef %s */\n",IncSym);
     fclose(HFile);
    END

   MsgFile=fopen(argv[2],OPENWRMODE);
   if (MsgFile==Nil)
    BEGIN
     perror(argv[2]); exit(4);
    END

   /* Magic-String */

   fwrite(IdentString,1,strlen(IdentString),MsgFile);
   Write4(MsgFile,&Id1); Write4(MsgFile,&Id2);

   /* Default nach vorne */

   if (DefCat>0)
    BEGIN
     TmpCat=MsgCats[0]; MsgCats[0]=MsgCats[DefCat]; MsgCats[DefCat]=TmpCat;
    END

   /* Startadressen String-Kataloge berechnen */

   RunPos=ftell(MsgFile)+1;
   for (z=MsgCats; z<MsgCats+CatCount; z++)
    RunPos+=(z->HeadLength=strlen(z->CtryName)+1+4+4+(4*z->CtryCodeCnt)+4);
   for (z=MsgCats; z<MsgCats+CatCount; z++)
    BEGIN
     z->FilePos=RunPos; RunPos+=z->TotLength+(4*MsgCounter);
    END

   /* Country-Records schreiben */

   for (z=MsgCats; z<MsgCats+CatCount; z++)
    BEGIN
     fwrite(z->CtryName,1,strlen(z->CtryName)+1,MsgFile);
     Write4(MsgFile,&(z->TotLength));
     Write4(MsgFile,&(z->CtryCodeCnt));
     for (c=0; c<z->CtryCodeCnt; c++) Write4(MsgFile,z->CtryCodes+c);
     Write4(MsgFile,&(z->FilePos));
    END
   Save='\0'; fwrite(&Save,1,1,MsgFile);

   /* Stringtabellen schreiben */

   for (z=MsgCats; z<MsgCats+CatCount; z++)
    BEGIN
     for (List=z->Messages; List!=Nil; List=List->Next)
      BEGIN
       StrPos=z->FilePos+(4*MsgCounter)+List->Position;
       Write4(MsgFile,&StrPos);
      END
     for (List=z->Messages; List!=Nil; List=List->Next)
      fwrite(List->Contents,1,strlen(List->Contents)+1,MsgFile);
    END

   /* faeaedisch... */

   fclose(MsgFile);

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

