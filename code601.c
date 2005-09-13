/* code601.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator PowerPC-Familie                                             */
/*                                                                           */
/* Historie: 17.10.1996 Grundsteinlegung                                     */
/*           30. 8.1998 Umstellung auf 32-Bit-Zugriffe                       */
/*                      Header-ID per Abfrage                                */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.1999 MMU-Instruktionen                                    */
/*           10. 3.1999 PPC403-MMU-Befehle                                   */
/*           28. 3.1999 PPC403GB auf GC korrigiert (erst der hat eine MMU)   */
/*            8. 9.1999 REG-Befehl nachgeruestet                             */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code601.c,v 1.3 2005/09/08 17:31:03 alfred Exp $                     */
/*****************************************************************************
 * $Log: code601.c,v $
 * Revision 1.3  2005/09/08 17:31:03  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 11:33:01  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "endian.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "asmitree.h"
#include "intpseudo.h"
#include "codevars.h"
#include "headids.h"

typedef struct 
         {
          char *Name;
          LongWord Code;
          Byte CPUMask;
         } BaseOrder;

#define FixedOrderCount      8
#define Reg1OrderCount       4
#define FReg1OrderCount      2
#define CReg1OrderCount      1
#define CBit1OrderCount      4
#define Reg2OrderCount       29
#define CReg2OrderCount      2
#define FReg2OrderCount      14
#define Reg2BOrderCount      2
#define Reg2SwapOrderCount   6
#define NoDestOrderCount     10
#define Reg3OrderCount       91
#define CReg3OrderCount      8
#define FReg3OrderCount      10
#define Reg3SwapOrderCount   49
#define MixedOrderCount      8
#define FReg4OrderCount      16
#define RegDispOrderCount    16
#define FRegDispOrderCount   8
#define Reg2ImmOrderCount    12
#define Imm16OrderCount      7
#define Imm16SwapOrderCount  6

static BaseOrder *FixedOrders;
static BaseOrder *Reg1Orders;
static BaseOrder *CReg1Orders;
static BaseOrder *CBit1Orders;
static BaseOrder *FReg1Orders;
static BaseOrder *Reg2Orders;
static BaseOrder *CReg2Orders;
static BaseOrder *FReg2Orders;
static BaseOrder *Reg2BOrders;
static BaseOrder *Reg2SwapOrders;
static BaseOrder *NoDestOrders;
static BaseOrder *Reg3Orders;
static BaseOrder *CReg3Orders;
static BaseOrder *FReg3Orders;
static BaseOrder *Reg3SwapOrders;
static BaseOrder *MixedOrders;
static BaseOrder *FReg4Orders;
static BaseOrder *RegDispOrders;
static BaseOrder *FRegDispOrders;
static BaseOrder *Reg2ImmOrders;
static BaseOrder *Imm16Orders;
static BaseOrder *Imm16SwapOrders;

static SimpProc SaveInitProc;
static Boolean BigEnd;

static CPUVar CPU403, CPU403C, CPU505, CPU601, CPU6000;

#define M_403 0x01
#define M_403C 0x02
#define M_505 0x04
#define M_601 0x08
#define M_6000 0x10

/*-------------------------------------------------------------------------*/
/*
        PROCEDURE EnterByte(b:Byte);
 BEGIN
   if Odd(CodeLen)
    BEGIN
     BAsmCode[CodeLen]:=BAsmCode[CodeLen-1];
     BAsmCode[CodeLen-1]:=b;
    END
   else
    BEGIN
     BAsmCode[CodeLen]:=b;
    END;
   Inc(CodeLen);
END;
*/
/*-------------------------------------------------------------------------*/

        static void AddFixed(char *NName1, char *NName2, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ++].CPUMask=NMask;
END

        static void AddReg1(char *NName1, char *NName2, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=Reg1OrderCount) exit(255);
   Reg1Orders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   Reg1Orders[InstrZ].Code=NCode;
   Reg1Orders[InstrZ++].CPUMask=NMask;
END

        static void AddCReg1(char *NName1, char *NName2, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=CReg1OrderCount) exit(255);
   CReg1Orders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   CReg1Orders[InstrZ].Code=NCode;
   CReg1Orders[InstrZ++].CPUMask=NMask;
END

        static void AddCBit1(char *NName1, char *NName2, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=CBit1OrderCount) exit(255);
   CBit1Orders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   CBit1Orders[InstrZ].Code=NCode;
   CBit1Orders[InstrZ++].CPUMask=NMask;
END

        static void AddFReg1(char *NName1, char *NName2, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=FReg1OrderCount) exit(255);
   FReg1Orders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   FReg1Orders[InstrZ].Code=NCode;
   FReg1Orders[InstrZ++].CPUMask=NMask;
END

        static void AddSReg2(char *NName, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=Reg2OrderCount) exit(255);
   if (NName==Nil) exit(255);
   Reg2Orders[InstrZ].Name=NName;
   Reg2Orders[InstrZ].Code=NCode;
   Reg2Orders[InstrZ++].CPUMask=NMask;
END

        static void AddReg2(char *NName1, char *NName2, LongInt NCode, Byte NMask, Boolean WithOE, Boolean WithFL)
BEGIN
   String NName;

   strcpy(NName,(MomCPU==CPU6000)?NName2:NName1);
   AddSReg2(strdup(NName),NCode,NMask);
   if (WithOE)
    BEGIN
     strcat(NName,"O");
     AddSReg2(strdup(NName),NCode | 0x400,NMask);
     NName[strlen(NName)-1]='\0';
    END
   if (WithFL)
    BEGIN
     strcat(NName,"."); 
     AddSReg2(strdup(NName),NCode | 0x001,NMask);
     NName[strlen(NName)-1]='\0';
     if (WithOE)
      BEGIN
       strcat(NName,"O."); 
       AddSReg2(strdup(NName),NCode | 0x401,NMask);
      END
    END
END

        static void AddCReg2(char *NName1, char *NName2, LongWord NCode, Byte NMask)
BEGIN
   if (InstrZ>=CReg2OrderCount) exit(255);
   CReg2Orders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   CReg2Orders[InstrZ].Code=NCode;
   CReg2Orders[InstrZ++].CPUMask=NMask;
END

        static void AddSFReg2(char *NName, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=FReg2OrderCount) exit(255);
   if (NName==Nil) exit(255);
   FReg2Orders[InstrZ].Name=NName;
   FReg2Orders[InstrZ].Code=NCode;
   FReg2Orders[InstrZ++].CPUMask=NMask;
END

        static void AddFReg2(char *NName1, char *NName2, LongInt NCode, Byte NMask, Boolean WithFL)
BEGIN
   String NName;

   strcpy(NName,(MomCPU==CPU6000)?NName2:NName1);
   AddSFReg2(strdup(NName),NCode,NMask);
   if (WithFL)
    BEGIN
     strcat(NName,"."); 
     AddSFReg2(strdup(NName),NCode | 0x001,NMask);
    END
END

        static void AddReg2B(char *NName1, char *NName2, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=Reg2BOrderCount) exit(255);
   Reg2BOrders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   Reg2BOrders[InstrZ].Code=NCode;
   Reg2BOrders[InstrZ++].CPUMask=NMask;
END

        static void AddSReg2Swap(char *NName, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=Reg2SwapOrderCount) exit(255);
   if (NName==Nil) exit(255);
   Reg2SwapOrders[InstrZ].Name=NName;
   Reg2SwapOrders[InstrZ].Code=NCode;
   Reg2SwapOrders[InstrZ++].CPUMask=NMask;
END

        static void AddReg2Swap(char *NName1, char *NName2, LongInt NCode, Byte NMask, Boolean WithOE, Boolean WithFL)
BEGIN
   String NName;

   strcpy(NName,(MomCPU==CPU6000)?NName2:NName1);
   AddSReg2Swap(strdup(NName),NCode,NMask);
   if (WithOE)
    BEGIN
     strcat(NName,"O");
     AddSReg2Swap(strdup(NName),NCode | 0x400,NMask);
     NName[strlen(NName)-1]='\0';
    END
   if (WithFL)
    BEGIN
     strcat(NName,"."); 
     AddSReg2Swap(strdup(NName),NCode | 0x001,NMask);
     NName[strlen(NName)-1]='\0';
     if (WithOE)
      BEGIN
       strcat(NName,"O."); 
       AddSReg2Swap(strdup(NName),NCode | 0x401,NMask);
      END
    END
END

        static void AddNoDest(char *NName1, char *NName2, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=NoDestOrderCount) exit(255);
   NoDestOrders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   NoDestOrders[InstrZ].Code=NCode;
   NoDestOrders[InstrZ++].CPUMask=NMask;
END

        static void AddSReg3(char *NName, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=Reg3OrderCount) exit(255);
   if (NName==Nil) exit(255);
   Reg3Orders[InstrZ].Name=NName;
   Reg3Orders[InstrZ].Code=NCode;
   Reg3Orders[InstrZ++].CPUMask=NMask;
END

        static void AddReg3(char *NName1, char *NName2, LongInt NCode, Byte NMask, Boolean WithOE, Boolean WithFL)
BEGIN
   String NName;

   strcpy(NName,(MomCPU==CPU6000)?NName2:NName1);
   AddSReg3(strdup(NName),NCode,NMask);
   if (WithOE)
    BEGIN
     strcat(NName,"O");
     AddSReg3(strdup(NName),NCode | 0x400,NMask);
     NName[strlen(NName)-1]='\0';
    END
   if (WithFL)
    BEGIN
     strcat(NName,"."); 
     AddSReg3(strdup(NName),NCode | 0x001,NMask);
     NName[strlen(NName)-1]='\0';
     if (WithOE)
      BEGIN
       strcat(NName,"O."); 
       AddSReg3(strdup(NName),NCode | 0x401,NMask);
      END
    END
END

        static void AddCReg3(char *NName, LongWord NCode, CPUVar NMask)
BEGIN
   if (InstrZ>=CReg3OrderCount) exit(255);
   CReg3Orders[InstrZ].Name=NName;
   CReg3Orders[InstrZ].Code=NCode;
   CReg3Orders[InstrZ++].CPUMask=NMask;
END

        static void AddSFReg3(char *NName, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=FReg3OrderCount) exit(255);
   if (NName==Nil) exit(255);
   FReg3Orders[InstrZ].Name=NName;
   FReg3Orders[InstrZ].Code=NCode;
   FReg3Orders[InstrZ++].CPUMask=NMask;
END

        static void AddFReg3(char *NName1, char *NName2, LongInt NCode, Byte NMask, Boolean WithFL)
BEGIN
   String NName;

   strcpy(NName,(MomCPU==CPU6000)?NName2:NName1);
   AddSFReg3(strdup(NName),NCode,NMask);
   if (WithFL)
    BEGIN
     strcat(NName,"."); 
     AddSFReg3(strdup(NName),NCode | 0x001,NMask);
    END
END

        static void AddSReg3Swap(char *NName, LongInt NCode, Byte NMask)
BEGIN
   if (InstrZ>=Reg3SwapOrderCount) exit(255);
   if (NName==Nil) exit(255);
   Reg3SwapOrders[InstrZ].Name=NName;
   Reg3SwapOrders[InstrZ].Code=NCode;
   Reg3SwapOrders[InstrZ++].CPUMask=NMask;
END

        static void AddReg3Swap(char *NName1, char *NName2, LongInt NCode, Byte NMask, Boolean WithFL)
BEGIN
   String NName;

   strcpy(NName,(MomCPU==CPU6000)?NName2:NName1);
   AddSReg3Swap(strdup(NName),NCode,NMask);
   if (WithFL)
    BEGIN
     strcat(NName,"."); 
     AddSReg3Swap(strdup(NName),NCode | 0x001,NMask);
    END
END

        static void AddMixed(char *NName1, char *NName2, LongWord NCode, Byte NMask)
BEGIN
   if (InstrZ>=MixedOrderCount) exit(255);
   MixedOrders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   MixedOrders[InstrZ].Code=NCode;
   MixedOrders[InstrZ++].CPUMask=NMask;
END

        static void AddSFReg4(char *NName, LongWord NCode, Byte NMask)
BEGIN
   if (InstrZ>=FReg4OrderCount) exit(255);
   if (NName==Nil) exit(255);
   FReg4Orders[InstrZ].Name=NName;
   FReg4Orders[InstrZ].Code=NCode;
   FReg4Orders[InstrZ++].CPUMask=NMask;
END

        static void AddFReg4(char *NName1, char *NName2, LongWord NCode, Byte NMask, Boolean WithFL)
BEGIN
   String NName;

   strcpy(NName,(MomCPU==CPU6000)?NName2:NName1);
   AddSFReg4(strdup(NName),NCode,NMask);
   if (WithFL)
    BEGIN
     strcat(NName,"."); 
     AddSFReg4(strdup(NName),NCode | 0x001,NMask);
    END
END

        static void AddRegDisp(char *NName1, char *NName2, LongWord NCode, Byte NMask)
BEGIN
   if (InstrZ>=RegDispOrderCount) exit(255);
   RegDispOrders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   RegDispOrders[InstrZ].Code=NCode;
   RegDispOrders[InstrZ++].CPUMask=NMask;
END

        static void AddFRegDisp(char *NName1, char *NName2, LongWord NCode, Byte NMask)
BEGIN
   if (InstrZ>=FRegDispOrderCount) exit(255);
   FRegDispOrders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   FRegDispOrders[InstrZ].Code=NCode;
   FRegDispOrders[InstrZ++].CPUMask=NMask;
END

        static void AddSReg2Imm(char *NName, LongWord NCode, Byte NMask)
BEGIN
   if (InstrZ>=Reg2ImmOrderCount) exit(255);
   if (NName==Nil) exit(255);
   Reg2ImmOrders[InstrZ].Name=NName;
   Reg2ImmOrders[InstrZ].Code=NCode;
   Reg2ImmOrders[InstrZ++].CPUMask=NMask;
END

        static void AddReg2Imm(char *NName1, char *NName2, LongWord NCode, Byte NMask, Boolean WithFL)
BEGIN
   String NName;

   strcpy(NName,(MomCPU==CPU6000)?NName2:NName1);
   AddSReg2Imm(strdup(NName),NCode,NMask);
   if (WithFL)
    BEGIN
     strcat(NName,"."); 
     AddSReg2Imm(strdup(NName),NCode | 0x001,NMask);
    END
END

        static void AddImm16(char *NName1, char *NName2, LongWord NCode, Byte NMask)
BEGIN
   if (InstrZ>=Imm16OrderCount) exit(255);
   Imm16Orders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   Imm16Orders[InstrZ].Code=NCode;
   Imm16Orders[InstrZ++].CPUMask=NMask;
END

        static void AddImm16Swap(char *NName1, char *NName2, LongWord NCode, Byte NMask)
BEGIN
   if (InstrZ>=Imm16SwapOrderCount) exit(255);
   Imm16SwapOrders[InstrZ].Name=(MomCPU==CPU6000)?NName2:NName1;
   Imm16SwapOrders[InstrZ].Code=NCode;
   Imm16SwapOrders[InstrZ++].CPUMask=NMask;
END

#ifdef __STDC__
#define T1  1lu
#define T3  3lu
#define T4  4lu
#define T7  7lu
#define T8  8lu
#define T9  9lu
#define T10 10lu
#define T11 11lu
#define T12 12lu
#define T13 13lu
#define T14 14lu
#define T15 15lu
#define T16 16lu
#define T17 17lu
#define T18 18lu
#define T19 19lu
#define T20 20lu
#define T21 21lu
#define T22 22lu
#define T23 23lu
#define T24 24lu
#define T25 25lu
#define T26 26lu
#define T27 27lu
#define T28 28lu
#define T29 29lu
#define T31 31lu
#define T32 32lu
#define T33 33lu
#define T34 34lu
#define T35 35lu
#define T36 36lu
#define T37 37lu
#define T38 38lu
#define T39 39lu
#define T40 40lu
#define T41 41lu
#define T42 42lu
#define T43 43lu
#define T44 44lu
#define T45 45lu
#define T46 46lu
#define T47 47lu
#define T48 48lu
#define T49 49lu
#define T50 50lu
#define T51 51lu
#define T52 52lu
#define T53 53lu
#define T54 54lu
#define T55 55lu
#define T59 59lu
#define T63 63lu
#else
#define T1  1l
#define T3  3l
#define T4  4l
#define T7  7l
#define T8  8l
#define T9  9l
#define T10 10l
#define T11 11l
#define T12 12l
#define T13 13l
#define T14 14l
#define T15 15l
#define T16 16l
#define T17 17l
#define T18 18l
#define T19 19l
#define T20 20l
#define T21 21l
#define T22 22l
#define T23 23l
#define T24 24l
#define T25 25l
#define T26 26l
#define T27 27l
#define T28 28l
#define T29 29l
#define T31 31l
#define T32 32l
#define T33 33l
#define T34 34l
#define T35 35l
#define T36 36l
#define T37 37l
#define T38 38l
#define T39 39l
#define T40 40l
#define T41 41l
#define T42 42l
#define T43 43l
#define T44 44l
#define T45 45l
#define T46 46l
#define T47 47l
#define T48 48l
#define T49 49l
#define T50 50l
#define T51 51l
#define T52 52l
#define T53 53l
#define T54 54l
#define T55 55l
#define T59 59l
#define T63 63l
#endif

        static void InitFields(void)
BEGIN
   /* --> 0 0 0 */

   FixedOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("EIEIO"  ,"EIEIO"  ,(T31 << 26)+(854 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddFixed("ISYNC"  ,"ICS"    ,(T19 << 26)+(150 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddFixed("RFI"    ,"RFI"    ,(T19 << 26)+( 50 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddFixed("SC"     ,"SVCA"   ,(T17 << 26)+(  1 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddFixed("SYNC"   ,"DCS"    ,(T31 << 26)+(598 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddFixed("RFCI"   ,"RFCI"   ,(T19 << 26)+( 51 << 1), M_403 | M_403C                         );
   AddFixed("TLBIA"  ,"TLBIA"  ,(T31 << 26)+(370 << 1),         M_403C                         );
   AddFixed("TLBSYNC","TLBSYNC",(T31 << 26)+(566 << 1),         M_403C                         );

   /* D --> D 0 0 */

   Reg1Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*Reg1OrderCount); InstrZ=0;
   AddReg1("MFCR"   ,"MFCR"    ,(T31 << 26)+( 19 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddReg1("MFMSR"  ,"MFMSR"   ,(T31 << 26)+( 83 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddReg1("MTMSR"  ,"MTMSR"   ,(T31 << 26)+(146 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddReg1("WRTEE"  ,"WRTEE"   ,(T31 << 26)+(131 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);

   /* crD --> D 0 0 */

   CReg1Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*CReg1OrderCount); InstrZ=0;
   AddCReg1("MCRXR"  ,"MCRXR"  ,(T31 << 26)+(512 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);

   /* crbD --> D 0 0 */

   CBit1Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*CBit1OrderCount); InstrZ=0;
   AddCBit1("MTFSB0" ,"MTFSB0" ,(T63 << 26)+( 70 << 1)  , M_601 | M_6000);
   AddCBit1("MTFSB0.","MTFSB0.",(T63 << 26)+( 70 << 1)+1, M_601 | M_6000);
   AddCBit1("MTFSB1" ,"MTFSB1" ,(T63 << 26)+( 38 << 1)  , M_601 | M_6000);
   AddCBit1("MTFSB1.","MTFSB1.",(T63 << 26)+( 38 << 1)+1, M_601 | M_6000);

   /* frD --> D 0 0 */

   FReg1Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*FReg1OrderCount); InstrZ=0;
   AddFReg1("MFFS"   ,"MFFS"  ,(T63 << 26)+(583 << 1)  , M_601 | M_6000);
   AddFReg1("MFFS."  ,"MFFS." ,(T63 << 26)+(583 << 1)+1, M_601 | M_6000);

   /* D,A --> D A 0 */

   Reg2Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*Reg2OrderCount); InstrZ=0;
   AddReg2("ABS"   ,"ABS"  ,(T31 << 26)+(360 << 1),                                  M_6000,True ,True );
   AddReg2("ADDME" ,"AME"  ,(T31 << 26)+(234 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg2("ADDZE" ,"AZE"  ,(T31 << 26)+(202 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg2("CLCS"  ,"CLCS" ,(T31 << 26)+(531 << 1),                                  M_6000,False,False);
   AddReg2("NABS"  ,"NABS" ,(T31 << 26)+(488 << 1),                                  M_6000,True ,True );
   AddReg2("NEG"   ,"NEG"  ,(T31 << 26)+(104 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg2("SUBFME","SFME" ,(T31 << 26)+(232 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg2("SUBFZE","SFZE" ,(T31 << 26)+(200 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );

   /* cD,cS --> D S 0 */

   CReg2Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*CReg2OrderCount); InstrZ=0;
   AddCReg2("MCRF"  ,"MCRF"  ,(T19 << 26)+(  0 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddCReg2("MCRFS" ,"MCRFS" ,(T63 << 26)+( 64 << 1),                          M_601 | M_6000);

   /* fD,fB --> D 0 B */

   FReg2Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*FReg2OrderCount); InstrZ=0;
   AddFReg2("FABS"  ,"FABS"  ,(T63 << 26)+(264 << 1), M_601 | M_6000,True );
   AddFReg2("FCTIW" ,"FCTIW" ,(T63 << 26)+( 14 << 1), M_601 | M_6000,True );
   AddFReg2("FCTIWZ","FCTIWZ",(T63 << 26)+( 15 << 1), M_601 | M_6000,True );
   AddFReg2("FMR"   ,"FMR"   ,(T63 << 26)+( 72 << 1), M_601 | M_6000,True );
   AddFReg2("FNABS" ,"FNABS" ,(T63 << 26)+(136 << 1), M_601 | M_6000,True );
   AddFReg2("FNEG"  ,"FNEG"  ,(T63 << 26)+( 40 << 1), M_601 | M_6000,True );
   AddFReg2("FRSP"  ,"FRSP"  ,(T63 << 26)+( 12 << 1), M_601 | M_6000,True );

   /* D,B --> D 0 B */

   Reg2BOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*Reg2BOrderCount); InstrZ=0;
   AddReg2B("MFSRIN","MFSRIN",(T31 << 26)+(659 << 1), M_601 | M_6000);
   AddReg2B("MTSRIN","MTSRI" ,(T31 << 26)+(242 << 1), M_601 | M_6000);

   /* A,S --> S A 0 */

   Reg2SwapOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*Reg2SwapOrderCount); InstrZ=0;
   AddReg2Swap("CNTLZW","CNTLZ" ,(T31 << 26)+( 26 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,True );
   AddReg2Swap("EXTSB ","EXTSB" ,(T31 << 26)+(954 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,True );
   AddReg2Swap("EXTSH ","EXTS"  ,(T31 << 26)+(922 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,True );

   /* A,B --> 0 A B */

   NoDestOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*NoDestOrderCount); InstrZ=0;
   AddNoDest("DCBF"  ,"DCBF"  ,(T31 << 26)+(  86 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddNoDest("DCBI"  ,"DCBI"  ,(T31 << 26)+( 470 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddNoDest("DCBST" ,"DCBST" ,(T31 << 26)+(  54 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddNoDest("DCBT"  ,"DCBT"  ,(T31 << 26)+( 278 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddNoDest("DCBTST","DCBTST",(T31 << 26)+( 246 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddNoDest("DCBZ"  ,"DCLZ"  ,(T31 << 26)+(1014 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddNoDest("DCCCI" ,"DCCCI" ,(T31 << 26)+( 454 << 1), M_403 | M_403C                         );
   AddNoDest("ICBI"  ,"ICBI"  ,(T31 << 26)+( 982 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddNoDest("ICBT"  ,"ICBT"  ,(T31 << 26)+( 262 << 1), M_403 | M_403C                         );
   AddNoDest("ICCCI" ,"ICCCI" ,(T31 << 26)+( 966 << 1), M_403 | M_403C                         );

   /* D,A,B --> D A B */

   Reg3Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*Reg3OrderCount); InstrZ=0;
   AddReg3("ADD"   ,"CAX"   ,(T31 << 26)+(266 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True, True );
   AddReg3("ADDC"  ,"A"     ,(T31 << 26)+( 10 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("ADDE"  ,"AE"    ,(T31 << 26)+(138 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("DIV"   ,"DIV"   ,(T31 << 26)+(331 << 1),                                  M_6000,True ,True );
   AddReg3("DIVS"  ,"DIVS"  ,(T31 << 26)+(363 << 1),                                  M_6000,True ,True );
   AddReg3("DIVW"  ,"DIVW"  ,(T31 << 26)+(491 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("DIVWU" ,"DIVWU" ,(T31 << 26)+(459 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("DOZ"   ,"DOZ"   ,(T31 << 26)+(264 << 1),                                  M_6000,True ,True );
   AddReg3("ECIWX" ,"ECIWX" ,(T31 << 26)+(310 << 1),                                  M_6000,False,False);
   AddReg3("LBZUX" ,"LBZUX" ,(T31 << 26)+(119 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LBZX"  ,"LBZX"  ,(T31 << 26)+( 87 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LHAUX" ,"LHAUX" ,(T31 << 26)+(375 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LHAX"  ,"LHAX"  ,(T31 << 26)+(343 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LHBRX" ,"LHBRX" ,(T31 << 26)+(790 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LHZUX" ,"LHZUX" ,(T31 << 26)+(311 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LHZX"  ,"LHZX"  ,(T31 << 26)+(279 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LSCBX" ,"LSCBX" ,(T31 << 26)+(277 << 1),                                  M_6000,False,True );
   AddReg3("LSWX"  ,"LSX"   ,(T31 << 26)+(533 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LWARX" ,"LWARX" ,(T31 << 26)+( 20 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LWBRX" ,"LBRX"  ,(T31 << 26)+(534 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LWZUX" ,"LUX"   ,(T31 << 26)+( 55 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("LWZX"  ,"LX"    ,(T31 << 26)+( 23 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("MUL"   ,"MUL"   ,(T31 << 26)+(107 << 1),                                  M_6000,True ,True );
   AddReg3("MULHW" ,"MULHW" ,(T31 << 26)+( 75 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,True );
   AddReg3("MULHWU","MULHWU",(T31 << 26)+( 11 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,True );
   AddReg3("MULLW" ,"MULS"  ,(T31 << 26)+(235 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("STBUX" ,"STBUX" ,(T31 << 26)+(247 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("STBX"  ,"STBX"  ,(T31 << 26)+(215 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("STHBRX","STHBRX",(T31 << 26)+(918 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("STHUX" ,"STHUX" ,(T31 << 26)+(439 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("STHX"  ,"STHX"  ,(T31 << 26)+(407 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("STSWX" ,"STSX"  ,(T31 << 26)+(661 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("STWBRX","STBRX" ,(T31 << 26)+(662 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("STWCX.","STWCX.",(T31 << 26)+(150 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("STWUX" ,"STUX"  ,(T31 << 26)+(183 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("STWX"  ,"STX"   ,(T31 << 26)+(151 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,False,False);
   AddReg3("SUBF"  ,"SUBF"  ,(T31 << 26)+( 40 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("SUB"   ,"SUB"   ,(T31 << 26)+( 40 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("SUBFC" ,"SF"    ,(T31 << 26)+(  8 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("SUBC"  ,"SUBC"  ,(T31 << 26)+(  8 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("SUBFE" ,"SFE"   ,(T31 << 26)+(136 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True ,True );
   AddReg3("TLBSX" ,"TLBSX" ,(T31 << 26)+(914 << 1),         M_403C                         ,False,True );

   /* cD,cA,cB --> D A B */

   CReg3Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*CReg3OrderCount); InstrZ=0;
   AddCReg3("CRAND"  ,(T19 << 26)+(257 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddCReg3("CRANDC" ,(T19 << 26)+(129 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddCReg3("CREQV"  ,(T19 << 26)+(289 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddCReg3("CRNAND" ,(T19 << 26)+(225 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddCReg3("CRNOR"  ,(T19 << 26)+( 33 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddCReg3("CROR"   ,(T19 << 26)+(449 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddCReg3("CRORC"  ,(T19 << 26)+(417 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddCReg3("CRXOR"  ,(T19 << 26)+(193 << 1), M_403 | M_403C | M_505 | M_601 | M_6000);

   /* fD,fA,fB --> D A B */

   FReg3Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*FReg3OrderCount); InstrZ=0;
   AddFReg3("FADD"  ,"FA"    ,(T63 << 26)+(21 << 1), M_601 | M_6000,True );
   AddFReg3("FADDS" ,"FADDS" ,(T59 << 26)+(21 << 1), M_601 | M_6000,True );
   AddFReg3("FDIV"  ,"FD"    ,(T63 << 26)+(18 << 1), M_601 | M_6000,True );
   AddFReg3("FDIVS" ,"FDIVS" ,(T59 << 26)+(18 << 1), M_601 | M_6000,True );
   AddFReg3("FSUB"  ,"FS"    ,(T63 << 26)+(20 << 1), M_601 | M_6000,True );

   /* A,S,B --> S A B */

   Reg3SwapOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*Reg3SwapOrderCount); InstrZ=0;
   AddReg3Swap("AND"   ,"AND"   ,(T31 << 26)+(  28 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("ANDC"  ,"ANDC"  ,(T31 << 26)+(  60 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("ECOWX" ,"ECOWX" ,(T31 << 26)+( 438 << 1),                          M_601 | M_6000,False);
   AddReg3Swap("EQV"   ,"EQV"   ,(T31 << 26)+( 284 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("MASKG" ,"MASKG" ,(T31 << 26)+(  29 << 1),                                  M_6000,True );
   AddReg3Swap("MASKIR","MASKIR",(T31 << 26)+( 541 << 1),                                  M_6000,True );
   AddReg3Swap("NAND"  ,"NAND"  ,(T31 << 26)+( 476 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("NOR"   ,"NOR"   ,(T31 << 26)+( 124 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("OR"    ,"OR"    ,(T31 << 26)+( 444 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("ORC"   ,"ORC"   ,(T31 << 26)+( 412 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("RRIB"  ,"RRIB"  ,(T31 << 26)+( 537 << 1),                                  M_6000,True );
   AddReg3Swap("SLE"   ,"SLE"   ,(T31 << 26)+( 153 << 1),                                  M_6000,True );
   AddReg3Swap("SLEQ"  ,"SLEQ"  ,(T31 << 26)+( 217 << 1),                                  M_6000,True );
   AddReg3Swap("SLLQ"  ,"SLLQ"  ,(T31 << 26)+( 216 << 1),                                  M_6000,True );
   AddReg3Swap("SLQ"   ,"SLQ"   ,(T31 << 26)+( 152 << 1),                                  M_6000,True );
   AddReg3Swap("SLW"   ,"SL"    ,(T31 << 26)+(  24 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("SRAQ"  ,"SRAQ"  ,(T31 << 26)+( 920 << 1),                                  M_6000,True );
   AddReg3Swap("SRAW"  ,"SRA"   ,(T31 << 26)+( 792 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("SRE"   ,"SRE"   ,(T31 << 26)+( 665 << 1),                                  M_6000,True );
   AddReg3Swap("SREA"  ,"SREA"  ,(T31 << 26)+( 921 << 1),                                  M_6000,True );
   AddReg3Swap("SREQ"  ,"SREQ"  ,(T31 << 26)+( 729 << 1),                                  M_6000,True );
   AddReg3Swap("SRLQ"  ,"SRLQ"  ,(T31 << 26)+( 728 << 1),                                  M_6000,True );
   AddReg3Swap("SRQ"   ,"SRQ"   ,(T31 << 26)+( 664 << 1),                                  M_6000,True );
   AddReg3Swap("SRW"   ,"SR"    ,(T31 << 26)+( 536 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );
   AddReg3Swap("XOR"   ,"XOR"   ,(T31 << 26)+( 316 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True );

   /* fD,A,B --> D A B */

   MixedOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*MixedOrderCount); InstrZ=0;
   AddMixed("LFDUX" ,"LFDUX" ,(T31 << 26)+(631 << 1), M_601 | M_6000);
   AddMixed("LFDX"  ,"LFDX"  ,(T31 << 26)+(599 << 1), M_601 | M_6000);
   AddMixed("LFSUX" ,"LFSUX" ,(T31 << 26)+(567 << 1), M_601 | M_6000);
   AddMixed("LFSX"  ,"LFSX"  ,(T31 << 26)+(535 << 1), M_601 | M_6000);
   AddMixed("STFDUX","STFDUX",(T31 << 26)+(759 << 1), M_601 | M_6000);
   AddMixed("STFDX" ,"STFDX" ,(T31 << 26)+(727 << 1), M_601 | M_6000);
   AddMixed("STFSUX","STFSUX",(T31 << 26)+(695 << 1), M_601 | M_6000);
   AddMixed("STFSX" ,"STFSX" ,(T31 << 26)+(663 << 1), M_601 | M_6000);

   /* fD,fA,fC,fB --> D A B C */

   FReg4Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*FReg4OrderCount); InstrZ=0;
   AddFReg4("FMADD"  ,"FMA"    ,(T63 << 26)+(29 << 1), M_601 | M_6000,True );
   AddFReg4("FMADDS" ,"FMADDS" ,(T59 << 26)+(29 << 1), M_601 | M_6000,True );
   AddFReg4("FMSUB"  ,"FMS"    ,(T63 << 26)+(28 << 1), M_601 | M_6000,True );
   AddFReg4("FMSUBS" ,"FMSUBS" ,(T59 << 26)+(28 << 1), M_601 | M_6000,True );
   AddFReg4("FNMADD" ,"FNMA"   ,(T63 << 26)+(31 << 1), M_601 | M_6000,True );
   AddFReg4("FNMADDS","FNMADDS",(T59 << 26)+(31 << 1), M_601 | M_6000,True );
   AddFReg4("FNMSUB" ,"FNMS"   ,(T63 << 26)+(30 << 1), M_601 | M_6000,True );
   AddFReg4("FNMSUBS","FNMSUBS",(T59 << 26)+(30 << 1), M_601 | M_6000,True );

   /* D,d(A) --> D A d */

   RegDispOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*RegDispOrderCount); InstrZ=0;
   AddRegDisp("LBZ"   ,"LBZ"   ,(T34 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("LBZU"  ,"LBZU"  ,(T35 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("LHA"   ,"LHA"   ,(T42 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("LHAU"  ,"LHAU"  ,(T43 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("LHZ"   ,"LHZ"   ,(T40 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("LHZU"  ,"LHZU"  ,(T41 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("LMW"   ,"LM"    ,(T46 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("LWZ"   ,"L"     ,(T32 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("LWZU"  ,"LU"    ,(T33 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("STB"   ,"STB"   ,(T38 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("STBU"  ,"STBU"  ,(T39 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("STH"   ,"STH"   ,(T44 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("STHU"  ,"STHU"  ,(T45 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("STMW"  ,"STM"   ,(T47 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("STW"   ,"ST"    ,(T36 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);
   AddRegDisp("STWU"  ,"STU"   ,(T37 << 26), M_403 | M_403C | M_505 | M_601 | M_6000);

   /* fD,d(A) --> D A d */

   FRegDispOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*FRegDispOrderCount); InstrZ=0;
   AddFRegDisp("LFD"   ,"LFD"   ,(T50 << 26), M_601 | M_6000);
   AddFRegDisp("LFDU"  ,"LFDU"  ,(T51 << 26), M_601 | M_6000);
   AddFRegDisp("LFS"   ,"LFS"   ,(T48 << 26), M_601 | M_6000);
   AddFRegDisp("LFSU"  ,"LFSU"  ,(T49 << 26), M_601 | M_6000);
   AddFRegDisp("STFD"  ,"STFD"  ,(T54 << 26), M_601 | M_6000);
   AddFRegDisp("STFDU" ,"STFDU" ,(T55 << 26), M_601 | M_6000);
   AddFRegDisp("STFS"  ,"STFS"  ,(T52 << 26), M_601 | M_6000);
   AddFRegDisp("STFSU" ,"STFSU" ,(T53 << 26), M_601 | M_6000);

   /* A,S,Imm5 --> S A Imm */

   Reg2ImmOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*Reg2ImmOrderCount); InstrZ=0;
   AddReg2Imm("SLIQ"  ,"SLIQ"  ,(T31 << 26)+(184 << 1),                                  M_6000,True);
   AddReg2Imm("SLLIQ" ,"SLLIQ" ,(T31 << 26)+(248 << 1),                                  M_6000,True);
   AddReg2Imm("SRAIQ" ,"SRAIQ" ,(T31 << 26)+(952 << 1),                                  M_6000,True);
   AddReg2Imm("SRAWI" ,"SRAI"  ,(T31 << 26)+(824 << 1), M_403 | M_403C | M_505 | M_601 | M_6000,True);
   AddReg2Imm("SRIQ"  ,"SRIQ"  ,(T31 << 26)+(696 << 1),                                  M_6000,True);
   AddReg2Imm("SRLIQ" ,"SRLIQ" ,(T31 << 26)+(760 << 1),                                  M_6000,True);

   /* D,A,Imm --> D A Imm */

   Imm16Orders=(BaseOrder *) malloc(sizeof(BaseOrder)*Imm16OrderCount); InstrZ=0;
   AddImm16("ADDI"   ,"CAL"    ,T14 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16("ADDIC"  ,"AI"     ,T12 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16("ADDIC." ,"AI."    ,T13 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16("ADDIS"  ,"CAU"    ,T15 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16("DOZI"   ,"DOZI"   , T9 << 26,                         M_6000);
   AddImm16("MULLI"  ,"MULI"   , T7 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16("SUBFIC" ,"SFI"    , T8 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);

   /* A,S,Imm --> S A Imm */

   Imm16SwapOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*Imm16SwapOrderCount); InstrZ=0;
   AddImm16Swap("ANDI."  ,"ANDIL." ,T28 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16Swap("ANDIS." ,"ANDIU." ,T29 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16Swap("ORI"    ,"ORIL"   ,T24 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16Swap("ORIS"   ,"ORIU"   ,T25 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16Swap("XORI"   ,"XORIL"  ,T26 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
   AddImm16Swap("XORIS"  ,"XORIU"  ,T27 << 26, M_403 | M_403C | M_505 | M_601 | M_6000);
END

        static void DeinitNames(BaseOrder *Orders, int OrderCount)
BEGIN
   int z;

   for (z=0; z<OrderCount; free(Orders[z++].Name));
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(Reg1Orders);
   free(FReg1Orders);
   free(CReg1Orders);
   free(CBit1Orders);
   DeinitNames(Reg2Orders,Reg2OrderCount); free(Reg2Orders);
   free(CReg2Orders);
   DeinitNames(FReg2Orders,FReg2OrderCount); free(FReg2Orders);
   free(Reg2BOrders);
   DeinitNames(Reg2SwapOrders,Reg2SwapOrderCount); free(Reg2SwapOrders);
   free(NoDestOrders);
   DeinitNames(Reg3Orders,Reg3OrderCount); free(Reg3Orders);
   free(CReg3Orders);
   DeinitNames(FReg3Orders,FReg3OrderCount); free(FReg3Orders);
   DeinitNames(Reg3SwapOrders,Reg3SwapOrderCount); free(Reg3SwapOrders);
   free(MixedOrders);
   DeinitNames(FReg4Orders,FReg4OrderCount); free(FReg4Orders);
   free(RegDispOrders);
   free(FRegDispOrders);
   DeinitNames(Reg2ImmOrders,Reg2ImmOrderCount); free(Reg2ImmOrders);
   free(Imm16Orders);
   free(Imm16SwapOrders);
END

/*-------------------------------------------------------------------------*/

        static void PutCode(LongWord Code)
BEGIN
#if 0
   memcpy(BAsmCode,&Code,4);
   if (NOT BigEndian) DSwap((void *)BAsmCode,4);
#endif
   DAsmCode[0]=Code;
END

        static void IncCode(LongWord Code)
BEGIN
#if 0
   BAsmCode[0]+=(Code >> 24) & 0xff;
   BAsmCode[1]+=(Code >> 16) & 0xff;
   BAsmCode[2]+=(Code >>  8) & 0xff;
   BAsmCode[3]+=(Code      ) & 0xff;
#endif
   DAsmCode[0]+=Code;
END

/*-------------------------------------------------------------------------*/

        static Boolean DecodeGenReg(char *Asc, LongWord *Erg)
BEGIN
   Boolean io;
   char *s;

   if (FindRegDef(Asc, &s)) Asc = s;

   if ((strlen(Asc) < 2) OR (toupper(*Asc) != 'R')) return False;
   else
    BEGIN
     *Erg = ConstLongInt(Asc+1,&io);
     return ((io) AND (*Erg <= 31));
    END
END

        static Boolean DecodeFPReg(char *Asc, LongWord *Erg)
BEGIN
   Boolean io;
   char *s;

   if (FindRegDef(Asc, &s)) Asc = s;

   if ((strlen(Asc)<3) OR (toupper(*Asc)!='F') OR (toupper(Asc[1])!='R')) return False;
   else
    BEGIN
     *Erg=ConstLongInt(Asc+2,&io);
     return ((io) AND (*Erg<=31));
    END
END

        static Boolean DecodeCondReg(char *Asc, LongWord *Erg)
BEGIN
   Boolean OK;

   *Erg=EvalIntExpression(Asc,UInt3,&OK) << 2;
   return ((OK) AND (*Erg<=31));
END

        static Boolean  DecodeCondBit(char *Asc, LongWord *Erg)
BEGIN
   Boolean OK;

   *Erg=EvalIntExpression(Asc,UInt5,&OK);
   return ((OK) AND (*Erg<=31));
END

        static Boolean DecodeRegDisp(char *Asc, LongWord *Erg)
BEGIN
   char *p;
   int l=strlen(Asc);
   LongInt Disp;
   Boolean OK;

   if (Asc[l-1]!=')') return False; Asc[l-1]='\0';  l--;
   p=Asc+l-1;  while ((p>=Asc) AND (*p!='(')) p--;
   if (p<Asc) return False;
   if (NOT DecodeGenReg(p+1,Erg)) return False;
   *p='\0';
   Disp=EvalIntExpression(Asc,Int16,&OK);  if (NOT OK) return False;
   *Erg=(*Erg << 16)+(Disp & 0xffff);  return True;
END

/*-------------------------------------------------------------------------*/

        static Boolean Convert6000(char *Name1, char *Name2)
BEGIN
   if (NOT Memo(Name1)) return True;
   if (MomCPU==CPU6000)
    BEGIN
     strmaxcpy(OpPart,Name2,255);
     return True;
    END
   else
    BEGIN
     WrError(1200); return False;
    END
END

        static Boolean PMemo(char *Name)
BEGIN
   String tmp;

   if (Memo(Name)) return True;   

   strmaxcpy(tmp,Name,255); strmaxcat(tmp,".",255);
   return (Memo(tmp));
END

        static void IncPoint(void)
BEGIN
   if (OpPart[strlen(OpPart)-1]=='.') IncCode(1);
END

        static void ChkSup(void)
BEGIN
   if (NOT SupAllowed) WrError(50);
END

        static Boolean ChkCPU(Byte Mask)
BEGIN
   return (((Mask >> (MomCPU-CPU403))&1)==1);
END

/*-------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
   if (Memo("REG"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else AddRegDef(LabPart,ArgStr[1]);
     return True;
    END

   return False;
END

        static void SwapCode(LongWord *Code)
BEGIN
   *Code=((*Code & 0x1f) << 5) | ((*Code >> 5) & 0x1f);
END

        static void MakeCode_601(void)
BEGIN
   int z;
   Integer Imm;
   LongWord Dest,Src1,Src2,Src3;
   LongInt Dist;
   Boolean OK;

   CodeLen=0; DontPrint=False;

   /* Nullanweisung */

   if ((Memo("")) AND (*AttrPart=='\0') AND (ArgCnt==0)) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(BigEnd)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (NOT ChkCPU(FixedOrders[z].CPUMask)) WrXError(1500,OpPart);
      else
       BEGIN
        CodeLen=4; PutCode(FixedOrders[z].Code);
        if (Memo("RFI")) ChkSup();
       END
      return;
     END

   /* ein Register */

   for (z=0; z<Reg1OrderCount; z++)
    if Memo(Reg1Orders[z].Name)
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (NOT ChkCPU(Reg1Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(Reg1Orders[z].Code+(Dest << 21));
        if (Memo("MTMSR")) ChkSup();
       END
      return;
     END

   /* ein Steuerregister */

   for (z=1; z<CReg1OrderCount; z++)
    if (Memo(CReg1Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (NOT ChkCPU(CReg1Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeCondReg(ArgStr[1],&Dest)) WrError(1350);
      else if ((Dest & 3)!=0) WrError(1351);
      else
       BEGIN
        CodeLen=4; PutCode(CReg1Orders[z].Code+(Dest << 21));
       END
      return;
     END

   /* ein Steuerregisterbit */

   for (z=0; z<CBit1OrderCount; z++)
    if (Memo(CBit1Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (NOT ChkCPU(CBit1Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeCondBit(ArgStr[1],&Dest)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(CBit1Orders[z].Code+(Dest << 21));
       END
      return;
     END

   /* ein Gleitkommaregister */

   for (z=0; z<FReg1OrderCount; z++)
    if (Memo(FReg1Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (NOT ChkCPU(FReg1Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeFPReg(ArgStr[1],&Dest)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(FReg1Orders[z].Code+(Dest << 21));
       END
      return;
     END

   /* 1/2 Integer-Register */

   for (z=0; z<Reg2OrderCount; z++)
    if (Memo(Reg2Orders[z].Name))
     BEGIN
      if (ArgCnt==1)
       BEGIN
        ArgCnt=2; strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=2) WrError(1110);
      else if (NOT ChkCPU(Reg2Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(Reg2Orders[z].Code+(Dest << 21)+(Src1 << 16));
       END
      return;
     END

   /* 2 Bedingungs-Bits */

   for (z=0; z<CReg2OrderCount; z++)
    if Memo(CReg2Orders[z].Name)
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (NOT ChkCPU(CReg2Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeCondReg(ArgStr[1],&Dest)) WrError(1350);
      else if ((Dest & 3)!=0) WrError(1351);
      else if (NOT DecodeCondReg(ArgStr[2],&Src1)) WrError(1350);
      else if ((Src1 & 3)!=0) WrError(1351);
      else
       BEGIN
        CodeLen=4; PutCode(CReg2Orders[z].Code+(Dest << 21)+(Src1 << 16));
       END
      return;
     END

   /* 1/2 Float-Register */

   for (z=0; z<FReg2OrderCount; z++)
    if Memo(FReg2Orders[z].Name)
     BEGIN
      if (ArgCnt==1)
       BEGIN
        ArgCnt=2; strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=2) WrError(1110);
      else if (NOT ChkCPU(FReg2Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeFPReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeFPReg(ArgStr[2],&Src1)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(FReg2Orders[z].Code+(Dest << 21)+(Src1 << 11));
       END
      return;
     END

   /* 1/2 Integer-Register, Quelle in B */

   for (z=0; z<Reg2BOrderCount; z++)
    if Memo(Reg2BOrders[z].Name)
     BEGIN
      if (ArgCnt==1)
       BEGIN
        ArgCnt=2; strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=2) WrError(1110);
      else if (NOT ChkCPU(Reg2BOrders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(Reg2BOrders[z].Code+(Dest << 21)+(Src1 << 11));
        ChkSup();
       END
      return;
     END

   /* 1/2 Integer-Register, getauscht */

   for (z=0; z<Reg2SwapOrderCount; z++)
    if (Memo(Reg2SwapOrders[z].Name))
     BEGIN
      if (ArgCnt==1)
       BEGIN
        ArgCnt=2; strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=2) WrError(1110);
      else if (NOT ChkCPU(Reg2SwapOrders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(Reg2SwapOrders[z].Code+(Dest << 16)+(Src1 << 21));
       END
      return;
     END

   /* 2 Integer-Register, kein Ziel */

   for (z=0; z<NoDestOrderCount; z++)
    if (Memo(NoDestOrders[z].Name))
      BEGIN
       if (ArgCnt!=2) WrError(1110);
       else if (NOT ChkCPU(NoDestOrders[z].CPUMask)) WrXError(1500,OpPart);
       else if (NOT DecodeGenReg(ArgStr[1],&Src1)) WrError(1350);
       else if (NOT DecodeGenReg(ArgStr[2],&Src2)) WrError(1350);
       else
        BEGIN
         CodeLen=4; PutCode(NoDestOrders[z].Code+(Src1 << 16)+(Src2 << 11));
        END
       return;
      END

   /* 2/3 Integer-Register */

   for (z=0; z<Reg3OrderCount; z++)
    if (Memo(Reg3Orders[z].Name))
     BEGIN
      if (ArgCnt==2)
       BEGIN
        ArgCnt=3; strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=3) WrError(1110);
      else if (NOT ChkCPU(Reg3Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[3],&Src2)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(Reg3Orders[z].Code+(Dest << 21)+(Src1 << 16)+(Src2 << 11));
       END
      return;
     END

   /* 2/3 Bedingungs-Bits */

   for (z=0; z<CReg3OrderCount; z++)
    if (Memo(CReg3Orders[z].Name))
     BEGIN
      if (ArgCnt==2)
       BEGIN
        ArgCnt=3; strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=3) WrError(1110);
      else if (NOT ChkCPU(CReg3Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeCondBit(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeCondBit(ArgStr[2],&Src1)) WrError(1350);
      else if (NOT DecodeCondBit(ArgStr[3],&Src2)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(CReg3Orders[z].Code+(Dest << 21)+(Src1 << 16)+(Src2 << 11));
       END
      return;
     END

   /* 2/3 Float-Register */

   for (z=0; z<FReg3OrderCount; z++)
    if (Memo(FReg3Orders[z].Name))
     BEGIN
      if (ArgCnt==2)
       BEGIN
        ArgCnt=3; strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=3) WrError(1110);
      else if (NOT ChkCPU(FReg3Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeFPReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeFPReg(ArgStr[2],&Src1)) WrError(1350);
      else if (NOT DecodeFPReg(ArgStr[3],&Src2)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(FReg3Orders[z].Code+(Dest << 21)+(Src1 << 16)+(Src2 << 11));
       END
      return;
     END

   /* 2/3 Integer-Register, Ziel & Quelle 1 getauscht */

   for (z=0; z<Reg3SwapOrderCount; z++)
    if (Memo(Reg3SwapOrders[z].Name))
     BEGIN
      if (ArgCnt==2)
       BEGIN
        ArgCnt=3; strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=3) WrError(1110);
      else if (NOT ChkCPU(Reg3SwapOrders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[3],&Src2)) WrError(1350);
      else
       BEGIN
        CodeLen=4; PutCode(Reg3SwapOrders[z].Code+(Dest << 16)+(Src1 << 21)+(Src2 << 11));
       END
      return;
     END

   /* 1 Float und 2 Integer-Register */

   for (z=0; z<MixedOrderCount; z++)
    if (Memo(MixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=3) WrError(1110);
      else if (NOT ChkCPU(MixedOrders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeFPReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[3],&Src2)) WrError(1350);
      else
        BEGIN
         CodeLen=4; PutCode(MixedOrders[z].Code+(Dest << 21)+(Src1 << 16)+(Src2 << 11));
        END
       return;
      END

   /* 3/4 Float-Register */

   for (z=0; z<FReg4OrderCount; z++)
    if (Memo(FReg4Orders[z].Name))
     BEGIN
      if (ArgCnt==3)
       BEGIN
        ArgCnt=4; strcpy(ArgStr[4],ArgStr[3]);
        strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=4) WrError(1110);
      else if (NOT ChkCPU(FReg4Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeFPReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeFPReg(ArgStr[2],&Src1)) WrError(1350);
      else if (NOT DecodeFPReg(ArgStr[3],&Src3)) WrError(1350);
      else if (NOT DecodeFPReg(ArgStr[4],&Src2)) WrError(1350);
      else
       BEGIN
        CodeLen=4;
        PutCode(FReg4Orders[z].Code+(Dest << 21)+(Src1 << 16)+(Src2 << 11)+(Src3 << 6));
       END
      return;
     END

   /* Register mit indiziertem Speicheroperandem */

   for (z=0; z<RegDispOrderCount; z++)
    if (Memo(RegDispOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeRegDisp(ArgStr[2],&Src1)) WrError(1350);
      else
       BEGIN
        PutCode(RegDispOrders[z].Code+(Dest << 21)+Src1); CodeLen=4;
       END
      return;
     END

   /* Gleitkommaregister mit indiziertem Speicheroperandem */

   for (z=0; z<FRegDispOrderCount; z++)
    if (Memo(FRegDispOrders[z].Name))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (NOT DecodeFPReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeRegDisp(ArgStr[2],&Src1)) WrError(1350);
      else
       BEGIN
        PutCode(FRegDispOrders[z].Code+(Dest << 21)+Src1); CodeLen=4;
       END
      return;
     END

   /* 2 verdrehte Register mit immediate */

   for (z=0; z<Reg2ImmOrderCount; z++)
    if (Memo(Reg2ImmOrders[z].Name))
     BEGIN
      if (ArgCnt!=3) WrError(1110);
      else if (NOT ChkCPU(Reg2ImmOrders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
      else
       BEGIN
        Src2=EvalIntExpression(ArgStr[3],UInt5,&OK);
        if (OK)
         BEGIN
          PutCode(Reg2ImmOrders[z].Code+(Src1 << 21)+(Dest << 16)+(Src2 << 11));
          CodeLen=4;
         END
       END
      return;
     END

   /* 2 Register+immediate */

   for (z=0; z<Imm16OrderCount; z++)
    if (Memo(Imm16Orders[z].Name))
     BEGIN
      if (ArgCnt==2)
       BEGIN
        ArgCnt=3; strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=3) WrError(1110);
      else if (NOT ChkCPU(Imm16Orders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
      else
       BEGIN
        Imm=EvalIntExpression(ArgStr[3],Int16,&OK);
        if (OK)
         BEGIN
          CodeLen=4; PutCode(Imm16Orders[z].Code+(Dest << 21)+(Src1 << 16)+(Imm & 0xffff));
         END
       END
      return;
     END

   /* 2 Register+immediate, Ziel & Quelle 1 getauscht */

   for (z=0; z<Imm16SwapOrderCount; z++)
    if (Memo(Imm16SwapOrders[z].Name))
     BEGIN
      if (ArgCnt==2)
       BEGIN
        ArgCnt=3; strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]);
       END
      if (ArgCnt!=3) WrError(1110);
      else if (NOT ChkCPU(Imm16SwapOrders[z].CPUMask)) WrXError(1500,OpPart);
      else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
      else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
      else
       BEGIN
        Imm=EvalIntExpression(ArgStr[3],Int16,&OK);
        if (OK)
         BEGIN
          CodeLen=4; PutCode(Imm16SwapOrders[z].Code+(Dest << 16)+(Src1 << 21)+(Imm & 0xffff));
         END
       END
      return;
     END

   /* Ausreisser... */

   if (NOT Convert6000("FM","FMUL")) return;
   if (NOT Convert6000("FM.","FMUL.")) return;

   if ((PMemo("FMUL")) OR (PMemo("FMULS")))
    BEGIN
     if (ArgCnt==2)
      BEGIN
       strcpy(ArgStr[3],ArgStr[2]); strcpy(ArgStr[2],ArgStr[1]); ArgCnt=3;
      END
     if (ArgCnt!=3) WrError(1110);
     else if (NOT DecodeFPReg(ArgStr[1],&Dest)) WrError(1350);
     else if (NOT DecodeFPReg(ArgStr[2],&Src1)) WrError(1350);
     else if (NOT DecodeFPReg(ArgStr[3],&Src2)) WrError(1350);
     else
      BEGIN
       PutCode((T59 << 26)+(25 << 1)+(Dest << 21)+(Src1 << 16)+(Src2 << 6));
       if (PMemo("FMUL")) IncCode(T4 << 26);
       IncPoint();
       CodeLen=4;
      END
     return;
    END

   if (NOT Convert6000("LSI","LSWI")) return;
   if (NOT Convert6000("STSI","STSWI")) return;

   if ((Memo("LSWI")) OR (Memo("STSWI")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
     else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
     else
      BEGIN
       Src2=EvalIntExpression(ArgStr[3],UInt5,&OK);
       if (OK)
        BEGIN
         PutCode((T31 << 26)+(597 << 1)+(Dest << 21)+(Src1 << 16)+(Src2 << 11));
         if (Memo("STSWI")) IncCode(128 << 1);
         CodeLen=4;
        END
      END
     return;
    END

   if ((Memo("MFSPR")) OR (Memo("MTSPR")))
    BEGIN
     if (Memo("MTSPR"))
      BEGIN
       strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
      END
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
     else
      BEGIN
       Src1=EvalIntExpression(ArgStr[2],UInt10,&OK);
       if (OK)
        BEGIN
         SwapCode(&Src1);
         PutCode((T31 << 26)+(Dest << 21)+(Src1 << 11));
         IncCode((Memo("MFSPR") ? 339 : 467) << 1);
         CodeLen=4;
        END
      END
     return;
    END

   if ((Memo("MFDCR")) OR (Memo("MTDCR")))
    BEGIN
     if (Memo("MTDCR"))
      BEGIN
       strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
      END
     if (ArgCnt!=2) WrError(1110);
     else if (MomCPU>=CPU505) WrXError(1500,OpPart);
     else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
     else
      BEGIN
       Src1=EvalIntExpression(ArgStr[2],UInt10,&OK);
       if (OK)
        BEGIN
         SwapCode(&Src1);
         PutCode((T31 << 26)+(Dest << 21)+(Src1 << 11));
         IncCode((Memo("MFDCR") ? 323 : 451) << 1);
         CodeLen=4;
        END
      END
     return;
    END

   if ((Memo("MFSR")) OR (Memo("MTSR")))
    BEGIN
     if (Memo("MTSR"))
      BEGIN
       strcpy(ArgStr[3],ArgStr[1]); strcpy(ArgStr[1],ArgStr[2]); strcpy(ArgStr[2],ArgStr[3]);
      END
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
     else
      BEGIN
       Src1=EvalIntExpression(ArgStr[2],UInt4,&OK);
       if (OK)
        BEGIN
         PutCode((T31 << 26)+(Dest << 21)+(Src1 << 16));
         IncCode((Memo("MFSR") ? 595 : 210) << 1);
         CodeLen=4; ChkSup();
        END
      END
     return;
    END

   if (Memo("MTCRF"))
    BEGIN
     if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[ArgCnt],&Src1)) WrError(1350);
     else
      BEGIN
       OK=True;
       if (ArgCnt==1) Dest=0xff;
       else Dest=EvalIntExpression(ArgStr[1],UInt8,&OK);
       if (OK)
        BEGIN
         PutCode((T31 << 26)+(Src1 << 21)+(Dest << 12)+(144 << 1));
         CodeLen=4;
        END
      END
     return;
    END

   if (PMemo("MTFSF"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeFPReg(ArgStr[2],&Src1)) WrError(1350);
     else
      BEGIN
       Dest=EvalIntExpression(ArgStr[1],UInt8,&OK);
       if (OK)
        BEGIN
         PutCode((T63 << 26)+(Dest << 17)+(Src1 << 11)+(711 << 1));
         IncPoint();
         CodeLen=4;
        END
      END
     return;
    END

   if (PMemo("MTFSFI"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeCondReg(ArgStr[1],&Dest)) WrError(1350);
     else if ((Dest & 3)!=0) WrError(1351);
     else
      BEGIN
       Src1=EvalIntExpression(ArgStr[2],UInt4,&OK);
       if (OK)
        BEGIN
         PutCode((T63 << 26)+(Dest << 21)+(Src1 << 12)+(134 << 1));
         IncPoint();
         CodeLen=4;
        END
      END
     return;
    END

   if (PMemo("RLMI"))
    BEGIN
     if (ArgCnt!=5) WrError(1110);
     else if (MomCPU<CPU6000) WrXError(1500,OpPart);
     else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
     else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
     else if (NOT DecodeGenReg(ArgStr[3],&Src2)) WrError(1350);
     else
      BEGIN
       Src3=EvalIntExpression(ArgStr[4],UInt5,&OK);
       if (OK)
        BEGIN
         Imm=EvalIntExpression(ArgStr[5],UInt5,&OK);
         if (OK)
          BEGIN
           PutCode((T22 << 26)+(Src1 << 21)+(Dest << 16)
                       +(Src2 << 11)+(Src3 << 6)+(Imm << 1));
           IncPoint();
           CodeLen=4;
          END
        END
      END
     return;
    END

   if (NOT Convert6000("RLNM","RLWNM")) return;
   if (NOT Convert6000("RLNM.","RLWNM.")) return;

   if (PMemo("RLWNM"))
    BEGIN
     if (ArgCnt!=5) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
     else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
     else if (NOT DecodeGenReg(ArgStr[3],&Src2)) WrError(1350);
     else
      BEGIN
       Src3=EvalIntExpression(ArgStr[4],UInt5,&OK);
       if (OK)
        BEGIN
         Imm=EvalIntExpression(ArgStr[5],UInt5,&OK);
         if (OK)
          BEGIN
           PutCode((T23 << 26)+(Src1 << 21)+(Dest << 16)
                       +(Src2 << 11)+(Src3 << 6)+(Imm << 1));
           IncPoint();
           CodeLen=4;
          END
        END
      END
     return;
    END

   if (NOT Convert6000("RLIMI","RLWIMI")) return;
   if (NOT Convert6000("RLIMI.","RLWIMI.")) return;
   if (NOT Convert6000("RLINM","RLWINM")) return;
   if (NOT Convert6000("RLINM.","RLWINM.")) return;

   if ((PMemo("RLWIMI")) OR (PMemo("RLWINM")))
    BEGIN
     if (ArgCnt!=5) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[1],&Dest)) WrError(1350);
     else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
     else
      BEGIN
       Src2=EvalIntExpression(ArgStr[3],UInt5,&OK);
       if (OK)
        BEGIN
         Src3=EvalIntExpression(ArgStr[4],UInt5,&OK);
         if (OK)
          BEGIN
           Imm=EvalIntExpression(ArgStr[5],UInt5,&OK);
           if (OK)
            BEGIN
             PutCode((T20 << 26)+(Dest << 16)+(Src1 << 21)
                         +(Src2 << 11)+(Src3 << 6)+(Imm << 1));
             if (PMemo("RLWINM")) IncCode(T1 << 26);
             IncPoint();
             CodeLen=4;
            END
          END
        END
      END
     return;
    END

   if (NOT Convert6000("TLBI","TLBIE")) return;

   if (Memo("TLBIE"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[1],&Src1)) WrError(1350);
     else
      BEGIN
       PutCode((T31 << 26)+(Src1 << 11)+(306 << 1));
       CodeLen=4; ChkSup();
      END
     return;
    END

   if (NOT Convert6000("T","TW")) return;

   if (Memo("TW"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
     else if (NOT DecodeGenReg(ArgStr[3],&Src2)) WrError(1350);
     else
      BEGIN
       Dest=EvalIntExpression(ArgStr[1],UInt5,&OK);
       if (OK)
        BEGIN
         PutCode((T31 << 26)+(Dest << 21)+(Src1 << 16)+(Src2 << 11)+(4 << 1));
         CodeLen=4;
        END
      END
     return;
    END

   if (NOT Convert6000("TI","TWI")) return;

   if (Memo("TWI"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[2],&Src1)) WrError(1350);
     else
      BEGIN
       Imm=EvalIntExpression(ArgStr[3],Int16,&OK);
       if (OK)
        BEGIN
         Dest=EvalIntExpression(ArgStr[1],UInt5,&OK);
         if (OK)
          BEGIN
           PutCode((T3 << 26)+(Dest << 21)+(Src1 << 16)+(Imm & 0xffff));
           CodeLen=4;
          END
        END
      END
     return;
    END

   if (Memo("WRTEEI"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU>=CPU505) WrXError(1500,OpPart);
     else
      BEGIN
       Src1=EvalIntExpression(ArgStr[1],UInt1,&OK) << 15;
       if (OK)
        BEGIN
         PutCode((T31 << 26)+Src1+(163 << 1));
         CodeLen=4;
        END
      END
     return;
    END

   /* Vergleiche */

   if ((Memo("CMP")) OR (Memo("CMPL")))
    BEGIN
     if (ArgCnt==3)
      BEGIN
       strcpy(ArgStr[4],ArgStr[3]); strcpy(ArgStr[3],ArgStr[2]); strmaxcpy(ArgStr[2],"0",255); ArgCnt=4;
      END
     if (ArgCnt!=4) WrError(1110);
     else if (NOT DecodeGenReg(ArgStr[4],&Src2)) WrError(1350);
     else if (NOT DecodeGenReg(ArgStr[3],&Src1)) WrError(1350);
     else if (NOT DecodeCondReg(ArgStr[1],&Dest)) WrError(1350);
     else if ((Dest & 3)!=0) WrError(1351);
     else
      BEGIN
       Src3=EvalIntExpression(ArgStr[2],UInt1,&OK);
       if (OK)
        BEGIN
         PutCode((T31 << 26)+(Dest << 21)+(Src3 << 21)+(Src1 << 16)
                     +(Src2 << 11));
         if (Memo("CMPL")) IncCode(32 << 1);
         CodeLen=4;
        END
      END
     return;
    END

   if ((Memo("FCMPO")) OR (Memo("FCMPU")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (NOT DecodeFPReg(ArgStr[3],&Src2)) WrError(1350);
     else if (NOT DecodeFPReg(ArgStr[2],&Src1)) WrError(1350);
     else if (NOT DecodeCondReg(ArgStr[1],&Dest)) WrError(1350);
     else if ((Dest & 3)!=0) WrError(1351);
     else
      BEGIN
       PutCode((T63 << 26)+(Dest << 21)+(Src1 << 16)+(Src2 << 11));
       if (Memo("FCMPO")) IncCode(32 << 1);
       CodeLen=4;
      END
     return;
    END

   if ((Memo("CMPI")) OR (Memo("CMPLI")))
    BEGIN
     if (ArgCnt==3)
      BEGIN
       strcpy(ArgStr[4],ArgStr[3]); strcpy(ArgStr[3],ArgStr[2]); strmaxcpy(ArgStr[2],"0",255); ArgCnt=4;
      END
     if (ArgCnt!=4) WrError(1110);
     else
      BEGIN
       Src2=EvalIntExpression(ArgStr[4],Int16,&OK);
       if (OK)
        BEGIN
         if (NOT DecodeGenReg(ArgStr[3],&Src1)) WrError(1350);
         else if (NOT DecodeCondReg(ArgStr[1],&Dest)) WrError(1350);
         else if ((Dest & 3)!=0) WrError(1351);
         else
          BEGIN
           Src3=EvalIntExpression(ArgStr[2],UInt1,&OK);
           if (OK)
            BEGIN
             PutCode((T10 << 26)+(Dest << 21)+(Src3 << 21)
                         +(Src1 << 16)+(Src2 & 0xffff));
             if (Memo("CMPI")) IncCode(T1 << 26);
             CodeLen=4;
            END
          END
        END
      END
     return;
    END

   /* Spruenge */

   if ((Memo("B")) OR (Memo("BL")) OR (Memo("BA")) OR (Memo("BLA")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       Dist=EvalIntExpression(ArgStr[1],Int32,&OK);
       if (OK)
        BEGIN
         if ((Memo("B")) OR (Memo("BL"))) Dist-=EProgCounter();
         if ((NOT SymbolQuestionable) AND (Dist>0x1ffffff)) WrError(1320);
         else if ((NOT SymbolQuestionable) AND (Dist<-0x2000000l)) WrError(1315);
         else if ((Dist & 3)!=0) WrError(1375);
         else
          BEGIN
           PutCode((T18 << 26)+(Dist & 0x03fffffc));
           if ((Memo("BA")) OR (Memo("BLA"))) IncCode(2);
           if ((Memo("BL")) OR (Memo("BLA"))) IncCode(1);
           CodeLen=4;
          END
        END
      END
     return;
    END

   if ((Memo("BC")) OR (Memo("BCL")) OR (Memo("BCA")) OR (Memo("BCLA")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       Src1=EvalIntExpression(ArgStr[1],UInt5,&OK); /* BO */
       if (OK)
        BEGIN
         Src2=EvalIntExpression(ArgStr[2],UInt5,&OK); /* BI */
         if (OK)
          BEGIN
           Dist=EvalIntExpression(ArgStr[3],Int32,&OK); /* ADR */
           if (OK)
            BEGIN
             if ((Memo("BC")) OR (Memo("BCL"))) Dist-=EProgCounter();
             if ((NOT SymbolQuestionable) AND (Dist>0x7fff)) WrError(1320);
             else if ((NOT SymbolQuestionable) AND (Dist<-0x8000l)) WrError(1315);
             else if ((Dist & 3)!=0) WrError(1375);
             else
              BEGIN
               PutCode((T16 << 26)+(Src1 << 21)+(Src2 << 16)+(Dist & 0xfffc));
               if ((Memo("BCA")) OR (Memo("BCLA"))) IncCode(2);
               if ((Memo("BCL")) OR (Memo("BCLA"))) IncCode(1);
               CodeLen=4;
              END
            END
          END
        END
      END
     return;
    END

   if (NOT Convert6000("BCC","BCCTR")) return;
   if (NOT Convert6000("BCCL","BCCTRL")) return;
   if (NOT Convert6000("BCR","BCLR")) return;
   if (NOT Convert6000("BCRL","BCLRL")) return;

   if ((Memo("BCCTR")) OR (Memo("BCCTRL")) OR (Memo("BCLR")) OR (Memo("BCLRL")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       Src1=EvalIntExpression(ArgStr[1],UInt5,&OK);
       if (OK)
        BEGIN
         Src2=EvalIntExpression(ArgStr[2],UInt5,&OK);
         if (OK)
          BEGIN
           PutCode((T19 << 26)+(Src1 << 21)+(Src2 << 16));
           if ((Memo("BCCTR")) OR (Memo("BCCTRL")))
            IncCode(528 << 1);
           else
            IncCode(16 << 1);
           if ((Memo("BCCTRL")) OR (Memo("BCLRL"))) IncCode(1);
           CodeLen=4;
          END
        END
      END
     return;
    END

   if ((Memo("TLBRE")) OR (Memo("TLBWE")))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (MomCPU != CPU403C) WrError(1500);
     else if (NOT DecodeGenReg(ArgStr[1], &Src1)) WrError(1350);
     else if (NOT DecodeGenReg(ArgStr[2], &Src2)) WrError(1350);
     else
      BEGIN
       Src3 = EvalIntExpression(ArgStr[3], UInt1, &OK);
       if (OK)
        BEGIN
         PutCode((T31 << 26) + (Src1 << 21) + (Src2 << 16) +
                 (Src3 << 11) + (946 << 1));
         if (Memo("TLBWE")) IncCode(32 << 1);
         CodeLen = 4;
        END
      END 
     return;
    END

   /* unbekannter Befehl */

   WrXError(1200,OpPart);
END

        static Boolean IsDef_601(void)
BEGIN
   return Memo("REG");
END

        static void InitPass_601(void)
BEGIN
   SaveInitProc();
   SetFlag(&BigEnd,BigEndianName,False);
END

        static void InternSymbol_601(char *Asc, TempResult *Erg)
BEGIN
   int l=strlen(Asc);

   Erg->Typ=TempNone;
   if ((l==3) OR (l==4))
    if ((toupper(*Asc)=='C') AND (toupper(Asc[1])=='R'))
     if ((Asc[l-1]>='0') AND (Asc[l-1]<='7'))
      if ((l==3) != ((toupper(Asc[2])=='F') OR (toupper(Asc[3])=='B')))
       BEGIN
        Erg->Typ=TempInt; Erg->Contents.Int=Asc[l-1]-'0';
       END
END

        static void SwitchFrom_601(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

        static void SwitchTo_601(void)
BEGIN
   PFamilyDescr FoundDscr;

   TurnWords=True; ConstMode=ConstModeC; SetIsOccupied=False;

   FoundDscr=FindFamilyByName("MPC601"); if (FoundDscr==Nil) exit(255);

   PCSymbol="*"; HeaderID=FoundDscr->Id; NOPCode=0x000000000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=4; SegInits[SegCode]=0;
#ifdef __STDC__
   SegLimits[SegCode] = 0xfffffffful;
#else
   SegLimits[SegCode] = 0xffffffffl;
#endif

   MakeCode=MakeCode_601; IsDef=IsDef_601;
   SwitchFrom=SwitchFrom_601; InternSymbol=InternSymbol_601;
   AddONOFF("SUPMODE",   &SupAllowed, SupAllowedName,False);
   AddONOFF("BIGENDIAN", &BigEnd,     BigEndianName, False);

   InitFields();
END


        void code601_init(void)
BEGIN
   CPU403  =AddCPU("PPC403", SwitchTo_601);
   CPU403C =AddCPU("PPC403GC", SwitchTo_601);
   CPU505  =AddCPU("MPC505", SwitchTo_601);
   CPU601  =AddCPU("MPC601", SwitchTo_601);
   CPU6000 =AddCPU("RS6000", SwitchTo_601);

   SaveInitProc=InitPassProc; InitPassProc=InitPass_601;
END

