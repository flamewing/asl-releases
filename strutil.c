/* strutil.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* haeufig benoetigte String-Funktionen                                      */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*           13. 8.1997 KillBlanks-Funktionen aus asmsub.c heruebergenommen  */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "strutil.h"
#undef strlen   /* VORSICHT, Rekursion!!! */

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
   static char *UDigitVals="0123456789ABCDEF",*LDigitVals="0123456789abcdef",*ptr;
   static String h[BufferCnt];
   static int z=0;

   if (Stellen>8) Stellen=8;
   h[z][0]='\0';
   do
    BEGIN
     memmove(h[z]+1,h[z],strlen(h[z])+1);
     h[z][0]=(HexLowerCase)?(LDigitVals[i&15]):(UDigitVals[i&15]);
     i>>=4;
    END
   while ((i!=0) OR (strlen(h[z])<Stellen));
   ptr=h[z];
   z=(z+1)%BufferCnt;
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
   while (toupper(*src1)==toupper(*src2))
    BEGIN
     if ((NOT *src1) AND (NOT *src2)) return 0;
     src1++; src2++;
    END
   return ((int) toupper(*src1))-((int) toupper(*src2));
END	

	int strncasecmp(const char *src1, const char *src2, int len)
BEGIN
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

/*---------------------------------------------------------------------------*/
/* wenn man mit unsigned arbeitet, gibt das boese Seiteneffekte bei direkter
   Arithmetik mit strlen... */

	signed int strslen(const char *s)
BEGIN
   return strlen(s);
END

/*---------------------------------------------------------------------------*/
/* das originale strncpy plaettet alle ueberstehenden Zeichen mit Nullen */

	void strmaxcpy(char *dest, const char *src, int Max)
BEGIN
   int cnt=strlen(src);

   if (cnt>Max) cnt=Max;
   memcpy(dest,src,cnt); dest[cnt]='\0';
END

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
   
   RLen=strlen(Src); if (RLen>MaxLen-strlen(Dest)) RLen=MaxLen-strlen(Dest);
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

   RLen=strlen(Src); if (RLen>MaxLen-strlen(Dest)) RLen=MaxLen-strlen(Dest);
   memmove(Dest+Pos+RLen,Dest+Pos,strlen(Dest)+1-Pos);
   memmove(Dest+Pos,Src,RLen);
END

/*---------------------------------------------------------------------------*/
/* Bis Zeilenende lesen */

        void ReadLn(FILE *Datei, char *Zeile)
BEGIN
/*   int Zeichen='\0'; 
   char *Run=Zeile;

   while ((Zeichen!='\n') AND (Zeichen!=EOF) AND (!feof(Datei)))
    BEGIN
     Zeichen=fgetc(Datei);
     if (Zeichen!=26) *Run++=Zeichen;
    END

   if ((Run>Zeile) AND ((Zeichen==EOF) OR (Zeichen=='\n'))) Run--;
   if ((Run>Zeile) AND (Run[-1]==13)) Run--;
   *Run++='\0';*/

   char *ptr;
   int l;

   *Zeile='\0'; ptr=fgets(Zeile,256,Datei);
   if ((ptr==Nil) AND (ferror(Datei)!=0)) *Zeile='\0';
   l=strlen(Zeile);
   if ((l>0) AND (Zeile[l-1]=='\n')) Zeile[--l]='\0';
   if ((l>0) AND (Zeile[l-1]=='\r')) Zeile[--l]='\0';
   if ((l>0) AND (Zeile[l-1]==26)) Zeile[--l]='\0';
END

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

   if (*inp=='-')
    BEGIN
     vorz=(-1); inp++;
    END


   /* Jetzt das Zahlensystem feststellen.  Vorgabe ist dezimal, was
      sich aber durch den Initialwert von Base jederzeit aendern
      laesst.  Der break-Befehl verhindert, dass mehrere Basenzeichen
      hintereinander eingegeben werden koennen */

   for (z=0; z<3; z++)
    if (*inp==Prefixes[z])
     BEGIN
      Base=Bases[z]; inp++; break;
     END

   /* jetzt die Zahlenzeichen der Reihe nach durchverwursten */

   erg=0; *err=False;
   for(; *inp; inp++)
    BEGIN
     /* Ziffern 0..9 ergeben selbiges */

     if ((*inp>='0') AND (*inp<='9')) val=(*inp)-'0';

     /* Grossbuchstaben fuer Hexziffern */

     else if ((*inp>='A') AND (*inp<='F')) val=(*inp)-'A'+10;

     /* Kleinbuchstaben nicht vergessen...! */

     else if ((*inp>='a') AND (*inp<='f')) val=(*inp)-'a'+10;

     /* alles andere ist Schrott */

     else return erg;

     /* entsprechend der Basis zulaessige Ziffer ? */

     if (val>=Base) return erg;

     /* Zahl linksschieben, zusammenfassen, naechster bitte */

     erg=erg*Base+val;
    END

   /* Vorzeichen beruecksichtigen */

   erg*=vorz;

   *err=True; return erg;
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
