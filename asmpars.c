/* asmpars.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Verwaltung von Symbolen und das ganze Drumherum...                        */
/*                                                                           */
/* Historie:  5. 5.1996 Grundsteinlegung                                     */
/*            4. 1.1997 Umstellung wg. case-sensitiv                         */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "bpemu.h"
#include "nls.h"
#include "stringutil.h"
#include "asmfnums.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"

#include "asmpars.h"

LargeWord IntMasks[IntTypeCnt]=
            {0x00000001l,0x00000002l,0x00000007l,0x00000007l,0x0000000fl,0x0000000fl,
             0x0000000fl,0x0000001fl,0x0000001fl,0x0000003fl,0x0000007fl,0x0000007fl,0x000000ffl,
             0x000000ffl,0x000001ffl,0x000003ffl,0x000003ffl,0x000007ffl,0x00000fffl,0x00000fffl,
             0x00001fffl,0x00007fffl,0x0000ffffl,0x0000ffffl,0x0003ffffl,0x0007ffffl,
             0x000fffffl,0x000fffffl,0x003fffffl,0x007fffffl,0x00ffffffl,0x00ffffffl,0xffffffffl,0xffffffffl,
             0xffffffffl
#ifdef HAS64
             ,0xffffffffffffffffllu
#endif
            };

LargeInt IntMins[IntTypeCnt]=
            {          0l,          0l,          0l,         -8l,          0l,         -8l,
                     -16l,          0l,        -16l,          0l,          0l,       -128l,          0l,
                    -128l,          0l,          0l,       -512l,          0l,          0l,      -2047l,
                       0l,     -32768l,          0l,     -32768l,          0l,    -524288l,
                       0l,    -524288l,          0l,   -8388608l,          0l,   -8388608l,-2147483647l,          0l,
             -2147483647l
#ifdef HAS64
             ,-9223372036854775807ll
#endif
            };

LargeInt IntMaxs[IntTypeCnt]=
            {          1l,          3l,         7l,          7l,         15l,         15l,
                      31l,         31l,        31l,         63l,        127l,        127l,        255l,
                     255l,        511l,      1023l,       1023l,       2047l,       4095l,       4095l,
                    8191l,      32767l,     65535l,      65535l,     262143l,     524287l,
#ifdef __STDC__
                 1048575l,    1048575l,   4194303l,    8388607l,   16777215l,   16777215l, 2147483647l, 4294967295ul,
              4294967295ul
#else
                 1048575l,    1048575l,   4194303l,    8388607l,   16777215l,   16777215l, 2147483647l, 4294967295l,
              4294967295l
#endif
#ifdef HAS64
             , 9223372036854775807ll
#endif
            };


Boolean FirstPassUnknown;      /* Hinweisflag: evtl. im ersten Pass unbe-
                                  kanntes Symbol, Ausdruck nicht ausgewertet */
Boolean SymbolQuestionable;    /* Hinweisflag:  Dadurch, dass Phasenfehler
                                  aufgetreten sind, ist dieser Symbolwert evtl.
                                  nicht mehr aktuell */
Boolean UsesForwards;          /* Hinweisflag: benutzt Vorwaertsdefinitionen */
LongInt MomLocHandle;          /* Merker, den lokale Symbole erhalten */

LongInt LocHandleCnt;          /* mom. verwendeter lokaler Handle */

Boolean BalanceTree;           /* Symbolbaum ausbalancieren */


#define ERRMSG
#include "as.rsc"

static char *DigitVals="0123456789ABCDEF";
static char BaseIds[3]={'%','@','$'};
static char BaseLetters[3]={'B','O','H'};
static Byte BaseVals[3]={2,8,16};

typedef struct _TCrossRef
         {
          struct _TCrossRef *Next;
          Byte FileNum;
          LongInt LineNum;
          Integer OccNum;
         } TCrossRef,*PCrossRef;

typedef struct
         {
          TempType Typ;
          union
           {
            LargeInt IWert;
            Double FWert;
            char *SWert;
           } Contents;
         } SymbolVal;

typedef struct _SymbolEntry
         {
          struct _SymbolEntry *Left,*Right;
          ShortInt Balance;
          LongInt Attribute;
          char *SymName;
          Byte SymType;
          ShortInt SymSize;
          Boolean Defined,Used,Changeable;
          SymbolVal SymWert;
          PCrossRef RefList;
          Byte FileNum;
          LongInt LineNum;
         } SymbolEntry,*SymbolPtr;

typedef struct _TSymbolStackEntry
         {
          struct _TSymbolStackEntry *Next;
          SymbolVal Contents;
         } TSymbolStackEntry,*PSymbolStackEntry;

typedef struct _TSymbolStack
         {
          struct _TSymbolStack *Next;
          char *Name;
          PSymbolStackEntry Contents;
         } TSymbolStack,*PSymbolStack;

typedef struct _TDefSymbol
         {
          struct _TDefSymbol *Next;
          char *SymName;
          TempResult Wert;
         } TDefSymbol,*PDefSymbol;

typedef struct _TCToken
         {
          struct _TCToken *Next;
          char *Name;
          LongInt Parent;
          ChunkList Usage;
         } TCToken,*PCToken;

typedef struct Operator
         {
          char *Id;
          int IdLen;
          Boolean Dyadic;
          Byte Priority;
          Boolean MayInt;
          Boolean MayFloat;
          Boolean MayString;
          Boolean Present;
         } Operator;

typedef struct _TLocHeap
         {
          struct _TLocHeap *Next;
          LongInt Cont;
         } TLocHeap,*PLocHandle;

static SymbolPtr FirstSymbol,FirstLocSymbol;
static PDefSymbol FirstDefSymbol;
static PCToken FirstSection;
static Boolean DoRefs;              /* Querverweise protokollieren */
static PLocHandle FirstLocHandle;
static PSymbolStack FirstStack;
static PCToken MomSection;

        void AsmParsInit(void)
BEGIN
   FirstSymbol=Nil;
   FirstLocSymbol=Nil; MomLocHandle=(-1); SetMomSection(-1);
   FirstSection=Nil;
   FirstLocHandle=Nil;
   DoRefs=True;
   FirstStack=Nil;
END


        Boolean RangeCheck(LargeInt Wert, IntType Typ)
BEGIN
#ifndef HAS64
   if (Typ>=SInt32) return True;
#else
   if (Typ>=Int64) return True;
#endif
   else return ((Wert>=IntMins[Typ]) AND (Wert<=IntMaxs[Typ]));
END

        Boolean FloatRangeCheck(Double Wert, FloatType Typ)
BEGIN
   switch (Typ)
    BEGIN
     case Float32  : return (fabs(Wert)<=3.4e38);
     case Float64  : return (fabs(Wert)<=1.7e308);
/**     case FloatCo  : FloatRangeCheck:=Abs(Wert)<=9.22e18;
     case Float80  : FloatRangeCheck:=True;
     case FloatDec : FloatRangeCheck:=True;**/
     default: return False;
    END
/**   IF (Typ=FloatDec) AND (Abs(Wert)>1e1000) THEN WrError(40);**/
END


        static void ReplaceBkSlashes(char *s)
BEGIN
   char *p,Save;
   Integer cnt;
   Boolean OK;
   char ErgChar;

   p=strchr(s,'\\');
   while (p!=Nil)
    BEGIN
     cnt=1; ErgChar='\\';
     switch (toupper(*(p+1)))
      BEGIN
       case '\'':
       case '\\':
       case '"':
        ErgChar=(*(p+1)); cnt=2; break;
       case 'H':
        ErgChar='\''; cnt=2; break;
       case 'I':
        ErgChar='"'; cnt=2; break;
       case 'B':
        ErgChar=Char_BS; cnt=2; break;
       case 'A':
        ErgChar=Char_BEL; cnt=2; break;
       case 'E':
        ErgChar=Char_ESC; cnt=2; break;
       case 'T':
        ErgChar=Char_HT; cnt=2; break;
       case 'N':
        ErgChar=Char_LF; cnt=2; break;
       case 'R':
        ErgChar=Char_CR; cnt=2; break;
       case '0': case '1': case '2': case '3': case '4': case '5':
       case '6': case '7': case '8': case '9':
        cnt=2;
        while ((cnt<4) AND (*(p+cnt)!='\0') AND (*(p+cnt)>='0') AND (*(p+cnt)<='9')) cnt++;
        Save=(*(p+cnt)); *(p+cnt)='\0';
        ErgChar=ConstLongInt(p+1,&OK);
        *(p+cnt)=Save;
        if (!OK) WrError(1320);
        break;
       default:
         WrError(1135);
      END;
     *p=ErgChar; strcpy(p+1,p+cnt);
     p=strchr(p+1,'\\');
    END
END


        Boolean ExpandSymbol(char *Name)
BEGIN
   char *p1,*p2;
   String h;
   Boolean OK;

   do
    BEGIN
     if ((p1=strchr(Name,'{'))==Nil) return True;
     strmaxcpy(h,p1+1,255);
     if ((p2=QuotPos(h,'}'))==Nil)
      BEGIN
       WrXError(1020,Name);
       return False;
      END
     strcpy(p1,p2+1); *p2='\0';
     FirstPassUnknown=False;
     EvalStringExpression(h,&OK,h);
     if (FirstPassUnknown)
      BEGIN
       WrError(1820); return False;
      END
     if (NOT CaseSensitive) UpString(h);
     strmaxins(Name,h,p1-Name,255);
    END
   while (p1!=Nil);
   return True;
END

        Boolean IdentifySection(char *Name, LongInt *Erg)
BEGIN
   PSaveSection SLauf;
   Integer Depth;

   if (NOT ExpandSymbol(Name)) return False;
   if (NOT CaseSensitive) NLS_UpString(Name);

   if (*Name=='\0')
    BEGIN
     *Erg=(-1); return True;
    END
   else if (((strlen(Name)==6) OR (strlen(Name)==7))
        AND (strncasecmp(Name,"PARENT",6)==0)
        AND ((strlen(Name)==6) OR ((Name[6]>='0') AND (Name[6]<='9'))))
    BEGIN
     if (strlen(Name)==6) Depth=1; else Depth=Name[6]-AscOfs;
     SLauf=SectionStack; *Erg=MomSectionHandle;
     while ((Depth>0) AND (*Erg!=(-2)))
      BEGIN
       if (SLauf==Nil) *Erg=(-2);
       else
        BEGIN
         *Erg=SLauf->Handle;
         SLauf=SLauf->Next;
        END
       Depth--;
      END
     if (*Erg==(-2))
      BEGIN
       WrError(1484); return False;
      END
     else return True;
    END
   else if (strcmp(Name,GetSectionName(MomSectionHandle))==0)
    BEGIN
     *Erg=MomSectionHandle; return True;
    END
   else
    BEGIN
     SLauf=SectionStack;
     while ((SLauf!=Nil) AND (strcmp(GetSectionName(SLauf->Handle),Name)!=0))
      SLauf=SLauf->Next;
     if (SLauf==Nil)
      BEGIN
       WrError(1484); return False;
      END
     else
      BEGIN
       *Erg=SLauf->Handle; return True;
      END
    END
END

        static Boolean GetSymSection(char *Name, LongInt *Erg)
BEGIN
   String Part;
   char *q;
   int l=strlen(Name);

   if (Name[l-1]!=']')
    BEGIN
     *Erg=(-2); return True;
    END

   Name[l-1]='\0';
   q=RQuotPos(Name,'[');
   Name[l-1]=']';
   if (Name+strlen(Name)-q<=2)
    BEGIN
     WrXError(1020,Name); return False; 
    END

   Name[strlen(Name)-1]='\0';
   strmaxcpy(Part,q+1,255); 
   *q='\0';

   return IdentifySection(Part,Erg);
END

        LargeInt ConstIntVal(char *Asc_O, IntType Typ, Boolean *Ok)
BEGIN
   String Asc;
   Byte Search,Base,Digit;
   LargeInt Wert;
   Boolean NegFlag;
   TConstMode ActMode=ConstModeC;
   Boolean Found;
   char *z,*h;

   *Ok=False; Wert=0; strmaxcpy(Asc,Asc_O,255);
   if (Asc[0]=='\0')
    BEGIN
     *Ok=True; return 0;
    END

   /* ASCII herausfiltern */

   else if (Asc[0]=='\'')
    BEGIN
     if (Asc[strlen(Asc)-1]!='\'') return -1;
     strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0'; ReplaceBkSlashes(Asc);
     for (Search=0; Search<strlen(Asc); Search++)
      BEGIN
       Digit=(unsigned int) Asc[Search];
       Wert=(Wert<<8)+CharTransTable[Digit];
      END
     NegFlag=False;
    END

   /* Zahlenkonstante */

   else
    BEGIN
     /* Vorzeichen */

     if (*Asc=='+') strcpy(Asc,Asc+1);
     NegFlag=(*Asc=='-');
     if (NegFlag) strcpy(Asc,Asc+1);

     if (RelaxedMode)
      BEGIN
       Found=False;
       if ((strlen(Asc)>=2) AND (*Asc=='0') AND (toupper(Asc[1])=='X'))
        BEGIN
         ActMode=ConstModeC; Found=True;
        END
       if ((NOT Found) AND (strlen(Asc)>=2))
        BEGIN
         for (Search=0; Search<3; Search++)
          if (Asc[0]==BaseIds[Search])
           BEGIN
            ActMode=ConstModeMoto; Found=True; break;
           END
        END
       if ((NOT Found) AND (strlen(Asc)>=2) AND (Asc[0]>='0') AND (Asc[0]<='9'))
        BEGIN
         for (Search=0; Search<3; Search++)
          if (toupper(Asc[strlen(Asc)-1])==BaseLetters[Search])
           BEGIN
            ActMode=ConstModeIntel; Found=True; break;
           END
        END
       if (NOT Found) ActMode=ConstModeC;
      END
     else ActMode=ConstMode;

     /* Zahlensystem ermitteln/pruefen */

     Base=10;
     switch (ActMode)
      BEGIN
       case ConstModeIntel:
        for (Search=0; Search<3; Search++) 
         if (toupper(Asc[strlen(Asc)-1])==BaseLetters[Search])
          BEGIN
           Base=BaseVals[Search]; Asc[strlen(Asc)-1]='\0'; break;
          END
        break;
       case ConstModeMoto:
        for (Search=0; Search<3; Search++)
         if (Asc[0]==BaseIds[Search])
          BEGIN
           Base=BaseVals[Search]; strcpy(Asc,Asc+1); break;
          END
        break;
       case ConstModeC:
        if (strcmp(Asc,"0")==0)
         BEGIN
          *Ok=True; return 0;
         END
        else if (Asc[0]!='0') Base=10;
        else if (strlen(Asc)<2) return -1;
        else
         BEGIN
          strcpy(Asc,Asc+1);
          switch (toupper(Asc[0]))
           BEGIN
            case 'X': strcpy(Asc,Asc+1); Base=16; break;
            case 'B': strcpy(Asc,Asc+1); Base=2; break;
            default: Base=8; break;
           END
          if (Asc[0]=='\0') return -1;
         END
      END

     if (Asc[0]=='\0') return -1;

     if (ActMode==ConstModeIntel)
      if ((Asc[0]<'0') OR (Asc[0]>'9')) return -1;

     for (z=Asc; *z!='\0'; z++)
      BEGIN
       if ((h=strchr(DigitVals,toupper(*z)))==Nil) return -1;
       else Search=h-DigitVals;
       if (Search>=Base) return -1;
       Wert=Wert*Base+Search;
      END
    END

   if (NegFlag) Wert=(-Wert);

   *Ok=RangeCheck(Wert,Typ);
   if (Ok) return Wert;
   else 
    BEGIN
     WrError(1320);
     return -1;
    END
END

        Double ConstFloatVal(char *Asc_O, FloatType Typ, Boolean *Ok)
BEGIN
   Double Erg;
   char *end;

   if (Typ);  /* satisfy some compilers */

   if (*Asc_O)
    BEGIN
     Erg=strtod(Asc_O,&end);
     *Ok=(*end=='\0');
    END
   else 
    BEGIN
     Erg=0.0;
     *Ok=True;
    END
   return Erg;
END

        void ConstStringVal(char *Asc_O, char *Erg, Boolean *OK)
BEGIN
   String Asc,tmp,Part;
   char *z,Save;
   Integer l;
   Boolean OK2;
   TempResult t;

   *OK=False;

   if ((strlen(Asc_O)<2) OR (*Asc_O!='"') OR (Asc_O[strlen(Asc_O)-1]!='"')) return;

   strmaxcpy(Asc,Asc_O+1,255); Asc[strlen(Asc)-1]='\0'; *tmp='\0';

   while (*Asc!='\0')
    BEGIN
     z=strchr(Asc,'\\'); if (z==Nil) z=Asc+strlen(Asc);
     Save=(*z); *z='\0'; if (strchr(Asc,'"')!=Nil) return;
     strmaxcat(tmp,Asc,255); *z=Save; strcpy(Asc,z);
     if (*Asc=='\\')
      BEGIN
       if (strlen(Asc)<2) return; 
       l=strlen(tmp); 
       switch (toupper(Asc[1]))
        BEGIN
         case '\'':
         case '\\':
         case '"':
          tmp[l++]=Asc[1]; tmp[l++]='\0'; z=Asc+2; break;
         case 'H':
          tmp[l++]='\''; tmp[l++]='\0'; z=Asc+2; break;
         case 'I':
          tmp[l++]='"'; tmp[l++]='\0'; z=Asc+2; break;
         case 'B':
          tmp[l++]=Char_BS; tmp[l++]='\0'; z=Asc+2; break;
         case 'A':
          tmp[l++]=Char_BEL; tmp[l++]='\0'; z=Asc+2; break;
         case 'E':
          tmp[l++]=Char_ESC; tmp[l++]='\0'; z=Asc+2; break;
         case 'T':
          tmp[l++]=Char_HT; tmp[l++]='\0'; z=Asc+2; break;
         case 'N':
          tmp[l++]=Char_LF; tmp[l++]='\0'; z=Asc+2; break;
         case 'R':
          tmp[l++]=Char_CR; tmp[l++]='\0'; z=Asc+2; break;
         case '0': case '1': case '2': case '3': case '4':
         case '5': case '6': case '7': case '8': case '9':
          z=Asc+1;
          while ((*z!='\0') AND (z-Asc<4) AND (*z>='0') AND (*z<='9')) z++;
          Save=(*z); *z='\0'; tmp[l++]=ConstLongInt(Asc+1,&OK2)&0xff; *z=Save;
          if (tmp[l-1]=='\0') WrError(240);
          if (NOT OK2) return; tmp[l++]='\0';
          break;
         case '{':
          z=QuotPos(Asc,'}'); if (z==Nil) return;
          FirstPassUnknown=False;
          *(z++)='\0'; strmaxcpy(Part,Asc+2,255); KillBlanks(Part);
          EvalExpression(Part,&t);
          if (FirstPassUnknown) 
           BEGIN
            WrXError(1820,Part); return;
           END
          else switch(t.Typ)
           BEGIN
            case TempInt: strmaxcat(tmp,HexString(t.Contents.Int,0),255); break;
            case TempFloat: strmaxcat(tmp,FloatString(t.Contents.Float),255); break;
            case TempString: strmaxcat(tmp,t.Contents.Ascii,255); break;
            default: return;
           END
          break;
         default:
          WrError(1135); z=Asc+2; break;
        END
       strcpy(Asc,z);
      END
    END

   *OK=True; strmaxcpy(Erg,tmp,255);
END


        static SymbolPtr FindLocNode(
#ifdef __PROTOS__
char *Name, TempType SearchType
#endif
);

        static SymbolPtr FindNode(
#ifdef __PROTOS__
char *Name, TempType SearchType
#endif
);


	static void EvalExpression_ChgFloat(TempResult *T)
BEGIN
   if (T->Typ!=TempInt) return;
   T->Typ=TempFloat; T->Contents.Float=T->Contents.Int;
END

        void EvalExpression(char *Asc_O, TempResult *Erg)
BEGIN
#define OpCnt 23
static Operator Operators[OpCnt+1]=
               /* Dummynulloperator */
               {{" " ,1 , False,  0, False, False, False, False},
               /* Einerkomplement */
               {"~" ,1 , False,  1, True , False, False, False},
               /* Linksschieben */
               {"<<",2 , True ,  3, True , False, False, False},
               /* Rechtsschieben */
               {">>",2 , True ,  3, True , False, False, False},
               /* Bitspiegelung */
               {"><",2 , True ,  4, True , False, False, False},
               /* binaeres AND */
               {"&" ,1 , True ,  5, True , False, False, False},
               /* binaeres OR */
               {"|" ,1 , True ,  6, True , False, False, False},
               /* binaeres EXOR */
               {"!" ,1 , True ,  7, True , False, False, False},
               /* allg. Potenz */
               {"^" ,1 , True ,  8, True , True , False, False},
               /* Produkt */
               {"*" ,1 , True , 11, True , True , False, False},
               /* Quotient */
               {"/" ,1 , True , 11, True , True , False, False},
               /* Modulodivision */
               {"#" ,1 , True , 11, True , False, False, False},
               /* Summe */
               {"+" ,1 , True , 13, True , True , True , False},
               /* Differenz */
               {"-" ,1 , True , 13, True , True , False, False},
               /* logisches NOT */
               {"~~",2 , False,  2, True , False, False, False},
               /* logisches AND */
               {"&&",2 , True , 15, True , False, False, False},
               /* logisches OR */
               {"||",2 , True , 16, True , False, False, False},
               /* logisches EXOR */
               {"!!",2 , True , 17, True , False, False, False},
               /* Gleichheit */
               {"=" ,1 , True , 23, True , True , True , False},
               /* Groesser als */
               {">" ,1 , True , 23, True , True , True , False},
               /* Kleiner als */
               {"<" ,1 , True , 23, True , True , True , False},
               /* Kleiner oder gleich */
               {"<=",2 , True , 23, True , True , True , False},
               /* Groesser oder gleich */
               {">=",2 , True , 23, True , True , True , False},
               /* Ungleichheit */
               {"<>",2 , True , 23, True , True , True , False}};
   static Operator *OpEnd=Operators+OpCnt;
   Operator *FOps[OpCnt+1];
   LongInt FOpCnt=0;

   Boolean OK,FFound;
   TempResult LVal,RVal;
   Integer z1;
   Operator *Op;
   char Save='\0';
   Integer LKlamm,RKlamm,WKlamm,zop;
   Integer OpMax,LocOpMax,OpPos=(-1),OpLen;
   Boolean OpFnd,InHyp,InQuot;
   LargeInt HVal;
   Double  FVal;
   SymbolPtr Ptr;
   PFunction ValFunc;
   String Asc,stemp,ftemp;
   char *KlPos,*zp;

   memset(&LVal,0,sizeof(LVal));
   memset(&RVal,0,sizeof(RVal));

   ChkStack();

   strmaxcpy(Asc,Asc_O,255);
   strmaxcpy(stemp,Asc,255); KillBlanks(Asc);
   if (MakeDebug) fprintf(Debug,"Parse %s",Asc);

   /* Annahme Fehler */

   Erg->Typ=TempNone;

   /* Programmzaehler ? */

   if (strcasecmp(Asc,PCSymbol)==0)
    BEGIN
     Erg->Typ=TempInt;
     Erg->Contents.Int=EProgCounter();
     return;
    END

   /* Konstanten ? */

   Erg->Contents.Int=ConstIntVal(Asc,IntTypeCnt-1,&OK);
   if (OK)
    BEGIN
     Erg->Typ=TempInt; return;
    END

   Erg->Contents.Float=ConstFloatVal(Asc,Float80,&OK);
   if (OK)
    BEGIN
     Erg->Typ=TempFloat; return;
    END

   ConstStringVal(Asc,Erg->Contents.Ascii,&OK);
   if (OK)
    BEGIN
     Erg->Typ=TempString; return;
    END

   InternSymbol(Asc,Erg); if (Erg->Typ!=TempNone) return;

   /* Zaehler initialisieren */

   LocOpMax=0; OpMax=0; LKlamm=0; RKlamm=0; WKlamm=0;
   InHyp=False; InQuot=False;
   for (Op=Operators+1; Op<=OpEnd; Op++)
    if (strstr(Asc,Op->Id)!=Nil) FOps[FOpCnt++]=Op;

   /* nach Operator hoechster Rangstufe ausserhalb Klammern suchen */

   for (zp=Asc; *zp!='\0'; zp++)
    BEGIN
     switch (*zp)
      BEGIN
       case '(': if (NOT (InHyp OR InQuot)) LKlamm++; break;
       case ')': if (NOT (InHyp OR InQuot)) RKlamm++; break;
       case '{': if (NOT (InHyp OR InQuot)) WKlamm++; break;
       case '}': if (NOT (InHyp OR InQuot)) WKlamm--; break;
       case '"': if (NOT InHyp) InQuot=NOT InQuot; break;
       case '\'':if (NOT InQuot) InHyp=NOT InHyp; break;
       default: 
        if ((LKlamm==RKlamm) AND (WKlamm==0) AND (NOT InHyp) AND (NOT InQuot))
         BEGIN
          OpFnd=False; OpLen=0; LocOpMax=0;
          for (zop=0; zop<FOpCnt; zop++)
           if (strncmp(zp,FOps[zop]->Id,FOps[zop]->IdLen)==0)
            if (FOps[zop]->IdLen>=OpLen)
             BEGIN
              OpFnd=True; OpLen=FOps[zop]->IdLen; LocOpMax=FOps[zop]-Operators;
              if (Operators[LocOpMax].Priority>=Operators[OpMax].Priority)
               BEGIN
                OpMax=LocOpMax; OpPos=zp-Asc;
               END
             END
          if (OpFnd) zp+=strlen(Operators[LocOpMax].Id)-1;
         END
      END
    END

   /* Klammerfehler ? */

   if (LKlamm!=RKlamm)
    BEGIN
     WrXError(1300,Asc); return;
    END

   /* Operator gefunden ? */

   if (OpMax!=0)
    BEGIN
     Op=Operators+OpMax;

     /* Minuszeichen sowohl mit einem als auch 2 Operanden */

     if (strcmp(Op->Id,"-")==0) Op->Dyadic=(OpPos>0);

     /* Operandenzahl pruefen */

     if (((Op->Dyadic) AND (OpPos==0)) OR ((NOT Op->Dyadic) AND (OpPos!=0)) OR (OpPos==strlen(Asc)-1))
      BEGIN
       WrError(1110); return;
      END

     /* Teilausdruecke rekursiv auswerten */

     Save=Asc[OpPos]; Asc[OpPos]='\0';
     if (Op->Dyadic) EvalExpression(Asc,&LVal);
     else
      BEGIN
       LVal.Typ=TempInt; LVal.Contents.Int=0;
      END
     EvalExpression(Asc+OpPos+strlen(Op->Id),&RVal);
     Asc[OpPos]=Save;

     /* Abbruch, falls dabei Fehler */

     if ((LVal.Typ==TempNone) OR (RVal.Typ==TempNone)) return;

     /* Typueberpruefung */

     if ((Op->Dyadic) AND (LVal.Typ!=RVal.Typ))
      BEGIN
       if ((LVal.Typ==TempString) OR (RVal.Typ==TempString))
        BEGIN
         WrError(1135); return;
        END
       if (LVal.Typ==TempInt) EvalExpression_ChgFloat(&LVal);
       if (RVal.Typ==TempInt) EvalExpression_ChgFloat(&RVal);
      END

     switch (RVal.Typ)
      BEGIN
       case TempInt:
        if (NOT Op->MayInt)
         if (NOT Op->MayFloat)
          BEGIN
           WrError(1135); return;
          END
         else
          BEGIN
           EvalExpression_ChgFloat(&RVal); 
           if (Op->Dyadic) EvalExpression_ChgFloat(&LVal);
          END
        break;
       case TempFloat: 
        if (NOT Op->MayFloat)
         BEGIN
          WrError(1135); return;
         END
        break;
       case TempString:
        if (NOT Op->MayString)
         BEGIN
          WrError(1135); return;
         END;
        break;
       default:
        break;
      END

     /* Operanden abarbeiten */

     switch (OpMax)
      BEGIN
       case 1:                                            /* ~ */
        Erg->Typ=TempInt;
        Erg->Contents.Int=~RVal.Contents.Int;
        break;
       case 2:                                            /* << */
        Erg->Typ=TempInt;
        Erg->Contents.Int=LVal.Contents.Int<<RVal.Contents.Int;
        break;
       case 3:                                            /* >> */
        Erg->Typ=TempInt;
        Erg->Contents.Int=LVal.Contents.Int>>RVal.Contents.Int;
        break;
       case 4:                                            /* >< */
        Erg->Typ=TempInt;
        if ((RVal.Contents.Int<1) OR (RVal.Contents.Int>32)) WrError(1320);
        else
         BEGIN
          Erg->Contents.Int=(LVal.Contents.Int>>RVal.Contents.Int)<<RVal.Contents.Int;
          RVal.Contents.Int--;
          for (z1=0; z1<=RVal.Contents.Int; z1++)
           BEGIN
            if ((LVal.Contents.Int&(1<<(RVal.Contents.Int-z1)))!=0)
            Erg->Contents.Int+=(1<<z1);
           END
         END
        break;
       case 5:                                          /* & */
        Erg->Typ=TempInt;
        Erg->Contents.Int=LVal.Contents.Int&RVal.Contents.Int;
        break;
       case 6:                                          /* | */
        Erg->Typ=TempInt;
        Erg->Contents.Int=LVal.Contents.Int|RVal.Contents.Int;
        break;
       case 7:
        Erg->Typ=TempInt;                               /* ! */
        Erg->Contents.Int=LVal.Contents.Int^RVal.Contents.Int;
        break;
       case 8:                                          /* ^ */
        switch (Erg->Typ=LVal.Typ)
         BEGIN
          case TempInt:
           if (RVal.Contents.Int<0) Erg->Contents.Int=0;
           else
            BEGIN
             Erg->Contents.Int=1;
             while (RVal.Contents.Int>0)
              BEGIN
               if ((RVal.Contents.Int&1)==1) Erg->Contents.Int*=LVal.Contents.Int;
               RVal.Contents.Int>>=1;
               if (RVal.Contents.Int!=0) LVal.Contents.Int*=LVal.Contents.Int;
              END
            END
           break;
          case TempFloat:
           if (RVal.Contents.Float==0.0) Erg->Contents.Float=1.0;
           else if (LVal.Contents.Float==0.0) Erg->Contents.Float=0.0;
           else if (LVal.Contents.Float>0) Erg->Contents.Float=pow(LVal.Contents.Float,RVal.Contents.Float);
           else if ((abs(RVal.Contents.Float)<=((double)MaxLongInt)) AND (floor(RVal.Contents.Float)==RVal.Contents.Float))
            BEGIN
             HVal=(LongInt) floor(RVal.Contents.Float+0.5);
             if (HVal<0)
              BEGIN
               LVal.Contents.Float=1/LVal.Contents.Float; HVal=(-HVal);
              END
             Erg->Contents.Float=1.0;
             while (HVal>0)
              BEGIN
               if ((HVal&1)==1) Erg->Contents.Float*=LVal.Contents.Float;
               LVal.Contents.Float*=LVal.Contents.Float; HVal>>=1;
              END
            END
           else
            BEGIN
             WrError(1890); Erg->Typ=TempNone;
            END
           break;
          default:
           break;
         END
        break;
       case 9:                                          /* * */
        switch (Erg->Typ=LVal.Typ)
         BEGIN
          case TempInt:Erg->Contents.Int=LVal.Contents.Int*RVal.Contents.Int; break;
          case TempFloat:Erg->Contents.Float=LVal.Contents.Float*RVal.Contents.Float; break;
          default: break;
         END
        break; 
       case 10:                                         /* / */
        switch (LVal.Typ)
         BEGIN
          case TempInt:
           if (RVal.Contents.Int==0) WrError(1310);
           else
            BEGIN
             Erg->Typ=TempInt;
             Erg->Contents.Int=LVal.Contents.Int/RVal.Contents.Int;
            END
           break;
          case TempFloat:
           if (RVal.Contents.Float==0.0) WrError(1310);
           else
            BEGIN
             Erg->Typ=TempFloat;
             Erg->Contents.Float=LVal.Contents.Float/RVal.Contents.Float;
            END
          default: 
           break;
         END
        break;
       case 11:                                         /* # */
        if (RVal.Contents.Int==0) WrError(1310);
        else
         BEGIN
          Erg->Typ=TempInt;
          Erg->Contents.Int=LVal.Contents.Int%RVal.Contents.Int;
         END
        break;
       case 12:                                         /* + */
        switch (Erg->Typ=LVal.Typ)
         BEGIN
          case TempInt   : 
           Erg->Contents.Int=LVal.Contents.Int+RVal.Contents.Int; break;
          case TempFloat :
            Erg->Contents.Float=LVal.Contents.Float+RVal.Contents.Float; break;
          case TempString: 
           strmaxcpy(Erg->Contents.Ascii,LVal.Contents.Ascii,255);
           strmaxcat(Erg->Contents.Ascii,RVal.Contents.Ascii,255); break;
          default: break;
         END
        break;
       case 13:                                         /* - */
        if (Op->Dyadic)
         switch (Erg->Typ=LVal.Typ)
          BEGIN
           case TempInt:
            Erg->Contents.Int=LVal.Contents.Int-RVal.Contents.Int; break;
           case TempFloat:
            Erg->Contents.Float=LVal.Contents.Float-RVal.Contents.Float; break;
           default: break;
          END
        else
         switch (Erg->Typ=RVal.Typ)
          BEGIN
           case TempInt:
            Erg->Contents.Int=(-RVal.Contents.Int); break;
           case TempFloat:
            Erg->Contents.Float=(-RVal.Contents.Float); break;
           default: break;
          END
        break;
       case 14:                                         /* ~~ */
        Erg->Typ=TempInt;
        Erg->Contents.Int=(RVal.Contents.Int==0)?1:0;
        break;
       case 15:                                         /* && */
        Erg->Typ=TempInt;
        Erg->Contents.Int=((LVal.Contents.Int!=0) AND (RVal.Contents.Int!=0))?1:0;
        break;
       case 16:                                         /* || */
        Erg->Typ=TempInt;
        Erg->Contents.Int=((LVal.Contents.Int!=0) OR (RVal.Contents.Int!=0))?1:0;
        break;
       case 17:                                         /* !! */
        Erg->Typ=TempInt;
        if ((LVal.Contents.Int!=0) AND (RVal.Contents.Int==0))
         Erg->Contents.Int=1;
        else if ((LVal.Contents.Int==0) AND (RVal.Contents.Int!=0))
         Erg->Contents.Int=1;
        else Erg->Contents.Int=0;
        break;
       case 18:                                         /* = */
         Erg->Typ=TempInt;
         switch (LVal.Typ)
          BEGIN
           case TempInt: 
            Erg->Contents.Int=(LVal.Contents.Int==RVal.Contents.Int)?1:0; break;
           case TempFloat:
            Erg->Contents.Int=(LVal.Contents.Float==RVal.Contents.Float)?1:0; break;
           case TempString:
            Erg->Contents.Int=(strcmp(LVal.Contents.Ascii,RVal.Contents.Ascii)==0)?1:0; break;
           default: 
            break;
          END
         break;
       case 19:                                         /* > */
         Erg->Typ=TempInt;
         switch (LVal.Typ)
          BEGIN
           case TempInt: 
            Erg->Contents.Int=(LVal.Contents.Int>RVal.Contents.Int)?1:0; break;
           case TempFloat:
            Erg->Contents.Int=(LVal.Contents.Float>RVal.Contents.Float)?1:0; break;
           case TempString:
            Erg->Contents.Int=(strcmp(LVal.Contents.Ascii,RVal.Contents.Ascii)>0)?1:0; break;
           default: 
            break;
          END
         break;
       case 20:                                         /* < */
         Erg->Typ=TempInt;
         switch (LVal.Typ)
          BEGIN
           case TempInt: 
            Erg->Contents.Int=(LVal.Contents.Int<RVal.Contents.Int)?1:0; break;
           case TempFloat:
            Erg->Contents.Int=(LVal.Contents.Float<RVal.Contents.Float)?1:0; break;
           case TempString:
            Erg->Contents.Int=(strcmp(LVal.Contents.Ascii,RVal.Contents.Ascii)<0)?1:0; break;
           default: 
            break;
          END
         break;
       case 21:                                         /* <= */
         Erg->Typ=TempInt;
         switch (LVal.Typ)
          BEGIN
           case TempInt: 
            Erg->Contents.Int=(LVal.Contents.Int<=RVal.Contents.Int)?1:0; break;
           case TempFloat:
            Erg->Contents.Int=(LVal.Contents.Float<=RVal.Contents.Float)?1:0; break;
           case TempString:
            Erg->Contents.Int=(strcmp(LVal.Contents.Ascii,RVal.Contents.Ascii)<=0)?1:0; break;
           default: 
            break;
          END
         break;
       case 22:                                         /* >= */
         Erg->Typ=TempInt;
         switch (LVal.Typ)
          BEGIN
           case TempInt: 
            Erg->Contents.Int=(LVal.Contents.Int>=RVal.Contents.Int)?1:0; break;
           case TempFloat:
            Erg->Contents.Int=(LVal.Contents.Float>=RVal.Contents.Float)?1:0; break;
           case TempString:
            Erg->Contents.Int=(strcmp(LVal.Contents.Ascii,RVal.Contents.Ascii)>=0)?1:0; break;
           default: 
            break;
          END
         break;
       case 23:                                         /* <> */
         Erg->Typ=TempInt;
         switch (LVal.Typ)
          BEGIN
           case TempInt: 
            Erg->Contents.Int=(LVal.Contents.Int!=RVal.Contents.Int)?1:0; break;
           case TempFloat:
            Erg->Contents.Int=(LVal.Contents.Float!=RVal.Contents.Float)?1:0; break;
           case TempString:
            Erg->Contents.Int=(strcmp(LVal.Contents.Ascii,RVal.Contents.Ascii)!=0)?1:0; break;
           default: 
            break;
          END
         break;
      END
     return;
    END

   /* kein Operator gefunden: Klammerausdruck ? */

   if (LKlamm!=0)
    BEGIN

     /* erste Klammer suchen, Funktionsnamen abtrennen */

     KlPos=strchr(Asc,'(');

     /* Funktionsnamen abschneiden */

     *KlPos='\0'; strmaxcpy(ftemp,Asc,255);
     strcpy(Asc,KlPos+1); Asc[strlen(Asc)-1]='\0'; 

     /* Nullfunktion: nur Argument */

     if (ftemp[0]=='\0')
      BEGIN
       EvalExpression(Asc,&LVal);
       *Erg=LVal; return;
      END

     /* selbstdefinierte Funktion ? */

     if ((ValFunc=FindFunction(ftemp))!=Nil)
      BEGIN
       strmaxcpy(ftemp,ValFunc->Definition,255);
       for (z1=1; z1<=ValFunc->ArguCnt; z1++)
        BEGIN
         if (Asc[0]=='\0')
          BEGIN
           WrError(1490); return;
          END;
         KlPos=QuotPos(Asc,','); if (KlPos!=Nil) *KlPos='\0';
         EvalExpression(Asc,&LVal);
         if (KlPos==Nil) Asc[0]='\0'; else strcpy(Asc,KlPos+1);
         switch (LVal.Typ)
          BEGIN
           case TempInt:
            sprintf(stemp,"%s",LargeString(LVal.Contents.Int));
            break;
           case TempFloat:
            sprintf(stemp,"%0.16e",LVal.Contents.Float); 
            KillBlanks(stemp);
            break;
           case TempString:
            strcpy(stemp,"\""); 
            strmaxcat(stemp,LVal.Contents.Ascii,255);
            strmaxcat(stemp,"\"",255);
            break;
           default:
            return;
          END
         memmove(stemp+1,stemp,strlen(stemp)+1);
         stemp[0]='('; strmaxcat(stemp,")",255);
         ExpandLine(stemp,z1,ftemp);
        END
       if (Asc[0]!='\0')
        BEGIN
         WrError(1490); return;
        END
       EvalExpression(ftemp,Erg);
       return;
      END

     /* Unterausdruck auswerten (interne Funktionen nur mit einem Argument */

     EvalExpression(Asc,&LVal);

     /* Abbruch bei Fehler */

     if (LVal.Typ==TempNone) return;

     /* hier einmal umwandeln ist effizienter */

     NLS_UpString(ftemp);

     /* Funktionen fuer Stringargumente */

     if (LVal.Typ==TempString)
      BEGIN
       /* in Grossbuchstaben wandeln ? */

       if (strcmp(ftemp,"UPSTRING")==0)
        BEGIN
         Erg->Typ=TempString; strmaxcpy(Erg->Contents.Ascii,LVal.Contents.Ascii,255);
         for (KlPos=Erg->Contents.Ascii; *KlPos!='\0'; KlPos++)
          *KlPos=toupper(*KlPos);
        END

       /* in Kleinbuchstaben wandeln ? */

       else if (strcmp(ftemp,"LOWSTRING")==0)
        BEGIN
         Erg->Typ=TempString; strmaxcpy(Erg->Contents.Ascii,LVal.Contents.Ascii,255);
         for (KlPos=Erg->Contents.Ascii; *KlPos!='\0'; KlPos++)
          *KlPos=tolower(*KlPos);
        END

       /* Parser aufrufen ? */

       else if (strcmp(ftemp,"VAL")==0)
        BEGIN
         EvalExpression(LVal.Contents.Ascii,Erg);
        END

       /* nix gefunden ? */

       else
        BEGIN
         WrXError(1860,ftemp); Erg->Typ=TempNone;
        END
      END

     /* Funktionen fuer Zahlenargumente */

     else
      BEGIN
       FFound=False; Erg->Typ=TempNone;

       /* reine Integerfunktionen */

       if (strcmp(ftemp,"TOUPPER")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else if ((LVal.Contents.Int<0) OR (LVal.Contents.Int>255)) WrError(1320);
         else
          BEGIN
           Erg->Typ=TempInt;
           Erg->Contents.Int=toupper(LVal.Contents.Int);
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"TOLOWER")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else if ((LVal.Contents.Int<0) OR (LVal.Contents.Int>255)) WrError(1320);
         else
          BEGIN
           Erg->Typ=TempInt;
           Erg->Contents.Int=tolower(LVal.Contents.Int);
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"BITCNT")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else
          BEGIN
           Erg->Typ=TempInt;
           Erg->Contents.Int=0;
#ifdef HAS64
           for (z1=0; z1<64; z1++)
#else
           for (z1=0; z1<32; z1++)
#endif
            BEGIN
             Erg->Contents.Int+=(LVal.Contents.Int & 1);
             LVal.Contents.Int=LVal.Contents.Int >> 1;
            END
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"FIRSTBIT")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else
          BEGIN
           Erg->Typ=TempInt;
           Erg->Contents.Int=0;
           do
            BEGIN
             if (NOT Odd(LVal.Contents.Int)) Erg->Contents.Int++;
             LVal.Contents.Int=LVal.Contents.Int >> 1;
            END
#ifdef HAS64
           while ((Erg->Contents.Int<64) AND (NOT Odd(LVal.Contents.Int)));
           if (Erg->Contents.Int>=64) Erg->Contents.Int=(-1);
#else
           while ((Erg->Contents.Int<32) AND (NOT Odd(LVal.Contents.Int)));
           if (Erg->Contents.Int>=32) Erg->Contents.Int=(-1);
#endif
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"LASTBIT")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else
          BEGIN
           Erg->Typ=TempInt;
           Erg->Contents.Int=(-1);
#ifdef HAS64
           for (z1=0; z1<64; z1++)
#else
           for (z1=0; z1<32; z1++)
#endif
            BEGIN
             if (Odd(LVal.Contents.Int)) Erg->Contents.Int=z1;
             LVal.Contents.Int=LVal.Contents.Int >> 1;
            END
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"BITPOS")==0)
        BEGIN
         if (LVal.Typ!=TempInt) WrError(1135);
         else
          BEGIN
           Erg->Typ=TempInt;
           Erg->Contents.Int=0;
           do
            BEGIN
             if (NOT Odd(LVal.Contents.Int)) Erg->Contents.Int++;
             if (NOT Odd(LVal.Contents.Int)) LVal.Contents.Int=LVal.Contents.Int >> 1;
            END
#ifdef HAS64
           while ((Erg->Contents.Int<64) AND (NOT Odd(LVal.Contents.Int)));
           if ((Erg->Contents.Int>=64) OR (LVal.Contents.Int!=1))
#else
           while ((Erg->Contents.Int<32) AND (NOT Odd(LVal.Contents.Int)));
           if ((Erg->Contents.Int>=32) OR (LVal.Contents.Int!=1))
#endif
            BEGIN
             Erg->Contents.Int=(-1); WrError(1540);
            END
          END
         FFound=True;
        END

       /* variable Integer/Float-Funktionen */

       else if (strcmp(ftemp,"ABS")==0)
        BEGIN
         switch (Erg->Typ=LVal.Typ)
          BEGIN
           case TempInt: Erg->Contents.Int=abs(LVal.Contents.Int); break;
           case TempFloat: Erg->Contents.Float=fabs(LVal.Contents.Float);break;
           default: break;
          END
         FFound=True;
        END

       else if (strcmp(ftemp,"SGN")==0)
        BEGIN
         Erg->Typ=TempInt;
         switch (LVal.Typ)
          BEGIN
           case TempInt:
            if (LVal.Contents.Int<0) Erg->Contents.Int=(-1);
            else if (LVal.Contents.Int>0) Erg->Contents.Int=1;
            else Erg->Contents.Int=0;
            break;
           case TempFloat:
            if (LVal.Contents.Float<0) Erg->Contents.Int=(-1);
            else if (LVal.Contents.Float>0) Erg->Contents.Int=1;
            else Erg->Contents.Int=0;
            break;
           default:
            break;
          END
         FFound=True;
        END

       /* Funktionen Float und damit auch Int */

       if (NOT FFound)
        BEGIN
         /* Typkonvertierung */

         EvalExpression_ChgFloat(&LVal); 
         Erg->Typ=TempFloat;

         /* Integerwandlung */

         if (strcmp(ftemp,"INT")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)>MaxLargeInt)
            BEGIN
             Erg->Typ=TempNone; WrError(1320);
            END
           else
            BEGIN
             Erg->Typ=TempInt; Erg->Contents.Int=(LargeInt) floor(LVal.Contents.Float);
            END
          END

         /* Quadratwurzel */

         else if (strcmp(ftemp,"SQRT")==0)
          BEGIN
           if (LVal.Contents.Float<0)
            BEGIN
             Erg->Typ=TempNone; WrError(1870);
            END
           else Erg->Contents.Float=sqrt(LVal.Contents.Float);
          END

         /* trigonometrische Funktionen */

         else if (strcmp(ftemp,"SIN")==0) Erg->Contents.Float=sin(LVal.Contents.Float);
         else if (strcmp(ftemp,"COS")==0) Erg->Contents.Float=cos(LVal.Contents.Float);
         else if (strcmp(ftemp,"TAN")==0) 
          BEGIN
           if (cos(LVal.Contents.Float)==0.0)
            BEGIN
             Erg->Typ=TempNone; WrError(1870);
            END
           else Erg->Contents.Float=tan(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"COT")==0)
          BEGIN
           if ((FVal=sin(LVal.Contents.Float))==0.0)
            BEGIN
             Erg->Typ=TempNone; WrError(1870);
            END
           else Erg->Contents.Float=cos(LVal.Contents.Float)/FVal;
          END

         /* inverse trigonometrische Funktionen */

         else if (strcmp(ftemp,"ASIN")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)>1)
            BEGIN
             Erg->Typ=TempNone; WrError(1870);
            END
           else Erg->Contents.Float=asin(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"ACOS")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)>1)
            BEGIN
             Erg->Typ=TempNone; WrError(1870);
            END
           else Erg->Contents.Float=acos(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"ATAN")==0) Erg->Contents.Float=atan(LVal.Contents.Float);
         else if (strcmp(ftemp,"ACOT")==0) Erg->Contents.Float=M_PI/2-(LVal.Contents.Float);

         /* exponentielle & hyperbolische Funktionen */

         else if (strcmp(ftemp,"EXP")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else Erg->Contents.Float=exp(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"ALOG")==0)
          BEGIN
           if (LVal.Contents.Float>308)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else Erg->Contents.Float=exp(LVal.Contents.Float*log(10.0));
          END
         else if (strcmp(ftemp,"ALD")==0)
          BEGIN
           if (LVal.Contents.Float>1022)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else Erg->Contents.Float=exp(LVal.Contents.Float*log(2.0));
          END
         else if (strcmp(ftemp,"SINH")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else Erg->Contents.Float=sinh(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"COSH")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else Erg->Contents.Float=cosh(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"TANH")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else Erg->Contents.Float=tanh(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"COTH")==0)
          BEGIN
           if (LVal.Contents.Float>709)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else if ((FVal=tanh(LVal.Contents.Float))==0.0)
            BEGIN
             Erg->Typ=TempNone; WrError(1870);
            END
           else Erg->Contents.Float=1.0/FVal;
          END

         /* logarithmische & inverse hyperbolische Funktionen */

         else if (strcmp(ftemp,"LN")==0)
          BEGIN
           if (LVal.Contents.Float<=0)
            BEGIN
             Erg->Typ=TempNone; WrError(1870);
            END
           else Erg->Contents.Float=log(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"LOG")==0)
          BEGIN
           if (LVal.Contents.Float<=0)
            BEGIN
             Erg->Typ=TempNone; WrError(1870);
            END
           else Erg->Contents.Float=log10(LVal.Contents.Float);
          END
         else if (strcmp(ftemp,"LD")==0)
          BEGIN
           if (LVal.Contents.Float<=0)
            BEGIN
             Erg->Typ=TempNone; WrError(1870);
            END
           else Erg->Contents.Float=log(LVal.Contents.Float)/log(2.0);
          END
         else if (strcmp(ftemp,"ASINH")==0) 
          Erg->Contents.Float=log(LVal.Contents.Float+sqrt(LVal.Contents.Float*LVal.Contents.Float+1));
         else if (strcmp(ftemp,"ACOSH")==0)
          BEGIN
           if (LVal.Contents.Float<1)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else Erg->Contents.Float=log(LVal.Contents.Float+sqrt(LVal.Contents.Float*LVal.Contents.Float-1));
          END
         else if (strcmp(ftemp,"ATANH")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)>=1)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else Erg->Contents.Float=0.5*log((1+LVal.Contents.Float)/(1-LVal.Contents.Float));
          END
         else if (strcmp(ftemp,"ACOTH")==0)
          BEGIN
           if (fabs(LVal.Contents.Float)<=1)
            BEGIN
             Erg->Typ=TempNone; WrError(1880);
            END
           else Erg->Contents.Float=0.5*log((LVal.Contents.Float+1)/(LVal.Contents.Float-1));
          END

         /* nix gefunden ? */

         else
          BEGIN
           WrXError(1860,ftemp); Erg->Typ=TempNone;
          END
        END
      END
     return;
    END

   /* nichts dergleichen, dann einfaches Symbol: */

   /* interne Symbole ? */

   strmaxcpy(Asc,stemp,255); KillPrefBlanks(Asc); KillPostBlanks(Asc);

   if (strcasecmp(Asc,"MOMFILE")==0)
    BEGIN
     Erg->Typ=TempString;
     strmaxcpy(Erg->Contents.Ascii,CurrFileName,255);
     return;
    END;

   if (strcasecmp(Asc,"MOMLINE")==0)
    BEGIN
     Erg->Typ=TempInt;
     Erg->Contents.Int=CurrLine;
     return;
    END

   if (strcasecmp(Asc,"MOMPASS")==0)
    BEGIN
     Erg->Typ=TempInt;
     Erg->Contents.Int=PassNo;
     return;
    END

   if (strcasecmp(Asc,"MOMSECTION")==0)
    BEGIN
     Erg->Typ=TempString;
     strmaxcpy(Erg->Contents.Ascii,GetSectionName(MomSectionHandle),255);
     return;
    END

   if (strcasecmp(Asc,"MOMSEGMENT")==0)
    BEGIN
     Erg->Typ=TempString;
     strmaxcpy(Erg->Contents.Ascii,SegNames[ActPC],255);
     return;
    END

   if (NOT ExpandSymbol(Asc)) return;

   KlPos=strchr(Asc,'[');
   if (KlPos!=Nil) 
    BEGIN
     Save=(*KlPos); *KlPos='\0';
    END
   OK=ChkSymbName(Asc);
   if (KlPos!=Nil) *KlPos=Save;
   if (NOT OK)
    BEGIN
     WrXError(1020,Asc); return;
    END;

   Ptr=FindLocNode(Asc,TempInt);
   if (Ptr==Nil) Ptr=FindNode(Asc,TempInt);
   if (Ptr!=Nil)
    BEGIN
     Erg->Typ=TempInt; Erg->Contents.Int=Ptr->SymWert.Contents.IWert;
     if (Ptr->SymType!=0) TypeFlag|=(1 << Ptr->SymType);
     if ((Ptr->SymSize!=(-1)) AND (SizeFlag==(-1))) SizeFlag=Ptr->SymSize;
     if (NOT Ptr->Defined)
      BEGIN
       if (Repass) SymbolQuestionable=True;
       UsesForwards=True;
      END
     Ptr->Used=True;
     return;
    END
   Ptr=FindLocNode(Asc,TempFloat);
   if (Ptr==Nil) Ptr=FindNode(Asc,TempFloat);
   if (Ptr!=Nil)
    BEGIN
     Erg->Typ=TempFloat; Erg->Contents.Float=Ptr->SymWert.Contents.FWert;
     if (Ptr->SymType!=0) TypeFlag|=(1 << Ptr->SymType);
     if ((Ptr->SymSize!=(-1)) AND (SizeFlag==(-1))) SizeFlag=Ptr->SymSize;
     if (NOT Ptr->Defined)
      BEGIN
       if (Repass) SymbolQuestionable=True;
       UsesForwards=True;
      END
     Ptr->Used=True;
     return;
    END
   Ptr=FindLocNode(Asc,TempString);
   if (Ptr==Nil) Ptr=FindNode(Asc,TempString);
   if (Ptr!=Nil)
    BEGIN
     Erg->Typ=TempString; strmaxcpy(Erg->Contents.Ascii,Ptr->SymWert.Contents.SWert,255);
     if (Ptr->SymType!=0) TypeFlag|=(1 << Ptr->SymType);
     if ((Ptr->SymSize!=(-1)) AND (SizeFlag==(-1))) SizeFlag=Ptr->SymSize;
     if (NOT Ptr->Defined)
      BEGIN
       if (Repass) SymbolQuestionable=True;
       UsesForwards=True;
      END
     Ptr->Used=True;
     return;
    END

   /* Symbol evtl. im ersten Pass unbekannt */

   if (PassNo<=MaxSymPass)
    BEGIN
     Erg->Typ=TempInt; Erg->Contents.Int=EProgCounter();
     Repass=True;
     if ((MsgIfRepass) AND (PassNo>=PassNoForMessage)) WrXError(170,Asc);
     FirstPassUnknown=True;
    END

   /* alles war nix, Fehler */

   else WrXError(1010,Asc);
END


        LargeInt EvalIntExpression(char *Asc, IntType Typ, Boolean *OK)
BEGIN
   TempResult t;

   *OK=False;
   TypeFlag=0; SizeFlag=(-1);
   UsesForwards=False;
   SymbolQuestionable=False;
   FirstPassUnknown=False;

   EvalExpression(Asc,&t);
   if (t.Typ!=TempInt)
    BEGIN
     if (t.Typ!=TempNone) WrError(1135);
     return -1;
    END

   if (FirstPassUnknown) t.Contents.Int&=IntMasks[Typ];

   if (NOT RangeCheck(t.Contents.Int,Typ))
    BEGIN
     WrError(1320); return -1;
    END

   *OK=True; return t.Contents.Int;
END

        Double EvalFloatExpression(char *Asc, FloatType Typ, Boolean *OK)
BEGIN
   TempResult t;

   *OK=False;
   TypeFlag=0; SizeFlag=(-1);
   UsesForwards=False;
   SymbolQuestionable=False;
   FirstPassUnknown=False;

   EvalExpression(Asc,&t);
   switch (t.Typ)
    BEGIN
     case TempNone:
      return -1;
     case TempInt:
      t.Contents.Float=t.Contents.Int;
      break;
     case TempString:
      WrError(1135); return -1;
     default:
      break;
    END

   if (NOT FloatRangeCheck(t.Contents.Float,Typ))
    BEGIN
     WrError(1320); return -1;
    END

   *OK=True; return t.Contents.Float;
END

        void EvalStringExpression(char *Asc, Boolean *OK, char *Result)
BEGIN
   TempResult t;

   *OK=False;
   TypeFlag=0; SizeFlag=(-1);
   UsesForwards=False;
   SymbolQuestionable=False;
   FirstPassUnknown=False;

   EvalExpression(Asc,&t);
   if (t.Typ!=TempString)
    BEGIN
     *Result='\0';
     if (t.Typ!=TempNone) WrError(1135);
     return;
    END

   strmaxcpy(Result,t.Contents.Ascii,255); *OK=True;
END


        static void FreeSymbol(SymbolPtr *Node)
BEGIN
   PCrossRef Lauf;

   free((*Node)->SymName);

   if ((*Node)->SymWert.Typ==TempString)
    free((*Node)->SymWert.Contents.SWert);

   while ((*Node)->RefList!=Nil)
    BEGIN
     Lauf=(*Node)->RefList->Next;
     free((*Node)->RefList);
     (*Node)->RefList=Lauf;
    END

   free(*Node); *Node=Nil;
END

static String serr;
static char snum[11];

        Boolean EnterTreeNode(SymbolPtr *Node, SymbolPtr Neu, Boolean MayChange, Boolean DoCross)
BEGIN
   SymbolPtr Hilf,p1,p2;
   Boolean Grown,Result;
   ShortInt CompErg;

   /* Stapelueberlauf pruefen, noch nichts eingefuegt */

   ChkStack(); Result=False;

   /* an einem Blatt angelangt--> einfach anfuegen */

   if (*Node==Nil)
    BEGIN
     (*Node)=Neu;
     (*Node)->Balance=0; (*Node)->Left=Nil; (*Node)->Right=Nil;
     (*Node)->Defined=True; (*Node)->Used=False; (*Node)->Changeable=MayChange;
     (*Node)->RefList=Nil;
     if (DoCross)
      BEGIN
       (*Node)->FileNum=GetFileNum(CurrFileName);
       (*Node)->LineNum=CurrLine;
      END
     return True;
    END

   CompErg=StrCmp(Neu->SymName,(*Node)->SymName,Neu->Attribute,(*Node)->Attribute);

   switch (CompErg)
    BEGIN
     case 1:
      Grown=EnterTreeNode(&((*Node)->Right),Neu,MayChange,DoCross);
      if ((BalanceTree) AND (Grown))
       switch ((*Node)->Balance)
        BEGIN
         case -1:
          (*Node)->Balance=0; break;
         case 0:
          (*Node)->Balance=1; Result=True; break;
         case 1:
          p1=(*Node)->Right;
          if (p1->Balance==1)
           BEGIN
            (*Node)->Right=p1->Left; p1->Left=(*Node);
            (*Node)->Balance=0; *Node=p1;
           END
          else
           BEGIN
            p2=p1->Left;
            p1->Left=p2->Right; p2->Right=p1;
            (*Node)->Right=p2->Left; p2->Left=(*Node);
            if (p2->Balance== 1) (*Node)->Balance=(-1); else (*Node)->Balance=0;
            if (p2->Balance==-1) p1     ->Balance=   1; else p1     ->Balance=0;
            *Node=p2;
           END
          (*Node)->Balance=0;
          break;
        END
      break;
     case -1:
      Grown=EnterTreeNode(&((*Node)->Left),Neu,MayChange,DoCross);
      if ((BalanceTree) AND (Grown))
       switch ((*Node)->Balance)
        BEGIN
         case 1:
          (*Node)->Balance=0; break;
         case 0:
          (*Node)->Balance=(-1); Result=True; break;
         case -1:
          p1=(*Node)->Left;
          if (p1->Balance==(-1))
           BEGIN
            (*Node)->Left=p1->Right; p1->Right=(*Node);
            (*Node)->Balance=0; (*Node)=p1;
           END
          else
           BEGIN
            p2=p1->Right;
            p1->Right=p2->Left; p2->Left=p1;
            (*Node)->Left=p2->Right; p2->Right=(*Node);
            if (p2->Balance==(-1)) (*Node)->Balance=   1; else (*Node)->Balance=0;
            if (p2->Balance==   1) p1     ->Balance=(-1); else p1     ->Balance=0;
            *Node=p2;
           END
          (*Node)->Balance=0;
          break;
        END
      break;
     case 0:
      if (((*Node)->Defined) AND (NOT MayChange))
       BEGIN
        strmaxcpy(serr,(*Node)->SymName,255);
        if (DoCross)
         BEGIN
          sprintf(snum,"%d",(*Node)->LineNum);
          strmaxcat(serr,", ",255); 
          strmaxcat(serr,PrevDefMsg,255);
          strmaxcat(serr," ",255); 
          strmaxcat(serr,GetFileName((*Node)->FileNum),255);
          strmaxcat(serr,":",255);
          strmaxcat(serr,snum,255);
         END
        WrXError(1000,serr);
       END
      else
       BEGIN
        if (NOT MayChange)
         BEGIN
          if ((Neu->SymWert.Typ!=(*Node)->SymWert.Typ)
           OR ((Neu->SymWert.Typ==TempString) AND (strcmp(Neu->SymWert.Contents.SWert,(*Node)->SymWert.Contents.SWert)!=0))
           OR ((Neu->SymWert.Typ==TempFloat ) AND (Neu->SymWert.Contents.FWert !=(*Node)->SymWert.Contents.FWert ))
           OR ((Neu->SymWert.Typ==TempInt   ) AND (Neu->SymWert.Contents.IWert !=(*Node)->SymWert.Contents.IWert )))
            BEGIN
             if ((NOT Repass) AND (JmpErrors>0))
              BEGIN
               if (ThrowErrors) ErrorCount-=JmpErrors;
               JmpErrors=0;
              END
             Repass=True;
             if ((MsgIfRepass) AND (PassNo>=PassNoForMessage))
              BEGIN
               strmaxcpy(serr,Neu->SymName,255);
               if (Neu->Attribute!=(-1)) 
                BEGIN
                 strmaxcat(serr,"[",255);
                 strmaxcat(serr,GetSectionName(Neu->Attribute),255);
                 strmaxcat(serr,"]",255);
                END
               WrXError(80,serr);
              END
            END
         END;
        Neu->Left=(*Node)->Left; Neu->Right=(*Node)->Right; 
        Neu->Balance=(*Node)->Balance;
        if (DoCross)
         BEGIN
          Neu->LineNum=(*Node)->LineNum; Neu->FileNum=(*Node)->FileNum;
         END
        Neu->RefList=(*Node)->RefList; (*Node)->RefList=Nil;
        Neu->Defined=True; Neu->Used=(*Node)->Used; Neu->Changeable=MayChange;
        Hilf=(*Node); *Node=Neu;
        FreeSymbol(&Hilf);
       END
      break;
    END
   return Result;
END

        static void EnterLocSymbol(SymbolPtr Neu)
BEGIN
   Neu->Attribute=MomLocHandle;
   if (NOT CaseSensitive) NLS_UpString(Neu->SymName);
   EnterTreeNode(&FirstLocSymbol,Neu,False,False);
END

        static void EnterSymbol_Search(PForwardSymbol *Lauf, PForwardSymbol *Prev,
                                       PForwardSymbol **RRoot, SymbolPtr Neu,
                                       PForwardSymbol *Root, Byte ResCode, Byte *SearchErg)
BEGIN
   *Lauf=(*Root); *Prev=Nil; *RRoot=Root;
   while ((*Lauf!=Nil) AND (strcmp((*Lauf)->Name,Neu->SymName)!=0))
    BEGIN
     *Prev=(*Lauf); *Lauf=(*Lauf)->Next;
    END
   if (*Lauf!=Nil) *SearchErg=ResCode;
END

        static void EnterSymbol(SymbolPtr Neu, Boolean MayChange, LongInt ResHandle)
BEGIN
   PForwardSymbol Lauf,Prev;
   PForwardSymbol *RRoot;
   Byte SearchErg;
   String CombName;
   PSaveSection RunSect;
   LongInt MSect;
   SymbolPtr Copy;

/*   Neu^.Attribute:=MomSectionHandle;
   IF SectionStack<>Nil THEN
    BEGIN
     Search(SectionStack^.GlobSyms);
     IF Lauf<>Nil THEN Neu^.Attribute:=Lauf^.DestSection
     ELSE Search(SectionStack^.LocSyms);
     IF Lauf<>Nil THEN
      BEGIN
       FreeMem(Lauf^.Name,Length(Lauf^.Name^)+1);
       IF Prev=Nil THEN RRoot^:=Lauf^.Next
       ELSE Prev^.Next:=Lauf^.Next;
       Dispose(Lauf);
      END;
    END;
   IF EnterTreeNode(FirstSymbol,Neu,MayChange,MakeCrossList) THEN;*/

   if (NOT CaseSensitive) NLS_UpString(Neu->SymName);

   SearchErg=0;
   Neu->Attribute=(ResHandle==(-2))?(MomSectionHandle):(ResHandle);
   if ((SectionStack!=Nil) AND (Neu->Attribute==MomSectionHandle))
    BEGIN
     EnterSymbol_Search(&Lauf,&Prev,&RRoot,Neu,&(SectionStack->LocSyms),1,&SearchErg);
     if (Lauf==Nil) EnterSymbol_Search(&Lauf,&Prev,&RRoot,Neu,&(SectionStack->GlobSyms),2,&SearchErg);
     if (Lauf==Nil) EnterSymbol_Search(&Lauf,&Prev,&RRoot,Neu,&(SectionStack->ExportSyms),3,&SearchErg);
     if (SearchErg==2) Neu->Attribute=Lauf->DestSection;
     if (SearchErg==3)
      BEGIN
       strmaxcpy(CombName,Neu->SymName,255);
       RunSect=SectionStack; MSect=MomSectionHandle;
       while ((MSect!=Lauf->DestSection) AND (RunSect!=Nil))
        BEGIN
         strmaxprep(CombName,"_",255);
         strmaxprep(CombName,GetSectionName(MSect),255);
         MSect=RunSect->Handle; RunSect=RunSect->Next;
        END
       Copy=(SymbolPtr) malloc(sizeof(SymbolEntry)); *Copy=(*Neu);
       Copy->SymName=strdup(CombName);
       Copy->Attribute=Lauf->DestSection;
       if (Copy->SymWert.Typ==TempString) 
        Copy->SymWert.Contents.SWert=strdup(Neu->SymWert.Contents.SWert);
       EnterTreeNode(&FirstSymbol,Copy,MayChange,MakeCrossList);
      END
     if (Lauf!=Nil)
      BEGIN
       free(Lauf->Name);
       if (Prev==Nil) *RRoot=Lauf->Next;
       else Prev->Next=Lauf->Next;
       free(Lauf);
      END
    END
   EnterTreeNode(&FirstSymbol,Neu,MayChange,MakeCrossList);
END

        void PrintSymTree(char *Name)
BEGIN
   fprintf(Debug,"---------------------\n");
   fprintf(Debug,"Enter Symbol %s\n\n",Name);
   PrintSymbolTree(); PrintSymbolDepth();
END

        void EnterIntSymbol(char *Name_O, LargeInt Wert, Byte Typ, Boolean MayChange)
BEGIN
   SymbolPtr Neu;
   LongInt DestHandle;   
   String Name;

   strmaxcpy(Name,Name_O,255);
   if (NOT ExpandSymbol(Name)) return;
   if (NOT GetSymSection(Name,&DestHandle)) return;
   if (NOT ChkSymbName(Name))
    BEGIN
     WrXError(1020,Name); return;
    END

   Neu=(SymbolPtr) malloc(sizeof(SymbolEntry));
   Neu->SymName=strdup(Name);
   Neu->SymWert.Typ=TempInt;
   Neu->SymWert.Contents.IWert=Wert;
   Neu->SymType=Typ;
   Neu->SymSize=(-1);

   if ((MomLocHandle==(-1)) OR (DestHandle!=(-2)))
    BEGIN
     EnterSymbol(Neu,MayChange,DestHandle);
     if (MakeDebug) PrintSymTree(Name);
    END
   else EnterLocSymbol(Neu);
END

        void EnterFloatSymbol(char *Name_O, Double Wert, Boolean MayChange)
BEGIN
   SymbolPtr Neu;
   LongInt DestHandle;
   String Name;

   strmaxcpy(Name,Name_O,255);
   if (NOT ExpandSymbol(Name)) return;
   if (NOT GetSymSection(Name,&DestHandle)) return;
   if (NOT ChkSymbName(Name))
    BEGIN
     WrXError(1020,Name); return;
    END
   Neu=(SymbolPtr) malloc(sizeof(SymbolEntry));
   Neu->SymName=strdup(Name);
   Neu->SymWert.Typ=TempFloat;
   Neu->SymWert.Contents.FWert=Wert;
   Neu->SymType=0;
   Neu->SymSize=(-1);

   if ((MomLocHandle==(-1)) OR (DestHandle!=(-2)))
    BEGIN
     EnterSymbol(Neu,MayChange,DestHandle);
     if (MakeDebug) PrintSymTree(Name);
    END
   else EnterLocSymbol(Neu);
END

        void EnterStringSymbol(char *Name_O, char *Wert, Boolean MayChange)
BEGIN
   SymbolPtr Neu;
   LongInt DestHandle;
   String Name;

   strmaxcpy(Name,Name_O,255);
   if (NOT ExpandSymbol(Name)) return;
   if (NOT GetSymSection(Name,&DestHandle)) return;
   if (NOT ChkSymbName(Name))
    BEGIN
     WrXError(1020,Name); return;
    END
   Neu=(SymbolPtr) malloc(sizeof(SymbolEntry));
   Neu->SymName=strdup(Name);
   Neu->SymWert.Contents.SWert=strdup(Wert);
   Neu->SymWert.Typ=TempString;
   Neu->SymType=0;
   Neu->SymSize=(-1);

   if ((MomLocHandle==(-1)) OR (DestHandle!=(-2)))
    BEGIN
     EnterSymbol(Neu,MayChange,DestHandle);
     if (MakeDebug) PrintSymTree(Name);
    END
   else EnterLocSymbol(Neu);
END

        static void AddReference(SymbolPtr Node)
BEGIN
   PCrossRef Lauf,Neu;

   /* Speicher belegen */

   Neu=(PCrossRef) malloc(sizeof(TCrossRef));
   Neu->LineNum=CurrLine; Neu->OccNum=1; Neu->Next=Nil;

   /* passende Datei heraussuchen */

   Neu->FileNum=GetFileNum(CurrFileName);

   /* suchen, ob Eintrag schon existiert */

   Lauf=Node->RefList;
   while ((Lauf!=Nil)
      AND ((Lauf->FileNum!=Neu->FileNum) OR (Lauf->LineNum!=Neu->LineNum)))
    Lauf=Lauf->Next;

   /* schon einmal in dieser Datei in dieser Zeile aufgetaucht: nur Zaehler
     rauf: */

   if (Lauf!=Nil)
    BEGIN
     Lauf->OccNum++; free(Neu);
    END

   /* ansonsten an Kettenende anhaengen */

   else if (Node->RefList==Nil) Node->RefList=Neu;

   else
    BEGIN
     Lauf=Node->RefList;
     while (Lauf->Next!=Nil) Lauf=Lauf->Next;
     Lauf->Next=Neu;
    END
END

        static Boolean FindNode_FNode(char *Name, TempType SearchType, 
                                      SymbolPtr *FindNode_Result, LongInt Handle)
BEGIN
   SymbolPtr Lauf=FirstSymbol;
   ShortInt SErg=(-1);
   Boolean Result=False;
 
   while ((Lauf!=Nil) AND (SErg!=0))
    BEGIN
     SErg=StrCmp(Name,Lauf->SymName,Handle,Lauf->Attribute);
     if (SErg==(-1)) Lauf=Lauf->Left;
     else if (SErg==1) Lauf=Lauf->Right;
    END
   if (Lauf!=Nil)
    if (Lauf->SymWert.Typ==SearchType)
     BEGIN
      *FindNode_Result=Lauf; Result=True;
      if (MakeCrossList AND DoRefs) AddReference(Lauf);
     END
   
   return Result;
END

        static Boolean FindNode_FSpec(char *Name, PForwardSymbol Root)
BEGIN
   while ((Root!=Nil) AND (strcmp(Root->Name,Name)!=0)) Root=Root->Next;
   return (Root!=Nil);
END

        static SymbolPtr FindNode(char *Name_O, TempType SearchType)
BEGIN
   PSaveSection Lauf;
   LongInt DestSection;
   SymbolPtr FindNode_Result;
   String Name;

   strmaxcpy(Name,Name_O,255);
   FindNode_Result=Nil;
   if (NOT GetSymSection(Name,&DestSection)) return FindNode_Result;
   if (NOT CaseSensitive) NLS_UpString(Name);
   if (SectionStack!=Nil)
    if (PassNo<=MaxSymPass)
     if (FindNode_FSpec(Name,SectionStack->LocSyms)) DestSection=MomSectionHandle;
/* if (FSpec(SectionStack->GlobSyms)) return; */
   if (DestSection==(-2))
    BEGIN
     if (FindNode_FNode(Name,SearchType,&FindNode_Result,MomSectionHandle)) return FindNode_Result;
     Lauf=SectionStack;
     while (Lauf!=Nil)
      BEGIN
       if (FindNode_FNode(Name,SearchType,&FindNode_Result,Lauf->Handle)) return FindNode_Result;
       Lauf=Lauf->Next;
      END
    END
   else FindNode_FNode(Name,SearchType,&FindNode_Result,DestSection);

   return FindNode_Result;
END

        static Boolean FindLocNode_FNode(char *Name, TempType SearchType,
                                         SymbolPtr *FindLocNode_Result, LongInt Handle)
BEGIN
   SymbolPtr Lauf=FirstLocSymbol;
   ShortInt SErg=(-1);
   Boolean Result=False;

   while ((Lauf!=Nil) AND (SErg!=0))
    BEGIN
     SErg=StrCmp(Name,Lauf->SymName,Handle,Lauf->Attribute);
     if (SErg==(-1)) Lauf=Lauf->Left;
     else if (SErg==1) Lauf=Lauf->Right;
    END

   if (Lauf!=Nil)
    if (Lauf->SymWert.Typ==SearchType)
     BEGIN
      *FindLocNode_Result=Lauf; Result=True;
     END

   return Result;
END

        static SymbolPtr FindLocNode(char *Name_O, TempType SearchType)
BEGIN
   PLocHandle RunLocHandle;
   SymbolPtr FindLocNode_Result;
   String Name;

   FindLocNode_Result=Nil;

   strmaxcpy(Name,Name_O,255); if (NOT CaseSensitive) NLS_UpString(Name);

   if (MomLocHandle==(-1)) return FindLocNode_Result;

   if (FindLocNode_FNode(Name,SearchType,&FindLocNode_Result,MomLocHandle))
    return FindLocNode_Result;

   RunLocHandle=FirstLocHandle;
   while ((RunLocHandle!=Nil) AND (RunLocHandle->Cont!=(-1)))
    BEGIN
     if (FindLocNode_FNode(Name,SearchType,&FindLocNode_Result,RunLocHandle->Cont)) 
      return FindLocNode_Result;
     RunLocHandle=RunLocHandle->Next;
    END

   return FindLocNode_Result;
END
/**
        void SetSymbolType(char *Name, Byte NTyp)
BEGIN
   Lauf:SymbolPtr;
   HRef:Boolean;

   IF NOT ExpandSymbol(Name) THEN Exit;
   HRef:=DoRefs; DoRefs:=False;
   Lauf:=FindLocNode(Name,TempInt);
   IF Lauf=Nil THEN Lauf:=FindNode(Name,TempInt);
   IF Lauf<>Nil THEN Lauf^.SymType:=NTyp;
   DoRefs:=HRef;
END
**/

        Boolean GetIntSymbol(char *Name, LargeInt *Wert)
BEGIN
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;
   Lauf=FindLocNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindNode(NName,TempInt);
   if (Lauf!=Nil)
    BEGIN
     *Wert=Lauf->SymWert.Contents.IWert;
     if (Lauf->SymType!=0) TypeFlag|=(1<<Lauf->SymType);
     if ((Lauf->SymSize!=(-1)) AND (SizeFlag!=(-1))) SizeFlag=Lauf->SymSize;
     Lauf->Used=True;
    END
   else
    BEGIN
     if (PassNo>MaxSymPass) WrXError(1010,Name);
     *Wert=EProgCounter();
    END
   return (Lauf!=Nil);
END

        Boolean GetFloatSymbol(char *Name, Double *Wert)
BEGIN
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;
   Lauf=FindLocNode(Name,TempFloat);
   if (Lauf==Nil) Lauf=FindNode(NName,TempFloat);
   if (Lauf!=Nil)
    BEGIN
     *Wert=Lauf->SymWert.Contents.FWert;
     Lauf->Used=True;
    END
   else
    BEGIN
     if (PassNo>MaxSymPass) WrXError(1010,Name);
     *Wert=0;
    END
   return (Lauf!=Nil);
END

        Boolean GetStringSymbol(char *Name, char *Wert)
BEGIN
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;
   Lauf=FindLocNode(NName,TempString);
   if (Lauf==Nil) Lauf=FindNode(NName,TempString);
   if (Lauf!=Nil)
    BEGIN
     strcpy(Wert,Lauf->SymWert.Contents.SWert);
     Lauf->Used=True;
    END
   else
    BEGIN
     if (PassNo>MaxSymPass) WrXError(1010,Name);
     *Wert='\0';
    END
   return (Lauf!=Nil);
END

        void SetSymbolSize(char *Name, ShortInt Size)
BEGIN
   SymbolPtr Lauf;
   Boolean HRef;
   String NName;

   strmaxcpy(NName,Name,255); 
   if (NOT ExpandSymbol(NName)) return;
   HRef=DoRefs; DoRefs=False;
   Lauf=FindLocNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindNode(Name,TempInt);
   if (Lauf!=Nil) Lauf->SymSize=Size;
   DoRefs=HRef;
END

        ShortInt GetSymbolSize(char *Name)
BEGIN
   SymbolPtr Lauf;
   String NName;
   
   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return -1;
   Lauf=FindLocNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindNode(NName,TempInt);
   return ((Lauf!=Nil) ? Lauf->SymSize : -1);
END

        Boolean IsSymbolFloat(char *Name)
BEGIN
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf=FindLocNode(NName,TempFloat);
   if (Lauf==Nil) Lauf=FindNode(NName,TempFloat);
   return ((Lauf!=Nil) AND (Lauf->SymWert.Typ==TempFloat));
END

        Boolean IsSymbolString(char *Name)
BEGIN
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf=FindLocNode(NName,TempString);
   if (Lauf==Nil) Lauf=FindNode(NName,TempString);
   return ((Lauf!=Nil) AND (Lauf->SymWert.Typ==TempString));
END

        Boolean IsSymbolDefined(char *Name)
BEGIN
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf=FindLocNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindLocNode(NName,TempFloat);
   if (Lauf==Nil) Lauf=FindLocNode(NName,TempString);
   if (Lauf==Nil) Lauf=FindNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindNode(NName,TempFloat);
   if (Lauf==Nil) Lauf=FindNode(NName,TempString);
   return ((Lauf!=Nil) AND (Lauf->Defined));
END

        Boolean IsSymbolUsed(char *Name)
BEGIN
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf=FindLocNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindLocNode(NName,TempFloat);
   if (Lauf==Nil) Lauf=FindLocNode(NName,TempString);
   if (Lauf==Nil) Lauf=FindNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindNode(NName,TempFloat);
   if (Lauf==Nil) Lauf=FindNode(NName,TempString);
   return ((Lauf!=Nil) AND (Lauf->Used));
END

        Boolean IsSymbolChangeable(char *Name)
BEGIN
   SymbolPtr Lauf;
   String NName;

   strmaxcpy(NName,Name,255);
   if (NOT ExpandSymbol(NName)) return False;

   Lauf=FindLocNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindLocNode(NName,TempFloat);
   if (Lauf==Nil) Lauf=FindLocNode(NName,TempString);
   if (Lauf==Nil) Lauf=FindNode(NName,TempInt);
   if (Lauf==Nil) Lauf=FindNode(NName,TempFloat);
   if (Lauf==Nil) Lauf=FindNode(NName,TempString);
   return ((Lauf!=Nil) AND (Lauf->Changeable));
END

        static void ConvertSymbolVal(SymbolVal *Inp, TempResult *Outp)
BEGIN
   switch (Outp->Typ=Inp->Typ)
    BEGIN
     case TempInt   :Outp->Contents.Int  =Inp->Contents.IWert; break;
     case TempFloat :Outp->Contents.Float=Inp->Contents.FWert; break;
     case TempString:strmaxcpy(Outp->Contents.Ascii,Inp->Contents.SWert,255); break;
     default: break;
    END
END

        static void PrintSymbolList_AddOut(char *s, char *Zeilenrest, Integer Width)
BEGIN
   if (strlen(s)+strlen(Zeilenrest)>Width)
    BEGIN
     Zeilenrest[strlen(Zeilenrest)-1]='\0';
     WrLstLine(Zeilenrest); strmaxcpy(Zeilenrest,s,255);
    END
   else strmaxcat(Zeilenrest,s,255);
END

        static void PrintSymbolList_PNode(SymbolPtr Node, Integer Width,
                                          LongInt *Sum, LongInt *USum,
                                          char *Zeilenrest)
BEGIN
   String s1,sh;
   int l1;
   TempResult t;

   ConvertSymbolVal(&(Node->SymWert),&t); StrSym(&t,False,s1);

   strmaxcpy(sh,Node->SymName,255);
   if (Node->Attribute!=(-1)) 
    BEGIN
     strmaxcat(sh," [",255);
     strmaxcat(sh,GetSectionName(Node->Attribute),255);
     strmaxcat(sh,"]",255);
    END
   strmaxprep(sh,(Node->Used)?" ":"*",255);
   l1=(strlen(s1)+strlen(sh)+6)%40;
   if (l1<38) strmaxprep(s1,Blanks(38-l1),255);
   strmaxprep(s1," : ",255);
   strmaxprep(s1,sh,255);
   strmaxcat(s1," ",255);
   s1[l1=strlen(s1)]=SegShorts[Node->SymType]; s1[l1+1]='\0';
   strmaxcat(s1," | ",255);
   PrintSymbolList_AddOut(s1,Zeilenrest,Width); (*Sum)++;
   if (NOT Node->Used) (*USum)++;
END

        static void PrintSymbolList_PrintNode(SymbolPtr Node, Integer Width,
                                              LongInt *Sum, LongInt *USum,
                                              char *Zeilenrest)
BEGIN
   ChkStack();

   if (Node==Nil) return;

   PrintSymbolList_PrintNode(Node->Left,Width,Sum,USum,Zeilenrest);
   PrintSymbolList_PNode(Node,Width,Sum,USum,Zeilenrest);
   PrintSymbolList_PrintNode(Node->Right,Width,Sum,USum,Zeilenrest);
END

        void PrintSymbolList(void)
BEGIN
   Integer Width;
   String Zeilenrest;
   LongInt Sum,USum;

   Width=(PageWidth==0)?80:PageWidth;
   NewPage(ChapDepth,True);
   WrLstLine(ListSymListHead1);
   WrLstLine(ListSymListHead2);
   WrLstLine("");

   Zeilenrest[0]='\0'; Sum=0; USum=0;
   PrintSymbolList_PrintNode(FirstSymbol,Width,&Sum,&USum,Zeilenrest);
   if (Zeilenrest[0]!='\0')
    BEGIN
     Zeilenrest[strlen(Zeilenrest)-1]='\0';
     WrLstLine(Zeilenrest);
    END
   WrLstLine("");
   sprintf(Zeilenrest,"%7d",Sum);
   strmaxcat(Zeilenrest,(Sum==1)?ListSymSumMsg:ListSymSumsMsg,255);
   WrLstLine(Zeilenrest);
   sprintf(Zeilenrest,"%7d",USum);
   strmaxcat(Zeilenrest,(USum==1)?ListUSymSumMsg:ListUSymSumsMsg,255);
   WrLstLine(Zeilenrest);
   WrLstLine("");
END

static Boolean HWritten;
static int Space;

	static void PrintDebSymbols_PNode(FILE *f, SymbolPtr Node)
BEGIN
   char *p;
   int l1;
   TempResult t;
   String s;

   if (NOT HWritten)
    BEGIN
     fprintf(f,"\n"); ChkIO(10004);
     fprintf(f,"Symbols in Segment %s\n",SegNames[Space]); ChkIO(10004);
     HWritten=True;
    END

   fprintf(f,"%s",Node->SymName); ChkIO(10004); l1=strlen(Node->SymName);
   if (Node->Attribute!=(-1))
    BEGIN
     sprintf(s,"[%d]",Node->Attribute);
     fprintf(f,"%s",s); ChkIO(10004);
     l1+=strlen(s);
    END
   fprintf(f,"%s ",Blanks(37-l1)); ChkIO(10004);
   switch (Node->SymWert.Typ)
    BEGIN
     case TempInt:    fprintf(f,"Int    "); break;
     case TempFloat:  fprintf(f,"Float  "); break;
     case TempString: fprintf(f,"String "); break;
     default: break;
    END
   ChkIO(10004);
   if (Node->SymWert.Typ==TempString)
    BEGIN
     l1=0; 
     for (p=Node->SymWert.Contents.SWert; *p!='\0'; p++)
      BEGIN
       if ((*p=='\\') OR (*p<=' '))
        BEGIN
         fprintf(f,"\\%03d",*p); l1+=4;
        END
       else
        BEGIN
         fputc(*p,f); ChkIO(10004); l1++;
        END
      END
    END
   else
    BEGIN
     ConvertSymbolVal(&(Node->SymWert),&t); StrSym(&t,False,s);
     l1=strlen(s);
     fprintf(f,"%s",s); ChkIO(10004);
    END
   fprintf(f,"%s %-3d %d\n",Blanks(25-l1),Node->SymSize,Node->Used); ChkIO(10004);
END

	static void PrintDebSymbols_PrintNode(FILE *f, SymbolPtr Node)
BEGIN
   ChkStack();

   if (Node==Nil) return;

   PrintDebSymbols_PrintNode(f,Node->Left);

   if (Node->SymType==Space) PrintDebSymbols_PNode(f,Node);

   PrintDebSymbols_PrintNode(f,Node->Right);
END

	void PrintDebSymbols(FILE *f)
BEGIN
   for (Space=0; Space<PCMax; Space++)
    BEGIN
     HWritten=False;
     PrintDebSymbols_PrintNode(f,FirstSymbol);
    END
END

        static void PrintSymbolTree_PrintNode(SymbolPtr Node, Integer Shift)
BEGIN
   Byte z;

   if (Node==Nil) return;

   PrintSymbolTree_PrintNode(Node->Left,Shift+1);

   for (z=1; z<=Shift; z++) fprintf(Debug,"%6s","");
   fprintf(Debug,"%s\n",Node->SymName);

   PrintSymbolTree_PrintNode(Node->Right,Shift+1);
END

        void PrintSymbolTree(void)
BEGIN
   PrintSymbolTree_PrintNode(FirstSymbol,0);
END

        static void ClearSymbolList_ClearNode(SymbolPtr *Node)
BEGIN
   if ((*Node)->Left!=Nil) ClearSymbolList_ClearNode(&((*Node)->Left));
   if ((*Node)->Right!=Nil) ClearSymbolList_ClearNode(&((*Node)->Right));
   FreeSymbol(Node);
END

        void ClearSymbolList(void)
BEGIN

   if (FirstSymbol!=Nil) ClearSymbolList_ClearNode(&FirstSymbol);

   if (FirstLocSymbol!=Nil) ClearSymbolList_ClearNode(&FirstLocSymbol);
END

/*-------------------------------------------------------------------------*/
/* Stack-Verwaltung */

        Boolean PushSymbol(char *SymName_O, char *StackName_O)
BEGIN
   SymbolPtr Src;
   PSymbolStack LStack,NStack,PStack;
   PSymbolStackEntry Elem;
   String SymName,StackName;

   strmaxcpy(SymName,SymName_O,255);
   if (NOT ExpandSymbol(SymName)) return False;

   Src=FindNode(SymName,TempInt);
   if (Src==Nil) Src=FindNode(SymName,TempFloat);
   if (Src==Nil) Src=FindNode(SymName,TempString);
   if (Src==Nil)
    BEGIN
     WrXError(1010,SymName); return False;
    END

   strmaxcpy(StackName,(*StackName_O=='\0')?DefStackName:StackName_O,255);
   if (NOT ExpandSymbol(StackName)) return False;
   if (NOT ChkSymbName(StackName))
    BEGIN
     WrXError(1020,StackName); return False;
    END

   LStack=FirstStack; PStack=Nil;
   while ((LStack!=Nil) AND (strcmp(LStack->Name,StackName)<0))
    BEGIN
     PStack=LStack;
     LStack=LStack->Next;
    END

   if ((LStack==Nil) OR (strcmp(LStack->Name,StackName)>0))
    BEGIN
     NStack=(PSymbolStack) malloc(sizeof(TSymbolStack));
     NStack->Name=strdup(StackName);
     NStack->Contents=Nil;
     NStack->Next=LStack;
     if (PStack==Nil) FirstStack=NStack; else PStack->Next=NStack;
     LStack=NStack;
    END

   Elem=(PSymbolStackEntry) malloc(sizeof(TSymbolStackEntry));
   Elem->Next=LStack->Contents;
   Elem->Contents=Src->SymWert;
   LStack->Contents=Elem;

   return True;
END

        Boolean PopSymbol(char *SymName_O, char *StackName_O)
BEGIN
   SymbolPtr Dest;
   PSymbolStack LStack,PStack;
   PSymbolStackEntry Elem;
   String SymName,StackName;

   strmaxcpy(SymName,SymName_O,255);
   if (NOT ExpandSymbol(SymName)) return False;

   Dest=FindNode(SymName,TempInt);
   if (Dest==Nil) Dest=FindNode(SymName,TempFloat);
   if (Dest==Nil) Dest=FindNode(SymName,TempString);
   if (Dest==Nil) 
    BEGIN
     WrXError(1010,SymName); return False;
    END

   strmaxcpy(StackName,(*StackName_O=='\0')?DefStackName:StackName_O,255);
   if (NOT ExpandSymbol(StackName)) return False;
   if (NOT ChkSymbName(StackName))
    BEGIN
     WrXError(1020,StackName); return False;
    END

   LStack=FirstStack; PStack=Nil;
   while ((LStack!=Nil) AND (strcmp(LStack->Name,StackName)<0))
    BEGIN
     PStack=LStack;
     LStack=LStack->Next;
    END

   if ((LStack==Nil) OR (strcmp(LStack->Name,StackName)>0))
    BEGIN
     WrXError(1530,StackName); return False;
    END

   Elem=LStack->Contents;
   Dest->SymWert=Elem->Contents;
   if ((LStack->Contents=Elem->Next)==Nil)
    BEGIN
     if (PStack==Nil) FirstStack=LStack->Next; else PStack->Next=LStack->Next;
     free(LStack->Name);
     free(LStack);
    END
   free(Elem);

   return True;
END

        void ClearStacks(void)
BEGIN
   PSymbolStack Act;
   PSymbolStackEntry Elem;
   Integer z;
   String s;

   while (FirstStack!=Nil)
    BEGIN
     z=0; Act=FirstStack;
     while (Act->Contents!=Nil)
      BEGIN
       Elem=Act->Contents; Act->Contents=Elem->Next;
       free(Elem); z++;
      END
     sprintf(s,"%s(%d)",Act->Name,z);
     WrXError(230,s);
     free(Act->Name);
     FirstStack=Act->Next; free(Act);
    END
END

/*-------------------------------------------------------------------------*/
/* Funktionsverwaltung */

        void EnterFunction(char *FName, char *FDefinition, Byte NewCnt)
BEGIN
   PFunction Neu;
   String FName_N;

   if (NOT CaseSensitive)
    BEGIN
     strmaxcpy(FName_N,FName,255); NLS_UpString(FName_N); FName=FName_N;
    END

   if (NOT ChkSymbName(FName))
    BEGIN
     WrXError(1020,FName); return;
    END

   if (FindFunction(FName)!=Nil)
    BEGIN
     if (PassNo==1) WrXError(1000,FName); return;
    END

   Neu=(PFunction) malloc(sizeof(TFunction));
   Neu->Next=FirstFunction; Neu->ArguCnt=NewCnt;
   Neu->Name=strdup(FName);
   Neu->Definition=strdup(FDefinition);
   FirstFunction=Neu;
END

        PFunction FindFunction(char *Name)
BEGIN
   PFunction Lauf=FirstFunction;  
   String Name_N;

   if (NOT CaseSensitive)
    BEGIN
     strmaxcpy(Name_N,Name,255); NLS_UpString(Name_N); Name=Name_N;
    END
 
   while ((Lauf!=Nil) AND (strcmp(Lauf->Name,Name)!=0)) Lauf=Lauf->Next;
   return Lauf;
END

        void PrintFunctionList(void)
BEGIN
   PFunction Lauf;
   String OneS;
   Boolean cnt;

   if (FirstFunction==Nil) return;

   NewPage(ChapDepth,True);
   WrLstLine(ListFuncListHead1);
   WrLstLine(ListFuncListHead2);
   WrLstLine("");

   OneS[0]='\0'; Lauf=FirstFunction; cnt=False;
   while (Lauf!=Nil)
    BEGIN
     strmaxcat(OneS,Lauf->Name,255);
     if (strlen(Lauf->Name)<37) strmaxcat(OneS,Blanks(37-strlen(Lauf->Name)),255);
     if (NOT cnt) strmaxcat(OneS," | ",255);
     else
      BEGIN
       WrLstLine(OneS); OneS[0]='\0';
      END
     cnt=NOT cnt;
     Lauf=Lauf->Next;
    END
   if (cnt)
    BEGIN
     OneS[strlen(OneS)-1]='\0';
     WrLstLine(OneS);
    END
   WrLstLine("");
END

        void ClearFunctionList(void)
BEGIN
   PFunction Lauf;

   while (FirstFunction!=Nil)
    BEGIN
     Lauf=FirstFunction->Next;
     free(FirstFunction->Name);
     free(FirstFunction->Definition);
     free(FirstFunction);
     FirstFunction=Lauf;
    END
END

/*-------------------------------------------------------------------------*/

        static void ResetSymbolDefines_ResetNode(SymbolPtr Node)
BEGIN
   if (Node->Left !=Nil) ResetSymbolDefines_ResetNode(Node->Left);
   if (Node->Right!=Nil) ResetSymbolDefines_ResetNode(Node->Right);
   Node->Defined=False; Node->Used=False;
END

        void ResetSymbolDefines(void)
BEGIN

   if (FirstSymbol!=Nil) ResetSymbolDefines_ResetNode(FirstSymbol);

   if (FirstLocSymbol!=Nil) ResetSymbolDefines_ResetNode(FirstLocSymbol);
END

        void SetFlag(Boolean *Flag, char *Name, Boolean Wert)
BEGIN
   *Flag=Wert; EnterIntSymbol(Name,(*Flag)?1:0,0,True);
END

        void AddDefSymbol(char *Name, TempResult *Value)
BEGIN
   PDefSymbol Neu;

   Neu=FirstDefSymbol;
   while (Neu!=Nil)
    BEGIN
     if (strcmp(Neu->SymName,Name)==0) return;
     Neu=Neu->Next;
    END

   Neu=(PDefSymbol) malloc(sizeof(TDefSymbol));
   Neu->Next=FirstDefSymbol;
   Neu->SymName=strdup(Name);
   Neu->Wert=(*Value);
   FirstDefSymbol=Neu;
END

        void RemoveDefSymbol(char *Name)
BEGIN
  PDefSymbol Save,Lauf;
 
  if (FirstDefSymbol==Nil) return;

  if (strcmp(FirstDefSymbol->SymName,Name)==0)
   BEGIN
    Save=FirstDefSymbol; FirstDefSymbol=FirstDefSymbol->Next;
   END
  else
   BEGIN
    Lauf=FirstDefSymbol;
    while ((Lauf->Next!=Nil) AND (strcmp(Lauf->Next->SymName,Name)!=0)) Lauf=Lauf->Next;
    if (Lauf->Next==Nil) return;
    Save=Lauf->Next; Lauf->Next=Lauf->Next->Next;
   END
  free(Save->SymName); free(Save);
END

        void CopyDefSymbols(void)
BEGIN
   PDefSymbol Lauf;

   Lauf=FirstDefSymbol;
   while (Lauf!=Nil)
    BEGIN
     switch (Lauf->Wert.Typ)
      BEGIN
       case TempInt:    EnterIntSymbol(Lauf->SymName,Lauf->Wert.Contents.Int,0,True); break;
       case TempFloat:  EnterFloatSymbol(Lauf->SymName,Lauf->Wert.Contents.Float,True); break;
       case TempString: EnterStringSymbol(Lauf->SymName,Lauf->Wert.Contents.Ascii,True); break;
       default: break;
      END
     Lauf=Lauf->Next;
    END
END

        static void PrintSymbolDepth_SearchTree(SymbolPtr Lauf, LongInt SoFar,
                                                LongInt *TreeMin, LongInt *TreeMax)
BEGIN
   if (Lauf==Nil)
    BEGIN
     if (SoFar>*TreeMax) *TreeMax=SoFar;
     if (SoFar<*TreeMin) *TreeMin=SoFar;
    END
   else
    BEGIN
     PrintSymbolDepth_SearchTree(Lauf->Right,SoFar+1,TreeMin,TreeMax);
     PrintSymbolDepth_SearchTree(Lauf->Left,SoFar+1,TreeMin,TreeMax);
    END
END

        void PrintSymbolDepth(void)
BEGIN
   LongInt TreeMin,TreeMax;

   TreeMin=MaxLongInt; TreeMax=0;
   PrintSymbolDepth_SearchTree(FirstSymbol,0,&TreeMin,&TreeMax);
   fprintf(Debug," MinTree %d\n",TreeMin);
   fprintf(Debug," MaxTree %d\n",TreeMax);
END

        LongInt GetSectionHandle(char *SName_O, Boolean AddEmpt, LongInt Parent)
BEGIN
   PCToken Lauf,Prev;
   LongInt z;
   String SName;

   strmaxcpy(SName,SName_O,255); if (NOT CaseSensitive) NLS_UpString(SName);

   Lauf=FirstSection; Prev=Nil; z=0;
   while ((Lauf!=Nil) AND ((strcmp(Lauf->Name,SName)!=0) OR (Lauf->Parent!=Parent)))
    BEGIN
     z++; Prev=Lauf; Lauf=Lauf->Next;
    END

   if (Lauf==Nil)
    if (AddEmpt)
     BEGIN
      Lauf=(PCToken) malloc(sizeof(TCToken));
      Lauf->Parent=MomSectionHandle;
      Lauf->Name=strdup(SName);
      Lauf->Next=Nil;
      InitChunk(&(Lauf->Usage));
      if (Prev==Nil) FirstSection=Lauf; else Prev->Next=Lauf;
     END
    else z=(-2);
   return z;
END

        char *GetSectionName(LongInt Handle)
BEGIN
   PCToken Lauf=FirstSection;
   static char *Dummy="";

   if (Handle==(-1)) return Dummy;
   while ((Handle>0) AND (Lauf!=Nil))
    BEGIN
     Lauf=Lauf->Next; Handle--;
    END
   return (Lauf==Nil)?Dummy:Lauf->Name;
END

        void SetMomSection(LongInt Handle)
BEGIN
   LongInt z;

   MomSectionHandle=Handle;
   if (Handle<0) MomSection=Nil;
   else
    BEGIN
     MomSection=FirstSection;
     for (z=1; z<=Handle; z++)
      if (MomSection!=Nil) MomSection=MomSection->Next;
    END
END

        void AddSectionUsage(LongInt Start,LongInt Length)
BEGIN
   if ((ActPC!=SegCode) OR (MomSection==Nil)) return;
   AddChunk(&(MomSection->Usage),Start,Length,False);
END

        static void PrintSectionList_PSection(LongInt Handle, Integer Indent)
BEGIN
   PCToken Lauf;
   LongInt Cnt;
   String h;

   ChkStack();
   if (Handle!=(-1)) 
    BEGIN
     strmaxcpy(h,Blanks(Indent<<1),255); 
     strmaxcat(h,GetSectionName(Handle),255);
     WrLstLine(h);
    END
   Lauf=FirstSection; Cnt=0;
   while (Lauf!=Nil)
    BEGIN
     if (Lauf->Parent==Handle) PrintSectionList_PSection(Cnt,Indent+1);
     Lauf=Lauf->Next; Cnt++;
    END
END

        void PrintSectionList(void)
BEGIN
   if (FirstSection==Nil) return;

   NewPage(ChapDepth,True);
   WrLstLine(ListSectionListHead1);
   WrLstLine(ListSectionListHead2);
   WrLstLine("");
   PrintSectionList_PSection(-1,0);
END

        void PrintDebSections(FILE *f)
BEGIN
   PCToken Lauf;
   LongInt Cnt,z,l,s;

   Lauf=FirstSection; Cnt=0;
   while (Lauf!=Nil)
    BEGIN
     fprintf(f,"\n"); ChkIO(10004);
     fprintf(f,"Info for Section %d %s %d\n",Cnt,GetSectionName(Cnt),Lauf->Parent); ChkIO(10004);
     for (z=0; z<Lauf->Usage.RealLen; z++)
      BEGIN
       l=Lauf->Usage.Chunks[z].Length;
       s=Lauf->Usage.Chunks[z].Start;
       fprintf(f,"%s",HexString(s,0)); ChkIO(10004);
       if (l==1) fprintf(f,"\n"); else fprintf(f,"-%s\n",HexString(s+l-1,0)); ChkIO(10004);
      END
     Lauf=Lauf->Next;
     Cnt++;
    END
END

        void ClearSectionList(void)
BEGIN
   PCToken Tmp;

   while (FirstSection!=Nil)
    BEGIN
     Tmp=FirstSection;
     free(Tmp->Name);
     ClearChunk(&(Tmp->Usage));
     FirstSection=Tmp->Next; free(Tmp);
    END
END

/*---------------------------------------------------------------------------------*/

        static void PrintCrossList_PNode(SymbolPtr Node)
BEGIN
   Integer FileZ;
   PCrossRef Lauf;
   String LinePart,LineAcc;
   String h,h2;
   TempResult t;

   if (Node->RefList==Nil) return;

   ConvertSymbolVal(&(Node->SymWert),&t);
   strcpy(h," (=");
   StrSym(&t,False,h2); strmaxcat(h,h2,255);
   strmaxcat(h,",",255);
   strmaxcat(h,GetFileName(Node->FileNum),255);
   strmaxcat(h,":",255);
   sprintf(h2,"%d",Node->LineNum); strmaxcat(h,h2,255);
   strmaxcat(h,"):",255);
   if (Node->Attribute!=(-1))
    BEGIN
     strmaxprep(h,"] ",255);
     strmaxprep(h,GetSectionName(Node->Attribute),255);
     strmaxprep(h," [",255);
    END

   strmaxprep(h,Node->SymName,255);
   strmaxprep(h,ListCrossSymName,255);
   WrLstLine(h);

   for (FileZ=0; FileZ<GetFileCount(); FileZ++)
    BEGIN
     Lauf=Node->RefList;

     while ((Lauf!=Nil) AND (Lauf->FileNum!=FileZ)) Lauf=Lauf->Next;

     if (Lauf!=Nil)
      BEGIN
       strcpy(h," ");
       strmaxcat(h,ListCrossFileName,255);
       strmaxcat(h,GetFileName(FileZ),255);
       strmaxcat(h," :",255);
       WrLstLine(h);
       strcpy(LineAcc,"   ");
       while (Lauf!=Nil)
        BEGIN
         sprintf(LinePart,"%5d",Lauf->LineNum); strmaxcat(LineAcc,LinePart,255);
         if (Lauf->OccNum!=1)
          BEGIN
           sprintf(LinePart,"(%2d)",Lauf->OccNum); strmaxcat(LineAcc,LinePart,255);
          END
         else strmaxcat(LineAcc,"    ",255);
         if (strlen(LineAcc)>=72)
          BEGIN
           WrLstLine(LineAcc); strcpy(LineAcc,"  ");
          END
         Lauf=Lauf->Next;
        END
       if (strcmp(LineAcc,"  ")!=0) WrLstLine(LineAcc);
      END
    END
   WrLstLine("");
END

        static void PrintCrossList_PrintNode(SymbolPtr Node)
BEGIN
   if (Node==Nil) return;

   PrintCrossList_PrintNode(Node->Left);

   PrintCrossList_PNode(Node);

   PrintCrossList_PrintNode(Node->Right);
END

        void PrintCrossList(void)
BEGIN

   WrLstLine("");
   WrLstLine(ListCrossListHead1);
   WrLstLine(ListCrossListHead2);
   WrLstLine("");
   PrintCrossList_PrintNode(FirstSymbol);
   WrLstLine("");
END

        static void ClearCrossList_CNode(SymbolPtr Node)
BEGIN
   PCrossRef Lauf;

   if (Node->Left!=Nil) ClearCrossList_CNode(Node->Left);

   if (Node!=Nil)
    while (Node->RefList!=Nil)
     BEGIN
      Lauf=Node->RefList->Next;
      free(Node->RefList);
      Node->RefList=Lauf;
     END

   if (Node->Right!=Nil) ClearCrossList_CNode(Node->Right);
END

        void ClearCrossList(void)
BEGIN
   ClearCrossList_CNode(FirstSymbol);
END

        LongInt GetLocHandle(void)
BEGIN
   return LocHandleCnt++;
END

        void PushLocHandle(LongInt NewLoc)
BEGIN
   PLocHandle NewLocHandle;

   NewLocHandle=(PLocHandle) malloc(sizeof(TLocHeap));
   NewLocHandle->Cont=MomLocHandle;
   NewLocHandle->Next=FirstLocHandle;
   FirstLocHandle=NewLocHandle; MomLocHandle=NewLoc;
END

        void PopLocHandle(void)
BEGIN
   PLocHandle OldLocHandle;

   OldLocHandle=FirstLocHandle;
   if (OldLocHandle==Nil) return;
   MomLocHandle=OldLocHandle->Cont; 
   FirstLocHandle=OldLocHandle->Next;
   free(OldLocHandle);
END

        void ClearLocStack()
BEGIN
   while (MomLocHandle!=(-1)) PopLocHandle();
END

	void asmpars_init(void)
BEGIN
   FirstDefSymbol=Nil;
   FirstFunction=Nil;
   BalanceTree=False;
   IntMins[Int32]--;
   IntMins[SInt32]--;
#ifdef HAS64
   IntMins[Int64]--;
#endif
END
