/* strutil.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* haeufig benoetigte String-Funktionen                                      */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*           13. 8.1997 KillBlanks-Funktionen aus asmsub.c heruebergenommen  */
/*           29. 8.1998 sprintf-Emulation                                    */
/*           17. 4.1999 strcasecmp gegen Nullzeiger geschuetzt               */
/*           30. 5.1999 ConstLongInt akzeptiert auf 0x fuer Hex-Zahlen       */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "strutil.h"
#undef strlen   /* VORSICHT, Rekursion!!! */
#ifdef BROKEN_SPRINTF
#undef sprintf
#endif

Boolean HexLowerCase;	    /* Hex-Konstanten mit Kleinbuchstaben */

/*--------------------------------------------------------------------------*/
/* eine bestimmte Anzahl Leerzeichen liefern */

        char *Blanks(int cnt)
BEGIN
   static char *BlkStr="                                                                                                           ";

   if (cnt<0) cnt=0;

   return BlkStr+(strlen(BlkStr)-cnt);
END


/****************************************************************************/
/* eine Integerzahl in eine Hexstring umsetzen. Weitere vordere Stellen als */
/* Nullen */

#define BufferCnt 8

	char *HexString(LargeWord i, Byte Stellen)
BEGIN
   static ShortString h[BufferCnt];
   static int z = 0, Cnt;
   LargeWord digit;
   char *ptr;

   if (Stellen > sizeof(ShortString) - 1)
    Stellen = sizeof(ShortString) - 1;

   ptr = h[z] + sizeof(ShortString) - 1;
   *ptr = '\0'; Cnt = Stellen;
   do
    BEGIN
     digit = i & 15;
     if (digit < 10)
      *(--ptr) = digit + '0';
     else if (HexLowerCase)
      *(--ptr) = digit - 10 + 'a';
     else
      *(--ptr) = digit - 10 + 'A';
     i = i >> 4;
     Cnt--;
    END
   while ((Cnt > 0) OR (i != 0));

   z = (z + 1) % BufferCnt;

   return ptr;
END

	char *SysString(LargeWord i, LargeWord System, Byte Stellen)
BEGIN
   static ShortString h[BufferCnt];
   static int z = 0, Cnt;
   LargeWord digit;
   char *ptr;

   if (Stellen > sizeof(ShortString) - 1)
    Stellen = sizeof(ShortString) - 1;

   ptr = h[z] + sizeof(ShortString) - 1;
   *ptr = '\0'; Cnt = Stellen;
   do
    BEGIN
     digit = i % System;
     if (digit < 10)
      *(--ptr) = digit + '0';
     else if (HexLowerCase)
      *(--ptr) = digit - 10 + 'a';
     else
      *(--ptr) = digit - 10 + 'A';
     i /= System;
     Cnt--;
    END
   while ((Cnt > 0) OR (i != 0));

   z = (z + 1) % BufferCnt;

   return ptr;
END

/*--------------------------------------------------------------------------*/
/* dito, nur vorne Leerzeichen */

        char *HexBlankString(LargeWord i, Byte Stellen)
BEGIN
   static String temp;
   
   strmaxcpy(temp,HexString(i,0),255);
   if (strlen(temp)<Stellen) strmaxprep(temp,Blanks(Stellen-strlen(temp)),255);
   return temp;
END

/*---------------------------------------------------------------------------*/
/* Da manche Systeme (SunOS) Probleme mit der Ausgabe extra langer Ints 
   haben, machen wir das jetzt zu Fuss :-( */

	char *LargeString(LargeInt i)
BEGIN
   Boolean SignFlag=False;
   static String s;
   String tmp;
   char *p,*p2;
   
   if (i<0)
    BEGIN
     i=(-i); SignFlag=True;
    END
    
   p=tmp;
   do
    BEGIN
     *(p++)='0'+(i%10);
     i/=10;
    END
   while (i>0);
   
   p2=s; if (SignFlag) *(p2++)='-';
   while (p>tmp) *(p2++)=(*(--p));
   *p2='\0'; return s;
END


/*---------------------------------------------------------------------------*/
/* manche haben's nicht... */

#if defined(NEEDS_STRDUP) || defined(CKMALLOC)
#ifdef CKMALLOC
	char *mystrdup(char *s)
BEGIN
#else
	char *strdup(char *s)
BEGIN
#endif
   char *ptr=(char *) malloc(strlen(s)+1);
#ifdef CKMALLOC
   if (ptr==Nil) 
    BEGIN
     fprintf(stderr,"strdup: out of memory?\n"); exit(255);
    END
#endif
   if (ptr!=0) strcpy(ptr,s);
   return ptr;
END
#endif

#ifdef NEEDS_CASECMP
	int strcasecmp(const char *src1, const char *src2)
BEGIN
   if (src1 == NULL) src1 = "";
   if (src2 == NULL) src2 = "";
   while (toupper(*src1)==toupper(*src2))
    BEGIN
     if ((NOT *src1) AND (NOT *src2)) return 0;
     src1++; src2++;
    END
   return ((int) toupper(*src1))-((int) toupper(*src2));
END	

	int strncasecmp(const char *src1, const char *src2, int len)
BEGIN
   if (src1 == NULL) src1 = "";
   if (src2 == NULL) src2 = "";
   while (toupper(*src1)==toupper(*src2))
    BEGIN
     if (--len==0) return 0;
     if ((NOT *src1) AND (NOT *src2)) return 0;
     src1++; src2++;
    END
   return ((int) toupper(*src1))-((int) toupper(*src2));
END	
#endif

#ifdef NEEDS_STRSTR
	char *strstr(char *haystack, char *needle)
BEGIN
   int lh=strlen(haystack), ln=strlen(needle);
   int z;
   char *p;

   for (z=0; z<=lh-ln; z++)
    if (strncmp(p=haystack+z,needle,ln)==0) return p;
   return Nil;
END
#endif

#ifdef BROKEN_SPRINTF
#include <varargs.h>

	int mysprintf(va_alist) va_dcl
BEGIN
   va_list pvar;
   char *form,*dest,*run;

   va_start(pvar);
   dest=va_arg(pvar,char*);
   form=va_arg(pvar,char*);
   vsprintf(dest,form,pvar);

   for (run=dest; *run!='\0'; run++);
   return run-dest;
END
#endif

/*---------------------------------------------------------------------------*/
/* das originale strncpy plaettet alle ueberstehenden Zeichen mit Nullen */

void strmaxcpy(char *dest, const char *src, int Max)
{
  int cnt = strlen(src);

  /* leave room for terminating NUL */

  if (cnt > (Max - 1))
    cnt = Max - 1;
  memcpy(dest, src, cnt); dest[cnt]='\0';
}

/*---------------------------------------------------------------------------*/
/* einfuegen, mit Begrenzung */

	void strmaxcat(char *Dest, const char *Src, int MaxLen)
BEGIN
   int TLen=strlen(Src),DLen=strlen(Dest);

   if (TLen>MaxLen-DLen) TLen=MaxLen-DLen;
   if (TLen>0)
    BEGIN
     memcpy(Dest+DLen,Src,TLen);
     Dest[DLen+TLen]='\0';
    END
END

	void strprep(char *Dest, const char *Src)
BEGIN
   memmove(Dest+strlen(Src),Dest,strlen(Dest)+1);
   memmove(Dest,Src,strlen(Src));
END

	void strmaxprep(char *Dest, const char *Src, int MaxLen)
BEGIN
   int RLen;
   
   RLen = strlen(Src); if ( RLen > MaxLen - ((int)strlen(Dest))) RLen = MaxLen - strlen(Dest);
   memmove(Dest+RLen,Dest,strlen(Dest)+1);
   memmove(Dest,Src,RLen);
END

	void strins(char *Dest, const char *Src, int Pos)
BEGIN
   memmove(Dest+Pos+strlen(Src),Dest+Pos,strlen(Dest)+1-Pos);
   memmove(Dest+Pos,Src,strlen(Src));
END

	void strmaxins(char *Dest, const char *Src, int Pos, int MaxLen)
BEGIN
   int RLen;

   RLen = strlen(Src); if (RLen > MaxLen - ((int)strlen(Dest))) RLen = MaxLen - strlen(Dest);
   memmove(Dest+Pos+RLen,Dest+Pos,strlen(Dest)+1-Pos);
   memmove(Dest+Pos,Src,RLen);
END

/*---------------------------------------------------------------------------*/
/* Bis Zeilenende lesen */

         void ReadLn(FILE *Datei, char *Zeile)
{
   char *ptr;
   int l;
 
   *Zeile='\0'; ptr=fgets(Zeile,256,Datei);
   if ((ptr==Nil) AND (ferror(Datei)!=0)) *Zeile='\0';
   l=strlen(Zeile);
   if ((l>0) AND (Zeile[l-1]=='\n')) Zeile[--l]='\0';
   if ((l>0) AND (Zeile[l-1]=='\r')) Zeile[--l]='\0';
   if ((l>0) AND (Zeile[l-1]==26)) Zeile[--l]='\0';
}

        int ReadLnCont(FILE *Datei, char *Zeile, int MaxLen)
{
   char *ptr, *pDest;
   int l, RemLen, Count;
   Boolean cont;

   /* read from input until either string has reached maximum length,
      or no continuation is present */

   RemLen = MaxLen; pDest = Zeile; Count = 0;
   do
   {
     /* get a line from file */

     *pDest = '\0'; ptr = fgets(pDest, RemLen, Datei);
     if ((ptr==Nil) AND (ferror(Datei) != 0))
       *pDest = '\0';

     /* strip off trailing CR/LF */

     l = strlen(pDest); cont = False;
     if ((l > 0) && (pDest[l - 1] == '\n'))
       pDest[--l] = '\0';
     if ((l > 0) && (pDest[l - 1] == '\r'))
       pDest[--l] = '\0';

     /* yes - this is necessary, when we get an old DOS textfile with
        Ctrl-Z as EOF */

     if ((l > 0) && (pDest[l - 1] == 26))
       pDest[--l] = '\0';

     /* optional line continuation */

     if ((l > 0) && (pDest[l - 1] == '\\'))
     {
       pDest[--l] = '\0';
       cont = True;
     }

     /* prepare for next chunk */

     RemLen -= l; pDest += l; Count++;
   }
   while ((RemLen > 2) && (cont));

   return Count;
}

/*--------------------------------------------------------------------*/
/* Zahlenkonstante umsetzen: $ hex, % binaer, @ oktal */
/* inp: Eingabezeichenkette */
/* erg: Zeiger auf Ergebnis-Longint */
/* liefert TRUE, falls fehlerfrei, sonst FALSE */

        LongInt ConstLongInt(const char *inp, Boolean *err)
BEGIN
   static char Prefixes[4]={'$','@','%','\0'}; /* die moeglichen Zahlensysteme */
   static LongInt Bases[3]={16,8,2};           /* die dazugehoerigen Basen */
   LongInt erg;
   LongInt Base=10,z,val,vorz=1;  /* Vermischtes */

   /* eventuelles Vorzeichen abspalten */

   if (*inp == '-')
    BEGIN
     vorz = (-1); inp++;
    END

   /* Sonderbehandlung 0x --> $ */

   if ((strlen(inp) >= 2) && (*inp == '0') && (toupper(inp[1]) == 'X'))
    BEGIN
     inp += 2;
     Base = 16;
    END

   /* Jetzt das Zahlensystem feststellen.  Vorgabe ist dezimal, was
      sich aber durch den Initialwert von Base jederzeit aendern
      laesst.  Der break-Befehl verhindert, dass mehrere Basenzeichen
      hintereinander eingegeben werden koennen */

   else
    for (z = 0; z < 3; z++)
     if (*inp == Prefixes[z])
      BEGIN
       Base = Bases[z]; inp++; break;
      END

   /* jetzt die Zahlenzeichen der Reihe nach durchverwursten */

   erg=0; *err=False;
   for(; *inp != '\0'; inp++)
    BEGIN
     /* Ziffern 0..9 ergeben selbiges */

     if ((*inp>='0') AND (*inp<='9')) val=(*inp)-'0';

     /* Grossbuchstaben fuer Hexziffern */

     else if ((*inp>='A') AND (*inp<='F')) val=(*inp)-'A'+10;

     /* Kleinbuchstaben nicht vergessen...! */

     else if ((*inp>='a') AND (*inp<='f')) val=(*inp)-'a'+10;

     /* alles andere ist Schrott */

     else break;

     /* entsprechend der Basis zulaessige Ziffer ? */

     if (val>=Base) break;

     /* Zahl linksschieben, zusammenfassen, naechster bitte */

     erg=erg*Base+val;
    END

   /* bis zum Ende durchgelaufen ? */
   
   if (*inp == '\0')
    BEGIN
     /* Vorzeichen beruecksichtigen */

     erg*=vorz;
     *err=True;
    END

   return erg;
END

/*--------------------------------------------------------------------------*/
/* alle Leerzeichen aus einem String loeschen */

        void KillBlanks(char *s)
BEGIN
   char *z,*dest;
   Boolean InHyp=False,InQuot=False;

   dest=s;
   for (z=s; *z!='\0'; z++)
    BEGIN
     switch (*z)
      BEGIN
       case '\'': if (NOT InQuot) InHyp=NOT InHyp; break;
       case '"': if (NOT InHyp) InQuot=NOT InQuot; break;
      END
     if ((NOT isspace((unsigned char)*z)) OR (InHyp) OR (InQuot)) *(dest++)=(*z);
    END
   *dest='\0';
END

/*--------------------------------------------------------------------------*/
/* fuehrende Leerzeichen loeschen */

        void KillPrefBlanks(char *s)
BEGIN
   char *z=s;

   while ((*z!='\0') AND (isspace((unsigned char)*z))) z++;
   if (z!=s) strcpy(s,z);
END

/*--------------------------------------------------------------------------*/
/* anhaengende Leerzeichen loeschen */

        void KillPostBlanks(char *s)
BEGIN
   char *z=s+strlen(s)-1;

   while ((z>=s) AND (isspace((unsigned char)*z))) *(z--)='\0';
END

/*--------------------------------------------------------------------------*/ 

	int strqcmp(const char *s1, const char *s2)
BEGIN
   int erg=(*s1)-(*s2);
   return (erg!=0) ? erg : strcmp(s1,s2);
END

	void strutil_init(void)
BEGIN
   HexLowerCase=False;
END
