/* code6816.c */
/*****************************************************************************/
/* AS-Portierung                                                             */
/*                                                                           */
/* Codegeneratormodul CPU16                                                  */
/*                                                                           */
/* Historie: 15.10.1996 Grundsteinlegung                                     */
/*                                                                           */
/*****************************************************************************/

#include "stdinc.h"

#include <string.h>

#include "nls.h"
#include "bpemu.h"
#include "strutil.h"
#include "asmdef.h"
#include "asmsub.h"
#include "asmpars.h"
#include "codepseudo.h"
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
          ShortInt Size;
          Word Code,ExtCode;
          Byte AdrMask,ExtShift;
         } GenOrder;

typedef struct
         {
          char *Name;
          Word Code1,Code2;
         } EmuOrder;

#define FixedOrderCnt 140
#define RelOrderCnt 18
#define LRelOrderCnt 3
#define GenOrderCnt 66
#define AuxOrderCnt 12
#define ImmOrderCnt 4
#define ExtOrderCnt 3
#define EmuOrderCnt 6
#define RegCnt 7

#define ModNone (-1)
#define ModDisp8 0
#define MModDisp8 (1 << ModDisp8)
#define ModDisp16 1
#define MModDisp16 (1 << ModDisp16)
#define ModDispE 2
#define MModDispE (1 << ModDispE)
#define ModAbs 3
#define MModAbs (1 << ModAbs)
#define ModImm 4
#define MModImm (1 << ModImm)
#define ModImmExt 5
#define MModImmExt (1 << ModImmExt)
#define ModDisp20 ModDisp16
#define MModDisp20 MModDisp16
#define ModAbs20 ModAbs
#define MModAbs20 MModAbs

static ShortInt OpSize;
static ShortInt AdrMode;
static Byte AdrPart;
static Byte AdrVals[4];

static LongInt Reg_EK;
static SimpProc SaveInitProc;

static FixedOrder *FixedOrders;
static FixedOrder *RelOrders;
static FixedOrder *LRelOrders;
static GenOrder *GenOrders;
static FixedOrder *AuxOrders;
static FixedOrder *ImmOrders;
static FixedOrder *ExtOrders;
static EmuOrder *EmuOrders;
static char **Regs;

static CPUVar CPU6816;

/*-------------------------------------------------------------------------*/

        static void AddFixed(char *NName, Word NCode)
BEGIN
   if (InstrZ>=FixedOrderCnt) exit(255);
   FixedOrders[InstrZ].Name=NName;
   FixedOrders[InstrZ++].Code=NCode;
END

        static void AddRel(char *NName, Word NCode)
BEGIN
   if (InstrZ>=RelOrderCnt) exit(255);
   RelOrders[InstrZ].Name=NName;
   RelOrders[InstrZ++].Code=NCode;
END

        static void AddLRel(char *NName, Word NCode)
BEGIN
   if (InstrZ>=LRelOrderCnt) exit(255);
   LRelOrders[InstrZ].Name=NName;
   LRelOrders[InstrZ++].Code=NCode;
END

        static void AddGen(char *NName, ShortInt NSize, Word NCode,
                           Word NExtCode, Byte NShift, Byte NMask)
BEGIN
   if (InstrZ>=GenOrderCnt) exit(255);
   GenOrders[InstrZ].Name=NName;
   GenOrders[InstrZ].Code=NCode;
   GenOrders[InstrZ].ExtCode=NExtCode;
   GenOrders[InstrZ].Size=NSize;
   GenOrders[InstrZ].AdrMask=NMask;
   GenOrders[InstrZ++].ExtShift=NShift;
END

        static void AddAux(char *NName, Word NCode)
BEGIN
   if (InstrZ>=AuxOrderCnt) exit(255);
   AuxOrders[InstrZ].Name=NName;
   AuxOrders[InstrZ++].Code=NCode;
END

        static void AddImm(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ImmOrderCnt) exit(255);
   ImmOrders[InstrZ].Name=NName;
   ImmOrders[InstrZ++].Code=NCode;
END

        static void AddExt(char *NName, Word NCode)
BEGIN
   if (InstrZ>=ExtOrderCnt) exit(255);
   ExtOrders[InstrZ].Name=NName; 
   ExtOrders[InstrZ++].Code=NCode;
END

        static void AddEmu(char *NName, Word NCode1, Word NCode2)
BEGIN
   if (InstrZ>=EmuOrderCnt) exit(255);
   EmuOrders[InstrZ].Name=NName;
   EmuOrders[InstrZ].Code1=NCode1;
   EmuOrders[InstrZ++].Code2=NCode2;
END

	static void InitFields(void)
BEGIN
   FixedOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*FixedOrderCnt); InstrZ=0;
   AddFixed("ABA"   ,0x370b); AddFixed("ABX"   ,0x374f);
   AddFixed("ABY"   ,0x375f); AddFixed("ABZ"   ,0x376f);
   AddFixed("ACE"   ,0x3722); AddFixed("ACED"  ,0x3723);
   AddFixed("ADE"   ,0x2778); AddFixed("ADX"   ,0x37cd);
   AddFixed("ADY"   ,0x37dd); AddFixed("ADZ"   ,0x37ed);
   AddFixed("AEX"   ,0x374d); AddFixed("AEY"   ,0x375d);
   AddFixed("AEZ"   ,0x376d); AddFixed("ASLA"  ,0x3704);
   AddFixed("ASLB"  ,0x3714); AddFixed("ASLD"  ,0x27f4);
   AddFixed("ASLE"  ,0x2774); AddFixed("ASLM"  ,0x27b6);
   AddFixed("LSLB"  ,0x3714); AddFixed("LSLD"  ,0x27f4);
   AddFixed("LSLE"  ,0x2774); AddFixed("LSLA"  ,0x3704);
   AddFixed("ASRA"  ,0x370d); AddFixed("ASRB"  ,0x371d);
   AddFixed("ASRD"  ,0x27fd); AddFixed("ASRE"  ,0x277d);
   AddFixed("ASRM"  ,0x27ba); AddFixed("BGND"  ,0x37a6);
   AddFixed("CBA"   ,0x371b); AddFixed("CLRA"  ,0x3705);
   AddFixed("CLRB"  ,0x3715); AddFixed("CLRD"  ,0x27f5);
   AddFixed("CLRE"  ,0x2775); AddFixed("CLRM"  ,0x27b7);
   AddFixed("COMA"  ,0x3700); AddFixed("COMB"  ,0x3710);
   AddFixed("COMD"  ,0x27f0); AddFixed("COME"  ,0x2770);
   AddFixed("DAA"   ,0x3721); AddFixed("DECA"  ,0x3701);
   AddFixed("DECB"  ,0x3711); AddFixed("EDIV"  ,0x3728);
   AddFixed("EDIVS" ,0x3729); AddFixed("EMUL"  ,0x3725);
   AddFixed("EMULS" ,0x3726); AddFixed("FDIV"  ,0x372b);
   AddFixed("FMULS" ,0x3727); AddFixed("IDIV"  ,0x372a);
   AddFixed("INCA"  ,0x3703); AddFixed("INCB"  ,0x3713);
   AddFixed("LPSTOP",0x27f1); AddFixed("LSRA"  ,0x370f);
   AddFixed("LSRB"  ,0x371f); AddFixed("LSRD"  ,0x27ff);
   AddFixed("LSRE"  ,0x277f); AddFixed("MUL"   ,0x3724);
   AddFixed("NEGA"  ,0x3702); AddFixed("NEGB"  ,0x3712);
   AddFixed("NEGD"  ,0x27f2); AddFixed("NEGE"  ,0x2772);
   AddFixed("NOP"   ,0x274c); AddFixed("PSHA"  ,0x3708);
   AddFixed("PSHB"  ,0x3718); AddFixed("PSHMAC",0x27b8);
   AddFixed("PULA"  ,0x3709); AddFixed("PULB"  ,0x3719);
   AddFixed("PULMAC",0x27b9); AddFixed("ROLA"  ,0x370c);
   AddFixed("ROLB"  ,0x371c); AddFixed("ROLD"  ,0x27fc);
   AddFixed("ROLE"  ,0x277c); AddFixed("RORA"  ,0x370e);
   AddFixed("RORB"  ,0x371e); AddFixed("RORD"  ,0x27fe);
   AddFixed("RORE"  ,0x277e); AddFixed("RTI"   ,0x2777);
   AddFixed("RTS"   ,0x27f7); AddFixed("SBA"   ,0x370a);
   AddFixed("SDE"   ,0x2779); AddFixed("SWI"   ,0x3720);
   AddFixed("SXT"   ,0x27f8); AddFixed("TAB"   ,0x3717);
   AddFixed("TAP"   ,0x37fd); AddFixed("TBA"   ,0x3707);
   AddFixed("TBEK"  ,0x27fa); AddFixed("TBSK"  ,0x379f);
   AddFixed("TBXK"  ,0x379c); AddFixed("TBYK"  ,0x379d);
   AddFixed("TBZK"  ,0x379e); AddFixed("TDE"   ,0x277b);
   AddFixed("TDMSK" ,0x372f); AddFixed("TDP"   ,0x372d);
   AddFixed("TED"   ,0x27fb); AddFixed("TEDM"  ,0x27b1);
   AddFixed("TEKB"  ,0x27bb); AddFixed("TEM"   ,0x27b2);
   AddFixed("TMER"  ,0x27b4); AddFixed("TMET"  ,0x27b5);
   AddFixed("TMXED" ,0x27b3); AddFixed("TPA"   ,0x37fc);
   AddFixed("TPD"   ,0x372c); AddFixed("TSKB"  ,0x37af);
   AddFixed("TSTA"  ,0x3706); AddFixed("TSTB"  ,0x3716);
   AddFixed("TSTD"  ,0x27f6); AddFixed("TSTE"  ,0x2776);
   AddFixed("TSX"   ,0x274f); AddFixed("TSY"   ,0x275f);
   AddFixed("TSZ"   ,0x276f); AddFixed("TXKB"  ,0x37ac);
   AddFixed("TXS"   ,0x374e); AddFixed("TXY"   ,0x275c);
   AddFixed("TXZ"   ,0x276c); AddFixed("TYKB"  ,0x37ad);
   AddFixed("TYS"   ,0x375e); AddFixed("TYX"   ,0x274d);
   AddFixed("TYZ"   ,0x276d); AddFixed("TZKB"  ,0x37ae);
   AddFixed("TZS"   ,0x376e); AddFixed("TZX"   ,0x274e);
   AddFixed("TZY"   ,0x275e); AddFixed("WAI"   ,0x27f3);
   AddFixed("XGAB"  ,0x371a); AddFixed("XGDE"  ,0x277a);
   AddFixed("XGDX"  ,0x37cc); AddFixed("XGDY"  ,0x37dc);
   AddFixed("XGDZ"  ,0x37ec); AddFixed("XGEX"  ,0x374c);
   AddFixed("XGEY"  ,0x375c); AddFixed("XGEZ"  ,0x376c);
   AddFixed("DES"   ,0x3fff); AddFixed("INS"   ,0x3f01);
   AddFixed("DEX"   ,0x3cff); AddFixed("INX"   ,0x3c01);
   AddFixed("DEY"   ,0x3dff); AddFixed("INY"   ,0x3d01);
   AddFixed("PSHX"  ,0x3404); AddFixed("PULX"  ,0x3510);
   AddFixed("PSHY"  ,0x3408); AddFixed("PULY"  ,0x3508);

   RelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*RelOrderCnt); InstrZ=0;
   AddRel("BCC", 4); AddRel("BCS", 5); AddRel("BEQ", 7);
   AddRel("BGE",12); AddRel("BGT",14); AddRel("BHI", 2);
   AddRel("BLE",15); AddRel("BLS", 3); AddRel("BLT",13);
   AddRel("BMI",11); AddRel("BNE", 6); AddRel("BPL",10);
   AddRel("BRA", 0); AddRel("BRN", 1); AddRel("BVC", 8);
   AddRel("BVS", 9); AddRel("BHS", 4); AddRel("BLO", 5);

   LRelOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*LRelOrderCnt); InstrZ=0;
   AddLRel("LBEV",0x3791); AddLRel("LBMV",0x3790); AddLRel("LBSR",0x27f9);

   GenOrders=(GenOrder *) malloc(sizeof(GenOrder)*GenOrderCnt); InstrZ=0;
   AddGen("ADCA",0,0x43,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ADCB",0,0xc3,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ADCD",1,0x83,0xffff,0x20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ADCE",1,0x03,0xffff,0x20,          MModImm+           MModDisp16+MModAbs          );
   AddGen("ADDA",0,0x41,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ADDB",0,0xc1,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ADDD",1,0x81,  0xfc,0x20,MModDisp8+MModImm+MModImmExt+MModDisp16+MModAbs+MModDispE);
   AddGen("ADDE",1,0x01,  0x7c,0x20,          MModImm+MModImmExt+MModDisp16+MModAbs          );
   AddGen("ANDA",0,0x46,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ANDB",0,0xc6,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ANDD",1,0x86,0xffff,0x20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ANDE",1,0x06,0xffff,0x20,          MModImm+           MModDisp16+MModAbs          );
   AddGen("ASL" ,0,0x04,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("ASLW",0,0x04,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("LSL" ,0,0x04,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("LSLW",0,0x04,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("ASR" ,0,0x0d,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("ASRW",0,0x0d,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("BITA",0,0x49,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("BITB",0,0xc9,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("CLR" ,0,0x05,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("CLRW",0,0x05,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("CMPA",0,0x48,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("CMPB",0,0xc8,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("COM" ,0,0x00,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("COMW",0,0x00,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("CPD" ,1,0x88,0xffff,0x20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("CPE" ,1,0x08,0xffff,0x20,          MModImm+           MModDisp16+MModAbs          );
   AddGen("DEC" ,0,0x01,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("DECW",0,0x01,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("EORA",0,0x44,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("EORB",0,0xc4,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("EORD",1,0x84,0xffff,0x20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("EORE",1,0x04,0xffff,0x20,          MModImm+           MModDisp16+MModAbs          );
   AddGen("INC" ,0,0x03,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("INCW",0,0x03,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("LDAA",0,0x45,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("LDAB",0,0xc5,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("LDD" ,1,0x85,0xffff,0x20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("LDE" ,1,0x05,0xffff,0x20,          MModImm+           MModDisp16+MModAbs          );
   AddGen("LSR" ,0,0x0f,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("LSRW",0,0x0f,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("NEG" ,0,0x02,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("NEGW",0,0x02,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("ORAA",0,0x47,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ORAB",0,0xc7,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ORD" ,1,0x87,0xffff,0x20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("ORE" ,1,0x07,0xffff,0x20,          MModImm+           MModDisp16+MModAbs          );
   AddGen("ROL" ,0,0x0c,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("ROLW",0,0x0c,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("ROR" ,0,0x0e,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("RORW",0,0x0e,0xffff,0x10,                             MModDisp16+MModAbs          );
   AddGen("SBCA",0,0x42,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("SBCB",0,0xc2,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("SBCD",1,0x82,0xffff,0x20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("SBCE",1,0x02,0xffff,0x20,          MModImm+           MModDisp16+MModAbs          );
   AddGen("STAA",0,0x4a,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs+MModDispE);
   AddGen("STAB",0,0xca,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs+MModDispE);
   AddGen("STD" ,1,0x8a,0xffff,0x20,MModDisp8+                   MModDisp16+MModAbs+MModDispE);
   AddGen("STE" ,1,0x0a,0xffff,0x20,                             MModDisp16+MModAbs          );
   AddGen("SUBA",0,0x40,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("SUBB",0,0xc0,0xffff,0x00,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("SUBD",1,0x80,0xffff,0x20,MModDisp8+MModImm+           MModDisp16+MModAbs+MModDispE);
   AddGen("SUBE",1,0x00,0xffff,0x20,          MModImm+           MModDisp16+MModAbs          );
   AddGen("TST" ,0,0x06,0xffff,0x00,MModDisp8+                   MModDisp16+MModAbs          );
   AddGen("TSTW",0,0x06,0xffff,0x10,                             MModDisp16+MModAbs          );

   AuxOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*AuxOrderCnt); InstrZ=0;
   AddAux("CPS",0x4f); AddAux("CPX",0x4c); AddAux("CPY",0x4d); AddAux("CPZ",0x4e);
   AddAux("LDS",0xcf); AddAux("LDX",0xcc); AddAux("LDY",0xcd); AddAux("LDZ",0xce);
   AddAux("STS",0x8f); AddAux("STX",0x8c); AddAux("STY",0x8d); AddAux("STZ",0x8e);

   ImmOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*ImmOrderCnt); InstrZ=0;
   AddImm("AIS",0x3f); AddImm("AIX",0x3c); AddImm("AIY",0x3d); AddImm("AIZ",0x3e);

   ExtOrders=(FixedOrder *) malloc(sizeof(FixedOrder)*ExtOrderCnt); InstrZ=0;
   AddExt("LDED",0x2771); AddExt("LDHI",0x27b0);AddExt("STED",0x2773);

   EmuOrders=(EmuOrder *) malloc(sizeof(EmuOrder)*EmuOrderCnt); InstrZ=0;
   AddEmu("CLC",0x373a,0xfeff); AddEmu("CLI",0x373a,0xff1f); AddEmu("CLV",0x373a,0xfdff);
   AddEmu("SEC",0x373b,0x0100); AddEmu("SEI",0x373b,0x00e0); AddEmu("SEV",0x373b,0x0200);

   Regs=(char **) malloc(sizeof(char *)*RegCnt);
   Regs[0]="D"; Regs[1]="E"; Regs[2]="X"; Regs[3]="Y";
   Regs[4]="Z"; Regs[5]="K"; Regs[6]="CCR";
END

	static void DeinitFields(void)
BEGIN
   free(FixedOrders);
   free(RelOrders);
   free(LRelOrders);
   free(GenOrders);
   free(AuxOrders);
   free(ImmOrders);
   free(ExtOrders);
   free(EmuOrders);
   free(Regs);
END

/*-------------------------------------------------------------------------*/

typedef enum {ShortDisp,LongDisp,NoDisp} DispType;

	static void ChkAdr(Byte Mask)
BEGIN
   if ((AdrMode!=ModNone) AND ((Mask AND (1 << AdrMode))==0))
    BEGIN
     WrError(1350);
     AdrMode=ModNone; AdrCnt=0;
    END
END

   	static void SplitSize(char *Asc, DispType *Erg)
BEGIN
   if (strlen(Asc)<1) *Erg=NoDisp;
   else if (*Asc=='>') *Erg=LongDisp;
   else if (*Asc=='<') *Erg=ShortDisp;
   else *Erg=NoDisp;
END

	static void DecodeAdr(int Start, int Stop, Boolean LongAdr, Byte Mask)
BEGIN
   Integer V16;
   LongInt V32;
   Boolean OK;
   String s;
   DispType Size;

   AdrMode=ModNone; AdrCnt=0;

   Stop-=Start-1;
   if (Stop<1)
    BEGIN
     WrError(1110); return;
    END

   /* immediate ? */

   if (*ArgStr[Start]=='#')
    BEGIN
     strmaxcpy(s,ArgStr[Start]+1,255); SplitSize(s,&Size);
     switch (OpSize)
      BEGIN
       case -1: WrError(1132); break;
       case 0:
        AdrVals[0]=EvalIntExpression(s,Int8,&OK);
        if (OK)
         BEGIN
          AdrCnt=1; AdrMode=ModImm;
         END
        break;
       case 1:
        V16=EvalIntExpression(s,(Size==ShortDisp)?SInt8:Int16,&OK);
        if ((Size==NoDisp) AND (V16>=-128) AND (V16<=127) AND ((Mask & MModImmExt)!=0))
         Size=ShortDisp;
        if (OK)
         if (Size==ShortDisp)
          BEGIN
           AdrVals[0]=Lo(V16);
           AdrCnt=1; AdrMode=ModImmExt;
          END
         else
          BEGIN
           AdrVals[0]=Hi(V16);
           AdrVals[1]=Lo(V16);
           AdrCnt=2; AdrMode=ModImm;
          END
        break;
       case 2:
        V32=EvalIntExpression(s,Int32,&OK);
        if (OK)
         BEGIN
          AdrVals[0]=(V32 >> 24) & 0xff;
          AdrVals[1]=(V32 >> 16) & 0xff;
          AdrVals[2]=(V32 >>  8) & 0xff;
          AdrVals[3]=V32 & 0xff;
          AdrCnt=4; AdrMode=ModImm;
         END
        break;
      END
     ChkAdr(Mask); return;
    END

   /* zusammengesetzt ? */

   if (Stop==2)
    BEGIN
     AdrPart=0xff;
     if (strcasecmp(ArgStr[Start+1],"X")==0) AdrPart=0x00;
     else if (strcasecmp(ArgStr[Start+1],"Y")==0) AdrPart=0x10;
     else if (strcasecmp(ArgStr[Start+1],"Z")==0) AdrPart=0x20;
     else WrXError(1445,ArgStr[Start+1]);
     if (AdrPart!=0xff)
      if (strcasecmp(ArgStr[Start],"E")==0) AdrMode=ModDispE;
      else
       BEGIN
        SplitSize(ArgStr[Start],&Size);
        if (Size==ShortDisp)
	 V32=EvalIntExpression(ArgStr[Start],UInt8,&OK);
        else if (LongAdr)
         V32=EvalIntExpression(ArgStr[Start],SInt20,&OK);
        else
         V32=EvalIntExpression(ArgStr[Start],SInt16,&OK);
        if (OK)
         BEGIN
          if (Size==NoDisp)
           if ((V32>=0) AND (V32<=255) AND ((Mask & MModDisp8)!=0)) Size=ShortDisp;
          if (Size==ShortDisp)
           BEGIN
            AdrVals[0]=V32 & 0xff;
	    AdrCnt=1; AdrMode=ModDisp8;
           END
          else if (LongAdr)
	   BEGIN
            AdrVals[0]=(V32 >> 16) & 0x0f;
            AdrVals[1]=(V32 >> 8) & 0xff;
            AdrVals[2]=V32 & 0xff;
            AdrCnt=3; AdrMode=ModDisp16;
           END
          else
           BEGIN
            AdrVals[0]=(V32 >> 8) & 0xff;
            AdrVals[1]=V32 & 0xff;
            AdrCnt=2; AdrMode=ModDisp16;
           END
         END
       END
     ChkAdr(Mask); return;
    END

   /* absolut ? */

   else
    BEGIN
     SplitSize(ArgStr[Start],&Size);
     V32=EvalIntExpression(ArgStr[Start],UInt20,&OK);
     if (OK)
      if (LongAdr)
       BEGIN
        AdrVals[0]=(V32 >> 16) & 0xff;
        AdrVals[1]=(V32 >> 8) & 0xff;
        AdrVals[2]=V32 & 0xff;
        AdrMode=ModAbs; AdrCnt=3;
       END
      else
       BEGIN
        if ((V32 >> 16)!=Reg_EK) WrError(110);
        AdrVals[0]=(V32 >> 8) & 0xff;
        AdrVals[1]=V32 & 0xff;
        AdrMode=ModAbs; AdrCnt=2;
       END
     ChkAdr(Mask); return;
    END
END

/*-------------------------------------------------------------------------*/

	static Boolean DecodePseudo(void)
BEGIN
#define ASSUME6816Count 1
   static ASSUMERec ASSUME6816s[ASSUME6816Count]=
	       {{"EK" , &Reg_EK , 0 , 0xff , 0x100}};

   if (Memo("ASSUME"))
    BEGIN
     CodeASSUME(ASSUME6816s,ASSUME6816Count);
     return True;
    END

   return False;
END

	static void MakeCode_6816(void)
BEGIN
   int z,z2;
   Boolean OK;
   Byte Mask;
   LongInt AdrLong;

   CodeLen=0; DontPrint=False; AdrCnt=0; OpSize=(-1);

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
      else
       BEGIN
        BAsmCode[0]=Hi(FixedOrders[z].Code); BAsmCode[1]=Lo(FixedOrders[z].Code);
        CodeLen=2;
       END
      return;
     END

   for (z=0; z<EmuOrderCnt; z++)
    if Memo(EmuOrders[z].Name)
     BEGIN
      if (ArgCnt!=0) WrError(1110);
      else
       BEGIN
        BAsmCode[0]=Hi(EmuOrders[z].Code1); BAsmCode[1]=Lo(EmuOrders[z].Code1);
        BAsmCode[2]=Hi(EmuOrders[z].Code2); BAsmCode[3]=Lo(EmuOrders[z].Code2);
        CodeLen=4;
       END
      return;
     END

   /* Datentransfer */

   for (z=0; z<AuxOrderCnt; z++)
    if (Memo(AuxOrders[z].Name))
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else
       BEGIN
        OpSize=1;
        DecodeAdr(1,ArgCnt,False,(*OpPart=='S'?0:MModImm)+MModDisp8+MModDisp16+MModAbs);
        switch (AdrMode)
         BEGIN
          case ModDisp8:
           BAsmCode[0]=AuxOrders[z].Code+AdrPart;
           BAsmCode[1]=AdrVals[0]; CodeLen=2;
           break;
          case ModDisp16:
           BAsmCode[0]=0x17;
           BAsmCode[1]=AuxOrders[z].Code+AdrPart;
           memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
           break;
          case ModAbs:
           BAsmCode[0]=0x17;
           BAsmCode[1]=AuxOrders[z].Code+0x30;
           memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
           break;
          case ModImm:
           BAsmCode[0]=0x37; BAsmCode[1]=AuxOrders[z].Code+0x30;
           if (*OpPart=='L') BAsmCode[1]-=0x40;
           memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
           break;
         END
       END
      return;
     END

   for (z=0; z<ExtOrderCnt; z++)
    if (Memo(ExtOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        OpSize=1; DecodeAdr(1,1,False,MModAbs);
        switch (AdrMode)
         BEGIN
          case ModAbs:
           BAsmCode[0]=Hi(ExtOrders[z].Code); BAsmCode[1]=Lo(ExtOrders[z].Code);
           memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
           break;
         END
       END
      return;
     END

   if ((Memo("PSHM")) OR (Memo("PULM")))
    BEGIN
     if (ArgCnt<1) WrError(1110);
     else
      BEGIN
       OK=True; Mask=0;
       for (z=1; z<=ArgCnt; z++)
        if (OK)
         BEGIN
          z2=0; NLS_UpString(ArgStr[z]);
	  while ((z2<RegCnt) AND (strcmp(ArgStr[z],Regs[z2])!=0)) z2++;
          if (z2>=RegCnt)
           BEGIN
            WrXError(1445,ArgStr[z]); OK=False;
           END
          else if (Memo("PSHM")) Mask+=(1 << z2);
          else Mask+=(1 << (RegCnt-1-z2));
         END
       if (OK)
        BEGIN
         BAsmCode[0]=0x34+Ord(Memo("PULM")); BAsmCode[1]=Mask;
         CodeLen=2;
        END
      END
     return;
    END

   if ((Memo("MOVB")) OR (Memo("MOVW")))
    BEGIN
     z=Ord(Memo("MOVW"));
     if (ArgCnt==2)
      BEGIN
       DecodeAdr(1,1,False,MModAbs);
       if (AdrMode==ModAbs)
        BEGIN
         memcpy(BAsmCode+2,AdrVals,2);
         DecodeAdr(2,2,False,MModAbs);
         if (AdrMode==ModAbs)
          BEGIN
           memcpy(BAsmCode+4,AdrVals,2);
           BAsmCode[0]=0x37; BAsmCode[1]=0xfe + z; /* ANSI :-0 */
           CodeLen=6;
          END
        END
      END
     else if (ArgCnt!=3) WrError(1110);
     else if (strcasecmp(ArgStr[2],"X")==0)
      BEGIN
       BAsmCode[1]=EvalIntExpression(ArgStr[1],SInt8,&OK);
       if (OK)
        BEGIN
         DecodeAdr(3,3,False,MModAbs);
         if (AdrMode==ModAbs)
          BEGIN
           memcpy(BAsmCode+2,AdrVals,2);
           BAsmCode[0]=0x30+z;
           CodeLen=4;
          END
        END
      END
     else if (strcasecmp(ArgStr[3],"X")==0)
      BEGIN
       BAsmCode[3]=EvalIntExpression(ArgStr[2],SInt8,&OK);
       if (OK)
        BEGIN
         DecodeAdr(1,1,False,MModAbs);
         if (AdrMode==ModAbs)
          BEGIN
           memcpy(BAsmCode+1,AdrVals,2);
           BAsmCode[0]=0x32+z;
           CodeLen=4;
          END
        END
      END
     else WrError(1350);
     return;
    END

   /* Arithmetik */

   for (z=0; z<GenOrderCnt; z++)
    if (Memo(GenOrders[z].Name))
     BEGIN
      if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
      else
       BEGIN
        OpSize=GenOrders[z].Size;
        DecodeAdr(1,ArgCnt,False,GenOrders[z].AdrMask);
        switch (AdrMode)
         BEGIN
          case ModDisp8:
           BAsmCode[0]=GenOrders[z].Code+AdrPart; 
           BAsmCode[1]=AdrVals[0]; CodeLen=2;
           break;
          case ModDisp16:
           BAsmCode[0]=0x17+GenOrders[z].ExtShift;
           BAsmCode[1]=GenOrders[z].Code+(OpSize << 6)+AdrPart;
           memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
           break;
          case ModDispE:
           BAsmCode[0]=0x27; BAsmCode[1]=GenOrders[z].Code+AdrPart;
           CodeLen=2;
           break;
          case ModAbs:
           BAsmCode[0]=0x17+GenOrders[z].ExtShift;
           BAsmCode[1]=GenOrders[z].Code+(OpSize << 6)+0x30;
           memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
           break;
          case ModImm:
           if (OpSize==0)
            BEGIN
             BAsmCode[0]=GenOrders[z].Code+0x30;
             BAsmCode[1]=AdrVals[0]; CodeLen=2;
            END
           else
            BEGIN
             BAsmCode[0]=0x37; BAsmCode[1]=GenOrders[z].Code+0x30;
             memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
            END
           break;
          case ModImmExt:
           BAsmCode[0]=GenOrders[z].ExtCode;
           BAsmCode[1]=AdrVals[0]; CodeLen=2;
           break;
         END
       END
      return;
     END

   for (z=0; z<ImmOrderCnt; z++)
    if (Memo(ImmOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        OpSize=1; DecodeAdr(1,1,False,MModImm+MModImmExt);
        switch (AdrMode)
         BEGIN
          case ModImm:
           BAsmCode[0]=0x37; BAsmCode[1]=ImmOrders[z].Code;
           memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
           break;
          case ModImmExt:
           BAsmCode[0]=ImmOrders[z].Code; BAsmCode[1]=AdrVals[0]; CodeLen=2;
           break;
         END
       END
      return;
     END

   if ((Memo("ANDP")) OR (Memo("ORP")))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       OpSize=1; DecodeAdr(1,1,False,MModImm);
       switch (AdrMode)
        BEGIN
         case ModImm:
          BAsmCode[0]=0x37; BAsmCode[1]=0x3a+Ord(Memo("ORP"));
          memcpy(BAsmCode+2,AdrVals,AdrCnt); CodeLen=2+AdrCnt;
          break;
        END
      END
     return;
    END

   if ((Memo("MAC")) OR (Memo("RMAC")))
    BEGIN
     if (ArgCnt!=2) WrError(1110);
     else
      BEGIN
       z=EvalIntExpression(ArgStr[1],UInt4,&OK);
       if (OK)
        BEGIN
         BAsmCode[1]=EvalIntExpression(ArgStr[2],UInt4,&OK);
         if (OK)
          BEGIN
           BAsmCode[1]+=(z << 4);
           BAsmCode[0]=0x7b+(Ord(Memo("RMAC")) << 7);
           CodeLen=2;
          END
        END
      END
     return;
    END

   /* Bitoperationen */

   if ((Memo("BCLR")) OR (Memo("BSET")))
    BEGIN
     if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
     else
      BEGIN
       OpSize=0; z=Ord(Memo("BSET"));
       DecodeAdr(ArgCnt,ArgCnt,False,MModImm);
       switch (AdrMode)
        BEGIN
         case ModImm:
          Mask=AdrVals[0];
          DecodeAdr(1,ArgCnt-1,False,MModDisp8+MModDisp16+MModAbs);
          switch (AdrMode)
           BEGIN
            case ModDisp8:
             BAsmCode[0]=0x17; BAsmCode[1]=0x08+z+AdrPart;
             BAsmCode[2]=Mask; BAsmCode[3]=AdrVals[0];
             CodeLen=4;
             break;
            case ModDisp16:
             BAsmCode[0]=0x08+z+AdrPart; BAsmCode[1]=Mask;
	     memcpy(BAsmCode+2,AdrVals,AdrCnt);
             CodeLen=2+AdrCnt;
             break;
            case ModAbs:
             BAsmCode[0]=0x38+z; BAsmCode[1]=Mask;
	     memcpy(BAsmCode+2,AdrVals,AdrCnt);
             CodeLen=2+AdrCnt;
             break;
           END
          break;
        END
      END
     return;
    END

   if ((Memo("BCLRW")) OR (Memo("BSETW")))
    BEGIN
     if ((ArgCnt<2) OR (ArgCnt>3)) WrError(1110);
     else
      BEGIN
       OpSize=1; z=Ord(Memo("BSETW"));
       DecodeAdr(ArgCnt,ArgCnt,False,MModImm);
       switch (AdrMode)
        BEGIN
         case ModImm:
          memcpy(BAsmCode+2,AdrVals,AdrCnt);
          DecodeAdr(1,ArgCnt-1,False,MModDisp16+MModAbs);
          switch (AdrMode)
           BEGIN
            case ModDisp16:
             BAsmCode[0]=0x27; BAsmCode[1]=0x08+z+AdrPart;
  	     memcpy(BAsmCode+4,AdrVals,AdrCnt);
             CodeLen=4+AdrCnt;
             break;
            case ModAbs:
             BAsmCode[0]=0x27; BAsmCode[1]=0x38+z;
  	     memcpy(BAsmCode+4,AdrVals,AdrCnt);
             CodeLen=4+AdrCnt;
             break;
           END
        END
      END
     return;
    END

   if ((Memo("BRCLR")) OR (Memo("BRSET")))
    BEGIN
     if ((ArgCnt<3) OR (ArgCnt>4)) WrError(1110);
     else
      BEGIN
       z=Ord(Memo("BRSET"));
       OpSize=0; DecodeAdr(ArgCnt-1,ArgCnt-1,False,MModImm);
       if (AdrMode==ModImm)
        BEGIN
         BAsmCode[1]=AdrVals[0];
         AdrLong=EvalIntExpression(ArgStr[ArgCnt],UInt20,&OK)-EProgCounter()-6;
         if (OK)
          BEGIN
           OK=SymbolQuestionable;
           DecodeAdr(1,ArgCnt-2,False,MModDisp8+MModDisp16+MModAbs);
           switch (AdrMode)
            BEGIN
             case ModDisp8:
              if ((AdrLong>=-128) AND (AdrLong<127))
               BEGIN
                BAsmCode[0]=0xcb-(z << 6)+AdrPart;
                BAsmCode[2]=AdrVals[0];
                BAsmCode[3]=AdrLong & 0xff;
                CodeLen=4;
               END
              else if ((NOT OK) AND ((AdrLong<-0x8000l) OR (AdrLong>0x7fffl))) WrError(1370);
              else
               BEGIN
                BAsmCode[0]=0x0a+AdrPart+z;
                BAsmCode[2]=0;
                BAsmCode[3]=AdrVals[0];
                BAsmCode[4]=(AdrLong >> 8) & 0xff;
	        BAsmCode[5]=AdrLong & 0xff;
	        CodeLen=6;
               END
              break;
             case ModDisp16:
              if ((NOT OK) AND ((AdrLong<-0x8000l) OR (AdrLong>0x7fffl))) WrError(1370);
              else
               BEGIN
                BAsmCode[0]=0x0a+AdrPart+z;
                memcpy(BAsmCode+2,AdrVals,2);
                BAsmCode[4]=(AdrLong >> 8) & 0xff;
	        BAsmCode[5]=AdrLong & 0xff;
	        CodeLen=6;
               END
              break;
             case ModAbs:
              if ((NOT OK) AND ((AdrLong<-0x8000l) OR (AdrLong>0x7fffl))) WrError(1370);
              else
               BEGIN
                BAsmCode[0]=0x3a+z;
                memcpy(BAsmCode+2,AdrVals,2);
                BAsmCode[4]=(AdrLong >> 8) & 0xff;
	        BAsmCode[5]=AdrLong & 0xff;
	        CodeLen=6;
               END
              break;
            END
          END
        END
      END
     return;
    END

   /* Spruenge */

   if ((Memo("JMP")) OR (Memo("JSR")))
    BEGIN
     if ((ArgCnt<1) OR (ArgCnt>2)) WrError(1110);
     else
      BEGIN
       OpSize=1;
       DecodeAdr(1,ArgCnt,True,MModAbs20+MModDisp20);
       switch (AdrMode)
        BEGIN
         case ModAbs20:
          BAsmCode[0]=0x7a+(Ord(Memo("JSR")) << 7);
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
          break;
         case ModDisp20:
          BAsmCode[0]=(Memo("JMP"))?0x4b:0x89;
	  BAsmCode[0]+=AdrPart;
          memcpy(BAsmCode+1,AdrVals,AdrCnt);
          CodeLen=1+AdrCnt;
          break;
        END
      END
     return;
    END

   for (z=0; z<RelOrderCnt; z++)
    if ((Memo(RelOrders[z].Name)) OR ((*OpPart=='L') AND (strcmp(OpPart+1,RelOrders[z].Name)==0)))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK)-EProgCounter()-6;
        if ((AdrLong&1)==1) WrError(1325);
        else if (*OpPart=='L')
         if ((NOT SymbolQuestionable) AND ((AdrLong>0x7fffl) OR (AdrLong<-0x8000l))) WrError(1370);
         else
          BEGIN
           BAsmCode[0]=0x37; BAsmCode[1]=RelOrders[z].Code+0x80;
           BAsmCode[2]=(AdrLong >> 8) & 0xff;
           BAsmCode[3]=AdrLong & 0xff;
           CodeLen=4;
          END
        else
         if ((NOT SymbolQuestionable) AND ((AdrLong>0x7fl) OR (AdrLong<-0x80l))) WrError(1370);
         else
          BEGIN
           BAsmCode[0]=0xb0+RelOrders[z].Code;
           BAsmCode[1]=AdrLong & 0xff;
           CodeLen=2;
          END
       END
      return;
     END

   for (z=0; z<LRelOrderCnt; z++)
    if (Memo(LRelOrders[z].Name))
     BEGIN
      if (ArgCnt!=1) WrError(1110);
      else
       BEGIN
        AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK)-EProgCounter()-6;
        if ((AdrLong&1)==1) WrError(1325);
        else if ((NOT SymbolQuestionable) AND ((AdrLong>0x7fffl) OR (AdrLong<-0x8000l))) WrError(1370);
        else
         BEGIN
          BAsmCode[0]=Hi(LRelOrders[z].Code); BAsmCode[1]=Lo(LRelOrders[z].Code);
          BAsmCode[2]=(AdrLong >> 8) & 0xff;
          BAsmCode[3]=AdrLong & 0xff;
          CodeLen=4;
         END
       END
      return;
     END

   if (Memo("BSR"))
    BEGIN
     if (ArgCnt!=1) WrError(1110);
     else
      BEGIN
       AdrLong=EvalIntExpression(ArgStr[1],UInt24,&OK)-EProgCounter()-6;
       if ((AdrLong&1)==1) WrError(1325);
       else if ((NOT SymbolQuestionable) AND ((AdrLong>0x7fl) OR (AdrLong<-0x80l))) WrError(1370);
       else
        BEGIN
         BAsmCode[0]=0x36;
         BAsmCode[1]=AdrLong & 0xff;
         CodeLen=2;
        END
      END
     return;
    END

   WrXError(1200,OpPart);
END

	static void InitCode_6816(void)
BEGIN
   SaveInitProc();
   Reg_EK=0;
END

	static Boolean ChkPC_6816(void)
BEGIN
   if (ActPC==SegCode) return (ProgCounter()<0x100000);
   else return False;
END

	static Boolean IsDef_6816(void)
BEGIN
   return False;
END

        static void SwitchFrom_6816(void)
BEGIN
   DeinitFields();
END

	static void SwitchTo_6816(void)
BEGIN
   TurnWords=False; ConstMode=ConstModeMoto; SetIsOccupied=False;

   PCSymbol="*"; HeaderID=0x65; NOPCode=0x274c;
   DivideChars=","; HasAttrs=False;

   ValidSegs=(1<<SegCode);
   Grans[SegCode]=1; ListGrans[SegCode]=1; SegInits[SegCode]=0;

   MakeCode=MakeCode_6816; ChkPC=ChkPC_6816; IsDef=IsDef_6816;
   SwitchFrom=SwitchFrom_6816;

   InitFields();
END

	void code6816_init(void)
BEGIN
   CPU6816=AddCPU("68HC16",SwitchTo_6816);

   SaveInitProc=InitPassProc; InitPassProc=InitCode_6816;
END
