/* code3201x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS3201x-Familie                                            */
/*                                                                           */
/* Historie: 28.11.1996 Grundsteinlegung                                     */
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1992 BookKeeping-Aufruf in RES                            */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "bpemu.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
#include "codevars.h"


typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Boolean Must1;
         } AdrOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Word AllowShifts;
         } AdrShiftOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Integer Min,Max;
          Word Mask;
         } ImmOrder;


#define FixedOrderCnt 14
#define JmpOrderCnt 11
#define AdrOrderCnt 21
#define AdrShiftOrderCnt 5
#define ImmOrderCnt 3


static Word AdrMode;
static Boolean AdrOK;

static CPUVar CPU32010,CPU32015;

static FixedOrder *FixedOrders;
static FixedOrder *JmpOrders;
static AdrOrder *AdrOrders;
static AdrShiftOrder *AdrShiftOrders;
static ImmOrder *ImmOrders;

/*----------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddJmp(char *NName, Word NCode)
BEGIN
   if (InstrZ>=JmpOrderCnt) exit(255);
   JmpOrders[InstrZ].Name=NName;
   JmpOrders[InstrZ++].Code=NCode;
END

        static void AddAdr(char *NName, Word NCode, Word NMust1)
BEGIN
   if (InstrZ>=AdrOrderCnt) exit(255);
   AdrOrders[InstrZ].Name=NName;
   AdrOrders[InstrZ].Code=NCode;
   AdrOrders[InstrZ++].Must1=NMust1;
END

        static void AddAdrShift(char *NName, Word NCode, Word NAllow)
BEGIN
   if (InstrZ>=AdrShiftOrderCnt) exit(255);
   AdrShiftOrders[InstrZ].Name=NName;
   AdrShiftOrders[InstrZ].Code=NCode;
   AdrShiftOrders[InstrZ++].AllowShifts=NAllow; 
END

        static void AddImm(char *NName, Word NCode, Integer NMin, Integer NMax, Word NMask)
BEGIN
   if (InstrZ>=ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Name=NName;
   ImmOrders[InstrZ].Code=NCode;
   ImmOrders[InstrZ].Min=NMin;
   ImmOrders[InstrZ].Max=NMax;
   ImmOrders[InstrZ++].Mask=NMask;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("ABS"   , 0x7f88);  AddFixed("APAC"  , 0x7f8f);
   AddFixed("CALA"  , 0x7f8c);  AddFixed("DINT"  , 0x7f81);
   AddFixed("EINT"  , 0x7f82);  AddFixed("NOP"   , 0x7f80);
   AddFixed("PAC"   , 0x7f8e);  AddFixed("POP"   , 0x7f9d);
   AddFixed("PUSH"  , 0x7f9c);  AddFixed("RET"   , 0x7f8d);
   AddFixed("ROVM"  , 0x7f8a);  AddFixed("SOVM"  , 0x7f8b);
   AddFixed("SPAC"  , 0x7f90);  AddFixed("ZAC"   , 0x7f89);

   JmpOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*JmpOrderCnt); InstrZ=0;
   AddJmp("B"     , 0xf900);  AddJmp("BANZ"  , 0xf400);
   AddJmp("BGEZ"  , 0xfd00);  AddJmp("BGZ"   , 0xfc00);
   AddJmp("BIOZ"  , 0xf600);  AddJmp("BLEZ"  , 0xfb00);
   AddJmp("BLZ"   , 0xfa00);  AddJmp("BNZ"   , 0xfe00);
   AddJmp("BV"    , 0xf500);  AddJmp("BZ"    , 0xff00);
   AddJmp("CALL"  , 0xf800);

   AdrOrders=(AdrOrder *) malloc(sizeof(AdrOrder)*AdrOrderCnt); InstrZ=0;
   AddAdr("ADDH"  , 0x6000, False);  AddAdr("ADDS"  , 0x6100, False);
   AddAdr("AND"   , 0x7900, False);  AddAdr("DMOV"  , 0x6900, False);
   AddAdr("LDP"   , 0x6f00, False);  AddAdr("LST"   , 0x7b00, False);
   AddAdr("LT"    , 0x6a00, False);  AddAdr("LTA"   , 0x6c00, False);
   AddAdr("LTD"   , 0x6b00, False);  AddAdr("MAR"   , 0x6800, False);
   AddAdr("MPY"   , 0x6d00, False);  AddAdr("OR"    , 0x7a00, False);
   AddAdr("SST"   , 0x7c00, True );  AddAdr("SUBC"  , 0x6400, False);
   AddAdr("SUBH"  , 0x6200, False);  AddAdr("SUBS"  , 0x6300, False);
   AddAdr("TBLR"  , 0x6700, False);  AddAdr("TBLW"  , 0x7d00, False);
   AddAdr("XOR"   , 0x7800, False);  AddAdr("ZALH"  , 0x6500, False);
   AddAdr("ZALS"  , 0x6600, False);

   AdrShiftOrders=(AdrShiftOrder *) malloc(sizeof(AdrShiftOrder)*AdrShiftOrderCnt); InstrZ=0;
   AddAdrShift("ADD"   , 0x0000, 0xffff);
   AddAdrShift("LAC"   , 0x2000, 0xffff);
   AddAdrShift("SACH"  , 0x5800, 0x0013);
   AddAdrShift("SACL"  , 0x5000, 0x0001);
   AddAdrShift("SUB"   , 0x1000, 0xffff);

   ImmOrders=(ImmOrder *) malloc(sizeof(ImmOrder)*ImmOrderCnt); InstrZ=0;
   AddImm("LACK", 0x7e00,     0,  255,   0xff);
   AddImm("LDPK", 0x6e00,     0,    1,    0x1);
   AddImm("MPYK", 0x8000, -4096, 4095, 0x1fff);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(JmpOrders);
   free(AdrOrders);
   free(AdrShiftOrders);
   free(ImmOrders);
END

/*----------------------------------------------------------------------------*/

        static Word EvalARExpression(char *Asc, Boolean *OK)
BEGIN
   *OK=True;
   if (strcasecmp(Asc,"AR0")==0) return 0;
   if (strcasecmp(Asc,"AR1")==0) return 1;
   return EvalIntExpression(Asc,UInt1,OK);
END

        static void DecodeAdr(char *Arg, int Aux, Boolean Must1)
BEGIN
   Byte h;
   char *p;

   AdrOK=False;

   if ((strcmp(Arg,"*")==0) OR (strcmp(Arg,"*-")==0) OR (strcmp(Arg,"*+")==0))
    BEGIN
     AdrMode=0x88;
     if (strlen(Arg)==2)
      AdrMode+=(Arg[1]=='+') ? 0x20 : 0x10;
     if (Aux<=ArgCnt)
      BEGIN
       h=EvalARExpression(ArgStr[Aux],&AdrOK);
       if (AdrOK)
        BEGIN
         AdrMode&=0xf7; AdrMode+=h;
        END
      END
     else AdrOK=True;
    END
   else if (Aux<=ArgCnt) WrError(1110);
   else
    BEGIN
     h=0;
     if ((strlen(Arg)>3) AND (strncasecmp(Arg,"DAT",3)==0))
      BEGIN
       AdrOK=True;
       for (p=Arg+3; *p!='\0'; p++)
        if ((*p>'9') OR (*p<'0')) AdrOK=False;
       if (AdrOK) h=EvalIntExpression(Arg+3,UInt8,&AdrOK);
      END
     if (NOT AdrOK) h=EvalIntExpression(Arg,Int8,&AdrOK);
     if (AdrOK)
      BEGIN
       if ((Must1) AND (h<0x80) AND (NOT FirstPassUnknown))
        BEGIN
         WrError(1315); AdrOK=False;
        END
       else
        BEGIN
         AdrMode=h & 0x7f; ChkSpace(SegData);
        END
      END
    END
END

        static Boolean DecodePseudo(void)
BEGIN
   Word Size;
   int z,z2;
   char *p;
   TempResult t;
   Boolean OK;

   if (Memo("PORT"))
    BEGIN
     CodeEquate(SegIO,0,7);
     return True;
    END

   if (Memo("RES"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Size=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (FirstPassUnknown) WrError(1820);
       if ((OK) AND (NOT FirstPassUnknown))
        BEGIN
         DontPrint=True;
         CodeLen=Size;
         BookKeeping();
        END
      END
     return True;
    END

   if (Memo("DATA"))
    BEGIN
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       OK=True;
       for (z=1; z<=ArgCnt; z++)
        if (OK)
         BEGIN
          EvalExpression(ArgStr[z],&t);
          switch (t.Typ)
           BEGIN
            case TempInt:
             if ((t.Contents.Int<-32768) OR (t.Contents.Int>0xffff))
              BEGIN
               WrError(1320); OK=False;
              END
             else WAsmCode[CodeLen++]=t.Contents.Int;
             break;
            case TempFloat:
             WrError(1135); OK=False;
             break;
            case TempString:
             for (p=t.Contents.Ascii,z2=0; *p!='\0'; p++,z2++)
              BEGIN
               if ((z2&1)==0)
                WAsmCode[CodeLen]=CharTransTable[((usint)*p)&0xff];
               else
                WAsmCode[CodeLen++]+=((Word) CharTransTable[((usint)*p)&0xff]) << 8;
              END
             if ((z2&1)==0) CodeLen++;
             break;
            default:
             OK=False;
           END
         END
       if (NOT OK) CodeLen=0;
      END
     return True;
    END

   return False;
END

        static void MakeCode_3201X(void)
BEGIN
   Boolean OK,HasSh;
   Word AdrWord;
   LongInt AdrLong;
   int z,Cnt;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* kein Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        CodeLen=1; WAsmCode[0]=FixedOrders[z].Code;
       END
      return;
     END

   /* Spruenge */

   for (z=0; z<JmpOrderCnt; z++)
    if (Memo(JmpOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        WAsmCode[1]=EvalIntExpression(ArgStr[1],UInt12,&OK);
        if (OK)
         BEGIN
          CodeLen=2; WAsmCode[0]=JmpOrders[z].Code;
         END
       END
      return;
     END

   /* nur Adresse */

   for (z=0; z<AdrOrderCnt; z++)
    if (Memo(AdrOrders[z].Name))
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else
       BEGIN
        DecodeAdr(ArgStr[1],2,AdrOrders[z].Must1);
        if (AdrOK)
         BEGIN
          CodeLen=1; WAsmCode[0]=AdrOrders[z].Code+AdrMode;
         END
       END
      return;
     END

   /* Adresse & schieben */

   for (z=0; z<AdrShiftOrderCnt; z++)
    if (Memo(AdrShiftOrders[z].Name))
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>3)) WrError(1110);
      else
       BEGIN
        if (*ArgStr[1]=='*')
         if (ArgCnt==2)
          if (strncasecmp(ArgStr[2],"AR",2)==0)
           BEGIN
            HasSh=False; Cnt=2;
           END
          else
           BEGIN
            HasSh=True; Cnt=3;
           END
         else 
          BEGIN
           HasSh=True; Cnt=3;
          END
        else 
         BEGIN
          Cnt=3; HasSh=(ArgCnt==2);
         END
        DecodeAdr(ArgStr[1],Cnt,False);
        if (AdrOK)
         BEGIN
          if (NOT HasSh)
           BEGIN
            OK=True; AdrWord=0;
           END
          else
           BEGIN
            AdrWord=EvalIntExpression(ArgStr[2],Int4,&OK);
            if ((OK) AND (FirstPassUnknown)) AdrWord=0;
           END
          if (OK)
           BEGIN
            if ((AdrShiftOrders[z].AllowShifts & (1 << AdrWord))==0) WrError(1380);
            else
             BEGIN
              CodeLen=1; WAsmCode[0]=AdrShiftOrders[z].Code+AdrMode+(AdrWord << 8);
             END
           END
         END
       END
      return;
     END

   /* Ein/Ausgabe */

   if ((Memo("IN")) OR (Memo("OUT")))
    BEGIN
     if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
     else
      BEGIN
       DecodeAdr(ArgStr[1],3,False);
       if (AdrOK)
        BEGIN
         AdrWord=EvalIntExpression(ArgStr[2],UInt3,&OK);
         if (OK)
          BEGIN
           ChkSpace(SegIO);
           CodeLen=1;
           WAsmCode[0]=0x4000+AdrMode+(AdrWord << 8);
           if (Memo("OUT")) WAsmCode[0]+=0x800;
          END
        END
      END
     return;
    END

   /* konstantes Argument */

   for (z=0; z<ImmOrderCnt; z++)
    if (Memo(ImmOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrLong=EvalIntExpression(ArgStr[1],Int32,&OK);
        if (OK)
         BEGIN
          if (FirstPassUnknown) AdrLong&=ImmOrders[z].Mask;
          if (AdrLong<ImmOrders[z].Min) WrError(1315);
          else if (AdrLong>ImmOrders[z].Max) WrError(1320);
          else
           BEGIN
            CodeLen=1; WAsmCode[0]=ImmOrders[z].Code+(AdrLong & ImmOrders[z].Mask);
           END
         END
       END
      return;
     END

   /* mit Hilfsregistern */

   if (Memo("LARP"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrWord=EvalARExpression(ArgStr[1],&OK);
       if (OK)
        BEGIN
         CodeLen=1; WAsmCode[0]=0x6880+AdrWord;
        END
      END
     return;
    END

   if ((Memo("LAR")) OR (Memo("SAR")))
    BEGIN
     if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
     else
      BEGIN
       AdrWord=EvalARExpression(ArgStr[1],&OK);
       if (OK)
        BEGIN
         DecodeAdr(ArgStr[2],3,False);
         if (AdrOK)
          BEGIN
           CodeLen=1;
           WAsmCode[0]=0x3000+AdrMode+(AdrWord << 8);
           if (Memo("LAR")) WAsmCode[0]+=0x800;
          END
        END
      END
     return;
    END

   if (Memo("LARK"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       AdrWord=EvalARExpression(ArgStr[1],&OK);
       if (OK)
        BEGIN
         WAsmCode[0]=EvalIntExpression(ArgStr[2],Int8,&OK);
         if (OK)
          BEGIN
           CodeLen=1;
           WAsmCode[0]=Lo(WAsmCode[0])+0x7000+(AdrWord << 8);
          END
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static Boolean IsDef_3201X(void)
BEGIN
   return (Memo("PORT"));
END

        static void SwitchFrom_3201X(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_3201X(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x74; NOPCode=0x7f80;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode)|(1<<SegData)|(1<<SegIO);
   Grans[SegCode]=2; ListGrans[SegCode]=2; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xfff;
   Grans[SegData]=2; ListGrans[SegData]=2; SegInits[SegData]=0;
   SegLimits[SegData] = (MomCPU==CPU32010) ? 0x8f : 0xff;
   Grans[SegIO  ]=2; ListGrans[SegIO  ]=2; SegInits[SegIO  ]=0;
   SegLimits[SegIO  ] = 7;

   MakeCode=MakeCode_3201X; IsDef=IsDef_3201X;
   SwitchFrom=SwitchFrom_3201X; InitFields();
END

        void code3201x_init(void)
BEGIN
   CPU32010=AddCPU("32010",SwitchTo_3201X);
   CPU32015=AddCPU("32015",SwitchTo_3201X);
END
