/* code29k.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegenerator AM29xxx-Familie                                             */
/*                                                                           */
/* Historie: 18.11.1996 Grundsteinlegung                                     */
/*            3. 1.1999 ChkPC-Anpassung                                      */
/*            9. 3.2000 'ambiguous else'-Warnungen beseitigt                 */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"
#include <string.h>
#include <ctype.h>

#include "nls.h"
#include "strutil.h"
#include "bpemu.h"
#include "stringlists.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "asmallg.h"
#include "codepseudo.h"
#include "codevars.h"

typedef struct
         {
          char *Name;
          Boolean MustSup;
          CPUVar MinCPU;
          LongWord Code;
         } StdOrder;

typedef struct
         {
          char *Name;
          Boolean HasReg,HasInd;
          CPUVar MinCPU;
          LongWord Code;
         } JmpOrder;

typedef struct
         {
          char *Name;
          LongWord Code;
         } SPReg;

#define StdOrderCount 51
#define NoImmOrderCount 22
#define VecOrderCount 10
#define JmpOrderCount 5
#define FixedOrderCount 2
#define MemOrderCount 7
#define SPRegCount 28

static StdOrder *StdOrders;
static StdOrder *NoImmOrders;
static StdOrder *VecOrders;
static JmpOrder *JmpOrders;
static StdOrder *FixedOrders;
static StdOrder *MemOrders;
static SPReg *SPRegs;


static CPUVar CPU29000,CPU29240,CPU29243,CPU29245;
static LongInt Reg_RBP;
static StringList Emulations;
static SimpProc SaveInitProc;

/*-------------------------------------------------------------------------*/

        static void AddStd(char *NName, CPUVar NMin, Boolean NSup, LongWord NCode)
BEGIN
   if (InstrZ>=StdOrderCount) exit(255);
   StdOrders[InstrZ].Name=NName;
   StdOrders[InstrZ].Code=NCode;
   StdOrders[InstrZ].MustSup=NSup;
   StdOrders[InstrZ++].MinCPU=NMin;
END

        static void AddNoImm(char *NName, CPUVar NMin, Boolean NSup, LongWord NCode)
BEGIN
   if (InstrZ>=NoImmOrderCount) exit(255);
   NoImmOrders[InstrZ].Name=NName;
   NoImmOrders[InstrZ].Code=NCode;
   NoImmOrders[InstrZ].MustSup=NSup;
   NoImmOrders[InstrZ++].MinCPU=NMin;
END

        static void AddVec(char *NName, CPUVar NMin, Boolean NSup, LongWord NCode)
BEGIN
   if (InstrZ>=VecOrderCount) exit(255);
   VecOrders[InstrZ].Name=NName;
   VecOrders[InstrZ].Code=NCode;
   VecOrders[InstrZ].MustSup=NSup;
   VecOrders[InstrZ++].MinCPU=NMin;
END

        static void AddJmp(char *NName, CPUVar NMin, Boolean NHas, Boolean NInd, LongWord NCode)
BEGIN
   if (InstrZ>=JmpOrderCount) exit(255);
   JmpOrders[InstrZ].Name=NName;
   JmpOrders[InstrZ].HasReg=NHas;
   JmpOrders[InstrZ].HasInd=NInd;
   JmpOrders[InstrZ].Code=NCode;
   JmpOrders[InstrZ++].MinCPU=NMin;
END

        static void AddFixed(char *NName, CPUVar NMin, Boolean NSup, LongWord NCode)
BEGIN
   if (InstrZ>=FixedOrderCount) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ].Code=NCode;
   FixedOrders[InstrZ].MustSup=NSup;
   FixedOrders[InstrZ++].MinCPU=NMin;
END

        static void AddMem(char *NName, CPUVar NMin, Boolean NSup, LongWord NCode)
BEGIN
   if (InstrZ>=MemOrderCount) exit(255);
   MemOrders[InstrZ].Name=NName;
   MemOrders[InstrZ].Code=NCode;
   MemOrders[InstrZ].MustSup=NSup;
   MemOrders[InstrZ++].MinCPU=NMin;
END

        static void AddSP(char *NName, LongWord NCode)
BEGIN
   if (InstrZ>=SPRegCount) exit(255);
   SPRegs[InstrZ].Name=NName;
   SPRegs[InstrZ++].Code=NCode;
END

        static void InitFields(void)
BEGIN
   StdOrders=(StdOrder *) malloc(sizeof(StdOrder)*StdOrderCount); InstrZ=0;
   AddStd("ADD"    ,CPU29245,False,0x14); AddStd("ADDC"   ,CPU29245,False,0x1c);
   AddStd("ADDCS"  ,CPU29245,False,0x18); AddStd("ADDCU"  ,CPU29245,False,0x1a);
   AddStd("ADDS"   ,CPU29245,False,0x10); AddStd("ADDU"   ,CPU29245,False,0x12);
   AddStd("AND"    ,CPU29245,False,0x90); AddStd("ANDN"   ,CPU29245,False,0x9c);
   AddStd("CPBYTE" ,CPU29245,False,0x2e); AddStd("CPEQ"   ,CPU29245,False,0x60);
   AddStd("CPGE"   ,CPU29245,False,0x4c); AddStd("CPGEU"  ,CPU29245,False,0x4e);
   AddStd("CPGT"   ,CPU29245,False,0x48); AddStd("CPGTU"  ,CPU29245,False,0x4a);
   AddStd("CPLE"   ,CPU29245,False,0x44); AddStd("CPLEU"  ,CPU29245,False,0x46);
   AddStd("CPLT"   ,CPU29245,False,0x40); AddStd("CPLTU"  ,CPU29245,False,0x42);
   AddStd("CPNEQ"  ,CPU29245,False,0x62); AddStd("DIV"    ,CPU29245,False,0x6a);
   AddStd("DIV0"   ,CPU29245,False,0x68); AddStd("DIVL"   ,CPU29245,False,0x6c);
   AddStd("DIVREM" ,CPU29245,False,0x6e); AddStd("EXBYTE" ,CPU29245,False,0x0a);
   AddStd("EXHW"   ,CPU29245,False,0x7c); AddStd("EXTRACT",CPU29245,False,0x7a);
   AddStd("INBYTE" ,CPU29245,False,0x0c); AddStd("INHW"   ,CPU29245,False,0x78);
   AddStd("MUL"    ,CPU29245,False,0x64); AddStd("MULL"   ,CPU29245,False,0x66);
   AddStd("MULU"   ,CPU29245,False,0x74); AddStd("NAND"   ,CPU29245,False,0x9a);
   AddStd("NOR"    ,CPU29245,False,0x98); AddStd("OR"     ,CPU29245,False,0x92);
   AddStd("SLL"    ,CPU29245,False,0x80); AddStd("SRA"    ,CPU29245,False,0x86);
   AddStd("SRL"    ,CPU29245,False,0x82); AddStd("SUB"    ,CPU29245,False,0x24);
   AddStd("SUBC"   ,CPU29245,False,0x2c); AddStd("SUBCS"  ,CPU29245,False,0x28);
   AddStd("SUBCU"  ,CPU29245,False,0x2a); AddStd("SUBR"   ,CPU29245,False,0x34);
   AddStd("SUBRC"  ,CPU29245,False,0x3c); AddStd("SUBRCS" ,CPU29245,False,0x38);
   AddStd("SUBRCU" ,CPU29245,False,0x3a); AddStd("SUBRS"  ,CPU29245,False,0x30);
   AddStd("SUBRU"  ,CPU29245,False,0x32); AddStd("SUBS"   ,CPU29245,False,0x20);
   AddStd("SUBU"   ,CPU29245,False,0x22); AddStd("XNOR"   ,CPU29245,False,0x96);
   AddStd("XOR"    ,CPU29245,False,0x94);

   NoImmOrders=(StdOrder *) malloc(sizeof(StdOrder)*NoImmOrderCount); InstrZ=0;
   AddNoImm("DADD"    ,CPU29000,False,0xf1); AddNoImm("DDIV"    ,CPU29000,False,0xf7);
   AddNoImm("DEQ"     ,CPU29000,False,0xeb); AddNoImm("DGE"     ,CPU29000,False,0xef);
   AddNoImm("DGT"     ,CPU29000,False,0xed); AddNoImm("DIVIDE"  ,CPU29000,False,0xe1);
   AddNoImm("DIVIDU"  ,CPU29000,False,0xe3); AddNoImm("DMUL"    ,CPU29000,False,0xf5);
   AddNoImm("DSUB"    ,CPU29000,False,0xf3); AddNoImm("FADD"    ,CPU29000,False,0xf0);
   AddNoImm("FDIV"    ,CPU29000,False,0xf6); AddNoImm("FDMUL"   ,CPU29000,False,0xf9);
   AddNoImm("FEQ"     ,CPU29000,False,0xea); AddNoImm("FGE"     ,CPU29000,False,0xee);
   AddNoImm("FGT"     ,CPU29000,False,0xec); AddNoImm("FMUL"    ,CPU29000,False,0xf4);
   AddNoImm("FSUB"    ,CPU29000,False,0xf2); AddNoImm("MULTIPLU",CPU29243,False,0xe2);
   AddNoImm("MULTIPLY",CPU29243,False,0xe0); AddNoImm("MULTM"   ,CPU29243,False,0xde);
   AddNoImm("MULTMU"  ,CPU29243,False,0xdf); AddNoImm("SETIP"   ,CPU29245,False,0x9e);

   VecOrders=(StdOrder *) malloc(sizeof(StdOrder)*VecOrderCount); InstrZ=0;
   AddVec("ASEQ"   ,CPU29245,False,0x70); AddVec("ASGE"   ,CPU29245,False,0x5c);
   AddVec("ASGEU"  ,CPU29245,False,0x5e); AddVec("ASGT"   ,CPU29245,False,0x58);
   AddVec("ASGTU"  ,CPU29245,False,0x5a); AddVec("ASLE"   ,CPU29245,False,0x54);
   AddVec("ASLEU"  ,CPU29245,False,0x56); AddVec("ASLT"   ,CPU29245,False,0x50);
   AddVec("ASLTU"  ,CPU29245,False,0x52); AddVec("ASNEQ"  ,CPU29245,False,0x72);

   JmpOrders=(JmpOrder *) malloc(sizeof(JmpOrder)*JmpOrderCount); InstrZ=0;
   AddJmp("CALL"   ,CPU29245,True ,True ,0xa8); AddJmp("JMP"    ,CPU29245,False,True ,0xa0);
   AddJmp("JMPF"   ,CPU29245,True ,True ,0xa4); AddJmp("JMPFDEC",CPU29245,True ,False,0xb4);
   AddJmp("JMPT"   ,CPU29245,True ,True ,0xac);

   FixedOrders=(StdOrder *) malloc(sizeof(StdOrder)*FixedOrderCount); InstrZ=0;
   AddFixed("HALT"   ,CPU29245,True,0x89); AddFixed("IRET"   ,CPU29245,True,0x88);

   MemOrders=(StdOrder *) malloc(sizeof(StdOrder)*MemOrderCount); InstrZ=0;
   AddMem("LOAD"   ,CPU29245,False,0x16); AddMem("LOADL"  ,CPU29245,False,0x06);
   AddMem("LOADM"  ,CPU29245,False,0x36); AddMem("LOADSET",CPU29245,False,0x26);
   AddMem("STORE"  ,CPU29245,False,0x1e); AddMem("STOREL" ,CPU29245,False,0x0e);
   AddMem("STOREM" ,CPU29245,False,0x3e);

   SPRegs=(SPReg *) malloc(sizeof(SPReg)*SPRegCount); InstrZ=0;
   AddSP("VAB",   0);
   AddSP("OPS",   1);
   AddSP("CPS",   2);
   AddSP("CFG",   3);
   AddSP("CHA",   4);
   AddSP("CHD",   5);
   AddSP("CHC",   6);
   AddSP("RBP",   7);
   AddSP("TMC",   8);
   AddSP("TMR",   9);
   AddSP("PC0",  10);
   AddSP("PC1",  11);
   AddSP("PC2",  12);
   AddSP("MMU",  13);
   AddSP("LRU",  14);
   AddSP("CIR",  29);
   AddSP("CDR",  30);
   AddSP("IPC", 128);
   AddSP("IPA", 129);
   AddSP("IPB", 130);
   AddSP("Q",   131);
   AddSP("ALU", 132);
   AddSP("BP",  133);
   AddSP("FC",  134);
   AddSP("CR",  135);
   AddSP("FPE", 160);
   AddSP("INTE",161);
   AddSP("FPS", 162);
END

        static void DeinitFields(void)
BEGIN
   free(StdOrders);
   free(NoImmOrders);
   free(VecOrders);
   free(JmpOrders);
   free(FixedOrders);
   free(MemOrders);
   free(SPRegs);
END

/*-------------------------------------------------------------------------*/

        static void ChkSup(void)
BEGIN
   if (NOT SupAllowed) WrError(50);
END

        static Boolean IsSup(LongWord RegNo)
BEGIN
   return ((RegNo<0x80) OR (RegNo>=0xa0));
END

        static Boolean ChkCPU(CPUVar Min)
BEGIN
   if (MomCPU>=Min) return True;
   else return (StringListPresent(Emulations,OpPart));
END

/*-------------------------------------------------------------------------*/

        static Boolean DecodeReg(char *Asc, LongWord *Erg)
BEGIN
   Boolean io,OK;

   if ((strlen(Asc)>=2) AND (toupper(*Asc)=='R'))
    BEGIN
     *Erg=ConstLongInt(Asc+1,&io);
     OK=((io) AND (*Erg<=255));
    END
   else if ((strlen(Asc)>=3) AND (toupper(*Asc)=='G') AND (toupper(Asc[1])=='R'))
    BEGIN
     *Erg=ConstLongInt(Asc+2,&io);
     OK=((io) AND (*Erg<=127));
    END
   else if ((strlen(Asc)>=3) AND (toupper(*Asc)=='L') AND (toupper(Asc[1])=='R'))
    BEGIN
     *Erg=ConstLongInt(Asc+2,&io);
     OK=((io) AND (*Erg<=127));
     *Erg+=128;
    END
   else OK=False;
   if (OK)
    if ((*Erg<127) AND (Odd(Reg_RBP >> ((*Erg) >> 4)))) ChkSup();
   return OK;
END

        static Boolean DecodeSpReg(char *Asc_O, LongWord *Erg)
BEGIN
   int z;
   String Asc;

   strmaxcpy(Asc,Asc_O,255); NLS_UpString(Asc);
   for (z=0; z<SPRegCount; z++)
    if (strcmp(Asc,SPRegs[z].Name)==0)
     BEGIN
      *Erg=SPRegs[z].Code;
      break;
     END
   return (z<SPRegCount);
END

/*-------------------------------------------------------------------------*/

        static Boolean DecodePseudo(void)
BEGIN
#define ASSUME29KCount 1
   static ASSUMERec ASSUME29Ks[ASSUME29KCount]=
             {{"RBP", &Reg_RBP, 0, 0xff, 0x00000000}};

   int z;

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME29Ks,ASSUME29KCount);
     return True;
    END

   if (Memo("EMULATED"))
    BEGIN
     if (ArgCnt<1) WrError(1110);
     else
      for (z=1; z<=ArgCnt; z++)
       BEGIN
        NLS_UpString(ArgStr[z]);
        if (NOT StringListPresent(Emulations,ArgStr[z]))
         AddStringListLast(&Emulations,ArgStr[z]);
       END
     return True;
    END

   return False;
END

        static void MakeCode_29K(void)
BEGIN
   int z,l;
   LongWord Dest,Src1,Src2,Src3,AdrLong;
   LongInt AdrInt;
   Boolean OK;

   CodeLen=0; DontPrint=False;

   /* Nullanweisung */

   if (Memo("") AND (*AttrPart=='\0') AND (ArgCnt==0)) return;

   /* Pseudoanweisungen */

   if (DecodePseudo()) return;

   if (DecodeIntelPseudo(True)) return;

   /* Variante 1: Register <-- Register op Register/uimm8 */

   for (z=0; z<StdOrderCount; z++)
    if (Memo(StdOrders[z].Name))
     BEGIN
      if ((ArgCnt>3) OR (ArgCnt<2)) WrError(1110);
      else if (NOT DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
      else
       BEGIN
        OK=True;
        if (ArgCnt==2) Src1=Dest;
        else OK=DecodeReg(ArgStr[2],&Src1);
        if (NOT OK) WrXError(1445,ArgStr[2]);
        else
         BEGIN
          if (DecodeReg(ArgStr[ArgCnt],&Src2))
           BEGIN
            OK=True; Src3=0;
           END
          else
           BEGIN
            Src2=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
            Src3=0x1000000;
           END
          if (OK)
           BEGIN
            CodeLen=4;
            DAsmCode[0]=(StdOrders[z].Code << 24)+Src3+(Dest << 16)+(Src1 << 8)+Src2;
            if (StdOrders[z].MustSup) ChkSup();
           END
         END
       END
      return;
     END

   /* Variante 2: Register <-- Register op Register */

   for (z=0; z<NoImmOrderCount; z++)
    if (Memo(NoImmOrders[z].Name))
     BEGIN
      if ((ArgCnt>3) OR (ArgCnt<2)) WrError(1110);
      else if (NOT DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
      else
       BEGIN
        OK=True;
        if (ArgCnt==2) Src1=Dest;
        else OK=DecodeReg(ArgStr[2],&Src1);
        if (NOT OK) WrError(1445);
        else if (NOT DecodeReg(ArgStr[ArgCnt],&Src2)) WrError(1445);
        else
         BEGIN
          CodeLen=4;
          DAsmCode[0]=(NoImmOrders[z].Code << 24)+(Dest << 16)+(Src1 << 8)+Src2;
          if (NoImmOrders[z].MustSup) ChkSup();
         END
       END
      return;
     END

   /* Variante 3: Vektor <-- Register op Register/uimm8 */

   for (z=0; z<VecOrderCount; z++)
    if (Memo(VecOrders[z].Name))
     BEGIN
      if (ArgCnt!=3) WrError(1110);
      else
       BEGIN
        FirstPassUnknown=False;
        Dest=EvalIntExpression(ArgStr[1],UInt8,&OK);
        if (FirstPassUnknown) Dest=64;
        if (OK)
         BEGIN
          if (NOT DecodeReg(ArgStr[2],&Src1)) WrError(1445);
          else
           BEGIN
            if (DecodeReg(ArgStr[ArgCnt],&Src2))
             BEGIN
              OK=True; Src3=0;
             END
            else
             BEGIN
              Src2=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
              Src3=0x1000000;
             END
            if (OK)
             BEGIN
              CodeLen=4;
              DAsmCode[0]=(VecOrders[z].Code << 24)+Src3+(Dest << 16)+(Src1 << 8)+Src2;
              if ((VecOrders[z].MustSup) OR (Dest<=63)) ChkSup();
             END
           END
         END
       END
      return;
     END

   /* Variante 4: ohne Operanden */

   for (z=0; z<FixedOrderCount; z++)
    if (Memo(FixedOrders[z].Name))
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        CodeLen=4; DAsmCode[0]=FixedOrders[z].Code << 24;
        if (FixedOrders[z].MustSup) ChkSup();
       END
      return;
     END

   /* Variante 5 : [0], Speichersteuerwort, Register, Register/uimm8 */

   for (z=0; z<MemOrderCount; z++)
    if (Memo(MemOrders[z].Name))
     BEGIN
      if ((ArgCnt!=3) AND (ArgCnt!=4)) WrError(1110);
      else
       BEGIN
        if (ArgCnt==3)
         BEGIN
          OK=True; AdrLong=0;
         END
        else
         BEGIN
          AdrLong=EvalIntExpression(ArgStr[1],Int32,&OK);
          if (OK) OK=ChkRange(AdrLong,0,0);
         END
        if (OK)
         BEGIN
          Dest=EvalIntExpression(ArgStr[ArgCnt-2],UInt7,&OK);
          if (OK)
           if (DecodeReg(ArgStr[ArgCnt-1],&Src1))
            BEGIN
             if (DecodeReg(ArgStr[ArgCnt],&Src2))
              BEGIN
               OK=True; Src3=0;
              END
             else
              BEGIN
               Src2=EvalIntExpression(ArgStr[ArgCnt],UInt8,&OK);
               Src3=0x1000000;
              END
             if (OK)
              BEGIN
               CodeLen=4;
               DAsmCode[0]=(MemOrders[z].Code << 24)+Src3+(Dest << 16)+(Src1 << 8)+Src2;
               if (MemOrders[z].MustSup) ChkSup();
              END
            END
         END
       END
      return;
     END

   /* Sprungbefehle */

   for (z=0; z<JmpOrderCount; z++)
    BEGIN
     l=strlen(JmpOrders[z].Name);
     if ((strncmp(OpPart,JmpOrders[z].Name,l)==0) AND ((OpPart[l]=='\0') OR (OpPart[l]=='I')))
      BEGIN
       if (ArgCnt!=1+Ord(JmpOrders[z].HasReg)) WrError(1110);
       else if (DecodeReg(ArgStr[ArgCnt],&Src1))
        BEGIN
         if (NOT JmpOrders[z].HasReg)
          BEGIN
           Dest=0; OK=True;
          END
         else OK=DecodeReg(ArgStr[1],&Dest);
         if (OK)
          BEGIN
           CodeLen=4;
           DAsmCode[0]=((JmpOrders[z].Code+0x20) << 24)+(Dest << 8)+Src1;
          END
        END
       else if (OpPart[l]=='I') WrError(1445);
       else
        BEGIN
         if (NOT JmpOrders[z].HasReg)
          BEGIN
           Dest=0; OK=True;
          END
         else OK=DecodeReg(ArgStr[1],&Dest);
         if (OK)
          BEGIN
           AdrLong=EvalIntExpression(ArgStr[ArgCnt],Int32,&OK); 
           AdrInt=AdrLong-EProgCounter();
           if (OK)
            BEGIN
             if ((AdrLong & 3)!=0) WrError(1325);
              else if ((AdrInt<=0x1ffff) AND (AdrInt>=-0x20000))
              BEGIN
               CodeLen=4;
               AdrLong-=EProgCounter();
               DAsmCode[0]=(JmpOrders[z].Code << 24)
                          +((AdrLong & 0x3fc00) << 6)
                          +(Dest << 8)+((AdrLong & 0x3fc) >> 2);
              END
             else if ((NOT SymbolQuestionable) AND (AdrLong>0x3fffff)) WrError(1370);
             else
              BEGIN
               CodeLen=4;
               DAsmCode[0]=((JmpOrders[z].Code+1) << 24)
                          +((AdrLong & 0x3fc00) << 6)
                          +(Dest << 8)+((AdrLong & 0x3fc) >> 2);
              END
            END
          END
        END
       return;
      END
    END

   /* Sonderfaelle */

   if (Memo("CLASS"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else if (NOT ChkCPU(CPU29000)) WrError(1500);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else if (NOT DecodeReg(ArgStr[2],&Src1)) WrError(1445);
     else
      BEGIN
       Src2=EvalIntExpression(ArgStr[3],UInt2,&OK);
       if (OK)
        BEGIN
         CodeLen=4;
         DAsmCode[0]=0xe6000000+(Dest << 16)+(Src1 << 8)+Src2;
        END
      END
     return;
    END

   if (Memo("EMULATE"))
    BEGIN
     if (ArgCnt!=3) WrError(1110);
     else
      BEGIN
       FirstPassUnknown=False;
       Dest=EvalIntExpression(ArgStr[1],UInt8,&OK);
       if (FirstPassUnknown) Dest=64;
       if (OK)
        BEGIN
         if (NOT DecodeReg(ArgStr[2],&Src1)) WrError(1445);
         else if (NOT DecodeReg(ArgStr[ArgCnt],&Src2)) WrError(1445);
         else
          BEGIN
           CodeLen=4;
           DAsmCode[0]=0xd7000000+(Dest << 16)+(Src1 << 8)+Src2;
           if (Dest<=63) ChkSup();
          END
        END
      END
     return;
    END

   if (Memo("SQRT"))
    BEGIN
     if ((ArgCnt!=3) AND (ArgCnt!=2)) WrError(1110);
     else if (NOT ChkCPU(CPU29000)) WrError(1500);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else
      BEGIN
       if (ArgCnt==2)
        BEGIN
         OK=True; Src1=Dest;
        END
       else OK=DecodeReg(ArgStr[2],&Src1);
       if (NOT OK) WrError(1445);
       else
        BEGIN
         Src2=EvalIntExpression(ArgStr[ArgCnt],UInt2,&OK);
         if (OK)
          BEGIN
           CodeLen=4;
           DAsmCode[0]=0xe5000000+(Dest << 16)+(Src1 << 8)+Src2;
          END
        END
      END
     return;
    END

   if (Memo("CLZ"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else
      BEGIN
       if (DecodeReg(ArgStr[2],&Src1))
        BEGIN
         OK=True; Src3=0;
        END
       else
        BEGIN
         Src1=EvalIntExpression(ArgStr[2],UInt8,&OK);
         Src3=0x1000000;
        END
       if (OK)
        BEGIN
         CodeLen=4;
         DAsmCode[0]=0x08000000+Src3+(Dest << 16)+Src1;
        END
      END
     return;
    END

   if (Memo("CONST"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[2],Int32,&OK);
       if (OK)
        BEGIN
         CodeLen=4;
         DAsmCode[0]=((AdrLong & 0xff00) << 8)+(Dest << 8)+(AdrLong & 0xff);
         AdrLong=AdrLong >> 16;
         if (AdrLong==0xffff) DAsmCode[0]+=0x01000000;
         else
          BEGIN
           DAsmCode[0]+=0x03000000;
           if (AdrLong!=0)
            BEGIN
             CodeLen=8;
             DAsmCode[1]=0x02000000+((AdrLong & 0xff00) << 16)+(Dest << 8)+(AdrLong & 0xff);
            END
          END
        END
      END
     return;
    END

   if ((Memo("CONSTH")) OR (Memo("CONSTN")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else
      BEGIN
       FirstPassUnknown=False;
       AdrLong=EvalIntExpression(ArgStr[2],Int32,&OK);
       if (FirstPassUnknown) AdrLong&=0xffff;
       if ((Memo("CONSTN")) AND ((AdrLong >> 16)==0xffff)) AdrLong&=0xffff;
       if (ChkRange(AdrLong,0,0xffff))
        BEGIN
         CodeLen=4;
         DAsmCode[0]=0x1000000+((AdrLong & 0xff00) << 8)+(Dest << 8)+(AdrLong & 0xff);
         if (Memo("CONSTH")) DAsmCode[0]+=0x1000000;
        END
      END
     return;
    END

   if (Memo("CONVERT"))
    BEGIN
     if (ArgCnt!=6) WrError(1110);
     else if (NOT ChkCPU(CPU29000)) WrError(1500);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else if (NOT DecodeReg(ArgStr[2],&Src1)) WrError(1445);
     else
      BEGIN
       Src2=0;
       Src2+=EvalIntExpression(ArgStr[3],UInt1,&OK) << 7;
       if (OK)
        BEGIN
         Src2+=EvalIntExpression(ArgStr[4],UInt3,&OK) << 4;
         if (OK)
          BEGIN
           Src2+=EvalIntExpression(ArgStr[5],UInt2,&OK) << 2;
           if (OK)
            BEGIN
             Src2+=EvalIntExpression(ArgStr[6],UInt2,&OK);
             if (OK)
              BEGIN
               CodeLen=4;
               DAsmCode[0]=0xe4000000+(Dest << 16)+(Src1 << 8)+Src2;
              END
            END
          END
        END
      END
     return;
    END

   if (Memo("EXHWS"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrError(1445);
     else if (NOT DecodeReg(ArgStr[2],&Src1)) WrError(1445);
     else
      BEGIN
       CodeLen=4;
       DAsmCode[0]=0x7e000000+(Dest << 16)+(Src1 << 8);
      END
     return;
    END

   if ((Memo("INV")) OR (Memo("IRETINV")))
    BEGIN
     if (ArgCnt>1) WrError(1110);
     else
      BEGIN
       if (ArgCnt==0)
        BEGIN
         Src1=0; OK=True;
        END
       else Src1=EvalIntExpression(ArgStr[1],UInt2,&OK);
       if (OK)
        BEGIN
         CodeLen=4;
         DAsmCode[0]=Src1 << 16;
         if (Memo("INV")) DAsmCode[0]+=0x9f000000;
         else DAsmCode[0]+=0x8c000000;
         ChkSup();
        END
      END
     return;
    END

   if (Memo("MFSR"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
     else if (NOT DecodeSpReg(ArgStr[2],&Src1)) WrXError(1440,ArgStr[2]);
     else
      BEGIN
       DAsmCode[0]=0xc6000000+(Dest << 16)+(Src1 << 8);
       CodeLen=4; if (IsSup(Src1)) ChkSup();
      END
     return;
    END

   if (Memo("MTSR"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeSpReg(ArgStr[1],&Dest)) WrXError(1440,ArgStr[1]);
     else if (NOT DecodeReg(ArgStr[2],&Src1)) WrXError(1445,ArgStr[2]);
     else
      BEGIN
       DAsmCode[0]=0xce000000+(Dest << 8)+Src1;
       CodeLen=4; if (IsSup(Dest)) ChkSup();
      END
     return;
    END

   if (Memo("MTSRIM"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeSpReg(ArgStr[1],&Dest)) WrXError(1440,ArgStr[1]);
     else
      BEGIN
       Src1=EvalIntExpression(ArgStr[2],UInt16,&OK);
       if (OK)
        BEGIN
         DAsmCode[0]=0x04000000+((Src1 & 0xff00) << 8)+(Dest << 8)+Lo(Src1);
         CodeLen=4; if (IsSup(Dest)) ChkSup();
        END
      END
     return;
    END

   if (Memo("MFTLB"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
     else if (NOT DecodeReg(ArgStr[2],&Src1)) WrXError(1445,ArgStr[2]);
     else
      BEGIN
       DAsmCode[0]=0xb6000000+(Dest << 16)+(Src1 << 8);
       CodeLen=4; ChkSup();
      END
     return;
    END

   if (Memo("MTTLB"))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else if (NOT DecodeReg(ArgStr[1],&Dest)) WrXError(1445,ArgStr[1]);
     else if (NOT DecodeReg(ArgStr[2],&Src1)) WrXError(1445,ArgStr[2]);
     else
      BEGIN
       DAsmCode[0]=0xbe000000+(Dest << 8)+Src1;
       CodeLen=4; ChkSup();
      END
     return;
    END

   /* unbekannter Befehl */

   WrXError(1200,OpPart);
END

        static void InitCode_29K(void)
BEGIN
   SaveInitProc();
   Reg_RBP=0; ClearStringList(&Emulations);
END

        static Boolean IsDef_29K(void)
BEGIN
   return False;
END

        static void SwitchFrom_29K(void)
BEGIN
   DeinitFields(); ClearONOFF();
END

        static void SwitchTo_29K(void)
BEGIN
   TurnWords=True; ConstMode=ConstModeC; SetIsOccupied=False;

   PCSymbol="$"; HeaderID=0x29; NOPCode=0x000000000;
   DivideChars=","; HasAttrs=False;

   ValidSegs=1<<SegCode;
   Grans[SegCode]=1; ListGrans[SegCode]=4; SegInits[SegCode]=0;
#ifdef __STDC__
   SegLimits[SegCode] = 0xfffffffful;
#else
   SegLimits[SegCode] = 0xffffffffl;
#endif

   MakeCode=MakeCode_29K; IsDef=IsDef_29K;
   AddONOFF("SUPMODE", &SupAllowed, SupAllowedName,False);

   SwitchFrom=SwitchFrom_29K; InitFields();
END

        void code29k_init(void)
BEGIN
   CPU29245=AddCPU("AM29245",SwitchTo_29K);
   CPU29243=AddCPU("AM29243",SwitchTo_29K);
   CPU29240=AddCPU("AM29240",SwitchTo_29K);
   CPU29000=AddCPU("AM29000",SwitchTo_29K);

   Emulations=Nil;

   SaveInitProc=InitPassProc; InitPassProc=InitCode_29K;
END

