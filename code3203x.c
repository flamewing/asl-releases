/* code3203x.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator TMS320C3x-Familie                                           */               
/*                                                                           */
/* Historie: 12.12.1996 Grundsteinlegung                                     */               
/*            7. 7.1998 Fix Zugriffe auf CharTransTable wg. signed chars     */
/*           18. 8.1998 BookKeeping-Aufruf in RES                            */
/*            3. 1.1998 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/
/* $Id: code3203x.c,v 1.2 2005/09/08 17:31:03 alfred Exp $                       */
/***************************************************************************** 
 * $Log: code3203x.c,v $
 * Revision 1.2  2005/09/08 17:31:03  alfred
 * - add missing include
 *
 * Revision 1.1  2003/11/06 02:49:19  alfred
 * - recreated
 *
 * Revision 1.4  2003/05/02 21:23:09  alfred
 * - strlen() updates
 *
 * Revision 1.3  2002/08/14 18:17:35  alfred
 * - warn about NULL allocation
 *
 * Revision 1.2  2002/07/14 18:39:58  alfred
 * - fixed TempAll-related warnings
 *
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "endian.h"
#include "bpemu.h"
#include "strutil.h"
#include "chunks.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmcode.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "codevars.h"


#define ConditionCount 28
#define FixedOrderCount 3
#define RotOrderCount 4
#define StkOrderCount 4
#define GenOrderCount 41
#define ParOrderCount 8
#define SingOrderCount 3

typedef struct
         {
          char *Name;
          Byte Code;
         } Condition;

typedef struct
         {
          char *Name;
          LongWord Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          int NameLen;
          Boolean May1,May3;
          Byte Code,Code3;
          Boolean OnlyMem;
          Boolean SwapOps;
          Boolean ImmFloat;
          Byte ParMask,Par3Mask;
          Byte PCodes[8],P3Codes[8];
         } GenOrder;

typedef struct
         {
          char *Name;
          LongWord Code;
          Byte Mask;
         } SingOrder;


static CPUVar CPU32030,CPU32031;
static SimpProc SaveInitProc;

static Boolean NextPar,ThisPar;
static Byte PrevARs,ARs;
static char PrevOp[7];
static int z2;
static ShortInt PrevSrc1Mode,PrevSrc2Mode,PrevDestMode;
static ShortInt CurrSrc1Mode,CurrSrc2Mode,CurrDestMode;
static Word PrevSrc1Part,PrevSrc2Part,PrevDestPart;
static Word CurrSrc1Part,CurrSrc2Part,CurrDestPart;

static Condition *Conditions;
static FixedOrder *FixedOrders;
static char **RotOrders;
static char **StkOrders;
static GenOrder *GenOrders;
static char **ParOrders;
static SingOrder *SingOrders;

static LongInt DPValue;

/*-------------------------------------------------------------------------*/
/* Befehlstabellenverwaltung */

        static void AddCondition(char *NName, Byte NCode)
BEGIN
   if (InstrZ>=ConditionCount) exit(255);
   Conditions[InstrZ].Name=NName;
   Conditions[InstrZ++].Code=NCode;
END

        static void AddFixed(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddSing(char *NName, LongWord NCode, Byte NMask)
BEGIN
   if (InstrZ>=SingOrderCount) exit(255);
   SingOrders[InstrZ].Name=NName;
   SingOrders[InstrZ].Code=NCode;
   SingOrders[InstrZ++].Mask=NMask;
END

        static void AddGen(char *NName, Boolean NMay1, Boolean NMay3,
                           Byte NCode, Byte NCode3,
                           Boolean NOnly, Boolean NSwap, Boolean NImm,
                           Byte NMask1, Byte NMask3,
                           Byte C20, Byte C21, Byte C22, Byte C23, Byte C24,
                           Byte C25, Byte C26, Byte C27, Byte C30, Byte C31,
                           Byte C32, Byte C33, Byte C34, Byte C35, Byte C36,
                           Byte C37)
BEGIN
   if (InstrZ>=GenOrderCount) exit(255);
   GenOrders[InstrZ].Name=NName;
   GenOrders[InstrZ].NameLen=strlen(NName);
   GenOrders[InstrZ].May1=NMay1; GenOrders[InstrZ].May3=NMay3;
   GenOrders[InstrZ].Code=NCode; GenOrders[InstrZ].Code3=NCode3;
   GenOrders[InstrZ].OnlyMem=NOnly; GenOrders[InstrZ].SwapOps=NSwap;
   GenOrders[InstrZ].ImmFloat=NImm;
   GenOrders[InstrZ].ParMask=NMask1; GenOrders[InstrZ].Par3Mask=NMask3;
   GenOrders[InstrZ].PCodes[0]=C20;  GenOrders[InstrZ].PCodes[1]=C21;
   GenOrders[InstrZ].PCodes[2]=C22;  GenOrders[InstrZ].PCodes[3]=C23;
   GenOrders[InstrZ].PCodes[4]=C24;  GenOrders[InstrZ].PCodes[5]=C25;
   GenOrders[InstrZ].PCodes[6]=C26;  GenOrders[InstrZ].PCodes[7]=C27;
   GenOrders[InstrZ].P3Codes[0]=C30; GenOrders[InstrZ].P3Codes[1]=C31;
   GenOrders[InstrZ].P3Codes[2]=C32; GenOrders[InstrZ].P3Codes[3]=C33;
   GenOrders[InstrZ].P3Codes[4]=C34; GenOrders[InstrZ].P3Codes[5]=C35;
   GenOrders[InstrZ].P3Codes[6]=C36; GenOrders[InstrZ++].P3Codes[7]=C37;
END

        static void InitFields(void)
BEGIN
   Conditions=(Condition *) malloc(sizeof(Condition)*ConditionCount); InstrZ=0;
   AddCondition("U"  ,0x00); AddCondition("LO" ,0x01);
   AddCondition("LS" ,0x02); AddCondition("HI" ,0x03);
   AddCondition("HS" ,0x04); AddCondition("EQ" ,0x05);
   AddCondition("NE" ,0x06); AddCondition("LT" ,0x07);
   AddCondition("LE" ,0x08); AddCondition("GT" ,0x09);
   AddCondition("GE" ,0x0a); AddCondition("Z"  ,0x05);
   AddCondition("NZ" ,0x06); AddCondition("P"  ,0x09);
   AddCondition("N"  ,0x07); AddCondition("NN" ,0x0a);
   AddCondition("NV" ,0x0c); AddCondition("V"  ,0x0d);
   AddCondition("NUF",0x0e); AddCondition("UF" ,0x0f);
   AddCondition("NC" ,0x04); AddCondition("C"  ,0x01);
   AddCondition("NLV",0x10); AddCondition("LV" ,0x11);
   AddCondition("NLUF",0x12);AddCondition("LUF",0x13);
   AddCondition("ZUF",0x14); AddCondition(""   ,0x00);

   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("IDLE",0x06000000); AddFixed("SIGI",0x16000000);
   AddFixed("SWI" ,0x66000000);

   RotOrders=(char **) malloc(sizeof(char *)*RotOrderCount); InstrZ=0;
   RotOrders[InstrZ++]="ROL"; RotOrders[InstrZ++]="ROLC";
   RotOrders[InstrZ++]="ROR"; RotOrders[InstrZ++]="RORC";

   StkOrders=(char **) malloc(sizeof(char *)*StkOrderCount); InstrZ=0;
   StkOrders[InstrZ++]="POP";  StkOrders[InstrZ++]="POPF";
   StkOrders[InstrZ++]="PUSH"; StkOrders[InstrZ++]="PUSHF";

   GenOrders=(GenOrder *) malloc(sizeof(GenOrder)*GenOrderCount); InstrZ=0;
/*         Name         May3      Cd3       Swap       PM1                                              PCodes3     */
/*                May1        Cd1     OMem        ImmF     PM3           PCodes1                                    */
   AddGen("ABSF" ,True ,False,0x00,0xff,False,False,True , 4, 0,
          0xff,0xff,0x04,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("ABSI" ,True ,False,0x01,0xff,False,False,False, 8, 0,
          0xff,0xff,0xff,0x05,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("ADDC" ,False,True ,0x02,0x00,False,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("ADDF" ,False,True ,0x03,0x01,False,False,True , 0, 4,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0x06,0xff,0xff,0xff,0xff,0xff);
   AddGen("ADDI" ,False,True ,0x04,0x02,False,False,False, 0, 8,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x07,0xff,0xff,0xff,0xff);
   AddGen("AND"  ,False,True ,0x05,0x03,False,False,False, 0, 8,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x08,0xff,0xff,0xff,0xff);
   AddGen("ANDN" ,False,True ,0x06,0x04,False,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("ASH"  ,False,True ,0x07,0x05,False,False,False, 0, 8,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x09,0xff,0xff,0xff,0xff);
   AddGen("CMPF" ,False,True ,0x08,0x06,False,False,True , 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("CMPI" ,False,True ,0x09,0x07,False,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("FIX"  ,True ,False,0x0a,0xff,False,False,True , 8, 0,
          0xff,0xff,0xff,0x0a,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("FLOAT",True ,False,0x0b,0xff,False,False,False, 4, 0,
          0xff,0xff,0x0b,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDE"  ,False,False,0x0d,0xff,False,False,True , 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDF"  ,False,False,0x0e,0xff,False,False,True , 5, 0,
          0x02,0xff,0x0c,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDFI" ,False,False,0x0f,0xff,True ,False,True , 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDI"  ,False,False,0x10,0xff,False,False,False,10, 0,
          0xff,0x03,0xff,0x0d,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDII" ,False,False,0x11,0xff,True ,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LDM"  ,False,False,0x12,0xff,False,False,True , 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("LSH"  ,False,True ,0x13,0x08,False,False,False, 0, 8,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x0e,0xff,0xff,0xff,0xff);
   AddGen("MPYF" ,False,True ,0x14,0x09,False,False,True , 0,52,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0x0f,0xff,0x00,0x01,0xff,0xff);
   AddGen("MPYI" ,False,True ,0x15,0x0a,False,False,False, 0,200,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x10,0xff,0xff,0x02,0x03);
   AddGen("NEGB" ,True ,False,0x16,0xff,False,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("NEGF" ,True ,False,0x17,0xff,False,False,True , 4, 0,
          0xff,0xff,0x11,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("NEGI" ,True ,False,0x18,0xff,False,False,False, 8, 0,
          0xff,0xff,0xff,0x12,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("NORM" ,True ,False,0x1a,0xff,False,False,True , 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("NOT"  ,True ,False,0x1b,0xff,False,False,False, 8, 0,
          0xff,0xff,0xff,0x13,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("OR"   ,False,True ,0x20,0x0b,False,False,False, 0, 8,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x14,0xff,0xff,0xff,0xff);
   AddGen("RND"  ,True ,False,0x22,0xff,False,False,True , 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("STF"  ,False,False,0x28,0xff,True ,True ,True , 4, 0,
          0xff,0xff,0x00,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("STFI" ,False,False,0x29,0xff,True ,True ,True , 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("STI"  ,False,False,0x2a,0xff,True ,True ,False, 8, 0,
          0xff,0xff,0xff,0x01,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("STII" ,False,False,0x2b,0xff,True ,True ,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBB" ,False,True ,0x2d,0x0c,False,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBC" ,False,False,0x2e,0xff,False,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBF" ,False,True ,0x2f,0x0d,False,False,True , 0, 4,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0x15,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBI" ,False,True ,0x30,0x0e,False,False,False, 0, 8,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x16,0xff,0xff,0xff,0xff);
   AddGen("SUBRB",False,False,0x31,0xff,False,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBRF",False,False,0x32,0xff,False,False,True , 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("SUBRI",False,False,0x33,0xff,False,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("TSTB" ,False,True ,0x34,0x0f,False,False,False, 0, 0,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff);
   AddGen("XOR"  ,False,True ,0x35,0x10,False,False,False, 0, 8,
          0xff,0xff,0xff,0xff,0xff,0xff,0xff,0xff, 0xff,0xff,0xff,0x17,0xff,0xff,0xff,0xff);

   ParOrders=(char **) malloc(sizeof(char *)*ParOrderCount); InstrZ=0;
   ParOrders[InstrZ++]="LDF";   ParOrders[InstrZ++]="LDI";
   ParOrders[InstrZ++]="STF";   ParOrders[InstrZ++]="STI";
   ParOrders[InstrZ++]="ADDF3"; ParOrders[InstrZ++]="SUBF3";
   ParOrders[InstrZ++]="ADDI3"; ParOrders[InstrZ++]="SUBI3";

   SingOrders=(SingOrder *) malloc(sizeof(SingOrder)*SingOrderCount); InstrZ=0;
   AddSing("IACK",0x1b000000,6);
   AddSing("NOP" ,0x0c800000,5);
   AddSing("RPTS",0x139b0000,15);
END

        static void DeinitFields(void)
BEGIN
   free(Conditions);
   free(FixedOrders);
   free(RotOrders);
   free(StkOrders);
   free(GenOrders);
   free(ParOrders);
   free(SingOrders);
END

/*-------------------------------------------------------------------------*/
/* Gleitkommawandler */

        static void SplitExt(Double Inp, LongInt *Expo, LongWord *Mant)
BEGIN
   Byte Field[8];
   Boolean Sign;
   int z;

   Double_2_ieee8(Inp,Field,False);
   Sign=(Field[7]>0x7f);
   *Expo=(((LongWord) Field[7]&0x7f)<<4)+(Field[6]>>4);
   *Mant=Field[6]&0x0f; if (*Expo!=0) *Mant|=0x10;
   for (z=5; z>2; z--) *Mant=((*Mant)<<8)|Field[z];
   *Mant=((*Mant)<<3)+(Field[2]>>5);
   *Expo-=0x3ff;
   if (Sign) *Mant=0xffffffff-(*Mant);
   *Mant=(*Mant)^0x80000000;
END

        static Boolean ExtToShort(Double Inp, Word *Erg)
BEGIN
   LongInt Expo;
   LongWord Mant;

   if (Inp==0) *Erg=0x8000;
   else
    BEGIN
     SplitExt(Inp,&Expo,&Mant);
     if (abs(Expo)>7)
      BEGIN
       WrError((Expo>0)?1320:1315);
       return False;
      END
     *Erg=((Expo << 12) & 0xf000) | ((Mant >> 20) & 0xfff);
    END
   return True;
END

        static Boolean ExtToSingle(Double Inp, LongWord *Erg)
BEGIN
   LongInt Expo;
   LongWord Mant;

   if (Inp==0) *Erg=0x80000000;
   else
    BEGIN
     SplitExt(Inp,&Expo,&Mant);
     if (abs(Expo)>127)
      BEGIN
       WrError((Expo>0)?1320:1315);
       return False;
      END
     *Erg=((Expo << 24) & 0xff000000)+(Mant >> 8);
    END
   return True;
END

        static Boolean ExtToExt(Double Inp, LongWord *ErgL, LongWord *ErgH)
BEGIN
   LongInt Exp;

   if (Inp==0)
    BEGIN
     *ErgH=0x80; *ErgL=0x00000000;
    END
   else
    BEGIN
     SplitExt(Inp,&Exp,ErgL);
     if (abs(Exp)>127)
      BEGIN
       WrError((Exp>0)?1320:1315);
       return False;
      END
     *ErgH=Exp&0xff;
    END
   return True;
END

/*-------------------------------------------------------------------------*/
/* Adressparser */

#define ModNone (-1)
#define ModReg 0
#define MModReg (1 << ModReg)
#define ModDir 1
#define MModDir (1 << ModDir)
#define ModInd 2
#define MModInd (1 << ModInd)
#define ModImm 3
#define MModImm (1 << ModImm)

static ShortInt AdrMode;
static LongInt AdrPart;

        static Boolean DecodeReg(char *Asc, Byte *Erg)
BEGIN
#define RegCnt 12
#define RegStart 0x10
    static char *Regs[RegCnt]=
        {"DP","IR0","IR1","BK","SP","ST","IE","IF","IOF","RS","RE","RC"};
    Boolean Err;

   if ((toupper(*Asc)=='R') AND (strlen(Asc)<=3) AND (strlen(Asc)>=2))
    BEGIN
     *Erg=ConstLongInt(Asc+1,&Err);
     if ((Err) AND (*Erg<=0x1b)) return True;
    END

   if ((strlen(Asc)==3) AND (toupper(*Asc)=='A') AND (toupper(Asc[1])=='R') AND (Asc[2]>='0') AND (Asc[2]<='7'))
    BEGIN
     *Erg=Asc[2]-'0'+8; return True;
    END

   *Erg=0;
   while ((*Erg<RegCnt) AND (strcasecmp(Regs[*Erg],Asc)!=0)) (*Erg)++;
   if (*Erg<RegCnt)
    BEGIN
     *Erg+=RegStart; return True;
    END

   return False;
END

        static void ChkAdr(Byte Erl)
BEGIN
   if ((AdrMode!=ModNone) AND ((Erl & (1 << AdrMode))==0))
    BEGIN
     AdrMode=ModNone; WrError(1350);
    END
END

        static void DecodeAdr(char *Asc, Byte Erl, Boolean ImmFloat)
BEGIN
   Byte HReg;
   Integer Disp;
   char *p;
   int l;
   Double f;
   Word fi;
   LongInt AdrLong;
   Boolean BitRev,Circ;
   String NDisp;
   Boolean OK;
   enum {ModBase,ModAdd,ModSub,ModPreInc,ModPreDec,ModPostInc,ModPostDec} Mode;

   KillBlanks(Asc);

   AdrMode=ModNone;

   /* I. Register? */

   if (DecodeReg(Asc,&HReg))
    BEGIN
     AdrMode=ModReg; AdrPart=HReg; ChkAdr(Erl); return;
    END

   /* II. indirekt ? */

   if (*Asc=='*')
    BEGIN
     /* II.1. Erkennungszeichen entfernen */

     strcpy(Asc,Asc+1);

     /* II.2. Extrawuerste erledigen */

     BitRev=False; Circ=False;
     if (toupper(Asc[strlen(Asc)-1])=='B')
      BEGIN
       BitRev=True; Asc[strlen(Asc)-1]='\0';
      END
     else if (Asc[strlen(Asc)-1]=='%')
      BEGIN
       Circ=True; Asc[strlen(Asc)-1]='\0';
      END

     /* II.3. Displacement entfernen und auswerten:
             0..255-->Displacement
             -1,-2 -->IR0,IR1
             -3    -->Default */

     p=QuotPos(Asc,'(');
     if (p!=Nil)
      BEGIN
       if (Asc[strlen(Asc)-1]!=')')
        BEGIN
         WrError(1350); return;
        END
       *p='\0'; strmaxcpy(NDisp,p+1,255); NDisp[strlen(NDisp)-1]='\0';
       if (strcasecmp(NDisp,"IR0")==0) Disp=(-1);
       else if (strcasecmp(NDisp,"IR1")==0) Disp=(-2);
       else
        BEGIN
         Disp=EvalIntExpression(NDisp,UInt8,&OK);
         if (NOT OK) return;
        END
      END
     else Disp=(-3);

     /* II.4. Addieren/Subtrahieren mit/ohne Update? */

     l=strlen(Asc);
     if (*Asc=='-')
      BEGIN
       if (Asc[1]=='-')
        BEGIN
         Mode=ModPreDec; strcpy(Asc,Asc+2);
        END
       else
        BEGIN
         Mode=ModSub; strcpy(Asc,Asc+1);
        END
      END
     else if (*Asc=='+')
      BEGIN
       if (Asc[1]=='+')
        BEGIN
         Mode=ModPreInc; strcpy(Asc,Asc+2);
        END
       else
        BEGIN
         Mode=ModAdd; strcpy(Asc,Asc+1);
        END
      END
     else if (Asc[l-1]=='-')
      BEGIN
       if (Asc[l-2]=='-')
        BEGIN
         Mode=ModPostDec; Asc[l-2]='\0';
        END
       else
        BEGIN
         WrError(1350); return;
        END
      END
     else if (Asc[l-1]=='+')
      BEGIN
       if (Asc[l-2]=='+')
        BEGIN
         Mode=ModPostInc; Asc[l-2]='\0';
        END
       else
        BEGIN
         WrError(1350); return;
        END
      END
     else Mode=ModBase;

     /* II.5. Rest muss Basisregister sein */

     if ((NOT DecodeReg(Asc,&HReg)) OR (HReg<8) OR (HReg>15))
      BEGIN
       WrError(1350); return;
      END
     HReg-=8;
     if ((ARs & (1l << HReg))==0) ARs+=1l << HReg;
     else WrXError(210,Asc);

     /* II.6. Default-Displacement explizit machen */

     if (Disp==-3)
      Disp=(Mode==ModBase) ? 0 : 1;

     /* II.7. Entscheidungsbaum */

     switch (Mode)
      BEGIN
       case ModBase:
       case ModAdd:
        if ((Circ) OR (BitRev)) WrError(1350);
        else
         BEGIN
          switch (Disp)
           BEGIN
            case -2: AdrPart=0x8000; break;
            case -1: AdrPart=0x4000; break;
            case  0: AdrPart=0xc000; break;
            default: AdrPart=Disp;
           END
          AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         END
        break;
       case ModSub:
        if ((Circ) OR (BitRev)) WrError(1350);
        else
         BEGIN
          switch (Disp)
           BEGIN
            case -2: AdrPart=0x8800; break;
            case -1: AdrPart=0x4800; break;
            case  0: AdrPart=0xc000; break;
            default: AdrPart=0x0800+Disp;
           END
          AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         END
        break;
       case ModPreInc:
        if ((Circ) OR (BitRev)) WrError(1350);
        else
         BEGIN
          switch (Disp)
           BEGIN
            case -2: AdrPart=0x9000; break;
            case -1: AdrPart=0x5000; break;
            default: AdrPart=0x1000+Disp;
           END
          AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         END
        break;
       case ModPreDec:
        if ((Circ) OR (BitRev)) WrError(1350);
        else
         BEGIN
          switch (Disp)
           BEGIN
            case -2: AdrPart=0x9800; break;
            case -1: AdrPart=0x5800; break;
            default: AdrPart=0x1800+Disp;
           END
          AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         END
        break;
       case ModPostInc:
        if (BitRev)
         BEGIN
          if (Disp!=-1) WrError(1350);
          else
           BEGIN
            AdrPart=0xc800+(((Word)HReg) << 8); AdrMode=ModInd;
           END
         END
        else
         BEGIN
          switch (Disp)
           BEGIN
            case -2: AdrPart=0xa000; break;
            case -1: AdrPart=0x6000; break;
            default: AdrPart=0x2000+Disp;
           END
          if (Circ) AdrPart+=0x1000;
          AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         END
        break;
       case ModPostDec:
        if (BitRev) WrError(1350);
        else
         BEGIN
          switch (Disp)
           BEGIN
            case -2: AdrPart=0xa800; break;
            case -1: AdrPart=0x6800; break;
            default: AdrPart=0x2800+Disp; break;
           END
          if (Circ) AdrPart+=0x1000;
          AdrPart+=((Word)HReg) << 8; AdrMode=ModInd;
         END
        break;
      END

     ChkAdr(Erl); return;
    END

   /* III. absolut */

   if (*Asc=='@')
    BEGIN
     AdrLong=EvalIntExpression(Asc+1,UInt24,&OK);
     if (OK)
      BEGIN
       if ((DPValue!=-1) AND ((AdrLong >> 16)!=DPValue)) WrError(110);
       AdrMode=ModDir; AdrPart=AdrLong & 0xffff;
      END
     ChkAdr(Erl); return;
    END

   /* IV. immediate */

   if (ImmFloat)
    BEGIN
     f=EvalFloatExpression(Asc,Float64,&OK);
     if (OK)
      if (ExtToShort(f,&fi))
       BEGIN
        AdrPart=fi; AdrMode=ModImm;
       END
    END
   else
    BEGIN
     AdrPart=EvalIntExpression(Asc,Int16,&OK);
     if (OK)
      BEGIN
       AdrPart&=0xffff; AdrMode=ModImm;
      END
    END

   ChkAdr(Erl);
END

        static Word EffPart(Byte Mode, Word Part)
BEGIN
   switch (Mode)
    BEGIN
     case ModReg: return Lo(Part);
     case ModInd: return Hi(Part);
     default: WrError(10000); return 0;
    END
END

/*-------------------------------------------------------------------------*/
/* Code-Erzeugung */

        static Boolean DecodePseudo(void)
BEGIN
#define ASSUME3203Count 1
   static ASSUMERec ASSUME3203s[ASSUME3203Count]=
                 {{"DP", &DPValue, -1, 0xff, 0x100}};

   Boolean OK;
   int z,z2;
   LongInt Size;
   Double f;
   TempResult t;

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME3203s,ASSUME3203Count);
     return True;
    END

   if (Memo("SINGLE"))
    BEGIN
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       OK=True;
       for (z=1; z<=ArgCnt; z++)
        if (OK)
         BEGIN
          f=EvalFloatExpression(ArgStr[z],Float64,&OK);
          if (OK)
           OK=OK AND ExtToSingle(f,DAsmCode+(CodeLen++));
         END
       if (NOT OK) CodeLen=0;
      END
     return True;
    END

   if (Memo("EXTENDED"))
    BEGIN
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       OK=True;
       for (z=1; z<=ArgCnt; z++)
        if (OK)
         BEGIN
          f=EvalFloatExpression(ArgStr[z],Float64,&OK);
          if (OK)
           OK=OK AND ExtToExt(f,DAsmCode+CodeLen+1,DAsmCode+CodeLen);
          CodeLen+=2;
         END
       if (NOT OK) CodeLen=0;
      END
     return True;
    END

   if (Memo("WORD"))
    BEGIN
     if (ArgCnt==0) WrError(1110);
     else
      BEGIN
       OK=True;
       for (z=1; z<=ArgCnt; z++)
        if (OK) DAsmCode[CodeLen++]=EvalIntExpression(ArgStr[z],Int32,&OK);
       if (NOT OK) CodeLen=0;
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
#ifdef HAS64
             if (NOT RangeCheck(t.Contents.Int,Int32))
              BEGIN
               OK=False; WrError(1320);
              END
             else
#endif
              DAsmCode[CodeLen++]=t.Contents.Int;
             break;
            case TempFloat:
             if (NOT ExtToSingle(t.Contents.Float,DAsmCode+(CodeLen++))) OK=False;
             break;
            case TempString:
             for (z2=0; z2<(int)strlen(t.Contents.Ascii); z2++)
              BEGIN
               if ((z2 & 3)==0) DAsmCode[CodeLen++]=0;
               DAsmCode[CodeLen-1]+=
                  (((LongWord)CharTransTable[((usint)t.Contents.Ascii[z2])&0xff])) << (8*(3-(z2 & 3)));
              END
             break;
            default:
             OK=False;
           END
         END
       if (NOT OK) CodeLen=0;
      END
     return True;
    END

   if (Memo("BSS"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Size=EvalIntExpression(ArgStr[1],UInt24,&OK);
       if (FirstPassUnknown) WrError(1820);
       if ((OK) AND (NOT FirstPassUnknown))
        BEGIN
         DontPrint=True;
         if (!Size) WrError(290);
         CodeLen=Size;
         BookKeeping();
        END
      END
     return True;
    END

   return False;
END

        static void JudgePar(GenOrder *Prim, int Sec, Byte *ErgMode, Byte *ErgCode)
BEGIN
   if (Sec>3) *ErgMode=3;
   else if (Prim->May3) *ErgMode=1;
   else *ErgMode=2;
   if (*ErgMode==2) *ErgCode=Prim->PCodes[Sec];
               else *ErgCode=Prim->P3Codes[Sec];
END

        static LongWord EvalAdrExpression(char *Asc, Boolean *OK)
BEGIN
   if (*Asc=='@') strcpy(Asc,Asc+1);
   return EvalIntExpression(Asc,UInt24,OK);
END

        static void SwapMode(ShortInt *M1, ShortInt *M2)
BEGIN
   AdrMode=(*M1); *M1=(*M2); *M2=AdrMode;
END

        static void SwapPart(Word *P1, Word *P2)
BEGIN
   AdrPart=(*P1); *P1=(*P2); *P2=AdrPart;
END

        static void MakeCode_3203X(void)
BEGIN
   Boolean OK,Is3;
   Byte HReg,HReg2,Sum;
   int z,z3,l;
   LongInt AdrLong,DFlag,Disp;
   String HOp,Form;

   CodeLen=0; DontPrint=False;

   ThisPar=(strcmp(LabPart,"||")==0);
   if ((strlen(OpPart)>2) AND (strncmp(OpPart,"||",2)==0))
    BEGIN
     ThisPar=True; strcpy(OpPart,OpPart+2);
    END
   if ((NOT NextPar) AND (ThisPar))
    BEGIN
     WrError(1950); return;
    END
   ARs=0;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (ThisPar) WrError(1950);
      else
       BEGIN
        DAsmCode[0]=FixedOrders[z].Code; CodeLen=1;
       END
      NextPar=False; return;
     END

   /* Arithmetik/Logik */

   for (z=0; z<GenOrderCount; z++)
    if ((strncmp(OpPart,GenOrders[z].Name,GenOrders[z].NameLen)==0)
    AND ((OpPart[GenOrders[z].NameLen]=='\0') OR (OpPart[GenOrders[z].NameLen]=='3')))
     BEGIN
      NextPar=False;
      /* Argumentzahl abgleichen */
      if (ArgCnt==1)
       BEGIN
        if (GenOrders[z].May1)
         BEGIN
          ArgCnt=2; strcpy(ArgStr[2],ArgStr[1]);
         END
        else
         BEGIN
          WrError(1110); return;
         END
       END
      if ((ArgCnt==3) AND (OpPart[strlen(OpPart)-1]!='3')) strcat(OpPart,"3");
      Is3=(OpPart[strlen(OpPart)-1]=='3');
      if ((GenOrders[z].SwapOps) AND (NOT Is3))
       BEGIN
        strcpy(ArgStr[3],ArgStr[1]); 
        strcpy(ArgStr[1],ArgStr[2]);
        strcpy(ArgStr[2],ArgStr[3]);
       END
      if ((Is3) AND (ArgCnt==2))
       BEGIN
        ArgCnt=3; strcpy(ArgStr[3],ArgStr[2]);
       END
      if ((ArgCnt<2) OR (ArgCnt>3) OR ((Is3) AND (NOT GenOrders[z].May3)))
       BEGIN
        WrError(1110); return;
       END
      /* Argumente parsen */
      if (Is3)
       BEGIN
        if (Memo("TSTB3"))
         BEGIN
          CurrDestMode=ModReg; CurrDestPart=0;
         END
        else
         BEGIN
          DecodeAdr(ArgStr[3],MModReg,GenOrders[z].ImmFloat);
          if (AdrMode==ModNone) return;
          CurrDestMode=AdrMode; CurrDestPart=AdrPart;
         END
        DecodeAdr(ArgStr[2],MModReg+MModInd,GenOrders[z].ImmFloat);
        if (AdrMode==ModNone) return;
        if ((AdrMode==ModInd) AND ((AdrPart & 0xe000)==0) AND (Lo(AdrPart)!=1))
         BEGIN
          WrError(1350); return;
         END
        CurrSrc2Mode=AdrMode; CurrSrc2Part=AdrPart;
        DecodeAdr(ArgStr[1],MModReg+MModInd,GenOrders[z].ImmFloat);
        if (AdrMode==ModNone) return;
        if ((AdrMode==ModInd) AND ((AdrPart & 0xe000)==0) AND (Lo(AdrPart)!=1))
         BEGIN
          WrError(1350); return;
         END
        CurrSrc1Mode=AdrMode; CurrSrc1Part=AdrPart;
       END
      else /* NOT Is3 */
       BEGIN
        DecodeAdr(ArgStr[1],MModDir+MModInd+((GenOrders[z].OnlyMem)?0:MModReg+MModImm),GenOrders[z].ImmFloat);
        if (AdrMode==ModNone) return;
        CurrSrc1Mode=AdrMode; CurrSrc1Part=AdrPart;
        DecodeAdr(ArgStr[2],MModReg+MModInd,GenOrders[z].ImmFloat);
        switch (AdrMode)
         BEGIN
          case ModReg:
           CurrDestMode=AdrMode; CurrDestPart=AdrPart;
           CurrSrc2Mode=CurrSrc1Mode; CurrSrc2Part=CurrSrc1Part;
           break;
          case ModInd:
           if (((strcmp(OpPart,"TSTB")!=0) AND (strcmp(OpPart,"CMPI")!=0) AND (strcmp(OpPart,"CMPF")!=0))
           OR  ((CurrSrc1Mode==ModDir) OR (CurrSrc1Mode==ModImm))
           OR  ((CurrSrc1Mode==ModInd) AND ((CurrSrc1Part & 0xe000)==0) AND (Lo(CurrSrc1Part)!=1))
           OR  (((AdrPart & 0xe000)==0) AND (Lo(AdrPart)!=1)))
            BEGIN
             WrError(1350); return;
            END
           else
            BEGIN
             Is3=True; CurrDestMode=ModReg; CurrDestPart=0;
             CurrSrc2Mode=AdrMode; CurrSrc2Part=AdrPart;
            END
           break;
          case ModNone: 
           return;
         END
       END
      /* auswerten: parallel... */
      if (ThisPar)
       BEGIN
        /* in Standardreihenfolge suchen */
        if (PrevOp[strlen(PrevOp)-1]=='3') HReg=GenOrders[z2].Par3Mask;
        else HReg=GenOrders[z2].ParMask;
        z3=0;
        while ((z3<ParOrderCount) AND ((NOT Odd(HReg)) OR (strcmp(ParOrders[z3],OpPart)!=0)))
         BEGIN
          z3++; HReg>>=1;
         END
        if (z3<ParOrderCount) JudgePar(GenOrders+z2,z3,&HReg,&HReg2);
        /* in gedrehter Reihenfolge suchen */
        else
         BEGIN
          if (OpPart[strlen(OpPart)-1]=='3') HReg=GenOrders[z].Par3Mask;
          else HReg=GenOrders[z].ParMask;
          z3=0;
          while ((z3<ParOrderCount) AND ((NOT Odd(HReg)) OR (strcmp(ParOrders[z3],PrevOp)!=0)))
           BEGIN
            z3++; HReg>>=1;
           END
          if (z3<ParOrderCount)
           BEGIN
            JudgePar(GenOrders+z,z3,&HReg,&HReg2);
            SwapMode(&CurrDestMode,&PrevDestMode);
            SwapMode(&CurrSrc1Mode,&PrevSrc1Mode);
            SwapMode(&CurrSrc2Mode,&PrevSrc2Mode);
            SwapPart(&CurrDestPart,&PrevDestPart);
            SwapPart(&CurrSrc1Part,&PrevSrc1Part);
            SwapPart(&CurrSrc2Part,&PrevSrc2Part);
           END
          else
           BEGIN
            WrError(1950); return;
           END
         END
        /* mehrfache Registernutzung ? */
        for (z3=0; z3<8; z3++)
         if ((ARs & PrevARs & (1l << z3))!=0)
          BEGIN
           sprintf(Form,"AR%d",z3); WrXError(210,Form);
          END
        /* 3 Basisfaelle */
        switch (HReg)
         BEGIN
          case 1:
           if ((strcmp(PrevOp,"LSH3")==0) OR (strcmp(PrevOp,"ASH3")==0) OR (strcmp(PrevOp,"SUBF3")==0) OR (strcmp(PrevOp,"SUBI3")==0))
            BEGIN
             SwapMode(&PrevSrc1Mode,&PrevSrc2Mode);
             SwapPart(&PrevSrc1Part,&PrevSrc2Part);
            END
           if ((PrevDestPart>7) OR (CurrDestPart>7))
            BEGIN
             WrError(1445); return;
            END
           /* Bei Addition und Multiplikation Kommutativitaet nutzen */
           if  ((PrevSrc2Mode==ModInd) AND (PrevSrc1Mode==ModReg)
            AND ((strncmp(PrevOp,"ADD",3)==0) OR (strncmp(PrevOp,"MPY",3)==0)
              OR (strncmp(PrevOp,"AND",3)==0) OR (strncmp(PrevOp,"XOR",3)==0)
              OR (strncmp(PrevOp,"OR",2)==0)))
            BEGIN
             SwapMode(&PrevSrc1Mode,&PrevSrc2Mode);
             SwapPart(&PrevSrc1Part,&PrevSrc2Part);
            END
           if ((PrevSrc2Mode!=ModReg) OR (PrevSrc2Part>7)
            OR (PrevSrc1Mode!=ModInd) OR (CurrSrc1Mode!=ModInd))
            BEGIN
             WrError(1355); return;
            END
           RetractWords(1);
           DAsmCode[0]=0xc0000000+(((LongWord)HReg2) << 25)
                      +(((LongWord)PrevDestPart) << 22)
                      +(((LongWord)PrevSrc2Part) << 19)
                      +(((LongWord)CurrDestPart) << 16)
                      +(CurrSrc1Part & 0xff00)+Hi(PrevSrc1Part);
           CodeLen=1; NextPar=False;
           break;
          case 2:
           if ((PrevDestPart>7) OR (CurrDestPart>7))
            BEGIN
             WrError(1445); return;
            END
           if ((PrevSrc1Mode!=ModInd) OR (CurrSrc1Mode!=ModInd))
            BEGIN
             WrError(1355); return;
            END
           RetractWords(1);
           DAsmCode[0]=0xc0000000+(((LongWord)HReg2) << 25)
                      +(((LongWord)PrevDestPart) << 22)
                      +(CurrSrc1Part & 0xff00)+Hi(PrevSrc1Part);
           if ((strcmp(PrevOp,OpPart)==0) AND (*OpPart=='L'))
            BEGIN
             DAsmCode[0]+=((LongWord)CurrDestPart) << 19;
             if (PrevDestPart==CurrDestPart) WrError(140);
            END
           else
            DAsmCode[0]+=((LongWord)CurrDestPart) << 16;
           CodeLen=1; NextPar=False;
           break;
          case 3:
           if ((PrevDestPart>1) OR (CurrDestPart<2) OR (CurrDestPart>3))
            BEGIN
             WrError(1445); return;
            END
           Sum=0;
           if (PrevSrc1Mode==ModInd) Sum++;
           if (PrevSrc2Mode==ModInd) Sum++;
           if (CurrSrc1Mode==ModInd) Sum++;
           if (CurrSrc2Mode==ModInd) Sum++;
           if (Sum!=2)
            BEGIN
             WrError(1355); return;
            END
           RetractWords(1);
           DAsmCode[0]=0x80000000+(((LongWord)HReg2) << 26)
                      +(((LongWord)PrevDestPart & 1) << 23)
                      +(((LongWord)CurrDestPart & 1) << 22);
           CodeLen=1;
           if (CurrSrc2Mode==ModReg)
            if (CurrSrc1Mode==ModReg)
             BEGIN
              DAsmCode[0]+=((LongWord)0x00000000)
                          +(((LongWord)CurrSrc2Part) << 19)
                          +(((LongWord)CurrSrc1Part) << 16)
                          +(PrevSrc2Part & 0xff00)+Hi(PrevSrc1Part);
             END
            else
             BEGIN
              DAsmCode[0]+=((LongWord)0x03000000)
                          +(((LongWord)CurrSrc2Part) << 16)
                          +Hi(CurrSrc1Part);
              if (PrevSrc1Mode==ModReg)
               DAsmCode[0]+=(((LongWord)PrevSrc1Part) << 19)+(PrevSrc2Part & 0xff00);
              else
               DAsmCode[0]+=(((LongWord)PrevSrc2Part) << 19)+(PrevSrc1Part & 0xff00);
             END
           else
            if (CurrSrc1Mode==ModReg)
             BEGIN
              DAsmCode[0]+=((LongWord)0x01000000)
                          +(((LongWord)CurrSrc1Part) << 16)
                          +Hi(CurrSrc2Part);
              if (PrevSrc1Mode==ModReg)
               DAsmCode[0]+=(((LongWord)PrevSrc1Part) << 19)+(PrevSrc2Part & 0xff00);
              else
               DAsmCode[0]+=(((LongWord)PrevSrc2Part) << 19)+(PrevSrc1Part & 0xff00);
             END
            else
             BEGIN
              DAsmCode[0]+=((LongWord)0x02000000)
                          +(((LongWord)PrevSrc2Part) << 19)
                          +(((LongWord)PrevSrc1Part) << 16)
                          +(CurrSrc2Part & 0xff00)+Hi(CurrSrc1Part);
             END
           break;
         END
       END
      /* ...sequentiell */
      else
       BEGIN
        PrevSrc1Mode=CurrSrc1Mode; PrevSrc1Part=CurrSrc1Part;
        PrevSrc2Mode=CurrSrc2Mode; PrevSrc2Part=CurrSrc2Part;
        PrevDestMode=CurrDestMode; PrevDestPart=CurrDestPart;
        strcpy(PrevOp,OpPart); PrevARs=ARs; z2=z;
        if (Is3)
         DAsmCode[0]=0x20000000+(((LongWord)GenOrders[z].Code3) << 23)
                    +(((LongWord)CurrDestPart) << 16)
                    +(((LongWord)CurrSrc2Mode) << 20)+(EffPart(CurrSrc2Mode,CurrSrc2Part) << 8)
                    +(((LongWord)CurrSrc1Mode) << 21)+EffPart(CurrSrc1Mode,CurrSrc1Part);
        else
         DAsmCode[0]=0x00000000+(((LongWord)GenOrders[z].Code) << 23)
                    +(((LongWord)CurrSrc1Mode) << 21)+CurrSrc1Part
                    +(((LongWord)CurrDestPart) << 16);
        CodeLen=1; NextPar=True;
       END
      return;
     END

   for (z=0; z<RotOrderCount; z++)
    if (Memo(RotOrders[z]))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (ThisPar) WrError(1950);
      else if (NOT DecodeReg(ArgStr[1],&HReg)) WrError(1350);
      else
       BEGIN
        DAsmCode[0]=0x11e00000+(((LongWord)z) << 23)+(((LongWord)HReg) << 16);
        CodeLen=1;
       END
      NextPar=False; return;
     END

   for (z=0; z<StkOrderCount; z++)
    if (Memo(StkOrders[z]))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (ThisPar) WrError(1950);
      else if (NOT DecodeReg(ArgStr[1],&HReg)) WrError(1350);
      else
       BEGIN
        DAsmCode[0]=0x0e200000+(((LongWord)z) << 23)+(((LongWord)HReg) << 16);
        CodeLen=1;
       END
      NextPar=False; return;
     END

   /* Datentransfer */

   if ((strncmp(OpPart,"LDI",3)==0) OR (strncmp(OpPart,"LDF",3)==0))
    BEGIN
     strcpy(HOp,OpPart); strcpy(OpPart,OpPart+3);
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       BEGIN
        if (ArgCnt!=2) WrError(1110);
        else if (ThisPar) WrError(1950);
        else
         BEGIN
          DecodeAdr(ArgStr[2],MModReg,False);
          if (AdrMode!=ModNone)
           BEGIN
            HReg=AdrPart;
            DecodeAdr(ArgStr[1],MModReg+MModDir+MModInd+MModImm,HOp[2]=='F');
            if (AdrMode!=ModNone)
             BEGIN
              DAsmCode[0]=0x40000000+(((LongWord)HReg) << 16)
                         +(((LongWord)Conditions[z].Code) << 23)
                         +(((LongWord)AdrMode) << 21)+AdrPart;
              if (HOp[2]=='I') DAsmCode[0]+=0x10000000;
              CodeLen=1;
             END
           END
         END
        NextPar=False; return;
       END
     WrXError(1200,HOp); NextPar=False; return;
    END

   /* Sonderfall NOP auch ohne Argumente */

   if ((Memo("NOP")) AND (ArgCnt==0))
    BEGIN
     CodeLen=1; DAsmCode[0]=NOPCode; return;
    END

   /* Sonderfaelle */

   for (z=0; z<SingOrderCount; z++)
    if (Memo(SingOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (ThisPar) WrError(1950);
      else
       BEGIN
        DecodeAdr(ArgStr[1],SingOrders[z].Mask,False);
        if (AdrMode!=ModNone)
         BEGIN
          DAsmCode[0]=SingOrders[z].Code+(((LongWord)AdrMode) << 21)+AdrPart;
          CodeLen=1;
         END
       END;
      NextPar=False; return;
     END

   if (Memo("LDP"))
    BEGIN
     if ((ArgCnt!=1) AND (ArgCnt!=2)) WrError(1110);
     else if (ThisPar) WrError(1950);
     else if ((ArgCnt==2) AND (strcasecmp(ArgStr[2],"DP")!=0)) WrError(1350);
     else
      BEGIN
       AdrLong=EvalAdrExpression(ArgStr[1],&OK);
       if (OK)
        BEGIN
         DAsmCode[0]=0x08700000+(AdrLong >> 16);
         CodeLen=1;
        END
      END
     NextPar=False; return;
    END

   /* Schleifen */

   if (Memo("RPTB"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (ThisPar) WrError(1950);
     else
      BEGIN
       AdrLong=EvalAdrExpression(ArgStr[1],&OK);
       if (OK)
        BEGIN
         DAsmCode[0]=0x64000000+AdrLong;
         CodeLen=1;
        END
      END
     NextPar=False; return;
    END

   /* Spruenge */

   if ((Memo("BR")) OR (Memo("BRD")) OR (Memo("CALL")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (ThisPar) WrError(1950);
     else
      BEGIN
       AdrLong=EvalAdrExpression(ArgStr[1],&OK);
       if (OK)
        BEGIN
         DAsmCode[0]=0x60000000+AdrLong;
         if (Memo("BRD")) DAsmCode[0]+=0x01000000;
         else if (Memo("CALL")) DAsmCode[0]+=0x02000000;
         CodeLen=1;
        END
      END
     NextPar=False; return;
    END

   if (*OpPart=='B')
    BEGIN
     strcpy(HOp,OpPart);
     strcpy(OpPart,OpPart+1);
     l=strlen(OpPart);
     if ((l>=1) AND (OpPart[l-1]=='D'))
      BEGIN
       OpPart[l-1]='\0'; DFlag=1l << 21;
       Disp=3;
      END
     else
      BEGIN
       DFlag=0; Disp=1;
      END
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       BEGIN
        if (ArgCnt!=1) WrError(1110);
        else if (ThisPar) WrError(1950);
        else if (DecodeReg(ArgStr[1],&HReg))
         BEGIN
          DAsmCode[0]=0x68000000+(((LongWord)Conditions[z].Code) << 16)+DFlag+HReg;
          CodeLen=1;
         END
        else
         BEGIN
          AdrLong=EvalAdrExpression(ArgStr[1],&OK)-(EProgCounter()+Disp);
          if (OK)
           BEGIN
            if ((NOT SymbolQuestionable) AND ((AdrLong>0x7fffl) OR (AdrLong<-0x8000l))) WrError(1370);
            else
             BEGIN
              DAsmCode[0]=0x6a000000+(((LongWord)Conditions[z].Code) << 16)+DFlag+(AdrLong & 0xffff);
              CodeLen=1;
             END
           END
         END
        NextPar=False; return;
       END
     WrXError(1200,HOp); NextPar=False; return;
    END

   if (strncmp(OpPart,"CALL",4)==0)
    BEGIN
     strcpy(HOp,OpPart); strcpy(OpPart,OpPart+4);
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       BEGIN
        if (ArgCnt!=1) WrError(1110);
        else if (ThisPar) WrError(1950);
        else if (DecodeReg(ArgStr[1],&HReg))
         BEGIN
          DAsmCode[0]=0x70000000+(((LongWord)Conditions[z].Code) << 16)+HReg;
          CodeLen=1;
         END
        else
         BEGIN
          AdrLong=EvalAdrExpression(ArgStr[1],&OK)-(EProgCounter()+1);
          if (OK)
           BEGIN
            if ((NOT SymbolQuestionable) AND ((AdrLong>0x7fffl) OR (AdrLong<-0x8000l))) WrError(1370);
            else
             BEGIN
              DAsmCode[0]=0x72000000+(((LongWord)Conditions[z].Code) << 16)+(AdrLong & 0xffff);
              CodeLen=1;
             END
           END
         END
        NextPar=False; return;
       END
     WrXError(1200,HOp); NextPar=False; return;
    END

   if (strncmp(OpPart,"DB",2)==0)
    BEGIN
     strcpy(HOp,OpPart);
     strcpy(OpPart,OpPart+2);
     l=strlen(OpPart);
     if ((l>=1) AND (OpPart[l-1]=='D'))
      BEGIN
       OpPart[l-1]='\0'; DFlag=1l << 21;
       Disp=3;
      END
     else
      BEGIN
       DFlag=0; Disp=1;
      END
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       BEGIN
        if (ArgCnt!=2) WrError(1110);
        else if (ThisPar) WrError(1950);
        else if (NOT DecodeReg(ArgStr[1],&HReg2)) WrError(1350);
        else if ((HReg2<8) OR (HReg2>15)) WrError(1350);
        else
         BEGIN
          HReg2-=8;
          if (DecodeReg(ArgStr[2],&HReg))
           BEGIN
            DAsmCode[0]=0x6c000000
                       +(((LongWord)Conditions[z].Code) << 16)
                       +DFlag
                       +(((LongWord)HReg2) << 22)
                       +HReg;
            CodeLen=1;
           END
          else
           BEGIN
            AdrLong=EvalAdrExpression(ArgStr[2],&OK)-(EProgCounter()+Disp);
            if (OK)
             BEGIN
              if ((NOT SymbolQuestionable) AND ((AdrLong>0x7fffl) OR (AdrLong<-0x8000l))) WrError(1370);
              else
               BEGIN
                DAsmCode[0]=0x6e000000
                           +(((LongWord)Conditions[z].Code) << 16)
                           +DFlag
                           +(((LongWord)HReg2) << 22)
                           +(AdrLong & 0xffff);
                CodeLen=1;
               END
             END
           END
         END
        NextPar=False; return;
       END
     WrXError(1200,HOp); NextPar=False; return;
    END

   if ((strncmp(OpPart,"RETI",4)==0) OR (strncmp(OpPart,"RETS",4)==0))
    BEGIN
     DFlag=(OpPart[3]=='S')?(1l << 23):(0);
     strcpy(HOp,OpPart); strcpy(OpPart,OpPart+4);
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       BEGIN
        if (ArgCnt!=0) WrError(1110);
        else if (ThisPar) WrError(1950);
        else
         BEGIN
          DAsmCode[0]=0x78000000+DFlag+(((LongWord)Conditions[z].Code) << 16);
          CodeLen=1;
         END
        NextPar=False; return;
       END
     WrXError(1200,HOp); NextPar=False; return;
    END

   if (strncmp(OpPart,"TRAP",4)==0)
    BEGIN
     strcpy(HOp,OpPart); strcpy(OpPart,OpPart+4);
     for (z=0; z<ConditionCount; z++)
      if (Memo(Conditions[z].Name))
       BEGIN
        if (ArgCnt!=1) WrError(1110);
        else if (ThisPar) WrError(1950);
        else
         BEGIN
          HReg=EvalIntExpression(ArgStr[1],UInt4,&OK);
          if (OK)
           BEGIN
            DAsmCode[0]=0x74000000+HReg+(((LongWord)Conditions[z].Code) << 16);
            CodeLen=1;
           END
         END
        NextPar=False; return;
       END
     WrXError(1200,HOp); NextPar=False; return;
    END

   WrXError(1200,OpPart); NextPar=False;
END

        static void InitCode_3203x(void)
BEGIN
   SaveInitProc();
   DPValue=0;
END

        static Boolean IsDef_3203X(void)
BEGIN
   return (strcmp(LabPart,"||")==0);
END

        static void SwitchFrom_3203X(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_3203X(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x76; NOPCode=0x0c800000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=4; ListGrans[SegCode]=4; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffffffl;

   MakeCode=MakeCode_3203X; IsDef=IsDef_3203X;
   SwitchFrom=SwitchFrom_3203X; InitFields(); NextPar=False;
END

        void code3203x_init(void)
BEGIN
   CPU32030=AddCPU("320C30",SwitchTo_3203X);
   CPU32031=AddCPU("320C31",SwitchTo_3203X);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_3203x;
END
