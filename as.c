/* as.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Hauptmodul                                                                */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           24. 6.1998 Zeichenübersetzungstabellen                          */
/*           30. 6.1998 Ausgabe in MacPro-File auch wenn Zeile nur aus       */
/*                      Kommentar oder Label besteht                         */
/*           18. 7.1998 IRPC-Statement                                       */
/*           24. 7.1998 Debug-Modus NoICE                                    */
/*           25. 7.1998 Formate glattgezogen                                 */
/*           16. 8.1998 Datei-Adressbereiche zurücksetzen                    */
/*           17. 8.1998 InMacroFlag nach asmdef verschoben                   */
/*           19. 8.1998 BranchExt-Initialisierung                            */
/*           25. 8.1998 i960-Initialisierung                                 */
/*           28. 8.1998 32-Bit-Listen gehen auch korrekt mit                 */
/*                      Codelaengen != 4*n um                                */
/*           30. 8.1998 uPD7720-Initialisierung                              */
/*                      Einrueckung fuer 'R' in Retracted-Zeilen im Listing  */
/*                      war nicht korrekt                                    */
/*           13. 9.1998 uPD77230-Initialisierung                             */
/*           30. 9.1998 SYM53C8xx-Initialisierung                            */
/*            3.12.1998 8008-Initialisierung                                 */
/*            9. 1.1999 PCs erst nach Schreiben des Codes hochzaehlen        */
/*                      ChkPC mit Adresse als Parameter                      */
/*           30. 1.1999 Formate maschinenunabhaengig gemacht                 */
/*           12. 2.1999 Compilerwarnungen beseitigt                          */
/*           25. 3.1999 SC14xxx-Initialisierung                              */
/*           17. 4.1999 CPU per Kommandozeile setzen                         */
/*           18. 4.1999 Ausgabeliste Sharefiles                              */
/*            4. 7.1999 F2MC8-Initialisierung                                */
/*            8. 8.1999 Externliste immer am  Ende einer Zeile loeschen      */
/*           14. 8.1999 Initialisierung ACE                                  */
/*            5.11.1999 ExtendErrors, 2. Stufe                               */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>
#include <setjmp.h>

#include "version.h"
#include "endian.h"
#include "bpemu.h"

#include "stdhandl.h"
#include "cmdarg.h"
#include "nls.h"
#include "nlmessages.h"
#include "as.rsc"
#include "ioerrs.h"
#include "strutil.h"
#include "stringlists.h"
#include "asmitree.h"
#include "chunks.h"
#include "asminclist.h"
#include "asmfnums.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmmac.h"
#include "asmif.h"
#include "asmcode.h"
#include "asmdebug.h"
#include "asmrelocs.h"
#include "asmallg.h"
#include "codepseudo.h"
#include "as.h"

#include "code68k.h"
#include "code56k.h"
#include "code601.h"
#include "codemcore.h"
#include "code68.h"
#include "code6805.h"
#include "code6809.h"
#include "code6812.h"
#include "code6816.h"
#include "codeh8_3.h"
#include "codeh8_5.h"
#include "code7000.h"
#include "code65.h"
#include "code7700.h"
#include "code4500.h"
#include "codem16.h"
#include "codem16c.h"
#include "code4004.h"
#include "code8008.h"
#include "code48.h"
#include "code51.h"
#include "code96.h"
#include "code85.h"
#include "code86.h"
#include "code960.h"
#include "code8x30x.h"
#include "codexa.h"
#include "codeavr.h"
#include "code29k.h"
#include "code166.h"
#include "codez80.h"
#include "codez8.h"
#include "code96c141.h"
#include "code90c141.h"
#include "code87c800.h"
#include "code47c00.h"
#include "code97c241.h"
#include "code16c5x.h"
#include "code16c8x.h"
#include "code17c4x.h"
#include "codest6.h"
#include "codest7.h"
#include "codest9.h"
#include "code6804.h"
#include "code3201x.h"
#include "code3202x.h"
#include "code3203x.h"
#include "code3205x.h"
#include "code3206x.h"
#include "code9900.h"
#include "codetms7.h"
#include "code370.h"
#include "codemsp.h"
#include "codescmp.h"
#include "codecop8.h"
#include "codesc14xxx.h"
#include "codeace.h"
#include "code78c10.h"
#include "code75k0.h"
#include "code78k0.h"
#include "code7720.h"
#include "code77230.h"
#include "code53c8xx.h"
#include "codefmc8.h"
#include "as1750.h"
/**          Code21xx};**/

/**
VAR
   ParCnt,k:Integer;
   CPU:CPUVar;**/
static String FileMask;
static long StartTime,StopTime;
static Boolean GlobErrFlag;
static Boolean MasterFile;

/*=== Zeilen einlesen ======================================================*/


#if 0
#define dbgentry(str) printf("***enter %s\n",str);
#define dbgexit(str) printf("***exit %s\n",str);
#else
#define dbgentry(str) {}
#define dbgexit(str) {}
#endif

        static void NULL_Restorer(PInputTag PInp)
BEGIN
END

        static Boolean NULL_GetPos(PInputTag PInp, char *dest)
BEGIN
   *dest='\0'; return False;
END

        static void GenerateProcessor(PInputTag *PInp)
BEGIN
   *PInp=(PInputTag) malloc(sizeof(TInputTag));
   (*PInp)->IsMacro=False;
   (*PInp)->Next=Nil;
   (*PInp)->First=True;
   (*PInp)->OrigDoLst=DoLst;
   (*PInp)->StartLine=CurrLine;
   (*PInp)->ParCnt=0; (*PInp)->ParZ=0;
   InitStringList(&((*PInp)->Params));
   (*PInp)->LineCnt=0; (*PInp)->LineZ=1;
   (*PInp)->Lines=Nil;
   (*PInp)->SpecName[0]='\0';
   (*PInp)->IsEmpty=False;
   (*PInp)->Buffer=Nil;
   (*PInp)->Datei=Nil;
   (*PInp)->IfLevel=SaveIFs();
   (*PInp)->Restorer=NULL_Restorer;
   (*PInp)->GetPos=NULL_GetPos;
END

/*=========================================================================*/
/* Listing erzeugen */

        static void MakeList_Gen2Line(char *h, Word EffLen, Word *n)
BEGIN
   int z,Rest;

   Rest=EffLen-(*n); if (Rest>8) Rest=8; if (DontPrint) Rest=0;
   for (z=0; z<(Rest>>1); z++)
    BEGIN
     strmaxcat(h,HexString(WAsmCode[(*n)>>1],4),255);
     strmaxcat(h," ",255); 
     (*n)+=2;
    END
   if ((Rest&1)!=0)
    BEGIN
     strmaxcat(h,HexString(BAsmCode[*n],2),255);
     strmaxcat(h,"   ",255); 
     (*n)++;
    END
   for (z=1; z<=(8-Rest)>>1; z++)
    strmaxcat(h,"     ",255);
END

	static void MakeList_Gen4Line(char *h, Word EffLen, Word *n)
BEGIN
   int z,Rest,wr=0;

   Rest=EffLen-(*n); if (Rest>8) Rest=8; if (DontPrint) Rest=0;
   for (z=0; z<(Rest>>2); z++)
    BEGIN
     strmaxcat(h,HexString(DAsmCode[(*n)>>2],8),255);
     strmaxcat(h," ",255);
     (*n)+=4; wr+=9;
    END
   for (z=0; z<(Rest&3); z++)
    BEGIN
     strmaxcat(h,HexString(BAsmCode[(*n)++],2),255);
     strmaxcat(h," ",255);
     wr+=3;
    END
   strmaxcat(h,Blanks(20-wr),255);
END

        static void MakeList(void)
BEGIN
   String h,h2,Tmp;
   Byte i,k;
   Word n;
   Word EffLen;

   EffLen=CodeLen*Granularity();

   if ((NOT ListToNull) AND (DoLst) AND ((ListMask&1)!=0) AND (NOT IFListMask()))
    BEGIN
     /* Zeilennummer / Programmzaehleradresse: */

     if (IncDepth==0) strmaxcpy(h2,"   ",255);
     else
      BEGIN
       sprintf(Tmp,IntegerFormat,IncDepth);
       sprintf(h2,"(%s)",Tmp);
      END
     if ((ListMask&0x10)!=0)
      BEGIN
       sprintf(h,"%5d/",CurrLine); strmaxcat(h2,h,255);
      END
     strmaxcpy(h,h2,255); strmaxcat(h,HexBlankString(EProgCounter()-CodeLen,8),255);
     strmaxcat(h,Retracted?" R ":" : ",255);

     /* Extrawurst in Listing ? */
     
     if (*ListLine!='\0')
      BEGIN
       strmaxcat(h,ListLine,255);
       strmaxcat(h,Blanks(20-strlen(ListLine)),255);
       strmaxcat(h,OneLine,255);
       WrLstLine(h);
       *ListLine='\0';
      END

     /* Code ausgeben */

     else
      switch (ActListGran)
       BEGIN
        case 4:
         n=0; MakeList_Gen4Line(h,EffLen,&n);
         strmaxcat(h,OneLine,255); WrLstLine(h);
         if (NOT DontPrint)
          while (n<EffLen)
           BEGIN
            strmaxcpy(h,"                    ",255);
            MakeList_Gen4Line(h,EffLen,&n);
            WrLstLine(h);
           END
         break;
        case 2:
         n=0; MakeList_Gen2Line(h,EffLen,&n);
         strmaxcat(h,OneLine,255); WrLstLine(h);
         if (NOT DontPrint)
          while (n<EffLen)
           BEGIN
            strmaxcpy(h,"                    ",255);
            MakeList_Gen2Line(h,EffLen,&n);
            WrLstLine(h);
           END
         break;
        default:
         if ((TurnWords) AND (Granularity()!=ActListGran)) DreheCodes();
         for (i=0; i<6; i++)
          if ((NOT DontPrint) AND (EffLen>i))
           BEGIN
            strmaxcat(h,HexString(BAsmCode[i],2),255); strmaxcat(h," ",255);
           END
          else strmaxcat(h,"   ",255);
         strmaxcat(h,"  ",255); strmaxcat(h,OneLine,255);
         WrLstLine(h);
         if ((EffLen>6) AND (NOT DontPrint))
          BEGIN
           EffLen-=6;
           n=EffLen/6; if ((EffLen%6)==0) n--;
           for (i=0; i<=n; i++)
            BEGIN
             strmaxcpy(h,"                    ",255);
             for (k=0; k<6; k++)
              if (EffLen>i*6+k) 
               BEGIN
                strmaxcat(h,HexString(BAsmCode[i*6+k+6],2),255);
                strmaxcat(h," ",255);
               END
             WrLstLine(h);
            END
          END
         if ((TurnWords) AND (Granularity()!=ActListGran)) DreheCodes();
       END

    END
END

/*=========================================================================*/
/* Makroprozessor */

/*-------------------------------------------------------------------------*/
/* allgemein gebrauchte Subfunktionen */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* werden gebraucht, um festzustellen, ob innerhalb eines Makrorumpfes weitere
   Makroschachtelungen auftreten */

        Boolean MacroStart(void)
BEGIN
   return ((Memo("MACRO")) OR (Memo("IRP")) OR (Memo("IRPC")) OR (Memo("REPT")) OR (Memo("WHILE")));
END

        Boolean MacroEnd(void)
BEGIN
   return (Memo("ENDM"));
END

/*-------------------------------------------------------------------------*/
/* Dieser Einleseprozessor dient nur dazu, eine fehlerhafte Makrodefinition
  bis zum Ende zu ueberlesen */

        static void WaitENDM_Processor(void)
BEGIN
   POutputTag Tmp;

   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;
   if (FirstOutputTag->NestLevel<=-1)
    BEGIN
     Tmp=FirstOutputTag;
     FirstOutputTag=Tmp->Next;
     free(Tmp);
    END
END

        static void AddWaitENDM_Processor(void)
BEGIN
   POutputTag Neu;

   Neu=(POutputTag) malloc(sizeof(TOutputTag));
   Neu->Processor=WaitENDM_Processor;
   Neu->NestLevel=0;
   Neu->Next=FirstOutputTag;
   FirstOutputTag=Neu;
END

/*-------------------------------------------------------------------------*/
/* normale Makros */

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine leitet die Quellcodezeilen bei der Makrodefinition in den
   Makro-Record um */

	static void MACRO_OutProcessor(void)
BEGIN
   POutputTag Tmp;
   int z;
   StringRecPtr l;
   PMacroRec GMacro;
   String s;

   if ((MacroOutput) AND (FirstOutputTag->DoExport))
    BEGIN
     errno=0;
     fprintf(MacroFile,"%s\n",OneLine);
     ChkIO(10004);
    END
   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;
   if (FirstOutputTag->NestLevel!=-1)
    BEGIN
     strmaxcpy(s,OneLine,255); KillCtrl(s);
     l=FirstOutputTag->Params;
     for (z=1; z<=FirstOutputTag->Mac->ParamCount; z++)
      CompressLine(GetStringListNext(&l),z,s);
     if (HasAttrs) CompressLine(AttrName,ParMax+1,s);
     AddStringListLast(&(FirstOutputTag->Mac->FirstLine),s);
    END

   if (FirstOutputTag->NestLevel==-1)
    BEGIN
     if (IfAsm)
      BEGIN
       AddMacro(FirstOutputTag->Mac,FirstOutputTag->PubSect,True);
       if ((FirstOutputTag->DoGlobCopy) AND (SectionStack!=Nil))
        BEGIN
         GMacro=(PMacroRec) malloc(sizeof(MacroRec));
         GMacro->Name=strdup(FirstOutputTag->GName);
         GMacro->ParamCount=FirstOutputTag->Mac->ParamCount;
         GMacro->FirstLine=DuplicateStringList(FirstOutputTag->Mac->FirstLine);
         AddMacro(GMacro,FirstOutputTag->GlobSect,False);
        END
      END
     else ClearMacroRec(&(FirstOutputTag->Mac));

     Tmp=FirstOutputTag; FirstOutputTag=Tmp->Next;
     ClearStringList(&(Tmp->Params)); free(Tmp);
    END
END

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Hierher kommen bei einem Makroaufruf die expandierten Zeilen */

        Boolean MACRO_Processor(PInputTag PInp, char *erg)
BEGIN
   StringRecPtr Lauf;
   int z;
   Boolean Result;
   
   Result=True;

   Lauf=PInp->Lines; for (z=1; z<=PInp->LineZ-1; z++) Lauf=Lauf->Next;
   strcpy(erg,Lauf->Content);
   Lauf=PInp->Params;
   for (z=1; z<=PInp->ParCnt; z++)
    BEGIN
     ExpandLine(Lauf->Content,z,erg);
     Lauf=Lauf->Next;
    END
   if (HasAttrs) ExpandLine(PInp->SaveAttr,ParMax+1,erg);

   CurrLine=PInp->StartLine;
   InMacroFlag=True;

   if (PInp->LineZ==1) PushLocHandle(GetLocHandle());

   if (++(PInp->LineZ)>PInp->LineCnt) Result=False;

   return Result;
END

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Initialisierung des Makro-Einleseprozesses */

        static Boolean ReadMacro_SearchArg(char *Test, char *Comp, Boolean *Erg)
BEGIN
   if (strcasecmp(Test,Comp)==0)
    BEGIN
     *Erg=True; return True;
    END
   else if ((strlen(Test)>2) AND (strncasecmp(Test,"NO",2)==0) AND (strcasecmp(Test+2,Comp)==0))
    BEGIN
     *Erg=False; return True;
    END
   else return False;
END

        static Boolean ReadMacro_SearchSect(char *Test_O, char *Comp, Boolean *Erg, LongInt *Section)
BEGIN
   char *p;
   String Test,Sect;

   strmaxcpy(Test,Test_O,255); KillBlanks(Test);
   p=strchr(Test,':'); 
   if (p==Nil) *Sect='\0';
   else
    BEGIN
     strmaxcpy(Sect,p+1,255); *p='\0';
    END
   if ((strlen(Test)>2) AND (strncasecmp(Test,"NO",2)==0) AND (strcasecmp(Test+2,Comp)==0))
    BEGIN
     *Erg=False; return True;
    END
   else if (strcasecmp(Test,Comp)==0)
    BEGIN
     *Erg=True;
     return (IdentifySection(Sect,Section));
    END
   else return False;
END

        static void ReadMacro(void)
BEGIN
   String PList;
   PSaveSection RunSection;
   PMacroRec OneMacro;
   int z1,z2;
   POutputTag Neu;

   Boolean DoMacExp,DoPublic;
   LongInt HSect;
   Boolean ErrFlag;

   CodeLen=0; ErrFlag=False;

   /* Makronamen pruefen */
   /* Definition nur im ersten Pass */

   if (PassNo!=1) ErrFlag=True;
   else if (NOT ExpandSymbol(LabPart)) ErrFlag=True;
   else if (NOT ChkSymbName(LabPart))
    BEGIN
     WrXError(1020,LabPart); ErrFlag=True;
    END

   Neu=(POutputTag) malloc(sizeof(TOutputTag));
   Neu->Processor=MACRO_OutProcessor;
   Neu->NestLevel=0;
   Neu->Params=Nil;
   Neu->DoExport=False;
   Neu->DoGlobCopy=False;
   Neu->Next=FirstOutputTag;

   /* Argumente ueberpruefen */

   DoMacExp=LstMacroEx; DoPublic=False;
   *PList='\0'; z2=0;
   for (z1=1; z1<=ArgCnt; z1++)
    if ((ArgStr[z1][0]=='{') AND (ArgStr[z1][strlen(ArgStr[z1])-1]=='}'))
     BEGIN
      strcpy(ArgStr[z1],ArgStr[z1]+1); ArgStr[z1][strlen(ArgStr[z1])-1]='\0';
      if (ReadMacro_SearchArg(ArgStr[z1],"EXPORT",&(Neu->DoExport)));
      else if (ReadMacro_SearchArg(ArgStr[z1],"EXPAND",&DoMacExp)) 
       BEGIN
        strmaxcat(PList,",",255); strmaxcat(PList,ArgStr[z1],255);
       END
      else if (ReadMacro_SearchSect(ArgStr[z1],"GLOBAL",&(Neu->DoGlobCopy),&(Neu->GlobSect)));
      else if (ReadMacro_SearchSect(ArgStr[z1],"PUBLIC",&DoPublic,&(Neu->PubSect)));
      else
       BEGIN
        WrXError(1465,ArgStr[z1]); ErrFlag=True;
       END
     END
    else
     BEGIN
      strmaxcat(PList,",",255); strmaxcat(PList,ArgStr[z1],255); z2++;
      if (NOT ChkMacSymbName(ArgStr[z1]))
       BEGIN
        WrXError(1020,ArgStr[z1]); ErrFlag=True;
       END
      AddStringListLast(&(Neu->Params),ArgStr[z1]);
     END

   /* Abbruch bei Fehler */

   if (ErrFlag)
    BEGIN
     ClearStringList(&(Neu->Params));
     free(Neu);
     AddWaitENDM_Processor();
     return;
    END

   /* Bei Globalisierung Namen des Extramakros ermitteln */

   if (Neu->DoGlobCopy)
    BEGIN
     strmaxcpy(Neu->GName,LabPart,255);
     RunSection=SectionStack; HSect=MomSectionHandle;
     while ((HSect!=Neu->GlobSect) AND (RunSection!=Nil))
      BEGIN
       strmaxprep(Neu->GName,"_",255); strmaxprep(Neu->GName,GetSectionName(HSect),255);
       HSect=RunSection->Handle; RunSection=RunSection->Next;
      END
    END
   if (NOT DoPublic) Neu->PubSect=MomSectionHandle;

   OneMacro=(PMacroRec) malloc(sizeof(MacroRec)); Neu->Mac=OneMacro;
   if ((MacroOutput) AND (Neu->DoExport))
    BEGIN 
     if (strlen(PList)!=0) strcpy(PList,PList+1);
     errno=0;
     if (Neu->DoGlobCopy) fprintf(MacroFile,"%s MACRO %s\n",Neu->GName,PList);
     else fprintf(MacroFile,"%s MACRO %s\n",LabPart,PList);
     ChkIO(10004); 
    END
   OneMacro->Used=False;
   OneMacro->Name=strdup(LabPart);
   OneMacro->ParamCount=z2;
   OneMacro->FirstLine=Nil; OneMacro->LocMacExp=DoMacExp;

   FirstOutputTag=Neu;
END

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Beendigung der Expansion eines Makros */

	static void MACRO_Cleanup(PInputTag PInp)
BEGIN
   ClearStringList(&(PInp->Params));
END

	static Boolean MACRO_GetPos(PInputTag PInp, char *dest)
BEGIN
   String Tmp;

   sprintf(Tmp,LongIntFormat,PInp->LineZ-1);
   sprintf(dest,"%s(%s) ",PInp->SpecName,Tmp);
   return False;
END

        static void MACRO_Restorer(PInputTag PInp)
BEGIN
   PopLocHandle();
   DoLst=PInp->OrigDoLst;
END

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Dies initialisiert eine Makroexpansion */

        static void ExpandMacro(PMacroRec OneMacro)
BEGIN
   int z1;
   StringRecPtr Lauf;
   PInputTag Tag;

   CodeLen=0;

   /* if (OneMacro->Used) WrError(1850);
   else */

    BEGIN
     OneMacro->Used=True;

     /* 1. Tag erzeugen */

     GenerateProcessor(&Tag);
     Tag->Processor=MACRO_Processor;
     Tag->Restorer =MACRO_Restorer;
     Tag->Cleanup  =MACRO_Cleanup;
     Tag->GetPos   =MACRO_GetPos;
     strmaxcpy(Tag->SpecName,OneMacro->Name,255);
     strmaxcpy(Tag->SaveAttr,AttrPart,255);
     Tag->IsMacro  =True;

     /* 2. Parameterzahl anpassen */

     if (ArgCnt<OneMacro->ParamCount)
      for (z1=ArgCnt+1; z1<=OneMacro->ParamCount; z1++) *(ArgStr[z1])='\0';
     ArgCnt=OneMacro->ParamCount;

     /* 3. Parameterliste aufbauen - umgekehrt einfacher */

     for (z1=ArgCnt; z1>=1; z1--)
      BEGIN
       if (NOT CaseSensitive) UpString(ArgStr[z1]);
       AddStringListFirst(&(Tag->Params),ArgStr[z1]);
      END
     Tag->ParCnt=ArgCnt;

     /* 4. Zeilenliste anhaengen */

     Tag->Lines=OneMacro->FirstLine; Tag->IsEmpty=(OneMacro->FirstLine==Nil);
     Lauf=OneMacro->FirstLine;
     while (Lauf!=Nil)
      BEGIN
       Tag->LineCnt++; Lauf=Lauf->Next;
      END
    END

   /* 5. anhaengen */

   if (IfAsm)
    BEGIN
     NextDoLst=(DoLst AND OneMacro->LocMacExp);
     Tag->Next=FirstInputTag; FirstInputTag=Tag;
    END
   else
    BEGIN
     ClearStringList(&(Tag->Params)); free(Tag);
    END
END

/*-------------------------------------------------------------------------*/
/* vorzeitiger Abbruch eines Makros */

        static void ExpandEXITM(void)
BEGIN
   if (ArgCnt!=0) WrError(1110);
   else if (FirstInputTag==Nil) WrError(1805);
   else if (NOT FirstInputTag->IsMacro) WrError(1805);
   else if (IfAsm)
    BEGIN
     FirstInputTag->Cleanup(FirstInputTag);
     RestoreIFs(FirstInputTag->IfLevel);
     FirstInputTag->IsEmpty=True;
    END
END

/*-------------------------------------------------------------------------*/
/*--- IRP (was das bei MASM auch immer heissen mag...)
      Ach ja: Individual Repeat! Danke Bernhard, jetzt hab'
      ich's gerafft! -----------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine liefert bei der Expansion eines IRP-Statements die expan-
  dierten Zeilen */

        Boolean IRP_Processor(PInputTag PInp, char *erg)
BEGIN
   StringRecPtr Lauf;
   int z;
   Boolean Result;

   Result=True;

   Lauf=PInp->Lines; for (z=1; z<=PInp->LineZ-1; z++) Lauf=Lauf->Next;
   strcpy(erg,Lauf->Content);
   Lauf=PInp->Params; for (z=1; z<=PInp->ParZ-1; z++) Lauf=Lauf->Next;
   ExpandLine(Lauf->Content,1,erg); CurrLine=PInp->StartLine+PInp->LineZ;

   if (PInp->LineZ==1)
    BEGIN
     if (NOT PInp->First) PopLocHandle(); PInp->First=False;
     PushLocHandle(GetLocHandle());
    END


   if (++(PInp->LineZ)>PInp->LineCnt)
    BEGIN
     PInp->LineZ=1; 
     if (++(PInp->ParZ)>PInp->ParCnt) Result=False;
    END

   return Result;
END

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Aufraeumroutine IRP/IRPC */

        static void IRP_Cleanup(PInputTag PInp)
BEGIN
   StringRecPtr Lauf;

   /* letzten Parameter sichern, wird evtl. noch fuer GetPos gebraucht!
      ... SaveAttr ist aber frei */
   if (PInp->Processor==IRP_Processor)
    BEGIN
     for (Lauf=PInp->Params; Lauf->Next!=Nil; Lauf=Lauf->Next);
     strmaxcpy(PInp->SaveAttr,Lauf->Content,255);
    END

   ClearStringList(&(PInp->Lines)); 
   ClearStringList(&(PInp->Params));
END

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Posisionsangabe im IRP(C) fuer Fehlermeldungen */

	static Boolean IRP_GetPos(PInputTag PInp, char *dest)
BEGIN
   StringRecPtr Lauf=PInp->Params;
   int z,z1=PInp->ParZ,z2=PInp->LineZ;
   char *IRPType,*IRPVal,tmp[10];

   if (PInp->Processor==IRP_Processor)
    BEGIN
     IRPType="IRP";
     if (*PInp->SaveAttr!='\0') IRPVal=PInp->SaveAttr;
     else
      BEGIN
       for (z=1; z<=z1-1; z++) Lauf=Lauf->Next;
       IRPVal=Lauf->Content;
      END
    END
   else
    BEGIN
     IRPType="IRPC"; 
     sprintf(tmp,"'%c'",PInp->SpecName[z1-1]); IRPVal=tmp;
    END

   if (--z2<=0)
    BEGIN
     z2=PInp->LineCnt; z1--;
    END
   sprintf(dest,"%s:%s/%ld ",IRPType,IRPVal,(long)z2);

   return False;
END

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine sammelt waehrend der Definition eines IRP(C)-Statements die
  Quellzeilen ein */

	static void IRP_OutProcessor(void)
BEGIN
   POutputTag Tmp;
   StringRecPtr Dummy;
   String s;

   /* Schachtelungen mitzaehlen */

   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;

   /* falls noch nicht zuende, weiterzaehlen */

   if (FirstOutputTag->NestLevel>-1)
    BEGIN
     strmaxcpy(s,OneLine,255); KillCtrl(s);
     CompressLine(GetStringListFirst(FirstOutputTag->Params,&Dummy),1,s);
     AddStringListLast(&(FirstOutputTag->Tag->Lines),s);
     FirstOutputTag->Tag->LineCnt++;
    END

   /* alles zusammen? Dann umhaengen */

   if (FirstOutputTag->NestLevel==-1)
    BEGIN
     Tmp=FirstOutputTag; FirstOutputTag=FirstOutputTag->Next;
     Tmp->Tag->IsEmpty=(Tmp->Tag->Lines==Nil);
     if (IfAsm)
      BEGIN
       NextDoLst=DoLst AND LstMacroEx;
       Tmp->Tag->Next=FirstInputTag; FirstInputTag=Tmp->Tag;
      END
     else
      BEGIN
       ClearStringList(&(Tmp->Tag->Lines)); ClearStringList(&(Tmp->Tag->Params));
       free(Tmp->Tag);
      END
     ClearStringList(&(Tmp->Params));
     free(Tmp);
    END
END

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Initialisierung der IRP-Bearbeitung */

        static void ExpandIRP(void)
BEGIN
   String Parameter;
   int z1;
   PInputTag Tag;
   POutputTag Neu;
   Boolean ErrFlag;

   /* 1.Parameter pruefen */

   if (ArgCnt<2)
    BEGIN
     WrError(1110); ErrFlag=True;
    END
   else
    BEGIN
     strmaxcpy(Parameter,ArgStr[1],255);
     if (NOT ChkMacSymbName(ArgStr[1]))
      BEGIN
       WrXError(1020,Parameter); ErrFlag=True;
      END
     else ErrFlag=False;
    END
   if (ErrFlag)
    BEGIN
     AddWaitENDM_Processor();
     return;
    END

   /* 2. Tag erzeugen */

   GenerateProcessor(&Tag);
   Tag->ParCnt=ArgCnt-1;
   Tag->Processor=IRP_Processor;
   Tag->Restorer =MACRO_Restorer;
   Tag->Cleanup  =IRP_Cleanup;
   Tag->GetPos   =IRP_GetPos;
   Tag->ParZ     =1;
   Tag->IsMacro  =True;
   *Tag->SaveAttr='\0';

   /* 3. Parameterliste aufbauen; rueckwaerts einen Tucken schneller */

   for (z1=ArgCnt; z1>=2; z1--)
    BEGIN
     UpString(ArgStr[z1]);
     AddStringListFirst(&(Tag->Params),ArgStr[z1]);
    END

   /* 4. einbetten */

   Neu=(POutputTag) malloc(sizeof(TOutputTag));
   Neu->Next=FirstOutputTag;
   Neu->Processor=IRP_OutProcessor;
   Neu->NestLevel=0;
   Neu->Tag=Tag;
   Neu->Params=Nil; AddStringListFirst(&(Neu->Params),ArgStr[1]);
   FirstOutputTag=Neu;
END

/*--- IRPC: dito für Zeichen eines Strings ---------------------------------*/

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Diese Routine liefert bei der Expansion eines IRPC-Statements die expan-
  dierten Zeilen */

        Boolean IRPC_Processor(PInputTag PInp, char *erg)
BEGIN
   StringRecPtr Lauf;
   int z;
   Boolean Result;
   char tmp[5];

   Result=True;

   Lauf=PInp->Lines; for (z=1; z<=PInp->LineZ-1; z++) Lauf=Lauf->Next;
   strcpy(erg,Lauf->Content);
   *tmp=PInp->SpecName[PInp->ParZ-1]; tmp[1]='\0';
   ExpandLine(tmp,1,erg); CurrLine=PInp->StartLine+PInp->LineZ;

   if (PInp->LineZ==1)
    BEGIN
     if (NOT PInp->First) PopLocHandle(); PInp->First=False;
     PushLocHandle(GetLocHandle());
    END


   if (++(PInp->LineZ)>PInp->LineCnt)
    BEGIN
     PInp->LineZ=1; 
     if (++(PInp->ParZ)>PInp->ParCnt) Result=False;
    END

   return Result;
END

/* - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - */
/* Initialisierung der IRPC-Bearbeitung */

        static void ExpandIRPC(void)
BEGIN
   String Parameter;
   PInputTag Tag;
   POutputTag Neu;
   Boolean ErrFlag;

   /* 1.Parameter pruefen */

   if (ArgCnt<2)
    BEGIN
     WrError(1110); ErrFlag=True;
    END
   else
    BEGIN
     strmaxcpy(Parameter,ArgStr[1],255);
     if (NOT ChkMacSymbName(ArgStr[1]))
      BEGIN
       WrXError(1020,Parameter); ErrFlag=True;
      END
     else ErrFlag=False;
    END
   if (NOT ErrFlag)
    BEGIN
     EvalStringExpression(ArgStr[2],&ErrFlag,Parameter);
     ErrFlag=NOT ErrFlag;
    END
   if (ErrFlag)
    BEGIN
     AddWaitENDM_Processor();
     return;
    END

   /* 2. Tag erzeugen */

   GenerateProcessor(&Tag);
   Tag->ParCnt   =strlen(Parameter);
   Tag->Processor=IRPC_Processor;
   Tag->Restorer =MACRO_Restorer;
   Tag->Cleanup  =IRP_Cleanup;
   Tag->GetPos   =IRP_GetPos;
   Tag->ParZ     =1;
   Tag->IsMacro  =True;
   *Tag->SaveAttr='\0';
   strmaxcpy(Tag->SpecName,Parameter,255);

   /* 4. einbetten */

   Neu=(POutputTag) malloc(sizeof(TOutputTag));
   Neu->Next=FirstOutputTag;
   Neu->Processor=IRP_OutProcessor;
   Neu->NestLevel=0;
   Neu->Tag=Tag;
   Neu->Params=Nil; AddStringListFirst(&(Neu->Params),ArgStr[1]);
   FirstOutputTag=Neu;
END

/*--- Repetition -----------------------------------------------------------*/

        static void REPT_Cleanup(PInputTag PInp)
BEGIN
   ClearStringList(&(PInp->Lines));
END

	static Boolean REPT_GetPos(PInputTag PInp, char *dest)
BEGIN
   int z1=PInp->ParZ,z2=PInp->LineZ;

   if (--z2<=0)
    BEGIN
     z2=PInp->LineCnt; z1--;
    END
   sprintf(dest,"REPT %ld/%ld",(long)z1,(long)z2);
   return False;
END

        Boolean REPT_Processor(PInputTag PInp, char *erg)
BEGIN
   StringRecPtr Lauf;
   int z;
   Boolean Result;

   Result=True;

   Lauf=PInp->Lines; for(z=1; z<=PInp->LineZ-1; z++) Lauf=Lauf->Next;
   strcpy(erg,Lauf->Content); CurrLine=PInp->StartLine+PInp->LineZ;

   if (PInp->LineZ==1)
    BEGIN
     if (NOT PInp->First) PopLocHandle(); PInp->First=False;
     PushLocHandle(GetLocHandle());
    END

    if ((++PInp->LineZ)>PInp->LineCnt)
    BEGIN
     PInp->LineZ=1;
     if ((++PInp->ParZ)>PInp->ParCnt) Result=False;
    END

   return Result;
END

	static void REPT_OutProcessor(void)
BEGIN
   POutputTag Tmp;

   /* Schachtelungen mitzaehlen */

   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;

   /* falls noch nicht zuende, weiterzaehlen */

   if (FirstOutputTag->NestLevel>-1)
    BEGIN
     AddStringListLast(&(FirstOutputTag->Tag->Lines),OneLine);
     FirstOutputTag->Tag->LineCnt++;
    END

   /* alles zusammen? Dann umhaengen */

   if (FirstOutputTag->NestLevel==-1)
    BEGIN
     Tmp=FirstOutputTag; FirstOutputTag=FirstOutputTag->Next;
     Tmp->Tag->IsEmpty=(Tmp->Tag->Lines==Nil);
     if ((IfAsm) AND (Tmp->Tag->ParCnt>0))
      BEGIN
       NextDoLst=(DoLst AND LstMacroEx);
       Tmp->Tag->Next=FirstInputTag; FirstInputTag=Tmp->Tag;
      END
     else
      BEGIN
       ClearStringList(&(Tmp->Tag->Lines));
       free(Tmp->Tag);
      END
     free(Tmp);
    END
END

        static void ExpandREPT(void)
BEGIN
   Boolean ValOK;
   LongInt ReptCount=0;
   PInputTag Tag;
   POutputTag Neu;
   Boolean ErrFlag;

   /* 1.Repetitionszahl ermitteln */

   if (ArgCnt!=1)
    BEGIN
     WrError(1110); ErrFlag=True;
    END
   else
    BEGIN
     FirstPassUnknown=False;
     ReptCount=EvalIntExpression(ArgStr[1],Int32,&ValOK);
     if (FirstPassUnknown) WrError(1820);
     ErrFlag=((NOT ValOK) OR (FirstPassUnknown));
    END
   if (ErrFlag)
    BEGIN
     AddWaitENDM_Processor();
     return;
    END

   /* 2. Tag erzeugen */

   GenerateProcessor(&Tag);
   Tag->ParCnt=ReptCount;
   Tag->Processor=REPT_Processor;
   Tag->Restorer =MACRO_Restorer;
   Tag->Cleanup  =REPT_Cleanup;
   Tag->GetPos   =REPT_GetPos;
   Tag->IsMacro  =True;
   Tag->ParZ=1;

   /* 3. einbetten */

   Neu=(POutputTag) malloc(sizeof(TOutputTag));
   Neu->Processor=REPT_OutProcessor;
   Neu->NestLevel=0;
   Neu->Next=FirstOutputTag;
   Neu->Tag=Tag;
   FirstOutputTag=Neu;
END

/*- bedingte Wiederholung -------------------------------------------------------*/

        static void WHILE_Cleanup(PInputTag PInp)
BEGIN
   ClearStringList(&(PInp->Lines));
END

	static Boolean WHILE_GetPos(PInputTag PInp, char *dest)
BEGIN
   int z1=PInp->ParZ,z2=PInp->LineZ;

   if (--z2<=0)
    BEGIN
     z2=PInp->LineCnt; z1--;
    END
   sprintf(dest, "WHILE %ld/%ld", (long)z1, (long)z2);
   return False;
END

        Boolean WHILE_Processor(PInputTag PInp, char *erg)
BEGIN
   StringRecPtr Lauf;
   int z;
   Boolean OK,Result;

   CurrLine=PInp->StartLine+PInp->LineZ;

   if (PInp->LineZ==1)
    BEGIN
     if (NOT PInp->First) PopLocHandle(); PInp->First=False;
     PushLocHandle(GetLocHandle());
    END
   else OK=True;

   Lauf=PInp->Lines; for (z=1; z<=PInp->LineZ-1; z++) Lauf=Lauf->Next;
   strcpy(erg,Lauf->Content);

   if ((++PInp->LineZ)>PInp->LineCnt)
    BEGIN
     PInp->LineZ=1; PInp->ParZ++;
     z=EvalIntExpression(PInp->SpecName,Int32,&OK);
     OK=(OK AND (z!=0));
     Result=OK;
    END
   else Result=True;

   return Result;
END

	static void WHILE_OutProcessor(void)
BEGIN
   POutputTag Tmp;
   Boolean OK;
   LongInt Erg;

   /* Schachtelungen mitzaehlen */

   if (MacroStart()) FirstOutputTag->NestLevel++;
   else if (MacroEnd()) FirstOutputTag->NestLevel--;

   /* falls noch nicht zuende, weiterzaehlen */

   if (FirstOutputTag->NestLevel>-1)
    BEGIN
     AddStringListLast(&(FirstOutputTag->Tag->Lines),OneLine);
     FirstOutputTag->Tag->LineCnt++;
    END

   /* alles zusammen? Dann umhaengen */

   if (FirstOutputTag->NestLevel==-1)
    BEGIN
     Tmp=FirstOutputTag; FirstOutputTag=FirstOutputTag->Next;
     Tmp->Tag->IsEmpty=(Tmp->Tag->Lines==Nil);
     FirstPassUnknown=False;
     Erg=EvalIntExpression(Tmp->Tag->SpecName,Int32,&OK);
     if (FirstPassUnknown) WrError(1820);
     OK=(OK AND (NOT FirstPassUnknown) AND (Erg!=0));
     if ((IfAsm) AND (OK))
      BEGIN
       NextDoLst=(DoLst AND LstMacroEx);
       Tmp->Tag->Next=FirstInputTag; FirstInputTag=Tmp->Tag;
      END
     else
      BEGIN
       ClearStringList(&(Tmp->Tag->Lines));
       free(Tmp->Tag);
      END
     free(Tmp);
    END
END

        static void ExpandWHILE(void)
BEGIN
   PInputTag Tag;
   POutputTag Neu;
   Boolean ErrFlag;

   /* 1.Bedingung ermitteln */

   if (ArgCnt!=1)
    BEGIN
     WrError(1110); ErrFlag=True;
    END
   else ErrFlag=False;
   if (ErrFlag)
    BEGIN
     AddWaitENDM_Processor();
     return;
    END

   /* 2. Tag erzeugen */

   GenerateProcessor(&Tag);
   Tag->Processor=WHILE_Processor;
   Tag->Restorer =MACRO_Restorer;
   Tag->Cleanup  =WHILE_Cleanup;
   Tag->GetPos   =WHILE_GetPos;
   Tag->IsMacro  =True;
   Tag->ParZ=1;
   strmaxcpy(Tag->SpecName,ArgStr[1],255);

   /* 3. einbetten */

   Neu=(POutputTag) malloc(sizeof(TOutputTag));
   Neu->Processor=WHILE_OutProcessor;
   Neu->NestLevel=0;
   Neu->Next=FirstOutputTag;
   Neu->Tag=Tag;
   FirstOutputTag=Neu;
END

/*--------------------------------------------------------------------------*/
/* Einziehen von Include-Files */

	static void INCLUDE_Cleanup(PInputTag PInp)
BEGIN
   String Tmp;

   fclose(PInp->Datei);
   free(PInp->Buffer);
   LineSum+=MomLineCounter;
   if ((*LstName!='\0') AND (NOT QuietMode))
    BEGIN
     sprintf(Tmp,LongIntFormat,CurrLine);
     printf("%s(%s)",NamePart(CurrFileName),Tmp);
     printf("%s\n",ClrEol); fflush(stdout);
    END
   if (MakeIncludeList) PopInclude();
END

	static Boolean INCLUDE_GetPos(PInputTag PInp, char *dest)
BEGIN
   String Tmp;

   sprintf(Tmp,LongIntFormat,CurrLine);
   sprintf(dest,"%s(%s) ",NamePart(CurrFileName),Tmp);
   return True;
END

        Boolean INCLUDE_Processor(PInputTag PInp, char *Erg)
BEGIN
   Boolean Result;

   Result=True;

   if (feof(PInp->Datei)) *Erg='\0';
   else
    BEGIN
     ReadLn(PInp->Datei,Erg);
     /**ChkIO(10003);**/
    END
   CurrLine=(++MomLineCounter);
   if (feof(PInp->Datei)) Result=False;

   return Result;
END

        static void INCLUDE_Restorer(PInputTag PInp)
BEGIN
   MomLineCounter=PInp->StartLine;
   strmaxcpy(CurrFileName,PInp->SpecName,255);
   IncDepth--;
END

        static void ExpandINCLUDE(Boolean SearchPath)
BEGIN
   PInputTag Tag;

   if (NOT IfAsm) return;

   if (ArgCnt!=1)
    BEGIN
     WrError(1110); return;
    END

   strmaxcpy(ArgPart,ArgStr[1],255);
   if (*ArgPart=='"') strcpy(ArgPart,ArgPart+1);
   if (ArgPart[strlen(ArgPart)-1]=='"') ArgPart[strlen(ArgPart)-1]='\0';
   AddSuffix(ArgPart,IncSuffix); strmaxcpy(ArgStr[1],ArgPart,255);
   if (SearchPath)
    BEGIN
     strmaxcpy(ArgPart,FExpand(FSearch(ArgPart,IncludeList)),255);
     if (ArgPart[strlen(ArgPart)-1]=='/') strmaxcat(ArgPart,ArgStr[1],255);
    END

   /* Tag erzeugen */

   GenerateProcessor(&Tag);
   Tag->Processor=INCLUDE_Processor;
   Tag->Restorer =INCLUDE_Restorer;
   Tag->Cleanup  =INCLUDE_Cleanup;
   Tag->GetPos   =INCLUDE_GetPos;
   Tag->Buffer   =(void *) malloc(BufferArraySize);

   /* Sicherung alter Daten */

   Tag->StartLine=MomLineCounter;
   strmaxcpy(Tag->SpecName,CurrFileName,255);

   /* Datei oeffnen */

   Tag->Datei=fopen(ArgPart,"r");
   if (Tag->Datei==Nil) ChkIO(10001);
   setvbuf(Tag->Datei,Tag->Buffer,_IOFBF,BufferArraySize);

   /* neu besetzen */

   strmaxcpy(CurrFileName,ArgPart,255); MomLineCounter=0;
   NextIncDepth++; AddFile(ArgPart);
   PushInclude(ArgPart);

   /* einhaengen */

   Tag->Next=FirstInputTag; FirstInputTag=Tag;
END

/*=========================================================================*/
/* Einlieferung von Zeilen */

        static void GetNextLine(char *Line)
BEGIN
   PInputTag HTag;

   InMacroFlag=False;

   while ((FirstInputTag!=Nil) AND (FirstInputTag->IsEmpty))
    BEGIN
     FirstInputTag->Restorer(FirstInputTag);
     HTag=FirstInputTag; FirstInputTag=HTag->Next;
     free(HTag);
    END

   if (FirstInputTag==Nil)
    BEGIN
     *Line='\0'; return;
    END

   if (NOT FirstInputTag->Processor(FirstInputTag,Line))
    BEGIN
     FirstInputTag->Cleanup(FirstInputTag);
     FirstInputTag->IsEmpty=True;
    END

   MacLineSum++;
END

	char *GetErrorPos(void)
BEGIN
   String ActPos;
   PInputTag RunTag;
   char *ErgPos=strdup(""),*tmppos;
   Boolean Last;
   
   for (RunTag=FirstInputTag; RunTag!=Nil; RunTag=RunTag->Next)
    BEGIN
     Last=RunTag->GetPos(RunTag,ActPos);
     tmppos=(char *) malloc(strlen(ErgPos)+strlen(ActPos)+1);
     strcpy(tmppos,ActPos); strcat(tmppos,ErgPos);
     free(ErgPos); ErgPos=tmppos;
     if (Last) break;
    END

   return ErgPos;
END

        static Boolean InputEnd(void)
BEGIN
   PInputTag Lauf;

   Lauf=FirstInputTag;
   while (Lauf!=Nil)
    BEGIN
     if (NOT Lauf->IsEmpty) return False;
     Lauf=Lauf->Next;
    END

   return True;
END


/*=== Eine Quelldatei ( Haupt-oder Includedatei ) bearbeiten ===============*/

/*--- aus der zerlegten Zeile Code erzeugen --------------------------------*/

        Boolean HasLabel(void)
BEGIN
   if (*LabPart=='\0') return False;
   if (IsDef()) return False;

   switch (*OpPart)
    BEGIN
     case '=':
      return (NOT Memo("="));
     case ':':
      return (NOT Memo(":"));
     case 'M':
      return (NOT Memo("MACRO"));
     case 'F':
      return (NOT Memo("FUNCTION"));
     case 'L':
      return (NOT Memo("LABEL"));
     case 'S':
      return ((NOT Memo("SET")) OR (SetIsOccupied)) AND (NOT Memo("STRUCT"));
     case 'E':
      return ((NOT Memo("EVAL")) OR (NOT SetIsOccupied)) AND (NOT Memo("EQU")) AND (NOT Memo("ENDSTRUCT"));
     default: 
      return True;
    END   
END

        static void Produce_Code(void)
BEGIN
   Byte z;
   PMacroRec OneMacro;
   Boolean SearchMacros,Found;
   String tmp,tmp2;
   PStructure ZStruct;

   ActListGran=ListGran();

   /* Makrosuche unterdruecken ? */

   if (*OpPart=='!')
    BEGIN
     SearchMacros=False; strcpy(OpPart,OpPart+1);
    END
   else
    BEGIN
     SearchMacros=True; ExpandSymbol(OpPart);
    END
   strcpy(LOpPart,OpPart); NLS_UpString(OpPart);

   /* Prozessor eingehaengt ? */

   if (FirstOutputTag!=Nil)
    BEGIN
     FirstOutputTag->Processor(); return;
    END

   /* ansonsten Code erzeugen */

   /* evtl. voranstehendes Label ablegen */

   if (IfAsm)
    BEGIN
     if (HasLabel()) 
      if (StructureStack!=Nil) 
       BEGIN
        strmaxcpy(tmp,LabPart,255);
        for (ZStruct=StructureStack; ZStruct!=Nil; ZStruct=ZStruct->Next)
         if (ZStruct->DoExt)
          BEGIN
           sprintf(tmp2,"%s_",ZStruct->Name);
           strmaxprep(tmp,tmp2,255);
          END
        EnterIntSymbol(tmp,EProgCounter(),SegNone,False);
       END
      else EnterIntSymbol(LabPart,EProgCounter(),ActPC,False);
    END

   Found=False;
   switch (*OpPart)
    BEGIN
     case 'I':
      /* Makroliste ? */
      if ((Found=Memo("IRP"))) ExpandIRP();
      else if ((Found=Memo("IRPC"))) ExpandIRPC();
      break;
     case 'R':
      /* Repetition ? */
      if ((Found=Memo("REPT"))) ExpandREPT(); break;
     case 'W':
      /* bedingte Repetition ? */
      if ((Found=Memo("WHILE"))) ExpandWHILE(); break;
    END

   /* bedingte Assemblierung ? */

   if (NOT Found) Found=CodeIFs();

   if (NOT Found)
    switch (*OpPart)
     BEGIN
      case 'M':
       /* Makrodefinition ? */
       if ((Found=Memo(("MACRO")))) ReadMacro(); break;
      case 'E':
       /* Abbruch Makroexpansion ? */
       if ((Found=Memo(("EXITM")))) ExpandEXITM(); break;
      case 'I':
       /* Includefile? */
       if ((Found=Memo(("INCLUDE"))))
        BEGIN
         ExpandINCLUDE(True);
         MasterFile=False;
        END
       break;
     END

   if (Found); 

   /* Makroaufruf ? */

   else if ((SearchMacros) AND (FoundMacro(&OneMacro)))
    BEGIN
     if (IfAsm) ExpandMacro(OneMacro);
     if (IfAsm) strmaxcpy(ListLine,"(MACRO)",255);
    END

   else
    BEGIN
     StopfZahl=0; CodeLen=0; DontPrint=False;

     if (IfAsm)
      BEGIN
       if (NOT CodeGlobalPseudo()) MakeCode(); 
       if ((MacProOutput) AND ((*OpPart!='\0') OR (*LabPart!='\0') OR (*CommPart!='\0')))
        BEGIN
         errno=0; fprintf(MacProFile,"%s\n",OneLine); ChkIO(10002);
        END
      END

     for (z=0; z<StopfZahl; z++)
      BEGIN
       switch (ActListGran)
        BEGIN
         case 4:DAsmCode[CodeLen>>2]=NOPCode; break;
         case 2:WAsmCode[CodeLen>>1]=NOPCode; break;
         case 1:BAsmCode[CodeLen   ]=NOPCode; break;
        END
       CodeLen+=ActListGran/Granularity();
      END

     if ((ActPC!=StructSeg) AND (NOT ChkPC(PCs[ActPC]+CodeLen-1)) AND (CodeLen!=0))
      WrError(1925);
     else
      BEGIN
       if ((NOT DontPrint) AND (ActPC!=StructSeg) AND (CodeLen>0)) BookKeeping();
       if (ActPC==StructSeg)
        BEGIN
         if ((CodeLen!=0) AND (NOT DontPrint)) WrError(1940);
        END
       else if (CodeOutput)
        BEGIN
         if (DontPrint) NewRecord(PCs[ActPC]+CodeLen);
         else WriteBytes();
        END
       PCs[ActPC]+=CodeLen;
      END
    END

   /* dies ueberprueft implizit, ob von der letzten Eval...-Operation noch
      externe Referenzen liegengeblieben sind. */

   SetRelocs(Nil);
END

/*--- Zeile in Listing zerteilen -------------------------------------------*/

        static void SplitLine(void)
BEGIN
   jmp_buf Retry;
   String h;
   char *i,*k,*p,*div,*run;
   int l;
   Boolean lpos;

   Retracted=False;

   /* Kommentar loeschen */

   strmaxcpy(h,OneLine,255); i=QuotPos(h,';');
   if (i!=Nil)
    BEGIN   
     strcpy(CommPart,i+1); 
     *i='\0';
    END
   else *CommPart='\0'; 
   
   /* alles in Grossbuchstaben wandeln, Praeprozessor laufen lassen */

   ExpandDefines(h);

   /* Label abspalten */

   if ((*h!='\0') AND (NOT isspace((unsigned char)*h)))
    BEGIN
     for (i=h; *i!='\0'; i++)
      if ((isspace(((unsigned char)*i)&0xff)) OR (*i==':')) break;
     if (*i=='\0')
      BEGIN
       strcpy(LabPart,h); *h='\0';
      END
     else
      BEGIN
       *i='\0'; strcpy(LabPart,h); strcpy(h,i+1);
      END
     if (LabPart[l=(strlen(LabPart)-1)]==':') LabPart[l]='\0';
    END
   else *LabPart='\0';

   /* Opcode & Argument trennen */
   setjmp(Retry);
   KillPrefBlanks(h);
   i=FirstBlank(h);
   SplitString(h,OpPart,ArgPart,i);

   /* Falls noch kein Label da war, kann es auch ein Label sein */

   i=strchr(OpPart,':');
   if ((*LabPart=='\0') AND (i!=Nil) AND (i[1]=='\0'))
    BEGIN
     *i='\0'; strcpy(LabPart,OpPart); strcpy(OpPart,i+1);
     if (*OpPart=='\0')
      BEGIN
       strcpy(h,ArgPart);
       longjmp(Retry,1);
      END
    END

   /* Attribut abspalten */

   if (HasAttrs)
    BEGIN
     k=Nil; AttrSplit=' ';
     for (run=AttrChars; *run!='\0'; run++)
      BEGIN
       p=strchr(OpPart,*run); 
       if (p!=Nil) if ((k==Nil) OR (p<k)) k=p;
      END
     if (k!=Nil)
      BEGIN
       AttrSplit=(*k);
       strmaxcpy(AttrPart,k+1,255); *k='\0';
       if ((*OpPart=='\0') AND (*AttrPart!='\0'))
        BEGIN
         strmaxcpy(OpPart,AttrPart,255); *AttrPart='\0';
        END 
      END
     else *AttrPart='\0';
    END
   else *AttrPart='\0';

   KillPostBlanks(ArgPart);

   /* Argumente zerteilen: Da alles aus einem String kommt und die Teile alle auch
      so lang sind, koennen wir uns Laengenabfragen sparen */
   ArgCnt=0; strcpy(h,ArgPart); run=h;
   if (*run!='\0')
    do
     BEGIN
      while ((*run!='\0') AND (isspace((unsigned char)*run))) run++;
      i=Nil;
      for (div=DivideChars; *div!='\0'; div++)
       BEGIN
        p=QuotPos(run,*div);
        if (p!=Nil) if ((i==Nil) OR (p<i)) i=p;
       END
      lpos=((i!=Nil) AND (i[1]=='\0'));
      if (i!=Nil) *i='\0';
      strcpy(ArgStr[++ArgCnt],run);
      if ((lpos) AND (ArgCnt!=ParMax)) *ArgStr[++ArgCnt]='\0';
      KillPostBlanks(ArgStr[ArgCnt]);
      run=(i==Nil) ? i : i+1;
     END
    while ((run!=Nil) AND (ArgCnt!=ParMax) AND (NOT lpos));
    
   if ((run!=Nil) AND (*run!='\0')) WrError(1140);
  
   Produce_Code();
END

/**
CONST
   LineBuffer:String='';
   InComment:Boolean=FALSE;

	static void C_SplitLine(void)
BEGIN
   p,p2:int;
   SaveLine,h:String;

   { alten Inhalt sichern }

   SaveLine:=OneLine; h:=OneLine;

   { Falls in Kommentar, nach schliessender Klammer suchen und den Teil bis
     dahin verwerfen; falls keine Klammer drin ist, die ganze Zeile weg-
     schmeissen; da wir dann OneLine bisher noch nicht veraendert hatten,
     stoert der Abbruch ohne Wiederherstellung von Oneline nicht. }

   IF InComment THEN
    BEGIN
     p:=Pos('}',h);
     IF p>Length(h) THEN Exit
     ELSE
      BEGIN
       Delete(h,1,p); InComment:=False;
      END;
    END;

   { in der Zeile befindliche Teile loeschen; falls am Ende keine
     schliessende Klammer kommt, muessen wir das Kommentarflag setzen. }

   REPEAT
    p:=QuotPos(h,'{');
    IF p>Length(h) THEN p:=0
    ELSE
     BEGIN
      p2:=QuotPos(h,'}');
      IF (p2>p) AND (Length(h)>=p2) THEN Delete(h,p,p2-p+1)
      ELSE
       BEGIN
        Byte(h[0]):=Pred(p);
        InComment:=True;
        p:=0;
       END;
     END;
   UNTIL p=0;

   { alten Inhalt zurueckkopieren }

   OneLine:=SaveLine;
END;**/

/*------------------------------------------------------------------------*/

        static void ProcessFile(String FileName)
BEGIN
   long NxtTime,ListTime;
   String Num;
   char *Name, *Run;
   
   dbgentry("ProcessFile");

   sprintf(OneLine," INCLUDE \"%s\"",FileName); MasterFile=False;
   NextIncDepth=IncDepth;
   SplitLine();
   IncDepth=NextIncDepth;
  
   ListTime=GTime();

   while ((NOT InputEnd()) AND (NOT ENDOccured))
    BEGIN
     /* Zeile lesen */
     
     GetNextLine(OneLine);
     
     /* Ergebnisfelder vorinitialisieren */

     DontPrint=False; CodeLen=0; *ListLine='\0';

     NextDoLst=DoLst;
     NextIncDepth=IncDepth;

     for (Run=OneLine; *Run!='\0'; Run++)
      if (NOT isspace(((unsigned int)*Run)&0xff)) break;
     if (*Run=='#') Preprocess();
     else SplitLine();
     
     MakeList();
     DoLst=NextDoLst;
     IncDepth=NextIncDepth;

     /* Zeilenzaehler */

     if (NOT QuietMode)
      BEGIN
       NxtTime=GTime();
       if (((NOT ListToStdout) OR ((ListMask&1)==0)) AND (DTime(ListTime,NxtTime)>50))
        BEGIN
         sprintf(Num,LongIntFormat,MomLineCounter); Name=NamePart(CurrFileName);
         printf("%s(%s)%s",Name,Num,ClrEol);
         /*for (z=0; z<strlen(Name)+strlen(Num)+2; z++) putchar('\b');*/
         putchar('\r'); fflush(stdout);
         ListTime=NxtTime;
        END
      END

     /* bei Ende Makroprozessor ausraeumen
       OK - das ist eine Hauruckmethode... */

     if (ENDOccured)
      while (FirstInputTag!=Nil) GetNextLine(OneLine);
    END

   while (FirstInputTag!=Nil) GetNextLine(OneLine);

   /* irgendeine Makrodefinition nicht abgeschlossen ? */

   if (FirstOutputTag!=Nil) WrError(1800);
   
   dbgexit("ProcessFile");
END

/****************************************************************************/

        static char *TWrite_Plur(int n)
BEGIN
   return (n!=1) ? getmessage(Num_ListPlurName) : "";
END

        static void TWrite_RWrite(char *dest, Double r, Byte Stellen)
BEGIN
   String s;

   sprintf(s,"%20.*f",Stellen,r);
   while (*s==' ') strcpy(s,s+1); strcat(dest,s);
END

        static void TWrite(Double DTime, char *dest)
BEGIN
   int h;
   String s;

   *dest='\0';
   h=(int) floor(DTime/3600.0);
   if (h>0)
    BEGIN
     sprintf(s,"%d",h); 
     strcat(dest,s);
     strcat(dest,getmessage(Num_ListHourName));
     strcat(dest,TWrite_Plur(h));
     strcat(dest,", ");
     DTime-=3600.0*h;
    END
   h=(int) floor(DTime/60.0);
   if (h>0)
    BEGIN
     sprintf(s,"%d",h); 
     strcat(dest,s);
     strcat(dest,getmessage(Num_ListMinuName));
     strcat(dest,TWrite_Plur(h));
     strcat(dest,", ");
     DTime-=60.0*h;
    END
   TWrite_RWrite(dest,DTime,2); strcat(dest,getmessage(Num_ListSecoName));
   if (DTime!=1) strcat(dest,getmessage(Num_ListPlurName));
END

/*--------------------------------------------------------------------------*/

        static void AssembleFile_InitPass(void)
BEGIN
   static char DateS[31],TimeS[31];
   int z;
   String ArchVal;
   
   dbgentry("AssembleFile_InitPass");

   FirstInputTag=Nil; FirstOutputTag=Nil;

   MomLineCounter=0; MomLocHandle=(-1); LocHandleCnt=0;

   SectionStack=Nil;
   FirstIfSave=Nil;
   FirstSaveState=Nil;
   StructureStack=Nil;

   InitPassProc();
     
   ActPC=SegCode; PCs[ActPC]=0; ENDOccured=False;
   ErrorCount=0; WarnCount=0; LineSum=0; MacLineSum=0;
   for (z=1; z<=StructSeg; z++)
    BEGIN
     PCsUsed[z]=(z==SegCode);
     Phases[z]=0;
     InitChunk(&SegChunks[z]);
    END
   
   TransTables=CurrTransTable=(PTransTable) malloc(sizeof(TTransTable));
   CurrTransTable->Next=Nil;
   CurrTransTable->Name=strdup("STANDARD");
   CurrTransTable->Table=(unsigned char *) malloc(256*sizeof(char));
   for (z=0; z<256; z++) CurrTransTable->Table[z]=z;

   strmaxcpy(CurrFileName,"INTERNAL",255); 
   AddFile(CurrFileName); CurrLine=0;
   
   IncDepth=(-1);
   DoLst=True;

   /* Pseudovariablen initialisieren */
   
   ResetSymbolDefines(); ResetMacroDefines();
   EnterIntSymbol(FlagTrueName,1,0,True);
   EnterIntSymbol(FlagFalseName,0,0,True);
   EnterFloatSymbol(PiName,4.0*atan(1.0),True);
   EnterIntSymbol(VerName,VerNo,0,True);
   sprintf(ArchVal,"%s-%s",ARCHPRNAME,ARCHSYSNAME);
   EnterStringSymbol(ArchName,ArchVal,True);
#ifdef HAS64
   EnterIntSymbol(Has64Name,1,0,True);
#else
   EnterIntSymbol(Has64Name,0,0,True);
#endif   
   EnterIntSymbol(CaseSensName,Ord(CaseSensitive),0,True);
   if (PassNo==0)
    BEGIN
     NLS_CurrDateString(DateS);
     NLS_CurrTimeString(False,TimeS);
    END
   EnterStringSymbol(DateName,DateS,True);
   EnterStringSymbol(TimeName,TimeS,True);

   if (*DefCPU == '\0')
    SetCPU(0, True);
   else
    if (NOT SetNCPU(DefCPU, True)) SetCPU(0, True);

   SetFlag(&SupAllowed,SupAllowedName,False);
   SetFlag(&FPUAvail,FPUAvailName,False);
   SetFlag(&DoPadding,DoPaddingName,True);
   SetFlag(&Maximum,MaximumName,False);
   SetFlag(&DoBranchExt,BranchExtName,False);
   EnterIntSymbol(ListOnName,ListOn=1,0,True);
   SetFlag(&LstMacroEx,LstMacroExName,True);
   SetFlag(&RelaxedMode,RelaxedName,False);
   CopyDefSymbols();
   
   ResetPageCounter();
   
   StartAdrPresent=False;
   
   Repass=False; PassNo++;

   dbgexit("AssembleFile_InitPass");
END

        static void AssembleFile_ExitPass(void)
BEGIN
   SwitchFrom();
   ClearLocStack();
   ClearStacks();
   TossRegDefs(-1);
   if (FirstIfSave!=Nil) WrError(1470);
   if (FirstSaveState!=Nil) WrError(1460);
   if (SectionStack!=Nil) WrError(1485);
   if (StructureStack!=Nil) WrXError(1551,StructureStack->Name);
END

        static void AssembleFile(char *Name)
BEGIN
   String s,Tmp;

   dbgentry("AssembleFile");

   strmaxcpy(SourceFile,Name,255);
   if (MakeDebug) fprintf(Debug,"File %s\n",SourceFile);

   /* Untermodule initialisieren */

   AsmDefInit(); AsmParsInit(); AsmIFInit(); InitFileList(); ResetStack();

   /* Kommandozeilenoptionen verarbeiten */

   strmaxcpy(OutName,GetFromOutList(),255);
   if (OutName[0]=='\0')
    BEGIN
     strmaxcpy(OutName,SourceFile,255); KillSuffix(OutName);
     AddSuffix(OutName,PrgSuffix);
    END

   if (*ErrorPath=='\0')
    BEGIN
     strmaxcpy(ErrorName,SourceFile,255);
     KillSuffix(ErrorName);
     AddSuffix(ErrorName,LogSuffix);
     unlink(ErrorName);
    END

   switch (ListMode)
    BEGIN
     case 0: strmaxcpy(LstName,NULLDEV,255); break;
     case 1: strmaxcpy(LstName,"!1",255); break;
     case 2:
      strmaxcpy(LstName,SourceFile,255);
      KillSuffix(LstName);
      AddSuffix(LstName,LstSuffix);
      break;
    END
   ListToStdout=(strcmp(LstName,"!1")==0);
   ListToNull=(strcmp(LstName,NULLDEV)==0);

   if (ShareMode!=0)
    BEGIN
     strmaxcpy(ShareName,GetFromShareOutList(),255);
     if (*ShareName == '\0')
      BEGIN
       strmaxcpy(ShareName,SourceFile,255);
       KillSuffix(ShareName);
       switch (ShareMode)
        BEGIN
         case 1: AddSuffix(ShareName,".inc"); break;
         case 2: AddSuffix(ShareName,".h"); break;
         case 3: AddSuffix(ShareName,IncSuffix); break;
        END
      END
    END

   if (MacProOutput)
    BEGIN
     strmaxcpy(MacProName,SourceFile,255); KillSuffix(MacProName);
     AddSuffix(MacProName,PreSuffix);
    END

   if (MacroOutput)
    BEGIN
     strmaxcpy(MacroName,SourceFile,255); KillSuffix(MacroName);
     AddSuffix(MacroName,MacSuffix);
    END

   ClearIncludeList();

   if (DebugMode!=DebugNone) InitLineInfo();

   /* Variablen initialisieren */

   StartTime=GTime();

   PassNo=0; MomLineCounter=0;

   /* Listdatei eroeffnen */

   if (NOT QuietMode) printf("%s%s\n",getmessage(Num_InfoMessAssembling),SourceFile);
   
   do
    BEGIN
     /* Durchlauf initialisieren */

     AssembleFile_InitPass(); AsmSubInit(); 
     if (NOT QuietMode)
      BEGIN
       sprintf(Tmp,IntegerFormat,PassNo);
       printf("%s%s%s\n",getmessage(Num_InfoMessPass),Tmp,ClrEol);
      END

     /* Dateien oeffnen */
     
     if (CodeOutput) OpenFile();
     
     if (ShareMode!=0)
      BEGIN
       ShareFile=fopen(ShareName,"w");
       if (ShareFile==Nil) ChkIO(10001);
       errno=0;
       switch (ShareMode)
        BEGIN
         case 1:fprintf(ShareFile,"(* %s-Includefile f%sr CONST-Sektion *)\n",SourceFile,CH_ue); break;
         case 2:fprintf(ShareFile,"/* %s-Includefile f%sr C-Programm */\n",SourceFile,CH_ue); break;
         case 3:fprintf(ShareFile,"; %s-Includefile f%sr Assembler-Programm\n",SourceFile,CH_ue); break;
        END
       ChkIO(10002);
      END

     if (MacProOutput)
      BEGIN
       MacProFile=fopen(MacProName,"w");
       if (MacProFile==Nil) ChkIO(10001);
      END

     if ((MacroOutput) AND (PassNo==1))
      BEGIN
       MacroFile=fopen(MacroName,"w");
       if (MacroFile==Nil) ChkIO(10001);
      END

     /* Listdatei oeffnen */

     RewriteStandard(&LstFile,LstName);
     if (LstFile==Nil) ChkIO(10001);
     errno=0; fprintf(LstFile,"%s",PrtInitString); ChkIO(10002);
     if ((ListMask&1)!=0) NewPage(0,False);

     /* assemblieren */

     ProcessFile(SourceFile);
     AssembleFile_ExitPass();

     /* Dateien schliessen */

     if (CodeOutput) CloseFile();

     if (ShareMode!=0)
      BEGIN
       errno=0;
       switch (ShareMode)
        BEGIN
         case 1: fprintf(ShareFile,"(* Ende Includefile f%sr CONST-Sektion *)\n",CH_ue); break;
         case 2: fprintf(ShareFile,"/* Ende Includefile f%sr C-Programm */\n",CH_ue); break;
         case 3: fprintf(ShareFile,"; Ende Includefile f%sr Assembler-Programm\n",CH_ue); break;
        END
       ChkIO(10002);
       fclose(ShareFile);
      END

     if (MacProOutput) fclose(MacProFile);
     if ((MacroOutput) AND (PassNo==1)) fclose(MacroFile);

     /* evtl. fuer naechsten Durchlauf aufraeumen */

     if ((ErrorCount==0) AND (Repass))
      BEGIN
       fclose(LstFile);
       if (CodeOutput) unlink(OutName);
       CleanupRegDefs();
       ClearCodepages();
       if (MakeUseList) ClearUseList();
       if (MakeCrossList) ClearCrossList();
       ClearDefineList();
       if (DebugMode!=DebugNone) ClearLineInfo();
       ClearIncludeList();
       if (DebugMode!=DebugNone) ResetAddressRanges();
      END
    END
   while ((ErrorCount==0) AND (Repass));

   /* bei Fehlern loeschen */

   if (ErrorCount!=0)
    BEGIN
     if (CodeOutput) unlink(OutName); 
     if (MacProOutput) unlink(MacProName);
     if ((MacroOutput) AND (PassNo==1))  unlink(MacroName);
     if (ShareMode!=0) unlink(ShareName);
     GlobErrFlag=True;
    END

   /* Debug-Ausgabe muss VOR die Symbollistenausgabe, weil letztere die
      Symbolliste loescht */

   if (DebugMode!=DebugNone)
    BEGIN
     if (ErrorCount==0) DumpDebugInfo();
     ClearLineInfo();
    END

   /* Listdatei abschliessen */

   if  (strcmp(LstName,NULLDEV)!=0)
    BEGIN
     if ((ListMask&2)!=0) PrintSymbolList();

     if ((ListMask&64)!=0) PrintRegDefs();

     if ((ListMask&4)!=0) PrintMacroList();

     if ((ListMask&8)!=0) PrintFunctionList();

     if ((ListMask&32)!=0) PrintDefineList();

     if ((ListMask&128)!=0) PrintCodepages();

     if (MakeUseList)
      BEGIN
       NewPage(ChapDepth,True);
       PrintUseList();
      END

     if (MakeCrossList)
      BEGIN
       NewPage(ChapDepth,True);
       PrintCrossList();
      END

     if (MakeSectionList) PrintSectionList();

     if (MakeIncludeList) PrintIncludeList();

     errno=0; fprintf(LstFile,"%s",PrtExitString); ChkIO(10002);
    END

   if (MakeUseList) ClearUseList();

   if (MakeCrossList) ClearCrossList();

   ClearSectionList();

   ClearIncludeList();

   if ((*ErrorPath=='\0') AND (IsErrorOpen))
    BEGIN
     fclose(ErrorFile); IsErrorOpen=False;
    END

   ClearUpProc();

   /* Statistik ausgeben */

   StopTime=GTime();
   TWrite(DTime(StartTime,StopTime)/100.0,s);
   if (NOT QuietMode) printf("\n%s%s%s\n\n",s,getmessage(Num_InfoMessAssTime),ClrEol);
   if (ListMode==2)
    BEGIN
     WrLstLine(""); 
     strmaxcat(s,getmessage(Num_InfoMessAssTime),255); WrLstLine(s); 
     WrLstLine("");
    END

   strcpy(s,Dec32BlankString(LineSum,7));
   strmaxcat(s,getmessage((LineSum==1)?Num_InfoMessAssLine:Num_InfoMessAssLines),255);
   if (NOT QuietMode) printf("%s%s\n",s,ClrEol);
   if (ListMode==2) WrLstLine(s);

   if (LineSum!=MacLineSum)
    BEGIN
     strcpy(s,Dec32BlankString(MacLineSum,7));
     strmaxcat(s,getmessage((MacLineSum==1)?Num_InfoMessMacAssLine:Num_InfoMessMacAssLines),255);
     if (NOT QuietMode) printf("%s%s\n",s,ClrEol);
     if (ListMode==2) WrLstLine(s);
    END

   strcpy(s,Dec32BlankString(PassNo,7));
   strmaxcat(s,getmessage((PassNo==1)?Num_InfoMessPassCnt:Num_InfoMessPPassCnt),255);
   if (NOT QuietMode) printf("%s%s\n",s,ClrEol);
   if (ListMode==2) WrLstLine(s);

   if ((ErrorCount>0) AND (Repass) AND (ListMode!=0))
    WrLstLine(getmessage(Num_InfoMessNoPass));

#ifdef __TURBOC__
   sprintf(s,"%s%s",Dec32BlankString(coreleft()>>10,7),getmessage(Num_InfoMessRemainMem));
   if (NOT QuietMode) printf("%s%s\n",s,ClrEol);
   if (ListMode==2) WrLstLine(s);

   sprintf(s,"%s%s",Dec32BlankString(StackRes(),7),getmessage(Num_InfoMessRemainStack));
   if (NOT QuietMode) printf("%s%s\n",s,ClrEol);
   if (ListMode==2) WrLstLine(s);
#endif

   sprintf(s,"%s%s",Dec32BlankString(ErrorCount,7),getmessage(Num_InfoMessErrCnt));
   if (ErrorCount!=1) strmaxcat(s,getmessage(Num_InfoMessErrPCnt),255);
   if (NOT QuietMode) printf("%s%s\n",s,ClrEol);
   if (ListMode==2) WrLstLine(s);

   sprintf(s,"%s%s",Dec32BlankString(WarnCount,7),getmessage(Num_InfoMessWarnCnt));
   if (WarnCount!=1) strmaxcat(s,getmessage(Num_InfoMessWarnPCnt),255);
   if (NOT QuietMode) printf("%s%s\n",s,ClrEol);
   if (ListMode==2) WrLstLine(s);

   fclose(LstFile);

   /* verstecktes */

   if (MakeDebug) PrintSymbolDepth();

   /* Speicher freigeben */

   ClearSymbolList();
   ClearRegDefs();
   ClearCodepages();
   ClearMacroList();
   ClearFunctionList();
   ClearDefineList();
   ClearFileList();

   dbgentry("AssembleFile");
END

        static void AssembleGroup(void)
BEGIN
   AddSuffix(FileMask,SrcSuffix);
   if (NOT DirScan(FileMask,AssembleFile))
    fprintf(stderr,"%s%s\n",FileMask,getmessage(Num_InfoMessNFilesFound));
END

/*-------------------------------------------------------------------------*/

        static CMDResult CMD_SharePascal(Boolean Negate, char *Arg)
BEGIN
   if (NOT Negate) ShareMode=1;
   else if (ShareMode==1) ShareMode=0;
   return CMDOK;
END

        static CMDResult CMD_ShareC(Boolean Negate, char *Arg)
BEGIN
   if (NOT Negate) ShareMode=2;
   else if (ShareMode==2) ShareMode=0;
   return CMDOK;
END

        static CMDResult CMD_ShareAssembler(Boolean Negate, char *Arg)
BEGIN
   if (NOT Negate) ShareMode=3;
   else if (ShareMode==3) ShareMode=0;
   return CMDOK;
END

        static CMDResult CMD_DebugMode(Boolean Negate, char *Arg)
BEGIN
   UpString(Arg);

   if (Negate)
    if (Arg[0]!='\0') return CMDErr;
    else
     BEGIN
      DebugMode=DebugNone; return CMDOK;
     END
   else if (strcmp(Arg,"")==0)
    BEGIN
     DebugMode=DebugMAP; return CMDOK;
    END
   else if (strcmp(Arg,"ATMEL")==0)
    BEGIN
     DebugMode=DebugAtmel; return CMDArg;
    END
   else if (strcmp(Arg,"MAP")==0)
    BEGIN
     DebugMode=DebugMAP; return CMDArg;
    END
   else if (strcmp(Arg,"NOICE")==0)
    BEGIN
     DebugMode=DebugNoICE; return CMDArg;
    END
   /*else if (strcmp(Arg,"A.OUT")==0)
    BEGIN
     DebugMode=DebugAOUT; return CMDArg;
    END
   else if (strcmp(Arg,"COFF")==0)
    BEGIN
     DebugMode=DebugCOFF; return CMDArg;
    END
   else if (strcmp(Arg,"ELF")==0)
    BEGIN
     DebugMode=DebugELF; return CMDArg;
    END*/
   else return CMDErr;

/*   if (Negate) DebugMode=DebugNone;
   else DebugMode=DebugMAP;
   return CMDOK;*/
END

        static CMDResult CMD_ListConsole(Boolean Negate, char *Arg)
BEGIN
   if (NOT Negate) ListMode=1;
   else if (ListMode==1) ListMode=0;
   return CMDOK;
END

        static CMDResult CMD_ListFile(Boolean Negate, char *Arg)
BEGIN
   if (NOT Negate) ListMode=2;
   else if (ListMode==2) ListMode=0;
   return CMDOK;
END

        static CMDResult CMD_SuppWarns(Boolean Negate, char *Arg)
BEGIN
   SuppWarns=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_UseList(Boolean Negate, char *Arg)
BEGIN
   MakeUseList=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_CrossList(Boolean Negate, char *Arg)
BEGIN
   MakeCrossList=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_SectionList(Boolean Negate, char *Arg)
BEGIN
   MakeSectionList=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_BalanceTree(Boolean Negate, char *Arg)
BEGIN
   BalanceTree=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_MakeDebug(Boolean Negate, char *Arg)
BEGIN
   if (NOT Negate)
    BEGIN
     MakeDebug=True;
     errno=0; Debug=fopen("as.deb","w"); 
     if (Debug==Nil) ChkIO(10002);
    END
   else if (MakeDebug)
    BEGIN
     MakeDebug=False;
     fclose(Debug);
    END
   return CMDOK;
END

        static CMDResult CMD_MacProOutput(Boolean Negate, char *Arg)
BEGIN
   MacProOutput=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_MacroOutput(Boolean Negate, char *Arg)
BEGIN
   MacroOutput=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_MakeIncludeList(Boolean Negate, char *Arg)
BEGIN
   MakeIncludeList=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_CodeOutput(Boolean Negate, char *Arg)
BEGIN
   CodeOutput=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_MsgIfRepass(Boolean Negate, String Arg)
BEGIN
   Boolean OK;

   MsgIfRepass=NOT Negate;
   if (MsgIfRepass)
    if (Arg[0]=='\0')
     BEGIN
      PassNoForMessage=1; return CMDOK;
     END
    else
     BEGIN
      PassNoForMessage=ConstLongInt(Arg,&OK);
      if (NOT OK)
       BEGIN
        PassNoForMessage=1; return CMDOK;
       END
      else if (PassNoForMessage<1) return CMDErr;
      else return CMDArg;
     END
   else return CMDOK;
END

        static CMDResult CMD_ExtendErrors(Boolean Negate, char *Arg)
BEGIN
   if ((Negate) AND (ExtendErrors > 0))
     ExtendErrors--;
   else if ((NOT Negate) AND (ExtendErrors < 2))
     ExtendErrors++;

   return CMDOK;
END

        static CMDResult CMD_NumericErrors(Boolean Negate, char *Arg)
BEGIN
   NumericErrors=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_HexLowerCase(Boolean Negate, char *Arg)
BEGIN
   HexLowerCase=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_QuietMode(Boolean Negate, char *Arg)
BEGIN
   QuietMode=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_ThrowErrors(Boolean Negate, char *Arg)
BEGIN
   ThrowErrors=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_CaseSensitive(Boolean Negate, char *Arg)
BEGIN
   CaseSensitive=NOT Negate;
   return CMDOK;
END

        static CMDResult CMD_IncludeList(Boolean Negate, char *Arg)
BEGIN
   char *p;
   String Copy,part;

   if (*Arg=='\0') return CMDErr;
   else
    BEGIN
     strncpy(Copy,Arg,255);
     do
      BEGIN
       p=strrchr(Copy,DIRSEP); 
       if (p==Nil)
        BEGIN
         strmaxcpy(part,Copy,255);
         *Copy='\0';
        END
       else
        BEGIN
         *p='\0'; strmaxcpy(part,p+1,255);
        END 
       if (Negate) RemoveIncludeList(part); else AddIncludeList(part);
      END
     while (Copy[0]!='\0');
     return CMDArg;
    END
END

        static CMDResult CMD_ListMask(Boolean Negate, char *Arg)
BEGIN
   Byte erg; 
   Boolean OK;

   if (Arg[0]=='\0') return CMDErr;
   else
    BEGIN
     erg=ConstLongInt(Arg,&OK);
     if ((NOT OK) OR (erg>31)) return CMDErr;
     else
      BEGIN
       if (Negate) ListMask&=(~erg);
       else ListMask|=erg;
       return CMDArg;
      END
    END
END

        static CMDResult CMD_DefSymbol(Boolean Negate, char *Arg)
BEGIN
   String Copy,Part,Name;
   char *p;
   TempResult t;

   if (Arg[0]=='\0') return CMDErr;

   strmaxcpy(Copy,Arg,255);
   do
    BEGIN
     p=QuotPos(Copy,',');
     if (p==Nil)
      BEGIN
       strmaxcpy(Part,Copy,255); Copy[0]='\0';
      END
     else
      BEGIN
       *p='\0'; strmaxcpy(Part,Copy,255); strcpy(Copy,p+1);
      END
    UpString(Part);
    p=QuotPos(Part,'=');
    if (p==Nil)
     BEGIN
      strmaxcpy(Name,Part,255); Part[0]='\0';
     END
    else
     BEGIN
      *p='\0'; strmaxcpy(Name,Part,255); strcpy(Part,p+1);
     END
    if (NOT ChkSymbName(Name)) return CMDErr;
    if (Negate) RemoveDefSymbol(Name);
    else
     BEGIN
      AsmParsInit();
      if (Part[0]!='\0')
       BEGIN
        FirstPassUnknown=False;
        EvalExpression(Part,&t);
        if ((t.Typ==TempNone) OR (FirstPassUnknown)) return CMDErr;
       END
      else
       BEGIN
        t.Typ=TempInt; t.Contents.Int=1;
       END
      AddDefSymbol(Name,&t);
     END
    END
   while (Copy[0]!='\0');

   return CMDArg;
END

        static CMDResult CMD_ErrorPath(Boolean Negate, String Arg)
BEGIN
   if (Negate) return CMDErr;
   else if (Arg[0]=='\0')
    BEGIN
     ErrorPath[0]='\0'; return CMDOK;
    END
   else
    BEGIN
     strncpy(ErrorPath,Arg,255); return CMDArg;
    END
END

        static CMDResult CMD_HardRanges(Boolean Negate, char *Arg)
BEGIN
   HardRanges=Negate; return CMDOK;
END

        static CMDResult CMD_OutFile(Boolean Negate, char *Arg)
BEGIN
   if (Arg[0]=='\0')
    if (Negate)
     BEGIN
      ClearOutList(); return CMDOK;
     END
    else return CMDErr;
   else
    BEGIN
     if (Negate) RemoveFromOutList(Arg);
     else AddToOutList(Arg);
     return CMDArg;
    END
END

        static CMDResult CMD_ShareOutFile(Boolean Negate, char *Arg)
BEGIN
   if (Arg[0]=='\0')
    if (Negate)
     BEGIN
      ClearShareOutList(); return CMDOK;
     END
    else return CMDErr;
   else
    BEGIN
     if (Negate) RemoveFromShareOutList(Arg);
     else AddToShareOutList(Arg);
     return CMDArg;
    END
END


   	static Boolean CMD_CPUAlias_ChkCPUName(char *s)
BEGIN
   int z;

   for(z=0; z<strlen(s); z++)
    if (NOT isalnum((unsigned int) s[z])) return False;
   return True;
END

	static CMDResult CMD_CPUAlias(Boolean Negate, char *Arg)
BEGIN
   char *p;
   String s1,s2;

   if (Negate) return CMDErr;
   else if (Arg[0]=='\0') return CMDErr;
   else
    BEGIN
     p=strchr(Arg,'=');
     if (p==Nil) return CMDErr;
     else
      BEGIN
       *p='\0'; 
       strmaxcpy(s1,Arg,255); UpString(s1);
       strmaxcpy(s2,p+1,255); UpString(s2);
       *p='=';
       if (NOT (CMD_CPUAlias_ChkCPUName(s1) AND CMD_CPUAlias_ChkCPUName(s2)))
        return CMDErr;
       else if (NOT AddCPUAlias(s2,s1)) return CMDErr;
       else return CMDArg;
      END
    END
END

	static CMDResult CMD_SetCPU(Boolean Negate, char *Arg)
BEGIN
   if (Negate)
    BEGIN
     *DefCPU = '\0';
     return CMDOK;
    END
   else
    BEGIN
     if (*Arg == '\0') return CMDErr;
     strmaxcpy(DefCPU, Arg, sizeof(DefCPU) - 1);
     NLS_UpString(DefCPU);
     return CMDArg;
    END
END

        static void ParamError(Boolean InEnv, char *Arg)
BEGIN
   printf("%s%s\n",getmessage((InEnv) ? Num_ErrMsgInvEnvParam : Num_ErrMsgInvParam),Arg);
   exit(4);
END

#define ASParamCnt 33
static CMDRec ASParams[ASParamCnt]=
              {{"A"    , CMD_BalanceTree},
               {"ALIAS", CMD_CPUAlias},
               {"a"    , CMD_ShareAssembler},
               {"C"    , CMD_CrossList},
               {"c"    , CMD_ShareC},
               {"CPU"  , CMD_SetCPU},
               {"D"    , CMD_DefSymbol},
               {"E"    , CMD_ErrorPath},
               {"g"    , CMD_DebugMode},
               {"G"    , CMD_CodeOutput},
               {"h"    , CMD_HexLowerCase},
               {"i"    , CMD_IncludeList},
               {"I"    , CMD_MakeIncludeList},
               {"L"    , CMD_ListFile},
               {"l"    , CMD_ListConsole},
               {"M"    , CMD_MacroOutput},
               {"n"    , CMD_NumericErrors},
               {"o"    , CMD_OutFile},
               {"P"    , CMD_MacProOutput},
               {"p"    , CMD_SharePascal},
               {"q"    , CMD_QuietMode},
               {"QUIET", CMD_QuietMode},
               {"r"    , CMD_MsgIfRepass},
               {"s"    , CMD_SectionList},
               {"SHAREOUT", CMD_ShareOutFile},
               {"t"    , CMD_ListMask},
               {"u"    , CMD_UseList},
               {"U"    , CMD_CaseSensitive},
               {"w"    , CMD_SuppWarns},
               {"WARNRANGES", CMD_HardRanges},
               {"x"    , CMD_ExtendErrors},
               {"X"    , CMD_MakeDebug},
               {"Y"    , CMD_ThrowErrors}};

/*--------------------------------------------------------------------------*/



#ifdef __sunos__

        extern void on_exit(void (*procp)(int status, caddr_t arg),caddr_t arg);

	static void GlobExitProc(int status, caddr_t arg)
BEGIN
   if (MakeDebug) fclose(Debug);
END

#else

        static void GlobExitProc(void)
BEGIN
   if (MakeDebug) fclose(Debug);
END

#endif

static int LineZ;

        static void NxtLine(void)
BEGIN
   if (++LineZ==23)
    BEGIN
     LineZ=0;
     if (Redirected!=NoRedir) return;
     printf("%s",getmessage(Num_KeyWaitMsg)); fflush(stdout);
     while (getchar()!='\n'); 
     printf("%s%s",CursUp,ClrEol);
    END
END

	static void WrHead(void)
BEGIN
   if (!QuietMode)
    BEGIN
     printf("%s%s\n",getmessage(Num_InfoMessMacroAss),Version); NxtLine();
     printf("(%s-%s)\n",ARCHPRNAME,ARCHSYSNAME); NxtLine();
     printf("%s\n",InfoMessCopyright); NxtLine();
     WriteCopyrights(NxtLine);
     printf("\n"); NxtLine();
    END
END

	int main(int argc, char **argv)
BEGIN
   char *Env,*ph1,*ph2;
   String Dummy;
   int i;
   static Boolean First=TRUE;
   CMDProcessed ParUnprocessed;     /* bearbeitete Kommandozeilenparameter */

   ParamCount=argc-1; ParamStr=argv;

   if (First)
    BEGIN
     endian_init(); nls_init(); bpemu_init(); stdhandl_init();
     strutil_init(); stringlists_init(); chunks_init();

     NLS_Initialize();

     nlmessages_init("as.msg",*argv,MsgId1,MsgId2); ioerrs_init(*argv);
     cmdarg_init(*argv);
     
     asmfnums_init(); asminclist_init(); asmitree_init(); 

     asmdef_init(); asmsub_init(); asmpars_init(); 

     asmmac_init(); asmif_init(); asmcode_init(); asmdebug_init(); 

     codeallg_init(); codepseudo_init();

     code68k_init();
     code56k_init(); 
     code601_init();
     codemcore_init();
     code68_init(); code6805_init(); code6809_init(); code6812_init(); code6816_init();
     codeh8_3_init(); codeh8_5_init(); code7000_init();
     code65_init(); code7700_init(); code4500_init(); codem16_init(); codem16c_init();
     code4004_init(); code8008_init(); code48_init(); code51_init(); code96_init(); code85_init(); code86_init(); code960_init();
     code8x30x_init(); codexa_init();
     codeavr_init();
     code29k_init();
     code166_init();
     codez80_init(); codez8_init();
     code96c141_init(); code90c141_init(); code87c800_init(); code47c00_init(); code97c241_init();
     code16c5x_init(); code16c8x_init(); code17c4x_init();
     codest6_init(); codest7_init(); codest9_init(); code6804_init();
     code3201x_init(); code3202x_init(); code3203x_init(); code3205x_init(); code3206x_init();
     code9900_init(); codetms7_init(); code370_init(); codemsp_init();
     code78c10_init(); code75k0_init(); code78k0_init(); code7720_init(); code77230_init();
     codescmp_init(); codecop8_init(); codesc14xxx_init();
     codeace_init();
     code53c8xx_init();
     codef2mc8_init();
     /*as1750_init();*/
     First=FALSE;
    END
 
#ifdef __sunos__
   on_exit(GlobExitProc,(caddr_t) Nil);
#else
#ifndef __MUNIX__
   atexit(GlobExitProc);
#endif
#endif

   *CursUp='\0'; *ClrEol='\0';
   switch (Redirected)
    BEGIN
     case NoRedir:
      Env=getenv("USEANSI"); strncpy(Dummy,(Env!=Nil)?Env:"Y",255);
      if (toupper(Dummy[0])=='N')
       BEGIN
       END
      else
       BEGIN
        strcpy(ClrEol," [K"); ClrEol[0]=Char_ESC;  /* ANSI-Sequenzen */
        strcpy(CursUp," [A"); CursUp[0]=Char_ESC;
       END
      break;
     case RedirToDevice:
      /* Basissteuerzeichen fuer Geraete */
      for (i=1; i<=20; i++) strcat(ClrEol," ");
      for (i=1; i<=20; i++) strcat(ClrEol,"\b");
      break;
     case RedirToFile:
      strcpy(ClrEol,"\n");  /* CRLF auf Datei */
    END

   ShareMode=0; ListMode=0; IncludeList[0]='\0'; SuppWarns=False;
   MakeUseList=False; MakeCrossList=False; MakeSectionList=False;
   MakeIncludeList=False; ListMask=0xff;
   MakeDebug=False; ExtendErrors=0;
   MacroOutput=False; MacProOutput=False; CodeOutput=True;
   strcpy(ErrorPath,"!2"); MsgIfRepass=False; QuietMode=False;
   NumericErrors=False; DebugMode=DebugNone; CaseSensitive=False;
   ThrowErrors=False; HardRanges=True;

   LineZ=0;

   if (ParamCount==0)
    BEGIN
     WrHead();
     printf("%s%s%s\n",getmessage(Num_InfoMessHead1),GetEXEName(),getmessage(Num_InfoMessHead2)); NxtLine();
     for (ph1=getmessage(Num_InfoMessHelp),ph2=strchr(ph1,'\n'); ph2!=Nil; ph1=ph2+1,ph2=strchr(ph1,'\n'))
      BEGIN
       *ph2='\0';
       printf("%s\n",ph1); NxtLine();
       *ph2='\n';
      END
     PrintCPUList(NxtLine);
     ClearCPUList();
     exit(1);
    END

#if defined(STDINCLUDES)
   CMD_IncludeList(False,STDINCLUDES);
#endif
   ProcessCMD(ASParams,ASParamCnt,ParUnprocessed,EnvName,ParamError);

   /* wegen QuietMode dahinter */

   WrHead();

   GlobErrFlag=False;
   if (ErrorPath[0]!='\0')
    BEGIN
     strcpy(ErrorName,ErrorPath);
     unlink(ErrorName);
    END
   IsErrorOpen=False;

   for (i=1; i<=ParamCount; i++)
    if (ParUnprocessed[i]) break;
   if (i>ParamCount)
    BEGIN
     printf("%s [%s] ",getmessage(Num_InvMsgSource),SrcSuffix); fflush(stdout);
     fgets(FileMask,255,stdin);
     if (FileMask[strlen(FileMask)-1]=='\n') FileMask[strlen(FileMask)-1]='\0';
     AssembleGroup();
    END
   else
    for (i=1; i<=ParamCount; i++)
    if (ParUnprocessed[i])
     BEGIN
      strmaxcpy(FileMask,ParamStr[i],255);
      AssembleGroup();
     END

   if ((ErrorPath[0]!='\0') AND (IsErrorOpen))
    BEGIN
     fclose(ErrorFile); IsErrorOpen=False;
    END

   ClearCPUList();

   if (GlobErrFlag) return (2); else return (0);
END
