/* motpseudo.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Haeufiger benutzte Motorola-Pseudo-Befehle                                */
/*                                                                           */
/*****************************************************************************/
/* $Id: motpseudo.c,v 1.3 2004/09/20 18:44:37 alfred Exp $                   */
/***************************************************************************** 
 * $Log: motpseudo.c,v $
 * Revision 1.3  2004/09/20 18:44:37  alfred
 * - formatting cleanups
 *
 * Revision 1.2  2004/05/29 14:57:56  alfred
 * - added missing include statements
 *
 * Revision 1.1  2004/05/29 12:04:48  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 *****************************************************************************/

/*****************************************************************************
 * Includes
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>
#include <math.h>

#include "bpemu.h"
#include "endian.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"        
#include "asmallg.h"

#include "motpseudo.h"

/***************************************************************************** 
 * Local Variables
 *****************************************************************************/

static Boolean M16Turn = False;

/*****************************************************************************
 * Local Functions
 *****************************************************************************/

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
{
   int z;
   Boolean OK;
   TempResult t;
   LongInt Rep,z2;
   UNUSED(Index);

   if (ArgCnt == 0) WrError(1110);
   else
   {
     z = 1; OK = True;
     do
     {
       if (!*ArgStr[z])
       {
         OK = FALSE;
         WrError(2050);
         break;
       }

       KillBlanks(ArgStr[z]);
       OK = CutRep(ArgStr[z], &Rep);
       if (!OK)
         break;

       EvalExpression(ArgStr[z], &t);
       switch (t.Typ)
       {
         case TempInt:
           if (NOT RangeCheck(t.Contents.Int, Int8))
           {
             WrError(1320); OK = False;
           }
           else if (CodeLen + Rep > MaxCodeLen)
           {
             WrError(1920); OK = False;
           }
           else
           {
             memset(BAsmCode + CodeLen, t.Contents.Int, Rep);
             CodeLen += Rep;
           }
           break;
         case TempFloat:
           WrError(1135); OK = False;
           break;
         case TempString:
           TranslateString(t.Contents.Ascii);
           if (CodeLen + Rep * strlen(t.Contents.Ascii) > MaxCodeLen)
           {
             WrError(1920); OK = False;
           }
           else
             for (z2 = 0; z2 < Rep; z2++)
             {
               memcpy(BAsmCode + CodeLen, t.Contents.Ascii, strlen(t.Contents.Ascii));
               CodeLen += strlen(t.Contents.Ascii);
             }
           break;
         default:
          OK = False; 
          break;
       }

       z++;
     }
     while ((z <= ArgCnt) AND (OK));

     if (NOT OK)
       CodeLen = 0;
   }
}

static void DecodeADR(Word Index)
{
   int z;
   Word HVal16;
   Boolean OK;
   LongInt Rep,z2;
   UNUSED(Index);

   if (ArgCnt == 0) WrError(1110);
   else
   {
     z = 1; OK = True;
     do
     {
       if (!*ArgStr[z])
       { 
         OK = FALSE;  
         WrError(2050);
         break;
       }

       OK = CutRep(ArgStr[z], &Rep);
       if (!OK)
         break; 

       if (CodeLen + (Rep << 1) > MaxCodeLen)
       {
         WrError(1920);
         OK = False;
         break;
       }

       HVal16 = EvalIntExpression(ArgStr[z], Int16, &OK);
       if (OK)
         for (z2 = 0; z2 < Rep; z2++)
         {
           if (M16Turn)
           {
             BAsmCode[CodeLen++] = Hi(HVal16);
             BAsmCode[CodeLen++] = Lo(HVal16);
           }
           else
           {
             BAsmCode[CodeLen++] = Lo(HVal16);
             BAsmCode[CodeLen++] = Hi(HVal16);
           }
         }

       z++;
     }
     while ((z <= ArgCnt) AND (OK));

     if (NOT OK)
       CodeLen = 0;
   }
}

static void DecodeFCC(Word Index)
{
   String SVal;
   Boolean OK;
   int z;
   LongInt Rep,z2;
   UNUSED(Index);

   if (ArgCnt == 0) WrError(1110);
   else
   {
     z = 1; OK = True;

     do
     {
       if (!*ArgStr[z])
       { 
         OK = FALSE;
         WrError(2050);
         break;
       }

       OK = CutRep(ArgStr[z], &Rep);
       if (!OK)  
         break;  

       EvalStringExpression(ArgStr[z], &OK, SVal);
       if (OK)
       {
         if (CodeLen + Rep * strlen(SVal) >= MaxCodeLen)
         {
           WrError(1920);
           OK = False;
         }
         else
         {
           TranslateString(SVal);
           for (z2 = 0; z2 < Rep; z2++)
           {
             memcpy(BAsmCode + CodeLen, SVal, strlen(SVal));
             CodeLen += strlen(SVal);
           }
         }
       }

       z++;
     }
     while ((z <= ArgCnt) AND (OK));

     if (NOT OK)
       CodeLen = 0;
   }
}

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

/*****************************************************************************
 * Global Functions
 *****************************************************************************/

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

        void ConvertMotoFloatDec(Double F, Word *w)
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
   if (((CodeLen & 1) == 1) && (!BigEndian) && (ListGran() != 1))
   {
     BAsmCode[CodeLen    ] = BAsmCode[CodeLen - 1];
     BAsmCode[CodeLen - 1] = b;
   }
   else
   {
     BAsmCode[CodeLen] = b;
   }
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
         if (!*ArgStr[z])
         { 
           OK = FALSE;
           WrError(2050);
           break;
         }

         FirstPassUnknown = False;
         OK = CutRep(ArgStr[z], &Rep);
         if (!OK)  
           break;  
         if (FirstPassUnknown)
         {
           OK = FALSE;
           WrError(1820);
           break;
         }

         if (!strcmp(ArgStr[z], "?"))
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
               {
                 case TempInt:
                  if (!RangeCheck(t.Contents.Int, Int8))
                  {
                    WrError(1320); OK = False;
                  }
                  else if (CodeLen+Rep > MaxCodeLen)
                  {
                    WrError(1920); OK = False;
                  }
                  else 
                    for (z2 = 0; z2 < Rep; z2++) 
                      EnterByte(t.Contents.Int);
                  break;
                 case TempFloat:
                   WrError(1135); OK = False;
                   break;
                 case TempString:
                   if (CodeLen+Rep*strlen(t.Contents.Ascii)>MaxCodeLen)
                   {
                     WrError(1920); OK=False;
                   }
                   else 
                    for (z2 = 0; z2 < Rep; z2++)
                      for (zp = t.Contents.Ascii; *zp != '\0'; EnterByte(CharTransTable[((usint) *(zp++)) & 0xff]));
                   break;
                 default:
                   OK = False;
               }
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
                  ConvertMotoFloatDec(DVal,TurnField);
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

