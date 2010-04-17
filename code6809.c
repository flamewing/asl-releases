/* code6809.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator 6809/6309                                                   */
/*                                                                           */
/* Historie: 10.10.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambigious else'-Warnungen beseitigt                 */
/*            3. 1.2001 fixed stack operations pushing/pulling opposite      */
/*                      stack pointer                                        */
/*            5. 1.2001 allow pushing/popping D as A/B                       */
/*           13. 1.2001 fix D register access                                */
/*                                                                           */
/*****************************************************************************/
/* $Id: code6809.c,v 1.5 2010/04/17 13:14:20 alfred Exp $                    */
/*****************************************************************************
 * $Log: code6809.c,v $
 * Revision 1.5  2010/04/17 13:14:20  alfred
 * - address overlapping strcpy()
 *
 * Revision 1.4  2007/11/24 22:48:04  alfred
 * - some NetBSD changes
 *
 * Revision 1.3  2005/09/08 17:31:04  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 12:04:46  alfred
 * - relocated DecodeMot(16)Pseudo into separate module
 *
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "nls.h"
#include "strutil.h"
#include "bpemu.h"

#include "asmdef.h"
#include "asmpars.h"
#include "asmsub.h"
#include "asmitree.h"
#include "codepseudo.h"
#include "motpseudo.h"
#include "codevars.h"

#include "code6809.h"

typedef struct
         {
          char *Name;
          Word Code;
          CPUVar MinCPU;
         } BaseOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Boolean Inv;
          CPUVar MinCPU;
         } FlagOrder;

typedef struct
         {
          char *Name;
          Word Code8;
          Word Code16;
          CPUVar MinCPU;
         } RelOrder;

typedef struct
         {
          char *Name;
          Word Code;
          Byte Op16;
          Boolean MayImm;
          CPUVar MinCPU;
         } ALUOrder;

#define ModNone (-1)
#define ModImm 1
#define ModDir 2
#define ModInd 3
#define ModExt 4

#define FixedOrderCnt 73
#define RelOrderCnt 19
#define ALUOrderCnt 65
#define ALU2OrderCnt 8
#define RMWOrderCnt 13
#define FlagOrderCnt 3
#define LEAOrderCnt 4
#define ImmOrderCnt 4
#define StackOrderCnt 4
#define BitOrderCnt 8

#define StackRegCnt 12
static char StackRegNames[StackRegCnt][4]=
                 {"CCR",  "A",  "B","DPR",  "X",  "Y","S/U", "PC", "CC", "DP",  "S",  "D"};
static Byte StackRegMasks[StackRegCnt]=
                 { 0x01, 0x02, 0x04, 0x08, 0x10, 0x20, 0x40, 0x80, 0x10, 0x08, 0x40, 0x06};

static char *FlagChars="CVZNIHFE";

static ShortInt AdrMode;
static Byte AdrVals[5];
static Byte OpSize;
static Boolean ExtFlag;
static LongInt DPRValue;

static BaseOrder  *FixedOrders;
static RelOrder   *RelOrders;
static ALUOrder   *ALUOrders;
static char **ALU2Orders;
static BaseOrder *RMWOrders;
static FlagOrder *FlagOrders;
static BaseOrder *LEAOrders;
static BaseOrder *ImmOrders;
static BaseOrder *StackOrders;
static char **BitOrders;

static SimpProc SaveInitProc;

static CPUVar CPU6809,CPU6309;

/*-------------------------------------------------------------------------*/
/* Erzeugung/Aufloesung Codetabellen*/

        static void AddFixed(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ++].MinCPU=NCPU;
END

        static void AddRel(char *NName, Word NCode8, Word NCode16)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ].Code8=NCode8;
   RelOrders[InstrZ++].Code16=NCode16;
END

        static void AddALU(char *NName, Word NCode, Byte NSize, Boolean NImm, CPUVar NCPU)
BEGIN
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrders[InstrZ].Name=NName;
   ALUOrders[InstrZ].Code=NCode;
   ALUOrders[InstrZ].Op16=NSize;
   ALUOrders[InstrZ].MayImm=NImm;
   ALUOrders[InstrZ++].MinCPU=NCPU;
END

        static void AddRMW(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=RMWOrderCnt) exit(255);
   RMWOrders[InstrZ].Name=NName;
   RMWOrders[InstrZ].Code=NCode;
   RMWOrders[InstrZ++].MinCPU=NCPU;
END

        static void AddFlag(char *NName, Word NCode, Boolean NInv, CPUVar NCPU)
BEGIN
   if (InstrZ>=FlagOrderCnt) exit(255);
   FlagOrders[InstrZ].Name=NName;
   FlagOrders[InstrZ].Code=NCode;
   FlagOrders[InstrZ].Inv=NInv;
   FlagOrders[InstrZ++].MinCPU=NCPU;
END

        static void AddLEA(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=LEAOrderCnt) exit(255);
   LEAOrders[InstrZ].Name=NName;
   LEAOrders[InstrZ].Code=NCode;
   LEAOrders[InstrZ++].MinCPU=NCPU;
END

        static void AddImm(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Name=NName;
   ImmOrders[InstrZ].Code=NCode;
   ImmOrders[InstrZ++].MinCPU=NCPU;
END

        static void AddStack(char *NName, Word NCode, CPUVar NCPU)
BEGIN
   if (InstrZ>=StackOrderCnt) exit(255);
   StackOrders[InstrZ].Name=NName;
   StackOrders[InstrZ].Code=NCode;
   StackOrders[InstrZ++].MinCPU=NCPU;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("NOP"  ,0x0012,CPU6809); AddFixed("SYNC" ,0x0013,CPU6809);
   AddFixed("DAA"  ,0x0019,CPU6809); AddFixed("SEX"  ,0x001d,CPU6809);
   AddFixed("RTS"  ,0x0039,CPU6809); AddFixed("ABX"  ,0x003a,CPU6809);
   AddFixed("RTI"  ,0x003b,CPU6809); AddFixed("MUL"  ,0x003d,CPU6809);
   AddFixed("SWI2" ,0x103f,CPU6809); AddFixed("SWI3" ,0x113f,CPU6809);
   AddFixed("NEGA" ,0x0040,CPU6809); AddFixed("COMA" ,0x0043,CPU6809);
   AddFixed("LSRA" ,0x0044,CPU6809); AddFixed("RORA" ,0x0046,CPU6809);
   AddFixed("ASRA" ,0x0047,CPU6809); AddFixed("ASLA" ,0x0048,CPU6809);
   AddFixed("LSLA" ,0x0048,CPU6809); AddFixed("ROLA" ,0x0049,CPU6809);
   AddFixed("DECA" ,0x004a,CPU6809); AddFixed("INCA" ,0x004c,CPU6809);
   AddFixed("TSTA" ,0x004d,CPU6809); AddFixed("CLRA" ,0x004f,CPU6809);
   AddFixed("NEGB" ,0x0050,CPU6809); AddFixed("COMB" ,0x0053,CPU6809);
   AddFixed("LSRB" ,0x0054,CPU6809); AddFixed("RORB" ,0x0056,CPU6809);
   AddFixed("ASRB" ,0x0057,CPU6809); AddFixed("ASLB" ,0x0058,CPU6809);
   AddFixed("LSLB" ,0x0058,CPU6809); AddFixed("ROLB" ,0x0059,CPU6809);
   AddFixed("DECB" ,0x005a,CPU6809); AddFixed("INCB" ,0x005c,CPU6809);
   AddFixed("TSTB" ,0x005d,CPU6809); AddFixed("CLRB" ,0x005f,CPU6809);
   AddFixed("PSHSW",0x1038,CPU6309); AddFixed("PULSW",0x1039,CPU6309);
   AddFixed("PSHUW",0x103a,CPU6309); AddFixed("PULUW",0x103b,CPU6309);
   AddFixed("SEXW" ,0x0014,CPU6309); AddFixed("NEGD" ,0x1040,CPU6309);
   AddFixed("COMD" ,0x1043,CPU6309); AddFixed("LSRD" ,0x1044,CPU6309);
   AddFixed("RORD" ,0x1046,CPU6309); AddFixed("ASRD" ,0x1047,CPU6309);
   AddFixed("ASLD" ,0x1048,CPU6309); AddFixed("LSLD" ,0x1048,CPU6309);
   AddFixed("ROLD" ,0x1049,CPU6309); AddFixed("DECD" ,0x104a,CPU6309);
   AddFixed("INCD" ,0x104c,CPU6309); AddFixed("TSTD" ,0x104d,CPU6309);
   AddFixed("CLRD" ,0x104f,CPU6309); AddFixed("COMW" ,0x1053,CPU6309);
   AddFixed("LSRW" ,0x1054,CPU6309); AddFixed("RORW" ,0x1056,CPU6309);
   AddFixed("ROLW" ,0x1059,CPU6309); AddFixed("DECW" ,0x105a,CPU6309);
   AddFixed("INCW" ,0x105c,CPU6309); AddFixed("TSTW" ,0x105d,CPU6309);
   AddFixed("CLRW" ,0x105f,CPU6309); AddFixed("COME" ,0x1143,CPU6309);
   AddFixed("DECE" ,0x114a,CPU6309); AddFixed("INCE" ,0x114c,CPU6309);
   AddFixed("TSTE" ,0x114d,CPU6309); AddFixed("CLRE" ,0x114f,CPU6309);
   AddFixed("COMF" ,0x1153,CPU6309); AddFixed("DECF" ,0x115a,CPU6309);
   AddFixed("INCF" ,0x115c,CPU6309); AddFixed("TSTF" ,0x115d,CPU6309);
   AddFixed("CLRF" ,0x115f,CPU6309); AddFixed("CLRS" ,0x1fd4,CPU6309);
   AddFixed("CLRV" ,0x1fd7,CPU6309); AddFixed("CLRX" ,0x1fd1,CPU6309);
   AddFixed("CLRY" ,0x1fd2,CPU6309);

   RelOrders=(RelOrder *) malloc(sizeof(RelOrder)*RelOrderCnt); InstrZ=0;
   AddRel("BRA",0x0020,0x0016); AddRel("BRN",0x0021,0x1021);
   AddRel("BHI",0x0022,0x1022); AddRel("BLS",0x0023,0x1023);
   AddRel("BHS",0x0024,0x1024); AddRel("BCC",0x0024,0x1024);
   AddRel("BLO",0x0025,0x1025); AddRel("BCS",0x0025,0x1025);
   AddRel("BNE",0x0026,0x1026); AddRel("BEQ",0x0027,0x1027);
   AddRel("BVC",0x0028,0x1028); AddRel("BVS",0x0029,0x1029);
   AddRel("BPL",0x002a,0x102a); AddRel("BMI",0x002b,0x102b);
   AddRel("BGE",0x002c,0x102c); AddRel("BLT",0x002d,0x102d);
   AddRel("BGT",0x002e,0x102e); AddRel("BLE",0x002f,0x102f);
   AddRel("BSR",0x008d,0x0017);

   ALUOrders=(ALUOrder *) malloc(sizeof(ALUOrder)*ALUOrderCnt); InstrZ=0;
   AddALU("LDA" ,0x0086,0,True ,CPU6809);
   AddALU("STA" ,0x0087,0,False,CPU6809);
   AddALU("CMPA",0x0081,0,True ,CPU6809);
   AddALU("ADDA",0x008b,0,True ,CPU6809);
   AddALU("ADCA",0x0089,0,True ,CPU6809);
   AddALU("SUBA",0x0080,0,True ,CPU6809);
   AddALU("SBCA",0x0082,0,True ,CPU6809);
   AddALU("ANDA",0x0084,0,True ,CPU6809);
   AddALU("ORA" ,0x008a,0,True ,CPU6809);
   AddALU("EORA",0x0088,0,True ,CPU6809);
   AddALU("BITA",0x0085,0,True ,CPU6809);

   AddALU("LDB" ,0x00c6,0,True ,CPU6809);
   AddALU("STB" ,0x00c7,0,False,CPU6809);
   AddALU("CMPB",0x00c1,0,True ,CPU6809);
   AddALU("ADDB",0x00cb,0,True ,CPU6809);
   AddALU("ADCB",0x00c9,0,True ,CPU6809);
   AddALU("SUBB",0x00c0,0,True ,CPU6809);
   AddALU("SBCB",0x00c2,0,True ,CPU6809);
   AddALU("ANDB",0x00c4,0,True ,CPU6809);
   AddALU("ORB" ,0x00ca,0,True ,CPU6809);
   AddALU("EORB",0x00c8,0,True ,CPU6809);
   AddALU("BITB",0x00c5,0,True ,CPU6809);

   AddALU("LDD" ,0x00cc,1,True ,CPU6809);
   AddALU("STD" ,0x00cd,1,False,CPU6809);
   AddALU("CMPD",0x1083,1,True ,CPU6809);
   AddALU("ADDD",0x00c3,1,True ,CPU6809);
   AddALU("ADCD",0x1089,1,True ,CPU6309);
   AddALU("SUBD",0x0083,1,True ,CPU6809);
   AddALU("SBCD",0x1082,1,True ,CPU6309);
   AddALU("MULD",0x118f,1,True ,CPU6309);
   AddALU("DIVD",0x118d,1,True ,CPU6309);
   AddALU("ANDD",0x1084,1,True ,CPU6309);
   AddALU("ORD" ,0x108a,1,True ,CPU6309);
   AddALU("EORD",0x1088,1,True ,CPU6309);
   AddALU("BITD",0x1085,1,True ,CPU6309);

   AddALU("LDW" ,0x1086,1,True ,CPU6309);
   AddALU("STW" ,0x1087,1,False,CPU6309);
   AddALU("CMPW",0x1081,1,True ,CPU6309);
   AddALU("ADDW",0x108b,1,True ,CPU6309);
   AddALU("SUBW",0x1080,1,True ,CPU6309);

   AddALU("STQ" ,0x10cd,1,True ,CPU6309);
   AddALU("DIVQ",0x118e,1,True ,CPU6309);

   AddALU("LDE" ,0x1186,0,True ,CPU6309);
   AddALU("STE" ,0x1187,0,False,CPU6309);
   AddALU("CMPE",0x1181,0,True ,CPU6309);
   AddALU("ADDE",0x118b,0,True ,CPU6309);
   AddALU("SUBE",0x1180,0,True ,CPU6309);

   AddALU("LDF" ,0x11c6,0,True ,CPU6309);
   AddALU("STF" ,0x11c7,0,False,CPU6309);
   AddALU("CMPF",0x11c1,0,True ,CPU6309);
   AddALU("ADDF",0x11cb,0,True ,CPU6309);
   AddALU("SUBF",0x11c0,0,True ,CPU6309);

   AddALU("LDX" ,0x008e,1,True ,CPU6809);
   AddALU("STX" ,0x008f,1,False,CPU6809);
   AddALU("CMPX",0x008c,1,True ,CPU6809);

   AddALU("LDY" ,0x108e,1,True ,CPU6809);
   AddALU("STY" ,0x108f,1,False,CPU6809);
   AddALU("CMPY",0x108c,1,True ,CPU6809);

   AddALU("LDU" ,0x00ce,1,True ,CPU6809);
   AddALU("STU" ,0x00cf,1,False,CPU6809);
   AddALU("CMPU",0x1183,1,True ,CPU6809);

   AddALU("LDS" ,0x10ce,1,True ,CPU6809);
   AddALU("STS" ,0x10cf,1,False,CPU6809);
   AddALU("CMPS",0x118c,1,True ,CPU6809);

   AddALU("JSR" ,0x008d,1,False,CPU6809);

   ALU2Orders=(char **) malloc(sizeof(char *)*ALU2OrderCnt);
   ALU2Orders[0]="ADD"; ALU2Orders[1]="ADC";
   ALU2Orders[2]="SUB"; ALU2Orders[3]="SBC";
   ALU2Orders[4]="AND"; ALU2Orders[5]="OR" ;
   ALU2Orders[6]="EOR"; ALU2Orders[7]="CMP";

   RMWOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*RMWOrderCnt); InstrZ=0;
   AddRMW("NEG",0x00,CPU6809);
   AddRMW("COM",0x03,CPU6809);
   AddRMW("LSR",0x04,CPU6809);
   AddRMW("ROR",0x06,CPU6809);
   AddRMW("ASR",0x07,CPU6809);
   AddRMW("ASL",0x08,CPU6809);
   AddRMW("LSL",0x08,CPU6809);
   AddRMW("ROL",0x09,CPU6809);
   AddRMW("DEC",0x0a,CPU6809);
   AddRMW("INC",0x0c,CPU6809);
   AddRMW("TST",0x0d,CPU6809);
   AddRMW("JMP",0x0e,CPU6809);
   AddRMW("CLR",0x0f,CPU6809);

   FlagOrders=(FlagOrder *) malloc(sizeof(FlagOrder)*FlagOrderCnt); InstrZ=0;
   AddFlag("CWAI" ,0x3c,True ,CPU6809);
   AddFlag("ANDCC",0x1c,True ,CPU6809);
   AddFlag("ORCC" ,0x1a,False,CPU6809);

   LEAOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*LEAOrderCnt); InstrZ=0;
   AddLEA("LEAX",0x30,CPU6809);
   AddLEA("LEAY",0x31,CPU6809);
   AddLEA("LEAS",0x32,CPU6809);
   AddLEA("LEAU",0x33,CPU6809);

   ImmOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*ImmOrderCnt); InstrZ=0;
   AddImm("AIM",0x02,CPU6309);
   AddImm("OIM",0x01,CPU6309);
   AddImm("EIM",0x05,CPU6309);
   AddImm("TIM",0x0b,CPU6309);

   StackOrders=(BaseOrder *) malloc(sizeof(BaseOrder)*StackOrderCnt); InstrZ=0;
   AddStack("PSHS",0x34,CPU6809);
   AddStack("PULS",0x35,CPU6809);
   AddStack("PSHU",0x36,CPU6809);
   AddStack("PULU",0x37,CPU6809);

   BitOrders=(char **) malloc(sizeof(char *)*BitOrderCnt);
   BitOrders[0]="BAND"; BitOrders[1]="BIAND";
   BitOrders[2]="BOR";  BitOrders[3]="BIOR" ;
   BitOrders[4]="BEOR"; BitOrders[5]="BIEOR";
   BitOrders[6]="LDBT"; BitOrders[7]="STBT" ;
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(RelOrders);
   free(ALUOrders);
   free(ALU2Orders);
   free(RMWOrders);
   free(FlagOrders);
   free(LEAOrders);
   free(ImmOrders);
   free(StackOrders);
   free(BitOrders);
END

/*-------------------------------------------------------------------------*/


        static Boolean CodeReg(char *ChIn, Byte *erg)
BEGIN
   static char Regs[5]="XYUS",*p;

   if (strlen(ChIn)!=1) return False;
   else
    BEGIN
     p=strchr(Regs,mytoupper(*ChIn));
     if (p==Nil) return False;
     *erg=p-Regs; return True;
    END
END

static void ChkZero(char *s, Byte *Erg)
{
  if (*s == '>')
  {
    strmov(s, s + 1); *Erg = 1;
  }
  else if (*s == '<')
  {
    if (1[s] == '<')
    {
      strmov(s, s + 2); *Erg = 3;
    }
    else
    {
      strmov(s, s + 1); *Erg = 2;
    }
  }
  else
    *Erg = 0;
}

        static Boolean MayShort(Integer Arg)
BEGIN
   return ((Arg>=-128) AND (Arg<127));
END

        static void DecodeAdr(void)
BEGIN
   String Asc,LAsc,temp;
   LongInt AdrLong;
   Word AdrWord;
   Boolean IndFlag,OK;
   Byte EReg,ZeroMode;
   char *p;
   Integer AdrInt;

   AdrMode=ModNone; AdrCnt=0;
   strmaxcpy(Asc,ArgStr[1],255); strmaxcpy(LAsc,ArgStr[ArgCnt],255);

   /* immediate */

   if (*Asc=='#')
    BEGIN
     switch (OpSize)
      BEGIN
       case 2:
        AdrLong=EvalIntExpression(Asc+1,Int32,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=Lo(AdrLong >> 24);
          AdrVals[1]=Lo(AdrLong >> 16);
          AdrVals[2]=Lo(AdrLong >>  8);
          AdrVals[3]=Lo(AdrLong);
          AdrCnt=4;
         END
        break;
       case 1:
        AdrWord=EvalIntExpression(Asc+1,Int16,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=Hi(AdrWord); AdrVals[1]=Lo(AdrWord);
          AdrCnt=2;
         END
        break;
       case 0:
        AdrVals[0]=EvalIntExpression(Asc+1,Int8,&OK);
        if (OK) AdrCnt=1;
        break;
      END
     if (OK) AdrMode=ModImm;
     return;
    END

   /* indirekter Ausdruck ? */

   if ((*Asc=='[') AND (Asc[strlen(Asc)-1]==']'))
    BEGIN
     IndFlag=True; strmov(Asc,Asc+1); Asc[strlen(Asc)-1]='\0';
     ArgCnt=0;
     while (*Asc!='\0')
      BEGIN
       ArgCnt++;
       p=QuotPos(Asc,',');
       if (p!=Nil)
        BEGIN
         *p='\0'; strmaxcpy(ArgStr[ArgCnt],Asc,255); strmov(Asc,p+1);
        END
       else
        BEGIN
         strmaxcpy(ArgStr[ArgCnt],Asc,255); *Asc='\0';
        END
      END
     strmaxcpy(Asc,ArgStr[1],255); strmaxcpy(LAsc,ArgStr[ArgCnt],255);
    END
   else IndFlag=False;

   /* Predekrement ? */

   if ((ArgCnt>=1) AND (ArgCnt<=2) AND (strlen(LAsc)==2) AND (*LAsc=='-') AND (CodeReg(LAsc+1,&EReg)))
    BEGIN
     if ((ArgCnt==2) AND (*Asc!='\0')) WrError(1350);
     else
      BEGIN
       AdrCnt=1; AdrVals[0]=0x82+(EReg << 5)+(Ord(IndFlag) << 4);
       AdrMode=ModInd;
      END
     return;
    END

   if ((ArgCnt>=1) AND (ArgCnt<=2) AND (strlen(LAsc)==3) AND (strncmp(LAsc,"--",2)==0) AND (CodeReg(LAsc+2,&EReg)))
    BEGIN
     if ((ArgCnt==2) AND (*Asc!='\0')) WrError(1350);
     else
      BEGIN
       AdrCnt=1; AdrVals[0]=0x83+(EReg << 5)+(Ord(IndFlag) << 4);
       AdrMode=ModInd;
      END
     return;
    END

   if ((ArgCnt>=1) AND (ArgCnt<=2) AND (strcasecmp(LAsc,"--W")==0))
    BEGIN
     if ((ArgCnt==2) AND (*Asc!='\0')) WrError(1350);
     else if (MomCPU<CPU6309) WrError(1505);
     else
      BEGIN
       AdrCnt=1; AdrVals[0]=0xef+Ord(IndFlag);
       AdrMode=ModInd;
      END
     return;
    END

   /* Postinkrement ? */

   if ((ArgCnt>=1) AND (ArgCnt<=2) AND (strlen(LAsc)==2) AND (LAsc[1]=='+'))
    BEGIN
     temp[0]=(*LAsc); temp[1]='\0';
     if (CodeReg(temp,&EReg))
      BEGIN
       if ((ArgCnt==2) AND (*Asc!='\0')) WrError(1350);
       else
        BEGIN
         AdrCnt=1; AdrVals[0]=0x80+(EReg << 5)+(Ord(IndFlag) << 4);
         AdrMode=ModInd;
        END
       return; 
      END
    END

   if ((ArgCnt>=1) AND (ArgCnt<=2) AND (strlen(LAsc)==3) AND (strncmp(LAsc+1,"++",2)==0))
    BEGIN
     temp[0]=(*LAsc); temp[1]='\0';
     if (CodeReg(temp,&EReg))
      BEGIN
       if ((ArgCnt==2) AND (*Asc!='\0')) WrError(1350);
       else
        BEGIN
         AdrCnt=1; AdrVals[0]=0x81+(EReg << 5)+(Ord(IndFlag) << 4);
         AdrMode=ModInd;
        END
       return;
      END
    END

   if ((ArgCnt>=1) AND (ArgCnt<=2) AND (strcasecmp(LAsc,"W++")==0))
    BEGIN
     if ((ArgCnt==2) AND (*Asc!='\0')) WrError(1350);
     else if (MomCPU<CPU6309) WrError(1505);
     else
      BEGIN
       AdrCnt=1; AdrVals[0]=0xcf+Ord(IndFlag);
       AdrMode=ModInd;
      END
     return;
    END

   /* 16-Bit-Register (mit Index) ? */

   if ((ArgCnt<=2) AND (ArgCnt>=1) AND (CodeReg(LAsc,&EReg)))
    BEGIN
     AdrVals[0]=(EReg << 5)+(Ord(IndFlag) << 4);

     /* nur 16-Bit-Register */

     if (ArgCnt==1)
      BEGIN
       AdrCnt=1; AdrVals[0]+=0x84;
       AdrMode=ModInd; return;
      END

     /* mit Index */

     if (strcasecmp(Asc,"A")==0)
      BEGIN
       AdrCnt=1; AdrVals[0]+=0x86;
       AdrMode=ModInd; return;
      END
     if (strcasecmp(Asc,"B")==0)
      BEGIN
       AdrCnt=1; AdrVals[0]+=0x85;
       AdrMode=ModInd; return;
      END
     if (strcasecmp(Asc,"D")==0)
      BEGIN
       AdrCnt=1; AdrVals[0]+=0x8b;
       AdrMode=ModInd; return;
      END
     if ((strcasecmp(Asc,"E")==0) AND (MomCPU>=CPU6309))
      BEGIN
       if (EReg!=0) WrError(1350);
       else
        BEGIN
         AdrCnt=1; AdrVals[0]+=0x87; AdrMode=ModInd;
        END
       return;
      END
     if ((strcasecmp(Asc,"F")==0) AND (MomCPU>=CPU6309))
      BEGIN
       if (EReg!=0) WrError(1350);
       else
        BEGIN
         AdrCnt=1; AdrVals[0]+=0x8a; AdrMode=ModInd;
        END
       return;
      END
     if ((strcasecmp(Asc,"W")==0) AND (MomCPU>=CPU6309))
      BEGIN
       if (EReg!=0) WrError(1350);
       else
        BEGIN
         AdrCnt=1; AdrVals[0]+=0x8e; AdrMode=ModInd;
        END
       return;
      END

     /* Displacement auswerten */

     ChkZero(Asc,&ZeroMode);
     if (ZeroMode>1)
      BEGIN
       AdrInt=EvalIntExpression(Asc,Int8,&OK);
       if ((FirstPassUnknown) AND (ZeroMode==3)) AdrInt&=0x0f;
      END
     else
      AdrInt=EvalIntExpression(Asc,Int16,&OK);

     /* Displacement 0 ? */

     if ((ZeroMode==0) AND (AdrInt==0))
      BEGIN
       AdrCnt=1; AdrVals[0]+=0x84;
       AdrMode=ModInd; return;
      END

     /* 5-Bit-Displacement */

     else if ((ZeroMode==3) OR ((ZeroMode==0) AND (NOT IndFlag) AND (AdrInt>=-16) AND (AdrInt<=15)))
      BEGIN
       if ((AdrInt<-16) OR (AdrInt>15)) WrError(1340);
       else if (IndFlag) WrError(1350);
       else
        BEGIN
         AdrMode=ModInd;
         AdrCnt=1; AdrVals[0]+=AdrInt & 0x1f;
        END
       return;
      END

     /* 8-Bit-Displacement */

     else if ((ZeroMode==2) OR ((ZeroMode==0) AND (MayShort(AdrInt))))
      BEGIN
       if (NOT MayShort(AdrInt)) WrError(1340);
       else
        BEGIN
         AdrMode=ModInd;
         AdrCnt=2; AdrVals[0]+=0x88; AdrVals[1]=Lo(AdrInt);
        END;
       return;
      END

     /* 16-Bit-Displacement */

     else
      BEGIN
       AdrMode=ModInd;
       AdrCnt=3; AdrVals[0]+=0x89;
       AdrVals[1]=Hi(AdrInt); AdrVals[2]=Lo(AdrInt);
       return;
      END
    END

   if ((ArgCnt<=2) AND (ArgCnt>=1) AND (MomCPU>=CPU6309) AND (strcasecmp(ArgStr[ArgCnt],"W")==0))
    BEGIN
     AdrVals[0]=0x8f+Ord(IndFlag);

     /* nur W-Register */

     if (ArgCnt==1)
      BEGIN
       AdrCnt=1; AdrMode=ModInd; return;
      END

     /* Displacement auswerten */
     ChkZero(Asc,&ZeroMode);
     AdrInt=EvalIntExpression(Asc,Int16,&OK);

     /* Displacement 0 ? */

     if ((ZeroMode==0) AND (AdrInt==0))
      BEGIN
       AdrCnt=1; AdrMode=ModInd; return;
      END

     /* 16-Bit-Displacement */

     else
      BEGIN
       AdrMode=ModInd;
       AdrCnt=3; AdrVals[0]+=0x20;
       AdrVals[1]=Hi(AdrInt); AdrVals[2]=Lo(AdrInt);
       return;
      END
    END

   /* PC-relativ ? */

   if ((ArgCnt==2) AND ((strcasecmp(ArgStr[2],"PCR")==0) OR (strcasecmp(ArgStr[2],"PC")==0)))
    BEGIN
     AdrVals[0]=Ord(IndFlag) << 4;
     ChkZero(Asc,&ZeroMode);
     AdrInt=EvalIntExpression(Asc,Int16,&OK);
     if (OK)
      BEGIN
       AdrInt-=EProgCounter()+3+Ord(ExtFlag);

       if (ZeroMode==3) WrError(1350);

       else if ((ZeroMode==2) OR ((ZeroMode==0) AND MayShort(AdrInt)))
        BEGIN
         if (NOT MayShort(AdrInt)) WrError(1320);
         else
          BEGIN
           AdrCnt=2; AdrVals[0]+=0x8c;
           AdrVals[1]=Lo(AdrInt);
           AdrMode=ModInd;
          END
        END

       else
        BEGIN
         AdrInt--;
         AdrCnt=3; AdrVals[0]+=0x8d;
         AdrVals[1]=Hi(AdrInt); AdrVals[2]=Lo(AdrInt);
         AdrMode=ModInd;
        END
      END
     return;
    END

   if (ArgCnt==1)
    BEGIN
     ChkZero(Asc,&ZeroMode);
     FirstPassUnknown=False;
     AdrInt=EvalIntExpression(Asc,Int16,&OK);
     if ((FirstPassUnknown) AND (ZeroMode==2))
      AdrInt=(AdrInt & 0xff)| (DPRValue << 8);

     if (OK)
      BEGIN
       if (ZeroMode==3) WrError(1350);

       else if ((ZeroMode==2) OR ((ZeroMode==0) AND (Hi(AdrInt)==DPRValue) AND (NOT IndFlag)))
        BEGIN
         if (IndFlag) WrError(1990);
         else if (Hi(AdrInt)!=DPRValue) WrError(1340);
         else
          BEGIN
           AdrCnt=1; AdrMode=ModDir; AdrVals[0]=Lo(AdrInt);
          END
        END

       else
        BEGIN
         if (IndFlag)
          BEGIN
           AdrMode=ModInd; AdrCnt=3; AdrVals[0]=0x9f;
           AdrVals[1]=Hi(AdrInt); AdrVals[2]=Lo(AdrInt);
          END
         else
          BEGIN
           AdrMode=ModExt; AdrCnt=2;
           AdrVals[0]=Hi(AdrInt); AdrVals[1]=Lo(AdrInt);
          END
        END
      END
     return;
    END

   if (AdrMode==ModNone) WrError(1350);
END

        static Boolean CodeCPUReg(char *Asc, Byte *Erg)
BEGIN
#define RegCnt 18
   static char *RegNames[RegCnt]={"D","X","Y","U","S","SP","PC","W","V","A","B","CCR","DPR","CC","DP","Z","E","F"};
   static Byte RegVals[RegCnt]  ={0  ,1  ,2  ,3  ,4  ,4   ,5   ,6  ,7  ,8  ,9  ,10   ,11   ,10  ,11  ,13 ,14 ,15 };

   int z;
   String Asc_N;

   strmaxcpy(Asc_N,Asc,255); NLS_UpString(Asc_N); Asc=Asc_N;

   for (z=0; z<RegCnt; z++)
    if (strcmp(Asc,RegNames[z])==0)
     BEGIN
      if (((RegVals[z] & 6)==6) AND (MomCPU<CPU6309)) WrError(1505);
      else
       BEGIN
        *Erg=RegVals[z]; return True;
       END
     END
   return False;
END

        static Boolean DecodePseudo(void)
BEGIN
#define ASSUME09Count 1
static ASSUMERec ASSUME09s[ASSUME09Count]=
             {{"DPR", &DPRValue, 0, 0xff, 0x100}};

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME09s,ASSUME09Count);
     return True;
    END

   return False;
END

        static void SplitPM(char *s, int *Erg)
BEGIN
   int l=strlen(s);

   if (l==0) *Erg=0;
   else if (s[l-1]=='+')
    BEGIN
     s[l-1]='\0'; *Erg=1;
    END
   else if (s[l-1]=='-')
    BEGIN
     s[l-1]='\0'; *Erg=(-1);
    END
   else *Erg=0;
END

        static Boolean SplitBit(char *Asc, int *Erg)
BEGIN
   char *p;
   Boolean OK;

   p=QuotPos(Asc,'.');
   if (p==Nil)
    BEGIN
     WrError(1510); return False;
    END
   *Erg=EvalIntExpression(p+1,UInt3,&OK);
   if (NOT OK) return False;
   *p='\0';
   return True;
END

        static void MakeCode_6809(void)
BEGIN
   char *p;
   int z,z2,z3;
   Integer AdrInt;
   Boolean LongFlag,OK,Extent;

   CodeLen=0; DontPrint=False; OpSize=0; ExtFlag=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeMotoPseudo(True)) return;

   /* Anweisungen ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if Memo(FixedOrders[z].Name)
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if (MomCPU<FixedOrders[z].MinCPU) WrError(1500);
      else if (Hi(FixedOrders[z].Code)==0)
       BEGIN
        BAsmCode[0]=Lo(FixedOrders[z].Code); CodeLen=1;
       END
      else
       BEGIN
        BAsmCode[0]=Hi(FixedOrders[z].Code);
        BAsmCode[1]=Lo(FixedOrders[z].Code); CodeLen=2;
       END
      return;
     END;

   /* Specials... */

   if (Memo("SWI"))
    BEGIN
     if (ArgCnt==0)
      BEGIN
       BAsmCode[0]=0x3f; CodeLen=1;
      END
     else if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"2")==0)
      BEGIN
       BAsmCode[0]=0x10; BAsmCode[1]=0x3f; CodeLen=2;
      END
     else if (strcasecmp(ArgStr[1],"3")==0)
      BEGIN
       BAsmCode[0]=0x11; BAsmCode[1]=0x3f; CodeLen=2;
      END
     else WrError(1135);
     return;
    END

   /* relative Spruenge */

   for (z=0; z<RelOrderCnt; z++)
    if ((Memo(RelOrders[z].Name)) OR ((*OpPart=='L') AND (strcmp(OpPart+1,RelOrders[z].Name)==0)))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        LongFlag=(*OpPart=='L'); ExtFlag=(LongFlag) AND (Hi(RelOrders[z].Code16)!=0);
        AdrInt=EvalIntExpression(ArgStr[1],UInt16,&OK);
        if (OK)
         BEGIN
          AdrInt-=EProgCounter()+2+Ord(LongFlag)+Ord(ExtFlag);
          if ((NOT SymbolQuestionable) AND (NOT LongFlag) AND ((AdrInt<-128) OR (AdrInt>127))) WrError(1370);
          else
           BEGIN
            CodeLen=1+Ord(ExtFlag);
            if (LongFlag)
             if (ExtFlag)
              BEGIN
               BAsmCode[0]=Hi(RelOrders[z].Code16); BAsmCode[1]=Lo(RelOrders[z].Code16);
              END
             else BAsmCode[0]=Lo(RelOrders[z].Code16);
            else BAsmCode[0]=Lo(RelOrders[z].Code8);
            if (LongFlag)
             BEGIN
              BAsmCode[CodeLen]=Hi(AdrInt); BAsmCode[CodeLen+1]=Lo(AdrInt);
              CodeLen+=2;
             END
            else
             BEGIN
              BAsmCode[CodeLen]=Lo(AdrInt);
              CodeLen++;
             END
           END
         END
       END
      return;
     END

   /* ALU-Operationen */

   for (z=0; z<ALUOrderCnt; z++)
    if Memo(ALUOrders[z].Name)
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else if (MomCPU<ALUOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        OpSize=ALUOrders[z].Op16; ExtFlag=(Hi(ALUOrders[z].Code)!=0);
        DecodeAdr();
        if (AdrMode!=ModNone)
         BEGIN
          if ((NOT ALUOrders[z].MayImm) AND (AdrMode==ModImm)) WrError(1350);
          else
           BEGIN
            CodeLen=Ord(ExtFlag)+1+AdrCnt;
            if (ExtFlag) BAsmCode[0]=Hi(ALUOrders[z].Code);
            BAsmCode[Ord(ExtFlag)]=Lo(ALUOrders[z].Code)+((AdrMode-1) << 4);
            memcpy(BAsmCode+1+Ord(ExtFlag),AdrVals,AdrCnt);
           END
         END
       END
      return;
     END

    if (Memo("LDQ"))
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else if (MomCPU<CPU6309) WrError(1500);
      else
       BEGIN
        OpSize=2;
        DecodeAdr();
        if (AdrMode==ModImm)
         BEGIN
          BAsmCode[0]=0xcd; memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
         END
        else
         BEGIN
          BAsmCode[0]=0x10; BAsmCode[1]=0xcc+((AdrMode-1) << 4);
          CodeLen=2+AdrCnt;
          memcpy(BAsmCode+2,AdrVals,AdrCnt);
         END
       END
      return;
     END

   /* Read-Modify-Write-Operationen */

   for (z=0; z<RMWOrderCnt; z++)
    if Memo(RMWOrders[z].Name)
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else if (MomCPU<RMWOrders[z].MinCPU) WrError(1500);
      else
       BEGIN
        DecodeAdr();
        if (AdrMode!=ModNone)
         BEGIN
          if (AdrMode==ModImm) WrError(1350);
          else
           BEGIN
            CodeLen=1+AdrCnt;
            switch (AdrMode)
             BEGIN
              case ModDir:BAsmCode[0]=RMWOrders[z].Code; break;
              case ModInd:BAsmCode[0]=RMWOrders[z].Code+0x60; break;
              case ModExt:BAsmCode[0]=RMWOrders[z].Code+0x70; break;
             END
            memcpy(BAsmCode+1,AdrVals,AdrCnt);
           END
         END
       END
      return;
     END

   /* Anweisungen mit Flag-Operand */

   for (z=0; z<FlagOrderCnt; z++)
    if (Memo(FlagOrders[z].Name))
     BEGIN
      if (ArgCnt<1) WrError(1110);
      else
       BEGIN
        OK=True;
        if (FlagOrders[z].Inv) BAsmCode[1]=0xff; else BAsmCode[1]=0x00;
        for (z2=1; z2<=ArgCnt; z2++)
         if (OK)
          BEGIN
           p=(strlen(ArgStr[z2])==1)?strchr(FlagChars,mytoupper(*ArgStr[z2])):Nil;
           if (p!=Nil)
            BEGIN
             z3=p-FlagChars;
             if (FlagOrders[z].Inv) BAsmCode[1]&=(0xff^(1 << z3));
             else BAsmCode[1]|=(1 << z3);
            END
           else if (*ArgStr[z2]!='#')
            BEGIN
             WrError(1120); OK=False;
            END
           else
            BEGIN
             BAsmCode[2]=EvalIntExpression(ArgStr[z2]+1,Int8,&OK);
             if (OK)
              BEGIN
               if (FlagOrders[z].Inv) BAsmCode[1]&=BAsmCode[2];
               else BAsmCode[1]|=BAsmCode[2];
              END
            END
          END
        if (OK)
         BEGIN
          CodeLen=2; BAsmCode[0]=FlagOrders[z].Code;
         END
       END
      return;
     END

   /* Bit-Befehle */

   for (z=0; z<ImmOrderCnt; z++)
    if (Memo(ImmOrders[z].Name))
     BEGIN
      if ((ArgCnt!=2) AND (ArgCnt!=3)) WrError(1110);
      else if (MomCPU<ImmOrders[z].MinCPU) WrError(1500);
      else if (*ArgStr[1]!='#') WrError(1120);
      else
       BEGIN
        BAsmCode[1]=EvalIntExpression(ArgStr[1]+1,Int8,&OK);
        if (OK)
         BEGIN
          for (z2=1; z2<ArgCnt; z2++) strcpy(ArgStr[z2],ArgStr[z2+1]);
          ArgCnt--; DecodeAdr();
          if (AdrMode==ModImm) WrError(1350);
          else
           BEGIN
            switch (AdrMode)
             BEGIN
              case ModDir:BAsmCode[0]=ImmOrders[z].Code; break;
              case ModExt:BAsmCode[0]=ImmOrders[z].Code+0x70; break;
              case ModInd:BAsmCode[0]=ImmOrders[z].Code+0x60; break;
             END
            memcpy(BAsmCode+2,AdrVals,AdrCnt);
            CodeLen=2+AdrCnt;
           END
         END
       END
      return;
     END

   for (z=0; z<BitOrderCnt; z++)
    if (Memo(BitOrders[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (MomCPU<CPU6309) WrError(1500);
      else
       if (SplitBit(ArgStr[1],&z2))
        if (SplitBit(ArgStr[2],&z3))
         BEGIN
          if (NOT CodeCPUReg(ArgStr[1],BAsmCode+2)) WrError(1980);
          else if ((BAsmCode[2]<8) OR (BAsmCode[2]>11)) WrError(1980);
          else
           BEGIN
            strcpy(ArgStr[1],ArgStr[2]); ArgCnt=1; DecodeAdr();
            if (AdrMode!=ModDir) WrError(1350);
            else
             BEGIN
              BAsmCode[2]-=7;
              if (BAsmCode[2]==3) BAsmCode[2]=0;
              BAsmCode[0]=0x11; BAsmCode[1]=0x30+z;
              BAsmCode[2]=(BAsmCode[2] << 6)+(z3 << 3)+z2;
              BAsmCode[3]=AdrVals[0];
              CodeLen=4;
             END
           END
         END
      return;
     END

   /* Register-Register-Operationen */

   if ((Memo("TFR")) OR (Memo("TFM")) OR (Memo("EXG")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       SplitPM(ArgStr[1],&z2); SplitPM(ArgStr[2],&z3);
       if ((z2!=0) OR (z3!=0))
        BEGIN
         if (Memo("EXG")) WrError(1350);
         else if (NOT CodeCPUReg(ArgStr[1],BAsmCode+3)) WrError(1980);
         else if (NOT CodeCPUReg(ArgStr[2],BAsmCode+2)) WrError(1980);
         else if ((BAsmCode[2]<1) OR (BAsmCode[2]>4)) WrError(1980);
         else if ((BAsmCode[3]<1) OR (BAsmCode[3]>4)) WrError(1980);
         else
          BEGIN
           BAsmCode[0]=0x11; BAsmCode[1]=0;
           BAsmCode[2]+=BAsmCode[3] << 4;
           if ((z2==1) AND (z3==1)) BAsmCode[1]=0x38;
           else if ((z2==-1) AND (z3==-1)) BAsmCode[1]=0x39;
           else if ((z2== 1) AND (z3== 0)) BAsmCode[1]=0x3a;
           else if ((z2== 0) AND (z3== 1)) BAsmCode[1]=0x3b;
           if (BAsmCode[1]==0) WrError(1350); else CodeLen=3;
          END
        END
       else if (Memo("TFM")) WrError(1350);
       else if (NOT CodeCPUReg(ArgStr[1],BAsmCode+2)) WrError(1980);
       else if (NOT CodeCPUReg(ArgStr[2],BAsmCode+1)) WrError(1980);
       else if ((BAsmCode[1]!=13) AND (BAsmCode[2]!=13) AND  /* Z-Register mit allen kompatibel */
                (((BAsmCode[1] ^ BAsmCode[2]) & 0x08)!=0)) WrError(1131);
       else
        BEGIN
         CodeLen=2;
         BAsmCode[0]=0x1e + Ord(Memo("TFR"));
         BAsmCode[1]+=BAsmCode[2] << 4;
        END
      END
     return;
    END

   for (z=0; z<ALU2OrderCnt; z++)
    if ((strncmp(OpPart,ALU2Orders[z],strlen(ALU2Orders[z]))==0) AND ((OpPart[strlen(OpPart)]=='\0') OR (OpPart[strlen(OpPart)-1]=='R')))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (NOT CodeCPUReg(ArgStr[1],BAsmCode+3)) WrError(1980);
      else if (NOT CodeCPUReg(ArgStr[2],BAsmCode+2)) WrError(1980);
      else if ((BAsmCode[1]!=13) AND (BAsmCode[2]!=13) AND  /* Z-Register mit allen kompatibel */
               (((BAsmCode[2] ^ BAsmCode[3]) & 0x08)!=0)) WrError(1131);
      else
       BEGIN
        CodeLen=3;
        BAsmCode[0]=0x10;
        BAsmCode[1]=0x30+z;
        BAsmCode[2]+=BAsmCode[3] << 4;
       END
      return;
     END

   /* Berechnung effektiver Adressen */

   for (z=0; z<LEAOrderCnt; z++)
    if Memo(LEAOrders[z].Name)
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else
       BEGIN
        DecodeAdr();
        if (AdrMode!=ModNone)
         BEGIN
          if (AdrMode!=ModInd) WrError(1350);
          else
           BEGIN
            CodeLen=1+AdrCnt;
            BAsmCode[0]=LEAOrders[z].Code;
            memcpy(BAsmCode+1,AdrVals,AdrCnt);
           END
         END
       END
      return;
     END

   /* Push/Pull */

   for (z = 0; z < StackOrderCnt; z++)
    if Memo(StackOrders[z].Name)
     BEGIN
      BAsmCode[1] = 0; OK = True; Extent = False;
      /* S oder U einsetzen, entsprechend Opcode */
      *StackRegNames[StackRegCnt - 2] =
       OpPart[strlen(OpPart) - 1] ^ ('S' ^ 'U');
      for (z2 = 1; z2 <= ArgCnt; z2++)
       if (OK)
        BEGIN
         if (strcasecmp(ArgStr[z2], "W") == 0)
          BEGIN
           if (MomCPU < CPU6309)
            BEGIN
             WrError(1500); OK = False;
            END
           else if (ArgCnt != 1)
            BEGIN
             WrError(1335); OK = False;
            END
           else Extent = True;
          END
         else
          BEGIN
           for (z3 = 0; z3 < StackRegCnt; z3++)
            if (strcasecmp(ArgStr[z2], StackRegNames[z3]) == 0)
             BEGIN
              BAsmCode[1] |= StackRegMasks[z3];
              break;
             END
           if (z3 >= StackRegCnt)
            BEGIN
             if (strcasecmp(ArgStr[z2], "ALL") == 0) BAsmCode[1] = 0xff;
             else if (*ArgStr[z2] != '#') OK = False;
             else
              BEGIN
               BAsmCode[2] = EvalIntExpression(ArgStr[z2] + 1, Int8, &OK);
               if (OK) BAsmCode[1] |= BAsmCode[2];
              END
            END
          END
       END
      if (OK)
       if (Extent)
        BEGIN
         CodeLen = 2; BAsmCode[0] = 0x10; BAsmCode[1] = StackOrders[z].Code + 4;
        END
       else
        BEGIN
         CodeLen = 2; BAsmCode[0] = StackOrders[z].Code;
        END
      else WrError(1980);
      return;
     END

   if ((Memo("BITMD")) OR (Memo("LDMD")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (MomCPU<CPU6309) WrError(1500);
     else if (*ArgStr[1]!='#') WrError(1120);
     else
      BEGIN
       BAsmCode[2]=EvalIntExpression(ArgStr[1]+1,Int8,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]=0x11;
         BAsmCode[1]=0x3c+Ord(Memo("LDMD"));
         CodeLen=3;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static void InitCode_6809()
BEGIN
   SaveInitProc();
   DPRValue=0;
END

        static Boolean IsDef_6809(void)
BEGIN
   return False;
END

        static void SwitchFrom_6809(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_6809(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x63; NOPCode=0x9d;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;

   MakeCode=MakeCode_6809; IsDef=IsDef_6809;

   SwitchFrom=SwitchFrom_6809; InitFields();
END

        void code6809_init(void)
BEGIN
   CPU6809=AddCPU("6809",SwitchTo_6809);
   CPU6309=AddCPU("6309",SwitchTo_6809);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_6809;
END
