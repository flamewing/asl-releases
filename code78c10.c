/* code78c10.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator NEC uPD78(C)1x                                              */
/*                                                                           */
/* Historie: 29.12.1996 Grundsteinlegung                                     */
/*            2. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*           24.10.2000 fixed some errors (MOV A<>mem/DCRW/ETMM/PUSH V/POP V)*/
/*           25.10.2000 accesses wrong argument for mov nnn,a                */
/*                                                                           */
/*****************************************************************************/
/* $Id: code78c10.c,v 1.3 2005/09/08 17:31:04 alfred Exp $                   */
/*****************************************************************************
 * $Log: code78c10.c,v $
 * Revision 1.3  2005/09/08 17:31:04  alfred
 * - add missing include
 *
 * Revision 1.2  2004/05/29 11:33:01  alfred
 * - relocated DecodeIntelPseudo() into own module
 *
 *****************************************************************************/

#include "stdinc.h"
#include <ctype.h>
#include <string.h>

#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmitree.h"  
#include "codepseudo.h"
#include "intpseudo.h"
#include "codevars.h"

/*---------------------------------------------------------------------------*/

typedef struct
         {
          char *Name;
          Word Code;
         } FixedOrder;

typedef struct
         {
          char *Name;
          Byte Code;
         } SReg;

#define FixedOrderCnt 23
#define ALUOrderCnt 15
#define AbsOrderCnt 10
#define Reg2OrderCnt 10
#define WorkOrderCnt 4
#define EAOrderCnt 4
#define SRegCnt 28


static LongInt WorkArea;

static SimpProc SaveInitProc;

static CPUVar CPU7810,CPU78C10;

static FixedOrder *FixedOrders;
static Byte *ALUOrderCodes;
static char **ALUOrderImmOps,**ALUOrderRegOps,**ALUOrderEAOps;
static FixedOrder *AbsOrders;
static FixedOrder *Reg2Orders;
static FixedOrder *WorkOrders;
static FixedOrder *EAOrders;
static SReg *SRegs;

/*--------------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddSReg(char *NName, Word NCode)
BEGIN
   if (InstrZ>=SRegCnt) exit(255);
   SRegs[InstrZ].Name=NName;
   SRegs[InstrZ++].Code=NCode;
END

        static void AddALU(Byte NCode, char *NName1, char *NName2, char *NName3)
BEGIN
   if (InstrZ>=ALUOrderCnt) exit(255);
   ALUOrderCodes[InstrZ]=NCode;
   ALUOrderImmOps[InstrZ]=NName1;
   ALUOrderRegOps[InstrZ]=NName2;
   ALUOrderEAOps[InstrZ++]=NName3;
END

        static void AddAbs(char *NName, Word NCode)
BEGIN
   if (InstrZ>=AbsOrderCnt) exit(255);
   AbsOrders[InstrZ].Name=NName;
   AbsOrders[InstrZ++].Code=NCode;
END

        static void AddReg2(char *NName, Word NCode)
BEGIN
   if (InstrZ>=Reg2OrderCnt) exit(255);
   Reg2Orders[InstrZ].Name=NName;
   Reg2Orders[InstrZ++].Code=NCode;
END

        static void AddWork(char *NName, Word NCode)
BEGIN
   if (InstrZ>=WorkOrderCnt) exit(255);
   WorkOrders[InstrZ].Name=NName;
   WorkOrders[InstrZ++].Code=NCode;
END

        static void AddEA(char *NName, Word NCode)
BEGIN
   if (InstrZ>=EAOrderCnt) exit(255);
   EAOrders[InstrZ].Name=NName;
   EAOrders[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("EXX"  , 0x0011); AddFixed("EXA"  , 0x0010);
   AddFixed("EXH"  , 0x0050); AddFixed("BLOCK", 0x0031);
   AddFixed("TABLE", 0x48a8); AddFixed("DAA"  , 0x0061);
   AddFixed("STC"  , 0x482b); AddFixed("CLC"  , 0x482a);
   AddFixed("NEGA" , 0x483a); AddFixed("RLD"  , 0x4838);
   AddFixed("RRD"  , 0x4839); AddFixed("JB"   , 0x0021);
   AddFixed("JEA"  , 0x4828); AddFixed("CALB" , 0x4829);
   AddFixed("SOFTI", 0x0072); AddFixed("RET"  , 0x00b8);
   AddFixed("RETS" , 0x00b9); AddFixed("RETI" , 0x0062);
   AddFixed("NOP"  , 0x0000); AddFixed("EI"   , 0x00aa);
   AddFixed("DI"   , 0x00ba); AddFixed("HLT"  , 0x483b);
   AddFixed("STOP" , 0x48bb);

   SRegs=(SReg *) malloc(sizeof(SReg)*SRegCnt); InstrZ=0;
   AddSReg("PA"  , 0x00); AddSReg("PB"  , 0x01);
   AddSReg("PC"  , 0x02); AddSReg("PD"  , 0x03);
   AddSReg("PF"  , 0x05); AddSReg("MKH" , 0x06);
   AddSReg("MKL" , 0x07); AddSReg("ANM" , 0x08);
   AddSReg("SMH" , 0x09); AddSReg("SML" , 0x0a);
   AddSReg("EOM" , 0x0b); AddSReg("ETMM", 0x0c);
   AddSReg("TMM" , 0x0d); AddSReg("MM"  , 0x10);
   AddSReg("MCC" , 0x11); AddSReg("MA"  , 0x12);
   AddSReg("MB"  , 0x13); AddSReg("MC"  , 0x14);
   AddSReg("MF"  , 0x17); AddSReg("TXB" , 0x18);
   AddSReg("RXB" , 0x19); AddSReg("TM0" , 0x1a);
   AddSReg("TM1" , 0x1b); AddSReg("CR0" , 0x20);
   AddSReg("CR1" , 0x21); AddSReg("CR2" , 0x22);
   AddSReg("CR3" , 0x23); AddSReg("ZCM" , 0x28);

   ALUOrderCodes=(Byte *) malloc(sizeof(Byte)*ALUOrderCnt);
   ALUOrderImmOps=(char **) malloc(sizeof(char *)*ALUOrderCnt);
   ALUOrderRegOps=(char **) malloc(sizeof(char *)*ALUOrderCnt);
   ALUOrderEAOps=(char **) malloc(sizeof(char *)*ALUOrderCnt); InstrZ=0;
   AddALU(10,"ACI"  ,"ADC"  ,"DADC"  );
   AddALU( 4,"ADINC","ADDNC","DADDNC");
   AddALU( 8,"ADI"  ,"ADD"  ,"DADD"  );
   AddALU( 1,"ANI"  ,"ANA"  ,"DAN"   );
   AddALU(15,"EQI"  ,"EQA"  ,"DEQ"   );
   AddALU( 5,"GTI"  ,"GTA"  ,"DGT"   );
   AddALU( 7,"LTI"  ,"LTA"  ,"DLT"   );
   AddALU(13,"NEI"  ,"NEA"  ,"DNE"   );
   AddALU(11,"OFFI" ,"OFFA" ,"DOFF"  );
   AddALU( 9,"ONI"  ,"ONA"  ,"DON"   );
   AddALU( 3,"ORI"  ,"ORA"  ,"DOR"   );
   AddALU(14,"SBI"  ,"SBB"  ,"DSBB"  );
   AddALU( 6,"SUINB","SUBNB","DSUBNB");
   AddALU(12,"SUI"  ,"SUB"  ,"DSUB"  );
   AddALU( 2,"XRI"  ,"XRA"  ,"DXR"   );

   AbsOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*AbsOrderCnt); InstrZ=0;
   AddAbs("CALL", 0x0040); AddAbs("JMP" , 0x0054);
   AddAbs("LBCD", 0x701f); AddAbs("LDED", 0x702f);
   AddAbs("LHLD", 0x703f); AddAbs("LSPD", 0x700f);
   AddAbs("SBCD", 0x701e); AddAbs("SDED", 0x702e);
   AddAbs("SHLD", 0x703e); AddAbs("SSPD", 0x700e);

   Reg2Orders=(FixedOrder *) malloc(sizeof(FixedOrder)*Reg2OrderCnt); InstrZ=0;
   AddReg2("DCR" , 0x0050); AddReg2("DIV" , 0x483c);
   AddReg2("INR" , 0x0040); AddReg2("MUL" , 0x482c);
   AddReg2("RLL" , 0x4834); AddReg2("RLR" , 0x4830);
   AddReg2("SLL" , 0x4824); AddReg2("SLR" , 0x4820);
   AddReg2("SLLC", 0x4804); AddReg2("SLRC", 0x4800);

   WorkOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*WorkOrderCnt); InstrZ=0;
   AddWork("DCRW", 0x30); AddWork("INRW", 0x20);
   AddWork("LDAW", 0x01); AddWork("STAW", 0x63);

   EAOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*EAOrderCnt); InstrZ=0;
   AddEA("DRLL", 0x48b4); AddEA("DRLR", 0x48b0);
   AddEA("DSLL", 0x48a4); AddEA("DSLR", 0x48a0);
END

        static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(ALUOrderCodes); free(ALUOrderImmOps); free(ALUOrderRegOps); free(ALUOrderEAOps);
   free(AbsOrders);
   free(Reg2Orders);
   free(WorkOrders);
   free(EAOrders);
   free(SRegs);
END

/*--------------------------------------------------------------------------------*/

        static Boolean Decode_r(char *Asc, ShortInt *Erg)
BEGIN
   static char *Names="VABCDEHL";
   char *p;

   if (strlen(Asc)!=1) return False;
   p=strchr(Names,toupper(*Asc));
   if (p==Nil) return False;
   *Erg=p-Names; return True;;
END

        static Boolean Decode_r1(char *Asc, ShortInt *Erg)
BEGIN
   if (strcasecmp(Asc,"EAL")==0) *Erg=1;
   else if (strcasecmp(Asc,"EAH")==0) *Erg=0;
   else
    BEGIN
     if (NOT Decode_r(Asc,Erg)) return False;
     return (*Erg>1);
    END
   return True;
END

        static Boolean Decode_r2(char *Asc, ShortInt *Erg)
BEGIN
   if (NOT Decode_r(Asc,Erg)) return False;
   return ((*Erg>0) AND (*Erg<4));
END

        static Boolean Decode_rp2(char *Asc, ShortInt *Erg)
BEGIN
#define RegCnt 5
   static char *Regs[RegCnt]={"SP","B","D","H","EA"};

   for (*Erg=0; *Erg<RegCnt; (*Erg)++)
    if (strcasecmp(Asc,Regs[*Erg])==0) break;
   return (*Erg<RegCnt);
END

        static Boolean Decode_rp(char *Asc, ShortInt *Erg)
BEGIN
   if (NOT Decode_rp2(Asc,Erg)) return False;
   return (*Erg<4);
END

        static Boolean Decode_rp1(char *Asc, ShortInt *Erg)
BEGIN
   if (strcasecmp(Asc,"V")==0) *Erg=0;
   else
    BEGIN
     if (NOT Decode_rp2(Asc,Erg)) return False;
     return (*Erg!=0);
    END
   return True;
END

        static Boolean Decode_rp3(char *Asc, ShortInt *Erg)
BEGIN
   if (NOT Decode_rp2(Asc,Erg)) return False;
   return ((*Erg<4) AND (*Erg>0));
END

        static Boolean Decode_rpa2(char *Asc, ShortInt *Erg, ShortInt *Disp)
BEGIN
#define OpCnt 13
   static char *OpNames[OpCnt]={"B","D","H","D+","H+","D-","H-",
                                   "H+A","A+H","H+B","B+H","H+EA","EA+H"};
   static Byte OpCodes[OpCnt]={1,2,3,4,5,6,7,12,12,13,13,14,14};

   int z;
   char *p,*pm;
   Boolean OK;

   for (z=0; z<OpCnt; z++)
    if (strcasecmp(Asc,OpNames[z])==0)
     BEGIN
      *Erg=OpCodes[z]; return True;
     END

   p=QuotPos(Asc,'+'); pm=QuotPos(Asc,'-');
   if ((p==Nil) OR ((pm!=Nil) AND (pm<p))) p=pm;
   if (p==Nil) return False;

   if (p==Asc+1)
    switch (toupper(*Asc))
     BEGIN
      case 'H': *Erg=15; break;
      case 'D': *Erg=11; break;
      default: return False;
     END
   else return False;
   *Disp=EvalIntExpression(p,SInt8,&OK);
   return OK;
END

        static Boolean Decode_rpa(char *Asc, ShortInt *Erg)
BEGIN
   ShortInt Dummy;

   if (NOT Decode_rpa2(Asc,Erg,&Dummy)) return False;
   return (*Erg<=7);
END

        static Boolean Decode_rpa1(char *Asc, ShortInt *Erg)
BEGIN
   ShortInt Dummy;

   if (NOT Decode_rpa2(Asc,Erg,&Dummy)) return False;
   return (*Erg<=3);
END

        static Boolean Decode_rpa3(char *Asc, ShortInt *Erg, ShortInt *Disp)
BEGIN
   if (strcasecmp(Asc,"D++")==0) *Erg=4;
   else if (strcasecmp(Asc,"H++")==0) *Erg=5;
   else
    BEGIN
     if (NOT Decode_rpa2(Asc,Erg,Disp)) return False;
     return ((*Erg==2) OR (*Erg==3) OR (*Erg>=8));
    END
   return True;
END

        static Boolean Decode_f(char *Asc, ShortInt *Erg)
BEGIN
#define FlagCnt 3
   static char *Flags[FlagCnt]={"CY","HC","Z"};

   for (*Erg=0; *Erg<FlagCnt; (*Erg)++)
    if (strcasecmp(Flags[*Erg],Asc)==0) break;
   *Erg+=2; return (*Erg<=4);
END

        static Boolean Decode_sr0(char *Asc, ShortInt *Erg)
BEGIN
   int z;

   for (z=0; z<SRegCnt; z++)
    if (strcasecmp(Asc,SRegs[z].Name)==0) break;
   if ((z==SRegCnt-1) AND (MomCPU==CPU7810))
    BEGIN
     WrError(1440); return False;
    END
   if (z<SRegCnt)
    BEGIN
     *Erg=SRegs[z].Code; return True;
    END
   else return False;
END

        static Boolean Decode_sr1(char *Asc, ShortInt *Erg)
BEGIN
   if (NOT Decode_sr0(Asc,Erg)) return False;
   return (((*Erg>=0) AND (*Erg<=9)) OR (*Erg==11) OR (*Erg==13) OR (*Erg==25) OR ((*Erg>=32) AND (*Erg<=35)));
END

        static Boolean Decode_sr(char *Asc, ShortInt *Erg)
BEGIN
   if (NOT Decode_sr0(Asc,Erg)) return False;
   return (((*Erg>=0) AND (*Erg<=24)) OR (*Erg==26) OR (*Erg==27) OR (*Erg==40));
END

        static Boolean Decode_sr2(char *Asc, ShortInt *Erg)
BEGIN
   if (NOT Decode_sr0(Asc,Erg)) return False;
   return (((*Erg>=0) AND (*Erg<=9)) OR (*Erg==11) OR (*Erg==13));
END

        static Boolean Decode_sr3(char *Asc, ShortInt *Erg)
BEGIN
   if (strcasecmp(Asc,"ETM0")==0) *Erg=0;
   else if (strcasecmp(Asc,"ETM1")==0) *Erg=1;
   else return False;
   return True;
END

        static Boolean Decode_sr4(char *Asc, ShortInt *Erg)
BEGIN
   if (strcasecmp(Asc,"ECNT")==0) *Erg=0;
   else if (strcasecmp(Asc,"ECPT")==0) *Erg=1;
   else return False;
   return True;
END

        static Boolean Decode_irf(char *Asc, ShortInt *Erg)
BEGIN
#undef FlagCnt
#define FlagCnt 18
   static char *FlagNames[FlagCnt]=
             {"NMI" ,"FT0" ,"FT1" ,"F1"  ,"F2"  ,"FE0" ,
              "FE1" ,"FEIN","FAD" ,"FSR" ,"FST" ,"ER"  ,
              "OV"  ,"AN4" ,"AN5" ,"AN6" ,"AN7" ,"SB"   };
   static ShortInt FlagCodes[FlagCnt]=
             {0,1,2,3,4,5,6,7,8,9,10,11,12,16,17,18,19,20};

   for (*Erg=0; *Erg<FlagCnt; (*Erg)++)
    if (strcasecmp(FlagNames[*Erg],Asc)==0) break;
   if (*Erg>=FlagCnt) return False;
   *Erg=FlagCodes[*Erg];
   return True;
END

        static Boolean Decode_wa(char *Asc, Byte *Erg)
BEGIN
   Word Adr;
   Boolean OK;

   FirstPassUnknown=False;
   Adr=EvalIntExpression(Asc,Int16,&OK); if (NOT OK) return False;
   if ((FirstPassUnknown) AND (Hi(Adr)!=WorkArea)) WrError(110);
   *Erg=Lo(Adr);
   return True;
END

        static Boolean HasDisp(ShortInt Mode)
BEGIN
   return ((Mode & 11)==11);
END

/*--------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
#define ASSUME78C10Count 1
static ASSUMERec ASSUME78C10s[ASSUME78C10Count]=
                 {{"V" , &WorkArea, 0, 0xff, 0x100}};

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME78C10s,ASSUME78C10Count);
     return True;
    END

   return False;
END

        static void MakeCode_78C10(void)
BEGIN
   int z;
   Integer AdrInt;
   ShortInt HVal8,HReg;
   Boolean OK;

   CodeLen=0; DontPrint=False;

   /* zu ignorierendes */

   if (Memo("")) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(False)) return;

   /* ohne Argument */

   for (z=0; z<FixedOrderCnt; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else if ((Memo("STOP")) AND (MomCPU==CPU7810)) WrError(1500);
      else
       BEGIN
        CodeLen=0;
        if (Hi(FixedOrders[z].Code)!=0) BAsmCode[CodeLen++]=Hi(FixedOrders[z].Code);
        BAsmCode[CodeLen++]=Lo(FixedOrders[z].Code);
       END
      return;
     END

   if (Memo("MOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"A")==0)
      if (Decode_sr1(ArgStr[2],&HReg))
       BEGIN
        CodeLen=2; BAsmCode[0]=0x4c;
        BAsmCode[1]=0xc0+HReg;
       END
      else if (Decode_r1(ArgStr[2],&HReg))
       BEGIN
        CodeLen=1; BAsmCode[0]=0x08+HReg;
       END
      else 
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK);
        if (OK)
         BEGIN
          CodeLen=4; BAsmCode[0]=0x70; BAsmCode[1]=0x69;
          BAsmCode[2]=Lo(AdrInt); BAsmCode[3]=Hi(AdrInt);
         END
       END
     else if (strcasecmp(ArgStr[2],"A")==0)
      if (Decode_sr(ArgStr[1],&HReg))
       BEGIN
        CodeLen=2; BAsmCode[0]=0x4d;
        BAsmCode[1]=0xc0+HReg;
       END
      else if (Decode_r1(ArgStr[1],&HReg))
       BEGIN
        CodeLen=1; BAsmCode[0]=0x18+HReg;
       END
      else
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
        if (OK)
         BEGIN
          CodeLen=4; BAsmCode[0]=0x70; BAsmCode[1]=0x79;
          BAsmCode[2]=Lo(AdrInt); BAsmCode[3]=Hi(AdrInt);
         END
       END
     else if (Decode_r(ArgStr[1],&HReg))
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK);
       if (OK)
        BEGIN
         CodeLen=4; BAsmCode[0]=0x70; BAsmCode[1]=0x68+HReg;
         BAsmCode[2]=Lo(AdrInt); BAsmCode[3]=Hi(AdrInt);
        END
      END
     else if (Decode_r(ArgStr[2],&HReg))
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
        BEGIN
         CodeLen=4; BAsmCode[0]=0x70; BAsmCode[1]=0x78+HReg;
         BAsmCode[2]=Lo(AdrInt); BAsmCode[3]=Hi(AdrInt);
        END
      END
     else WrError(1350);
     return;
    END

   if (Memo("MVI"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
        BEGIN
         if (Decode_r(ArgStr[1],&HReg))
          BEGIN
           CodeLen=2; BAsmCode[0]=0x68+HReg;
          END
         else if (Decode_sr2(ArgStr[1],&HReg))
          BEGIN
           CodeLen=3; BAsmCode[2]=BAsmCode[1]; BAsmCode[0]=0x64;
           BAsmCode[1]=(HReg & 7)+((HReg & 8) << 4);
          END
         else WrError(1350);
        END
      END
     return;
    END

   if (Memo("MVIW"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (Decode_wa(ArgStr[1],BAsmCode+1))
      BEGIN
       BAsmCode[2]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
        BEGIN
         CodeLen=3; BAsmCode[0]=0x71;
        END
      END
     return;
    END

   if (Memo("MVIX"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT Decode_rpa1(ArgStr[1],&HReg)) WrError(1350);
     else
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[2],Int8,&OK);
       if (OK)
        BEGIN
         BAsmCode[0]=0x48+HReg; CodeLen=2;
        END
      END
     return;
    END

   if ((Memo("LDAX")) OR (Memo("STAX")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT Decode_rpa2(ArgStr[1],&HReg,(ShortInt *) BAsmCode+1)) WrError(1350);
     else
      BEGIN
       CodeLen=1+Ord(HasDisp(HReg));
       BAsmCode[0]=0x28+(Ord(Memo("STAX")) << 4)+((HReg & 8) << 4)+(HReg & 7);
      END
     return;
    END

   if ((Memo("LDEAX")) OR (Memo("STEAX")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT Decode_rpa3(ArgStr[1],&HReg,(ShortInt *) BAsmCode+2)) WrError(1350);
     else
      BEGIN
       CodeLen=2+Ord(HasDisp(HReg)); BAsmCode[0]=0x48;
       BAsmCode[1]=0x80+(Ord(Memo("STEAX")) << 4)+HReg;
      END
     return;
    END

   if (Memo("LXI"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT Decode_rp2(ArgStr[1],&HReg)) WrError(1350);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[2],Int16,&OK);
       if (OK)
        BEGIN
         CodeLen=3; BAsmCode[0]=0x04+(HReg << 4);
         BAsmCode[1]=Lo(AdrInt); BAsmCode[2]=Hi(AdrInt);
        END
      END
     return;
    END

   if ((Memo("PUSH")) OR (Memo("POP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT Decode_rp1(ArgStr[1],&HReg)) WrError(1350);
     else
      BEGIN
       CodeLen=1;
       BAsmCode[0]=0xa0+(Ord(Memo("PUSH")) << 4)+HReg;
      END
     return;
    END

   if (Memo("DMOV"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       if (strcasecmp(ArgStr[1],"EA")!=0)
        BEGIN
         strcpy(ArgStr[3],ArgStr[1]);
         strcpy(ArgStr[1],ArgStr[2]);
         strcpy(ArgStr[2],ArgStr[3]);
         OK=True;
        END
       else OK=False;
       if (strcasecmp(ArgStr[1],"EA")!=0) WrError(1350);
       else if (Decode_rp3(ArgStr[2],&HReg))
        BEGIN
         CodeLen=1; BAsmCode[0]=0xa4+HReg;
         if (OK) BAsmCode[0]+=0x10;
        END
       else if (((OK) AND (Decode_sr3(ArgStr[2],&HReg)))
             OR ((NOT OK) AND (Decode_sr4(ArgStr[2],&HReg))))
        BEGIN
         CodeLen=2; BAsmCode[0]=0x48; BAsmCode[1]=0xc0+HReg;
         if (OK) BAsmCode[1]+=0x12;
        END
       else WrError(1350);
      END
     return;
    END

   for (z=0; z<ALUOrderCnt; z++)
    if (Memo(ALUOrderImmOps[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        HVal8=EvalIntExpression(ArgStr[2],Int8,&OK);
        if (OK)
         BEGIN
          if (strcasecmp(ArgStr[1],"A")==0)
           BEGIN
            CodeLen=2;
            BAsmCode[0]=0x06+((ALUOrderCodes[z] & 14) << 3)+(ALUOrderCodes[z] & 1);
            BAsmCode[1]=HVal8;
           END
          else if (Decode_r(ArgStr[1],&HReg))
           BEGIN
            CodeLen=3; BAsmCode[0]=0x74; BAsmCode[2]=HVal8;
            BAsmCode[1]=HReg+(ALUOrderCodes[z] << 3);
           END
          else if (Decode_sr2(ArgStr[1],&HReg))
           BEGIN
            CodeLen=3; BAsmCode[0]=0x64; BAsmCode[2]=HVal8;
            BAsmCode[1]=(HReg & 7)+(ALUOrderCodes[z] << 3)+((HReg & 8) << 4);
           END
          else WrError(1350);
         END
       END
      return;
     END
    else if (Memo(ALUOrderRegOps[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else
       BEGIN
        if (strcasecmp(ArgStr[1],"A")!=0)
         BEGIN
          strcpy(ArgStr[3],ArgStr[1]);
          strcpy(ArgStr[1],ArgStr[2]);
          strcpy(ArgStr[2],ArgStr[3]);
          OK=False;
         END
        else OK=True;
        if (strcasecmp(ArgStr[1],"A")!=0) WrError(1350);
        else if (NOT Decode_r(ArgStr[2],&HReg)) WrError(1350);
        else
         BEGIN
          CodeLen=2; BAsmCode[0]=0x60;
          BAsmCode[1]=(ALUOrderCodes[z] << 3)+HReg;
          if ((OK) OR (Memo("ONA")) OR (Memo("OFFA")))
           BAsmCode[1]+=0x80;
         END
       END
      return;
     END
    else if ((OpPart[strlen(OpPart)-1]=='W') AND (strncmp(ALUOrderRegOps[z],OpPart,strlen(ALUOrderRegOps[z]))==0))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (Decode_wa(ArgStr[1],BAsmCode+2))
       BEGIN
        CodeLen=3; BAsmCode[0]=0x74;
        BAsmCode[1]=0x80+(ALUOrderCodes[z] << 3);
       END
      return;
     END
    else if ((OpPart[strlen(OpPart)-1]=='X') AND (strncmp(ALUOrderRegOps[z],OpPart,strlen(ALUOrderRegOps[z]))==0))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (NOT Decode_rpa(ArgStr[1],&HReg)) WrError(1350);
      else
       BEGIN
        CodeLen=2; BAsmCode[0]=0x70;
        BAsmCode[1]=0x80+(ALUOrderCodes[z] << 3)+HReg;
       END
      return;
     END
    else if (Memo(ALUOrderEAOps[z]))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (strcasecmp(ArgStr[1],"EA")!=0) WrError(1350);
      else if (NOT Decode_rp3(ArgStr[2],&HReg)) WrError(1350);
      else
       BEGIN
        CodeLen=2; BAsmCode[0]=0x74;
        BAsmCode[1]=0x84+(ALUOrderCodes[z] << 3)+HReg;
       END
      return;
     END
    else if (((OpPart[strlen(OpPart)-1]=='W') AND (strncmp(ALUOrderImmOps[z],OpPart,strlen(ALUOrderImmOps[z]))==0)) AND (Odd(ALUOrderCodes[z])))
     BEGIN
      if (ArgCnt!=2) WrError(1110);
      else if (Decode_wa(ArgStr[1],BAsmCode+1))
       BEGIN
        BAsmCode[2]=EvalIntExpression(ArgStr[2],Int8,&OK);
        if (OK)
         BEGIN
          CodeLen=3;
          BAsmCode[0]=0x05+((ALUOrderCodes[z] >> 1) << 4);
         END
       END
      return;
     END

   for (z=0; z<AbsOrderCnt; z++)
    if (Memo(AbsOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
        if (OK)
         BEGIN
          CodeLen=0;
          if (Hi(AbsOrders[z].Code)!=0) BAsmCode[CodeLen++]=Hi(AbsOrders[z].Code);
          BAsmCode[CodeLen++]=Lo(AbsOrders[z].Code);
          BAsmCode[CodeLen++]=Lo(AdrInt);
          BAsmCode[CodeLen++]=Hi(AdrInt);
         END
       END
      return;
     END

   for (z=0; z<Reg2OrderCnt; z++)
    if (Memo(Reg2Orders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (NOT Decode_r2(ArgStr[1],&HReg)) WrError(1350);
      else
       BEGIN
        CodeLen=0;
        if (Hi(Reg2Orders[z].Code)!=0) BAsmCode[CodeLen++]=Hi(Reg2Orders[z].Code);
        BAsmCode[CodeLen++]=Lo(Reg2Orders[z].Code)+HReg;
       END
      return;
     END

   for (z=0; z<WorkOrderCnt; z++)
    if (Memo(WorkOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (Decode_wa(ArgStr[1],BAsmCode+1))
       BEGIN
        CodeLen=2; BAsmCode[0]=WorkOrders[z].Code;
       END
      return;
     END

   if ((Memo("DCX")) OR (Memo("INX")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (strcasecmp(ArgStr[1],"EA")==0)
      BEGIN
       CodeLen=1; BAsmCode[0]=0xa8+Ord(Memo("DCX"));
      END
     else if (Decode_rp(ArgStr[1],&HReg))
      BEGIN
       CodeLen=1; BAsmCode[0]=0x02+Ord(Memo("DCX"))+(HReg << 4);
      END
     else WrError(1350);
     return;
    END

   if ((Memo("EADD")) OR (Memo("ESUB")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (strcasecmp(ArgStr[1],"EA")!=0) WrError(1350);
     else if (NOT Decode_r2(ArgStr[2],&HReg)) WrError(1350);
     else
      BEGIN
       CodeLen=2; BAsmCode[0]=0x70;
       BAsmCode[1]=0x40+(Ord(Memo("ESUB")) << 5)+HReg;
      END
     return;
    END

   if ((Memo("JR")) OR (Memo("JRE")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK)-(EProgCounter()+1+Ord(Memo("JRE")));
       if (OK)
        BEGIN
         if (Memo("JR"))
          if ((NOT SymbolQuestionable) AND ((AdrInt<-32) OR (AdrInt>31))) WrError(1370);
          else
           BEGIN
            CodeLen=1; BAsmCode[0]=0xc0+(AdrInt & 0x3f);
           END
         else
          if ((NOT SymbolQuestionable) AND ((AdrInt<-256) OR (AdrInt>255))) WrError(1370);
          else
           BEGIN
            if ((AdrInt>=-32) AND (AdrInt<=31)) WrError(20);
            CodeLen=2; BAsmCode[0]=0x4e + (Hi(AdrInt) & 1); /* ANSI :-O */
            BAsmCode[1]=Lo(AdrInt);
           END
        END
      END
     return;
    END

   if (Memo("CALF"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
        BEGIN
         if ((NOT FirstPassUnknown) AND ((AdrInt >> 11)!=1)) WrError(1905);
         else
          BEGIN
           CodeLen=2;
           BAsmCode[0]=Hi(AdrInt)+0x70; BAsmCode[1]=Lo(AdrInt);
          END
        END
      END
     return;
    END

   if (Memo("CALT"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrInt=EvalIntExpression(ArgStr[1],Int16,&OK);
       if (OK)
        BEGIN
         if ((NOT FirstPassUnknown) AND ((AdrInt & 0xffc1)!=0x80)) WrError(1905);
         else
          BEGIN
           CodeLen=1;
           BAsmCode[0]=0x80+((AdrInt & 0x3f) >> 1);
          END
        END
      END
     return;
    END

   if (Memo("BIT"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       HReg=EvalIntExpression(ArgStr[1],UInt3,&OK);
       if (OK)
        if (Decode_wa(ArgStr[2],BAsmCode+1))
         BEGIN
          CodeLen=2; BAsmCode[0]=0x58+HReg;
         END
      END
     return;
    END

   for (z=0; z<EAOrderCnt; z++)
    if (Memo(EAOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else if (strcasecmp(ArgStr[1],"EA")!=0) WrError(1350);
      else
       BEGIN
        CodeLen=2; BAsmCode[0]=Hi(EAOrders[z].Code); BAsmCode[1]=Lo(EAOrders[z].Code);
       END
      return;
     END

   if ((Memo("SK")) OR (Memo("SKN")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT Decode_f(ArgStr[1],&HReg)) WrError(1350);
     else
      BEGIN
       CodeLen=2; BAsmCode[0]=0x48;
       BAsmCode[1]=0x08+(Ord(Memo("SKN")) << 4)+HReg;
      END
     return;
    END

   if ((Memo("SKIT")) OR (Memo("SKINT")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else if (NOT Decode_irf(ArgStr[1],&HReg)) WrError(1350);
     else
      BEGIN
       CodeLen=2; BAsmCode[0]=0x48;
       BAsmCode[1]=0x40+(Ord(Memo("SKINT")) << 5)+HReg;
      END
     return;
    END

   WrXError(1200,OpPart);
END

        static void InitCode_78C10(void)
BEGIN
   SaveInitProc();
   WorkArea=0x100;
END

        static Boolean IsDef_78C10(void)
BEGIN
   return False;
END

        static void SwitchFrom_78C10(void)
BEGIN
   DeinitFields();
END

        static void SwitchTo_78C10(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeIntel; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x7a; NOPCode=0x00;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;
   SegLimits[SegCode] = 0xffff;

   MakeCode=MakeCode_78C10; IsDef=IsDef_78C10;
   SwitchFrom=SwitchFrom_78C10; InitFields();
END

        void code78c10_init(void)
BEGIN
   CPU7810 =AddCPU("7810" ,SwitchTo_78C10);
   CPU78C10=AddCPU("78C10",SwitchTo_78C10);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_78C10;
END
