/* asmsub.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterfunktionen, vermischtes                                              */
/*                                                                           */
/* Historie:  4. 5. 1996  Grundsteinlegung                                   */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "stdhandl.h"
#include "nls.h"
#include "stringutil.h"
#include "stringlists.h"
#include "chunks.h"
#include "ioerrors.h"
#include "asmdef.h"

#include "asmsub.h"


Word ErrorCount,WarnCount;
static StringList CopyrightList,OutList;

static LongWord StartStack,MinStack,LowStack;

#define ERRMSG
#include "as.rsc"
#include "ioerrors.rsc"

/****************************************************************************/
/* Modulinitialisierung */


        void AsmSubInit(void)
BEGIN
   PageLength=60; PageWidth=0;
   ErrorCount=0; WarnCount=0;
END

/****************************************************************************/
/* neuen Prozessor definieren */

        CPUVar AddCPU(char *NewName, TSwitchProc Switcher)
BEGIN
   PCPUDef Lauf,Neu;
   char *p;

   Neu=(PCPUDef) malloc(sizeof(TCPUDef));
   Neu->Name=strdup(NewName); 
   /* kein UpString, weil noch nicht initialisiert ! */
   for (p=Neu->Name; *p!='\0'; p++) *p=toupper(*p);
   Neu->SwitchProc=Switcher;
   Neu->Next=Nil;
   Neu->Number=Neu->Orig=CPUCnt;

   Lauf=FirstCPUDef;
   if (Lauf==Nil) FirstCPUDef=Neu;
   else
    BEGIN
     while (Lauf->Next!=Nil) Lauf=Lauf->Next;
     Lauf->Next=Neu;
    END

   return CPUCnt++;
END

        Boolean AddCPUAlias(char *OrigName, char *AliasName)
BEGIN
   PCPUDef Lauf=FirstCPUDef,Neu;

   while ((Lauf!=Nil) AND (strcmp(Lauf->Name,OrigName)!=0)) Lauf=Lauf->Next;

   if (Lauf==Nil) return False;
   else
    BEGIN
     Neu=(PCPUDef) malloc(sizeof(TCPUDef));
     Neu->Next=Nil; 
     Neu->Name=strdup(AliasName);
     Neu->Number=CPUCnt++;
     Neu->Orig=Lauf->Orig;
     Neu->SwitchProc=Lauf->SwitchProc;
     while (Lauf->Next!=Nil) Lauf=Lauf->Next;
     Lauf->Next=Neu;
     return True;
    END
END

	void PrintCPUList(TSwitchProc NxtProc)
BEGIN
   PCPUDef Lauf;
   TSwitchProc Proc;

   Lauf=FirstCPUDef; Proc=NullProc;
   while (Lauf!=Nil)
    BEGIN
     if (Lauf->Number==Lauf->Orig)
      BEGIN
       if (Lauf->SwitchProc!=Proc)
        BEGIN
         Proc=Lauf->SwitchProc; printf("\n"); NxtProc();
        END
       printf("%-10s",Lauf->Name);
      END
     Lauf=Lauf->Next;
    END
   printf("\n"); NxtProc();
END

	void ClearCPUList(void)
BEGIN
   PCPUDef Save;
   
   while (FirstCPUDef!=Nil)
    BEGIN
     Save=FirstCPUDef; FirstCPUDef=Save->Next;
     free(Save->Name); free(Save);
    END
END	

/****************************************************************************/
/* Copyrightlistenverwaltung */

        void AddCopyright(char *NewLine)
BEGIN
   AddStringListLast(&CopyrightList,NewLine);
END

        void WriteCopyrights(TSwitchProc NxtProc)
BEGIN
   StringRecPtr Lauf;

   if (NOT StringListEmpty(CopyrightList)) 
    BEGIN
     printf("%s\n",GetStringListFirst(CopyrightList,&Lauf)); NxtProc();
     while (Lauf!=Nil) 
      BEGIN
       printf("%s\n",GetStringListNext(&Lauf)); NxtProc();
      END
    END
END

/*--------------------------------------------------------------------------*/
/* ermittelt das erste/letzte Auftauchen eines Zeichens ausserhalb */
/* "geschuetzten" Bereichen */

        char *QuotPos(char *s, char Zeichen)
BEGIN
   register ShortInt Brack=0,AngBrack=0;
   char *i;
   register LongWord Flag=0;

   for (i=s; *i!='\0'; i++)
    if (*i==Zeichen)
     BEGIN
      if ((AngBrack|Brack|Flag)==0) return i;
     END
    else switch(*i)
     BEGIN
      case '"': if ((Brack==0) AND (AngBrack==0) AND ((Flag&2)==0)) Flag^=1; break;
      case '\'':if ((Brack==0) AND (AngBrack==0) AND ((Flag&1)==0)) Flag^=2; break;
      case '(': if ((AngBrack|Flag)==0) Brack++; break;
      case ')': if ((AngBrack|Flag)==0) Brack--; break;
      case '[': if ((Brack|Flag)==0) AngBrack++; break;
      case ']': if ((Brack|Flag)==0) AngBrack--; break;
     END

   return Nil;    
END

        char *RQuotPos(char *s, char Zeichen)
BEGIN
   ShortInt Brack=0,AngBrack=0;
   char *i;
   Boolean Quot=False,Paren=False;

   for (i=s+strlen(s)-1; i>=s; i--)
    if (*i==Zeichen)
     BEGIN
      if ((AngBrack==0) AND (Brack==0) AND (NOT Paren) AND (NOT Quot)) return i;
     END
    else switch (*i)
     BEGIN
      case '"': if ((Brack==0) AND (AngBrack==0) AND (NOT Quot)) Paren=NOT Paren; break;
      case '\'':if ((Brack==0) AND (AngBrack==0) AND (NOT Paren)) Quot=NOT Quot; break;
      case ')': if ((AngBrack==0) AND (NOT Paren) AND (NOT Quot)) Brack++; break;
      case '(': if ((AngBrack==0) AND (NOT Paren) AND (NOT Quot)) Brack--; break;
      case ']': if ((Brack==0) AND (NOT Paren) AND (NOT Quot)) AngBrack++; break;
      case '[': if ((Brack==0) AND (NOT Paren) AND (NOT Quot)) AngBrack--; break;
     END

   return Nil;
END

/*--------------------------------------------------------------------------*/
/* ermittelt das erste Leerzeichen in einem String */

        char *FirstBlank(char *s)
BEGIN
   char *h,*Min=Nil;

   h=strchr(s,' ');  
   if (h!=Nil) if ((Min==Nil) OR (h<Min)) Min=h;
   h=strchr(s,Char_HT);
   if (h!=Nil) if ((Min==Nil) OR (h<Min)) Min=h;
   return Min;
END

/*--------------------------------------------------------------------------*/
/* einen String in zwei Teile zerlegen */

	void SplitString(char *Source, char *Left, char *Right, char *Trenner)
BEGIN
   char Save;
   LongInt slen=strlen(Source);

   if ((Trenner==Nil) OR (Trenner>=Source+slen))
    Trenner=Source+slen;
   Save=(*Trenner); *Trenner='\0';
   strcpy(Left,Source); *Trenner=Save;
   if (Trenner>=Source+slen) *Right='\0';
   else strcpy(Right,Trenner+1);
END

/*--------------------------------------------------------------------------*/
/* verbesserte Grossbuchstabenfunktion */

/* einen String in Grossbuchstaben umwandeln.  Dabei Stringkonstanten in Ruhe */
/* lassen */

	void UpString(char *s)
BEGIN
   char *z;
   Integer hyp=0,quot=0;

   for (z=s; *z!='\0'; z++)
    BEGIN
     if ((*z=='\'') AND ((quot&1)==0)) hyp^=1;
     else if ((*z=='"') AND ((hyp&1)==0)) quot^=1;
     else if ((quot|hyp)==0) *z=UpCaseTable[(int)*z];
    END
END

/*--------------------------------------------------------------------------*/
/* alle Leerzeichen aus einem String loeschen */

        void KillBlanks(char *s)
BEGIN
   char *z;
   Integer dest=0;
   Boolean InHyp=False,InQuot=False;

   for (z=s; *z!='\0'; z++)
    BEGIN
     switch (*z)
      BEGIN
       case '\'': if (NOT InQuot) InHyp=NOT InHyp; break;
       case '"': if (NOT InHyp) InQuot=NOT InQuot; break;
      END
     if ((NOT isspace(*z)) OR (InHyp) OR (InQuot)) s[dest++]=(*z);
    END
   s[dest]='\0';
END


/*--------------------------------------------------------------------------*/
/* fuehrende Leerzeichen loeschen */

        void KillPrefBlanks(char *s)
BEGIN
   char *z=s;

   while ((*z!='\0') AND (isspace(*z))) z++;
   if (z!=s) strcpy(s,z);
END

/*--------------------------------------------------------------------------*/
/* anhaengende Leerzeichen loeschen */

        void KillPostBlanks(char *s)
BEGIN
   char *z=s+strlen(s)-1;

   while ((z>=s) AND (isspace(*z))) *(z--)='\0';
END

/****************************************************************************/

        void TranslateString(char *s)
BEGIN
   char *z;

   for (z=s; *z!='\0'; z++) *z=CharTransTable[((unsigned int)(*z))&0xff];
END

	ShortInt StrCmp(char *s1, char *s2, LongInt Hand1, LongInt Hand2)
BEGIN
   ShortInt tmp;

   tmp=strcmp(s1,s2); 
   if (tmp<0) return -1; 
   else if (tmp>0) return 1;
   else if (Hand1<Hand2) return -1;
   else if (Hand1>Hand2) return 1;
   else return 0;
END

/****************************************************************************/
/* an einen Dateinamen eine Endung anhaengen */

        void AddSuffix(char *s, char *Suff)
BEGIN
   char *p,*z,*Part;

   p=Nil;
   for (z=s; *z!='\0'; z++) if (*z=='\\') p=z;
   Part=(p!=Nil)?(p):(s);
   if (strchr(Part,'.')==Nil) strmaxcat(s,Suff,255);
END


/*--------------------------------------------------------------------------*/
/* von einem Dateinamen die Endung loeschen */

        void KillSuffix(char *s)
BEGIN
   char *p,*z,*Part;

   p=Nil;
   for (z=s; *z!='\0'; z++) if (*z=='\\') p=z;
   Part=(p!=Nil)?(p):(s); Part=strchr(Part,'.');
   if (Part!=Nil) *Part='\0';
END

/*--------------------------------------------------------------------------*/
/* Pfadanteil (Laufwerk+Verzeichnis) von einem Dateinamen abspalten */

        char *PathPart(char *Name)
BEGIN
   static String s;
   char *p;

   strmaxcpy(s,Name,255);

   p=strrchr(Name,PATHSEP);
#ifdef DRSEP
   if (p==Nil) p=strrchr(Name,DRSEP);
#endif

   if (p==Nil) *s='\0'; else s[1]='\0';

   return s;
END

/*--------------------------------------------------------------------------*/
/* Namensanteil von einem Dateinamen abspalten */

        char *NamePart(char *Name)
BEGIN
   char *p=strrchr(Name,PATHSEP);

#ifdef DRSEP
   if (p==Nil) p=strrchr(Name,DRSEP);
#endif

   return (p==Nil)?(Name):(p+1);
END

/****************************************************************************/
/* eine Gleitkommazahl in einen String umwandeln */

        char *FloatString(Double f)
BEGIN
#define MaxLen 18
   static String s;
   char *p,*d;
   Integer n,ExpVal,nzeroes;
   Boolean WithE,OK;

   /* 1. mit Maximallaenge wandeln, fuehrendes Vorzeichen weg */

   sprintf(s,"%27.15e",f); 
   while ((s[0]==' ') OR (s[0]=='+')) strcpy(s,s+1);

   /* 2. Exponenten soweit als moeglich kuerzen, evtl. ganz streichen */

   p=strchr(s,'e'); 
   switch (*(++p))
    BEGIN
     case '+': strcpy(p,p+1); break;
     case '-': p++; break;
    END

   while (*p=='0') strcpy(p,p+1);
   WithE=(*p!='\0');
   if (NOT WithE) s[strlen(s)-1]='\0';

   /* 3. Nullen am Ende der Mantisse entfernen, Komma bleibt noch */

   if (WithE) p=strchr(s,'e'); else p=s+strlen(s); p--;
   while (*p=='0') 
    BEGIN
     strcpy(p,p+1); p--;
    END

   /* 4. auf die gewuenschte Maximalstellenzahl begrenzen */

   if (WithE) p=strchr(s,'e'); else p=s+strlen(s);
   d=strchr(s,'.');
   n=p-d-1;

   /* 5. Maximallaenge ueberschritten ? */

   if (strlen(s)>MaxLen) strcpy(d+(n-(strlen(s)-MaxLen)),d+n);

   /* 6. Exponentenwert berechnen */

   if (WithE)
    BEGIN
     p=strchr(s,'e');
     ExpVal=ConstLongInt(p+1,&OK);
    END
   else
    BEGIN
     p=s+strlen(s);
     ExpVal=0;
    END

   /* 7. soviel Platz, dass wir den Exponenten weglassen und evtl. Nullen
        anhaengen koennen ? */

   if (ExpVal>0)
    BEGIN
     nzeroes=ExpVal-(p-strchr(s,'.')-1); /* = Zahl von Nullen, die anzuhaengen waere */

     /* 7a. nur Kommaverschiebung erforderlich. Exponenten loeschen und
           evtl. auch Komma */

     if (nzeroes<=0)
      BEGIN
       *p='\0';
       d=strchr(s,'.'); strcpy(d,d+1);
       if (nzeroes!=0) 
        BEGIN
         memmove(s+strlen(s)+nzeroes+1,s+strlen(s)+nzeroes,-nzeroes);
         s[strlen(s)-1+nzeroes]='.';
        END
      END

     /* 7b. Es muessen Nullen angehaengt werden. Schauen, ob nach Loeschen von
           Punkt und E-Teil genuegend Platz ist */

     else
      BEGIN
       n=strlen(p)+1+(MaxLen-strlen(s)); /* = Anzahl freizubekommender Zeichen+Gutschrift */ 
       if (n>=nzeroes)
        BEGIN
         *p='\0'; d=strchr(s,'.'); strcpy(d,d+1);
         d=s+strlen(s); 
         for (n=0; n<nzeroes; n++) *(d++)='0'; *d='\0';
        END
      END
    END

   /* 8. soviel Platz, dass Exponent wegkann und die Zahl mit vielen Nullen
        vorne geschrieben werden kann ? */

   else if (ExpVal<0)
    BEGIN
     n=(-ExpVal)-(strlen(p)); /* = Verlaengerung nach Operation */
     if (strlen(s)+n<=MaxLen)
      BEGIN
       *p='\0'; d=strchr(s,'.'); strcpy(d,d+1);
       if (s[0]=='-') d=s+1; else d=s;
       memmove(d-ExpVal+1,d,strlen(s)+1);
       *(d++)='0'; *(d++)='.';
       for (n=0; n<-ExpVal-1; n++) *(d++)='0';
      END
    END


   /* 9. Ueberfluessiges Komma entfernen */

   if (WithE) 
    BEGIN
     p=strchr(s,'e'); if (p!=Nil) *p='E';
    END
   else p=s+strlen(s);
   if ((p!=Nil) AND (*(p-1)=='.')) strcpy(p-1,p);

   return s;
END

/****************************************************************************/
/* Symbol in String wandeln */

        void StrSym(TempResult *t, Boolean WithSystem, char *Dest)
BEGIN
   switch (t->Typ)
    BEGIN
     case TempInt:
      strcpy(Dest,HexString(t->Contents.Int,1));
      if (WithSystem)
       switch (ConstMode)
        BEGIN
         case ConstModeIntel : strcat(Dest,"H"); break;
         case ConstModeMoto  : strprep(Dest,"$"); break;
         case ConstModeC     : strprep(Dest,"0x"); break;
        END
      break;
     case TempFloat:
      strcpy(Dest,FloatString(t->Contents.Float)); break;
     case TempString:
      strcpy(Dest,t->Contents.Ascii); break;
     default: strcpy(Dest,"???");
    END
END

/****************************************************************************/
/* Listingzaehler zuruecksetzen */

        void ResetPageCounter(void)
BEGIN
   Integer z;

   for (z=0; z<=ChapMax; z++) PageCounter[z]=0;
   LstCounter=0; ChapDepth=0;
END

/*--------------------------------------------------------------------------*/
/* eine neue Seite im Listing beginnen */

        void NewPage(ShortInt Level, Boolean WithFF)
BEGIN
   ShortInt z;
   String Header,s;
   char Save;

   if (ListOn==0) return;

   LstCounter=0;

   if (ChapDepth<(Byte) Level)
    BEGIN
     memmove(PageCounter+(Level-ChapDepth),PageCounter,(ChapDepth+1)*sizeof(Word));
     for (z=0; z<=Level-ChapDepth; PageCounter[z++]=1);
     ChapDepth=Level;
    END
   for (z=0; z<=Level-1; PageCounter[z++]=1);
   PageCounter[Level]++;

   if (WithFF)
    BEGIN
     errno=0; fprintf(LstFile,"%c",Char_FF); ChkIO(10002);
    END

   sprintf(Header," AS V%s%s%s",Version,HeadingFileNameLab,NamePart(SourceFile));
   if ((strcmp(CurrFileName,"INTERNAL")!=0) AND (strcmp(NamePart(CurrFileName),NamePart(SourceFile))!=0))
    BEGIN
     strmaxcat(Header,"(",255);
     strmaxcat(Header,NamePart(CurrFileName),255);
     strmaxcat(Header,")",255);
    END
   strmaxcat(Header,HeadingPageLab,255);

   for (z=ChapDepth; z>=0; z--)
    BEGIN
     sprintf(s,"%d",PageCounter[z]); strmaxcat(Header,s,255);
     if (z!=0) strmaxcat(Header,".",255);
    END

   strmaxcat(Header," - ",255);
   NLS_CurrDateString(s); strmaxcat(Header,s,255);
   strmaxcat(Header," ",255);
   NLS_CurrTimeString(False,s); strmaxcat(Header,s,255);

   if (PageWidth!=0)
    while (strlen(Header)>PageWidth)
     BEGIN
      Save=Header[PageWidth]; Header[PageWidth]='\0';
      errno=0; fprintf(LstFile,"%s\n",Header); ChkIO(10002); 
      Header[PageWidth]=Save; strcpy(Header,Header+PageWidth);
     END
   errno=0; fprintf(LstFile,"%s\n",Header); ChkIO(10002);

   if (PrtTitleString[0]!='\0')
    BEGIN
     errno=0; fprintf(LstFile,"%s\n",PrtTitleString); ChkIO(10002);
    END

   errno=0; fprintf(LstFile,"\n\n"); ChkIO(10002);
END


/*--------------------------------------------------------------------------*/
/* eine Zeile ins Listing schieben */

        void WrLstLine(char *Line)
BEGIN
   Integer LLength;
   char bbuf[2500];
   String LLine;
   Integer blen=0,hlen,z,Start;

   if (ListOn==0) return;

   if (PageLength==0)
    BEGIN
     errno=0; fprintf(LstFile,"%s\n",Line); ChkIO(10002);
    END
   else
    BEGIN
     if ((PageWidth==0) OR ((strlen(Line)<<3)<PageWidth)) LLength=1;
     else
      BEGIN
       blen=0;
       for (z=0; z<strlen(Line);  z++)
        if (Line[z]==Char_HT)
         BEGIN
          memset(bbuf+blen,8-(blen&7),' ');
          blen+=8-(blen&7);
         END
        else bbuf[blen++]=Line[z];
       LLength=blen/PageWidth; if (blen%PageWidth!=0) LLength++;
      END
     if (LLength==1)
      BEGIN
       errno=0; fprintf(LstFile,"%s\n",Line); ChkIO(10002);
       if ((++LstCounter)==PageLength) NewPage(0,True);
      END
     else
      BEGIN
       Start=0;
       for (z=1; z<=LLength; z++)
        BEGIN
         hlen=PageWidth; if (blen-Start<hlen) hlen=blen-Start;
         memcpy(LLine,bbuf+Start,hlen); LLine[hlen]='\0';
         errno=0; fprintf(LstFile,"%s\n",LLine); 
         if ((++LstCounter)==PageLength) NewPage(0,True);
         Start+=hlen;
        END
      END
    END
END

/*****************************************************************************/
/* Ausdruck in Spalte vor Listing */


        void SetListLineVal(TempResult *t)
BEGIN
   StrSym(t,True,ListLine); strmaxprep(ListLine,"=",255);
   if (strlen(ListLine)>14)
    BEGIN
     ListLine[12]='\0'; strmaxcat(ListLine,"..",255);
    END
END

/****************************************************************************/
/* einen Symbolnamen auf Gueltigkeit ueberpruefen */

        Boolean ChkSymbName(char *sym)
BEGIN
   char *z;

   if (*sym=='\0') return False;
   if (NOT (isalpha(*sym) OR (*sym=='_') OR (*sym=='.'))) return False;
   for (z=sym; *z!='\0'; z++)
    if (NOT (isalnum(*z) OR (*z=='_') OR (*z=='.'))) return False;
   return True;
END

        Boolean ChkMacSymbName(char *sym)
BEGIN
   Integer z;

   if (*sym=='\0') return False;
   if (NOT isalpha(*sym)) return False;
   for (z=1; z<strlen(sym); z++)
    if (NOT isalnum(sym[z])) return False;
   return True;
END

/****************************************************************************/
/* Fehlerkanal offen ? */

        static void ForceErrorOpen(void)
BEGIN
   if (NOT IsErrorOpen)
    BEGIN
     RewriteStandard(&ErrorFile,ErrorName); IsErrorOpen=True;
     if (ErrorFile==Nil) ChkIO(10001);
    END
END

/*--------------------------------------------------------------------------*/
/* eine Fehlermeldung  mit Klartext ausgeben */

	static void EmergencyStop(void)
BEGIN
   if ((IsErrorOpen) AND (ErrorFile!=Nil)) fclose(ErrorFile);
   fclose(LstFile);
   if (ShareMode!=0)
    BEGIN
     fclose(ShareFile); unlink(ShareName);
    END
   if (MacProOutput)
    BEGIN
     fclose(MacProFile); unlink(MacProName);
    END
   if (MacroOutput)
    BEGIN
     fclose(MacroFile); unlink(MacroName);
    END
   if (MakeDebug) fclose(Debug);
   if (CodeOutput)
    BEGIN
     fclose(PrgFile); unlink(OutName);
    END
END

        void WrErrorString(char *Message, char *Add, Boolean Warning, Boolean Fatal)
BEGIN
   String h,h2;

   strmaxcpy(h,ErrorPos,255);
   if (NOT Warning)
    BEGIN
     strmaxcat(h,ErrName,255);
     strmaxcat(h,Add,255);
     strmaxcat(h," : ",255);
     ErrorCount++;
    END
   else
    BEGIN
     strmaxcat(h,WarnName,255);
     strmaxcat(h,Add,255);
     strmaxcat(h," : ",255);
     WarnCount++;
    END

   if ((strcmp(LstName,"/dev/null")!=0) AND (NOT Fatal))
    BEGIN
     strmaxcpy(h2,h,255); strmaxcat(h2,Message,255); WrLstLine(h2);
     if ((ExtendErrors) AND (*ExtendError!='\0'))
      BEGIN
       sprintf(h2,"> > > %s",ExtendError); WrLstLine(h2);
      END
    END
   ForceErrorOpen();
   if ((strcmp(LstName,"!1")!=0) OR (Fatal))
    BEGIN
     fprintf((ErrorFile==Nil)?stdout:ErrorFile,"%s%s%s\n",h,Message,ClrEol);
     if ((ExtendErrors) AND (*ExtendError!='\0'))
      fprintf((ErrorFile==Nil)?stdout:ErrorFile,"> > > %s%s\n",ExtendError,ClrEol);
    END
   *ExtendError='\0';

   if (Fatal)
    BEGIN
     fprintf((ErrorFile==Nil)?stdout:ErrorFile,"%s\n",ErrMsgIsFatal);
     EmergencyStop();
     exit(3);
    END
END

/*--------------------------------------------------------------------------*/
/* eine Fehlermeldung ueber Code ausgeben */

        static void WrErrorNum(Word Num)
BEGIN
   String h;
   char Add[11];
   
   if ((NOT CodeOutput) AND (Num==1200)) return;

   if ((SuppWarns) AND (Num<1000)) return;

   switch (Num)
    BEGIN
     case    0: strmaxcpy(h,ErrMsgUselessDisp,255); break;
     case   10: strmaxcpy(h,ErrMsgShortAddrPossible,255); break;
     case   20: strmaxcpy(h,ErrMsgShortJumpPossible,255); break;
     case   30: strmaxcpy(h,ErrMsgNoShareFile,255); break;
     case   40: strmaxcpy(h,ErrMsgBigDecFloat,255); break;
     case   50: strmaxcpy(h,ErrMsgPrivOrder,255); break;
     case   60: strmaxcpy(h,ErrMsgDistNull,255); break;
     case   70: strmaxcpy(h,ErrMsgWrongSegment,255); break;
     case   75: strmaxcpy(h,ErrMsgInAccSegment,255); break;
     case   80: strmaxcpy(h,ErrMsgPhaseErr,255); break;
     case   90: strmaxcpy(h,ErrMsgOverlap,255); break;
     case  100: strmaxcpy(h,ErrMsgNoCaseHit,255); break;
     case  110: strmaxcpy(h,ErrMsgInAccPage,255); break;
     case  120: strmaxcpy(h,ErrMsgRMustBeEven,255); break;
     case  130: strmaxcpy(h,ErrMsgObsolete,255); break;
     case  140: strmaxcpy(h,ErrMsgUnpredictable,255); break;
     case  150: strmaxcpy(h,ErrMsgAlphaNoSense,255); break;
     case  160: strmaxcpy(h,ErrMsgSenseless,255); break;
     case  170: strmaxcpy(h,ErrMsgRepassUnknown,255); break;
     case  180: strmaxcpy(h,ErrMsgAddrNotAligned,255); break;
     case  190: strmaxcpy(h,ErrMsgIOAddrNotAllowed,255); break;
     case  200: strmaxcpy(h,ErrMsgPipeline,255); break;
     case  210: strmaxcpy(h,ErrMsgDoubleAdrRegUse,255); break;
     case  220: strmaxcpy(h,ErrMsgNotBitAddressable,255); break;
     case  230: strmaxcpy(h,ErrMsgStackNotEmpty,255); break;
     case  240: strmaxcpy(h,ErrMsgNULCharacter ,255); break;
     case 1000: strmaxcpy(h,ErrMsgDoubleDef,255); break;
     case 1010: strmaxcpy(h,ErrMsgSymbolUndef,255); break;
     case 1020: strmaxcpy(h,ErrMsgInvSymName,255); break;
     case 1090: strmaxcpy(h,ErrMsgInvFormat,255); break;
     case 1100: strmaxcpy(h,ErrMsgUseLessAttr,255); break;
     case 1105: strmaxcpy(h,ErrMsgTooLongAttr,255); break;
     case 1107: strmaxcpy(h,ErrMsgUndefAttr,255); break;
     case 1110: strmaxcpy(h,ErrMsgWrongArgCnt,255); break;
     case 1115: strmaxcpy(h,ErrMsgWrongOptCnt,255); break;
     case 1120: strmaxcpy(h,ErrMsgOnlyImmAddr,255); break;
     case 1130: strmaxcpy(h,ErrMsgInvOpsize,255); break;
     case 1131: strmaxcpy(h,ErrMsgConfOpSizes,255); break;
     case 1132: strmaxcpy(h,ErrMsgUndefOpSizes,255); break;
     case 1135: strmaxcpy(h,ErrMsgInvOpType,255); break;
     case 1140: strmaxcpy(h,ErrMsgTooMuchArgs,255); break;
     case 1200: strmaxcpy(h,ErrMsgUnknownOpcode,255); break;
     case 1300: strmaxcpy(h,ErrMsgBrackErr,255); break;
     case 1310: strmaxcpy(h,ErrMsgDivByZero,255); break;
     case 1315: strmaxcpy(h,ErrMsgUnderRange,255); break;
     case 1320: strmaxcpy(h,ErrMsgOverRange,255); break;
     case 1325: strmaxcpy(h,ErrMsgNotAligned,255); break;
     case 1330: strmaxcpy(h,ErrMsgDistTooBig,255); break;
     case 1335: strmaxcpy(h,ErrMsgInAccReg,255); break;
     case 1340: strmaxcpy(h,ErrMsgNoShortAddr,255); break;
     case 1350: strmaxcpy(h,ErrMsgInvAddrMode,255); break;
     case 1351: strmaxcpy(h,ErrMsgMustBeEven,255); break;
     case 1355: strmaxcpy(h,ErrMsgInvParAddrMode,255); break;
     case 1360: strmaxcpy(h,ErrMsgUndefCond,255); break;
     case 1370: strmaxcpy(h,ErrMsgJmpDistTooBig,255); break;
     case 1375: strmaxcpy(h,ErrMsgDistIsOdd,255); break;
     case 1380: strmaxcpy(h,ErrMsgInvShiftArg,255); break;
     case 1390: strmaxcpy(h,ErrMsgRange18,255); break;
     case 1400: strmaxcpy(h,ErrMsgShiftCntTooBig,255); break;
     case 1410: strmaxcpy(h,ErrMsgInvRegList,255); break;
     case 1420: strmaxcpy(h,ErrMsgInvCmpMode,255); break;
     case 1430: strmaxcpy(h,ErrMsgInvCPUType,255); break;
     case 1440: strmaxcpy(h,ErrMsgInvCtrlReg,255); break;
     case 1445: strmaxcpy(h,ErrMsgInvReg,255); break;
     case 1450: strmaxcpy(h,ErrMsgNoSaveFrame,255); break;
     case 1460: strmaxcpy(h,ErrMsgNoRestoreFrame,255); break;
     case 1465: strmaxcpy(h,ErrMsgUnknownMacArg,255); break;
     case 1470: strmaxcpy(h,ErrMsgMissEndif,255); break;
     case 1480: strmaxcpy(h,ErrMsgInvIfConst,255); break;
     case 1483: strmaxcpy(h,ErrMsgDoubleSection,255); break;
     case 1484: strmaxcpy(h,ErrMsgInvSection,255); break;
     case 1485: strmaxcpy(h,ErrMsgMissingEndSect,255); break;
     case 1486: strmaxcpy(h,ErrMsgWrongEndSect,255); break;
     case 1487: strmaxcpy(h,ErrMsgNotInSection,255); break;
     case 1488: strmaxcpy(h,ErrMsgUndefdForward,255); break;
     case 1489: strmaxcpy(h,ErrMsgContForward,255); break;
     case 1490: strmaxcpy(h,ErrMsgInvFuncArgCnt,255); break;
     case 1495: strmaxcpy(h,ErrMsgMissingLTORG,255); break;
     case 1500: strmaxcpy(h,ErrMsgNotOnThisCPU1,255); 
                strmaxcat(h,MomCPUIdent,255); 
                strmaxcat(h,ErrMsgNotOnThisCPU2,255); break;
     case 1505: strmaxcpy(h,ErrMsgNotOnThisCPU3,255);
                strmaxcat(h,MomCPUIdent,255);
                strmaxcat(h,ErrMsgNotOnThisCPU2,255); break;
     case 1510: strmaxcpy(h,ErrMsgInvBitPos,255); break;
     case 1520: strmaxcpy(h,ErrMsgOnlyOnOff,255); break;
     case 1530: strmaxcpy(h,ErrMsgStackEmpty,255); break;
     case 1600: strmaxcpy(h,ErrMsgShortRead,255); break;
     case 1700: strmaxcpy(h,ErrMsgRomOffs063,255); break;
     case 1710: strmaxcpy(h,ErrMsgInvFCode,255); break;
     case 1720: strmaxcpy(h,ErrMsgInvFMask,255); break;
     case 1730: strmaxcpy(h,ErrMsgInvMMUReg,255); break;
     case 1740: strmaxcpy(h,ErrMsgLevel07,255); break;
     case 1750: strmaxcpy(h,ErrMsgInvBitMask,255); break;
     case 1760: strmaxcpy(h,ErrMsgInvRegPair,255); break;
     case 1800: strmaxcpy(h,ErrMsgOpenMacro,255); break;
     case 1805: strmaxcpy(h,ErrMsgEXITMOutsideMacro,255); break;
     case 1810: strmaxcpy(h,ErrMsgTooManyMacParams,255); break;
     case 1815: strmaxcpy(h,ErrMsgDoubleMacro,255); break;
     case 1820: strmaxcpy(h,ErrMsgFirstPassCalc,255); break;
     case 1830: strmaxcpy(h,ErrMsgTooManyNestedIfs,255); break;
     case 1840: strmaxcpy(h,ErrMsgMissingIf,255); break;
     case 1850: strmaxcpy(h,ErrMsgRekMacro,255); break;
     case 1860: strmaxcpy(h,ErrMsgUnknownFunc,255); break;
     case 1870: strmaxcpy(h,ErrMsgInvFuncArg,255); break;
     case 1880: strmaxcpy(h,ErrMsgFloatOverflow,255); break;
     case 1890: strmaxcpy(h,ErrMsgInvArgPair,255); break;
     case 1900: strmaxcpy(h,ErrMsgNotOnThisAddress,255); break;
     case 1905: strmaxcpy(h,ErrMsgNotFromThisAddress,255); break;
     case 1910: strmaxcpy(h,ErrMsgTargOnDiffPage,255); break;
     case 1920: strmaxcpy(h,ErrMsgCodeOverflow,255); break;
     case 1925: strmaxcpy(h,ErrMsgAdrOverflow,255); break;
     case 1930: strmaxcpy(h,ErrMsgMixDBDS,255); break;
     case 1940: strmaxcpy(h,ErrMsgOnlyInCode,255); break;
     case 1950: strmaxcpy(h,ErrMsgParNotPossible,255); break;
     case 1960: strmaxcpy(h,ErrMsgInvSegment,255); break;
     case 1961: strmaxcpy(h,ErrMsgUnknownSegment,255); break;
     case 1962: strmaxcpy(h,ErrMsgUnknownSegReg,255); break;
     case 1970: strmaxcpy(h,ErrMsgInvString,255); break;
     case 1980: strmaxcpy(h,ErrMsgInvRegName,255); break;
     case 1985: strmaxcpy(h,ErrMsgInvArg,255); break;
     case 1990: strmaxcpy(h,ErrMsgNoIndir,255); break;
     case 1995: strmaxcpy(h,ErrMsgNotInThisSegment,255); break;
     case 1996: strmaxcpy(h,ErrMsgNotInMaxmode,255); break;
     case 1997: strmaxcpy(h,ErrMsgOnlyInMaxmode,255); break;
     case 10001: strmaxcpy(h,ErrMsgOpeningFile,255); break;
     case 10002: strmaxcpy(h,ErrMsgListWrError,255); break;
     case 10003: strmaxcpy(h,ErrMsgFileReadError,255); break;
     case 10004: strmaxcpy(h,ErrMsgFileWriteError,255); break;
     case 10006: strmaxcpy(h,ErrMsgHeapOvfl,255); break;
     case 10007: strmaxcpy(h,ErrMsgStackOvfl,255); break;
     default  : strmaxcpy(h,ErrMsgIntError,255);
    END

   if (((Num==1910) OR (Num==1370)) AND (NOT Repass)) JmpErrors++;

   if (NumericErrors) sprintf(Add,"#%d",Num); 
   else *Add='\0';
   WrErrorString(h,Add,Num<1000,Num>=10000);
END

        void WrError(Word Num)
BEGIN
   *ExtendError='\0'; WrErrorNum(Num);
END

        void WrXError(Word Num, char *Message)
BEGIN
   strmaxcpy(ExtendError,Message,255); WrErrorNum(Num);
END

/*--------------------------------------------------------------------------*/
/* I/O-Fehler */

        void ChkIO(Word ErrNo)
BEGIN
   int io;

   io=errno; if ((io==0) OR (io==19)) return;

   WrXError(ErrNo,GetErrorMsg(io));
END

/*--------------------------------------------------------------------------*/
/* Bereichsfehler */

        Boolean ChkRange(LargeInt Value, LargeInt Min, LargeInt Max)
BEGIN
   char s1[100],s2[100];

   if (Value<Min)
    BEGIN
     strmaxcpy(s1,LargeString(Value),99); 
     strmaxcpy(s2,LargeString(Min),99);
     strmaxcat(s1,"<",99); strmaxcat(s1,s2,99);
     WrXError(1315,s1); return False;
    END
   else if (Value>Max)
    BEGIN
     strmaxcpy(s1,LargeString(Value),99);
     strmaxcpy(s2,LargeString(Max),99);
     strmaxcat(s1,">",99); strmaxcat(s1,s2,99);
     WrXError(1320,s1); return False;
    END
   else return True;
END

/****************************************************************************/

        LargeWord ProgCounter(void)
BEGIN
   return PCs[ActPC];
END

/*--------------------------------------------------------------------------*/
/* aktuellen Programmzaehler mit Phasenverschiebung holen */

        LargeWord EProgCounter(void)
BEGIN
   return PCs[ActPC]+Phases[ActPC];
END


/*--------------------------------------------------------------------------*/
/* Granularitaet des aktuellen Segments holen */

        Word Granularity(void)
BEGIN
   return Grans[ActPC];
END

/*--------------------------------------------------------------------------*/
/* Linstingbreite des aktuellen Segments holen */

        Word ListGran(void)
BEGIN
   return ListGrans[ActPC];
END

/*--------------------------------------------------------------------------*/
/* pruefen, ob alle Symbole einer Formel im korrekten Adressraum lagen */

        void ChkSpace(Byte Space)
BEGIN
   Byte Mask=0xff-(1<<Space);

   if ((TypeFlag&Mask)!=0) WrError(70);
END

/****************************************************************************/
/* eine Chunkliste im Listing ausgeben & Speicher loeschen */

        void PrintChunk(ChunkList *NChunk)
BEGIN
   LargeWord NewMin,FMin;
   Boolean Found;
   Word p=0,z;
   Integer BufferZ;
   String BufferS;

   NewMin=0; BufferZ=0; *BufferS='\0';

   do  
    BEGIN 
     /* niedrigsten Start finden, der ueberhalb des letzten Endes liegt */
     Found=False;
#ifdef __STDC__
     FMin=0xffffffffu;
#else
     FMin=0xffffffff;
#endif
     for (z=0; z<NChunk->RealLen; z++)
      if (NChunk->Chunks[z].Start>=NewMin)
       if (FMin>NChunk->Chunks[z].Start)
        BEGIN
         Found=True; FMin=NChunk->Chunks[z].Start; p=z;
        END

     if (Found)
      BEGIN
       strmaxcat(BufferS,HexString(NChunk->Chunks[p].Start,0),255);
       if (NChunk->Chunks[p].Length!=1)
        BEGIN 
         strmaxcat(BufferS,"-",255);
         strmaxcat(BufferS,HexString(NChunk->Chunks[p].Start+NChunk->Chunks[p].Length-1,0),255);
        END
       strmaxcat(BufferS,Blanks(19-strlen(BufferS)%19),255);
       if (++BufferZ==4)
        BEGIN
         WrLstLine(BufferS); *BufferS='\0'; BufferZ=0;
        END
       NewMin=NChunk->Chunks[p].Start+NChunk->Chunks[p].Length;
      END
    END
   while (Found);

   if (BufferZ!=0) WrLstLine(BufferS);
END

/*--------------------------------------------------------------------------*/
/* Listen ausgeben */

        void PrintUseList(void)
BEGIN
   Integer z,z2,l;
   String s;

   for (z=1; z<=PCMax; z++)
    if (SegChunks[z].Chunks!=Nil)
     BEGIN
      sprintf(s,"  %s%s%s",ListSegListHead1,SegNames[z],ListSegListHead2);
      WrLstLine(s);
      strcpy(s,"  ");
      l=strlen(SegNames[z])+strlen(ListSegListHead1)+strlen(ListSegListHead2);
      for (z2=0; z2<l; z2++) strmaxcat(s,"-",255);
      WrLstLine(s);
      WrLstLine("");
      PrintChunk(SegChunks+z);
      WrLstLine("");
     END
END

        void ClearUseList(void)
BEGIN
   Integer z;

   for (z=1; z<=PCMax; z++)
    ClearChunk(SegChunks+z);
END

/****************************************************************************/
/* Include-Pfadlistenverarbeitung */

        static char *GetPath(char *Acc)
BEGIN
   char *p;
   static String tmp;

   p=strchr(Acc,':');
   if (p==Nil)
    BEGIN
     strmaxcpy(tmp,Acc,255); Acc[0]='\0';
    END
   else
    BEGIN
     *p='\0'; strmaxcpy(tmp,Acc,255); strcpy(Acc,p+1);
    END
   return tmp;
END

        void AddIncludeList(char *NewPath)
BEGIN
   String Test;

   strmaxcpy(Test,IncludeList,255);
   while (*Test!='\0')
    if (strcmp(GetPath(Test),NewPath)==0) return;
   if (*IncludeList!='\0') strmaxprep(IncludeList,":",255);
   strmaxprep(IncludeList,NewPath,255);
END


        void RemoveIncludeList(char *RemPath)
BEGIN
   String Save;
   char *Part;

   strmaxcpy(IncludeList,Save,255); IncludeList[0]='\0';
   while (Save[0]!='\0')
    BEGIN
     Part=GetPath(Save);
     if (strcmp(Part,RemPath)!=0)
      BEGIN
       if (IncludeList[0]!='\0') strmaxcat(IncludeList,":",255);
       strmaxcat(IncludeList,Part,255);
      END
    END
END

/****************************************************************************/
/* Liste mit Ausgabedateien */

        void ClearOutList(void)
BEGIN
   ClearStringList(&OutList);
END

        void AddToOutList(char *NewName)
BEGIN
   AddStringListLast(&OutList,NewName);
END

        void RemoveFromOutList(char *OldName)
BEGIN
   RemoveStringList(&OutList,OldName);
END

        char *GetFromOutList(void)
BEGIN
   return GetAndCutStringList(&OutList);
END

/****************************************************************************/
/* Tokenverarbeitung */

	static Boolean CompressLine_NErl(char ch)
BEGIN
   return (((ch>='A') AND (ch<='Z')) OR ((ch>='a') AND (ch<='z')) OR ((ch>='0') AND (ch<='9')));
END

        void CompressLine(char *TokNam, Byte Num, char *Line)
BEGIN
   Integer z,e,tlen,llen;
   Boolean SFound;

   z=0; tlen=strlen(TokNam); llen=strlen(Line);
   while (z<=llen-tlen)
    BEGIN
     e=z+strlen(TokNam); 
     SFound=(CaseSensitive) ? (strncmp(Line+z,TokNam,tlen)==0)
                            : (strncasecmp(Line+z,TokNam,tlen)==0);
     if  ( (SFound) 
     AND   ((z==0) OR (NOT CompressLine_NErl(Line[z-1])))
     AND   ((e>=strlen(Line)) OR (NOT CompressLine_NErl(Line[e]))) )
      BEGIN
       strcpy(Line+z+1,Line+e); Line[z]=Num;
       llen=strlen(Line);
      END;
     z++; 
    END
END

        void ExpandLine(char *TokNam, Byte Num, char *Line)
BEGIN
    char *z;

    do
     BEGIN
      z=strchr(Line,Num);
      if (z!=Nil)
       BEGIN
        strcpy(z,z+1); 
        strmaxins(Line,TokNam,z-Line,255);
       END
     END
    while (z!=0);
END

        void KillCtrl(char *Line)
BEGIN
   char *z;
   
   if (*(z=Line)=='\0') return;
   do
    BEGIN
     if (*z=='\0');
     else if (*z==Char_HT)
      BEGIN
       strcpy(z,z+1);
       strprep(z,Blanks(8-((z-Line)%8)));
      END
     else if (*z<' ') *z=' ';
     z++;
    END
   while (*z!='\0');
END

/****************************************************************************/
/* Differenz zwischen zwei Zeiten mit Jahresueberlauf berechnen */

        long DTime(long t1, long t2)
BEGIN
   LongInt d;

   d=t2-t1; if (d<0) d+=(24*360000);
   return (d>0) ? d : -d;
END

/*--------------------------------------------------------------------------*/
/* Zeit holen */

#ifdef __MSDOS__

#include <dos.h>

        long GTime(void)
BEGIN
   struct time ti;
   struct date da;

   gettime(&ti); getdate (&da);
   return (dostounix(&da,&ti)*100)+ti.ti_hund;
END

#else

#include <sys/time.h>

        long GTime(void)
BEGIN
   struct timeval tv;
  
   gettimeofday(&tv,Nil);
   return (tv.tv_sec*100)+(tv.tv_usec/10000);
END

#endif
/**
{****************************************************************************}
{ Heapfehler abfedern }

        FUNCTION MyHeapError(Size:Word):Integer;
        Far;
 BEGIN
   IF Size<>0 THEN WrError(10006);
   MyHeapError:=1;
END;
**/
/*-------------------------------------------------------------------------*/
/* Stackfehler abfangen - bis auf DOS nur Dummies */

#ifdef __TURBOC__
unsigned _stklen=65520;
#include <malloc.h>
#endif

        void ChkStack(void)
BEGIN
#ifdef __TURBOC__
   LongWord avail=stackavail();
   if (avail<MinStack) WrError(10007);   
   if (avail<LowStack) LowStack=avail;
#endif
END

        void ResetStack(void)
BEGIN
#ifdef __TURBOC__
   LowStack=stackavail();
#endif
END

        LongWord StackRes(void)
BEGIN
#ifdef __TURBOC__
   return LowStack-MinStack;
#else
   return 0;
#endif
END

/****************************************************************************/
/**
{$IFDEF DPMI}
        FUNCTION MemInitSwapFile(FileName: pChar; FileSize: LongInt): INTEGER;
        EXTERNAL 'RTM' INDEX 35;

        FUNCTION MemCloseSwapFile(Delete: INTEGER): INTEGER;
        EXTERNAL 'RTM' INDEX 36;
{$ENDIF}

VAR
   Cnt:Char;
   FileLen:LongInt;
   p,err:Integer;
   MemFlag,TempName:String;**/

	void asmsub_init(void)
BEGIN
   char *CMess=InfoMessCopyright;
   Word z;
   LongWord XORVal;

/**
   { Fuer DPMI evtl. Swapfile anlegen }

{$IFDEF DPMI}
   MemFlag:=GetEnv('ASXSWAP');
   IF MemFlag<>'' THEN
    BEGIN
     p:=Pos(',',MemFlag);
     IF p=0 THEN TempName:='ASX.TMP'
     ELSE
      BEGIN
       TempName:=Copy(MemFlag,p+1,Length(MemFlag)-p);
       MemFlag:=Copy(MemFlag,1,p-1);
      END;
     KillBlanks(TempName); KillBlanks(MemFlag);
     TempName:=TempName+#0;
     Val(MemFlag,FileLen,Err);
     IF Err<>0 THEN
      BEGIN
       WriteLn(StdErr,ErrMsgInvSwapSize); Halt(4);
      END;
     IF MemInitSwapFile(@TempName[1],FileLen SHL 20)<>0 THEN
      BEGIN
       WriteLn(StdErr,ErrMsgSwapTooBig); Halt(4);
      END;
    END;
{$ENDIF}

   HeapError:=@MyHeapError;
**/

   for (z=0; z<strlen(CMess); z++)
    BEGIN
     XORVal=CMess[z];
     XORVal=XORVal << (((z+1) % 4)*8);
     Magic=Magic ^ XORVal;
    END

   InitStringList(&CopyrightList);
   InitStringList(&OutList);

#ifdef __TURBOC__
   StartStack=stackavail(); LowStack=stackavail();
   MinStack=StartStack-65520+0x800;
#else
   StartStack=LowStack=MinStack=0;
#endif
END

