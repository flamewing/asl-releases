/* codepseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Pseudo-Befehle                                         */
/*                                                                           */
/* Historie: 23. 5.1996 Grundsteinlegung                                     */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1998 BookKeeping-Aufrufe bei SPeicherreservierungen       */
/*            8. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*           14. 1.2001 silenced warnings about unused parameters            */
/*           2001-11-11 added DecodeTIPseudo                                 */
/*           2002-01-13 Borland Pascal 3.1 doesn't like empty default clause */
/*                                                                           */
/*****************************************************************************/
/* $Id: codepseudo.c,v 1.5 2003/05/02 21:23:12 alfred Exp $                          */
/***************************************************************************** 
 * $Log: codepseudo.c,v $
 * Revision 1.5  2003/05/02 21:23:12  alfred
 * - strlen() updates
 *
 * Revision 1.4  2002/10/26 09:57:47  alfred
 * - allow DC with ? as argument
 *
 * Revision 1.3  2002/08/14 18:43:49  alfred
 * - warn null allocation, remove some warnings
 *
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <math.h>

#include "nls.h"
#include "bpemu.h"
#include "endian.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"

#include "codepseudo.h"


        int FindInst(void *Field, int Size, int Count)
BEGIN
   char *cptr,**ptr;

#ifdef OPT
   int l=0,r=Count-1,m,res;

   while (TRUE)
    BEGIN
     m=(l+r)>>1; cptr=((char *) Field)+(Size*m);
     ptr=(char**) cptr;
     res=strcmp(*ptr,OpPart);
     if (res==0) return m;
     else if (l==r) return -1;
     else if (res<0)
      BEGIN
       if (r-l==1) return -1; else l=m;
      END
     else r=m;
    END

#else
   int z,res;

   cptr=(char *) Field;
   for (z=0; z<Count; z++)
    BEGIN
     ptr=(char**) cptr;
     res=strcmp(*ptr,OpPart);
     if (res==0) return z;
     if (res>0) return -1;
     cptr+=Size;
    END
   return -1;
#endif
END      


        Boolean IsIndirect(char *Asc)
BEGIN
   int z,Level,l;

   if (((l=strlen(Asc))<=2) OR (Asc[0]!='(') OR (Asc[l-1]!=')')) return False;

   Level=0;
   for (z=1; z<=l-2; z++)
    BEGIN
     if (Asc[z]=='(') Level++;
     if (Asc[z]==')') Level--;
     if (Level<0) return False;
    END

   return True;
END


static enum{DSNone,DSConstant,DSSpace} DSFlag;
typedef Boolean (*TLayoutFunc)(
#ifdef __PROTOS__
                               char *Asc, Word *Cnt, Boolean Turn
#endif
                                                                 );

        static Boolean LayoutByte(char *Asc, Word *Cnt, Boolean Turn)
BEGIN
   Boolean Result;
   TempResult t;
   UNUSED(Turn);

   Result=False;

   if (strcmp(Asc,"?")==0)
    BEGIN
     if (DSFlag==DSConstant) WrError(1930);
     else
      BEGIN
       *Cnt=1; Result=True; DSFlag=DSSpace; CodeLen++;
      END
     return Result;
    END
   else
    BEGIN
     if (DSFlag==DSSpace)
      BEGIN
       WrError(1930); return Result;
      END
     else DSFlag=DSConstant;
    END

   FirstPassUnknown=False; EvalExpression(Asc,&t);
   switch (t.Typ)
    BEGIN
     case TempInt:
      if (FirstPassUnknown) t.Contents.Int&=0xff;
      if (NOT RangeCheck(t.Contents.Int,Int8)) WrError(1320);
      else
       BEGIN
        BAsmCode[CodeLen++]=t.Contents.Int; *Cnt=1;
        Result=True;
       END;
      break;
     case TempFloat: 
      WrError(1135);
      break;
     case TempString:
      TranslateString(t.Contents.Ascii);
      memcpy(BAsmCode+CodeLen,t.Contents.Ascii,strlen(t.Contents.Ascii));
      CodeLen+=(*Cnt=strlen(t.Contents.Ascii));
      Result=True;
      break;
     default:
      break;
    END

   return Result;
END


        static Boolean LayoutWord(char *Asc, Word *Cnt, Boolean Turn)
BEGIN
   Boolean OK,Result;
   Word erg;

   *Cnt=2; Result=False;

   if (strcmp(Asc,"?")==0)
    BEGIN
     if (DSFlag==DSConstant) WrError(1930);
     else
      BEGIN
       Result=True; DSFlag=DSSpace; CodeLen+=2;
      END
     return Result;
    END
   else
    BEGIN
     if (DSFlag==DSSpace)
      BEGIN
       WrError(1930); return Result;
      END
     else DSFlag=DSConstant;
    END

   if (CodeLen+2>MaxCodeLen)
    BEGIN
     WrError(1920); return Result;
    END
   erg=EvalIntExpression(Asc,Int16,&OK);
   if (OK)
    BEGIN
     if (Turn) erg=((erg>>8)&0xff)+((erg&0xff)<<8);
     BAsmCode[CodeLen]=erg&0xff; BAsmCode[CodeLen+1]=erg>>8;
     CodeLen+=2;
    END
   return OK;
END


        static Boolean LayoutDoubleWord(char *Asc, Word *Cnt, Boolean Turn)
BEGIN
   TempResult erg;
   Boolean Result=False;

   *Cnt=4;

   if (strcmp(Asc,"?")==0)
    BEGIN
     if (DSFlag==DSConstant) WrError(1930);
     else 
      BEGIN
       Result=True; DSFlag=DSSpace; CodeLen+=4;
      END
     return Result;
    END
   else
    BEGIN
     if (DSFlag==DSSpace)
      BEGIN
       WrError(1930); return Result;
      END
     else DSFlag=DSConstant;
    END

   if (CodeLen+4>MaxCodeLen)
    BEGIN
     WrError(1920); return Result;
    END  

   KillBlanks(Asc); EvalExpression(Asc,&erg);
   switch (erg.Typ)
    BEGIN
     case TempNone:
      return Result;
     case TempInt:
      if (RangeCheck(erg.Contents.Int,Int32))
       BEGIN
        BAsmCode[CodeLen  ]=((erg.Contents.Int    )&0xff);
        BAsmCode[CodeLen+1]=((erg.Contents.Int>> 8)&0xff);
        BAsmCode[CodeLen+2]=((erg.Contents.Int>>16)&0xff);
        BAsmCode[CodeLen+3]=((erg.Contents.Int>>24)&0xff);
        CodeLen+=4;
       END
      else
       BEGIN
        WrError(1320);
        return Result;
       END
      break;
     case TempFloat:
      if (FloatRangeCheck(erg.Contents.Float,Float32))
       BEGIN
        Double_2_ieee4(erg.Contents.Float,BAsmCode+CodeLen,False);
        CodeLen+=4;
       END
      else
       BEGIN
        WrError(1320);
        return Result;
       END
      break;
     case TempString:
     case TempAll:
      WrError(1135);
      return Result;
    END

   if (Turn) DSwap(BAsmCode+CodeLen-4,4);
   return True;
END


        static Boolean LayoutQuadWord(char *Asc, Word *Cnt, Boolean Turn)
BEGIN
   Boolean Result;
   TempResult erg;
#ifndef HAS64
   int z;
#endif

   Result=False; *Cnt=8;

   if (strcmp(Asc,"?")==0)
    BEGIN
     if (DSFlag==DSConstant) WrError(1930);
     else
      BEGIN
       Result=True; DSFlag=DSSpace; CodeLen+=8;
      END
     return Result;
    END
   else
    BEGIN
     if (DSFlag==DSSpace)
      BEGIN
       WrError(1930); return Result;
      END
     else DSFlag=DSConstant;
    END

   if (CodeLen+8>MaxCodeLen)
    BEGIN
     WrError(1920); return Result;
    END

   KillBlanks(Asc); EvalExpression(Asc,&erg);
   switch(erg.Typ)
    BEGIN
     case TempNone:
      return Result;
     case TempInt:
      memcpy(BAsmCode+CodeLen,&(erg.Contents.Int),sizeof(LargeInt));
#ifdef HAS64
      if (BigEndian) QSwap(BAsmCode+CodeLen,8);
#else
      if (BigEndian) DSwap(BAsmCode+CodeLen,4);
      for (z=4; z<8; BAsmCode[CodeLen+(z++)]=(BAsmCode[CodeLen+3]>=0x80)?0xff:0x00);
#endif
      CodeLen+=8;
      break;
     case TempFloat:
      Double_2_ieee8(erg.Contents.Float,BAsmCode+CodeLen,False);
      CodeLen+=8;
      break;
     case TempString:
     case TempAll:
      WrError(1135);
      return Result;
    END
 
   if (Turn) QSwap(BAsmCode+CodeLen-8,8);
   return True;
END


        static Boolean LayoutTenBytes(char *Asc, Word *Cnt, Boolean Turn)
BEGIN
   Boolean OK,Result;
   Double erg;
   int z;
   Byte Exg;

   Result=False; *Cnt=10;

   if (strcmp(Asc,"?")==0)
    BEGIN
     if (DSFlag==DSConstant) WrError(1930);
     else
      BEGIN
       Result=True; DSFlag=DSSpace; CodeLen+=10;
      END
     return Result;
    END
   else
    BEGIN
     if (DSFlag==DSSpace)
      BEGIN
       WrError(1930); return Result;
      END
     else DSFlag=DSConstant;
    END

   if (CodeLen+10>MaxCodeLen)
    BEGIN
     WrError(1920); return Result;
    END
   erg=EvalFloatExpression(Asc,Float64,&OK);
   if (OK)
    BEGIN
     Double_2_ieee10(erg,BAsmCode+CodeLen,False);
     CodeLen+=10;
     if (Turn)
      for (z=0; z<5; z++)
       BEGIN
        Exg=BAsmCode[CodeLen-10+z];
        BAsmCode[CodeLen-10+z]=BAsmCode[CodeLen-1-z];
        BAsmCode[CodeLen-1-z]=Exg;
       END
    END
   return OK;
END


        static Boolean DecodeIntelPseudo_ValidSymChar(char ch)
BEGIN
   return (((ch>='A') AND (ch<='Z')) OR ((ch>='0') AND (ch<='9')) OR (ch=='_') OR (ch=='.'));
END

        static Boolean DecodeIntelPseudo_LayoutMult(char *Asc_O, Word *Cnt,
                                                    TLayoutFunc LayoutFunc,
                                                    Boolean Turn)
BEGIN
   int z,Depth,Fnd,ALen;
   String Asc,Part;
   Word SumCnt,ECnt,SInd;
   LongInt Rep;
   Boolean OK,Hyp;

   strmaxcpy(Asc,Asc_O,255);

   /* nach DUP suchen */

   Depth=0; Fnd=0; ALen=strlen(Asc);
   for (z=0; z<ALen-2; z++)
    BEGIN
     if (Asc[z]=='(') Depth++;
     else if (Asc[z]==')') Depth--;
     else if (Depth==0)
      if ( ((z==0) OR (NOT DecodeIntelPseudo_ValidSymChar(Asc[z-1])))
      AND  (NOT DecodeIntelPseudo_ValidSymChar(Asc[z+3]))
      AND (strncasecmp(Asc+z,"DUP",3)==0)) Fnd=z;
    END

   /* DUP gefunden: */

   if (Fnd!=0)
    BEGIN
     /* Anzahl ausrechnen */

     FirstPassUnknown=False;
     Asc[Fnd]='\0'; Rep=EvalIntExpression(Asc,Int32,&OK); Asc[Fnd]='D';
     if (FirstPassUnknown)
      BEGIN
       WrError(1820); return False;
      END
     if (NOT OK) return False;

     /* Nullargument vergessen, bei negativem warnen */

     if (Rep<0) WrError(270);
     if (Rep<=0) return True;

     /* Einzelteile bilden & evaluieren */

     strcpy(Asc,Asc+Fnd+3); KillPrefBlanks(Asc); SumCnt=0; 
     if ((strlen(Asc)>=2) AND (*Asc=='(') AND (Asc[strlen(Asc)-1]==')'))
      BEGIN
       strcpy(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
      END
     do
      BEGIN
       Fnd=0; z=0; Hyp=False; Depth=0;
       do
        BEGIN
         if (Asc[z]=='\'') Hyp=NOT Hyp;
         else if (NOT Hyp)
          BEGIN
           if (Asc[z]=='(') Depth++;
           else if (Asc[z]==')') Depth--;
           else if ((Depth==0) AND (Asc[z]==',')) Fnd=z;
          END
         z++;
        END
       while ((z<(int)strlen(Asc)) AND (Fnd==0));
       if (Fnd==0)
        BEGIN
         strmaxcpy(Part,Asc,255); *Asc='\0';
        END
       else
        BEGIN
         Asc[Fnd]='\0'; strmaxcpy(Part,Asc,255);
         strcpy(Asc,Asc+Fnd+1);
        END
       if (NOT DecodeIntelPseudo_LayoutMult(Part,&ECnt,LayoutFunc,Turn))
        return False; 
       SumCnt+=ECnt;
      END
     while (*Asc!='\0');

     /* Ergebnis vervielfachen */

     if (DSFlag==DSConstant)
      BEGIN
       SInd=CodeLen-SumCnt;
       if (CodeLen+SumCnt*(Rep-1)>MaxCodeLen)
        BEGIN
         WrError(1920); return False;
        END
       for (z=1; z<=Rep-1; z++)
        BEGIN
         if (CodeLen+SumCnt>MaxCodeLen) return False;
         memcpy(BAsmCode+CodeLen,BAsmCode+SInd,SumCnt);
         CodeLen+=SumCnt;
        END
      END
     else CodeLen+=SumCnt*(Rep-1);
     *Cnt=SumCnt*Rep; return True;
    END

   /* kein DUP: einfacher Ausdruck */

   else return LayoutFunc(Asc,Cnt,Turn);
END

        Boolean DecodeIntelPseudo(Boolean Turn)
BEGIN
   Word Dummy;
   int z;
   TLayoutFunc LayoutFunc=Nil;
   Boolean OK;
   LongInt HVal;
   char Ident;

   if ((strlen(OpPart)!=2) OR (*OpPart!='D')) return False;
   Ident=OpPart[1];

   if ((Ident=='B') OR (Ident=='W') OR (Ident=='D') OR (Ident=='Q') OR (Ident=='T'))
    BEGIN
     DSFlag=DSNone;
     switch (Ident)
      BEGIN
       case 'B':
        LayoutFunc=LayoutByte;
        if (*LabPart!='\0') SetSymbolSize(LabPart,0);
        break;
       case 'W':
        LayoutFunc=LayoutWord;
        if (*LabPart!='\0') SetSymbolSize(LabPart,1);
        break;
       case 'D':
        LayoutFunc=LayoutDoubleWord;
        if (*LabPart!='\0') SetSymbolSize(LabPart,2);
        break;
       case 'Q':
        LayoutFunc=LayoutQuadWord;
        if (*LabPart!='\0') SetSymbolSize(LabPart,3);
        break;
       case 'T':
        LayoutFunc=LayoutTenBytes;
        if (*LabPart!='\0') SetSymbolSize(LabPart,4);
        break;
      END
     z=1;
     do
      BEGIN
       OK=DecodeIntelPseudo_LayoutMult(ArgStr[z],&Dummy,LayoutFunc,Turn);
       if (NOT OK) CodeLen=0;
       z++;
      END
     while ((OK) AND (z<=ArgCnt));
     DontPrint = (DSFlag == DSSpace);
     if (DontPrint)
     {
       BookKeeping();
       if (!CodeLen) WrError(290);
     }
     if (OK) ActListGran=1;
     return True;
    END

   if (Ident == 'S')
   {
     if (ArgCnt != 1) WrError(1110);
     else
     {
       FirstPassUnknown = False;
       HVal = EvalIntExpression(ArgStr[1], Int32, &OK);
       if (FirstPassUnknown) WrError(1820);
       else if (OK)
       {
         DontPrint = True; CodeLen = HVal;
         if (!HVal) WrError(290);
         BookKeeping();
       }
     }
     return True;
   }

   return False;
END

/*--------------------------------------------------------------------------*/

static Boolean M16Turn=False;

        static Boolean CutRep(char *Asc, LongInt *Erg)
BEGIN
   char *p;
   Boolean OK;

   if (*Asc!='[')
    BEGIN
     *Erg=1; return True;
    END
   else
    BEGIN
     strcpy(Asc,Asc+1); p=QuotPos(Asc,']');
     if (p==Nil) 
      BEGIN
       WrError(1300); return False;
      END
     else
      BEGIN
       *p='\0';
       *Erg=EvalIntExpression(Asc,Int32,&OK);
       strcpy(Asc,p+1); return OK;
      END
    END
END     
        
        static void DecodeBYT(Word Index)
BEGIN
   int z;
   Boolean OK;
   TempResult t;
   LongInt Rep,z2;
   UNUSED(Index);

   if (ArgCnt==0) WrError(1110);
   else
    BEGIN
     z=1; OK=True;
     do
      BEGIN
       KillBlanks(ArgStr[z]);
       OK=CutRep(ArgStr[z],&Rep);
       if (OK)
        BEGIN
         EvalExpression(ArgStr[z],&t);
         switch (t.Typ)
          BEGIN
           case TempInt:
            if (NOT RangeCheck(t.Contents.Int,Int8))
             BEGIN
              WrError(1320); OK=False;
             END
            else if (CodeLen+Rep>MaxCodeLen)
             BEGIN
              WrError(1920); OK=False;
             END
            else
             BEGIN
              memset(BAsmCode+CodeLen,t.Contents.Int,Rep);
              CodeLen+=Rep;
             END
            break;
           case TempFloat:
            WrError(1135); OK=False;
            break;
           case TempString:
            TranslateString(t.Contents.Ascii);
            if (CodeLen+Rep*strlen(t.Contents.Ascii)>MaxCodeLen)
             BEGIN
              WrError(1920); OK=False;
             END
            else for (z2=0; z2<Rep; z2++)
             BEGIN
              memcpy(BAsmCode+CodeLen,t.Contents.Ascii,strlen(t.Contents.Ascii));
              CodeLen+=strlen(t.Contents.Ascii);
             END
            break;
           default:
            OK=False; 
            break;
          END
        END  
       z++;
      END
     while ((z<=ArgCnt) AND (OK));
     if (NOT OK) CodeLen=0;
    END
END

        static void DecodeADR(Word Index)
BEGIN
   int z;
   Word HVal16;
   Boolean OK;
   LongInt Rep,z2;
   UNUSED(Index);

   if (ArgCnt==0) WrError(1110);
   else
    BEGIN
     z=1; OK=True;
     do
      BEGIN
       OK=CutRep(ArgStr[z],&Rep);
       if (OK)
        BEGIN
         if (CodeLen+(Rep<<1)>MaxCodeLen)
          BEGIN
           WrError(1920); OK=False;
          END
         else
          BEGIN
           HVal16=EvalIntExpression(ArgStr[z],Int16,&OK);
           if (OK)
            for (z2=0; z2<Rep; z2++)
             BEGIN
              if (M16Turn)
               BEGIN
                BAsmCode[CodeLen++]=Hi(HVal16);
                BAsmCode[CodeLen++]=Lo(HVal16);
               END
              else
               BEGIN
                BAsmCode[CodeLen++]=Lo(HVal16);
                BAsmCode[CodeLen++]=Hi(HVal16);
               END
            END
          END
        z++;
       END
      END
     while ((z<=ArgCnt) AND (OK));
     if (NOT OK) CodeLen=0;
    END
END

        static void DecodeFCC(Word Index)
BEGIN
   String SVal;
   Boolean OK;
   int z;
   LongInt Rep,z2;
   UNUSED(Index);

   if (ArgCnt==0) WrError(1110);
   else
    BEGIN
     z=1; OK=True;
     do
      BEGIN
       OK=CutRep(ArgStr[z],&Rep);
       if (OK)
        BEGIN
         EvalStringExpression(ArgStr[z],&OK,SVal);
         if (OK)
          BEGIN
           if (CodeLen+Rep*strlen(SVal)>=MaxCodeLen)
            BEGIN
             WrError(1920); OK=False;
            END
           else
            BEGIN
             TranslateString(SVal);
             for (z2=0; z2<Rep; z2++)
              BEGIN
               memcpy(BAsmCode+CodeLen,SVal,strlen(SVal));
               CodeLen+=strlen(SVal);
              END
            END
          END
        END   
       z++;
      END
     while ((z<=ArgCnt) AND (OK));
     if (NOT OK) CodeLen=0;
    END
END

        static void DecodeDFS(Word Index)
BEGIN
   Word HVal16;
   Boolean OK;
   UNUSED(Index);

   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     FirstPassUnknown=False;
     HVal16=EvalIntExpression(ArgStr[1],Int16,&OK);
     if (FirstPassUnknown) WrError(1820);
     else if (OK)
      BEGIN
       DontPrint=True; CodeLen=HVal16;
       if (!HVal16) WrError(290);
       BookKeeping();
      END
    END
END

        Boolean DecodeMotoPseudo(Boolean Turn)
BEGIN
   static PInstTable InstTable=Nil;

   if (InstTable==Nil)
    BEGIN
     InstTable=CreateInstTable(17);
     AddInstTable(InstTable,"BYT",0,DecodeBYT);
     AddInstTable(InstTable,"FCB",0,DecodeBYT);
     AddInstTable(InstTable,"ADR",0,DecodeADR);
     AddInstTable(InstTable,"FDB",0,DecodeADR);
     AddInstTable(InstTable,"FCC",0,DecodeFCC);
     AddInstTable(InstTable,"DFS",0,DecodeDFS);
     AddInstTable(InstTable,"RMB",0,DecodeDFS);
    END

   M16Turn=Turn;
   return LookupInstTable(InstTable,OpPart);
END

        static void DigIns(char Ch, Byte Pos, Word *w)
BEGIN
   Byte wpos=Pos>>2,bpos=(Pos&3)*4;
   Word dig=Ch-'0';

   w[wpos]|=(dig<<bpos);
END

        void ConvertDec(Double F, Word *w)
BEGIN
   char s[30],Man[30],Exp[30];
   char *h;
   LongInt z;
   Byte epos;

   sprintf(s,"%0.16e",F); h=strchr(s,'e');
   if (h==Nil)
    BEGIN
     strcpy(Man,s); strcpy(Exp,"+0000");
    END
   else
    BEGIN
     *h='\0';
     strcpy(Man,s); strcpy(Exp,h+1);
    END
   memset(w,0,12);
   if (*Man=='-')
    BEGIN
     w[5]|=0x8000; strcpy(Man,Man+1);
    END
   else if (*Man=='+') strcpy(Man,Man+1);
   if (*Exp=='-')
    BEGIN
     w[5]|=0x4000; strcpy(Exp,Exp+1);
    END
   else if (*Exp=='+') strcpy(Exp,Exp+1);
   DigIns(*Man,16,w); strcpy(Man,Man+2);
   if (strlen(Man)>16) Man[16]='\0';
   for (z=0; z<(int)strlen(Man); z++) DigIns(Man[z],15-z,w);
   if (strlen(Exp)>4) strcpy(Exp,Exp+strlen(Exp)-4);
   for (z=strlen(Exp)-1; z>=0; z--)
    BEGIN
     epos=strlen(Exp)-1-z;
     if (epos==3) DigIns(Exp[z],19,w); else DigIns(Exp[z],epos+20,w);
    END
END


        static void EnterByte(Byte b)
BEGIN
   if (((CodeLen&1)==1) AND (NOT BigEndian) AND (ListGran()!=1))
    BEGIN
     BAsmCode[CodeLen]=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]=b;
    END
   else
    BEGIN
     BAsmCode[CodeLen]=b;
    END
   CodeLen++;
END

        void AddMoto16PseudoONOFF(void)
BEGIN
   AddONOFF("PADDING",&DoPadding,DoPaddingName,False);
END

        Boolean DecodeMoto16Pseudo(ShortInt OpSize, Boolean Turn)
BEGIN
   Byte z;
   Word TurnField[8];
   char *zp;
   LongInt z2;
   LongInt WSize,Rep = 0;
   LongInt NewPC,HVal,WLen;
#ifdef HAS64
   QuadInt QVal;
#endif
   Integer HVal16;
   Double DVal;
   TempResult t;
   Boolean OK, ValOK;
   ShortInt SpaceFlag;

   UNUSED(Turn);

   if (OpSize < 0)
     OpSize = 1;

   switch (OpSize)
   {
     case 0: WSize = 1; break;
     case 1: WSize = 2; break;
     case 2:
     case 4: WSize = 4; break;
     case 3:
     case 5: WSize = 8; break;
     case 6:
     case 7: WSize = 12; break;
     default: WSize = 0;
   }

   if (*OpPart != 'D')
     return False;

   if (Memo("DC"))
   {
     if (ArgCnt == 0) WrError(1110);
     else
     {
       OK = True; z = 1; WLen = 0; SpaceFlag = -1;

       while ((z <= ArgCnt) && (OK))
       {
         FirstPassUnknown = False;
         OK = CutRep(ArgStr[z], &Rep);
         if (OK)
         {
           if (FirstPassUnknown) WrError(1820);
           else if (!strcmp(ArgStr[z], "?"))
           {
             if (SpaceFlag == 0)
             {
               WrError(1930);
               OK = FALSE;
             }
             else
             {
               SpaceFlag = 1;
               CodeLen += (Rep * WSize);
             }
           }
           else if (SpaceFlag == 1)
           {
             WrError(1930);
             OK = FALSE;
           }
           else
           {
             SpaceFlag = 0;
             switch (OpSize)
             {
               case 0:
                FirstPassUnknown = False;
                EvalExpression(ArgStr[z], &t);
                if ((FirstPassUnknown) AND (t.Typ == TempInt)) t.Contents.Int &= 0xff;
                switch (t.Typ)
                 BEGIN
                  case TempInt:
                   if (NOT RangeCheck(t.Contents.Int,Int8))
                    BEGIN
                     WrError(1320); OK=False;
                    END
                   else if (CodeLen+Rep>MaxCodeLen)
                    BEGIN
                     WrError(1920); OK=False;
                    END
                   else for (z2=0; z2<Rep; z2++) EnterByte(t.Contents.Int);
                   break;
                  case TempFloat:
                   WrError(1135); OK=False;
                   break;
                  case TempString:
                   if (CodeLen+Rep*strlen(t.Contents.Ascii)>MaxCodeLen)
                    BEGIN
                     WrError(1920); OK=False;
                    END
                   else 
                    for (z2=0; z2<Rep; z2++)
                     for (zp=t.Contents.Ascii; *zp!='\0'; EnterByte(CharTransTable[((usint) *(zp++))&0xff]));
                   break;
                  default: OK=False;
                 END
                break;
               case 1:
                HVal16=EvalIntExpression(ArgStr[z],Int16,&OK);
                if (OK)
                 BEGIN
                  if (CodeLen+(Rep<<1)>MaxCodeLen)                
                   BEGIN
                    WrError(1920); OK=False;
                   END
                  else
                   BEGIN
                    if (ListGran()==1)
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       BAsmCode[(WLen<<1)  ]=Hi(HVal16);
                       BAsmCode[(WLen<<1)+1]=Lo(HVal16);
                       WLen++;  
                      END
                    else
                     for (z2=0; z2<Rep; z2++) WAsmCode[WLen++]=HVal16; 
                    CodeLen+=Rep<<1;
                   END
                 END
                break;
               case 2:
                HVal=EvalIntExpression(ArgStr[z],Int32,&OK);
                if (OK)
                 BEGIN
                  if (CodeLen+(Rep<<2)>MaxCodeLen)
                   BEGIN
                    WrError(1920); OK=False;
                   END
                  else
                   BEGIN
                    if (ListGran()==1)
                     for (z2=0; z2<Rep; z2++)
                      BEGIN   
                       BAsmCode[(WLen<<1)  ]=(HVal >> 24) & 0xff;
                       BAsmCode[(WLen<<1)+1]=(HVal >> 16) & 0xff;
                       BAsmCode[(WLen<<1)+2]=(HVal >>  8) & 0xff;
                       BAsmCode[(WLen<<1)+3]=(HVal      ) & 0xff;
                       WLen+=2;
                      END
                    else
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       WAsmCode[WLen++]=HVal >> 16;
                       WAsmCode[WLen++]=HVal & 0xffff;
                      END
                    CodeLen+=Rep<<2;
                   END
                 END
                break;
#ifdef HAS64
               case 3:
                QVal=EvalIntExpression(ArgStr[z],Int64,&OK);
                if (OK)
                 BEGIN
                  if (CodeLen+(Rep<<3)>MaxCodeLen)
                   BEGIN
                    WrError(1920); OK=False;
                   END
                  else
                   BEGIN
                    if (ListGran()==1)
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       BAsmCode[(WLen<<1)  ]=(QVal >> 56) & 0xff;
                       BAsmCode[(WLen<<1)+1]=(QVal >> 48) & 0xff;
                       BAsmCode[(WLen<<1)+2]=(QVal >> 40) & 0xff;
                       BAsmCode[(WLen<<1)+3]=(QVal >> 32) & 0xff;
                       BAsmCode[(WLen<<1)+4]=(QVal >> 24) & 0xff;
                       BAsmCode[(WLen<<1)+5]=(QVal >> 16) & 0xff;
                       BAsmCode[(WLen<<1)+6]=(QVal >>  8) & 0xff;
                       BAsmCode[(WLen<<1)+7]=(QVal      ) & 0xff;
                       WLen+=4;
                      END
                    else
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       WAsmCode[WLen++]=(QVal >> 48) & 0xffff;
                       WAsmCode[WLen++]=(QVal >> 32) & 0xffff;
                       WAsmCode[WLen++]=(QVal >> 16) & 0xffff;
                       WAsmCode[WLen++]=QVal & 0xffff;
                      END
                    CodeLen+=Rep<<3;
                   END
                 END
                break;
#endif
               case 4:
                DVal=EvalFloatExpression(ArgStr[z],Float32,&OK);
                if (OK)
                 BEGIN
                  if (CodeLen+(Rep<<2)>MaxCodeLen)
                   BEGIN
                    WrError(1920); OK=False;
                   END
                  else
                   BEGIN
                    Double_2_ieee4(DVal,(Byte *) TurnField,BigEndian); 
                    if (BigEndian) DWSwap((void*) TurnField,4);
                    if (ListGran()==1)
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       BAsmCode[(WLen<<1)  ]=Hi(TurnField[1]);
                       BAsmCode[(WLen<<1)+1]=Lo(TurnField[1]);
                       BAsmCode[(WLen<<1)+2]=Hi(TurnField[0]);
                       BAsmCode[(WLen<<1)+3]=Lo(TurnField[0]);
                       WLen+=2;
                      END
                     else
                      for (z2=0; z2<Rep; z2++)
                      BEGIN
                       WAsmCode[WLen++]=TurnField[1];
                       WAsmCode[WLen++]=TurnField[0];
                      END
                    CodeLen+=Rep<<2;
                   END
                 END
                break;
               case 5:
                DVal=EvalFloatExpression(ArgStr[z],Float64,&OK);
                if (OK)
                 BEGIN
                  if (CodeLen+(Rep<<3)>MaxCodeLen)
                   BEGIN
                    WrError(1920); OK=False;
                   END
                  else
                   BEGIN
                    Double_2_ieee8(DVal,(Byte *) TurnField,BigEndian);
                    if (BigEndian) QWSwap((void *) TurnField,8);
                    if (ListGran()==1)
                     for (z2=0; z2<Rep; z2++)   
                      BEGIN
                       BAsmCode[(WLen<<1)  ]=Hi(TurnField[3]);
                       BAsmCode[(WLen<<1)+1]=Lo(TurnField[3]);
                       BAsmCode[(WLen<<1)+2]=Hi(TurnField[2]);
                       BAsmCode[(WLen<<1)+3]=Lo(TurnField[2]);
                       BAsmCode[(WLen<<1)+4]=Hi(TurnField[1]);
                       BAsmCode[(WLen<<1)+5]=Lo(TurnField[1]);
                       BAsmCode[(WLen<<1)+6]=Hi(TurnField[0]);
                       BAsmCode[(WLen<<1)+7]=Lo(TurnField[0]);
                       WLen+=4;
                      END
                    else
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       WAsmCode[WLen++]=TurnField[3];
                       WAsmCode[WLen++]=TurnField[2];
                       WAsmCode[WLen++]=TurnField[1];
                       WAsmCode[WLen++]=TurnField[0];
                      END
                    CodeLen+=Rep<<3;
                   END
                 END
                break;
               case 6:
                DVal=EvalFloatExpression(ArgStr[z],Float64,&OK);
                if (OK)
                 BEGIN
                  if (CodeLen+(Rep*12)>MaxCodeLen)
                   BEGIN
                    WrError(1920); OK=False;
                   END
                  else
                   BEGIN
                    Double_2_ieee10(DVal,(Byte *) TurnField,False);
                    if (BigEndian) WSwap((void *) TurnField,10);
                    if (ListGran()==1)
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       BAsmCode[(WLen<<1)   ]=Hi(TurnField[4]);
                       BAsmCode[(WLen<<1)+ 1]=Lo(TurnField[4]);
                       BAsmCode[(WLen<<1)+ 2]=0;
                       BAsmCode[(WLen<<1)+ 3]=0;
                       BAsmCode[(WLen<<1)+ 4]=Hi(TurnField[3]);
                       BAsmCode[(WLen<<1)+ 5]=Lo(TurnField[3]);
                       BAsmCode[(WLen<<1)+ 6]=Hi(TurnField[2]);
                       BAsmCode[(WLen<<1)+ 7]=Lo(TurnField[2]);
                       BAsmCode[(WLen<<1)+ 8]=Hi(TurnField[1]);
                       BAsmCode[(WLen<<1)+ 9]=Lo(TurnField[1]);
                       BAsmCode[(WLen<<1)+10]=Hi(TurnField[0]);
                       BAsmCode[(WLen<<1)+11]=Lo(TurnField[0]);
                       WLen+=6;
                      END
                    else
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       WAsmCode[WLen++]=TurnField[4];
                       WAsmCode[WLen++]=0;
                       WAsmCode[WLen++]=TurnField[3];
                       WAsmCode[WLen++]=TurnField[2];
                       WAsmCode[WLen++]=TurnField[1];
                       WAsmCode[WLen++]=TurnField[0];
                      END
                    CodeLen+=Rep*12;
                   END
                 END
                break;
               case 7:
                DVal=EvalFloatExpression(ArgStr[z],Float64,&OK);
                if (OK)
                 BEGIN
                  if (CodeLen+(Rep*12)>MaxCodeLen)
                   BEGIN
                    WrError(1920); OK=False;
                   END
                  else
                   BEGIN
                    ConvertDec(DVal,TurnField);
                    if (ListGran()==1)
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       BAsmCode[(WLen<<1)   ]=Hi(TurnField[5]);
                       BAsmCode[(WLen<<1)+ 1]=Lo(TurnField[5]);
                       BAsmCode[(WLen<<1)+ 2]=Hi(TurnField[4]);
                       BAsmCode[(WLen<<1)+ 3]=Lo(TurnField[4]);
                       BAsmCode[(WLen<<1)+ 4]=Hi(TurnField[3]);
                       BAsmCode[(WLen<<1)+ 5]=Lo(TurnField[3]);
                       BAsmCode[(WLen<<1)+ 6]=Hi(TurnField[2]);
                       BAsmCode[(WLen<<1)+ 7]=Lo(TurnField[2]);
                       BAsmCode[(WLen<<1)+ 8]=Hi(TurnField[1]);
                       BAsmCode[(WLen<<1)+ 9]=Lo(TurnField[1]);
                       BAsmCode[(WLen<<1)+10]=Hi(TurnField[0]);
                       BAsmCode[(WLen<<1)+11]=Lo(TurnField[0]);
                       WLen+=6;
                      END
                    else
                     for (z2=0; z2<Rep; z2++)
                      BEGIN
                       WAsmCode[WLen++]=TurnField[5];
                       WAsmCode[WLen++]=TurnField[4];
                       WAsmCode[WLen++]=TurnField[3];
                       WAsmCode[WLen++]=TurnField[2];
                       WAsmCode[WLen++]=TurnField[1];
                       WAsmCode[WLen++]=TurnField[0];
                      END
                    CodeLen+=Rep*12;
                   END
                 END
                break;
             }
           }
         }
         z++;
       }

       /* purge results if an error occured */

       if (NOT OK) CodeLen = 0;

       /* just space reservation ? */

       else if (SpaceFlag == 1)
       {
         DontPrint = True;
         if ((DoPadding) && (CodeLen & 1))
           CodeLen++;
       }

       /* otherwise, we actually disposed values */

       else
       {
         if ((DoPadding) && ((CodeLen&1)==1))
           EnterByte(0);
       }
     }
     return True;
   }

   if (Memo("DS"))
   {
     if (ArgCnt != 1) WrError(1110);
     else
     {
       FirstPassUnknown = False;
       HVal=EvalIntExpression(ArgStr[1], Int32, &ValOK);
       if (FirstPassUnknown) WrError(1820);
       if ((ValOK) AND (NOT FirstPassUnknown))
       {
         DontPrint = True;
         switch (OpSize)
         {
           case 0: WSize = 1; if (((HVal & 1) == 1) AND (DoPadding)) HVal++; break;
           case 1: WSize = 2; break;
           case 2:
           case 4: WSize = 4; break;
           case 3:
           case 5: WSize = 8; break;
           case 6:
           case 7: WSize = 12; break;
           default: WSize = 0;
         }

         /* value of 0 means aligning the PC.  Doesn't make sense
            for bytes, since all adresses are integral numbers :-) */

         if (HVal == 0)
         {
           NewPC = ProgCounter() + WSize - 1;
           NewPC = NewPC-(NewPC % WSize);
           CodeLen = NewPC - ProgCounter();
           if (CodeLen == 0)
           {
             DontPrint = False;
             if (WSize == 1) WrError(290);
           }
         }
         else
           CodeLen = HVal * WSize;
         if (DontPrint) BookKeeping();
       }
     }
     return True;
   }

   return False;
END

static void define_untyped_label(void)
{
  if (LabPart[0])
  {
    PushLocHandle(-1);
    EnterIntSymbol(LabPart, EProgCounter(), SegNone, False);
    PopLocHandle();
  }
}

static void pseudo_qxx(Integer num)
{
  int z;
  Boolean ok;
  double res;

  if (!ArgCnt)
  {
    WrError(1110);
    return;
  }
  for(z = 1; z <= ArgCnt; z++)
  {
    res = ldexp(EvalFloatExpression(ArgStr[z], Float64, &ok), num);
    if (!ok)
    {
      CodeLen = 0;
      return;
    }
    if ((res > 32767.49) || (res < -32768.49))
    {
      CodeLen = 0;
      WrError(1320);
      return;
    }
    WAsmCode[CodeLen++] = res;
  }
}

static void pseudo_lqxx(Integer num)
{
  int z;
  Boolean ok;
  double res;
  LongInt resli;

  if (!ArgCnt)
  {
    WrError(1110);
    return;
  }
  for(z = 1; z <= ArgCnt; z++)
  {
    res = ldexp(EvalFloatExpression(ArgStr[z], Float64, &ok), num);
    if (!ok) 
    {
      CodeLen = 0;
      return;
    }
    if ((res > 2147483647.49) || (res < -2147483647.49))
    {
      CodeLen = 0;
      WrError(1320);
      return;
    }
    resli = res;
    WAsmCode[CodeLen++] = resli & 0xffff;
    WAsmCode[CodeLen++] = resli >> 16;
  }
}

typedef void (*tcallback)(
#ifdef __PROTOS__
Boolean *, int *, LongInt
#endif
);

static void pseudo_store(tcallback callback)
{
  Boolean ok = True;
  int adr = 0;
  int z;
  TempResult t;
  unsigned char *cp;

  if (!ArgCnt)
  {
    WrError(1110);
    return;
  }
  define_untyped_label();
  for(z = 1; z <= ArgCnt; z++)
  {
    if (!ok)
      return;
    EvalExpression(ArgStr[z], &t);
    switch(t.Typ)
    {
      case TempInt:
        callback(&ok, &adr, t.Contents.Int);
        break;
      case TempFloat:
        WrError(1135);
        return;
      case TempString:
        cp = (unsigned char *)t.Contents.Ascii;
        while (*cp) 
          callback(&ok, &adr, CharTransTable[((usint)*cp++)&0xff]);
        break;
      default:
        WrError(1135);
        return;
    }
  }
}

static void wr_code_byte(Boolean *ok, int *adr, LongInt val)
{
  if ((val < -128) || (val > 0xff))
  {
    WrError(1320);
    *ok = False;
    return;
  }
  WAsmCode[(*adr)++] = val & 0xff;
  CodeLen = *adr;
}

static void wr_code_word(Boolean *ok, int *adr, LongInt val)
{
  if ((val < -32768) || (val > 0xffff))
  {
    WrError(1320);
    *ok = False;
    return;
  }
  WAsmCode[(*adr)++] = val;
  CodeLen = *adr;
}

static void wr_code_long(Boolean *ok, int *adr, LongInt val)
{
  UNUSED(ok);

  WAsmCode[(*adr)++] = val & 0xffff;
  WAsmCode[(*adr)++] = val >> 16;
  CodeLen = *adr;
}

static void wr_code_byte_hilo(Boolean *ok, int *adr, LongInt val)
{
  if ((val < -128) || (val > 0xff))
  {
    WrError(1320);
    *ok = False;
    return;
  }
  if ((*adr) & 1) 
    WAsmCode[((*adr)++) / 2] |= val & 0xff;
  else 
    WAsmCode[((*adr)++) / 2] = val << 8;
  CodeLen = ((*adr) + 1) / 2;
}

static void wr_code_byte_lohi(Boolean *ok, int *adr, LongInt val)
{
  if ((val < -128) || (val > 0xff))
  {
    WrError(1320);
    *ok = False;
    return;
  }
  if ((*adr) & 1) 
    WAsmCode[((*adr)++) / 2] |= val << 8;
  else 
    WAsmCode[((*adr)++) / 2] = val & 0xff;
  CodeLen = ((*adr) + 1) / 2;
}

Boolean DecodeTIPseudo(void)
{
  Boolean ok;
  int z, z2, exp;
  double dbl, mant;
  float flt;
  long lmant;
  Word w, size;
  TempResult t;
  unsigned char *cp;

  if (Memo("RES") || Memo("BSS"))
  {
    if (ArgCnt != 1)
    {
      WrError(1110);
      return True;
    }
    if (Memo("BSS"))
      define_untyped_label();
    FirstPassUnknown = False;
    size = EvalIntExpression(ArgStr[1], Int16, &ok);
    if (FirstPassUnknown)
    {
      WrError(1820);
      return True;
    }
    if (!ok) 
      return True;
    DontPrint = True;
    if (!size) WrError(290);
    CodeLen = size;
    BookKeeping();
    return True;
  }

  if (Memo("DATA"))
  {
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    ok = True;
    for(z = 1; (z <= ArgCnt) && ok; z++)
    {
      EvalExpression(ArgStr[z], &t);
      switch(t.Typ)
      {
        case TempInt:  
          if (!ChkRange(t.Contents.Int, -32768, 0xffff))
            ok = False;
          else
            WAsmCode[CodeLen++] = t.Contents.Int;
          break;
        case TempFloat:
          WrError(1135); 
          ok = False;
          break;
        case TempString:
          z2 = 0;
          cp = (unsigned char *)t.Contents.Ascii;
          while (*cp)
          {
            if (z2 & 1)
              WAsmCode[CodeLen++] |= (CharTransTable[((usint)*cp++)&0xff] << 8);
            else
              WAsmCode[CodeLen] = CharTransTable[((usint)*cp++)&0xff];
            z2++;
          }
          if (z2 & 1)
            CodeLen++;
          break;
        default:
          break;
      }
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }

  if (Memo("STRING"))
  {
    pseudo_store(wr_code_byte_hilo); 
    return True;
  }
  if (Memo("RSTRING"))
  {
    pseudo_store(wr_code_byte_lohi); 
    return True;
  }
  if (Memo("BYTE"))
  {
    pseudo_store(wr_code_byte); 
    return True;
  }
  if (Memo("WORD"))
  {
    pseudo_store(wr_code_word); 
    return True;
  }
  if (Memo("LONG"))
  {
    pseudo_store(wr_code_long); 
    return True;
  }

  /* Qxx */

  if ((OpPart[0] == 'Q') && (OpPart[1] >= '0') && (OpPart[1] <= '9') &&
     (OpPart[2] >= '0') && (OpPart[2] <= '9') && (OpPart[3] == '\0'))
  {
    pseudo_qxx(10 * (OpPart[1] - '0') + OpPart[2] - '0');
    return True;
  }

  /* LQxx */

  if ((OpPart[0] == 'L') && (OpPart[1] == 'Q') && (OpPart[2] >= '0') && 
     (OpPart[2] <= '9') && (OpPart[3] >= '0') && (OpPart[3] <= '9') && 
     (OpPart[4] == '\0'))
  {
    pseudo_lqxx(10 * (OpPart[2] - '0') + OpPart[3] - '0');
    return True;
  }

  /* Floating point definitions */

  if (Memo("FLOAT"))
  {
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for(z = 1; (z <= ArgCnt) && ok; z++)
    {
      flt = EvalFloatExpression(ArgStr[z], Float32, &ok);
      memcpy(WAsmCode+CodeLen, &flt, sizeof(float));
      if (BigEndian)
      {
        w = WAsmCode[CodeLen];
        WAsmCode[CodeLen] = WAsmCode[CodeLen+1];
        WAsmCode[CodeLen+1] = w;
      }
      CodeLen += sizeof(float)/2;
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }

  if (Memo("DOUBLE"))
  {
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for(z = 1; (z <= ArgCnt) && ok; z++)
    {
      dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
      memcpy(WAsmCode+CodeLen, &dbl, sizeof(dbl));
      if (BigEndian)
      {
        w = WAsmCode[CodeLen];
        WAsmCode[CodeLen] = WAsmCode[CodeLen+3];
        WAsmCode[CodeLen+3] = w;
        w = WAsmCode[CodeLen+1];
        WAsmCode[CodeLen+1] = WAsmCode[CodeLen+2];
        WAsmCode[CodeLen+2] = w;
      }
      CodeLen += sizeof(dbl)/2;
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }

  if (Memo("EFLOAT"))
  {
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for(z = 1; (z <= ArgCnt) && ok; z++)
    {
      dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
      mant = frexp(dbl, &exp);
      WAsmCode[CodeLen++] = ldexp(mant, 15);
      WAsmCode[CodeLen++] = exp-1;
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }

  if (Memo("BFLOAT"))
  {
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for(z = 1; (z <= ArgCnt) && ok; z++)
    {
      dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
      mant = frexp(dbl, &exp);
      lmant = ldexp(mant, 31);
      WAsmCode[CodeLen++] = (lmant & 0xffff);
      WAsmCode[CodeLen++] = (lmant >> 16);
      WAsmCode[CodeLen++] = exp-1;
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }

  if (Memo("TFLOAT"))
  {
    if (!ArgCnt)
    {
      WrError(1110);
      return True;
    }
    define_untyped_label();
    ok = True;
    for(z = 1; (z <= ArgCnt) && ok; z++)
    {
      dbl = EvalFloatExpression(ArgStr[z], Float64, &ok);
      mant = frexp(dbl, &exp);
      mant = modf(ldexp(mant, 15), &dbl);
      WAsmCode[CodeLen + 3] = dbl;
      mant = modf(ldexp(mant, 16), &dbl);
      WAsmCode[CodeLen + 2] = dbl;
      mant = modf(ldexp(mant, 16), &dbl);
      WAsmCode[CodeLen + 1] = dbl;
      mant = modf(ldexp(mant, 16), &dbl);
      WAsmCode[CodeLen] = dbl;
      CodeLen += 4;
      WAsmCode[CodeLen++] = ((exp - 1) & 0xffff);
      WAsmCode[CodeLen++] = ((exp - 1) >> 16);
    }
    if (!ok)
      CodeLen = 0;
    return True;
  }
  return False;
}

        void CodeEquate(ShortInt DestSeg, LargeInt Min, LargeInt Max)
BEGIN
   Boolean OK;
   TempResult t;
   LargeInt Erg;

   FirstPassUnknown=False;
   if (ArgCnt!=1) WrError(1110);
   else
    BEGIN
     Erg=EvalIntExpression(ArgStr[1],Int32,&OK);
     if ((OK) AND (NOT FirstPassUnknown))
      BEGIN
       if (Min>Erg) WrError(1315);
       else if (Erg>Max) WrError(1320);
       else
        BEGIN
         PushLocHandle(-1);
         EnterIntSymbol(LabPart,Erg,DestSeg,False);
         PopLocHandle();
         if (MakeUseList)
          if (AddChunk(SegChunks+DestSeg,Erg,1,False)) WrError(90);
         t.Typ=TempInt; t.Contents.Int=Erg; SetListLineVal(&t);
        END
      END
    END
END

        void CodeASSUME(ASSUMERec *Def, Integer Cnt)
BEGIN
   int z1,z2;
   Boolean OK;
   LongInt HVal;
   String RegPart,ValPart;

   if (ArgCnt==0) WrError(1110);
   else
    BEGIN
     z1=1; OK=True;
     while ((z1<=ArgCnt) AND (OK))
      BEGIN
       SplitString(ArgStr[z1],RegPart,ValPart,QuotPos(ArgStr[z1],':'));
       z2=0; NLS_UpString(RegPart);
       while ((z2<Cnt) AND (strcmp(Def[z2].Name,RegPart)!=0)) z2++;
       OK=(z2<Cnt);
       if (NOT OK) WrXError(1980,RegPart);
       else
        if (strcmp(ValPart,"NOTHING")==0)
         if (Def[z2].NothingVal==-1) WrError(1350);
         else *Def[z2].Dest=Def[z2].NothingVal;
        else
         BEGIN
          FirstPassUnknown=False;
          HVal=EvalIntExpression(ValPart,Int32,&OK);
          if (OK)
           BEGIN
            if (FirstPassUnknown)
             BEGIN
              WrError(1820); OK=False;
             END
            else if (ChkRange(HVal,Def[z2].Min,Def[z2].Max)) *Def[z2].Dest=HVal;
           END
         END
       z1++;
      END
    END
END

        void codepseudo_init(void)
BEGIN
END

