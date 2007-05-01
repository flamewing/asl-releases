/* asmsub.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Unterfunktionen, vermischtes                                              */
/*                                                                           */
/* Historie:  4. 5.1996 Grundsteinlegung                                     */
/*           13. 8.1997 KillBlanks-Funktionen nach stringutil.c geschoben    */
/*           26. 6.1998 Fehlermeldung Codepage nicht gefunden                */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           17. 8.1998 Unterfunktion zur Buchhaltung Adressbereiche         */
/*            1. 9.1998 FloatString behandelte Sonderwerte nicht korrekt     */
/*           13. 9.1998 Prozessorliste macht Zeilenvorschub nach 6 Namen     */
/*           14.10.1998 Fehlerzeilen mit > > >                               */
/*           30. 1.1999 Formatstrings maschinenunabhaengig gemacht           */
/*           18. 4.1999 Ausgabeliste Sharefiles                              */
/*           13. 7.1999 Fehlermeldungen relokatible Symbole                  */
/*           13. 9.1999 I/O-Fehler 25 ignorieren                             */
/*            5.11.1999 ExtendErrors ist jetzt ShortInt                      */
/*           13. 2.2000 Ausgabeliste Listing                                 */
/*            6. 8.2000 added ValidSymChar array                             */
/*           21. 7.2001 added not repeatable message                         */
/*           2001-08-01 QuotPos also works for ) resp. ] characters          */
/*           2001-09-03 added warning message about X-indexed conversion     */
/*           2001-10-21 additions for GNU-style errors                       */
/*           2002-03-31 fixed operand order of memset                        */
/*                                                                           */
/*****************************************************************************/
/* $Id: asmsub.c,v 1.6 2007/04/30 18:37:52 alfred Exp $                      */
/*****************************************************************************
 * $Log: asmsub.c,v $
 * Revision 1.6  2007/04/30 18:37:52  alfred
 * - add weird integer coding
 *
 * Revision 1.5  2006/12/09 19:54:53  alfred
 * - remove unplausible part in time computation
 *
 * Revision 1.4  2005/10/02 10:00:44  alfred
 * - ConstLongInt gets default base, correct length check on KCPSM3 registers
 *
 * Revision 1.3  2004/10/03 11:44:58  alfred
 * - addition for MinGW
 *
 * Revision 1.2  2004/05/31 15:19:26  alfred
 * - add StrCaseCmp
 *
 * Revision 1.1  2003/11/06 02:49:19  alfred
 * - recreated
 *
 * Revision 1.12  2003/10/04 15:38:47  alfred
 * - differentiate constant/variable messages
 *
 * Revision 1.11  2003/10/04 14:00:39  alfred
 * - complain about empty arguments
 *
 * Revision 1.10  2003/09/21 21:15:54  alfred
 * - fix string length
 *
 * Revision 1.9  2003/05/20 17:45:03  alfred
 * - StrSym with length spec
 *
 * Revision 1.8  2003/05/02 21:23:09  alfred
 * - strlen() updates
 *
 * Revision 1.7  2002/11/16 20:52:18  alfred
 * - added ErrMsgStructNameMissing
 *
 * Revision 1.6  2002/11/04 19:04:26  alfred
 * - prevent modification of constants with SET
 *
 * Revision 1.5  2002/08/14 18:43:47  alfred
 * - warn null allocation, remove some warnings
 *
 * Revision 1.4  2002/05/13 18:17:13  alfred
 * - added error 2010/2020
 *
 * Revision 1.3  2002/05/12 20:56:28  alfred
 * - added 3206x error messages
 *
 *****************************************************************************/


#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "version.h"
#include "endian.h"
#include "stdhandl.h"
#include "nls.h"
#include "nlmessages.h"
#include "as.rsc"
#include "strutil.h"
#include "stringlists.h"
#include "chunks.h"
#include "ioerrs.h"
#include "asmdef.h"
#include "asmpars.h"
#include "asmdebug.h"
#include "as.h"

#include "asmsub.h"


#ifdef __TURBOC__
#ifdef __DPMI16__
#define STKSIZE 40960
#else
#define STKSIZE 49152
#endif
#endif

#define VALID_S1 1
#define VALID_SN 2
#define VALID_M1 4
#define VALID_MN 8

Word ErrorCount,WarnCount;
static StringList CopyrightList, OutList, ShareOutList, ListOutList;

static LongWord StartStack,MinStack,LowStack;

static Byte *ValidSymChar;

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
   int cnt;

   Lauf=FirstCPUDef; Proc=NullProc; cnt=0;
   while (Lauf!=Nil)
    BEGIN
     if (Lauf->Number==Lauf->Orig)
      BEGIN
       if ((Lauf->SwitchProc!=Proc) OR (cnt==7))
        BEGIN
         Proc=Lauf->SwitchProc; printf("\n"); NxtProc(); cnt=0;
        END
       printf("%-10s",Lauf->Name); cnt++;
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

#if 0
	char *QuotPos(char *s, char Zeichen)
BEGIN
   register int Cnt=0;
   register char *i;
   register char ch,Cmp2,Cmp3;

   for (i=s; (ch=*i)!='\0'; i++)
    if (Cnt==0)
     BEGIN
      if (ch==Zeichen) return i;
      else switch (ch)
       BEGIN
        case '"':
        case '\'': Cmp2='\0'; Cmp3=ch; Cnt=1; break;
        case '(': Cmp2='('; Cmp3=')'; Cnt=1; break;
        case '[': Cmp2='['; Cmp3=']'; Cnt=1; break;
       END
     END
    else
     BEGIN
      if (ch==Cmp2) Cnt++;
      else if (ch==Cmp3) Cnt--;
     END

   return Nil;
END
#else
        char *QuotPos(char *s, char Zeichen)
BEGIN
   register ShortInt Brack=0,AngBrack=0;
   register char *i;
   register LongWord Flag=0;
   static Boolean First=True,Imp[256],Save;

   if (First)
    BEGIN
     memset(Imp,False,256);
     Imp['"']=Imp['\'']=Imp['(']=Imp[')']=Imp['[']=Imp[']']=True;
     First=False;
    END
   
   Save=Imp[(unsigned char)Zeichen]; Imp[(unsigned char)Zeichen]=True;
   for (i=s; *i!='\0'; i++)
    if (Imp[(unsigned char)*i])
     BEGIN
      if (*i==Zeichen)
       BEGIN
        if ((AngBrack|Brack|Flag)==0)
         { Imp[(unsigned char)Zeichen]=Save; return i;}
       END
      switch(*i)
       BEGIN
        case '"': if (((Brack|AngBrack)==0) AND ((Flag&2)==0)) Flag^=1; break;
        case '\'':if (((Brack|AngBrack)==0) AND ((Flag&1)==0)) Flag^=2; break;
        case '(': if ((AngBrack|Flag)==0) Brack++; break;
        case ')': if ((AngBrack|Flag)==0) Brack--; break;
        case '[': if ((Brack|Flag)==0) AngBrack++; break;
        case ']': if ((Brack|Flag)==0) AngBrack--; break;
       END
     END

   Imp[(unsigned char)Zeichen]=Save; return Nil;    
END
#endif
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
   int hypquot=0;

   for (z=s; *z!='\0'; z++)
    BEGIN
     if ((*z=='\'') AND ((hypquot&2)==0)) hypquot^=1;
     else if ((*z=='"') AND ((hypquot&1)==0)) hypquot^=2;
     else if (hypquot==0) *z=UpCaseTable[(int)*z];
    END
END

/****************************************************************************/

        void TranslateString(char *s)
BEGIN
   char *z;

   for (z=s; *z!='\0'; z++) *z=CharTransTable[((usint)(*z))&0xff];
END

ShortInt StrCmp(const char *s1, const char *s2, LongInt Hand1, LongInt Hand2)
{
  int tmp;

  tmp = (*s1) - (*s2);
  if (tmp == 0) tmp = strcmp(s1,s2);
  if (tmp == 0) tmp = Hand1 - Hand2;
  if (tmp < 0) return -1;
  if (tmp > 0) return 1;
  return 0;
}

ShortInt StrCaseCmp(const char *s1, const char *s2, LongInt Hand1, LongInt Hand2)
{
  int tmp;

  tmp = toupper(*s1) - toupper(*s2);
  if (tmp == 0) tmp = strcasecmp(s1,s2);
  if (tmp == 0) tmp = Hand1 - Hand2;
  if (tmp < 0) return -1;
  if (tmp > 0) return 1;
  return 0;
}

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
   sint n,ExpVal,nzeroes;
   Boolean WithE,OK;

   /* 1. mit Maximallaenge wandeln, fuehrendes Vorzeichen weg */

   sprintf(s,"%27.15e",f); 
   for (p=s; (*p==' ') OR (*p=='+'); p++);
   if (p!=s) strcpy(s,p);

   /* 2. Exponenten soweit als moeglich kuerzen, evtl. ganz streichen */

   p=strchr(s,'e'); 
   if (p==Nil) return s;
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
     ExpVal=ConstLongInt(p + 1, &OK, 10);
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

void StrSym(TempResult *t, Boolean WithSystem, char *Dest, int DestLen)
{
  switch (t->Typ)
  {
    case TempInt:
      strmaxcpy(Dest, HexString(t->Contents.Int,1), DestLen - 3);
      if (WithSystem)
        switch (ConstMode)
        {
          case ConstModeIntel : strcat(Dest,"H"); break;
          case ConstModeMoto  : strprep(Dest,"$"); break;
          case ConstModeC     : strprep(Dest,"0x"); break;
          case ConstModeWeird : strprep(Dest,"x'"); strcat(Dest, "'"); break;
        }
      break;
    case TempFloat:
      strmaxcpy(Dest, FloatString(t->Contents.Float), DestLen);
      break;
    case TempString:
      strmaxcpy(Dest, t->Contents.Ascii, DestLen);
      break;
    default:
      strmaxcpy(Dest, "???", DestLen);
  }
}

/****************************************************************************/
/* Listingzaehler zuruecksetzen */

        void ResetPageCounter(void)
BEGIN
   int z;

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

   sprintf(Header," AS V%s%s%s",Version,getmessage(Num_HeadingFileNameLab),NamePart(SourceFile));
   if ((strcmp(CurrFileName,"INTERNAL")!=0) AND (strcmp(NamePart(CurrFileName),NamePart(SourceFile))!=0))
    BEGIN
     strmaxcat(Header,"(",255);
     strmaxcat(Header,NamePart(CurrFileName),255);
     strmaxcat(Header,")",255);
    END
   strmaxcat(Header,getmessage(Num_HeadingPageLab),255);

   for (z=ChapDepth; z>=0; z--)
    BEGIN
     sprintf(s, IntegerFormat, PageCounter[z]);
     strmaxcat(Header,s,255);
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
   int LLength;
   char bbuf[2500];
   String LLine;
   int blen=0,hlen,z,Start;

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
       for (z=0; z<(int)strlen(Line);  z++)
        if (Line[z]==Char_HT)
         BEGIN
          memset(bbuf+blen, ' ', 8-(blen&7));
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
{
   StrSym(t, True, ListLine, STRINGSIZE);
   strmaxprep(ListLine, "=", STRINGSIZE - 1);
   if (strlen(ListLine) > 14)
   {
     ListLine[12] = '\0';
     strmaxcat(ListLine, "..", STRINGSIZE - 1);
   }
}

/****************************************************************************/
/* einen Symbolnamen auf Gueltigkeit ueberpruefen */

        Boolean ChkSymbName(char *sym)
BEGIN
   char *z;

   if (!(ValidSymChar[((unsigned int) *sym) & 0xff] & VALID_S1))
    return False;

   for (z = sym + 1; *z != '\0'; z++)
    if (!(ValidSymChar[((unsigned int) *z) & 0xff] & VALID_SN))
     return False;

   return True;
END

        Boolean ChkMacSymbName(char *sym)
BEGIN
   char *z;

   if (!(ValidSymChar[((unsigned int) *sym) & 0xff] & VALID_M1))
    return False;

   for (z = sym + 1; *z != '\0'; z++)
    if (!(ValidSymChar[((unsigned int) *z) & 0xff] & VALID_MN))
     return False;

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
   char *p;
   FILE *errfile;
   int l;

   *h = '\0';
   if (!GNUErrors)
     strcpy(h,"> > >");
   p = GetErrorPos();
   if (p[l = strlen(p) - 1] == ' ')
     p[l] = '\0';
   strmaxcat(h, p, 255); free(p);
   if (Warning)
    BEGIN
     strmaxcat(h,": ",255);
     strmaxcat(h,getmessage(Num_WarnName),255);
     strmaxcat(h,Add,255);
     strmaxcat(h,": ",255);
     WarnCount++;
    END
   else
    BEGIN
     if (!GNUErrors)
     {
       strmaxcat(h,": ",255);
       strmaxcat(h, getmessage(Num_ErrName), 255); 
     }
     strmaxcat(h,Add,255);
     strmaxcat(h,": ",255);
     ErrorCount++;
    END

   if ((strcmp(LstName, "/dev/null") != 0) AND (NOT Fatal))
    BEGIN
     strmaxcpy(h2, h, 255); strmaxcat(h2, Message, 255); WrLstLine(h2);
     if ((ExtendErrors > 0) AND (*ExtendError != '\0'))
      BEGIN
       sprintf(h2, "> > > %s", ExtendError); WrLstLine(h2);
      END
     if (ExtendErrors > 1)
      BEGIN
       sprintf(h2, "> > > %s", OneLine); WrLstLine(h2);
      END
    END

   ForceErrorOpen();
   if ((strcmp(LstName, "!1")!=0) OR (Fatal))
    BEGIN
     errfile = (ErrorFile == Nil) ? stdout : ErrorFile;
     fprintf(errfile, "%s%s%s\n", h, Message, ClrEol);
     if ((ExtendErrors > 0) AND (*ExtendError != '\0'))
      fprintf(errfile, "> > > %s%s\n", ExtendError, ClrEol);
     if (ExtendErrors > 1)
      fprintf(errfile, "> > > %s%s\n", OneLine, ClrEol);
    END
   *ExtendError = '\0';

   if (Fatal)
    BEGIN
     fprintf((ErrorFile==Nil)?stdout:ErrorFile,"%s\n",getmessage(Num_ErrMsgIsFatal));
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
   int msgno;
   
   if ((NOT CodeOutput) AND (Num==1200)) return;

   if ((SuppWarns) AND (Num<1000)) return;

   switch (Num)
    BEGIN
     case    0: msgno = Num_ErrMsgUselessDisp; break;
     case   10: msgno = Num_ErrMsgShortAddrPossible; break;
     case   20: msgno = Num_ErrMsgShortJumpPossible; break;
     case   30: msgno = Num_ErrMsgNoShareFile; break;
     case   40: msgno = Num_ErrMsgBigDecFloat; break;
     case   50: msgno = Num_ErrMsgPrivOrder; break;
     case   60: msgno = Num_ErrMsgDistNull; break;
     case   70: msgno = Num_ErrMsgWrongSegment; break;
     case   75: msgno = Num_ErrMsgInAccSegment; break;
     case   80: msgno = Num_ErrMsgPhaseErr; break;
     case   90: msgno = Num_ErrMsgOverlap; break;
     case  100: msgno = Num_ErrMsgNoCaseHit; break;
     case  110: msgno = Num_ErrMsgInAccPage; break;
     case  120: msgno = Num_ErrMsgRMustBeEven; break;
     case  130: msgno = Num_ErrMsgObsolete; break;
     case  140: msgno = Num_ErrMsgUnpredictable; break;
     case  150: msgno = Num_ErrMsgAlphaNoSense; break;
     case  160: msgno = Num_ErrMsgSenseless; break;
     case  170: msgno = Num_ErrMsgRepassUnknown; break;
     case  180: msgno = Num_ErrMsgAddrNotAligned; break;
     case  190: msgno = Num_ErrMsgIOAddrNotAllowed; break;
     case  200: msgno = Num_ErrMsgPipeline; break;
     case  210: msgno = Num_ErrMsgDoubleAdrRegUse; break;
     case  220: msgno = Num_ErrMsgNotBitAddressable; break;
     case  230: msgno = Num_ErrMsgStackNotEmpty; break;
     case  240: msgno = Num_ErrMsgNULCharacter; break;
     case  250: msgno = Num_ErrMsgPageCrossing; break;
     case  260: msgno = Num_ErrMsgWOverRange; break;
     case  270: msgno = Num_ErrMsgNegDUP; break;
     case  280: msgno = Num_ErrMsgConvIndX; break;
     case  290: msgno = Num_ErrMsgNullResMem; break;
     case 1000: msgno = Num_ErrMsgDoubleDef; break;
     case 1010: msgno = Num_ErrMsgSymbolUndef; break;
     case 1020: msgno = Num_ErrMsgInvSymName; break;
     case 1090: msgno = Num_ErrMsgInvFormat; break;
     case 1100: msgno = Num_ErrMsgUseLessAttr; break;
     case 1105: msgno = Num_ErrMsgTooLongAttr; break;
     case 1107: msgno = Num_ErrMsgUndefAttr; break;
     case 1110: msgno = Num_ErrMsgWrongArgCnt; break;
     case 1115: msgno = Num_ErrMsgWrongOptCnt; break;
     case 1120: msgno = Num_ErrMsgOnlyImmAddr; break;
     case 1130: msgno = Num_ErrMsgInvOpsize; break;
     case 1131: msgno = Num_ErrMsgConfOpSizes; break;
     case 1132: msgno = Num_ErrMsgUndefOpSizes; break;
     case 1135: msgno = Num_ErrMsgInvOpType; break;
     case 1140: msgno = Num_ErrMsgTooMuchArgs; break;
     case 1150: msgno = Num_ErrMsgNoRelocs; break;
     case 1155: msgno = Num_ErrMsgUnresRelocs; break;
     case 1156: msgno = Num_ErrMsgUnexportable; break;
     case 1200: msgno = Num_ErrMsgUnknownOpcode; break;
     case 1300: msgno = Num_ErrMsgBrackErr; break;
     case 1310: msgno = Num_ErrMsgDivByZero; break;
     case 1315: msgno = Num_ErrMsgUnderRange; break;
     case 1320: msgno = Num_ErrMsgOverRange; break;
     case 1325: msgno = Num_ErrMsgNotAligned; break;
     case 1330: msgno = Num_ErrMsgDistTooBig; break;
     case 1335: msgno = Num_ErrMsgInAccReg; break;
     case 1340: msgno = Num_ErrMsgNoShortAddr; break;
     case 1350: msgno = Num_ErrMsgInvAddrMode; break;
     case 1351: msgno = Num_ErrMsgMustBeEven; break;
     case 1355: msgno = Num_ErrMsgInvParAddrMode; break;
     case 1360: msgno = Num_ErrMsgUndefCond; break;
     case 1365: msgno = Num_ErrMsgIncompCond; break;
     case 1370: msgno = Num_ErrMsgJmpDistTooBig; break;
     case 1375: msgno = Num_ErrMsgDistIsOdd; break;
     case 1380: msgno = Num_ErrMsgInvShiftArg; break;
     case 1390: msgno = Num_ErrMsgRange18; break;
     case 1400: msgno = Num_ErrMsgShiftCntTooBig; break;
     case 1410: msgno = Num_ErrMsgInvRegList; break;
     case 1420: msgno = Num_ErrMsgInvCmpMode; break;
     case 1430: msgno = Num_ErrMsgInvCPUType; break;
     case 1440: msgno = Num_ErrMsgInvCtrlReg; break;
     case 1445: msgno = Num_ErrMsgInvReg; break;
     case 1450: msgno = Num_ErrMsgNoSaveFrame; break;
     case 1460: msgno = Num_ErrMsgNoRestoreFrame; break;
     case 1465: msgno = Num_ErrMsgUnknownMacArg; break;
     case 1470: msgno = Num_ErrMsgMissEndif; break;
     case 1480: msgno = Num_ErrMsgInvIfConst; break;
     case 1483: msgno = Num_ErrMsgDoubleSection; break;
     case 1484: msgno = Num_ErrMsgInvSection; break;
     case 1485: msgno = Num_ErrMsgMissingEndSect; break;
     case 1486: msgno = Num_ErrMsgWrongEndSect; break;
     case 1487: msgno = Num_ErrMsgNotInSection; break;
     case 1488: msgno = Num_ErrMsgUndefdForward; break;
     case 1489: msgno = Num_ErrMsgContForward; break;
     case 1490: msgno = Num_ErrMsgInvFuncArgCnt; break;
     case 1495: msgno = Num_ErrMsgMissingLTORG; break;
     case 1500: msgno = -1;
                sprintf(h,"%s%s%s",getmessage(Num_ErrMsgNotOnThisCPU1), 
                        MomCPUIdent,getmessage(Num_ErrMsgNotOnThisCPU2));
                break;
     case 1505: msgno = -1;
                sprintf(h,"%s%s%s",getmessage(Num_ErrMsgNotOnThisCPU3),
                        MomCPUIdent,getmessage(Num_ErrMsgNotOnThisCPU2));
                break;
     case 1510: msgno = Num_ErrMsgInvBitPos; break;
     case 1520: msgno = Num_ErrMsgOnlyOnOff; break;
     case 1530: msgno = Num_ErrMsgStackEmpty; break;
     case 1540: msgno = Num_ErrMsgNotOneBit; break;
     case 1550: msgno = Num_ErrMsgMissingStruct; break;
     case 1551: msgno = Num_ErrMsgOpenStruct; break;
     case 1552: msgno = Num_ErrMsgWrongStruct; break;
     case 1553: msgno = Num_ErrMsgPhaseDisallowed; break;
     case 1554: msgno = Num_ErrMsgInvStructDir; break;
     case 1560: msgno = Num_ErrMsgNotRepeatable; break;
     case 1600: msgno = Num_ErrMsgShortRead; break;
     case 1610: msgno = Num_ErrMsgUnknownCodepage; break;
     case 1700: msgno = Num_ErrMsgRomOffs063; break;
     case 1710: msgno = Num_ErrMsgInvFCode; break;
     case 1720: msgno = Num_ErrMsgInvFMask; break;
     case 1730: msgno = Num_ErrMsgInvMMUReg; break;
     case 1740: msgno = Num_ErrMsgLevel07; break;
     case 1750: msgno = Num_ErrMsgInvBitMask; break;
     case 1760: msgno = Num_ErrMsgInvRegPair; break;
     case 1800: msgno = Num_ErrMsgOpenMacro; break;
     case 1805: msgno = Num_ErrMsgEXITMOutsideMacro; break;
     case 1810: msgno = Num_ErrMsgTooManyMacParams; break;
     case 1815: msgno = Num_ErrMsgDoubleMacro; break;
     case 1820: msgno = Num_ErrMsgFirstPassCalc; break;
     case 1830: msgno = Num_ErrMsgTooManyNestedIfs; break;
     case 1840: msgno = Num_ErrMsgMissingIf; break;
     case 1850: msgno = Num_ErrMsgRekMacro; break;
     case 1860: msgno = Num_ErrMsgUnknownFunc; break;
     case 1870: msgno = Num_ErrMsgInvFuncArg; break;
     case 1880: msgno = Num_ErrMsgFloatOverflow; break;
     case 1890: msgno = Num_ErrMsgInvArgPair; break;
     case 1900: msgno = Num_ErrMsgNotOnThisAddress; break;
     case 1905: msgno = Num_ErrMsgNotFromThisAddress; break;
     case 1910: msgno = Num_ErrMsgTargOnDiffPage; break;
     case 1920: msgno = Num_ErrMsgCodeOverflow; break;
     case 1925: msgno = Num_ErrMsgAdrOverflow; break;
     case 1930: msgno = Num_ErrMsgMixDBDS; break;
     case 1940: msgno = Num_ErrMsgNotInStruct; break;
     case 1950: msgno = Num_ErrMsgParNotPossible; break;
     case 1960: msgno = Num_ErrMsgInvSegment; break;
     case 1961: msgno = Num_ErrMsgUnknownSegment; break;
     case 1962: msgno = Num_ErrMsgUnknownSegReg; break;
     case 1970: msgno = Num_ErrMsgInvString; break;
     case 1980: msgno = Num_ErrMsgInvRegName; break;
     case 1985: msgno = Num_ErrMsgInvArg; break;
     case 1990: msgno = Num_ErrMsgNoIndir; break;
     case 1995: msgno = Num_ErrMsgNotInThisSegment; break;
     case 1996: msgno = Num_ErrMsgNotInMaxmode; break;
     case 1997: msgno = Num_ErrMsgOnlyInMaxmode; break;
     case 2000: msgno = Num_ErrMsgPackCrossBoundary; break;
     case 2001: msgno = Num_ErrMsgUnitMultipleUsed; break;
     case 2002: msgno = Num_ErrMsgMultipleLongRead; break;
     case 2003: msgno = Num_ErrMsgMultipleLongWrite; break;
     case 2004: msgno = Num_ErrMsgLongReadWithStore; break;
     case 2005: msgno = Num_ErrMsgTooManyRegisterReads; break;
     case 2006: msgno = Num_ErrMsgOverlapDests; break;
     case 2008: msgno = Num_ErrMsgTooManyBranchesInExPacket; break;
     case 2009: msgno = Num_ErrMsgCannotUseUnit; break;
     case 2010: msgno = Num_ErrMsgInvEscSequence; break;
     case 2020: msgno = Num_ErrMsgInvPrefixCombination; break;
     case 2030: msgno = Num_ErrConstantRedefinedAsVariable; break;
     case 2035: msgno = Num_ErrVariableRedefinedAsConstant; break;
     case 2040: msgno = Num_ErrMsgStructNameMissing; break;
     case 2050: msgno =  Num_ErrMsgEmptyArgument; break;
     case 10001: msgno = Num_ErrMsgOpeningFile; break;
     case 10002: msgno = Num_ErrMsgListWrError; break;
     case 10003: msgno = Num_ErrMsgFileReadError; break;
     case 10004: msgno = Num_ErrMsgFileWriteError; break;
     case 10006: msgno = Num_ErrMsgHeapOvfl; break;
     case 10007: msgno = Num_ErrMsgStackOvfl; break;
     default  : msgno= -1;
                sprintf(h,"%s %d",getmessage(Num_ErrMsgIntError),(int) Num);
    END
   if (msgno!=-1) strmaxcpy(h,getmessage(msgno),255);   

   if (((Num==1910) OR (Num==1370)) AND (NOT Repass)) JmpErrors++;

   if (NumericErrors) sprintf(Add," #%d", (int)Num); 
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

   io=errno; if ((io == 0) OR (io == 19) OR (io == 25)) return;

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
   int BufferZ;
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
   int z,z2,l;
   String s;

   for (z=1; z<=PCMax; z++)
    if (SegChunks[z].Chunks!=Nil)
     BEGIN
      sprintf(s,"  %s%s%s",getmessage(Num_ListSegListHead1),SegNames[z],
                           getmessage(Num_ListSegListHead2));
      WrLstLine(s);
      strcpy(s,"  ");
      l=strlen(SegNames[z])+strlen(getmessage(Num_ListSegListHead1))+strlen(getmessage(Num_ListSegListHead2));
      for (z2=0; z2<l; z2++) strmaxcat(s,"-",255);
      WrLstLine(s);
      WrLstLine("");
      PrintChunk(SegChunks+z);
      WrLstLine("");
     END
END

        void ClearUseList(void)
BEGIN
   int z;

   for (z=1; z<=PCMax; z++)
    ClearChunk(SegChunks+z);
END

/****************************************************************************/
/* Include-Pfadlistenverarbeitung */

        static char *GetPath(char *Acc)
BEGIN
   char *p;
   static String tmp;

   p=strchr(Acc,DIRSEP);
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
   if (*IncludeList!='\0') strmaxprep(IncludeList,SDIRSEP,255);
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
       if (IncludeList[0]!='\0') strmaxcat(IncludeList,SDIRSEP,255);
       strmaxcat(IncludeList,Part,255);
      END
    END
END

/****************************************************************************/
/* Listen mit Ausgabedateien */

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

        void ClearShareOutList(void)
BEGIN
   ClearStringList(&ShareOutList);
END

        void AddToShareOutList(char *NewName)
BEGIN
   AddStringListLast(&ShareOutList,NewName);
END

        void RemoveFromShareOutList(char *OldName)
BEGIN
   RemoveStringList(&ShareOutList,OldName);
END

        char *GetFromShareOutList(void)
BEGIN
   return GetAndCutStringList(&ShareOutList);
END

        void ClearListOutList(void)
BEGIN
   ClearStringList(&ListOutList);
END

        void AddToListOutList(char *NewName)
BEGIN
   AddStringListLast(&ListOutList,NewName);
END

        void RemoveFromListOutList(char *OldName)
BEGIN
   RemoveStringList(&ListOutList,OldName);
END

        char *GetFromListOutList(void)
BEGIN
   return GetAndCutStringList(&ListOutList);
END

/****************************************************************************/
/* Tokenverarbeitung */

	static Boolean CompressLine_NErl(char ch)
BEGIN
   return (((ch>='A') AND (ch<='Z')) OR ((ch>='a') AND (ch<='z')) OR ((ch>='0') AND (ch<='9')));
END

        void CompressLine(char *TokNam, Byte Num, char *Line)
BEGIN
   int z,e,tlen,llen;
   Boolean SFound;

   z=0; tlen=strlen(TokNam); llen=strlen(Line);
   while (z<=llen-tlen)
    BEGIN
     e=z+strlen(TokNam); 
     SFound=(CaseSensitive) ? (strncmp(Line+z,TokNam,tlen)==0)
                            : (strncasecmp(Line+z,TokNam,tlen)==0);
     if  ( (SFound) 
     AND   ((z==0) OR (NOT CompressLine_NErl(Line[z-1])))
     AND   ((e>=(int)strlen(Line)) OR (NOT CompressLine_NErl(Line[e]))) )
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
     else if ((*z&0xe0)==0) *z=' ';
     z++;
    END
   while (*z!='\0');
END

/****************************************************************************/
/* Buchhaltung */

	void BookKeeping(void)
BEGIN
   if (MakeUseList)
    if (AddChunk(SegChunks+ActPC,ProgCounter(),CodeLen,ActPC==SegCode)) WrError(90);
   if (DebugMode!=DebugNone)
    BEGIN
     AddSectionUsage(ProgCounter(),CodeLen);
     AddLineInfo(InMacroFlag,CurrLine,CurrFileName,ActPC,PCs[ActPC],CodeLen);
    END
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
   static unsigned long *tick=MK_FP(0x40,0x6c);
   double tmp=*tick;

   return ((long) (tmp*5.4931641));
END

#elif __IBMC__

#include <time.h>
#define INCL_DOSDATETIME
#include <os2.h>

        long GTime(void)
BEGINM
   DATETIME dt;
   struct tm ts;
   DosGetDateTime(&dt);
   memset(&ts,0,sizeof(ts));
   ts.tm_year = dt.year-1900;
   ts.tm_mon  = dt.month-1;  
   ts.tm_mday = dt.day;
   ts.tm_hour = dt.hours;
   ts.tm_min  = dt.minutes;
   ts.tm_sec  = dt.seconds;
   return (mktime(&ts)*100)+(dt.hundredths);
END

#elif __MINGW32__

/* distribution by Gunnar Wallmann */

#include <sys/time.h>

/*time from 1 Jan 1601 to 1 Jan 1970 in 100ns units */
#define _W32_FT_OFFSET (116444736000000000LL)

typedef struct _FILETIME
        {
          unsigned long dwLowDateTime;
          unsigned long dwHighDateTime;
        } FILETIME;

void __stdcall GetSystemTimeAsFileTime(FILETIME*);

long GTime(void)
{
  union
  {
    long long ns100; /*time since 1 Jan 1601 in 100ns units */
    FILETIME ft;
  } _now;

  GetSystemTimeAsFileTime(&(_now.ft));
  return
      (_now.ns100 - _W32_FT_OFFSET) / 100000LL
#if 0
      + ((_now.ns100 / 10) % 1000000LL) / 10000
#endif
      ;
}

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

#ifdef __DPMI16__
#else
unsigned _stklen=STKSIZE;
unsigned _ovrbuffer=64*48;
#endif
#include <malloc.h>

        void ChkStack(void)
BEGIN
   LongWord avail=stackavail();
   if (avail<MinStack) WrError(10007);   
   if (avail<LowStack) LowStack=avail;
END

        void ResetStack(void)
BEGIN
   LowStack=stackavail();
END

        LongWord StackRes(void)
BEGIN
   return LowStack-MinStack;
END
#endif /* __TURBOC__ */

#ifdef CKMALLOC
#undef malloc
#undef realloc

        void *ckmalloc(size_t s)
BEGIN
   void *tmp=malloc(s);
   if (tmp==NULL) WrError(10006);
   return tmp;
END

        void *ckrealloc(void *p, size_t s)
BEGIN
   void *tmp=realloc(p,s);
   if (tmp==NULL) WrError(10006);
   return tmp;
END
#endif

	void asmsub_init(void)
BEGIN
   Word z;

#ifdef __TURBOC__
#ifdef __MSDOS__
#ifdef __DPMI16__
   char *MemFlag,*p;
   String MemVal,TempName;
   unsigned long FileLen;
#else
   char *envval;
   int ovrerg;
#endif
#endif
#endif

   InitStringList(&CopyrightList);
   InitStringList(&OutList);
   InitStringList(&ShareOutList);
   InitStringList(&ListOutList);

#ifdef __TURBOC__
#ifdef __MSDOS__
#ifdef __DPMI16__
   /* Fuer DPMI evtl. Swapfile anlegen */

   MemFlag=getenv("ASXSWAP");
   if (MemFlag!=Nil)
    BEGIN
     strmaxcpy(MemVal,MemFlag,255);
     p=strchr(MemVal,',');
     if (p==Nil) strcpy(TempName,"ASX.TMP");
     else
      BEGIN
       *p=Nil; strcpy(TempName,MemVal);
       strcpy(MemVal,p+1);
      END;
     KillBlanks(TempName); KillBlanks(MemVal);
     FileLen=strtol(MemFlag,&p,0);
     if (*p!='\0')
      BEGIN
       fputs(getmessage(Num_ErrMsgInvSwapSize),stderr); exit(4);
      END;
     if (MEMinitSwapFile(TempName,FileLen << 20)!=RTM_OK)
      BEGIN
       fputs(getmessage(Num_ErrMsgSwapTooBig),stderr); exit(4);
      END
    END
#else
   /* Bei DOS Auslagerung Overlays in XMS/EMS versuchen */

   envval=getenv("USEXMS");
   if ((envval!=Nil) AND (toupper(*envval)=='N')) ovrerg=-1;
   else ovrerg=_OvrInitExt(0,0);
   if (ovrerg!=0)
    BEGIN
     envval=getenv("USEEMS");
     if ((envval==Nil) OR (toupper(*envval)!='N')) _OvrInitEms(0,0,0);
    END
#endif
#endif
#endif

#ifdef __TURBOC__
   StartStack=stackavail(); LowStack=stackavail();
   MinStack=StartStack-STKSIZE+0x800;
#else
   StartStack=LowStack=MinStack=0;
#endif

   /* initialize array of valid characters */

   ValidSymChar = (Byte*) malloc(sizeof(Byte) * 256);
   memset(ValidSymChar, 0, sizeof(Byte) * 256);
   for (z = 'a'; z <= 'z'; z++)
     ValidSymChar[z] = VALID_S1 | VALID_SN | VALID_M1 | VALID_MN;
   for (z = 'A'; z <= 'Z'; z++)
     ValidSymChar[z] = VALID_S1 | VALID_SN | VALID_M1 | VALID_MN;
   for (z = '0'; z <= '9'; z++)
     ValidSymChar[z] =            VALID_SN |            VALID_MN;
   ValidSymChar[(unsigned int) '.'] = VALID_S1 | VALID_SN;
   ValidSymChar[(unsigned int) '_'] = VALID_S1 | VALID_SN;
#if (defined CHARSET_IBM437) || (defined CHARSET_IBM850)
   for (z = 128; z <= 165; z++)
     ValidSymChar[z] = VALID_S1 | VALID_SN;
   ValidSymChar[225] = VALID_S1 | VALID_SN;
#elif defined CHARSET_ISO8859_1
   for (z = 192; z <= 255; z++)
     ValidSymChar[z] = VALID_S1 | VALID_SN;
#elif defined CHARSET_ASCII7
#else
#error Oops, unkown charset - you will have to add some work here...
#endif

   version_init();
END

